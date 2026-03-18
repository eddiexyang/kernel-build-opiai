#include "hi_osal.h"
#include "Os.h"

// 使用自旋锁需要在oil文件中定义一个resource，并在每个可能竞争自旋锁的task中关联该resource

/*
 *   mutex既是入参也是出参：
 *     做入参时，调用本接口前需要初始化lock->lock指针，指向一块u32内存，内存中存储resource id
 */
hi_s32 osal_spin_lock_init(osal_spinlock_t *lock)
{
    ResourceType *res = NULL;

    if ((lock == NULL) || (lock->lock == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }
    res = osal_kmalloc(sizeof(ResourceType), osal_gfp_kernel);
    if (res == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "kmalloc error!\n");
        return -1;
    }
    *res = *((ResourceType *)(lock->lock));
    lock->lock = res;
    return 0;
}

void osal_spin_lock_destroy(osal_spinlock_t *lock)
{
    ResourceType *res = NULL;

    if ((lock == NULL) || (lock->lock == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return;
    }
    res = (ResourceType *)(lock->lock);
    if (res != NULL) {
        osal_kfree(res);
        lock->lock = NULL;
    }
}

void osal_spin_lock(const osal_spinlock_t *lock)
{
    StatusType ret;
    ResourceType res;

    if ((lock == NULL) || (lock->lock == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return;
    }

    res = *((ResourceType*)lock->lock);
    ret = GetResource(res);
    if (ret != E_OK) {
        HI_TRACE_OSAL(HI_DBG_ERR, " GetResource fail, ret:%d!\n", ret);
    }
}

hi_s32 osal_spin_trylock(const osal_spinlock_t *lock)
{
    StatusType ret;
    ResourceType res;

    if ((lock == NULL) || (lock->lock == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }

    res = *((ResourceType*)lock->lock);
    ret = GetResource(res);
    if (ret != E_OK) {
        HI_TRACE_OSAL(HI_DBG_ERR, " GetResource fail, ret:%d!\n", ret);
        return -1;
    } else {
        return 0;
    }
}

void osal_spin_unlock(const osal_spinlock_t *lock)
{
    StatusType ret;
    ResourceType res;

    if ((lock == NULL) || (lock->lock == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return;
    }

    res = *((ResourceType*)lock->lock);
    ret = ReleaseResource(res);
    if (ret != E_OK) {
        HI_TRACE_OSAL(HI_DBG_ERR, " ReleaseResource fail, ret:%d!\n", ret);
    }
}
void osal_spin_lock_irqsave(const osal_spinlock_t *lock, hi_ulong *flags)
{
    osal_spin_lock(lock);
}

void osal_spin_unlock_irqrestore(const osal_spinlock_t *lock, const hi_ulong *flags)
{
    osal_spin_unlock(lock);
}
