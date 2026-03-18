#include "hi_osal.h"
#include "Bmc.h"

int osal_atomic_init(osal_atomic_t *atomic)
{
    sint32 *p = NULL;

    if (atomic == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }
    p = (sint32 *)osal_kmalloc(sizeof(sint32), 0);
    if (p == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "kmalloc error!\n");
        return -1;
    }

    (void)memset_s((void *)p, sizeof(sint32), 0, sizeof(sint32));
    atomic->atomic = (void *)p;
    return 0;
}

void osal_atomic_destory(osal_atomic_t *atomic)
{
    if ((atomic == NULL) || (atomic->atomic == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return;
    }

    osal_kfree(atomic->atomic);
    atomic->atomic = NULL;
}

hi_s32 osal_atomic_read(const osal_atomic_t *atomic)
{
    const volatile sint32 *p = NULL;
    if ((atomic == NULL) || (atomic->atomic == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "atomic is null!\n");
        return -1;
    }
    p = (sint32 *)(atomic->atomic);
    return Bmc_Load_s32(p);
}

void osal_atomic_set(const osal_atomic_t *atomic, hi_s32 i)
{
    const volatile sint32 *p = NULL;
    if ((atomic == NULL) || (atomic->atomic == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "atomic is null!\n");
        return;
    }
    p = (sint32 *)(atomic->atomic);
    Bmc_Store_s32(p, i);
}

hi_s32 osal_atomic_inc_return(const osal_atomic_t *atomic)
{
    const volatile sint32 *p = NULL;
    sint32 inc = 1;
    sint32 old_val;

    if ((atomic == NULL) || (atomic->atomic == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "atomic is null\n");
        return -1;
    }
    p = (sint32 *)(atomic->atomic);
    old_val = Bmc_FetchAdd_s32(p,  inc);
    return old_val + 1;
}

hi_s32 osal_atomic_dec_return(const osal_atomic_t *atomic)
{
    const volatile sint32 *p = NULL;
    sint32 dec = 1;
    sint32 old_val;
    if ((atomic == NULL) || (atomic->atomic == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "atomic invalid!\n");
        return -1;
    }
    p = (sint32 *)(atomic->atomic);
    old_val = Bmc_FetchSub_s32(p,  dec);
    return old_val - 1;
}