/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2016-2020.
 * Description: osal timer source file.
 * Author: Hisilicon multimedia software group
 * Create: 2016-11-11
 */
#include "hi_osal.h"
#include "los_spinlock.h"
#include "los_swtmr.h"
#include "los_tick.h"

#define NSECS_PER_MSEC 1000000
#define NSECS_PER_USEC 1000
#define NSECS_PSE_SEC 1000000000

typedef unsigned long ULONG;

typedef struct timer_list {
    ULONG expires;
    VOID (*function)(ULONG);
    ULONG data;
    UINT16 timerid;
    UINT32 flag;
    BOOL created;
#define TIMER_VALID 0xABCDDCBA
#define TIMER_UNVALID 0xDCBAABCD
    SPIN_LOCK_S lock;
} timer_list_t;

static void init_timer(struct timer_list *timer)
{
    UINT32 intSave;
    if (timer == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "timer is NULL\n");
        return;
    }
    LOS_SpinInit(&timer->lock);
    LOS_SpinLockSave(&timer->lock, &intSave);
    timer->flag = TIMER_UNVALID;
    LOS_SpinUnlockRestore(&timer->lock, intSave);
}

static UINT32 DoDeleteTimer(struct timer_list *timer)
{
    UINT32 ret;

    ret = LOS_SwtmrDelete(timer->timerid);
    if (ret == LOS_OK) {
        timer->flag = TIMER_UNVALID;
    }
    return ret;
}

static UINT32 DoAddTimer(struct timer_list *timer)
{
    UINT32 ret;

    ret = LOS_SwtmrCreate(timer->expires, LOS_SWTMR_MODE_NO_SELFDELETE,
                          (SWTMR_PROC_FUNC)timer->function, &timer->timerid,
                          timer->data);
    if (ret != LOS_OK) {
        HI_TRACE_OSAL(HI_DBG_ERR, "timer create failed: %u \n", ret);
        return ret;
    }
    (VOID) LOS_SwtmrStart(timer->timerid);

    return ret;
}

static void add_timer(struct timer_list *timer)
{
    UINT32 intSave;

    if (timer == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "timer is NULL\n");
        return;
    }

    LOS_SpinLockSave(&timer->lock, &intSave);
    if (timer->flag == TIMER_VALID) {
        if (DoDeleteTimer(timer) != LOS_OK) {
            goto ERROUT;
        }
    }

    if (DoAddTimer(timer) != LOS_OK) {
        goto ERROUT;
    }
    timer->flag = TIMER_VALID;
ERROUT:
    LOS_SpinUnlockRestore(&timer->lock, intSave);
    return;
}

static int __del_timer(struct timer_list *timer)
{
    UINT32 intSave;
    INT32 ret = 0;

    if (timer == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "timer is NULL\n");
        return ret;
    }

    LOS_SpinLockSave(&timer->lock, &intSave);
    if (timer->flag == TIMER_VALID) {
        ret = (DoDeleteTimer(timer) == LOS_OK) ? 1 : 0;
    }
    LOS_SpinUnlockRestore(&timer->lock, intSave);

    return ret;
}

hi_ulong osal_timer_get_private_data(const void *data)
{
    return (unsigned long)(uintptr_t)data;
}

hi_s32 osal_timer_init(osal_timer_t *timer)
{
    struct timer_list *t = NULL;

    if (timer == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }

    t = (struct timer_list *)osal_kmalloc(sizeof(struct timer_list), 0);
    if (t == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "kmalloc error!\n");
        return -1;
    }

    init_timer(t);
    timer->timer = t;
    return 0;
}

hi_s32 osal_set_timer(const osal_timer_t *timer, hi_ulong interval)
{
    struct timer_list *t = NULL;

    if ((timer == NULL) || (timer->timer == NULL) ||
        (timer->function == NULL) || (interval == 0)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }
    t = timer->timer;
    t->function = timer->function;
    t->data = timer->data;
    t->expires = LOS_MS2Tick(interval);
    add_timer(t);

    return 0;
}

hi_s32 osal_del_timer(const osal_timer_t *timer)
{
    struct timer_list *t = NULL;

    if ((timer == NULL) || (timer->timer == NULL) ||
        (timer->function == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }
    t = timer->timer;
    return __del_timer(t);
}

hi_s32 osal_stop_timer(const osal_timer_t *timer)
{
    return osal_del_timer(timer);
}

hi_s32 osal_timer_destory(osal_timer_t *timer)
{
    struct timer_list *t = NULL;

    if ((timer == NULL) || (timer->timer == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }
    t = timer->timer;

    (hi_void)__del_timer(t);
    osal_kfree(t);
    timer->timer = NULL;
    return 0;
}

hi_ulong osal_msleep(hi_u32 msecs)
{
    LOS_Msleep(msecs);
    return 0;
}

void osal_msleep_uninterrupt(hi_u32 msecs)
{
    LOS_Msleep(msecs);
}

void osal_udelay(hi_u32 usecs)
{
    LOS_Udelay(usecs);
}

hi_u32 osal_get_tickcount(void)
{
    return (unsigned int)LOS_Tick2MS(LOS_TickCountGet());
}

hi_u64 osal_sched_clock(void)
{
    return LOS_CurrNanosec();
}

hi_u64 osal_asm_get_clock(void)
{
    /* 获取绝对时间纳秒 */
    return LOS_CurrNanosec();
}

void osal_gettimeofday(osal_timeval_t *tv)
{
    hi_u64 ns = LOS_CurrNanosec();
    tv->tv_sec = ns / NSECS_PSE_SEC;
    tv->tv_usec = (ns % NSECS_PSE_SEC) / NSECS_PER_USEC;
}

void osal_rtc_time_to_tm(hi_ulong local_time, osal_rtc_time_t *tm)
{
    HI_UNUSED(local_time);
    HI_UNUSED(tm);
    HI_TRACE_OSAL(HI_DBG_ERR, "not supported\n");
}

void osal_hrtime_nsleep(hi_u64 ns)
{
    LOS_Msleep(ns / NSECS_PER_MSEC);
}

hi_u64 osal_asm_get_sys_cnt(void)
{
    return osal_sched_clock(); // 获取sys cnt
}

hi_u64 osal_get_tick_count(void)
{
    return LOS_TickCountGet();
}