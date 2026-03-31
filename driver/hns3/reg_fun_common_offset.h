/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: reg_fun_common_offset
 * Author: huawei
 * Create: 2021-12-28
 */

#ifndef __REG_FUN_COMMON_OFFSET_H__
#define __REG_FUN_COMMON_OFFSET_H__

/* FUN_COMMON Base address of Module's Register */
#define FUN_COMMON_BASE                       (0x0)

/******************************************************************************/
/*                      xxx FUN_COMMON Registers' Definitions                 */
/******************************************************************************/

#define FUN_COMMON_TQP_INT_CTRL_0_REG                (FUN_COMMON_BASE + 0x0)   /* 队列中断控制寄存器 */
#define FUN_COMMON_TQP_INT_CTRL_1_REG                (FUN_COMMON_BASE + 0x4)   /* 队列中断控制寄存器 */
#define FUN_COMMON_TQP_INT_CTRL_2_REG                (FUN_COMMON_BASE + 0x8)   /* 队列中断控制寄存器 */
#define FUN_COMMON_TQP_INT_CTRL_3_REG                (FUN_COMMON_BASE + 0xC)   /* 队列中断控制寄存器 */
#define FUN_COMMON_TQP_INT_CTRL_4_REG                (FUN_COMMON_BASE + 0x10)  /* 队列中断控制寄存器 */
#define FUN_COMMON_TQP_INT_CTRL_5_REG                (FUN_COMMON_BASE + 0x14)  /* 队列中断控制寄存器 */
#define FUN_COMMON_TQP_INT_CTRL_6_REG                (FUN_COMMON_BASE + 0x18)  /* 队列中断控制寄存器 */
#define FUN_COMMON_TQP_INT_CTRL_7_REG                (FUN_COMMON_BASE + 0x1C)  /* 队列中断控制寄存器 */
#define FUN_COMMON_TQP_INT_CTRL_8_REG                (FUN_COMMON_BASE + 0x20)  /* 队列中断控制寄存器 */
#define FUN_COMMON_TQP_INT_CTRL_9_REG                (FUN_COMMON_BASE + 0x24)  /* 队列中断控制寄存器 */
#define FUN_COMMON_TQP_INT_CTRL_10_REG               (FUN_COMMON_BASE + 0x28)  /* 队列中断控制寄存器 */
#define FUN_COMMON_TQP_INT_CTRL_11_REG               (FUN_COMMON_BASE + 0x2C)  /* 队列中断控制寄存器 */
#define FUN_COMMON_TQP_INT_CTRL_12_REG               (FUN_COMMON_BASE + 0x30)  /* 队列中断控制寄存器 */
#define FUN_COMMON_TQP_INT_CTRL_13_REG               (FUN_COMMON_BASE + 0x34)  /* 队列中断控制寄存器 */
#define FUN_COMMON_TQP_INT_CTRL_14_REG               (FUN_COMMON_BASE + 0x38)  /* 队列中断控制寄存器 */
#define FUN_COMMON_TQP_INT_CTRL_15_REG               (FUN_COMMON_BASE + 0x3C)  /* 队列中断控制寄存器 */
#define FUN_COMMON_TQP_INT_GL0_0_REG                 (FUN_COMMON_BASE + 0x100) /* 队列中断间隔配置寄存器0 */
#define FUN_COMMON_TQP_INT_GL0_1_REG                 (FUN_COMMON_BASE + 0x104) /* 队列中断间隔配置寄存器0 */
#define FUN_COMMON_TQP_INT_GL0_2_REG                 (FUN_COMMON_BASE + 0x108) /* 队列中断间隔配置寄存器0 */
#define FUN_COMMON_TQP_INT_GL0_3_REG                 (FUN_COMMON_BASE + 0x10C) /* 队列中断间隔配置寄存器0 */
#define FUN_COMMON_TQP_INT_GL0_4_REG                 (FUN_COMMON_BASE + 0x110) /* 队列中断间隔配置寄存器0 */
#define FUN_COMMON_TQP_INT_GL0_5_REG                 (FUN_COMMON_BASE + 0x114) /* 队列中断间隔配置寄存器0 */
#define FUN_COMMON_TQP_INT_GL0_6_REG                 (FUN_COMMON_BASE + 0x118) /* 队列中断间隔配置寄存器0 */
#define FUN_COMMON_TQP_INT_GL0_7_REG                 (FUN_COMMON_BASE + 0x11C) /* 队列中断间隔配置寄存器0 */
#define FUN_COMMON_TQP_INT_GL0_8_REG                 (FUN_COMMON_BASE + 0x120) /* 队列中断间隔配置寄存器0 */
#define FUN_COMMON_TQP_INT_GL0_9_REG                 (FUN_COMMON_BASE + 0x124) /* 队列中断间隔配置寄存器0 */
#define FUN_COMMON_TQP_INT_GL0_10_REG                (FUN_COMMON_BASE + 0x128) /* 队列中断间隔配置寄存器0 */
#define FUN_COMMON_TQP_INT_GL0_11_REG                (FUN_COMMON_BASE + 0x12C) /* 队列中断间隔配置寄存器0 */
#define FUN_COMMON_TQP_INT_GL0_12_REG                (FUN_COMMON_BASE + 0x130) /* 队列中断间隔配置寄存器0 */
#define FUN_COMMON_TQP_INT_GL0_13_REG                (FUN_COMMON_BASE + 0x134) /* 队列中断间隔配置寄存器0 */
#define FUN_COMMON_TQP_INT_GL0_14_REG                (FUN_COMMON_BASE + 0x138) /* 队列中断间隔配置寄存器0 */
#define FUN_COMMON_TQP_INT_GL0_15_REG                (FUN_COMMON_BASE + 0x13C) /* 队列中断间隔配置寄存器0 */
#define FUN_COMMON_TQP_INT_GL1_0_REG                 (FUN_COMMON_BASE + 0x200) /* 队列中断间隔配置寄存器1 */
#define FUN_COMMON_TQP_INT_GL1_1_REG                 (FUN_COMMON_BASE + 0x204) /* 队列中断间隔配置寄存器1 */
#define FUN_COMMON_TQP_INT_GL1_2_REG                 (FUN_COMMON_BASE + 0x208) /* 队列中断间隔配置寄存器1 */
#define FUN_COMMON_TQP_INT_GL1_3_REG                 (FUN_COMMON_BASE + 0x20C) /* 队列中断间隔配置寄存器1 */
#define FUN_COMMON_TQP_INT_GL1_4_REG                 (FUN_COMMON_BASE + 0x210) /* 队列中断间隔配置寄存器1 */
#define FUN_COMMON_TQP_INT_GL1_5_REG                 (FUN_COMMON_BASE + 0x214) /* 队列中断间隔配置寄存器1 */
#define FUN_COMMON_TQP_INT_GL1_6_REG                 (FUN_COMMON_BASE + 0x218) /* 队列中断间隔配置寄存器1 */
#define FUN_COMMON_TQP_INT_GL1_7_REG                 (FUN_COMMON_BASE + 0x21C) /* 队列中断间隔配置寄存器1 */
#define FUN_COMMON_TQP_INT_GL1_8_REG                 (FUN_COMMON_BASE + 0x220) /* 队列中断间隔配置寄存器1 */
#define FUN_COMMON_TQP_INT_GL1_9_REG                 (FUN_COMMON_BASE + 0x224) /* 队列中断间隔配置寄存器1 */
#define FUN_COMMON_TQP_INT_GL1_10_REG                (FUN_COMMON_BASE + 0x228) /* 队列中断间隔配置寄存器1 */
#define FUN_COMMON_TQP_INT_GL1_11_REG                (FUN_COMMON_BASE + 0x22C) /* 队列中断间隔配置寄存器1 */
#define FUN_COMMON_TQP_INT_GL1_12_REG                (FUN_COMMON_BASE + 0x230) /* 队列中断间隔配置寄存器1 */
#define FUN_COMMON_TQP_INT_GL1_13_REG                (FUN_COMMON_BASE + 0x234) /* 队列中断间隔配置寄存器1 */
#define FUN_COMMON_TQP_INT_GL1_14_REG                (FUN_COMMON_BASE + 0x238) /* 队列中断间隔配置寄存器1 */
#define FUN_COMMON_TQP_INT_GL1_15_REG                (FUN_COMMON_BASE + 0x23C) /* 队列中断间隔配置寄存器1 */
#define FUN_COMMON_TQP_INT_GL2_0_REG                 (FUN_COMMON_BASE + 0x300) /* 队列中断间隔配置寄存器2 */
#define FUN_COMMON_TQP_INT_GL2_1_REG                 (FUN_COMMON_BASE + 0x304) /* 队列中断间隔配置寄存器2 */
#define FUN_COMMON_TQP_INT_GL2_2_REG                 (FUN_COMMON_BASE + 0x308) /* 队列中断间隔配置寄存器2 */
#define FUN_COMMON_TQP_INT_GL2_3_REG                 (FUN_COMMON_BASE + 0x30C) /* 队列中断间隔配置寄存器2 */
#define FUN_COMMON_TQP_INT_GL2_4_REG                 (FUN_COMMON_BASE + 0x310) /* 队列中断间隔配置寄存器2 */
#define FUN_COMMON_TQP_INT_GL2_5_REG                 (FUN_COMMON_BASE + 0x314) /* 队列中断间隔配置寄存器2 */
#define FUN_COMMON_TQP_INT_GL2_6_REG                 (FUN_COMMON_BASE + 0x318) /* 队列中断间隔配置寄存器2 */
#define FUN_COMMON_TQP_INT_GL2_7_REG                 (FUN_COMMON_BASE + 0x31C) /* 队列中断间隔配置寄存器2 */
#define FUN_COMMON_TQP_INT_GL2_8_REG                 (FUN_COMMON_BASE + 0x320) /* 队列中断间隔配置寄存器2 */
#define FUN_COMMON_TQP_INT_GL2_9_REG                 (FUN_COMMON_BASE + 0x324) /* 队列中断间隔配置寄存器2 */
#define FUN_COMMON_TQP_INT_GL2_10_REG                (FUN_COMMON_BASE + 0x328) /* 队列中断间隔配置寄存器2 */
#define FUN_COMMON_TQP_INT_GL2_11_REG                (FUN_COMMON_BASE + 0x32C) /* 队列中断间隔配置寄存器2 */
#define FUN_COMMON_TQP_INT_GL2_12_REG                (FUN_COMMON_BASE + 0x330) /* 队列中断间隔配置寄存器2 */
#define FUN_COMMON_TQP_INT_GL2_13_REG                (FUN_COMMON_BASE + 0x334) /* 队列中断间隔配置寄存器2 */
#define FUN_COMMON_TQP_INT_GL2_14_REG                (FUN_COMMON_BASE + 0x338) /* 队列中断间隔配置寄存器2 */
#define FUN_COMMON_TQP_INT_GL2_15_REG                (FUN_COMMON_BASE + 0x33C) /* 队列中断间隔配置寄存器2 */
#define FUN_COMMON_TQP_INT_RL_0_REG                  (FUN_COMMON_BASE + 0x900) /* 队列中断速率配置寄存器 */
#define FUN_COMMON_TQP_INT_RL_1_REG                  (FUN_COMMON_BASE + 0x904) /* 队列中断速率配置寄存器 */
#define FUN_COMMON_TQP_INT_RL_2_REG                  (FUN_COMMON_BASE + 0x908) /* 队列中断速率配置寄存器 */
#define FUN_COMMON_TQP_INT_RL_3_REG                  (FUN_COMMON_BASE + 0x90C) /* 队列中断速率配置寄存器 */
#define FUN_COMMON_TQP_INT_RL_4_REG                  (FUN_COMMON_BASE + 0x910) /* 队列中断速率配置寄存器 */
#define FUN_COMMON_TQP_INT_RL_5_REG                  (FUN_COMMON_BASE + 0x914) /* 队列中断速率配置寄存器 */
#define FUN_COMMON_TQP_INT_RL_6_REG                  (FUN_COMMON_BASE + 0x918) /* 队列中断速率配置寄存器 */
#define FUN_COMMON_TQP_INT_RL_7_REG                  (FUN_COMMON_BASE + 0x91C) /* 队列中断速率配置寄存器 */
#define FUN_COMMON_TQP_INT_RL_8_REG                  (FUN_COMMON_BASE + 0x920) /* 队列中断速率配置寄存器 */
#define FUN_COMMON_TQP_INT_RL_9_REG                  (FUN_COMMON_BASE + 0x924) /* 队列中断速率配置寄存器 */
#define FUN_COMMON_TQP_INT_RL_10_REG                 (FUN_COMMON_BASE + 0x928) /* 队列中断速率配置寄存器 */
#define FUN_COMMON_TQP_INT_RL_11_REG                 (FUN_COMMON_BASE + 0x92C) /* 队列中断速率配置寄存器 */
#define FUN_COMMON_TQP_INT_RL_12_REG                 (FUN_COMMON_BASE + 0x930) /* 队列中断速率配置寄存器 */
#define FUN_COMMON_TQP_INT_RL_13_REG                 (FUN_COMMON_BASE + 0x934) /* 队列中断速率配置寄存器 */
#define FUN_COMMON_TQP_INT_RL_14_REG                 (FUN_COMMON_BASE + 0x938) /* 队列中断速率配置寄存器 */
#define FUN_COMMON_TQP_INT_RL_15_REG                 (FUN_COMMON_BASE + 0x93C) /* 队列中断速率配置寄存器 */
#define FUN_COMMON_TQP_INT_QL_TX_0_REG               (FUN_COMMON_BASE + 0xE00) /* TX队列中断个数间隔配置寄存器 */
#define FUN_COMMON_TQP_INT_QL_TX_1_REG               (FUN_COMMON_BASE + 0xE04) /* TX队列中断个数间隔配置寄存器 */
#define FUN_COMMON_TQP_INT_QL_TX_2_REG               (FUN_COMMON_BASE + 0xE08) /* TX队列中断个数间隔配置寄存器 */
#define FUN_COMMON_TQP_INT_QL_TX_3_REG               (FUN_COMMON_BASE + 0xE0C) /* TX队列中断个数间隔配置寄存器 */
#define FUN_COMMON_TQP_INT_QL_TX_4_REG               (FUN_COMMON_BASE + 0xE10) /* TX队列中断个数间隔配置寄存器 */
#define FUN_COMMON_TQP_INT_QL_TX_5_REG               (FUN_COMMON_BASE + 0xE14) /* TX队列中断个数间隔配置寄存器 */
#define FUN_COMMON_TQP_INT_QL_TX_6_REG               (FUN_COMMON_BASE + 0xE18) /* TX队列中断个数间隔配置寄存器 */
#define FUN_COMMON_TQP_INT_QL_TX_7_REG               (FUN_COMMON_BASE + 0xE1C) /* TX队列中断个数间隔配置寄存器 */
#define FUN_COMMON_TQP_INT_QL_TX_8_REG               (FUN_COMMON_BASE + 0xE20) /* TX队列中断个数间隔配置寄存器 */
#define FUN_COMMON_TQP_INT_QL_TX_9_REG               (FUN_COMMON_BASE + 0xE24) /* TX队列中断个数间隔配置寄存器 */
#define FUN_COMMON_TQP_INT_QL_TX_10_REG              (FUN_COMMON_BASE + 0xE28) /* TX队列中断个数间隔配置寄存器 */
#define FUN_COMMON_TQP_INT_QL_TX_11_REG              (FUN_COMMON_BASE + 0xE2C) /* TX队列中断个数间隔配置寄存器 */
#define FUN_COMMON_TQP_INT_QL_TX_12_REG              (FUN_COMMON_BASE + 0xE30) /* TX队列中断个数间隔配置寄存器 */
#define FUN_COMMON_TQP_INT_QL_TX_13_REG              (FUN_COMMON_BASE + 0xE34) /* TX队列中断个数间隔配置寄存器 */
#define FUN_COMMON_TQP_INT_QL_TX_14_REG              (FUN_COMMON_BASE + 0xE38) /* TX队列中断个数间隔配置寄存器 */
#define FUN_COMMON_TQP_INT_QL_TX_15_REG              (FUN_COMMON_BASE + 0xE3C) /* TX队列中断个数间隔配置寄存器 */
#define FUN_COMMON_TQP_INT_QL_RX_0_REG               (FUN_COMMON_BASE + 0xF00) /* RX队列中断个数间隔配置寄存器 */
#define FUN_COMMON_TQP_INT_QL_RX_1_REG               (FUN_COMMON_BASE + 0xF04) /* RX队列中断个数间隔配置寄存器 */
#define FUN_COMMON_TQP_INT_QL_RX_2_REG               (FUN_COMMON_BASE + 0xF08) /* RX队列中断个数间隔配置寄存器 */
#define FUN_COMMON_TQP_INT_QL_RX_3_REG               (FUN_COMMON_BASE + 0xF0C) /* RX队列中断个数间隔配置寄存器 */
#define FUN_COMMON_TQP_INT_QL_RX_4_REG               (FUN_COMMON_BASE + 0xF10) /* RX队列中断个数间隔配置寄存器 */
#define FUN_COMMON_TQP_INT_QL_RX_5_REG               (FUN_COMMON_BASE + 0xF14) /* RX队列中断个数间隔配置寄存器 */
#define FUN_COMMON_TQP_INT_QL_RX_6_REG               (FUN_COMMON_BASE + 0xF18) /* RX队列中断个数间隔配置寄存器 */
#define FUN_COMMON_TQP_INT_QL_RX_7_REG               (FUN_COMMON_BASE + 0xF1C) /* RX队列中断个数间隔配置寄存器 */
#define FUN_COMMON_TQP_INT_QL_RX_8_REG               (FUN_COMMON_BASE + 0xF20) /* RX队列中断个数间隔配置寄存器 */
#define FUN_COMMON_TQP_INT_QL_RX_9_REG               (FUN_COMMON_BASE + 0xF24) /* RX队列中断个数间隔配置寄存器 */
#define FUN_COMMON_TQP_INT_QL_RX_10_REG              (FUN_COMMON_BASE + 0xF28) /* RX队列中断个数间隔配置寄存器 */
#define FUN_COMMON_TQP_INT_QL_RX_11_REG              (FUN_COMMON_BASE + 0xF2C) /* RX队列中断个数间隔配置寄存器 */
#define FUN_COMMON_TQP_INT_QL_RX_12_REG              (FUN_COMMON_BASE + 0xF30) /* RX队列中断个数间隔配置寄存器 */
#define FUN_COMMON_TQP_INT_QL_RX_13_REG              (FUN_COMMON_BASE + 0xF34) /* RX队列中断个数间隔配置寄存器 */
#define FUN_COMMON_TQP_INT_QL_RX_14_REG              (FUN_COMMON_BASE + 0xF38) /* RX队列中断个数间隔配置寄存器 */
#define FUN_COMMON_TQP_INT_QL_RX_15_REG              (FUN_COMMON_BASE + 0xF3C) /* RX队列中断个数间隔配置寄存器 */
#define FUN_COMMON_TQP_INT_EQ_EN_GL0_REG             (FUN_COMMON_BASE + 0xD00) /* 队列中断间GL模式配置寄存器0 */
#define FUN_COMMON_TQP_INT_EQ_EN_GL1_REG             (FUN_COMMON_BASE + 0xD04) /* 队列中断间GL模式配置寄存器1 */
#define FUN_COMMON_TQP_INT_EQ_EN_GL2_REG             (FUN_COMMON_BASE + 0xD08) /* 队列中断间GL模式配置寄存器2 */
#define FUN_COMMON_VECTOR0_PORT_OTHER_INT_CTRL_0_REG (FUN_COMMON_BASE + 0x700) /* 0号中断除CMDQ外其他中断使能 */
#define FUN_COMMON_VECTOR0_PORT_OTHER_INT_CTRL_1_REG (FUN_COMMON_BASE + 0x740) /* 0号中断除CMDQ外其他中断使能 */
#define FUN_COMMON_VECTOR0_PORT_OTHER_INT_CTRL_2_REG (FUN_COMMON_BASE + 0x780) /* 0号中断除CMDQ外其他中断使能 */
#define FUN_COMMON_VECTOR0_PORT_OTHER_INT_CTRL_3_REG (FUN_COMMON_BASE + 0x7C0) /* 0号中断除CMDQ外其他中断使能 */
#define FUN_COMMON_VECTOR0_PORT_OTHER_INT_SRC_0_REG  (FUN_COMMON_BASE + 0x704) /* 0号中断除CMDQ外其他中断原始状态 */
#define FUN_COMMON_VECTOR0_PORT_OTHER_INT_SRC_1_REG  (FUN_COMMON_BASE + 0x744) /* 0号中断除CMDQ外其他中断原始状态 */
#define FUN_COMMON_VECTOR0_PORT_OTHER_INT_SRC_2_REG  (FUN_COMMON_BASE + 0x784) /* 0号中断除CMDQ外其他中断原始状态 */
#define FUN_COMMON_VECTOR0_PORT_OTHER_INT_SRC_3_REG  (FUN_COMMON_BASE + 0x7C4) /* 0号中断除CMDQ外其他中断原始状态 */
#define FUN_COMMON_VECTOR0_PORT_OTHER_INT_STS_0_REG  (FUN_COMMON_BASE + 0x708) /* 0号中断除CMDQ外其他中断状态 */
#define FUN_COMMON_VECTOR0_PORT_OTHER_INT_STS_1_REG  (FUN_COMMON_BASE + 0x748) /* 0号中断除CMDQ外其他中断状态 */
#define FUN_COMMON_VECTOR0_PORT_OTHER_INT_STS_2_REG  (FUN_COMMON_BASE + 0x788) /* 0号中断除CMDQ外其他中断状态 */
#define FUN_COMMON_VECTOR0_PORT_OTHER_INT_STS_3_REG  (FUN_COMMON_BASE + 0x7C8) /* 0号中断除CMDQ外其他中断状态 */
#define FUN_COMMON_RAS_PORT_OTHER_INT_STS_0_REG      (FUN_COMMON_BASE + 0x710) /* RAS中断状态 */
#define FUN_COMMON_RAS_PORT_OTHER_INT_STS_1_REG      (FUN_COMMON_BASE + 0x750) /* RAS中断状态 */
#define FUN_COMMON_RAS_PORT_OTHER_INT_STS_2_REG      (FUN_COMMON_BASE + 0x790) /* RAS中断状态 */
#define FUN_COMMON_RAS_PORT_OTHER_INT_STS_3_REG      (FUN_COMMON_BASE + 0x7D0) /* RAS中断状态 */

#endif // __REG_FUN_COMMON_OFFSET_H__
