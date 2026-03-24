#include "hi_osal.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/completion.h>
#ifndef AOS_LLVM_BUILD
#include <linux/sched/signal.h>
#endif // #ifndef AOS_LLVM_BUILD
#include <linux/slab.h>

#ifdef AOS_LLVM_BUILD
#include <linux/export.h>
#include <linux/hrtimer.h>
#endif // #ifdef AOS_LLVM_BUILD

// 当前AOS未提供wait exclusive接口，先内部封装
#ifdef AOS_LLVM_BUILD

#define wake_up_nr(wait, nr) AOS_WakeUp(wait, OS_TSK_INTERRUPTIBLE | OS_TSK_UNINTERRUPTIBLE, nr)

#define wait_event_interruptible_timeout_exclusive(wait, condition, timeout) ({ \
    INT64 _ret = timeout; \
    if (!(condition)) { \
        _ret = _ret > 0 ? tick_to_usecs(jiffies_to_tick(_ret)) : 0; \
        for (; ;) { \
            _ret = wait_event_ex(wait, OS_TSK_INTERRUPTIBLE, 1, _ret, condition); \
            if (_ret == -AOS_ERROR_ETIME) { \
                _ret = !!(condition); \
                break; \
            } else if (_ret == -AOS_ERROR_ERESTARTSYS) { \
                _ret = -ERESTARTSYS; \
                break; \
            } else if (_ret == -AOS_ERROR_EPERM) { \
                _ret = -EPERM; \
                break; \
            } else { \
                if (condition) { \
                    _ret = tick_to_jiffies(usecs_to_tick(_ret)); \
                    break; \
                } \
            } \
        } \
    } \
    _ret; \
})

#define wait_event_interruptible_hrtimeout_exclusive(wait, condition, timeout) ({ \
    INT64 _ret = timeout; \
    if (!(condition)) { \
        _ret = _ret > 0 ? ns_to_us(ktime_to_ns(_ret)) : 0; \
        for (; ;) { \
            _ret = wait_event_ex(wait, OS_TSK_INTERRUPTIBLE, 1, _ret, condition); \
            if (_ret == -AOS_ERROR_ETIME) { \
                _ret = !!(condition); \
                break; \
            } else if (_ret == -AOS_ERROR_ERESTARTSYS) { \
                _ret = -ERESTARTSYS; \
                break; \
            } else if (_ret == -AOS_ERROR_EPERM) { \
                _ret = -EPERM; \
                break; \
            } else { \
                if (condition) { \
                    _ret = ns_to_ktime(us_to_ns(_ret)); \
                    break; \
                } \
            } \
        } \
    } \
    _ret; \
})

#endif // #ifdef AOS_LLVM_BUILD

hi_ulong osal_msecs_to_jiffies(const hi_u32 m)
{
    return msecs_to_jiffies(m);
}
EXPORT_SYMBOL(osal_msecs_to_jiffies);

hi_s32 osal_wait_init(osal_wait_t *wait)
{
    wait_queue_head_t *wq = NULL;
    if (wait == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }
    wq = (wait_queue_head_t *)osal_kmalloc(sizeof(wait_queue_head_t), osal_gfp_atomic);
    if (wq == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "kmalloc error!\n");
        return -1;
    }
    init_waitqueue_head(wq);
    wait->wait = wq;
    return 0;
}
EXPORT_SYMBOL(osal_wait_init);

hi_s32 osal_wait_interruptible(const osal_wait_t *wait, osal_wait_cond_func_t func, const void *param)
{
#ifndef AOS_LLVM_BUILD
    wait_queue_head_t *wq = NULL;
    DEFINE_WAIT(wait__);
    hi_slong ret = 0;
    hi_s32 condition = 0;

    if (wait == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }

    wq = (wait_queue_head_t *)(wait->wait);
    if (wq == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "wait->wait is NULL!\n");
        return -1;
    }
    prepare_to_wait(wq, &wait__, TASK_INTERRUPTIBLE);
    /* if wakeup the queue brefore prepare_to_wait, the func will return true. And will not go to schedule */
    if (func != NULL) {
        condition = func(param);
    }

    if (condition == 0) {
        if (signal_pending(current) == 0) {
            schedule();
        }
        if (signal_pending(current) != 0) {
            ret = -ERESTARTSYS;
        }
    }

    finish_wait(wq, &wait__);
    return ret;
#else // #ifndef AOS_LLVM_BUILD
    return 0;
#endif // #ifndef AOS_LLVM_BUILD
}
EXPORT_SYMBOL(osal_wait_interruptible);

hi_s32 osal_wait_uninterruptible(const osal_wait_t *wait, osal_wait_cond_func_t func, const void *param)
{
#ifndef AOS_LLVM_BUILD
    wait_queue_head_t *wq = NULL;
    DEFINE_WAIT(wait__);
    hi_slong ret = 0;
    hi_s32 condition = 0;

    if (wait == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }

    wq = (wait_queue_head_t *)(wait->wait);
    if (wq == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "wait->wait is NULL!\n");
        return -1;
    }
    prepare_to_wait(wq, &wait__, TASK_UNINTERRUPTIBLE);
    /* if wakeup the queue brefore prepare_to_wait, the func will return true. And will not go to schedule */
    if (func != NULL) {
        condition = func(param);
    }

    if (condition == 0) {
        schedule();
    }

    finish_wait(wq, &wait__);
    return ret;
#else // #ifndef AOS_LLVM_BUILD
    return 0;
#endif // #ifndef AOS_LLVM_BUILD
}
EXPORT_SYMBOL(osal_wait_uninterruptible);

hi_s32 osal_wait_timeout_interruptible(
    const osal_wait_t *wait, osal_wait_cond_func_t func, const void *param, hi_s32 ms)
{
#ifndef AOS_LLVM_BUILD
    DEFINE_WAIT(wait__);
    wait_queue_head_t *wq = NULL;
    hi_s32 condition = 0;
    hi_s32 ret = ms;

    if (wait == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }

    wq = (wait_queue_head_t *)(wait->wait);
    if (wq == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "wait->wait is NULL!\n");
        return -1;
    }
    prepare_to_wait(wq, &wait__, TASK_INTERRUPTIBLE);
    /* if wakeup the queue brefore prepare_to_wait, the func will return true. And will not go to schedule */
    if (func != NULL) {
        condition = func(param);
    }

    if (condition == 0) {
        if (signal_pending(current) == 0) {
            hi_slong remain_jiffies = schedule_timeout((hi_slong)msecs_to_jiffies((hi_u32)ms));
            hi_u32 remain_ms = jiffies_to_msecs((hi_ulong)remain_jiffies);
            ret = (remain_ms > (hi_u32)INT_MAX) ? INT_MAX : (hi_s32)remain_ms; // 防止返回值uint转int反转为负数
        }
        if (signal_pending(current) != 0) {
            ret = -ERESTARTSYS;
        }
    }

    finish_wait(wq, &wait__);

    return ret;
#else // #ifndef AOS_LLVM_BUILD
    wait_queue_head_t *wq = NULL;

    if ((wait == NULL) || (func == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter wait invalid!\n");
        return -1;
    }

    wq = (wait_queue_head_t *)(wait->wait);
    if (wq == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "wait->wait is NULL!\n");
        return -1;
    }

    return wait_event_interruptible_timeout(*wq, func(param), msecs_to_jiffies(ms));
#endif // #ifndef AOS_LLVM_BUILD
}
EXPORT_SYMBOL(osal_wait_timeout_interruptible);

hi_s32 osal_wait_timeout_interruptible_exclusive(
    const osal_wait_t *wait, osal_wait_cond_func_t func, const void *param, hi_s32 ms)
{
#ifndef AOS_LLVM_BUILD
    DEFINE_WAIT(wait__);
    wait_queue_head_t *wq = NULL;
    hi_s32 condition = 0;
    hi_s32 ret = ms;

    if (wait == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }

    wq = (wait_queue_head_t *)(wait->wait);
    if (wq == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "wait->wait is NULL!\n");
        return -1;
    }

    prepare_to_wait_exclusive(wq, &wait__, TASK_INTERRUPTIBLE);

    /* if wakeup the queue brefore prepare_to_wait, the func will return true. And will not go to schedule */
    if (func != NULL) {
        condition = func(param);
    }

    if (condition == 0) {
        if (signal_pending(current) == 0) {
            hi_slong remain_jiffies = schedule_timeout((hi_slong)msecs_to_jiffies((hi_u32)ms));
            hi_u32 remain_ms = jiffies_to_msecs((hi_ulong)remain_jiffies);
            ret = (remain_ms > (hi_u32)INT_MAX) ? INT_MAX : (hi_s32)remain_ms; // 防止返回值uint转int反转为负数
        }
        if (signal_pending(current) != 0) {
            ret = -ERESTARTSYS;
        }
    }

    finish_wait(wq, &wait__);

    return ret;
#else // #ifndef AOS_LLVM_BUILD
    wait_queue_head_t *wq = NULL;

    if ((wait == NULL) || (func == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter wait invalid!\n");
        return -1;
    }

    wq = (wait_queue_head_t *)(wait->wait);
    if (wq == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "wait->wait is NULL!\n");
        return -1;
    }

    return wait_event_interruptible_timeout_exclusive(*wq, func(param), msecs_to_jiffies(ms));
#endif // #ifndef AOS_LLVM_BUILD
}
EXPORT_SYMBOL(osal_wait_timeout_interruptible_exclusive);

hi_s32 osal_wait_timeout_uninterruptible(
    const osal_wait_t *wait, osal_wait_cond_func_t func, const void *param, hi_s32 ms)
{
#ifndef AOS_LLVM_BUILD
    wait_queue_head_t *wq = NULL;
    DEFINE_WAIT(wait__);
    hi_s32 ret = ms;
    hi_s32 condition = 0;

    if (wait == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }

    wq = (wait_queue_head_t *)(wait->wait);
    if (wq == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "wait->wait is NULL!\n");
        return -1;
    }
    prepare_to_wait(wq, &wait__, TASK_UNINTERRUPTIBLE);
    /* if wakeup the queue brefore prepare_to_wait, the func will return true. And will not go to schedule */
    if (func != NULL) {
        condition = func(param);
    }

    if (condition == 0) {
        hi_slong remain_jiffies = schedule_timeout((hi_slong)msecs_to_jiffies((hi_u32)ms));
        hi_u32 remain_ms = jiffies_to_msecs((hi_ulong)remain_jiffies);
        ret = (remain_ms > (hi_u32)INT_MAX) ? INT_MAX : (hi_s32)remain_ms; // 防止返回值uint转int反转为负数
    }

    finish_wait(wq, &wait__);

    return ret;
#else // #ifndef AOS_LLVM_BUILD
    wait_queue_head_t *wq = NULL;

    if ((wait == NULL) || (func == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter wait invalid!\n");
        return -1;
    }
    wq = (wait_queue_head_t *)(wait->wait);
    if (wq == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "wait->wait is NULL!\n");
        return -1;
    }

    return wait_event_timeout(*wq, func(param), msecs_to_jiffies(ms));
#endif // #ifndef AOS_LLVM_BUILD
}
EXPORT_SYMBOL(osal_wait_timeout_uninterruptible);

static hi_s64 osal_wait_hrtimeout_interruptible(const osal_wait_t *wait, hi_s64 ns, hi_u32 nr_exclusive)
{
#ifndef AOS_LLVM_BUILD
    wait_queue_head_t *wq = NULL;
    hi_s64 ret = ns;
    DEFINE_WAIT(wait__);

    hi_u64 start_time = osal_asm_get_clock();

    wq = (wait_queue_head_t *)(wait->wait);
    if (wq == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "wait->wait is NULL!\n");
        return -1;
    }
    if (nr_exclusive != 0) {
        prepare_to_wait_exclusive(wq, &wait__, TASK_INTERRUPTIBLE);
    } else {
        prepare_to_wait(wq, &wait__, TASK_INTERRUPTIBLE);
    }

    if (signal_pending(current) == 0) {
        ktime_t wait_ktime = ns_to_ktime((hi_u64)ns);
        hi_s32 sched_ret = schedule_hrtimeout(&wait_ktime, HRTIMER_MODE_REL);
        if (sched_ret == 0) { // 计时器超时唤醒
            ret = 0;
        } else {
            hi_u64 time_cost = osal_asm_get_clock() - start_time;
            ret = ((hi_u64)ret > time_cost) ? (ret - (hi_s64)time_cost) : 0;
        }
    }
    // 有可能schedule睡眠是信号唤醒，因此此处需要再次判断是否信号唤醒
    if (signal_pending(current) != 0) {
        ret = -ERESTARTSYS;
    }

    finish_wait(wq, &wait__);

    return ret;
#else // #ifndef AOS_LLVM_BUILD
    return 0;
#endif // #ifndef AOS_LLVM_BUILD
}

void osal_hrtime_nsleep(hi_u64 ns)
{
#ifndef AOS_LLVM_BUILD
    ktime_t wait_ktime = ns_to_ktime(ns);
    set_current_state(TASK_UNINTERRUPTIBLE);
    schedule_hrtimeout(&wait_ktime, HRTIMER_MODE_REL);
#endif // #ifndef AOS_LLVM_BUILD
}
EXPORT_SYMBOL(osal_hrtime_nsleep);

hi_s64 osal_wait_event_hrtimeout_interruptible(
    const osal_wait_t *wait, osal_wait_cond_func_t func, const void *param, hi_s64 timeout)
{
#ifndef AOS_LLVM_BUILD
    hi_s64 ret = timeout;
    if ((wait == NULL) || (func == NULL) || (timeout < 0)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }
    if ((func(param) != 0) && (ret == 0)) {
        return 1;
    }

    while (func(param) == 0) {
        ret = osal_wait_hrtimeout_interruptible(wait, ret, 0);
        if (ret <= 0) { // timeout, signal interrupt or error
            break;
        }
    }
    return ret;
#else
#ifndef ETIME
#define ETIME 62
#endif
    wait_queue_head_t *wq = NULL;
    if ((wait == NULL) || (func == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter wait invalid!\n");
        return -1;
    }
    wq = (wait_queue_head_t *)(wait->wait);
    if (wq == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "wait->wait is NULL!\n");
        return -1;
    }

    return wait_event_interruptible_hrtimeout(*wq, func(param), ns_to_ktime(timeout));
#endif
}
EXPORT_SYMBOL(osal_wait_event_hrtimeout_interruptible);

hi_s64 osal_wait_event_hrtimeout_interruptible_exclusive(
    const osal_wait_t *wait, osal_wait_cond_func_t func, const void *param, hi_s64 timeout)
{
#ifndef AOS_LLVM_BUILD
    hi_s64 ret = timeout;
    if ((wait == NULL) || (func == NULL) || (timeout < 0)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }
    if ((func(param) != 0) && (ret == 0)) {
        return 1;
    }

    while (func(param) == 0) {
        ret = osal_wait_hrtimeout_interruptible(wait, ret, 1);
        if (ret <= 0) { // timeout, signal interrupt or error
            break;
        }
    }
    return ret;
#else
#ifndef ETIME
#define ETIME 62
#endif
    wait_queue_head_t *wq = NULL;
    if ((wait == NULL) || (func == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter wait invalid!\n");
        return -1;
    }
    wq = (wait_queue_head_t *)(wait->wait);
    if (wq == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "wait->wait is NULL!\n");
        return -1;
    }

    return wait_event_interruptible_hrtimeout_exclusive(*wq, func(param), ns_to_ktime(timeout));
#endif
}
EXPORT_SYMBOL(osal_wait_event_hrtimeout_interruptible_exclusive);

void osal_wakeup(const osal_wait_t *wait)
{
    wait_queue_head_t *wq = NULL;
    if (wait == NULL) {
        return;
    }

    wq = (wait_queue_head_t *)(wait->wait);
    if (wq == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "wait->wait is NULL!\n");
        return;
    }
    wake_up_all(wq);
}
EXPORT_SYMBOL(osal_wakeup);

void osal_wakeup_one(const osal_wait_t *wait)
{
    wait_queue_head_t *wq = NULL;
    if (wait == NULL) {
        return;
    }

    wq = (wait_queue_head_t *)(wait->wait);
    if (wq == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "wait->wait is NULL!\n");
        return;
    }
    wake_up(wq);
}
EXPORT_SYMBOL(osal_wakeup_one);

void osal_wakeup_nr(const osal_wait_t *wait, int nr)
{
    wait_queue_head_t *wq = NULL;
    if (wait == NULL) {
        return;
    }

    wq = (wait_queue_head_t *)(wait->wait);
    if (wq == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "wait->wait is NULL!\n");
        return;
    }
    wake_up_nr(wq, nr);
}
EXPORT_SYMBOL(osal_wakeup_nr);

void osal_wait_destroy(osal_wait_t *wait)
{
    const wait_queue_head_t *wq = NULL;
    if (wait == NULL) {
        return;
    }

    wq = (wait_queue_head_t *)(wait->wait);
    if (wq == NULL) {
        return;
    }
    osal_kfree(wq);
    wait->wait = NULL;
}
EXPORT_SYMBOL(osal_wait_destroy);

void osal_wait_destory(osal_wait_t *wait)
__attribute__((alias("osal_wait_destroy")));
EXPORT_SYMBOL(osal_wait_destory);

void osal_init_completion(void *x)
{
    if (x == NULL) {
        return;
    }
    init_completion((struct completion *)x);
    return;
}
EXPORT_SYMBOL(osal_init_completion);

void osal_wait_for_completion(void *x)
{
    if (x == NULL) {
        return;
    }
    wait_for_completion((struct completion *)x);
    return;
}
EXPORT_SYMBOL(osal_wait_for_completion);

void osal_complete(void *x)
{
    if (x == NULL) {
        return;
    }
    complete((struct completion *)x);
    return;
}
EXPORT_SYMBOL(osal_complete);