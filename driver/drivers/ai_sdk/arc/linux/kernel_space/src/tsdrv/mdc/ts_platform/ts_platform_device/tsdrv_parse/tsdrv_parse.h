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

#ifndef TSDRV_PARSE_H
#define TSDRV_PARSE_H

#include <linux/types.h>
#include "devdrv_common.h"

#define TSDRV_MDC_SLOT_NUM 1

int tsdrv_get_aicpu_occupy_bitmap(u32 dev_id, u32 plat_type, u32 *bitmap);
int tsdrv_get_plat_info(struct devdrv_platform_data *pdata);
int devdrv_get_base_addr_info(struct platform_device *pdev, struct devdrv_platform_data *pdata);

void tsdrv_dfx_cq_irq_bind_core(struct devdrv_platform_data *pdata);
struct devdrv_platform_data *devdrv_parse_pdata(struct platform_device *pdev);
void devdrv_destroy_pdata(struct platform_device *pdev, struct devdrv_platform_data *pdata);
int devdrv_get_aicpu_freq_from_tee(u32 dev_id, u32 *aicpu_freq);

#ifdef CFG_SOC_PLATFORM_MDC_V51
unsigned int get_ts_stl_test_period_from_device_node(u32 devid, u32 tsid);
#endif

#endif /* __TSDRV_PARSE_H */
