/*
 * Copyright     :  Copyright (C) 2021, Huawei Technologies Co. Ltd.
 * File name     :  gicr_vlpi_regs_reg_offset.h
 * Project line  :  Platform And Key Technologies Development
 * Department    :  CAD Development Department
 * Author        :  xxx
 * Version       :  1
 * Date          :  2013/3/10
 * Description   :  The description of xxx project
 * Others        :  Generated automatically by nManager V5.1 
 * History       :  xxx 2021/10/25 08:35:56 Create file
 */

#ifndef __GICR_VLPI_REGS_REG_OFFSET_H__
#define __GICR_VLPI_REGS_REG_OFFSET_H__

/* GICR_VLPI_REGS Base address of Module's Register */
#define SOC_GICR_VLPI_REGS_BASE                       (0x50000000)

/******************************************************************************/
/*                      SOC GICR_VLPI_REGS Registers' Definitions                            */
/******************************************************************************/

#define SOC_GICR_VLPI_REGS_GICR_VSETLPIR_L_REG   (SOC_GICR_VLPI_REGS_BASE + 0x40)  /* 产生VLPI寄存器[31:0] */
#define SOC_GICR_VLPI_REGS_GICR_VSETLPIR_H_REG   (SOC_GICR_VLPI_REGS_BASE + 0x44)  /* 产生VLPI寄存器[63:32] */
#define SOC_GICR_VLPI_REGS_GICR_VCLRLPIR_L_REG   (SOC_GICR_VLPI_REGS_BASE + 0x48)  /* 清除VLPI寄存器[31:0] */
#define SOC_GICR_VLPI_REGS_GICR_VCLRLPIR_H_REG   (SOC_GICR_VLPI_REGS_BASE + 0x4C)  /* 清除VLPI寄存器[63:32] */
#define SOC_GICR_VLPI_REGS_GICR_VPROPBASER_L_REG (SOC_GICR_VLPI_REGS_BASE + 0x70)  /* 当前VM的VLPIs配置信息表基地址寄存器[31:0] */
#define SOC_GICR_VLPI_REGS_GICR_VPROPBASER_H_REG (SOC_GICR_VLPI_REGS_BASE + 0x74)  /* 当前VM的VLPIs配置信息表基地址寄存器[63:32] */
#define SOC_GICR_VLPI_REGS_GICR_VPENDBASER_L_REG (SOC_GICR_VLPI_REGS_BASE + 0x78)  /* 当前VM的VLPIs状态信息表基地址寄存器[31:0] */
#define SOC_GICR_VLPI_REGS_GICR_VPENDBASER_H_REG (SOC_GICR_VLPI_REGS_BASE + 0x7C)  /* 当前VM的VLPIs状态信息表基地址寄存器[63:32] */
#define SOC_GICR_VLPI_REGS_GICR_VINVR_L_REG      (SOC_GICR_VLPI_REGS_BASE + 0xA0)  /* 无效单个VLPI寄存器[31:0] */
#define SOC_GICR_VLPI_REGS_GICR_VINVR_H_REG      (SOC_GICR_VLPI_REGS_BASE + 0xA4)  /* 无效单个VLPI寄存器[63:32] */
#define SOC_GICR_VLPI_REGS_GICR_VINVALLR_L_REG   (SOC_GICR_VLPI_REGS_BASE + 0xB0)  /* 无效所有VLPI寄存器[31:0] */
#define SOC_GICR_VLPI_REGS_GICR_VINVALLR_H_REG   (SOC_GICR_VLPI_REGS_BASE + 0xB4)  /* 无效所有VLPI寄存器[63:32] */
#define SOC_GICR_VLPI_REGS_GICR_VSYNCR_REG       (SOC_GICR_VLPI_REGS_BASE + 0xC0)  /* VLPIs同步寄存器 */
#define SOC_GICR_VLPI_REGS_GICR_VSRCR0_L_REG     (SOC_GICR_VLPI_REGS_BASE + 0x100) /* 源VLPIs状态信息表基地址寄存器[31:0] */
#define SOC_GICR_VLPI_REGS_GICR_VSRCR0_H_REG     (SOC_GICR_VLPI_REGS_BASE + 0x104) /* 源VLPIs状态信息表基地址寄存器[63:32] */
#define SOC_GICR_VLPI_REGS_GICR_VSRCR1_L_REG     (SOC_GICR_VLPI_REGS_BASE + 0x108) /* 源VLPIs状态信息表基地址寄存器[31:0] */
#define SOC_GICR_VLPI_REGS_GICR_VSRCR1_H_REG     (SOC_GICR_VLPI_REGS_BASE + 0x10C) /* 源VLPIs状态信息表基地址寄存器[63:32] */
#define SOC_GICR_VLPI_REGS_GICR_VSRCR2_L_REG     (SOC_GICR_VLPI_REGS_BASE + 0x110) /* 源VLPIs状态信息表基地址寄存器[31:0] */
#define SOC_GICR_VLPI_REGS_GICR_VSRCR2_H_REG     (SOC_GICR_VLPI_REGS_BASE + 0x114) /* 源VLPIs状态信息表基地址寄存器[63:32] */
#define SOC_GICR_VLPI_REGS_GICR_VDESTR0_L_REG    (SOC_GICR_VLPI_REGS_BASE + 0x180) /* 目标VLPIs状态信息表基地址寄存器[31:0] */
#define SOC_GICR_VLPI_REGS_GICR_VDESTR0_H_REG    (SOC_GICR_VLPI_REGS_BASE + 0x184) /* 目标VLPIs状态信息表基地址寄存器[63:32] */
#define SOC_GICR_VLPI_REGS_GICR_VDESTR1_L_REG    (SOC_GICR_VLPI_REGS_BASE + 0x188) /* 目标VLPIs状态信息表基地址寄存器[31:0] */
#define SOC_GICR_VLPI_REGS_GICR_VDESTR1_H_REG    (SOC_GICR_VLPI_REGS_BASE + 0x18C) /* 目标VLPIs状态信息表基地址寄存器[63:32] */
#define SOC_GICR_VLPI_REGS_GICR_VDESTR2_L_REG    (SOC_GICR_VLPI_REGS_BASE + 0x190) /* 目标VLPIs状态信息表基地址寄存器[31:0] */
#define SOC_GICR_VLPI_REGS_GICR_VDESTR2_H_REG    (SOC_GICR_VLPI_REGS_BASE + 0x194) /* 目标VLPIs状态信息表基地址寄存器[63:32] */
#define SOC_GICR_VLPI_REGS_GICR_VMOVLPIR0_L_REG  (SOC_GICR_VLPI_REGS_BASE + 0x200) /* 转移单个VLPI寄存器[31:0] */
#define SOC_GICR_VLPI_REGS_GICR_VMOVLPIR0_H_REG  (SOC_GICR_VLPI_REGS_BASE + 0x204) /* 转移单个VLPI寄存器[63:32] */
#define SOC_GICR_VLPI_REGS_GICR_VMOVLPIR1_L_REG  (SOC_GICR_VLPI_REGS_BASE + 0x208) /* 转移单个VLPI寄存器[31:0] */
#define SOC_GICR_VLPI_REGS_GICR_VMOVLPIR1_H_REG  (SOC_GICR_VLPI_REGS_BASE + 0x20C) /* 转移单个VLPI寄存器[63:32] */
#define SOC_GICR_VLPI_REGS_GICR_VMOVLPIR2_L_REG  (SOC_GICR_VLPI_REGS_BASE + 0x210) /* 转移单个VLPI寄存器[31:0] */
#define SOC_GICR_VLPI_REGS_GICR_VMOVLPIR2_H_REG  (SOC_GICR_VLPI_REGS_BASE + 0x214) /* 转移单个VLPI寄存器[63:32] */

#endif // __GICR_VLPI_REGS_REG_OFFSET_H__
