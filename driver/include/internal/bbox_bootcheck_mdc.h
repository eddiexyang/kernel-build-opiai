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

#ifndef BBOX_BOOTCHECK_MDC_H
#define BBOX_BOOTCHECK_MDC_H

#include "../bbox_bootcheck_pub.h"

#define LPM_PMU_DDR_BASE_ADDRESS   0x31205400ULL
#define LPM_PMU_DDR_BASE_LEN       0x400

s32 bbox_bootcheck_get_einfo(struct bbox_bootcheck_exception_table *table);
s32 bbox_bootcheck_send_sram_data(u32 devid, const struct bbox_time *tm);
s32 bbox_bootcheck_send_registers_data(u32 devid, const struct bbox_time *tm);
s32 bbox_bootcheck_send_tee_data(u32 devid, const struct bbox_time *time);
s32 bbox_bootcheck_send_atf_data(u32 devid, const struct bbox_time *time);
s32 bbox_bootcheck_send_sd_data(u32 devid, const struct bbox_time *tm);

#endif

