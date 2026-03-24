/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: rtc_common_reg_offset
 * Author: huawei
 * Create: 2021-12-28
 */

#ifndef __RTC_COMMON_REG_OFFSET_H__
#define __RTC_COMMON_REG_OFFSET_H__

#include "platform_mac.h"

/* RTC_COMMON Base address of Module's Register */
#define RTC_COMMON_BASE                       (HCLGE_MAG_REG_BASE + 0x2000)

/******************************************************************************/
/*                      RTC_COMMON Registers' Definitions                            */
/******************************************************************************/

#define RTC_COMMON_RTC_PPS_INT_ENABLE_REG     (RTC_COMMON_BASE + 0x0)   /* RTC秒脉冲中断使能寄存器 */
#define RTC_COMMON_RTC_PPS_INT_STATUS_REG     (RTC_COMMON_BASE + 0x20)  /* RTC秒脉冲中断状态寄存器 */
#define RTC_COMMON_RTC_PPS_INT_SET_REG        (RTC_COMMON_BASE + 0x40)  /* RTC秒脉冲中断注入寄存器 */
#define RTC_COMMON_RTC_PPS_INT_SOURCE_REG     (RTC_COMMON_BASE + 0x60)  /* RTC秒脉冲中断原始中断寄存器 */
#define RTC_COMMON_RTC_PPS_INT_RPT_TYPE_REG   (RTC_COMMON_BASE + 0x80)  /* RTC秒脉冲中断上报类型寄存器 */
#define RTC_COMMON_RTC_PPS_INT_CE_STATUS_REG  (RTC_COMMON_BASE + 0x90)  /* RTC秒脉冲CE中断状态寄存器 */
#define RTC_COMMON_RTC_PPS_INT_NFE_STATUS_REG (RTC_COMMON_BASE + 0xA0)  /* RTC秒脉冲NFE中断状态寄存器 */
#define RTC_COMMON_RTC_PPS_INT_FE_STATUS_REG  (RTC_COMMON_BASE + 0xB0)  /* RTC秒脉冲FE中断状态寄存器 */
#define RTC_COMMON_CFG_TIME_SYNC_H_REG        (RTC_COMMON_BASE + 0x100) /* 时间同步秒时间高位配置寄存器。 */
#define RTC_COMMON_CFG_TIME_SYNC_M_REG        (RTC_COMMON_BASE + 0x104) /* 时间同步秒时间低位配置寄存器。 */
#define RTC_COMMON_CFG_TIME_SYNC_L_REG        (RTC_COMMON_BASE + 0x108) /* 时间同步纳秒时间配置寄存器。 */
#define RTC_COMMON_CFG_TIME_SYNC_RDY_REG      (RTC_COMMON_BASE + 0x10C) /* 时间同步使能配置寄存器。 */
#define RTC_COMMON_CFG_PTP_OFFSET_ADD_RDY_REG (RTC_COMMON_BASE + 0x110) /* 时间同步偏移使能配置寄存器。 */
#define RTC_COMMON_CFG_TIME_CYC_NS_REG        (RTC_COMMON_BASE + 0x114) /* RTC计时时钟周期时间纳秒整数值配置寄存器。 */
#define RTC_COMMON_CFG_TIME_CYC_NS_DEC0_REG   (RTC_COMMON_BASE + 0x118) /* RTC计时时钟周期时间纳秒小数值配置寄存器。 */
#define RTC_COMMON_CFG_TIME_CYC_NS_DEC1_REG   (RTC_COMMON_BASE + 0x11C) /* RTC计时时钟周期时间纳秒小数值配置寄存器。 */
#define RTC_COMMON_CFG_TIME_CYC_EN_REG        (RTC_COMMON_BASE + 0x120) /* RTC计时时钟周期配置有效使能 */
#define RTC_COMMON_CURR_TIME_OUT_H_REG        (RTC_COMMON_BASE + 0x124) /* 本地时间秒高位输出寄存器。 */
#define RTC_COMMON_CURR_TIME_OUT_L_REG        (RTC_COMMON_BASE + 0x128) /* 本地时间秒低位输出寄存器。 */
#define RTC_COMMON_CURR_TIME_OUT_NS_REG       (RTC_COMMON_BASE + 0x12C) /* 本地时间纳秒输出寄存器。 */
#define RTC_COMMON_CFG_PPS_WIDTH_REG          (RTC_COMMON_BASE + 0x130) /* 1588主从时间同步测试PPS信号脉冲宽度 */
#define RTC_COMMON_CFG_PPS_PERIOD_REG         (RTC_COMMON_BASE + 0x134) /* 1588主从时间同步测试PPS脉冲周期 */
#define RTC_COMMON_CFG_PPS_INVERT_REG         (RTC_COMMON_BASE + 0x138) /* 1588主从时间同步测试PPS脉冲极性设置 */
#define RTC_COMMON_CFG_RTC_PPE_REG            (RTC_COMMON_BASE + 0x13C) /* RTC同步配置寄存器 */

#endif // __RTC_COMMON_REG_OFFSET_H__
