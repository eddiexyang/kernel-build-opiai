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

#include "bbox_platform.h"
#include "common/bbox_common.h"

#define STAGE_KERNEL_SUCCESS 128

struct bbox_bootcheck_start_info {
    struct bbox_time start;
    u32 time_seq;
};

// data fetch mode, combined in bits
enum bbox_fetch_pattern {
    BBOX_FETCH_NULL         = 1 << 0,
    BBOX_FETCH_SD           = 1 << 1,
    BBOX_FETCH_FLASH_MULTI  = 1 << 2,
    BBOX_FETCH_DDR          = 1 << 3,
    BBOX_FETCH_REGISTERS    = 1 << 4,
    BBOX_FETCH_SRAM         = 1 << 5,
    BBOX_FETCH_ATF          = 1 << 6,
    BBOX_FETCH_TEE          = 1 << 7
};

// boot devmem相关
#define DDR_BOOT_DUMP_MAGIC     0x18273645
#define DDR_BOOT_DUMP_END_MAGIC 0x1A2B3C4D
#define DDR_BOOT_DUMP_VALID     0x1

enum DDR_UPLOAD_TYPE {
    BBOX_DUMP = 0x1,
    KERNEL_LOG,
    KERNEL_DUMP,
    TEE_DUMP,
    TS_DUMP,
    AICPU_DUMP,
    LPM_FLASH_DUMP,
    DUMP_TYPE_NUM = LPM_FLASH_DUMP,
    DUMP_TRY,
    DUMP_FINISH,
    DUMP_UNFINISHED,
    DUMP_TYPE_MAX
};

#define BBOX_DUMP_TYPE_NUM (u32)DUMP_TYPE_NUM

struct ddr_boot_dump {
    u32 valid;
    u32 len;
    u64 offset;
};

struct ddr_bios_hand_s {
    u32 magic;
    u32 reserve;
    struct ddr_boot_dump info[BBOX_DUMP_TYPE_NUM];
    u32 end_magic;
    char date[0];
};

s32 bbox_bootcheck_send_sram_data(u32 devid, const struct bbox_time *tm);
s32 bbox_bootcheck_send_registers_data(u32 devid, const struct bbox_time *tm);
s32 bbox_bootcheck_send_tee_data(u32 devid, const struct bbox_time *tm);
s32 bbox_bootcheck_send_atf_data(u32 devid, const struct bbox_time *tm);
s32 bbox_bootcheck_send_sd_data(u32 devid, const struct bbox_time *tm);
s32 bbox_bootcheck_init(void);
void bbox_bootcheck_exit(void);

#endif

