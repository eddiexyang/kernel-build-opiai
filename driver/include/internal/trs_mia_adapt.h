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
* Create: 2022-10-15
*/

#ifndef TRS_MIA_ADAPT_H
#define TRS_MIA_ADAPT_H

int trs_mia_adapt_get_cq_group(struct trs_id_inst *inst, u32 group[], u32 group_num, u32 *valid_group_num);

int trs_mia_adapt_alloc_id(struct trs_id_inst *inst, int type, u32 *id);
void trs_mia_adapt_free_id(struct trs_id_inst *inst, int type, u32 id);

int trs_mia_adapt_get_id_range(struct trs_id_inst *inst, int type, u32 *start, u32 *end);
int trs_mia_adapt_get_id_total_num(struct trs_id_inst *inst, int type, u32 *total_num);
int trs_mia_adapt_get_id_split(struct trs_id_inst *inst, int type, u32 *split);
int trs_mia_core_ops_get_support_proc_num(struct trs_id_inst *inst, u32 *proc_num);

#endif

