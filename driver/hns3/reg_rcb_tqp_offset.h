/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: reg_rcb_tqp_offset
 * Author: huawei
 * Create: 2021-12-28
 */

#ifndef __REG_RCB_TQP_OFFSET_H__
#define __REG_RCB_TQP_OFFSET_H__

/* PPE_RCB_TQP Base address of Module's Register */
#define PPE_RCB_TQP_BASE                       (0x0)

/******************************************************************************/
/*                      xxx PPE_RCB_TQP Registers' Definitions                */
/******************************************************************************/

#define PPE_RCB_TQP_RCB_CFG_RX_RING_BASEADDR_L_0_REG     (PPE_RCB_TQP_BASE + 0x0)    /* 接收Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BASEADDR_L_1_REG     (PPE_RCB_TQP_BASE + 0x200)  /* 接收Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BASEADDR_L_2_REG     (PPE_RCB_TQP_BASE + 0x400)  /* 接收Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BASEADDR_L_3_REG     (PPE_RCB_TQP_BASE + 0x600)  /* 接收Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BASEADDR_L_4_REG     (PPE_RCB_TQP_BASE + 0x800)  /* 接收Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BASEADDR_L_5_REG     (PPE_RCB_TQP_BASE + 0xA00)  /* 接收Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BASEADDR_L_6_REG     (PPE_RCB_TQP_BASE + 0xC00)  /* 接收Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BASEADDR_L_7_REG     (PPE_RCB_TQP_BASE + 0xE00)  /* 接收Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BASEADDR_L_8_REG     (PPE_RCB_TQP_BASE + 0x1000) /* 接收Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BASEADDR_L_9_REG     (PPE_RCB_TQP_BASE + 0x1200) /* 接收Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BASEADDR_L_10_REG    (PPE_RCB_TQP_BASE + 0x1400) /* 接收Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BASEADDR_L_11_REG    (PPE_RCB_TQP_BASE + 0x1600) /* 接收Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BASEADDR_L_12_REG    (PPE_RCB_TQP_BASE + 0x1800) /* 接收Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BASEADDR_L_13_REG    (PPE_RCB_TQP_BASE + 0x1A00) /* 接收Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BASEADDR_L_14_REG    (PPE_RCB_TQP_BASE + 0x1C00) /* 接收Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BASEADDR_L_15_REG    (PPE_RCB_TQP_BASE + 0x1E00) /* 接收Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BASEADDR_H_0_REG     (PPE_RCB_TQP_BASE + 0x4)    /* 接收Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BASEADDR_H_1_REG     (PPE_RCB_TQP_BASE + 0x204)  /* 接收Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BASEADDR_H_2_REG     (PPE_RCB_TQP_BASE + 0x404)  /* 接收Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BASEADDR_H_3_REG     (PPE_RCB_TQP_BASE + 0x604)  /* 接收Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BASEADDR_H_4_REG     (PPE_RCB_TQP_BASE + 0x804)  /* 接收Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BASEADDR_H_5_REG     (PPE_RCB_TQP_BASE + 0xA04)  /* 接收Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BASEADDR_H_6_REG     (PPE_RCB_TQP_BASE + 0xC04)  /* 接收Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BASEADDR_H_7_REG     (PPE_RCB_TQP_BASE + 0xE04)  /* 接收Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BASEADDR_H_8_REG     (PPE_RCB_TQP_BASE + 0x1004) /* 接收Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BASEADDR_H_9_REG     (PPE_RCB_TQP_BASE + 0x1204) /* 接收Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BASEADDR_H_10_REG    (PPE_RCB_TQP_BASE + 0x1404) /* 接收Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BASEADDR_H_11_REG    (PPE_RCB_TQP_BASE + 0x1604) /* 接收Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BASEADDR_H_12_REG    (PPE_RCB_TQP_BASE + 0x1804) /* 接收Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BASEADDR_H_13_REG    (PPE_RCB_TQP_BASE + 0x1A04) /* 接收Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BASEADDR_H_14_REG    (PPE_RCB_TQP_BASE + 0x1C04) /* 接收Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BASEADDR_H_15_REG    (PPE_RCB_TQP_BASE + 0x1E04) /* 接收Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_NUM_0_REG         (PPE_RCB_TQP_BASE + 0x8)    /* 接收队列的BD个数 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_NUM_1_REG         (PPE_RCB_TQP_BASE + 0x208)  /* 接收队列的BD个数 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_NUM_2_REG         (PPE_RCB_TQP_BASE + 0x408)  /* 接收队列的BD个数 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_NUM_3_REG         (PPE_RCB_TQP_BASE + 0x608)  /* 接收队列的BD个数 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_NUM_4_REG         (PPE_RCB_TQP_BASE + 0x808)  /* 接收队列的BD个数 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_NUM_5_REG         (PPE_RCB_TQP_BASE + 0xA08)  /* 接收队列的BD个数 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_NUM_6_REG         (PPE_RCB_TQP_BASE + 0xC08)  /* 接收队列的BD个数 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_NUM_7_REG         (PPE_RCB_TQP_BASE + 0xE08)  /* 接收队列的BD个数 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_NUM_8_REG         (PPE_RCB_TQP_BASE + 0x1008) /* 接收队列的BD个数 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_NUM_9_REG         (PPE_RCB_TQP_BASE + 0x1208) /* 接收队列的BD个数 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_NUM_10_REG        (PPE_RCB_TQP_BASE + 0x1408) /* 接收队列的BD个数 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_NUM_11_REG        (PPE_RCB_TQP_BASE + 0x1608) /* 接收队列的BD个数 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_NUM_12_REG        (PPE_RCB_TQP_BASE + 0x1808) /* 接收队列的BD个数 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_NUM_13_REG        (PPE_RCB_TQP_BASE + 0x1A08) /* 接收队列的BD个数 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_NUM_14_REG        (PPE_RCB_TQP_BASE + 0x1C08) /* 接收队列的BD个数 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_NUM_15_REG        (PPE_RCB_TQP_BASE + 0x1E08) /* 接收队列的BD个数 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_LEN_0_REG         (PPE_RCB_TQP_BASE + 0xC)    /* 接收队列的Bd代表buffer大小 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_LEN_1_REG         (PPE_RCB_TQP_BASE + 0x20C)  /* 接收队列的Bd代表buffer大小 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_LEN_2_REG         (PPE_RCB_TQP_BASE + 0x40C)  /* 接收队列的Bd代表buffer大小 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_LEN_3_REG         (PPE_RCB_TQP_BASE + 0x60C)  /* 接收队列的Bd代表buffer大小 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_LEN_4_REG         (PPE_RCB_TQP_BASE + 0x80C)  /* 接收队列的Bd代表buffer大小 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_LEN_5_REG         (PPE_RCB_TQP_BASE + 0xA0C)  /* 接收队列的Bd代表buffer大小 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_LEN_6_REG         (PPE_RCB_TQP_BASE + 0xC0C)  /* 接收队列的Bd代表buffer大小 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_LEN_7_REG         (PPE_RCB_TQP_BASE + 0xE0C)  /* 接收队列的Bd代表buffer大小 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_LEN_8_REG         (PPE_RCB_TQP_BASE + 0x100C) /* 接收队列的Bd代表buffer大小 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_LEN_9_REG         (PPE_RCB_TQP_BASE + 0x120C) /* 接收队列的Bd代表buffer大小 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_LEN_10_REG        (PPE_RCB_TQP_BASE + 0x140C) /* 接收队列的Bd代表buffer大小 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_LEN_11_REG        (PPE_RCB_TQP_BASE + 0x160C) /* 接收队列的Bd代表buffer大小 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_LEN_12_REG        (PPE_RCB_TQP_BASE + 0x180C) /* 接收队列的Bd代表buffer大小 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_LEN_13_REG        (PPE_RCB_TQP_BASE + 0x1A0C) /* 接收队列的Bd代表buffer大小 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_LEN_14_REG        (PPE_RCB_TQP_BASE + 0x1C0C) /* 接收队列的Bd代表buffer大小 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_LEN_15_REG        (PPE_RCB_TQP_BASE + 0x1E0C) /* 接收队列的Bd代表buffer大小 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_MERGE_EN_0_REG       (PPE_RCB_TQP_BASE + 0x14)   /* 接收Ring回写BD merge使能 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_MERGE_EN_1_REG       (PPE_RCB_TQP_BASE + 0x214)  /* 接收Ring回写BD merge使能 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_MERGE_EN_2_REG       (PPE_RCB_TQP_BASE + 0x414)  /* 接收Ring回写BD merge使能 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_MERGE_EN_3_REG       (PPE_RCB_TQP_BASE + 0x614)  /* 接收Ring回写BD merge使能 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_MERGE_EN_4_REG       (PPE_RCB_TQP_BASE + 0x814)  /* 接收Ring回写BD merge使能 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_MERGE_EN_5_REG       (PPE_RCB_TQP_BASE + 0xA14)  /* 接收Ring回写BD merge使能 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_MERGE_EN_6_REG       (PPE_RCB_TQP_BASE + 0xC14)  /* 接收Ring回写BD merge使能 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_MERGE_EN_7_REG       (PPE_RCB_TQP_BASE + 0xE14)  /* 接收Ring回写BD merge使能 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_MERGE_EN_8_REG       (PPE_RCB_TQP_BASE + 0x1014) /* 接收Ring回写BD merge使能 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_MERGE_EN_9_REG       (PPE_RCB_TQP_BASE + 0x1214) /* 接收Ring回写BD merge使能 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_MERGE_EN_10_REG      (PPE_RCB_TQP_BASE + 0x1414) /* 接收Ring回写BD merge使能 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_MERGE_EN_11_REG      (PPE_RCB_TQP_BASE + 0x1614) /* 接收Ring回写BD merge使能 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_MERGE_EN_12_REG      (PPE_RCB_TQP_BASE + 0x1814) /* 接收Ring回写BD merge使能 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_MERGE_EN_13_REG      (PPE_RCB_TQP_BASE + 0x1A14) /* 接收Ring回写BD merge使能 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_MERGE_EN_14_REG      (PPE_RCB_TQP_BASE + 0x1C14) /* 接收Ring回写BD merge使能 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_MERGE_EN_15_REG      (PPE_RCB_TQP_BASE + 0x1E14) /* 接收Ring回写BD merge使能 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_TAIL_0_REG           (PPE_RCB_TQP_BASE + 0x18)   /* 接收Ring的tail指针 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_TAIL_1_REG           (PPE_RCB_TQP_BASE + 0x218)  /* 接收Ring的tail指针 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_TAIL_2_REG           (PPE_RCB_TQP_BASE + 0x418)  /* 接收Ring的tail指针 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_TAIL_3_REG           (PPE_RCB_TQP_BASE + 0x618)  /* 接收Ring的tail指针 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_TAIL_4_REG           (PPE_RCB_TQP_BASE + 0x818)  /* 接收Ring的tail指针 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_TAIL_5_REG           (PPE_RCB_TQP_BASE + 0xA18)  /* 接收Ring的tail指针 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_TAIL_6_REG           (PPE_RCB_TQP_BASE + 0xC18)  /* 接收Ring的tail指针 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_TAIL_7_REG           (PPE_RCB_TQP_BASE + 0xE18)  /* 接收Ring的tail指针 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_TAIL_8_REG           (PPE_RCB_TQP_BASE + 0x1018) /* 接收Ring的tail指针 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_TAIL_9_REG           (PPE_RCB_TQP_BASE + 0x1218) /* 接收Ring的tail指针 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_TAIL_10_REG          (PPE_RCB_TQP_BASE + 0x1418) /* 接收Ring的tail指针 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_TAIL_11_REG          (PPE_RCB_TQP_BASE + 0x1618) /* 接收Ring的tail指针 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_TAIL_12_REG          (PPE_RCB_TQP_BASE + 0x1818) /* 接收Ring的tail指针 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_TAIL_13_REG          (PPE_RCB_TQP_BASE + 0x1A18) /* 接收Ring的tail指针 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_TAIL_14_REG          (PPE_RCB_TQP_BASE + 0x1C18) /* 接收Ring的tail指针 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_TAIL_15_REG          (PPE_RCB_TQP_BASE + 0x1E18) /* 接收Ring的tail指针 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_HEAD_0_REG           (PPE_RCB_TQP_BASE + 0x1C)   /* 接收Ring的head指针 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_HEAD_1_REG           (PPE_RCB_TQP_BASE + 0x21C)  /* 接收Ring的head指针 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_HEAD_2_REG           (PPE_RCB_TQP_BASE + 0x41C)  /* 接收Ring的head指针 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_HEAD_3_REG           (PPE_RCB_TQP_BASE + 0x61C)  /* 接收Ring的head指针 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_HEAD_4_REG           (PPE_RCB_TQP_BASE + 0x81C)  /* 接收Ring的head指针 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_HEAD_5_REG           (PPE_RCB_TQP_BASE + 0xA1C)  /* 接收Ring的head指针 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_HEAD_6_REG           (PPE_RCB_TQP_BASE + 0xC1C)  /* 接收Ring的head指针 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_HEAD_7_REG           (PPE_RCB_TQP_BASE + 0xE1C)  /* 接收Ring的head指针 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_HEAD_8_REG           (PPE_RCB_TQP_BASE + 0x101C) /* 接收Ring的head指针 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_HEAD_9_REG           (PPE_RCB_TQP_BASE + 0x121C) /* 接收Ring的head指针 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_HEAD_10_REG          (PPE_RCB_TQP_BASE + 0x141C) /* 接收Ring的head指针 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_HEAD_11_REG          (PPE_RCB_TQP_BASE + 0x161C) /* 接收Ring的head指针 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_HEAD_12_REG          (PPE_RCB_TQP_BASE + 0x181C) /* 接收Ring的head指针 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_HEAD_13_REG          (PPE_RCB_TQP_BASE + 0x1A1C) /* 接收Ring的head指针 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_HEAD_14_REG          (PPE_RCB_TQP_BASE + 0x1C1C) /* 接收Ring的head指针 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_HEAD_15_REG          (PPE_RCB_TQP_BASE + 0x1E1C) /* 接收Ring的head指针 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_FBDNUM_0_REG         (PPE_RCB_TQP_BASE + 0x20)   /* 接收Ring上F-BD的个数 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_FBDNUM_1_REG         (PPE_RCB_TQP_BASE + 0x220)  /* 接收Ring上F-BD的个数 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_FBDNUM_2_REG         (PPE_RCB_TQP_BASE + 0x420)  /* 接收Ring上F-BD的个数 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_FBDNUM_3_REG         (PPE_RCB_TQP_BASE + 0x620)  /* 接收Ring上F-BD的个数 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_FBDNUM_4_REG         (PPE_RCB_TQP_BASE + 0x820)  /* 接收Ring上F-BD的个数 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_FBDNUM_5_REG         (PPE_RCB_TQP_BASE + 0xA20)  /* 接收Ring上F-BD的个数 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_FBDNUM_6_REG         (PPE_RCB_TQP_BASE + 0xC20)  /* 接收Ring上F-BD的个数 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_FBDNUM_7_REG         (PPE_RCB_TQP_BASE + 0xE20)  /* 接收Ring上F-BD的个数 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_FBDNUM_8_REG         (PPE_RCB_TQP_BASE + 0x1020) /* 接收Ring上F-BD的个数 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_FBDNUM_9_REG         (PPE_RCB_TQP_BASE + 0x1220) /* 接收Ring上F-BD的个数 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_FBDNUM_10_REG        (PPE_RCB_TQP_BASE + 0x1420) /* 接收Ring上F-BD的个数 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_FBDNUM_11_REG        (PPE_RCB_TQP_BASE + 0x1620) /* 接收Ring上F-BD的个数 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_FBDNUM_12_REG        (PPE_RCB_TQP_BASE + 0x1820) /* 接收Ring上F-BD的个数 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_FBDNUM_13_REG        (PPE_RCB_TQP_BASE + 0x1A20) /* 接收Ring上F-BD的个数 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_FBDNUM_14_REG        (PPE_RCB_TQP_BASE + 0x1C20) /* 接收Ring上F-BD的个数 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_FBDNUM_15_REG        (PPE_RCB_TQP_BASE + 0x1E20) /* 接收Ring上F-BD的个数 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_OFFSET_0_REG         (PPE_RCB_TQP_BASE + 0x24)   /* 接收Ring上EBD与head指针的距离 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_OFFSET_1_REG         (PPE_RCB_TQP_BASE + 0x224)  /* 接收Ring上EBD与head指针的距离 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_OFFSET_2_REG         (PPE_RCB_TQP_BASE + 0x424)  /* 接收Ring上EBD与head指针的距离 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_OFFSET_3_REG         (PPE_RCB_TQP_BASE + 0x624)  /* 接收Ring上EBD与head指针的距离 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_OFFSET_4_REG         (PPE_RCB_TQP_BASE + 0x824)  /* 接收Ring上EBD与head指针的距离 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_OFFSET_5_REG         (PPE_RCB_TQP_BASE + 0xA24)  /* 接收Ring上EBD与head指针的距离 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_OFFSET_6_REG         (PPE_RCB_TQP_BASE + 0xC24)  /* 接收Ring上EBD与head指针的距离 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_OFFSET_7_REG         (PPE_RCB_TQP_BASE + 0xE24)  /* 接收Ring上EBD与head指针的距离 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_OFFSET_8_REG         (PPE_RCB_TQP_BASE + 0x1024) /* 接收Ring上EBD与head指针的距离 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_OFFSET_9_REG         (PPE_RCB_TQP_BASE + 0x1224) /* 接收Ring上EBD与head指针的距离 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_OFFSET_10_REG        (PPE_RCB_TQP_BASE + 0x1424) /* 接收Ring上EBD与head指针的距离 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_OFFSET_11_REG        (PPE_RCB_TQP_BASE + 0x1624) /* 接收Ring上EBD与head指针的距离 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_OFFSET_12_REG        (PPE_RCB_TQP_BASE + 0x1824) /* 接收Ring上EBD与head指针的距离 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_OFFSET_13_REG        (PPE_RCB_TQP_BASE + 0x1A24) /* 接收Ring上EBD与head指针的距离 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_OFFSET_14_REG        (PPE_RCB_TQP_BASE + 0x1C24) /* 接收Ring上EBD与head指针的距离 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_OFFSET_15_REG        (PPE_RCB_TQP_BASE + 0x1E24) /* 接收Ring上EBD与head指针的距离 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_FBDOFFSET_0_REG      (PPE_RCB_TQP_BASE + 0x28)   /* 接收Ring上回写FBD时与head指针的距离 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_FBDOFFSET_1_REG      (PPE_RCB_TQP_BASE + 0x228)  /* 接收Ring上回写FBD时与head指针的距离 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_FBDOFFSET_2_REG      (PPE_RCB_TQP_BASE + 0x428)  /* 接收Ring上回写FBD时与head指针的距离 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_FBDOFFSET_3_REG      (PPE_RCB_TQP_BASE + 0x628)  /* 接收Ring上回写FBD时与head指针的距离 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_FBDOFFSET_4_REG      (PPE_RCB_TQP_BASE + 0x828)  /* 接收Ring上回写FBD时与head指针的距离 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_FBDOFFSET_5_REG      (PPE_RCB_TQP_BASE + 0xA28)  /* 接收Ring上回写FBD时与head指针的距离 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_FBDOFFSET_6_REG      (PPE_RCB_TQP_BASE + 0xC28)  /* 接收Ring上回写FBD时与head指针的距离 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_FBDOFFSET_7_REG      (PPE_RCB_TQP_BASE + 0xE28)  /* 接收Ring上回写FBD时与head指针的距离 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_FBDOFFSET_8_REG      (PPE_RCB_TQP_BASE + 0x1028) /* 接收Ring上回写FBD时与head指针的距离 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_FBDOFFSET_9_REG      (PPE_RCB_TQP_BASE + 0x1228) /* 接收Ring上回写FBD时与head指针的距离 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_FBDOFFSET_10_REG     (PPE_RCB_TQP_BASE + 0x1428) /* 接收Ring上回写FBD时与head指针的距离 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_FBDOFFSET_11_REG     (PPE_RCB_TQP_BASE + 0x1628) /* 接收Ring上回写FBD时与head指针的距离 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_FBDOFFSET_12_REG     (PPE_RCB_TQP_BASE + 0x1828) /* 接收Ring上回写FBD时与head指针的距离 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_FBDOFFSET_13_REG     (PPE_RCB_TQP_BASE + 0x1A28) /* 接收Ring上回写FBD时与head指针的距离 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_FBDOFFSET_14_REG     (PPE_RCB_TQP_BASE + 0x1C28) /* 接收Ring上回写FBD时与head指针的距离 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_FBDOFFSET_15_REG     (PPE_RCB_TQP_BASE + 0x1E28) /* 接收Ring上回写FBD时与head指针的距离 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_PKTNUM_RECORD_0_REG  (PPE_RCB_TQP_BASE + 0x2C)   /* 接收Ring接收包的统计 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_PKTNUM_RECORD_1_REG  (PPE_RCB_TQP_BASE + 0x22C)  /* 接收Ring接收包的统计 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_PKTNUM_RECORD_2_REG  (PPE_RCB_TQP_BASE + 0x42C)  /* 接收Ring接收包的统计 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_PKTNUM_RECORD_3_REG  (PPE_RCB_TQP_BASE + 0x62C)  /* 接收Ring接收包的统计 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_PKTNUM_RECORD_4_REG  (PPE_RCB_TQP_BASE + 0x82C)  /* 接收Ring接收包的统计 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_PKTNUM_RECORD_5_REG  (PPE_RCB_TQP_BASE + 0xA2C)  /* 接收Ring接收包的统计 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_PKTNUM_RECORD_6_REG  (PPE_RCB_TQP_BASE + 0xC2C)  /* 接收Ring接收包的统计 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_PKTNUM_RECORD_7_REG  (PPE_RCB_TQP_BASE + 0xE2C)  /* 接收Ring接收包的统计 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_PKTNUM_RECORD_8_REG  (PPE_RCB_TQP_BASE + 0x102C) /* 接收Ring接收包的统计 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_PKTNUM_RECORD_9_REG  (PPE_RCB_TQP_BASE + 0x122C) /* 接收Ring接收包的统计 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_PKTNUM_RECORD_10_REG (PPE_RCB_TQP_BASE + 0x142C) /* 接收Ring接收包的统计 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_PKTNUM_RECORD_11_REG (PPE_RCB_TQP_BASE + 0x162C) /* 接收Ring接收包的统计 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_PKTNUM_RECORD_12_REG (PPE_RCB_TQP_BASE + 0x182C) /* 接收Ring接收包的统计 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_PKTNUM_RECORD_13_REG (PPE_RCB_TQP_BASE + 0x1A2C) /* 接收Ring接收包的统计 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_PKTNUM_RECORD_14_REG (PPE_RCB_TQP_BASE + 0x1C2C) /* 接收Ring接收包的统计 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_PKTNUM_RECORD_15_REG (PPE_RCB_TQP_BASE + 0x1E2C) /* 接收Ring接收包的统计 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_LPID_0_REG           (PPE_RCB_TQP_BASE + 0x30)   /* 接收队列的stash配置 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_LPID_1_REG           (PPE_RCB_TQP_BASE + 0x230)  /* 接收队列的stash配置 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_LPID_2_REG           (PPE_RCB_TQP_BASE + 0x430)  /* 接收队列的stash配置 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_LPID_3_REG           (PPE_RCB_TQP_BASE + 0x630)  /* 接收队列的stash配置 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_LPID_4_REG           (PPE_RCB_TQP_BASE + 0x830)  /* 接收队列的stash配置 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_LPID_5_REG           (PPE_RCB_TQP_BASE + 0xA30)  /* 接收队列的stash配置 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_LPID_6_REG           (PPE_RCB_TQP_BASE + 0xC30)  /* 接收队列的stash配置 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_LPID_7_REG           (PPE_RCB_TQP_BASE + 0xE30)  /* 接收队列的stash配置 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_LPID_8_REG           (PPE_RCB_TQP_BASE + 0x1030) /* 接收队列的stash配置 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_LPID_9_REG           (PPE_RCB_TQP_BASE + 0x1230) /* 接收队列的stash配置 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_LPID_10_REG          (PPE_RCB_TQP_BASE + 0x1430) /* 接收队列的stash配置 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_LPID_11_REG          (PPE_RCB_TQP_BASE + 0x1630) /* 接收队列的stash配置 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_LPID_12_REG          (PPE_RCB_TQP_BASE + 0x1830) /* 接收队列的stash配置 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_LPID_13_REG          (PPE_RCB_TQP_BASE + 0x1A30) /* 接收队列的stash配置 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_LPID_14_REG          (PPE_RCB_TQP_BASE + 0x1C30) /* 接收队列的stash配置 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_LPID_15_REG          (PPE_RCB_TQP_BASE + 0x1E30) /* 接收队列的stash配置 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_ERR_0_REG         (PPE_RCB_TQP_BASE + 0x34)   /* 接收Ring上BD ERROR状态 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_ERR_1_REG         (PPE_RCB_TQP_BASE + 0x234)  /* 接收Ring上BD ERROR状态 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_ERR_2_REG         (PPE_RCB_TQP_BASE + 0x434)  /* 接收Ring上BD ERROR状态 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_ERR_3_REG         (PPE_RCB_TQP_BASE + 0x634)  /* 接收Ring上BD ERROR状态 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_ERR_4_REG         (PPE_RCB_TQP_BASE + 0x834)  /* 接收Ring上BD ERROR状态 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_ERR_5_REG         (PPE_RCB_TQP_BASE + 0xA34)  /* 接收Ring上BD ERROR状态 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_ERR_6_REG         (PPE_RCB_TQP_BASE + 0xC34)  /* 接收Ring上BD ERROR状态 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_ERR_7_REG         (PPE_RCB_TQP_BASE + 0xE34)  /* 接收Ring上BD ERROR状态 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_ERR_8_REG         (PPE_RCB_TQP_BASE + 0x1034) /* 接收Ring上BD ERROR状态 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_ERR_9_REG         (PPE_RCB_TQP_BASE + 0x1234) /* 接收Ring上BD ERROR状态 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_ERR_10_REG        (PPE_RCB_TQP_BASE + 0x1434) /* 接收Ring上BD ERROR状态 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_ERR_11_REG        (PPE_RCB_TQP_BASE + 0x1634) /* 接收Ring上BD ERROR状态 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_ERR_12_REG        (PPE_RCB_TQP_BASE + 0x1834) /* 接收Ring上BD ERROR状态 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_ERR_13_REG        (PPE_RCB_TQP_BASE + 0x1A34) /* 接收Ring上BD ERROR状态 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_ERR_14_REG        (PPE_RCB_TQP_BASE + 0x1C34) /* 接收Ring上BD ERROR状态 */
#define PPE_RCB_TQP_RCB_CFG_RX_RING_BD_ERR_15_REG        (PPE_RCB_TQP_BASE + 0x1E34) /* 接收Ring上BD ERROR状态 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BASEADDR_L_0_REG     (PPE_RCB_TQP_BASE + 0x40)   /* 发送Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BASEADDR_L_1_REG     (PPE_RCB_TQP_BASE + 0x240)  /* 发送Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BASEADDR_L_2_REG     (PPE_RCB_TQP_BASE + 0x440)  /* 发送Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BASEADDR_L_3_REG     (PPE_RCB_TQP_BASE + 0x640)  /* 发送Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BASEADDR_L_4_REG     (PPE_RCB_TQP_BASE + 0x840)  /* 发送Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BASEADDR_L_5_REG     (PPE_RCB_TQP_BASE + 0xA40)  /* 发送Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BASEADDR_L_6_REG     (PPE_RCB_TQP_BASE + 0xC40)  /* 发送Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BASEADDR_L_7_REG     (PPE_RCB_TQP_BASE + 0xE40)  /* 发送Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BASEADDR_L_8_REG     (PPE_RCB_TQP_BASE + 0x1040) /* 发送Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BASEADDR_L_9_REG     (PPE_RCB_TQP_BASE + 0x1240) /* 发送Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BASEADDR_L_10_REG    (PPE_RCB_TQP_BASE + 0x1440) /* 发送Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BASEADDR_L_11_REG    (PPE_RCB_TQP_BASE + 0x1640) /* 发送Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BASEADDR_L_12_REG    (PPE_RCB_TQP_BASE + 0x1840) /* 发送Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BASEADDR_L_13_REG    (PPE_RCB_TQP_BASE + 0x1A40) /* 发送Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BASEADDR_L_14_REG    (PPE_RCB_TQP_BASE + 0x1C40) /* 发送Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BASEADDR_L_15_REG    (PPE_RCB_TQP_BASE + 0x1E40) /* 发送Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BASEADDR_H_0_REG     (PPE_RCB_TQP_BASE + 0x44)   /* 发送Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BASEADDR_H_1_REG     (PPE_RCB_TQP_BASE + 0x244)  /* 发送Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BASEADDR_H_2_REG     (PPE_RCB_TQP_BASE + 0x444)  /* 发送Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BASEADDR_H_3_REG     (PPE_RCB_TQP_BASE + 0x644)  /* 发送Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BASEADDR_H_4_REG     (PPE_RCB_TQP_BASE + 0x844)  /* 发送Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BASEADDR_H_5_REG     (PPE_RCB_TQP_BASE + 0xA44)  /* 发送Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BASEADDR_H_6_REG     (PPE_RCB_TQP_BASE + 0xC44)  /* 发送Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BASEADDR_H_7_REG     (PPE_RCB_TQP_BASE + 0xE44)  /* 发送Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BASEADDR_H_8_REG     (PPE_RCB_TQP_BASE + 0x1044) /* 发送Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BASEADDR_H_9_REG     (PPE_RCB_TQP_BASE + 0x1244) /* 发送Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BASEADDR_H_10_REG    (PPE_RCB_TQP_BASE + 0x1444) /* 发送Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BASEADDR_H_11_REG    (PPE_RCB_TQP_BASE + 0x1644) /* 发送Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BASEADDR_H_12_REG    (PPE_RCB_TQP_BASE + 0x1844) /* 发送Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BASEADDR_H_13_REG    (PPE_RCB_TQP_BASE + 0x1A44) /* 发送Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BASEADDR_H_14_REG    (PPE_RCB_TQP_BASE + 0x1C44) /* 发送Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BASEADDR_H_15_REG    (PPE_RCB_TQP_BASE + 0x1E44) /* 发送Ring在DDR中的基地址 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BD_NUM_0_REG         (PPE_RCB_TQP_BASE + 0x48)   /* 发送Ring的BD数量 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BD_NUM_1_REG         (PPE_RCB_TQP_BASE + 0x248)  /* 发送Ring的BD数量 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BD_NUM_2_REG         (PPE_RCB_TQP_BASE + 0x448)  /* 发送Ring的BD数量 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BD_NUM_3_REG         (PPE_RCB_TQP_BASE + 0x648)  /* 发送Ring的BD数量 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BD_NUM_4_REG         (PPE_RCB_TQP_BASE + 0x848)  /* 发送Ring的BD数量 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BD_NUM_5_REG         (PPE_RCB_TQP_BASE + 0xA48)  /* 发送Ring的BD数量 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BD_NUM_6_REG         (PPE_RCB_TQP_BASE + 0xC48)  /* 发送Ring的BD数量 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BD_NUM_7_REG         (PPE_RCB_TQP_BASE + 0xE48)  /* 发送Ring的BD数量 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BD_NUM_8_REG         (PPE_RCB_TQP_BASE + 0x1048) /* 发送Ring的BD数量 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BD_NUM_9_REG         (PPE_RCB_TQP_BASE + 0x1248) /* 发送Ring的BD数量 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BD_NUM_10_REG        (PPE_RCB_TQP_BASE + 0x1448) /* 发送Ring的BD数量 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BD_NUM_11_REG        (PPE_RCB_TQP_BASE + 0x1648) /* 发送Ring的BD数量 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BD_NUM_12_REG        (PPE_RCB_TQP_BASE + 0x1848) /* 发送Ring的BD数量 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BD_NUM_13_REG        (PPE_RCB_TQP_BASE + 0x1A48) /* 发送Ring的BD数量 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BD_NUM_14_REG        (PPE_RCB_TQP_BASE + 0x1C48) /* 发送Ring的BD数量 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BD_NUM_15_REG        (PPE_RCB_TQP_BASE + 0x1E48) /* 发送Ring的BD数量 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_PRI_0_REG            (PPE_RCB_TQP_BASE + 0x4C)   /* 发送Ring的优先级配置 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_PRI_1_REG            (PPE_RCB_TQP_BASE + 0x24C)  /* 发送Ring的优先级配置 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_PRI_2_REG            (PPE_RCB_TQP_BASE + 0x44C)  /* 发送Ring的优先级配置 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_PRI_3_REG            (PPE_RCB_TQP_BASE + 0x64C)  /* 发送Ring的优先级配置 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_PRI_4_REG            (PPE_RCB_TQP_BASE + 0x84C)  /* 发送Ring的优先级配置 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_PRI_5_REG            (PPE_RCB_TQP_BASE + 0xA4C)  /* 发送Ring的优先级配置 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_PRI_6_REG            (PPE_RCB_TQP_BASE + 0xC4C)  /* 发送Ring的优先级配置 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_PRI_7_REG            (PPE_RCB_TQP_BASE + 0xE4C)  /* 发送Ring的优先级配置 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_PRI_8_REG            (PPE_RCB_TQP_BASE + 0x104C) /* 发送Ring的优先级配置 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_PRI_9_REG            (PPE_RCB_TQP_BASE + 0x124C) /* 发送Ring的优先级配置 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_PRI_10_REG           (PPE_RCB_TQP_BASE + 0x144C) /* 发送Ring的优先级配置 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_PRI_11_REG           (PPE_RCB_TQP_BASE + 0x164C) /* 发送Ring的优先级配置 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_PRI_12_REG           (PPE_RCB_TQP_BASE + 0x184C) /* 发送Ring的优先级配置 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_PRI_13_REG           (PPE_RCB_TQP_BASE + 0x1A4C) /* 发送Ring的优先级配置 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_PRI_14_REG           (PPE_RCB_TQP_BASE + 0x1C4C) /* 发送Ring的优先级配置 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_PRI_15_REG           (PPE_RCB_TQP_BASE + 0x1E4C) /* 发送Ring的优先级配置 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_TC_0_REG             (PPE_RCB_TQP_BASE + 0x50)   /* 发送Ring在MAC口内的TC号 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_TC_1_REG             (PPE_RCB_TQP_BASE + 0x250)  /* 发送Ring在MAC口内的TC号 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_TC_2_REG             (PPE_RCB_TQP_BASE + 0x450)  /* 发送Ring在MAC口内的TC号 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_TC_3_REG             (PPE_RCB_TQP_BASE + 0x650)  /* 发送Ring在MAC口内的TC号 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_TC_4_REG             (PPE_RCB_TQP_BASE + 0x850)  /* 发送Ring在MAC口内的TC号 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_TC_5_REG             (PPE_RCB_TQP_BASE + 0xA50)  /* 发送Ring在MAC口内的TC号 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_TC_6_REG             (PPE_RCB_TQP_BASE + 0xC50)  /* 发送Ring在MAC口内的TC号 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_TC_7_REG             (PPE_RCB_TQP_BASE + 0xE50)  /* 发送Ring在MAC口内的TC号 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_TC_8_REG             (PPE_RCB_TQP_BASE + 0x1050) /* 发送Ring在MAC口内的TC号 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_TC_9_REG             (PPE_RCB_TQP_BASE + 0x1250) /* 发送Ring在MAC口内的TC号 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_TC_10_REG            (PPE_RCB_TQP_BASE + 0x1450) /* 发送Ring在MAC口内的TC号 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_TC_11_REG            (PPE_RCB_TQP_BASE + 0x1650) /* 发送Ring在MAC口内的TC号 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_TC_12_REG            (PPE_RCB_TQP_BASE + 0x1850) /* 发送Ring在MAC口内的TC号 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_TC_13_REG            (PPE_RCB_TQP_BASE + 0x1A50) /* 发送Ring在MAC口内的TC号 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_TC_14_REG            (PPE_RCB_TQP_BASE + 0x1C50) /* 发送Ring在MAC口内的TC号 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_TC_15_REG            (PPE_RCB_TQP_BASE + 0x1E50) /* 发送Ring在MAC口内的TC号 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_MERGE_EN_0_REG       (PPE_RCB_TQP_BASE + 0x54)   /* 发送Ring回写BD merge使能 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_MERGE_EN_1_REG       (PPE_RCB_TQP_BASE + 0x254)  /* 发送Ring回写BD merge使能 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_MERGE_EN_2_REG       (PPE_RCB_TQP_BASE + 0x454)  /* 发送Ring回写BD merge使能 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_MERGE_EN_3_REG       (PPE_RCB_TQP_BASE + 0x654)  /* 发送Ring回写BD merge使能 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_MERGE_EN_4_REG       (PPE_RCB_TQP_BASE + 0x854)  /* 发送Ring回写BD merge使能 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_MERGE_EN_5_REG       (PPE_RCB_TQP_BASE + 0xA54)  /* 发送Ring回写BD merge使能 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_MERGE_EN_6_REG       (PPE_RCB_TQP_BASE + 0xC54)  /* 发送Ring回写BD merge使能 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_MERGE_EN_7_REG       (PPE_RCB_TQP_BASE + 0xE54)  /* 发送Ring回写BD merge使能 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_MERGE_EN_8_REG       (PPE_RCB_TQP_BASE + 0x1054) /* 发送Ring回写BD merge使能 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_MERGE_EN_9_REG       (PPE_RCB_TQP_BASE + 0x1254) /* 发送Ring回写BD merge使能 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_MERGE_EN_10_REG      (PPE_RCB_TQP_BASE + 0x1454) /* 发送Ring回写BD merge使能 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_MERGE_EN_11_REG      (PPE_RCB_TQP_BASE + 0x1654) /* 发送Ring回写BD merge使能 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_MERGE_EN_12_REG      (PPE_RCB_TQP_BASE + 0x1854) /* 发送Ring回写BD merge使能 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_MERGE_EN_13_REG      (PPE_RCB_TQP_BASE + 0x1A54) /* 发送Ring回写BD merge使能 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_MERGE_EN_14_REG      (PPE_RCB_TQP_BASE + 0x1C54) /* 发送Ring回写BD merge使能 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_MERGE_EN_15_REG      (PPE_RCB_TQP_BASE + 0x1E54) /* 发送Ring回写BD merge使能 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_TAIL_0_REG           (PPE_RCB_TQP_BASE + 0x58)   /* 发送Ring的tail指针 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_TAIL_1_REG           (PPE_RCB_TQP_BASE + 0x258)  /* 发送Ring的tail指针 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_TAIL_2_REG           (PPE_RCB_TQP_BASE + 0x458)  /* 发送Ring的tail指针 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_TAIL_3_REG           (PPE_RCB_TQP_BASE + 0x658)  /* 发送Ring的tail指针 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_TAIL_4_REG           (PPE_RCB_TQP_BASE + 0x858)  /* 发送Ring的tail指针 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_TAIL_5_REG           (PPE_RCB_TQP_BASE + 0xA58)  /* 发送Ring的tail指针 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_TAIL_6_REG           (PPE_RCB_TQP_BASE + 0xC58)  /* 发送Ring的tail指针 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_TAIL_7_REG           (PPE_RCB_TQP_BASE + 0xE58)  /* 发送Ring的tail指针 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_TAIL_8_REG           (PPE_RCB_TQP_BASE + 0x1058) /* 发送Ring的tail指针 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_TAIL_9_REG           (PPE_RCB_TQP_BASE + 0x1258) /* 发送Ring的tail指针 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_TAIL_10_REG          (PPE_RCB_TQP_BASE + 0x1458) /* 发送Ring的tail指针 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_TAIL_11_REG          (PPE_RCB_TQP_BASE + 0x1658) /* 发送Ring的tail指针 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_TAIL_12_REG          (PPE_RCB_TQP_BASE + 0x1858) /* 发送Ring的tail指针 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_TAIL_13_REG          (PPE_RCB_TQP_BASE + 0x1A58) /* 发送Ring的tail指针 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_TAIL_14_REG          (PPE_RCB_TQP_BASE + 0x1C58) /* 发送Ring的tail指针 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_TAIL_15_REG          (PPE_RCB_TQP_BASE + 0x1E58) /* 发送Ring的tail指针 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_HEAD_0_REG           (PPE_RCB_TQP_BASE + 0x5C)   /* 发送Ring的head指针 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_HEAD_1_REG           (PPE_RCB_TQP_BASE + 0x25C)  /* 发送Ring的head指针 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_HEAD_2_REG           (PPE_RCB_TQP_BASE + 0x45C)  /* 发送Ring的head指针 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_HEAD_3_REG           (PPE_RCB_TQP_BASE + 0x65C)  /* 发送Ring的head指针 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_HEAD_4_REG           (PPE_RCB_TQP_BASE + 0x85C)  /* 发送Ring的head指针 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_HEAD_5_REG           (PPE_RCB_TQP_BASE + 0xA5C)  /* 发送Ring的head指针 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_HEAD_6_REG           (PPE_RCB_TQP_BASE + 0xC5C)  /* 发送Ring的head指针 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_HEAD_7_REG           (PPE_RCB_TQP_BASE + 0xE5C)  /* 发送Ring的head指针 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_HEAD_8_REG           (PPE_RCB_TQP_BASE + 0x105C) /* 发送Ring的head指针 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_HEAD_9_REG           (PPE_RCB_TQP_BASE + 0x125C) /* 发送Ring的head指针 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_HEAD_10_REG          (PPE_RCB_TQP_BASE + 0x145C) /* 发送Ring的head指针 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_HEAD_11_REG          (PPE_RCB_TQP_BASE + 0x165C) /* 发送Ring的head指针 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_HEAD_12_REG          (PPE_RCB_TQP_BASE + 0x185C) /* 发送Ring的head指针 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_HEAD_13_REG          (PPE_RCB_TQP_BASE + 0x1A5C) /* 发送Ring的head指针 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_HEAD_14_REG          (PPE_RCB_TQP_BASE + 0x1C5C) /* 发送Ring的head指针 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_HEAD_15_REG          (PPE_RCB_TQP_BASE + 0x1E5C) /* 发送Ring的head指针 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_FBDNUM_0_REG         (PPE_RCB_TQP_BASE + 0x60)   /* 发送Ring的映射空间上F-BD的个数 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_FBDNUM_1_REG         (PPE_RCB_TQP_BASE + 0x260)  /* 发送Ring的映射空间上F-BD的个数 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_FBDNUM_2_REG         (PPE_RCB_TQP_BASE + 0x460)  /* 发送Ring的映射空间上F-BD的个数 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_FBDNUM_3_REG         (PPE_RCB_TQP_BASE + 0x660)  /* 发送Ring的映射空间上F-BD的个数 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_FBDNUM_4_REG         (PPE_RCB_TQP_BASE + 0x860)  /* 发送Ring的映射空间上F-BD的个数 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_FBDNUM_5_REG         (PPE_RCB_TQP_BASE + 0xA60)  /* 发送Ring的映射空间上F-BD的个数 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_FBDNUM_6_REG         (PPE_RCB_TQP_BASE + 0xC60)  /* 发送Ring的映射空间上F-BD的个数 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_FBDNUM_7_REG         (PPE_RCB_TQP_BASE + 0xE60)  /* 发送Ring的映射空间上F-BD的个数 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_FBDNUM_8_REG         (PPE_RCB_TQP_BASE + 0x1060) /* 发送Ring的映射空间上F-BD的个数 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_FBDNUM_9_REG         (PPE_RCB_TQP_BASE + 0x1260) /* 发送Ring的映射空间上F-BD的个数 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_FBDNUM_10_REG        (PPE_RCB_TQP_BASE + 0x1460) /* 发送Ring的映射空间上F-BD的个数 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_FBDNUM_11_REG        (PPE_RCB_TQP_BASE + 0x1660) /* 发送Ring的映射空间上F-BD的个数 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_FBDNUM_12_REG        (PPE_RCB_TQP_BASE + 0x1860) /* 发送Ring的映射空间上F-BD的个数 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_FBDNUM_13_REG        (PPE_RCB_TQP_BASE + 0x1A60) /* 发送Ring的映射空间上F-BD的个数 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_FBDNUM_14_REG        (PPE_RCB_TQP_BASE + 0x1C60) /* 发送Ring的映射空间上F-BD的个数 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_FBDNUM_15_REG        (PPE_RCB_TQP_BASE + 0x1E60) /* 发送Ring的映射空间上F-BD的个数 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_OFFSET_0_REG         (PPE_RCB_TQP_BASE + 0x64)   /* 发送Ring上head指针与第一个FBD的距离 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_OFFSET_1_REG         (PPE_RCB_TQP_BASE + 0x264)  /* 发送Ring上head指针与第一个FBD的距离 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_OFFSET_2_REG         (PPE_RCB_TQP_BASE + 0x464)  /* 发送Ring上head指针与第一个FBD的距离 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_OFFSET_3_REG         (PPE_RCB_TQP_BASE + 0x664)  /* 发送Ring上head指针与第一个FBD的距离 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_OFFSET_4_REG         (PPE_RCB_TQP_BASE + 0x864)  /* 发送Ring上head指针与第一个FBD的距离 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_OFFSET_5_REG         (PPE_RCB_TQP_BASE + 0xA64)  /* 发送Ring上head指针与第一个FBD的距离 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_OFFSET_6_REG         (PPE_RCB_TQP_BASE + 0xC64)  /* 发送Ring上head指针与第一个FBD的距离 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_OFFSET_7_REG         (PPE_RCB_TQP_BASE + 0xE64)  /* 发送Ring上head指针与第一个FBD的距离 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_OFFSET_8_REG         (PPE_RCB_TQP_BASE + 0x1064) /* 发送Ring上head指针与第一个FBD的距离 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_OFFSET_9_REG         (PPE_RCB_TQP_BASE + 0x1264) /* 发送Ring上head指针与第一个FBD的距离 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_OFFSET_10_REG        (PPE_RCB_TQP_BASE + 0x1464) /* 发送Ring上head指针与第一个FBD的距离 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_OFFSET_11_REG        (PPE_RCB_TQP_BASE + 0x1664) /* 发送Ring上head指针与第一个FBD的距离 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_OFFSET_12_REG        (PPE_RCB_TQP_BASE + 0x1864) /* 发送Ring上head指针与第一个FBD的距离 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_OFFSET_13_REG        (PPE_RCB_TQP_BASE + 0x1A64) /* 发送Ring上head指针与第一个FBD的距离 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_OFFSET_14_REG        (PPE_RCB_TQP_BASE + 0x1C64) /* 发送Ring上head指针与第一个FBD的距离 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_OFFSET_15_REG        (PPE_RCB_TQP_BASE + 0x1E64) /* 发送Ring上head指针与第一个FBD的距离 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_EBDNUM_0_REG         (PPE_RCB_TQP_BASE + 0x68)   /* 发送Ring上EBD的数量 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_EBDNUM_1_REG         (PPE_RCB_TQP_BASE + 0x268)  /* 发送Ring上EBD的数量 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_EBDNUM_2_REG         (PPE_RCB_TQP_BASE + 0x468)  /* 发送Ring上EBD的数量 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_EBDNUM_3_REG         (PPE_RCB_TQP_BASE + 0x668)  /* 发送Ring上EBD的数量 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_EBDNUM_4_REG         (PPE_RCB_TQP_BASE + 0x868)  /* 发送Ring上EBD的数量 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_EBDNUM_5_REG         (PPE_RCB_TQP_BASE + 0xA68)  /* 发送Ring上EBD的数量 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_EBDNUM_6_REG         (PPE_RCB_TQP_BASE + 0xC68)  /* 发送Ring上EBD的数量 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_EBDNUM_7_REG         (PPE_RCB_TQP_BASE + 0xE68)  /* 发送Ring上EBD的数量 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_EBDNUM_8_REG         (PPE_RCB_TQP_BASE + 0x1068) /* 发送Ring上EBD的数量 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_EBDNUM_9_REG         (PPE_RCB_TQP_BASE + 0x1268) /* 发送Ring上EBD的数量 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_EBDNUM_10_REG        (PPE_RCB_TQP_BASE + 0x1468) /* 发送Ring上EBD的数量 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_EBDNUM_11_REG        (PPE_RCB_TQP_BASE + 0x1668) /* 发送Ring上EBD的数量 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_EBDNUM_12_REG        (PPE_RCB_TQP_BASE + 0x1868) /* 发送Ring上EBD的数量 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_EBDNUM_13_REG        (PPE_RCB_TQP_BASE + 0x1A68) /* 发送Ring上EBD的数量 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_EBDNUM_14_REG        (PPE_RCB_TQP_BASE + 0x1C68) /* 发送Ring上EBD的数量 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_EBDNUM_15_REG        (PPE_RCB_TQP_BASE + 0x1E68) /* 发送Ring上EBD的数量 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_PKTNUM_RECORD_0_REG  (PPE_RCB_TQP_BASE + 0x6C)   /* 发送Ring发送包的统计 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_PKTNUM_RECORD_1_REG  (PPE_RCB_TQP_BASE + 0x26C)  /* 发送Ring发送包的统计 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_PKTNUM_RECORD_2_REG  (PPE_RCB_TQP_BASE + 0x46C)  /* 发送Ring发送包的统计 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_PKTNUM_RECORD_3_REG  (PPE_RCB_TQP_BASE + 0x66C)  /* 发送Ring发送包的统计 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_PKTNUM_RECORD_4_REG  (PPE_RCB_TQP_BASE + 0x86C)  /* 发送Ring发送包的统计 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_PKTNUM_RECORD_5_REG  (PPE_RCB_TQP_BASE + 0xA6C)  /* 发送Ring发送包的统计 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_PKTNUM_RECORD_6_REG  (PPE_RCB_TQP_BASE + 0xC6C)  /* 发送Ring发送包的统计 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_PKTNUM_RECORD_7_REG  (PPE_RCB_TQP_BASE + 0xE6C)  /* 发送Ring发送包的统计 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_PKTNUM_RECORD_8_REG  (PPE_RCB_TQP_BASE + 0x106C) /* 发送Ring发送包的统计 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_PKTNUM_RECORD_9_REG  (PPE_RCB_TQP_BASE + 0x126C) /* 发送Ring发送包的统计 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_PKTNUM_RECORD_10_REG (PPE_RCB_TQP_BASE + 0x146C) /* 发送Ring发送包的统计 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_PKTNUM_RECORD_11_REG (PPE_RCB_TQP_BASE + 0x166C) /* 发送Ring发送包的统计 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_PKTNUM_RECORD_12_REG (PPE_RCB_TQP_BASE + 0x186C) /* 发送Ring发送包的统计 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_PKTNUM_RECORD_13_REG (PPE_RCB_TQP_BASE + 0x1A6C) /* 发送Ring发送包的统计 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_PKTNUM_RECORD_14_REG (PPE_RCB_TQP_BASE + 0x1C6C) /* 发送Ring发送包的统计 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_PKTNUM_RECORD_15_REG (PPE_RCB_TQP_BASE + 0x1E6C) /* 发送Ring发送包的统计 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_EBD_OFFSET_0_REG     (PPE_RCB_TQP_BASE + 0x70)   /* 发送Ring上head指针与第一个FBD的距离 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_EBD_OFFSET_1_REG     (PPE_RCB_TQP_BASE + 0x270)  /* 发送Ring上head指针与第一个FBD的距离 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_EBD_OFFSET_2_REG     (PPE_RCB_TQP_BASE + 0x470)  /* 发送Ring上head指针与第一个FBD的距离 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_EBD_OFFSET_3_REG     (PPE_RCB_TQP_BASE + 0x670)  /* 发送Ring上head指针与第一个FBD的距离 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_EBD_OFFSET_4_REG     (PPE_RCB_TQP_BASE + 0x870)  /* 发送Ring上head指针与第一个FBD的距离 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_EBD_OFFSET_5_REG     (PPE_RCB_TQP_BASE + 0xA70)  /* 发送Ring上head指针与第一个FBD的距离 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_EBD_OFFSET_6_REG     (PPE_RCB_TQP_BASE + 0xC70)  /* 发送Ring上head指针与第一个FBD的距离 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_EBD_OFFSET_7_REG     (PPE_RCB_TQP_BASE + 0xE70)  /* 发送Ring上head指针与第一个FBD的距离 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_EBD_OFFSET_8_REG     (PPE_RCB_TQP_BASE + 0x1070) /* 发送Ring上head指针与第一个FBD的距离 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_EBD_OFFSET_9_REG     (PPE_RCB_TQP_BASE + 0x1270) /* 发送Ring上head指针与第一个FBD的距离 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_EBD_OFFSET_10_REG    (PPE_RCB_TQP_BASE + 0x1470) /* 发送Ring上head指针与第一个FBD的距离 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_EBD_OFFSET_11_REG    (PPE_RCB_TQP_BASE + 0x1670) /* 发送Ring上head指针与第一个FBD的距离 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_EBD_OFFSET_12_REG    (PPE_RCB_TQP_BASE + 0x1870) /* 发送Ring上head指针与第一个FBD的距离 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_EBD_OFFSET_13_REG    (PPE_RCB_TQP_BASE + 0x1A70) /* 发送Ring上head指针与第一个FBD的距离 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_EBD_OFFSET_14_REG    (PPE_RCB_TQP_BASE + 0x1C70) /* 发送Ring上head指针与第一个FBD的距离 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_EBD_OFFSET_15_REG    (PPE_RCB_TQP_BASE + 0x1E70) /* 发送Ring上head指针与第一个FBD的距离 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BD_ERR_0_REG         (PPE_RCB_TQP_BASE + 0x74)   /* 发送Ring上BD ERROR状态 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BD_ERR_1_REG         (PPE_RCB_TQP_BASE + 0x274)  /* 发送Ring上BD ERROR状态 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BD_ERR_2_REG         (PPE_RCB_TQP_BASE + 0x474)  /* 发送Ring上BD ERROR状态 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BD_ERR_3_REG         (PPE_RCB_TQP_BASE + 0x674)  /* 发送Ring上BD ERROR状态 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BD_ERR_4_REG         (PPE_RCB_TQP_BASE + 0x874)  /* 发送Ring上BD ERROR状态 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BD_ERR_5_REG         (PPE_RCB_TQP_BASE + 0xA74)  /* 发送Ring上BD ERROR状态 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BD_ERR_6_REG         (PPE_RCB_TQP_BASE + 0xC74)  /* 发送Ring上BD ERROR状态 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BD_ERR_7_REG         (PPE_RCB_TQP_BASE + 0xE74)  /* 发送Ring上BD ERROR状态 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BD_ERR_8_REG         (PPE_RCB_TQP_BASE + 0x1074) /* 发送Ring上BD ERROR状态 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BD_ERR_9_REG         (PPE_RCB_TQP_BASE + 0x1274) /* 发送Ring上BD ERROR状态 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BD_ERR_10_REG        (PPE_RCB_TQP_BASE + 0x1474) /* 发送Ring上BD ERROR状态 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BD_ERR_11_REG        (PPE_RCB_TQP_BASE + 0x1674) /* 发送Ring上BD ERROR状态 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BD_ERR_12_REG        (PPE_RCB_TQP_BASE + 0x1874) /* 发送Ring上BD ERROR状态 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BD_ERR_13_REG        (PPE_RCB_TQP_BASE + 0x1A74) /* 发送Ring上BD ERROR状态 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BD_ERR_14_REG        (PPE_RCB_TQP_BASE + 0x1C74) /* 发送Ring上BD ERROR状态 */
#define PPE_RCB_TQP_RCB_CFG_TX_RING_BD_ERR_15_REG        (PPE_RCB_TQP_BASE + 0x1E74) /* 发送Ring上BD ERROR状态 */
#define PPE_RCB_TQP_RCB_CFG_EN_0_REG                     (PPE_RCB_TQP_BASE + 0x90)   /* 队列使能 */
#define PPE_RCB_TQP_RCB_CFG_EN_1_REG                     (PPE_RCB_TQP_BASE + 0x290)  /* 队列使能 */
#define PPE_RCB_TQP_RCB_CFG_EN_2_REG                     (PPE_RCB_TQP_BASE + 0x490)  /* 队列使能 */
#define PPE_RCB_TQP_RCB_CFG_EN_3_REG                     (PPE_RCB_TQP_BASE + 0x690)  /* 队列使能 */
#define PPE_RCB_TQP_RCB_CFG_EN_4_REG                     (PPE_RCB_TQP_BASE + 0x890)  /* 队列使能 */
#define PPE_RCB_TQP_RCB_CFG_EN_5_REG                     (PPE_RCB_TQP_BASE + 0xA90)  /* 队列使能 */
#define PPE_RCB_TQP_RCB_CFG_EN_6_REG                     (PPE_RCB_TQP_BASE + 0xC90)  /* 队列使能 */
#define PPE_RCB_TQP_RCB_CFG_EN_7_REG                     (PPE_RCB_TQP_BASE + 0xE90)  /* 队列使能 */
#define PPE_RCB_TQP_RCB_CFG_EN_8_REG                     (PPE_RCB_TQP_BASE + 0x1090) /* 队列使能 */
#define PPE_RCB_TQP_RCB_CFG_EN_9_REG                     (PPE_RCB_TQP_BASE + 0x1290) /* 队列使能 */
#define PPE_RCB_TQP_RCB_CFG_EN_10_REG                    (PPE_RCB_TQP_BASE + 0x1490) /* 队列使能 */
#define PPE_RCB_TQP_RCB_CFG_EN_11_REG                    (PPE_RCB_TQP_BASE + 0x1690) /* 队列使能 */
#define PPE_RCB_TQP_RCB_CFG_EN_12_REG                    (PPE_RCB_TQP_BASE + 0x1890) /* 队列使能 */
#define PPE_RCB_TQP_RCB_CFG_EN_13_REG                    (PPE_RCB_TQP_BASE + 0x1A90) /* 队列使能 */
#define PPE_RCB_TQP_RCB_CFG_EN_14_REG                    (PPE_RCB_TQP_BASE + 0x1C90) /* 队列使能 */
#define PPE_RCB_TQP_RCB_CFG_EN_15_REG                    (PPE_RCB_TQP_BASE + 0x1E90) /* 队列使能 */

#endif // __REG_RCB_TQP_OFFSET_H__
