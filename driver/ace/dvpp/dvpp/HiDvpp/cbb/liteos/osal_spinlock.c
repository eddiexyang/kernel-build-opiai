/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2016-2020.
 * Description: osal spinlock source file.
 * Author: Hisilicon multimedia software group
 * Create: 2016-11-11
 */

#include "hi_osal.h"
#include "los_spinlock.h"

hi_s32 osal_spin_lock_init(osal_spinlock_t *lock)
{
    SPIN_LOCK_S *p = NULL;
    if (lock == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }
    p = (SPIN_LOCK_S *)osal_kmalloc((UINT32)sizeof(SPIN_LOCK_S), 0);
    if (p == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "kmalloc error!\n");
        return -1;
    }

    LOS_SpinInit(p);
    lock->lock = p;
    return 0;
}

void osal_spin_lock(const osal_spinlock_t *lock)
{
    if ((lock == NULL) || (lock->lock == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return;
    }
    LOS_SpinLock((SPIN_LOCK_S *)(lock->lock));
}

void osal_spin_unlock(const osal_spinlock_t *lock)
{
    if ((lock == NULL) || (lock->lock == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return;
    }

    LOS_SpinUnlock((SPIN_LOCK_S *)(lock->lock));
}
void osal_spin_lock_irqsave(const osal_spinlock_t *lock, hi_ulong *flags)
{
    UINT32 f;
    if ((lock == NULL) || (lock->lock == NULL) || (flags == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return;
    }

    LOS_SpinLockSave((SPIN_LOCK_S *)(lock->lock), &f);
    *flags = f;
}
void osal_spin_unlock_irqrestore(const osal_spinlock_t *lock, const hi_ulong *flags)
{
    UINT32 f;
    if ((lock == NULL) || (lock->lock == NULL) || (flags == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return;
    }

    f = *flags;
    LOS_SpinUnlockRestore((SPIN_LOCK_S *)(lock->lock), f);
}
void osal_spin_lock_destory(osal_spinlock_t *lock)
{
    if ((lock == NULL) || (lock->lock == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return;
    }

    osal_kfree(lock->lock);
    lock->lock = NULL;
}
