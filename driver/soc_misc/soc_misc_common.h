/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
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
* Create: 2019-10-15
*/

#ifndef __SOC_MISC_COMMON_H
#define __SOC_MISC_COMMON_H

#include "fpdc.h"
#include "soc_misc_spec.h"

#define REG_OP_TYPE_WR 0
#define REG_OP_TYPE_RD 1

#define NUM_TO_STR(num) #num

unsigned int soc_misc_dev_num(unsigned int dev_num);
int soc_misc_check_dev_id(unsigned int dev_id);
int soc_misc_reg_op(unsigned char op_type, unsigned long base_phy_reg, unsigned long reg_offset,
    unsigned long map_size, unsigned int *val);
#endif /* __SOC_MISC_COMMON_H */
