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
* Create: 2022-12-15
*/

#include "trs_chip_def.h"

int trs_get_host_irq_group(struct trs_id_inst *inst, u32 group[], u32 group_num, u32 *valid_group_num)
{
    u32 i;

    *valid_group_num = (group_num > trs_get_cq_group_num()) ? trs_get_cq_group_num() : group_num;
    for (i = 0; i < *valid_group_num; i++) {
        group[i] = i;
    }

    return 0;
}
EXPORT_SYMBOL(trs_get_host_irq_group);
