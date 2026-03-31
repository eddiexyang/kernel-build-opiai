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


#include "devdrv_manager.h"
#include "devdrv_ipc.h"
#include "drv_ipc_common.h"
#include "icm_interface.h"

STATIC int drv_ipc_process_lp_data(struct drv_ipc_user_msg *user_msg, struct devdrv_ipc_imu *msg)
{
    struct ioctl_ipc *ioctl_data = NULL;
    u32 data;
    int ret;

    ioctl_data = (struct ioctl_ipc *)user_msg->priv;
    ioctl_data->msg.recv_len = sizeof(u32);
    switch (user_msg->msg.cmd_id) {
        case IPC_SUB_CMD_SOC_TEMP:
            data = (*(u32 *)&msg->cmd_data0 + PERCENT_INTER_MEDIATE_DATE) / PERCENT_UNIT;
            break;

        case IPC_SUB_CMD_AIC1_FREQ:
            data = msg->data1;
            break;

        case IPC_SUB_CMD_AIC0_FREQ:
        case IPC_SUB_CMD_DDR_FREQ:
        case IPC_SUB_CMD_AIV_FREQ:
        case IPC_SUB_CMD_CCPU_FREQ:
        case IPC_SUB_CMD_SOC_POWER:
            data =  *(u32 *)&msg->cmd_data0;
            break;

        default:
            return -ENOSYS;
    }

    ret = copy_to_user_safe((void *)((uintptr_t)ioctl_data->msg.recv_buf), &data, sizeof(u32));
    if (ret) {
        devdrv_drv_err("copy_from_user_safe fail, ret(%d).\n", ret);
        return ret;
    }
    return 0;
}

STATIC int drv_ipc_get_lp_cmdtype(struct drv_ipc_msg *ipc_msg, struct devdrv_ipc_imu *msg)
{
    if (ipc_msg->cmd_type != IPC_MAIN_CMD_QUERY) {
        devdrv_drv_err("invalid cmd type(%d).\n", ipc_msg->cmd_type);
        return -EINVAL;
    }

    switch (ipc_msg->cmd_id) {
        case IPC_SUB_CMD_SOC_TEMP:
            msg->cmd_type0 = CMD_INQUIRY;
            msg->cmd_type1 = SUB_CMD_QUERY_SOC_TEMP;
            return 0;

        case IPC_SUB_CMD_CCPU_FREQ:
            msg->cmd_type0 = CMD_INQUIRY;
            msg->cmd_type1 = SUB_CMD_QUERY_CTRLCPUFREQ;
            return 0;

        case IPC_SUB_CMD_AIC0_FREQ:
        case IPC_SUB_CMD_AIC1_FREQ:
            msg->cmd_type0 = CMD_INQUIRY;
            msg->cmd_type1 = SUB_CMD_QUERY_AICFREQ;
            return 0;

        case IPC_SUB_CMD_DDR_FREQ:
            msg->cmd_type0 = CMD_INQUIRY;
            msg->cmd_type1 = SUB_CMD_QUERY_DDR_FREQUENCY;
            return 0;

        case IPC_SUB_CMD_AIV_FREQ:
            msg->cmd_type0 = CMD_INQUIRY;
            msg->cmd_type1 = SUB_CMD_QUERY_AIVFREQ;
            return 0;

        case IPC_SUB_CMD_SOC_POWER:
            msg->cmd_type0 = CMD_INQUIRY;
            msg->cmd_type1 = SUB_CMD_QUERY_SOC_PWC;
            return 0;

        default:
            return -ENOSYS;
    }
}

int drv_ipc_target_imu_handler(struct drv_ipc_user_msg *user_msg)
{
    return -ENOSYS;
}

int drv_ipc_target_lp_handler(struct drv_ipc_user_msg *user_msg)
{
    struct devdrv_ipc_imu msg = {0};
    struct devdrv_ipc_imu ack = {0};
    u32 devid;
    int ret;

    if (user_msg == NULL || user_msg->priv == NULL) {
        devdrv_drv_err("invalid para, user_msg or priv is null.\n");
        return -EINVAL;
    }

    devid = user_msg->dev_id;
    msg.target_id = OBJ_IMU;
    msg.source_id = 0;
    ret = drv_ipc_get_lp_cmdtype(&user_msg->msg, &msg);
    if (ret) {
        devdrv_drv_err("drv_ipc_get_imu_cmdtype failed, ret=%d.\n", ret);
        return ret;
    }

    ret = icm_msg_send_sync(ICM_FD_BUILD(devid, HISI_RPROC_LP_Q_TX_RPID4_ACPU0), (rproc_msg_t *)&msg,
                        IPCDRV_RPROC_MSG_LENGTH, (rproc_msg_t *)&ack, IPCDRV_RPROC_MSG_LENGTH);
    if (ret) {
        devdrv_drv_err("ipc message send fail, ret(%d).\n", ret);
        return ret;
    }
    if ((ack.source_id != msg.target_id) || (ack.target_id != msg.source_id) ||
        (ack.cmd_type0 != msg.cmd_type0) || (ack.cmd_type1 != msg.cmd_type1)) {
        devdrv_drv_err("get invalid ipc message.\n"
                       "ack.source_id = %d, ack.target_id = %d.\n"
                       "ack.cmd_type0 = %d, ipc.cmd_type0 = %d.\n"
                       "ack.cmd_type1 = %d, ipc.cmd_type1 = %d.\n",
                       ack.source_id, ack.target_id,
                       ack.cmd_type0, msg.cmd_type0, ack.cmd_type1, msg.cmd_type1);
        return -EINVAL;
    }

    ret = drv_ipc_process_lp_data(user_msg, &ack);
    if (ret) {
        devdrv_drv_err("drv_ipc_process_imu_data fail, ret(%d).\n", ret);
        return ret;
    }

    return 0;
}
