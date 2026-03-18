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
* Create: 2023-05-09
*/
#ifdef CFG_FEATURE_FAULT

#include "tsmng_interface.h"
#include "icm_interface.h"
#include "drv_ipc.h"
#include "tsdrv_ipc.h"
#include "tsdrv_fault_init.h"
#include "tsdrv_fault_comm.h"
#include "tsdrv_log.h"
#include "tsdrv_non_ras.h"

#if (defined(CFG_SOC_PLATFORM_MINI) && !defined(CFG_SOC_PLATFORM_MINIV2) && !defined(CFG_SOC_PLATFORM_MINIV3))
#else
static int tsdrv_enqueue_fault_event(u32 devid, u32 sensor_node,
    const struct ras_message *info)
{
    struct ras_error error = {0};
    int ret;

    error.node_type = info->node_type;
    error.sensor_status = info->sensor_status;
    error.event_attr = info->event_attr;

    if (devid >= TSDRV_MAX_DAVINCI_NUM) {
        TSDRV_PRINT_ERR("invalid device id. (devid=%u)\n", devid);
        return -EINVAL;
    }

    ret = tsdrv_fault_event_process(devid, info->node_type, 0, sensor_node, &error);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Process fault event info faild. (devid=%u; node_id=%u; tsid=%d; sensor_node=%u; ret=%d)\n",
            devid, (u32)info->node_id, 0, sensor_node, ret);
        return ret;
    }

    return 0;
}

static bool tsdrv_is_fault_ipc_node_type(u32 node_id)
{
    return ((node_id == DEV_NODE_HWTS_STARS) || (node_id == DEV_NODE_TS) || (node_id == DEV_NODE_AIC) ||
        (node_id == DEV_NODE_SDMAM) || (node_id == DEV_NODE_AIV) || (node_id == DEV_NODE_DSA) ||
        (node_id == DEV_NODE_TSFW));
}

STATIC int tsdrv_check_fault_ipc_data(struct ras_message *info, u32 devid)
{
    u32 node_id = get_devnode_id(info->node_type);
    int node_type;

    if (!tsdrv_is_fault_ipc_node_type(node_id)) {
        TSDRV_PRINT_ERR("Device node type is not from ts. (devid=%u; node_id=0x%x)\n", devid, node_id);
        return -EINVAL;
    }

    node_type = fault_get_node_type(devid, node_id);
    if (info->node_type != node_type) {
        TSDRV_PRINT_ERR("The node_type is invalid. (devid=%u; node_id=%u; node_type=0x%x; info->node_type=0x%x)\n",
            devid, node_id, node_type, info->node_type);
        return -EINVAL;
    }

    if (info->sensor_status >= RAS_ERROR_TYPE_ERR_TYPE_MAX || info->event_attr >= EVENT_ATTR_MAX) {
        TSDRV_PRINT_ERR("Invalid fault type. (sensor_status=%u; event_attr=%u; node_id=%u)\n",
            (u32)info->sensor_status, (u32)info->event_attr, node_id);
        return -EINVAL;
    }

    return 0;
}

void tsdrv_ts_fault_msg_handle(struct notifier_block *nb, unsigned long len, void *data)
{
    struct tsdrv_ipc_chan *ipc_chan = NULL;
#ifdef CFG_FEATURE_USE_ICM_CHAN
    struct icmdrv_ipc_msg_info *ipc_msg = (struct icmdrv_ipc_msg_info *)data;
#else
    struct ipcdrv_message *ipc_msg = (struct ipcdrv_message *)data;
#endif
    struct ras_message *info = NULL;
    u32 devid;
    u32 node_id;
    int ret;

    ipc_chan = notifier_block_to_ipc_chan(nb);
    devid = ipc_chan->devid;

#ifdef CFG_FEATURE_USE_ICM_CHAN
    info = (struct ras_message *)(ipc_msg->data);
    if (ipc_msg->sub_cmd != ICM_SUB_CMD_ALARM) {
        return;
    }
#else
    info = (struct ras_message *)(ipc_msg->ipcdrv_payload);
    if (tsmng_chk_ipc_crc16(ipc_msg) != 0) {
        TSDRV_PRINT_ERR("crc mismatched\n");
        return;
    }
#endif
    node_id = get_devnode_id(info->node_type);
    TSDRV_PRINT_DEBUG("Debug. (ras_err_code=0x%llx; node_type=0x%x; node_id=0x%x; sensor_type=0x%x; "
        "sensor_status=0x%x; event_attr=0x%x).\n", info->ras_err_code, (u32)info->node_type, node_id,
        (u32)info->sensor_type, (u32)info->sensor_status, (u32)info->event_attr);
    ret = tsdrv_check_fault_ipc_data(info, devid);
    if (ret != 0) {
        TSDRV_PRINT_ERR("IPC data is invalid. (devid=%u; sensor_status=%u; node_type=%u; node_id=%u; ret=%d)\n",
            devid, (u32)info->sensor_status, (u32)info->node_type, node_id, ret);
        return;
    }

    info->node_id = (u8)node_id;
    (void)tsdrv_enqueue_fault_event(devid, get_sensor_node_id(info->node_type), info);
}
#endif
#endif

