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
#ifndef _AGENTDRV_MSG_TRANS_H_
#define _AGENTDRV_MSG_TRANS_H_

#include <linux/types.h>

#include "devdrv_interface.h"
#include "agentdrv_msg.h"

struct agentdrv_trans_msg_client_ctrl {
    u32 status;
    struct agentdrv_trans_msg_client trans_msg_client;
};

struct agentdrv_trans_msg_client *agentdrv_find_client_by_type(u32 type);

int agentdrv_register_trans_msg_client(const struct agentdrv_trans_msg_client *msg_client);
int agentdrv_unregister_trans_msg_client(const struct agentdrv_trans_msg_client *msg_client);

int agentdrv_trans_rx_msg_notify(struct agentdrv_msg_chan *chan);
int agentdrv_trans_tx_msg_finish(struct agentdrv_msg_chan *chan);
#endif
