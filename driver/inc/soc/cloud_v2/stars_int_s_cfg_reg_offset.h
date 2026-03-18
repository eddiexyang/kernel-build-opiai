// ******************************************************************************
// Copyright     :  Copyright (C) 2021, Hisilicon Technologies Co. Ltd.
// File name     :  stars_int_s_cfg_reg_offset.h
// Project line  :  Platform And Key Technologies Development
// Department    :  CAD Development Department
// Author        :  xxx
// Version       :  1
// Date          :  2020/04/01
// Description   :  The description of xxx project
// Others        :  Generated automatically by nManager V4.2
// History       :  xxx 2021/09/15 10:34:16 Create file
// ******************************************************************************

#ifndef __STARS_INT_S_CFG_REG_OFFSET_H__
#define __STARS_INT_S_CFG_REG_OFFSET_H__

/* STARS_INT_S_CFG Base address of Module's Register */
#define SOC_STARS_INT_S_CFG_BASE                       (0x4008000)

/******************************************************************************/
/*                      SOC STARS_INT_S_CFG Registers' Definitions                            */
/******************************************************************************/

#define SOC_STARS_INT_S_CFG_STARS_INT_HTIC_AXCACHE_SETTING_REG    (SOC_STARS_INT_S_CFG_BASE + 0x800)  /* AXI CACHE配置寄存器 */
#define SOC_STARS_INT_S_CFG_STARS_INT_HTIC_NS_AXPROT_SETTING_REG  (SOC_STARS_INT_S_CFG_BASE + 0x804)  /* AXI PROT 非安全配置寄存器 */
#define SOC_STARS_INT_S_CFG_STARS_INT_HTIC_ADDR_L_REG             (SOC_STARS_INT_S_CFG_BASE + 0x808)  /* 上报HOST中断地址低bit */
#define SOC_STARS_INT_S_CFG_STARS_INT_HTIC_ADDR_H_REG             (SOC_STARS_INT_S_CFG_BASE + 0x80C)  /* 上报HOST中断地址高bit */
#define SOC_STARS_INT_S_CFG_STARS_INT_HTIC_ADDR_IS_VIRTUAL_NS_REG (SOC_STARS_INT_S_CFG_BASE + 0x810)  /* 上报HOST中断地址是否为虚拟地址 */
#define SOC_STARS_INT_S_CFG_STARS_INT_CONFIG_ERR_FORCE_S_REG      (SOC_STARS_INT_S_CFG_BASE + 0x2000) /* pool_conflict和ecc_error中断注错寄存器 */
#define SOC_STARS_INT_S_CFG_STARS_INT_CONFIG_ERR_RAW_S_REG        (SOC_STARS_INT_S_CFG_BASE + 0x2004) /* pool_conflict和ecc_error原始中断状态寄存器 */
#define SOC_STARS_INT_S_CFG_STARS_INT_CONFIG_ERR_STS0_S_REG       (SOC_STARS_INT_S_CFG_BASE + 0x2008) /* pool_conflict和ecc_error屏蔽后中断状态寄存器，上报至TSCPU */
#define SOC_STARS_INT_S_CFG_STARS_INT_CONFIG_ERR_MASK0_S_REG      (SOC_STARS_INT_S_CFG_BASE + 0x200C) /* pool_conflict和ecc_error中断屏蔽寄存器，上报至TSCPU */
#define SOC_STARS_INT_S_CFG_STARS_INT_CONFIG_ERR_CLR0_S_REG       (SOC_STARS_INT_S_CFG_BASE + 0x2010) /* pool_conflict和ecc_error中断清除寄存器 */
#define SOC_STARS_INT_S_CFG_STARS_INT_CONFIG_ERR_STS1_S_REG       (SOC_STARS_INT_S_CFG_BASE + 0x2014) /* pool_conflict和ecc_error屏蔽后中断状态寄存器，上报至ControlCPU */
#define SOC_STARS_INT_S_CFG_STARS_INT_CONFIG_ERR_MASK1_S_REG      (SOC_STARS_INT_S_CFG_BASE + 0x2018) /* pool_conflict和ecc_error中断屏蔽寄存器，上报至ControlCPU */
#define SOC_STARS_INT_S_CFG_STARS_INT_CONFIG_ERR_CLR1_S_REG       (SOC_STARS_INT_S_CFG_BASE + 0x201C) /* pool_conflict和ecc_error中断清除寄存器 */
#define SOC_STARS_INT_S_CFG_STARS_INT_ECC_FORCE_S_REG             (SOC_STARS_INT_S_CFG_BASE + 0x2030) /* pool_conflict和ecc_error中断注错寄存器 */
#define SOC_STARS_INT_S_CFG_STARS_INT_ECC_RAW_S_REG               (SOC_STARS_INT_S_CFG_BASE + 0x2034) /* pool_conflict和ecc_error原始中断状态寄存器 */
#define SOC_STARS_INT_S_CFG_STARS_INT_ECC_STS0_S_REG              (SOC_STARS_INT_S_CFG_BASE + 0x2038) /* pool_conflict和ecc_error屏蔽后中断状态寄存器，上报至TSCPU */
#define SOC_STARS_INT_S_CFG_STARS_INT_ECC_MASK0_S_REG             (SOC_STARS_INT_S_CFG_BASE + 0x203C) /* pool_conflict和ecc_error中断屏蔽寄存器，上报至TSCPU */
#define SOC_STARS_INT_S_CFG_STARS_INT_ECC_CLR0_S_REG              (SOC_STARS_INT_S_CFG_BASE + 0x2040) /* pool_conflict和ecc_error中断清除寄存器 */
#define SOC_STARS_INT_S_CFG_STARS_INT_ECC_STS1_S_REG              (SOC_STARS_INT_S_CFG_BASE + 0x2044) /* pool_conflict和ecc_error屏蔽后中断状态寄存器，上报至ControlCPU */
#define SOC_STARS_INT_S_CFG_STARS_INT_ECC_MASK1_S_REG             (SOC_STARS_INT_S_CFG_BASE + 0x2048) /* pool_conflict和ecc_error中断屏蔽寄存器，上报至ControlCPU */
#define SOC_STARS_INT_S_CFG_STARS_INT_ECC_CLR1_S_REG              (SOC_STARS_INT_S_CFG_BASE + 0x204C) /* pool_conflict和ecc_error中断清除寄存器 */
#define SOC_STARS_INT_S_CFG_STARS_INT_BUS0_ERROR_MASK0_S_REG      (SOC_STARS_INT_S_CFG_BASE + 0x2060) /* NOC_ARB0的bus erro中断屏蔽寄存器，上报至TSCPU */
#define SOC_STARS_INT_S_CFG_STARS_INT_BUS0_ERROR_MASK1_S_REG      (SOC_STARS_INT_S_CFG_BASE + 0x2064) /* NOC_ARB0的bus erro中断屏蔽寄存器，上报至ControlCPU */
#define SOC_STARS_INT_S_CFG_STARS_INT_BUS0_ERROR_STS0_S_REG       (SOC_STARS_INT_S_CFG_BASE + 0x206C) /* NOC_ARB0的bus erro中断状态寄存器，上报至TSCPU */
#define SOC_STARS_INT_S_CFG_STARS_INT_BUS0_ERROR_STS1_S_REG       (SOC_STARS_INT_S_CFG_BASE + 0x2070) /* NOC_ARB0的bus erro中断状态寄存器，上报至ControlCPU */
#define SOC_STARS_INT_S_CFG_STARS_INT_BUS0_ERROR_CLR0_S_REG       (SOC_STARS_INT_S_CFG_BASE + 0x2074) /* NOC_ARB0的bus erro中断清除寄存器，上报至TSCPU */
#define SOC_STARS_INT_S_CFG_STARS_INT_BUS0_ERROR_CLR1_S_REG       (SOC_STARS_INT_S_CFG_BASE + 0x2078) /* NOC_ARB0的bus erro中断清除寄存器，上报至ControlCPU */
#define SOC_STARS_INT_S_CFG_STARS_INT_BUS0_ERROR_RAW0_S_REG       (SOC_STARS_INT_S_CFG_BASE + 0x207C) /* NOC_ARB0的bus erro中断原始寄存器，上报至TSCPU */
#define SOC_STARS_INT_S_CFG_STARS_INT_BUS0_ERROR_RAW1_S_REG       (SOC_STARS_INT_S_CFG_BASE + 0x2080) /* NOC_ARB0的bus erro中断原始寄存器，上报至ControlCPU */
#define SOC_STARS_INT_S_CFG_STARS_INT_BUS0_ERROR_FORCE_S_REG      (SOC_STARS_INT_S_CFG_BASE + 0x2084) /* NOC_ARB0的bus erro中断原始寄存器，上报至ControlCPU */

#endif // __STARS_INT_S_CFG_REG_OFFSET_H__
