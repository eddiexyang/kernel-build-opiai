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
* Create: 2022-7-15
*/
#ifndef SOC_ADAPT_RES_MINI_V3_H__
#define SOC_ADAPT_RES_MINI_V3_H__

#include <linux/types.h>

#include "trs_pub_def.h"
#include "trs_rsv_mem.h"
#include "trs_pm_adapt.h"

size_t trs_soc_get_mini_v3_notify_size(void);
u32 trs_soc_get_mini_v3_notify_offset(u32 notify_id);
size_t trs_soc_get_mini_v3_db_stride(void);
int trs_soc_get_mini_v3_db_cfg(int db_type, u32 *start, u32 *end);
size_t trs_soc_get_mini_v3_stars_sched_stride(void);

static inline u32 trs_soc_get_mini_v3_sq_mem_side(u32 devid)
{
    return TRS_CHAN_DEV_MEM;
}

static inline u32 trs_soc_get_mini_v3_cq_mem_side(u32 devid)
{
    return TRS_CHAN_DEV_MEM;
}

static inline int trs_soc_get_mini_v3_hwcq_rsv_mem_type(void)
{
    return RSV_MEM_HW_SQCQ;
}

#endif /* SOC_ADAPT_RES_MINI_V3_H__ */
