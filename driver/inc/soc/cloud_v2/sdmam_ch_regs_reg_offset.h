// ******************************************************************************
// Copyright     :  Copyright (C) 2021, Hisilicon Technologies Co. Ltd.
// File name     :  sdmam_ch_regs_reg_offset.h
// Project line  :  Platform And Key Technologies Development
// Department    :  CAD Development Department
// Author        :  xxx
// Version       :  1
// Date          :  2013/3/10
// Description   :  The description of xxx project
// Others        :  Generated automatically by nManager V4.2
// History       :  xxx 2021/06/02 15:54:13 Create file
// ******************************************************************************

#ifndef __SDMAM_CH_REGS_REG_OFFSET_H__
#define __SDMAM_CH_REGS_REG_OFFSET_H__

/* SDMAM_CH_REGS Base address of Module's Register */
#define SOC_SDMAM_CH_REGS_BASE                       (0x0)

/******************************************************************************/
/*                      SOC SDMAM_CH_REGS Registers' Definitions                            */
/******************************************************************************/

#define SOC_SDMAM_CH_REGS_SDMAM_CH_CTRL_REG         (SOC_SDMAM_CH_REGS_BASE + 0x0)   /* SDMAM_CHL控制寄存器 */
#define SOC_SDMAM_CH_REGS_SDMAM_CH_IIDR_REG         (SOC_SDMAM_CH_REGS_BASE + 0x4)   /* SDMAM_CHL版本寄存器 */
#define SOC_SDMAM_CH_REGS_SDMAM_CH_TYPER_REG        (SOC_SDMAM_CH_REGS_BASE + 0x8)   /* SDMAM_CHL特性寄存器 */
#define SOC_SDMAM_CH_REGS_SDMAM_IRQ_STATUS_REG      (SOC_SDMAM_CH_REGS_BASE + 0xC)   /* SDMAM_中断状态寄存器 */
#define SOC_SDMAM_CH_REGS_SDMAM_IRQ_CTRL_REG        (SOC_SDMAM_CH_REGS_BASE + 0x10)  /* SDMAM中断控制寄存器 */
#define SOC_SDMAM_CH_REGS_SDMAM_CH_BYPASS_CTRL_REG  (SOC_SDMAM_CH_REGS_BASE + 0x14)  /* BYPASS SMMU 模式，通道搬运属性控制寄存器 */
#define SOC_SDMAM_CH_REGS_SDMAM_CH_CQE_STATUS_REG   (SOC_SDMAM_CH_REGS_BASE + 0x18)  /* SDMAM CQE状态寄存器 */
#define SOC_SDMAM_CH_REGS_SDMAM_CH_SQBASER_L_REG    (SOC_SDMAM_CH_REGS_BASE + 0x40)  /* SQ基地址低32bit配置寄存器 */
#define SOC_SDMAM_CH_REGS_SDMAM_CH_SQBASER_H_REG    (SOC_SDMAM_CH_REGS_BASE + 0x44)  /* SQ基地址高32bit配置寄存器 */
#define SOC_SDMAM_CH_REGS_SDMAM_CH_SQ_ATTR_REG      (SOC_SDMAM_CH_REGS_BASE + 0x48)  /* SQ配置寄存器 */
#define SOC_SDMAM_CH_REGS_SDMAM_CH_SQTDBR_REG       (SOC_SDMAM_CH_REGS_BASE + 0x4C)  /* SQ队列尾指针寄存器 */
#define SOC_SDMAM_CH_REGS_SDMAM_CH_SQHDBR_REG       (SOC_SDMAM_CH_REGS_BASE + 0x50)  /* SQ队列头指针寄存器 */
#define SOC_SDMAM_CH_REGS_SDMAM_CH_CQBASER_L_REG    (SOC_SDMAM_CH_REGS_BASE + 0x80)  /* CQ基地址低32bit配置寄存器 */
#define SOC_SDMAM_CH_REGS_SDMAM_CH_CQBASER_H_REG    (SOC_SDMAM_CH_REGS_BASE + 0x84)  /* CQ基地址高32bit配置寄存器 */
#define SOC_SDMAM_CH_REGS_SDMAM_CH_CQ_ATTR_REG      (SOC_SDMAM_CH_REGS_BASE + 0x88)  /* CQ配置寄存器 */
#define SOC_SDMAM_CH_REGS_SDMAM_CH_CQTDBR_REG       (SOC_SDMAM_CH_REGS_BASE + 0x8C)  /* CQ队列尾指针寄存器 */
#define SOC_SDMAM_CH_REGS_SDMAM_CH_CQHDBR_REG       (SOC_SDMAM_CH_REGS_BASE + 0x90)  /* CQ队列头指针寄存器 */
#define SOC_SDMAM_CH_REGS_SDMAM_CH_ICDT_OSTD_TH_REG (SOC_SDMAM_CH_REGS_BASE + 0x94)  /* 给SDMAA发送任务的outstanding数限制 */
#define SOC_SDMAM_CH_REGS_SDMAM_ATOMICERR_MASK_REG  (SOC_SDMAM_CH_REGS_BASE + 0x98)  /* ATOMIC ERROR上报mask配置寄存器 */
#define SOC_SDMAM_CH_REGS_DFX_CTRL0_REG             (SOC_SDMAM_CH_REGS_BASE + 0x200) /* DFX ctrl Register0 */
#define SOC_SDMAM_CH_REGS_DFX_INF0_REG              (SOC_SDMAM_CH_REGS_BASE + 0x204) /* DFX信息0 */
#define SOC_SDMAM_CH_REGS_DFX_INF1_REG              (SOC_SDMAM_CH_REGS_BASE + 0x208) /* DFX信息1 */
#define SOC_SDMAM_CH_REGS_DFX_INF2_REG              (SOC_SDMAM_CH_REGS_BASE + 0x20C) /* DFX信息2 */
#define SOC_SDMAM_CH_REGS_DFX_INF3_REG              (SOC_SDMAM_CH_REGS_BASE + 0x210) /* DFX信息3 */
#define SOC_SDMAM_CH_REGS_DFX_INF4_REG              (SOC_SDMAM_CH_REGS_BASE + 0x214) /* DFX信息4 */
#define SOC_SDMAM_CH_REGS_DFX_INF5_REG              (SOC_SDMAM_CH_REGS_BASE + 0x218) /* DFX信息5 */
#define SOC_SDMAM_CH_REGS_DFX_INF6_REG              (SOC_SDMAM_CH_REGS_BASE + 0x21C) /* DFX信息6 */
#define SOC_SDMAM_CH_REGS_DFX_INF7_REG              (SOC_SDMAM_CH_REGS_BASE + 0x220) /* DFX信息7 */
#define SOC_SDMAM_CH_REGS_DFX_EMU_INFO0_REG         (SOC_SDMAM_CH_REGS_BASE + 0x300) /* SDMAM EMU PRESS CNT寄存器0 */
#define SOC_SDMAM_CH_REGS_DFX_EMU_INFO1_REG         (SOC_SDMAM_CH_REGS_BASE + 0x304) /* SDMAM EMU PRESS CNT寄存器1 */
#define SOC_SDMAM_CH_REGS_DFX_EMU_INFO2_REG         (SOC_SDMAM_CH_REGS_BASE + 0x308) /* SDMAM EMU PRESS CNT寄存器2 */
#define SOC_SDMAM_CH_REGS_DFX_EMU_INFO3_REG         (SOC_SDMAM_CH_REGS_BASE + 0x30C) /* SDMAM EMU PRESS CNT寄存器3 */
#define SOC_SDMAM_CH_REGS_DFX_EMU_INFO4_REG         (SOC_SDMAM_CH_REGS_BASE + 0x310) /* SDMAM EMU PRESS CNT寄存器4 */
#define SOC_SDMAM_CH_REGS_DFX_EMU_INFO5_REG         (SOC_SDMAM_CH_REGS_BASE + 0x314) /* SDMAM EMU PRESS CNT寄存器5 */
#define SOC_SDMAM_CH_REGS_DFX_EMU_INFO6_REG         (SOC_SDMAM_CH_REGS_BASE + 0x318) /* SDMAM EMU PRESS CNT寄存器6 */
#define SOC_SDMAM_CH_REGS_DFX_EMU_INFO7_REG         (SOC_SDMAM_CH_REGS_BASE + 0x31C) /* SDMAM EMU PRESS CNT寄存器7 */
#define SOC_SDMAM_CH_REGS_DFX_EMU_INFO8_REG         (SOC_SDMAM_CH_REGS_BASE + 0x320) /* SDMAM EMU PRESS CNT寄存器8 */
#define SOC_SDMAM_CH_REGS_DFX_EMU_INFO9_REG         (SOC_SDMAM_CH_REGS_BASE + 0x324) /* SDMAM EMU PRESS CNT寄存器9 */
#define SOC_SDMAM_CH_REGS_DFX_EMU_INFO10_REG        (SOC_SDMAM_CH_REGS_BASE + 0x328) /* SDMAM EMU PRESS CNT寄存器10 */
#define SOC_SDMAM_CH_REGS_DFX_EMU_INFO11_REG        (SOC_SDMAM_CH_REGS_BASE + 0x32C) /* SDMAM EMU PRESS CNT寄存器11 */
#define SOC_SDMAM_CH_REGS_DFX_EMU_INFO12_REG        (SOC_SDMAM_CH_REGS_BASE + 0x330) /* SDMAM EMU PRESS CNT寄存器12 */
#define SOC_SDMAM_CH_REGS_DFX_EMU_INFO13_REG        (SOC_SDMAM_CH_REGS_BASE + 0x334) /* SDMAM EMU PRESS CNT寄存器13 */
#define SOC_SDMAM_CH_REGS_DFX_EMU_INFO14_REG        (SOC_SDMAM_CH_REGS_BASE + 0x338) /* SDMAM EMU PRESS CNT寄存器14 */
#define SOC_SDMAM_CH_REGS_DFX_EMU_INFO15_REG        (SOC_SDMAM_CH_REGS_BASE + 0x33C) /* SDMAM EMU PRESS CNT寄存器15 */

#endif // __SDMAM_CH_REGS_REG_OFFSET_H__
