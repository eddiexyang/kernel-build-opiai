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

#ifndef HILINK_ADAP_LANE_CSR_REG_OFFSET_H
#define HILINK_ADAP_LANE_CSR_REG_OFFSET_H

/* ADAP_LANE_CSR Base address of Module's Register */
#define ADAP_LANE_CSR_BASE                       (0x50000)

/* ADAP_LANE_CSR Registers' Definitions */

#define ADAP_LANE_CSR_ADAP_CSR0_REG  (ADAP_LANE_CSR_BASE + 0x0)  /* Common Ctrl */
#define ADAP_LANE_CSR_ADAP_CSR1_REG  (ADAP_LANE_CSR_BASE + 0x4)  /* MBUS */
#define ADAP_LANE_CSR_ADAP_CSR2_REG  (ADAP_LANE_CSR_BASE + 0x8)  /* MBUS */
#define ADAP_LANE_CSR_ADAP_CSR3_REG  (ADAP_LANE_CSR_BASE + 0xC)  /* MBUS */
#define ADAP_LANE_CSR_ADAP_CSR4_REG  (ADAP_LANE_CSR_BASE + 0x10) /* MBUS */
#define ADAP_LANE_CSR_ADAP_CSR5_REG  (ADAP_LANE_CSR_BASE + 0x14) /* MBUS */
#define ADAP_LANE_CSR_ADAP_CSR6_REG  (ADAP_LANE_CSR_BASE + 0x18) /* MBUS */
#define ADAP_LANE_CSR_ADAP_CSR7_REG  (ADAP_LANE_CSR_BASE + 0x1C) /* TX SDPI */
#define ADAP_LANE_CSR_ADAP_CSR8_REG  (ADAP_LANE_CSR_BASE + 0x20)
#define ADAP_LANE_CSR_ADAP_CSR9_REG  (ADAP_LANE_CSR_BASE + 0x24)
#define ADAP_LANE_CSR_ADAP_CSR10_REG (ADAP_LANE_CSR_BASE + 0x28)
#define ADAP_LANE_CSR_ADAP_CSR11_REG (ADAP_LANE_CSR_BASE + 0x2C)
#define ADAP_LANE_CSR_ADAP_CSR12_REG (ADAP_LANE_CSR_BASE + 0x30)
#define ADAP_LANE_CSR_ADAP_CSR13_REG (ADAP_LANE_CSR_BASE + 0x34)
#define ADAP_LANE_CSR_ADAP_CSR14_REG (ADAP_LANE_CSR_BASE + 0x38)
#define ADAP_LANE_CSR_ADAP_CSR15_REG (ADAP_LANE_CSR_BASE + 0x3C)

#endif // __HILINK_ADAP_LANE_CSR_REG_OFFSET_H__
