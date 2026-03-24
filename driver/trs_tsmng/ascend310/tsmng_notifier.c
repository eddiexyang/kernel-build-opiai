/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
 * Create: 2022-08-13
 */
#ifndef TSDRV_UT

/* ===must include */
#include "tsmng_interface.h"
#include "tsmng_log.h"
#include "tsmng_common.h"
/* must include=== */

#include "tsmng_pm.h"
#include "devdrv_platform_resource.h"

static struct notifier_block mbox_ts_nb;

static int devdrv_ipc_notifier(struct notifier_block *nb, unsigned long len, void *data)
{
    struct icmdrv_msg_chan *mbx_info = NULL;
    struct ipcdrv_message *ipc_msg = NULL;
    int dev_id;
    u32 type;

    if ((data == NULL) || (nb == NULL)) {
        tsmng_drv_err("Data NULL=%d; nb NULL=%d\n", (data==NULL), (nb==NULL));
        return 0;
    }

    mbx_info = container_of(nb, struct icmdrv_msg_chan, icm_monitor);
    if (mbx_info == NULL || mbx_info->magic != ICM_MAGIC_WORD) {
        return 0;
    }
    dev_id = mbx_info->dev_id;
    if (dev_id >= MAX_CHIP_NUM) {
        tsmng_drv_err("Device does not exist. (dev_id=%u; max_chip_num=%u)\n", dev_id, MAX_CHIP_NUM);
        return -EINVAL;
    }

    ipc_msg = (struct ipcdrv_message *)data;
    type = ipc_msg->ipc_msg_header.cmd_type;

    tsmng_drv_debug("Receive ipc messages, cmd_type: %d.\n", type);

    if (type == IPCDRV_TS_INFORM_TS_IDLE) {
        (void)tsmng_inform_ts_idle((void *)ipc_msg, dev_id);
    } else if ((type == IPCDRV_TS_UPPER_DDR_FREQ) || (type == IPCDRV_TS_LOWER_DDR_FREQ)) {
        (void)tsmng_ipc_chan_proc(type, (void *)ipc_msg, dev_id);
    } else if (type == IPCDRV_TS_HEARTBEAT_TOAICPU) {
        (void)tsmng_ipc_chan_proc(type, (void *)ipc_msg, dev_id);
    }

    return 0;
}

int tsmng_mailbox_rx_register(u32 dev_id)
{
    int ret;

    if (dev_id >= MAX_CHIP_NUM) {
        tsmng_drv_err("Dev id is invalid. (dev_id=%u; max_chip_num=%d)\n", dev_id, MAX_CHIP_NUM);
        return -EINVAL;
    }

    tsmng_ipc_manager_init(dev_id);

    mbox_ts_nb.notifier_call = devdrv_ipc_notifier;
    ret = icm_rx_register(dev_id, HISI_RPROC_MAX, IPC_OBJ_TS, &mbox_ts_nb);
    if (ret != 0) {
        tsmng_drv_err("Ts mailbox register fail. (ret=%d)\n", ret);
        tsmng_ipc_manager_exit(dev_id);
        return ret;
    }

    return ret;
}
EXPORT_SYMBOL(tsmng_mailbox_rx_register);

void tsmng_mailbox_rx_unregister(u32 dev_id)
{
    if (dev_id >= MAX_CHIP_NUM) {
        tsmng_drv_err("Dev id is invalid. (dev_id=%u; max_chip_num=%d)\n", dev_id, MAX_CHIP_NUM);
        return;
    }

    (void)icm_rx_unregister(dev_id, HISI_RPROC_MAX, IPC_OBJ_TS, &mbox_ts_nb);
    tsmng_ipc_manager_exit(dev_id);
}
EXPORT_SYMBOL(tsmng_mailbox_rx_unregister);

#else
void tsmng_notifier_310_ut_stub(void)
{
    return;
}
#endif
