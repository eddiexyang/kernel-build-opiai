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
* Create: 2022-08-18
*/

#ifndef __SET_SOC_RESMNG_H
#define __SET_SOC_RESMNG_H
#include "devdrv_platform_resource.h"

int set_soc_resmng_subsys_tscpu(u32 devid, struct devdrv_ts_pdata *ts_pdata);
int set_soc_resmng_subsys_stars(u32 devid, struct devdrv_platform_data *pdata);
int set_soc_resmng_subsys_ras(u32 devid, struct devdrv_platform_data *pdata);
int set_soc_resmng_bitmap_res(u32 devid, u32 tsid);
int set_id_pool(u32 devid, u32 tsid);

#endif /* __SET_SOC_RESMNG_H */
