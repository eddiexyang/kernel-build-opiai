/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
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
 * Create: 2019-10-15
 */

#ifndef __AGENTDRV_CONFIG_SAFE_REG_H
#define __AGENTDRV_CONFIG_SAFE_REG_H

#include <linux/types.h>

#define DEVDRV_REG_READ 0x83000005
#define DEVDRV_REG_WRITE 0x83000006

struct tf_smc_param {
    u64 a0;
    u64 a1;
    u64 a2;
    u64 a3;
    u64 a4;
    u64 a5;
    u64 a6;
    u64 a7;
};

int smc_jump_to_atf(struct tf_smc_param *param);
u64 atf_write_reg_el3(u64 addr, u32 value);
u64 atf_read_reg_el3(u64 addr, u32 *value);

#endif
