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

#include "bbox_ioctl.h"
#include <linux/errno.h>
#include <linux/securec.h>
#include "device/bbox_types.h"
#include "common/bbox_print.h"
#include "common/bbox_common.h"
#include "bbox_ioctl_drv.h"
#include "bbox_inner.h"

struct ioctl_channel_manage {
    u32 segment;                                       // ioctl segment
    struct ioctl_channel_info *info;                   // channel manage info
    u8 use_channel_num;
    u8 total_channel_num;
    spinlock_t create_lock;
};

STATIC struct ioctl_channel_manage    g_bbox_ioctl_manage;
#define IOCTL_CHANNEL_USE_NUM         g_bbox_ioctl_manage.use_channel_num
#define IOCTL_CHANNEL_CAPACITY        g_bbox_ioctl_manage.segment

/**
 * @brief       : get ioctl channel by device id and channel type
 * @param [in]  : u32 devid             device id
 * @param [in]  : u32 type              channel type
 * @param [out] : u32 *sid              service id
 * @return      : channel info
 */
struct ioctl_channel_info *bbox_ioctl_get_channel(u32 devid, u32 type, u32 *sid)
{
    s32 i;
    BB_CHECK_PTR(sid, return NULL, "invalid param, sid is NULL.\n");

    for (i = 0; i < (s32)g_bbox_ioctl_manage.total_channel_num; i++) {
        if ((type == g_bbox_ioctl_manage.info[i].channel_type) &&
            (devid == g_bbox_ioctl_manage.info[i].devid)) {
            *sid = (u32)i;
            return &(g_bbox_ioctl_manage.info[i]);
        }
    }
    return NULL;
}

/**
 * @brief       : send msg. add msg to cdev sending list
 * @param [in]  : u32 sid                   service id
 * @param [in]  : const char *data          send data
 * @param [in]  : u32 data_len              send data length
 * @return      : !=0 failure; ==0 success
 */
STATIC s32 bbox_ioctl_channel_send(u32 sid, const char *data, u32 data_len)
{
    s32 ret;
    bbox_jiffies_t jif = msecs_to_jiffies(IOCTL_WTSEND_GAP);
    struct ioctl_channel_info *info = NULL;

    BB_CHECK_PTR(data, return BBOX_FAILURE, "invalid param, addr is NULL.\n");
    BB_CHECK_EXP_ACT(sid >= IOCTL_CHANNEL_USE_NUM, return BBOX_FAILURE, "invalid channel sid: %u.\n", sid);
    BB_CHECK_EXP_ACT(((data_len > IOCTL_CHANNEL_CAPACITY) || (data_len == 0)), return BBOX_FAILURE,
        "invalid param, len:%u, max segment:%u.\n", data_len, IOCTL_CHANNEL_CAPACITY);

    info = &g_bbox_ioctl_manage.info[sid];
    BB_CHECK_EXP_ACT(info->status != CONNECTED, return BBOX_COMM_INVAL,
        "ioctl serv[%u] send not connected.\n", sid);

    spin_lock(&info->send_node.lock);
    ret = memcpy_s(info->send_node.data, IOCTL_CHANNEL_CAPACITY, data, data_len);
    if (ret != EOK) {
        spin_unlock(&info->send_node.lock);
        BB_PRINT_ERR("copy send data in cdev failed.\n");
        return BBOX_FAILURE;
    }
    info->send_node.len = data_len;
    info->send_node.clear_flag = false;
    spin_unlock(&info->send_node.lock);

    // notify process epoll function
    bbox_dev_ioctl_wakeup_poll();
    if (down_timeout(&info->send_node.sem, (bbox_long_t)jif) != 0) {
        info->send_node.clear_flag = true;
        BB_PRINT_WARN("Ioctl serv[%u] send timeout: %d.\n", sid, IOCTL_WTSEND_GAP);
        return BBOX_COMM_INVAL;
    }
    // clear_flag will be set to true, if got the semphore
    return BBOX_SUCCESS;
}

/**
 * @brief       : receive msg
 * @param [in]  : u32 sid                   service id
 * @param [out] : char *buffer              recv buffer
 * @param [in]  : int len                   buffer length. > 0, guaranteed by caller
 * @return      : <=0 failure; >0 recv msg length
 */
STATIC s32 bbox_ioctl_channel_recv(u32 sid, char *buffer, u32 len)
{
    s32 ret;
    struct ioctl_channel_info *info = NULL;
    bbox_jiffies_t jif = msecs_to_jiffies(IOCTL_WTRECV_GAP);

    BB_CHECK_EXP_ACT(sid >= IOCTL_CHANNEL_USE_NUM, return BBOX_FAILURE, "invalid cdev-ioctl sid: %u.\n", sid);
    BB_CHECK_PTR(buffer, return BBOX_FAILURE, "invalid param, buffer is NULL.\n");

    info = &(g_bbox_ioctl_manage.info[sid]);
    mutex_lock(&info->lock);
    if (info->status != CONNECTED) {
        mutex_unlock(&info->lock);
        BB_PRINT_INFO("ioctl serv[%u] recv not connected.\n", sid);
        return BBOX_COMM_INVAL;
    }

    if (down_timeout(&info->recv_node.sem, (bbox_long_t)jif) != 0) {
        mutex_unlock(&info->lock);
        return BBOX_COMM_TIMEOUT;
    }

    if (len < info->recv_node.len) {
        info->recv_node.clear_flag = true;
        mutex_unlock(&info->lock);
        BB_PRINT_ERR("Ioctl serv[%u] recv bad length %u.\n", sid, info->recv_node.len);
        return BBOX_MSG_INVAL;
    }

    ret = memcpy_s(buffer, len, info->recv_node.data, info->recv_node.len);
    if (ret != EOK) {
        info->recv_node.clear_flag = true;
        mutex_unlock(&info->lock);
        BB_PRINT_ERR("[%s][%4d] memcpy_s failed.\n", __func__, __LINE__);
        return BBOX_FAILURE;
    }

    info->recv_node.clear_flag = true; // clear data flag
    mutex_unlock(&info->lock);
    return (s32)info->recv_node.len;
}

/**
 * @brief       : create ioctl channel
 * @param [in]  : u32 devid                 device id
 * @param [in]  : u32 type                  channel type
 * @return      : !=0 failure; >=0 service id
 */
STATIC s32 bbox_ioctl_channel_create(u32 devid, u32 type)
{
    u32 id = INVALID_SESSION_ID;
    struct ioctl_channel_info *info = NULL;

    BB_CHECK_EXP_ACT(devid >= IOCTL_DEVICE_MAX_NUM, return BBOX_FAILURE, "invalid channel devid: %u.\n", devid);

    spin_lock(&g_bbox_ioctl_manage.create_lock);
    info = bbox_ioctl_get_channel(devid, type, &id);
    if (info == NULL) {
        id = g_bbox_ioctl_manage.use_channel_num;
        if (id >= g_bbox_ioctl_manage.total_channel_num) {
            spin_unlock(&g_bbox_ioctl_manage.create_lock);
            BB_PRINT_ERR("Cdev-ioctl sid invalid, %u.\n", id);
            return BBOX_FAILURE;
        }
        info = &g_bbox_ioctl_manage.info[id];
        info->devid = devid;
        info->channel_type = type;
        g_bbox_ioctl_manage.use_channel_num++;
        info->magic = OPEN_CHAR_DEV_MAGIC;
        info->status = FIRST_WAIT_CONN;
    }
    spin_unlock(&g_bbox_ioctl_manage.create_lock);

    mutex_lock(&info->lock);
    if (info->status == RECONNECTED) {
        info->status = CONNECTED;
    } else if (info->status != CONNECTED) {
        mutex_unlock(&info->lock);
        return BBOX_FAILURE;
    }
    mutex_unlock(&info->lock);

    return (s32)id;
}

/**
 * @brief       : close ioctl channel
 * @param [in]  : u32 sid         service id
 * @return      : !=0 failure; ==0 success
 */
STATIC s32 bbox_ioctl_channel_close(u32 sid)
{
    struct ioctl_channel_info *info = NULL;

    BB_CHECK_EXP_ACT(sid >= IOCTL_CHANNEL_USE_NUM, return BBOX_FAILURE, "invalid ioctl sid: %u.\n", sid);

    info = &g_bbox_ioctl_manage.info[sid];
    mutex_lock(&info->lock);
    // RECONNECTED means host already refreshed the info, do not clear.
    if (info->status != RECONNECTED) {
        if (info->dev != NULL) {
            info->dev->accepted_filep = NULL;
            info->dev = NULL;
        }
        info->magic = OPEN_CHAR_DEV_MAGIC;
        info->status = WAIT_CONNECTION;
    }
    mutex_unlock(&info->lock);
    // notify host to get new epoll status
    bbox_dev_ioctl_wakeup_poll();
    return BBOX_SUCCESS;
}

/**
 * @brief       : get segment
 * @return      : segment
 */
STATIC u32 bbox_ioctl_capacity(void)
{
    return g_bbox_ioctl_manage.segment;
}

/**
 * @brief       : get channel connection status
 * @param [in]  : sid    channel service id
 * @return      : 1: connected, 0: not connected
 */
STATIC s32 bbox_ioctl_is_connected(u32 sid)
{
    BB_CHECK_EXP_ACT(sid >= IOCTL_CHANNEL_USE_NUM, return BBOX_FALSE, "invalid ioctl sid: %u.\n", sid);

    return g_bbox_ioctl_manage.info[sid].status == CONNECTED;
}

/**
 * @brief       : init ioctl channel info
 * @param [in]  : u32 num               channel num
 * @return      : !=0 failure; ==0 success
 */
STATIC s32 bbox_ioctl_channel_init(u32 num)
{
    s32 i;
    g_bbox_ioctl_manage.info = (struct ioctl_channel_info *)bbox_vmalloc(sizeof(struct ioctl_channel_info) * num);
    BB_CHECK_PTR(g_bbox_ioctl_manage.info, return BBOX_FAILURE, "malloc channel info failed.\n");

    for (i = 0; i < (s32)num; i++) {
        struct ioctl_channel_info *info = &g_bbox_ioctl_manage.info[i];
        info->magic = 0;
        info->devid = BBOX_INVALID_DEVID;
        info->channel_type = 0;
        info->recv_node.len = 0;
        info->send_node.len = 0;
        info->recv_node.clear_flag = true;
        info->send_node.clear_flag = true;
        info->recv_node.data = (char *)bbox_vmalloc(IOCTL_CHANNEL_CAPACITY);
        BB_CHECK_PTR(info->recv_node.data, return BBOX_FAILURE, "malloc recv_node data failed.\n");

        info->send_node.data = (char *)bbox_vmalloc(IOCTL_CHANNEL_CAPACITY);
        BB_CHECK_PTR(info->send_node.data, return BBOX_FAILURE, "malloc send_node data failed.\n");

        sema_init(&info->recv_node.sem, 0);
        sema_init(&info->send_node.sem, 0);
        spin_lock_init(&info->recv_node.lock);
        spin_lock_init(&info->send_node.lock);
        mutex_init(&info->lock);
        info->status = INITIALISED;
    }
    return BBOX_SUCCESS;
}

/**
 * @brief       : exit ioctl channel info
 * @return      : NA
 */
STATIC void bbox_ioctl_channel_exit(void)
{
    s32 i;

    if (g_bbox_ioctl_manage.info == NULL) {
        return;
    }

    for (i = 0; i < (s32)g_bbox_ioctl_manage.total_channel_num; i++) {
        g_bbox_ioctl_manage.info[i].status = UNINITIALISED;

        g_bbox_ioctl_manage.info[i].recv_node.clear_flag = true;
        sema_init(&g_bbox_ioctl_manage.info[i].recv_node.sem, 0);
        BBOX_VFREE(g_bbox_ioctl_manage.info[i].recv_node.data);

        g_bbox_ioctl_manage.info[i].send_node.clear_flag = true;
        sema_init(&g_bbox_ioctl_manage.info[i].send_node.sem, 0);
        BBOX_VFREE(g_bbox_ioctl_manage.info[i].send_node.data);
    }

    BBOX_VFREE(g_bbox_ioctl_manage.info);
}

/**
 * @brief       : init ioctl channel
 * @param [in]  : u32 num               channel num
 * @return      : !=0 failure; ==0 success
 */
STATIC s32 bbox_ioctl_init(u32 num)
{
    s32 ret;

    BB_PRINT_INFO("ioctl channel init, num: %u.\n", num);
    if ((!bbox_multiples_of_two(num)) || (num > IOCTL_CHANNEL_MAX_NUM)) {
        BB_PRINT_ERR("invlid channel num: %u.\n", num);
        return BBOX_FAILURE;
    }

    g_bbox_ioctl_manage.segment = IOCTL_SEGMENT;
    spin_lock_init(&g_bbox_ioctl_manage.create_lock);
    g_bbox_ioctl_manage.use_channel_num = 0;
    g_bbox_ioctl_manage.total_channel_num = (u8)num;

    ret = bbox_ioctl_channel_init(num);
    if (ret != BBOX_SUCCESS) {
        bbox_ioctl_channel_exit();
        return BBOX_FAILURE;
    }

    ret = bbox_dev_ioctl_init(num);
    if (ret != BBOX_SUCCESS) {
        bbox_ioctl_channel_exit();
        return BBOX_FAILURE;
    }

    return BBOX_SUCCESS;
}

/**
 * @brief       : exit ioctl channel
 * @return      : !=0 failure; ==0 success
 */
STATIC s32 bbox_ioctl_exit(void)
{
    s32 ret;
    size_t size = sizeof(g_bbox_ioctl_manage);

    BB_PRINT_INFO("ioctl channel exit.\n");

    bbox_dev_ioctl_exit(g_bbox_ioctl_manage.total_channel_num);
    bbox_ioctl_channel_exit();

    ret = memset_s(&g_bbox_ioctl_manage, size, 0, size);
    BB_CHECK_SECUREC(BB_CHECK_MEMSET, ret);
    return BBOX_SUCCESS;
}

/**
 * @brief       : ioctl interface register
 * @param [out] : struct channel_ops *ops    struct to store func ptr
 * @return      : NA
 */
void bbox_ioctl_register(struct channel_ops *ops)
{
    BB_CHECK_PTR(ops, return, "channel register ioctl ops is NULL.\n");
    ops->init = bbox_ioctl_init;
    ops->exit = bbox_ioctl_exit;
    ops->send = bbox_ioctl_channel_send;
    ops->recv = bbox_ioctl_channel_recv;
    ops->create = bbox_ioctl_channel_create;
    ops->close = bbox_ioctl_channel_close;
    ops->capacity = bbox_ioctl_capacity;
    ops->is_connected = bbox_ioctl_is_connected;
    BB_PRINT_INFO("success register ioctl interface.\n");
}
