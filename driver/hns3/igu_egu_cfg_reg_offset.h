/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: igu_egu_cfg_reg_offset
 * Author: huawei
 * Create: 2021-12-28
 */

#ifndef __IGU_EGU_CFG_REG_OFFSET_H__
#define __IGU_EGU_CFG_REG_OFFSET_H__

#include "platform_mac.h"

/* IGU_EGU_CFG Base address of Module's Register */
#define IGU_EGU_CFG_BASE                       (HCLGE_MAG_REG_BASE + 0x0)

/******************************************************************************/
/*                      IGU_EGU_CFG Registers' Definitions                            */
/******************************************************************************/

#define IGU_EGU_CFG_IGU_EGU_TNL_INT_ENABLE_0_REG        (IGU_EGU_CFG_BASE + 0x0)    /* IGU和EGU MAC通道异常中断使能寄存器 */
#define IGU_EGU_CFG_IGU_EGU_TNL_INT_ENABLE_1_REG        (IGU_EGU_CFG_BASE + 0x100)  /* IGU和EGU MAC通道异常中断使能寄存器 */
#define IGU_EGU_CFG_IGU_EGU_TNL_INT_ENABLE_2_REG        (IGU_EGU_CFG_BASE + 0x200)  /* IGU和EGU MAC通道异常中断使能寄存器 */
#define IGU_EGU_CFG_IGU_EGU_TNL_INT_ENABLE_3_REG        (IGU_EGU_CFG_BASE + 0x300)  /* IGU和EGU MAC通道异常中断使能寄存器 */
#define IGU_EGU_CFG_IGU_EGU_TNL_INT_STATUS_0_REG        (IGU_EGU_CFG_BASE + 0x20)   /* IGU和EGU MAC通道异常中断状态寄存器 */
#define IGU_EGU_CFG_IGU_EGU_TNL_INT_STATUS_1_REG        (IGU_EGU_CFG_BASE + 0x120)  /* IGU和EGU MAC通道异常中断状态寄存器 */
#define IGU_EGU_CFG_IGU_EGU_TNL_INT_STATUS_2_REG        (IGU_EGU_CFG_BASE + 0x220)  /* IGU和EGU MAC通道异常中断状态寄存器 */
#define IGU_EGU_CFG_IGU_EGU_TNL_INT_STATUS_3_REG        (IGU_EGU_CFG_BASE + 0x320)  /* IGU和EGU MAC通道异常中断状态寄存器 */
#define IGU_EGU_CFG_IGU_EGU_TNL_INT_SET_0_REG           (IGU_EGU_CFG_BASE + 0x40)   /* IGU和EGU MAC通道异常原始中断注入 */
#define IGU_EGU_CFG_IGU_EGU_TNL_INT_SET_1_REG           (IGU_EGU_CFG_BASE + 0x140)  /* IGU和EGU MAC通道异常原始中断注入 */
#define IGU_EGU_CFG_IGU_EGU_TNL_INT_SET_2_REG           (IGU_EGU_CFG_BASE + 0x240)  /* IGU和EGU MAC通道异常原始中断注入 */
#define IGU_EGU_CFG_IGU_EGU_TNL_INT_SET_3_REG           (IGU_EGU_CFG_BASE + 0x340)  /* IGU和EGU MAC通道异常原始中断注入 */
#define IGU_EGU_CFG_IGU_EGU_TNL_INT_SOURCE_0_REG        (IGU_EGU_CFG_BASE + 0x60)   /* IGU和EGU MAC通道异常原始中断状态寄存器 */
#define IGU_EGU_CFG_IGU_EGU_TNL_INT_SOURCE_1_REG        (IGU_EGU_CFG_BASE + 0x160)  /* IGU和EGU MAC通道异常原始中断状态寄存器 */
#define IGU_EGU_CFG_IGU_EGU_TNL_INT_SOURCE_2_REG        (IGU_EGU_CFG_BASE + 0x260)  /* IGU和EGU MAC通道异常原始中断状态寄存器 */
#define IGU_EGU_CFG_IGU_EGU_TNL_INT_SOURCE_3_REG        (IGU_EGU_CFG_BASE + 0x360)  /* IGU和EGU MAC通道异常原始中断状态寄存器 */
#define IGU_EGU_CFG_IGU_EGU_TNL_INT_RPT_TYPE_0_REG      (IGU_EGU_CFG_BASE + 0x80)   /* IGU和EGU MAC通道中断上报类型寄存器 */
#define IGU_EGU_CFG_IGU_EGU_TNL_INT_RPT_TYPE_1_REG      (IGU_EGU_CFG_BASE + 0x180)  /* IGU和EGU MAC通道中断上报类型寄存器 */
#define IGU_EGU_CFG_IGU_EGU_TNL_INT_RPT_TYPE_2_REG      (IGU_EGU_CFG_BASE + 0x280)  /* IGU和EGU MAC通道中断上报类型寄存器 */
#define IGU_EGU_CFG_IGU_EGU_TNL_INT_RPT_TYPE_3_REG      (IGU_EGU_CFG_BASE + 0x380)  /* IGU和EGU MAC通道中断上报类型寄存器 */
#define IGU_EGU_CFG_IGU_EGU_TNL_INT_CE_STATUS_0_REG     (IGU_EGU_CFG_BASE + 0x90)   /* IGU和EGU MAC通道异常CE中断状态寄存器 */
#define IGU_EGU_CFG_IGU_EGU_TNL_INT_CE_STATUS_1_REG     (IGU_EGU_CFG_BASE + 0x190)  /* IGU和EGU MAC通道异常CE中断状态寄存器 */
#define IGU_EGU_CFG_IGU_EGU_TNL_INT_CE_STATUS_2_REG     (IGU_EGU_CFG_BASE + 0x290)  /* IGU和EGU MAC通道异常CE中断状态寄存器 */
#define IGU_EGU_CFG_IGU_EGU_TNL_INT_CE_STATUS_3_REG     (IGU_EGU_CFG_BASE + 0x390)  /* IGU和EGU MAC通道异常CE中断状态寄存器 */
#define IGU_EGU_CFG_IGU_EGU_TNL_INT_NFE_STATUS_0_REG    (IGU_EGU_CFG_BASE + 0xA0)   /* IGU和EGU MAC通道异常NFE中断状态寄存器 */
#define IGU_EGU_CFG_IGU_EGU_TNL_INT_NFE_STATUS_1_REG    (IGU_EGU_CFG_BASE + 0x1A0)  /* IGU和EGU MAC通道异常NFE中断状态寄存器 */
#define IGU_EGU_CFG_IGU_EGU_TNL_INT_NFE_STATUS_2_REG    (IGU_EGU_CFG_BASE + 0x2A0)  /* IGU和EGU MAC通道异常NFE中断状态寄存器 */
#define IGU_EGU_CFG_IGU_EGU_TNL_INT_NFE_STATUS_3_REG    (IGU_EGU_CFG_BASE + 0x3A0)  /* IGU和EGU MAC通道异常NFE中断状态寄存器 */
#define IGU_EGU_CFG_IGU_EGU_TNL_INT_FE_STATUS_0_REG     (IGU_EGU_CFG_BASE + 0xB0)   /* IGU和EGU MAC通道异常FE中断状态寄存器 */
#define IGU_EGU_CFG_IGU_EGU_TNL_INT_FE_STATUS_1_REG     (IGU_EGU_CFG_BASE + 0x1B0)  /* IGU和EGU MAC通道异常FE中断状态寄存器 */
#define IGU_EGU_CFG_IGU_EGU_TNL_INT_FE_STATUS_2_REG     (IGU_EGU_CFG_BASE + 0x2B0)  /* IGU和EGU MAC通道异常FE中断状态寄存器 */
#define IGU_EGU_CFG_IGU_EGU_TNL_INT_FE_STATUS_3_REG     (IGU_EGU_CFG_BASE + 0x3B0)  /* IGU和EGU MAC通道异常FE中断状态寄存器 */
#define IGU_EGU_CFG_IGU_MAC_EN_CFG_0_REG                (IGU_EGU_CFG_BASE + 0x500)  /* MAC通道配置寄存器 */
#define IGU_EGU_CFG_IGU_MAC_EN_CFG_1_REG                (IGU_EGU_CFG_BASE + 0x700)  /* MAC通道配置寄存器 */
#define IGU_EGU_CFG_IGU_MAC_EN_CFG_2_REG                (IGU_EGU_CFG_BASE + 0x900)  /* MAC通道配置寄存器 */
#define IGU_EGU_CFG_IGU_MAC_EN_CFG_3_REG                (IGU_EGU_CFG_BASE + 0xB00)  /* MAC通道配置寄存器 */
#define IGU_EGU_CFG_IGU_MAC_MODE_CFG_0_REG              (IGU_EGU_CFG_BASE + 0x504)  /* MAC类型配置寄存器 */
#define IGU_EGU_CFG_IGU_MAC_MODE_CFG_1_REG              (IGU_EGU_CFG_BASE + 0x704)  /* MAC类型配置寄存器 */
#define IGU_EGU_CFG_IGU_MAC_MODE_CFG_2_REG              (IGU_EGU_CFG_BASE + 0x904)  /* MAC类型配置寄存器 */
#define IGU_EGU_CFG_IGU_MAC_MODE_CFG_3_REG              (IGU_EGU_CFG_BASE + 0xB04)  /* MAC类型配置寄存器 */
#define IGU_EGU_CFG_IGU_DROP_EN_0_REG                   (IGU_EGU_CFG_BASE + 0x508)  /* 接收丢包使能寄存器 */
#define IGU_EGU_CFG_IGU_DROP_EN_1_REG                   (IGU_EGU_CFG_BASE + 0x708)  /* 接收丢包使能寄存器 */
#define IGU_EGU_CFG_IGU_DROP_EN_2_REG                   (IGU_EGU_CFG_BASE + 0x908)  /* 接收丢包使能寄存器 */
#define IGU_EGU_CFG_IGU_DROP_EN_3_REG                   (IGU_EGU_CFG_BASE + 0xB08)  /* 接收丢包使能寄存器 */
#define IGU_EGU_CFG_IGU_EGU_LOOP_0_REG                  (IGU_EGU_CFG_BASE + 0x50C)  /* 内外环回配置 */
#define IGU_EGU_CFG_IGU_EGU_LOOP_1_REG                  (IGU_EGU_CFG_BASE + 0x70C)  /* 内外环回配置 */
#define IGU_EGU_CFG_IGU_EGU_LOOP_2_REG                  (IGU_EGU_CFG_BASE + 0x90C)  /* 内外环回配置 */
#define IGU_EGU_CFG_IGU_EGU_LOOP_3_REG                  (IGU_EGU_CFG_BASE + 0xB0C)  /* 内外环回配置 */
#define IGU_EGU_CFG_IGU_MAX_FRAME_LEN_0_REG             (IGU_EGU_CFG_BASE + 0x510)  /* IGU接收最大包长配置寄存器 */
#define IGU_EGU_CFG_IGU_MAX_FRAME_LEN_1_REG             (IGU_EGU_CFG_BASE + 0x710)  /* IGU接收最大包长配置寄存器 */
#define IGU_EGU_CFG_IGU_MAX_FRAME_LEN_2_REG             (IGU_EGU_CFG_BASE + 0x910)  /* IGU接收最大包长配置寄存器 */
#define IGU_EGU_CFG_IGU_MAX_FRAME_LEN_3_REG             (IGU_EGU_CFG_BASE + 0xB10)  /* IGU接收最大包长配置寄存器 */
#define IGU_EGU_CFG_IGU_MIN_FRAME_LEN_0_REG             (IGU_EGU_CFG_BASE + 0x514)  /* IGU接收最小包长配置寄存器 */
#define IGU_EGU_CFG_IGU_MIN_FRAME_LEN_1_REG             (IGU_EGU_CFG_BASE + 0x714)  /* IGU接收最小包长配置寄存器 */
#define IGU_EGU_CFG_IGU_MIN_FRAME_LEN_2_REG             (IGU_EGU_CFG_BASE + 0x914)  /* IGU接收最小包长配置寄存器 */
#define IGU_EGU_CFG_IGU_MIN_FRAME_LEN_3_REG             (IGU_EGU_CFG_BASE + 0xB14)  /* IGU接收最小包长配置寄存器 */
#define IGU_EGU_CFG_IGU_RX_FIFO_THRESHOLD_0_REG         (IGU_EGU_CFG_BASE + 0x520)  /* IGU FIFO水线配置 */
#define IGU_EGU_CFG_IGU_RX_FIFO_THRESHOLD_1_REG         (IGU_EGU_CFG_BASE + 0x720)  /* IGU FIFO水线配置 */
#define IGU_EGU_CFG_IGU_RX_FIFO_THRESHOLD_2_REG         (IGU_EGU_CFG_BASE + 0x920)  /* IGU FIFO水线配置 */
#define IGU_EGU_CFG_IGU_RX_FIFO_THRESHOLD_3_REG         (IGU_EGU_CFG_BASE + 0xB20)  /* IGU FIFO水线配置 */
#define IGU_EGU_CFG_EGU_TX_FIFO_THRESHOLD_0_REG         (IGU_EGU_CFG_BASE + 0x524)  /* EGU FIFO水线配置 */
#define IGU_EGU_CFG_EGU_TX_FIFO_THRESHOLD_1_REG         (IGU_EGU_CFG_BASE + 0x724)  /* EGU FIFO水线配置 */
#define IGU_EGU_CFG_EGU_TX_FIFO_THRESHOLD_2_REG         (IGU_EGU_CFG_BASE + 0x924)  /* EGU FIFO水线配置 */
#define IGU_EGU_CFG_EGU_TX_FIFO_THRESHOLD_3_REG         (IGU_EGU_CFG_BASE + 0xB24)  /* EGU FIFO水线配置 */
#define IGU_EGU_CFG_EGU_MAC_APP_LOOP_AE_THRESHOLD_0_REG (IGU_EGU_CFG_BASE + 0x528)  /* MAC app侧外环回启动发送报文水线配置 */
#define IGU_EGU_CFG_EGU_MAC_APP_LOOP_AE_THRESHOLD_1_REG (IGU_EGU_CFG_BASE + 0x728)  /* MAC app侧外环回启动发送报文水线配置 */
#define IGU_EGU_CFG_EGU_MAC_APP_LOOP_AE_THRESHOLD_2_REG (IGU_EGU_CFG_BASE + 0x928)  /* MAC app侧外环回启动发送报文水线配置 */
#define IGU_EGU_CFG_EGU_MAC_APP_LOOP_AE_THRESHOLD_3_REG (IGU_EGU_CFG_BASE + 0xB28)  /* MAC app侧外环回启动发送报文水线配置 */
#define IGU_EGU_CFG_EGU_SEND_1588_OPC_EN_0_REG          (IGU_EGU_CFG_BASE + 0x530)  /* 发送方向1588报文的opcode使能 */
#define IGU_EGU_CFG_EGU_SEND_1588_OPC_EN_1_REG          (IGU_EGU_CFG_BASE + 0x730)  /* 发送方向1588报文的opcode使能 */
#define IGU_EGU_CFG_EGU_SEND_1588_OPC_EN_2_REG          (IGU_EGU_CFG_BASE + 0x930)  /* 发送方向1588报文的opcode使能 */
#define IGU_EGU_CFG_EGU_SEND_1588_OPC_EN_3_REG          (IGU_EGU_CFG_BASE + 0xB30)  /* 发送方向1588报文的opcode使能 */
#define IGU_EGU_CFG_EGU_PTP_RETURN_TYPE_0_REG           (IGU_EGU_CFG_BASE + 0x534)  /* 发送方向1588回传时戳的类型 */
#define IGU_EGU_CFG_EGU_PTP_RETURN_TYPE_1_REG           (IGU_EGU_CFG_BASE + 0x734)  /* 发送方向1588回传时戳的类型 */
#define IGU_EGU_CFG_EGU_PTP_RETURN_TYPE_2_REG           (IGU_EGU_CFG_BASE + 0x934)  /* 发送方向1588回传时戳的类型 */
#define IGU_EGU_CFG_EGU_PTP_RETURN_TYPE_3_REG           (IGU_EGU_CFG_BASE + 0xB34)  /* 发送方向1588回传时戳的类型 */
#define IGU_EGU_CFG_IGU_PAUSE_PKT_DMAC_H_CFG_0_REG      (IGU_EGU_CFG_BASE + 0x550)  /* IGU Pause帧DMAC高16 bits配置寄存器 */
#define IGU_EGU_CFG_IGU_PAUSE_PKT_DMAC_H_CFG_1_REG      (IGU_EGU_CFG_BASE + 0x750)  /* IGU Pause帧DMAC高16 bits配置寄存器 */
#define IGU_EGU_CFG_IGU_PAUSE_PKT_DMAC_H_CFG_2_REG      (IGU_EGU_CFG_BASE + 0x950)  /* IGU Pause帧DMAC高16 bits配置寄存器 */
#define IGU_EGU_CFG_IGU_PAUSE_PKT_DMAC_H_CFG_3_REG      (IGU_EGU_CFG_BASE + 0xB50)  /* IGU Pause帧DMAC高16 bits配置寄存器 */
#define IGU_EGU_CFG_IGU_PAUSE_PKT_DMAC_L_CFG_0_REG      (IGU_EGU_CFG_BASE + 0x554)  /* IGU Pause帧DMAC低32 bits配置寄存器 */
#define IGU_EGU_CFG_IGU_PAUSE_PKT_DMAC_L_CFG_1_REG      (IGU_EGU_CFG_BASE + 0x754)  /* IGU Pause帧DMAC低32 bits配置寄存器 */
#define IGU_EGU_CFG_IGU_PAUSE_PKT_DMAC_L_CFG_2_REG      (IGU_EGU_CFG_BASE + 0x954)  /* IGU Pause帧DMAC低32 bits配置寄存器 */
#define IGU_EGU_CFG_IGU_PAUSE_PKT_DMAC_L_CFG_3_REG      (IGU_EGU_CFG_BASE + 0xB54)  /* IGU Pause帧DMAC低32 bits配置寄存器 */
#define IGU_EGU_CFG_EGU_DYINGASP_EN_0_REG               (IGU_EGU_CFG_BASE + 0x55C)  /* 发送方向响应Dyinggasp告警使能配置寄存器。 */
#define IGU_EGU_CFG_EGU_DYINGASP_EN_1_REG               (IGU_EGU_CFG_BASE + 0x75C)  /* 发送方向响应Dyinggasp告警使能配置寄存器。 */
#define IGU_EGU_CFG_EGU_DYINGASP_EN_2_REG               (IGU_EGU_CFG_BASE + 0x95C)  /* 发送方向响应Dyinggasp告警使能配置寄存器。 */
#define IGU_EGU_CFG_EGU_DYINGASP_EN_3_REG               (IGU_EGU_CFG_BASE + 0xB5C)  /* 发送方向响应Dyinggasp告警使能配置寄存器。 */
#define IGU_EGU_CFG_EGU_DYINGGASP_PKT_CFG0_REG          (IGU_EGU_CFG_BASE + 0x560)  /* Dying gasp报文配置寄存器0 */
#define IGU_EGU_CFG_EGU_DYINGGASP_PKT_CFG1_REG          (IGU_EGU_CFG_BASE + 0x564)  /* Dying gasp报文配置寄存器1 */
#define IGU_EGU_CFG_EGU_DYINGGASP_PKT_CFG2_REG          (IGU_EGU_CFG_BASE + 0x568)  /* Dying gasp报文配置寄存器2 */
#define IGU_EGU_CFG_EGU_TX_UNDERUN_THRESHOLD_REG        (IGU_EGU_CFG_BASE + 0x56C)  /* 发送断流水线设置 */
#define IGU_EGU_CFG_IGU_RX_PKT_INNER_GAP_REG            (IGU_EGU_CFG_BASE + 0x574)  /* IGU接收报文的最大帧间隔限制 */
#define IGU_EGU_CFG_IGU_CNT_CYC_CLR_CTRL_REG            (IGU_EGU_CFG_BASE + 0x578)  /* CNT_CYC类型寄存器读清使能 */
#define IGU_EGU_CFG_IGU_LEVEL1_WRR_WEIGHT_REG           (IGU_EGU_CFG_BASE + 0x57C)  /* IGU第一级调度，一组内4个端口之间WRR优先级调度权重 */
#define IGU_EGU_CFG_IGU_AUTO_GATE_EN_REG                (IGU_EGU_CFG_BASE + 0x580)  /* 动态门控使能开关 */
#define IGU_EGU_CFG_IGU_GATING_WINDOW_REG               (IGU_EGU_CFG_BASE + 0x584)  /* 统计门控打开的窗口大小，单位是64K个该模块工作时钟。 */
#define IGU_EGU_CFG_IGU_GATING_STS_REG                  \
    (IGU_EGU_CFG_BASE + 0x588)  /* 统计门控打开的窗口中打开时钟的周期，单位是64K个该模块工作时钟的。 */
#define IGU_EGU_CFG_IGU_ECO_RW_RSV0_REG                 (IGU_EGU_CFG_BASE + 0x590)  /* ECO保留寄存器0 */
#define IGU_EGU_CFG_IGU_ECO_RW_RSV1_REG                 (IGU_EGU_CFG_BASE + 0x594)  /* ECO保留寄存器1 */
#define IGU_EGU_CFG_IGU_ECO_RW_RSV2_REG                 (IGU_EGU_CFG_BASE + 0x598)  /* ECO保留寄存器2 */
#define IGU_EGU_CFG_IGU_ECO_RW_RSV3_REG                 (IGU_EGU_CFG_BASE + 0x59C)  /* ECO保留寄存器3 */
#define IGU_EGU_CFG_IGU_BUF_ECC_ERR_ADDR_0_REG          (IGU_EGU_CFG_BASE + 0x1000) /* IGU RAM ECC错误地址 */
#define IGU_EGU_CFG_IGU_BUF_ECC_ERR_ADDR_1_REG          (IGU_EGU_CFG_BASE + 0x1300) /* IGU RAM ECC错误地址 */
#define IGU_EGU_CFG_IGU_BUF_ECC_ERR_ADDR_2_REG          (IGU_EGU_CFG_BASE + 0x1600) /* IGU RAM ECC错误地址 */
#define IGU_EGU_CFG_IGU_BUF_ECC_ERR_ADDR_3_REG          (IGU_EGU_CFG_BASE + 0x1900) /* IGU RAM ECC错误地址 */
#define IGU_EGU_CFG_EGU_BUF_ECC_ERR_ADDR_0_REG          (IGU_EGU_CFG_BASE + 0x1004) /* EGU RAM ECC错误地址 */
#define IGU_EGU_CFG_EGU_BUF_ECC_ERR_ADDR_1_REG          (IGU_EGU_CFG_BASE + 0x1304) /* EGU RAM ECC错误地址 */
#define IGU_EGU_CFG_EGU_BUF_ECC_ERR_ADDR_2_REG          (IGU_EGU_CFG_BASE + 0x1604) /* EGU RAM ECC错误地址 */
#define IGU_EGU_CFG_EGU_BUF_ECC_ERR_ADDR_3_REG          (IGU_EGU_CFG_BASE + 0x1904) /* EGU RAM ECC错误地址 */
#define IGU_EGU_CFG_TX_1588_RETURN_TYPE_ERROR_0_REG     \
    (IGU_EGU_CFG_BASE + 0x1020) /* 2步法，软件填充的1588 opcode的返回时戳类型域检查出错告警 */
#define IGU_EGU_CFG_TX_1588_RETURN_TYPE_ERROR_1_REG     \
    (IGU_EGU_CFG_BASE + 0x1320) /* 2步法，软件填充的1588 opcode的返回时戳类型域检查出错告警 */
#define IGU_EGU_CFG_TX_1588_RETURN_TYPE_ERROR_2_REG     \
    (IGU_EGU_CFG_BASE + 0x1620) /* 2步法，软件填充的1588 opcode的返回时戳类型域检查出错告警 */
#define IGU_EGU_CFG_TX_1588_RETURN_TYPE_ERROR_3_REG     \
    (IGU_EGU_CFG_BASE + 0x1920) /* 2步法，软件填充的1588 opcode的返回时戳类型域检查出错告警 */
#define IGU_EGU_CFG_IGU_RX_ERR_PKT_0_REG                (IGU_EGU_CFG_BASE + 0x1050) /* IGU接收的错误报文统计计数器 */
#define IGU_EGU_CFG_IGU_RX_ERR_PKT_1_REG                (IGU_EGU_CFG_BASE + 0x1350) /* IGU接收的错误报文统计计数器 */
#define IGU_EGU_CFG_IGU_RX_ERR_PKT_2_REG                (IGU_EGU_CFG_BASE + 0x1650) /* IGU接收的错误报文统计计数器 */
#define IGU_EGU_CFG_IGU_RX_ERR_PKT_3_REG                (IGU_EGU_CFG_BASE + 0x1950) /* IGU接收的错误报文统计计数器 */
#define IGU_EGU_CFG_IGU_RX_NO_SOF_PKT_0_REG             (IGU_EGU_CFG_BASE + 0x1054) /* IGU接收的无首报文统计计算器 */
#define IGU_EGU_CFG_IGU_RX_NO_SOF_PKT_1_REG             (IGU_EGU_CFG_BASE + 0x1354) /* IGU接收的无首报文统计计算器 */
#define IGU_EGU_CFG_IGU_RX_NO_SOF_PKT_2_REG             (IGU_EGU_CFG_BASE + 0x1654) /* IGU接收的无首报文统计计算器 */
#define IGU_EGU_CFG_IGU_RX_NO_SOF_PKT_3_REG             (IGU_EGU_CFG_BASE + 0x1954) /* IGU接收的无首报文统计计算器 */
#define IGU_EGU_CFG_IGU_RX_CTRL_DROP_PKT_0_REG          (IGU_EGU_CFG_BASE + 0x1058) /* IGU过滤掉的流控统计计数器 */
#define IGU_EGU_CFG_IGU_RX_CTRL_DROP_PKT_1_REG          (IGU_EGU_CFG_BASE + 0x1358) /* IGU过滤掉的流控统计计数器 */
#define IGU_EGU_CFG_IGU_RX_CTRL_DROP_PKT_2_REG          (IGU_EGU_CFG_BASE + 0x1658) /* IGU过滤掉的流控统计计数器 */
#define IGU_EGU_CFG_IGU_RX_CTRL_DROP_PKT_3_REG          (IGU_EGU_CFG_BASE + 0x1958) /* IGU过滤掉的流控统计计数器 */
#define IGU_EGU_CFG_IGU_RX_OVERSIZE_PKT_0_REG           (IGU_EGU_CFG_BASE + 0x105C) /* IGU接收的超长报文统计计数器 */
#define IGU_EGU_CFG_IGU_RX_OVERSIZE_PKT_1_REG           (IGU_EGU_CFG_BASE + 0x135C) /* IGU接收的超长报文统计计数器 */
#define IGU_EGU_CFG_IGU_RX_OVERSIZE_PKT_2_REG           (IGU_EGU_CFG_BASE + 0x165C) /* IGU接收的超长报文统计计数器 */
#define IGU_EGU_CFG_IGU_RX_OVERSIZE_PKT_3_REG           (IGU_EGU_CFG_BASE + 0x195C) /* IGU接收的超长报文统计计数器 */
#define IGU_EGU_CFG_IGU_RX_UNDERSIZE_PKT_0_REG          (IGU_EGU_CFG_BASE + 0x1060) /* IGU接收的超短报文统计计数器 */
#define IGU_EGU_CFG_IGU_RX_UNDERSIZE_PKT_1_REG          (IGU_EGU_CFG_BASE + 0x1360) /* IGU接收的超短报文统计计数器 */
#define IGU_EGU_CFG_IGU_RX_UNDERSIZE_PKT_2_REG          (IGU_EGU_CFG_BASE + 0x1660) /* IGU接收的超短报文统计计数器 */
#define IGU_EGU_CFG_IGU_RX_UNDERSIZE_PKT_3_REG          (IGU_EGU_CFG_BASE + 0x1960) /* IGU接收的超短报文统计计数器 */
#define IGU_EGU_CFG_IGU_RX_NO_EOF_PKT_0_REG             (IGU_EGU_CFG_BASE + 0x1064) /* IGU接收的无尾报文统计计算器 */
#define IGU_EGU_CFG_IGU_RX_NO_EOF_PKT_1_REG             (IGU_EGU_CFG_BASE + 0x1364) /* IGU接收的无尾报文统计计算器 */
#define IGU_EGU_CFG_IGU_RX_NO_EOF_PKT_2_REG             (IGU_EGU_CFG_BASE + 0x1664) /* IGU接收的无尾报文统计计算器 */
#define IGU_EGU_CFG_IGU_RX_NO_EOF_PKT_3_REG             (IGU_EGU_CFG_BASE + 0x1964) /* IGU接收的无尾报文统计计算器 */
#define IGU_EGU_CFG_EGU_TX_ERR_PKT_0_REG                (IGU_EGU_CFG_BASE + 0x1068) /* EGU发送的错误报文统计计数器 */
#define IGU_EGU_CFG_EGU_TX_ERR_PKT_1_REG                (IGU_EGU_CFG_BASE + 0x1368) /* EGU发送的错误报文统计计数器 */
#define IGU_EGU_CFG_EGU_TX_ERR_PKT_2_REG                (IGU_EGU_CFG_BASE + 0x1668) /* EGU发送的错误报文统计计数器 */
#define IGU_EGU_CFG_EGU_TX_ERR_PKT_3_REG                (IGU_EGU_CFG_BASE + 0x1968) /* EGU发送的错误报文统计计数器 */
#define IGU_EGU_CFG_EGU_TX_SHORT_DROP_PKT_0_REG         (IGU_EGU_CFG_BASE + 0x106C) /* EGU丢弃的小于33byte报文统计计数器 */
#define IGU_EGU_CFG_EGU_TX_SHORT_DROP_PKT_1_REG         (IGU_EGU_CFG_BASE + 0x136C) /* EGU丢弃的小于33byte报文统计计数器 */
#define IGU_EGU_CFG_EGU_TX_SHORT_DROP_PKT_2_REG         (IGU_EGU_CFG_BASE + 0x166C) /* EGU丢弃的小于33byte报文统计计数器 */
#define IGU_EGU_CFG_EGU_TX_SHORT_DROP_PKT_3_REG         (IGU_EGU_CFG_BASE + 0x196C) /* EGU丢弃的小于33byte报文统计计数器 */
#define IGU_EGU_CFG_IGU_RX_OUT_ALL_PKT_0_REG            (IGU_EGU_CFG_BASE + 0x1100) /* IGU发送的所有报文统计计数器 */
#define IGU_EGU_CFG_IGU_RX_OUT_ALL_PKT_1_REG            (IGU_EGU_CFG_BASE + 0x1400) /* IGU发送的所有报文统计计数器 */
#define IGU_EGU_CFG_IGU_RX_OUT_ALL_PKT_2_REG            (IGU_EGU_CFG_BASE + 0x1700) /* IGU发送的所有报文统计计数器 */
#define IGU_EGU_CFG_IGU_RX_OUT_ALL_PKT_3_REG            (IGU_EGU_CFG_BASE + 0x1A00) /* IGU发送的所有报文统计计数器 */
#define IGU_EGU_CFG_EGU_TX_OUT_ALL_PKT_0_REG            (IGU_EGU_CFG_BASE + 0x1108) /* EGU发送的所有报文统计计数器 */
#define IGU_EGU_CFG_EGU_TX_OUT_ALL_PKT_1_REG            (IGU_EGU_CFG_BASE + 0x1408) /* EGU发送的所有报文统计计数器 */
#define IGU_EGU_CFG_EGU_TX_OUT_ALL_PKT_2_REG            (IGU_EGU_CFG_BASE + 0x1708) /* EGU发送的所有报文统计计数器 */
#define IGU_EGU_CFG_EGU_TX_OUT_ALL_PKT_3_REG            (IGU_EGU_CFG_BASE + 0x1A08) /* EGU发送的所有报文统计计数器 */
#define IGU_EGU_CFG_IGU_RX_STP_FIFO_CNT_0_REG           (IGU_EGU_CFG_BASE + 0x1110) /* IGU 接收时戳缓存中存储的时间戳的个数 */
#define IGU_EGU_CFG_IGU_RX_STP_FIFO_CNT_1_REG           (IGU_EGU_CFG_BASE + 0x1410) /* IGU 接收时戳缓存中存储的时间戳的个数 */
#define IGU_EGU_CFG_IGU_RX_STP_FIFO_CNT_2_REG           (IGU_EGU_CFG_BASE + 0x1710) /* IGU 接收时戳缓存中存储的时间戳的个数 */
#define IGU_EGU_CFG_IGU_RX_STP_FIFO_CNT_3_REG           (IGU_EGU_CFG_BASE + 0x1A10) /* IGU 接收时戳缓存中存储的时间戳的个数 */
#define IGU_EGU_CFG_EGU_TX_1588_PKT_0_REG               (IGU_EGU_CFG_BASE + 0x1114) /* EGU发送的1588报文统计计数器 */
#define IGU_EGU_CFG_EGU_TX_1588_PKT_1_REG               (IGU_EGU_CFG_BASE + 0x1414) /* EGU发送的1588报文统计计数器 */
#define IGU_EGU_CFG_EGU_TX_1588_PKT_2_REG               (IGU_EGU_CFG_BASE + 0x1714) /* EGU发送的1588报文统计计数器 */
#define IGU_EGU_CFG_EGU_TX_1588_PKT_3_REG               (IGU_EGU_CFG_BASE + 0x1A14) /* EGU发送的1588报文统计计数器 */
#define IGU_EGU_CFG_EGU_TX_DYINGGASP_CNT_0_REG          (IGU_EGU_CFG_BASE + 0x1118) /* EGU发送的Dying gasp帧统计计数器 */
#define IGU_EGU_CFG_EGU_TX_DYINGGASP_CNT_1_REG          (IGU_EGU_CFG_BASE + 0x1418) /* EGU发送的Dying gasp帧统计计数器 */
#define IGU_EGU_CFG_EGU_TX_DYINGGASP_CNT_2_REG          (IGU_EGU_CFG_BASE + 0x1718) /* EGU发送的Dying gasp帧统计计数器 */
#define IGU_EGU_CFG_EGU_TX_DYINGGASP_CNT_3_REG          (IGU_EGU_CFG_BASE + 0x1A18) /* EGU发送的Dying gasp帧统计计数器 */
#define IGU_EGU_CFG_IGU_EGU_FIFO_STATUS_0_REG           (IGU_EGU_CFG_BASE + 0x1120) /* IGU和EGU 缓存状态寄存器 */
#define IGU_EGU_CFG_IGU_EGU_FIFO_STATUS_1_REG           (IGU_EGU_CFG_BASE + 0x1420) /* IGU和EGU 缓存状态寄存器 */
#define IGU_EGU_CFG_IGU_EGU_FIFO_STATUS_2_REG           (IGU_EGU_CFG_BASE + 0x1720) /* IGU和EGU 缓存状态寄存器 */
#define IGU_EGU_CFG_IGU_EGU_FIFO_STATUS_3_REG           (IGU_EGU_CFG_BASE + 0x1A20) /* IGU和EGU 缓存状态寄存器 */

#endif // __IGU_EGU_CFG_REG_OFFSET_H__
