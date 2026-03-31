/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2023. All rights reserved.
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
#include <linux/cpumask.h>

#include "devdrv_common.h"

int tsdrv_get_aicpu_occupy_bitmap(u32 dev_id, u32 plat_type, u32 *bitmap);
int tsdrv_get_plat_info(struct devdrv_platform_data *pdata);
int devdrv_get_base_addr_info(struct platform_device *pdev, struct devdrv_platform_data *pdata);

void tsdrv_dfx_cq_irq_bind_core(struct devdrv_platform_data *pdata);
struct devdrv_platform_data *devdrv_parse_pdata(struct platform_device *pdev);
void devdrv_destroy_pdata(struct platform_device *pdev, struct devdrv_platform_data *pdata);
int devdrv_get_aicpu_freq_from_tee(u32 dev_id, u32 *aicpu_freq);

static inline u32 tsdrv_get_cpu_node_num(void)
{
    return ((u32)cpu_to_node(num_online_cpus() - 1) + 1);
}

#endif /* __TSDRV_PARSE_H */
