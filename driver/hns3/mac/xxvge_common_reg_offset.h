/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: xxvge_common_reg_offset
 * Author: huawei
 * Create: 2021-12-28
 */

#ifndef __XXVGE_COMMON_REG_OFFSET_H__
#define __XXVGE_COMMON_REG_OFFSET_H__

#include "platform_mac.h"

/* XXVGE_COMMON Base address of Module's Register */
#define XXVGE_COMMON_BASE                       (HCLGE_MAG_REG_BASE + 0x40000)

/******************************************************************************/
/*                      XXVGE_COMMON Registers' Definitions                            */
/******************************************************************************/

#define XXVGE_COMMON_AFIFO_TNL_INT_ENABLE_REG        (XXVGE_COMMON_BASE + 0x0)   /* 异步FIFO通道异常中断使能寄存器 */
#define XXVGE_COMMON_AFIFO_TNL_INT_STATUS_REG        (XXVGE_COMMON_BASE + 0x20)  /* 异步FIFO通道异常中断状态寄存器 */
#define XXVGE_COMMON_AFIFO_TNL_INT_SET_REG           (XXVGE_COMMON_BASE + 0x40)  /* 异步FIFO通道异常中断注入寄存器 */
#define XXVGE_COMMON_AFIFO_TNL_INT_SOURCE_REG        (XXVGE_COMMON_BASE + 0x60)  /* 异步FIFO通道异常中断原始中断寄存器 */
#define XXVGE_COMMON_AFIFO_TNL_INT_RPT_TYPE_REG      (XXVGE_COMMON_BASE + 0x80)  /* 异步FIFO通道异常中断上报类型寄存器 */
#define XXVGE_COMMON_AFIFO_TNL_INT_CE_STATUS_REG     (XXVGE_COMMON_BASE + 0x90)  /* 异步FIFO通道异常CE中断状态寄存器 */
#define XXVGE_COMMON_AFIFO_TNL_INT_NFE_STATUS_REG    (XXVGE_COMMON_BASE + 0xA0)  /* 异步FIFO通道异常NFE中断状态寄存器 */
#define XXVGE_COMMON_AFIFO_TNL_INT_FE_STATUS_REG     (XXVGE_COMMON_BASE + 0xB0)  /* 异步FIFO通道异常FE中断状态寄存器 */
#define XXVGE_COMMON_MAC_CFGIF_RSTART_REG            (XXVGE_COMMON_BASE + 0x100) /* T接口访问桥复位 */
#define XXVGE_COMMON_MAC_RD_CLEAR_REG                (XXVGE_COMMON_BASE + 0x104) /* MAC/AN_TRAIN统计寄存器读清控制 */
#define XXVGE_COMMON_MAC_CLK_GATE_ENABLE_REG         (XXVGE_COMMON_BASE + 0x108) /* HiMAC时钟使能 */
#define XXVGE_COMMON_MAC_ASYNC_FIFO_AE_THRESHOLD_REG (XXVGE_COMMON_BASE + 0x208) /* MAC 异步 FIFO 水线 */
#define XXVGE_COMMON_MAC_REG_TIMEOUT_THRESHOLD_REG   (XXVGE_COMMON_BASE + 0x20C) /* HiMAC寄存器访问超时时间配置寄存器 */
#define XXVGE_COMMON_LGE_IGU_AFIFO_STATUS_REG        (XXVGE_COMMON_BASE + 0x260) /* MAC与IGU之间的异步FIFO状态 */
#define XXVGE_COMMON_LGE_EGU_AFIFO_STATUS_REG        (XXVGE_COMMON_BASE + 0x264) /* MAC与EGU之间的异步FIFO状态 */
#define XXVGE_COMMON_MAC_REG_ACS_TIMEOUT_STATUS_REG  (XXVGE_COMMON_BASE + 0x268) /* MAC XXVGE寄存器访问超时状态 */

#endif // __XXVGE_COMMON_REG_OFFSET_H__
