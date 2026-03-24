/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
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

#ifndef DEVDRV_MANAGER_FREQ_H
#define DEVDRV_MANAGER_FREQ_H

#include <linux/spinlock.h>

#define DDR_FREQ_ORIGIN 0
#define DDR_FREQ_INITED 1
#define DDR_FREQ_LAST_IS_UP 2
#define DDR_FREQ_LAST_IS_DOWN 3

struct devdrv_ddr_freq_manager {
    u64 ticket_pool;  // only support 64(BITS_PER_LONG) modules at most
    u8 last_ipc_cmd;  // record last freq switch cmd, up or down
    u8 inited;        // flag inited or not
};

enum devdrv_freq_module {
    DEVDRV_FREQ_TS = 0x0,
    DEVDRV_FREQ_DVPP,
    DEVDRV_FREQ_MAX,  // at most 64(BITS_PER_LONG)
};

int devdrv_upper_ddr_freq(u32 devid, u32 module_id);
int devdrv_lower_ddr_freq(u32 devid, u32 module_id);
void devdrv_freq_control_register(u32 dev_id);

#endif
