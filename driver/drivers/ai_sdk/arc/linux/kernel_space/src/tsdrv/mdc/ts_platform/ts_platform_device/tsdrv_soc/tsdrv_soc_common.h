/* *
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
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

#ifndef TSDRV_SOC_COMMON_H
#define TSDRV_SOC_COMMON_H

#include <linux/types.h>
#include <linux/compiler.h>

#define DEVDRV_SC_TESTREG_TS 0
#define DEVDRV_SC_TESTREG_FIRST_AI_CPU 1
#define DEVDRV_SC_TESTREG_LAST_AI_CPU  4
#define DEVDRV_SC_TESTREG_CORE_INFO    11
#define DEVDRV_SC_TESTREG_MAX_NUM 16

#define DEVDRV_SC_TESTREG_INIT         0
#define DEVDRV_SC_TESTREG_TS_READY     0x06
#define DEVDRV_SC_TESTREG_AI_CPU_READY 0x5A
#define DEVDRV_SC_TESTREG_AI_CPU_BUSY  0x6B
#define DEVDRV_SC_TESTREG_AI_CPU_ERROR 0xFF

#define DEVDRV_DISP_TS_DAW_INDEX 7
#define DEVDRV_POLL_TS_TIME 5
#define DEVDRV_POLL_TS_RETRY_INTERVAL 100000
#define DEVDRV_POLL_TS_RETRY_INTERVAL_MAX 100100

static inline void set_tscpu_reg(void *rst_reg_base, u32 rst_val)
{
    (void)pr_info("rst_cpu, value: 0x%x\n", rst_val);
    writel(rst_val, rst_reg_base);
}

#endif
