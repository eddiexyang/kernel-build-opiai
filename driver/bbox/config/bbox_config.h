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

#ifndef BBOX_CONFIG_H
#define BBOX_CONFIG_H

#include "device/bbox_types.h"
#include "../common/bbox_common.h"

struct bbox_config_regs {
    u32 reg_num;
    const char *reg_name[REGS_DUMP_MAX_NUM];
    u64 addr[REGS_DUMP_MAX_NUM];
    u64 size[REGS_DUMP_MAX_NUM];
};

struct bbox_reserv_area {
    u32 area_num;
    const char *area_name[BBOX_AREA_MAXIMUM];
    u32 area_size[BBOX_AREA_MAXIMUM];
};

struct bbox_addr {
    u64 addr;
    u64 length;
};

struct bbox_config_data {
    u32                     spmi_channel;
    u32                     wait_timeout;
    struct bbox_addr        bbox_data;
    struct bbox_reserv_area reserved_area;
    struct bbox_addr        ddr_mntn;
    struct bbox_addr        pmu_reset_reg;
    struct bbox_config_regs ap_regs_info;
    struct bbox_addr        hdr;
    struct bbox_addr        cdr;
    struct bbox_addr        boot_param;
};

#define BBOX_FILE_NAME_MAX_LEN  32U
struct bbox_config_bootstatus_node {
    u32        min_valid_value;   // exclude min
    u32        max_valid_value;   // exclude max
    u32        file_id;           // file id, file name unique identification
};

#define BBOX_BOOT_STATUS_FILE_MAX_NUM  32U
struct bbox_config_bootstatus {
    u32        block_id;
    u32        stage_id;          // stage_id id, 0 for Secure Boot, 1 for rootfs cms
    u32        file_num;
    struct bbox_config_bootstatus_node file_list[BBOX_BOOT_STATUS_FILE_MAX_NUM];
};

struct bbox_config_rootfscms {
    u32        block_id;
    u32        stage_id;          // stage_id id, 0 for Secure Boot, 1 for rootfs cms
    u32        result;
    u32        min_valid_value;   // exclude min
    u32        max_valid_value;   // exclude max
};
struct bbox_config_rootfscms bbox_get_config_rtoofscms(void);
const struct bbox_config_bootstatus *bbox_get_config_bootstatus(u32 *num);

s32 bbox_config_init(void);
const struct bbox_config_data *bbox_get_config(void);
u32 bbox_config_get_spmi_channel(void);
s32 bbox_config_get_hdr(u64 *paddr, u64 *size);
s32 bbox_config_get_rdr(u64 *paddr, u64 *size);
s32 bbox_config_get_cdr(u64 *paddr, u64 *size);
s32 bbox_config_get_boot_param(u64 *paddr, u64 *size);
u32 bbox_config_get_dumplog_timeout(void);

#endif

