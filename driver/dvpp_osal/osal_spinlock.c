#include "hi_osal.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#ifdef AOS_LLVM_BUILD
#include <linux/export.h>
#endif // #ifdef AOS_LLVM_BUILD
hi_s32 osal_spin_lock_init(osal_spinlock_t *lock)
{
    spinlock_t *p = NULL;
    if (lock == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }

    p = (spinlock_t *)osal_kmalloc(sizeof(spinlock_t), osal_gfp_kernel);
    if (p == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "kmalloc error!\n");
        return -1;
    }
    spin_lock_init(p);
    lock->lock = p;
    return 0;
}
EXPORT_SYMBOL(osal_spin_lock_init);

void osal_spin_lock(const osal_spinlock_t *lock)
{
    spinlock_t *p = NULL;
    if (lock == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return;
    }

    p = (spinlock_t *)(lock->lock);
    if (p == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "spin_lock fail, lock->lock is null!\n");
        return;
    }
    spin_lock(p);
}
EXPORT_SYMBOL(osal_spin_lock);

hi_s32 osal_spin_trylock(const osal_spinlock_t *lock)
{
    spinlock_t *p = NULL;
    if (lock == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }
    p = (spinlock_t *)(lock->lock);
    if (p == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "spin_trylock fail, lock->lock is null!\n");
        return -1;
    }
    return spin_trylock(p);
}
EXPORT_SYMBOL(osal_spin_trylock);

void osal_spin_unlock(const osal_spinlock_t *lock)
{
    spinlock_t *p = NULL;
    if (lock == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return;
    }

    p = (spinlock_t *)(lock->lock);
    if (p == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "spin_unlock fail, lock->lock is null!\n");
        return;
    }
    spin_unlock(p);
}
EXPORT_SYMBOL(osal_spin_unlock);

void osal_spin_lock_irqsave(const osal_spinlock_t *lock, hi_ulong *flags)
{
    spinlock_t *p = NULL;
    hi_ulong f;
    if ((lock == NULL) || (flags == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return;
    }

    p = (spinlock_t *)(lock->lock);
    if (p == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "osal_spin_lock_irqsave fail, lock->lock is null!\n");
        return;
    }
    spin_lock_irqsave(p, f);
    *flags = f;
}
EXPORT_SYMBOL(osal_spin_lock_irqsave);

void osal_spin_unlock_irqrestore(const osal_spinlock_t *lock, const hi_ulong *flags)
{
    spinlock_t *p = NULL;
    hi_ulong f;
    if ((lock == NULL) || (flags == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return;
    }

    p = (spinlock_t *)(lock->lock);
    if (p == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "osal_spin_unlock_irqrestore fail, lock->lock is null!\n");
        return;
    }
    f = *flags;
    spin_unlock_irqrestore(p, f);
}
EXPORT_SYMBOL(osal_spin_unlock_irqrestore);

void osal_spin_lock_destroy(osal_spinlock_t *lock)
{
    const spinlock_t *p = NULL;
    if (lock == NULL) {
        return;
    }
    p = (spinlock_t *)(lock->lock);
    if (p != NULL) {
        osal_kfree(p);
        lock->lock = NULL;
    }
}
EXPORT_SYMBOL(osal_spin_lock_destroy);

void osal_spin_lock_destory(osal_spinlock_t *lock)
__attribute__((alias("osal_spin_lock_destroy")));
EXPORT_SYMBOL(osal_spin_lock_destory);
