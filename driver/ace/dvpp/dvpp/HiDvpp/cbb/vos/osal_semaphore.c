#include "hi_osal.h"
#include "Os.h"

// 使用信号量需要在oil文件中定义一个resource+一个event，并在每个可能竞争信号量的task中关联该resource和event

typedef struct vos_semaphore {
    struct osal_list_head task_list;
    EventMaskType event_mask;
    ResourceType res_id;
    hi_s32 sem_val;
} vos_semaphore_t;

typedef struct vos_task_info {
    struct osal_list_head list;
    TaskType task_id;
} vos_task_info_t;

/*
 * 信号量初始化要求：
 *   1、调用本接口前需要初始化sem->sem指针，指向的内存需要满足：
 *      a、第一个u64字节位置存储event mask
 *      b、第二个u64字节位置存储resource id
 */
hi_s32 osal_sema_init(osal_semaphore_t *sem, hi_s32 val)
{
    struct vos_semaphore *p = NULL;

    if ((sem == NULL) || (sem->sem == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }
    p = osal_kmalloc(sizeof(struct vos_semaphore), osal_gfp_kernel);
    if (p == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "kmalloc error!\n");
        return -1;
    }
    OSAL_INIT_LIST_HEAD(&p->task_list);
    p->sem_val = val;
    p->event_mask = *((EventMaskType *)(sem->sem));
    p->res_id = *((ResourceType *)(sem->sem + 8U));
    sem->sem = p;
    return 0;
}

hi_s32 osal_down(const osal_semaphore_t *sem)
{
    StatusType ret = E_NOT_OK;
    TaskType task_id = 0;
    struct vos_semaphore *p = NULL;
    struct vos_task_info *task_info = NULL;

    if ((sem == NULL) || (sem->sem == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }

    p = (struct vos_semaphore *)(sem->sem);
    // 获取spinlock
    ret = GetResource(p->res_id);
    if (ret != E_OK) {
        HI_TRACE_OSAL(HI_DBG_ERR, " GetResource fail, ret:%d!\n", ret);
        return -1;
    }
    // 这是slow path （少数情况下信号量已被消耗完）
    if (p->sem_val <= 0) {
        // 获取信号量失败后，先将task加入到待唤醒队列中
        task_info = osal_kmalloc(sizeof(struct vos_task_info), osal_gfp_kernel);
        if (task_info == NULL) {
            HI_TRACE_OSAL(HI_DBG_ERR, "kmalloc error!\n");
            ret = ReleaseResource(p->res_id);
            if (ret != E_OK) {
                HI_TRACE_OSAL(HI_DBG_ERR, " ReleaseResource fail, ret:%d!\n", ret);
            }
            return -1;
        }
        GetTaskID(&task_id);
        task_info->task_id = task_id;
        osal_list_add_tail(&task_info->list, &p->task_list);
        ret = ReleaseResource(p->res_id);
        if (ret != E_OK) {
            /*
             * 这里解锁失败时，不能将刚挂到链表中的节点回滚删除(链表操作已无并发保护)，当前的处理方法是
             * 将链表节点继续保留在链表中，up操作遍历链表会尝试setevent，这里只打印err日志提示已出错
             * 下面几处异常分支也做相同处理
             */
            HI_TRACE_OSAL(HI_DBG_ERR, " ReleaseResource fail, ret:%d!\n", ret);
            return -1;
        }
retry:
        // 在ReleaseResource释放锁之后WaitEvent之间，持锁的task释放锁，调用SetEvent会将本task状态置为ready，
        // 随后调用WaitEvent会立刻返回成功，不会将本task调度出去
        ret = WaitEvent(p->event_mask);
        if (ret != E_OK) {
            HI_TRACE_OSAL(HI_DBG_ERR, "WaitEvent fail\n");
            return -1;
        }
        ret = GetResource(p->res_id);
        if (ret != E_OK) {
            HI_TRACE_OSAL(HI_DBG_ERR, " GetResource fail, ret:%d!\n", ret);
            return -1;
        }
        if (p->sem_val <= 0) {
            ret = ReleaseResource(p->res_id);
            if (ret != E_OK) {
                HI_TRACE_OSAL(HI_DBG_ERR, " ReleaseResource fail, ret:%d!\n", ret);
                return -1;
            }
            goto retry;
        } else {
            osal_list_del(&task_info->list);
            osal_kfree(task_info);
            p->sem_val--;
            ret = ReleaseResource(p->res_id);
            if (ret != E_OK) {
                HI_TRACE_OSAL(HI_DBG_ERR, " ReleaseResource fail, ret:%d!\n", ret);
            }
            return 0;
        }
    } else {
        // 这里是fast patch,（大部分情况下可以顺利拿到信号量）
        p->sem_val--;
        ret = ReleaseResource(p->res_id);
        if (ret != E_OK) {
            HI_TRACE_OSAL(HI_DBG_ERR, " ReleaseResource fail, ret:%d!\n", ret);
        }
        return 0;
    }
    return 0;
}

// 无调用者，暂不实现
int osal_down_timeout(osal_semaphore_t *sem, long msecs)
{
    return -1;
}

hi_s32 osal_down_interruptible(const osal_semaphore_t *sem)
{
    return osal_down(sem);
}

hi_s32 osal_down_trylock(const osal_semaphore_t *sem)
{
    StatusType ret = E_NOT_OK;
    struct vos_semaphore *p = NULL;

    if ((sem == NULL) || (sem->sem == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }
    p = (struct vos_semaphore *)(sem->sem);

    ret = GetResource(p->res_id);
    if (ret != E_OK) {
        HI_TRACE_OSAL(HI_DBG_ERR, " GetResource fail, ret:%d!\n", ret);
        return -1;
    }
    if (p->sem_val <= 0) {
        ret = ReleaseResource(p->res_id);
        if (ret != E_OK) {
            HI_TRACE_OSAL(HI_DBG_ERR, " ReleaseResource fail, ret:%d!\n", ret);
        }
        return 1;
    } else {
        p->sem_val--;
    }
    ret = ReleaseResource(p->res_id);
    if (ret != E_OK) {
        HI_TRACE_OSAL(HI_DBG_ERR, " ReleaseResource fail, ret:%d!\n", ret);
        // 这里已经持有信号量，虽然spinlock解锁失败，但仍需返回0
        return 0;
    }
    return 0;
}

void osal_up(const osal_semaphore_t *sem)
{
    StatusType ret = E_NOT_OK;
    struct vos_semaphore *p = NULL;
    vos_task_info_t *task_info = NULL;
    vos_task_info_t *tmp = NULL;

    if ((sem == NULL) || (sem->sem == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return;
    }

    p = (struct vos_semaphore *)(sem->sem);
    ret = GetResource(p->res_id);
    if (ret != E_OK) {
        HI_TRACE_OSAL(HI_DBG_ERR, " GetResource fail, ret:%d!\n", ret);
        return;
    }
    p->sem_val++;
    osal_list_for_each_entry_safe(task_info, tmp, &p->task_list, list) {
        SetEvent(task_info->task_id, p->event_mask);
    }
    ret = ReleaseResource(p->res_id);
    if (ret != E_OK) {
        HI_TRACE_OSAL(HI_DBG_ERR, " ReleaseResource fail, ret:%d!\n", ret);
        return;
    }
}

void osal_sema_destroy(osal_semaphore_t *sem)
{
    const struct vos_semaphore *p = NULL;
    vos_task_info_t *task_info = NULL;
    vos_task_info_t *tmp = NULL;

    if ((sem == NULL) || (sem->sem == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return;
    }
    p = (struct vos_semaphore *)(sem->sem);
    if (p != NULL) {
        osal_list_for_each_entry_safe(task_info, tmp, &p->task_list, list) {
            osal_list_del(&task_info->list);
            osal_kfree(task_info);
        }
        osal_kfree(p);
        sem->sem = NULL;
    }
}

void osal_sema_destory(osal_semaphore_t *sem)
{
    osal_sema_destroy(sem);
}
