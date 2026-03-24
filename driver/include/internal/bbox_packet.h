/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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

#ifndef BBOX_PACKET_H
#define BBOX_PACKET_H

#include "device/bbox_types.h"
#include "bbox_communication.h"
#include "bbox_message.h"

#define BBOX_MSG_UNKOWN (-1)
#define BBOX_MSG_SYNC   0
#define BBOX_MSG_ASYNC  1

#define RESEND_GAP      1000
#define SERIES_MAX      20

/* enum type of segment flag */
enum bbox_pkt_seg_flag {
    BBOX_PKT_SEG_NULL = 0, // no multi-msg received
    BBOX_PKT_SEG_STRT = 1, // the first segment pkt
    BBOX_PKT_SEG_CONT = 2, // middle segment pkts
    BBOX_PKT_SEG_FINL = 3  // the last segment pkt
};

struct bbox_packet_header {
    u16 type;       // stream type: up-stream or down-stream
    u16 devid;      // physical device id
    u32 p_len;      // whole packet length
    u32 m_len;      // multi-packet whole length
    u16 sequence;   // sequence number
    u8  segflg;     // segment flag for multi-part pkg
    u8  reserved;
};

#define PACKET_SEG_FLAG(first, remainder) \
    ((first) ? (u8)BBOX_PKT_SEG_STRT : \
    (((remainder) <= PACKET_DATA_MAX_LEN) ? (u8)BBOX_PKT_SEG_FINL : (u8)BBOX_PKT_SEG_CONT))

s32 bbox_upward_send_packet(struct channel_info *channel, const struct bbox_msg_header *msg);
s32 bbox_downward_recv_packet(const struct channel_info *channel);
s32 bbox_recv_consult_packet(struct channel_info *channel);
s32 bbox_send_consult_packet(struct channel_info *channel);

#endif // RDR_PACKET_H
