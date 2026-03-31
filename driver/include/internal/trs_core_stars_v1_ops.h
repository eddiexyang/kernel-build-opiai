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
* Create: 2022-7-15
*/
#ifndef TRS_CORE_STARS_V1_OPS_H
#define TRS_CORE_STARS_V1_OPS_H

#include <linux/types.h>

#include "trs_pub_def.h"
#include "trs_core.h"

#define TRS_SUPPORT_PROC_NUM 0U

struct trs_core_adapt_ops *trs_core_get_stars_v1_adapt_ops(void);
int trs_sq_send_trigger_db_init(struct trs_id_inst *inst);
void trs_sq_send_trigger_db_uninit(struct trs_id_inst *inst);

#endif /* TRS_CORE_STARS_V1_OPS_H */
