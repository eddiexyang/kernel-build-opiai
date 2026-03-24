/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
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
 * Create: 2021-11-20
 */
#ifndef SOC_MISC_MN_H
#define SOC_MISC_MN_H

#include "drvfault_user_common.h"
#include "dms_define.h"
#include "fpdc.h"

#define MN_SUBSYS_NUM   2
#define MN_SRC_NUM      2

/*
 * src id, from EMU_SUBS sheet, intterrupt number
 */
#define AO_MN_INT_SAFETY_BIT_ID 13
#define AO_MN_ERR_SAFETY_BIT_ID 29
#define IO_MN_INT_SAFETY_BIT_ID 15
#define IO_MN_ERR_SAFETY_BIT_ID 31

/*
 * physics base addr MN in subsys
 */
#define AO_MN_REG_BASE (0x81250000U)
#define IO_MN_REG_BASE (0xA0780000U)

#define MN_SAFETY_SRC          0x0824
#define MN_SAFETY_CLR          0x082C
#define MN_SAFETY_INT_MASK     0x0820

#define MN_SAFETY_ERR_SRC      0x0834
#define MN_SAFETY_ERR_CLR      0x083C
#define MN_SAFETY_ERR_INT_MASK 0x0830

extern struct safety_irq_info g_int_fhi_mn[MN_SUBSYS_NUM];

int soc_misc_mn_ops_init(struct dms_node *device);
void soc_misc_mn_ops_uninit(struct dms_node *device);
void soc_misc_mn_fault_handler(const struct notify_data *pdata);
int soc_misc_mn_sensor_scan(unsigned long long private_data, struct dms_sensor_event_data *data);

#endif
