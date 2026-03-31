/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
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
#ifndef _AGENTDRV_COMMON_MSG_H_
#define _AGENTDRV_COMMON_MSG_H_

#include "devdrv_interface.h"
#include "agentdrv_msg.h"
#include "agentdrv_unit.h"
#include "nvme_drv.h"

#define AGENTDEV_COMMON_WORK_QUEUE_RESQ_TIME 10

struct agentdrv_common_msg_ctx {
    int (*rx_func)(u32 devid, void *data, u32 in_data_len, u32 out_data_len, u32 *real_out_len);
    void (*init_notify)(u32 devid);
    int notify_flag[MAX_AGENTDEV_CNT];
    int work_flag;
    struct work_struct notify_work;
};

struct agentdrv_common_msg_stat {
    u64 tx_total_cnt;
    u64 tx_success_cnt;
    u64 tx_einval_err;
    u64 tx_enodev_err;
    u64 tx_enosys_err;
    u64 tx_etimedout_err;
    u64 tx_default_err;
    u64 rx_total_cnt;
    u64 rx_success_cnt;
    u64 rx_para_err;
    u64 rx_work_max_time;
    u64 rx_work_delay_cnt;
};

struct agentdrv_common_msg {
    struct agentdrv_msg_chan *msg_chan[MAX_AGENTDEV_CNT];
    struct agentdrv_common_msg_ctx msg_ctx[AGENTDRV_COMMON_MSG_TYPE_MAX];
    struct agentdrv_common_msg_stat com_msg_stat[MAX_AGENTDEV_CNT][AGENTDRV_COMMON_MSG_TYPE_MAX];
};

extern struct agentdrv_common_msg g_pcie_common_msg;

enum agentdrv_common_msg_enum {
    /* ram info */
    AGENTDRV_COMMON_MSG_GET_RAM_INFO = 0,
    AGENTDRV_COMMON_MSG_ENUM_MAX
};

int agentdrv_init_common_msg_chan(void *msg_chan);
void agentdrv_uninit_common_msg_chan(void *msg_chan);
int agentdrv_rx_msg_common_msg_process(void *msg_chan, void *data, u32 in_data_len, u32 out_data_len,
                                       u32 *real_out_len);
void agentdrv_init_common_msg(void);

struct agentdrv_common_msg_stat *agentdrv_get_common_msg_stat_by_type(u32 dev_id, u32 msg_type);

#endif
