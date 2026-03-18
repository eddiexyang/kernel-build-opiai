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

#ifndef BBOX_REGISTER_H
#define BBOX_REGISTER_H

#include "device/bbox_types.h"

enum reg_type_list {
    REG_TYPE_PMU = 1,
    REG_TYPE_SCTRL = 2,
    REG_TYPE_UNKNOWN
};

enum reg_type_list bbox_register_type(const char *name, u32 nlen);
void *bbox_register_ioremap(const char *name, u32 nlen, u64 addr, u32 size);
void bbox_register_iounmap(const char *name, u32 nlen, void *map_addr);
void bbox_register_dump(enum reg_type_list reg_type, u8 *dst, u32 dstsz, const u8 *src, u32 srcsz);

s32 bbox_get_pmu_info(char *buffer, u32 length);
void bbox_record_exce_type(u8 e_type);
void bbox_record_kernel_stage(u8 stage);
s32 bbox_register_init(void);
void bbox_register_exit(void);

#endif

