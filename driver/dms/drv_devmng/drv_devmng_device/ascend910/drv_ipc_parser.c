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

#ifdef DEVMNGI_MINI_UT
#define MAILBOX_TO_IMU_MBX1       1
#define HISI_RPROC_TX_IMU_MBX25   5
#endif

STATIC int drv_ipc_process_imu_data(struct drv_ipc_user_msg *user_msg, struct devdrv_ipc_imu *msg)
{
    struct ioctl_ipc *ioctl_data = NULL;
    struct _vrd *vrd_temp = NULL;
    int data1;
    u32 data2;
    int ret;

    ioctl_data = (struct ioctl_ipc *)user_msg->priv;
    ioctl_data->msg.recv_len = sizeof(u32);
    data2 = ((msg->cmd_data3) << IPC_DATA3_OFFSET) | ((msg->cmd_data2) << IPC_DATA2_OFFSET) |
            ((msg->cmd_data1) << IPC_DATA1_OFFSET) | (msg->cmd_data0);
    switch (user_msg->msg.cmd_id) {
        case IPC_SUB_CMD_SOC_TEMP:
            data1 = *((s8 *)&msg->cmd_data0);
            data2 = (u32)data1;
            break;

        case IPC_SUB_CMD_HBM_TEMP:
            data1 = *((s8 *)&msg->cmd_data1);
            data2 = (u32)data1;
            break;

        case IPC_SUB_CMD_NDIE_TEMP:
            data1 = *((s8 *)&msg->cmd_data2);
            data2 = (u32)data1;
            break;

        case IPC_SUB_CMD_VRD_TEMP:
            data2 = 0;
            vrd_temp = (struct _vrd *)&data2;
            vrd_temp->vrd_temp_0 = (char)msg->cmd_data3;
            vrd_temp->vrd_temp_1 = (char)(msg->data1 & IPC_VRD_MASK);
            vrd_temp->vrd_temp_2 = (char)((msg->data1  >> IPC_VRD_SHIFT) & IPC_VRD_MASK);
            break;

        case IPC_SUB_CMD_AIC0_FREQ:
        case IPC_SUB_CMD_DDR_FREQ:
        case IPC_SUB_CMD_HBM_FREQ:
        case IPC_SUB_CMD_SOC_POWER:
            break;

        default:
            return -ENOSYS;
    }

    ret = copy_to_user_safe((void *)((uintptr_t)ioctl_data->msg.recv_buf), &data2, sizeof(u32));
    if (ret) {
        devdrv_drv_err("copy_from_user_safe fail, ret(%d).\n", ret);
        return ret;
    }

    return 0;

}

STATIC int drv_ipc_get_imu_cmdtype(struct drv_ipc_msg *ipc_msg, struct devdrv_ipc_imu *msg)
{
    if (ipc_msg->cmd_type != IPC_MAIN_CMD_QUERY) {
        devdrv_drv_err("invalid cmd type(%d).\n", ipc_msg->cmd_type);
        return -EINVAL;
    }

    switch (ipc_msg->cmd_id) {
        case IPC_SUB_CMD_SOC_TEMP:
        case IPC_SUB_CMD_HBM_TEMP:
        case IPC_SUB_CMD_NDIE_TEMP:
        case IPC_SUB_CMD_VRD_TEMP:
            msg->cmd_type0 = IPC_IMU_TEMP;
            msg->cmd_type1 = IPC_IMU_TEMP_1;
            return 0;

        case IPC_SUB_CMD_AIC0_FREQ:
            msg->cmd_type0 = IPC_IMU_AICORE;
            msg->cmd_type1 = CMD_INQUIRY;
            return 0;

        case IPC_SUB_CMD_DDR_FREQ:
            msg->cmd_type0 = IPC_IMU_DDR;
            msg->cmd_type1 = IPC_IMU_FREQ_1;
            return 0;

        case IPC_SUB_CMD_HBM_FREQ:
            msg->cmd_type0 = IPC_IMU_HBM;
            msg->cmd_type1 = IPC_IMU_FREQ_1;
            return 0;

        case IPC_SUB_CMD_SOC_POWER:
            msg->cmd_type0 = IPC_IMU_POWER;
            msg->cmd_type1 = IPC_IMU_POWER_1;
            return 0;

        default:
            return -ENOSYS;
    }
}

int drv_ipc_target_imu_handler(struct drv_ipc_user_msg *user_msg)
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
    msg.target_id = OBJ_IMU_DMP;
    msg.source_id = OBJ_AP_DMP;
    ret = drv_ipc_get_imu_cmdtype(&user_msg->msg, &msg);
    if (ret) {
        devdrv_drv_err("drv_ipc_get_imu_cmdtype failed, ret=%d.\n", ret);
        return ret;
    }

    ret = icm_msg_send_sync(ICM_FD_BUILD(devid, HISI_RPROC_TX_IMU_MBX25), (rproc_msg_t *)&msg,
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

    ret = drv_ipc_process_imu_data(user_msg, &ack);
    if (ret) {
        devdrv_drv_err("drv_ipc_process_imu_data fail, ret(%d).\n", ret);
        return ret;
    }

    return 0;
}

int drv_ipc_target_lp_handler(struct drv_ipc_user_msg *user_msg)
{
    return -ENOSYS;
}
