/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2016-2020.
 * Description: osal atomic source file.
 * Author: Hisilicon multimedia software group
 * Create: 2016-11-11
 */

#include "hi_osal.h"
#include "los_atomic.h"
#include "securec.h"

typedef Atomic atomic_t;
typedef Atomic64 atomic64_t;

#define ATOMIC_INIT(x) (x)
#define atomic_read(v) LOS_AtomicRead(v)
#define atomic_set(v, i) LOS_AtomicSet(v, i)
#define atomic_inc(v) LOS_AtomicInc(v)
#define atomic_dec(v) LOS_AtomicDec(v)
#define atomic_add(i, v) LOS_AtomicAdd(v, i)
#define atomic_sub(i, v) LOS_AtomicSub(v, i)

#define atomic_add_return(i, v) LOS_AtomicAdd(v, i)
#define atomic_inc_return(v) LOS_AtomicIncRet(v)
#define atomic_dec_return(v) LOS_AtomicDecRet(v)

#define ATOMIC64_INIT(x) (x)
#define atomic64_read(v) LOS_Atomic64Read(v)
#define atomic64_set(v, i) LOS_Atomic64Set(v, i)
#define atomic64_inc(v) LOS_Atomic64Inc(v)
#define atomic64_dec(v) LOS_Atomic64Dec(v)
#define atomic64_add(i, v) LOS_Atomic64Add(v, i)
#define atomic64_sub(i, v) LOS_Atomic64Sub(v, i)

#define atomic64_add_return(i, v) LOS_Atomic64Add(v, i)
#define atomic64_inc_return(v) LOS_Atomic64IncRet(v)
#define atomic64_dec_return(v) LOS_Atomic64DecRet(v)

int osal_atomic_init(osal_atomic_t *atomic)
{
    atomic_t *p = NULL;

    if (atomic == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }
    p = (atomic_t *)osal_kmalloc(sizeof(atomic_t), 0);
    if (p == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "kmalloc error!\n");
        return -1;
    }

    (void)memset_s((void *)p, sizeof(atomic_t), 0, sizeof(atomic_t));
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

int osal_atomic_read(const osal_atomic_t *atomic)
{
    atomic_t *p = NULL;

    if ((atomic == NULL) || (atomic->atomic == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }
    p = (atomic_t *)(atomic->atomic);
    return atomic_read(p);
}

void osal_atomic_set(const osal_atomic_t *atomic, hi_s32 i)
{
    atomic_t *p = NULL;

    if ((atomic == NULL) || (atomic->atomic == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return;
    }

    p = (atomic_t *)(atomic->atomic);
    atomic_set(p, i);
}

int osal_atomic_inc_return(const osal_atomic_t *atomic)
{
    atomic_t *p = NULL;

    if ((atomic == NULL) || (atomic->atomic == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }
    p = (atomic_t *)(atomic->atomic);
    return atomic_inc_return(p);
}

int osal_atomic_dec_return(const osal_atomic_t *atomic)
{
    atomic_t *p = NULL;

    if ((atomic == NULL) || (atomic->atomic == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }

    p = (atomic_t *)(atomic->atomic);
    return atomic_dec_return(p);
}
