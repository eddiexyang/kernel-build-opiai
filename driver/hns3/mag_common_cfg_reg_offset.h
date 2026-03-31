/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: mag_common_cfg_reg_offset
 * Author: huawei
 * Create: 2021-12-28
 */

#ifndef __MAG_COMMON_CFG_REG_OFFSET_H__
#define __MAG_COMMON_CFG_REG_OFFSET_H__

#include "platform_mac.h"

/* MAG_COMMON_CFG Base address of Module's Register */
#define MAG_COMMON_CFG_BASE                       (HCLGE_MAG_REG_BASE + 0x44000)

/******************************************************************************/
/*                      MAG_COMMON_CFG Registers' Definitions                            */
/******************************************************************************/

#define MAG_COMMON_CFG_MAC_HSS_INVERT_0_REG          (MAG_COMMON_CFG_BASE + 0x600) /* MAC出Serdes的数据极性设置 */
#define MAG_COMMON_CFG_MAC_HSS_INVERT_1_REG          (MAG_COMMON_CFG_BASE + 0x604) /* MAC出Serdes的数据极性设置 */
#define MAG_COMMON_CFG_MAC_HSS_INVERT_2_REG          (MAG_COMMON_CFG_BASE + 0x608) /* MAC出Serdes的数据极性设置 */
#define MAG_COMMON_CFG_MAC_HSS_INVERT_3_REG          (MAG_COMMON_CFG_BASE + 0x60C) /* MAC出Serdes的数据极性设置 */
#define MAG_COMMON_CFG_MAC_SERDES_LOS_MASK_0_REG     (MAG_COMMON_CFG_BASE + 0x700) /* 将serdes的los屏蔽寄存器 */
#define MAG_COMMON_CFG_MAC_SERDES_LOS_MASK_1_REG     (MAG_COMMON_CFG_BASE + 0x704) /* 将serdes的los屏蔽寄存器 */
#define MAG_COMMON_CFG_MAC_SERDES_LOS_MASK_2_REG     (MAG_COMMON_CFG_BASE + 0x708) /* 将serdes的los屏蔽寄存器 */
#define MAG_COMMON_CFG_MAC_SERDES_LOS_MASK_3_REG     (MAG_COMMON_CFG_BASE + 0x70C) /* 将serdes的los屏蔽寄存器 */
#define MAG_COMMON_CFG_MAC_COEFUP_TIMER_REG          (MAG_COMMON_CFG_BASE + 0x800) /* Hilink 寄存器访问1us时长配置 */
#define MAG_COMMON_CFG_MAC_LED_MOD_REG               (MAG_COMMON_CFG_BASE + 0x804) /* LED_MOD为并行点灯模式控制寄存器。 */
#define MAG_COMMON_CFG_MAC_PAR_LED_SEL_REG           (MAG_COMMON_CFG_BASE + 0x808) /* 网口并行点灯选择配置 */
#define MAG_COMMON_CFG_MAC_MUX_FIFO_AF_THRESHOLD_REG (MAG_COMMON_CFG_BASE + 0x810) /* MAC端口汇聚FIFO的反压水线 */
#define MAG_COMMON_CFG_MAC_TP_MEM_CTRL_REG           (MAG_COMMON_CFG_BASE + 0x814) /* TP MEM控制寄存器 */
#define MAG_COMMON_CFG_MAC_SP_MEM_CTRL_REG           (MAG_COMMON_CFG_BASE + 0x818) /* SP MEM控制寄存器 */
#define MAG_COMMON_CFG_MAC_MEM_POWER_MODE_REG        (MAG_COMMON_CFG_BASE + 0x81C) /* ram power mode控制寄存器 */
#define MAG_COMMON_CFG_RTC_PPS_INT_MUX_REG           (MAG_COMMON_CFG_BASE + 0x820) /* RTC PPS脉冲输出选路配置寄存器 */
#define MAG_COMMON_CFG_RTC_PTP_MUX_0_REG             (MAG_COMMON_CFG_BASE + 0x830) /* RTC端口时戳选路配置寄存器 */
#define MAG_COMMON_CFG_RTC_PTP_MUX_1_REG             (MAG_COMMON_CFG_BASE + 0x834) /* RTC端口时戳选路配置寄存器 */
#define MAG_COMMON_CFG_RTC_PTP_MUX_2_REG             (MAG_COMMON_CFG_BASE + 0x838) /* RTC端口时戳选路配置寄存器 */
#define MAG_COMMON_CFG_RTC_PTP_MUX_3_REG             (MAG_COMMON_CFG_BASE + 0x83C) /* RTC端口时戳选路配置寄存器 */
#define MAG_COMMON_CFG_RGMII_INTF_MODE_0_REG         (MAG_COMMON_CFG_BASE + 0x840) /* RGMII接口模式配置寄存器 */
#define MAG_COMMON_CFG_RGMII_INTF_MODE_1_REG         (MAG_COMMON_CFG_BASE + 0x844) /* RGMII接口模式配置寄存器 */
#define MAG_COMMON_CFG_RGMII_INTF_DELAY_0_REG        (MAG_COMMON_CFG_BASE + 0x850) /* RGMII接口DELAY配置寄存器 */
#define MAG_COMMON_CFG_RGMII_INTF_DELAY_1_REG        (MAG_COMMON_CFG_BASE + 0x854) /* RGMII接口DELAY配置寄存器 */
#define MAG_COMMON_CFG_RGMII_INTF_RESET_0_REG        (MAG_COMMON_CFG_BASE + 0x860) /* RGMII接口复位配置寄存器 */
#define MAG_COMMON_CFG_RGMII_INTF_RESET_1_REG        (MAG_COMMON_CFG_BASE + 0x864) /* RGMII接口复位配置寄存器 */
#define MAG_COMMON_CFG_RGMII_INTF_MUX_0_REG          (MAG_COMMON_CFG_BASE + 0x870) /* RGMII接口选路配置寄存器 */
#define MAG_COMMON_CFG_RGMII_INTF_MUX_1_REG          (MAG_COMMON_CFG_BASE + 0x874) /* RGMII接口选路配置寄存器 */
#define MAG_COMMON_CFG_MAC_LINK_SYNC_STATUS_REG      (MAG_COMMON_CFG_BASE + 0x890) /* MAC链路同步状态 */
#define MAG_COMMON_CFG_MAC_HILINK_LOS_STATUS_REG     (MAG_COMMON_CFG_BASE + 0x894) /* 接收到Hilink链路Los状态 */
#define MAG_COMMON_CFG_MAC_FIFO_FULL_HIS_REG         (MAG_COMMON_CFG_BASE + 0x8A4) /* MAC端口汇聚FIFO的满历史状态 */
#define MAG_COMMON_CFG_MAC_LINK_SYNC_HIS_REG         (MAG_COMMON_CFG_BASE + 0x8A8) /* MAC链路同步历史状态 */

#endif // __MAG_COMMON_CFG_REG_OFFSET_H__
