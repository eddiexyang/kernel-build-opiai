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

#ifndef TRS_STARS_COMM_H
#define TRS_STARS_COMM_H
#include "trs_pub_def.h"
/* TRS STARS res id ctrl cmd */
enum {
    TRS_STARS_RES_OP_RESET = 0,
    TRS_STARS_RES_OP_RECORD,
    TRS_STARS_RES_OP_ENABLE,
    TRS_STARS_RES_OP_DISABLE,
    TRS_STARS_RES_OP_CHECK_AND_RESET,
    TRS_STARS_RES_OP_MAX,
};
int trs_stars_soc_res_ctrl(struct trs_id_inst *inst, u32 type, u32 id, u32 cmd);
#endif
