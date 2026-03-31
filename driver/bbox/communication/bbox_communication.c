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

#include "bbox_communication.h"

#include <linux/kthread.h>
#include <linux/semaphore.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/securec.h>
#include "device/bbox_pub.h"
#include "bbox_inner.h"
#include "bbox_packet.h"
#include "bbox_message.h"
#include "bbox_channel_status.h"
#include "bootparam/bbox_boot_param.h"
#include "register/bbox_register.h"

STATIC s32 g_comm_init_done = false;
STATIC struct communication_management g_bbox_communication;
#define g_channel_ops           g_bbox_communication.ops

#define CHANNEL_INIT(num)               \
    ((g_channel_ops.init != NULL) ? g_channel_ops.init(num) : BBOX_FAILURE)
#define CHANNEL_EXIT()                  \
    ((g_channel_ops.exit != NULL) ? g_channel_ops.exit() : BBOX_FAILURE)
#define CHANNEL_CAPACITY()              \
    ((g_channel_ops.capacity != NULL) ? g_channel_ops.capacity() : 0U)
#define SERVER_CREATE(devid, type) \
    ((g_channel_ops.create != NULL) ? g_channel_ops.create(devid, type) : BBOX_FAILURE)
#define SERVER_CLOSE(sid)               \
    ((g_channel_ops.close != NULL) ? g_channel_ops.close(sid) : BBOX_FAILURE)
#define CHANNEL_SEND(sid, data, len)    \
    ((g_channel_ops.send != NULL) ? g_channel_ops.send(sid, data, len) : BBOX_FAILURE)
#define CHANNEL_RECV(sid, buffer, len)  \
    ((g_channel_ops.recv != NULL) ? g_channel_ops.recv(sid, buffer, len) : BBOX_FAILURE)

static void bbox_free_node(struct bbox_msg_node *node)
{
    node->notify = NULL;
    if (node->arg != NULL) {
        bbox_vfree(node->arg);
        node->arg = NULL;
    }

    if (node->node_type == MSG_NODE_TYPE_STATIC) {
        return;
    }

    bbox_vfree(node->msg);
    bbox_vfree(node);
}

/*
 * @brief       : free msg node
 * @param [in]  : struct bbox_msg_node *node     msg node
 * @return      : NA
 */
STATIC void bbox_free_msg_node(struct bbox_msg_node *node)
{
    if (node == NULL) {
        return;
    }

    bbox_free_node(node);
}

/*
 * @brief       : free notify node
 * @param [in]  : struct bbox_msg_node *node     msg node
 * @return      : NA
 */
STATIC void bbox_free_notify_node(struct bbox_msg_node *node)
{
    if (node == NULL) {
        return;
    }

    if (node->notify != NULL) {
        node->notify(node->arg);
    }

    bbox_free_node(node);
}

/*
 * @brief       : free msg lst
 * @param [in]  : struct msg_list *lst      send msg list
 * @return      : NA
 */
STATIC void bbox_free_msg_lst(struct msg_list *lst)
{
    struct bbox_msg_node *node = NULL;
    struct list_head *head = NULL;
    struct list_head *pos = NULL;
    struct list_head *next = NULL;

    BB_CHECK_PTR(lst, return, "invalid param, msg list is NULL.\n");

    /* list is in module exit process, no need lock. */
    if (list_empty(&lst->list) != 0) {
        lst->num = 0;
        return;
    }

    head = &(lst->list);
    list_for_each_safe(pos, next, head) {
        node = (struct bbox_msg_node *)list_entry(pos, struct bbox_msg_node, list);
        list_del(pos);
        bbox_free_msg_node(node);   /* vfree can sleep, can't lock. */
        node = NULL;
    }
    lst->num = 0;
}

/*
 * @brief       : get channel handle by device id and channel type
 * @param [in]  : u32 devid     device id
 * @param [in]  : u32 type      channel type
 * @return      : channel info
 */
STATIC struct channel_info *bbox_get_channel(u32 devid, u32 type)
{
    s32 i;
    for (i = 0; i < (s32)g_bbox_communication.channel_num; i++) {
        if ((type == g_bbox_communication.channel[i].type) &&
            (devid == g_bbox_communication.channel[i].devid)) {
            return &(g_bbox_communication.channel[i]);
        }
    }
    return NULL;
}

/*
 * @brief       : get channels connected status
 * @param [in]  : u32 *connected
 * @return      : true: some channel is connected
 *                false: all channel is not connected
 */
bool bbox_channels_established(u32 *connected)
{
    s32 i;
    u32 num = 0;

    if ((connected == NULL) || (g_comm_init_done == false)) {
        return false;
    }
    for (i = 0; i < (s32)g_bbox_communication.channel_num; i++) {
        if (g_bbox_communication.channel == NULL) {
            break;
        }
        if (g_bbox_communication.channel[i].status == CHANNEL_STATUS_NORMAL) {
            num++;
        }
    }

    *connected = num;
    if (num != 0) {
        return true;
    }

    return false;
}

/*
 * @brief       : judge some channels status is unnoremal
 * @return      : BBOX_TRUE: some channel status is unnoremal
 *                BBOX_FALSE: all channel status is noremal
 */
s32 bbox_channels_unnormal(void)
{
    s32 i;

    for (i = 0; i < (s32)g_bbox_communication.channel_num; i++) {
        if (g_bbox_communication.channel[i].status == CHANNEL_STATUS_UNNORMAL) {
            return BBOX_TRUE;
        }
    }
    return BBOX_FALSE;
}

/*
 * @brief       : put the exception node into send list
 * @param [in]  : struct bbox_msg_node *node    send msg node
 * @return      : NA
 */
void bbox_submit_message(struct bbox_msg_node *node)
{
    struct channel_info *channel = NULL;

    BB_CHECK_PTR(node, return, "invalid param, submit node is NULL.\n");

    if (node->msg == NULL) {
        BB_PRINT_ERR("submit node msg is NULL\n");
        bbox_free_msg_node(node);
        return;
    }

    if (node->devid >= bbox_get_device_num()) {
        BB_PRINT_ERR("bad device id %u\n", node->devid);
        bbox_free_msg_node(node);
        return;
    }

    channel = bbox_get_channel(node->devid, BBOX_CHANNEL_UPWARD);
    if (channel == NULL) {
        BB_PRINT_ERR("this channel is invalid, id %u.\n", node->devid);
        bbox_free_msg_node(node);
        return;
    }

    // limit node number
    if ((channel->msg_lst.num > NODE_RUNTIME_MAX_NUM) ||
        ((bbox_channel_get_status(channel) == CHANNEL_STATUS_UNNORMAL) &&
        (channel->msg_lst.num >= NODE_MAX_NUM))) {
        BB_PRINT_INFO("directly free device-%u node because exceed nodes limit %u.\n",
                      node->devid, channel->msg_lst.num);
        bbox_free_msg_node(node);
        return;
    }

    spin_lock(&channel->msg_lst.lock);
    list_add_tail(&node->list, &channel->msg_lst.list);
    ++channel->msg_lst.num;
    spin_unlock(&channel->msg_lst.lock);
    up(&channel->msg_lst.sem);
}

/*
 * @brief       : upward channel send message
 * @param [in]  : struct channel_info *channel          channel info
 * @return      : !=0 failure; ==0 success
 */
STATIC s32 bbox_channel_upward_send_msg(struct channel_info *channel)
{
    s32 ret;
    struct bbox_msg_node *node = NULL;
    struct list_head *pos = NULL;
    struct list_head *next = NULL;
    struct list_head *head = NULL;
    u64 tv_nsec = 0;

    BB_CHECK_PTR(channel, return BBOX_FAILURE, "invalid param, channel is NULL.\n");
    spin_lock(&channel->msg_lst.lock);
    // if empty, return
    if (list_empty(&channel->msg_lst.list) != 0) {
        spin_unlock(&channel->msg_lst.lock);
        return BBOX_SUCCESS;
    }

    // get first node
    head = &(channel->msg_lst.list);
    list_for_each_safe(pos, next, head) {
        node = (struct bbox_msg_node *)list_entry(pos, struct bbox_msg_node, list);
        list_del(pos);
        --channel->msg_lst.num;
        break;
    }
    spin_unlock(&(channel->msg_lst.lock));

    if (node == NULL) {
        BB_PRINT_ERR("channel[%u] send node is NULL.\n", channel->cid);
        return BBOX_FAILURE;
    }

    // process node
    bbox_message_set_time_seq(channel, node->msg, &tv_nsec);
    ret = bbox_upward_send_packet(channel, node->msg);
    if (ret == BBOX_SUCCESS) {
        bbox_free_notify_node(node);
    } else if (ret == BBOX_COMM_INVAL) {
        // set the timestamp back if it needs to be resend
        bbox_message_set_time_seq(channel, node->msg, &tv_nsec);
        spin_lock(&(channel->msg_lst.lock));
        list_add(&node->list, &channel->msg_lst.list);
        ++channel->msg_lst.num;
        spin_unlock(&(channel->msg_lst.lock));
        up(&channel->msg_lst.sem);
        BB_PRINT_INFO("channel[%u] send node unsuccessful with %d, add back to list. left:%u\n",
            channel->cid, ret, channel->msg_lst.num);
    } else {
        bbox_free_msg_node(node);
        BB_PRINT_ERR("channel[%u] send node failed with %d, node data invalid. left:%u\n",
            channel->cid, ret, channel->msg_lst.num);
    }

    return ret;
}

/*
 * @brief       : upward channel process function
 * @param [in]  : struct channel_info *channel          channel info
 * @return      : !=0 failure; ==0 success
 */
STATIC s32 bbox_channel_upward_process(struct channel_info *channel)
{
    BB_CHECK_PTR(channel, return BBOX_FAILURE, "invalid param, channel is NULL.\n");
    while (!kthread_should_stop()) {
        s32 ret;
        bbox_jiffies_t sem_jiffies = msecs_to_jiffies(WTSEND_GAP);
        if (down_timeout(&channel->msg_lst.sem, (bbox_long_t)sem_jiffies) != 0) {
            continue;
        }

        ret = bbox_channel_upward_send_msg(channel);
        if (ret != BBOX_SUCCESS) {
            if (ret != BBOX_COMM_INVAL) {
                BB_PRINT_ERR("channel[%u] send message failed with %d.\n", channel->cid, ret);
            }
            return ret;
        }
    }
    return BBOX_FAILURE;
}

/*
 * @brief       : downward channel process function
 * @param [in]  : struct channel_info *channel          channel info
 * @return      : !=0 failure; ==0 success
 */
STATIC s32 bbox_channel_downward_process(const struct channel_info *channel)
{
    BB_CHECK_PTR(channel, return BBOX_FAILURE, "invalid param, channel is NULL.\n");
    while (!kthread_should_stop()) {
        s32 ret = bbox_downward_recv_packet(channel);
        if (ret != BBOX_SUCCESS) {
            if (ret == BBOX_COMM_TIMEOUT) {
                msleep(WTRECV_GAP);
                continue;
            } else if (ret == BBOX_COMM_INVAL) {
                return ret;
            } else {
                BB_PRINT_ERR("channel[%u] receive message failed with %d.\n", channel->cid, ret);
                return ret;
            }
        }
    }

    return BBOX_FAILURE;
}

/*
 * @brief       : channel consult up and down
 * @param [in]  : struct channel_info *channel          channel info
 * @return      : !=0 failure; ==0 success
 */
STATIC s32 bbox_channel_consult(struct channel_info *channel)
{
    s32 ret;

    BB_CHECK_PTR(channel, return BBOX_FAILURE, "invalid param, channel is NULL.\n");

    // recv hello msg
    ret = bbox_recv_consult_packet(channel);
    if (ret != BBOX_SUCCESS) {
        if (ret == BBOX_COMM_TIMEOUT) {
            BB_LOG_CTRL_INTV(BB_PRINT_INFO, channel->log_jiffies, LOG_INTERVAL, KEEP_TMSP,
                "channel[%u] no receive consult message.\n", channel->cid);
            ret = BBOX_COMM_INVAL;
        } else {
            BB_PRINT_ERR("channel[%u] receive consult message failed with %d.\n", channel->cid, ret);
        }
        return ret;
    }
    BB_PRINT_INFO("channel[%u] success receive hello message.\n", channel->cid);
    // send hello msg
    return bbox_send_consult_packet(channel);
}

/*
 * @brief       : service connect and channel buffer init
 * @param [in]  : struct channel_info *channel          channel info
 * @return      : !=0 failure; ==0 success
 */
STATIC s32 bbox_channel_connect(struct channel_info *channel)
{
    s32 ret;

    BB_CHECK_PTR(channel, return BBOX_FAILURE, "invalid param, channel is NULL.\n");
    ret = SERVER_CREATE(channel->devid, channel->type);
    if (ret < 0) {
        return BBOX_FAILURE;
    }
    channel->sid = (u32)ret;

    ret = bbox_channel_buffer_init(channel);
    if (ret != BBOX_SUCCESS) {
        ret = SERVER_CLOSE(channel->sid);
        BB_CHECK_RET(ret != BBOX_SUCCESS, "channel[%u] destroy failed.\n", channel->cid);
        return BBOX_FAILURE;
    }

    BB_LOG_CTRL_INTV(BB_PRINT_INFO, channel->log_jiffies, LOG_INTERVAL, KEEP_TMSP,
        "channel[%u] connect success, service id: %u.\n", channel->cid, channel->sid);
    return BBOX_SUCCESS;
}

static inline void bbox_channel_close_server(u32 cid, u32 sid)
{
    s32 ret = SERVER_CLOSE(sid);
    BB_CHECK_RET(ret != BBOX_SUCCESS, "channel[%u] service[%u] destroy failed.\n", cid, sid);
}

/*
 * @brief       : send thread
 * @param [in]  : void *arg     arg
 * @return      : !=0 failure; ==0 success
 */
STATIC s32 bbox_channel_thread_body(void *arg)
{
    s32 ret;
    enum channel_status_type status;
    u32 cid;
    struct channel_info *channel = NULL;

    BB_CHECK_EXP_ACT(arg == NULL, return BBOX_FAILURE, "invalid param, arg is NULL.\n");
    cid = *(u32 *)arg;
    BB_CHECK_EXP_ACT(cid >= (u32)g_bbox_communication.channel_num,
        return BBOX_FAILURE, "invalid param, cid is %u.\n", cid);

    BB_PRINT_INFO("enter channel[%u] thread.\n", cid);
    channel = &(g_bbox_communication.channel[cid]);
    while (!kthread_should_stop()) {
        // if channel status is not ok, reconnect
        status = bbox_channel_get_status(channel);
        if (status != CHANNEL_STATUS_NORMAL) {
            ret = bbox_channel_connect(channel);
            if (ret != BBOX_SUCCESS) {
                msleep(WTINIT_GAP);
                continue;
            }
            ret = bbox_channel_consult(channel);
            if (ret != BBOX_SUCCESS) {
                BB_LOG_CTRL_INTV(BB_PRINT_INFO, channel->log_jiffies, LOG_INTERVAL, UPDATE_TMSP,
                    "channel[%u] consult unsuccessful, reconnect.\n", cid);
                bbox_channel_close_server(cid, channel->sid);
                msleep(WTINIT_GAP);
                continue;
            }
            BB_PRINT_INFO("channel[%u] service[%u] consult success.\n", cid, channel->sid);
            bbox_channel_set_status(channel, CHANNEL_STATUS_NORMAL);
        }

        if (channel->type == BBOX_CHANNEL_UPWARD) {
            ret = bbox_channel_upward_process(channel);
        } else {
            ret = bbox_channel_downward_process(channel);
        }

        if (ret == BBOX_COMM_INVAL) {
            // if return communication failure, then reconnect
            BB_PRINT_INFO("channel[%u] communication unsuccessful, reconnect\n", cid);
            bbox_channel_set_status(channel, CHANNEL_STATUS_UNNORMAL);
            bbox_channel_close_server(cid, channel->sid);
        }
    }

    status = bbox_channel_get_status(channel);
    if (status == CHANNEL_STATUS_NORMAL) {
        bbox_channel_close_server(cid, channel->sid);
    }
    BB_PRINT_INFO("exit channel[%u] thread.\n", cid);
    return BBOX_SUCCESS;
}

/*
 * @brief       : channel send interface
 * @param [in]  : struct channel_info *channel      channel info
 * @param [in]  : const char *data                  send data
 * @param [in]  : int data_len                      send data length
 * @return      : !=0 failure; ==0 success
 */
s32 bbox_channel_send(const struct channel_info *channel, const char *data, u32 data_len)
{
    BB_CHECK_PTR(channel, return BBOX_FAILURE, "invalid param, channel is NULL.\n");
    BB_CHECK_PTR(data, return BBOX_FAILURE, "invalid param, data is NULL.\n");
    BB_CHECK_EXP_ACT(data_len == 0, return BBOX_FAILURE, "invalid param, data length is 0.\n");
    return CHANNEL_SEND(channel->sid, data, data_len);
}

/*
 * @brief       : channel recv msg with timeout
 * @param [in]  : const struct channel_info *channel        channel info
 * @param [in]  : char *buffer                              recv buffer
 * @param [in]  : u32 len                                   recv buffer length
 * @param [in]  : int timeout                               recv timeout
 * @return      : <=0 failure; >= recv data length
 */
s32 bbox_channel_recv_try(const struct channel_info *channel, char *buffer, u32 len, s32 timeout)
{
    s32 timeouts = BBOX_MAX(MIN_TIMEOUT, timeout);

    BB_CHECK_PTR(channel, return BBOX_FAILURE, "invalid param, channel is NULL.\n");
    BB_CHECK_PTR(buffer, return BBOX_FAILURE, "invalid param, buffer is NULL.\n");

    while (!kthread_should_stop()) {
        s32 ret = CHANNEL_RECV(channel->sid, buffer, len);
        if (ret != BBOX_COMM_TIMEOUT) {
            return ret;
        }

        msleep(WTRECV_TRY_GAP);
        timeouts -= WTRECV_TRY_GAP;
        if (timeouts < 0) {
            return BBOX_COMM_TIMEOUT;
        }
    }

    BB_PRINT_ERR("channel[%u] recv cancel.\n", channel->cid);
    return BBOX_FAILURE;
}

/*
 * @brief       : channel recv msg
 * @param [in]  : const struct channel_info *channel        channel info
 * @param [in]  : char *buffer                              recv buffer
 * @param [in]  : u32 len                                   recv buffer length
 * @return      : <=0 failure; >= recv data length
 */
s32 bbox_channel_recv(const struct channel_info *channel, char *buffer, u32 len)
{
    s32 ret = bbox_channel_recv_try(channel, buffer, len, WTRECV_TIMEOUT);
    if (ret == BBOX_COMM_TIMEOUT) {
        ret = BBOX_COMM_INVAL;
    }

    return ret;
}

/*
 * @brief       : get channel capacity
 * @return      : channel capacity
 */
u32 bbox_channel_get_capacity(void)
{
    return (u32)g_bbox_communication.capacity;
}

/*
 * @brief       : set channel capacity
 * @return      : NA
 */
void bbox_channel_set_capacity(size_t capacity)
{
    g_bbox_communication.capacity = BBOX_MIN(capacity, g_bbox_communication.capacity);
}

/*
 * @brief       : choice channel bottom interface
 * @param [in]  : struct channel_ops *ops       channel ops interface
 * @return      : NA
 */
STATIC void bbox_channel_register(struct channel_ops *ops)
{
    bbox_ioctl_register(ops);
}

/*
 * @brief       : free channel bottom interface
 * @param [in]  : struct channel_ops *ops       channel ops interface
 * @return      : NA
 */
STATIC void bbox_channel_unregister(struct channel_ops *ops)
{
    BB_CHECK_PTR(ops, return, "channel unregister ops is NULL.\n");

    ops->init = NULL;
    ops->exit = NULL;
    ops->send = NULL;
    ops->recv = NULL;
    ops->create = NULL;
    ops->close = NULL;
    ops->capacity = NULL;
}

/*
 * @brief       : channel info init
 * @param [in]  : u32 devid     device id
 * @param [in]  : u32 cid       channel id
 * @return      : !=0 failure; ==0 success
 */
STATIC s32 bbox_channel_init(u32 devid, u32 cid)
{
    struct channel_info *channel = NULL;

    BB_CHECK_EXP_ACT(cid >= (u32)g_bbox_communication.channel_num,
        return BBOX_FAILURE, "invalid param, cid is %u.\n", cid);

    channel = &(g_bbox_communication.channel[cid]);
    channel->cid = cid;
    channel->type = bbox_cid_is_upward(cid) ? BBOX_CHANNEL_UPWARD : BBOX_CHANNEL_DOWNWARD;
    channel->devid = devid;
    channel->r_devid = devid;
    channel->sid = INVALID_SESSION_ID;
    channel->status = CHANNEL_STATUS_INIT;
    channel->msg_seq = 0;
    channel->send_seq = 0;
    channel->msg_lst.num = 0;
    channel->log_jiffies = 0;
    INIT_LIST_HEAD(&channel->msg_lst.list);
    spin_lock_init(&channel->msg_lst.lock);
    sema_init(&channel->msg_lst.sem, 0);
    channel->send.len = 0;
    channel->send.buf = NULL;
    channel->recv.len = 0;
    channel->recv.buf = NULL;

    if (channel->task == NULL) {
        s32 ret;
        char name[THREAD_NAME_LEN] = {0};
        if (bbox_cid_is_upward(cid)) {
            ret = sprintf_s(name, THREAD_NAME_LEN, "bbox_up_%u", devid);
        } else {
            ret = sprintf_s(name, THREAD_NAME_LEN, "bbox_down_%u", devid);
        }
        BB_CHECK_SPRINTF(ret, return BBOX_FAILURE);

        // run process thread
        channel->task = bbox_kthread_proc(bbox_channel_thread_body, &channel->cid, (const char *)name);
        if (channel->task == NULL) {
            BB_PRINT_ERR("failed to create %s thread[%u].\n", name, cid);
            return BBOX_FAILURE;
        }
        BB_PRINT_INFO("succ to create %s thread[%u].\n", name, cid);
    }

    return BBOX_SUCCESS;
}

/*
 * @brief       : channel buffer info init
 * @param [in]  : struct channel_info *channel      channel info
 * @return      : !=0 failure; ==0 success
 */
s32 bbox_channel_buffer_init(struct channel_info *channel)
{
    u32 capacity;

    BB_CHECK_PTR(channel, return BBOX_FAILURE, "invalid param, channel is NULL.\n");

    if (channel->send.len != 0) {
        return BBOX_SUCCESS;
    }

    capacity = CHANNEL_CAPACITY();
    if ((capacity <= sizeof(struct bbox_packet_header)) || (capacity > BBOX_CHANNEL_CAPACITY)) {
        BB_PRINT_ERR("Channel capacity is invalid: %u.\n", capacity);
        return BBOX_FAILURE;
    }
    if (channel->send.buf == NULL) {
        channel->send.buf = (char *)bbox_vmalloc(capacity);
        if (channel->send.buf == NULL) {
            BB_PRINT_ERR("[%s][%4d] malloc failed.\n", __func__, __LINE__);
            return BBOX_FAILURE;
        }
    }

    if (channel->recv.buf == NULL) {
        channel->recv.buf = (char *)bbox_vmalloc(capacity);
        if (channel->recv.buf == NULL) {
            BB_PRINT_ERR("[%s][%4d] malloc failed.\n", __func__, __LINE__);
            BBOX_VFREE(channel->send.buf);
            return BBOX_FAILURE;
        }
    }

    channel->send.len = capacity;
    channel->recv.len = capacity;
    g_bbox_communication.capacity = capacity;
    return BBOX_SUCCESS;
}

/*
 * @brief       : channel exit, release channel info
 * @param [in]  : struct channel_info *channel      channel info
 * @return      : NA
 */
void bbox_channel_exit(struct channel_info *channel)
{
    BB_CHECK_PTR(channel, return, "invalid param, channel is NULL.\n");

    // exit thread
    if (channel->task != NULL) {
        (void)kthread_stop(channel->task);
        channel->task = NULL;
    }
    BBOX_VFREE(channel->recv.buf);
    BBOX_VFREE(channel->send.buf);
    if (channel->msg_lst.num != 0) {
        bbox_free_msg_lst(&channel->msg_lst);
    }
}

static inline u32 bbox_get_channel_num(u32 dev_num)
{
    return bbox_double(dev_num);
}

/*
 * @brief       : get cid by devid
 *                every device create 2 channel,
 *                cid is (devid * 2) and (device * 2 + 1),
 *                (devid * 2) is upward, (device * 2 + 1) is downward.
 * @param [in]  : u32 cid               channel id
 * @return      : devid
 */
static inline u32 bbox_cid_to_devid(u32 cid)
{
    return cid >> 1U;
}

/*
 * @brief       : communication init
 * @return      : !=0 failure; ==0 success
 */
s32 bbox_communication_init(void)
{
    u32 i;
    s32 ret;
    u32 device_num;
    u32 channel_num;

    if (g_comm_init_done == true) {
        return BBOX_SUCCESS;
    }
    BB_PRINT_INFO("communication init start.\n");
    // get channel interface
    bbox_channel_register(&g_bbox_communication.ops);

    // init channel interface
    device_num = bbox_get_device_num();
    channel_num = bbox_get_channel_num(device_num);
    ret = CHANNEL_INIT(channel_num);
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("channel init failed.");
        bbox_communication_exit();
        return BBOX_FAILURE;
    }

    BB_PRINT_INFO("need create %u channel.\n", channel_num);
    g_bbox_communication.capacity = 0;
    g_bbox_communication.channel_num = channel_num;
    g_bbox_communication.channel = (struct channel_info *)bbox_vmalloc(sizeof(struct channel_info) * channel_num);
    if (g_bbox_communication.channel == NULL) {
        BB_PRINT_ERR("[%s][%4d] malloc failed.\n", __func__, __LINE__);
        bbox_communication_exit();
        return BBOX_FAILURE;
    }

    for (i = 0; i < channel_num; i++) {
        ret = bbox_channel_init(bbox_cid_to_devid(i), i);
        if (ret != BBOX_SUCCESS) {
            bbox_communication_exit();
            return BBOX_FAILURE;
        }
    }

    g_comm_init_done = true;
    return BBOX_SUCCESS;
}

/*
 * @brief       : communication exit
 * @return      : NA
 */
void bbox_communication_exit(void)
{
    s32 ret;

    if (g_comm_init_done == false) {
        return;
    }
    BB_PRINT_INFO("communication exit.\n");
    if (g_bbox_communication.channel != NULL) {
        s32 i;
        for (i = 0; i < (s32)g_bbox_communication.channel_num; i++) {
            bbox_channel_exit(&g_bbox_communication.channel[i]);
        }
        BBOX_VFREE(g_bbox_communication.channel);
    }

    // exit channel
    ret = CHANNEL_EXIT();
    BB_CHECK_RET(ret != BBOX_SUCCESS, "channel exit failed.");
    bbox_channel_unregister(&g_bbox_communication.ops);
    g_comm_init_done = false;
}

