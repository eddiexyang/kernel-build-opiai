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
#ifndef SOC_ADAPT_H__
#define SOC_ADAPT_H__

#include <linux/types.h>

#include "trs_pub_def.h"

int trs_soc_get_db_stride(struct trs_id_inst *inst, size_t *size);
int trs_soc_get_db_cfg(struct trs_id_inst *inst, int db_type, u32 *start, u32 *end);
void trs_soc_set_chip_type(u32 phy_devid, int chip_type);
int trs_soc_get_chip_type(u32 phy_devid);
int trs_soc_get_notify_offset(struct trs_id_inst *inst, u32 notify_id, u32 *offset);
int trs_soc_get_notify_size(struct trs_id_inst *inst, size_t *notify_size);
u32 trs_soc_get_sq_mem_side(struct trs_id_inst *inst);
u32 trs_soc_get_cq_mem_side(struct trs_id_inst *inst);
int trs_soc_get_stars_sched_stride(struct trs_id_inst *inst, size_t *stride);
int trs_soc_get_hw_type(u32 phy_devid);

int trs_soc_get_hwcq_rsv_mem_type(struct trs_id_inst *inst, int *rsv_mem_type);

#endif /* SOC_ADAPT_H__ */