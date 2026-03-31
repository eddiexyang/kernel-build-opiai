/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: reg_rcb_com_offset
 * Author: huawei
 * Create: 2021-12-28
 */

#ifndef __REG_RCB_COM_OFFSET_H__
#define __REG_RCB_COM_OFFSET_H__

/* PPE_RCB_COM Base address of Module's Register */
#define PPE_RCB_COM_BASE                       (0x0)

/******************************************************************************/
/*                      xxx PPE_RCB_COM Registers' Definitions                */
/******************************************************************************/

#define PPE_RCB_COM_RCB_CFG_ENDIAN_REG                (PPE_RCB_COM_BASE + 0x0)    /* Ring中BD在DDR中存储大小端配置 */
#define PPE_RCB_COM_RCB_INIT_EN_REG                   (PPE_RCB_COM_BASE + 0xC)    /* 软件配置内部SRAM初始化使能 */
#define PPE_RCB_COM_RCB_CFG_INIT_FLAG_REG             (PPE_RCB_COM_BASE + 0x10)   /* RCB内部的SRAM初始化完成 */
#define PPE_RCB_COM_RCB_USER_CFG0_0_REG               (PPE_RCB_COM_BASE + 0x400)  /* RCB读写描述符的user控制 */
#define PPE_RCB_COM_RCB_USER_CFG0_1_REG               (PPE_RCB_COM_BASE + 0x420)  /* RCB读写描述符的user控制 */
#define PPE_RCB_COM_RCB_USER_CFG0_2_REG               (PPE_RCB_COM_BASE + 0x440)  /* RCB读写描述符的user控制 */
#define PPE_RCB_COM_RCB_USER_CFG0_3_REG               (PPE_RCB_COM_BASE + 0x460)  /* RCB读写描述符的user控制 */
#define PPE_RCB_COM_RCB_USER_CFG1_0_REG               (PPE_RCB_COM_BASE + 0x500)  /* PPU SMMU相关的user控制 */
#define PPE_RCB_COM_RCB_USER_CFG1_1_REG               (PPE_RCB_COM_BASE + 0x520)  /* PPU SMMU相关的user控制 */
#define PPE_RCB_COM_RCB_USER_CFG1_2_REG               (PPE_RCB_COM_BASE + 0x540)  /* PPU SMMU相关的user控制 */
#define PPE_RCB_COM_RCB_USER_CFG1_3_REG               (PPE_RCB_COM_BASE + 0x560)  /* PPU SMMU相关的user控制 */
#define PPE_RCB_COM_STASH_PKT_WL_0_REG                (PPE_RCB_COM_BASE + 0x508)  /* 接收方向stash水线配置 */
#define PPE_RCB_COM_STASH_PKT_WL_1_REG                (PPE_RCB_COM_BASE + 0x528)  /* 接收方向stash水线配置 */
#define PPE_RCB_COM_STASH_PKT_WL_2_REG                (PPE_RCB_COM_BASE + 0x548)  /* 接收方向stash水线配置 */
#define PPE_RCB_COM_STASH_PKT_WL_3_REG                (PPE_RCB_COM_BASE + 0x568)  /* 接收方向stash水线配置 */
#define PPE_RCB_COM_FA_PKT_WL_0_REG                   (PPE_RCB_COM_BASE + 0x50C)  /* FA水线配置 */
#define PPE_RCB_COM_FA_PKT_WL_1_REG                   (PPE_RCB_COM_BASE + 0x52C)  /* FA水线配置 */
#define PPE_RCB_COM_FA_PKT_WL_2_REG                   (PPE_RCB_COM_BASE + 0x54C)  /* FA水线配置 */
#define PPE_RCB_COM_FA_PKT_WL_3_REG                   (PPE_RCB_COM_BASE + 0x56C)  /* FA水线配置 */
#define PPE_RCB_COM_USER_SEL_REG                      (PPE_RCB_COM_BASE + 0x900)  /* 性能相关USER选择配置 */
#define PPE_RCB_COM_RCB_AXQOS_CFG_REG                 (PPE_RCB_COM_BASE + 0x904)  /* AXQOS配置 */
#define PPE_RCB_COM_RCB_AXCACHE_CFG_REG               (PPE_RCB_COM_BASE + 0x908)  /* AXCACHE配置 */
#define PPE_RCB_COM_RCB_CLK_GATING_CTRL_REG           (PPE_RCB_COM_BASE + 0x980)  /* 时钟自动门控使能寄存器。 */
/* 统计门控打开的窗口大小，单位是64K个该模块工作时钟的。 */
#define PPE_RCB_COM_RCB_GATING_WINDOW_REG             (PPE_RCB_COM_BASE + 0x984)

/* 统计门控打开的窗口中打开时钟的周期，单位是64K个该模块工作时钟的。 */
#define PPE_RCB_COM_RCB_TX_GATING_CNT_REG             (PPE_RCB_COM_BASE + 0x988)
#define PPE_RCB_COM_RCB_RX_GATING_CNT_REG             (PPE_RCB_COM_BASE + 0x98C)
#define PPE_RCB_COM_RCB_SLV_GATING_CNT_REG            (PPE_RCB_COM_BASE + 0x990)

/* 指示门控打开的窗口中时钟门控是否有效 */
#define PPE_RCB_COM_RCB_GATING_STS_REG                (PPE_RCB_COM_BASE + 0x994)
#define PPE_RCB_COM_RCB_USER_SSV_0_REG                (PPE_RCB_COM_BASE + 0xA00)  /* USER域中的SSV配置 */
#define PPE_RCB_COM_RCB_USER_SSV_1_REG                (PPE_RCB_COM_BASE + 0xA10)  /* USER域中的SSV配置 */
#define PPE_RCB_COM_RCB_USER_SSV_2_REG                (PPE_RCB_COM_BASE + 0xA20)  /* USER域中的SSV配置 */
#define PPE_RCB_COM_RCB_USER_SSV_3_REG                (PPE_RCB_COM_BASE + 0xA30)  /* USER域中的SSV配置 */
#define PPE_RCB_COM_RCB_USER_SSV_4_REG                (PPE_RCB_COM_BASE + 0xA40)  /* USER域中的SSV配置 */
#define PPE_RCB_COM_RCB_USER_SSV_5_REG                (PPE_RCB_COM_BASE + 0xA50)  /* USER域中的SSV配置 */
#define PPE_RCB_COM_RCB_USER_SSV_6_REG                (PPE_RCB_COM_BASE + 0xA60)  /* USER域中的SSV配置 */
#define PPE_RCB_COM_RCB_USER_SSV_7_REG                (PPE_RCB_COM_BASE + 0xA70)  /* USER域中的SSV配置 */
#define PPE_RCB_COM_RCB_USER_SSV_8_REG                (PPE_RCB_COM_BASE + 0xA80)  /* USER域中的SSV配置 */
#define PPE_RCB_COM_RCB_USER_SSV_9_REG                (PPE_RCB_COM_BASE + 0xA90)  /* USER域中的SSV配置 */
#define PPE_RCB_COM_RCB_USER_SSV_10_REG               (PPE_RCB_COM_BASE + 0xAA0)  /* USER域中的SSV配置 */
#define PPE_RCB_COM_RCB_USER_SSV_11_REG               (PPE_RCB_COM_BASE + 0xAB0)  /* USER域中的SSV配置 */
#define PPE_RCB_COM_RCB_USER_SSV_12_REG               (PPE_RCB_COM_BASE + 0xAC0)  /* USER域中的SSV配置 */
#define PPE_RCB_COM_RCB_USER_SSV_13_REG               (PPE_RCB_COM_BASE + 0xAD0)  /* USER域中的SSV配置 */
#define PPE_RCB_COM_RCB_USER_SSV_14_REG               (PPE_RCB_COM_BASE + 0xAE0)  /* USER域中的SSV配置 */
#define PPE_RCB_COM_RCB_USER_SSV_15_REG               (PPE_RCB_COM_BASE + 0xAF0)  /* USER域中的SSV配置 */
#define PPE_RCB_COM_RCB_USER_STRMID_0_REG             (PPE_RCB_COM_BASE + 0xA04)  /* USER域中的STREAM_ID配置 */
#define PPE_RCB_COM_RCB_USER_STRMID_1_REG             (PPE_RCB_COM_BASE + 0xA14)  /* USER域中的STREAM_ID配置 */
#define PPE_RCB_COM_RCB_USER_STRMID_2_REG             (PPE_RCB_COM_BASE + 0xA24)  /* USER域中的STREAM_ID配置 */
#define PPE_RCB_COM_RCB_USER_STRMID_3_REG             (PPE_RCB_COM_BASE + 0xA34)  /* USER域中的STREAM_ID配置 */
#define PPE_RCB_COM_RCB_USER_STRMID_4_REG             (PPE_RCB_COM_BASE + 0xA44)  /* USER域中的STREAM_ID配置 */
#define PPE_RCB_COM_RCB_USER_STRMID_5_REG             (PPE_RCB_COM_BASE + 0xA54)  /* USER域中的STREAM_ID配置 */
#define PPE_RCB_COM_RCB_USER_STRMID_6_REG             (PPE_RCB_COM_BASE + 0xA64)  /* USER域中的STREAM_ID配置 */
#define PPE_RCB_COM_RCB_USER_STRMID_7_REG             (PPE_RCB_COM_BASE + 0xA74)  /* USER域中的STREAM_ID配置 */
#define PPE_RCB_COM_RCB_USER_STRMID_8_REG             (PPE_RCB_COM_BASE + 0xA84)  /* USER域中的STREAM_ID配置 */
#define PPE_RCB_COM_RCB_USER_STRMID_9_REG             (PPE_RCB_COM_BASE + 0xA94)  /* USER域中的STREAM_ID配置 */
#define PPE_RCB_COM_RCB_USER_STRMID_10_REG            (PPE_RCB_COM_BASE + 0xAA4)  /* USER域中的STREAM_ID配置 */
#define PPE_RCB_COM_RCB_USER_STRMID_11_REG            (PPE_RCB_COM_BASE + 0xAB4)  /* USER域中的STREAM_ID配置 */
#define PPE_RCB_COM_RCB_USER_STRMID_12_REG            (PPE_RCB_COM_BASE + 0xAC4)  /* USER域中的STREAM_ID配置 */
#define PPE_RCB_COM_RCB_USER_STRMID_13_REG            (PPE_RCB_COM_BASE + 0xAD4)  /* USER域中的STREAM_ID配置 */
#define PPE_RCB_COM_RCB_USER_STRMID_14_REG            (PPE_RCB_COM_BASE + 0xAE4)  /* USER域中的STREAM_ID配置 */
#define PPE_RCB_COM_RCB_USER_STRMID_15_REG            (PPE_RCB_COM_BASE + 0xAF4)  /* USER域中的STREAM_ID配置 */
#define PPE_RCB_COM_TSO_MSS_MIN_REG                   (PPE_RCB_COM_BASE + 0xC00)  /* TSO的MSS长度下限 */
#define PPE_RCB_COM_TSO_MSS_MAX_REG                   (PPE_RCB_COM_BASE + 0xC04)  /* TSO的MSS长度上限 */
#define PPE_RCB_COM_PPU_RX_DROP_CFG0_REG              (PPE_RCB_COM_BASE + 0xC10)  /* 接收方向报文丢弃配置寄存器 */
#define PPE_RCB_COM_PPU_TO_BE_RST_REG                 (PPE_RCB_COM_BASE + 0xC18)  /* 软件需要复位PPU模块的使能信号 */
#define PPE_RCB_COM_PPU_COULD_BE_RST_REG              (PPE_RCB_COM_BASE + 0xC1C)  /* PPU模块可以复位状态指示 */
#define PPE_RCB_COM_PPU_FUN_RST_FLAG_REG              (PPE_RCB_COM_BASE + 0xC20)  /* 软件需要function复位的指示信号 */
#define PPE_RCB_COM_SC_MEM_CTRL_SP_REG                (PPE_RCB_COM_BASE + 0xC24)  /* PPU MEMORY控制寄存器 */
#define PPE_RCB_COM_SC_MEM_CTRL_TP_REG                (PPE_RCB_COM_BASE + 0xC28)  /* PPU MEMORY控制寄存器 */
#define PPE_RCB_COM_SC_MEM_CTRL_POWER_REG             (PPE_RCB_COM_BASE + 0xC2C)  /* PPU MEMORY控制寄存器 */
#define PPE_RCB_COM_TCAM_MEM_CTRL_REG                 (PPE_RCB_COM_BASE + 0xC30)  /* TCAM MEMORY控制寄存器 */
#define PPE_RCB_COM_TM_BYPASS_EN_REG                  (PPE_RCB_COM_BASE + 0xC80)  /* TM调度bypass使能 */
#define PPE_RCB_COM_ECC_BYPASS_EN_REG                 (PPE_RCB_COM_BASE + 0xC8C)  /* memory ecc bypass使能 */
#define PPE_RCB_COM_TCAM_ECC_BYPASS_EN_REG            (PPE_RCB_COM_BASE + 0xC90)  /* tcam ecc bypass使能 */
#define PPE_RCB_COM_CNT_CLR_CE_REG                    (PPE_RCB_COM_BASE + 0xC94)  /* CNT_CYC类型寄存器读清使能 */
#define PPE_RCB_COM_TM_COMP_QID_REG                   (PPE_RCB_COM_BASE + 0xC98)  /* RCB向TM补偿队列选择 */
#define PPE_RCB_COM_RCB_TQP_MAP_ECC_DATA_GEN_REG      (PPE_RCB_COM_BASE + 0xC9C)  /* TCAM ECC值寄存器 */
#define PPE_RCB_COM_RCB_TQP_MAP_ECC_DATA_CHK0_REG     (PPE_RCB_COM_BASE + 0xCA0)  /* TCAM ECC值配置寄存器 */
#define PPE_RCB_COM_RCB_TQP_MAP_ECC_DATA_CHK1_REG     (PPE_RCB_COM_BASE + 0xCA4)  /* TCAM ECC状态寄存器 */
#define PPE_RCB_COM_FSM_DFX_ST0_REG                   (PPE_RCB_COM_BASE + 0x1000) /* 内部状态机状态 */
#define PPE_RCB_COM_FSM_DFX_ST1_REG                   (PPE_RCB_COM_BASE + 0x1004) /* 内部状态机状态 */
#define PPE_RCB_COM_FSM_DFX_ST2_REG                   (PPE_RCB_COM_BASE + 0x1008) /* 内部状态机状态 */
#define PPE_RCB_COM_FIFO_DFX_ST0_REG                  (PPE_RCB_COM_BASE + 0x100C) /* 内部FIFO空满状态 */
#define PPE_RCB_COM_FIFO_DFX_ST1_REG                  (PPE_RCB_COM_BASE + 0x1010) /* 内部FIFO空满状态 */
#define PPE_RCB_COM_FIFO_DFX_ST2_REG                  (PPE_RCB_COM_BASE + 0x1014) /* 内部FIFO空满状态 */
#define PPE_RCB_COM_FIFO_DFX_ST3_REG                  (PPE_RCB_COM_BASE + 0x1018) /* 内部FIFO空满状态 */
#define PPE_RCB_COM_RCB_INNER_ST_REG                  (PPE_RCB_COM_BASE + 0x101C) /* 内部工作状态 */
#define PPE_RCB_COM_FIFO_DFX_ST4_0_REG                (PPE_RCB_COM_BASE + 0x1020) /* 内部FIFO空满状态 */
#define PPE_RCB_COM_FIFO_DFX_ST4_1_REG                (PPE_RCB_COM_BASE + 0x1024) /* 内部FIFO空满状态 */
#define PPE_RCB_COM_FIFO_DFX_ST4_2_REG                (PPE_RCB_COM_BASE + 0x1028) /* 内部FIFO空满状态 */
#define PPE_RCB_COM_FIFO_DFX_ST4_3_REG                (PPE_RCB_COM_BASE + 0x102C) /* 内部FIFO空满状态 */
#define PPE_RCB_COM_FIFO_DFX_ST5_0_REG                (PPE_RCB_COM_BASE + 0x1080) /* 内部FIFO空满状态 */
#define PPE_RCB_COM_FIFO_DFX_ST5_1_REG                (PPE_RCB_COM_BASE + 0x1084) /* 内部FIFO空满状态 */
#define PPE_RCB_COM_FIFO_DFX_ST5_2_REG                (PPE_RCB_COM_BASE + 0x1088) /* 内部FIFO空满状态 */
#define PPE_RCB_COM_FIFO_DFX_ST5_3_REG                (PPE_RCB_COM_BASE + 0x108C) /* 内部FIFO空满状态 */
#define PPE_RCB_COM_Q_CREDIT_VLD_0_REG                (PPE_RCB_COM_BASE + 0x1200) /* 队列当前credit状态 */
#define PPE_RCB_COM_Q_CREDIT_VLD_1_REG                (PPE_RCB_COM_BASE + 0x1204) /* 队列当前credit状态 */
#define PPE_RCB_COM_Q_CREDIT_VLD_2_REG                (PPE_RCB_COM_BASE + 0x1208) /* 队列当前credit状态 */
#define PPE_RCB_COM_Q_CREDIT_VLD_3_REG                (PPE_RCB_COM_BASE + 0x120C) /* 队列当前credit状态 */
#define PPE_RCB_COM_Q_CREDIT_VLD_4_REG                (PPE_RCB_COM_BASE + 0x1210) /* 队列当前credit状态 */
#define PPE_RCB_COM_Q_CREDIT_VLD_5_REG                (PPE_RCB_COM_BASE + 0x1214) /* 队列当前credit状态 */
#define PPE_RCB_COM_Q_CREDIT_VLD_6_REG                (PPE_RCB_COM_BASE + 0x1218) /* 队列当前credit状态 */
#define PPE_RCB_COM_Q_CREDIT_VLD_7_REG                (PPE_RCB_COM_BASE + 0x121C) /* 队列当前credit状态 */
#define PPE_RCB_COM_Q_CREDIT_VLD_8_REG                (PPE_RCB_COM_BASE + 0x1220) /* 队列当前credit状态 */
#define PPE_RCB_COM_Q_CREDIT_VLD_9_REG                (PPE_RCB_COM_BASE + 0x1224) /* 队列当前credit状态 */
#define PPE_RCB_COM_Q_CREDIT_VLD_10_REG               (PPE_RCB_COM_BASE + 0x1228) /* 队列当前credit状态 */
#define PPE_RCB_COM_Q_CREDIT_VLD_11_REG               (PPE_RCB_COM_BASE + 0x122C) /* 队列当前credit状态 */
#define PPE_RCB_COM_Q_CREDIT_VLD_12_REG               (PPE_RCB_COM_BASE + 0x1230) /* 队列当前credit状态 */
#define PPE_RCB_COM_Q_CREDIT_VLD_13_REG               (PPE_RCB_COM_BASE + 0x1234) /* 队列当前credit状态 */
#define PPE_RCB_COM_Q_CREDIT_VLD_14_REG               (PPE_RCB_COM_BASE + 0x1238) /* 队列当前credit状态 */
#define PPE_RCB_COM_Q_CREDIT_VLD_15_REG               (PPE_RCB_COM_BASE + 0x123C) /* 队列当前credit状态 */
#define PPE_RCB_COM_Q_CREDIT_VLD_16_REG               (PPE_RCB_COM_BASE + 0x1240) /* 队列当前credit状态 */
#define PPE_RCB_COM_Q_CREDIT_VLD_17_REG               (PPE_RCB_COM_BASE + 0x1244) /* 队列当前credit状态 */
#define PPE_RCB_COM_Q_CREDIT_VLD_18_REG               (PPE_RCB_COM_BASE + 0x1248) /* 队列当前credit状态 */
#define PPE_RCB_COM_Q_CREDIT_VLD_19_REG               (PPE_RCB_COM_BASE + 0x124C) /* 队列当前credit状态 */
#define PPE_RCB_COM_Q_CREDIT_VLD_20_REG               (PPE_RCB_COM_BASE + 0x1250) /* 队列当前credit状态 */
#define PPE_RCB_COM_Q_CREDIT_VLD_21_REG               (PPE_RCB_COM_BASE + 0x1254) /* 队列当前credit状态 */
#define PPE_RCB_COM_Q_CREDIT_VLD_22_REG               (PPE_RCB_COM_BASE + 0x1258) /* 队列当前credit状态 */
#define PPE_RCB_COM_Q_CREDIT_VLD_23_REG               (PPE_RCB_COM_BASE + 0x125C) /* 队列当前credit状态 */
#define PPE_RCB_COM_Q_CREDIT_VLD_24_REG               (PPE_RCB_COM_BASE + 0x1260) /* 队列当前credit状态 */
#define PPE_RCB_COM_Q_CREDIT_VLD_25_REG               (PPE_RCB_COM_BASE + 0x1264) /* 队列当前credit状态 */
#define PPE_RCB_COM_Q_CREDIT_VLD_26_REG               (PPE_RCB_COM_BASE + 0x1268) /* 队列当前credit状态 */
#define PPE_RCB_COM_Q_CREDIT_VLD_27_REG               (PPE_RCB_COM_BASE + 0x126C) /* 队列当前credit状态 */
#define PPE_RCB_COM_Q_CREDIT_VLD_28_REG               (PPE_RCB_COM_BASE + 0x1270) /* 队列当前credit状态 */
#define PPE_RCB_COM_Q_CREDIT_VLD_29_REG               (PPE_RCB_COM_BASE + 0x1274) /* 队列当前credit状态 */
#define PPE_RCB_COM_Q_CREDIT_VLD_30_REG               (PPE_RCB_COM_BASE + 0x1278) /* 队列当前credit状态 */
#define PPE_RCB_COM_Q_CREDIT_VLD_31_REG               (PPE_RCB_COM_BASE + 0x127C) /* 队列当前credit状态 */
#define PPE_RCB_COM_Q_CREDIT_VLD_32_REG               (PPE_RCB_COM_BASE + 0x1280) /* 队列当前credit状态 */
#define PPE_RCB_COM_Q_CREDIT_VLD_33_REG               (PPE_RCB_COM_BASE + 0x1284) /* 队列当前credit状态 */
#define PPE_RCB_COM_Q_CREDIT_VLD_34_REG               (PPE_RCB_COM_BASE + 0x1288) /* 队列当前credit状态 */
#define PPE_RCB_COM_Q_CREDIT_VLD_35_REG               (PPE_RCB_COM_BASE + 0x128C) /* 队列当前credit状态 */
#define PPE_RCB_COM_Q_CREDIT_VLD_36_REG               (PPE_RCB_COM_BASE + 0x1290) /* 队列当前credit状态 */
#define PPE_RCB_COM_Q_CREDIT_VLD_37_REG               (PPE_RCB_COM_BASE + 0x1294) /* 队列当前credit状态 */
#define PPE_RCB_COM_Q_CREDIT_VLD_38_REG               (PPE_RCB_COM_BASE + 0x1298) /* 队列当前credit状态 */
#define PPE_RCB_COM_Q_CREDIT_VLD_39_REG               (PPE_RCB_COM_BASE + 0x129C) /* 队列当前credit状态 */
#define PPE_RCB_COM_MPF_ABNORMAL_INT_TYPE_REG         (PPE_RCB_COM_BASE + 0x1300) /* 多PF异常中断类型寄存器 */
#define PPE_RCB_COM_MPF_ABNORMAL_INT_EN_REG           (PPE_RCB_COM_BASE + 0x1310) /* 多PF异常中断使能寄存器 */
#define PPE_RCB_COM_MPF_ABNORMAL_INT_ST_REG           (PPE_RCB_COM_BASE + 0x1320) /* 多PF异常中断状态寄存器 */
#define PPE_RCB_COM_MPF_ABNORMAL_INT_SRC_REG          (PPE_RCB_COM_BASE + 0x1330) /* 多PF异常中断源寄存器 */
#define PPE_RCB_COM_MPF_ABNORMAL_INT_SET_REG          (PPE_RCB_COM_BASE + 0x1340) /* 多PF异常中断注入寄存器 */
#define PPE_RCB_COM_MPF_ABNORMAL_INT_CE_REG           (PPE_RCB_COM_BASE + 0x1350) /* 多PF异常中断源CE寄存器 */
#define PPE_RCB_COM_MPF_ABNORMAL_INT_NFE_REG          (PPE_RCB_COM_BASE + 0x1360) /* 多PF异常中断源NFE寄存器 */
#define PPE_RCB_COM_MPF_ABNORMAL_INT_FE_REG           (PPE_RCB_COM_BASE + 0x1370) /* 多PF异常中断源FE寄存器 */
#define PPE_RCB_COM_RCB_MEM_SERR_CNT_REG              (PPE_RCB_COM_BASE + 0x1380) /* RCB memory单bit错误次数统计 */
#define PPE_RCB_COM_RCB_MEM_MERR_CNT_REG              (PPE_RCB_COM_BASE + 0x1384) /* RCB memory多bit错误次数统计 */
#define PPE_RCB_COM_RCB_SERR_ADDR_REG                 (PPE_RCB_COM_BASE + 0x1390) /* RCB memory单bit错误信息记录 */
#define PPE_RCB_COM_RCB_MERR_ADDR_REG                 (PPE_RCB_COM_BASE + 0x1394) /* RCB memory多bit错误信息记录 */

/* Mem发生sbit错误时，上报ECC错误中断的阀值 */
#define PPE_RCB_COM_RCB_MEM_SBIT_ECC_THD_REG          (PPE_RCB_COM_BASE + 0x1398)
#define PPE_RCB_COM_REG_SEARCH_MISS_REG               (PPE_RCB_COM_BASE + 0x13A0) /* Slave通路TCAM查表miss状态 */
#define PPE_RCB_COM_RX_Q_SEARCH_MISS_REG              (PPE_RCB_COM_BASE + 0x13A4) /* RX方向TCAM查表miss状态 */
#define PPE_RCB_COM_PPU_RW_RSV0_REG                   (PPE_RCB_COM_BASE + 0x13B0) /* PPU保留RW寄存器0 */
#define PPE_RCB_COM_PPU_RW_RSV1_REG                   (PPE_RCB_COM_BASE + 0x13B4) /* PPU保留RW寄存器1 */
#define PPE_RCB_COM_PPU_RW_RSV2_REG                   (PPE_RCB_COM_BASE + 0x13B8) /* PPU保留RW寄存器2 */
#define PPE_RCB_COM_PPU_RW_RSV3_REG                   (PPE_RCB_COM_BASE + 0x13BC) /* PPU保留RW寄存器3 */
#define PPE_RCB_COM_RCB_MEM_ERR_INSERT_REG            (PPE_RCB_COM_BASE + 0x13C0) /* PPE RCB MEMORY错误植入 寄存器1 */
#define PPE_RCB_COM_ETS_TC_SP_MODE_REG                (PPE_RCB_COM_BASE + 0x1400) /* ETS MAC侧 TC nodes sp属性配置 */
#define PPE_RCB_COM_ETS_SHAPPING_BYPASS_REG           (PPE_RCB_COM_BASE + 0x1404) /* ETS MAC侧 SHAPING bypass配置寄存器 */
#define PPE_RCB_COM_ETS_TCG_SP_MODE_REG               (PPE_RCB_COM_BASE + 0x1408) /* ETS MAC侧 TCG nodes sp属性配置 */
#define PPE_RCB_COM_ETS_WEIGHT_OFFSET_REG             (PPE_RCB_COM_BASE + 0x140C) /* ETS MAC侧 调度权重偏移值 */

/* ETS MAC侧 TC Group nodes weight配置表 */
#define PPE_RCB_COM_ETS_TCG_WEIGHT_0_REG              (PPE_RCB_COM_BASE + 0x1480)
#define PPE_RCB_COM_ETS_TCG_WEIGHT_1_REG              (PPE_RCB_COM_BASE + 0x1484)
#define PPE_RCB_COM_ETS_TCG_WEIGHT_2_REG              (PPE_RCB_COM_BASE + 0x1488)
#define PPE_RCB_COM_ETS_TCG_WEIGHT_3_REG              (PPE_RCB_COM_BASE + 0x148C)

/* ETS MAC侧 TC nodes weight配置表 */
#define PPE_RCB_COM_ETS_TC_WEIGHT_0_REG               (PPE_RCB_COM_BASE + 0x1500)
#define PPE_RCB_COM_ETS_TC_WEIGHT_1_REG               (PPE_RCB_COM_BASE + 0x1504)
#define PPE_RCB_COM_ETS_TC_WEIGHT_2_REG               (PPE_RCB_COM_BASE + 0x1508)
#define PPE_RCB_COM_ETS_TC_WEIGHT_3_REG               (PPE_RCB_COM_BASE + 0x150C)
#define PPE_RCB_COM_ETS_TC_WEIGHT_4_REG               (PPE_RCB_COM_BASE + 0x1510)
#define PPE_RCB_COM_ETS_TC_WEIGHT_5_REG               (PPE_RCB_COM_BASE + 0x1514)
#define PPE_RCB_COM_ETS_TC_WEIGHT_6_REG               (PPE_RCB_COM_BASE + 0x1518)
#define PPE_RCB_COM_ETS_TC_WEIGHT_7_REG               (PPE_RCB_COM_BASE + 0x151C)
#define PPE_RCB_COM_ETS_TC_WEIGHT_8_REG               (PPE_RCB_COM_BASE + 0x1520)
#define PPE_RCB_COM_ETS_TC_WEIGHT_9_REG               (PPE_RCB_COM_BASE + 0x1524)
#define PPE_RCB_COM_ETS_TC_WEIGHT_10_REG              (PPE_RCB_COM_BASE + 0x1528)
#define PPE_RCB_COM_ETS_TC_WEIGHT_11_REG              (PPE_RCB_COM_BASE + 0x152C)
#define PPE_RCB_COM_ETS_TC_WEIGHT_12_REG              (PPE_RCB_COM_BASE + 0x1530)
#define PPE_RCB_COM_ETS_TC_WEIGHT_13_REG              (PPE_RCB_COM_BASE + 0x1534)
#define PPE_RCB_COM_ETS_TC_WEIGHT_14_REG              (PPE_RCB_COM_BASE + 0x1538)
#define PPE_RCB_COM_ETS_TC_WEIGHT_15_REG              (PPE_RCB_COM_BASE + 0x153C)

/* ETS MAC侧 TC Group nodes 级的shaper配置参数 */
#define PPE_RCB_COM_ETS_TCG_SHAPING_0_REG             (PPE_RCB_COM_BASE + 0x1580)
#define PPE_RCB_COM_ETS_TCG_SHAPING_1_REG             (PPE_RCB_COM_BASE + 0x1584)
#define PPE_RCB_COM_ETS_TCG_SHAPING_2_REG             (PPE_RCB_COM_BASE + 0x1588)
#define PPE_RCB_COM_ETS_TCG_SHAPING_3_REG             (PPE_RCB_COM_BASE + 0x158C)

/* ETS MAC侧 TC nodes 级的shaper配置参数 */
#define PPE_RCB_COM_ETS_TC_SHAPING_0_REG              (PPE_RCB_COM_BASE + 0x1600)
#define PPE_RCB_COM_ETS_TC_SHAPING_1_REG              (PPE_RCB_COM_BASE + 0x1604)
#define PPE_RCB_COM_ETS_TC_SHAPING_2_REG              (PPE_RCB_COM_BASE + 0x1608)
#define PPE_RCB_COM_ETS_TC_SHAPING_3_REG              (PPE_RCB_COM_BASE + 0x160C)
#define PPE_RCB_COM_ETS_TC_SHAPING_4_REG              (PPE_RCB_COM_BASE + 0x1610)
#define PPE_RCB_COM_ETS_TC_SHAPING_5_REG              (PPE_RCB_COM_BASE + 0x1614)
#define PPE_RCB_COM_ETS_TC_SHAPING_6_REG              (PPE_RCB_COM_BASE + 0x1618)
#define PPE_RCB_COM_ETS_TC_SHAPING_7_REG              (PPE_RCB_COM_BASE + 0x161C)
#define PPE_RCB_COM_ETS_TC_SHAPING_8_REG              (PPE_RCB_COM_BASE + 0x1620)
#define PPE_RCB_COM_ETS_TC_SHAPING_9_REG              (PPE_RCB_COM_BASE + 0x1624)
#define PPE_RCB_COM_ETS_TC_SHAPING_10_REG             (PPE_RCB_COM_BASE + 0x1628)
#define PPE_RCB_COM_ETS_TC_SHAPING_11_REG             (PPE_RCB_COM_BASE + 0x162C)
#define PPE_RCB_COM_ETS_TC_SHAPING_12_REG             (PPE_RCB_COM_BASE + 0x1630)
#define PPE_RCB_COM_ETS_TC_SHAPING_13_REG             (PPE_RCB_COM_BASE + 0x1634)
#define PPE_RCB_COM_ETS_TC_SHAPING_14_REG             (PPE_RCB_COM_BASE + 0x1638)
#define PPE_RCB_COM_ETS_TC_SHAPING_15_REG             (PPE_RCB_COM_BASE + 0x163C)

/* ETS MAC侧 PORT Maping bitmap配置 */
#define PPE_RCB_COM_ETS_PORT_MAPING_0_REG             (PPE_RCB_COM_BASE + 0x1680)
#define PPE_RCB_COM_ETS_PORT_MAPING_1_REG             (PPE_RCB_COM_BASE + 0x1684)
#define PPE_RCB_COM_ETS_PORT_MAPING_2_REG             (PPE_RCB_COM_BASE + 0x1688)
#define PPE_RCB_COM_ETS_PORT_MAPING_3_REG             (PPE_RCB_COM_BASE + 0x168C)

/* ETS MAC侧 TC Group Maping bitmap配置 */
#define PPE_RCB_COM_ETS_TCG_MAPING_0_REG              (PPE_RCB_COM_BASE + 0x1700)
#define PPE_RCB_COM_ETS_TCG_MAPING_1_REG              (PPE_RCB_COM_BASE + 0x1704)
#define PPE_RCB_COM_ETS_TCG_MAPING_2_REG              (PPE_RCB_COM_BASE + 0x1708)
#define PPE_RCB_COM_ETS_TCG_MAPING_3_REG              (PPE_RCB_COM_BASE + 0x170C)

/* ETS MAC侧 TC Maping bitmap配置 */
#define PPE_RCB_COM_ETS_TC_MAPING_0_REG               (PPE_RCB_COM_BASE + 0x1780)
#define PPE_RCB_COM_ETS_TC_MAPING_1_REG               (PPE_RCB_COM_BASE + 0x1784)
#define PPE_RCB_COM_ETS_TC_MAPING_2_REG               (PPE_RCB_COM_BASE + 0x1788)
#define PPE_RCB_COM_ETS_TC_MAPING_3_REG               (PPE_RCB_COM_BASE + 0x178C)
#define PPE_RCB_COM_ETS_TC_MAPING_4_REG               (PPE_RCB_COM_BASE + 0x1790)
#define PPE_RCB_COM_ETS_TC_MAPING_5_REG               (PPE_RCB_COM_BASE + 0x1794)
#define PPE_RCB_COM_ETS_TC_MAPING_6_REG               (PPE_RCB_COM_BASE + 0x1798)
#define PPE_RCB_COM_ETS_TC_MAPING_7_REG               (PPE_RCB_COM_BASE + 0x179C)
#define PPE_RCB_COM_ETS_TC_MAPING_8_REG               (PPE_RCB_COM_BASE + 0x17A0)
#define PPE_RCB_COM_ETS_TC_MAPING_9_REG               (PPE_RCB_COM_BASE + 0x17A4)
#define PPE_RCB_COM_ETS_TC_MAPING_10_REG              (PPE_RCB_COM_BASE + 0x17A8)
#define PPE_RCB_COM_ETS_TC_MAPING_11_REG              (PPE_RCB_COM_BASE + 0x17AC)
#define PPE_RCB_COM_ETS_TC_MAPING_12_REG              (PPE_RCB_COM_BASE + 0x17B0)
#define PPE_RCB_COM_ETS_TC_MAPING_13_REG              (PPE_RCB_COM_BASE + 0x17B4)
#define PPE_RCB_COM_ETS_TC_MAPING_14_REG              (PPE_RCB_COM_BASE + 0x17B8)
#define PPE_RCB_COM_ETS_TC_MAPING_15_REG              (PPE_RCB_COM_BASE + 0x17BC)

/* ETS MAC侧 PORT node 级的shaper配置参数 */
#define PPE_RCB_COM_ETS_PORT_SHAPING_0_REG            (PPE_RCB_COM_BASE + 0x1800)
#define PPE_RCB_COM_ETS_PORT_SHAPING_1_REG            (PPE_RCB_COM_BASE + 0x1804)
#define PPE_RCB_COM_ETS_PORT_SHAPING_2_REG            (PPE_RCB_COM_BASE + 0x1808)
#define PPE_RCB_COM_ETS_PORT_SHAPING_3_REG            (PPE_RCB_COM_BASE + 0x180C)

/* ETS MAC侧 queue node 级的shaper配置参数 */
#define PPE_RCB_COM_ETS_QUEUE_SHAPING_0_REG           (PPE_RCB_COM_BASE + 0x1880)
#define PPE_RCB_COM_ETS_QUEUE_SHAPING_1_REG           (PPE_RCB_COM_BASE + 0x1884)
#define PPE_RCB_COM_ETS_QUEUE_SHAPING_2_REG           (PPE_RCB_COM_BASE + 0x1888)
#define PPE_RCB_COM_ETS_QUEUE_SHAPING_3_REG           (PPE_RCB_COM_BASE + 0x188C)
#define PPE_RCB_COM_ETS_QUEUE_SHAPING_4_REG           (PPE_RCB_COM_BASE + 0x1890)
#define PPE_RCB_COM_ETS_QUEUE_SHAPING_5_REG           (PPE_RCB_COM_BASE + 0x1894)
#define PPE_RCB_COM_ETS_QUEUE_SHAPING_6_REG           (PPE_RCB_COM_BASE + 0x1898)
#define PPE_RCB_COM_ETS_QUEUE_SHAPING_7_REG           (PPE_RCB_COM_BASE + 0x189C)
#define PPE_RCB_COM_ETS_QUEUE_SHAPING_8_REG           (PPE_RCB_COM_BASE + 0x18A0)
#define PPE_RCB_COM_ETS_QUEUE_SHAPING_9_REG           (PPE_RCB_COM_BASE + 0x18A4)
#define PPE_RCB_COM_ETS_QUEUE_SHAPING_10_REG          (PPE_RCB_COM_BASE + 0x18A8)
#define PPE_RCB_COM_ETS_QUEUE_SHAPING_11_REG          (PPE_RCB_COM_BASE + 0x18AC)
#define PPE_RCB_COM_ETS_QUEUE_SHAPING_12_REG          (PPE_RCB_COM_BASE + 0x18B0)
#define PPE_RCB_COM_ETS_QUEUE_SHAPING_13_REG          (PPE_RCB_COM_BASE + 0x18B4)
#define PPE_RCB_COM_ETS_QUEUE_SHAPING_14_REG          (PPE_RCB_COM_BASE + 0x18B8)
#define PPE_RCB_COM_ETS_QUEUE_SHAPING_15_REG          (PPE_RCB_COM_BASE + 0x18BC)

/* ETS MAC侧 基于队列配置的预减包长 */
#define PPE_RCB_COM_ETS_PRE_SUB_OFFSET_LEN_CFG_0_REG  (PPE_RCB_COM_BASE + 0x1900)
#define PPE_RCB_COM_ETS_PRE_SUB_OFFSET_LEN_CFG_1_REG  (PPE_RCB_COM_BASE + 0x1904)
#define PPE_RCB_COM_ETS_PRE_SUB_OFFSET_LEN_CFG_2_REG  (PPE_RCB_COM_BASE + 0x1908)
#define PPE_RCB_COM_ETS_PRE_SUB_OFFSET_LEN_CFG_3_REG  (PPE_RCB_COM_BASE + 0x190C)
#define PPE_RCB_COM_ETS_PRE_SUB_OFFSET_LEN_CFG_4_REG  (PPE_RCB_COM_BASE + 0x1910)
#define PPE_RCB_COM_ETS_PRE_SUB_OFFSET_LEN_CFG_5_REG  (PPE_RCB_COM_BASE + 0x1914)
#define PPE_RCB_COM_ETS_PRE_SUB_OFFSET_LEN_CFG_6_REG  (PPE_RCB_COM_BASE + 0x1918)
#define PPE_RCB_COM_ETS_PRE_SUB_OFFSET_LEN_CFG_7_REG  (PPE_RCB_COM_BASE + 0x191C)
#define PPE_RCB_COM_ETS_PRE_SUB_OFFSET_LEN_CFG_8_REG  (PPE_RCB_COM_BASE + 0x1920)
#define PPE_RCB_COM_ETS_PRE_SUB_OFFSET_LEN_CFG_9_REG  (PPE_RCB_COM_BASE + 0x1924)
#define PPE_RCB_COM_ETS_PRE_SUB_OFFSET_LEN_CFG_10_REG (PPE_RCB_COM_BASE + 0x1928)
#define PPE_RCB_COM_ETS_PRE_SUB_OFFSET_LEN_CFG_11_REG (PPE_RCB_COM_BASE + 0x192C)
#define PPE_RCB_COM_ETS_PRE_SUB_OFFSET_LEN_CFG_12_REG (PPE_RCB_COM_BASE + 0x1930)
#define PPE_RCB_COM_ETS_PRE_SUB_OFFSET_LEN_CFG_13_REG (PPE_RCB_COM_BASE + 0x1934)
#define PPE_RCB_COM_ETS_PRE_SUB_OFFSET_LEN_CFG_14_REG (PPE_RCB_COM_BASE + 0x1938)
#define PPE_RCB_COM_ETS_PRE_SUB_OFFSET_LEN_CFG_15_REG (PPE_RCB_COM_BASE + 0x193C)

/* ETS MAC侧 queue node 级的队列绑定关系配置 */
#define PPE_RCB_COM_ETS_QUEUE_LINK_TC_0_REG           (PPE_RCB_COM_BASE + 0x1A00)
#define PPE_RCB_COM_ETS_QUEUE_LINK_TC_1_REG           (PPE_RCB_COM_BASE + 0x1A04)
#define PPE_RCB_COM_ETS_QUEUE_LINK_TC_2_REG           (PPE_RCB_COM_BASE + 0x1A08)
#define PPE_RCB_COM_ETS_QUEUE_LINK_TC_3_REG           (PPE_RCB_COM_BASE + 0x1A0C)
#define PPE_RCB_COM_ETS_QUEUE_LINK_TC_4_REG           (PPE_RCB_COM_BASE + 0x1A10)
#define PPE_RCB_COM_ETS_QUEUE_LINK_TC_5_REG           (PPE_RCB_COM_BASE + 0x1A14)
#define PPE_RCB_COM_ETS_QUEUE_LINK_TC_6_REG           (PPE_RCB_COM_BASE + 0x1A18)
#define PPE_RCB_COM_ETS_QUEUE_LINK_TC_7_REG           (PPE_RCB_COM_BASE + 0x1A1C)
#define PPE_RCB_COM_ETS_QUEUE_LINK_TC_8_REG           (PPE_RCB_COM_BASE + 0x1A20)
#define PPE_RCB_COM_ETS_QUEUE_LINK_TC_9_REG           (PPE_RCB_COM_BASE + 0x1A24)
#define PPE_RCB_COM_ETS_QUEUE_LINK_TC_10_REG          (PPE_RCB_COM_BASE + 0x1A28)
#define PPE_RCB_COM_ETS_QUEUE_LINK_TC_11_REG          (PPE_RCB_COM_BASE + 0x1A2C)
#define PPE_RCB_COM_ETS_QUEUE_LINK_TC_12_REG          (PPE_RCB_COM_BASE + 0x1A30)
#define PPE_RCB_COM_ETS_QUEUE_LINK_TC_13_REG          (PPE_RCB_COM_BASE + 0x1A34)
#define PPE_RCB_COM_ETS_QUEUE_LINK_TC_14_REG          (PPE_RCB_COM_BASE + 0x1A38)
#define PPE_RCB_COM_ETS_QUEUE_LINK_TC_15_REG          (PPE_RCB_COM_BASE + 0x1A3C)

/* ETS MAC侧 queue node 级的使能配置 */
#define PPE_RCB_COM_ETS_QUEUE_EN_CFG_0_REG            (PPE_RCB_COM_BASE + 0x1A80)
#define PPE_RCB_COM_ETS_QUEUE_EN_CFG_1_REG            (PPE_RCB_COM_BASE + 0x1A84)
#define PPE_RCB_COM_ETS_QUEUE_EN_CFG_2_REG            (PPE_RCB_COM_BASE + 0x1A88)
#define PPE_RCB_COM_ETS_QUEUE_EN_CFG_3_REG            (PPE_RCB_COM_BASE + 0x1A8C)
#define PPE_RCB_COM_ETS_QUEUE_EN_CFG_4_REG            (PPE_RCB_COM_BASE + 0x1A90)
#define PPE_RCB_COM_ETS_QUEUE_EN_CFG_5_REG            (PPE_RCB_COM_BASE + 0x1A94)
#define PPE_RCB_COM_ETS_QUEUE_EN_CFG_6_REG            (PPE_RCB_COM_BASE + 0x1A98)
#define PPE_RCB_COM_ETS_QUEUE_EN_CFG_7_REG            (PPE_RCB_COM_BASE + 0x1A9C)
#define PPE_RCB_COM_ETS_QUEUE_EN_CFG_8_REG            (PPE_RCB_COM_BASE + 0x1AA0)
#define PPE_RCB_COM_ETS_QUEUE_EN_CFG_9_REG            (PPE_RCB_COM_BASE + 0x1AA4)
#define PPE_RCB_COM_ETS_QUEUE_EN_CFG_10_REG           (PPE_RCB_COM_BASE + 0x1AA8)
#define PPE_RCB_COM_ETS_QUEUE_EN_CFG_11_REG           (PPE_RCB_COM_BASE + 0x1AAC)
#define PPE_RCB_COM_ETS_QUEUE_EN_CFG_12_REG           (PPE_RCB_COM_BASE + 0x1AB0)
#define PPE_RCB_COM_ETS_QUEUE_EN_CFG_13_REG           (PPE_RCB_COM_BASE + 0x1AB4)
#define PPE_RCB_COM_ETS_QUEUE_EN_CFG_14_REG           (PPE_RCB_COM_BASE + 0x1AB8)
#define PPE_RCB_COM_ETS_QUEUE_EN_CFG_15_REG           (PPE_RCB_COM_BASE + 0x1ABC)

/* ETS MAC侧 TC级的CBS算法中发送斜率SENDSLOPE配置 */
#define PPE_RCB_COM_ETS_TC_SENDSLOPE_0_REG            (PPE_RCB_COM_BASE + 0x1B00)
#define PPE_RCB_COM_ETS_TC_SENDSLOPE_1_REG            (PPE_RCB_COM_BASE + 0x1B04)
#define PPE_RCB_COM_ETS_TC_SENDSLOPE_2_REG            (PPE_RCB_COM_BASE + 0x1B08)
#define PPE_RCB_COM_ETS_TC_SENDSLOPE_3_REG            (PPE_RCB_COM_BASE + 0x1B0C)
#define PPE_RCB_COM_ETS_TC_SENDSLOPE_4_REG            (PPE_RCB_COM_BASE + 0x1B10)
#define PPE_RCB_COM_ETS_TC_SENDSLOPE_5_REG            (PPE_RCB_COM_BASE + 0x1B14)
#define PPE_RCB_COM_ETS_TC_SENDSLOPE_6_REG            (PPE_RCB_COM_BASE + 0x1B18)
#define PPE_RCB_COM_ETS_TC_SENDSLOPE_7_REG            (PPE_RCB_COM_BASE + 0x1B1C)
#define PPE_RCB_COM_ETS_TC_SENDSLOPE_8_REG            (PPE_RCB_COM_BASE + 0x1B20)
#define PPE_RCB_COM_ETS_TC_SENDSLOPE_9_REG            (PPE_RCB_COM_BASE + 0x1B24)
#define PPE_RCB_COM_ETS_TC_SENDSLOPE_10_REG           (PPE_RCB_COM_BASE + 0x1B28)
#define PPE_RCB_COM_ETS_TC_SENDSLOPE_11_REG           (PPE_RCB_COM_BASE + 0x1B2C)
#define PPE_RCB_COM_ETS_TC_SENDSLOPE_12_REG           (PPE_RCB_COM_BASE + 0x1B30)
#define PPE_RCB_COM_ETS_TC_SENDSLOPE_13_REG           (PPE_RCB_COM_BASE + 0x1B34)
#define PPE_RCB_COM_ETS_TC_SENDSLOPE_14_REG           (PPE_RCB_COM_BASE + 0x1B38)
#define PPE_RCB_COM_ETS_TC_SENDSLOPE_15_REG           (PPE_RCB_COM_BASE + 0x1B3C)

/* ETS MAC侧 TC级的CBS算法中累积斜率IDLESLOPE配置 */
#define PPE_RCB_COM_ETS_TC_IDLESLOPE_0_REG            (PPE_RCB_COM_BASE + 0x1B80)
#define PPE_RCB_COM_ETS_TC_IDLESLOPE_1_REG            (PPE_RCB_COM_BASE + 0x1B84)
#define PPE_RCB_COM_ETS_TC_IDLESLOPE_2_REG            (PPE_RCB_COM_BASE + 0x1B88)
#define PPE_RCB_COM_ETS_TC_IDLESLOPE_3_REG            (PPE_RCB_COM_BASE + 0x1B8C)
#define PPE_RCB_COM_ETS_TC_IDLESLOPE_4_REG            (PPE_RCB_COM_BASE + 0x1B90)
#define PPE_RCB_COM_ETS_TC_IDLESLOPE_5_REG            (PPE_RCB_COM_BASE + 0x1B94)
#define PPE_RCB_COM_ETS_TC_IDLESLOPE_6_REG            (PPE_RCB_COM_BASE + 0x1B98)
#define PPE_RCB_COM_ETS_TC_IDLESLOPE_7_REG            (PPE_RCB_COM_BASE + 0x1B9C)
#define PPE_RCB_COM_ETS_TC_IDLESLOPE_8_REG            (PPE_RCB_COM_BASE + 0x1BA0)
#define PPE_RCB_COM_ETS_TC_IDLESLOPE_9_REG            (PPE_RCB_COM_BASE + 0x1BA4)
#define PPE_RCB_COM_ETS_TC_IDLESLOPE_10_REG           (PPE_RCB_COM_BASE + 0x1BA8)
#define PPE_RCB_COM_ETS_TC_IDLESLOPE_11_REG           (PPE_RCB_COM_BASE + 0x1BAC)
#define PPE_RCB_COM_ETS_TC_IDLESLOPE_12_REG           (PPE_RCB_COM_BASE + 0x1BB0)
#define PPE_RCB_COM_ETS_TC_IDLESLOPE_13_REG           (PPE_RCB_COM_BASE + 0x1BB4)
#define PPE_RCB_COM_ETS_TC_IDLESLOPE_14_REG           (PPE_RCB_COM_BASE + 0x1BB8)
#define PPE_RCB_COM_ETS_TC_IDLESLOPE_15_REG           (PPE_RCB_COM_BASE + 0x1BBC)

/* ETS MAC侧 TC级的CBS算法中高水线HICREDIT配置 */
#define PPE_RCB_COM_ETS_TC_HICREDIT_0_REG             (PPE_RCB_COM_BASE + 0x1C00)
#define PPE_RCB_COM_ETS_TC_HICREDIT_1_REG             (PPE_RCB_COM_BASE + 0x1C04)
#define PPE_RCB_COM_ETS_TC_HICREDIT_2_REG             (PPE_RCB_COM_BASE + 0x1C08)
#define PPE_RCB_COM_ETS_TC_HICREDIT_3_REG             (PPE_RCB_COM_BASE + 0x1C0C)
#define PPE_RCB_COM_ETS_TC_HICREDIT_4_REG             (PPE_RCB_COM_BASE + 0x1C10)
#define PPE_RCB_COM_ETS_TC_HICREDIT_5_REG             (PPE_RCB_COM_BASE + 0x1C14)
#define PPE_RCB_COM_ETS_TC_HICREDIT_6_REG             (PPE_RCB_COM_BASE + 0x1C18)
#define PPE_RCB_COM_ETS_TC_HICREDIT_7_REG             (PPE_RCB_COM_BASE + 0x1C1C)
#define PPE_RCB_COM_ETS_TC_HICREDIT_8_REG             (PPE_RCB_COM_BASE + 0x1C20)
#define PPE_RCB_COM_ETS_TC_HICREDIT_9_REG             (PPE_RCB_COM_BASE + 0x1C24)
#define PPE_RCB_COM_ETS_TC_HICREDIT_10_REG            (PPE_RCB_COM_BASE + 0x1C28)
#define PPE_RCB_COM_ETS_TC_HICREDIT_11_REG            (PPE_RCB_COM_BASE + 0x1C2C)
#define PPE_RCB_COM_ETS_TC_HICREDIT_12_REG            (PPE_RCB_COM_BASE + 0x1C30)
#define PPE_RCB_COM_ETS_TC_HICREDIT_13_REG            (PPE_RCB_COM_BASE + 0x1C34)
#define PPE_RCB_COM_ETS_TC_HICREDIT_14_REG            (PPE_RCB_COM_BASE + 0x1C38)
#define PPE_RCB_COM_ETS_TC_HICREDIT_15_REG            (PPE_RCB_COM_BASE + 0x1C3C)

/* ETS MAC侧 TC级的CBS算法中低水线LOCREDIT配置 */
#define PPE_RCB_COM_ETS_TC_LOCREDIT_0_REG             (PPE_RCB_COM_BASE + 0x1C80)
#define PPE_RCB_COM_ETS_TC_LOCREDIT_1_REG             (PPE_RCB_COM_BASE + 0x1C84)
#define PPE_RCB_COM_ETS_TC_LOCREDIT_2_REG             (PPE_RCB_COM_BASE + 0x1C88)
#define PPE_RCB_COM_ETS_TC_LOCREDIT_3_REG             (PPE_RCB_COM_BASE + 0x1C8C)
#define PPE_RCB_COM_ETS_TC_LOCREDIT_4_REG             (PPE_RCB_COM_BASE + 0x1C90)
#define PPE_RCB_COM_ETS_TC_LOCREDIT_5_REG             (PPE_RCB_COM_BASE + 0x1C94)
#define PPE_RCB_COM_ETS_TC_LOCREDIT_6_REG             (PPE_RCB_COM_BASE + 0x1C98)
#define PPE_RCB_COM_ETS_TC_LOCREDIT_7_REG             (PPE_RCB_COM_BASE + 0x1C9C)
#define PPE_RCB_COM_ETS_TC_LOCREDIT_8_REG             (PPE_RCB_COM_BASE + 0x1CA0)
#define PPE_RCB_COM_ETS_TC_LOCREDIT_9_REG             (PPE_RCB_COM_BASE + 0x1CA4)
#define PPE_RCB_COM_ETS_TC_LOCREDIT_10_REG            (PPE_RCB_COM_BASE + 0x1CA8)
#define PPE_RCB_COM_ETS_TC_LOCREDIT_11_REG            (PPE_RCB_COM_BASE + 0x1CAC)
#define PPE_RCB_COM_ETS_TC_LOCREDIT_12_REG            (PPE_RCB_COM_BASE + 0x1CB0)
#define PPE_RCB_COM_ETS_TC_LOCREDIT_13_REG            (PPE_RCB_COM_BASE + 0x1CB4)
#define PPE_RCB_COM_ETS_TC_LOCREDIT_14_REG            (PPE_RCB_COM_BASE + 0x1CB8)
#define PPE_RCB_COM_ETS_TC_LOCREDIT_15_REG            (PPE_RCB_COM_BASE + 0x1CBC)

/* ETS MAC侧 TC node 级的CBS使能配置 */
#define PPE_RCB_COM_ETS_TC_CBS_EN_0_REG               (PPE_RCB_COM_BASE + 0x1D00)
#define PPE_RCB_COM_ETS_TC_CBS_EN_1_REG               (PPE_RCB_COM_BASE + 0x1D04)
#define PPE_RCB_COM_ETS_TC_CBS_EN_2_REG               (PPE_RCB_COM_BASE + 0x1D08)
#define PPE_RCB_COM_ETS_TC_CBS_EN_3_REG               (PPE_RCB_COM_BASE + 0x1D0C)
#define PPE_RCB_COM_ETS_TC_CBS_EN_4_REG               (PPE_RCB_COM_BASE + 0x1D10)
#define PPE_RCB_COM_ETS_TC_CBS_EN_5_REG               (PPE_RCB_COM_BASE + 0x1D14)
#define PPE_RCB_COM_ETS_TC_CBS_EN_6_REG               (PPE_RCB_COM_BASE + 0x1D18)
#define PPE_RCB_COM_ETS_TC_CBS_EN_7_REG               (PPE_RCB_COM_BASE + 0x1D1C)
#define PPE_RCB_COM_ETS_TC_CBS_EN_8_REG               (PPE_RCB_COM_BASE + 0x1D20)
#define PPE_RCB_COM_ETS_TC_CBS_EN_9_REG               (PPE_RCB_COM_BASE + 0x1D24)
#define PPE_RCB_COM_ETS_TC_CBS_EN_10_REG              (PPE_RCB_COM_BASE + 0x1D28)
#define PPE_RCB_COM_ETS_TC_CBS_EN_11_REG              (PPE_RCB_COM_BASE + 0x1D2C)
#define PPE_RCB_COM_ETS_TC_CBS_EN_12_REG              (PPE_RCB_COM_BASE + 0x1D30)
#define PPE_RCB_COM_ETS_TC_CBS_EN_13_REG              (PPE_RCB_COM_BASE + 0x1D34)
#define PPE_RCB_COM_ETS_TC_CBS_EN_14_REG              (PPE_RCB_COM_BASE + 0x1D38)
#define PPE_RCB_COM_ETS_TC_CBS_EN_15_REG              (PPE_RCB_COM_BASE + 0x1D3C)

/* ETS MAC侧 TC node 级的对应MAC端口速率配置 */
#define PPE_RCB_COM_ETS_TC_SPEED_SEL_0_REG            (PPE_RCB_COM_BASE + 0x1D80)
#define PPE_RCB_COM_ETS_TC_SPEED_SEL_1_REG            (PPE_RCB_COM_BASE + 0x1D84)
#define PPE_RCB_COM_ETS_TC_SPEED_SEL_2_REG            (PPE_RCB_COM_BASE + 0x1D88)
#define PPE_RCB_COM_ETS_TC_SPEED_SEL_3_REG            (PPE_RCB_COM_BASE + 0x1D8C)
#define PPE_RCB_COM_ETS_TC_SPEED_SEL_4_REG            (PPE_RCB_COM_BASE + 0x1D90)
#define PPE_RCB_COM_ETS_TC_SPEED_SEL_5_REG            (PPE_RCB_COM_BASE + 0x1D94)
#define PPE_RCB_COM_ETS_TC_SPEED_SEL_6_REG            (PPE_RCB_COM_BASE + 0x1D98)
#define PPE_RCB_COM_ETS_TC_SPEED_SEL_7_REG            (PPE_RCB_COM_BASE + 0x1D9C)
#define PPE_RCB_COM_ETS_TC_SPEED_SEL_8_REG            (PPE_RCB_COM_BASE + 0x1DA0)
#define PPE_RCB_COM_ETS_TC_SPEED_SEL_9_REG            (PPE_RCB_COM_BASE + 0x1DA4)
#define PPE_RCB_COM_ETS_TC_SPEED_SEL_10_REG           (PPE_RCB_COM_BASE + 0x1DA8)
#define PPE_RCB_COM_ETS_TC_SPEED_SEL_11_REG           (PPE_RCB_COM_BASE + 0x1DAC)
#define PPE_RCB_COM_ETS_TC_SPEED_SEL_12_REG           (PPE_RCB_COM_BASE + 0x1DB0)
#define PPE_RCB_COM_ETS_TC_SPEED_SEL_13_REG           (PPE_RCB_COM_BASE + 0x1DB4)
#define PPE_RCB_COM_ETS_TC_SPEED_SEL_14_REG           (PPE_RCB_COM_BASE + 0x1DB8)
#define PPE_RCB_COM_ETS_TC_SPEED_SEL_15_REG           (PPE_RCB_COM_BASE + 0x1DBC)

/* ETS MAC侧 queue node 级的队列FLUSH的请求信号 */
#define PPE_RCB_COM_ETS_QUEUE_FLUSH_REQ_0_REG         (PPE_RCB_COM_BASE + 0x1DC0)
#define PPE_RCB_COM_ETS_QUEUE_FLUSH_REQ_1_REG         (PPE_RCB_COM_BASE + 0x1DC4)
#define PPE_RCB_COM_ETS_QUEUE_FLUSH_REQ_2_REG         (PPE_RCB_COM_BASE + 0x1DC8)
#define PPE_RCB_COM_ETS_QUEUE_FLUSH_REQ_3_REG         (PPE_RCB_COM_BASE + 0x1DCC)
#define PPE_RCB_COM_ETS_QUEUE_FLUSH_REQ_4_REG         (PPE_RCB_COM_BASE + 0x1DD0)
#define PPE_RCB_COM_ETS_QUEUE_FLUSH_REQ_5_REG         (PPE_RCB_COM_BASE + 0x1DD4)
#define PPE_RCB_COM_ETS_QUEUE_FLUSH_REQ_6_REG         (PPE_RCB_COM_BASE + 0x1DD8)
#define PPE_RCB_COM_ETS_QUEUE_FLUSH_REQ_7_REG         (PPE_RCB_COM_BASE + 0x1DDC)
#define PPE_RCB_COM_ETS_QUEUE_FLUSH_REQ_8_REG         (PPE_RCB_COM_BASE + 0x1DE0)
#define PPE_RCB_COM_ETS_QUEUE_FLUSH_REQ_9_REG         (PPE_RCB_COM_BASE + 0x1DE4)
#define PPE_RCB_COM_ETS_QUEUE_FLUSH_REQ_10_REG        (PPE_RCB_COM_BASE + 0x1DE8)
#define PPE_RCB_COM_ETS_QUEUE_FLUSH_REQ_11_REG        (PPE_RCB_COM_BASE + 0x1DEC)
#define PPE_RCB_COM_ETS_QUEUE_FLUSH_REQ_12_REG        (PPE_RCB_COM_BASE + 0x1DF0)
#define PPE_RCB_COM_ETS_QUEUE_FLUSH_REQ_13_REG        (PPE_RCB_COM_BASE + 0x1DF4)
#define PPE_RCB_COM_ETS_QUEUE_FLUSH_REQ_14_REG        (PPE_RCB_COM_BASE + 0x1DF8)
#define PPE_RCB_COM_ETS_QUEUE_FLUSH_REQ_15_REG        (PPE_RCB_COM_BASE + 0x1DFC)

/* ETS MAC侧 queue node 级的队列FLUSH的完成信号 */
#define PPE_RCB_COM_ETS_QUEUE_FLUSH_DONE_0_REG        (PPE_RCB_COM_BASE + 0x1E00)
#define PPE_RCB_COM_ETS_QUEUE_FLUSH_DONE_1_REG        (PPE_RCB_COM_BASE + 0x1E04)
#define PPE_RCB_COM_ETS_QUEUE_FLUSH_DONE_2_REG        (PPE_RCB_COM_BASE + 0x1E08)
#define PPE_RCB_COM_ETS_QUEUE_FLUSH_DONE_3_REG        (PPE_RCB_COM_BASE + 0x1E0C)
#define PPE_RCB_COM_ETS_QUEUE_FLUSH_DONE_4_REG        (PPE_RCB_COM_BASE + 0x1E10)
#define PPE_RCB_COM_ETS_QUEUE_FLUSH_DONE_5_REG        (PPE_RCB_COM_BASE + 0x1E14)
#define PPE_RCB_COM_ETS_QUEUE_FLUSH_DONE_6_REG        (PPE_RCB_COM_BASE + 0x1E18)
#define PPE_RCB_COM_ETS_QUEUE_FLUSH_DONE_7_REG        (PPE_RCB_COM_BASE + 0x1E1C)
#define PPE_RCB_COM_ETS_QUEUE_FLUSH_DONE_8_REG        (PPE_RCB_COM_BASE + 0x1E20)
#define PPE_RCB_COM_ETS_QUEUE_FLUSH_DONE_9_REG        (PPE_RCB_COM_BASE + 0x1E24)
#define PPE_RCB_COM_ETS_QUEUE_FLUSH_DONE_10_REG       (PPE_RCB_COM_BASE + 0x1E28)
#define PPE_RCB_COM_ETS_QUEUE_FLUSH_DONE_11_REG       (PPE_RCB_COM_BASE + 0x1E2C)
#define PPE_RCB_COM_ETS_QUEUE_FLUSH_DONE_12_REG       (PPE_RCB_COM_BASE + 0x1E30)
#define PPE_RCB_COM_ETS_QUEUE_FLUSH_DONE_13_REG       (PPE_RCB_COM_BASE + 0x1E34)
#define PPE_RCB_COM_ETS_QUEUE_FLUSH_DONE_14_REG       (PPE_RCB_COM_BASE + 0x1E38)
#define PPE_RCB_COM_ETS_QUEUE_FLUSH_DONE_15_REG       (PPE_RCB_COM_BASE + 0x1E3C)

#define PPE_RCB_COM_ETS_BYTE_INFO_CFG_REG             (PPE_RCB_COM_BASE + 0x1E58) /* ETS补偿包长配置寄存器 */
#define PPE_RCB_COM_ETS_QUEUE_OFFSET_LEN_0_REG        (PPE_RCB_COM_BASE + 0x1E60) /* QUEUE补偿长度配置 */
#define PPE_RCB_COM_ETS_QUEUE_OFFSET_LEN_1_REG        (PPE_RCB_COM_BASE + 0x1E64) /* QUEUE补偿长度配置 */
#define PPE_RCB_COM_ETS_QUEUE_OFFSET_LEN_2_REG        (PPE_RCB_COM_BASE + 0x1E68) /* QUEUE补偿长度配置 */
#define PPE_RCB_COM_ETS_QUEUE_OFFSET_LEN_3_REG        (PPE_RCB_COM_BASE + 0x1E6C) /* QUEUE补偿长度配置 */
#define PPE_RCB_COM_ETS_QUEUE_OFFSET_LEN_4_REG        (PPE_RCB_COM_BASE + 0x1E70) /* QUEUE补偿长度配置 */
#define PPE_RCB_COM_ETS_QUEUE_OFFSET_LEN_5_REG        (PPE_RCB_COM_BASE + 0x1E74) /* QUEUE补偿长度配置 */
#define PPE_RCB_COM_ETS_QUEUE_OFFSET_LEN_6_REG        (PPE_RCB_COM_BASE + 0x1E78) /* QUEUE补偿长度配置 */
#define PPE_RCB_COM_ETS_QUEUE_OFFSET_LEN_7_REG        (PPE_RCB_COM_BASE + 0x1E7C) /* QUEUE补偿长度配置 */
#define PPE_RCB_COM_ETS_QUEUE_OFFSET_LEN_8_REG        (PPE_RCB_COM_BASE + 0x1E80) /* QUEUE补偿长度配置 */
#define PPE_RCB_COM_ETS_QUEUE_OFFSET_LEN_9_REG        (PPE_RCB_COM_BASE + 0x1E84) /* QUEUE补偿长度配置 */
#define PPE_RCB_COM_ETS_QUEUE_OFFSET_LEN_10_REG       (PPE_RCB_COM_BASE + 0x1E88) /* QUEUE补偿长度配置 */
#define PPE_RCB_COM_ETS_QUEUE_OFFSET_LEN_11_REG       (PPE_RCB_COM_BASE + 0x1E8C) /* QUEUE补偿长度配置 */
#define PPE_RCB_COM_ETS_QUEUE_OFFSET_LEN_12_REG       (PPE_RCB_COM_BASE + 0x1E90) /* QUEUE补偿长度配置 */
#define PPE_RCB_COM_ETS_QUEUE_OFFSET_LEN_13_REG       (PPE_RCB_COM_BASE + 0x1E94) /* QUEUE补偿长度配置 */
#define PPE_RCB_COM_ETS_QUEUE_OFFSET_LEN_14_REG       (PPE_RCB_COM_BASE + 0x1E98) /* QUEUE补偿长度配置 */
#define PPE_RCB_COM_ETS_QUEUE_OFFSET_LEN_15_REG       (PPE_RCB_COM_BASE + 0x1E9C) /* QUEUE补偿长度配置 */

/* ETS MAC侧 TC node 级的TC绑定关系配置 */
#define PPE_RCB_COM_ETS_TC_LINK_TCG_0_REG             (PPE_RCB_COM_BASE + 0x1EA0)
#define PPE_RCB_COM_ETS_TC_LINK_TCG_1_REG             (PPE_RCB_COM_BASE + 0x1EA4)
#define PPE_RCB_COM_ETS_TC_LINK_TCG_2_REG             (PPE_RCB_COM_BASE + 0x1EA8)
#define PPE_RCB_COM_ETS_TC_LINK_TCG_3_REG             (PPE_RCB_COM_BASE + 0x1EAC)
#define PPE_RCB_COM_ETS_TC_LINK_TCG_4_REG             (PPE_RCB_COM_BASE + 0x1EB0)
#define PPE_RCB_COM_ETS_TC_LINK_TCG_5_REG             (PPE_RCB_COM_BASE + 0x1EB4)
#define PPE_RCB_COM_ETS_TC_LINK_TCG_6_REG             (PPE_RCB_COM_BASE + 0x1EB8)
#define PPE_RCB_COM_ETS_TC_LINK_TCG_7_REG             (PPE_RCB_COM_BASE + 0x1EBC)
#define PPE_RCB_COM_ETS_TC_LINK_TCG_8_REG             (PPE_RCB_COM_BASE + 0x1EC0)
#define PPE_RCB_COM_ETS_TC_LINK_TCG_9_REG             (PPE_RCB_COM_BASE + 0x1EC4)
#define PPE_RCB_COM_ETS_TC_LINK_TCG_10_REG            (PPE_RCB_COM_BASE + 0x1EC8)
#define PPE_RCB_COM_ETS_TC_LINK_TCG_11_REG            (PPE_RCB_COM_BASE + 0x1ECC)
#define PPE_RCB_COM_ETS_TC_LINK_TCG_12_REG            (PPE_RCB_COM_BASE + 0x1ED0)
#define PPE_RCB_COM_ETS_TC_LINK_TCG_13_REG            (PPE_RCB_COM_BASE + 0x1ED4)
#define PPE_RCB_COM_ETS_TC_LINK_TCG_14_REG            (PPE_RCB_COM_BASE + 0x1ED8)
#define PPE_RCB_COM_ETS_TC_LINK_TCG_15_REG            (PPE_RCB_COM_BASE + 0x1EDC)

/* ETS MAC侧 TCG node 级的TCG绑定关系配置 */
#define PPE_RCB_COM_ETS_TCG_LINK_PORT_0_REG           (PPE_RCB_COM_BASE + 0x1EE0)
#define PPE_RCB_COM_ETS_TCG_LINK_PORT_1_REG           (PPE_RCB_COM_BASE + 0x1EE4)
#define PPE_RCB_COM_ETS_TCG_LINK_PORT_2_REG           (PPE_RCB_COM_BASE + 0x1EE8)
#define PPE_RCB_COM_ETS_TCG_LINK_PORT_3_REG           (PPE_RCB_COM_BASE + 0x1EEC)

#define PPE_RCB_COM_RCB_TX_Q_INTCTL_0_REG             (PPE_RCB_COM_BASE + 0x2000) /* 发送队列中断配置 */
#define PPE_RCB_COM_RCB_TX_Q_INTCTL_1_REG             (PPE_RCB_COM_BASE + 0x2040) /* 发送队列中断配置 */
#define PPE_RCB_COM_RCB_TX_Q_INTCTL_2_REG             (PPE_RCB_COM_BASE + 0x2080) /* 发送队列中断配置 */
#define PPE_RCB_COM_RCB_TX_Q_INTCTL_3_REG             (PPE_RCB_COM_BASE + 0x20C0) /* 发送队列中断配置 */
#define PPE_RCB_COM_RCB_TX_Q_INTCTL_4_REG             (PPE_RCB_COM_BASE + 0x2100) /* 发送队列中断配置 */
#define PPE_RCB_COM_RCB_TX_Q_INTCTL_5_REG             (PPE_RCB_COM_BASE + 0x2140) /* 发送队列中断配置 */
#define PPE_RCB_COM_RCB_TX_Q_INTCTL_6_REG             (PPE_RCB_COM_BASE + 0x2180) /* 发送队列中断配置 */
#define PPE_RCB_COM_RCB_TX_Q_INTCTL_7_REG             (PPE_RCB_COM_BASE + 0x21C0) /* 发送队列中断配置 */
#define PPE_RCB_COM_RCB_TX_Q_INTCTL_8_REG             (PPE_RCB_COM_BASE + 0x2200) /* 发送队列中断配置 */
#define PPE_RCB_COM_RCB_TX_Q_INTCTL_9_REG             (PPE_RCB_COM_BASE + 0x2240) /* 发送队列中断配置 */
#define PPE_RCB_COM_RCB_TX_Q_INTCTL_10_REG            (PPE_RCB_COM_BASE + 0x2280) /* 发送队列中断配置 */
#define PPE_RCB_COM_RCB_TX_Q_INTCTL_11_REG            (PPE_RCB_COM_BASE + 0x22C0) /* 发送队列中断配置 */
#define PPE_RCB_COM_RCB_TX_Q_INTCTL_12_REG            (PPE_RCB_COM_BASE + 0x2300) /* 发送队列中断配置 */
#define PPE_RCB_COM_RCB_TX_Q_INTCTL_13_REG            (PPE_RCB_COM_BASE + 0x2340) /* 发送队列中断配置 */
#define PPE_RCB_COM_RCB_TX_Q_INTCTL_14_REG            (PPE_RCB_COM_BASE + 0x2380) /* 发送队列中断配置 */
#define PPE_RCB_COM_RCB_TX_Q_INTCTL_15_REG            (PPE_RCB_COM_BASE + 0x23C0) /* 发送队列中断配置 */
#define PPE_RCB_COM_RCB_RX_Q_INTCTL_0_REG             (PPE_RCB_COM_BASE + 0x2004) /* 接收队列中断配置 */
#define PPE_RCB_COM_RCB_RX_Q_INTCTL_1_REG             (PPE_RCB_COM_BASE + 0x2044) /* 接收队列中断配置 */
#define PPE_RCB_COM_RCB_RX_Q_INTCTL_2_REG             (PPE_RCB_COM_BASE + 0x2084) /* 接收队列中断配置 */
#define PPE_RCB_COM_RCB_RX_Q_INTCTL_3_REG             (PPE_RCB_COM_BASE + 0x20C4) /* 接收队列中断配置 */
#define PPE_RCB_COM_RCB_RX_Q_INTCTL_4_REG             (PPE_RCB_COM_BASE + 0x2104) /* 接收队列中断配置 */
#define PPE_RCB_COM_RCB_RX_Q_INTCTL_5_REG             (PPE_RCB_COM_BASE + 0x2144) /* 接收队列中断配置 */
#define PPE_RCB_COM_RCB_RX_Q_INTCTL_6_REG             (PPE_RCB_COM_BASE + 0x2184) /* 接收队列中断配置 */
#define PPE_RCB_COM_RCB_RX_Q_INTCTL_7_REG             (PPE_RCB_COM_BASE + 0x21C4) /* 接收队列中断配置 */
#define PPE_RCB_COM_RCB_RX_Q_INTCTL_8_REG             (PPE_RCB_COM_BASE + 0x2204) /* 接收队列中断配置 */
#define PPE_RCB_COM_RCB_RX_Q_INTCTL_9_REG             (PPE_RCB_COM_BASE + 0x2244) /* 接收队列中断配置 */
#define PPE_RCB_COM_RCB_RX_Q_INTCTL_10_REG            (PPE_RCB_COM_BASE + 0x2284) /* 接收队列中断配置 */
#define PPE_RCB_COM_RCB_RX_Q_INTCTL_11_REG            (PPE_RCB_COM_BASE + 0x22C4) /* 接收队列中断配置 */
#define PPE_RCB_COM_RCB_RX_Q_INTCTL_12_REG            (PPE_RCB_COM_BASE + 0x2304) /* 接收队列中断配置 */
#define PPE_RCB_COM_RCB_RX_Q_INTCTL_13_REG            (PPE_RCB_COM_BASE + 0x2344) /* 接收队列中断配置 */
#define PPE_RCB_COM_RCB_RX_Q_INTCTL_14_REG            (PPE_RCB_COM_BASE + 0x2384) /* 接收队列中断配置 */
#define PPE_RCB_COM_RCB_RX_Q_INTCTL_15_REG            (PPE_RCB_COM_BASE + 0x23C4) /* 接收队列中断配置 */
#define PPE_RCB_COM_RCB_TQP_MAP_X_0_REG               (PPE_RCB_COM_BASE + 0x2008) /* RCB队列映射表的TCAM bit X配置 */
#define PPE_RCB_COM_RCB_TQP_MAP_X_1_REG               (PPE_RCB_COM_BASE + 0x2048) /* RCB队列映射表的TCAM bit X配置 */
#define PPE_RCB_COM_RCB_TQP_MAP_X_2_REG               (PPE_RCB_COM_BASE + 0x2088) /* RCB队列映射表的TCAM bit X配置 */
#define PPE_RCB_COM_RCB_TQP_MAP_X_3_REG               (PPE_RCB_COM_BASE + 0x20C8) /* RCB队列映射表的TCAM bit X配置 */
#define PPE_RCB_COM_RCB_TQP_MAP_X_4_REG               (PPE_RCB_COM_BASE + 0x2108) /* RCB队列映射表的TCAM bit X配置 */
#define PPE_RCB_COM_RCB_TQP_MAP_X_5_REG               (PPE_RCB_COM_BASE + 0x2148) /* RCB队列映射表的TCAM bit X配置 */
#define PPE_RCB_COM_RCB_TQP_MAP_X_6_REG               (PPE_RCB_COM_BASE + 0x2188) /* RCB队列映射表的TCAM bit X配置 */
#define PPE_RCB_COM_RCB_TQP_MAP_X_7_REG               (PPE_RCB_COM_BASE + 0x21C8) /* RCB队列映射表的TCAM bit X配置 */
#define PPE_RCB_COM_RCB_TQP_MAP_X_8_REG               (PPE_RCB_COM_BASE + 0x2208) /* RCB队列映射表的TCAM bit X配置 */
#define PPE_RCB_COM_RCB_TQP_MAP_X_9_REG               (PPE_RCB_COM_BASE + 0x2248) /* RCB队列映射表的TCAM bit X配置 */
#define PPE_RCB_COM_RCB_TQP_MAP_X_10_REG              (PPE_RCB_COM_BASE + 0x2288) /* RCB队列映射表的TCAM bit X配置 */
#define PPE_RCB_COM_RCB_TQP_MAP_X_11_REG              (PPE_RCB_COM_BASE + 0x22C8) /* RCB队列映射表的TCAM bit X配置 */
#define PPE_RCB_COM_RCB_TQP_MAP_X_12_REG              (PPE_RCB_COM_BASE + 0x2308) /* RCB队列映射表的TCAM bit X配置 */
#define PPE_RCB_COM_RCB_TQP_MAP_X_13_REG              (PPE_RCB_COM_BASE + 0x2348) /* RCB队列映射表的TCAM bit X配置 */
#define PPE_RCB_COM_RCB_TQP_MAP_X_14_REG              (PPE_RCB_COM_BASE + 0x2388) /* RCB队列映射表的TCAM bit X配置 */
#define PPE_RCB_COM_RCB_TQP_MAP_X_15_REG              (PPE_RCB_COM_BASE + 0x23C8) /* RCB队列映射表的TCAM bit X配置 */
#define PPE_RCB_COM_RCB_TQP_MAP_Y_0_REG               (PPE_RCB_COM_BASE + 0x200C) /* RCB队列映射表的TCAM bit Y配置 */
#define PPE_RCB_COM_RCB_TQP_MAP_Y_1_REG               (PPE_RCB_COM_BASE + 0x204C) /* RCB队列映射表的TCAM bit Y配置 */
#define PPE_RCB_COM_RCB_TQP_MAP_Y_2_REG               (PPE_RCB_COM_BASE + 0x208C) /* RCB队列映射表的TCAM bit Y配置 */
#define PPE_RCB_COM_RCB_TQP_MAP_Y_3_REG               (PPE_RCB_COM_BASE + 0x20CC) /* RCB队列映射表的TCAM bit Y配置 */
#define PPE_RCB_COM_RCB_TQP_MAP_Y_4_REG               (PPE_RCB_COM_BASE + 0x210C) /* RCB队列映射表的TCAM bit Y配置 */
#define PPE_RCB_COM_RCB_TQP_MAP_Y_5_REG               (PPE_RCB_COM_BASE + 0x214C) /* RCB队列映射表的TCAM bit Y配置 */
#define PPE_RCB_COM_RCB_TQP_MAP_Y_6_REG               (PPE_RCB_COM_BASE + 0x218C) /* RCB队列映射表的TCAM bit Y配置 */
#define PPE_RCB_COM_RCB_TQP_MAP_Y_7_REG               (PPE_RCB_COM_BASE + 0x21CC) /* RCB队列映射表的TCAM bit Y配置 */
#define PPE_RCB_COM_RCB_TQP_MAP_Y_8_REG               (PPE_RCB_COM_BASE + 0x220C) /* RCB队列映射表的TCAM bit Y配置 */
#define PPE_RCB_COM_RCB_TQP_MAP_Y_9_REG               (PPE_RCB_COM_BASE + 0x224C) /* RCB队列映射表的TCAM bit Y配置 */
#define PPE_RCB_COM_RCB_TQP_MAP_Y_10_REG              (PPE_RCB_COM_BASE + 0x228C) /* RCB队列映射表的TCAM bit Y配置 */
#define PPE_RCB_COM_RCB_TQP_MAP_Y_11_REG              (PPE_RCB_COM_BASE + 0x22CC) /* RCB队列映射表的TCAM bit Y配置 */
#define PPE_RCB_COM_RCB_TQP_MAP_Y_12_REG              (PPE_RCB_COM_BASE + 0x230C) /* RCB队列映射表的TCAM bit Y配置 */
#define PPE_RCB_COM_RCB_TQP_MAP_Y_13_REG              (PPE_RCB_COM_BASE + 0x234C) /* RCB队列映射表的TCAM bit Y配置 */
#define PPE_RCB_COM_RCB_TQP_MAP_Y_14_REG              (PPE_RCB_COM_BASE + 0x238C) /* RCB队列映射表的TCAM bit Y配置 */
#define PPE_RCB_COM_RCB_TQP_MAP_Y_15_REG              (PPE_RCB_COM_BASE + 0x23CC) /* RCB队列映射表的TCAM bit Y配置 */
#define PPE_RCB_COM_RCB_RING_TO_BE_RST_0_REG          (PPE_RCB_COM_BASE + 0x2010) /* 软件需要复位该队列的使能信号 */
#define PPE_RCB_COM_RCB_RING_TO_BE_RST_1_REG          (PPE_RCB_COM_BASE + 0x2050) /* 软件需要复位该队列的使能信号 */
#define PPE_RCB_COM_RCB_RING_TO_BE_RST_2_REG          (PPE_RCB_COM_BASE + 0x2090) /* 软件需要复位该队列的使能信号 */
#define PPE_RCB_COM_RCB_RING_TO_BE_RST_3_REG          (PPE_RCB_COM_BASE + 0x20D0) /* 软件需要复位该队列的使能信号 */
#define PPE_RCB_COM_RCB_RING_TO_BE_RST_4_REG          (PPE_RCB_COM_BASE + 0x2110) /* 软件需要复位该队列的使能信号 */
#define PPE_RCB_COM_RCB_RING_TO_BE_RST_5_REG          (PPE_RCB_COM_BASE + 0x2150) /* 软件需要复位该队列的使能信号 */
#define PPE_RCB_COM_RCB_RING_TO_BE_RST_6_REG          (PPE_RCB_COM_BASE + 0x2190) /* 软件需要复位该队列的使能信号 */
#define PPE_RCB_COM_RCB_RING_TO_BE_RST_7_REG          (PPE_RCB_COM_BASE + 0x21D0) /* 软件需要复位该队列的使能信号 */
#define PPE_RCB_COM_RCB_RING_TO_BE_RST_8_REG          (PPE_RCB_COM_BASE + 0x2210) /* 软件需要复位该队列的使能信号 */
#define PPE_RCB_COM_RCB_RING_TO_BE_RST_9_REG          (PPE_RCB_COM_BASE + 0x2250) /* 软件需要复位该队列的使能信号 */
#define PPE_RCB_COM_RCB_RING_TO_BE_RST_10_REG         (PPE_RCB_COM_BASE + 0x2290) /* 软件需要复位该队列的使能信号 */
#define PPE_RCB_COM_RCB_RING_TO_BE_RST_11_REG         (PPE_RCB_COM_BASE + 0x22D0) /* 软件需要复位该队列的使能信号 */
#define PPE_RCB_COM_RCB_RING_TO_BE_RST_12_REG         (PPE_RCB_COM_BASE + 0x2310) /* 软件需要复位该队列的使能信号 */
#define PPE_RCB_COM_RCB_RING_TO_BE_RST_13_REG         (PPE_RCB_COM_BASE + 0x2350) /* 软件需要复位该队列的使能信号 */
#define PPE_RCB_COM_RCB_RING_TO_BE_RST_14_REG         (PPE_RCB_COM_BASE + 0x2390) /* 软件需要复位该队列的使能信号 */
#define PPE_RCB_COM_RCB_RING_TO_BE_RST_15_REG         (PPE_RCB_COM_BASE + 0x23D0) /* 软件需要复位该队列的使能信号 */
#define PPE_RCB_COM_RCB_RING_COULD_BE_RST_0_REG       (PPE_RCB_COM_BASE + 0x2014) /* 硬件完成复位之后 */
#define PPE_RCB_COM_RCB_RING_COULD_BE_RST_1_REG       (PPE_RCB_COM_BASE + 0x2054) /* 硬件完成复位之后 */
#define PPE_RCB_COM_RCB_RING_COULD_BE_RST_2_REG       (PPE_RCB_COM_BASE + 0x2094) /* 硬件完成复位之后 */
#define PPE_RCB_COM_RCB_RING_COULD_BE_RST_3_REG       (PPE_RCB_COM_BASE + 0x20D4) /* 硬件完成复位之后 */
#define PPE_RCB_COM_RCB_RING_COULD_BE_RST_4_REG       (PPE_RCB_COM_BASE + 0x2114) /* 硬件完成复位之后 */
#define PPE_RCB_COM_RCB_RING_COULD_BE_RST_5_REG       (PPE_RCB_COM_BASE + 0x2154) /* 硬件完成复位之后 */
#define PPE_RCB_COM_RCB_RING_COULD_BE_RST_6_REG       (PPE_RCB_COM_BASE + 0x2194) /* 硬件完成复位之后 */
#define PPE_RCB_COM_RCB_RING_COULD_BE_RST_7_REG       (PPE_RCB_COM_BASE + 0x21D4) /* 硬件完成复位之后 */
#define PPE_RCB_COM_RCB_RING_COULD_BE_RST_8_REG       (PPE_RCB_COM_BASE + 0x2214) /* 硬件完成复位之后 */
#define PPE_RCB_COM_RCB_RING_COULD_BE_RST_9_REG       (PPE_RCB_COM_BASE + 0x2254) /* 硬件完成复位之后 */
#define PPE_RCB_COM_RCB_RING_COULD_BE_RST_10_REG      (PPE_RCB_COM_BASE + 0x2294) /* 硬件完成复位之后 */
#define PPE_RCB_COM_RCB_RING_COULD_BE_RST_11_REG      (PPE_RCB_COM_BASE + 0x22D4) /* 硬件完成复位之后 */
#define PPE_RCB_COM_RCB_RING_COULD_BE_RST_12_REG      (PPE_RCB_COM_BASE + 0x2314) /* 硬件完成复位之后 */
#define PPE_RCB_COM_RCB_RING_COULD_BE_RST_13_REG      (PPE_RCB_COM_BASE + 0x2354) /* 硬件完成复位之后 */
#define PPE_RCB_COM_RCB_RING_COULD_BE_RST_14_REG      (PPE_RCB_COM_BASE + 0x2394) /* 硬件完成复位之后 */
#define PPE_RCB_COM_RCB_RING_COULD_BE_RST_15_REG      (PPE_RCB_COM_BASE + 0x23D4) /* 硬件完成复位之后 */
#define PPE_RCB_COM_RCB_RING_PORT_MAP_0_REG           (PPE_RCB_COM_BASE + 0x2018) /* RING_NUM对应的PORT_NUM */
#define PPE_RCB_COM_RCB_RING_PORT_MAP_1_REG           (PPE_RCB_COM_BASE + 0x2058) /* RING_NUM对应的PORT_NUM */
#define PPE_RCB_COM_RCB_RING_PORT_MAP_2_REG           (PPE_RCB_COM_BASE + 0x2098) /* RING_NUM对应的PORT_NUM */
#define PPE_RCB_COM_RCB_RING_PORT_MAP_3_REG           (PPE_RCB_COM_BASE + 0x20D8) /* RING_NUM对应的PORT_NUM */
#define PPE_RCB_COM_RCB_RING_PORT_MAP_4_REG           (PPE_RCB_COM_BASE + 0x2118) /* RING_NUM对应的PORT_NUM */
#define PPE_RCB_COM_RCB_RING_PORT_MAP_5_REG           (PPE_RCB_COM_BASE + 0x2158) /* RING_NUM对应的PORT_NUM */
#define PPE_RCB_COM_RCB_RING_PORT_MAP_6_REG           (PPE_RCB_COM_BASE + 0x2198) /* RING_NUM对应的PORT_NUM */
#define PPE_RCB_COM_RCB_RING_PORT_MAP_7_REG           (PPE_RCB_COM_BASE + 0x21D8) /* RING_NUM对应的PORT_NUM */
#define PPE_RCB_COM_RCB_RING_PORT_MAP_8_REG           (PPE_RCB_COM_BASE + 0x2218) /* RING_NUM对应的PORT_NUM */
#define PPE_RCB_COM_RCB_RING_PORT_MAP_9_REG           (PPE_RCB_COM_BASE + 0x2258) /* RING_NUM对应的PORT_NUM */
#define PPE_RCB_COM_RCB_RING_PORT_MAP_10_REG          (PPE_RCB_COM_BASE + 0x2298) /* RING_NUM对应的PORT_NUM */
#define PPE_RCB_COM_RCB_RING_PORT_MAP_11_REG          (PPE_RCB_COM_BASE + 0x22D8) /* RING_NUM对应的PORT_NUM */
#define PPE_RCB_COM_RCB_RING_PORT_MAP_12_REG          (PPE_RCB_COM_BASE + 0x2318) /* RING_NUM对应的PORT_NUM */
#define PPE_RCB_COM_RCB_RING_PORT_MAP_13_REG          (PPE_RCB_COM_BASE + 0x2358) /* RING_NUM对应的PORT_NUM */
#define PPE_RCB_COM_RCB_RING_PORT_MAP_14_REG          (PPE_RCB_COM_BASE + 0x2398) /* RING_NUM对应的PORT_NUM */
#define PPE_RCB_COM_RCB_RING_PORT_MAP_15_REG          (PPE_RCB_COM_BASE + 0x23D8) /* RING_NUM对应的PORT_NUM */
#define PPE_RCB_COM_PF_ABNORMAL_INT_EN_0_REG          (PPE_RCB_COM_BASE + 0xC000) /* PF异常中断使能寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_EN_1_REG          (PPE_RCB_COM_BASE + 0xC080) /* PF异常中断使能寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_EN_2_REG          (PPE_RCB_COM_BASE + 0xC100) /* PF异常中断使能寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_EN_3_REG          (PPE_RCB_COM_BASE + 0xC180) /* PF异常中断使能寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_ST_0_REG          (PPE_RCB_COM_BASE + 0xC004) /* PF异常中断状态寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_ST_1_REG          (PPE_RCB_COM_BASE + 0xC084) /* PF异常中断状态寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_ST_2_REG          (PPE_RCB_COM_BASE + 0xC104) /* PF异常中断状态寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_ST_3_REG          (PPE_RCB_COM_BASE + 0xC184) /* PF异常中断状态寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_SRC_0_REG         (PPE_RCB_COM_BASE + 0xC008) /* PF异常中断源寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_SRC_1_REG         (PPE_RCB_COM_BASE + 0xC088) /* PF异常中断源寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_SRC_2_REG         (PPE_RCB_COM_BASE + 0xC108) /* PF异常中断源寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_SRC_3_REG         (PPE_RCB_COM_BASE + 0xC188) /* PF异常中断源寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_SET_0_REG         (PPE_RCB_COM_BASE + 0xC00C) /* PF异常中断注入寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_SET_1_REG         (PPE_RCB_COM_BASE + 0xC08C) /* PF异常中断注入寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_SET_2_REG         (PPE_RCB_COM_BASE + 0xC10C) /* PF异常中断注入寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_SET_3_REG         (PPE_RCB_COM_BASE + 0xC18C) /* PF异常中断注入寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_CE_0_REG          (PPE_RCB_COM_BASE + 0xC010) /* PF异常中断源CE寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_CE_1_REG          (PPE_RCB_COM_BASE + 0xC090) /* PF异常中断源CE寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_CE_2_REG          (PPE_RCB_COM_BASE + 0xC110) /* PF异常中断源CE寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_CE_3_REG          (PPE_RCB_COM_BASE + 0xC190) /* PF异常中断源CE寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_NFE_0_REG         (PPE_RCB_COM_BASE + 0xC014) /* PF异常中断源NFE寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_NFE_1_REG         (PPE_RCB_COM_BASE + 0xC094) /* PF异常中断源NFE寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_NFE_2_REG         (PPE_RCB_COM_BASE + 0xC114) /* PF异常中断源NFE寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_NFE_3_REG         (PPE_RCB_COM_BASE + 0xC194) /* PF异常中断源NFE寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_FE_0_REG          (PPE_RCB_COM_BASE + 0xC018) /* PF异常中断源FE寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_FE_1_REG          (PPE_RCB_COM_BASE + 0xC098) /* PF异常中断源FE寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_FE_2_REG          (PPE_RCB_COM_BASE + 0xC118) /* PF异常中断源FE寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_FE_3_REG          (PPE_RCB_COM_BASE + 0xC198) /* PF异常中断源FE寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_TYPE_0_REG        (PPE_RCB_COM_BASE + 0xC01C) /* PF异常中断类型寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_TYPE_1_REG        (PPE_RCB_COM_BASE + 0xC09C) /* PF异常中断类型寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_TYPE_2_REG        (PPE_RCB_COM_BASE + 0xC11C) /* PF异常中断类型寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_TYPE_3_REG        (PPE_RCB_COM_BASE + 0xC19C) /* PF异常中断类型寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_DFX0_0_REG        (PPE_RCB_COM_BASE + 0xC020) /* PF异常中断出错队列号寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_DFX0_1_REG        (PPE_RCB_COM_BASE + 0xC0A0) /* PF异常中断出错队列号寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_DFX0_2_REG        (PPE_RCB_COM_BASE + 0xC120) /* PF异常中断出错队列号寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_DFX0_3_REG        (PPE_RCB_COM_BASE + 0xC1A0) /* PF异常中断出错队列号寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_DFX1_0_REG        (PPE_RCB_COM_BASE + 0xC024) /* PF异常中断出错队列号寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_DFX1_1_REG        (PPE_RCB_COM_BASE + 0xC0A4) /* PF异常中断出错队列号寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_DFX1_2_REG        (PPE_RCB_COM_BASE + 0xC124) /* PF异常中断出错队列号寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_DFX1_3_REG        (PPE_RCB_COM_BASE + 0xC1A4) /* PF异常中断出错队列号寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_DFX2_0_REG        (PPE_RCB_COM_BASE + 0xC028) /* PF异常中断出错队列号寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_DFX2_1_REG        (PPE_RCB_COM_BASE + 0xC0A8) /* PF异常中断出错队列号寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_DFX2_2_REG        (PPE_RCB_COM_BASE + 0xC128) /* PF异常中断出错队列号寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_DFX2_3_REG        (PPE_RCB_COM_BASE + 0xC1A8) /* PF异常中断出错队列号寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_DFX3_0_REG        (PPE_RCB_COM_BASE + 0xC02C) /* PF异常中断出错队列号寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_DFX3_1_REG        (PPE_RCB_COM_BASE + 0xC0AC) /* PF异常中断出错队列号寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_DFX3_2_REG        (PPE_RCB_COM_BASE + 0xC12C) /* PF异常中断出错队列号寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_DFX3_3_REG        (PPE_RCB_COM_BASE + 0xC1AC) /* PF异常中断出错队列号寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_DFX4_0_REG        (PPE_RCB_COM_BASE + 0xC030) /* PF异常中断出错队列号寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_DFX4_1_REG        (PPE_RCB_COM_BASE + 0xC0B0) /* PF异常中断出错队列号寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_DFX4_2_REG        (PPE_RCB_COM_BASE + 0xC130) /* PF异常中断出错队列号寄存器 */
#define PPE_RCB_COM_PF_ABNORMAL_INT_DFX4_3_REG        (PPE_RCB_COM_BASE + 0xC1B0) /* PF异常中断出错队列号寄存器 */

#endif // __REG_RCB_COM_OFFSET_H__
