/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 and
* only version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* Description:
* Author: huawei
* Create: 2019-10-15
*/

#include <linux/errno.h>
#include <linux/slab.h>
#include "dms_define.h"
#include "dms_event.h"
#include "dms_event_distribute.h"
#include "dms_event_converge.h"
#include "dms_event_distribute_proc.h"
#include "dms_event_bbox_map.h"
#include "dms_kernel_version_adapt.h"
#include "kernel_version_adapt.h"
add_exception_handle g_add_exception_func = NULL;

#if (!defined AOS_LLVM_BUILD) && (!defined CFG_FEATURE_NEW_EVENT_CODE)
static struct dms_event_bbox_map *dms_event_bbox_map(DMS_EVENT_NODE_STRU *exception_node)
{
    unsigned int map_size, i;

    map_size = sizeof(g_event_bbox_map) / sizeof(struct dms_event_bbox_map);

    for (i = 0; i < map_size; i++) {
        if ((exception_node->event.event_id == g_event_bbox_map[i].event_id) &&
            (exception_node->event.assertion == g_event_bbox_map[i].assertion)) {
            return &g_event_bbox_map[i];
        }
    }

    return NULL;
}
#endif

int dms_event_set_add_exception_handle(add_exception_handle func)
{
    g_add_exception_func = func;
    return 0;
}
EXPORT_SYMBOL(dms_event_set_add_exception_handle);

int dms_event_call_add_exception_handle(u32 devid, u32 code)
{
    struct timespec stamp = current_kernel_time();
    if (g_add_exception_func != NULL) {
        return g_add_exception_func(devid, code, stamp);
    }
    return 0;
}

static int dms_event_distribute_to_bbox(DMS_EVENT_NODE_STRU *exception_node)
{
#if (!defined AOS_LLVM_BUILD) && (!defined CFG_FEATURE_NEW_EVENT_CODE)
    struct dms_event_bbox_map *bbox_map = NULL;
    int ret = DRV_ERROR_NONE;

#ifdef CFG_HOST_ENV
    if (!DMS_EVENT_CODE_IS_HOST(exception_node->event.event_id)) {
        return ret;
    }
#else
    if (!DMS_EVENT_CODE_IS_DEVICE(exception_node->event.event_id)) {
        return ret;
    }
#endif
    bbox_map = dms_event_bbox_map(exception_node);
    if (bbox_map == NULL) {
#ifdef CFG_HOST_ENV
        dms_warn("Unsupport bbox event_code. (dev_id=%u; event_code=0x%x)\n",
            exception_node->event.deviceid, exception_node->event.event_code);
#endif
        return ret;
    }
    ret = dms_event_call_add_exception_handle(exception_node->event.deviceid, bbox_map->bbox_code);
    if (ret) {
        dms_err("Add event_id to bbox failed. (dev_id=%u; event_code=0x%x; ret=%d)\n",
            exception_node->event.deviceid, exception_node->event.event_code, ret);
        return ret;
    }
    dms_event("Add event_id to bbox. (dev_id=%u; event_id=0x%x;bbox_code=0x%x)\n",
        exception_node->event.deviceid, exception_node->event.event_id, bbox_map->bbox_code);
#endif
    return DRV_ERROR_NONE;
}

STATIC int dms_event_add_report_to_dfx(DMS_EVENT_NODE_STRU *exception_node)
{
    struct dms_event_dfx_table *dfx_table = NULL;
    struct dms_dev_ctrl_block *dev_cb = NULL;

    dev_cb = dms_get_dev_cb(exception_node->event.deviceid);
    if (dev_cb == NULL) {
        dms_err("Get dev_ctrl block failed. (dev_id=%u)\n", exception_node->event.deviceid);
        return DRV_ERROR_NO_DEVICE;
    }

    dfx_table = &dev_cb->dev_event_cb.dfx_table;
    if (exception_node->event.assertion < DMS_EVENT_TYPE_MAX) {
        atomic_inc(&dfx_table->report_to_consumer[exception_node->event.assertion]);
        return DRV_ERROR_NONE;
    }

    dms_err("Invalid parameter. (event_assertion=%u)\n", exception_node->event.assertion);
    return DRV_ERROR_PARA_ERROR;
}

int dms_event_distribute_handle_init(void)
{
    int ret_bbox, ret_process, ret_dfx;

    ret_bbox = dms_event_subscribe_register(dms_event_distribute_to_bbox, DMS_DISTRIBUTE_PRIORITY0);
    ret_process = dms_event_subscribe_register(dms_event_distribute_to_process, DMS_DISTRIBUTE_PRIORITY2);
    ret_dfx = dms_event_subscribe_register(dms_event_add_report_to_dfx, DMS_DISTRIBUTE_PRIORITY3);
    if (ret_bbox || ret_process || ret_dfx) {
        dms_err("Distribute handle register failed. "
                "(bbox=%d; process=%d; ret_dfx=%d)\n",
                ret_bbox, ret_process, ret_dfx);
        dms_event_subscribe_unregister_all();
        return DRV_ERROR_INNER_ERR;
    }

    return DRV_ERROR_NONE;
}

