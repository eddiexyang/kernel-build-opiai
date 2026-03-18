#include "hi_osal.h"
#include "Os.h"

#define WQ_FLAG_DEFAULT	0x00
#define WQ_FLAG_EXCLUSIVE	0x01

/*
 * 1、当前实现中timeout相关接口设定timeout时间不生效，暂时实现为永不超时，待后续vos提供waitevent支持timeout接口后再修正
 * 2、使用waitque需要在oil文件中定义一个resource+一个event，并在task中关联该resource和event
 */

typedef struct vos_waitqueue {
    struct osal_list_head wait_list;
    EventMaskType event_mask;
    ResourceType res_id;
} vos_waitqueue_t;

typedef struct vos_wait {
    struct osal_list_head list;
    hi_u32 exclusive;
    hi_bool wakeup_flag;
    TaskType task_id;
} vos_wait_t;

/*
 * wait初始化要求：
 *   1、调用本接口前需要初始化wait->wait指针，指向的内存需要满足：
 *      a、第一个u64字节位置存储event mask
 *      b、第二个u64字节位置存储resource id
 */
hi_s32 osal_wait_init(osal_wait_t *wait)
{
    struct vos_waitqueue *wq = NULL;
    if ((wait == NULL) || (wait->wait == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }
    wq = osal_kmalloc(sizeof(struct vos_waitqueue), osal_gfp_kernel);
    if (wq == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "kmalloc error!\n");
        return -1;
    }
    OSAL_INIT_LIST_HEAD(&wq->wait_list);
    wq->event_mask = *((EventMaskType *)(wait->wait));
    wq->res_id = *((ResourceType *)(wait->wait + 8U));
    wait->wait = wq;
    return 0;
}

hi_s32 osal_wait(const osal_wait_t *wait, osal_wait_cond_func_t func, const void *param, hi_u32 exclusive)
{
    TaskType task_id = 0;
    hi_s32 condition = 0;
    StatusType ret = E_NOT_OK;
    struct vos_waitqueue *wq = NULL;
    struct vos_wait *vos_wait = NULL;

    if ((wait == NULL) || (wait->wait == NULL) || (func == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }

    wq = (struct vos_waitqueue *)(wait->wait);

    /* if wakeup the queue brefore prepare_to_wait, the func will return true. And will not go to schedule */
    if (func != NULL) {
        condition = func(param);
    }

    if (condition == 0) {
        /*
         * 这是不使用局部变量，而是通过kmalloc申请堆内存的原因是本函数中持锁接口GetResource
         * 可能会失败，失败后无法将加入到链表中的节点直接删除，故需要申请堆内存，失败时将内存节点残留
         * 在链表中，wakeup时不再唤醒对应task。在destory时将申请的堆内存释放
         */
        vos_wait = osal_kmalloc(sizeof(struct vos_wait), osal_gfp_kernel);
        if (vos_wait == NULL) {
            HI_TRACE_OSAL(HI_DBG_ERR, "kmalloc error!\n");
            return -1;
        }
        GetTaskID(&task_id);
        vos_wait->task_id = task_id;
        vos_wait->exclusive = exclusive;
        vos_wait->wakeup_flag = 1;
        ret = GetResource(wq->res_id);
        if (ret != E_OK) {
            HI_TRACE_OSAL(HI_DBG_ERR, " GetResource fail, ret:%d!\n", ret);
            osal_kfree(vos_wait);
            return -1;
        }
        if (exclusive == WQ_FLAG_EXCLUSIVE) {
            osal_list_add_tail(&vos_wait->list, &wq->wait_list);
        } else {
            osal_list_add(&vos_wait->list, &wq->wait_list);
        }
        ret = ReleaseResource(wq->res_id);
        if (ret != E_OK) {
            /*
             * 这里解锁失败时，不能将刚挂到链表中的节点回滚删除(链表操作已无并发保护)，当前的处理方法是
             * 将链表节点继续保留在链表中，wakeup操作遍历链表会尝试setevent，这里只打印err日志提示已出错
             * 下面几处异常分支也做相同处理
             */
            HI_TRACE_OSAL(HI_DBG_ERR, " ReleaseResource fail, ret:%d!\n", ret);
        }
        ret = WaitEvent(wq->event_mask);
        if (ret != E_OK) {
            // 此时wait失败了，不需要在wakeup中再次setevent
            vos_wait->wakeup_flag = 0;
            HI_TRACE_OSAL(HI_DBG_ERR, "WaitEvent fail\n");
            return -1;
        }
        // 重新调度回来执行，从链表中删除
        ret= GetResource(wq->res_id);
        if (ret != E_OK) {
            // 此时task已重新调度回来，失败时无法从链表中删除，设置标志不需要在wakeup中再次setevent
            vos_wait->wakeup_flag = 0;
            HI_TRACE_OSAL(HI_DBG_ERR, " GetResource fail, ret:%d!\n", ret);
            return -1;
        }
        // 这里不再重新检查condition是否满足，为了与linux实现行为保持一致
        osal_list_del(&vos_wait->list);
        osal_kfree(vos_wait);
        ret = ReleaseResource(wq->res_id);
        if (ret != E_OK) {
            HI_TRACE_OSAL(HI_DBG_ERR, "ReleaseResource fail, ret:%d!\n", ret);
            return -1;
        }
    }

    return 0;
}

hi_s32 osal_wait_uninterruptible(const osal_wait_t *wait, osal_wait_cond_func_t func, const void *param)
{
    return osal_wait(wait, func, param, WQ_FLAG_DEFAULT);
}

// vos上没有task发信号机制,这里按不可打断方式处理
hi_s32 osal_wait_interruptible(const osal_wait_t *wait, osal_wait_cond_func_t func, const void *param)
{
    return osal_wait(wait, func, param, WQ_FLAG_DEFAULT);
}

// 当前实现针对timeout时间强制设置为死等，待后续替换为vos支持waitevent设置timeout版本
hi_s32 osal_wait_timeout_interruptible(
    const osal_wait_t *wait, osal_wait_cond_func_t func, const void *param, hi_s32 ms)
{
    hi_s32 ret = 0;
    HI_UNUSED(ms);

    ret = osal_wait(wait, func, param, WQ_FLAG_DEFAULT);
    // osal_wait成功，返回值强制为1，表示未超期
    if (ret == 0) {
        ret = 1;
    }
    return ret;
}

hi_s32 osal_wait_timeout_uninterruptible(
    const osal_wait_t *wait, osal_wait_cond_func_t func, const void *param, hi_s32 ms)
{
    hi_s32 ret = 0;
    HI_UNUSED(ms);

    ret = osal_wait(wait, func, param, WQ_FLAG_DEFAULT);
    // osal_wait成功，返回值强制为1，表示未超期
    if (ret == 0) {
        ret = 1;
    }
    return ret;
}

hi_s32 osal_wait_timeout_interruptible_exclusive(
    const osal_wait_t *wait, osal_wait_cond_func_t func, const void *param, hi_s32 ms)
{
    hi_s32 ret = 0;
    HI_UNUSED(ms);

    ret = osal_wait(wait, func, param, WQ_FLAG_EXCLUSIVE);
    // osal_wait成功，返回值强制为1，表示未超期
    if (ret == 0) {
        ret = 1;
    }
    return ret;
}

hi_s64 osal_wait_event_hrtimeout_interruptible(
    const osal_wait_t *wait, osal_wait_cond_func_t func, const void *param, hi_s64 timeout)
{
    hi_s64 ret = 0;
    if ((wait == NULL) || (wait->wait == NULL) || (func == NULL)  || (timeout < 0)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }
    if ((func(param) != 0) && (timeout == 0)) {
        return 1;
    }

    while (func(param) == 0) {
        ret = osal_wait_timeout_uninterruptible(wait, func, param, ret);
        if (ret <= 0) {
            break;
        }
    }
    return ret;
}

hi_s64 osal_wait_event_hrtimeout_interruptible_exclusive(
    const osal_wait_t *wait, osal_wait_cond_func_t func, const void *param, hi_s64 timeout)
{
    hi_s64 ret = 0;
    if ((wait == NULL) || (wait->wait == NULL) || (func == NULL) || (timeout < 0)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }
    if ((func(param) != 0) && (timeout == 0)) {
        return 1;
    }

    while (func(param) == 0) {
        ret = osal_wait_timeout_interruptible_exclusive(wait, func, param, ret);
        if (ret <= 0) { // timeout, signal interrupt or error
            break;
        }
    }
    return ret;
}

void vos_osal_wakeup(const osal_wait_t *wait, int nr_exclusive)
{
    StatusType ret = E_NOT_OK;
    struct vos_wait *vos_wait = NULL;
    struct vos_wait *tmp = NULL;
    struct vos_waitqueue *wq = NULL;
    int nr;
    if ((wait == NULL) || (wait->wait == NULL)) {
        return;
    }

    nr = nr_exclusive;
    wq = (struct vos_waitqueue *)(wait->wait);
    ret = GetResource(wq->res_id);
    if (ret != E_OK) {
        HI_TRACE_OSAL(HI_DBG_ERR, " GetResource fail, ret:%d!\n", ret);
        return;
    }
    osal_list_for_each_entry_safe(vos_wait, tmp, &wq->wait_list, list) {
        if (vos_wait->wakeup_flag == 0) {
            continue;
        }
        ret = SetEvent(vos_wait->task_id, wq->event_mask);
        if (ret != E_OK) {
            HI_TRACE_OSAL(HI_DBG_ERR, " SetEvent fail, ret:%d!\n", ret);
        }
        if ((vos_wait->exclusive == WQ_FLAG_EXCLUSIVE) && !(--nr)) {
            break;
        }
    }
    ret = ReleaseResource(wq->res_id);
    if (ret != E_OK) {
        HI_TRACE_OSAL(HI_DBG_ERR, " ReleaseResource fail, ret:%d!\n", ret);
    }
}

void osal_wakeup(const osal_wait_t *wait)
{
    vos_osal_wakeup(wait, 0);
}

void osal_wakeup_one(const osal_wait_t *wait)
{
    vos_osal_wakeup(wait, 1);
}

void osal_wakeup_nr(const osal_wait_t *wait, int nr)
{
    vos_osal_wakeup(wait, nr);
}

void osal_wait_destroy(osal_wait_t *wait)
{
    struct vos_wait *tmp = NULL;
    struct vos_wait *vos_wait = NULL;
    const struct vos_waitqueue *wq = NULL;
    if ((wait == NULL) || (wait->wait == NULL)) {
        return;
    }

    wq = (struct vos_waitqueue *)(wait->wait);

    osal_list_for_each_entry_safe(vos_wait, tmp, &wq->wait_list, list) {
        osal_list_del(&vos_wait->list);
        osal_kfree(vos_wait);
    }
    osal_kfree(wq);
    wait->wait = NULL;
}

void osal_wait_destory(osal_wait_t *wait)
{
    osal_wait_destroy(wait);
}

// timeout相关接口不暂不支持超时时间配置，暂时按永不超期处理，这里直接返回1，待timeout实现是修改本处
hi_ulong osal_msecs_to_jiffies(const hi_u32 m)
{
    return 1;
}