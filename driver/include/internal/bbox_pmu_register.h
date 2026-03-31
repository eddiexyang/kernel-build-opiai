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

#ifndef BBOX_PMU_REGISTER_H
#define BBOX_PMU_REGISTER_H

#include "device/bbox_types.h"

#define PMU_REG_NAME                "pmu"
#define PMU_REG_NAME_LEN            strlen(PMU_REG_NAME)

#define PMU_REG_MAX_SIZE    16

#define PMU_RESET_REASON_OFFSET     0x24D
#define PMU_KERNEL_STAGE_OFFSET     (PMU_RESET_REASON_OFFSET + 1)

#define PMU_EACH_READ_ONE_NUM 1
#define PMU_EACH_READ_MAX_NUM 16

#define SMPI_SLAVEID_MAIN      0x0 /* 主PMU */
#define SMPI_SLAVEID_SUBA      0xB /* 副PMU */
#define SMPI_SLAVEID_SUBB      0x2 /* 副PMU */

#define MAIN_PMU_DUMP_REG_START        0x222     /* 主pmu异常时，dump异常事件寄存器：0x222~~0x230 */
#define MAIN_PMU_DUMP_REG_END          0x230
#define MAIN_PMU_DUMP_REG_NUM          ((MAIN_PMU_DUMP_REG_END - MAIN_PMU_DUMP_REG_START) + 1)
#define SUBA_PMU_DUMP_REG_START        0xE0      /* 副pmu异常时，dump异常事件寄存器：0xE0~~0xE3 */
#define SUBA_PMU_DUMP_REG_END          0xE3
#define SUBA_PMU_DUMP_REG_NUM          ((SUBA_PMU_DUMP_REG_END - SUBA_PMU_DUMP_REG_START) + 1)
#define SUBB_PMU_DUMP_REG_START        0xE0      /* 副pmu异常时，dump异常事件寄存器：0xE0~~0xE3 */
#define SUBB_PMU_DUMP_REG_END          0xE3
#define SUBB_PMU_DUMP_REG_NUM          ((SUBB_PMU_DUMP_REG_END - SUBB_PMU_DUMP_REG_START) + 1)

#define MAIN_PMU_HEX_22B 0x22B
#define MAIN_PMU_HEX_22B_MASK 0xF7
#define MAIN_PMU_HEX_22C 0x22C
#define MAIN_PMU_HEX_22C_MASK 0xF8
#define COMM_PMU_MASK 0xFF


struct bbox_pmu_info {
    u16 type;
    u16 offset;
    u32 size;
    u8 data[0];
};
#endif

