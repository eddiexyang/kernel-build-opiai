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
#ifndef BBOX_XPC_COMMON_H
#define BBOX_XPC_COMMON_H

#define BBOX_REPEAT_NUM 2000U
#define BBOX_WAITTIME_MIN 5000U  // us
#define BBOX_WAITTIME_MAX 5100U  // us

#define BBOX_POLL_TIMEOUT 2000U  // ms
#define BBOX_READ_SIZE    640U   // Bytes

struct bbox_area_s {
    u64 offset;     // area addr, unit is bytes(1 bytes)
    u32 length;     // area len, unit is bytes
    u8 coreid;      // module id
    u8 reserve[3];  // reserve 3 bytes
};

typedef void (*func)(char *data, u32 len);

s32 bbox_xpcshm_open_channel(s32 *chlid);
void bbox_xpchsm_close_channel(s32 chlid);
s32 bbox_xpc_poll_single(s32 chlid, struct chl_poll_ret *pollret);
void bbox_xpc_read(s32 chlid, u32 nexttype, func operfunc);
s32 bbox_xpc_read_msg(s32 chlid, struct chl_poll_ret *pollret, func operfunc);

#endif

