/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2016-2020.
 * Description: osal wait source file.
 * Author: Hisilicon multimedia software group
 * Create: 2016-11-11
 */

#include "hi_osal.h"
#include "los_event.h"
#include "los_sys.h"

typedef struct wait_queue_head {
    EVENT_CB_S stEvent;
} wait_queue_head_t;

static void init_waitqueue_head(wait_queue_head_t *wait)
{
    if (wait == NULL) {
        return;
    }
    (VOID)LOS_EventInit(&wait->stEvent);
}

hi_ulong osal_msecs_to_jiffies(const hi_u32 m)
{
    return (unsigned long)LOS_MS2Tick(m);
}

hi_s32 osal_wait_init(osal_wait_t *wait)
{
    wait_queue_head_t *wq = NULL;

    if (wait == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }
    wq = (wait_queue_head_t *)osal_kmalloc(sizeof(wait_queue_head_t), 0);
    if (wq == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "kmalloc error!\n");
        return -1;
    }
    init_waitqueue_head(wq);
    wait->wait = wq;
    return 0;
}

hi_s32 osal_wait_uninterruptible(const osal_wait_t *wait, osal_wait_cond_func_t func, const void *param)
{
    wait_queue_head_t *wq = NULL;

    HI_UNUSED(func);
    HI_UNUSED(param);

    if (wait == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }

    wq = (wait_queue_head_t *)(wait->wait);
    if (wq == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }

    (void)LOS_EventRead(&wq->stEvent, 0x1, LOS_WAITMODE_AND | LOS_WAITMODE_CLR,
                        LOS_WAIT_FOREVER);

    return 0;
}

hi_s32 osal_wait_timeout_uninterruptible(const osal_wait_t *wait, osal_wait_cond_func_t func,
                                         const void *param, hi_s32 ms)
{
    unsigned int ret;
    unsigned long tick;
    wait_queue_head_t *wq = NULL;
    unsigned long ticks_now, ticks_gap;

    HI_UNUSED(func);
    HI_UNUSED(param);

    if (wait == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }

    wq = (wait_queue_head_t *)(wait->wait);
    if (wq == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }

    tick = LOS_MS2Tick(ms);
    ticks_now = (unsigned long)LOS_TickCountGet();
    ret = LOS_EventRead(&wq->stEvent, 0x1, LOS_WAITMODE_AND | LOS_WAITMODE_CLR, tick);

    ticks_gap = ((unsigned long)LOS_TickCountGet() - ticks_now);
    if (tick <= ticks_gap) {
        return 0; /* timeout */
    } else {
        tick -= ticks_gap;
    }

    if (ret == LOS_ERRNO_EVENT_READ_TIMEOUT) {
        return 0; /* timeout */
    } else {
        return LOS_Tick2MS(tick); /* read wq event  ok */
    }
}

hi_s32 osal_wait_timeout_interruptible(const osal_wait_t *wait, osal_wait_cond_func_t func,
                                       const void *param, hi_s32 ms)
{
    /* LiteOS do not support signal */
    return osal_wait_timeout_uninterruptible(wait, func, param, ms);
}

void osal_wakeup(const osal_wait_t *wait)
{
    wait_queue_head_t *wq = NULL;

    if (wait == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return;
    }
    wq = (wait_queue_head_t *)(wait->wait);
    if (wq == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return;
    }

    (VOID)LOS_EventWrite(&wq->stEvent, 0x1);
}

void osal_wait_destory(osal_wait_t *wait)
{
    wait_queue_head_t *wq = NULL;

    if (wait == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return;
    }

    wq = (wait_queue_head_t *)(wait->wait);
    if (wq == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return;
    }

    (VOID)LOS_EventDestroy(&wq->stEvent);
    osal_kfree(wq);
    wait->wait = NULL;
}
