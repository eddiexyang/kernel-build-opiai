/*
* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
* Create: 2023-1-10
*/
#ifndef SOC_ADAPT_RES_MINI_V2_H__
#define SOC_ADAPT_RES_MINI_V2_H__
#include "trs_rsv_mem.h"
#include "trs_pm_adapt.h"

size_t trs_soc_get_mini_v2_notify_size(void);
u32 trs_soc_get_mini_v2_notify_offset(u32 notify_id);
size_t trs_soc_get_mini_v2_db_stride(void);
int trs_soc_get_mini_v2_db_cfg(int db_type, u32 *start, u32 *end);

static inline u32 trs_soc_get_mini_v2_sq_mem_side(u32 devid)
{
    return TRS_CHAN_DEV_MEM;
}

static inline u32 trs_soc_get_mini_v2_cq_mem_side(u32 devid)
{
    return TRS_CHAN_HOST_MEM;
}

static inline int trs_soc_get_mini_v2_hwcq_rsv_mem_type(void)
{
    return RSV_MEM_HW_SQCQ;
}

#endif /* SOC_ADAPT_RES_MINI_V2_H__ */
