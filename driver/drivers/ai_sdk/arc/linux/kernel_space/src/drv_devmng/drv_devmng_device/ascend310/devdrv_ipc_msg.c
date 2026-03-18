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

#include <linux/securec.h>
#include "devdrv_ipc_msg.h"
#include "devdrv_manager_common.h"
#include "devdrv_platform_resource.h"
#include "icm_interface.h"
#include "tsmng_interface.h"

#ifndef DEVMNG_UT
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 16, 0)
#include <linux/sched/signal.h>
#endif

#define DEVDRV_IPCMSG_PORT_NUM 256
struct devdrv_cce_context *cce_context_port[DEVDRV_IPCMSG_PORT_NUM];

struct ipc_message_manager {
    struct ipcdrv_msg ipc_message;
    int thread_stop;
    struct list_head list;
};

#define STATUS_WAIT 0x1
#define STATUS_MODIFIED 0x2

wait_queue_head_t ts_ipc_wait_queue[MAX_CHIP_NUM];
STATIC  atomic_t ts_ipc_recv_status[MAX_CHIP_NUM] = {0};
struct ipcdrv_message ts_recv_msg[MAX_CHIP_NUM];

int devdrv_ipc_ts_to_aicpu_hb(void *data, u32 dev_id)
{
    int ret;
    struct ipcdrv_message *ipc_msg = NULL;

    if (data == NULL || dev_id >= MAX_CHIP_NUM) {
        devdrv_drv_err("ipc msg err,data=%pK,dev_id=%d.\n", data, dev_id);
        return -1;
    }

    devdrv_drv_debug("get heart_breat_msg from ts to aicpu.\n");
    ipc_msg = (struct ipcdrv_message *)data;
    if (atomic_read(&ts_ipc_recv_status[dev_id]) != STATUS_WAIT) {
        devdrv_drv_warn("no waiting list, ipc msg will miss\n");
        return -1;
    }
    ret = memcpy_s((void *)&ts_recv_msg[dev_id], sizeof(struct ipcdrv_message), ipc_msg, sizeof(struct ipcdrv_message));
    if (ret != 0) {
        devdrv_drv_err("copy to ts_recv_msg fail, ret=%d\n", ret);
        return -1;
    }
    atomic_set(&ts_ipc_recv_status[dev_id], STATUS_MODIFIED);

    wake_up_interruptible(&ts_ipc_wait_queue[dev_id]);

    return 0;
}

void devdrv_ipc_aicpu_register(u32 dev_id)
{
    int ret;

    ret = tsmng_handler_register_ts_heartbeat_toaicpu(dev_id, devdrv_ipc_ts_to_aicpu_hb);
    if (ret != 0) {
        devdrv_drv_err("Register ipc aicpu handler failed.\n");
    }
}

int devdrv_ipc_msg_send_recv(struct devdrv_info *dev_info, unsigned long arg)
{
    int ret;
    struct ipcdrv_msg ipc_message = {
        .ipc_message = {
            .ipc_msg_header = {0},
            .ipcdrv_payload = {0}
                },
        .channel_type = 0,
        .eventCallbackfunc = NULL
    };

    if (dev_info == NULL || dev_info->dev_id >= MAX_CHIP_NUM) {
        devdrv_drv_err("ipc msg err,dev_info=%pK,dev_id=%d.\n", dev_info, (dev_info ? dev_info->dev_id : 0));
        return -EFAULT;
    }

    // wait recv
    atomic_set(&ts_ipc_recv_status[dev_info->dev_id], STATUS_WAIT);
    ret = wait_event_interruptible(ts_ipc_wait_queue[dev_info->dev_id],
        atomic_read(&ts_ipc_recv_status[dev_info->dev_id]) == STATUS_MODIFIED);
    if (ret) {
        if (ret == -ERESTARTSYS) {
            devdrv_drv_info("wait event interruptible: %d\n", ret);
            return -ERESTARTSYS;
        }
        return -EFAULT;
    }
    // copy msg to user
    ret = memcpy_s((void *)&ipc_message.ipc_message, sizeof(struct ipcdrv_message),
        (void *)&ts_recv_msg[dev_info->dev_id], sizeof(struct ipcdrv_message));
    if (ret != 0) {
        devdrv_drv_err("copy ipc_message failed\n");
        return -EFAULT;
    }
    if (copy_to_user_safe((void *)((uintptr_t)arg), (void *)&ipc_message, sizeof(struct ipcdrv_msg))) {
        devdrv_drv_err("copy to user failed\n");
        return -EFAULT;
    }

    return 0;
}

#define TS_AICPU_IPC_CMD 10
int devdrv_ipc_msg_send_async(struct devdrv_info *dev_info, unsigned long arg)
{
    struct ipcdrv_msg ipc_message = {
        .ipc_message = {
            .ipc_msg_header = {0},
            .ipcdrv_payload = {0}
        },
        .channel_type = 0,
        .eventCallbackfunc = NULL
    };

    if (copy_from_user_safe(&ipc_message, (void *)((uintptr_t)arg), sizeof(struct ipcdrv_msg))) {
        devdrv_drv_err("copy from user failed\n");
        return -EINVAL;
    }

    if (ipc_message.channel_type > HISI_RPROC_TX_LPM3) {
        devdrv_drv_err("invalid channel type, channel_type = %d\n", ipc_message.channel_type);
        return -EINVAL;
    }

    if (ipc_message.ipc_message.ipc_msg_header.cmd_type != TS_AICPU_IPC_CMD) {
        devdrv_drv_err("the cmd type (%u) not support!\n", ipc_message.ipc_message.ipc_msg_header.cmd_type);
        return -EINVAL;
    }

    if (icm_msg_send_async(ICM_FD_BUILD(dev_info->dev_id, (rproc_id_t)ipc_message.channel_type),
        (rproc_msg_t *)&ipc_message.ipc_message, IPCDRV_RPROC_MSG_LENGTH)) {
        devdrv_drv_err("ipc message sync send failed\n");
        return -EFAULT;
    }

    return 0;
}
#else
int devdrv_ipc_msg_send_recv(struct devdrv_info *dev_info, unsigned long arg)
{
    return 0;
}
int devdrv_ipc_msg_send_async(struct devdrv_info *dev_info, unsigned long arg)
{
    return 0;
}

#endif
