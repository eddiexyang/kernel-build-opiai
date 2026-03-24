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

#ifndef BBOX_BOOTCHECK_CLOUD_H
#define BBOX_BOOTCHECK_CLOUD_H

#include "../bbox_bootcheck_pub.h"

#define LPFW_PMU_DDR_BASE_ADDRESS   0x7805400
#define LPFW_PMU_DDR_BASE_LEN       256
#define LPFW_PMU_EVENT_MAX          7

// pmu register may be read for serveral times, the following length is read for 1 time
#define LPFW_PMU_REG_INFO_LEN_FOR_EACH 32
struct pum_event {
    u8 pmu_reg_info[LPFW_PMU_REG_INFO_LEN_FOR_EACH];
};

struct bbox_lpfw_pmu_info {
    u32 pmu_count;
    struct pum_event pmu_events[0];
};

s32 bbox_bootcheck_get_einfo(struct bbox_bootcheck_exception_table *table);
s32 bbox_bootcheck_send_sram_data(u32 devid, const struct bbox_time *time);
s32 bbox_bootcheck_send_registers_data(u32 devid, const struct bbox_time *time);
s32 bbox_bootcheck_send_tee_data(u32 devid, const struct bbox_time *time);
s32 bbox_bootcheck_send_atf_data(u32 devid, const struct bbox_time *time);
s32 bbox_bootcheck_send_sd_data(u32 devid, const struct bbox_time *time);

#endif

