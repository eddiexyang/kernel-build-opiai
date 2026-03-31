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

#ifndef TRS_ADAPT_ID_H__
#define TRS_ADAPT_ID_H__

#include <linux/types.h>

#include "trs_pub_def.h"
#include "trs_pm_adapt.h"

int trs_id_config(struct trs_id_inst *inst);
void trs_id_deconfig(struct trs_id_inst *inst);

#endif /* TRS_ADAPT_ID_H__ */
