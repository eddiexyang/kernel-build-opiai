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

#ifndef _AGENTDRV_SYSFS_H_
#define _AGENTDRV_SYSFS_H_

#include <linux/types.h>
#include "agentdrv_unit.h"
#include "agentdrv_common_msg.h"

#define AGENTDRV_SYSFS_RX_IN_0_LANE0 0x14405c
#define AGENTDRV_SYSFS_RX_IN_0_LANE1 0x14445c
#define AGENTDRV_SYSFS_RX_IN_0_LANE2 0x14485c
#define AGENTDRV_SYSFS_RX_IN_0_LANE3 0x144c5c
#define AGENTDRV_SYSFS_RX_IN_1_LANE0 0x144060
#define AGENTDRV_SYSFS_RX_IN_1_LANE1 0x144460
#define AGENTDRV_SYSFS_RX_IN_1_LANE2 0x144860
#define AGENTDRV_SYSFS_RX_IN_1_LANE3 0x144c60

#define AGENTDRV_SYSFS_TX_IN_1_LANE0 0x144048
#define AGENTDRV_SYSFS_TX_IN_1_LANE1 0x144448
#define AGENTDRV_SYSFS_TX_IN_1_LANE2 0x144848
#define AGENTDRV_SYSFS_TX_IN_1_LANE3 0x144c48
#define AGENTDRV_SYSFS_TX_IN_2_LANE0 0x14404c
#define AGENTDRV_SYSFS_TX_IN_2_LANE1 0x14444c
#define AGENTDRV_SYSFS_TX_IN_2_LANE2 0x14484c
#define AGENTDRV_SYSFS_TX_IN_2_LANE3 0x144c4c

#define AGENTDRV_SYSFS_AER_CNT_INFO_REG 0x1042BC

#define AGENTDRV_SYSFS_RX_LANE_MAX 4
#define AGENTDRV_SYSFS_TX_LANE_MAX 4

#define AGENTDRV_SYSFS_MSG_OUT_DATA_LEN 128

#define AGENTDRV_SYSFS_DMA_CHAN_NUM 6

enum agentdrv_sysfs_msg_type {
    AGENTDRV_SYSFS_RX_PARA = 0,
    AGENTDRV_SYSFS_TX_PARA,
    AGENTDRV_SYSFS_LINK_INFO,
    AGENTDRV_SYSFS_AER_COUNT,
    AGENTDRV_SYSFS_AER_CLEAR,
    AGENTDRV_SYSFS_COMMON_MSG,
    AGENTDRV_SYSFS_NON_TRANS_MSG,
    AGENTDRV_SYSFS_SYNC_DMA_INFO,
    AGENTDRV_SYSFS_MSG_TYPE_MAX
};

struct agentdrv_sysfs_link_info {
    enum agentdrv_sysfs_msg_type type;
    u32 link_speed;
    u32 link_width;
    u32 link_status;
};

struct agentdrv_sysfs_rx_lane_para {
    u32 att;
    u32 gain;
    u32 boost;
    u32 tap1;
    u32 tap2;
    u32 valid;
};
struct agentdrv_sysfs_rx_para {
    u32 lane_count;
    struct agentdrv_sysfs_rx_lane_para lane_rx_para[AGENTDRV_SYSFS_RX_LANE_MAX];
};

struct agentdrv_sysfs_tx_lane_para {
    u32 pre;
    u32 main;
    u32 post;
    u32 valid;
};
struct agentdrv_sysfs_tx_para {
    u32 lane_count;
    struct agentdrv_sysfs_tx_lane_para lane_tx_para[AGENTDRV_SYSFS_TX_LANE_MAX];
};

struct agentdrv_sysfs_msg {
    enum agentdrv_sysfs_msg_type type;
    union {
        u32 data[AGENTDRV_SYSFS_MSG_OUT_DATA_LEN];
        struct agentdrv_sysfs_link_info link_info;
        struct agentdrv_sysfs_rx_para rx_para;
        struct agentdrv_sysfs_tx_para tx_para;
        struct agentdrv_common_msg_stat com_msg_stat[AGENTDRV_COMMON_MSG_TYPE_MAX];
        struct agentdrv_msg_chan_stat chan_stat[agentdrv_msg_client_max];
        struct devdrv_sync_dma_stat sync_dma_stat[AGENTDRV_SYSFS_DMA_CHAN_NUM];
    };
};

void agentdrv_sysfs_get_link_info(struct agentdrv_devctrl *agent_dev, struct agentdrv_sysfs_msg *msg);

void agentdrv_sysfs_get_rx_para(struct agentdrv_devctrl *agent_dev, struct agentdrv_sysfs_msg *msg);

void agentdrv_sysfs_get_tx_para(struct agentdrv_devctrl *agent_dev, struct agentdrv_sysfs_msg *msg);
void agentdrv_sysfs_get_aer_count(struct agentdrv_devctrl *agent_dev, struct agentdrv_sysfs_msg *msg);

void agentdrv_sysfs_clear_aer_count(struct agentdrv_devctrl *agent_dev, struct agentdrv_sysfs_msg *msg);
int agentdrv_sysfs_msg_recv(u32 devid, void *data, u32 in_data_len, u32 out_data_len, u32 *real_out_len);
void agentdrv_sysfs_comm_client_register(void);
struct agentdrv_msg_chan *agentdrv_get_common_msg_chan_by_id(u32 dev_id);
struct agentdrv_msg_dev *agentdrv_get_msgdev(int dev_id);

#endif
