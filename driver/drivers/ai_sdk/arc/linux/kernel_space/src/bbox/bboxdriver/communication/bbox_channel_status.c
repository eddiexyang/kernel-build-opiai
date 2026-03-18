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

#include "bbox_channel_status.h"
#include <linux/spinlock.h>

STATIC LIST_HEAD(g_channel_status_list);
STATIC DEFINE_MUTEX(g_channel_status_list_lock);

/*
 * @brief       : register module
 * @param [in]  : struct bbox_notifier_block *nb  notifier block with callback func
 * @return      : <0 failure; ==0 success
 */
s32 bbox_register_channel_notifier(struct bbox_notifier_block *nb)
{
    BB_CHECK_PTR(nb, return BBOX_FAILURE, "invalid parameter, nb is NULL.\n");
    mutex_lock(&g_channel_status_list_lock);
    list_add_tail(&(nb->s_list), &g_channel_status_list);
    mutex_unlock(&g_channel_status_list_lock);
    return BBOX_SUCCESS;
}

/*
 * @brief       : unregister module, new interface
 * @param [in]  : const struct bbox_notifier_block *nb  notifier block to be unregistered
 * @return      : <0 failure; ==0 success
 */
s32 bbox_unregister_channel_notifier(const struct bbox_notifier_block *nb)
{
    const struct bbox_notifier_block *node = NULL;
    struct list_head *cur = NULL;
    struct list_head *next = NULL;

    mutex_lock(&g_channel_status_list_lock);
    list_for_each_safe(cur, next, &g_channel_status_list) {
        node = (struct bbox_notifier_block *)list_entry(cur, struct bbox_notifier_block, s_list);
        if (node == nb) {
            list_del(cur);
            break;
        }
    }
    mutex_unlock(&g_channel_status_list_lock);
    return BBOX_SUCCESS;
}

/*
 * @brief       : call registered functions
 * @param [in]  : struct bbox_callback_info *info  contains callback info
 * @return      : NA
 */
STATIC void bbox_channel_notifier_callback(struct bbox_callback_info *info)
{
    struct bbox_notifier_block *node = NULL;
    struct list_head *cur = NULL;
    struct list_head *next = NULL;

    mutex_lock(&g_channel_status_list_lock);
    list_for_each_safe(cur, next, &g_channel_status_list) {
        node = (struct bbox_notifier_block *)list_entry(cur, struct bbox_notifier_block, s_list);
        if ((node == NULL) || (node->notifier_call == NULL)) {
            continue;
        }
        node->notifier_call(node, 0, info);
    }
    mutex_unlock(&g_channel_status_list_lock);
}

/*
 * @brief       : get channel status
 * @param [in]  : const struct channel_info *channel          channel info
 * @return      : channel status
 */
enum channel_status_type bbox_channel_get_status(const struct channel_info *channel)
{
    BB_CHECK_PTR(channel, return CHANNEL_STATUS_UNNORMAL, "invalid param, channel is NULL.\n");
    return channel->status;
}

/*
 * @brief       : set channel status
 * @param [in]  : struct channel_info *channel          channel info
 * @param [in]  : enum channel_status_type              status
 * @return      : NA
 */
void bbox_channel_set_status(struct channel_info *channel, enum channel_status_type status)
{
    struct bbox_callback_info info;
    BB_CHECK_PTR(channel, return, "invalid param, channel is NULL.\n");

    info.type = channel->type;
    info.devid = channel->devid;
    info.new_status = status;
    info.old_status = channel->status;
    channel->status = status;
    bbox_channel_notifier_callback(&info);
}

/*
 * @brief       : compare new status and current status to determine whether need to record
 * @param [in]  : struct bbox_callback_info *info    contains current status and channel info
 * @return      : NA
 */
s32 bbox_channel_need_record_status(const struct bbox_callback_info *info)
{
    BB_CHECK_PTR(info, return BBOX_FALSE, "invalid param, channel info is NULL.\n");

    // record states unnormal only for upward link
    if ((info->type == BBOX_CHANNEL_UPWARD) &&
        (info->new_status == CHANNEL_STATUS_UNNORMAL) &&
        (info->old_status == CHANNEL_STATUS_NORMAL)) {
        return BBOX_TRUE;
    }

    // record states become normal when all channels are connected.
    if ((info->new_status == CHANNEL_STATUS_NORMAL) &&
        (info->old_status == CHANNEL_STATUS_UNNORMAL) &&
        (bbox_channels_unnormal() == BBOX_FALSE)) {
        return BBOX_TRUE;
    }

    return BBOX_FALSE;
}

