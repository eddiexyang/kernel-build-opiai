/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2023. All rights reserved.
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kallsyms.h>
#include <linux/errno.h>
#include <linux/pci.h>
#include <linux/jiffies.h>
#include <linux/securec.h>
#include <linux/sched.h>

#include "icm_core.h"
#include "icm_ipc.h"

struct icmdrv_ctrl *icm_ctrl = NULL;

struct icmdrv_chan_adapter icm_adapter = {
    .chan_type = ICM_CHAN_TYPE_IPC,
    .chan_num = HISI_RPROC_MAX,
    .init = icm_ipc_res_init,
    .uninit = icm_ipc_res_uninit,
    .alloc = icm_ipc_msg_chan_alloc,
    .free = icm_ipc_msg_chan_free,
    .send_async = icm_ipc_msg_send_async,
    .fast_send_async = icm_ipc_msg_fast_send_async,
    .send_sync = icm_ipc_msg_send_sync,
    .rx_register = icm_ipc_rx_register,
    .rx_unregister = icm_ipc_rx_unregister,
};

int icmdrv_init(void)
{
    u32 i, j;

    icm_ctrl = (struct icmdrv_ctrl *)vzalloc(sizeof(struct icmdrv_ctrl));
    if (icm_ctrl == NULL) {
        icm_err("Calling alloc failed. (size=%ld)\n", sizeof(struct icmdrv_ctrl));
        return -EINVAL;
    }

    for (i = 0; i < MAX_CHIP_NUM; i++) {
        icm_ctrl->msg_chan[i] = (struct icmdrv_msg_chan *)vzalloc(sizeof(struct icmdrv_msg_chan)
            * icm_adapter.chan_num);
        if (icm_ctrl->msg_chan[i] == NULL) {
            icm_err("Calling alloc failed. (size=%ld, i=%u)\n",
                sizeof(struct icmdrv_msg_chan) * icm_adapter.chan_num, i);
            goto OUT;
        }
        for (j = 0; j < icm_adapter.chan_num; j++) {
            icm_ctrl->msg_chan[i][j].dev_id = i;
            icm_ctrl->msg_chan[i][j].chan_id = j;
            icm_ctrl->msg_chan[i][j].magic = ICM_MAGIC_WORD;
            atomic_set(&icm_ctrl->msg_chan[i][j].wait_flag, 0);
            init_waitqueue_head(&icm_ctrl->msg_chan[i][j].wait_queue);
            mutex_init(&icm_ctrl->msg_chan[i][j].psci_mutex);
            icm_ctrl->msg_chan[i][j].wait_time = ICM_WAIT_TIMEOUT;
            icm_ctrl->msg_chan[i][j].wait_timeout_count = 0;
        }
    }

    icm_ctrl->adapter = &icm_adapter;

    if (icm_ctrl->adapter->init != NULL) {
        icm_ctrl->adapter->init();
    }

    return 0;

OUT:
    for (j = 0; j < i; j++) {
        vfree(icm_ctrl->msg_chan[j]);
        icm_ctrl->msg_chan[j] = NULL;
    }
    vfree(icm_ctrl);
    icm_ctrl = NULL;
    return -EINVAL;
}

void icmdrv_uninit(void)
{
    u32 i, j;

    if (icm_ctrl->adapter->uninit != NULL) {
        icm_ctrl->adapter->uninit();
    }

    for (i = 0; i < MAX_CHIP_NUM; i++) {
        for (j = 0; j < icm_adapter.chan_num; j++) {
            mutex_destroy(&icm_ctrl->msg_chan[i][j].psci_mutex);
        }
        vfree(icm_ctrl->msg_chan[i]);
        icm_ctrl->msg_chan[i] = NULL;
    }

    icm_ctrl->adapter = NULL;
    vfree(icm_ctrl);
    icm_ctrl = NULL;
}

STATIC int __init icmdrv_init_module(void)
{
    int ret;

    ret = icmdrv_init();
    if (ret != 0) {
        icm_err("Icm_init failed, (ret=%d)\n", ret);
        return ret;
    }

    icm_info("Init_module success\n");
    return 0;
}

STATIC void __exit icmdrv_exit_module(void)
{
    icmdrv_uninit();

    icm_info("Exit_module success\n");
}

module_init(icmdrv_init_module);
module_exit(icmdrv_exit_module);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("icm device driver");

