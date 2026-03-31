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

#ifndef BBOX_CHANNEL_STATUS_H
#define BBOX_CHANNEL_STATUS_H

#include <linux/list.h>
#include "bbox_communication.h"

struct bbox_notifier_block {
    struct list_head s_list;
    s32 (*notifier_call)(struct bbox_notifier_block *self, unsigned long event, void *args);
};

struct bbox_callback_info {
    u32 type;                                      // channel type: up or down
    u32 devid;                                     // device id
    enum channel_status_type new_status;           // new channel usable flag
    enum channel_status_type old_status;           // previous channel usable flag
};

enum channel_status_type bbox_channel_get_status(const struct channel_info *channel);
void bbox_channel_set_status(struct channel_info *channel, enum channel_status_type status);
s32 bbox_channel_need_record_status(const struct bbox_callback_info *info);
s32 bbox_register_channel_notifier(struct bbox_notifier_block *nb);
s32 bbox_unregister_channel_notifier(const struct bbox_notifier_block *nb);

#endif
