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

#ifndef HILINK_CP_CSR_REG_OFFSET_H
#define HILINK_CP_CSR_REG_OFFSET_H

/* HILINK_CP_CSR Base address of Module's Register */
#define CP_CSR_BASE                       (0x40000)

/* CP_CSR Registers' Definitions */
#define CP_CSR_CP_CSR0_REG  (CP_CSR_BASE + 0x0)
#define CP_CSR_CP_CSR1_REG  (CP_CSR_BASE + 0x4)  /* FSM */
#define CP_CSR_CP_CSR2_REG  (CP_CSR_BASE + 0x8)  /* FSM */
#define CP_CSR_CP_CSR3_REG  (CP_CSR_BASE + 0xC)  /* FSM */
#define CP_CSR_CP_CSR4_REG  (CP_CSR_BASE + 0x10) /* FSM */
#define CP_CSR_CP_CSR5_REG  (CP_CSR_BASE + 0x14) /* FSM */
#define CP_CSR_CP_CSR6_REG  (CP_CSR_BASE + 0x18) /* FSM */
#define CP_CSR_CP_CSR7_REG  (CP_CSR_BASE + 0x1C) /* FSM */
#define CP_CSR_CP_CSR8_REG  (CP_CSR_BASE + 0x20) /* FSM */
#define CP_CSR_CP_CSR9_REG  (CP_CSR_BASE + 0x24) /* FSM */
#define CP_CSR_CP_CSR10_REG (CP_CSR_BASE + 0x28) /* FSM */
#define CP_CSR_CP_CSR11_REG (CP_CSR_BASE + 0x2C) /* FSM */
#define CP_CSR_CP_CSR12_REG (CP_CSR_BASE + 0x30) /* FSM */
#define CP_CSR_CP_CSR13_REG (CP_CSR_BASE + 0x34) /* FSM */
#define CP_CSR_CP_CSR14_REG (CP_CSR_BASE + 0x38) /* TOP_FSM_CTRL */
#define CP_CSR_CP_CSR15_REG (CP_CSR_BASE + 0x3C) /* TOP_FSM_CTRL */
#define CP_CSR_CP_CSR16_REG (CP_CSR_BASE + 0x40)
#define CP_CSR_CP_CSR17_REG (CP_CSR_BASE + 0x44) /* INT */
#define CP_CSR_CP_CSR18_REG (CP_CSR_BASE + 0x48)
#define CP_CSR_CP_CSR19_REG (CP_CSR_BASE + 0x4C)
#define CP_CSR_CP_CSR20_REG (CP_CSR_BASE + 0x50)
#define CP_CSR_CP_CSR21_REG (CP_CSR_BASE + 0x54) /* CRG */
#define CP_CSR_CP_CSR22_REG (CP_CSR_BASE + 0x58) /* itcm_start_addr */
#define CP_CSR_CP_CSR23_REG (CP_CSR_BASE + 0x5C) /* itcm_end_addr */
#define CP_CSR_CP_CSR24_REG (CP_CSR_BASE + 0x60) /* dtcm_start_addr */
#define CP_CSR_CP_CSR25_REG (CP_CSR_BASE + 0x64) /* dtcm_end_addr */
#define CP_CSR_CP_CSR26_REG (CP_CSR_BASE + 0x68) /* rgn_start_addr */
#define CP_CSR_CP_CSR27_REG (CP_CSR_BASE + 0x6C) /* rgn_end_addr */
#define CP_CSR_CP_CSR28_REG (CP_CSR_BASE + 0x70) /* por_rst_pc */
#define CP_CSR_CP_CSR29_REG (CP_CSR_BASE + 0x74) /* timer_thr */
#define CP_CSR_CP_CSR30_REG (CP_CSR_BASE + 0x78) /* timer_cnt */
#define CP_CSR_CP_CSR31_REG (CP_CSR_BASE + 0x7C) /* stack_pointer */
#define CP_CSR_CP_CSR32_REG (CP_CSR_BASE + 0x80) /* core_pc */
#define CP_CSR_CP_CSR33_REG (CP_CSR_BASE + 0x84)
#define CP_CSR_CP_CSR34_REG (CP_CSR_BASE + 0x88) /* CS TOP CTRL */
#define CP_CSR_CP_CSR35_REG (CP_CSR_BASE + 0x8C) /* Macro spare */
#define CP_CSR_CP_CSR36_REG (CP_CSR_BASE + 0x90) /* Macro spare */
#define CP_CSR_CP_CSR37_REG (CP_CSR_BASE + 0x94) /* Macro spare */
#define CP_CSR_CP_CSR38_REG (CP_CSR_BASE + 0x98) /* INT source */
#define CP_CSR_CP_CSR39_REG (CP_CSR_BASE + 0x9C) /* INT mask */
#define CP_CSR_CP_CSR40_REG (CP_CSR_BASE + 0xA0) /* INT */

#endif // HILINK_CP_CSR_REG_OFFSET_H
