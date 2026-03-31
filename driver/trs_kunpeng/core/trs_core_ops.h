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
* Create: 2023-8-15
*/
#ifndef TRS_CORE_OPS_H__
#define TRS_CORE_OPS_H__

#include <linux/types.h>

#include "trs_pub_def.h"
#include "trs_core.h"

#define TRS_SUPPORT_PROC_NUM 0

struct trs_core_adapt_ops *trs_core_get_adapt_ops(void);
int trs_core_ops_notice_ts(struct trs_id_inst *inst, u8 *msg, u32 len);

int trs_core_ops_get_ts_inst_status(struct trs_id_inst *inst, u32 *status);
int trs_sq_send_trigger_db_init(struct trs_id_inst *inst);
void trs_sq_send_trigger_db_uninit(struct trs_id_inst *inst);
int trs_core_ops_get_sq_trigger_irq(struct trs_id_inst *inst, u32 *irq);
int trs_core_ops_get_trigger_sqid(struct trs_id_inst *inst, u32 *sqid);
void trs_core_ops_set_trigger_irq_affinity(struct trs_id_inst *inst, u32 irq, u32 op);
#endif /* TRS_CORE_OPS_H__ */

