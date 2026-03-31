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
#ifndef TRS_STARS_REG_DEF_V1_H
#define TRS_STARS_REG_DEF_V1_H

#include <linux/types.h>

#define TRS_STARS_SCHED_SQ_TAIL_OFFSET              0x8
#define TRS_STARS_SCHED_SQ_HEAD_OFFSET              0x10
#define TRS_STARS_SCHED_SQ_STATUS_OFFSET            0x14
#define TRS_STARS_SCHED_CQ_HEAD_OFFSET              0x888
#define TRS_STARS_SCHED_CQ_TAIL_OFFSET              0x890

#define TRS_STARS_CQINT_L1_MASK_OFFSET              0x2020
#define TRS_STARS_CQINT_L1_STATUS_OFFSET            0x2024

#define TRS_STARS_CQINT_MID_REG_NUM       2

#define TRS_STARS_CQINT_MID_STATUS_OFFSET(mid_index)        (0x2040 + (mid_index) * 4)

#ifdef CFG_MEMORY_OPTIMIZE
#define TRS_STARS_CQINT_MID_STATUS_WIDTH                    8
#endif

#define TRS_STARS_CQINT_L2_STATUS_OFFSET(l2_index)          (0x2180 + (l2_index) * 4)
#define TRS_STARS_CQINT_L2_STATUS_WIDTH                     32

#define TRS_STARS_CQINT_L2_CTRL_OFFSET(l2_index)            (0x2280 + (l2_index) * 4)

#define TRS_STARS_HOST_CQINT_L1_STATUS_OFFSET(group)    (((group) * 0x10000) + 0x100010)
#define TRS_STARS_HOST_CQINT_L1_CTRL_OFFSET(group)      (((group) * 0x10000) + 0x100014)

#define TRS_STARS_HOST_CQINT_MID_STATUS_OFFSET(group)   (((group) * 0x10000) + 0x100040)

#define TRS_STARS_HOST_CQINT_L2_STATUS_OFFSET(group, mid_bit)   (((group) * 0x10000) + ((mid_bit) * 0x4) + 0x100090)
#define TRS_STARS_HOST_CQINT_L2_CTRL_OFFSET(group, mid_bit)     (((group) * 0x10000) + ((mid_bit) * 0x4) + 0x1000A0)
#endif /* TRS_STARS_REG_DEF_V1_H */
