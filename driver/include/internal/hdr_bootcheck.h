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

#ifndef HDR_BOOTCHECK_H
#define HDR_BOOTCHECK_H

#include "common/bbox_common.h"
#include "device/bbox_pub.h"
#include "bbox_inner.h"


struct hdr_exception_info {
    u32 reset_num;
    u32 excepid;
    u8 coreid;
    u8 etype;
};

struct hdr_lastbootstatus {
    u32 stageid;     /* 0 for Secure Boot, 1 for rootfs cms */
    u32 blockid;
    u32 currentstatus;
    u32 result;
};

static inline u8 hdr_bootcheck_gen_coreid(u32 coreid, u8 default_coreid)
{
    return ((bbox_check_coreid_valid(coreid) == BBOX_SUCCESS) ? (u8)coreid : default_coreid);
}

static inline u32 hdr_bootcheck_gen_excepid(u8 coreid)
{
    const u8 default_sub = 0x1E;
    const u16 default_code = 0xFFF;
    return bbox_excepid_generate((u8)BBOX_EXCEPID_POSITION_DEVICE, BBOX_CRITICAL, coreid, default_sub, default_code);
}

static inline u32 hdr_bootcheck_gen_boot_excepid(u32 excepid, u8 coreid)
{
    return ((bbox_excepid_check(excepid) == true) ? excepid : hdr_bootcheck_gen_excepid(coreid));
}

s32 hdr_bootcheck_process(const struct bbox_time *start);

#endif

