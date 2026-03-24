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

#include "bbox_packet.h"

#include <linux/securec.h>
#include <linux/delay.h>

#include "common/bbox_common.h"
#include "common/bbox_print.h"
#include "bbox_communication.h"
#include "bbox_message.h"

#define PACKET_MAX_LEN          bbox_channel_get_capacity()
#define PACKET_HEADER_LEN       ((u32)sizeof(struct bbox_packet_header))
#define PACKET_DATA_MAX_LEN     ((PACKET_MAX_LEN > PACKET_HEADER_LEN) ? \
                                (PACKET_MAX_LEN - PACKET_HEADER_LEN) : 0U)
#define PACKET_HEAD(buf)        ((struct bbox_packet_header *)(buf))
#define PACKET_DATA(buf)        (&((char *)(buf))[PACKET_HEADER_LEN])
#define PACKET_LENGTH(len)      ((len) + PACKET_HEADER_LEN)
#define PACKET_SET_SINGLE_HEAD(pkt, _type, _plen, _mlen, _devid, _flg, _seq) do { \
    (pkt)->type = (u16)(_type); \
    (pkt)->devid = (u16)(_devid); \
    (pkt)->p_len = (_plen); \
    (pkt)->m_len = (_mlen); \
    (pkt)->segflg = (_flg); \
    (pkt)->sequence = (_seq); \
} while (0)

#define PACKET_SET_MULT_HEAD_BASIC(pkt, _type, _mlen, _devid, _seq) do { \
    (pkt)->type = (u16)(_type); \
    (pkt)->devid = (u16)(_devid); \
    (pkt)->m_len = (_mlen); \
    (pkt)->sequence = (_seq); \
} while (0)

#define PACKET_SET_MULT_HEAD_EX(pkt, _plen, _flg) do { \
    (pkt)->p_len = (_plen); \
    (pkt)->segflg = (_flg); \
} while (0)


/*
 * @brief       : get msg partition
 * @param [in]  : u32 type          send msg type
 * @return      : msg partition
 *                BBOX_MSG_SYNC:  sync message
 *                BBOX_MSG_ASYNC: async message
 *                BBOX_MSG_UNKOWN: unkown message
 */
STATIC s32 bbox_packet_get_partition(u32 type)
{
    s32 partition = BBOX_MSG_UNKOWN;

    switch (type) {
        case BBOX_MSG_EXCEPTION:
        case BBOX_MSG_OOM:
        case BBOX_MSG_REBOOT:
            partition = BBOX_MSG_SYNC;
            break;
        case BBOX_MSG_HELLO:
            partition = BBOX_MSG_ASYNC;
            break;
        default:
            BB_PRINT_ERR("Unkown msg type: %u\n", type);
            partition = BBOX_MSG_UNKOWN;
            break;
    }

    return partition;
}

/*
 * @brief       : get message sequence, use to judge the resend message package
 * @param [in]  : const struct channel_info *channel    channel info
 * @return      : message sequence
 */
STATIC u16 bbox_packet_get_sequence(const struct channel_info *channel)
{
    BB_CHECK_PTR(channel, return 0, "invalid param, channel is NULL.\n");
    return (u16)(((channel->msg_seq) > MAX_MESSAGE_SEQ) ? 0U : channel->msg_seq);
}

/*
 * @brief       : increase message sequence, use to judge the resend message package
 * @param [in]  : const struct channel_info *channel    channel info
 * @return      : NA
 */
STATIC void bbox_packet_inc_sequence(struct channel_info *channel)
{
    BB_CHECK_PTR(channel, return, "invalid param, channel is NULL.\n");
    if (channel->msg_seq >= MAX_MESSAGE_SEQ) {
        channel->msg_seq = 0;
    } else {
        channel->msg_seq++;
    }
}

/*
 * @brief       : decrease message sequence, use to judge the resend message package
 * @param [in]  : const struct channel_info *channel    channel info
 * @return      : NA
 */
STATIC void bbox_packet_dec_sequence(struct channel_info *channel)
{
    BB_CHECK_PTR(channel, return, "invalid param, channel is NULL.\n");
    if (channel->msg_seq == 0) {
        channel->msg_seq = MAX_MESSAGE_SEQ;
    } else {
        channel->msg_seq--;
    }
}

/*
 * @brief       : check recv packet legitimacy
 * @param [in]  : struct channel_info *channel              channel info
 * @param [in]  : const struct bbox_packet_header *pkt      packet
 * @param [in]  : u32 len                                   packet length
 * @return      : !=0 failure; ==0 success
 */
STATIC s32 bbox_packet_check(const struct channel_info *channel, const struct bbox_packet_header *pkt, u32 len)
{
    BB_CHECK_PTR(channel, return BBOX_FAILURE, "invalid param, channel is NULL.\n");
    BB_CHECK_PTR(pkt, return BBOX_FAILURE, "invalid param, pkt is NULL.\n");
    BB_CHECK_EXP_ACT((len <= PACKET_HEADER_LEN) || (len > PACKET_MAX_LEN),
        return BBOX_FAILURE, "invalid param, len is %u.\n", len);

    if (pkt->devid != channel->r_devid) {
        BB_PRINT_ERR("invalid packet, devid: %hu\n", pkt->devid);
        return BBOX_FAILURE;
    }

    if (pkt->p_len != len) {
        BB_PRINT_ERR("Invalid packet, packet len: %u, except: %u\n", pkt->p_len, len);
        return BBOX_FAILURE;
    }

    if (pkt->type != channel->type) {
        BB_PRINT_ERR("invalid packet, type: %hu\n", pkt->type);
        return BBOX_FAILURE;
    }

    return BBOX_SUCCESS;
}

/*
 * @brief       : check recv reply packet legitimacy
 * @param [in]  : struct channel_info *channel              channel info
 * @param [in]  : const struct bbox_packet_header *pkt      packet
 * @param [in]  : u32 plen                                  packet length
 * @param [in]  : u16 sequence                              packet sequence
 * @return      : !=0 failure; ==0 success
 */
STATIC s32 bbox_packet_reply_check(const struct channel_info *channel,
                                   const struct bbox_packet_header *pkt, u32 plen, u16 sequence)
{
    s32 ret;
    u32 length = (u32)sizeof(struct bbox_reply_msg);
    u32 packet_len = PACKET_LENGTH(length);
    const struct bbox_reply_msg *reply = NULL;

    BB_CHECK_PTR(channel, return BBOX_FAILURE, "invalid param, channel is NULL.\n");
    BB_CHECK_PTR(pkt, return BBOX_FAILURE, "invalid param, pkt is NULL.\n");

    if (plen != packet_len) {
        BB_PRINT_ERR("Invalid reply packet, len: %u, except: %u.\n", plen, packet_len);
        return BBOX_MSG_INVAL;
    }

    ret = bbox_packet_check(channel, pkt, plen);
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("Receive reply dismatched.\n");
        return BBOX_MSG_INVAL;
    }

    if (pkt->sequence != sequence) {
        BB_PRINT_ERR("invalid reply packet, sequence: %hu, expect: %hu.\n", pkt->sequence, sequence);
        return BBOX_MSG_INVAL;
    }

    if (pkt->m_len != length) {
        BB_PRINT_ERR("Invalid reply packet, len: %u, expect: %u.\n", pkt->m_len, length);
        return BBOX_MSG_INVAL;
    }

    reply = (const struct bbox_reply_msg *)PACKET_DATA(pkt);
    if (reply->header.len != length) {
        BB_PRINT_ERR("Invalid reply message, len: %u, expect: %u.\n", reply->header.len, length);
        return BBOX_MSG_INVAL;
    }

    if (reply->header.type != (u32)BBOX_MSG_ACK) {
        BB_PRINT_ERR("Invalid reply message, type: %u.\n", reply->header.type);
        return BBOX_MSG_INVAL;
    }
    return BBOX_SUCCESS;
}

/*
 * @brief       : send msg, receive ack reply msg if msg is aync msg
 * @param [in]  : struct channel_info *channel                  channel info
 * @param [in]  : const struct bbox_msg_header *msg             message
 * @param [in]  : s32 type                                      message type: async or sync
 * @return      : !=0 failure; ==0 success
 *                BBOX_MSG_INVAL: message is nvalid
 *                BBOX_COMM_INVAL: communication is invalid
 *                BBOX_FAILURE: other failure
 *                BBOX_SUCCESS: success
 */
STATIC s32 bbox_send_single_packet(struct channel_info *channel, const struct bbox_msg_header *msg, s32 type)
{
    s32 ret;
    s32 nak_cnt = 0;

    // set packet
    u16 seq = bbox_packet_get_sequence(channel);
    u32 s_len = PACKET_LENGTH(msg->len);
    struct bbox_packet_header *pkt = PACKET_HEAD(channel->send.buf);
    PACKET_SET_SINGLE_HEAD(pkt, channel->type, s_len, msg->len, channel->r_devid, BBOX_PKT_SEG_NULL, seq);
    ret = memcpy_s(PACKET_DATA(channel->send.buf), PACKET_DATA_MAX_LEN, msg, msg->len);
    BB_CHECK_MEMCPY(ret, return BBOX_FAILURE);
    bbox_packet_inc_sequence(channel);

    while (1) {
        // send msg
        ret = bbox_channel_send(channel, channel->send.buf, s_len);
        if (ret != BBOX_SUCCESS) {
            if (ret != BBOX_COMM_INVAL) {
                BB_PRINT_ERR("channel[%u] send msg failed with %d.\n", channel->cid, ret);
            }
            bbox_packet_dec_sequence(channel);
            return ret;
        }

        if (type == BBOX_MSG_ASYNC) {
            return BBOX_SUCCESS;
        }

        // receive reply msg
        ret = bbox_channel_recv(channel, channel->recv.buf, PACKET_LENGTH(sizeof(struct bbox_reply_msg)));
        if (ret <= 0) {
            BB_PRINT_ERR("channel[%u] recv single packet failed with %d.\n", channel->cid, ret);
            bbox_packet_dec_sequence(channel);
            return ret;
        }

        // check reply
        ret = bbox_packet_reply_check(channel, PACKET_HEAD(channel->recv.buf), (u32)ret, seq);
        if (ret == BBOX_SUCCESS) {
            return BBOX_SUCCESS;
        } else {
            // send msg unusable, retrun nak, send again
            ++nak_cnt;
            if (nak_cnt < MAX_NAK_TIMES) {
                msleep(RESEND_GAP);
                continue;
            } else {
                BB_PRINT_ERR("recv nak %d times, link is invalid, reconnect!\n", nak_cnt);
                bbox_packet_dec_sequence(channel);
                return BBOX_COMM_INVAL;
            }
        }
    }
}

/*
 * @brief       : send msg by mult packets, receive ack reply msg if msg is aync msg
 * @param [in]  : struct channel_info *channel                  channel info
 * @param [in]  : const struct bbox_msg_header *msg             message
 * @param [in]  : s32 type                                      message type: async or sync
 * @return      : !=0 failure; ==0 success
 *                BBOX_MSG_INVAL: message is nvalid
 *                BBOX_COMM_INVAL: communication is invalid
 *                BBOX_FAILURE: other failure
 *                BBOX_SUCCESS: success
 */
STATIC s32 bbox_send_mult_packet(struct channel_info *channel, const struct bbox_msg_header *msg, s32 type)
{
    s32 nak_cnt = 0;

    // set packet
    u16 seq = bbox_packet_get_sequence(channel);
    u32 dlen = msg->len;
    const char *data = (const char *)msg;
    struct bbox_packet_header *pkt = PACKET_HEAD(channel->send.buf);
    PACKET_SET_MULT_HEAD_BASIC(pkt, channel->type, msg->len, channel->r_devid, seq);
    bbox_packet_inc_sequence(channel);

    do {
        // set pkt head
        u32 cpy_len = (dlen > PACKET_DATA_MAX_LEN) ? PACKET_DATA_MAX_LEN : dlen;
        s32 ret = memcpy_s(PACKET_DATA(channel->send.buf), PACKET_DATA_MAX_LEN, data, cpy_len);
        BB_CHECK_MEMCPY(ret, return BBOX_FAILURE);
        PACKET_SET_MULT_HEAD_EX(pkt, PACKET_LENGTH(cpy_len), PACKET_SEG_FLAG((data == (const char *)msg), dlen));

        // send msg
        ret = bbox_channel_send(channel, channel->send.buf, pkt->p_len);
        if (ret != BBOX_SUCCESS) {
#ifndef BBOX_UT
            if (ret != BBOX_COMM_INVAL) {
                BB_PRINT_ERR("Send mult packet failed. (channelid=%u; ret=%d)\n", channel->cid, ret);
            }
#endif
            bbox_packet_dec_sequence(channel);
            return ret;
        }

        if (type == BBOX_MSG_ASYNC) {
            dlen -= cpy_len;
            data = &data[cpy_len];
            continue;
        }

        // receive reply msg
        ret = bbox_channel_recv(channel, channel->recv.buf, PACKET_LENGTH(sizeof(struct bbox_reply_msg)));
        if (ret <= 0) {
            BB_PRINT_ERR("channel[%u] recv mult packet failed with %d.\n", channel->cid, ret);
            bbox_packet_dec_sequence(channel);
            return ret;
        }

        // check reply
        if (bbox_packet_reply_check(channel, PACKET_HEAD(channel->recv.buf), (u32)ret, seq) == BBOX_SUCCESS) {
            dlen -= cpy_len;
            data = &data[cpy_len];
            nak_cnt = 0;
            continue;
        }
        // send msg unusable, retrun nak, send again
        ++nak_cnt;
        if (nak_cnt >= MAX_NAK_TIMES) {
            BB_PRINT_ERR("recv nak %d times, link is invalid, reconnect!\n", nak_cnt);
            bbox_packet_dec_sequence(channel);
            return BBOX_COMM_INVAL;
        }
        msleep(RESEND_GAP);
    } while (dlen > 0);

    return BBOX_SUCCESS;
}

/*
 * @brief       : upward send msg, receive ack reply msg
 * @param [in]  : struct channel_info *channel            channel info
 * @param [in]  : const struct bbox_msg_header *msg       message
 * @return      : !=0 failure; ==0 success
 */
s32 bbox_upward_send_packet(struct channel_info *channel, const struct bbox_msg_header *msg)
{
    s32 type;

    BB_CHECK_PTR(channel, return BBOX_FAILURE, "invalid param, channel is NULL.\n");
    BB_CHECK_PTR(msg, return BBOX_FAILURE, "invalid param, msg is NULL.\n");
    BB_CHECK_EXP_ACT(msg->len == 0, return BBOX_FAILURE, "invalid param, msg length is %u.\n", msg->len);

    type = bbox_packet_get_partition(msg->type);
    BB_CHECK_EXP_ACT(type == BBOX_MSG_UNKOWN, return BBOX_FAILURE, "error msg type: %u\n", msg->type);

    if (msg->len <= (u32)PACKET_DATA_MAX_LEN) {
        // send single packet
        return bbox_send_single_packet(channel, msg, type);
    } else {
        // send mult packet
        return bbox_send_mult_packet(channel, msg, type);
    }
}

/*
 * @brief       : downward recv msg, send ack reply msg
 * @param [in]  : const struct channel_info *channel      channel info
 * @return      : !=0 failure; ==0 success
 */
s32 bbox_downward_recv_packet(const struct channel_info *channel)
{
    s32 ret;

    BB_CHECK_PTR(channel, return BBOX_FAILURE, "invalid param, channel is NULL.\n");
    // receive msg
    ret = bbox_channel_recv_try(channel, channel->recv.buf, channel->recv.len, WTRECV_TIMEOUT);
    if (ret <= 0) {
        return ret;
    }

    return BBOX_SUCCESS;
}

/*
 * @brief       : recv consult msg
 * @param [in]  : struct channel_info *channel      channel info
 * @return      : !=0 failure; ==0 success
 */
s32 bbox_recv_consult_packet(struct channel_info *channel)
{
    s32 ret;
    u32 msg_len = (u32)sizeof(struct bbox_hello_msg);
    u32 packet_len = PACKET_LENGTH(msg_len);
    struct bbox_hello_msg *msg = NULL;
    const struct bbox_packet_header *pkt = NULL;

    // recv hello msg
    ret = bbox_channel_recv_try(channel, channel->recv.buf, packet_len, WTRECV_TIMEOUT);
    if (ret <= 0) {
        return ret;
    }

    // check msg
    pkt = PACKET_HEAD(channel->recv.buf);
    if (pkt->p_len != (u32)ret) {
        BB_PRINT_ERR("Channel[%u] Bad hello packet, plen: %u, recv len: %d.\n", channel->cid, pkt->p_len, ret);
    }

    msg = (pkt->p_len >= packet_len) ? (struct bbox_hello_msg *)PACKET_DATA(pkt) : NULL;
    if ((pkt->p_len != packet_len) || (pkt->m_len != msg_len) || (msg == NULL)) {
        BB_PRINT_ERR("Channel[%u] Invalid hello packet,"
                     "packet(devid: %hu, plen: %u, mlen: %u, mtype: %u), "
                     "expect(devid: %u, plen: %u, mlen: %u).\n",
                     channel->cid, pkt->devid, pkt->p_len, pkt->m_len,
                     (msg != NULL) ? msg->header.type : (u32)BBOX_MSG_MAX,
                     channel->devid, packet_len, msg_len);
        return BBOX_MSG_INVAL;
    }

    msg = (struct bbox_hello_msg *)PACKET_DATA(channel->recv.buf);
    if ((msg->header.type != (u32)BBOX_MSG_HELLO) ||
        (msg->header.len != msg_len)) {
        return BBOX_MSG_INVAL;
    }

    if (msg->capacity <= sizeof(struct bbox_packet_header)) {
        return BBOX_MSG_INVAL;
    }
    bbox_channel_set_capacity(msg->capacity);

    BB_PRINT_INFO("channel[%u] change time stamp sequence from %u to %u.\n",
                  channel->cid, channel->send_seq, msg->sequence + 1U);
    // update timestamp sequence only on first consult
    if (channel->send_seq == 0) {
        channel->send_seq = BBOX_MIN(msg->sequence, MAX_TIMESTAMP_SEQ) + 1U;
        channel->send_seq %= MAX_TIMESTAMP_SEQ;
        // 0 reserved for host, device seq start from 1
        channel->send_seq = (channel->send_seq == 0) ? 1U : channel->send_seq;
    }
    channel->r_devid = pkt->devid;
    return BBOX_SUCCESS;
}

/*
 * @brief       : send consult msg
 * @param [in]  : struct channel_info *channel      channel info
 * @return      : !=0 failure; ==0 success
 */
s32 bbox_send_consult_packet(struct channel_info *channel)
{
    u16 seq;
    s32 ret;
    u32 msg_len = (u32)sizeof(struct bbox_hello_msg);
    u32 packet_len = PACKET_LENGTH(msg_len);
    struct bbox_hello_msg *msg = NULL;
    struct bbox_packet_header *pkt = PACKET_HEAD(channel->send.buf);

    seq = bbox_packet_get_sequence(channel);
    PACKET_SET_SINGLE_HEAD(pkt, channel->type, packet_len, msg_len,
                           channel->r_devid, BBOX_PKT_SEG_NULL, seq);
    msg = (struct bbox_hello_msg *)PACKET_DATA(channel->send.buf);
    msg->header.type = BBOX_MSG_HELLO;
    msg->header.len = msg_len;
    msg->capacity = PACKET_MAX_LEN;
    msg->flag = channel->type;
    // send hello msg
    ret = bbox_channel_send(channel, channel->send.buf, packet_len);
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("Channel[%u] sent hello message falied.\n", channel->cid);
        return ret;
    }

    bbox_packet_inc_sequence(channel);

    // recv hello ack msg
    ret = bbox_channel_recv(channel, channel->recv.buf, PACKET_LENGTH(sizeof(struct bbox_reply_msg)));
    if (ret <= 0) {
        BB_PRINT_ERR("Channel[%u] receive hello ack message falied.\n", channel->cid);
        return ret;
    }

    // check reply
    return bbox_packet_reply_check(channel, PACKET_HEAD(channel->recv.buf), (u32)ret, seq);
}

