#include "hi_osal.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#ifdef AOS_LLVM_BUILD
#include <linux/export.h>
#endif // #ifdef AOS_LLVM_BUILD
hi_s32 osal_mutex_init(osal_mutex_t *mutex)
{
    struct mutex *p = NULL;
    if (mutex == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }
    p = osal_kmalloc(sizeof(struct mutex), osal_gfp_kernel);
    if (p == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "kmalloc error!\n");
        return -1;
    }
    mutex_init(p);
    mutex->mutex = p;
    return 0;
}
EXPORT_SYMBOL(osal_mutex_init);

hi_s32 osal_mutex_lock(const osal_mutex_t *mutex)
{
    struct mutex *p = NULL;
    if (mutex == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }
    p = (struct mutex *)(mutex->mutex);
    if (p == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "mutex_lock fail, mutex->mutex is null!\n");
        return -1;
    }
    mutex_lock(p);
    return 0;
}
EXPORT_SYMBOL(osal_mutex_lock);

hi_s32 osal_mutex_lock_interruptible(const osal_mutex_t *mutex)
{
    struct mutex *p = NULL;
    if (mutex == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }
    p = (struct mutex *)(mutex->mutex);
    if (p == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "lock_interruptible fail, mutex->mutex is null!\n");
        return -1;
    }
    return mutex_lock_interruptible(p);
}
EXPORT_SYMBOL(osal_mutex_lock_interruptible);

hi_s32 osal_mutex_trylock(const osal_mutex_t *mutex)
{
    struct mutex *p = NULL;
    if (mutex == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }
    p = (struct mutex *)(mutex->mutex);
    if (p == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "trylock fail, mutex->mutex is null!\n");
        return -1;
    }
    return mutex_trylock(p);
}
EXPORT_SYMBOL(osal_mutex_trylock);

void osal_mutex_unlock(const osal_mutex_t *mutex)
{
    struct mutex *p = NULL;
    if (mutex == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return;
    }
    p = (struct mutex *)(mutex->mutex);
    if (p == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "unlock fail, mutex->mutex is null!\n");
        return;
    }
    mutex_unlock(p);
}
EXPORT_SYMBOL(osal_mutex_unlock);

void osal_mutex_destroy(osal_mutex_t *mutex)
{
    const struct mutex *p = NULL;
    if (mutex == NULL) {
        return;
    }
    p = (struct mutex *)(mutex->mutex);
    if (p != NULL) {
        osal_kfree(p);
        mutex->mutex = NULL;
    }
}
EXPORT_SYMBOL(osal_mutex_destroy);

void osal_mutex_destory(osal_mutex_t *mutex)
__attribute__((alias("osal_mutex_destroy")));
EXPORT_SYMBOL(osal_mutex_destory);
