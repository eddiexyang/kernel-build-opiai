#include "hi_osal.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#ifdef AOS_LLVM_BUILD
#include <linux/export.h>
#endif // #ifdef AOS_LLVM_BUILD
hi_s32 osal_sema_init(osal_semaphore_t *sem, hi_s32 val)
{
    struct semaphore *p = NULL;
    if (sem == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }
    p = osal_kmalloc(sizeof(struct semaphore), osal_gfp_kernel);
    if (p == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "kmalloc error!\n");
        return -1;
    }
    sema_init(p, val);
    sem->sem = p;
    return 0;
}
EXPORT_SYMBOL(osal_sema_init);

hi_s32 osal_down(const osal_semaphore_t *sem)
{
    struct semaphore *p = NULL;
    if (sem == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }
    p = (struct semaphore *)(sem->sem);
    if (p == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "sem->sem is NULL\n");
        return -1;
    }
    down(p);
    return 0;
}
EXPORT_SYMBOL(osal_down);
int osal_down_timeout(osal_semaphore_t *sem, long msecs)
{
    struct semaphore *p = NULL;
    long osal_jiffies = 0;
    if (sem == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }
    p = (struct semaphore *)(sem->sem);
    if (p == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "sem->sem is NULL\n");
        return -1;
    }
    osal_jiffies = osal_msecs_to_jiffies(msecs);
    return down_timeout(p, osal_jiffies);
}
EXPORT_SYMBOL(osal_down_timeout); //lint !e508
hi_s32 osal_down_interruptible(const osal_semaphore_t *sem)
{
    struct semaphore *p = NULL;
    if (sem == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }
    p = (struct semaphore *)(sem->sem);
    if (p == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "sem->sem is NULL\n");
        return -1;
    }
    return down_interruptible(p);
}
EXPORT_SYMBOL(osal_down_interruptible);

hi_s32 osal_down_trylock(const osal_semaphore_t *sem)
{
    struct semaphore *p = NULL;
    if (sem == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }
    p = (struct semaphore *)(sem->sem);
    if (p == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "sem->sem is NULL\n");
        return -1;
    }
    return down_trylock(p);
}
EXPORT_SYMBOL(osal_down_trylock);

void osal_up(const osal_semaphore_t *sem)
{
    struct semaphore *p = NULL;
    if (sem == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "sem is NULL\n");
        return;
    }
    p = (struct semaphore *)(sem->sem);
    if (p == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "sem->sem is NULL\n");
        return;
    }
    up(p);
}
EXPORT_SYMBOL(osal_up);

void osal_sema_destroy(osal_semaphore_t *sem)
{
    const struct semaphore *p = NULL;
    if (sem == NULL) {
        return;
    }
    p = (struct semaphore *)(sem->sem);
    if (p != NULL) {
        osal_kfree(p);
        sem->sem = NULL;
    }
}
EXPORT_SYMBOL(osal_sema_destroy);

void osal_sema_destory(osal_semaphore_t *sem)
__attribute__((alias("osal_sema_destroy")));
EXPORT_SYMBOL(osal_sema_destory);
