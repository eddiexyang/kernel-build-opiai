/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2016-2020.
 * Description: osal mutex source file.
 * Author: Hisilicon multimedia software group
 * Create: 2016-11-11
 */

#include "hi_osal.h"
#include "los_mux.h"

int osal_mutex_init(osal_mutex_t *mutex)
{
    UINT32 *p;
    UINT32 ret;

    if (mutex == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }

    p = (UINT32 *)osal_kmalloc(sizeof(UINT32), 0);
    if (p == NULL) {
        HI_TRACE_OSAL(HI_DBG_ERR, "kmalloc error!\n");
        return -1;
    }

    ret = LOS_MuxCreate(p);
    if (ret != LOS_OK) {
        HI_TRACE_OSAL(HI_DBG_ERR, "los mux create, ret=0x%x!\n", ret);
        osal_kfree(p);
        return -1;
    }

    mutex->mutex = p;
    return 0;
}

int osal_mutex_lock(const osal_mutex_t *mutex)
{
    UINT32 handle;
    UINT32 ret;
    if ((mutex == NULL) || (mutex->mutex == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return -1;
    }

    handle = *(UINT32 *)(mutex->mutex);
    ret = LOS_MuxPend(handle, LOS_WAIT_FOREVER);
    if (ret != LOS_OK) {
        HI_TRACE_OSAL(HI_DBG_ERR, "los mux pend, ret=0x%x!\n", ret);
        return -1;
    }
    return 0;
}

int osal_mutex_lock_interruptible(const osal_mutex_t *mutex)
{
    return osal_mutex_lock(mutex);
}

void osal_mutex_unlock(const osal_mutex_t *mutex)
{
    UINT32 handle;
    UINT32 ret;
    if ((mutex == NULL) || (mutex->mutex == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return;
    }

    handle = *(UINT32 *)(mutex->mutex);
    ret = LOS_MuxPost(handle);
    if (ret != LOS_OK) {
        HI_TRACE_OSAL(HI_DBG_ERR, "los mux pend, ret=0x%x!\n", ret);
        return;
    }
}

void osal_mutex_destory(osal_mutex_t *mutex)
{
    UINT32 handle;
    UINT32 ret;
    if ((mutex == NULL) || (mutex->mutex == NULL)) {
        HI_TRACE_OSAL(HI_DBG_ERR, "parameter invalid!\n");
        return;
    }

    handle = *(UINT32 *)(mutex->mutex);
    ret = LOS_MuxDelete(handle);
    if (ret != LOS_OK) {
        HI_TRACE_OSAL(HI_DBG_ERR, "los mux destroy, ret=0x%x!\n", ret);
    }
    osal_kfree(mutex->mutex);
    mutex->mutex = NULL;
}
