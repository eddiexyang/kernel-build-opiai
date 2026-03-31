// ******************************************************************************
// Copyright     :  Copyright (C) 2021, Hisilicon Technologies Co. Ltd.
// File name     :  stars_sram_reg_offset.h
// Project line  :  Platform And Key Technologies Development
// Department    :  CAD Development Department
// Author        :  xxx
// Version       :  1.0
// Date          :  2013/3/10
// Description   :  The description of xxx project
// Others        :  Generated automatically by nManager V4.2
// History       :  xxx 2021/10/23 10:06:44 Create file
// ******************************************************************************

#ifndef __STARS_SRAM_REG_OFFSET_H__
#define __STARS_SRAM_REG_OFFSET_H__

/* STARS_SRAM Base address of Module's Register */
#define xxx_STARS_SRAM_BASE                       (0x0)

/******************************************************************************/
/*                      xxx STARS_SRAM Registers' Definitions                            */
/******************************************************************************/

#define xxx_STARS_SRAM_SRAM_CTRL_REG       (xxx_STARS_SRAM_BASE + 0x0)  /* SRAM控制寄存器 */
#define xxx_STARS_SRAM_SRAM_STATE_REG      (xxx_STARS_SRAM_BASE + 0x4)  /* SRAM状态寄存器 */
#define xxx_STARS_SRAM_SRAM_SEC_CTRL_REG   (xxx_STARS_SRAM_BASE + 0x10) /* SRAM安全空间控制寄存器 */
#define xxx_STARS_SRAM_SRAM_SEC_ADDR_REG   (xxx_STARS_SRAM_BASE + 0x14) /* SRAM安全空间地址寄存器 */
#define xxx_STARS_SRAM_SRAM_SEC_SIZE_REG   (xxx_STARS_SRAM_BASE + 0x18) /* SRAM安全空间Size寄存器 */
#define xxx_STARS_SRAM_SRAM_BASE_ADDR_REG  (xxx_STARS_SRAM_BASE + 0x20) /* SRAM 基地址地址寄存器 */
#define xxx_STARS_SRAM_SRAM_INTMASK_REG    (xxx_STARS_SRAM_BASE + 0x40) /* SRAM中断屏蔽寄存器 */
#define xxx_STARS_SRAM_SRAM_RAWINT_REG     (xxx_STARS_SRAM_BASE + 0x44) /* SRAM原始中断状态寄存器 */
#define xxx_STARS_SRAM_SRAM_INTSTS_REG     (xxx_STARS_SRAM_BASE + 0x48) /* SRAM中断状态寄存器 */
#define xxx_STARS_SRAM_SRAM_INTCLR_REG     (xxx_STARS_SRAM_BASE + 0x4C) /* SRAM中断清除寄存器 */
#define xxx_STARS_SRAM_SRAM_ECC_INJECT_REG (xxx_STARS_SRAM_BASE + 0x60) /* SRAM ECC注入寄存器 */
#define xxx_STARS_SRAM_SRAM_ERR_INFO_REG   (xxx_STARS_SRAM_BASE + 0x64) /* SRAM错误操作信息寄存器 */
#define xxx_STARS_SRAM_SRAM_ERR_ADDR_REG   (xxx_STARS_SRAM_BASE + 0x68) /* SRAM错误操作地址寄存器 */
#define xxx_STARS_SRAM_SRAM_ECC_INFO_REG   (xxx_STARS_SRAM_BASE + 0x6C) /* SRAM ECC错误寄存器 */
#define xxx_STARS_SRAM_SRAM_CLR_STAT_REG   (xxx_STARS_SRAM_BASE + 0x70) /* SRAM状态清除寄存器 */
#define xxx_STARS_SRAM_SRAM_ECC_CNT_REG    (xxx_STARS_SRAM_BASE + 0x80) /* SRAM ECC错误计数器 */
#define xxx_STARS_SRAM_SRAM_CNT_TYPE_REG   (xxx_STARS_SRAM_BASE + 0x84) /* SRAM DFX计数器类型控制寄存器 */
#define xxx_STARS_SRAM_SRAM_CNT0_REG       (xxx_STARS_SRAM_BASE + 0x88) /* SRAM DFX计数器0 */
#define xxx_STARS_SRAM_SRAM_CNT1_REG       (xxx_STARS_SRAM_BASE + 0x8C) /* SRAM DFX计数器1 */
#define xxx_STARS_SRAM_SRAM_CNT2_REG       (xxx_STARS_SRAM_BASE + 0x90) /* SRAM DFX计数器2 */
#define xxx_STARS_SRAM_SRAM_CNT3_REG       (xxx_STARS_SRAM_BASE + 0x94) /* SRAM DFX计数器3 */

#endif // __STARS_SRAM_REG_OFFSET_H__
