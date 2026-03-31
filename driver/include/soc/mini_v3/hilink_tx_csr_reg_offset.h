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

#ifndef HILINK_TX_CSR_REG_OFFSET_H
#define HILINK_TX_CSR_REG_OFFSET_H

/* HILINK_TX_CSR Base address of Module's Register */
#define TX_CSR_BASE                       (0x48000)

/* HILINK_TX_CSR Registers' Definitions */
#define TX_CSR_TX_CSR0_REG  (TX_CSR_BASE + 0x0)  /* COMMON */
#define TX_CSR_TX_CSR1_REG  (TX_CSR_BASE + 0x4)  /* FSM */
#define TX_CSR_TX_CSR2_REG  (TX_CSR_BASE + 0x8)  /* FSM */
#define TX_CSR_TX_CSR3_REG  (TX_CSR_BASE + 0xC)  /* FSM */
#define TX_CSR_TX_CSR4_REG  (TX_CSR_BASE + 0x10) /* FSM */
#define TX_CSR_TX_CSR5_REG  (TX_CSR_BASE + 0x14) /* FSM */
#define TX_CSR_TX_CSR6_REG  (TX_CSR_BASE + 0x18) /* FSM */
#define TX_CSR_TX_CSR7_REG  (TX_CSR_BASE + 0x1C) /* FSM */
#define TX_CSR_TX_CSR8_REG  (TX_CSR_BASE + 0x20) /* FSM */
#define TX_CSR_TX_CSR9_REG  (TX_CSR_BASE + 0x24) /* FSM */
#define TX_CSR_TX_CSR10_REG (TX_CSR_BASE + 0x28) /* FSM */
#define TX_CSR_TX_CSR11_REG (TX_CSR_BASE + 0x2C) /* FSM */
#define TX_CSR_TX_CSR12_REG (TX_CSR_BASE + 0x30) /* FSM */
#define TX_CSR_TX_CSR13_REG (TX_CSR_BASE + 0x34) /* FSM */
#define TX_CSR_TX_CSR14_REG (TX_CSR_BASE + 0x38) /* RESERVED */
#define TX_CSR_TX_CSR15_REG (TX_CSR_BASE + 0x3C) /* CMD CTRL */
#define TX_CSR_TX_CSR16_REG (TX_CSR_BASE + 0x40) /* CMD CTRL */
#define TX_CSR_TX_CSR17_REG (TX_CSR_BASE + 0x44) /* CMD CTRL */
#define TX_CSR_TX_CSR18_REG (TX_CSR_BASE + 0x48) /* RESERVED */
#define TX_CSR_TX_CSR19_REG (TX_CSR_BASE + 0x4C) /* CMD CTRL */
#define TX_CSR_TX_CSR20_REG (TX_CSR_BASE + 0x50) /* CMD CTRL */
#define TX_CSR_TX_CSR21_REG (TX_CSR_BASE + 0x54) /* CMD CTRL */
#define TX_CSR_TX_CSR22_REG (TX_CSR_BASE + 0x58) /* CRG */
#define TX_CSR_TX_CSR23_REG (TX_CSR_BASE + 0x5C) /* CRG */
#define TX_CSR_TX_CSR24_REG (TX_CSR_BASE + 0x60) /* LFDATA_IDLE */
#define TX_CSR_TX_CSR25_REG (TX_CSR_BASE + 0x64) /* LFDATA_IDLE */
#define TX_CSR_TX_CSR26_REG (TX_CSR_BASE + 0x68) /* TXDECTRX */
#define TX_CSR_TX_CSR27_REG (TX_CSR_BASE + 0x6C) /* TXDECTRX */
#define TX_CSR_TX_CSR28_REG (TX_CSR_BASE + 0x70) /* FFE */
#define TX_CSR_TX_CSR29_REG (TX_CSR_BASE + 0x74) /* FFE */
#define TX_CSR_TX_CSR30_REG (TX_CSR_BASE + 0x78) /* FFE */
#define TX_CSR_TX_CSR31_REG (TX_CSR_BASE + 0x7C) /* GRBX_PA */
#define TX_CSR_TX_CSR32_REG (TX_CSR_BASE + 0x80) /* PRBS */
#define TX_CSR_TX_CSR33_REG (TX_CSR_BASE + 0x84) /* PRBS */
#define TX_CSR_TX_CSR34_REG (TX_CSR_BASE + 0x88) /* PRBS */
#define TX_CSR_TX_CSR35_REG (TX_CSR_BASE + 0x8C) /* PRBS */
#define TX_CSR_TX_CSR36_REG (TX_CSR_BASE + 0x90) /* PRBS */
#define TX_CSR_TX_CSR37_REG (TX_CSR_BASE + 0x94) /* CALIB */
#define TX_CSR_TX_CSR38_REG (TX_CSR_BASE + 0x98) /* CALIB */
#define TX_CSR_TX_CSR39_REG (TX_CSR_BASE + 0x9C) /* CALIB */
#define TX_CSR_TX_CSR40_REG (TX_CSR_BASE + 0xA0) /* CALIB */
#define TX_CSR_TX_CSR41_REG (TX_CSR_BASE + 0xA4) /* CALIB */
#define TX_CSR_TX_CSR42_REG (TX_CSR_BASE + 0xA8) /* RESERVED */
#define TX_CSR_TX_CSR43_REG (TX_CSR_BASE + 0xAC) /* INT */
#define TX_CSR_TX_CSR44_REG (TX_CSR_BASE + 0xB0) /* INT */
#define TX_CSR_TX_CSR45_REG (TX_CSR_BASE + 0xB4) /* INT */
#define TX_CSR_TX_CSR46_REG (TX_CSR_BASE + 0xB8) /* SPARE */
#define TX_CSR_TX_CSR47_REG (TX_CSR_BASE + 0xBC) /* SPARE */
#define TX_CSR_TX_CSR48_REG (TX_CSR_BASE + 0xC0) /* SPARE */
#define TX_CSR_TX_CSR49_REG (TX_CSR_BASE + 0xC4) /* RESERVED */
#define TX_CSR_TX_CSR50_REG (TX_CSR_BASE + 0xC8) /* POWER SEQ */
#define TX_CSR_TX_CSR51_REG (TX_CSR_BASE + 0xCC) /* RATE SWITCH */
#define TX_CSR_TX_CSR52_REG (TX_CSR_BASE + 0xD0) /* ANA */
#define TX_CSR_TX_CSR53_REG (TX_CSR_BASE + 0xD4) /* ANA */
#define TX_CSR_TX_CSR54_REG (TX_CSR_BASE + 0xD8) /* ANA */
#define TX_CSR_TX_CSR55_REG (TX_CSR_BASE + 0xDC) /* ANA */
#define TX_CSR_TX_CSR56_REG (TX_CSR_BASE + 0xE0) /* ANA */
#define TX_CSR_TX_CSR57_REG (TX_CSR_BASE + 0xE4) /* ANA */
#define TX_CSR_TX_CSR58_REG (TX_CSR_BASE + 0xE8) /* ANA */
#define TX_CSR_TX_CSR59_REG (TX_CSR_BASE + 0xEC) /* ANA */
#define TX_CSR_TX_CSR60_REG (TX_CSR_BASE + 0xF0) /* DS_COM */
#define TX_CSR_TX_CSR61_REG (TX_CSR_BASE + 0xF4) /* DS_COM */
#define TX_CSR_TX_CSR62_REG (TX_CSR_BASE + 0xF8) /* DDS_COM */
#define TX_CSR_TX_CSR63_REG (TX_CSR_BASE + 0xFC) /* DDS_COM */

#endif // __HILINK_TX_CSR_REG_OFFSET_H__
