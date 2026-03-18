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
#ifndef _AGENTDRV_MSG_NON_TRANS_H_
#define _AGENTDRV_MSG_NON_TRANS_H_

#include <linux/types.h>
#include <linux/workqueue.h>

#include "devdrv_interface.h"
#include "agentdrv_msg.h"

#define AGENTDRV_DMA_COPY_RETRY_TIME 3000

#define DEVDRV_MSG_RETRY_MAX_CYCLE 50

struct agentdrv_non_trans_msg_client_ctrl {
    u32 status;
    struct agentdrv_non_trans_msg_client non_trans_msg_client;
};

struct agentdrv_non_trans_msg_send_data_para {
    void *data;
    u32 in_data_len;
    u32 out_data_len;
    u32 *real_out_len;
};

int agentdrv_sync_non_trans_msg_send(struct agentdrv_msg_chan *msg_chan, void *data, u32 in_data_len, u32 out_data_len,
                                     u32 *real_out_len, enum agentdrv_common_msg_type msg_type);
struct agentdrv_non_trans_msg_client *agentdrv_find_non_trans_msg_client_by_type(u32 type);
int agentdrv_non_trans_wakeup_sq_db_work(struct agentdrv_msg_chan *msg_chan);
void agentdrv_non_trans_rx_msg_task(struct work_struct *p_work);
#endif
