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
#include "drv_ipc_common.h"

#ifdef CFG_SOC_PLATFORM_MINIV3
int drv_ipc_target_lp_handler(struct drv_ipc_user_msg *user_msg)
{
    return -ENOSYS;
}

int drv_ipc_target_imu_handler(struct drv_ipc_user_msg *user_msg)
{
    return -ENOSYS;
}
#endif

static struct drv_ipc_module_info g_drv_ipc_table[MAX_TARGET_ID] = {
    [TARGET_ID_LP] = {
        .target_id = TARGET_ID_LP,
        .drv_ipc_handler = drv_ipc_target_lp_handler,
        .drv_ipc_notifier = drv_ipc_notifier_handle,
    },

    [TARGET_ID_IMU] = {
        .target_id = TARGET_ID_IMU,
        .drv_ipc_handler = drv_ipc_target_imu_handler,
        .drv_ipc_notifier = drv_ipc_notifier_handle,
    },

    [TARGET_ID_TSC] = {
        .target_id = TARGET_ID_TSC,
        .drv_ipc_handler = drv_ipc_target_tsc_handler,
        .drv_ipc_notifier = drv_ipc_notifier_handle,
    },

    [TARGET_ID_TSV] = {
        .target_id = TARGET_ID_TSV,
        .drv_ipc_handler = drv_ipc_target_tsv_handler,
        .drv_ipc_notifier = drv_ipc_notifier_handle,
    },

    [TARGET_ID_SAFETYISLAND] = {
        .target_id = TARGET_ID_SAFETYISLAND,
        .drv_ipc_handler = drv_ipc_target_safetyisland_handler,
        .drv_ipc_notifier = drv_ipc_notifier_handle,
    },
};

int drv_ipc_target_tsc_handler(struct drv_ipc_user_msg *user_msg)
{
    return -ENOSYS;
}

int drv_ipc_target_tsv_handler(struct drv_ipc_user_msg *user_msg)
{
    return -ENOSYS;
}

int drv_ipc_target_safetyisland_handler(struct drv_ipc_user_msg *user_msg)
{
    return -ENOSYS;
}

int drv_ipc_notifier_handle(struct notifier_block *nb, unsigned long len, void *data)
{
    return -ENOSYS;
}

STATIC int drv_ipc_msg_pack(struct drv_ipc_user_msg *user_msg, struct ioctl_ipc *ipc_data)
{
    struct drv_ipc_msg *ipc_msg = NULL;
    struct ipc_msg *msg;

    msg = &ipc_data->msg;
    if ((msg->send_buf == NULL && msg->send_len != 0) ||
        (msg->recv_buf == NULL && msg->recv_len != 0)) {
        devdrv_drv_err("invalid para send_len(%u), recv_len(%u).\n",
            msg->send_len, msg->recv_len);
        return -EINVAL;
    }

    if (ipc_data->dev_id >= DEVDRV_MAX_DAVINCI_NUM ||
        ipc_data->main_cmd >= MAX_IPC_MAIN_CMD_NUM ||
        ipc_data->sub_cmd >= MAX_IPC_SUB_CMD_NUM ||
        ipc_data->target_id >= MAX_TARGET_ID) {
        devdrv_drv_err("invalid para main_cmd(%d), sub_cmd(%d), targer_id(%d).\n",
            ipc_data->main_cmd, ipc_data->sub_cmd, ipc_data->target_id);
        return -EINVAL;
    }

    user_msg->dev_id = ipc_data->dev_id;
    user_msg->target_id = ipc_data->target_id;
    user_msg->main_cmd = ipc_data->main_cmd;
    user_msg->sub_cmd = ipc_data->sub_cmd;
    user_msg->priv = ipc_data;

    ipc_msg = &user_msg->msg;
    ipc_msg->cmd_type = ipc_data->main_cmd;  //main cmd
    ipc_msg->cmd_id = ipc_data->sub_cmd;     //sub cmd
    ipc_msg->dest = ipc_data->target_id;     //target id

    return 0;
}

int drv_ipc_msg_request(struct file *filep, unsigned int cmd, unsigned long arg)
{
    struct drv_ipc_user_msg user_msg = {0};
    struct ioctl_ipc ioctl_data = {0};
    int ret;

    ret = copy_from_user_safe(&ioctl_data, (void *)((uintptr_t)arg), sizeof(struct ioctl_ipc));
    if (ret) {
        devdrv_drv_err("copy_from_user_safe fail, ret(%d).\n", ret);
        return ret;
    }

    ret = drv_ipc_msg_pack(&user_msg, &ioctl_data);
    if (ret) {
        devdrv_drv_err("check_ipc_msg fail, ret(%d).\n", ret);
        return ret;
    }

    if (ioctl_data.target_id >= MAX_TARGET_ID) {
        devdrv_drv_err("target_id invalid: (%d).\n", ioctl_data.target_id);
        return -EINVAL;
    }

    ret = g_drv_ipc_table[ioctl_data.target_id].drv_ipc_handler(&user_msg);
    if (ret) {
        devdrv_drv_err("drv_ipc_send_unified_msg, ret(%d).\n", ret);
        return ret;
    }

    ret = copy_to_user_safe((void *)((uintptr_t)arg), &ioctl_data, sizeof(struct ioctl_ipc));
    if (ret) {
        devdrv_drv_err("copy_to_user_safe fail, ret(%d).\n", ret);
        return ret;
    }

    return 0;
}
