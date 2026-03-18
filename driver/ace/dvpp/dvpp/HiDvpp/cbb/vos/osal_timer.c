/*
* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
*/

#include "hi_osal.h"
#include "Os.h"
struct timer_info {
    AlarmType alarm_type;
    hi_ulong data;
};

hi_ulong osal_timer_get_private_data(const void *data)
{
    const struct timer_info *list_info = osal_container_of(data, struct timer_info, alarm_type);

    return list_info->data;
}

hi_s32 osal_timer_init(osal_timer_t *timer)
{
    struct timer_info *t = NULL;

    if (timer == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "init timer: timer can not be null!\n");
        return -1;
    }

    AlarmType alarm_type = *((AlarmType *)timer->timer);
    t = (struct timer_info *)osal_kmalloc(sizeof(struct timer_info), osal_gfp_kernel);
    if (t == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "init timer: kmalloc error while allocating timer_info!\n");
        return -1;
    }

    t->data = timer->data;
    t->alarm_type = alarm_type;
    timer->timer = t;
    return 0;
}

hi_s32 osal_set_timer(const osal_timer_t *timer, hi_ulong interval)
{
    struct timer_info *list_info = NULL;
    if ((timer == NULL) || (timer->timer == NULL) || (interval == 0)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }

    list_info = (struct timer_info *)timer->timer;
    list_info->data = timer->data;
    AlarmType alarm_type = list_info->alarm_type;
    return SetRelAlarm(alarm_type, interval, 0);
}

hi_void osal_set_timer_on_cpu(const osal_timer_t *timer, hi_ulong interval)
{
    HI_TRACE_OSAL(HI_DBG_WARN, "not support osal_set_timer_on_cpu!\n");
    return;
}

hi_s32 osal_del_timer(const osal_timer_t *timer)
{
    struct timer_info *list_info = NULL;
    if ((timer == NULL) || (timer->timer == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, " parameter invalid!\n");
        return -1;
    }
    list_info = (struct timer_info *)timer->timer;
    // 反复CancelAlarm同一个type会报错，此处忽略返回值
    (void)CancelAlarm(list_info->alarm_type);
    return 0;
}

hi_s32 osal_timer_destroy(osal_timer_t *timer)
{
    struct timer_info *list_info = NULL;
    if (timer == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, " parameter invalid!\n");
        return -1;
    }
    list_info = (struct timer_info *)timer->timer;
    (void)CancelAlarm(list_info->alarm_type);
    osal_kfree(list_info);
    timer->timer = NULL;
    return 0;
}

hi_s32 osal_stop_timer(const osal_timer_t *timer)
{
    hi_s32 ret;
    // vos中没有定时器的同步停止机制，osal_stop_timer与osal_del_timer处理方式一致
    ret = osal_del_timer(timer);
    return ret;
}

hi_ulong osal_msleep(hi_u32 msecs)
{
    // 待 vos 提供
    return 0;
}

void osal_msleep_uninterrupt(hi_u32 msecs)
{
    return;
}

void osal_udelay(hi_u32 usecs)
{
    delay(usecs);
}

hi_u32 osal_get_tickcount(void)
{
    // 待vos 提供
}

hi_u64 osal_sched_clock(void)
{
    return 0;
}

hi_u64 osal_asm_get_clock(void)
{
}

void osal_get_real_ts64(osal_timespec64_t *tv)
{
}

void osal_gettimeofday(osal_timeval_t *tv)
{
    // 待实现
}

void osal_rtc_time_to_tm(hi_ulong local_time, osal_rtc_time_t *tm)
{
}

void osal_ktime_get_raw_ts64(osal_timespec64_t *tv)
{
}

hi_u64 osal_asm_get_sys_cnt(void)
{
    return 0;
}

void osal_timer_get_timestamp_type(hi_void)
{
}