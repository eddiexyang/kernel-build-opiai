#include "hi_osal.h"
#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/atomic.h>
#include <linux/printk.h>
#include <linux/slab.h>
#ifdef AOS_LLVM_BUILD
#include <linux/export.h>
#include <linux/atomic.h>
#endif
#include "securec.h"

hi_s32 osal_atomic_init(osal_atomic_t *atomic)
{
    hi_s32 ret = 0;
    atomic_t *p = NULL;
    if (atomic == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }
    p = (atomic_t *)osal_kmalloc(sizeof(atomic_t), osal_gfp_kernel);
    if (p == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "kmalloc error!\n");
        return -1;
    }
    ret = memset_s(p, sizeof(atomic_t), 0, sizeof(atomic_t));
    if (ret != 0) {
        HI_TRACE_OSAL(HI_DBG_ERR, "memset p failed! Ret = %d\n", ret);
        osal_kfree(p);
        p = NULL;
        return -1;
    }
    atomic->atomic = p;
    return 0;
}
EXPORT_SYMBOL(osal_atomic_init);

void osal_atomic_destroy(osal_atomic_t *atomic)
{
    if (atomic == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "atomic is null\n");
        return;
    }
    if (atomic->atomic == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "atomic->atomic is null\n");
        return;
    }
    osal_kfree(atomic->atomic);
    atomic->atomic = NULL;
}
EXPORT_SYMBOL(osal_atomic_destroy);

void osal_atomic_destory(osal_atomic_t *atomic)
__attribute__((alias("osal_atomic_destroy")));
EXPORT_SYMBOL(osal_atomic_destory);

hi_s32 osal_atomic_read(const osal_atomic_t *atomic)
{
    const atomic_t *p = NULL;
    if (atomic == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "atomic is null!\n");
        return -1;
    }
    if (atomic->atomic == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "atomic->atomic is null\n");
        return -1;
    }
    p = (atomic_t *)(atomic->atomic);
    return atomic_read(p);
}
EXPORT_SYMBOL(osal_atomic_read);

void osal_atomic_set(const osal_atomic_t *atomic, hi_s32 i)
{
    atomic_t *p = NULL;
    if (atomic == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "atomic is null\n");
        return;
    }
    if (atomic->atomic == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "atomic->atomic is null\n");
        return;
    }
    p = (atomic_t *)(atomic->atomic);
    atomic_set(p, i);
}
EXPORT_SYMBOL(osal_atomic_set);

hi_s32 osal_atomic_inc_return(const osal_atomic_t *atomic)
{
    atomic_t *p = NULL;
    if (atomic == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "atomic is null\n");
        return -1;
    }
    if (atomic->atomic == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "atomic->atomic is null\n");
        return -1;
    }
    p = (atomic_t *)(atomic->atomic);
    return atomic_inc_return(p);
}
EXPORT_SYMBOL(osal_atomic_inc_return);

hi_s32 osal_atomic_dec_return(const osal_atomic_t *atomic)
{
    atomic_t *p = NULL;
    if (atomic == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "atomic invalid!\n");
        return -1;
    }
    p = (atomic_t *)(atomic->atomic);
    if (p == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "atomic->atomic is NULL!\n");
        return -1;
    }
    return atomic_dec_return(p);
}
EXPORT_SYMBOL(osal_atomic_dec_return);