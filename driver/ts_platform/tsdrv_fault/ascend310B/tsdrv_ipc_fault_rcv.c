/*
* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
* Create: 2023-06-26
*/
#include <linux/kernel.h>
#include <linux/securec.h>
#ifndef AOS_LLVM_BUILD
#include <linux/securectype.h>
#endif
#include <linux/cdev.h>
#include <linux/string.h>
#include <linux/notifier.h>
#ifndef AOS_LLVM_BUILD
#include <linux/cper.h>
#endif
#include <linux/uuid.h>
#include <linux/slab.h>
#ifndef AOS_LLVM_BUILD
#include <acpi/ghes.h>
#include <acpi/apei.h>
#endif
#include "tsdrv_ipc_fault_rcv.h"
#include "tsdrv_fault_init.h"
#include "tsdrv_safety_fault.h"
#include "tsdrv_common_fault_platform.h"
#include "tsdrv_log.h"
#include "ascend_kernel_hal.h"
#include "drv_ipc.h"
#include "drv_log.h"
#include "devdrv_manager_comm.h"
#include "devdrv_manager.h"
#include "devdrv_ipc.h"
#include "tsdrv_ipc.h"
#include "dms_sensor_notify.h"
#include "icm_interface.h"
#include "tsmng_interface.h"
#include "dfm_dev_register.h"
#include "tsdrv_ipc_fault_init.h"
#include "dfm_report.h"

const char *get_fault_source(enum ts_dev_node_id node_type_id)
{
#ifndef TSDRV_UT
    const char *fault_source[TS_DEV_NODE_ID_MAX] = {
        "from hwts", "from tscpu", "from aic", "from sdma", "from aiv", "from ts", "from hwtscpu", "from tsfw"};
    return fault_source[node_type_id];
#endif
}

int tsdrv_get_ipc_event(dfm_event *event, struct ras_message *info, enum ts_dev_node_id node_type_id)
{
#ifndef TSDRV_UT
    int ret;
    const char *fault_source = get_fault_source(node_type_id);
    event->subsys_id = info->node_type;
    event->module_id = info->node_type;
    event->section_type = RAS_SEC_OTHER;
    ret = strcpy_s((char *)&event->describe[0], DMS_MAX_EVENT_DATA_LENGTH, fault_source);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Copy fail. (ret=%d)\n", ret);
        return ret;
    }
    event->sensor_type = info->sensor_type;
    event->error_type = info->sensor_status;
    return 0;
#endif
}

#ifndef TSDRV_UT
void tsdrv_ipc_fault_sensor_scan_immediately(u32 devid, struct dfm_struct *dfm, struct ras_message *info)
{
    u32 i;
    struct dfm_node *dev_node = &dfm->dev_nodes[info->node_id];
    for (i = 0; i < dev_node->sensor_num; ++i) {
        if (dev_node->sensors[i].dms_sensor.sensor_type != info->sensor_type) {
            continue;
        }
        if (dms_sensor_event_notify(devid, &dev_node->sensors[i].dms_sensor) != 0) {
            TSDRV_PRINT_ERR("Notify sensor scan event failed. (node_type=0x%x; sensor_id=%u)\n",
                info->node_type, info->sensor_type);
        }
    }
    return;
}
#endif
#ifndef TSDRV_UT
STATIC int tsdrv_ipc_fault_msg_para_check(u32 devid, struct notifier_block *nb, struct icmdrv_ipc_msg_info *ipc_msg)
{
    struct dfm_struct *dfm = NULL;
    enum ts_dev_node_id node_type_id;
    struct ras_message *info = (struct ras_message *)(ipc_msg->data);

    if (ipc_msg->sub_cmd != ICM_SUB_CMD_ALARM) {
        return -EINVAL;
    }

    TSDRV_PRINT_INFO("Tsdrv ipc fault info. (ras_err_code=0x%llx, node_type=0x%x, node_id=0x%x, sensor_type=0x%x, "
        "sensor_status=0x%x, event_attr=0x%x)\n", info->ras_err_code, (u32)info->node_type, (u32)info->node_id,
        (u32)info->sensor_type, (u32)info->sensor_status, (u32)info->event_attr);

    if (devid >= fault_ctrl_get_num()) {
        TSDRV_PRINT_ERR("Invalid devid id. (devid=%u)\n", devid);
        return -EINVAL;
    }

    node_type_id = get_node_type_id_ipc(info->node_type);
    if (node_type_id == TS_DEV_NODE_ID_MAX) {
        TSDRV_PRINT_ERR("Invalid node type. (node_type=0x%x)\n", info->node_type);
        return -EINVAL;
    }
    dfm = tsdrv_get_dfm_struct(node_type_id);
    if (info->node_id >= dfm->node_num) {
        TSDRV_PRINT_ERR("Invalid node id. (node_id=%u)\n", info->node_id);
        return -EINVAL;
    }
    if (info->sensor_status >= RAS_ERROR_TYPE_ERR_TYPE_MAX || info->event_attr >= EVENT_ATTR_MAX) {
        TSDRV_PRINT_ERR("Invalid fault type. (sensor_status=%u; event_attr=%u; node_id=%u)\n",
            (u32)info->sensor_status, (u32)info->event_attr, info->node_id);
        return -EINVAL;
    }
    return 0;
}
#endif
void tsdrv_ipc_fault_event_handler(struct notifier_block *nb, unsigned long len, void *data)
{
#ifndef TSDRV_UT
    int ret;
    struct tsdrv_ipc_chan *ipc_chan = NULL;
    struct ras_message *info = NULL;
    enum ts_dev_node_id node_type_id;
    struct dfm_struct *dfm = NULL;
    dfm_event event = {0};

    struct icmdrv_ipc_msg_info *ipc_msg = (struct icmdrv_ipc_msg_info *)data;
    if (nb == NULL || data == NULL) {
        TSDRV_PRINT_ERR("Pointer is null.\n");
        return;
    }
    ipc_chan = notifier_block_to_ipc_chan(nb);
    ret = tsdrv_ipc_fault_msg_para_check(ipc_chan->devid, nb, ipc_msg);
    if (ret != 0) {
        return;
    }

    info = (struct ras_message *)(ipc_msg->data);
    node_type_id = get_node_type_id_ipc(info->node_type);
    dfm = tsdrv_get_dfm_struct(node_type_id);
    ret = tsdrv_get_ipc_event(&event, info, node_type_id);
    if (ret != 0) {
        return;
    }

    if (info->event_attr == EVENT_RESUME_ATTR) {
        ret = dfm_remove_event(dfm, info->node_id, &event);
        if (ret != 0) {
            TSDRV_PRINT_ERR("Dfm remove fail. (node_id=%u; ret=%d)\n", info->node_id, ret);
            return;
        }
        return;
    }

    ret = dfm_add_event(dfm, info->node_id, &event);
    if ((ret != 0) && (ret != -EEXIST)) {
        TSDRV_PRINT_ERR("Process fault event info failed. (devid=%u; node_id=%u; ret=%d)\n",
            ipc_chan->devid, info->node_id, ret);
    }

    if (ret == 0) {
        /* notify the sensor to scan event immediately */
        tsdrv_ipc_fault_sensor_scan_immediately(ipc_chan->devid, dfm, info);
    }

    return;
#endif
}
