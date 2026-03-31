/*
 * Copyright     :  Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
 * File name     :  mata_reg_offset.h
 * Project line  :  Platform And crux Technologies Development
 * Department    :  CAD Development Department
 * Author        :  xxx
 * Version       :  1.0
 * Date          :  2022/11/11
 * Description   :  The description of xxx project
 * Others        :  Generated automatically by nManager V5.1
 * History       :  xxx 2022/11/11 10:39:30 Create file
 */

#ifndef __MATA_REG_OFFSET_H__
#define __MATA_REG_OFFSET_H__

/* MATA Base address of Module's Register */
#define MATA_BASE                       (0x0)

/******************************************************************************/
/*                      P688 MATA Registers' Definitions                            */
/******************************************************************************/
#define MATA_MATA_PERF_ENABLE_REG        (MATA_BASE + 0x1E00) /* MATA统计事件全局使能寄存器 */
#define MATA_MATA_EVENT_CTRL_REG         (MATA_BASE + 0x1E04) /* MATA的事件统计控制寄存器 */
#define MATA_MATA_EVENT_TYPE0_REG        (MATA_BASE + 0x1E80) /* MATA_EVENT_TYPE0为MATA的事件统计类型配置寄存器 */
#define MATA_MATA_EVENT_TYPE1_REG        (MATA_BASE + 0x1E84) /* MATA_EVENT_TYPE1为MATA的事件统计类型配置寄存器 */
#define MATA_MATA_EVENT_CNT0L_REG        (MATA_BASE + 0x1F00) /* MATA_EVENT_CNT0L为事件统计寄存器0的低位部分 */
#define MATA_MATA_EVENT_CNT0H_REG        (MATA_BASE + 0x1F04) /* MATA_EVENT_CNT0H为事件统计寄存器0的高位部分 */
#define MATA_MATA_EVENT_CNT1L_REG        (MATA_BASE + 0x1F08) /* MATA_EVENT_CNT1L为事件统计寄存器1 */
#define MATA_MATA_EVENT_CNT1H_REG        (MATA_BASE + 0x1F0C) /* MATA_EVENT_CNT1H为事件统计寄存器1的高位部分 */
#define MATA_MATA_EVENT_CNT2L_REG        (MATA_BASE + 0x1F10) /* MATA_EVENT_CNT2L为事件统计寄存器2的低位部分 */
#define MATA_MATA_EVENT_CNT2H_REG        (MATA_BASE + 0x1F14) /* MATA_EVENT_CNT2H为事件统计寄存器2的高位部分 */
#define MATA_MATA_EVENT_CNT3L_REG        (MATA_BASE + 0x1F18) /* MATA_EVENT_CNT3L为事件统计寄存器3的低位部分 */
#define MATA_MATA_EVENT_CNT3H_REG        (MATA_BASE + 0x1F1C) /* MATA_EVENT_CNT3H为事件统计寄存器3的高位部分 */
#define MATA_MATA_EVENT_CNT4L_REG        (MATA_BASE + 0x1F20) /* MATA_EVENT_CNT4为事件统计寄存器4的低位部分 */
#define MATA_MATA_EVENT_CNT4H_REG        (MATA_BASE + 0x1F24) /* MATA_EVENT_CNT4H为事件统计寄存器4的高位部分 */
#define MATA_MATA_EVENT_CNT5L_REG        (MATA_BASE + 0x1F28) /* MATA_EVENT_CNT5L为事件统计寄存器5的低位部分 */
#define MATA_MATA_EVENT_CNT5H_REG        (MATA_BASE + 0x1F2C) /* MATA_EVENT_CNT5H为事件统计寄存器5的高位部分 */
#define MATA_MATA_EVENT_CNT6L_REG        (MATA_BASE + 0x1F30) /* MATA_EVENT_CNT6L为事件统计寄存器6的低位部分 */
#define MATA_MATA_EVENT_CNT6H_REG        (MATA_BASE + 0x1F34) /* MATA_EVENT_CNT6H为事件统计寄存器6的高位部分 */
#define MATA_MATA_EVENT_CNT7L_REG        (MATA_BASE + 0x1F38) /* MATA_EVENT_CNT7为事件统计寄存器7的低位部分 */
#define MATA_MATA_EVENT_CNT7H_REG        (MATA_BASE + 0x1F3C) /* MATA_EVENT_CNT7H为事件统计寄存器7的高位部分 */

#endif // __MATA_REG_OFFSET_H__
