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
#include "devdrv_platform.h"

#include "devdrv_ipc_msg.h"
#include "devdrv_ipc.h"
#include "icm_interface.h"


extern wait_queue_head_t ts_ipc_wait_queue[MAX_CHIP_NUM];
struct notifier_block mbox_lp_nb;

extern int devdrv_lpm3_notifier(struct notifier_block *nb, unsigned long len, void *data);

void devdrv_ipc_move_data(u8 dst[], u8 dst_len, u8 src[], u8 src_len)
{
    u8 i;

    if (dst_len >= src_len) {
        for (i = 0; i < src_len; i++)
            dst[i] = src[i];
    }
}


void devdrv_imu_dmp_msg_rx(struct devdrv_ipc_cmd_data *cmd_data, u8 *msg, u8 len) {}

int devdrv_ipc_msg_check(void)
{
    return 0;
}

int devdrv_ipc_send_to_imu(unsigned long arg)
{
    return -1;
}

int devdrv_ipc_recv_from_imu(unsigned long arg)
{
    return -1;
}

STATIC void devdrv_ipc_manager_init(u32 dev_id)
{
    init_waitqueue_head(&ts_ipc_wait_queue[dev_id]);
}

STATIC void devdrv_ipc_manager_exit(u32 dev_id)
{

}

int devdrv_lpm3_notifier(struct notifier_block *nb, unsigned long len, void *data)
{
    struct icmdrv_msg_chan *chan_info = NULL;
    struct devdrv_ipc_imu *msg = NULL;
    u8 msg_len = (u8)(sizeof(rproc_msg_t) * len);
    u32 dev_id;
    int ret = 0;

    if ((nb == NULL) || (data == NULL)) {
        devdrv_drv_err("data NULL=%d; nb NULL=%d\n", (data==NULL), (nb==NULL));
        return 0;
    }
    if (msg_len > MAX_IPCDRV_MSG_LENGTH) {
        devdrv_drv_err("ipc msg len %d error.\n", msg_len);
        return 0;
    }
    chan_info = container_of(nb, struct icmdrv_msg_chan, icm_monitor);
    if (chan_info == NULL || chan_info->magic != ICM_MAGIC_WORD) {
        return 0;
    }
    dev_id = chan_info->dev_id;
    if (dev_id >= MAX_CHIP_NUM) {
        devdrv_drv_err("device does not exist.\n");
        return 0;
    }

    msg = (struct devdrv_ipc_imu *)data;
    if (msg->source_id == LPM3_ID && msg->target_id == LPM3_EDP) {
        devdrv_m3_notice_aicore_freq_state(msg->cmd_para0);
    } else if (msg->source_id == LPM3_ID && msg->target_id == DEVICE_CCPU_ID
        && chan_info->chan_id == HISI_RPROC_RX_LPM3_MBX7) {
        ret = devdrv_lpm3_notifier_chan2(dev_id, len, data);
    } else if (msg->source_id == LPM3_ID && msg->target_id == DEVICE_CCPU_ID) {
        ret = devdrv_lpm3_notifier_handle(dev_id, len, data);
    } else {
        devdrv_drv_warn("ipc sync msg from lpm3, source_id (%d), target_id(%d).\n", msg->source_id, msg->target_id);
    }

    return ret;
}
int ipc_mailbox_rx_register(u32 dev_id)
{
    int ret;
    devdrv_ipc_manager_init(dev_id);

    mbox_lp_nb.notifier_call = devdrv_lpm3_notifier;
    ret = icm_rx_register(dev_id, HISI_RPROC_MAX, IPC_OBJ_LP, &mbox_lp_nb);
    if (ret != 0) {
        devdrv_drv_err("lpm3 mailbox MBX5 register fail...err:%d\n", ret);
        devdrv_ipc_manager_exit(dev_id);
        return ret;
    }

    return ret;
}

void ipc_mailbox_rx_unregister(u32 dev_id)
{
    devdrv_ipc_manager_exit(dev_id);
    (void)icm_rx_unregister(dev_id, HISI_RPROC_MAX, IPC_OBJ_LP, &mbox_lp_nb);
}

