/*
* Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
* Create: 2022-10-08
*/
#ifndef SOC_ADAPT_RES_CLOUD_V2_H__
#define SOC_ADAPT_RES_CLOUD_V2_H__

#include <linux/types.h>

#include "trs_pm_adapt.h"
#include "trs_pub_def.h"
#include "trs_rsv_mem.h"

size_t trs_soc_get_cloud_v2_notify_size(void);
u32 trs_soc_get_cloud_v2_notify_offset(u32 id);
size_t trs_soc_get_cloud_v2_db_stride(void);
int trs_soc_get_cloud_v2_db_cfg(int db_type, u32 *start, u32 *end);
size_t trs_soc_get_cloud_v2_stars_sched_stride(void);
u32 trs_soc_get_cloud_v2_sq_mem_side(u32 devid);
u32 trs_soc_get_cloud_v2_cq_mem_side(u32 devid);

static inline  int trs_soc_get_cloud_v2_hwcq_rsv_mem_type(void)
{
    return RSV_MEM_HW_SQCQ;
}

#endif /* SOC_ADAPT_RES_CLOUD_V2_H__ */
