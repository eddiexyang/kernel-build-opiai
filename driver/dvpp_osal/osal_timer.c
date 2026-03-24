#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/timer.h>
#include <linux/time.h>
#include <linux/time64.h>
#include <linux/version.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/rtc.h>
#include <linux/sched/clock.h>
#include <linux/jiffies.h>
#include <linux/fs.h>
#include <linux/stat.h>
#include "hi_osal.h"

#ifdef DVPP_MINI_V2_MDC
#include <linux/virt_wall_time.h>
#endif

#define NS_PER_US 1000
#ifdef AOS_LLVM_BUILD
#include <linux/export.h>
#include <linux/timecounter.h>
#endif // #ifdef AOS_LLVM_BUILD

#define BOOTARGS_FILE_PATH      "/sys/firmware/devicetree/base/chosen/bootargs"
#define TMIESTAMP_TYPE_DPCLK0   0U
#define TMIESTAMP_TYPE_DPCLK100 100U
#define DPCLK100      "dpclk=100"
#define BOOTARGS_MAX_SIZE       2000U
struct timer_list_info {
    struct timer_list time_list;
    hi_ulong data;
};
typedef void (*timer_callback_fun)(struct timer_list * data);

static hi_u32 g_timestamp_type = TMIESTAMP_TYPE_DPCLK0;

hi_ulong osal_timer_get_private_data(const void *data)
{
    const struct timer_list_info *list_info = osal_container_of(data, struct timer_list_info, time_list);

    return list_info->data;
}
EXPORT_SYMBOL(osal_timer_get_private_data);

hi_s32 osal_timer_init(osal_timer_t *timer)
{
    struct timer_list_info *t = NULL;

    if (timer == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }

    t = (struct timer_list_info *)osal_kmalloc(sizeof(struct timer_list_info), osal_gfp_kernel);
    if (t == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "kmalloc error!\n");
        return -1;
    }

    t->data = timer->data;
    timer_setup(&t->time_list, (timer_callback_fun)timer->function, 0);
    timer->timer = t;
    return 0;
}
EXPORT_SYMBOL(osal_timer_init);

hi_s32 osal_set_timer(const osal_timer_t *timer, hi_ulong interval)
{
    struct timer_list_info *list_info = NULL;
    if ((timer == NULL) || (timer->timer == NULL) || (timer->function == NULL) || (interval == 0)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }
    list_info = (struct timer_list_info *)timer->timer;
    list_info->data = timer->data;
    list_info->time_list.function = (timer_callback_fun)timer->function;
    return mod_timer(&list_info->time_list, jiffies + msecs_to_jiffies(interval) - 1U);
}
EXPORT_SYMBOL(osal_set_timer);

// 注意：add_timer_on只能在timer未启动或时间到期时调用，否则会panic
hi_void osal_set_timer_on_cpu(const osal_timer_t *timer, hi_ulong interval)
{
    struct timer_list_info *list_info = NULL;
    if ((timer == NULL) || (timer->timer == NULL) || (timer->function == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return;
    }
    list_info = (struct timer_list_info *)timer->timer;
    list_info->data = timer->data;
    list_info->time_list.function = (timer_callback_fun)timer->function;
    list_info->time_list.expires = jiffies + msecs_to_jiffies(interval) - 1U;
    add_timer_on(&list_info->time_list, (hi_s32)timer->bind_cpu_id);
}
EXPORT_SYMBOL(osal_set_timer_on_cpu);

hi_s32 osal_del_timer(const osal_timer_t *timer)
{
    struct timer_list *t = NULL;
    if ((timer == NULL) || (timer->timer == NULL) || (timer->function == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, " parameter invalid!\n");
        return -1;
    }
    t = timer->timer;
    return del_timer_sync(t);
}
EXPORT_SYMBOL(osal_del_timer);

hi_s32 osal_timer_destroy(osal_timer_t *timer)
{
    struct timer_list *t = NULL;
    if (timer == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, " parameter invalid!\n");
        return -1;
    }
    t = timer->timer;
    (void)del_timer_sync(t);
    osal_kfree(t);
    timer->timer = NULL;
    return 0;
}
EXPORT_SYMBOL(osal_timer_destroy);

hi_s32 osal_timer_destory(osal_timer_t *timer)
__attribute__((alias("osal_timer_destroy")));
EXPORT_SYMBOL(osal_timer_destory);

hi_s32 osal_stop_timer(const osal_timer_t *timer)
{
    struct timer_list *t = NULL;
    if ((timer == NULL) || (timer->timer == NULL) || (timer->function == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }
    t = timer->timer;
    return del_timer(t);
}
EXPORT_SYMBOL(osal_stop_timer);

hi_ulong osal_msleep(hi_u32 msecs)
{
    return msleep_interruptible(msecs);
}
EXPORT_SYMBOL(osal_msleep);

void osal_msleep_uninterrupt(hi_u32 msecs)
{
    msleep(msecs);
}
EXPORT_SYMBOL(osal_msleep_uninterrupt);

void osal_udelay(hi_u32 usecs)
{
    udelay(usecs);
}
EXPORT_SYMBOL(osal_udelay);

hi_u32 osal_get_tickcount(void)
{
    return jiffies_to_msecs(jiffies);
}
EXPORT_SYMBOL(osal_get_tickcount);

hi_u64 osal_sched_clock(void)
{
    return sched_clock();
}
EXPORT_SYMBOL(osal_sched_clock);

hi_u64 osal_asm_get_clock(void)
{
    const hi_u64 TICK_FREQ = 26; // one syscnt = 625/24 ns
    hi_u64 tick;
    hi_u64 ns;
    isb();
#ifndef DVPP_VCAST_UTST
    asm volatile("mrs %0, cntvct_el0" : "=r" (tick) :: "memory");
#endif // #ifndef DVPP_VCAST_UTST
    ns = tick * TICK_FREQ;

    return ns;
}
EXPORT_SYMBOL(osal_asm_get_clock);

void osal_get_real_ts64(osal_timespec64_t *tv)
{
    struct timespec64 ts;
    if (tv == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return;
    }
#ifdef DVPP_MINI_V2_MDC
    if (g_timestamp_type == TMIESTAMP_TYPE_DPCLK100) {
        ktime_get_virtual_ts64(&ts);
    } else {
        ktime_get_real_ts64(&ts);
    }
#else
    ktime_get_real_ts64(&ts);
#endif
    tv->tv_sec = ts.tv_sec;
    tv->tv_nsec = ts.tv_nsec;
}
EXPORT_SYMBOL(osal_get_real_ts64);

void osal_gettimeofday(osal_timeval_t *tv)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)) || defined AOS_LLVM_BUILD
    struct timeval t;
    if (tv == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return;
    }
    do_gettimeofday(&t);

    tv->tv_sec = t.tv_sec;
    tv->tv_usec = t.tv_usec;
#else
    osal_timespec64_t t;
    if (tv == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return;
    }
    osal_get_real_ts64(&t);
    tv->tv_sec = t.tv_sec;
    tv->tv_usec = t.tv_nsec / NS_PER_US;
#endif
}
EXPORT_SYMBOL(osal_gettimeofday);

void osal_rtc_time_to_tm(hi_ulong local_time, osal_rtc_time_t *tm)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 10, 0)) || defined AOS_LLVM_BUILD
    struct rtc_time rtc_tm = { 0 };
    if (tm == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return;
    }

    rtc_time_to_tm(local_time, &rtc_tm);

    tm->tm_sec = rtc_tm.tm_sec;
    tm->tm_min = rtc_tm.tm_min;
    tm->tm_hour = rtc_tm.tm_hour;
    tm->tm_mday = rtc_tm.tm_mday;
    tm->tm_mon = rtc_tm.tm_mon;
    tm->tm_year = rtc_tm.tm_year;
    tm->tm_wday = rtc_tm.tm_wday;
    tm->tm_yday = rtc_tm.tm_yday;
    tm->tm_isdst = rtc_tm.tm_isdst;
#else
    struct tm rtc_tm = { 0 };
    if (tm == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return;
    }

    time64_to_tm(local_time, 0, &rtc_tm);

    tm->tm_sec = rtc_tm.tm_sec;
    tm->tm_min = rtc_tm.tm_min;
    tm->tm_hour = rtc_tm.tm_hour;
    tm->tm_mday = rtc_tm.tm_mday;
    tm->tm_mon = rtc_tm.tm_mon;
    tm->tm_year = rtc_tm.tm_year;
    tm->tm_wday = rtc_tm.tm_wday;
    tm->tm_yday = rtc_tm.tm_yday;
#endif
}
EXPORT_SYMBOL(osal_rtc_time_to_tm);

void osal_ktime_get_raw_ts64(osal_timespec64_t *tv)
{
    struct timespec64 t;
    if (tv == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return;
    }
    ktime_get_raw_ts64(&t);

    tv->tv_sec = t.tv_sec;
    tv->tv_nsec = t.tv_nsec;
}
EXPORT_SYMBOL(osal_ktime_get_raw_ts64);

hi_u64 osal_asm_get_sys_cnt(void)
{
    hi_u64 syscnt;
    isb();
#ifndef DVPP_VCAST_UTST
    asm volatile("mrs %0, cntvct_el0" : "=r" (syscnt) :: "memory");
#endif // #ifndef DVPP_VCAST_UTST

    return syscnt;
}
EXPORT_SYMBOL(osal_asm_get_sys_cnt);

void osal_timer_get_timestamp_type(hi_void)
{
#ifndef AOS_LLVM_BUILD
    struct file *src_filp = NULL;
    struct kstat src_stat = {0};
    char *bootargs = NULL;
    const char *target = NULL;
    ssize_t bootargs_size;
    loff_t offset = 0;
    hi_s32 ret;

    src_filp = filp_open(BOOTARGS_FILE_PATH, O_RDONLY, S_IRUSR);
    if (IS_ERR(src_filp)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "unable to open file: %s, errno = %ld.\n", BOOTARGS_FILE_PATH, PTR_ERR(src_filp));
        return;
    }

    ret = vfs_getattr(&src_filp->f_path, &src_stat, STATX_BASIC_STATS, AT_NO_AUTOMOUNT);
    if ((ret != 0) || (src_stat.size <= 0)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "vfs_getattr failed, file: %s, src_stat.size = %lld, ret = %d.\n",
            BOOTARGS_FILE_PATH, src_stat.size, ret);
        goto error_getattr_failed;
    }

    if (src_stat.size <= BOOTARGS_MAX_SIZE) {
        bootargs = osal_kmalloc((hi_ulong)src_stat.size, osal_gfp_kernel);
        if (bootargs == NULL) {
            HI_TRACE_OSAL(HI_DBG_ERR, "kmalloc failed, size=%lld\n", src_stat.size);
            goto error_getattr_failed;
        }
    } else {
        HI_TRACE_OSAL(HI_DBG_ERR, "kmalloc size=%lld is larger than threshold:%u\n",
            src_stat.size, BOOTARGS_MAX_SIZE);
        goto error_getattr_failed;
    }

    bootargs_size = kernel_read(src_filp, bootargs, (size_t)src_stat.size, &offset);
    if (bootargs_size != src_stat.size) {
        HI_TRACE_OSAL(HI_DBG_ERR, "kernel_read failed, out size=%ld, file size=%lld\n", bootargs_size, src_stat.size);
        goto error_read_failed;
    }

    target = strstr(bootargs, DPCLK100);
    if (target != NULL) {
        g_timestamp_type = TMIESTAMP_TYPE_DPCLK100;
    }

error_read_failed:
    kfree(bootargs);
error_getattr_failed:
    (void)filp_close(src_filp, NULL);
#endif
}