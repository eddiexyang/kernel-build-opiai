/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: reg_rpu_offset
 * Author: huawei
 * Create: 2021-12-28
 */

#ifndef __REG_RPU_OFFSET_H__
#define __REG_RPU_OFFSET_H__

/* PPE_RPU Base address of Module's Register */
#define PPE_RPU_BASE                       (0x0)

/******************************************************************************/
/*                      xxx PPE_RPU Registers' Definitions                    */
/******************************************************************************/

#define PPE_RPU_RPU_PF_INT_ENABLE0_REG          (PPE_RPU_BASE + 0x0)    /* 中断使能寄存器0 */
#define PPE_RPU_RPU_PF_INT_ENABLE2_0_REG        (PPE_RPU_BASE + 0x8)    /* 中断使能寄存器3 */
#define PPE_RPU_RPU_PF_INT_ENABLE2_1_REG        (PPE_RPU_BASE + 0x18)   /* 中断使能寄存器3 */
#define PPE_RPU_RPU_PF_INT_ENABLE2_2_REG        (PPE_RPU_BASE + 0x28)   /* 中断使能寄存器3 */
#define PPE_RPU_RPU_PF_INT_ENABLE2_3_REG        (PPE_RPU_BASE + 0x38)   /* 中断使能寄存器3 */
#define PPE_RPU_RPU_PF_INT_STATUS0_REG          (PPE_RPU_BASE + 0x400)  /* 中断状态寄存器0 */
#define PPE_RPU_RPU_PF_INT_STATUS2_0_REG        (PPE_RPU_BASE + 0x408)  /* 中断使能寄存器2 */
#define PPE_RPU_RPU_PF_INT_STATUS2_1_REG        (PPE_RPU_BASE + 0x418)  /* 中断使能寄存器2 */
#define PPE_RPU_RPU_PF_INT_STATUS2_2_REG        (PPE_RPU_BASE + 0x428)  /* 中断使能寄存器2 */
#define PPE_RPU_RPU_PF_INT_STATUS2_3_REG        (PPE_RPU_BASE + 0x438)  /* 中断使能寄存器2 */
#define PPE_RPU_RPU_PF_INT_SET0_REG             (PPE_RPU_BASE + 0x800)  /* 中断注入寄存器0 */
#define PPE_RPU_RPU_PF_INT_SET2_0_REG           (PPE_RPU_BASE + 0x808)  /* 中断注入寄存器2 */
#define PPE_RPU_RPU_PF_INT_SET2_1_REG           (PPE_RPU_BASE + 0x818)  /* 中断注入寄存器2 */
#define PPE_RPU_RPU_PF_INT_SET2_2_REG           (PPE_RPU_BASE + 0x828)  /* 中断注入寄存器2 */
#define PPE_RPU_RPU_PF_INT_SET2_3_REG           (PPE_RPU_BASE + 0x838)  /* 中断注入寄存器2 */
#define PPE_RPU_RPU_PF_INT_SOURCE0_REG          (PPE_RPU_BASE + 0xC00)  /* 原始中断状态寄存器0(记录第一次的中断状态，写1清中断） */
#define PPE_RPU_RPU_PF_INT_SOURCE2_0_REG        (PPE_RPU_BASE + 0xC08)  /* 原始中断状态寄存器2(记录第一次的中断状态，写1清中断） */
#define PPE_RPU_RPU_PF_INT_SOURCE2_1_REG        (PPE_RPU_BASE + 0xC18)  /* 原始中断状态寄存器2(记录第一次的中断状态，写1清中断） */
#define PPE_RPU_RPU_PF_INT_SOURCE2_2_REG        (PPE_RPU_BASE + 0xC28)  /* 原始中断状态寄存器2(记录第一次的中断状态，写1清中断） */
#define PPE_RPU_RPU_PF_INT_SOURCE2_3_REG        (PPE_RPU_BASE + 0xC38)  /* 原始中断状态寄存器2(记录第一次的中断状态，写1清中断） */
#define PPE_RPU_RPU_PREP_FIFO_WR_INT_REG        (PPE_RPU_BASE + 0x1000) /* PPE RPU模块FIFO满写中断状态寄存器 */
#define PPE_RPU_RPU_AXI_FIFO_WR_INT0_REG        (PPE_RPU_BASE + 0x1004) /* RPU中 FIFO满写中断状态寄存器1 */
#define PPE_RPU_RPU_STORE_FIFO_WR_INT_0_REG     (PPE_RPU_BASE + 0x1040) /* PPE RPU模块FIFO满写中断状态寄存器 */
#define PPE_RPU_RPU_STORE_FIFO_WR_INT_1_REG     (PPE_RPU_BASE + 0x1048) /* PPE RPU模块FIFO满写中断状态寄存器 */
#define PPE_RPU_RPU_STORE_FIFO_WR_INT_2_REG     (PPE_RPU_BASE + 0x1050) /* PPE RPU模块FIFO满写中断状态寄存器 */
#define PPE_RPU_RPU_STORE_FIFO_WR_INT_3_REG     (PPE_RPU_BASE + 0x1058) /* PPE RPU模块FIFO满写中断状态寄存器 */
#define PPE_RPU_RPU_AXI_FIFO_WR_INT1_REG        (PPE_RPU_BASE + 0x1044) /* RPU中 FIFO满写中断状态寄存器0 */
#define PPE_RPU_RPU_PREP_FIFO_RD_INT_REG        (PPE_RPU_BASE + 0x1400) /* PPE RPU模块FIFO满写中断状态寄存器 */
#define PPE_RPU_RPU_AXI_FIFO_RD_INT0_REG        (PPE_RPU_BASE + 0x1404) /* RPU中 FIFO满写中断状态寄存器1 */
#define PPE_RPU_RPU_STORE_FIFO_RD_INT_0_REG     (PPE_RPU_BASE + 0x1440) /* PPE RPU模块FIFO满写中断状态寄存器 */
#define PPE_RPU_RPU_STORE_FIFO_RD_INT_1_REG     (PPE_RPU_BASE + 0x1448) /* PPE RPU模块FIFO满写中断状态寄存器 */
#define PPE_RPU_RPU_STORE_FIFO_RD_INT_2_REG     (PPE_RPU_BASE + 0x1450) /* PPE RPU模块FIFO满写中断状态寄存器 */
#define PPE_RPU_RPU_STORE_FIFO_RD_INT_3_REG     (PPE_RPU_BASE + 0x1458) /* PPE RPU模块FIFO满写中断状态寄存器 */
#define PPE_RPU_RPU_AXI_FIFO_RD_INT1_REG        (PPE_RPU_BASE + 0x1444) /* RPU中 FIFO满写中断状态寄存器0 */
#define PPE_RPU_RPU_MEM_ERR_INFO0_REG           (PPE_RPU_BASE + 0x1600) /* PPE RPU MEMORY错误记录的伴随信息 */
#define PPE_RPU_RPU_MEM_ERR_INFO1_REG           (PPE_RPU_BASE + 0x1604) /* PPE RPU MEMORY错误记录的伴随信息 */

/* PPE RPU 模块 写DDR操作返回error中断伴随信息ID寄存器（记录第一次中断的，对应中断被清除后才会记录清除后第一次中断的）。 */
#define PPE_RPU_RPU_WR_AXI_ERR_INFO_REG         (PPE_RPU_BASE + 0x1610)
#define PPE_RPU_RPU_TNL_INT_INFO_0_REG          (PPE_RPU_BASE + 0x1644) /* 基于TNL的部分中断信息 */
#define PPE_RPU_RPU_TNL_INT_INFO_1_REG          (PPE_RPU_BASE + 0x1664) /* 基于TNL的部分中断信息 */
#define PPE_RPU_RPU_TNL_INT_INFO_2_REG          (PPE_RPU_BASE + 0x1684) /* 基于TNL的部分中断信息 */
#define PPE_RPU_RPU_TNL_INT_INFO_3_REG          (PPE_RPU_BASE + 0x16A4) /* 基于TNL的部分中断信息 */
#define PPE_RPU_RPU_TX_1588_INT_INFO_0_REG      (PPE_RPU_BASE + 0x1648) /* 基于TNL的部分中断信息 */
#define PPE_RPU_RPU_TX_1588_INT_INFO_1_REG      (PPE_RPU_BASE + 0x1668) /* 基于TNL的部分中断信息 */
#define PPE_RPU_RPU_TX_1588_INT_INFO_2_REG      (PPE_RPU_BASE + 0x1688) /* 基于TNL的部分中断信息 */
#define PPE_RPU_RPU_TX_1588_INT_INFO_3_REG      (PPE_RPU_BASE + 0x16A8) /* 基于TNL的部分中断信息 */
#define PPE_RPU_RPU_PF_INT_TYPE0_REG            (PPE_RPU_BASE + 0x1AC0) /* 中断上报类型寄存器0 */
#define PPE_RPU_RPU_PF_INT_CE0_REG              (PPE_RPU_BASE + 0x1E00) /* CE原始中断状态寄存器0(记录第一次的中断状态，写1清中断） */
#define PPE_RPU_RPU_PF_INT_CE2_0_REG            (PPE_RPU_BASE + 0x1E08) /* 原始中断状态寄存器2(记录第一次的中断状态，写1清中断） */
#define PPE_RPU_RPU_PF_INT_CE2_1_REG            (PPE_RPU_BASE + 0x1E18) /* 原始中断状态寄存器2(记录第一次的中断状态，写1清中断） */
#define PPE_RPU_RPU_PF_INT_CE2_2_REG            (PPE_RPU_BASE + 0x1E28) /* 原始中断状态寄存器2(记录第一次的中断状态，写1清中断） */
#define PPE_RPU_RPU_PF_INT_CE2_3_REG            (PPE_RPU_BASE + 0x1E38) /* 原始中断状态寄存器2(记录第一次的中断状态，写1清中断） */
#define PPE_RPU_RPU_PF_INT_NFE0_REG             (PPE_RPU_BASE + 0x2200) /* NFE原始中断状态寄存器0(记录第一次的中断状态，写1清中断） */
#define PPE_RPU_RPU_PF_INT_NFE2_0_REG           (PPE_RPU_BASE + 0x2208) /* NFE原始中断状态寄存器2(记录第一次的中断状态，写1清中断） */
#define PPE_RPU_RPU_PF_INT_NFE2_1_REG           (PPE_RPU_BASE + 0x2218) /* NFE原始中断状态寄存器2(记录第一次的中断状态，写1清中断） */
#define PPE_RPU_RPU_PF_INT_NFE2_2_REG           (PPE_RPU_BASE + 0x2228) /* NFE原始中断状态寄存器2(记录第一次的中断状态，写1清中断） */
#define PPE_RPU_RPU_PF_INT_NFE2_3_REG           (PPE_RPU_BASE + 0x2238) /* NFE原始中断状态寄存器2(记录第一次的中断状态，写1清中断） */
#define PPE_RPU_RPU_PF_INT_FE0_REG              (PPE_RPU_BASE + 0x2400) /* FE原始中断状态寄存器0(记录第一次的中断状态，写1清中断） */
#define PPE_RPU_RPU_PF_INT_FE2_0_REG            (PPE_RPU_BASE + 0x2408) /* FE原始中断状态寄存器2(记录第一次的中断状态，写1清中断） */
#define PPE_RPU_RPU_PF_INT_FE2_1_REG            (PPE_RPU_BASE + 0x2418) /* FE原始中断状态寄存器2(记录第一次的中断状态，写1清中断） */
#define PPE_RPU_RPU_PF_INT_FE2_2_REG            (PPE_RPU_BASE + 0x2428) /* FE原始中断状态寄存器2(记录第一次的中断状态，写1清中断） */
#define PPE_RPU_RPU_PF_INT_FE2_3_REG            (PPE_RPU_BASE + 0x2438) /* FE原始中断状态寄存器2(记录第一次的中断状态，写1清中断） */
#define PPE_RPU_RPU_MEM_SBIT_ECC_THD_REG        (PPE_RPU_BASE + 0x2600) /* Mem发生sbit错误时，上报ECC错误中断的阀值 */
#define PPE_RPU_RPU_MEM_SBIT_ERR_CNT_REG        (PPE_RPU_BASE + 0x2604) /* 单bit ECC统计寄存器 */
#define PPE_RPU_RPU_MEM_MBIT_ERR_CNT_REG        (PPE_RPU_BASE + 0x2608) /* 多bit ECC统计寄存器 */
#define PPE_RPU_RPU_MEM_INIT_START_REG          (PPE_RPU_BASE + 0x2800) /* 配置MEM开始自动初始化。 */
#define PPE_RPU_RPU_MEM_INIT_DONE_REG           (PPE_RPU_BASE + 0x2804) /* MEM自动初始化完成。 */
#define PPE_RPU_RPU_MEM_CTRL_SP_REG             (PPE_RPU_BASE + 0x2808) /* MEM时序配置。 */
#define PPE_RPU_RPU_MEM_CTRL_TP_REG             (PPE_RPU_BASE + 0x280C) /* MEM时序配置。 */
#define PPE_RPU_RPU_MEM_CTRL_POWER_REG          (PPE_RPU_BASE + 0x2810) /* MEM时序配置。 */
#define PPE_RPU_RPU_MEM_ECC_BYPASS_REG          (PPE_RPU_BASE + 0x2814) /* MEM ECC 控制信号 */
#define PPE_RPU_RPU_MEM_ECC_SBIT_THD_REG        (PPE_RPU_BASE + 0x2818) /* Mem发生sbit错误时，上报ECC错误中断的阀值 */
#define PPE_RPU_RPU_TCAM_MBIST_CFG_REG          (PPE_RPU_BASE + 0x281C) /* RPU TCAM mbist 控制寄存器 */
#define PPE_RPU_RPU_TCAM_MBIST_TEST_REG         (PPE_RPU_BASE + 0x2820) /* RPU TCAM mbist 测试寄存器 */
#define PPE_RPU_RPU_MEM_ERR_INSERT_0_REG        (PPE_RPU_BASE + 0x2840) /* PPE RPU MEMORY ECC注入请求寄存器0 */
#define PPE_RPU_RPU_MEM_ERR_INSERT_1_REG        (PPE_RPU_BASE + 0x2844) /* PPE RPU MEMORY ECC注入请求寄存器1 */
#define PPE_RPU_RPU_MEM_ERR_INSERT_2_REG        (PPE_RPU_BASE + 0x2848) /* PPE RPU MEMORY ECC注入请求寄存器2 */
#define PPE_RPU_RPU_MEM_ERR_INSERT_3_REG        (PPE_RPU_BASE + 0x284C) /* PPE RPU MEMORY ECC注入请求寄存器3 */
#define PPE_RPU_RPU_MEM_ERR_INSERT_4_REG        (PPE_RPU_BASE + 0x2850) /* PPE RPU MEMORY ECC注入请求寄存器4 */
#define PPE_RPU_RPU_MEM_ERR_INSERT_5_REG        (PPE_RPU_BASE + 0x2854) /* PPE RPU MEMORY ECC注入请求寄存器5 */
#define PPE_RPU_RPU_MEM_ERR_INSERT_6_REG        (PPE_RPU_BASE + 0x2858) /* PPE RPU MEMORY ECC注入请求寄存器6 */
#define PPE_RPU_RPU_MEM_ERR_INSERT_7_REG        (PPE_RPU_BASE + 0x285C) /* PPE RPU MEMORY ECC注入请求寄存器7 */
#define PPE_RPU_RPU_FSM_HEAT_TIMEOUT_REG        (PPE_RPU_BASE + 0x2A00) /* PPE RPU内部状态机超时检查时间配置寄存器 */
#define PPE_RPU_RPU_BUF_EMPTY_CFG_REG           (PPE_RPU_BASE + 0x2A04) /* PPE RPU 接收方向MAM返回empty重复申请次数配置寄存器 */
#define PPE_RPU_RPU_CNT_CYC_CLR_CTRL_REG        (PPE_RPU_BASE + 0x2A08) /* CNT_CYC类型控制寄存器 */
#define PPE_RPU_RPU_SINGLE_TNL_FASTER_EN_REG    (PPE_RPU_BASE + 0x2A0C) /* 单TNL性能提升使能寄存器 */

/* RPU模块安全锁定寄存器，内部*SEC_FIELD_LOCKED类型的寄存器仅在该寄存器处于解锁状态才可改配。 */
#define PPE_RPU_RPU_NS_CTRL_REG_REG             (PPE_RPU_BASE + 0x2A10)
#define PPE_RPU_RPU_CFG_OK_REG_REG              (PPE_RPU_BASE + 0x2A14) /* RPU配置完毕通知寄存器 */
#define PPE_RPU_RPU_TNL_CTRL_0_REG              (PPE_RPU_BASE + 0x2A40) /* PPE RPU通道控制寄存器 */
#define PPE_RPU_RPU_TNL_CTRL_1_REG              (PPE_RPU_BASE + 0x2A80) /* PPE RPU通道控制寄存器 */
#define PPE_RPU_RPU_TNL_CTRL_2_REG              (PPE_RPU_BASE + 0x2AC0) /* PPE RPU通道控制寄存器 */
#define PPE_RPU_RPU_TNL_CTRL_3_REG              (PPE_RPU_BASE + 0x2B00) /* PPE RPU通道控制寄存器 */
#define PPE_RPU_RPU_NON_JSF_TAG_CFG_0_REG       (PPE_RPU_BASE + 0x2A44) /* PPE RPU非JSF方式下TAG[23:16]配置寄存器 */
#define PPE_RPU_RPU_NON_JSF_TAG_CFG_1_REG       (PPE_RPU_BASE + 0x2A84) /* PPE RPU非JSF方式下TAG[23:16]配置寄存器 */
#define PPE_RPU_RPU_NON_JSF_TAG_CFG_2_REG       (PPE_RPU_BASE + 0x2AC4) /* PPE RPU非JSF方式下TAG[23:16]配置寄存器 */
#define PPE_RPU_RPU_NON_JSF_TAG_CFG_3_REG       (PPE_RPU_BASE + 0x2B04) /* PPE RPU非JSF方式下TAG[23:16]配置寄存器 */
#define PPE_RPU_RPU_CLK_GATING_CTRL_REG         (PPE_RPU_BASE + 0x3C00) /* 时钟自动门控使能寄存器。 */
#define PPE_RPU_RPU_GATING_WINDOW_REG           (PPE_RPU_BASE + 0x3C04) /* 统计门控打开的窗口大小，单位是64K个该模块工作时钟的。 */
#define PPE_RPU_RPU_STORE_GATING_STS_REG        (PPE_RPU_BASE + 0x3C20) /* 指示门控打开的窗口中时钟门控是否有效 */
#define PPE_RPU_RPU_PA_GATING_STS_REG           (PPE_RPU_BASE + 0x3C24) /* 指示门控打开的窗口中时钟门控是否有效 */
#define PPE_RPU_RPU_OTHER_GATING_STS_REG        (PPE_RPU_BASE + 0x3C40) /* 指示门控打开的窗口中时钟门控是否有效 */
#define PPE_RPU_RPU_RW_CFG_GATING_CNT_REG       (PPE_RPU_BASE + 0x3C44) /* 统计门控打开的窗口中打开时钟的周期，单位是64K个该模块工作时钟的。 */
#define PPE_RPU_RPU_DBW1_GATING_CNT_REG         (PPE_RPU_BASE + 0x3C48) /* 统计门控打开的窗口中打开时钟的周期，单位是64K个该模块工作时钟的。 */
#define PPE_RPU_RPU_DBW0_GATING_CNT_REG         (PPE_RPU_BASE + 0x3C4C) /* 统计门控打开的窗口中打开时钟的周期，单位是64K个该模块工作时钟的。 */
#define PPE_RPU_RPU_CTRL_GATING_CNT_REG         (PPE_RPU_BASE + 0x3C50) /* 统计门控打开的窗口中打开时钟的周期，单位是64K个该模块工作时钟的。 */
#define PPE_RPU_RPU_INF_GATING_CNT_REG          (PPE_RPU_BASE + 0x3C54) /* 统计门控打开的窗口中打开时钟的周期，单位是64K个该模块工作时钟的。 */
#define PPE_RPU_RPU_TEST_GATING_CNT_REG         (PPE_RPU_BASE + 0x3C58) /* 统计门控打开的窗口中打开时钟的周期，单位是64K个该模块工作时钟的。 */
#define PPE_RPU_RPU_GATING_CNT_REG              (PPE_RPU_BASE + 0x3C5C) /* 统计门控打开的窗口中打开时钟的周期，单位是64K个该模块工作时钟的。 */
#define PPE_RPU_RPU_PREP_FIFO_THD_REG           (PPE_RPU_BASE + 0x3D00) /* PPE RPU PREP模块FIFO水线寄存器 */
#define PPE_RPU_RPU_WR_AXI_FIFO_THD0_REG        (PPE_RPU_BASE + 0x3D04) /* PPE RPU WR_AXI模块FIFO水线寄存器 */
#define PPE_RPU_RPU_WR_AXI_FIFO_THD1_REG        (PPE_RPU_BASE + 0x3D08) /* PPE RPU WR_AXI模块FIFO水线寄存器 */
#define PPE_RPU_RPU_DDR_USER_FNA_SEL_REG        (PPE_RPU_BASE + 0x3E00) /* user域中FNA配置选择寄存器 */
#define PPE_RPU_RPU_AXCACHEC_CFG_REG            (PPE_RPU_BASE + 0x3E04) /* axcache配置寄存器 */
#define PPE_RPU_RPU_AXQOS_CFG_REG               (PPE_RPU_BASE + 0x3E08) /* axqos配置寄存器 */
#define PPE_RPU_RPU_DDR_USER_PROPERTY_REG       (PPE_RPU_BASE + 0x3E20) /* 写DDR总线操作CACHE属性配置寄器 */
#define PPE_RPU_RPU_USER_SSET_REG               (PPE_RPU_BASE + 0x3E28) /* 写总线操作的SSET配置 */
#define PPE_RPU_RPU_USER_STREAMID_REG           (PPE_RPU_BASE + 0x3E2C) /* 写总线操作的STREAMID配置 */
#define PPE_RPU_RPU_USER_CMD_TYPE_REG           (PPE_RPU_BASE + 0x3E30) /* 写DDR和POE总线操作CACHE属性配置寄器 */
#define PPE_RPU_RPU_BD_MERGE_AGE_CNT_REG        (PPE_RPU_BASE + 0x5E00) /* RPU中BD回写的老化时间 */
#define PPE_RPU_RPU_BD_NUM_WL_0_REG             (PPE_RPU_BASE + 0x5E10) /* RX BD聚合参数控制 */
#define PPE_RPU_RPU_BD_NUM_WL_1_REG             (PPE_RPU_BASE + 0x5E20) /* RX BD聚合参数控制 */
#define PPE_RPU_RPU_BD_NUM_WL_2_REG             (PPE_RPU_BASE + 0x5E30) /* RX BD聚合参数控制 */
#define PPE_RPU_RPU_BD_NUM_WL_3_REG             (PPE_RPU_BASE + 0x5E40) /* RX BD聚合参数控制 */
#define PPE_RPU_RPU_PF_ECO_CFG0_REG             (PPE_RPU_BASE + 0x6B00) /* PPE RPU模块PF ECO预留配置寄存器0 */
#define PPE_RPU_RPU_PF_ECO_CFG1_REG             (PPE_RPU_BASE + 0x6B04) /* PPE RPU模块PF ECO预留配置寄存器1 */
#define PPE_RPU_RPU_PF_ECO_CFG2_REG             (PPE_RPU_BASE + 0x6B08) /* PPE RPU模块PF ECO预留配置寄存器2 */
#define PPE_RPU_RPU_PREP_FIFO_CNT0_REG          (PPE_RPU_BASE + 0x6C00) /* RPU_PREP模块 FIFO cfg_counter值查询寄存器0 */
#define PPE_RPU_RPU_PREP_FIFO_CNT1_0_REG        (PPE_RPU_BASE + 0x6C10) /* RPU_PREP模块 FIFO cfg_counter值查询寄存器1 */
#define PPE_RPU_RPU_PREP_FIFO_CNT1_1_REG        (PPE_RPU_BASE + 0x6C18) /* RPU_PREP模块 FIFO cfg_counter值查询寄存器1 */
#define PPE_RPU_RPU_PREP_FIFO_CNT1_2_REG        (PPE_RPU_BASE + 0x6C20) /* RPU_PREP模块 FIFO cfg_counter值查询寄存器1 */
#define PPE_RPU_RPU_PREP_FIFO_CNT1_3_REG        (PPE_RPU_BASE + 0x6C28) /* RPU_PREP模块 FIFO cfg_counter值查询寄存器1 */
#define PPE_RPU_RPU_WR_AXI_FIFO_CNT0_REG        (PPE_RPU_BASE + 0x6C14) /* RPU_WR_AXI模块 FIFO cfg_counter值查询寄存器0 */
#define PPE_RPU_RPU_WR_AXI_FIFO_CNT1_REG        (PPE_RPU_BASE + 0x6E80) /* RPU_WR_AXI模块 FIFO cfg_counter值查询寄存器1 */
#define PPE_RPU_RPU_WR_AXI_FIFO_CNT2_REG        (PPE_RPU_BASE + 0x6EC0) /* RPU_WR_AXI模块 FIFO cfg_counter值查询寄存器2 */
#define PPE_RPU_RPU_MF_FIFO_CNT0_REG            (PPE_RPU_BASE + 0x6ED0) /* RPU_MF模块 FIFO cfg_counter值查询寄存器0 */
#define PPE_RPU_RPU_MF_FIFO_CNT1_REG            (PPE_RPU_BASE + 0x6ED4) /* RPU_MF模块 FIFO cfg_counter值查询寄存器1 */
#define PPE_RPU_RPU_PREP_FIFO_STATUS0_0_REG     (PPE_RPU_BASE + 0x6F80) /* RPU_PREP模块 FIFO cfg_counter值查询寄存器0 */
#define PPE_RPU_RPU_PREP_FIFO_STATUS0_1_REG     (PPE_RPU_BASE + 0x6F88) /* RPU_PREP模块 FIFO cfg_counter值查询寄存器0 */
#define PPE_RPU_RPU_PREP_FIFO_STATUS0_2_REG     (PPE_RPU_BASE + 0x6F90) /* RPU_PREP模块 FIFO cfg_counter值查询寄存器0 */
#define PPE_RPU_RPU_PREP_FIFO_STATUS0_3_REG     (PPE_RPU_BASE + 0x6F98) /* RPU_PREP模块 FIFO cfg_counter值查询寄存器0 */
#define PPE_RPU_RPU_PREP_FIFO_STATUS1_REG       (PPE_RPU_BASE + 0x71D0) /* RPU中 FIFO空满状态查询寄存器1 */
#define PPE_RPU_RPU_WR_AXI_FIFO_STATUS0_REG     (PPE_RPU_BASE + 0x71E0) /* RPU中 FIFO空满状态查询寄存器0 */
#define PPE_RPU_RPU_WR_AXI_FIFO_STATUS1_REG     (PPE_RPU_BASE + 0x7200) /* RPU中 FIFO空满状态查询寄存器1 */
#define PPE_RPU_RPU_MF_FIFO_STATUS_REG          (PPE_RPU_BASE + 0x7210) /* RPU_MF模块 FIFO 空满状态查询寄存器 */
#define PPE_RPU_RPU_TX_1588_CNT_REG             (PPE_RPU_BASE + 0x7380) /* RPU收到的MAG发送的1588时间戳统计值 */
#define PPE_RPU_RPU_WR_AXI_DBW0_BP_CNT_REG      (PPE_RPU_BASE + 0x7384) /* 写总线被反压次数统计 */
#define PPE_RPU_RPU_WR_AXI_DBW1_BP_CNT_REG      (PPE_RPU_BASE + 0x7388) /* 写总线被反压次数统计 */
#define PPE_RPU_RPU_MF_INPUT_CNT_REG            (PPE_RPU_BASE + 0x738C) /* RPU收到的从SSU发送过来的MF报文个数统计 */
#define PPE_RPU_RPU_MF_PAYLOAD_ERR_CNT_REG      (PPE_RPU_BASE + 0x7390) /* MF报文将报文头剥掉之后，payload为0的报文个数统计寄存器 */
#define PPE_RPU_RPU_MF_ERR_DROP_CNT_REG         (PPE_RPU_BASE + 0x7394) /* RPU在入口处（INF模块）丢弃的错误的MF报文个数统计寄存器 */
#define PPE_RPU_RPU_Q_MISS_DROP_CNT_REG         (PPE_RPU_BASE + 0x7398) /* RING模式下RPU在入口处（INF模块）查询物理qid失败次数统计寄存器 */
#define PPE_RPU_RPU_ALL_INPUT_CNT_REG           (PPE_RPU_BASE + 0x739C) /* 进入RPU的所有报文个数统计 */
#define PPE_RPU_RPU_ALL_DBW0_POE_PD_CNT_REG     (PPE_RPU_BASE + 0x73A0) /* RPU推给POE的POE描述符个数统计 */
#define PPE_RPU_RPU_ALL_DBW0_POE_PKT_CNT_REG    (PPE_RPU_BASE + 0x73A4) /* RPU推给POE的所有报文个数统计 */
#define PPE_RPU_RPU_ALL_DBW1_POE_PD_CNT_REG     (PPE_RPU_BASE + 0x73A8) /* RPU推给POE的POE描述符个数统计 */
#define PPE_RPU_RPU_ALL_DBW1_POE_PKT_CNT_REG    (PPE_RPU_BASE + 0x73AC) /* RPU推给POE的所有报文个数统计 */
#define PPE_RPU_RPU_SEND_RCB_FBD_CNT_REG        (PPE_RPU_BASE + 0x73B0) /* RPU发送给RCB的FBD个数统计 */
#define PPE_RPU_RPU_SEND_RCB_FE_CNT_REG         (PPE_RPU_BASE + 0x73B4) /* RPU发送给RCB的FE FBD个数统计 */
#define PPE_RPU_RPU_SEND_RCB_PKT_CNT_REG        (PPE_RPU_BASE + 0x73B8) /* RPU发送给RCB的报文个数统计 */
#define PPE_RPU_RPU_MF_OUT_PKT_CNT_REG          (PPE_RPU_BASE + 0x73C0) /* RPU从MF的保序通道写入DDR的MF报文个数统计 */
#define PPE_RPU_RPU_TEST_TIMEOUT_INT_CNT_0_REG  (PPE_RPU_BASE + 0x7400) /* 调试网口超时中断次数统计寄存器 */
#define PPE_RPU_RPU_TEST_TIMEOUT_INT_CNT_1_REG  (PPE_RPU_BASE + 0x7420) /* 调试网口超时中断次数统计寄存器 */
#define PPE_RPU_RPU_TEST_TIMEOUT_INT_CNT_2_REG  (PPE_RPU_BASE + 0x7440) /* 调试网口超时中断次数统计寄存器 */
#define PPE_RPU_RPU_TEST_TIMEOUT_INT_CNT_3_REG  (PPE_RPU_BASE + 0x7460) /* 调试网口超时中断次数统计寄存器 */
#define PPE_RPU_RPU_TEST_PORT_PKT_CNT_0_REG     (PPE_RPU_BASE + 0x7404) /* RPU向主存中写入的调试网口报文数据个数统计寄存器 */
#define PPE_RPU_RPU_TEST_PORT_PKT_CNT_1_REG     (PPE_RPU_BASE + 0x7424) /* RPU向主存中写入的调试网口报文数据个数统计寄存器 */
#define PPE_RPU_RPU_TEST_PORT_PKT_CNT_2_REG     (PPE_RPU_BASE + 0x7444) /* RPU向主存中写入的调试网口报文数据个数统计寄存器 */
#define PPE_RPU_RPU_TEST_PORT_PKT_CNT_3_REG     (PPE_RPU_BASE + 0x7464) /* RPU向主存中写入的调试网口报文数据个数统计寄存器 */
#define PPE_RPU_RPU_TEST_VLD_INT_CNT_0_REG      (PPE_RPU_BASE + 0x7408) /* 调试网口正常中断次数统计寄存器 */
#define PPE_RPU_RPU_TEST_VLD_INT_CNT_1_REG      (PPE_RPU_BASE + 0x7428) /* 调试网口正常中断次数统计寄存器 */
#define PPE_RPU_RPU_TEST_VLD_INT_CNT_2_REG      (PPE_RPU_BASE + 0x7448) /* 调试网口正常中断次数统计寄存器 */
#define PPE_RPU_RPU_TEST_VLD_INT_CNT_3_REG      (PPE_RPU_BASE + 0x7468) /* 调试网口正常中断次数统计寄存器 */
#define PPE_RPU_RPU_INTERFACE_STATUS0_REG       (PPE_RPU_BASE + 0x7C00) /* RPU接口工作状态寄存器0 */
#define PPE_RPU_RPU_INTERFACE_STATUS1_REG       (PPE_RPU_BASE + 0x7C04) /* RPU接口工作状态寄存器1 */
#define PPE_RPU_RPU_INNER_STATUS0_REG           (PPE_RPU_BASE + 0x7C08) /* RPU内部工作状态寄存器 */
#define PPE_RPU_RPU_RECB_FLOW_STS0_REG          (PPE_RPU_BASE + 0x7C0C) /* 内部重组流状态寄存器0 */
#define PPE_RPU_RPU_RECB_FLOW_STS1_REG          (PPE_RPU_BASE + 0x7C10) /* 内部重组流状态寄存器1 */
#define PPE_RPU_RPU_RECB_FLOW_STS2_REG          (PPE_RPU_BASE + 0x7C14) /* 内部重组流状态寄存器2 */
#define PPE_RPU_RPU_RECB_FLOW_STS3_REG          (PPE_RPU_BASE + 0x7C18) /* 内部重组流状态寄存器3 */
#define PPE_RPU_RPU_PREP_LIST_CPT_STS_0_REG     (PPE_RPU_BASE + 0x7C40) /* PREP_LIST模块每个ppe_tnl的free_pid任务状态指示 */
#define PPE_RPU_RPU_PREP_LIST_CPT_STS_1_REG     (PPE_RPU_BASE + 0x7C80) /* PREP_LIST模块每个ppe_tnl的free_pid任务状态指示 */
#define PPE_RPU_RPU_PREP_LIST_CPT_STS_2_REG     (PPE_RPU_BASE + 0x7CC0) /* PREP_LIST模块每个ppe_tnl的free_pid任务状态指示 */
#define PPE_RPU_RPU_PREP_LIST_CPT_STS_3_REG     (PPE_RPU_BASE + 0x7D00) /* PREP_LIST模块每个ppe_tnl的free_pid任务状态指示 */
#define PPE_RPU_RPU_PREP_LIST_USE_STS_0_REG     (PPE_RPU_BASE + 0x7C44) /* PREP_LIST模块每个ppe_tnl的free_pid使用状态指示 */
#define PPE_RPU_RPU_PREP_LIST_USE_STS_1_REG     (PPE_RPU_BASE + 0x7C84) /* PREP_LIST模块每个ppe_tnl的free_pid使用状态指示 */
#define PPE_RPU_RPU_PREP_LIST_USE_STS_2_REG     (PPE_RPU_BASE + 0x7CC4) /* PREP_LIST模块每个ppe_tnl的free_pid使用状态指示 */
#define PPE_RPU_RPU_PREP_LIST_USE_STS_3_REG     (PPE_RPU_BASE + 0x7D04) /* PREP_LIST模块每个ppe_tnl的free_pid使用状态指示 */
#define PPE_RPU_RPU_FSM_STATUS_0_REG            (PPE_RPU_BASE + 0x7C48) /* PPE RPU 状态机信息寄存器 */
#define PPE_RPU_RPU_FSM_STATUS_1_REG            (PPE_RPU_BASE + 0x7C88) /* PPE RPU 状态机信息寄存器 */
#define PPE_RPU_RPU_FSM_STATUS_2_REG            (PPE_RPU_BASE + 0x7CC8) /* PPE RPU 状态机信息寄存器 */
#define PPE_RPU_RPU_FSM_STATUS_3_REG            (PPE_RPU_BASE + 0x7D08) /* PPE RPU 状态机信息寄存器 */
#define PPE_RPU_RPU_PERF_DBW0_POE_PKT_CNT_REG   (PPE_RPU_BASE + 0x9000) /* 最近100ms时间内RPU推给POE的报文个数统计 */
#define PPE_RPU_RPU_PERF_DBW1_POE_PKT_CNT_REG   (PPE_RPU_BASE + 0x9004) /* 最近100ms时间内RPU推给POE的报文个数统计 */
#define PPE_RPU_RPU_PERF_BUS_DELAY_CFG_REG      (PPE_RPU_BASE + 0x9008) /* PPE RPU 时延配置寄存器 */
#define PPE_RPU_RPU_PERF_BUS_DELAY_CNT0_REG     (PPE_RPU_BASE + 0x9010) /* PPE RPU 时延统计寄存器 */
#define PPE_RPU_RPU_PERF_BUS_DELAY_CNT1_REG     (PPE_RPU_BASE + 0x9018) /* PPE RPU 时延统计寄存器 */
#define PPE_RPU_RPU_PERF_BUS_DELAY_CNT2_REG     (PPE_RPU_BASE + 0x9020) /* PPE RPU 时延统计寄存器 */
#define PPE_RPU_RPU_PERF_BUS_DELAY_CNT3_REG     (PPE_RPU_BASE + 0x9028) /* PPE RPU 时延统计寄存器 */
#define PPE_RPU_RPU_PERF_BUS_DELAY_CNT4_REG     (PPE_RPU_BASE + 0x9030) /* PPE RPU 时延统计寄存器 */
#define PPE_RPU_RPU_PERF_BUS_DELAY_CNT5_REG     (PPE_RPU_BASE + 0x9038) /* PPE RPU 时延统计寄存器 */
#define PPE_RPU_RPU_PERF_BUS_DELAY_CNT6_REG     (PPE_RPU_BASE + 0x9040) /* PPE RPU 时延统计寄存器 */
#define PPE_RPU_RPU_PERF_BUS_DELAY_CNT7_REG     (PPE_RPU_BASE + 0x9048) /* PPE RPU 时延统计寄存器 */
#define PPE_RPU_RPU_RING_FLUSH_STS_REG          (PPE_RPU_BASE + 0x9400) /* 队列复位时RPU内部状态指示 */
#define PPE_RPU_RPU_RING_FLUSH_SNAP_CNT_0_REG   (PPE_RPU_BASE + 0x9500) /* 队列复位时RPU内部状态指示 */
#define PPE_RPU_RPU_RING_FLUSH_SNAP_CNT_1_REG   (PPE_RPU_BASE + 0x9520) /* 队列复位时RPU内部状态指示 */
#define PPE_RPU_RPU_RING_FLUSH_SNAP_CNT_2_REG   (PPE_RPU_BASE + 0x9540) /* 队列复位时RPU内部状态指示 */
#define PPE_RPU_RPU_RING_FLUSH_SNAP_CNT_3_REG   (PPE_RPU_BASE + 0x9560) /* 队列复位时RPU内部状态指示 */
#define PPE_RPU_RPU_MODE_INT_ENABLE_REG         (PPE_RPU_BASE + 0x0)    /* 中断使能寄存器 */
#define PPE_RPU_RPU_MODE_INT_STATUS_REG         (PPE_RPU_BASE + 0x20)   /* 中断状态寄存器 */
#define PPE_RPU_RPU_MODE_INT_SET_REG            (PPE_RPU_BASE + 0x40)   /* 中断注入寄存器 */
#define PPE_RPU_RPU_MODE_INT_SOURCE_REG         (PPE_RPU_BASE + 0x60)   /* 原始中断状态寄存器(记录第一次的中断状态，写1清中断） */

/* RPU向MAM申请buf返回的drop/empty/disable/buf_len时的异常中断信息寄存器 */
#define PPE_RPU_RPU_BUF_REQ_INT_INFO_REG        (PPE_RPU_BASE + 0x80)
#define PPE_RPU_RPU_MODE_INT_TYPE_REG           (PPE_RPU_BASE + 0xC0)   /* 中断上报类型寄存器 */
#define PPE_RPU_RPU_MODE_INT_CE_REG             (PPE_RPU_BASE + 0x100)  /* CE中断状态寄存器 */
#define PPE_RPU_RPU_MODE_INT_NFE_REG            (PPE_RPU_BASE + 0x120)  /* NFE中断状态寄存器 */
#define PPE_RPU_RPU_MODE_INT_FE_REG             (PPE_RPU_BASE + 0x140)  /* FE中断状态寄存器 */
#define PPE_RPU_RPU_VF_ECO_CFG0_REG             (PPE_RPU_BASE + 0xF00)  /* PPE RPU模块VF ECO预留配置寄存器0 */
#define PPE_RPU_RPU_VF_ECO_CFG1_REG             (PPE_RPU_BASE + 0xF04)  /* PPE RPU模块VF ECO预留配置寄存器1 */
#define PPE_RPU_RPU_VF_ECO_CFG2_REG             (PPE_RPU_BASE + 0xF08)  /* PPE RPU模块VF ECO预留配置寄存器2 */
#define PPE_RPU_RPU_QOS_PKT_CNT_0_REG           (PPE_RPU_BASE + 0x1000) /* RPU接收到的不同qos优先级的报文个数统计 */
#define PPE_RPU_RPU_QOS_PKT_CNT_1_REG           (PPE_RPU_BASE + 0x1004) /* RPU接收到的不同qos优先级的报文个数统计 */
#define PPE_RPU_RPU_QOS_PKT_CNT_2_REG           (PPE_RPU_BASE + 0x1008) /* RPU接收到的不同qos优先级的报文个数统计 */
#define PPE_RPU_RPU_QOS_PKT_CNT_3_REG           (PPE_RPU_BASE + 0x100C) /* RPU接收到的不同qos优先级的报文个数统计 */
#define PPE_RPU_RPU_QOS_PKT_CNT_4_REG           (PPE_RPU_BASE + 0x1010) /* RPU接收到的不同qos优先级的报文个数统计 */
#define PPE_RPU_RPU_QOS_PKT_CNT_5_REG           (PPE_RPU_BASE + 0x1014) /* RPU接收到的不同qos优先级的报文个数统计 */
#define PPE_RPU_RPU_QOS_PKT_CNT_6_REG           (PPE_RPU_BASE + 0x1018) /* RPU接收到的不同qos优先级的报文个数统计 */
#define PPE_RPU_RPU_QOS_PKT_CNT_7_REG           (PPE_RPU_BASE + 0x101C) /* RPU接收到的不同qos优先级的报文个数统计 */
#define PPE_RPU_RPU_ALL_PKT_CNT_0_REG           (PPE_RPU_BASE + 0x1080) /* RPU从SSU接收到的报文个数统计 */
#define PPE_RPU_RPU_ALL_PKT_CNT_1_REG           (PPE_RPU_BASE + 0x10A0) /* RPU从SSU接收到的报文个数统计 */
#define PPE_RPU_RPU_ALL_PKT_CNT_2_REG           (PPE_RPU_BASE + 0x10C0) /* RPU从SSU接收到的报文个数统计 */
#define PPE_RPU_RPU_ALL_PKT_CNT_3_REG           (PPE_RPU_BASE + 0x10E0) /* RPU从SSU接收到的报文个数统计 */
#define PPE_RPU_RPU_OK_PKT_CNT_0_REG            (PPE_RPU_BASE + 0x1084) /* 进入RPU内部模块处理的报文个数统计 */
#define PPE_RPU_RPU_OK_PKT_CNT_1_REG            (PPE_RPU_BASE + 0x10A4) /* 进入RPU内部模块处理的报文个数统计 */
#define PPE_RPU_RPU_OK_PKT_CNT_2_REG            (PPE_RPU_BASE + 0x10C4) /* 进入RPU内部模块处理的报文个数统计 */
#define PPE_RPU_RPU_OK_PKT_CNT_3_REG            (PPE_RPU_BASE + 0x10E4) /* 进入RPU内部模块处理的报文个数统计 */
#define PPE_RPU_RPU_MST0_SEND_POE_PD_CNT_0_REG  (PPE_RPU_BASE + 0x1900) /* RPU接收方向通过MST0发送给POE的POE消息个数统计 */
#define PPE_RPU_RPU_MST0_SEND_POE_PD_CNT_1_REG  (PPE_RPU_BASE + 0x1940) /* RPU接收方向通过MST0发送给POE的POE消息个数统计 */
#define PPE_RPU_RPU_MST0_SEND_POE_PD_CNT_2_REG  (PPE_RPU_BASE + 0x1980) /* RPU接收方向通过MST0发送给POE的POE消息个数统计 */
#define PPE_RPU_RPU_MST0_SEND_POE_PD_CNT_3_REG  (PPE_RPU_BASE + 0x19C0) /* RPU接收方向通过MST0发送给POE的POE消息个数统计 */
#define PPE_RPU_RPU_MST0_SEND_POE_PKT_CNT_0_REG (PPE_RPU_BASE + 0x1904) /* RPU接收方向通过MST0发送给POE的报文个数统计 */
#define PPE_RPU_RPU_MST0_SEND_POE_PKT_CNT_1_REG (PPE_RPU_BASE + 0x1944) /* RPU接收方向通过MST0发送给POE的报文个数统计 */
#define PPE_RPU_RPU_MST0_SEND_POE_PKT_CNT_2_REG (PPE_RPU_BASE + 0x1984) /* RPU接收方向通过MST0发送给POE的报文个数统计 */
#define PPE_RPU_RPU_MST0_SEND_POE_PKT_CNT_3_REG (PPE_RPU_BASE + 0x19C4) /* RPU接收方向通过MST0发送给POE的报文个数统计 */
#define PPE_RPU_RPU_MST1_SEND_POE_PD_CNT_0_REG  (PPE_RPU_BASE + 0x1908) /* RPU接收方向通过MST1发送给POE的POE消息个数统计 */
#define PPE_RPU_RPU_MST1_SEND_POE_PD_CNT_1_REG  (PPE_RPU_BASE + 0x1948) /* RPU接收方向通过MST1发送给POE的POE消息个数统计 */
#define PPE_RPU_RPU_MST1_SEND_POE_PD_CNT_2_REG  (PPE_RPU_BASE + 0x1988) /* RPU接收方向通过MST1发送给POE的POE消息个数统计 */
#define PPE_RPU_RPU_MST1_SEND_POE_PD_CNT_3_REG  (PPE_RPU_BASE + 0x19C8) /* RPU接收方向通过MST1发送给POE的POE消息个数统计 */
#define PPE_RPU_RPU_MST1_SEND_POE_PKT_CNT_0_REG (PPE_RPU_BASE + 0x190C) /* RPU接收方向通过MST1发送给POE的报文个数统计 */
#define PPE_RPU_RPU_MST1_SEND_POE_PKT_CNT_1_REG (PPE_RPU_BASE + 0x194C) /* RPU接收方向通过MST1发送给POE的报文个数统计 */
#define PPE_RPU_RPU_MST1_SEND_POE_PKT_CNT_2_REG (PPE_RPU_BASE + 0x198C) /* RPU接收方向通过MST1发送给POE的报文个数统计 */
#define PPE_RPU_RPU_MST1_SEND_POE_PKT_CNT_3_REG (PPE_RPU_BASE + 0x19CC) /* RPU接收方向通过MST1发送给POE的报文个数统计 */
#define PPE_RPU_RPU_EMPTY_INT_CNT_REG           (PPE_RPU_BASE + 0x2C00) /* PPE RPU接收方向上报empty中断次数统计寄存器 */
#define PPE_RPU_RPU_BUF_DROP_INT_CNT_REG        (PPE_RPU_BASE + 0x2C04) /* PPE RPU接收方向上报drop中断次数统计寄存器 */
#define PPE_RPU_RPU_BUF_DISABLE_INT_CNT_REG     (PPE_RPU_BASE + 0x2C08) /* PPE RPU接收方向上报disable中断次数统计寄存器 */
#define PPE_RPU_RPU_BUF_LEN_ZERO_INT_CNT_REG    (PPE_RPU_BASE + 0x2C0C) /* PPE RPU接收方向上报buf_len为0中断次数统计寄存器 */
#define PPE_RPU_RPU_EMPTY_DROP_PKT_CNT_REG      (PPE_RPU_BASE + 0x2C10) /* PPE RPU接收方向上报empty_drop中断次数统计寄存器 */
#define PPE_RPU_RPU_RECV_ERR_PKT_CNT_REG        (PPE_RPU_BASE + 0x2C14) /* RPU接收到SSU的err报文个数统计寄存器 */
#define PPE_RPU_RPU_BUF_LEN_ERR_INT_CNT_REG     (PPE_RPU_BASE + 0x2C18) /* RPU申请buf时MAM返回的buf_len中断次数统计寄存器 */

#endif // __REG_RPU_OFFSET_H__
