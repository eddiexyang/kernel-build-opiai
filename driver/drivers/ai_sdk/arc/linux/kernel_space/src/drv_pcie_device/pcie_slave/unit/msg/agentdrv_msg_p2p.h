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
#ifndef _AGENTDRV_MSG_P2P_H_
#define _AGENTDRV_MSG_P2P_H_

#include "agentdrv_msg.h"
#include "agentdrv_unit.h"

struct agentdrv_p2p_msg_send_data_para {
    void *data;
    u32 data_len;
    u32 in_len;
    u32 *out_len;
};

int agentdrv_cfg_p2p_msg_chan(struct agentdrv_msg_dev *msg_dev, void *data);
int agentdrv_cfg_p2p_tx_atu(struct agentdrv_msg_dev *msg_dev, void *data);

void agentdrv_p2p_msg_chan_init(struct agentdrv_msg_dev *msg_dev, int dst_devid, u64 msg_base, u64 db_base, u32 op);
void agentdrv_p2p_msg_chan_doorbell_proc(struct agentdrv_p2p_msg_chan *chan);
void agentdrv_p2p_msg_init(struct agentdrv_msg_dev *msg_dev);
void agentdrv_p2p_msg_exit(struct agentdrv_msg_dev *msg_dev);
#endif
