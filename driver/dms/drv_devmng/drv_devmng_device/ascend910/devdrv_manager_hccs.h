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

#ifndef DEVDRV_MANAGER_HCCS_H
#define DEVDRV_MANAGER_HCCS_H

#include <linux/time.h>
#include "ascend_platform.h"

struct hccs_flow_info {
    unsigned long hccs_phy_addr;
    unsigned char __iomem *hccs_base_addr[HCCS_NUM];
    bool reg_map_flag;
    struct timespec64 current_time;
    unsigned long long send_amount;
    unsigned long long receive_amount;
};

#define MAX_DEVICE_COUNT 4
#define HCCS_OFFSET_INVALID 0xFFFFFFFF

unsigned int hccs_reg_read(unsigned long vir_addr);
void hccs_reg_write(unsigned long vir_addr, unsigned int val);

#define HCCS_REG_RD(base_addr, reg) hccs_reg_read(((unsigned long)(uintptr_t)base_addr) + (reg))
#define HCCS_REG_WR(base_addr, reg, val) \
    hccs_reg_write(((unsigned long)(uintptr_t)base_addr) + (reg), (unsigned int)val)

#ifndef CHECK_RET
#define CHECK_RET(a, ret) \
    {                      \
        if (!(a)) {        \
            CHECK_OUT(a)  \
            return (ret);  \
        }                  \
    }
#endif

#ifndef CHECK_OUT
#ifdef _DEBUG_
#define CHECK_OUT(a)                       \
    {                                       \
        devdrv_drv_err("Assert: %s\n", #a); \
    }

#else
#define CHECK_OUT(a)                       \
    {                                       \
        devdrv_drv_err("Assert: %s\n", #a); \
    }
#endif
#endif

#ifndef CHECK_DO_SOMETHING
#define CHECK_DO_SOMETHING(a, something) \
    {                                     \
        if (!(a)) {                       \
            something;                    \
        }                                 \
    }
#endif

#endif
