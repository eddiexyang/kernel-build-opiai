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

#include "devdrv_ipc_msg.h"
#include "devdrv_manager_common.h"
#include "devdrv_platform_resource.h"
#include "icm_interface.h"


#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 16, 0)
#include <linux/sched/signal.h>
#endif

#define STATUS_WAIT 0x1
#define STATUS_MODIFIED 0x2


#ifdef CFG_SOC_PLATFORM_MDC_V51
STATIC struct safetyisland_ipc_recv_msg g_sils_kfifo[MAX_CHIP_NUM][SILS_IPC_CHANNEL_MAX];
void devdrv_safetyisland_ipc_init(u32 dev_id)
{
    int i;
    int ret;

    for (i = 0; i < SILS_IPC_CHANNEL_MAX; i++) {
        init_waitqueue_head(&g_sils_kfifo[dev_id][i].wait_queue);
        ret = kfifo_alloc(&g_sils_kfifo[dev_id][i].kfifo, SILS_IPC_MSG_KFIFO_SIZE, GFP_KERNEL | __GFP_ACCOUNT);
        if (ret != 0) {
            devdrv_drv_err("kfifo_alloc of sils ipc recv msg failed, i:%d, ret:%d, dev_id:%u\n", i, ret, dev_id);
            goto EXIT;
        }
        g_sils_kfifo[dev_id][i].in_kfifo = 0;
        g_sils_kfifo[dev_id][i].out_kfifo = 0;
        g_sils_kfifo[dev_id][i].lost_kfifo = 0;
    }

    return;
EXIT:
    for (i = i - 1; i >= 0; i--) {
        kfifo_free(&g_sils_kfifo[dev_id][i].kfifo);
    }
    return;
}

void devdrv_safetyisland_ipc_exit(u32 dev_id)
{
    int i;

    for (i = 0; i < SILS_IPC_CHANNEL_MAX; i++) {
        kfifo_free(&g_sils_kfifo[dev_id][i].kfifo);
    }

    return;
}

void devdrv_safetyisland_ipc_bh(u32 dev_id, u32 chan_id, void *data)
{
    unsigned long flags;
    unsigned char channel_id;
    struct IpcMessage *ipc_msg = NULL;

    if ((data == NULL) || (dev_id >= MAX_CHIP_NUM)) {
        devdrv_drv_err("ipc msg err,data:%pK,dev_id:%u.\n", data, dev_id);
        return;
    }

    devdrv_drv_info("get ipc msg from safetyisland, chan_id:%u, dev_id:%u.\n", chan_id, dev_id);
    if (chan_id == HISI_RPROC_SI_Q_TX_RPID17) {
        channel_id = SILS_IPC_CHANNEL_MASTER;
    } else if (chan_id == HISI_RPROC_SI_Q_TX_RPID15) {
        channel_id = SILS_IPC_CHANNEL_SLAVE;
    } else {
        devdrv_drv_err("unsupport chan_id:%u, dev_id:%u.\n", chan_id, dev_id);
        return;
    }
    ipc_msg = (struct IpcMessage *)data;
    if (atomic_read(&g_sils_kfifo[dev_id][channel_id].status) != SILS_IPC_STATUS_WAIT) {
        devdrv_drv_warn("no waiting list, drop ipc msg, channel_id:%u sequence:%u dev_id:%u.\n",
            channel_id, ipc_msg->head.sequence, dev_id);
        return;
    }
    spin_lock_irqsave(&g_sils_kfifo[dev_id][channel_id].kfifo_lock, flags);
    if (kfifo_avail(&g_sils_kfifo[dev_id][channel_id].kfifo) < SILS_IPC_MSG_ONE_SIZE) {
        g_sils_kfifo[dev_id][channel_id].lost_kfifo++;
        spin_unlock_irqrestore(&g_sils_kfifo[dev_id][channel_id].kfifo_lock, flags);
        devdrv_drv_err("g_sils_kfifo[%u][%u].lost_kfifo:%llu, kfifo is full, dev_id:%u.\n",
            dev_id, channel_id, g_sils_kfifo[dev_id][channel_id].lost_kfifo, dev_id);
        return;
    }
    kfifo_in(&g_sils_kfifo[dev_id][channel_id].kfifo, ipc_msg, sizeof(*ipc_msg));
    g_sils_kfifo[dev_id][channel_id].in_kfifo++;
    spin_unlock_irqrestore(&g_sils_kfifo[dev_id][channel_id].kfifo_lock, flags);

    atomic_set(&g_sils_kfifo[dev_id][channel_id].status, SILS_IPC_STATUS_MODIFIED);
    wake_up_interruptible(&g_sils_kfifo[dev_id][channel_id].wait_queue);

    return;
}
int devdrv_safetyisland_ipc_send(struct devIpcMessage *msg)
{
    int ret;
    unsigned int dev_id;
    rproc_id_t channel;

    if (msg == NULL) {
        devdrv_drv_err("ipc msg is null.\n");
        return -EINVAL;
    }

    dev_id = msg->devId;
    if (dev_id >= MAX_CHIP_NUM) {
        devdrv_drv_err("invalid msg->devId, dev_id:%u.\n", dev_id);
        return -EINVAL;
    }

#ifdef AOS_LLVM_BUILD
    channel = HISI_RPROC_SI_Q_TX_RPID17;
#else
    channel = HISI_RPROC_SI_Q_TX_RPID15;
#endif
    ret = icm_msg_send_async(ICM_FD_BUILD(dev_id, channel), (rproc_msg_t *)&msg->ipcMsg,
        sizeof(msg->ipcMsg) / sizeof(rproc_msg_t));
    if (ret != 0) {
        devdrv_drv_err("ipc message sync send failed, ret:%d dev_id:%u.\n", ret, dev_id);
        return -EFAULT;
    }

    return 0;
}

int devdrv_safetyisland_ipc_recv(struct devIpcMessage *msg)
{
    int ret;
    int len;
    unsigned int dev_id;
    unsigned long flags;
    unsigned char channel_id;

    if ((msg == NULL) || (msg->devId >= MAX_CHIP_NUM)) {
        devdrv_drv_err("msg is null:%u dev_id:%u.\n", (msg == NULL), (msg == NULL) ? MAX_CHIP_NUM : msg->devId);
        return -EINVAL;
    }

    dev_id = msg->devId;

    if (msg->channelId == SILS_IPC_CHANNEL_MASTER) {
        channel_id = SILS_IPC_CHANNEL_MASTER;
    } else if (msg->channelId == SILS_IPC_CHANNEL_SLAVE) {
        channel_id = SILS_IPC_CHANNEL_SLAVE;
    } else {
        devdrv_drv_err("unsupport channelId:%u dev_id:%u.\n", msg->channelId, dev_id);
        return -EINVAL;
    }

    if (atomic_read(&g_sils_kfifo[dev_id][channel_id].status) == SILS_IPC_STATUS_WAIT) {
        devdrv_drv_err("not support multiple call, channel_id:%u, dev_id:%u.\n", channel_id, dev_id);
        return -EPERM;
    }

    atomic_set(&g_sils_kfifo[dev_id][channel_id].status, SILS_IPC_STATUS_WAIT);
    ret = wait_event_interruptible(g_sils_kfifo[dev_id][channel_id].wait_queue,
        atomic_read(&g_sils_kfifo[dev_id][channel_id].status) == SILS_IPC_STATUS_MODIFIED);
    if (ret) {
        devdrv_drv_warn("wait event failed, status:%d ret:%d dev_id:%u.\n",
            g_sils_kfifo[dev_id][channel_id].status.counter, ret, dev_id);
        return -EFAULT;
    }

    spin_lock_irqsave(&g_sils_kfifo[dev_id][channel_id].kfifo_lock, flags);
    if (!kfifo_is_empty(&g_sils_kfifo[dev_id][channel_id].kfifo)) {
        len = kfifo_out(&g_sils_kfifo[dev_id][channel_id].kfifo, &msg->ipcMsg, SILS_IPC_MSG_ONE_SIZE);
        if (len != SILS_IPC_MSG_ONE_SIZE) {
            spin_unlock_irqrestore(&g_sils_kfifo[dev_id][channel_id].kfifo_lock, flags);
            devdrv_drv_err("kfifo_out ipc msg failed, len:%d channelId:%u dev_id:%u\n", len, channel_id, dev_id);
            return -EINVAL;
        }
        g_sils_kfifo[dev_id][channel_id].out_kfifo++;
    }
    spin_unlock_irqrestore(&g_sils_kfifo[dev_id][channel_id].kfifo_lock, flags);

    devdrv_drv_debug("[end] wait recv safetyisland ipc msg, g_sils_kfifo[%u][%u] in_kfifo:%llu out_kfifo:%llu \
        lost_kfifo:%llu dev_id:%u.\n", dev_id, channel_id, g_sils_kfifo[dev_id][channel_id].in_kfifo,
        g_sils_kfifo[dev_id][channel_id].out_kfifo, g_sils_kfifo[dev_id][channel_id].lost_kfifo, dev_id);

    return 0;
}
#endif

