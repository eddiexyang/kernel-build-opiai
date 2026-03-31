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

#ifndef CDR_BOOTCHECK_H
#define CDR_BOOTCHECK_H

#include "device/bbox_types.h"
#include "common/bbox_common.h"
#include "communication/bbox_message.h"

#define CHIP_DATA_RECORD_MAGIC      0x63686970  // ascii for chip
#define CHIP_DATA_RECORD_VERSION    0x0100      // 0x0100表示V1.0
#define CDR_AREA_DATA_NUM           0x6

enum CDR_AREA_DATA_FLAG {
    CDR_FLAG_READ_CLEAN = 0,
    CDR_FLAG_WRITE_DATA = 1
};

enum CDR_AREA_DATA_TYPE {
    CDR_AREA_DATA_SRAM  = 0x0,
    CDR_AREA_DATA_DDR   = 0x1
};

struct cdr_area_head {
    u8 flag;               // AREA_DATA_FLAG
    u8 type;               // AREA_DATA_TYPE
    u8 reserve[6];         // reserve 6 bytes
    u32 offset;
    u32 length;
};

// total size: 128
struct cdr_head {          // cdr: chip data record
    u32 magic;             // CHIP_DFX_MAGIC
    u32 version;           // CHIP_DFX_VERSION
    u8 reserve[24];        // reserve 24 bytes
    struct cdr_area_head area[CDR_AREA_DATA_NUM];
};

s32 cdr_bootcheck_process(const struct bbox_time *start);
s32 bbox_bootcheck_send_cdr_data(u32 devid, const node_free_notify_ptr notify,
    const struct bbox_time *tm, const char *data, u32 len);

#endif