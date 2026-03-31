/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: tde handle
 * Author: Hisilicon multimedia software group
 * Create: 2018/10/30
 */

#include "tde_define.h"
#include "tde_handle.h"
#include "tde_check_para.h"

/********************** Global Variable declaration ****************************************/
tde_handle_mgr *g_tde_handle_list = TD_NULL; /* Manager list of global handle */

static td_s32 g_handle = 1;

static osal_spinlock_t g_handlelock;

tde_handle_mgr *tde_get_handle_list(td_void)
{
    return g_tde_handle_list;
}
td_s32 initial_handle(td_void)
{
    td_s32 ret;
    if (g_tde_handle_list == TD_NULL) {
        g_tde_handle_list = (tde_handle_mgr *)osal_kmalloc(sizeof(tde_handle_mgr), osal_gfp_kernel);
        if (g_tde_handle_list == TD_NULL) {
            return TD_FAILURE;
        }

        /* Initialize list head */
        OSAL_INIT_LIST_HEAD(&g_tde_handle_list->list_head);
        ret = osal_spin_lock_init(&g_tde_handle_list->lock);
        if (ret != 0) {
            tde_error("Initialize g_tde_handle_list->lock failed!\n");
            osal_kfree(g_tde_handle_list);
            g_tde_handle_list = NULL;
            return TD_FAILURE;
        }
        ret = osal_spin_lock_init(&g_handlelock);
        if (ret != 0) {
            tde_error("Initialize g_handlelock failed!\n");
            osal_spin_lock_destroy(&g_tde_handle_list->lock);
            osal_kfree(g_tde_handle_list);
            g_tde_handle_list = NULL;
            return TD_FAILURE;
        }
    }
    g_handle = 1;
    return TD_SUCCESS;
}

td_void get_handle(tde_handle_mgr *handle_mgr, td_s32 *handle)
{
    td_ulong lockflags = 0;
    td_ulong handlockflags = 0;

    tde_check_nullpointer_return_novalue(handle_mgr);
    tde_check_nullpointer_return_novalue(handle);
    tde_check_nullpointer_return_novalue(g_tde_handle_list);
    tde_spin_lock(&g_handlelock, handlockflags);
    /* Jump over unlawful handle */
    if (g_handle == TDE_MAX_HANDLE_VALUE) {
        g_handle = 1;
    }
    handle_mgr->handle = g_handle;
    *handle = g_handle++;
    tde_spin_unlock(&g_handlelock, handlockflags);
    tde_spin_lock(&g_tde_handle_list->lock, lockflags);
    osal_list_add_tail(&handle_mgr->list_head, &g_tde_handle_list->list_head);
    tde_spin_unlock(&g_tde_handle_list->lock, lockflags);
    return;
}

/* To accelerate the speed of find, find form back to front */
td_bool tde_query_handle(td_s32 handle, tde_handle_mgr **res)
{
    unsigned long lockflags;
    tde_handle_mgr *hdl = TD_NULL;
    if (res == TD_NULL) {
        return TD_FALSE;
    }
    if (g_tde_handle_list == TD_NULL) {
        return TD_FALSE;
    }
    tde_spin_lock(&g_tde_handle_list->lock, lockflags);
    hdl = osal_list_entry(g_tde_handle_list->list_head.prev, tde_handle_mgr, list_head);
    if (hdl == TD_NULL) {
        tde_spin_unlock(&g_tde_handle_list->lock, lockflags);
        return TD_FALSE;
    }
    while (hdl != g_tde_handle_list) {
        if (hdl->handle == handle) {
            *res = hdl;
            tde_spin_unlock(&g_tde_handle_list->lock, lockflags);
            return TD_TRUE;
        }
        hdl = osal_list_entry(hdl->list_head.prev, tde_handle_mgr, list_head);
        if (hdl == TD_NULL) {
            tde_spin_unlock(&g_tde_handle_list->lock, lockflags);
            return TD_FALSE;
        }
    }
    tde_spin_unlock(&g_tde_handle_list->lock, lockflags);
    return TD_FALSE;
}

td_bool release_handle(td_s32 handle)
{
    td_ulong lockflags = 0;
    tde_handle_mgr *pst_handle = TD_NULL;
    tde_check_nullpointer_return_value(g_tde_handle_list, TD_FALSE);
    tde_spin_lock(&g_tde_handle_list->lock, lockflags);
    pst_handle = osal_list_entry(g_tde_handle_list->list_head.next, tde_handle_mgr, list_head);
    if (pst_handle == TD_NULL) {
        tde_spin_unlock(&g_tde_handle_list->lock, lockflags);
        return TD_FALSE;
    }
    while (pst_handle != g_tde_handle_list) {
        if (pst_handle->handle == handle) {
            osal_list_del_init(&pst_handle->list_head);
            tde_spin_unlock(&g_tde_handle_list->lock, lockflags);
            return TD_TRUE;
        }
        pst_handle = osal_list_entry(pst_handle->list_head.next, tde_handle_mgr, list_head);
        if (pst_handle == TD_NULL) {
            tde_spin_unlock(&g_tde_handle_list->lock, lockflags);
            return TD_FALSE;
        }
    }
    tde_spin_unlock(&g_tde_handle_list->lock, lockflags);
    return TD_FALSE;
}

td_void destroy_handle(td_void)
{
    /* Free head node, note: other nodes are all loaded, their resource  are responsibilited by its own module */
    if (g_tde_handle_list != TD_NULL) {
        osal_spin_lock_destroy(&g_tde_handle_list->lock);
        osal_spin_lock_destroy(&g_handlelock);
        osal_kfree(g_tde_handle_list);

        g_tde_handle_list = TD_NULL;
    }

    return;
}
