/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2016-2020.
 * Description: osal semaphore source file.
 * Author: Hisilicon multimedia software group
 * Create: 2016-11-11
 */

#include "hi_osal.h"
#include "los_sem_pri.h"

hi_s32 osal_sema_init(osal_semaphore_t *sem, hi_s32 val)
{
    UINT32 *p = NULL;
    UINT32 ret = 0;

    if (sem == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }

    p = (UINT32 *)osal_kmalloc(sizeof(UINT32), 0);
    if (p == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "kmalloc error!\n");
        return -1;
    }

    ret = LOS_SemCreate((UINT16)val, p);
    if (ret != LOS_OK) {
        osal_kfree(p);
        HI_TRACE_OSAL(HI_DBG_ERR, "los create sem, ret = 0x%x!\n", ret);
        return -1;
    }
    sem->sem = p;
    return 0;
}

int osal_down(const osal_semaphore_t *sem)
{
    UINT32 sem_handle;
    UINT32 ret;
    if ((sem == NULL) || (sem->sem == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }

    sem_handle = *(UINT32 *)(sem->sem);
    ret = LOS_SemPend(sem_handle, LOS_WAIT_FOREVER);
    if (ret == LOS_OK) {
        return 0;
    }

    HI_TRACE_OSAL(HI_DBG_ERR, "los sem pend, ret = 0x%x!\n", ret);
    return -1;
}

int osal_down_interruptible(const osal_semaphore_t *sem)
{
    return osal_down(sem);
}

int osal_down_trylock(const osal_semaphore_t *sem)
{
    UINT32 sem_handle;
    UINT32 ret;
    if ((sem == NULL) || (sem->sem == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }

    sem_handle = *(UINT32 *)(sem->sem);
    ret = LOS_SemPend(sem_handle, LOS_NO_WAIT);
    if (ret == LOS_OK) {
        return 0;
    }

    return -1;
}

void osal_up(const osal_semaphore_t *sem)
{
    UINT32 sem_handle;
    UINT32 ret;
    if ((sem == NULL) || (sem->sem == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return;
    }

    sem_handle = *(UINT32 *)(sem->sem);
    ret = LOS_SemPost(sem_handle);
    if (ret == LOS_OK) {
        return;
    }

    HI_TRACE_OSAL(HI_DBG_ERR, "los sem up, ret = 0x%x!\n", ret);
    return;
}

void osal_sema_destory(osal_semaphore_t *sem)
{
    UINT32 sem_handle;
    UINT32 ret;
    if ((sem == NULL) || (sem->sem == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return;
    }

    sem_handle = *(UINT32 *)(sem->sem);
    ret = LOS_SemDelete(sem_handle);
    if (ret != LOS_OK) {
        HI_TRACE_OSAL(HI_DBG_ERR, " los sem delete, ret = 0x%x!\n", ret);
    }

    osal_kfree(sem->sem);
    sem->sem = NULL;
}
