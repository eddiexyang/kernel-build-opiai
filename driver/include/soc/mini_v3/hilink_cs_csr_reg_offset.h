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

#ifndef HILINK_CS_CSR_REG_OFFSET_H
#define HILINK_CS_CSR_REG_OFFSET_H

/* HILINK_CS_CSR Base address of Module's Register */
#define CS_CSR_BASE                       (0x40400)

/* CS_CSR Registers' Definitions */
#define CS_CSR_CS_CSR0_REG  (CS_CSR_BASE + 0x0)   /* COMMON */
#define CS_CSR_CS_CSR1_REG  (CS_CSR_BASE + 0x4)   /* CRG */
#define CS_CSR_CS_CSR2_REG  (CS_CSR_BASE + 0x8)   /* CRG */
#define CS_CSR_CS_CSR3_REG  (CS_CSR_BASE + 0xC)   /* ADPLL_FSM */
#define CS_CSR_CS_CSR4_REG  (CS_CSR_BASE + 0x10)  /* ADPLL_FSM */
#define CS_CSR_CS_CSR5_REG  (CS_CSR_BASE + 0x14)  /* ADPLL_FSM */
#define CS_CSR_CS_CSR6_REG  (CS_CSR_BASE + 0x18)  /* ADPLL_DTC */
#define CS_CSR_CS_CSR7_REG  (CS_CSR_BASE + 0x1C)  /* ADPLL_DTC */
#define CS_CSR_CS_CSR8_REG  (CS_CSR_BASE + 0x20)  /* ADPLL_DTC */
#define CS_CSR_CS_CSR9_REG  (CS_CSR_BASE + 0x24)  /* ADPLL_DTC */
#define CS_CSR_CS_CSR10_REG (CS_CSR_BASE + 0x28)  /* ADPLL_DTC */
#define CS_CSR_CS_CSR11_REG (CS_CSR_BASE + 0x2C)  /* ADPLL_DTC */
#define CS_CSR_CS_CSR12_REG (CS_CSR_BASE + 0x30)  /* ADPLL_DTC(adpll calib) */
#define CS_CSR_CS_CSR13_REG (CS_CSR_BASE + 0x34)  /* ADPLL_PFD */
#define CS_CSR_CS_CSR14_REG (CS_CSR_BASE + 0x38)  /* ADPLL_PFD */
#define CS_CSR_CS_CSR15_REG (CS_CSR_BASE + 0x3C)  /* ADPLL_PFD */
#define CS_CSR_CS_CSR16_REG (CS_CSR_BASE + 0x40)  /* ADPLL_PFD */
#define CS_CSR_CS_CSR17_REG (CS_CSR_BASE + 0x44)  /* ADPLL_LPF_MASH */
#define CS_CSR_CS_CSR18_REG (CS_CSR_BASE + 0x48)  /* ADPLL_LPF_MASH */
#define CS_CSR_CS_CSR19_REG (CS_CSR_BASE + 0x4C)  /* ADPLL_LPF_MASH */
#define CS_CSR_CS_CSR20_REG (CS_CSR_BASE + 0x50)  /* ADPLL_OLT */
#define CS_CSR_CS_CSR21_REG (CS_CSR_BASE + 0x54)  /* ADPLL_OLT */
#define CS_CSR_CS_CSR22_REG (CS_CSR_BASE + 0x58)  /* ADPLL_TCAL */
#define CS_CSR_CS_CSR23_REG (CS_CSR_BASE + 0x5C)  /* ADPLL_TCAL */
#define CS_CSR_CS_CSR24_REG (CS_CSR_BASE + 0x60)  /* ADPLL_AFC */
#define CS_CSR_CS_CSR25_REG (CS_CSR_BASE + 0x64)  /* ADPLL_AFC */
#define CS_CSR_CS_CSR26_REG (CS_CSR_BASE + 0x68)  /* ADPLL_SSCG(rate switch) */
#define CS_CSR_CS_CSR27_REG (CS_CSR_BASE + 0x6C)  /* ADPLL_SSCG(rate switch) */
#define CS_CSR_CS_CSR28_REG (CS_CSR_BASE + 0x70)  /* ADPLL_MISC(rate switch) */
#define CS_CSR_CS_CSR29_REG (CS_CSR_BASE + 0x74)  /* ADPLL_MISC(rate switch) */
#define CS_CSR_CS_CSR30_REG (CS_CSR_BASE + 0x78)  /* ADPLL_MISC(adpll calib) */
#define CS_CSR_CS_CSR31_REG (CS_CSR_BASE + 0x7C)  /* ADPLL_MISC(adpll calib) */
#define CS_CSR_CS_CSR32_REG (CS_CSR_BASE + 0x80)  /* LLD */
#define CS_CSR_CS_CSR33_REG (CS_CSR_BASE + 0x84)  /* LLD */
#define CS_CSR_CS_CSR34_REG (CS_CSR_BASE + 0x88)  /* LLD */
#define CS_CSR_CS_CSR35_REG (CS_CSR_BASE + 0x8C)  /* LLD */
#define CS_CSR_CS_CSR36_REG (CS_CSR_BASE + 0x90)  /* RESRVED */
#define CS_CSR_CS_CSR37_REG (CS_CSR_BASE + 0x94)  /* BOOSTOSC_AFC */
#define CS_CSR_CS_CSR38_REG (CS_CSR_BASE + 0x98)  /* BOOSTOSC_AFC */
#define CS_CSR_CS_CSR39_REG (CS_CSR_BASE + 0x9C)  /* BOOSTOSC_AFC */
#define CS_CSR_CS_CSR40_REG (CS_CSR_BASE + 0xA0)  /* CALIB */
#define CS_CSR_CS_CSR41_REG (CS_CSR_BASE + 0xA4)  /* CALIB */
#define CS_CSR_CS_CSR42_REG (CS_CSR_BASE + 0xA8)  /* CALIB */
#define CS_CSR_CS_CSR43_REG (CS_CSR_BASE + 0xAC)  /* CALIB */
#define CS_CSR_CS_CSR44_REG (CS_CSR_BASE + 0xB0)  /* CALIB */
#define CS_CSR_CS_CSR45_REG (CS_CSR_BASE + 0xB4)  /* CALIB */
#define CS_CSR_CS_CSR46_REG (CS_CSR_BASE + 0xB8)  /* RESRVED */
#define CS_CSR_CS_CSR47_REG (CS_CSR_BASE + 0xBC)  /* INT */
#define CS_CSR_CS_CSR48_REG (CS_CSR_BASE + 0xC0)  /* INT */
#define CS_CSR_CS_CSR49_REG (CS_CSR_BASE + 0xC4)  /* INT */
#define CS_CSR_CS_CSR50_REG (CS_CSR_BASE + 0xC8)  /* SPARE */
#define CS_CSR_CS_CSR51_REG (CS_CSR_BASE + 0xCC)  /* SPARE */
#define CS_CSR_CS_CSR52_REG (CS_CSR_BASE + 0xD0)  /* SPARE */
#define CS_CSR_CS_CSR53_REG (CS_CSR_BASE + 0xD4)  /* RESRVED */
#define CS_CSR_CS_CSR54_REG (CS_CSR_BASE + 0xD8)  /* POWER SEQ */
#define CS_CSR_CS_CSR55_REG (CS_CSR_BASE + 0xDC)  /* POWER SEQ */
#define CS_CSR_CS_CSR56_REG (CS_CSR_BASE + 0xE0)  /* RESRVED */
#define CS_CSR_CS_CSR57_REG (CS_CSR_BASE + 0xE4)  /* ANA */
#define CS_CSR_CS_CSR58_REG (CS_CSR_BASE + 0xE8)  /* ANA */
#define CS_CSR_CS_CSR59_REG (CS_CSR_BASE + 0xEC)  /* ANA */
#define CS_CSR_CS_CSR60_REG (CS_CSR_BASE + 0xF0)  /* ANA */
#define CS_CSR_CS_CSR61_REG (CS_CSR_BASE + 0xF4)  /* ANA */
#define CS_CSR_CS_CSR62_REG (CS_CSR_BASE + 0xF8)  /* ANA */
#define CS_CSR_CS_CSR63_REG (CS_CSR_BASE + 0xFC)  /* ANA */
#define CS_CSR_CS_CSR64_REG (CS_CSR_BASE + 0x100) /* ANA */
#define CS_CSR_CS_CSR65_REG (CS_CSR_BASE + 0x104) /* ANA */
#define CS_CSR_CS_CSR66_REG (CS_CSR_BASE + 0x108) /* ANA */

#endif // __HILINK_CS_CSR_REG_OFFSET_H__
