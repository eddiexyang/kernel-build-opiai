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

#ifndef HILINK_RX_CSR_REG_OFFSET_H
#define HILINK_RX_CSR_REG_OFFSET_H

/* HILINK_RX_CSR Base address of Module's Register */
#define RX_CSR_BASE                       (0x48400)

/* hilink HILINK_RX_CSR Registers' Definitions*/
#define RX_CSR_RX_CSR0_REG   (RX_CSR_BASE + 0x0)   /* SLICE ID */
#define RX_CSR_RX_CSR1_REG   (RX_CSR_BASE + 0x4)   /* FSM */
#define RX_CSR_RX_CSR2_REG   (RX_CSR_BASE + 0x8)   /* FSM */
#define RX_CSR_RX_CSR3_REG   (RX_CSR_BASE + 0xC)   /* FSM */
#define RX_CSR_RX_CSR4_REG   (RX_CSR_BASE + 0x10)  /* FSM */
#define RX_CSR_RX_CSR5_REG   (RX_CSR_BASE + 0x14)  /* FSM */
#define RX_CSR_RX_CSR6_REG   (RX_CSR_BASE + 0x18)  /* FSM */
#define RX_CSR_RX_CSR7_REG   (RX_CSR_BASE + 0x1C)  /* FSM */
#define RX_CSR_RX_CSR8_REG   (RX_CSR_BASE + 0x20)  /* FSM */
#define RX_CSR_RX_CSR9_REG   (RX_CSR_BASE + 0x24)  /* FSM */
#define RX_CSR_RX_CSR10_REG  (RX_CSR_BASE + 0x28)  /* FSM */
#define RX_CSR_RX_CSR11_REG  (RX_CSR_BASE + 0x2C)  /* FSM */
#define RX_CSR_RX_CSR12_REG  (RX_CSR_BASE + 0x30)  /* FSM */
#define RX_CSR_RX_CSR13_REG  (RX_CSR_BASE + 0x34)  /* FSM */
#define RX_CSR_RX_CSR14_REG  (RX_CSR_BASE + 0x38)  /* RX CMD CTRL */
#define RX_CSR_RX_CSR15_REG  (RX_CSR_BASE + 0x3C)  /* RX CMD CTRL */
#define RX_CSR_RX_CSR16_REG  (RX_CSR_BASE + 0x40)  /* RX CMD CTRL */
#define RX_CSR_RX_CSR17_REG  (RX_CSR_BASE + 0x44)  /* RX CMD CTRL */
#define RX_CSR_RX_CSR18_REG  (RX_CSR_BASE + 0x48)  /* RX CMD CTRL */
#define RX_CSR_RX_CSR19_REG  (RX_CSR_BASE + 0x4C)  /* RX CMD CTRL */
#define RX_CSR_RX_CSR20_REG  (RX_CSR_BASE + 0x50)  /* RESERVED */
#define RX_CSR_RX_CSR21_REG  (RX_CSR_BASE + 0x54)  /* RX_CRG */
#define RX_CSR_RX_CSR22_REG  (RX_CSR_BASE + 0x58)  /* RX_CRG */
#define RX_CSR_RX_CSR23_REG  (RX_CSR_BASE + 0x5C)  /* RX_PRBS */
#define RX_CSR_RX_CSR24_REG  (RX_CSR_BASE + 0x60)  /* RX_LOS */
#define RX_CSR_RX_CSR25_REG  (RX_CSR_BASE + 0x64)  /* RX_LOS */
#define RX_CSR_RX_CSR26_REG  (RX_CSR_BASE + 0x68)  /* RX_GEARBOX */
#define RX_CSR_RX_CSR27_REG  (RX_CSR_BASE + 0x6C)  /* RX_PIN */
#define RX_CSR_RX_CSR28_REG  (RX_CSR_BASE + 0x70)  /* RX_CDR0(RATE SWITCH) */
#define RX_CSR_RX_CSR29_REG  (RX_CSR_BASE + 0x74)  /* RX_CDR1 */
#define RX_CSR_RX_CSR30_REG  (RX_CSR_BASE + 0x78)  /* RX_CDR2 */
#define RX_CSR_RX_CSR31_REG  (RX_CSR_BASE + 0x7C)  /* RX_CDR3 */
#define RX_CSR_RX_CSR32_REG  (RX_CSR_BASE + 0x80)  /* RX_CDR4 */
#define RX_CSR_RX_CSR33_REG  (RX_CSR_BASE + 0x84)  /* RX_CDR5 */
#define RX_CSR_RX_CSR34_REG  (RX_CSR_BASE + 0x88)  /* RX_CDR6 */
#define RX_CSR_RX_CSR35_REG  (RX_CSR_BASE + 0x8C)  /* RX_CDR7 */
#define RX_CSR_RX_CSR36_REG  (RX_CSR_BASE + 0x90)  /* RX_CDR8 */
#define RX_CSR_RX_CSR37_REG  (RX_CSR_BASE + 0x94)  /* RX_CD9 */
#define RX_CSR_RX_CSR38_REG  (RX_CSR_BASE + 0x98)  /* RX CM Coarse Align */
#define RX_CSR_RX_CSR39_REG  (RX_CSR_BASE + 0x9C)  /* RX PERIOD PATTERN DET */
#define RX_CSR_RX_CSR40_REG  (RX_CSR_BASE + 0xA0)
#define RX_CSR_RX_CSR41_REG  (RX_CSR_BASE + 0xA4)  /* RX ADAP-LMS */
#define RX_CSR_RX_CSR42_REG  (RX_CSR_BASE + 0xA8)  /* RX ADAP-LMS */
#define RX_CSR_RX_CSR43_REG  (RX_CSR_BASE + 0xAC)  /* RX ADAP-LMS */
#define RX_CSR_RX_CSR44_REG  (RX_CSR_BASE + 0xB0)  /* RX ADAP-EM */
#define RX_CSR_RX_CSR45_REG  (RX_CSR_BASE + 0xB4)  /* RX ADAP-EM */
#define RX_CSR_RX_CSR46_REG  (RX_CSR_BASE + 0xB8)  /* RX ADAP-EM */
#define RX_CSR_RX_CSR47_REG  (RX_CSR_BASE + 0xBC)  /* RX ADAP-EM */
#define RX_CSR_RX_CSR48_REG  (RX_CSR_BASE + 0xC0)  /* RX ADAP-EM */
#define RX_CSR_RX_CSR49_REG  (RX_CSR_BASE + 0xC4)  /* RX ADAP-EM */
#define RX_CSR_RX_CSR50_REG  (RX_CSR_BASE + 0xC8)  /* RX ADAP-EM */
#define RX_CSR_RX_CSR51_REG  (RX_CSR_BASE + 0xCC)  /* RX ADAP-EM */
#define RX_CSR_RX_CSR52_REG  (RX_CSR_BASE + 0xD0)  /* RX ADAP-EM */
#define RX_CSR_RX_CSR53_REG  (RX_CSR_BASE + 0xD4)  /* RX ADAP-EM */
#define RX_CSR_RX_CSR54_REG  (RX_CSR_BASE + 0xD8)  /* RX ADAP-EM */
#define RX_CSR_RX_CSR55_REG  (RX_CSR_BASE + 0xDC)  /* RX ADAP-EM */
#define RX_CSR_RX_CSR56_REG  (RX_CSR_BASE + 0xE0)  /* RX ADAP-EM */
#define RX_CSR_RX_CSR57_REG  (RX_CSR_BASE + 0xE4)
#define RX_CSR_RX_CSR58_REG  (RX_CSR_BASE + 0xE8)  /* RX_CALIB */
#define RX_CSR_RX_CSR59_REG  (RX_CSR_BASE + 0xEC)  /* RX_CALIB */
#define RX_CSR_RX_CSR60_REG  (RX_CSR_BASE + 0xF0)  /* RX_CALIB */
#define RX_CSR_RX_CSR61_REG  (RX_CSR_BASE + 0xF4)  /* RX_CALIB */
#define RX_CSR_RX_CSR62_REG  (RX_CSR_BASE + 0xF8)  /* RX_CALIB */
#define RX_CSR_RX_CSR63_REG  (RX_CSR_BASE + 0xFC)  /* RX_CALIB */
#define RX_CSR_RX_CSR64_REG  (RX_CSR_BASE + 0x100) /* RX_CALIB */
#define RX_CSR_RX_CSR65_REG  (RX_CSR_BASE + 0x104) /* RX_CALIB */
#define RX_CSR_RX_CSR66_REG  (RX_CSR_BASE + 0x108) /* RX_CALIB */
#define RX_CSR_RX_CSR67_REG  (RX_CSR_BASE + 0x10C) /* RX_CALIB */
#define RX_CSR_RX_CSR68_REG  (RX_CSR_BASE + 0x110) /* RX_CALIB */
#define RX_CSR_RX_CSR69_REG  (RX_CSR_BASE + 0x114) /* RX_CALIB_VTH */
#define RX_CSR_RX_CSR70_REG  (RX_CSR_BASE + 0x118) /* RX_CALIB_VTH */
#define RX_CSR_RX_CSR71_REG  (RX_CSR_BASE + 0x11C)
#define RX_CSR_RX_CSR72_REG  (RX_CSR_BASE + 0x120) /* INT source */
#define RX_CSR_RX_CSR73_REG  (RX_CSR_BASE + 0x124) /* INT mask */
#define RX_CSR_RX_CSR74_REG  (RX_CSR_BASE + 0x128) /* INT RO */
#define RX_CSR_RX_CSR75_REG  (RX_CSR_BASE + 0x12C) /* RX_DIG_SPARE */
#define RX_CSR_RX_CSR76_REG  (RX_CSR_BASE + 0x130) /* RX_DIG_SPARE */
#define RX_CSR_RX_CSR77_REG  (RX_CSR_BASE + 0x134)
#define RX_CSR_RX_CSR78_REG  (RX_CSR_BASE + 0x138)
#define RX_CSR_RX_CSR79_REG  (RX_CSR_BASE + 0x13C)
#define RX_CSR_RX_CSR80_REG  (RX_CSR_BASE + 0x140) /* ANA */
#define RX_CSR_RX_CSR81_REG  (RX_CSR_BASE + 0x144) /* POWER SEQUENCE */
#define RX_CSR_RX_CSR82_REG  (RX_CSR_BASE + 0x148) /* POWER SEQUENCE */
#define RX_CSR_RX_CSR83_REG  (RX_CSR_BASE + 0x14C) /* POWER SEQUENCE */
#define RX_CSR_RX_CSR84_REG  (RX_CSR_BASE + 0x150) /* POWER SEQUENCE */
#define RX_CSR_RX_CSR85_REG  (RX_CSR_BASE + 0x154) /* POWER SEQUENCE */
#define RX_CSR_RX_CSR86_REG  (RX_CSR_BASE + 0x158) /* RATE SWITCH */
#define RX_CSR_RX_CSR87_REG  (RX_CSR_BASE + 0x15C) /* RATE SWITCH */
#define RX_CSR_RX_CSR88_REG  (RX_CSR_BASE + 0x160)
#define RX_CSR_RX_CSR89_REG  (RX_CSR_BASE + 0x164) /* ATT A/D CONTROL */
#define RX_CSR_RX_CSR90_REG  (RX_CSR_BASE + 0x168)
#define RX_CSR_RX_CSR91_REG  (RX_CSR_BASE + 0x16C)
#define RX_CSR_RX_CSR92_REG  (RX_CSR_BASE + 0x170)
#define RX_CSR_RX_CSR93_REG  (RX_CSR_BASE + 0x174)
#define RX_CSR_RX_CSR94_REG  (RX_CSR_BASE + 0x178)
#define RX_CSR_RX_CSR95_REG  (RX_CSR_BASE + 0x17C)
#define RX_CSR_RX_CSR96_REG  (RX_CSR_BASE + 0x180)
#define RX_CSR_RX_CSR97_REG  (RX_CSR_BASE + 0x184)
#define RX_CSR_RX_CSR98_REG  (RX_CSR_BASE + 0x188)
#define RX_CSR_RX_CSR99_REG  (RX_CSR_BASE + 0x18C)
#define RX_CSR_RX_CSR100_REG (RX_CSR_BASE + 0x190)
#define RX_CSR_RX_CSR101_REG (RX_CSR_BASE + 0x194)
#define RX_CSR_RX_CSR102_REG (RX_CSR_BASE + 0x198) /* RX ATB A/D CONTROL */
#define RX_CSR_RX_CSR103_REG (RX_CSR_BASE + 0x19C) /* RX ATB A/D CONTROL */
#define RX_CSR_RX_CSR104_REG (RX_CSR_BASE + 0x1A0)
#define RX_CSR_RX_CSR105_REG (RX_CSR_BASE + 0x1A4)
#define RX_CSR_RX_CSR106_REG (RX_CSR_BASE + 0x1A8)
#define RX_CSR_RX_CSR107_REG (RX_CSR_BASE + 0x1AC)

#endif // __HILINK_RX_CSR_REG_OFFSET_H__
