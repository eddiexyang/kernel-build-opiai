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

#include <linux/export.h>

#include "distribute/fpdc_distribute_notify.h"
#include "fpdc_define.h"
#include "log_restrain/log_restrain.h"


static FAULT_NOTIFY_FUNC g_notify_funcs[DMS_DEV_TYPE_MAX] = {0};
/*
 * Description: call registed function when node_type modules error occur
 * History£º2021-11-19
 */
int fpdc_register_fault_notifier(DMS_DEVICE_NODE_TYPE node_type, FAULT_NOTIFY_FUNC notify_func)
{
    if (node_type < DMS_DEV_TYPE_SOC || node_type >= DMS_DEV_TYPE_MAX) {
        fpdc_err("Invalid node type. (node_type=0x%x)\n", node_type);
        return -EINVAL;
    }
    if (notify_func == NULL) {
        fpdc_err("Notify func is NULL. (node_type=0x%x)\n", node_type);
        return -EINVAL;
    }
    if (g_notify_funcs[node_type] != NULL) {
        return 0;
    }
    fpdc_info("Register node type successfully. (node_type=0x%x)\n", node_type);
    g_notify_funcs[node_type] = notify_func;
    return 0;
}
EXPORT_SYMBOL(fpdc_register_fault_notifier);


int fpdc_unregister_fault_notifier(DMS_DEVICE_NODE_TYPE node_type)
{
    if (node_type < DMS_DEV_TYPE_SOC || node_type >= DMS_DEV_TYPE_MAX) {
        fpdc_err("Invalid node type. (node_type=0x%x)\n", node_type);
        return -EINVAL;
    }
    if (g_notify_funcs[node_type] == NULL) {
        return 0;
    }
    fpdc_info("Unregister node type successfully. (node_type=0x%x; func=%d)\n", node_type, !!g_notify_funcs[node_type]);
    g_notify_funcs[node_type] = NULL;
    return 0;
}
EXPORT_SYMBOL(fpdc_unregister_fault_notifier);

#define DISTRIBUTE_UNREG_LOG_PERIOD 60
#define DISTRIBUTE_UNREG_LOG_MAX    10
/*
 * Description: various sources use this function notify error
 * History£º2021-11-19
 */
int fpdc_distribute_notify(const struct notify_data *pdata)
{
    static struct log_res restrain = {0};

    if (pdata == NULL) {
        fpdc_err("The pdata is NULL.\n");
        return -EINVAL;
    }
    if (pdata->node_type >= DMS_DEV_TYPE_MAX) {
        fpdc_err("Invalid node type. (node_type=0x%x)\n", pdata->node_type);
        return -EINVAL;
    }
    if (g_notify_funcs[pdata->node_type] == NULL) {
        if (!log_restrain(&restrain, DISTRIBUTE_UNREG_LOG_PERIOD, DISTRIBUTE_UNREG_LOG_MAX, __func__)) {
            fpdc_event("Unregisted node type. (node_type=0x%x)\n", pdata->node_type);
        }
        return -ENOENT;
    }
    g_notify_funcs[pdata->node_type](pdata);
    return 0;
}


