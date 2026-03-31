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

#ifndef BBOX_BOOTCHECK_PUB_H
#define BBOX_BOOTCHECK_PUB_H

#include "device/bbox_types.h"
#include "device/bbox_pub.h"
#include "common/bbox_common.h"
#include "communication/bbox_message.h"
#include "config/bbox_config.h"
#include "register/bbox_register.h"

struct bbox_bootcheck_einfo {
    u32 excepid;                        // exception id
    u8 etype;                           // the type of exception
    u8 coreid;                          // the core of happen exception
};

struct bbox_bootcheck_exception_table {
    u32 size;
    struct bbox_bootcheck_einfo *list;
};

struct bbox_bootcheck_proxy_info {
    u64 buffer;
    u32 buffer_size;
    u32 offset;
    u32 len;
    u32 excepid;
    struct bbox_time tm;
};

s32 bbox_bootcheck_get_einfo(struct bbox_bootcheck_exception_table *table);
u32 bbox_bootcheck_get_excepid(u8 etype);
u8 bbox_bootcheck_get_coreid(u32 etype);
s32 bbox_bootcheck_get_proxy_module(const struct rdr_exception_msg_info *info);

#endif

