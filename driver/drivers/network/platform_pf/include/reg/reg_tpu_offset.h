/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: reg_tpu_offset
 * Author: huawei
 * Create: 2021-12-28
 */

#ifndef __REG_TPU_OFFSET_H__
#define __REG_TPU_OFFSET_H__

/* PPE_TPU Base address of Module's Register */
#define PPE_TPU_BASE                       (0x0)

/******************************************************************************/
/*                      xxx PPE_TPU Registers' Definitions                    */
/******************************************************************************/

#define PPE_TPU_TPU_INT_ENABLE_REG                  (PPE_TPU_BASE + 0x0)    /* 中断使能寄存器 */
#define PPE_TPU_TPU_INT_STATUS_REG                  (PPE_TPU_BASE + 0x200)  /* 中断状态寄存器 */
#define PPE_TPU_TPU_INT_SET_REG                     (PPE_TPU_BASE + 0x400)  /* 中断注入寄存器 */
#define PPE_TPU_TPU_INT_SOURCE_REG                  (PPE_TPU_BASE + 0x600)  /* 原始中断状态寄存器(记录第一次的中断状态，写1清中断） */
#define PPE_TPU_TPU_MEM_ERR_INSERT1_REG             (PPE_TPU_BASE + 0x804)  /* PPE TPU MEMORY错误植入 寄存器1 */
#define PPE_TPU_TPU_MEM_ERR_INFO_REG                (PPE_TPU_BASE + 0x810)  /* PPE TPU MEMORY错误记录的伴随信息 */
#define PPE_TPU_TPU_MEM_ERR_INFO1_REG               (PPE_TPU_BASE + 0x814)  /* PPE TPU MEMORY错误记录的伴随信息1 */
#define PPE_TPU_TPU_TX_AXI_ERR_INFO_REG             (PPE_TPU_BASE + 0x840)  /* PPE TPU读写AXI异常伴随信息 */
#define PPE_TPU_TPU_FIFO_WR_INT0_REG                (PPE_TPU_BASE + 0xA00)  /* PPE TPU模块FIFO满写 中断寄存器0 */
#define PPE_TPU_TPU_FIFO_WR_INT1_REG                (PPE_TPU_BASE + 0xA04)  /* PPE TPU模块FIFO满写 中断寄存器1 */
#define PPE_TPU_TPU_FIFO_WR_INT2_REG                (PPE_TPU_BASE + 0xA08)  /* PPE TPU模块FIFO满写 中断寄存器2 */
#define PPE_TPU_TPU_FIFO_WR_INT3_REG                (PPE_TPU_BASE + 0xA0C)  /* PPE TPU模块FIFO满写 中断寄存器3 */
#define PPE_TPU_TPU_FIFO_WR_INT4_REG                (PPE_TPU_BASE + 0xA10)  /* PPE TPU模块FIFO满写 中断寄存器4 */
#define PPE_TPU_TPU_FIFO_WR_INT5_REG                (PPE_TPU_BASE + 0xA14)  /* PPE TPU模块FIFO满写 中断寄存器5 */
#define PPE_TPU_TPU_FIFO_WR_INT6_REG                (PPE_TPU_BASE + 0xA18)  /* PPE TPU模块FIFO满写 中断寄存器6 */
#define PPE_TPU_TPU_FIFO_WR_INT7_REG                (PPE_TPU_BASE + 0xA1C)  /* PPE TPU模块FIFO满写 中断寄存器7 */
#define PPE_TPU_TPU_FIFO_WR_INT8_REG                (PPE_TPU_BASE + 0xA20)  /* PPE TPU模块FIFO满写 中断寄存器8 */
#define PPE_TPU_TPU_FIFO_WR_INT9_REG                (PPE_TPU_BASE + 0xA24)  /* PPE TPU模块FIFO满写 中断寄存器9 */
#define PPE_TPU_TPU_FIFO_WR_INT10_REG               (PPE_TPU_BASE + 0xA28)  /* PPE TPU模块FIFO满写 中断寄存器10 */
#define PPE_TPU_TPU_FIFO_WR_INT11_REG               (PPE_TPU_BASE + 0xA2C)  /* PPE TPU模块FIFO满写 中断寄存器11 */
#define PPE_TPU_TPU_FIFO_WR_INT12_REG               (PPE_TPU_BASE + 0xA30)  /* PPE TPU模块FIFO满写 中断寄存器12 */
#define PPE_TPU_TPU_FIFO_WR_INT13_REG               (PPE_TPU_BASE + 0xA34)  /* PPE TPU模块FIFO满写 中断寄存器13 */
#define PPE_TPU_TPU_FIFO_WR_INT14_REG               (PPE_TPU_BASE + 0xA38)  /* PPE TPU模块FIFO满写 中断寄存器14 */
#define PPE_TPU_TPU_FIFO_WR_INT15_REG               (PPE_TPU_BASE + 0xA3C)  /* PPE TPU模块FIFO满写 中断寄存器15 */
#define PPE_TPU_TPU_FIFO_WR_INT16_REG               (PPE_TPU_BASE + 0xA40)  /* PPE TPU模块FIFO满写 中断寄存器16 */
#define PPE_TPU_TPU_FIFO_WR_INT17_REG               (PPE_TPU_BASE + 0xA44)  /* PPE TPU模块FIFO满写 中断寄存器17 */
#define PPE_TPU_TPU_FIFO_WR_INT18_REG               (PPE_TPU_BASE + 0xA48)  /* PPE TPU模块FIFO满写 中断寄存器18 */
#define PPE_TPU_TPU_FIFO_RD_INT0_REG                (PPE_TPU_BASE + 0xB00)  /* PPE TPU模块FIFO空读 中断寄存器0 */
#define PPE_TPU_TPU_FIFO_RD_INT1_REG                (PPE_TPU_BASE + 0xB04)  /* PPE TPU模块FIFO空读 中断寄存器1 */
#define PPE_TPU_TPU_FIFO_RD_INT2_REG                (PPE_TPU_BASE + 0xB08)  /* PPE TPU模块FIFO空读 中断寄存器2 */
#define PPE_TPU_TPU_FIFO_RD_INT3_REG                (PPE_TPU_BASE + 0xB0C)  /* PPE TPU模块FIFO空读 中断寄存器3 */
#define PPE_TPU_TPU_FIFO_RD_INT4_REG                (PPE_TPU_BASE + 0xB10)  /* PPE TPU模块FIFO空读 中断寄存器4 */
#define PPE_TPU_TPU_FIFO_RD_INT5_REG                (PPE_TPU_BASE + 0xB14)  /* PPE TPU模块FIFO空读 中断寄存器5 */
#define PPE_TPU_TPU_FIFO_RD_INT6_REG                (PPE_TPU_BASE + 0xB18)  /* PPE TPU模块FIFO空读 中断寄存器6 */
#define PPE_TPU_TPU_FIFO_RD_INT7_REG                (PPE_TPU_BASE + 0xB1C)  /* PPE TPU模块FIFO空读 中断寄存器7 */
#define PPE_TPU_TPU_FIFO_RD_INT8_REG                (PPE_TPU_BASE + 0xB20)  /* PPE TPU模块FIFO空读 中断寄存器8 */
#define PPE_TPU_TPU_FIFO_RD_INT9_REG                (PPE_TPU_BASE + 0xB24)  /* PPE TPU模块FIFO空读 中断寄存器9 */
#define PPE_TPU_TPU_FIFO_RD_INT10_REG               (PPE_TPU_BASE + 0xB28)  /* PPE TPU模块FIFO空读 中断寄存器10 */
#define PPE_TPU_TPU_FIFO_RD_INT11_REG               (PPE_TPU_BASE + 0xB2C)  /* PPE TPU模块FIFO空读 中断寄存器11 */
#define PPE_TPU_TPU_FIFO_RD_INT12_REG               (PPE_TPU_BASE + 0xB30)  /* PPE TPU模块FIFO空读 中断寄存器12 */
#define PPE_TPU_TPU_FIFO_RD_INT13_REG               (PPE_TPU_BASE + 0xB34)  /* PPE TPU模块FIFO空读 中断寄存器13 */
#define PPE_TPU_TPU_FIFO_RD_INT14_REG               (PPE_TPU_BASE + 0xB38)  /* PPE TPU模块FIFO空读 中断寄存器14 */
#define PPE_TPU_TPU_FIFO_RD_INT15_REG               (PPE_TPU_BASE + 0xB3C)  /* PPE TPU模块FIFO空读 中断寄存器15 */
#define PPE_TPU_TPU_FIFO_RD_INT16_REG               (PPE_TPU_BASE + 0xB40)  /* PPE TPU模块FIFO空读 中断寄存器16 */
#define PPE_TPU_TPU_FIFO_RD_INT17_REG               (PPE_TPU_BASE + 0xB44)  /* PPE TPU模块FIFO空读 中断寄存器17 */
#define PPE_TPU_TPU_FIFO_RD_INT18_REG               (PPE_TPU_BASE + 0xB48)  /* PPE TPU模块FIFO空读 中断寄存器18 */
#define PPE_TPU_TPU_PF_INT_TYPE0_REG                (PPE_TPU_BASE + 0xE00)  /* 中断上报类型寄存器0 */
#define PPE_TPU_TPU_INT_CE0_REG                     (PPE_TPU_BASE + 0x1200) /* CE中断寄存器0 */
#define PPE_TPU_TPU_INT_NFE0_REG                    (PPE_TPU_BASE + 0x1400) /* NFE中断寄存器0 */
#define PPE_TPU_TPU_INT_FE0_REG                     (PPE_TPU_BASE + 0x1600) /* FE中断寄存器0 */
#define PPE_TPU_TPU_MEM_INIT_START_REG              (PPE_TPU_BASE + 0x2800) /* 软件配置内部SRAM开始初始化 */
#define PPE_TPU_TPU_MEM_INIT_DONE_REG               (PPE_TPU_BASE + 0x2804) /* TPU内部的SRAM初始化完成 */
#define PPE_TPU_TPU_MEM_CTRL_TPTMOD_REG             (PPE_TPU_BASE + 0x2808) /* TPMEMORY控制寄存器 */
#define PPE_TPU_TPU_MEM_CTRL_SPTMOD_REG             (PPE_TPU_BASE + 0x280C) /* SPMEMORY控制寄存器 */
#define PPE_TPU_TPU_MEM_CTRL_POWER_CTRL_REG         (PPE_TPU_BASE + 0x2810) /* MEMORY控制power */
#define PPE_TPU_TPU_MEM_ECC_BYPASS_REG              (PPE_TPU_BASE + 0x2814) /* memory ecc bypass使能 */
#define PPE_TPU_TPU_MEM_SERR_THD_REG                \
    (PPE_TPU_BASE + 0x2818) /* PPE TPU内部MEMORY错误统计 门限配置寄存器(针对1bit ECC错误) */
#define PPE_TPU_TPU_ECO0_REG                        (PPE_TPU_BASE + 0x2900) /* TPU ECO预留寄存器 */
#define PPE_TPU_TPU_ECO1_REG                        (PPE_TPU_BASE + 0x2904) /* TPU ECO预留寄存器 */
#define PPE_TPU_TPU_FSM_HEAT_TIMEOUT_REG            (PPE_TPU_BASE + 0x2908) /* PPE TPU内部状态机超时 检查时间配置寄存器 */
#define PPE_TPU_TPU_CNT_CLR_CE_REG                  (PPE_TPU_BASE + 0x290C) /* CNT_CYC类型寄存器读清使能 */
#define PPE_TPU_TPU_NS_REG_REG                      \
    (PPE_TPU_BASE + 0x2910) /* TPU模块安全锁定寄存器，内部*SEC_FIELD_LOCKED类型的寄存器仅在该寄存器处于解锁状态才可改配。 */
#define PPE_TPU_TPU_MECC_PROCESS_SEL_REG            (PPE_TPU_BASE + 0x2920) /* TPU模块在出现内部ECC多bit错误时，出口报文处理方式选择 */
#define PPE_TPU_TPU_CLK_GATING_CTRL_REG             (PPE_TPU_BASE + 0x2A00) /* 时钟自动门控使能寄存器。 */
#define PPE_TPU_TPU_GATING_WINDOW_REG               (PPE_TPU_BASE + 0x2A04) /* 统计门控打开的窗口大小，单位是64K个该模块工作时钟的。 */
#define PPE_TPU_TPU_GATING_STS_REG                  (PPE_TPU_BASE + 0x2A08) /* 统计门控打开的窗口中打开时钟的周期，单位是64K个该模块工作时钟的。 */
#define PPE_TPU_TPU_MAX_PKTLEN_REG                  (PPE_TPU_BASE + 0x3000) /* PPE TPU最大报文长度配置 */
#define PPE_TPU_TPU_MIN_PKTLEN_PADDING_EN_0_REG     (PPE_TPU_BASE + 0x3208) /* PPE TUP短包是否加padding使能 */
#define PPE_TPU_TPU_MIN_PKTLEN_PADDING_EN_1_REG     (PPE_TPU_BASE + 0x3248) /* PPE TUP短包是否加padding使能 */
#define PPE_TPU_TPU_MIN_PKTLEN_PADDING_EN_2_REG     (PPE_TPU_BASE + 0x3288) /* PPE TUP短包是否加padding使能 */
#define PPE_TPU_TPU_MIN_PKTLEN_PADDING_EN_3_REG     (PPE_TPU_BASE + 0x32C8) /* PPE TUP短包是否加padding使能 */
#define PPE_TPU_TPU_TX_WL_CNT_0_REG                 (PPE_TPU_BASE + 0x320C) /* TX方向发送水线配置 */
#define PPE_TPU_TPU_TX_WL_CNT_1_REG                 (PPE_TPU_BASE + 0x324C) /* TX方向发送水线配置 */
#define PPE_TPU_TPU_TX_WL_CNT_2_REG                 (PPE_TPU_BASE + 0x328C) /* TX方向发送水线配置 */
#define PPE_TPU_TPU_TX_WL_CNT_3_REG                 (PPE_TPU_BASE + 0x32CC) /* TX方向发送水线配置 */
#define PPE_TPU_TPU_OUTSTAND_CTRL_REG               (PPE_TPU_BASE + 0x5000) /* PPE TPU OUTSTAND配置寄存器 */
#define PPE_TPU_TPU_PKT_RD_BURST_CTRL_0_REG         (PPE_TPU_BASE + 0x5004) /* 读报文总线操作间隔配置寄存器0 */
#define PPE_TPU_TPU_PKT_RD_BURST_CTRL_1_REG         (PPE_TPU_BASE + 0x5008) /* 读报文总线操作间隔配置寄存器1 */
#define PPE_TPU_TPU_PLY_STORE_GAP_REG               (PPE_TPU_BASE + 0x5010) /* PPE TPU PLY FIFO总可用深度GAP */
#define PPE_TPU_TPU_PREBD_FIFO_THR_REG              (PPE_TPU_BASE + 0x5020) /* PPE 预取描述符FIFO门限配置寄存器 */
#define PPE_TPU_TPU_TNL_FIFO_DEPTH_0_REG            (PPE_TPU_BASE + 0x5100) /* PPE TPU 每个通道预取报文分配的FIFO深度配置 */
#define PPE_TPU_TPU_TNL_FIFO_DEPTH_1_REG            (PPE_TPU_BASE + 0x5110) /* PPE TPU 每个通道预取报文分配的FIFO深度配置 */
#define PPE_TPU_TPU_TNL_FIFO_DEPTH_2_REG            (PPE_TPU_BASE + 0x5120) /* PPE TPU 每个通道预取报文分配的FIFO深度配置 */
#define PPE_TPU_TPU_TNL_FIFO_DEPTH_3_REG            (PPE_TPU_BASE + 0x5130) /* PPE TPU 每个通道预取报文分配的FIFO深度配置 */
#define PPE_TPU_TPU_ENDIAN_CTRL_0_REG               (PPE_TPU_BASE + 0x5A00) /* PPE TPU数据大小端配置寄存器 */
#define PPE_TPU_TPU_ENDIAN_CTRL_1_REG               (PPE_TPU_BASE + 0x5A18) /* PPE TPU数据大小端配置寄存器 */
#define PPE_TPU_TPU_ENDIAN_CTRL_2_REG               (PPE_TPU_BASE + 0x5A30) /* PPE TPU数据大小端配置寄存器 */
#define PPE_TPU_TPU_ENDIAN_CTRL_3_REG               (PPE_TPU_BASE + 0x5A48) /* PPE TPU数据大小端配置寄存器 */
#define PPE_TPU_TPU_TSO_OIP_ID_TYPE_REG             (PPE_TPU_BASE + 0x6000) /* 隧道报文TSO外层IP的ID配置寄存器 */
#define PPE_TPU_TPU_TSO_FLAG_MSK_FST_REG            (PPE_TPU_BASE + 0x6004) /* TSO的首segment的flag mask */
#define PPE_TPU_TPU_TSO_FLAG_MSK_MID_REG            (PPE_TPU_BASE + 0x6008) /* TSO的中间segment的flag mask */
#define PPE_TPU_TPU_TSO_FLAG_MSK_LAST_REG           (PPE_TPU_BASE + 0x600C) /* TSO的尾segment的flag mask */
#define PPE_TPU_TPU_L3CKS_FIFO_GAP_REG              (PPE_TPU_BASE + 0x6100) /* TPU L3 checksum配置寄存器 */
#define PPE_TPU_TPU_RB_WB_FIFO_AGUL_GAP_REG         (PPE_TPU_BASE + 0x6700) /* Ring Buffer模式回写功能TC调度权重 */
#define PPE_TPU_TPU_RB_TC_WEIGHT_0_REG              (PPE_TPU_BASE + 0x6800) /* Ring Buffer模式各TC描述符调度权重配置 */
#define PPE_TPU_TPU_RB_TC_WEIGHT_1_REG              (PPE_TPU_BASE + 0x6804) /* Ring Buffer模式各TC描述符调度权重配置 */
#define PPE_TPU_TPU_RB_TC_WEIGHT_2_REG              (PPE_TPU_BASE + 0x6808) /* Ring Buffer模式各TC描述符调度权重配置 */
#define PPE_TPU_TPU_RB_TC_WEIGHT_3_REG              (PPE_TPU_BASE + 0x680C) /* Ring Buffer模式各TC描述符调度权重配置 */
#define PPE_TPU_TPU_DDR_PKT_SMMU_USER_STRMID_0_REG  (PPE_TPU_BASE + 0x7004) /* 访问DDR报文时，总线的user中streamid域配置。 */
#define PPE_TPU_TPU_DDR_PKT_SMMU_USER_STRMID_1_REG  (PPE_TPU_BASE + 0x7024) /* 访问DDR报文时，总线的user中streamid域配置。 */
#define PPE_TPU_TPU_DDR_PKT_SMMU_USER_STRMID_2_REG  (PPE_TPU_BASE + 0x7044) /* 访问DDR报文时，总线的user中streamid域配置。 */
#define PPE_TPU_TPU_DDR_PKT_SMMU_USER_STRMID_3_REG  (PPE_TPU_BASE + 0x7064) /* 访问DDR报文时，总线的user中streamid域配置。 */
#define PPE_TPU_TPU_DDR_PKT_SMMU_USER_STRMID_4_REG  (PPE_TPU_BASE + 0x7084) /* 访问DDR报文时，总线的user中streamid域配置。 */
#define PPE_TPU_TPU_DDR_PKT_SMMU_USER_STRMID_5_REG  (PPE_TPU_BASE + 0x70A4) /* 访问DDR报文时，总线的user中streamid域配置。 */
#define PPE_TPU_TPU_DDR_PKT_SMMU_USER_STRMID_6_REG  (PPE_TPU_BASE + 0x70C4) /* 访问DDR报文时，总线的user中streamid域配置。 */
#define PPE_TPU_TPU_DDR_PKT_SMMU_USER_STRMID_7_REG  (PPE_TPU_BASE + 0x70E4) /* 访问DDR报文时，总线的user中streamid域配置。 */
#define PPE_TPU_TPU_DDR_PKT_SMMU_USER_STRMID_8_REG  (PPE_TPU_BASE + 0x7104) /* 访问DDR报文时，总线的user中streamid域配置。 */
#define PPE_TPU_TPU_DDR_PKT_SMMU_USER_STRMID_9_REG  (PPE_TPU_BASE + 0x7124) /* 访问DDR报文时，总线的user中streamid域配置。 */
#define PPE_TPU_TPU_DDR_PKT_SMMU_USER_STRMID_10_REG (PPE_TPU_BASE + 0x7144) /* 访问DDR报文时，总线的user中streamid域配置。 */
#define PPE_TPU_TPU_DDR_PKT_SMMU_USER_STRMID_11_REG (PPE_TPU_BASE + 0x7164) /* 访问DDR报文时，总线的user中streamid域配置。 */
#define PPE_TPU_TPU_DDR_PKT_SMMU_USER_STRMID_12_REG (PPE_TPU_BASE + 0x7184) /* 访问DDR报文时，总线的user中streamid域配置。 */
#define PPE_TPU_TPU_DDR_PKT_SMMU_USER_STRMID_13_REG (PPE_TPU_BASE + 0x71A4) /* 访问DDR报文时，总线的user中streamid域配置。 */
#define PPE_TPU_TPU_DDR_PKT_SMMU_USER_STRMID_14_REG (PPE_TPU_BASE + 0x71C4) /* 访问DDR报文时，总线的user中streamid域配置。 */
#define PPE_TPU_TPU_DDR_PKT_SMMU_USER_STRMID_15_REG (PPE_TPU_BASE + 0x71E4) /* 访问DDR报文时，总线的user中streamid域配置。 */
#define PPE_TPU_TPU_SMMU_USER_SSET_0_REG            (PPE_TPU_BASE + 0x7008) /* 访问总线的user中sset域配置。 */
#define PPE_TPU_TPU_SMMU_USER_SSET_1_REG            (PPE_TPU_BASE + 0x7028) /* 访问总线的user中sset域配置。 */
#define PPE_TPU_TPU_SMMU_USER_SSET_2_REG            (PPE_TPU_BASE + 0x7048) /* 访问总线的user中sset域配置。 */
#define PPE_TPU_TPU_SMMU_USER_SSET_3_REG            (PPE_TPU_BASE + 0x7068) /* 访问总线的user中sset域配置。 */
#define PPE_TPU_TPU_SMMU_USER_SSET_4_REG            (PPE_TPU_BASE + 0x7088) /* 访问总线的user中sset域配置。 */
#define PPE_TPU_TPU_SMMU_USER_SSET_5_REG            (PPE_TPU_BASE + 0x70A8) /* 访问总线的user中sset域配置。 */
#define PPE_TPU_TPU_SMMU_USER_SSET_6_REG            (PPE_TPU_BASE + 0x70C8) /* 访问总线的user中sset域配置。 */
#define PPE_TPU_TPU_SMMU_USER_SSET_7_REG            (PPE_TPU_BASE + 0x70E8) /* 访问总线的user中sset域配置。 */
#define PPE_TPU_TPU_SMMU_USER_SSET_8_REG            (PPE_TPU_BASE + 0x7108) /* 访问总线的user中sset域配置。 */
#define PPE_TPU_TPU_SMMU_USER_SSET_9_REG            (PPE_TPU_BASE + 0x7128) /* 访问总线的user中sset域配置。 */
#define PPE_TPU_TPU_SMMU_USER_SSET_10_REG           (PPE_TPU_BASE + 0x7148) /* 访问总线的user中sset域配置。 */
#define PPE_TPU_TPU_SMMU_USER_SSET_11_REG           (PPE_TPU_BASE + 0x7168) /* 访问总线的user中sset域配置。 */
#define PPE_TPU_TPU_SMMU_USER_SSET_12_REG           (PPE_TPU_BASE + 0x7188) /* 访问总线的user中sset域配置。 */
#define PPE_TPU_TPU_SMMU_USER_SSET_13_REG           (PPE_TPU_BASE + 0x71A8) /* 访问总线的user中sset域配置。 */
#define PPE_TPU_TPU_SMMU_USER_SSET_14_REG           (PPE_TPU_BASE + 0x71C8) /* 访问总线的user中sset域配置。 */
#define PPE_TPU_TPU_SMMU_USER_SSET_15_REG           (PPE_TPU_BASE + 0x71E8) /* 访问总线的user中sset域配置。 */
#define PPE_TPU_TPU_DDR_SMMU_USER_0_REG             (PPE_TPU_BASE + 0x700C) /* AXI总线，读写DDR时AXUSER 配置项 */
#define PPE_TPU_TPU_DDR_SMMU_USER_1_REG             (PPE_TPU_BASE + 0x702C) /* AXI总线，读写DDR时AXUSER 配置项 */
#define PPE_TPU_TPU_DDR_SMMU_USER_2_REG             (PPE_TPU_BASE + 0x704C) /* AXI总线，读写DDR时AXUSER 配置项 */
#define PPE_TPU_TPU_DDR_SMMU_USER_3_REG             (PPE_TPU_BASE + 0x706C) /* AXI总线，读写DDR时AXUSER 配置项 */
#define PPE_TPU_TPU_DDR_SMMU_USER1_0_REG            (PPE_TPU_BASE + 0x7010) /* AXI总线，读写DDR时AXUSER 配置项1 */
#define PPE_TPU_TPU_DDR_SMMU_USER1_1_REG            (PPE_TPU_BASE + 0x7030) /* AXI总线，读写DDR时AXUSER 配置项1 */
#define PPE_TPU_TPU_DDR_SMMU_USER1_2_REG            (PPE_TPU_BASE + 0x7050) /* AXI总线，读写DDR时AXUSER 配置项1 */
#define PPE_TPU_TPU_DDR_SMMU_USER1_3_REG            (PPE_TPU_BASE + 0x7070) /* AXI总线，读写DDR时AXUSER 配置项1 */
#define PPE_TPU_TPU_DDR_SMMU_USER2_0_REG            (PPE_TPU_BASE + 0x7014) /* AXI总线，读写DDR时AXUSER 配置项2 */
#define PPE_TPU_TPU_DDR_SMMU_USER2_1_REG            (PPE_TPU_BASE + 0x7034) /* AXI总线，读写DDR时AXUSER 配置项2 */
#define PPE_TPU_TPU_DDR_SMMU_USER2_2_REG            (PPE_TPU_BASE + 0x7054) /* AXI总线，读写DDR时AXUSER 配置项2 */
#define PPE_TPU_TPU_DDR_SMMU_USER2_3_REG            (PPE_TPU_BASE + 0x7074) /* AXI总线，读写DDR时AXUSER 配置项2 */
#define PPE_TPU_TPU_CFG_BUS_QOS_REG                 (PPE_TPU_BASE + 0x7310) /* 访问总线的QOS配置 */
#define PPE_TPU_TPU_CFG_BUS_CACHE_REG               (PPE_TPU_BASE + 0x7314) /* 访问总线的CACHE配置 */
#define PPE_TPU_TPU_RCB_ADD_PKT_CNT_0_REG           (PPE_TPU_BASE + 0x8010) /* RCB添加的分片报文数统计 */
#define PPE_TPU_TPU_RCB_ADD_PKT_CNT_1_REG           (PPE_TPU_BASE + 0x8070) /* RCB添加的分片报文数统计 */
#define PPE_TPU_TPU_RCB_ADD_PKT_CNT_2_REG           (PPE_TPU_BASE + 0x80D0) /* RCB添加的分片报文数统计 */
#define PPE_TPU_TPU_RCB_ADD_PKT_CNT_3_REG           (PPE_TPU_BASE + 0x8130) /* RCB添加的分片报文数统计 */
#define PPE_TPU_TPU_SEND_FRAG_CNT_0_REG             (PPE_TPU_BASE + 0x8020) /* TPU发送分片报文统计，以分片包为单位 */
#define PPE_TPU_TPU_SEND_FRAG_CNT_1_REG             (PPE_TPU_BASE + 0x8080) /* TPU发送分片报文统计，以分片包为单位 */
#define PPE_TPU_TPU_SEND_FRAG_CNT_2_REG             (PPE_TPU_BASE + 0x80E0) /* TPU发送分片报文统计，以分片包为单位 */
#define PPE_TPU_TPU_SEND_FRAG_CNT_3_REG             (PPE_TPU_BASE + 0x8140) /* TPU发送分片报文统计，以分片包为单位 */
#define PPE_TPU_TPU_SEND_PKT_CNT_0_REG              (PPE_TPU_BASE + 0x8028) /* TPU发送报文统计，以TSO前大包为单位 */
#define PPE_TPU_TPU_SEND_PKT_CNT_1_REG              (PPE_TPU_BASE + 0x8088) /* TPU发送报文统计，以TSO前大包为单位 */
#define PPE_TPU_TPU_SEND_PKT_CNT_2_REG              (PPE_TPU_BASE + 0x80E8) /* TPU发送报文统计，以TSO前大包为单位 */
#define PPE_TPU_TPU_SEND_PKT_CNT_3_REG              (PPE_TPU_BASE + 0x8148) /* TPU发送报文统计，以TSO前大包为单位 */
#define PPE_TPU_TPU_MEM_SBIT_ERR_CNT_REG            (PPE_TPU_BASE + 0x9000) /* 单bit ECC统计寄存器 */
#define PPE_TPU_TPU_MEM_MBIT_ERR_CNT_REG            (PPE_TPU_BASE + 0x9004) /* 2bit ECC统计寄存器 */
#define PPE_TPU_SSU_TPU_BP_STATUS_REG               (PPE_TPU_BASE + 0xA000) /* PPE SSU对TPU的整体反压状态 */
#define PPE_TPU_SSU_TPU_TNL_BP_STATUS_REG           (PPE_TPU_BASE + 0xA010) /* PPE SSU对TPU的通道级反压状态 */
#define PPE_TPU_TPU_RB_WB_BD_RDY_REG                (PPE_TPU_BASE + 0xA030) /* TPU Ring Buffer回写Ready寄存器 */
#define PPE_TPU_TPU_RDPKT_BP_STATUS_REG             (PPE_TPU_BASE + 0xA050) /* PPE TPU读报文反压状态寄存器 */
#define PPE_TPU_TPU_PREBD_BUF_BP_STATUS_REG         (PPE_TPU_BASE + 0xA070) /* PPE TPU伴随信息反压状态寄存器 */
#define PPE_TPU_TPU_TNL_BEAT_CNT_BP_REG             (PPE_TPU_BASE + 0xA080) /* PPE_TPU各通道读报文数据反压状态 */
#define PPE_TPU_TPU_TNL_PKT_CNT_BP_REG              (PPE_TPU_BASE + 0xA090) /* 报文个数引起的反压 */
#define PPE_TPU_TPU_RB_BD_WB_BP_REG                 (PPE_TPU_BASE + 0xA0B0) /* PPE_TPU RB模式回写反压 */
#define PPE_TPU_TPU_TNL_EMPTY_STATUS_REG            (PPE_TPU_BASE + 0xA0E0) /* PPE TPU通道级空状态寄存器 */
#define PPE_TPU_TPU_DATAPATH_EMPTY_STATUS_REG       (PPE_TPU_BASE + 0xA0F0) /* PPE TPU整体空状态寄存器 */
#define PPE_TPU_TPU_FIFO_DFX0_REG                   (PPE_TPU_BASE + 0xA800) /* PPE TPU内部FIFO状态寄存器0 */
#define PPE_TPU_TPU_FIFO_DFX1_REG                   (PPE_TPU_BASE + 0xA804) /* PPE TPU内部FIFO状态寄存器1 */
#define PPE_TPU_TPU_FIFO_DFX2_REG                   (PPE_TPU_BASE + 0xA808) /* PPE TPU内部FIFO状态寄存器2 */
#define PPE_TPU_TPU_FIFO_DFX3_REG                   (PPE_TPU_BASE + 0xA80C) /* PPE TPU内部FIFO状态寄存器3 */
#define PPE_TPU_TPU_FIFO_DFX4_REG                   (PPE_TPU_BASE + 0xA810) /* PPE TPU内部FIFO状态寄存器4 */
#define PPE_TPU_TPU_FIFO_DFX5_REG                   (PPE_TPU_BASE + 0xA814) /* PPE TPU内部FIFO状态寄存器5 */
#define PPE_TPU_TPU_FIFO_DFX6_REG                   (PPE_TPU_BASE + 0xA818) /* PPE TPU内部FIFO状态寄存器6 */
#define PPE_TPU_TPU_FIFO_DFX7_REG                   (PPE_TPU_BASE + 0xA81C) /* PPE TPU内部FIFO状态寄存器7 */
#define PPE_TPU_TPU_FIFO_DFX8_REG                   (PPE_TPU_BASE + 0xA820) /* PPE TPU内部FIFO状态寄存器8 */
#define PPE_TPU_TPU_FIFO_DFX9_REG                   (PPE_TPU_BASE + 0xA824) /* PPE TPU内部FIFO状态寄存器9 */
#define PPE_TPU_TPU_FIFO_DFX10_REG                  (PPE_TPU_BASE + 0xA828) /* PPE TPU内部FIFO状态寄存器10 */
#define PPE_TPU_TPU_FIFO_DFX11_REG                  (PPE_TPU_BASE + 0xA82C) /* PPE TPU内部FIFO状态寄存器11 */
#define PPE_TPU_TPU_FIFO_DFX12_REG                  (PPE_TPU_BASE + 0xA830) /* PPE TPU内部FIFO状态寄存器12 */
#define PPE_TPU_TPU_FIFO_DFX13_REG                  (PPE_TPU_BASE + 0xA834) /* PPE TPU内部FIFO状态寄存器13 */
#define PPE_TPU_TPU_FIFO_DFX14_REG                  (PPE_TPU_BASE + 0xA838) /* PPE TPU内部FIFO状态寄存器14 */
#define PPE_TPU_TPU_FIFO_DFX15_REG                  (PPE_TPU_BASE + 0xA83C) /* PPE TPU内部FIFO状态寄存器15 */
#define PPE_TPU_TPU_FIFO_DFX16_REG                  (PPE_TPU_BASE + 0xA840) /* PPE TPU内部FIFO状态寄存器16 */
#define PPE_TPU_TPU_FIFO_DFX17_REG                  (PPE_TPU_BASE + 0xA844) /* PPE TPU内部FIFO状态寄存器17 */
#define PPE_TPU_TPU_FIFO_DFX18_REG                  (PPE_TPU_BASE + 0xA848) /* PPE TPU内部FIFO状态寄存器18 */
#define PPE_TPU_TPU_FIFO_DFX19_REG                  (PPE_TPU_BASE + 0xA84C) /* PPE TPU内部FIFO状态寄存器19 */
#define PPE_TPU_TPU_FIFO_DFX20_REG                  (PPE_TPU_BASE + 0xA850) /* PPE TPU内部FIFO状态寄存器20 */
#define PPE_TPU_TPU_FIFO_DFX21_REG                  (PPE_TPU_BASE + 0xA854) /* PPE TPU内部FIFO状态寄存器21 */
#define PPE_TPU_TPU_FIFO_DFX22_REG                  (PPE_TPU_BASE + 0xA858) /* PPE TPU内部FIFO状态寄存器22 */
#define PPE_TPU_TPU_FIFO_DFX23_REG                  (PPE_TPU_BASE + 0xA85C) /* PPE TPU内部FIFO状态寄存器23 */
#define PPE_TPU_TPU_FIFO_DFX24_REG                  (PPE_TPU_BASE + 0xA860) /* PPE TPU内部FIFO状态寄存器24 */
#define PPE_TPU_TPU_FIFO_DFX25_REG                  (PPE_TPU_BASE + 0xA864) /* PPE TPU内部FIFO状态寄存器25 */
#define PPE_TPU_TPU_FIFO_DFX26_REG                  (PPE_TPU_BASE + 0xA868) /* PPE TPU内部FIFO状态寄存器26 */
#define PPE_TPU_TPU_FIFO_DFX27_REG                  (PPE_TPU_BASE + 0xA86C) /* PPE TPU内部FIFO状态寄存器27 */
#define PPE_TPU_TPU_FIFO_DFX28_REG                  (PPE_TPU_BASE + 0xA870) /* PPE TPU内部FIFO状态寄存器28 */
#define PPE_TPU_TPU_FIFO_DFX29_REG                  (PPE_TPU_BASE + 0xA874) /* PPE TPU内部FIFO状态寄存器29 */
#define PPE_TPU_TPU_FIFO_DFX30_REG                  (PPE_TPU_BASE + 0xA878) /* PPE TPU内部FIFO状态寄存器30 */
#define PPE_TPU_TPU_FIFO_DFX31_REG                  (PPE_TPU_BASE + 0xA87C) /* PPE TPU内部FIFO状态寄存器31 */
#define PPE_TPU_TPU_FIFO_DFX32_REG                  (PPE_TPU_BASE + 0xA880) /* PPE TPU内部FIFO状态寄存器32 */
#define PPE_TPU_TPU_FIFO_DFX33_REG                  (PPE_TPU_BASE + 0xA884) /* PPE TPU内部FIFO状态寄存器33 */
#define PPE_TPU_TPU_FIFO_DFX34_REG                  (PPE_TPU_BASE + 0xA888) /* PPE TPU内部FIFO状态寄存器34 */
#define PPE_TPU_TPU_FIFO_DFX35_REG                  (PPE_TPU_BASE + 0xA88C) /* PPE TPU内部FIFO状态寄存器35 */
#define PPE_TPU_TPU_FIFO_DFX36_REG                  (PPE_TPU_BASE + 0xA890) /* PPE TPU内部FIFO状态寄存器36 */
#define PPE_TPU_TPU_FIFO_DFX37_REG                  (PPE_TPU_BASE + 0xA894) /* PPE TPU内部FIFO状态寄存器37 */
#define PPE_TPU_ROCE_TX_IN_PKT_CNT_0_REG            (PPE_TPU_BASE + 0xC000) /* ROCE_TX_IN统计寄存器 */
#define PPE_TPU_ROCE_TX_IN_PKT_CNT_1_REG            (PPE_TPU_BASE + 0xC040) /* ROCE_TX_IN统计寄存器 */
#define PPE_TPU_ROCE_TX_IN_PKT_CNT_2_REG            (PPE_TPU_BASE + 0xC080) /* ROCE_TX_IN统计寄存器 */
#define PPE_TPU_ROCE_TX_IN_PKT_CNT_3_REG            (PPE_TPU_BASE + 0xC0C0) /* ROCE_TX_IN统计寄存器 */
#define PPE_TPU_ROCE_TX_OUT_PKT_CNT_0_REG           (PPE_TPU_BASE + 0xC018) /* ROCE_TX_OUT统计寄存器 */
#define PPE_TPU_ROCE_TX_OUT_PKT_CNT_1_REG           (PPE_TPU_BASE + 0xC058) /* ROCE_TX_OUT统计寄存器 */
#define PPE_TPU_ROCE_TX_OUT_PKT_CNT_2_REG           (PPE_TPU_BASE + 0xC098) /* ROCE_TX_OUT统计寄存器 */
#define PPE_TPU_ROCE_TX_OUT_PKT_CNT_3_REG           (PPE_TPU_BASE + 0xC0D8) /* ROCE_TX_OUT统计寄存器 */
#define PPE_TPU_NIC_TX_IN_AR_CNT_0_REG              (PPE_TPU_BASE + 0xC020) /* NIC_TX_IN AR统计寄存器 */
#define PPE_TPU_NIC_TX_IN_AR_CNT_1_REG              (PPE_TPU_BASE + 0xC060) /* NIC_TX_IN AR统计寄存器 */
#define PPE_TPU_NIC_TX_IN_AR_CNT_2_REG              (PPE_TPU_BASE + 0xC0A0) /* NIC_TX_IN AR统计寄存器 */
#define PPE_TPU_NIC_TX_IN_AR_CNT_3_REG              (PPE_TPU_BASE + 0xC0E0) /* NIC_TX_IN AR统计寄存器 */
#define PPE_TPU_ROCE_TX_IN_AR_CNT_0_REG             (PPE_TPU_BASE + 0xC028) /* ROCE_TX_IN AR统计寄存器 */
#define PPE_TPU_ROCE_TX_IN_AR_CNT_1_REG             (PPE_TPU_BASE + 0xC068) /* ROCE_TX_IN AR统计寄存器 */
#define PPE_TPU_ROCE_TX_IN_AR_CNT_2_REG             (PPE_TPU_BASE + 0xC0A8) /* ROCE_TX_IN AR统计寄存器 */
#define PPE_TPU_ROCE_TX_IN_AR_CNT_3_REG             (PPE_TPU_BASE + 0xC0E8) /* ROCE_TX_IN AR统计寄存器 */
#define PPE_TPU_AXI_DATA_NIC_CNT_0_REG              \
    (PPE_TPU_BASE + 0xC030) /* AXI端口读回来的数据发到TPU DATA CTRL 的统计寄存器,基于AR的burst统计 */
#define PPE_TPU_AXI_DATA_NIC_CNT_1_REG              \
    (PPE_TPU_BASE + 0xC070) /* AXI端口读回来的数据发到TPU DATA CTRL 的统计寄存器,基于AR的burst统计 */
#define PPE_TPU_AXI_DATA_NIC_CNT_2_REG              \
    (PPE_TPU_BASE + 0xC0B0) /* AXI端口读回来的数据发到TPU DATA CTRL 的统计寄存器,基于AR的burst统计 */
#define PPE_TPU_AXI_DATA_NIC_CNT_3_REG              \
    (PPE_TPU_BASE + 0xC0F0) /* AXI端口读回来的数据发到TPU DATA CTRL 的统计寄存器,基于AR的burst统计 */
#define PPE_TPU_AXI_DATA_ROCE_CNT_0_REG             \
    (PPE_TPU_BASE + 0xC038) /* AXI端口读回来的数据发到ROCE PACK 的统计寄存器,基于AR的burst统计 */
#define PPE_TPU_AXI_DATA_ROCE_CNT_1_REG             \
    (PPE_TPU_BASE + 0xC078) /* AXI端口读回来的数据发到ROCE PACK 的统计寄存器,基于AR的burst统计 */
#define PPE_TPU_AXI_DATA_ROCE_CNT_2_REG             \
    (PPE_TPU_BASE + 0xC0B8) /* AXI端口读回来的数据发到ROCE PACK 的统计寄存器,基于AR的burst统计 */
#define PPE_TPU_AXI_DATA_ROCE_CNT_3_REG             \
    (PPE_TPU_BASE + 0xC0F8) /* AXI端口读回来的数据发到ROCE PACK 的统计寄存器,基于AR的burst统计 */
#define PPE_TPU_TXDMA_AR_CNT_MSTER0_REG             (PPE_TPU_BASE + 0xD000) /* TXDMA MSTER0发起的读请求个数 */
#define PPE_TPU_TXDMA_AR_CNT_MSTER1_REG             (PPE_TPU_BASE + 0xD008) /* TXDMA MSTER1发起的读请求个数 */
#define PPE_TPU_TXDMA_R_CNT_MSTER0_REG              (PPE_TPU_BASE + 0xD010) /* TXDMA MSTER0收到读数据个数 */
#define PPE_TPU_TXDMA_R_CNT_MSTER1_REG              (PPE_TPU_BASE + 0xD018) /* TXDMA MSTER1收到读数据个数 */
#define PPE_TPU_TXDMA_AXI_TX_OUT_CNT_REG            (PPE_TPU_BASE + 0xD020) /* TXDMA AXI出口发出的数据总数 */
#define PPE_TPU_TPU_PERF_TEST_SEL_REG               (PPE_TPU_BASE + 0xDF00) /* TPU性能测试使能寄存器 */
#define PPE_TPU_TPU_PERF_TEST_PORT_REG              (PPE_TPU_BASE + 0xDF04) /* TPU性能测试端口寄存器 */
#define PPE_TPU_TPU_BUS_DELAY_GROUP_SEL_REG         (PPE_TPU_BASE + 0xDF08) /* TPU总线延时统计的Master口选择 */
#define PPE_TPU_TPU2SSU_PERF_TEST_BYTE_STATUS_REG   (PPE_TPU_BASE + 0xDF10) /* TPU到SSU口字节性能状态寄存器 */
#define PPE_TPU_TPU2SSU_PERF_TEST_CNT_STATUS_REG    (PPE_TPU_BASE + 0xDF14) /* TPU到SSU口包率性能状态寄存器 */
#define PPE_TPU_DMAPORT_PERF_TEST_CNT_STATUS_REG    (PPE_TPU_BASE + 0xDF30) /* DMA口性能统计 */
#define PPE_TPU_TPU_BUS_DELAY_CNT0_REG              (PPE_TPU_BASE + 0xDF40) /* PPE 总线时延测量统计寄存器0 */
#define PPE_TPU_TPU_BUS_DELAY_CNT1_REG              (PPE_TPU_BASE + 0xDF48) /* PPE 总线时延测量统计寄存器1 */
#define PPE_TPU_TPU_BUS_DELAY_CNT2_REG              (PPE_TPU_BASE + 0xDF50) /* PPE 总线时延测量统计寄存器2 */
#define PPE_TPU_TPU_BUS_DELAY_CNT3_REG              (PPE_TPU_BASE + 0xDF58) /* PPE 总线时延测量统计寄存器3 */
#define PPE_TPU_TPU_BUS_DELAY_CNT4_REG              (PPE_TPU_BASE + 0xDF60) /* PPE 总线时延测量统计寄存器4 */
#define PPE_TPU_TPU_BUS_DELAY_CNT5_REG              (PPE_TPU_BASE + 0xDF68) /* PPE 总线时延测量统计寄存器5 */
#define PPE_TPU_TPU_BUS_DELAY_CNT6_REG              (PPE_TPU_BASE + 0xDF70) /* PPE 总线时延测量统计寄存器6 */
#define PPE_TPU_TPU_BUS_DELAY_CNT7_REG              (PPE_TPU_BASE + 0xDF78) /* PPE 总线时延测量统计寄存器7 */

#endif // __REG_TPU_OFFSET_H__
