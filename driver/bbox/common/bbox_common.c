/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * Description:
 * Author: huawei
 * Create: 2022-08-13
 */

#include "bbox_common.h"
#include <linux/slab.h>
#include <linux/securec.h>
#ifndef AOS_LLVM_BUILD
#include <linux/securectype.h>
#endif
#include <linux/time.h>
#include <linux/uaccess.h>
#include <linux/rtc.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#if (defined BBOX_SOC_PLATFORM_MDC) && (!defined BBOX_UT)
#include <linux/cpumask.h>
#include "linux/firmware.h"
#include <linux/aos/cpu_domain_info.h>
#include "bbox_platform.h"
#include "drv_cpu_type.h"
#endif
#ifdef CFG_FEATURE_MDC_CLOCK_CONFIG
#include <linux/virt_wall_time.h>
#include "bootparam/bbox_boot_param.h"
#endif
#include "device/bbox_types.h"
#include "bbox_print.h"

#ifdef AOS_LLVM_BUILD
#include <linux/ktime.h>
#include <linux/timecounter.h>
void bbox_get_safe_systime(struct bbox_time *tm)
{
    if (tm != NULL) {
        const u64 nsec_per_sec = 1000000000ULL;
        ktime_t nsec = ktime_get_real();
        u64 ns = ktime_to_ns(nsec);
        tm->tv_sec = ns / nsec_per_sec;
        tm->tv_nsec = ns % nsec_per_sec;
    }
}

void bbox_get_systime(struct bbox_time *tm)
{
    bbox_get_safe_systime(tm);
}

void bbox_get_sysdate(const struct bbox_time *tm, char *date, u32 len)
{
    s32 ret;
    struct rtc_time time;

    BB_CHECK_PTR(tm, return, "Invalid param, time is NULL.\n");
    BB_CHECK_PTR(date, return, "Invalid param, date is NULL.\n");
    BB_CHECK_EXP_ACT((len == 0), return, "Invalid param, date len is %u.\n", len);

    ret = memset_s(&time, sizeof(struct rtc_time), 0, sizeof(struct rtc_time));
    BB_CHECK_SECUREC(BB_CHECK_MEMSET, ret);

    rtc_time_to_tm((s64)tm->tv_sec, &time);
    ret = sprintf_s(date, len, "%04ld-%02d-%02d-%02d:%02d:%02d.%06llu",
                    time.tm_year + TWENTY_CENTURY, time.tm_mon + JANUARY, time.tm_mday,
                    time.tm_hour, time.tm_min, time.tm_sec, tm->tv_nsec / KILO);
    BB_CHECK_SPRINTF(ret, return);
}

void bbox_get_date(const struct bbox_time *tm, char *date, u32 len)
{
    s32 ret;
    struct rtc_time time;
    char databuf[DATA_MAXLEN] = {0};

    BB_CHECK_PTR(tm, return, "Invalid param, time is NULL.\n");
    BB_CHECK_PTR(date, return, "Invalid param, date is NULL.\n");

    ret = memset_s(&time, sizeof(struct rtc_time), 0, sizeof(struct rtc_time));
    BB_CHECK_SECUREC(BB_CHECK_MEMSET, ret);

    rtc_time_to_tm((s64)tm->tv_sec, &time);
    ret = sprintf_s(databuf, DATA_MAXLEN, "%04ld%02d%02d%02d%02d%02d",
                    time.tm_year + TWENTY_CENTURY, time.tm_mon + JANUARY, time.tm_mday,
                    time.tm_hour, time.tm_min, time.tm_sec);
    BB_CHECK_SPRINTF(ret, return);
    ret = sprintf_s(date, len, "%s-%06llu", databuf, tm->tv_nsec / KILO);
    BB_CHECK_SPRINTF(ret, return);
}

u32 bbox_get_device_num(void)
{
    return DEVICE_NUM;
}
#else
/*
 * @brief       : get the current utc time
 * @param [in]  : struct bbox_time *tm       time
 * @return      : NA
 */
void bbox_get_systime(struct bbox_time *tm)
{
    s32 ret;
    struct timespec64 tv;

    BB_CHECK_PTR(tm, return, "invalid param, time is NULL.\n");
    ret = memset_s(&tv, sizeof(struct timespec64), 0, sizeof(struct timespec64));
    BB_CHECK_SECUREC(BB_CHECK_MEMSET, ret);
#ifdef CFG_FEATURE_MDC_CLOCK_CONFIG
    if (bbox_use_real_dpclk()) {
        ktime_get_real_ts64(&tv);
    } else {
        ktime_get_virtual_ts64(&tv);
    }
#else
    ktime_get_real_ts64(&tv);
#endif

    tm->tv_sec = (u64)tv.tv_sec;
    tm->tv_nsec = (u64)tv.tv_nsec;
    return;
}

/*
 * @brief       : get the current utc time, NMI safe
 * @param [in]  : struct bbox_time *tm       time
 * @return      : NA
 */
void bbox_get_safe_systime(struct bbox_time *tm)
{
    if (tm != NULL) {
        const u64 nsec_per_sec = 1000000000ULL;
        u64 nsec = ktime_get_real_fast_ns();
        tm->tv_sec = nsec / nsec_per_sec;
        tm->tv_nsec = nsec % nsec_per_sec;
    }
}

/*
 * @brief       : get the date of the time, use in device
 * @param [in]  : const struct bbox_time *tm    time
 * @param [in]  : char *date                    date array
 * @param [in]  : u32 len                       date array length
 * @return      : NA
 */
void bbox_get_sysdate(const struct bbox_time *tm, char *date, u32 len)
{
    s32 ret;
    struct tm time;

    BB_CHECK_PTR(tm, return, "invalid param, time is NULL.\n");
    BB_CHECK_PTR(date, return, "invalid param, date is NULL.\n");
    BB_CHECK_EXP_ACT((len == 0), return, "invalid param, date len is %u.\n", len);

    ret = memset_s(&time, sizeof(struct rtc_time), 0, sizeof(struct rtc_time));
    BB_CHECK_SECUREC(BB_CHECK_MEMSET, ret);

    time64_to_tm((s64)tm->tv_sec, 0, &time);
    ret = sprintf_s(date, len, "%04ld-%02d-%02d-%02d:%02d:%02d.%06llu",
                    time.tm_year + TWENTY_CENTURY, time.tm_mon + JANUARY, time.tm_mday,
                    time.tm_hour, time.tm_min, time.tm_sec, tm->tv_nsec / KILO);
    BB_CHECK_SPRINTF(ret, return);
}

/*
 * @brief       : get the date of the time, use for exception info
 * @param [in]  : const struct bbox_time *tm    time
 * @param [in]  : char *date                    date array
 * @param [in]  : u32 len                       date array length
 * @return      : NA
 */
void bbox_get_date(const struct bbox_time *tm, char *date, u32 len)
{
    s32 ret;
    struct tm time;
    char databuf[DATA_MAXLEN] = {0};

    BB_CHECK_PTR(tm, return, "invalid param, time is NULL.\n");
    BB_CHECK_PTR(date, return, "invalid param, date is NULL.\n");

    ret = memset_s(&time, sizeof(struct rtc_time), 0, sizeof(struct rtc_time));
    BB_CHECK_SECUREC(BB_CHECK_MEMSET, ret);

    time64_to_tm((s64)tm->tv_sec, 0, &time);
    ret = sprintf_s(databuf, DATA_MAXLEN, "%04ld%02d%02d%02d%02d%02d",
                    time.tm_year + TWENTY_CENTURY, time.tm_mon + JANUARY, time.tm_mday,
                    time.tm_hour, time.tm_min, time.tm_sec);
    BB_CHECK_SPRINTF(ret, return);
    ret = sprintf_s(date, len, "%s-%06llu", databuf, tm->tv_nsec / KILO);
    BB_CHECK_SPRINTF(ret, return);
}

static u32 g_time_sequence = 0;
static DEFINE_SPINLOCK(g_time_sequence_lock);

/*
 * @brief       : get time sequence
 * @return      : int time sequence
 */
u32 bbox_get_time_seq(void)
{
    lock_flag_t flags = 0;
    u32 time_seq;
    spin_lock_irqsave(&g_time_sequence_lock, flags);
    time_seq = g_time_sequence;
    g_time_sequence++;
    g_time_sequence = g_time_sequence % KILO;
    spin_unlock_irqrestore(&g_time_sequence_lock, flags);
    return time_seq;
}

/*
 * @brief       : update time->tv_nsec with time sequence
 * @param [in]  : struct struct bbox_time *tm     timestamp
 * @return      : NA
 */
void bbox_update_time_seq(struct bbox_time *tm)
{
    BB_CHECK_PTR(tm, return, "invalid param, time is NULL.\n");

    if ((tm->tv_nsec % KILO) != 0) {
        BB_PRINT_INFO("time sequence has been set : %llu.\n", tm->tv_nsec % KILO);
        return;
    }

    tm->tv_nsec += bbox_get_time_seq();
    return;
}
#endif

/*
 * @brief       : kmalloc buffer, then zero it, limit size in 0x0-0x8000000
 * @param [in]  : size_t size       buffer size
 * @return      : buffer point
 */
void *bbox_kmalloc(size_t size)
{
    s32 ret;
    void *buffer = NULL;

    if ((size == 0) || (size > BBOX_MALLOC_MAX_SIZE)) {
        BB_PRINT_INFO("invalid malloc size [0x%lx].\n", size);
        return NULL;
    }

    buffer = kmalloc(size, GFP_ATOMIC);
    if (buffer == NULL) {
        return NULL;
    }

    ret = memset_s(buffer, size, 0, size);
    if (ret != EOK) {
        kfree(buffer);
        return NULL;
    }
    return buffer;
}

void bbox_kfree(void *buffer)
{
    if (buffer != NULL) {
        kfree(buffer);
    }
}

/*
 * @brief       : vmalloc buffer, then zero it, limit size in 0x0-0x8000000
 * @param [in]  : size_t size       buffer size
 * @return      : NULL: failure; buffer point: success
 */
void *bbox_vmalloc(size_t size)
{
    s32 ret;
    void *buffer = NULL;

    if ((size == 0) || (size > BBOX_MALLOC_MAX_SIZE)) {
        BB_PRINT_INFO("invalid malloc size [0x%lx]\n", size);
        return NULL;
    }

    buffer = vmalloc(size);
    if (buffer == NULL) {
        return NULL;
    }

    ret = memset_s(buffer, size, 0, size);
    if (ret != EOK) {
        vfree(buffer);
        return NULL;
    }
    return buffer;
}

void bbox_vfree(void *buffer)
{
    if (buffer != NULL) {
        vfree(buffer);
    }
}

/*
 * @brief       : map physics add to virtual add
 * @param [in]  : phys_addr_t paddr     physics add
 * @param [in]  : size_t size           buffer size
 * @return      : virtual add
 */
void *bbox_ioremap(u64 paddr, u64 size)
{
    return ioremap_wc((phys_addr_t)paddr, (size_t)size);
}

/*
 * @brief       : unmap virtual add
 * @param [in]  : const void *vaddr     virtual add
 * @return      : NA
 */
void bbox_iounmap(const void *vaddr)
{
    if (vaddr != NULL) {
        iounmap((void __iomem *)vaddr);
    }
}

/*
 * @brief       : transfer string to int
 * @param [in]  : const char *s     string
 * @return      : u32               int
 */
s32 bbox_strtouint_h(const char *s, u32 *val)
{
    char c = 0;
    u64 ret = 0;
    const char *p = s;

    if ((s == NULL) || (val == NULL)) {
        return BBOX_FAILURE;
    }
    *val = 0;
    if ((strncmp(s, "0x", FORMAT_HEAD_LEN) == 0) || (strncmp(s, "0X", FORMAT_HEAD_LEN) == 0)) {
        p = &p[FORMAT_HEAD_LEN];
    }
    c = *p;
    while (c != '\0') {
        if (((c >= '0') && (c <= '9')) || ((c >= 'a') && (c <= 'f')) || ((c >= 'A') && (c <= 'F'))) {
            s32 num = (bbox_isdigit(c) ? (c - '0') : ((TOLOWER(c) - 'a') + FORMAT_DEC));
            ret *= FORMAT_HEX;
            ret += (u32)num;
            if (ret > ((u32)~0U)) {
                return BBOX_FAILURE;
            }
        } else {
            break;
        }
        p++;
        c = *p;
    }
    *val = (u32)ret;
    return BBOX_SUCCESS;
}

/*
 * @brief       : transfer string to u64
 * @param [in]  : const char *s     string
 * @return      : u64               val
 */
s32 bbox_strtol_h(const char *s, u64 *val)
{
    char c;
    u64 ret = 0;
    const char *p = s;

    if ((s == NULL) || (val == NULL)) {
        return BBOX_FAILURE;
    }
    *val = 0;
    if ((strncmp(s, "0x", FORMAT_HEAD_LEN) == 0) || (strncmp(s, "0X", FORMAT_HEAD_LEN) == 0)) {
        p = &p[FORMAT_HEAD_LEN];
    }

    if (strlen(p) > STRTOL_LEN) {
        return BBOX_FAILURE;
    }
    c = *p;
    while (c != '\0') {
        if (((c >= '0') && (c <= '9')) || ((c >= 'a') && (c <= 'f')) || ((c >= 'A') && (c <= 'F'))) {
            s32 num = (bbox_isdigit(c) ? (c - '0') : ((TOLOWER(c) - 'a') + FORMAT_DEC));
            ret *= FORMAT_HEX;
            ret += (u32)num;
        } else {
            break;
        }
        p++;
        c = *p;
    }
    *val = ret;
    return BBOX_SUCCESS;
}

#if (defined BBOX_SOC_PLATFORM_MDC) && (!defined BBOX_UT)
/*
 * @brief       : Copy memory in batches due to performance problems
 * @param [in]  : void *dest     dest addr
 * @param [in]  : char *src      source addr
 * @param [in]  : u32 len        copy length
 * @return      : s32            return value
 */
s32 bbox_copy_exception_msg(void *dest, const char *src, u32 len)
{
    u32 lastmem = len;
    u32 cpylen;
    char *tempsrc = (char *)src;
    void *tempdest = dest;
    s32 ret;

    while (lastmem > 0) {
        cpylen = (lastmem > BBOX_MEM_CPY_LEN) ? BBOX_MEM_CPY_LEN : lastmem;
        lastmem -= cpylen;
        ret = memcpy_s(tempdest, cpylen, (const void *)tempsrc, cpylen);
        if (ret != EOK) {
            BB_PRINT_ERR("[%s][%4d] memcpy_s failed.\n", __func__, __LINE__);
            return ret;
        }
        tempsrc += cpylen;
        tempdest += cpylen;
        usleep_range(BBOX_WAIT_MIN_TIME, BBOX_WAIT_MAX_TIME);
    }

    return BBOX_SUCCESS;
}

STATIC cpumask_t g_ctrlcpumask;

STATIC void bbox_bind_to_ctrl_cpu(struct task_struct *thread)
{
    if (cpumask_available(&g_ctrlcpumask)) {
        /* set cpumask */
        set_cpus_allowed_ptr(thread, &g_ctrlcpumask);
    }
}

s32 bbox_get_ctrlcpu_mask(void)
{
    return drv_get_ctrlcpu_mask(&g_ctrlcpumask);
}
#endif

struct task_struct *bbox_kthread_proc(pkthreadFunc func, void *data, const char *threadname)
{
#if (defined BBOX_SOC_PLATFORM_MDC) && (!defined BBOX_UT)
    struct task_struct *threadstruct;
    threadstruct = kthread_create(func, data, "%s", threadname);
    if (threadstruct == NULL) {
        BB_PRINT_ERR("Create thread failed. (threadname = %s)\n", threadname);
        return NULL;
    }
    bbox_bind_to_ctrl_cpu(threadstruct);
    (void)wake_up_process(threadstruct);
    return threadstruct;
#else
    return kthread_run(func, data, "%s", threadname);
#endif
}

