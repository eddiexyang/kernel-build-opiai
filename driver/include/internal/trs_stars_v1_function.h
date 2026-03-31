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
 * Create: 2023-1-11
 */

#ifndef TRS_STARS_V1_FUNC_H
#define TRS_STARS_V1_FUNC_H
#include <linux/types.h>

#include "trs_pub_def.h"
#include "trs_stars_com.h"
struct trs_stars_ops *trs_stars_func_op_get(void);
int trs_stars_func_init(struct trs_id_inst *inst);
void trs_stars_func_uninit(struct trs_id_inst *inst);
int trs_stars_notifier_register(void);
void trs_stars_notifier_unregister(void);
#endif
