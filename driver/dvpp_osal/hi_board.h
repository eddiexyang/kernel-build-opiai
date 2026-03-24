/*
* Copyright (c) Hisilicon Technologies Co., Ltd. 2011-2019. All rights reserved.
* Description: Definitions of registers and boards
* Author: Hisilicon multimedia software group
* Create: 2011/06/28
*/

#ifndef __HI_BOARD_H__
#define __HI_BOARD_H__

#include "hi_common.h"

#define DDR_BUS_FR                310000000

/* 暂不提供DDRC控制器相关寄存器设置 */
#define DDRC0_REG_ADDR            0x20110000 /* base addr of DDRCB */

#define DDRC0_REG_STAT_CFG_ADDR   (DDRC0_REG_ADDR + 0x260)
#define DDRC0_REG_STAT_WRITE_ADDR (DDRC0_REG_ADDR + 0x264)
#define DDRC0_REG_STAT_READ_ADDR  (DDRC0_REG_ADDR + 0x268)

#define CRG_REGS_ADDR             0x12010000

#define SYS_REGS_ADDR             0x12020000

#define MISC_REGS_ADDR            0x12030000

#define VPSS0_REGS_ADDR           0x3001E0000UL // 0x11040000
#define VPSS_REGS_SIZE            0x20000

#define VI_CAP0_REGS_ADDR         0x300400000UL // 0x11300000
#define VI_PROC0_REGS_ADDR        0x300200000UL // 0x11000000

#define VICAP_WRAP_BASE_PA        0x300600000UL
#define VIPROC_WRAP_BASE_PA       0x300240000UL
#define VIPE_WRAP_BASE_PA         0x3001F0000UL
#define VGS_WRAP_BASE_PA          0x2010D0000UL
#define GDC_WRAP_BASE_PA          0x2010B0000UL

/* Interrupt Request Number  */
#define VOU_IRQ_NR                90
#define VOU1_IRQ_NR               91
#define VI_CAP0_IRQ_NR            88
#define VI_PROC0_IRQ_NR           76
#define VPSS0_IRQ_NR              75
#define TDE_IRQ_NR                67
#define VGS0_IRQ_NR               70
#define AIO_IRQ_NR                87
#define VEDU_0_IRQ_NR             72

#define JPEGU_IRQ_NR              68
#define JPEGD_IRQ_NR              77
#define IVE_IRQ_NR                69
#define GDC0_IRQ_NR               74
#define SVP_NNIE_IRQ_NR0          73
#define CRG_PERCTL10_ADDR         (0x28 + CRG_REGS_ADDR)
#define CRG_PERCTL11_ADDR         (0x2C + CRG_REGS_ADDR)
#define CRG_PERCTL17_ADDR         (0x44 + CRG_REGS_ADDR)
#define CRG_PERCTL18_ADDR         (0x48 + CRG_REGS_ADDR)
#define CRG_PERCTL19_ADDR         (0x4c + CRG_REGS_ADDR)
#define CRG_PERCTL22_ADDR         (0x58 + CRG_REGS_ADDR)
#define CRG_PERCTL26_ADDR         (0x68 + CRG_REGS_ADDR)
#define CRG_PERCTL31_ADDR         (0x7C + CRG_REGS_ADDR)
#define CRG_PERCTL55_ADDR         (0xdC + CRG_REGS_ADDR)
#define CRG_PERCTL57_ADDR         (0xe4 + CRG_REGS_ADDR)
#define CRG_PERCTL63_ADDR         (0xfC + CRG_REGS_ADDR)
#define CRG_PERCTL64_ADDR         (0x100 + CRG_REGS_ADDR)
#define CRG_PERCTL65_ADDR         (0x104 + CRG_REGS_ADDR)
#define CRG_PERCTL67_ADDR         (0x10c + CRG_REGS_ADDR)
#define CRG_PERCTL71_ADDR         (0x11c + CRG_REGS_ADDR)
#define CRG_PERCTL72_ADDR         (0x120 + CRG_REGS_ADDR)
#define CRG_PERCTL73_ADDR         (0x124 + CRG_REGS_ADDR)
#define CRG_PERCTL75_ADDR         (0x12C + CRG_REGS_ADDR)
#define CRG_PERCTL77_ADDR         (0x134 + CRG_REGS_ADDR)
#define CRG_PERCTL79_ADDR         (0x13C + CRG_REGS_ADDR)
#define CRG_PERCTL84_ADDR         (0x150 + CRG_REGS_ADDR)
#define CRG_PERCTL85_ADDR         (0x154 + CRG_REGS_ADDR)
#define CRG_PERCTL87_ADDR         (0x15c + CRG_REGS_ADDR)
#define CRG_PERCTL88_ADDR         (0x160 + CRG_REGS_ADDR)
#define CRG_PERCTL89_ADDR         (0x164 + CRG_REGS_ADDR)
#define CRG_PERCTL90_ADDR         (0x168 + CRG_REGS_ADDR)

#define MISC_CTL12C_ADDR          (0x12C + MISC_REGS_ADDR)
#define MISC_CTL98_ADDR           (0x98 + MISC_REGS_ADDR)
#define MISC_CTL9C_ADDR           (0x9C + MISC_REGS_ADDR)
#define MISC_CTLAC_ADDR           (0xAC + MISC_REGS_ADDR)
#define MISC_CTLB0_ADDR           (0xB0 + MISC_REGS_ADDR)
#define MISC_CTL2014_ADDR         (0x2014 + MISC_REGS_ADDR)

#endif /* __HI_BOARD_H__ */
