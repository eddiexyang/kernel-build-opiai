/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: reg_ssu_offset
 * Author: huawei
 * Create: 2021-12-28
 */

#ifndef __REG_SSU_OFFSET_H__
#define __REG_SSU_OFFSET_H__

/* PPE_SSU Base address of Module's Register */
#define PPE_SSU_BASE                       (0x0)

/******************************************************************************/
/*                      xxx PPE_SSU Registers' Definitions                    */
/******************************************************************************/

#define PPE_SSU_SSU_INT_ENABLE_REG                         (PPE_SSU_BASE + 0x0)    /* 中断使能寄存器 */
#define PPE_SSU_SSU_INT_STATUS_REG                         (PPE_SSU_BASE + 0x4)    /* 中断状态寄存器 */
#define PPE_SSU_SSU_INT_SET_REG                            (PPE_SSU_BASE + 0x8)    /* 中断注入寄存器 */
/* 原始中断状态寄存器(记录第一次的中断状态，写1清中断） */
#define PPE_SSU_SSU_INT_SOURCE_REG                         (PPE_SSU_BASE + 0xC)
#define PPE_SSU_SSU_MEM_ERR_INSERT0_REG                    (PPE_SSU_BASE + 0x40)   /* MEMORY错误植入寄存器0 */
#define PPE_SSU_SSU_MEM_ERR_INSERT1_REG                    (PPE_SSU_BASE + 0x44)   /* MEMORY错误植入寄存器1 */
#define PPE_SSU_SSU_MEM_ERR_INSERT2_REG                    (PPE_SSU_BASE + 0x48)   /* MEMORY错误植入寄存器2 */
#define PPE_SSU_SSU_MEM_SBIT_ERR_INFO_REG                  (PPE_SSU_BASE + 0x50)   /* MEMORY错误记录的伴随信息 */
#define PPE_SSU_SSU_MEM_MBIT_ERR_INFO_REG                  (PPE_SSU_BASE + 0x54)   /* MEMORY错误记录的伴随信息 */
#define PPE_SSU_SSU_FIFO_WR_INT0_REG                       (PPE_SSU_BASE + 0x60)   /* FIFO满写中断寄存器 */
#define PPE_SSU_SSU_FIFO_WR_INT1_REG                       (PPE_SSU_BASE + 0x64)   /* FIFO满写中断寄存器 */
#define PPE_SSU_SSU_FIFO_WR_INT2_REG                       (PPE_SSU_BASE + 0x68)   /* FIFO满写中断寄存器 */
#define PPE_SSU_SSU_FIFO_WR_INT3_REG                       (PPE_SSU_BASE + 0x6C)   /* FIFO满写中断寄存器 */
#define PPE_SSU_SSU_FIFO_WR_INT4_REG                       (PPE_SSU_BASE + 0x70)   /* FIFO满写中断寄存器 */
#define PPE_SSU_SSU_FIFO_WR_INT5_REG                       (PPE_SSU_BASE + 0x74)   /* FIFO满写中断寄存器 */
#define PPE_SSU_SSU_FIFO_WR_INT6_REG                       (PPE_SSU_BASE + 0x78)   /* FIFO满写中断寄存器 */
#define PPE_SSU_SSU_FIFO_WR_INT7_REG                       (PPE_SSU_BASE + 0x7C)   /* FIFO满写中断寄存器 */
#define PPE_SSU_SSU_FIFO_RD_INT0_REG                       (PPE_SSU_BASE + 0xA0)   /* FIFO空读中断寄存器 */
#define PPE_SSU_SSU_FIFO_RD_INT1_REG                       (PPE_SSU_BASE + 0xA4)   /* FIFO空读中断寄存器 */
#define PPE_SSU_SSU_FIFO_RD_INT2_REG                       (PPE_SSU_BASE + 0xA8)   /* FIFO空读中断寄存器 */
#define PPE_SSU_SSU_FIFO_RD_INT3_REG                       (PPE_SSU_BASE + 0xAC)   /* FIFO空读中断寄存器 */
#define PPE_SSU_SSU_FIFO_RD_INT4_REG                       (PPE_SSU_BASE + 0xB0)   /* FIFO空读中断寄存器 */
#define PPE_SSU_SSU_FIFO_RD_INT5_REG                       (PPE_SSU_BASE + 0xB4)   /* FIFO空读中断寄存器 */
#define PPE_SSU_SSU_FIFO_RD_INT6_REG                       (PPE_SSU_BASE + 0xB8)   /* FIFO空读中断寄存器 */
#define PPE_SSU_SSU_FIFO_RD_INT7_REG                       (PPE_SSU_BASE + 0xBC)   /* FIFO空读中断寄存器 */
#define PPE_SSU_SSU_BMP_RSC_ERR_INFO_REG                   (PPE_SSU_BASE + 0xE0)   /* 缓存资源bitmap错误信息 */
#define PPE_SSU_SSU_BMP_AGE_ERR_INFO_REG                   (PPE_SSU_BASE + 0xE4)   /* 缓存资源bitmap老化信息 */
#define PPE_SSU_SSU_CREDIT_ERR_INFO_REG                    (PPE_SSU_BASE + 0xF0)   /* RPU接口CREDIT异常中断 */
#define PPE_SSU_SSU_ROCEE_CREDIT_ERR_INFO_REG              (PPE_SSU_BASE + 0xF8)   /* ROCEE接口CREDIT异常中断 */
#define PPE_SSU_SSU_INT_TYPE_REG                           (PPE_SSU_BASE + 0x180)  /* 中断上报类型寄存器 */

/* 原始中断状态寄存器(记录第一次的中断状态，写1清中断） */
#define PPE_SSU_SSU_INT_MSIX_REG                           (PPE_SSU_BASE + 0x190)
#define PPE_SSU_SSU_INT_CE_REG                             (PPE_SSU_BASE + 0x1A0)
#define PPE_SSU_SSU_INT_NFE_REG                            (PPE_SSU_BASE + 0x1B0)
#define PPE_SSU_SSU_INT_FE_REG                             (PPE_SSU_BASE + 0x1C0)

#define PPE_SSU_SSU_MEM_INIT_START_REG                     (PPE_SSU_BASE + 0x400)  /* SSU内部MEMORY初始化启动寄存器 */
#define PPE_SSU_SSU_MEM_INIT_DONE_REG                      (PPE_SSU_BASE + 0x404)  /* SSU内部MEMORY初始化完成寄存器 */
#define PPE_SSU_SSU_SPMEM_TMOD_CTRL_REG                    (PPE_SSU_BASE + 0x410)  /* SP MEMORY 时序控制寄存器 */
#define PPE_SSU_SSU_TPMEM_TMOD_CTRL_REG                    (PPE_SSU_BASE + 0x414)  /* TP MEMORY 时序控制寄存器 */
#define PPE_SSU_SSU_MEM_POWER_CTRL_REG                     (PPE_SSU_BASE + 0x418)  /* MEMORY功耗控制寄存器 */
#define PPE_SSU_SSU_ECC_BYPASS_REG                         (PPE_SSU_BASE + 0x420)  /* memory ecc bypass使能 */
#define PPE_SSU_SSU_BMU_POOL_PAUSE_REQ_REG                 (PPE_SSU_BASE + 0x440)  /* BMU的流控请求 */
#define PPE_SSU_SSU_CNT_CLR_CE_REG                         (PPE_SSU_BASE + 0x480)  /* CNT_CYC类型寄存器读清使能 */
#define PPE_SSU_SSU_ECC_SBIT_THD_REG                       (PPE_SSU_BASE + 0x484)  /* MEMORY ECC 1BIT错误上报门限 */
#define PPE_SSU_SSU_LO_FORCE_HI_PRI_REG                    (PPE_SSU_BASE + 0x488)  /* SSU 入队报文优先强制成高优先 */
#define PPE_SSU_SSU_ROCEE_TYPE_CFG_REG                     (PPE_SSU_BASE + 0x48C)  /* ROCEE报文类型配置寄存器 */
#define PPE_SSU_SSU_RPU_PORT_INIT_ENABLE_REG               (PPE_SSU_BASE + 0x490)  /* RPU接口CREDIT初始化使能 */
#define PPE_SSU_SSU_ROCEE_PORT_INIT_ENABLE_REG             (PPE_SSU_BASE + 0x494)  /* ROCEE接口CREDIT初始化使能 */
#define PPE_SSU_SSU_RPU_INIT_CREDIT_REG                    (PPE_SSU_BASE + 0x498)  /* RPU接口CREDIT初始值 */
#define PPE_SSU_SSU_RX_OQ_SCHEDULE_EN_REG                  (PPE_SSU_BASE + 0x4A0)  /* 接收方向出口队列调度使能 */
#define PPE_SSU_SSU_TX_OQ_SCHEDULE_EN_REG                  (PPE_SSU_BASE + 0x4A4)  /* 发送方向出口队列调度使能 */
#define PPE_SSU_SSU_DROP_OQ_SCHEDULE_EN_REG                (PPE_SSU_BASE + 0x4A8)  /* 丢弃队列调度使能 */
#define PPE_SSU_SSU_L2ERR_DROP_EN_REG                      (PPE_SSU_BASE + 0x4AC)  /* L2 ERROR的报文丢弃使能 */
#define PPE_SSU_SSU_IN_PORT_SEL_REG                        (PPE_SSU_BASE + 0x4B0)  /* 入口队列状态维测选择寄存器 */
#define PPE_SSU_SSU_OT_PORT_STATUS_SEL_REG                 (PPE_SSU_BASE + 0x4B4)  /* 出口端口统计维测选择寄存器 */
#define PPE_SSU_SSU_OQ_STATUS_SEL_REG                      (PPE_SSU_BASE + 0x4B8)  /* 出口队列统计维测选择寄存器 */

/* 接收方向出口TC的映射关系（基于PPP查表结果） */
#define PPE_SSU_SSU_RX_OQ_TC_MAP_REG                       (PPE_SSU_BASE + 0x4C0)
/* 发送方向出口TC的映射关系（基于PPP查表结果） */
#define PPE_SSU_SSU_TX_OQ_TC_MAP_REG                       (PPE_SSU_BASE + 0x4C4)

#define PPE_SSU_SSU_SOFTWARE_GEN_RX_PAUSE_REG              (PPE_SSU_BASE + 0x4C8)  /* 接收方向软件生成流控配置寄存器 */
#define PPE_SSU_SSU_SOFTWARE_GEN_TX_PAUSE_REG              (PPE_SSU_BASE + 0x4CC)  /* 发送方向软件生成流控配置寄存器 */
#define PPE_SSU_SSU_PKT_MAX_DROP_ENABLE_REG                (PPE_SSU_BASE + 0x4D0)  /* SSU缓存报文最大长度丢弃使能寄存器 */
#define PPE_SSU_SSU_PKT_MAX_CELL_NUM_REG                   (PPE_SSU_BASE + 0x4D4)  /* SSU缓存报文最大长度 */
#define PPE_SSU_SSU_BANK_BALANCE_REG                       (PPE_SSU_BASE + 0x4D8)  /* BANK均衡配置寄存器 */
#define PPE_SSU_SSU_DROP_REV_PD_THR_REG                    (PPE_SSU_BASE + 0x4E0)  /* 丢弃队列预留PD配置寄存器 */
#define PPE_SSU_SSU_PPP_MB_RD_OST_NUM_REG                  (PPE_SSU_BASE + 0x4E4)  /* 读取PPP多播信息outstand控制寄存器 */
#define PPE_SSU_SSU_EPLA_INF_IND_REG                       (PPE_SSU_BASE + 0x4E8)  /* EPLA模块接口对接指示信号 */
#define PPE_SSU_SSU_PKT_PADDING_IND_REG                    (PPE_SSU_BASE + 0x800)  /* 短包是否做padding指示 */
#define PPE_SSU_SSU_TX_TM_INFO_BYPASS_REG                  (PPE_SSU_BASE + 0x804)  /* HOST方向补偿信息bypass寄存器 */
#define PPE_SSU_SSU_TX_SCH_INFO_BYPASS_REG                 (PPE_SSU_BASE + 0x808)  /* HOST方向补偿信息bypass寄存器 */
#define PPE_SSU_SSU_DUP_UP_LENTH_OFFSET_REG                (PPE_SSU_BASE + 0x810)  /* 多播调度的packet长度补偿 */
#define PPE_SSU_SSU_ETS_UP_LENTH_OFFSET_REG                (PPE_SSU_BASE + 0x814)  /* 发送方向ETS调度报文补偿长度 */
#define PPE_SSU_SSU_MULT_QCN_UP_LENTH_OFFSET_REG           (PPE_SSU_BASE + 0x818)  /* 多播QCN BYTE INFO的packet长度补偿 */
#define PPE_SSU_SSU_SGL_QCN_UP_LENTH_OFFSET_REG            (PPE_SSU_BASE + 0x81C)  /* 单播QCN BYTE INFO的packet长度补偿 */
#define PPE_SSU_SSU_CLK_GATING_CTRL_REG                    (PPE_SSU_BASE + 0x840)  /* 时钟自动门控控制寄存器。 */
#define PPE_SSU_SSU_GATING_WINDOW_REG                      (PPE_SSU_BASE + 0x844)  /* 统计门控打开的窗口大小 */
#define PPE_SSU_SSU_GATING_STS_REG                         (PPE_SSU_BASE + 0x848)  /* 统计门控打开的窗口中打开时钟的周期 */
#define PPE_SSU_SSU_CLK_GATING_EN0_REG                     (PPE_SSU_BASE + 0x850)  /* 时钟自动门控使能寄存器0。 */
#define PPE_SSU_SSU_CLK_GATING_EN1_REG                     (PPE_SSU_BASE + 0x854)  /* 时钟自动门控使能寄存器1。 */
#define PPE_SSU_SSU_CLK_GATING_EN2_REG                     (PPE_SSU_BASE + 0x858)  /* 时钟自动门控使能寄存器2。 */
#define PPE_SSU_SSU_CLK_GATING_EN3_REG                     (PPE_SSU_BASE + 0x85C)  /* 时钟自动门控使能寄存器3。 */
#define PPE_SSU_SSU_BMP_AGEING_CTRL_REG                    (PPE_SSU_BASE + 0xA00)  /* BANK BITMAP老化扫描配置寄存器 */
#define PPE_SSU_SSU_BMP_AGEING_SAMPLE_THD_REG              (PPE_SSU_BASE + 0xA04)  /* BANK BITMAP采样门限配置寄存器 */
#define PPE_SSU_SSU_BMP_AGEING_INT_THD_REG                 (PPE_SSU_BASE + 0xA08)  /* BANK BITMAP超时老化中断上报门限 */
#define PPE_SSU_SSU_BMP_AGEING_RLS_THD_REG                 (PPE_SSU_BASE + 0xA0C)  /* 老化释放中断上报水线 */
#define PPE_SSU_SSU_BMP_AGEING_TIMEOUT_L_REG               (PPE_SSU_BASE + 0xA10)  /* BANK BITMAP老化时间配置寄存器（低32bit） */
#define PPE_SSU_SSU_BMP_AGEING_TIMEOUT_H_REG               (PPE_SSU_BASE + 0xA14)  /* BANK BITMAP老化时间配置寄存器（低32bit） */
#define PPE_SSU_SSU_BMP_AGEING_ST_BANK0_REG                (PPE_SSU_BASE + 0xA20)  /* BANK0 BITMAP老化状态寄存器 */
#define PPE_SSU_SSU_BMP_AGEING_ST_BANK1_REG                (PPE_SSU_BASE + 0xA24)  /* BANK1 BITMAP老化状态寄存器 */
#define PPE_SSU_SSU_BMP_AGEING_ST_BANK2_REG                (PPE_SSU_BASE + 0xA28)  /* BANK1 BITMAP老化状态寄存器 */
#define PPE_SSU_SSU_BMP_AGEING_ST_BANK3_REG                (PPE_SSU_BASE + 0xA2C)  /* BANK1 BITMAP老化状态寄存器 */
#define PPE_SSU_SSU_BMP_AGEING_RLS_CNT_BANK0_REG           (PPE_SSU_BASE + 0xA30)  /* BANK BITMAP老化释放CELL计数器 */
#define PPE_SSU_SSU_BMP_AGEING_RLS_CNT_BANK1_REG           (PPE_SSU_BASE + 0xA34)  /* BANK BITMAP老化释放CELL计数器 */
#define PPE_SSU_SSU_BMP_AGEING_RLS_CNT_BANK2_REG           (PPE_SSU_BASE + 0xA38)  /* BANK BITMAP老化释放CELL计数器 */
#define PPE_SSU_SSU_BMP_AGEING_RLS_CNT_BANK3_REG           (PPE_SSU_BASE + 0xA3C)  /* BANK BITMAP老化释放CELL计数器 */
#define PPE_SSU_SSU_RUN_MODE_REG                           (PPE_SSU_BASE + 0xB00)  /* SSU工作模式 */
#define PPE_SSU_SSU_CGE_RUN_MODE_REG                       (PPE_SSU_BASE + 0xB04)  /* SSU 模块CGE工作模式 */
#define PPE_SSU_SSU_CGE_PORT_EXD_MODE_REG                  (PPE_SSU_BASE + 0xB08)  /* CGE端口扩展模式配置寄存器 */
#define PPE_SSU_SSU_CGE_RX_PORT_INIT_REG                   (PPE_SSU_BASE + 0xB0C)  /* CGE端口接收方向端分流初始寄存器 */
#define PPE_SSU_SSU_EG_GROUP0_PORT_BITMAP_REG              (PPE_SSU_BASE + 0xB20)  /* EG模块端口与GROUP0映射关系配置寄存器 */
#define PPE_SSU_SSU_EG_GROUP1_PORT_BITMAP_REG              (PPE_SSU_BASE + 0xB24)  /* EG模块端口与GROUP1映射关系配置寄存器 */
#define PPE_SSU_SSU_EG_GROUP2_PORT_BITMAP_REG              (PPE_SSU_BASE + 0xB28)  /* EG模块端口与GROUP2映射关系配置寄存器 */
#define PPE_SSU_SSU_PERF_TEST_SEL_REG                      (PPE_SSU_BASE + 0xB40)  /* SSU出口性能测试使能寄存器 */
#define PPE_SSU_SSU_PERF_TEST_PORT_REG                     (PPE_SSU_BASE + 0xB44)  /* SSU出口性能测试端口寄存器 */
#define PPE_SSU_SSU_IG_HOST_PERF_TEST_STATUS_REG           (PPE_SSU_BASE + 0xB50)  /* SSU出口性能状态寄存器 */
#define PPE_SSU_SSU_IG_MAC_PERF_TEST_STATUS_REG            (PPE_SSU_BASE + 0xB54)  /* SSU出口性能状态寄存器 */
#define PPE_SSU_SSU_EG_HOST_PERF_TEST_STATUS_REG           (PPE_SSU_BASE + 0xB58)  /* SSU出口性能状态寄存器 */
#define PPE_SSU_SSU_EG_MAC_PERF_TEST_STATUS_REG            (PPE_SSU_BASE + 0xB5C)  /* SSU出口性能状态寄存器 */
#define PPE_SSU_SSU_IG_HOST_PERF_TEST_PPS_STATUS_REG       (PPE_SSU_BASE + 0xB60)  /* SSU出口性能PPS状态寄存器 */
#define PPE_SSU_SSU_IG_MAC_PERF_TEST_PPS_STATUS_REG        (PPE_SSU_BASE + 0xB64)  /* SSU出口性能PPS状态寄存器 */
#define PPE_SSU_SSU_EG_HOST_PERF_TEST_PPS_STATUS_REG       (PPE_SSU_BASE + 0xB68)  /* SSU出口性能PPS状态寄存器 */
#define PPE_SSU_SSU_EG_MAC_PERF_TEST_PPS_STATUS_REG        (PPE_SSU_BASE + 0xB6C)  /* SSU出口性能PPS状态寄存器 */
#define PPE_SSU_SSU_EG_MAC_SCH_ROUND_NUM_0_REG             (PPE_SSU_BASE + 0xB80)  /* SSU出口调度轮数控制寄存器 */
#define PPE_SSU_SSU_EG_MAC_SCH_ROUND_NUM_1_REG             (PPE_SSU_BASE + 0xB84)  /* SSU出口调度轮数控制寄存器 */
#define PPE_SSU_SSU_EG_MAC_SCH_ROUND_NUM_2_REG             (PPE_SSU_BASE + 0xB88)  /* SSU出口调度轮数控制寄存器 */
#define PPE_SSU_SSU_EG_MAC_SCH_ROUND_NUM_3_REG             (PPE_SSU_BASE + 0xB8C)  /* SSU出口调度轮数控制寄存器 */
#define PPE_SSU_SSU_OQ_DUP_BP_THD_REG                      (PPE_SSU_BASE + 0xC00)  /* 出口队列总量反压DUP MEMORY水线 */
#define PPE_SSU_SSU_OQ_DROP_THD_REG                        (PPE_SSU_BASE + 0xC04)  /* 出口队列总量丢包水线 */
#define PPE_SSU_SSU_OQ_DUP_COPY_THD_REG                    (PPE_SSU_BASE + 0xC08)  /* 出口队列多播复制水线 */
#define PPE_SSU_SSU_LO_PRI_SGL_BUF_SIZE_REG                (PPE_SSU_BASE + 0xC10)  /* 低优先级单播buffer大小 */
#define PPE_SSU_SSU_LO_PRI_MULT_BUF_SIZE_REG               (PPE_SSU_BASE + 0xC14)  /* 低优先级多播buffer大小 */
#define PPE_SSU_SSU_HI_PRI_MULT_BUF_SIZE_REG               (PPE_SSU_BASE + 0xC18)  /* 高优先级多播buffer大小 */
#define PPE_SSU_SSU_LO_PRI_SGL_DROP_THD_REG                (PPE_SSU_BASE + 0xC20)  /* 低优先级单播丢弃水线 */
#define PPE_SSU_SSU_LO_PRI_MULT_DROP_THD_REG               (PPE_SSU_BASE + 0xC24)  /* 低优先级多播丢弃水线 */
#define PPE_SSU_SSU_HI_PRI_MULT_DROP_THD_REG               (PPE_SSU_BASE + 0xC28)  /* 高优先级多播丢弃水线 */
#define PPE_SSU_SSU_DUP_WRR_WEIGHT_REG                     (PPE_SSU_BASE + 0xC30)  /* 多播调度权重 */
#define PPE_SSU_SSU_DUP_SHAPING_BYPASS_REG                 (PPE_SSU_BASE + 0xC34)  /* 多播复制shap bypass使能信号 */
#define PPE_SSU_SSU_HI_PRI_MUL_SHAPING_REG                 (PPE_SSU_BASE + 0xC40)  /* 高优先级多播的shaper配置参数 */
#define PPE_SSU_SSU_LO_PRI_MUL_SHAPING_REG                 (PPE_SSU_BASE + 0xC44)  /* 低优先级多播的shaper配置参数 */
#define PPE_SSU_SSU_LO_PRI_UNI_SHAPING_REG                 (PPE_SSU_BASE + 0xC48)  /* 低优先级单播的shaper配置参数 */
#define PPE_SSU_SSU_CAR_CFG0_0_REG                         (PPE_SSU_BASE + 0xD00)  /* CAR算法桶速率的配置寄存器0 */
#define PPE_SSU_SSU_CAR_CFG0_1_REG                         (PPE_SSU_BASE + 0xD10)  /* CAR算法桶速率的配置寄存器0 */
#define PPE_SSU_SSU_CAR_CFG0_2_REG                         (PPE_SSU_BASE + 0xD20)  /* CAR算法桶速率的配置寄存器0 */
#define PPE_SSU_SSU_CAR_CFG0_3_REG                         (PPE_SSU_BASE + 0xD30)  /* CAR算法桶速率的配置寄存器0 */
#define PPE_SSU_SSU_CAR_CFG0_4_REG                         (PPE_SSU_BASE + 0xD40)  /* CAR算法桶速率的配置寄存器0 */
#define PPE_SSU_SSU_CAR_CFG0_5_REG                         (PPE_SSU_BASE + 0xD50)  /* CAR算法桶速率的配置寄存器0 */
#define PPE_SSU_SSU_CAR_CFG0_6_REG                         (PPE_SSU_BASE + 0xD60)  /* CAR算法桶速率的配置寄存器0 */
#define PPE_SSU_SSU_CAR_CFG0_7_REG                         (PPE_SSU_BASE + 0xD70)  /* CAR算法桶速率的配置寄存器0 */
#define PPE_SSU_SSU_CAR_CFG1_0_REG                         (PPE_SSU_BASE + 0xD04)  /* CAR算法桶深度的配置寄存器1 */
#define PPE_SSU_SSU_CAR_CFG1_1_REG                         (PPE_SSU_BASE + 0xD14)  /* CAR算法桶深度的配置寄存器1 */
#define PPE_SSU_SSU_CAR_CFG1_2_REG                         (PPE_SSU_BASE + 0xD24)  /* CAR算法桶深度的配置寄存器1 */
#define PPE_SSU_SSU_CAR_CFG1_3_REG                         (PPE_SSU_BASE + 0xD34)  /* CAR算法桶深度的配置寄存器1 */
#define PPE_SSU_SSU_CAR_CFG1_4_REG                         (PPE_SSU_BASE + 0xD44)  /* CAR算法桶深度的配置寄存器1 */
#define PPE_SSU_SSU_CAR_CFG1_5_REG                         (PPE_SSU_BASE + 0xD54)  /* CAR算法桶深度的配置寄存器1 */
#define PPE_SSU_SSU_CAR_CFG1_6_REG                         (PPE_SSU_BASE + 0xD64)  /* CAR算法桶深度的配置寄存器1 */
#define PPE_SSU_SSU_CAR_CFG1_7_REG                         (PPE_SSU_BASE + 0xD74)  /* CAR算法桶深度的配置寄存器1 */
#define PPE_SSU_SSU_CAR_CFG2_0_REG                         (PPE_SSU_BASE + 0xD08)  /* CAR算法操作配置寄存器2 */
#define PPE_SSU_SSU_CAR_CFG2_1_REG                         (PPE_SSU_BASE + 0xD18)  /* CAR算法操作配置寄存器2 */
#define PPE_SSU_SSU_CAR_CFG2_2_REG                         (PPE_SSU_BASE + 0xD28)  /* CAR算法操作配置寄存器2 */
#define PPE_SSU_SSU_CAR_CFG2_3_REG                         (PPE_SSU_BASE + 0xD38)  /* CAR算法操作配置寄存器2 */
#define PPE_SSU_SSU_CAR_CFG2_4_REG                         (PPE_SSU_BASE + 0xD48)  /* CAR算法操作配置寄存器2 */
#define PPE_SSU_SSU_CAR_CFG2_5_REG                         (PPE_SSU_BASE + 0xD58)  /* CAR算法操作配置寄存器2 */
#define PPE_SSU_SSU_CAR_CFG2_6_REG                         (PPE_SSU_BASE + 0xD68)  /* CAR算法操作配置寄存器2 */
#define PPE_SSU_SSU_CAR_CFG2_7_REG                         (PPE_SSU_BASE + 0xD78)  /* CAR算法操作配置寄存器2 */
#define PPE_SSU_SSU_CAR_CFG3_0_REG                         (PPE_SSU_BASE + 0xD0C)  /* CAR算法操作配置寄存器3 */
#define PPE_SSU_SSU_CAR_CFG3_1_REG                         (PPE_SSU_BASE + 0xD1C)  /* CAR算法操作配置寄存器3 */
#define PPE_SSU_SSU_CAR_CFG3_2_REG                         (PPE_SSU_BASE + 0xD2C)  /* CAR算法操作配置寄存器3 */
#define PPE_SSU_SSU_CAR_CFG3_3_REG                         (PPE_SSU_BASE + 0xD3C)  /* CAR算法操作配置寄存器3 */
#define PPE_SSU_SSU_CAR_CFG3_4_REG                         (PPE_SSU_BASE + 0xD4C)  /* CAR算法操作配置寄存器3 */
#define PPE_SSU_SSU_CAR_CFG3_5_REG                         (PPE_SSU_BASE + 0xD5C)  /* CAR算法操作配置寄存器3 */
#define PPE_SSU_SSU_CAR_CFG3_6_REG                         (PPE_SSU_BASE + 0xD6C)  /* CAR算法操作配置寄存器3 */
#define PPE_SSU_SSU_CAR_CFG3_7_REG                         (PPE_SSU_BASE + 0xD7C)  /* CAR算法操作配置寄存器3 */
#define PPE_SSU_SSU_IG_RX_PKT_BP_GAP_REG                   (PPE_SSU_BASE + 0xF00)  /* 入口接收报文反压水线 */
#define PPE_SSU_SSU_IG_TX_PKT_BP_GAP_REG                   (PPE_SSU_BASE + 0xF04)  /* 入口发送报文反压水线 */
#define PPE_SSU_SSU_TM_INF_FIFO_GAP_REG                    (PPE_SSU_BASE + 0xF08)  /* TM补偿信息缓存fifo反压水线 */
#define PPE_SSU_SSU_PPP_RLT_FIFO_GAP_REG                   (PPE_SSU_BASE + 0xF0C)  /* PPP查表结果缓存fifo反压水线 */
#define PPE_SSU_SSU_CMD_PKT_ID_FIFO_GAP_REG                (PPE_SSU_BASE + 0xF10)  /* PPP查表ID缓存fifo反压水线 */
#define PPE_SSU_SSU_DUP_STORE_INFO_FIFO_GAP_REG            (PPE_SSU_BASE + 0xF14)  /* 多播缓存信息fifo反压水线 */
#define PPE_SSU_SSU_DUP_MULT_FIFO_GAP_REG                  (PPE_SSU_BASE + 0xF18)  /* 多播复制任务缓存FIFO反压水线 */
#define PPE_SSU_SSU_DUP_SGL_FIFO_GAP_REG                   (PPE_SSU_BASE + 0xF1C)  /* 单播任务缓存FIFO反压水线 */
#define PPE_SSU_SSU_END_DROP_FIFO_GAP_REG                  (PPE_SSU_BASE + 0xF20)  /* 多播任务丢弃缓存FIFO反压水线 */
#define PPE_SSU_SSU_MB_RLT_FIFO_GAP_REG                    (PPE_SSU_BASE + 0xF24)  /* 多播对赌结果缓存FIFO反压水线 */
#define PPE_SSU_SSU_MB_INFO_FIFO_GAP_REG                   (PPE_SSU_BASE + 0xF28)  /* 多播任务fifo反压水线 */
#define PPE_SSU_SSU_DUP_DROP_FIFO_GAP_REG                  (PPE_SSU_BASE + 0xF2C)  /* 多播丢弃fifo反压水线 */
#define PPE_SSU_SSU_IG_PKT_PORT_BP_THD_REG                 (PPE_SSU_BASE + 0xF30)  /* 入口报文端口反压水线 */
#define PPE_SSU_SSU_RX_CMD_FIFO_AFUL_THD_REG               (PPE_SSU_BASE + 0xF34)  /* 接收方向查表ID管理资源水线 */
#define PPE_SSU_SSU_TX_CMD_FIFO_AFUL_THD_REG               (PPE_SSU_BASE + 0xF38)  /* 发送方向查表ID管理资源水线 */
#define PPE_SSU_SSU_DROP_CMD_FIFO_AFUL_THD_REG             (PPE_SSU_BASE + 0xF3C)  /* 丢弃查表ID管理资源水线 */
#define PPE_SSU_SSU_ECO_CFG0_REG                           (PPE_SSU_BASE + 0xF80)  /* 保留配置寄存器0 */
#define PPE_SSU_SSU_ECO_CFG1_REG                           (PPE_SSU_BASE + 0xF84)  /* 保留配置寄存器1 */
#define PPE_SSU_SSU_ECO_CFG2_REG                           (PPE_SSU_BASE + 0xF88)  /* 保留配置寄存器2 */
#define PPE_SSU_SSU_ECO_CFG3_REG                           (PPE_SSU_BASE + 0xF8C)  /* 保留配置寄存器3 */
#define PPE_SSU_SSU_ECO_STATUS0_REG                        (PPE_SSU_BASE + 0xF90)  /* 保留状态寄存器0 */
#define PPE_SSU_SSU_ECO_STATUS1_REG                        (PPE_SSU_BASE + 0xF94)  /* 保留状态寄存器1 */
#define PPE_SSU_SSU_ECO_STATUS2_REG                        (PPE_SSU_BASE + 0xF98)  /* 保留状态寄存器2 */
#define PPE_SSU_SSU_ECO_STATUS3_REG                        (PPE_SSU_BASE + 0xF9C)  /* 保留状态寄存器3 */
#define PPE_SSU_SSU_IG_RX_PKT_CNT_0_REG                    (PPE_SSU_BASE + 0x4000) /* 入口接收报文统计（来自MAC方向） */
#define PPE_SSU_SSU_IG_RX_PKT_CNT_1_REG                    (PPE_SSU_BASE + 0x4080) /* 入口接收报文统计（来自MAC方向） */
#define PPE_SSU_SSU_IG_RX_PKT_CNT_2_REG                    (PPE_SSU_BASE + 0x4100) /* 入口接收报文统计（来自MAC方向） */
#define PPE_SSU_SSU_IG_RX_PKT_CNT_3_REG                    (PPE_SSU_BASE + 0x4180) /* 入口接收报文统计（来自MAC方向） */
#define PPE_SSU_SSU_IG_TX_PKT_CNT_0_REG                    (PPE_SSU_BASE + 0x4008) /* 入口发送报文统计（来自HOST方向） */
#define PPE_SSU_SSU_IG_TX_PKT_CNT_1_REG                    (PPE_SSU_BASE + 0x4088) /* 入口发送报文统计（来自HOST方向） */
#define PPE_SSU_SSU_IG_TX_PKT_CNT_2_REG                    (PPE_SSU_BASE + 0x4108) /* 入口发送报文统计（来自HOST方向） */
#define PPE_SSU_SSU_IG_TX_PKT_CNT_3_REG                    (PPE_SSU_BASE + 0x4188) /* 入口发送报文统计（来自HOST方向） */
#define PPE_SSU_SSU_EG_RX_PKT_CNT_0_REG                    (PPE_SSU_BASE + 0x4010) /* 出口接收报文统计（送往HOST方向） */
#define PPE_SSU_SSU_EG_RX_PKT_CNT_1_REG                    (PPE_SSU_BASE + 0x4090) /* 出口接收报文统计（送往HOST方向） */
#define PPE_SSU_SSU_EG_RX_PKT_CNT_2_REG                    (PPE_SSU_BASE + 0x4110) /* 出口接收报文统计（送往HOST方向） */
#define PPE_SSU_SSU_EG_RX_PKT_CNT_3_REG                    (PPE_SSU_BASE + 0x4190) /* 出口接收报文统计（送往HOST方向） */
#define PPE_SSU_SSU_EG_TX_PKT_CNT_0_REG                    (PPE_SSU_BASE + 0x4018) /* 出口发送报文统计（送往MAC方向） */
#define PPE_SSU_SSU_EG_TX_PKT_CNT_1_REG                    (PPE_SSU_BASE + 0x4098) /* 出口发送报文统计（送往MAC方向） */
#define PPE_SSU_SSU_EG_TX_PKT_CNT_2_REG                    (PPE_SSU_BASE + 0x4118) /* 出口发送报文统计（送往MAC方向） */
#define PPE_SSU_SSU_EG_TX_PKT_CNT_3_REG                    (PPE_SSU_BASE + 0x4198) /* 出口发送报文统计（送往MAC方向） */
#define PPE_SSU_SSU_IG_RX_SHORT_PKT_CNT_0_REG              (PPE_SSU_BASE + 0x4020) /* 入口接收超短报文统计（来自HOST方向） */
#define PPE_SSU_SSU_IG_RX_SHORT_PKT_CNT_1_REG              (PPE_SSU_BASE + 0x40A0) /* 入口接收超短报文统计（来自HOST方向） */
#define PPE_SSU_SSU_IG_RX_SHORT_PKT_CNT_2_REG              (PPE_SSU_BASE + 0x4120) /* 入口接收超短报文统计（来自HOST方向） */
#define PPE_SSU_SSU_IG_RX_SHORT_PKT_CNT_3_REG              (PPE_SSU_BASE + 0x41A0) /* 入口接收超短报文统计（来自HOST方向） */
#define PPE_SSU_SSU_IG_TX_SHORT_PKT_CNT_0_REG              (PPE_SSU_BASE + 0x4024) /* 入口发送超短报文统计（来自HOST方向） */
#define PPE_SSU_SSU_IG_TX_SHORT_PKT_CNT_1_REG              (PPE_SSU_BASE + 0x40A4) /* 入口发送超短报文统计（来自HOST方向） */
#define PPE_SSU_SSU_IG_TX_SHORT_PKT_CNT_2_REG              (PPE_SSU_BASE + 0x4124) /* 入口发送超短报文统计（来自HOST方向） */
#define PPE_SSU_SSU_IG_TX_SHORT_PKT_CNT_3_REG              (PPE_SSU_BASE + 0x41A4) /* 入口发送超短报文统计（来自HOST方向） */
#define PPE_SSU_SSU_IG_RX_ERR_PKT_CNT_0_REG                (PPE_SSU_BASE + 0x4028) /* 入口接收错误报文统计（来自MAC方向） */
#define PPE_SSU_SSU_IG_RX_ERR_PKT_CNT_1_REG                (PPE_SSU_BASE + 0x40A8) /* 入口接收错误报文统计（来自MAC方向） */
#define PPE_SSU_SSU_IG_RX_ERR_PKT_CNT_2_REG                (PPE_SSU_BASE + 0x4128) /* 入口接收错误报文统计（来自MAC方向） */
#define PPE_SSU_SSU_IG_RX_ERR_PKT_CNT_3_REG                (PPE_SSU_BASE + 0x41A8) /* 入口接收错误报文统计（来自MAC方向） */
#define PPE_SSU_SSU_IG_TX_ERR_PKT_CNT_0_REG                (PPE_SSU_BASE + 0x402C) /* 入口发送错误报文统计（来自HOST方向） */
#define PPE_SSU_SSU_IG_TX_ERR_PKT_CNT_1_REG                (PPE_SSU_BASE + 0x40AC) /* 入口发送错误报文统计（来自HOST方向） */
#define PPE_SSU_SSU_IG_TX_ERR_PKT_CNT_2_REG                (PPE_SSU_BASE + 0x412C) /* 入口发送错误报文统计（来自HOST方向） */
#define PPE_SSU_SSU_IG_TX_ERR_PKT_CNT_3_REG                (PPE_SSU_BASE + 0x41AC) /* 入口发送错误报文统计（来自HOST方向） */
#define PPE_SSU_SSU_EG_RX_ROCEE_PKT_CNT_0_REG              (PPE_SSU_BASE + 0x4030) /* 出口接收报文统计（送往ROCEE方向） */
#define PPE_SSU_SSU_EG_RX_ROCEE_PKT_CNT_1_REG              (PPE_SSU_BASE + 0x40B0) /* 出口接收报文统计（送往ROCEE方向） */
#define PPE_SSU_SSU_EG_RX_ROCEE_PKT_CNT_2_REG              (PPE_SSU_BASE + 0x4130) /* 出口接收报文统计（送往ROCEE方向） */
#define PPE_SSU_SSU_EG_RX_ROCEE_PKT_CNT_3_REG              (PPE_SSU_BASE + 0x41B0) /* 出口接收报文统计（送往ROCEE方向） */
#define PPE_SSU_SSU_TM_BYTE_INFO_CNT_REG                   (PPE_SSU_BASE + 0x6000) /* 发送给TM的补偿信息统计 */
#define PPE_SSU_SSU_TXSCH_BYTE_INFO_CNT_REG                (PPE_SSU_BASE + 0x6004) /* 发送给TXSCH的补偿信息统计 */
#define PPE_SSU_SSU_PKT_BANK0_RSC_CNT_REG                  (PPE_SSU_BASE + 0x6010) /* 报文数据缓存BANK0剩余资源统计 */
#define PPE_SSU_SSU_PKT_BANK1_RSC_CNT_REG                  (PPE_SSU_BASE + 0x6014) /* 报文数据缓存BANK1剩余资源统计 */
#define PPE_SSU_SSU_PKT_BANK2_RSC_CNT_REG                  (PPE_SSU_BASE + 0x6018) /* 报文数据缓存BANK2剩余资源统计 */
#define PPE_SSU_SSU_PKT_BANK3_RSC_CNT_REG                  (PPE_SSU_BASE + 0x601C) /* 报文数据缓存BANK3剩余资源统计 */
#define PPE_SSU_SSU_OQ_PD_RSC_CNT_REG                      (PPE_SSU_BASE + 0x6020) /* 报文PD缓存BANK0剩余资源统计 */
#define PPE_SSU_SSU_PPP_MULT_TOTAL_CNT_REG                 (PPE_SSU_BASE + 0x6024) /* PPP返回给SSU多播总个数统计 */
#define PPE_SSU_SSU_PPP_RX_KEY_REQ_CNT_REG                 (PPE_SSU_BASE + 0x6030) /* SSU给PPP接收查表请求统计 */
#define PPE_SSU_SSU_PPP_TX_KEY_REQ_CNT_REG                 (PPE_SSU_BASE + 0x6034) /* SSU给PPP发送查表请求统计 */
#define PPE_SSU_PPP_SSU_RX_KEY_RLT_CNT_REG                 (PPE_SSU_BASE + 0x6038) /* PPP返回接收查表结果统计 */
#define PPE_SSU_PPP_SSU_TX_KEY_RLT_CNT_REG                 (PPE_SSU_BASE + 0x603C) /* PPP返回发送查表结果统计 */
#define PPE_SSU_PPP_SSU_KEY_DROP_CNT_REG                   (PPE_SSU_BASE + 0x6040) /* PPP返回丢弃查表统计 */
#define PPE_SSU_PPP_SSU_RLT_DROP_CNT_REG                   (PPE_SSU_BASE + 0x6044) /* PPP返回丢弃查表统计 */
#define PPE_SSU_SSU_RX_L2ERR_DROP_CNT_REG                  (PPE_SSU_BASE + 0x6048) /* 接收方向L2错误丢弃报文统计 */
#define PPE_SSU_SSU_TX_L2ERR_DROP_CNT_REG                  (PPE_SSU_BASE + 0x604C) /* 发送方向L2错误丢弃报文统计 */
#define PPE_SSU_SSU_RX_PKT_FULL_DROP_CNT_REG               (PPE_SSU_BASE + 0x6050) /* 接收报文整包丢弃统计 */
#define PPE_SSU_SSU_TX_PKT_FULL_DROP_CNT_REG               (PPE_SSU_BASE + 0x6054) /* 发送报文整包丢弃统计 */
#define PPE_SSU_SSU_RX_PKT_PART_DROP_CNT_REG               (PPE_SSU_BASE + 0x6058) /* 接收报文截包丢弃统计 */
#define PPE_SSU_SSU_TX_PKT_PART_DROP_CNT_REG               (PPE_SSU_BASE + 0x605C) /* 发送报文截包丢弃统计 */
#define PPE_SSU_SSU_HI_PRI_MULT_DROP_CNT_REG               (PPE_SSU_BASE + 0x6060) /* 高优先级多播丢弃统计 */
#define PPE_SSU_SSU_LO_PRI_MULT_DROP_CNT_REG               (PPE_SSU_BASE + 0x6064) /* 低优先级多播丢弃统计 */
#define PPE_SSU_SSU_HI_PRI_SGL_DROP_CNT_REG                (PPE_SSU_BASE + 0x6068) /* 高优先级单播丢弃统计 */
#define PPE_SSU_SSU_LO_PRI_SGL_DROP_CNT_REG                (PPE_SSU_BASE + 0x606C) /* 低优先级单播丢弃统计 */
#define PPE_SSU_SSU_CAR_DROP_CNT_REG                       (PPE_SSU_BASE + 0x6070) /* 接收方向CAR丢弃报文统计 */
#define PPE_SSU_SSU_MULT_UNCOPY_CNT_REG                    (PPE_SSU_BASE + 0x6074) /* 多播失败导致丢弃统计 */
#define PPE_SSU_SSU_SYS_ABNORMAL_DROP_CNT_REG              (PPE_SSU_BASE + 0x6078) /* 系统异常导致的丢弃统计 */
#define PPE_SSU_SSU_OQ_LIST_PTR_CNT_REG                    (PPE_SSU_BASE + 0x6080) /* 选择出口队列报文个数统计 */
#define PPE_SSU_SSU_IN_PORT_CELL_CNT_REG                   (PPE_SSU_BASE + 0x6084) /* 选择入口端口报文cell统计 */
#define PPE_SSU_SSU_IN_PORT_PD_CNT_REG                     (PPE_SSU_BASE + 0x6088) /* 选择入口端口报文PD统计 */
#define PPE_SSU_SSU_OT_TOTAL_PD_CNT_REG                    (PPE_SSU_BASE + 0x6090) /* 出口空间中总的待处理报文个数统计 */
#define PPE_SSU_SSU_OT_TOTAL_MULT_PD_CNT_REG               (PPE_SSU_BASE + 0x6094) /* 出口空间中总的待处理多播报文个数统计 */
#define PPE_SSU_SSU_OT_PORT_MULT_PD_CNT_REG                (PPE_SSU_BASE + 0x6098) /* 出口端口中多播报文个数统计 */
#define PPE_SSU_SSU_OT_PORT_MULT_CELL_CNT_REG              (PPE_SSU_BASE + 0x609C) /* 出口端口中多播报文CELL统计 */
#define PPE_SSU_SSU_OT_PORT_PD_CNT_REG                     (PPE_SSU_BASE + 0x60A0) /* 出口端口中报文个数统计 */
#define PPE_SSU_SSU_OT_PORT_CELL_CNT_REG                   (PPE_SSU_BASE + 0x60A4) /* 出口端口中报文CELL统计 */
#define PPE_SSU_SSU_OQ_PD_CNT_REG                          (PPE_SSU_BASE + 0x60A8) /* 出口队列中报文个数统计 */
#define PPE_SSU_SSU_OQ_CELL_CNT_REG                        (PPE_SSU_BASE + 0x60AC) /* 出口队列中报文CELL统计 */
#define PPE_SSU_SSU_MEM_SBIT_ERR_CNT_REG                   (PPE_SSU_BASE + 0x60B0) /* 单bit ECC统计寄存器 */
#define PPE_SSU_SSU_MEM_MBIT_ERR_CNT_REG                   (PPE_SSU_BASE + 0x60B4) /* 2bit ECC统计寄存器 */
#define PPE_SSU_SSU_FIFO_DFX0_REG                          (PPE_SSU_BASE + 0x6100) /* FIFO空满指示状态 */
#define PPE_SSU_SSU_FIFO_DFX1_REG                          (PPE_SSU_BASE + 0x6104) /* FIFO空满指示状态 */
#define PPE_SSU_SSU_FIFO_DFX2_REG                          (PPE_SSU_BASE + 0x6108) /* FIFO空满指示状态 */
#define PPE_SSU_SSU_FIFO_DFX3_REG                          (PPE_SSU_BASE + 0x610C) /* FIFO空满指示状态 */
#define PPE_SSU_SSU_FIFO_DFX4_REG                          (PPE_SSU_BASE + 0x6110) /* FIFO空满指示状态 */
#define PPE_SSU_SSU_FIFO_DFX5_REG                          (PPE_SSU_BASE + 0x6114) /* FIFO空满指示状态 */
#define PPE_SSU_SSU_FIFO_DFX6_REG                          (PPE_SSU_BASE + 0x6118) /* FIFO空满指示状态 */
#define PPE_SSU_SSU_FIFO_DFX7_REG                          (PPE_SSU_BASE + 0x611C) /* FIFO空满指示状态 */
#define PPE_SSU_SSU_FIFO_DFX8_REG                          (PPE_SSU_BASE + 0x6120) /* FIFO空满指示状态 */
#define PPE_SSU_SSU_FIFO_DFX9_REG                          (PPE_SSU_BASE + 0x6124) /* FIFO空满指示状态 */
#define PPE_SSU_SSU_FIFO_DFX10_REG                         (PPE_SSU_BASE + 0x6128) /* FIFO空满指示状态 */
#define PPE_SSU_SSU_FIFO_DFX11_REG                         (PPE_SSU_BASE + 0x612C) /* FIFO空满指示状态 */
#define PPE_SSU_SSU_FIFO_DFX12_REG                         (PPE_SSU_BASE + 0x6130) /* FIFO空满指示状态 */
#define PPE_SSU_SSU_FIFO_DFX13_REG                         (PPE_SSU_BASE + 0x6134) /* FIFO空满指示状态 */
#define PPE_SSU_SSU_FIFO_DFX14_REG                         (PPE_SSU_BASE + 0x6138) /* FIFO空满指示状态 */
#define PPE_SSU_SSU_FIFO_DFX15_REG                         (PPE_SSU_BASE + 0x613C) /* FIFO空满指示状态 */
#define PPE_SSU_SSU_BP_STATUS0_REG                         (PPE_SSU_BASE + 0x6180) /* 内部反压状态寄存器0 */
#define PPE_SSU_SSU_BP_STATUS1_REG                         (PPE_SSU_BASE + 0x6184) /* 内部反压状态寄存器1 */
#define PPE_SSU_SSU_BP_STATUS2_REG                         (PPE_SSU_BASE + 0x6188) /* 内部反压状态寄存器2 */
#define PPE_SSU_SSU_BP_STATUS3_REG                         (PPE_SSU_BASE + 0x618C) /* 内部反压状态寄存器3 */
#define PPE_SSU_SSU_BP_STATUS4_REG                         (PPE_SSU_BASE + 0x6190) /* 内部反压状态寄存器4 */
#define PPE_SSU_SSU_BP_STATUS5_REG                         (PPE_SSU_BASE + 0x6194) /* 内部反压状态寄存器5 */
#define PPE_SSU_SSU_BP_STATUS6_REG                         (PPE_SSU_BASE + 0x6198) /* 内部反压状态寄存器6 */
#define PPE_SSU_SSU_BP_STATUS7_REG                         (PPE_SSU_BASE + 0x619C) /* 内部反压状态寄存器7 */
#define PPE_SSU_SSU_BANK_BALANCE_STATUS_REG                (PPE_SSU_BASE + 0x61B0) /* BANK Balance状态寄存器 */
#define PPE_SSU_SSU_DUP_SHAP_BMP_REG                       (PPE_SSU_BASE + 0x61B4) /* DUP调度中shaping状态 */
#define PPE_SSU_SSU_ETS_SHAP_BMP0_REG                      (PPE_SSU_BASE + 0x61C0) /* ETS调度中shaping状态0 */
#define PPE_SSU_SSU_ETS_SHAP_BMP1_REG                      (PPE_SSU_BASE + 0x61C4) /* ETS调度中shaping状态1 */
#define PPE_SSU_SSU_ETS_SHAP_BMP2_REG                      (PPE_SSU_BASE + 0x61C8) /* ETS调度中shaping状态2 */
#define PPE_SSU_SSU_ETS_SHAP_BMP3_REG                      (PPE_SSU_BASE + 0x61CC) /* ETS调度中shaping状态3 */
#define PPE_SSU_SSU_ETS_SHAP_BMP4_REG                      (PPE_SSU_BASE + 0x61D0) /* ETS调度中shaping状态4 */
#define PPE_SSU_SSU_ETS_SHAP_BMP5_REG                      (PPE_SSU_BASE + 0x61D4) /* ETS调度中shaping状态5 */
#define PPE_SSU_SSU_ETS_SHAP_BMP6_REG                      (PPE_SSU_BASE + 0x61D8) /* ETS调度中shaping状态6 */
#define PPE_SSU_SSU_ETS_SHAP_BMP7_REG                      (PPE_SSU_BASE + 0x61DC) /* ETS调度中shaping状态7 */
#define PPE_SSU_QCN_TC_QSET_0_REG                          (PPE_SSU_BASE + 0x7000) /* 每个PF/VF基于TC的qset设置 */
#define PPE_SSU_QCN_TC_QSET_1_REG                          (PPE_SSU_BASE + 0x7004) /* 每个PF/VF基于TC的qset设置 */
#define PPE_SSU_QCN_TC_QSET_2_REG                          (PPE_SSU_BASE + 0x7008) /* 每个PF/VF基于TC的qset设置 */
#define PPE_SSU_QCN_TC_QSET_3_REG                          (PPE_SSU_BASE + 0x700C) /* 每个PF/VF基于TC的qset设置 */
#define PPE_SSU_QCN_TC_QSET_4_REG                          (PPE_SSU_BASE + 0x7010) /* 每个PF/VF基于TC的qset设置 */
#define PPE_SSU_QCN_TC_QSET_5_REG                          (PPE_SSU_BASE + 0x7014) /* 每个PF/VF基于TC的qset设置 */
#define PPE_SSU_QCN_TC_QSET_6_REG                          (PPE_SSU_BASE + 0x7018) /* 每个PF/VF基于TC的qset设置 */
#define PPE_SSU_QCN_TC_QSET_7_REG                          (PPE_SSU_BASE + 0x701C) /* 每个PF/VF基于TC的qset设置 */
#define PPE_SSU_QCN_TC_QSET_8_REG                          (PPE_SSU_BASE + 0x7020) /* 每个PF/VF基于TC的qset设置 */
#define PPE_SSU_QCN_TC_QSET_9_REG                          (PPE_SSU_BASE + 0x7024) /* 每个PF/VF基于TC的qset设置 */
#define PPE_SSU_QCN_TC_QSET_10_REG                         (PPE_SSU_BASE + 0x7028) /* 每个PF/VF基于TC的qset设置 */
#define PPE_SSU_QCN_TC_QSET_11_REG                         (PPE_SSU_BASE + 0x702C) /* 每个PF/VF基于TC的qset设置 */
#define PPE_SSU_QCN_TC_QSET_12_REG                         (PPE_SSU_BASE + 0x7030) /* 每个PF/VF基于TC的qset设置 */
#define PPE_SSU_QCN_TC_QSET_13_REG                         (PPE_SSU_BASE + 0x7034) /* 每个PF/VF基于TC的qset设置 */
#define PPE_SSU_QCN_TC_QSET_14_REG                         (PPE_SSU_BASE + 0x7038) /* 每个PF/VF基于TC的qset设置 */
#define PPE_SSU_QCN_TC_QSET_15_REG                         (PPE_SSU_BASE + 0x703C) /* 每个PF/VF基于TC的qset设置 */
#define PPE_SSU_SSU_RX_SHARE_BUFFER_SIZE_0_REG             (PPE_SSU_BASE + 0x8000) /* 端口接收方向的packet buffer个数。 */
#define PPE_SSU_SSU_RX_SHARE_BUFFER_SIZE_1_REG             (PPE_SSU_BASE + 0x8080) /* 端口接收方向的packet buffer个数。 */
#define PPE_SSU_SSU_RX_SHARE_BUFFER_SIZE_2_REG             (PPE_SSU_BASE + 0x8100) /* 端口接收方向的packet buffer个数。 */
#define PPE_SSU_SSU_RX_SHARE_BUFFER_SIZE_3_REG             (PPE_SSU_BASE + 0x8180) /* 端口接收方向的packet buffer个数。 */
#define PPE_SSU_SSU_RX_SHARE_THD_0_REG                     (PPE_SSU_BASE + 0x8004) /* 接收方向packet buffer的流控水线 */
#define PPE_SSU_SSU_RX_SHARE_THD_1_REG                     (PPE_SSU_BASE + 0x8084) /* 接收方向packet buffer的流控水线 */
#define PPE_SSU_SSU_RX_SHARE_THD_2_REG                     (PPE_SSU_BASE + 0x8104) /* 接收方向packet buffer的流控水线 */
#define PPE_SSU_SSU_RX_SHARE_THD_3_REG                     (PPE_SSU_BASE + 0x8184) /* 接收方向packet buffer的流控水线 */
#define PPE_SSU_SSU_TX_SHARE_BUFFER_SIZE_0_REG             (PPE_SSU_BASE + 0x8008) /* 端口发送方向的packet buffer个数。 */
#define PPE_SSU_SSU_TX_SHARE_BUFFER_SIZE_1_REG             (PPE_SSU_BASE + 0x8088) /* 端口发送方向的packet buffer个数。 */
#define PPE_SSU_SSU_TX_SHARE_BUFFER_SIZE_2_REG             (PPE_SSU_BASE + 0x8108) /* 端口发送方向的packet buffer个数。 */
#define PPE_SSU_SSU_TX_SHARE_BUFFER_SIZE_3_REG             (PPE_SSU_BASE + 0x8188) /* 端口发送方向的packet buffer个数。 */
#define PPE_SSU_SSU_TX_SHARE_THD_0_REG                     (PPE_SSU_BASE + 0x800C) /* 发送方向packet buffer的流控水线 */
#define PPE_SSU_SSU_TX_SHARE_THD_1_REG                     (PPE_SSU_BASE + 0x808C) /* 发送方向packet buffer的流控水线 */
#define PPE_SSU_SSU_TX_SHARE_THD_2_REG                     (PPE_SSU_BASE + 0x810C) /* 发送方向packet buffer的流控水线 */
#define PPE_SSU_SSU_TX_SHARE_THD_3_REG                     (PPE_SSU_BASE + 0x818C) /* 发送方向packet buffer的流控水线 */
#define PPE_SSU_SSU_PAUSE_TIME_OUT_EN_0_REG                (PPE_SSU_BASE + 0x8010) /* 流控超时功能开关 */
#define PPE_SSU_SSU_PAUSE_TIME_OUT_EN_1_REG                (PPE_SSU_BASE + 0x8090) /* 流控超时功能开关 */
#define PPE_SSU_SSU_PAUSE_TIME_OUT_EN_2_REG                (PPE_SSU_BASE + 0x8110) /* 流控超时功能开关 */
#define PPE_SSU_SSU_PAUSE_TIME_OUT_EN_3_REG                (PPE_SSU_BASE + 0x8190) /* 流控超时功能开关 */
#define PPE_SSU_SSU_PAUSE_TIME_OUT_0_REG                   (PPE_SSU_BASE + 0x8014) /* 流控超时门限配置寄存器 */
#define PPE_SSU_SSU_PAUSE_TIME_OUT_1_REG                   (PPE_SSU_BASE + 0x8094) /* 流控超时门限配置寄存器 */
#define PPE_SSU_SSU_PAUSE_TIME_OUT_2_REG                   (PPE_SSU_BASE + 0x8114) /* 流控超时门限配置寄存器 */
#define PPE_SSU_SSU_PAUSE_TIME_OUT_3_REG                   (PPE_SSU_BASE + 0x8194) /* 流控超时门限配置寄存器 */
#define PPE_SSU_SSU_BMU_POOL_PAUSE_EN_0_REG                (PPE_SSU_BASE + 0x8020) /* BMU POOL与PPE 端口的映射寄存器 */
#define PPE_SSU_SSU_BMU_POOL_PAUSE_EN_1_REG                (PPE_SSU_BASE + 0x80A0) /* BMU POOL与PPE 端口的映射寄存器 */
#define PPE_SSU_SSU_BMU_POOL_PAUSE_EN_2_REG                (PPE_SSU_BASE + 0x8120) /* BMU POOL与PPE 端口的映射寄存器 */
#define PPE_SSU_SSU_BMU_POOL_PAUSE_EN_3_REG                (PPE_SSU_BASE + 0x81A0) /* BMU POOL与PPE 端口的映射寄存器 */
#define PPE_SSU_SSU_RX_PORT_TYPE_0_REG                     (PPE_SSU_BASE + 0x8040) /* 接收端口类型配置寄存器 */
#define PPE_SSU_SSU_RX_PORT_TYPE_1_REG                     (PPE_SSU_BASE + 0x80C0) /* 接收端口类型配置寄存器 */
#define PPE_SSU_SSU_RX_PORT_TYPE_2_REG                     (PPE_SSU_BASE + 0x8140) /* 接收端口类型配置寄存器 */
#define PPE_SSU_SSU_RX_PORT_TYPE_3_REG                     (PPE_SSU_BASE + 0x81C0) /* 接收端口类型配置寄存器 */
#define PPE_SSU_SSU_TX_PORT_TYPE_0_REG                     (PPE_SSU_BASE + 0x8044) /* 发送端口类型配置寄存器 */
#define PPE_SSU_SSU_TX_PORT_TYPE_1_REG                     (PPE_SSU_BASE + 0x80C4) /* 发送端口类型配置寄存器 */
#define PPE_SSU_SSU_TX_PORT_TYPE_2_REG                     (PPE_SSU_BASE + 0x8144) /* 发送端口类型配置寄存器 */
#define PPE_SSU_SSU_TX_PORT_TYPE_3_REG                     (PPE_SSU_BASE + 0x81C4) /* 发送端口类型配置寄存器 */
#define PPE_SSU_SSU_RX_VLAN_TYPE_CFG_0_REG                 (PPE_SSU_BASE + 0x8048) /* VLAN报文编辑的VLAN_TYPE值 */
#define PPE_SSU_SSU_RX_VLAN_TYPE_CFG_1_REG                 (PPE_SSU_BASE + 0x80C8) /* VLAN报文编辑的VLAN_TYPE值 */
#define PPE_SSU_SSU_RX_VLAN_TYPE_CFG_2_REG                 (PPE_SSU_BASE + 0x8148) /* VLAN报文编辑的VLAN_TYPE值 */
#define PPE_SSU_SSU_RX_VLAN_TYPE_CFG_3_REG                 (PPE_SSU_BASE + 0x81C8) /* VLAN报文编辑的VLAN_TYPE值 */
#define PPE_SSU_SSU_TX_VLAN_TYPE_CFG_0_REG                 (PPE_SSU_BASE + 0x804C) /* VLAN报文编辑的VLAN_TYPE值 */
#define PPE_SSU_SSU_TX_VLAN_TYPE_CFG_1_REG                 (PPE_SSU_BASE + 0x80CC) /* VLAN报文编辑的VLAN_TYPE值 */
#define PPE_SSU_SSU_TX_VLAN_TYPE_CFG_2_REG                 (PPE_SSU_BASE + 0x814C) /* VLAN报文编辑的VLAN_TYPE值 */
#define PPE_SSU_SSU_TX_VLAN_TYPE_CFG_3_REG                 (PPE_SSU_BASE + 0x81CC) /* VLAN报文编辑的VLAN_TYPE值 */
#define PPE_SSU_SSU_PORT_ENCTRPT_INFO_0_REG                (PPE_SSU_BASE + 0x8050) /* MAC交换侧加密信息寄存器 */
#define PPE_SSU_SSU_PORT_ENCTRPT_INFO_1_REG                (PPE_SSU_BASE + 0x80D0) /* MAC交换侧加密信息寄存器 */
#define PPE_SSU_SSU_PORT_ENCTRPT_INFO_2_REG                (PPE_SSU_BASE + 0x8150) /* MAC交换侧加密信息寄存器 */
#define PPE_SSU_SSU_PORT_ENCTRPT_INFO_3_REG                (PPE_SSU_BASE + 0x81D0) /* MAC交换侧加密信息寄存器 */
#define PPE_SSU_SSU_MAC_ETS_TC_WEIGHT_0_REG                (PPE_SSU_BASE + 0xA000) /* ETS MAC侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_MAC_ETS_TC_WEIGHT_1_REG                (PPE_SSU_BASE + 0xA004) /* ETS MAC侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_MAC_ETS_TC_WEIGHT_2_REG                (PPE_SSU_BASE + 0xA008) /* ETS MAC侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_MAC_ETS_TC_WEIGHT_3_REG                (PPE_SSU_BASE + 0xA00C) /* ETS MAC侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_MAC_ETS_TC_WEIGHT_4_REG                (PPE_SSU_BASE + 0xA010) /* ETS MAC侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_MAC_ETS_TC_WEIGHT_5_REG                (PPE_SSU_BASE + 0xA014) /* ETS MAC侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_MAC_ETS_TC_WEIGHT_6_REG                (PPE_SSU_BASE + 0xA018) /* ETS MAC侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_MAC_ETS_TC_WEIGHT_7_REG                (PPE_SSU_BASE + 0xA01C) /* ETS MAC侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_MAC_ETS_TC_WEIGHT_8_REG                (PPE_SSU_BASE + 0xA020) /* ETS MAC侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_MAC_ETS_TC_WEIGHT_9_REG                (PPE_SSU_BASE + 0xA024) /* ETS MAC侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_MAC_ETS_TC_WEIGHT_10_REG               (PPE_SSU_BASE + 0xA028) /* ETS MAC侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_MAC_ETS_TC_WEIGHT_11_REG               (PPE_SSU_BASE + 0xA02C) /* ETS MAC侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_MAC_ETS_TC_WEIGHT_12_REG               (PPE_SSU_BASE + 0xA030) /* ETS MAC侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_MAC_ETS_TC_WEIGHT_13_REG               (PPE_SSU_BASE + 0xA034) /* ETS MAC侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_MAC_ETS_TC_WEIGHT_14_REG               (PPE_SSU_BASE + 0xA038) /* ETS MAC侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_MAC_ETS_TC_WEIGHT_15_REG               (PPE_SSU_BASE + 0xA03C) /* ETS MAC侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_MAC_ETS_TC_WEIGHT_16_REG               (PPE_SSU_BASE + 0xA040) /* ETS MAC侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_MAC_ETS_TC_WEIGHT_17_REG               (PPE_SSU_BASE + 0xA044) /* ETS MAC侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_MAC_ETS_TC_WEIGHT_18_REG               (PPE_SSU_BASE + 0xA048) /* ETS MAC侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_MAC_ETS_TC_WEIGHT_19_REG               (PPE_SSU_BASE + 0xA04C) /* ETS MAC侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_MAC_ETS_TC_WEIGHT_20_REG               (PPE_SSU_BASE + 0xA050) /* ETS MAC侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_MAC_ETS_TC_WEIGHT_21_REG               (PPE_SSU_BASE + 0xA054) /* ETS MAC侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_MAC_ETS_TC_WEIGHT_22_REG               (PPE_SSU_BASE + 0xA058) /* ETS MAC侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_MAC_ETS_TC_WEIGHT_23_REG               (PPE_SSU_BASE + 0xA05C) /* ETS MAC侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_MAC_ETS_TC_WEIGHT_24_REG               (PPE_SSU_BASE + 0xA060) /* ETS MAC侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_MAC_ETS_TC_WEIGHT_25_REG               (PPE_SSU_BASE + 0xA064) /* ETS MAC侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_MAC_ETS_TC_WEIGHT_26_REG               (PPE_SSU_BASE + 0xA068) /* ETS MAC侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_MAC_ETS_TC_WEIGHT_27_REG               (PPE_SSU_BASE + 0xA06C) /* ETS MAC侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_MAC_ETS_TC_WEIGHT_28_REG               (PPE_SSU_BASE + 0xA070) /* ETS MAC侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_MAC_ETS_TC_WEIGHT_29_REG               (PPE_SSU_BASE + 0xA074) /* ETS MAC侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_MAC_ETS_TC_WEIGHT_30_REG               (PPE_SSU_BASE + 0xA078) /* ETS MAC侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_MAC_ETS_TC_WEIGHT_31_REG               (PPE_SSU_BASE + 0xA07C) /* ETS MAC侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_MAC_ETS_TC_SP_MODE_REG                 (PPE_SSU_BASE + 0xA100) /* ETS MAC侧 TC nodes sp属性配置 */
#define PPE_SSU_SSU_MAC_ETS_TCG_WEIGHT_0_REG               \
    (PPE_SSU_BASE + 0xA104) /* ETS MAC侧 TC Group nodes weight配置表 */
#define PPE_SSU_SSU_MAC_ETS_TCG_WEIGHT_1_REG              \
    (PPE_SSU_BASE + 0xA108) /* ETS MAC侧 TC Group nodes weight配置表 */
#define PPE_SSU_SSU_MAC_ETS_TCG_WEIGHT_2_REG               \
    (PPE_SSU_BASE + 0xA10C) /* ETS MAC侧 TC Group nodes weight配置表 */
#define PPE_SSU_SSU_MAC_ETS_TCG_WEIGHT_3_REG               \
    (PPE_SSU_BASE + 0xA110) /* ETS MAC侧 TC Group nodes weight配置表 */
#define PPE_SSU_SSU_MAC_ETS_TCG_SP_MODE_REG                (PPE_SSU_BASE + 0xA204) /* ETS MAC侧 TCG nodes sp属性配置 */

/* ETS MAC侧 TC Group nodes 级的shaper配置参数 */
#define PPE_SSU_SSU_MAC_ETS_TCG_SHAPING_0_REG              (PPE_SSU_BASE + 0xA208)
#define PPE_SSU_SSU_MAC_ETS_TCG_SHAPING_1_REG              (PPE_SSU_BASE + 0xA20C)
#define PPE_SSU_SSU_MAC_ETS_TCG_SHAPING_2_REG              (PPE_SSU_BASE + 0xA210)
#define PPE_SSU_SSU_MAC_ETS_TCG_SHAPING_3_REG              (PPE_SSU_BASE + 0xA214)

/* ETS MAC侧 TC nodes 级的shaper配置参数 */
#define PPE_SSU_SSU_MAC_ETS_TC_SHAPING_0_REG               (PPE_SSU_BASE + 0xA218)
#define PPE_SSU_SSU_MAC_ETS_TC_SHAPING_1_REG               (PPE_SSU_BASE + 0xA21C)
#define PPE_SSU_SSU_MAC_ETS_TC_SHAPING_2_REG               (PPE_SSU_BASE + 0xA220)
#define PPE_SSU_SSU_MAC_ETS_TC_SHAPING_3_REG               (PPE_SSU_BASE + 0xA224)
#define PPE_SSU_SSU_MAC_ETS_TC_SHAPING_4_REG               (PPE_SSU_BASE + 0xA228)
#define PPE_SSU_SSU_MAC_ETS_TC_SHAPING_5_REG               (PPE_SSU_BASE + 0xA22C)
#define PPE_SSU_SSU_MAC_ETS_TC_SHAPING_6_REG               (PPE_SSU_BASE + 0xA230)
#define PPE_SSU_SSU_MAC_ETS_TC_SHAPING_7_REG               (PPE_SSU_BASE + 0xA234)
#define PPE_SSU_SSU_MAC_ETS_TC_SHAPING_8_REG               (PPE_SSU_BASE + 0xA238)
#define PPE_SSU_SSU_MAC_ETS_TC_SHAPING_9_REG               (PPE_SSU_BASE + 0xA23C)
#define PPE_SSU_SSU_MAC_ETS_TC_SHAPING_10_REG              (PPE_SSU_BASE + 0xA240)
#define PPE_SSU_SSU_MAC_ETS_TC_SHAPING_11_REG              (PPE_SSU_BASE + 0xA244)
#define PPE_SSU_SSU_MAC_ETS_TC_SHAPING_12_REG              (PPE_SSU_BASE + 0xA248)
#define PPE_SSU_SSU_MAC_ETS_TC_SHAPING_13_REG              (PPE_SSU_BASE + 0xA24C)
#define PPE_SSU_SSU_MAC_ETS_TC_SHAPING_14_REG              (PPE_SSU_BASE + 0xA250)
#define PPE_SSU_SSU_MAC_ETS_TC_SHAPING_15_REG              (PPE_SSU_BASE + 0xA254)
#define PPE_SSU_SSU_MAC_ETS_TC_SHAPING_16_REG              (PPE_SSU_BASE + 0xA258)
#define PPE_SSU_SSU_MAC_ETS_TC_SHAPING_17_REG              (PPE_SSU_BASE + 0xA25C)
#define PPE_SSU_SSU_MAC_ETS_TC_SHAPING_18_REG              (PPE_SSU_BASE + 0xA260)
#define PPE_SSU_SSU_MAC_ETS_TC_SHAPING_19_REG              (PPE_SSU_BASE + 0xA264)
#define PPE_SSU_SSU_MAC_ETS_TC_SHAPING_20_REG              (PPE_SSU_BASE + 0xA268)
#define PPE_SSU_SSU_MAC_ETS_TC_SHAPING_21_REG              (PPE_SSU_BASE + 0xA26C)
#define PPE_SSU_SSU_MAC_ETS_TC_SHAPING_22_REG              (PPE_SSU_BASE + 0xA270)
#define PPE_SSU_SSU_MAC_ETS_TC_SHAPING_23_REG              (PPE_SSU_BASE + 0xA274)
#define PPE_SSU_SSU_MAC_ETS_TC_SHAPING_24_REG              (PPE_SSU_BASE + 0xA278)
#define PPE_SSU_SSU_MAC_ETS_TC_SHAPING_25_REG              (PPE_SSU_BASE + 0xA27C)
#define PPE_SSU_SSU_MAC_ETS_TC_SHAPING_26_REG              (PPE_SSU_BASE + 0xA280)
#define PPE_SSU_SSU_MAC_ETS_TC_SHAPING_27_REG              (PPE_SSU_BASE + 0xA284)
#define PPE_SSU_SSU_MAC_ETS_TC_SHAPING_28_REG              (PPE_SSU_BASE + 0xA288)
#define PPE_SSU_SSU_MAC_ETS_TC_SHAPING_29_REG              (PPE_SSU_BASE + 0xA28C)
#define PPE_SSU_SSU_MAC_ETS_TC_SHAPING_30_REG              (PPE_SSU_BASE + 0xA290)
#define PPE_SSU_SSU_MAC_ETS_TC_SHAPING_31_REG              (PPE_SSU_BASE + 0xA294)

#define PPE_SSU_SSU_MAC_ETS_WEIGHT_OFFSET_REG              (PPE_SSU_BASE + 0xA318) /* ETS MAC侧 调度权重偏移值 */
#define PPE_SSU_SSU_MAC_ETS_PORT_MAPING_0_REG              (PPE_SSU_BASE + 0xA31C) /* ETS MAC侧 PORT Maping bitmap配置 */
#define PPE_SSU_SSU_MAC_ETS_PORT_MAPING_1_REG              (PPE_SSU_BASE + 0xA320) /* ETS MAC侧 PORT Maping bitmap配置 */
#define PPE_SSU_SSU_MAC_ETS_PORT_MAPING_2_REG              (PPE_SSU_BASE + 0xA324) /* ETS MAC侧 PORT Maping bitmap配置 */
#define PPE_SSU_SSU_MAC_ETS_PORT_MAPING_3_REG              (PPE_SSU_BASE + 0xA328) /* ETS MAC侧 PORT Maping bitmap配置 */
#define PPE_SSU_SSU_MAC_ETS_TCG_MAPING_0_REG               \
    (PPE_SSU_BASE + 0xA32C) /* ETS MAC侧 TC Group Maping bitmap配置 */
#define PPE_SSU_SSU_MAC_ETS_TCG_MAPING_1_REG               \
    (PPE_SSU_BASE + 0xA330) /* ETS MAC侧 TC Group Maping bitmap配置 */
#define PPE_SSU_SSU_MAC_ETS_TCG_MAPING_2_REG               \
    (PPE_SSU_BASE + 0xA334) /* ETS MAC侧 TC Group Maping bitmap配置 */
#define PPE_SSU_SSU_MAC_ETS_TCG_MAPING_3_REG               \
    (PPE_SSU_BASE + 0xA338) /* ETS MAC侧 TC Group Maping bitmap配置 */
#define PPE_SSU_SSU_MAC_ETS_TC_MAPING_0_REG                (PPE_SSU_BASE + 0xA33C) /* ETS MAC侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_MAC_ETS_TC_MAPING_1_REG                (PPE_SSU_BASE + 0xA340) /* ETS MAC侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_MAC_ETS_TC_MAPING_2_REG                (PPE_SSU_BASE + 0xA344) /* ETS MAC侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_MAC_ETS_TC_MAPING_3_REG                (PPE_SSU_BASE + 0xA348) /* ETS MAC侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_MAC_ETS_TC_MAPING_4_REG                (PPE_SSU_BASE + 0xA34C) /* ETS MAC侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_MAC_ETS_TC_MAPING_5_REG                (PPE_SSU_BASE + 0xA350) /* ETS MAC侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_MAC_ETS_TC_MAPING_6_REG                (PPE_SSU_BASE + 0xA354) /* ETS MAC侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_MAC_ETS_TC_MAPING_7_REG                (PPE_SSU_BASE + 0xA358) /* ETS MAC侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_MAC_ETS_TC_MAPING_8_REG                (PPE_SSU_BASE + 0xA35C) /* ETS MAC侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_MAC_ETS_TC_MAPING_9_REG                (PPE_SSU_BASE + 0xA360) /* ETS MAC侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_MAC_ETS_TC_MAPING_10_REG               (PPE_SSU_BASE + 0xA364) /* ETS MAC侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_MAC_ETS_TC_MAPING_11_REG               (PPE_SSU_BASE + 0xA368) /* ETS MAC侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_MAC_ETS_TC_MAPING_12_REG               (PPE_SSU_BASE + 0xA36C) /* ETS MAC侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_MAC_ETS_TC_MAPING_13_REG               (PPE_SSU_BASE + 0xA370) /* ETS MAC侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_MAC_ETS_TC_MAPING_14_REG               (PPE_SSU_BASE + 0xA374) /* ETS MAC侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_MAC_ETS_TC_MAPING_15_REG               (PPE_SSU_BASE + 0xA378) /* ETS MAC侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_MAC_ETS_TC_MAPING_16_REG               (PPE_SSU_BASE + 0xA37C) /* ETS MAC侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_MAC_ETS_TC_MAPING_17_REG               (PPE_SSU_BASE + 0xA380) /* ETS MAC侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_MAC_ETS_TC_MAPING_18_REG               (PPE_SSU_BASE + 0xA384) /* ETS MAC侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_MAC_ETS_TC_MAPING_19_REG               (PPE_SSU_BASE + 0xA388) /* ETS MAC侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_MAC_ETS_TC_MAPING_20_REG               (PPE_SSU_BASE + 0xA38C) /* ETS MAC侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_MAC_ETS_TC_MAPING_21_REG               (PPE_SSU_BASE + 0xA390) /* ETS MAC侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_MAC_ETS_TC_MAPING_22_REG               (PPE_SSU_BASE + 0xA394) /* ETS MAC侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_MAC_ETS_TC_MAPING_23_REG               (PPE_SSU_BASE + 0xA398) /* ETS MAC侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_MAC_ETS_TC_MAPING_24_REG               (PPE_SSU_BASE + 0xA39C) /* ETS MAC侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_MAC_ETS_TC_MAPING_25_REG               (PPE_SSU_BASE + 0xA3A0) /* ETS MAC侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_MAC_ETS_TC_MAPING_26_REG               (PPE_SSU_BASE + 0xA3A4) /* ETS MAC侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_MAC_ETS_TC_MAPING_27_REG               (PPE_SSU_BASE + 0xA3A8) /* ETS MAC侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_MAC_ETS_TC_MAPING_28_REG               (PPE_SSU_BASE + 0xA3AC) /* ETS MAC侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_MAC_ETS_TC_MAPING_29_REG               (PPE_SSU_BASE + 0xA3B0) /* ETS MAC侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_MAC_ETS_TC_MAPING_30_REG               (PPE_SSU_BASE + 0xA3B4) /* ETS MAC侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_MAC_ETS_TC_MAPING_31_REG               (PPE_SSU_BASE + 0xA3B8) /* ETS MAC侧 TC Maping bitmap配置 */

/* ETS MAC侧 PORT node 级的shaper配置参数 */
#define PPE_SSU_SSU_MAC_ETS_PORT_SHAPING_0_REG             (PPE_SSU_BASE + 0xA43C)
#define PPE_SSU_SSU_MAC_ETS_PORT_SHAPING_1_REG             (PPE_SSU_BASE + 0xA440)
#define PPE_SSU_SSU_MAC_ETS_PORT_SHAPING_2_REG             (PPE_SSU_BASE + 0xA444)
#define PPE_SSU_SSU_MAC_ETS_PORT_SHAPING_3_REG             (PPE_SSU_BASE + 0xA448)

/* ETS MAC侧 queue node 级的shaper配置参数 */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_SHAPING_0_REG            (PPE_SSU_BASE + 0xA44C)
#define PPE_SSU_SSU_MAC_ETS_QUEUE_SHAPING_1_REG            (PPE_SSU_BASE + 0xA450)
#define PPE_SSU_SSU_MAC_ETS_QUEUE_SHAPING_2_REG            (PPE_SSU_BASE + 0xA454)
#define PPE_SSU_SSU_MAC_ETS_QUEUE_SHAPING_3_REG            (PPE_SSU_BASE + 0xA458)
#define PPE_SSU_SSU_MAC_ETS_QUEUE_SHAPING_4_REG            (PPE_SSU_BASE + 0xA45C)
#define PPE_SSU_SSU_MAC_ETS_QUEUE_SHAPING_5_REG            (PPE_SSU_BASE + 0xA460)
#define PPE_SSU_SSU_MAC_ETS_QUEUE_SHAPING_6_REG            (PPE_SSU_BASE + 0xA464)
#define PPE_SSU_SSU_MAC_ETS_QUEUE_SHAPING_7_REG            (PPE_SSU_BASE + 0xA468)
#define PPE_SSU_SSU_MAC_ETS_QUEUE_SHAPING_8_REG            (PPE_SSU_BASE + 0xA46C)
#define PPE_SSU_SSU_MAC_ETS_QUEUE_SHAPING_9_REG            (PPE_SSU_BASE + 0xA470)
#define PPE_SSU_SSU_MAC_ETS_QUEUE_SHAPING_10_REG           (PPE_SSU_BASE + 0xA474)
#define PPE_SSU_SSU_MAC_ETS_QUEUE_SHAPING_11_REG           (PPE_SSU_BASE + 0xA478)
#define PPE_SSU_SSU_MAC_ETS_QUEUE_SHAPING_12_REG           (PPE_SSU_BASE + 0xA47C)
#define PPE_SSU_SSU_MAC_ETS_QUEUE_SHAPING_13_REG           (PPE_SSU_BASE + 0xA480)
#define PPE_SSU_SSU_MAC_ETS_QUEUE_SHAPING_14_REG           (PPE_SSU_BASE + 0xA484)
#define PPE_SSU_SSU_MAC_ETS_QUEUE_SHAPING_15_REG           (PPE_SSU_BASE + 0xA488)
#define PPE_SSU_SSU_MAC_ETS_QUEUE_SHAPING_16_REG           (PPE_SSU_BASE + 0xA48C)
#define PPE_SSU_SSU_MAC_ETS_QUEUE_SHAPING_17_REG           (PPE_SSU_BASE + 0xA490)
#define PPE_SSU_SSU_MAC_ETS_QUEUE_SHAPING_18_REG           (PPE_SSU_BASE + 0xA494)
#define PPE_SSU_SSU_MAC_ETS_QUEUE_SHAPING_19_REG           (PPE_SSU_BASE + 0xA498)
#define PPE_SSU_SSU_MAC_ETS_QUEUE_SHAPING_20_REG           (PPE_SSU_BASE + 0xA49C)
#define PPE_SSU_SSU_MAC_ETS_QUEUE_SHAPING_21_REG           (PPE_SSU_BASE + 0xA4A0)
#define PPE_SSU_SSU_MAC_ETS_QUEUE_SHAPING_22_REG           (PPE_SSU_BASE + 0xA4A4)
#define PPE_SSU_SSU_MAC_ETS_QUEUE_SHAPING_23_REG           (PPE_SSU_BASE + 0xA4A8)
#define PPE_SSU_SSU_MAC_ETS_QUEUE_SHAPING_24_REG           (PPE_SSU_BASE + 0xA4AC)
#define PPE_SSU_SSU_MAC_ETS_QUEUE_SHAPING_25_REG           (PPE_SSU_BASE + 0xA4B0)
#define PPE_SSU_SSU_MAC_ETS_QUEUE_SHAPING_26_REG           (PPE_SSU_BASE + 0xA4B4)
#define PPE_SSU_SSU_MAC_ETS_QUEUE_SHAPING_27_REG           (PPE_SSU_BASE + 0xA4B8)
#define PPE_SSU_SSU_MAC_ETS_QUEUE_SHAPING_28_REG           (PPE_SSU_BASE + 0xA4BC)
#define PPE_SSU_SSU_MAC_ETS_QUEUE_SHAPING_29_REG           (PPE_SSU_BASE + 0xA4C0)
#define PPE_SSU_SSU_MAC_ETS_QUEUE_SHAPING_30_REG           (PPE_SSU_BASE + 0xA4C4)
#define PPE_SSU_SSU_MAC_ETS_QUEUE_SHAPING_31_REG           (PPE_SSU_BASE + 0xA4C8)

#define PPE_SSU_SSU_MAC_ETS_PRE_SUB_OFFSET_LEN_CFG_0_REG   (PPE_SSU_BASE + 0xA54C) /* ETS MAC侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_MAC_ETS_PRE_SUB_OFFSET_LEN_CFG_1_REG   (PPE_SSU_BASE + 0xA550) /* ETS MAC侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_MAC_ETS_PRE_SUB_OFFSET_LEN_CFG_2_REG   (PPE_SSU_BASE + 0xA554) /* ETS MAC侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_MAC_ETS_PRE_SUB_OFFSET_LEN_CFG_3_REG   (PPE_SSU_BASE + 0xA558) /* ETS MAC侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_MAC_ETS_PRE_SUB_OFFSET_LEN_CFG_4_REG   (PPE_SSU_BASE + 0xA55C) /* ETS MAC侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_MAC_ETS_PRE_SUB_OFFSET_LEN_CFG_5_REG   (PPE_SSU_BASE + 0xA560) /* ETS MAC侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_MAC_ETS_PRE_SUB_OFFSET_LEN_CFG_6_REG   (PPE_SSU_BASE + 0xA564) /* ETS MAC侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_MAC_ETS_PRE_SUB_OFFSET_LEN_CFG_7_REG   (PPE_SSU_BASE + 0xA568) /* ETS MAC侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_MAC_ETS_PRE_SUB_OFFSET_LEN_CFG_8_REG   (PPE_SSU_BASE + 0xA56C) /* ETS MAC侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_MAC_ETS_PRE_SUB_OFFSET_LEN_CFG_9_REG   (PPE_SSU_BASE + 0xA570) /* ETS MAC侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_MAC_ETS_PRE_SUB_OFFSET_LEN_CFG_10_REG  (PPE_SSU_BASE + 0xA574) /* ETS MAC侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_MAC_ETS_PRE_SUB_OFFSET_LEN_CFG_11_REG  (PPE_SSU_BASE + 0xA578) /* ETS MAC侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_MAC_ETS_PRE_SUB_OFFSET_LEN_CFG_12_REG  (PPE_SSU_BASE + 0xA57C) /* ETS MAC侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_MAC_ETS_PRE_SUB_OFFSET_LEN_CFG_13_REG  (PPE_SSU_BASE + 0xA580) /* ETS MAC侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_MAC_ETS_PRE_SUB_OFFSET_LEN_CFG_14_REG  (PPE_SSU_BASE + 0xA584) /* ETS MAC侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_MAC_ETS_PRE_SUB_OFFSET_LEN_CFG_15_REG  (PPE_SSU_BASE + 0xA588) /* ETS MAC侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_MAC_ETS_PRE_SUB_OFFSET_LEN_CFG_16_REG  (PPE_SSU_BASE + 0xA58C) /* ETS MAC侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_MAC_ETS_PRE_SUB_OFFSET_LEN_CFG_17_REG  (PPE_SSU_BASE + 0xA590) /* ETS MAC侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_MAC_ETS_PRE_SUB_OFFSET_LEN_CFG_18_REG  (PPE_SSU_BASE + 0xA594) /* ETS MAC侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_MAC_ETS_PRE_SUB_OFFSET_LEN_CFG_19_REG  (PPE_SSU_BASE + 0xA598) /* ETS MAC侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_MAC_ETS_PRE_SUB_OFFSET_LEN_CFG_20_REG  (PPE_SSU_BASE + 0xA59C) /* ETS MAC侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_MAC_ETS_PRE_SUB_OFFSET_LEN_CFG_21_REG  (PPE_SSU_BASE + 0xA5A0) /* ETS MAC侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_MAC_ETS_PRE_SUB_OFFSET_LEN_CFG_22_REG  (PPE_SSU_BASE + 0xA5A4) /* ETS MAC侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_MAC_ETS_PRE_SUB_OFFSET_LEN_CFG_23_REG  (PPE_SSU_BASE + 0xA5A8) /* ETS MAC侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_MAC_ETS_PRE_SUB_OFFSET_LEN_CFG_24_REG  (PPE_SSU_BASE + 0xA5AC) /* ETS MAC侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_MAC_ETS_PRE_SUB_OFFSET_LEN_CFG_25_REG  (PPE_SSU_BASE + 0xA5B0) /* ETS MAC侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_MAC_ETS_PRE_SUB_OFFSET_LEN_CFG_26_REG  (PPE_SSU_BASE + 0xA5B4) /* ETS MAC侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_MAC_ETS_PRE_SUB_OFFSET_LEN_CFG_27_REG  (PPE_SSU_BASE + 0xA5B8) /* ETS MAC侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_MAC_ETS_PRE_SUB_OFFSET_LEN_CFG_28_REG  (PPE_SSU_BASE + 0xA5BC) /* ETS MAC侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_MAC_ETS_PRE_SUB_OFFSET_LEN_CFG_29_REG  (PPE_SSU_BASE + 0xA5C0) /* ETS MAC侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_MAC_ETS_PRE_SUB_OFFSET_LEN_CFG_30_REG  (PPE_SSU_BASE + 0xA5C4) /* ETS MAC侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_MAC_ETS_PRE_SUB_OFFSET_LEN_CFG_31_REG  (PPE_SSU_BASE + 0xA5C8) /* ETS MAC侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_MAC_ETS_SHAPPING_BYPASS_REG            (PPE_SSU_BASE + 0xA64C) /* ETS MAC侧 SHAPING bypass配置寄存器 */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_LINK_TC_CFG_0_REG        (PPE_SSU_BASE + 0xA650) /* ETS MAC侧 queue node对应的TC id */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_LINK_TC_CFG_1_REG        (PPE_SSU_BASE + 0xA654) /* ETS MAC侧 queue node对应的TC id */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_LINK_TC_CFG_2_REG        (PPE_SSU_BASE + 0xA658) /* ETS MAC侧 queue node对应的TC id */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_LINK_TC_CFG_3_REG        (PPE_SSU_BASE + 0xA65C) /* ETS MAC侧 queue node对应的TC id */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_LINK_TC_CFG_4_REG        (PPE_SSU_BASE + 0xA660) /* ETS MAC侧 queue node对应的TC id */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_LINK_TC_CFG_5_REG        (PPE_SSU_BASE + 0xA664) /* ETS MAC侧 queue node对应的TC id */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_LINK_TC_CFG_6_REG        (PPE_SSU_BASE + 0xA668) /* ETS MAC侧 queue node对应的TC id */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_LINK_TC_CFG_7_REG        (PPE_SSU_BASE + 0xA66C) /* ETS MAC侧 queue node对应的TC id */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_LINK_TC_CFG_8_REG        (PPE_SSU_BASE + 0xA670) /* ETS MAC侧 queue node对应的TC id */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_LINK_TC_CFG_9_REG        (PPE_SSU_BASE + 0xA674) /* ETS MAC侧 queue node对应的TC id */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_LINK_TC_CFG_10_REG       (PPE_SSU_BASE + 0xA678) /* ETS MAC侧 queue node对应的TC id */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_LINK_TC_CFG_11_REG       (PPE_SSU_BASE + 0xA67C) /* ETS MAC侧 queue node对应的TC id */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_LINK_TC_CFG_12_REG       (PPE_SSU_BASE + 0xA680) /* ETS MAC侧 queue node对应的TC id */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_LINK_TC_CFG_13_REG       (PPE_SSU_BASE + 0xA684) /* ETS MAC侧 queue node对应的TC id */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_LINK_TC_CFG_14_REG       (PPE_SSU_BASE + 0xA688) /* ETS MAC侧 queue node对应的TC id */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_LINK_TC_CFG_15_REG       (PPE_SSU_BASE + 0xA68C) /* ETS MAC侧 queue node对应的TC id */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_LINK_TC_CFG_16_REG       (PPE_SSU_BASE + 0xA690) /* ETS MAC侧 queue node对应的TC id */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_LINK_TC_CFG_17_REG       (PPE_SSU_BASE + 0xA694) /* ETS MAC侧 queue node对应的TC id */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_LINK_TC_CFG_18_REG       (PPE_SSU_BASE + 0xA698) /* ETS MAC侧 queue node对应的TC id */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_LINK_TC_CFG_19_REG       (PPE_SSU_BASE + 0xA69C) /* ETS MAC侧 queue node对应的TC id */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_LINK_TC_CFG_20_REG       (PPE_SSU_BASE + 0xA6A0) /* ETS MAC侧 queue node对应的TC id */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_LINK_TC_CFG_21_REG       (PPE_SSU_BASE + 0xA6A4) /* ETS MAC侧 queue node对应的TC id */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_LINK_TC_CFG_22_REG       (PPE_SSU_BASE + 0xA6A8) /* ETS MAC侧 queue node对应的TC id */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_LINK_TC_CFG_23_REG       (PPE_SSU_BASE + 0xA6AC) /* ETS MAC侧 queue node对应的TC id */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_LINK_TC_CFG_24_REG       (PPE_SSU_BASE + 0xA6B0) /* ETS MAC侧 queue node对应的TC id */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_LINK_TC_CFG_25_REG       (PPE_SSU_BASE + 0xA6B4) /* ETS MAC侧 queue node对应的TC id */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_LINK_TC_CFG_26_REG       (PPE_SSU_BASE + 0xA6B8) /* ETS MAC侧 queue node对应的TC id */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_LINK_TC_CFG_27_REG       (PPE_SSU_BASE + 0xA6BC) /* ETS MAC侧 queue node对应的TC id */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_LINK_TC_CFG_28_REG       (PPE_SSU_BASE + 0xA6C0) /* ETS MAC侧 queue node对应的TC id */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_LINK_TC_CFG_29_REG       (PPE_SSU_BASE + 0xA6C4) /* ETS MAC侧 queue node对应的TC id */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_LINK_TC_CFG_30_REG       (PPE_SSU_BASE + 0xA6C8) /* ETS MAC侧 queue node对应的TC id */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_LINK_TC_CFG_31_REG       (PPE_SSU_BASE + 0xA6CC) /* ETS MAC侧 queue node对应的TC id */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_EN_CFG_0_REG             (PPE_SSU_BASE + 0xA750) /* ETS MAC侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_EN_CFG_1_REG             (PPE_SSU_BASE + 0xA754) /* ETS MAC侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_EN_CFG_2_REG             (PPE_SSU_BASE + 0xA758) /* ETS MAC侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_EN_CFG_3_REG             (PPE_SSU_BASE + 0xA75C) /* ETS MAC侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_EN_CFG_4_REG             (PPE_SSU_BASE + 0xA760) /* ETS MAC侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_EN_CFG_5_REG             (PPE_SSU_BASE + 0xA764) /* ETS MAC侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_EN_CFG_6_REG             (PPE_SSU_BASE + 0xA768) /* ETS MAC侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_EN_CFG_7_REG             (PPE_SSU_BASE + 0xA76C) /* ETS MAC侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_EN_CFG_8_REG             (PPE_SSU_BASE + 0xA770) /* ETS MAC侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_EN_CFG_9_REG             (PPE_SSU_BASE + 0xA774) /* ETS MAC侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_EN_CFG_10_REG            (PPE_SSU_BASE + 0xA778) /* ETS MAC侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_EN_CFG_11_REG            (PPE_SSU_BASE + 0xA77C) /* ETS MAC侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_EN_CFG_12_REG            (PPE_SSU_BASE + 0xA780) /* ETS MAC侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_EN_CFG_13_REG            (PPE_SSU_BASE + 0xA784) /* ETS MAC侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_EN_CFG_14_REG            (PPE_SSU_BASE + 0xA788) /* ETS MAC侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_EN_CFG_15_REG            (PPE_SSU_BASE + 0xA78C) /* ETS MAC侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_EN_CFG_16_REG            (PPE_SSU_BASE + 0xA790) /* ETS MAC侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_EN_CFG_17_REG            (PPE_SSU_BASE + 0xA794) /* ETS MAC侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_EN_CFG_18_REG            (PPE_SSU_BASE + 0xA798) /* ETS MAC侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_EN_CFG_19_REG            (PPE_SSU_BASE + 0xA79C) /* ETS MAC侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_EN_CFG_20_REG            (PPE_SSU_BASE + 0xA7A0) /* ETS MAC侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_EN_CFG_21_REG            (PPE_SSU_BASE + 0xA7A4) /* ETS MAC侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_EN_CFG_22_REG            (PPE_SSU_BASE + 0xA7A8) /* ETS MAC侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_EN_CFG_23_REG            (PPE_SSU_BASE + 0xA7AC) /* ETS MAC侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_EN_CFG_24_REG            (PPE_SSU_BASE + 0xA7B0) /* ETS MAC侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_EN_CFG_25_REG            (PPE_SSU_BASE + 0xA7B4) /* ETS MAC侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_EN_CFG_26_REG            (PPE_SSU_BASE + 0xA7B8) /* ETS MAC侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_EN_CFG_27_REG            (PPE_SSU_BASE + 0xA7BC) /* ETS MAC侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_EN_CFG_28_REG            (PPE_SSU_BASE + 0xA7C0) /* ETS MAC侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_EN_CFG_29_REG            (PPE_SSU_BASE + 0xA7C4) /* ETS MAC侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_EN_CFG_30_REG            (PPE_SSU_BASE + 0xA7C8) /* ETS MAC侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_EN_CFG_31_REG            (PPE_SSU_BASE + 0xA7CC) /* ETS MAC侧 queue node 级的使能配置 */

/* ETS MAC侧 TC级的CBS算法中发送斜率SENDSLOPE配置 */
#define PPE_SSU_SSU_MAC_ETS_TC_SENDSLOPE_0_REG             (PPE_SSU_BASE + 0xA850)
#define PPE_SSU_SSU_MAC_ETS_TC_SENDSLOPE_1_REG             (PPE_SSU_BASE + 0xA854)
#define PPE_SSU_SSU_MAC_ETS_TC_SENDSLOPE_2_REG             (PPE_SSU_BASE + 0xA858)
#define PPE_SSU_SSU_MAC_ETS_TC_SENDSLOPE_3_REG             (PPE_SSU_BASE + 0xA85C)
#define PPE_SSU_SSU_MAC_ETS_TC_SENDSLOPE_4_REG             (PPE_SSU_BASE + 0xA860)
#define PPE_SSU_SSU_MAC_ETS_TC_SENDSLOPE_5_REG             (PPE_SSU_BASE + 0xA864)
#define PPE_SSU_SSU_MAC_ETS_TC_SENDSLOPE_6_REG             (PPE_SSU_BASE + 0xA868)
#define PPE_SSU_SSU_MAC_ETS_TC_SENDSLOPE_7_REG             (PPE_SSU_BASE + 0xA86C)
#define PPE_SSU_SSU_MAC_ETS_TC_SENDSLOPE_8_REG             (PPE_SSU_BASE + 0xA870)
#define PPE_SSU_SSU_MAC_ETS_TC_SENDSLOPE_9_REG             (PPE_SSU_BASE + 0xA874)
#define PPE_SSU_SSU_MAC_ETS_TC_SENDSLOPE_10_REG            (PPE_SSU_BASE + 0xA878)
#define PPE_SSU_SSU_MAC_ETS_TC_SENDSLOPE_11_REG            (PPE_SSU_BASE + 0xA87C)
#define PPE_SSU_SSU_MAC_ETS_TC_SENDSLOPE_12_REG            (PPE_SSU_BASE + 0xA880)
#define PPE_SSU_SSU_MAC_ETS_TC_SENDSLOPE_13_REG            (PPE_SSU_BASE + 0xA884)
#define PPE_SSU_SSU_MAC_ETS_TC_SENDSLOPE_14_REG            (PPE_SSU_BASE + 0xA888)
#define PPE_SSU_SSU_MAC_ETS_TC_SENDSLOPE_15_REG            (PPE_SSU_BASE + 0xA88C)
#define PPE_SSU_SSU_MAC_ETS_TC_SENDSLOPE_16_REG            (PPE_SSU_BASE + 0xA890)
#define PPE_SSU_SSU_MAC_ETS_TC_SENDSLOPE_17_REG            (PPE_SSU_BASE + 0xA894)
#define PPE_SSU_SSU_MAC_ETS_TC_SENDSLOPE_18_REG            (PPE_SSU_BASE + 0xA898)
#define PPE_SSU_SSU_MAC_ETS_TC_SENDSLOPE_19_REG            (PPE_SSU_BASE + 0xA89C)
#define PPE_SSU_SSU_MAC_ETS_TC_SENDSLOPE_20_REG            (PPE_SSU_BASE + 0xA8A0)
#define PPE_SSU_SSU_MAC_ETS_TC_SENDSLOPE_21_REG            (PPE_SSU_BASE + 0xA8A4)
#define PPE_SSU_SSU_MAC_ETS_TC_SENDSLOPE_22_REG            (PPE_SSU_BASE + 0xA8A8)
#define PPE_SSU_SSU_MAC_ETS_TC_SENDSLOPE_23_REG            (PPE_SSU_BASE + 0xA8AC)
#define PPE_SSU_SSU_MAC_ETS_TC_SENDSLOPE_24_REG            (PPE_SSU_BASE + 0xA8B0)
#define PPE_SSU_SSU_MAC_ETS_TC_SENDSLOPE_25_REG            (PPE_SSU_BASE + 0xA8B4)
#define PPE_SSU_SSU_MAC_ETS_TC_SENDSLOPE_26_REG            (PPE_SSU_BASE + 0xA8B8)
#define PPE_SSU_SSU_MAC_ETS_TC_SENDSLOPE_27_REG            (PPE_SSU_BASE + 0xA8BC)
#define PPE_SSU_SSU_MAC_ETS_TC_SENDSLOPE_28_REG            (PPE_SSU_BASE + 0xA8C0)
#define PPE_SSU_SSU_MAC_ETS_TC_SENDSLOPE_29_REG            (PPE_SSU_BASE + 0xA8C4)
#define PPE_SSU_SSU_MAC_ETS_TC_SENDSLOPE_30_REG            (PPE_SSU_BASE + 0xA8C8)
#define PPE_SSU_SSU_MAC_ETS_TC_SENDSLOPE_31_REG            (PPE_SSU_BASE + 0xA8CC)

/* ETS MAC侧 TC级的CBS算法中累积斜率IDLESLOPE配置 */
#define PPE_SSU_SSU_MAC_ETS_TC_IDLESLOPE_0_REG             (PPE_SSU_BASE + 0xA950)
#define PPE_SSU_SSU_MAC_ETS_TC_IDLESLOPE_1_REG             (PPE_SSU_BASE + 0xA954)
#define PPE_SSU_SSU_MAC_ETS_TC_IDLESLOPE_2_REG             (PPE_SSU_BASE + 0xA958)
#define PPE_SSU_SSU_MAC_ETS_TC_IDLESLOPE_3_REG             (PPE_SSU_BASE + 0xA95C)
#define PPE_SSU_SSU_MAC_ETS_TC_IDLESLOPE_4_REG             (PPE_SSU_BASE + 0xA960)
#define PPE_SSU_SSU_MAC_ETS_TC_IDLESLOPE_5_REG             (PPE_SSU_BASE + 0xA964)
#define PPE_SSU_SSU_MAC_ETS_TC_IDLESLOPE_6_REG             (PPE_SSU_BASE + 0xA968)
#define PPE_SSU_SSU_MAC_ETS_TC_IDLESLOPE_7_REG             (PPE_SSU_BASE + 0xA96C)
#define PPE_SSU_SSU_MAC_ETS_TC_IDLESLOPE_8_REG             (PPE_SSU_BASE + 0xA970)
#define PPE_SSU_SSU_MAC_ETS_TC_IDLESLOPE_9_REG             (PPE_SSU_BASE + 0xA974)
#define PPE_SSU_SSU_MAC_ETS_TC_IDLESLOPE_10_REG            (PPE_SSU_BASE + 0xA978)
#define PPE_SSU_SSU_MAC_ETS_TC_IDLESLOPE_11_REG            (PPE_SSU_BASE + 0xA97C)
#define PPE_SSU_SSU_MAC_ETS_TC_IDLESLOPE_12_REG            (PPE_SSU_BASE + 0xA980)
#define PPE_SSU_SSU_MAC_ETS_TC_IDLESLOPE_13_REG            (PPE_SSU_BASE + 0xA984)
#define PPE_SSU_SSU_MAC_ETS_TC_IDLESLOPE_14_REG            (PPE_SSU_BASE + 0xA988)
#define PPE_SSU_SSU_MAC_ETS_TC_IDLESLOPE_15_REG            (PPE_SSU_BASE + 0xA98C)
#define PPE_SSU_SSU_MAC_ETS_TC_IDLESLOPE_16_REG            (PPE_SSU_BASE + 0xA990)
#define PPE_SSU_SSU_MAC_ETS_TC_IDLESLOPE_17_REG            (PPE_SSU_BASE + 0xA994)
#define PPE_SSU_SSU_MAC_ETS_TC_IDLESLOPE_18_REG            (PPE_SSU_BASE + 0xA998)
#define PPE_SSU_SSU_MAC_ETS_TC_IDLESLOPE_19_REG            (PPE_SSU_BASE + 0xA99C)
#define PPE_SSU_SSU_MAC_ETS_TC_IDLESLOPE_20_REG            (PPE_SSU_BASE + 0xA9A0)
#define PPE_SSU_SSU_MAC_ETS_TC_IDLESLOPE_21_REG            (PPE_SSU_BASE + 0xA9A4)
#define PPE_SSU_SSU_MAC_ETS_TC_IDLESLOPE_22_REG            (PPE_SSU_BASE + 0xA9A8)
#define PPE_SSU_SSU_MAC_ETS_TC_IDLESLOPE_23_REG            (PPE_SSU_BASE + 0xA9AC)
#define PPE_SSU_SSU_MAC_ETS_TC_IDLESLOPE_24_REG            (PPE_SSU_BASE + 0xA9B0)
#define PPE_SSU_SSU_MAC_ETS_TC_IDLESLOPE_25_REG            (PPE_SSU_BASE + 0xA9B4)
#define PPE_SSU_SSU_MAC_ETS_TC_IDLESLOPE_26_REG            (PPE_SSU_BASE + 0xA9B8)
#define PPE_SSU_SSU_MAC_ETS_TC_IDLESLOPE_27_REG            (PPE_SSU_BASE + 0xA9BC)
#define PPE_SSU_SSU_MAC_ETS_TC_IDLESLOPE_28_REG            (PPE_SSU_BASE + 0xA9C0)
#define PPE_SSU_SSU_MAC_ETS_TC_IDLESLOPE_29_REG            (PPE_SSU_BASE + 0xA9C4)
#define PPE_SSU_SSU_MAC_ETS_TC_IDLESLOPE_30_REG            (PPE_SSU_BASE + 0xA9C8)
#define PPE_SSU_SSU_MAC_ETS_TC_IDLESLOPE_31_REG            (PPE_SSU_BASE + 0xA9CC)

/* ETS MAC侧 TC级的CBS算法中高水线HICREDIT配置 */
#define PPE_SSU_SSU_MAC_ETS_TC_HICREDIT_0_REG              (PPE_SSU_BASE + 0xAA50)
#define PPE_SSU_SSU_MAC_ETS_TC_HICREDIT_1_REG              (PPE_SSU_BASE + 0xAA54)
#define PPE_SSU_SSU_MAC_ETS_TC_HICREDIT_2_REG              (PPE_SSU_BASE + 0xAA58)
#define PPE_SSU_SSU_MAC_ETS_TC_HICREDIT_3_REG              (PPE_SSU_BASE + 0xAA5C)
#define PPE_SSU_SSU_MAC_ETS_TC_HICREDIT_4_REG              (PPE_SSU_BASE + 0xAA60)
#define PPE_SSU_SSU_MAC_ETS_TC_HICREDIT_5_REG              (PPE_SSU_BASE + 0xAA64)
#define PPE_SSU_SSU_MAC_ETS_TC_HICREDIT_6_REG              (PPE_SSU_BASE + 0xAA68)
#define PPE_SSU_SSU_MAC_ETS_TC_HICREDIT_7_REG              (PPE_SSU_BASE + 0xAA6C)
#define PPE_SSU_SSU_MAC_ETS_TC_HICREDIT_8_REG              (PPE_SSU_BASE + 0xAA70)
#define PPE_SSU_SSU_MAC_ETS_TC_HICREDIT_9_REG              (PPE_SSU_BASE + 0xAA74)
#define PPE_SSU_SSU_MAC_ETS_TC_HICREDIT_10_REG             (PPE_SSU_BASE + 0xAA78)
#define PPE_SSU_SSU_MAC_ETS_TC_HICREDIT_11_REG             (PPE_SSU_BASE + 0xAA7C)
#define PPE_SSU_SSU_MAC_ETS_TC_HICREDIT_12_REG             (PPE_SSU_BASE + 0xAA80)
#define PPE_SSU_SSU_MAC_ETS_TC_HICREDIT_13_REG             (PPE_SSU_BASE + 0xAA84)
#define PPE_SSU_SSU_MAC_ETS_TC_HICREDIT_14_REG             (PPE_SSU_BASE + 0xAA88)
#define PPE_SSU_SSU_MAC_ETS_TC_HICREDIT_15_REG             (PPE_SSU_BASE + 0xAA8C)
#define PPE_SSU_SSU_MAC_ETS_TC_HICREDIT_16_REG             (PPE_SSU_BASE + 0xAA90)
#define PPE_SSU_SSU_MAC_ETS_TC_HICREDIT_17_REG             (PPE_SSU_BASE + 0xAA94)
#define PPE_SSU_SSU_MAC_ETS_TC_HICREDIT_18_REG             (PPE_SSU_BASE + 0xAA98)
#define PPE_SSU_SSU_MAC_ETS_TC_HICREDIT_19_REG             (PPE_SSU_BASE + 0xAA9C)
#define PPE_SSU_SSU_MAC_ETS_TC_HICREDIT_20_REG             (PPE_SSU_BASE + 0xAAA0)
#define PPE_SSU_SSU_MAC_ETS_TC_HICREDIT_21_REG             (PPE_SSU_BASE + 0xAAA4)
#define PPE_SSU_SSU_MAC_ETS_TC_HICREDIT_22_REG             (PPE_SSU_BASE + 0xAAA8)
#define PPE_SSU_SSU_MAC_ETS_TC_HICREDIT_23_REG             (PPE_SSU_BASE + 0xAAAC)
#define PPE_SSU_SSU_MAC_ETS_TC_HICREDIT_24_REG             (PPE_SSU_BASE + 0xAAB0)
#define PPE_SSU_SSU_MAC_ETS_TC_HICREDIT_25_REG             (PPE_SSU_BASE + 0xAAB4)
#define PPE_SSU_SSU_MAC_ETS_TC_HICREDIT_26_REG             (PPE_SSU_BASE + 0xAAB8)
#define PPE_SSU_SSU_MAC_ETS_TC_HICREDIT_27_REG             (PPE_SSU_BASE + 0xAABC)
#define PPE_SSU_SSU_MAC_ETS_TC_HICREDIT_28_REG             (PPE_SSU_BASE + 0xAAC0)
#define PPE_SSU_SSU_MAC_ETS_TC_HICREDIT_29_REG             (PPE_SSU_BASE + 0xAAC4)
#define PPE_SSU_SSU_MAC_ETS_TC_HICREDIT_30_REG             (PPE_SSU_BASE + 0xAAC8)
#define PPE_SSU_SSU_MAC_ETS_TC_HICREDIT_31_REG             (PPE_SSU_BASE + 0xAACC)

/* ETS MAC侧 TC级的CBS算法中低水线LOCREDIT配置 */
#define PPE_SSU_SSU_MAC_ETS_TC_LOCREDIT_0_REG              (PPE_SSU_BASE + 0xAB50)
#define PPE_SSU_SSU_MAC_ETS_TC_LOCREDIT_1_REG              (PPE_SSU_BASE + 0xAB54)
#define PPE_SSU_SSU_MAC_ETS_TC_LOCREDIT_2_REG              (PPE_SSU_BASE + 0xAB58)
#define PPE_SSU_SSU_MAC_ETS_TC_LOCREDIT_3_REG              (PPE_SSU_BASE + 0xAB5C)
#define PPE_SSU_SSU_MAC_ETS_TC_LOCREDIT_4_REG              (PPE_SSU_BASE + 0xAB60)
#define PPE_SSU_SSU_MAC_ETS_TC_LOCREDIT_5_REG              (PPE_SSU_BASE + 0xAB64)
#define PPE_SSU_SSU_MAC_ETS_TC_LOCREDIT_6_REG              (PPE_SSU_BASE + 0xAB68)
#define PPE_SSU_SSU_MAC_ETS_TC_LOCREDIT_7_REG              (PPE_SSU_BASE + 0xAB6C)
#define PPE_SSU_SSU_MAC_ETS_TC_LOCREDIT_8_REG              (PPE_SSU_BASE + 0xAB70)
#define PPE_SSU_SSU_MAC_ETS_TC_LOCREDIT_9_REG              (PPE_SSU_BASE + 0xAB74)
#define PPE_SSU_SSU_MAC_ETS_TC_LOCREDIT_10_REG             (PPE_SSU_BASE + 0xAB78)
#define PPE_SSU_SSU_MAC_ETS_TC_LOCREDIT_11_REG             (PPE_SSU_BASE + 0xAB7C)
#define PPE_SSU_SSU_MAC_ETS_TC_LOCREDIT_12_REG             (PPE_SSU_BASE + 0xAB80)
#define PPE_SSU_SSU_MAC_ETS_TC_LOCREDIT_13_REG             (PPE_SSU_BASE + 0xAB84)
#define PPE_SSU_SSU_MAC_ETS_TC_LOCREDIT_14_REG             (PPE_SSU_BASE + 0xAB88)
#define PPE_SSU_SSU_MAC_ETS_TC_LOCREDIT_15_REG             (PPE_SSU_BASE + 0xAB8C)
#define PPE_SSU_SSU_MAC_ETS_TC_LOCREDIT_16_REG             (PPE_SSU_BASE + 0xAB90)
#define PPE_SSU_SSU_MAC_ETS_TC_LOCREDIT_17_REG             (PPE_SSU_BASE + 0xAB94)
#define PPE_SSU_SSU_MAC_ETS_TC_LOCREDIT_18_REG             (PPE_SSU_BASE + 0xAB98)
#define PPE_SSU_SSU_MAC_ETS_TC_LOCREDIT_19_REG             (PPE_SSU_BASE + 0xAB9C)
#define PPE_SSU_SSU_MAC_ETS_TC_LOCREDIT_20_REG             (PPE_SSU_BASE + 0xABA0)
#define PPE_SSU_SSU_MAC_ETS_TC_LOCREDIT_21_REG             (PPE_SSU_BASE + 0xABA4)
#define PPE_SSU_SSU_MAC_ETS_TC_LOCREDIT_22_REG             (PPE_SSU_BASE + 0xABA8)
#define PPE_SSU_SSU_MAC_ETS_TC_LOCREDIT_23_REG             (PPE_SSU_BASE + 0xABAC)
#define PPE_SSU_SSU_MAC_ETS_TC_LOCREDIT_24_REG             (PPE_SSU_BASE + 0xABB0)
#define PPE_SSU_SSU_MAC_ETS_TC_LOCREDIT_25_REG             (PPE_SSU_BASE + 0xABB4)
#define PPE_SSU_SSU_MAC_ETS_TC_LOCREDIT_26_REG             (PPE_SSU_BASE + 0xABB8)
#define PPE_SSU_SSU_MAC_ETS_TC_LOCREDIT_27_REG             (PPE_SSU_BASE + 0xABBC)
#define PPE_SSU_SSU_MAC_ETS_TC_LOCREDIT_28_REG             (PPE_SSU_BASE + 0xABC0)
#define PPE_SSU_SSU_MAC_ETS_TC_LOCREDIT_29_REG             (PPE_SSU_BASE + 0xABC4)
#define PPE_SSU_SSU_MAC_ETS_TC_LOCREDIT_30_REG             (PPE_SSU_BASE + 0xABC8)
#define PPE_SSU_SSU_MAC_ETS_TC_LOCREDIT_31_REG             (PPE_SSU_BASE + 0xABCC)

/* ETS MAC侧 TC node 级的CBS使能配置 */
#define PPE_SSU_SSU_MAC_ETS_TC_CBS_EN_0_REG                (PPE_SSU_BASE + 0xAC50)
#define PPE_SSU_SSU_MAC_ETS_TC_CBS_EN_1_REG                (PPE_SSU_BASE + 0xAC54)
#define PPE_SSU_SSU_MAC_ETS_TC_CBS_EN_2_REG                (PPE_SSU_BASE + 0xAC58)
#define PPE_SSU_SSU_MAC_ETS_TC_CBS_EN_3_REG                (PPE_SSU_BASE + 0xAC5C)
#define PPE_SSU_SSU_MAC_ETS_TC_CBS_EN_4_REG                (PPE_SSU_BASE + 0xAC60)
#define PPE_SSU_SSU_MAC_ETS_TC_CBS_EN_5_REG                (PPE_SSU_BASE + 0xAC64)
#define PPE_SSU_SSU_MAC_ETS_TC_CBS_EN_6_REG                (PPE_SSU_BASE + 0xAC68)
#define PPE_SSU_SSU_MAC_ETS_TC_CBS_EN_7_REG                (PPE_SSU_BASE + 0xAC6C)
#define PPE_SSU_SSU_MAC_ETS_TC_CBS_EN_8_REG                (PPE_SSU_BASE + 0xAC70)
#define PPE_SSU_SSU_MAC_ETS_TC_CBS_EN_9_REG                (PPE_SSU_BASE + 0xAC74)
#define PPE_SSU_SSU_MAC_ETS_TC_CBS_EN_10_REG               (PPE_SSU_BASE + 0xAC78)
#define PPE_SSU_SSU_MAC_ETS_TC_CBS_EN_11_REG               (PPE_SSU_BASE + 0xAC7C)
#define PPE_SSU_SSU_MAC_ETS_TC_CBS_EN_12_REG               (PPE_SSU_BASE + 0xAC80)
#define PPE_SSU_SSU_MAC_ETS_TC_CBS_EN_13_REG               (PPE_SSU_BASE + 0xAC84)
#define PPE_SSU_SSU_MAC_ETS_TC_CBS_EN_14_REG               (PPE_SSU_BASE + 0xAC88)
#define PPE_SSU_SSU_MAC_ETS_TC_CBS_EN_15_REG               (PPE_SSU_BASE + 0xAC8C)
#define PPE_SSU_SSU_MAC_ETS_TC_CBS_EN_16_REG               (PPE_SSU_BASE + 0xAC90)
#define PPE_SSU_SSU_MAC_ETS_TC_CBS_EN_17_REG               (PPE_SSU_BASE + 0xAC94)
#define PPE_SSU_SSU_MAC_ETS_TC_CBS_EN_18_REG               (PPE_SSU_BASE + 0xAC98)
#define PPE_SSU_SSU_MAC_ETS_TC_CBS_EN_19_REG               (PPE_SSU_BASE + 0xAC9C)
#define PPE_SSU_SSU_MAC_ETS_TC_CBS_EN_20_REG               (PPE_SSU_BASE + 0xACA0)
#define PPE_SSU_SSU_MAC_ETS_TC_CBS_EN_21_REG               (PPE_SSU_BASE + 0xACA4)
#define PPE_SSU_SSU_MAC_ETS_TC_CBS_EN_22_REG               (PPE_SSU_BASE + 0xACA8)
#define PPE_SSU_SSU_MAC_ETS_TC_CBS_EN_23_REG               (PPE_SSU_BASE + 0xACAC)
#define PPE_SSU_SSU_MAC_ETS_TC_CBS_EN_24_REG               (PPE_SSU_BASE + 0xACB0)
#define PPE_SSU_SSU_MAC_ETS_TC_CBS_EN_25_REG               (PPE_SSU_BASE + 0xACB4)
#define PPE_SSU_SSU_MAC_ETS_TC_CBS_EN_26_REG               (PPE_SSU_BASE + 0xACB8)
#define PPE_SSU_SSU_MAC_ETS_TC_CBS_EN_27_REG               (PPE_SSU_BASE + 0xACBC)
#define PPE_SSU_SSU_MAC_ETS_TC_CBS_EN_28_REG               (PPE_SSU_BASE + 0xACC0)
#define PPE_SSU_SSU_MAC_ETS_TC_CBS_EN_29_REG               (PPE_SSU_BASE + 0xACC4)
#define PPE_SSU_SSU_MAC_ETS_TC_CBS_EN_30_REG               (PPE_SSU_BASE + 0xACC8)
#define PPE_SSU_SSU_MAC_ETS_TC_CBS_EN_31_REG               (PPE_SSU_BASE + 0xACCC)

/* ETS MAC侧 TC node 级的对应MAC端口速率配置 */
#define PPE_SSU_SSU_MAC_ETS_TC_SPEED_SEL_0_REG             (PPE_SSU_BASE + 0xAD50)
#define PPE_SSU_SSU_MAC_ETS_TC_SPEED_SEL_1_REG             (PPE_SSU_BASE + 0xAD54)
#define PPE_SSU_SSU_MAC_ETS_TC_SPEED_SEL_2_REG             (PPE_SSU_BASE + 0xAD58)
#define PPE_SSU_SSU_MAC_ETS_TC_SPEED_SEL_3_REG             (PPE_SSU_BASE + 0xAD5C)
#define PPE_SSU_SSU_MAC_ETS_TC_SPEED_SEL_4_REG             (PPE_SSU_BASE + 0xAD60)
#define PPE_SSU_SSU_MAC_ETS_TC_SPEED_SEL_5_REG             (PPE_SSU_BASE + 0xAD64)
#define PPE_SSU_SSU_MAC_ETS_TC_SPEED_SEL_6_REG             (PPE_SSU_BASE + 0xAD68)
#define PPE_SSU_SSU_MAC_ETS_TC_SPEED_SEL_7_REG             (PPE_SSU_BASE + 0xAD6C)
#define PPE_SSU_SSU_MAC_ETS_TC_SPEED_SEL_8_REG             (PPE_SSU_BASE + 0xAD70)
#define PPE_SSU_SSU_MAC_ETS_TC_SPEED_SEL_9_REG             (PPE_SSU_BASE + 0xAD74)
#define PPE_SSU_SSU_MAC_ETS_TC_SPEED_SEL_10_REG            (PPE_SSU_BASE + 0xAD78)
#define PPE_SSU_SSU_MAC_ETS_TC_SPEED_SEL_11_REG            (PPE_SSU_BASE + 0xAD7C)
#define PPE_SSU_SSU_MAC_ETS_TC_SPEED_SEL_12_REG            (PPE_SSU_BASE + 0xAD80)
#define PPE_SSU_SSU_MAC_ETS_TC_SPEED_SEL_13_REG            (PPE_SSU_BASE + 0xAD84)
#define PPE_SSU_SSU_MAC_ETS_TC_SPEED_SEL_14_REG            (PPE_SSU_BASE + 0xAD88)
#define PPE_SSU_SSU_MAC_ETS_TC_SPEED_SEL_15_REG            (PPE_SSU_BASE + 0xAD8C)
#define PPE_SSU_SSU_MAC_ETS_TC_SPEED_SEL_16_REG            (PPE_SSU_BASE + 0xAD90)
#define PPE_SSU_SSU_MAC_ETS_TC_SPEED_SEL_17_REG            (PPE_SSU_BASE + 0xAD94)
#define PPE_SSU_SSU_MAC_ETS_TC_SPEED_SEL_18_REG            (PPE_SSU_BASE + 0xAD98)
#define PPE_SSU_SSU_MAC_ETS_TC_SPEED_SEL_19_REG            (PPE_SSU_BASE + 0xAD9C)
#define PPE_SSU_SSU_MAC_ETS_TC_SPEED_SEL_20_REG            (PPE_SSU_BASE + 0xADA0)
#define PPE_SSU_SSU_MAC_ETS_TC_SPEED_SEL_21_REG            (PPE_SSU_BASE + 0xADA4)
#define PPE_SSU_SSU_MAC_ETS_TC_SPEED_SEL_22_REG            (PPE_SSU_BASE + 0xADA8)
#define PPE_SSU_SSU_MAC_ETS_TC_SPEED_SEL_23_REG            (PPE_SSU_BASE + 0xADAC)
#define PPE_SSU_SSU_MAC_ETS_TC_SPEED_SEL_24_REG            (PPE_SSU_BASE + 0xADB0)
#define PPE_SSU_SSU_MAC_ETS_TC_SPEED_SEL_25_REG            (PPE_SSU_BASE + 0xADB4)
#define PPE_SSU_SSU_MAC_ETS_TC_SPEED_SEL_26_REG            (PPE_SSU_BASE + 0xADB8)
#define PPE_SSU_SSU_MAC_ETS_TC_SPEED_SEL_27_REG            (PPE_SSU_BASE + 0xADBC)
#define PPE_SSU_SSU_MAC_ETS_TC_SPEED_SEL_28_REG            (PPE_SSU_BASE + 0xADC0)
#define PPE_SSU_SSU_MAC_ETS_TC_SPEED_SEL_29_REG            (PPE_SSU_BASE + 0xADC4)
#define PPE_SSU_SSU_MAC_ETS_TC_SPEED_SEL_30_REG            (PPE_SSU_BASE + 0xADC8)
#define PPE_SSU_SSU_MAC_ETS_TC_SPEED_SEL_31_REG            (PPE_SSU_BASE + 0xADCC)

/* ETS MAC侧 queue node 级的队列FLUSH的请求信号 */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_FLUSH_REQ_REG            (PPE_SSU_BASE + 0xAE50)
/* ETS MAC侧 queue node 级的队列FLUSH的完成信号 */
#define PPE_SSU_SSU_MAC_ETS_QUEUE_FLUSH_DONE_REG           (PPE_SSU_BASE + 0xAE54)

#define PPE_SSU_SSU_HOST_ETS_TC_WEIGHT_0_REG               (PPE_SSU_BASE + 0xB000) /* ETS HOST侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_HOST_ETS_TC_WEIGHT_1_REG               (PPE_SSU_BASE + 0xB004) /* ETS HOST侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_HOST_ETS_TC_WEIGHT_2_REG               (PPE_SSU_BASE + 0xB008) /* ETS HOST侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_HOST_ETS_TC_WEIGHT_3_REG               (PPE_SSU_BASE + 0xB00C) /* ETS HOST侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_HOST_ETS_TC_WEIGHT_4_REG               (PPE_SSU_BASE + 0xB010) /* ETS HOST侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_HOST_ETS_TC_WEIGHT_5_REG               (PPE_SSU_BASE + 0xB014) /* ETS HOST侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_HOST_ETS_TC_WEIGHT_6_REG               (PPE_SSU_BASE + 0xB018) /* ETS HOST侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_HOST_ETS_TC_WEIGHT_7_REG               (PPE_SSU_BASE + 0xB01C) /* ETS HOST侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_HOST_ETS_TC_WEIGHT_8_REG               (PPE_SSU_BASE + 0xB020) /* ETS HOST侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_HOST_ETS_TC_WEIGHT_9_REG               (PPE_SSU_BASE + 0xB024) /* ETS HOST侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_HOST_ETS_TC_WEIGHT_10_REG              (PPE_SSU_BASE + 0xB028) /* ETS HOST侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_HOST_ETS_TC_WEIGHT_11_REG              (PPE_SSU_BASE + 0xB02C) /* ETS HOST侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_HOST_ETS_TC_WEIGHT_12_REG              (PPE_SSU_BASE + 0xB030) /* ETS HOST侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_HOST_ETS_TC_WEIGHT_13_REG              (PPE_SSU_BASE + 0xB034) /* ETS HOST侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_HOST_ETS_TC_WEIGHT_14_REG              (PPE_SSU_BASE + 0xB038) /* ETS HOST侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_HOST_ETS_TC_WEIGHT_15_REG              (PPE_SSU_BASE + 0xB03C) /* ETS HOST侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_HOST_ETS_TC_WEIGHT_16_REG              (PPE_SSU_BASE + 0xB040) /* ETS HOST侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_HOST_ETS_TC_WEIGHT_17_REG              (PPE_SSU_BASE + 0xB044) /* ETS HOST侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_HOST_ETS_TC_WEIGHT_18_REG              (PPE_SSU_BASE + 0xB048) /* ETS HOST侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_HOST_ETS_TC_WEIGHT_19_REG              (PPE_SSU_BASE + 0xB04C) /* ETS HOST侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_HOST_ETS_TC_WEIGHT_20_REG              (PPE_SSU_BASE + 0xB050) /* ETS HOST侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_HOST_ETS_TC_WEIGHT_21_REG              (PPE_SSU_BASE + 0xB054) /* ETS HOST侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_HOST_ETS_TC_WEIGHT_22_REG              (PPE_SSU_BASE + 0xB058) /* ETS HOST侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_HOST_ETS_TC_WEIGHT_23_REG              (PPE_SSU_BASE + 0xB05C) /* ETS HOST侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_HOST_ETS_TC_WEIGHT_24_REG              (PPE_SSU_BASE + 0xB060) /* ETS HOST侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_HOST_ETS_TC_WEIGHT_25_REG              (PPE_SSU_BASE + 0xB064) /* ETS HOST侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_HOST_ETS_TC_WEIGHT_26_REG              (PPE_SSU_BASE + 0xB068) /* ETS HOST侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_HOST_ETS_TC_WEIGHT_27_REG              (PPE_SSU_BASE + 0xB06C) /* ETS HOST侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_HOST_ETS_TC_WEIGHT_28_REG              (PPE_SSU_BASE + 0xB070) /* ETS HOST侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_HOST_ETS_TC_WEIGHT_29_REG              (PPE_SSU_BASE + 0xB074) /* ETS HOST侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_HOST_ETS_TC_WEIGHT_30_REG              (PPE_SSU_BASE + 0xB078) /* ETS HOST侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_HOST_ETS_TC_WEIGHT_31_REG              (PPE_SSU_BASE + 0xB07C) /* ETS HOST侧 TC nodes weight配置表 */
#define PPE_SSU_SSU_HOST_ETS_TC_SP_MODE_REG                (PPE_SSU_BASE + 0xB100) /* ETS HOST侧 TC nodes sp属性配置 */

/* ETS HOST侧 TC Group nodes weight配置表 */
#define PPE_SSU_SSU_HOST_ETS_TCG_WEIGHT_0_REG              (PPE_SSU_BASE + 0xB104)
#define PPE_SSU_SSU_HOST_ETS_TCG_WEIGHT_1_REG              (PPE_SSU_BASE + 0xB108)
#define PPE_SSU_SSU_HOST_ETS_TCG_WEIGHT_2_REG              (PPE_SSU_BASE + 0xB10C)
#define PPE_SSU_SSU_HOST_ETS_TCG_WEIGHT_3_REG              (PPE_SSU_BASE + 0xB110)

#define PPE_SSU_SSU_HOST_ETS_TCG_SP_MODE_REG               (PPE_SSU_BASE + 0xB204) /* ETS HOST侧 TCG nodes sp属性配置 */

/* ETS HOST侧 TC Group nodes 级的shaper配置参数 */
#define PPE_SSU_SSU_HOST_ETS_TCG_SHAPING_0_REG             (PPE_SSU_BASE + 0xB208)
#define PPE_SSU_SSU_HOST_ETS_TCG_SHAPING_1_REG             (PPE_SSU_BASE + 0xB20C)
#define PPE_SSU_SSU_HOST_ETS_TCG_SHAPING_2_REG             (PPE_SSU_BASE + 0xB210)
#define PPE_SSU_SSU_HOST_ETS_TCG_SHAPING_3_REG             (PPE_SSU_BASE + 0xB214)

/* ETS HOST侧 TC nodes 级的shaper配置参数 */
#define PPE_SSU_SSU_HOST_ETS_TC_SHAPING_0_REG              (PPE_SSU_BASE + 0xB218)
#define PPE_SSU_SSU_HOST_ETS_TC_SHAPING_1_REG              (PPE_SSU_BASE + 0xB21C)
#define PPE_SSU_SSU_HOST_ETS_TC_SHAPING_2_REG              (PPE_SSU_BASE + 0xB220)
#define PPE_SSU_SSU_HOST_ETS_TC_SHAPING_3_REG              (PPE_SSU_BASE + 0xB224)
#define PPE_SSU_SSU_HOST_ETS_TC_SHAPING_4_REG              (PPE_SSU_BASE + 0xB228)
#define PPE_SSU_SSU_HOST_ETS_TC_SHAPING_5_REG              (PPE_SSU_BASE + 0xB22C)
#define PPE_SSU_SSU_HOST_ETS_TC_SHAPING_6_REG              (PPE_SSU_BASE + 0xB230)
#define PPE_SSU_SSU_HOST_ETS_TC_SHAPING_7_REG              (PPE_SSU_BASE + 0xB234)
#define PPE_SSU_SSU_HOST_ETS_TC_SHAPING_8_REG              (PPE_SSU_BASE + 0xB238)
#define PPE_SSU_SSU_HOST_ETS_TC_SHAPING_9_REG              (PPE_SSU_BASE + 0xB23C)
#define PPE_SSU_SSU_HOST_ETS_TC_SHAPING_10_REG             (PPE_SSU_BASE + 0xB240)
#define PPE_SSU_SSU_HOST_ETS_TC_SHAPING_11_REG             (PPE_SSU_BASE + 0xB244)
#define PPE_SSU_SSU_HOST_ETS_TC_SHAPING_12_REG             (PPE_SSU_BASE + 0xB248)
#define PPE_SSU_SSU_HOST_ETS_TC_SHAPING_13_REG             (PPE_SSU_BASE + 0xB24C)
#define PPE_SSU_SSU_HOST_ETS_TC_SHAPING_14_REG             (PPE_SSU_BASE + 0xB250)
#define PPE_SSU_SSU_HOST_ETS_TC_SHAPING_15_REG             (PPE_SSU_BASE + 0xB254)
#define PPE_SSU_SSU_HOST_ETS_TC_SHAPING_16_REG             (PPE_SSU_BASE + 0xB258)
#define PPE_SSU_SSU_HOST_ETS_TC_SHAPING_17_REG             (PPE_SSU_BASE + 0xB25C)
#define PPE_SSU_SSU_HOST_ETS_TC_SHAPING_18_REG             (PPE_SSU_BASE + 0xB260)
#define PPE_SSU_SSU_HOST_ETS_TC_SHAPING_19_REG             (PPE_SSU_BASE + 0xB264)
#define PPE_SSU_SSU_HOST_ETS_TC_SHAPING_20_REG             (PPE_SSU_BASE + 0xB268)
#define PPE_SSU_SSU_HOST_ETS_TC_SHAPING_21_REG             (PPE_SSU_BASE + 0xB26C)
#define PPE_SSU_SSU_HOST_ETS_TC_SHAPING_22_REG             (PPE_SSU_BASE + 0xB270)
#define PPE_SSU_SSU_HOST_ETS_TC_SHAPING_23_REG             (PPE_SSU_BASE + 0xB274)
#define PPE_SSU_SSU_HOST_ETS_TC_SHAPING_24_REG             (PPE_SSU_BASE + 0xB278)
#define PPE_SSU_SSU_HOST_ETS_TC_SHAPING_25_REG             (PPE_SSU_BASE + 0xB27C)
#define PPE_SSU_SSU_HOST_ETS_TC_SHAPING_26_REG             (PPE_SSU_BASE + 0xB280)
#define PPE_SSU_SSU_HOST_ETS_TC_SHAPING_27_REG             (PPE_SSU_BASE + 0xB284)
#define PPE_SSU_SSU_HOST_ETS_TC_SHAPING_28_REG             (PPE_SSU_BASE + 0xB288)
#define PPE_SSU_SSU_HOST_ETS_TC_SHAPING_29_REG             (PPE_SSU_BASE + 0xB28C)
#define PPE_SSU_SSU_HOST_ETS_TC_SHAPING_30_REG             (PPE_SSU_BASE + 0xB290)
#define PPE_SSU_SSU_HOST_ETS_TC_SHAPING_31_REG             (PPE_SSU_BASE + 0xB294)

#define PPE_SSU_SSU_HOST_ETS_WEIGHT_OFFSET_REG             (PPE_SSU_BASE + 0xB318) /* ETS HOST侧 调度权重偏移值 */
#define PPE_SSU_SSU_HOST_ETS_PORT_MAPING_0_REG             (PPE_SSU_BASE + 0xB31C) /* ETS HOST侧 PORT Maping bitmap配置 */
#define PPE_SSU_SSU_HOST_ETS_PORT_MAPING_1_REG             (PPE_SSU_BASE + 0xB320) /* ETS HOST侧 PORT Maping bitmap配置 */
#define PPE_SSU_SSU_HOST_ETS_PORT_MAPING_2_REG             (PPE_SSU_BASE + 0xB324) /* ETS HOST侧 PORT Maping bitmap配置 */
#define PPE_SSU_SSU_HOST_ETS_PORT_MAPING_3_REG             (PPE_SSU_BASE + 0xB328) /* ETS HOST侧 PORT Maping bitmap配置 */

/* ETS HOST侧 TC Group Maping bitmap配置 */
#define PPE_SSU_SSU_HOST_ETS_TCG_MAPING_0_REG              (PPE_SSU_BASE + 0xB32C)
#define PPE_SSU_SSU_HOST_ETS_TCG_MAPING_1_REG              (PPE_SSU_BASE + 0xB330)
#define PPE_SSU_SSU_HOST_ETS_TCG_MAPING_2_REG              (PPE_SSU_BASE + 0xB334)
#define PPE_SSU_SSU_HOST_ETS_TCG_MAPING_3_REG              (PPE_SSU_BASE + 0xB338)

#define PPE_SSU_SSU_HOST_ETS_TC_MAPING_0_REG               (PPE_SSU_BASE + 0xB33C) /* ETS HOST侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_HOST_ETS_TC_MAPING_1_REG               (PPE_SSU_BASE + 0xB340) /* ETS HOST侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_HOST_ETS_TC_MAPING_2_REG               (PPE_SSU_BASE + 0xB344) /* ETS HOST侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_HOST_ETS_TC_MAPING_3_REG               (PPE_SSU_BASE + 0xB348) /* ETS HOST侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_HOST_ETS_TC_MAPING_4_REG               (PPE_SSU_BASE + 0xB34C) /* ETS HOST侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_HOST_ETS_TC_MAPING_5_REG               (PPE_SSU_BASE + 0xB350) /* ETS HOST侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_HOST_ETS_TC_MAPING_6_REG               (PPE_SSU_BASE + 0xB354) /* ETS HOST侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_HOST_ETS_TC_MAPING_7_REG               (PPE_SSU_BASE + 0xB358) /* ETS HOST侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_HOST_ETS_TC_MAPING_8_REG               (PPE_SSU_BASE + 0xB35C) /* ETS HOST侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_HOST_ETS_TC_MAPING_9_REG               (PPE_SSU_BASE + 0xB360) /* ETS HOST侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_HOST_ETS_TC_MAPING_10_REG              (PPE_SSU_BASE + 0xB364) /* ETS HOST侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_HOST_ETS_TC_MAPING_11_REG              (PPE_SSU_BASE + 0xB368) /* ETS HOST侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_HOST_ETS_TC_MAPING_12_REG              (PPE_SSU_BASE + 0xB36C) /* ETS HOST侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_HOST_ETS_TC_MAPING_13_REG              (PPE_SSU_BASE + 0xB370) /* ETS HOST侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_HOST_ETS_TC_MAPING_14_REG              (PPE_SSU_BASE + 0xB374) /* ETS HOST侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_HOST_ETS_TC_MAPING_15_REG              (PPE_SSU_BASE + 0xB378) /* ETS HOST侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_HOST_ETS_TC_MAPING_16_REG              (PPE_SSU_BASE + 0xB37C) /* ETS HOST侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_HOST_ETS_TC_MAPING_17_REG              (PPE_SSU_BASE + 0xB380) /* ETS HOST侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_HOST_ETS_TC_MAPING_18_REG              (PPE_SSU_BASE + 0xB384) /* ETS HOST侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_HOST_ETS_TC_MAPING_19_REG              (PPE_SSU_BASE + 0xB388) /* ETS HOST侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_HOST_ETS_TC_MAPING_20_REG              (PPE_SSU_BASE + 0xB38C) /* ETS HOST侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_HOST_ETS_TC_MAPING_21_REG              (PPE_SSU_BASE + 0xB390) /* ETS HOST侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_HOST_ETS_TC_MAPING_22_REG              (PPE_SSU_BASE + 0xB394) /* ETS HOST侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_HOST_ETS_TC_MAPING_23_REG              (PPE_SSU_BASE + 0xB398) /* ETS HOST侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_HOST_ETS_TC_MAPING_24_REG              (PPE_SSU_BASE + 0xB39C) /* ETS HOST侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_HOST_ETS_TC_MAPING_25_REG              (PPE_SSU_BASE + 0xB3A0) /* ETS HOST侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_HOST_ETS_TC_MAPING_26_REG              (PPE_SSU_BASE + 0xB3A4) /* ETS HOST侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_HOST_ETS_TC_MAPING_27_REG              (PPE_SSU_BASE + 0xB3A8) /* ETS HOST侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_HOST_ETS_TC_MAPING_28_REG              (PPE_SSU_BASE + 0xB3AC) /* ETS HOST侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_HOST_ETS_TC_MAPING_29_REG              (PPE_SSU_BASE + 0xB3B0) /* ETS HOST侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_HOST_ETS_TC_MAPING_30_REG              (PPE_SSU_BASE + 0xB3B4) /* ETS HOST侧 TC Maping bitmap配置 */
#define PPE_SSU_SSU_HOST_ETS_TC_MAPING_31_REG              (PPE_SSU_BASE + 0xB3B8) /* ETS HOST侧 TC Maping bitmap配置 */

/* ETS HOST侧 PORT node 级的shaper配置参数 */
#define PPE_SSU_SSU_HOST_ETS_PORT_SHAPING_0_REG            (PPE_SSU_BASE + 0xB43C)
#define PPE_SSU_SSU_HOST_ETS_PORT_SHAPING_1_REG            (PPE_SSU_BASE + 0xB440)
#define PPE_SSU_SSU_HOST_ETS_PORT_SHAPING_2_REG            (PPE_SSU_BASE + 0xB444)
#define PPE_SSU_SSU_HOST_ETS_PORT_SHAPING_3_REG            (PPE_SSU_BASE + 0xB448)
#define PPE_SSU_SSU_HOST_ETS_QUEUE_SHAPING_0_REG           (PPE_SSU_BASE + 0xB44C)
#define PPE_SSU_SSU_HOST_ETS_QUEUE_SHAPING_1_REG           (PPE_SSU_BASE + 0xB450)
#define PPE_SSU_SSU_HOST_ETS_QUEUE_SHAPING_2_REG           (PPE_SSU_BASE + 0xB454)
#define PPE_SSU_SSU_HOST_ETS_QUEUE_SHAPING_3_REG           (PPE_SSU_BASE + 0xB458)
#define PPE_SSU_SSU_HOST_ETS_QUEUE_SHAPING_4_REG           (PPE_SSU_BASE + 0xB45C)
#define PPE_SSU_SSU_HOST_ETS_QUEUE_SHAPING_5_REG           (PPE_SSU_BASE + 0xB460)
#define PPE_SSU_SSU_HOST_ETS_QUEUE_SHAPING_6_REG           (PPE_SSU_BASE + 0xB464)
#define PPE_SSU_SSU_HOST_ETS_QUEUE_SHAPING_7_REG           (PPE_SSU_BASE + 0xB468)
#define PPE_SSU_SSU_HOST_ETS_QUEUE_SHAPING_8_REG           (PPE_SSU_BASE + 0xB46C)
#define PPE_SSU_SSU_HOST_ETS_QUEUE_SHAPING_9_REG           (PPE_SSU_BASE + 0xB470)
#define PPE_SSU_SSU_HOST_ETS_QUEUE_SHAPING_10_REG          (PPE_SSU_BASE + 0xB474)
#define PPE_SSU_SSU_HOST_ETS_QUEUE_SHAPING_11_REG          (PPE_SSU_BASE + 0xB478)
#define PPE_SSU_SSU_HOST_ETS_QUEUE_SHAPING_12_REG          (PPE_SSU_BASE + 0xB47C)
#define PPE_SSU_SSU_HOST_ETS_QUEUE_SHAPING_13_REG          (PPE_SSU_BASE + 0xB480)
#define PPE_SSU_SSU_HOST_ETS_QUEUE_SHAPING_14_REG          (PPE_SSU_BASE + 0xB484)
#define PPE_SSU_SSU_HOST_ETS_QUEUE_SHAPING_15_REG          (PPE_SSU_BASE + 0xB488)
#define PPE_SSU_SSU_HOST_ETS_QUEUE_SHAPING_16_REG          (PPE_SSU_BASE + 0xB48C)
#define PPE_SSU_SSU_HOST_ETS_QUEUE_SHAPING_17_REG          (PPE_SSU_BASE + 0xB490)
#define PPE_SSU_SSU_HOST_ETS_QUEUE_SHAPING_18_REG          (PPE_SSU_BASE + 0xB494)
#define PPE_SSU_SSU_HOST_ETS_QUEUE_SHAPING_19_REG          (PPE_SSU_BASE + 0xB498)
#define PPE_SSU_SSU_HOST_ETS_QUEUE_SHAPING_20_REG          (PPE_SSU_BASE + 0xB49C)
#define PPE_SSU_SSU_HOST_ETS_QUEUE_SHAPING_21_REG          (PPE_SSU_BASE + 0xB4A0)
#define PPE_SSU_SSU_HOST_ETS_QUEUE_SHAPING_22_REG          (PPE_SSU_BASE + 0xB4A4)
#define PPE_SSU_SSU_HOST_ETS_QUEUE_SHAPING_23_REG          (PPE_SSU_BASE + 0xB4A8)
#define PPE_SSU_SSU_HOST_ETS_QUEUE_SHAPING_24_REG          (PPE_SSU_BASE + 0xB4AC)
#define PPE_SSU_SSU_HOST_ETS_QUEUE_SHAPING_25_REG          (PPE_SSU_BASE + 0xB4B0)
#define PPE_SSU_SSU_HOST_ETS_QUEUE_SHAPING_26_REG          (PPE_SSU_BASE + 0xB4B4)
#define PPE_SSU_SSU_HOST_ETS_QUEUE_SHAPING_27_REG          (PPE_SSU_BASE + 0xB4B8)
#define PPE_SSU_SSU_HOST_ETS_QUEUE_SHAPING_28_REG          (PPE_SSU_BASE + 0xB4BC)
#define PPE_SSU_SSU_HOST_ETS_QUEUE_SHAPING_29_REG          (PPE_SSU_BASE + 0xB4C0)
#define PPE_SSU_SSU_HOST_ETS_QUEUE_SHAPING_30_REG          (PPE_SSU_BASE + 0xB4C4)
#define PPE_SSU_SSU_HOST_ETS_QUEUE_SHAPING_31_REG          (PPE_SSU_BASE + 0xB4C8)

#define PPE_SSU_SSU_HOST_ETS_PRE_SUB_OFFSET_LEN_CFG_0_REG  (PPE_SSU_BASE + 0xB54C) /* ETS HOST侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_HOST_ETS_PRE_SUB_OFFSET_LEN_CFG_1_REG  (PPE_SSU_BASE + 0xB550) /* ETS HOST侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_HOST_ETS_PRE_SUB_OFFSET_LEN_CFG_2_REG  (PPE_SSU_BASE + 0xB554) /* ETS HOST侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_HOST_ETS_PRE_SUB_OFFSET_LEN_CFG_3_REG  (PPE_SSU_BASE + 0xB558) /* ETS HOST侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_HOST_ETS_PRE_SUB_OFFSET_LEN_CFG_4_REG  (PPE_SSU_BASE + 0xB55C) /* ETS HOST侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_HOST_ETS_PRE_SUB_OFFSET_LEN_CFG_5_REG  (PPE_SSU_BASE + 0xB560) /* ETS HOST侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_HOST_ETS_PRE_SUB_OFFSET_LEN_CFG_6_REG  (PPE_SSU_BASE + 0xB564) /* ETS HOST侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_HOST_ETS_PRE_SUB_OFFSET_LEN_CFG_7_REG  (PPE_SSU_BASE + 0xB568) /* ETS HOST侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_HOST_ETS_PRE_SUB_OFFSET_LEN_CFG_8_REG  (PPE_SSU_BASE + 0xB56C) /* ETS HOST侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_HOST_ETS_PRE_SUB_OFFSET_LEN_CFG_9_REG  (PPE_SSU_BASE + 0xB570) /* ETS HOST侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_HOST_ETS_PRE_SUB_OFFSET_LEN_CFG_10_REG (PPE_SSU_BASE + 0xB574) /* ETS HOST侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_HOST_ETS_PRE_SUB_OFFSET_LEN_CFG_11_REG (PPE_SSU_BASE + 0xB578) /* ETS HOST侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_HOST_ETS_PRE_SUB_OFFSET_LEN_CFG_12_REG (PPE_SSU_BASE + 0xB57C) /* ETS HOST侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_HOST_ETS_PRE_SUB_OFFSET_LEN_CFG_13_REG (PPE_SSU_BASE + 0xB580) /* ETS HOST侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_HOST_ETS_PRE_SUB_OFFSET_LEN_CFG_14_REG (PPE_SSU_BASE + 0xB584) /* ETS HOST侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_HOST_ETS_PRE_SUB_OFFSET_LEN_CFG_15_REG (PPE_SSU_BASE + 0xB588) /* ETS HOST侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_HOST_ETS_PRE_SUB_OFFSET_LEN_CFG_16_REG (PPE_SSU_BASE + 0xB58C) /* ETS HOST侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_HOST_ETS_PRE_SUB_OFFSET_LEN_CFG_17_REG (PPE_SSU_BASE + 0xB590) /* ETS HOST侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_HOST_ETS_PRE_SUB_OFFSET_LEN_CFG_18_REG (PPE_SSU_BASE + 0xB594) /* ETS HOST侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_HOST_ETS_PRE_SUB_OFFSET_LEN_CFG_19_REG (PPE_SSU_BASE + 0xB598) /* ETS HOST侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_HOST_ETS_PRE_SUB_OFFSET_LEN_CFG_20_REG (PPE_SSU_BASE + 0xB59C) /* ETS HOST侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_HOST_ETS_PRE_SUB_OFFSET_LEN_CFG_21_REG (PPE_SSU_BASE + 0xB5A0) /* ETS HOST侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_HOST_ETS_PRE_SUB_OFFSET_LEN_CFG_22_REG (PPE_SSU_BASE + 0xB5A4) /* ETS HOST侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_HOST_ETS_PRE_SUB_OFFSET_LEN_CFG_23_REG (PPE_SSU_BASE + 0xB5A8) /* ETS HOST侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_HOST_ETS_PRE_SUB_OFFSET_LEN_CFG_24_REG (PPE_SSU_BASE + 0xB5AC) /* ETS HOST侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_HOST_ETS_PRE_SUB_OFFSET_LEN_CFG_25_REG (PPE_SSU_BASE + 0xB5B0) /* ETS HOST侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_HOST_ETS_PRE_SUB_OFFSET_LEN_CFG_26_REG (PPE_SSU_BASE + 0xB5B4) /* ETS HOST侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_HOST_ETS_PRE_SUB_OFFSET_LEN_CFG_27_REG (PPE_SSU_BASE + 0xB5B8) /* ETS HOST侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_HOST_ETS_PRE_SUB_OFFSET_LEN_CFG_28_REG (PPE_SSU_BASE + 0xB5BC) /* ETS HOST侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_HOST_ETS_PRE_SUB_OFFSET_LEN_CFG_29_REG (PPE_SSU_BASE + 0xB5C0) /* ETS HOST侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_HOST_ETS_PRE_SUB_OFFSET_LEN_CFG_30_REG (PPE_SSU_BASE + 0xB5C4) /* ETS HOST侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_HOST_ETS_PRE_SUB_OFFSET_LEN_CFG_31_REG (PPE_SSU_BASE + 0xB5C8) /* ETS HOST侧 基于队列配置的预减包长 */
#define PPE_SSU_SSU_HOST_ETS_SHAPPING_BYPASS_REG           (PPE_SSU_BASE + 0xB64C) /* ETS HOST侧 SHAPING bypass配置寄存器 */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_LINK_TC_CFG_0_REG       (PPE_SSU_BASE + 0xB650) /* ETS HOST侧 queue node对应的TC id */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_LINK_TC_CFG_1_REG       (PPE_SSU_BASE + 0xB654) /* ETS HOST侧 queue node对应的TC id */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_LINK_TC_CFG_2_REG       (PPE_SSU_BASE + 0xB658) /* ETS HOST侧 queue node对应的TC id */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_LINK_TC_CFG_3_REG       (PPE_SSU_BASE + 0xB65C) /* ETS HOST侧 queue node对应的TC id */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_LINK_TC_CFG_4_REG       (PPE_SSU_BASE + 0xB660) /* ETS HOST侧 queue node对应的TC id */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_LINK_TC_CFG_5_REG       (PPE_SSU_BASE + 0xB664) /* ETS HOST侧 queue node对应的TC id */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_LINK_TC_CFG_6_REG       (PPE_SSU_BASE + 0xB668) /* ETS HOST侧 queue node对应的TC id */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_LINK_TC_CFG_7_REG       (PPE_SSU_BASE + 0xB66C) /* ETS HOST侧 queue node对应的TC id */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_LINK_TC_CFG_8_REG       (PPE_SSU_BASE + 0xB670) /* ETS HOST侧 queue node对应的TC id */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_LINK_TC_CFG_9_REG       (PPE_SSU_BASE + 0xB674) /* ETS HOST侧 queue node对应的TC id */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_LINK_TC_CFG_10_REG      (PPE_SSU_BASE + 0xB678) /* ETS HOST侧 queue node对应的TC id */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_LINK_TC_CFG_11_REG      (PPE_SSU_BASE + 0xB67C) /* ETS HOST侧 queue node对应的TC id */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_LINK_TC_CFG_12_REG      (PPE_SSU_BASE + 0xB680) /* ETS HOST侧 queue node对应的TC id */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_LINK_TC_CFG_13_REG      (PPE_SSU_BASE + 0xB684) /* ETS HOST侧 queue node对应的TC id */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_LINK_TC_CFG_14_REG      (PPE_SSU_BASE + 0xB688) /* ETS HOST侧 queue node对应的TC id */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_LINK_TC_CFG_15_REG      (PPE_SSU_BASE + 0xB68C) /* ETS HOST侧 queue node对应的TC id */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_LINK_TC_CFG_16_REG      (PPE_SSU_BASE + 0xB690) /* ETS HOST侧 queue node对应的TC id */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_LINK_TC_CFG_17_REG      (PPE_SSU_BASE + 0xB694) /* ETS HOST侧 queue node对应的TC id */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_LINK_TC_CFG_18_REG      (PPE_SSU_BASE + 0xB698) /* ETS HOST侧 queue node对应的TC id */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_LINK_TC_CFG_19_REG      (PPE_SSU_BASE + 0xB69C) /* ETS HOST侧 queue node对应的TC id */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_LINK_TC_CFG_20_REG      (PPE_SSU_BASE + 0xB6A0) /* ETS HOST侧 queue node对应的TC id */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_LINK_TC_CFG_21_REG      (PPE_SSU_BASE + 0xB6A4) /* ETS HOST侧 queue node对应的TC id */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_LINK_TC_CFG_22_REG      (PPE_SSU_BASE + 0xB6A8) /* ETS HOST侧 queue node对应的TC id */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_LINK_TC_CFG_23_REG      (PPE_SSU_BASE + 0xB6AC) /* ETS HOST侧 queue node对应的TC id */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_LINK_TC_CFG_24_REG      (PPE_SSU_BASE + 0xB6B0) /* ETS HOST侧 queue node对应的TC id */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_LINK_TC_CFG_25_REG      (PPE_SSU_BASE + 0xB6B4) /* ETS HOST侧 queue node对应的TC id */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_LINK_TC_CFG_26_REG      (PPE_SSU_BASE + 0xB6B8) /* ETS HOST侧 queue node对应的TC id */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_LINK_TC_CFG_27_REG      (PPE_SSU_BASE + 0xB6BC) /* ETS HOST侧 queue node对应的TC id */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_LINK_TC_CFG_28_REG      (PPE_SSU_BASE + 0xB6C0) /* ETS HOST侧 queue node对应的TC id */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_LINK_TC_CFG_29_REG      (PPE_SSU_BASE + 0xB6C4) /* ETS HOST侧 queue node对应的TC id */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_LINK_TC_CFG_30_REG      (PPE_SSU_BASE + 0xB6C8) /* ETS HOST侧 queue node对应的TC id */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_LINK_TC_CFG_31_REG      (PPE_SSU_BASE + 0xB6CC) /* ETS HOST侧 queue node对应的TC id */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_EN_CFG_0_REG            (PPE_SSU_BASE + 0xB750) /* ETS HOST侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_EN_CFG_1_REG            (PPE_SSU_BASE + 0xB754) /* ETS HOST侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_EN_CFG_2_REG            (PPE_SSU_BASE + 0xB758) /* ETS HOST侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_EN_CFG_3_REG            (PPE_SSU_BASE + 0xB75C) /* ETS HOST侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_EN_CFG_4_REG            (PPE_SSU_BASE + 0xB760) /* ETS HOST侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_EN_CFG_5_REG            (PPE_SSU_BASE + 0xB764) /* ETS HOST侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_EN_CFG_6_REG            (PPE_SSU_BASE + 0xB768) /* ETS HOST侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_EN_CFG_7_REG            (PPE_SSU_BASE + 0xB76C) /* ETS HOST侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_EN_CFG_8_REG            (PPE_SSU_BASE + 0xB770) /* ETS HOST侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_EN_CFG_9_REG            (PPE_SSU_BASE + 0xB774) /* ETS HOST侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_EN_CFG_10_REG           (PPE_SSU_BASE + 0xB778) /* ETS HOST侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_EN_CFG_11_REG           (PPE_SSU_BASE + 0xB77C) /* ETS HOST侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_EN_CFG_12_REG           (PPE_SSU_BASE + 0xB780) /* ETS HOST侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_EN_CFG_13_REG           (PPE_SSU_BASE + 0xB784) /* ETS HOST侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_EN_CFG_14_REG           (PPE_SSU_BASE + 0xB788) /* ETS HOST侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_EN_CFG_15_REG           (PPE_SSU_BASE + 0xB78C) /* ETS HOST侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_EN_CFG_16_REG           (PPE_SSU_BASE + 0xB790) /* ETS HOST侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_EN_CFG_17_REG           (PPE_SSU_BASE + 0xB794) /* ETS HOST侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_EN_CFG_18_REG           (PPE_SSU_BASE + 0xB798) /* ETS HOST侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_EN_CFG_19_REG           (PPE_SSU_BASE + 0xB79C) /* ETS HOST侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_EN_CFG_20_REG           (PPE_SSU_BASE + 0xB7A0) /* ETS HOST侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_EN_CFG_21_REG           (PPE_SSU_BASE + 0xB7A4) /* ETS HOST侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_EN_CFG_22_REG           (PPE_SSU_BASE + 0xB7A8) /* ETS HOST侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_EN_CFG_23_REG           (PPE_SSU_BASE + 0xB7AC) /* ETS HOST侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_EN_CFG_24_REG           (PPE_SSU_BASE + 0xB7B0) /* ETS HOST侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_EN_CFG_25_REG           (PPE_SSU_BASE + 0xB7B4) /* ETS HOST侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_EN_CFG_26_REG           (PPE_SSU_BASE + 0xB7B8) /* ETS HOST侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_EN_CFG_27_REG           (PPE_SSU_BASE + 0xB7BC) /* ETS HOST侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_EN_CFG_28_REG           (PPE_SSU_BASE + 0xB7C0) /* ETS HOST侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_EN_CFG_29_REG           (PPE_SSU_BASE + 0xB7C4) /* ETS HOST侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_EN_CFG_30_REG           (PPE_SSU_BASE + 0xB7C8) /* ETS HOST侧 queue node 级的使能配置 */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_EN_CFG_31_REG           (PPE_SSU_BASE + 0xB7CC) /* ETS HOST侧 queue node 级的使能配置 */

/* ETS HOST侧 TC级的CBS算法中发送斜率SENDSLOPE配置 */
#define PPE_SSU_SSU_HOST_ETS_TC_SENDSLOPE_0_REG            (PPE_SSU_BASE + 0xB850)
#define PPE_SSU_SSU_HOST_ETS_TC_SENDSLOPE_1_REG            (PPE_SSU_BASE + 0xB854)
#define PPE_SSU_SSU_HOST_ETS_TC_SENDSLOPE_2_REG            (PPE_SSU_BASE + 0xB858)
#define PPE_SSU_SSU_HOST_ETS_TC_SENDSLOPE_3_REG            (PPE_SSU_BASE + 0xB85C)
#define PPE_SSU_SSU_HOST_ETS_TC_SENDSLOPE_4_REG            (PPE_SSU_BASE + 0xB860)
#define PPE_SSU_SSU_HOST_ETS_TC_SENDSLOPE_5_REG            (PPE_SSU_BASE + 0xB864)
#define PPE_SSU_SSU_HOST_ETS_TC_SENDSLOPE_6_REG            (PPE_SSU_BASE + 0xB868)
#define PPE_SSU_SSU_HOST_ETS_TC_SENDSLOPE_7_REG            (PPE_SSU_BASE + 0xB86C)
#define PPE_SSU_SSU_HOST_ETS_TC_SENDSLOPE_8_REG            (PPE_SSU_BASE + 0xB870)
#define PPE_SSU_SSU_HOST_ETS_TC_SENDSLOPE_9_REG            (PPE_SSU_BASE + 0xB874)
#define PPE_SSU_SSU_HOST_ETS_TC_SENDSLOPE_10_REG           (PPE_SSU_BASE + 0xB878)
#define PPE_SSU_SSU_HOST_ETS_TC_SENDSLOPE_11_REG           (PPE_SSU_BASE + 0xB87C)
#define PPE_SSU_SSU_HOST_ETS_TC_SENDSLOPE_12_REG           (PPE_SSU_BASE + 0xB880)
#define PPE_SSU_SSU_HOST_ETS_TC_SENDSLOPE_13_REG           (PPE_SSU_BASE + 0xB884)
#define PPE_SSU_SSU_HOST_ETS_TC_SENDSLOPE_14_REG           (PPE_SSU_BASE + 0xB888)
#define PPE_SSU_SSU_HOST_ETS_TC_SENDSLOPE_15_REG           (PPE_SSU_BASE + 0xB88C)
#define PPE_SSU_SSU_HOST_ETS_TC_SENDSLOPE_16_REG           (PPE_SSU_BASE + 0xB890)
#define PPE_SSU_SSU_HOST_ETS_TC_SENDSLOPE_17_REG           (PPE_SSU_BASE + 0xB894)
#define PPE_SSU_SSU_HOST_ETS_TC_SENDSLOPE_18_REG           (PPE_SSU_BASE + 0xB898)
#define PPE_SSU_SSU_HOST_ETS_TC_SENDSLOPE_19_REG           (PPE_SSU_BASE + 0xB89C)
#define PPE_SSU_SSU_HOST_ETS_TC_SENDSLOPE_20_REG           (PPE_SSU_BASE + 0xB8A0)
#define PPE_SSU_SSU_HOST_ETS_TC_SENDSLOPE_21_REG           (PPE_SSU_BASE + 0xB8A4)
#define PPE_SSU_SSU_HOST_ETS_TC_SENDSLOPE_22_REG           (PPE_SSU_BASE + 0xB8A8)
#define PPE_SSU_SSU_HOST_ETS_TC_SENDSLOPE_23_REG           (PPE_SSU_BASE + 0xB8AC)
#define PPE_SSU_SSU_HOST_ETS_TC_SENDSLOPE_24_REG           (PPE_SSU_BASE + 0xB8B0)
#define PPE_SSU_SSU_HOST_ETS_TC_SENDSLOPE_25_REG           (PPE_SSU_BASE + 0xB8B4)
#define PPE_SSU_SSU_HOST_ETS_TC_SENDSLOPE_26_REG           (PPE_SSU_BASE + 0xB8B8)
#define PPE_SSU_SSU_HOST_ETS_TC_SENDSLOPE_27_REG           (PPE_SSU_BASE + 0xB8BC)
#define PPE_SSU_SSU_HOST_ETS_TC_SENDSLOPE_28_REG           (PPE_SSU_BASE + 0xB8C0)
#define PPE_SSU_SSU_HOST_ETS_TC_SENDSLOPE_29_REG           (PPE_SSU_BASE + 0xB8C4)
#define PPE_SSU_SSU_HOST_ETS_TC_SENDSLOPE_30_REG           (PPE_SSU_BASE + 0xB8C8)
#define PPE_SSU_SSU_HOST_ETS_TC_SENDSLOPE_31_REG           (PPE_SSU_BASE + 0xB8CC)

/* ETS HOST侧 TC级的CBS算法中累积斜率IDLESLOPE配置 */
#define PPE_SSU_SSU_HOST_ETS_TC_IDLESLOPE_0_REG            (PPE_SSU_BASE + 0xB950)
#define PPE_SSU_SSU_HOST_ETS_TC_IDLESLOPE_1_REG            (PPE_SSU_BASE + 0xB954)
#define PPE_SSU_SSU_HOST_ETS_TC_IDLESLOPE_2_REG            (PPE_SSU_BASE + 0xB958)
#define PPE_SSU_SSU_HOST_ETS_TC_IDLESLOPE_3_REG            (PPE_SSU_BASE + 0xB95C)
#define PPE_SSU_SSU_HOST_ETS_TC_IDLESLOPE_4_REG            (PPE_SSU_BASE + 0xB960)
#define PPE_SSU_SSU_HOST_ETS_TC_IDLESLOPE_5_REG            (PPE_SSU_BASE + 0xB964)
#define PPE_SSU_SSU_HOST_ETS_TC_IDLESLOPE_6_REG            (PPE_SSU_BASE + 0xB968)
#define PPE_SSU_SSU_HOST_ETS_TC_IDLESLOPE_7_REG            (PPE_SSU_BASE + 0xB96C)
#define PPE_SSU_SSU_HOST_ETS_TC_IDLESLOPE_8_REG            (PPE_SSU_BASE + 0xB970)
#define PPE_SSU_SSU_HOST_ETS_TC_IDLESLOPE_9_REG            (PPE_SSU_BASE + 0xB974)
#define PPE_SSU_SSU_HOST_ETS_TC_IDLESLOPE_10_REG           (PPE_SSU_BASE + 0xB978)
#define PPE_SSU_SSU_HOST_ETS_TC_IDLESLOPE_11_REG           (PPE_SSU_BASE + 0xB97C)
#define PPE_SSU_SSU_HOST_ETS_TC_IDLESLOPE_12_REG           (PPE_SSU_BASE + 0xB980)
#define PPE_SSU_SSU_HOST_ETS_TC_IDLESLOPE_13_REG           (PPE_SSU_BASE + 0xB984)
#define PPE_SSU_SSU_HOST_ETS_TC_IDLESLOPE_14_REG           (PPE_SSU_BASE + 0xB988)
#define PPE_SSU_SSU_HOST_ETS_TC_IDLESLOPE_15_REG           (PPE_SSU_BASE + 0xB98C)
#define PPE_SSU_SSU_HOST_ETS_TC_IDLESLOPE_16_REG           (PPE_SSU_BASE + 0xB990)
#define PPE_SSU_SSU_HOST_ETS_TC_IDLESLOPE_17_REG           (PPE_SSU_BASE + 0xB994)
#define PPE_SSU_SSU_HOST_ETS_TC_IDLESLOPE_18_REG           (PPE_SSU_BASE + 0xB998)
#define PPE_SSU_SSU_HOST_ETS_TC_IDLESLOPE_19_REG           (PPE_SSU_BASE + 0xB99C)
#define PPE_SSU_SSU_HOST_ETS_TC_IDLESLOPE_20_REG           (PPE_SSU_BASE + 0xB9A0)
#define PPE_SSU_SSU_HOST_ETS_TC_IDLESLOPE_21_REG           (PPE_SSU_BASE + 0xB9A4)
#define PPE_SSU_SSU_HOST_ETS_TC_IDLESLOPE_22_REG           (PPE_SSU_BASE + 0xB9A8)
#define PPE_SSU_SSU_HOST_ETS_TC_IDLESLOPE_23_REG           (PPE_SSU_BASE + 0xB9AC)
#define PPE_SSU_SSU_HOST_ETS_TC_IDLESLOPE_24_REG           (PPE_SSU_BASE + 0xB9B0)
#define PPE_SSU_SSU_HOST_ETS_TC_IDLESLOPE_25_REG           (PPE_SSU_BASE + 0xB9B4)
#define PPE_SSU_SSU_HOST_ETS_TC_IDLESLOPE_26_REG           (PPE_SSU_BASE + 0xB9B8)
#define PPE_SSU_SSU_HOST_ETS_TC_IDLESLOPE_27_REG           (PPE_SSU_BASE + 0xB9BC)
#define PPE_SSU_SSU_HOST_ETS_TC_IDLESLOPE_28_REG           (PPE_SSU_BASE + 0xB9C0)
#define PPE_SSU_SSU_HOST_ETS_TC_IDLESLOPE_29_REG           (PPE_SSU_BASE + 0xB9C4)
#define PPE_SSU_SSU_HOST_ETS_TC_IDLESLOPE_30_REG           (PPE_SSU_BASE + 0xB9C8)
#define PPE_SSU_SSU_HOST_ETS_TC_IDLESLOPE_31_REG           (PPE_SSU_BASE + 0xB9CC)

/* ETS HOST侧 TC级的CBS算法中高水线HICREDIT配置 */
#define PPE_SSU_SSU_HOST_ETS_TC_HICREDIT_0_REG             (PPE_SSU_BASE + 0xBA50)
#define PPE_SSU_SSU_HOST_ETS_TC_HICREDIT_1_REG             (PPE_SSU_BASE + 0xBA54)
#define PPE_SSU_SSU_HOST_ETS_TC_HICREDIT_2_REG             (PPE_SSU_BASE + 0xBA58)
#define PPE_SSU_SSU_HOST_ETS_TC_HICREDIT_3_REG             (PPE_SSU_BASE + 0xBA5C)
#define PPE_SSU_SSU_HOST_ETS_TC_HICREDIT_4_REG             (PPE_SSU_BASE + 0xBA60)
#define PPE_SSU_SSU_HOST_ETS_TC_HICREDIT_5_REG             (PPE_SSU_BASE + 0xBA64)
#define PPE_SSU_SSU_HOST_ETS_TC_HICREDIT_6_REG             (PPE_SSU_BASE + 0xBA68)
#define PPE_SSU_SSU_HOST_ETS_TC_HICREDIT_7_REG             (PPE_SSU_BASE + 0xBA6C)
#define PPE_SSU_SSU_HOST_ETS_TC_HICREDIT_8_REG             (PPE_SSU_BASE + 0xBA70)
#define PPE_SSU_SSU_HOST_ETS_TC_HICREDIT_9_REG             (PPE_SSU_BASE + 0xBA74)
#define PPE_SSU_SSU_HOST_ETS_TC_HICREDIT_10_REG            (PPE_SSU_BASE + 0xBA78)
#define PPE_SSU_SSU_HOST_ETS_TC_HICREDIT_11_REG            (PPE_SSU_BASE + 0xBA7C)
#define PPE_SSU_SSU_HOST_ETS_TC_HICREDIT_12_REG            (PPE_SSU_BASE + 0xBA80)
#define PPE_SSU_SSU_HOST_ETS_TC_HICREDIT_13_REG            (PPE_SSU_BASE + 0xBA84)
#define PPE_SSU_SSU_HOST_ETS_TC_HICREDIT_14_REG            (PPE_SSU_BASE + 0xBA88)
#define PPE_SSU_SSU_HOST_ETS_TC_HICREDIT_15_REG            (PPE_SSU_BASE + 0xBA8C)
#define PPE_SSU_SSU_HOST_ETS_TC_HICREDIT_16_REG            (PPE_SSU_BASE + 0xBA90)
#define PPE_SSU_SSU_HOST_ETS_TC_HICREDIT_17_REG            (PPE_SSU_BASE + 0xBA94)
#define PPE_SSU_SSU_HOST_ETS_TC_HICREDIT_18_REG            (PPE_SSU_BASE + 0xBA98)
#define PPE_SSU_SSU_HOST_ETS_TC_HICREDIT_19_REG            (PPE_SSU_BASE + 0xBA9C)
#define PPE_SSU_SSU_HOST_ETS_TC_HICREDIT_20_REG            (PPE_SSU_BASE + 0xBAA0)
#define PPE_SSU_SSU_HOST_ETS_TC_HICREDIT_21_REG            (PPE_SSU_BASE + 0xBAA4)
#define PPE_SSU_SSU_HOST_ETS_TC_HICREDIT_22_REG            (PPE_SSU_BASE + 0xBAA8)
#define PPE_SSU_SSU_HOST_ETS_TC_HICREDIT_23_REG            (PPE_SSU_BASE + 0xBAAC)
#define PPE_SSU_SSU_HOST_ETS_TC_HICREDIT_24_REG            (PPE_SSU_BASE + 0xBAB0)
#define PPE_SSU_SSU_HOST_ETS_TC_HICREDIT_25_REG            (PPE_SSU_BASE + 0xBAB4)
#define PPE_SSU_SSU_HOST_ETS_TC_HICREDIT_26_REG            (PPE_SSU_BASE + 0xBAB8)
#define PPE_SSU_SSU_HOST_ETS_TC_HICREDIT_27_REG            (PPE_SSU_BASE + 0xBABC)
#define PPE_SSU_SSU_HOST_ETS_TC_HICREDIT_28_REG            (PPE_SSU_BASE + 0xBAC0)
#define PPE_SSU_SSU_HOST_ETS_TC_HICREDIT_29_REG            (PPE_SSU_BASE + 0xBAC4)
#define PPE_SSU_SSU_HOST_ETS_TC_HICREDIT_30_REG            (PPE_SSU_BASE + 0xBAC8)
#define PPE_SSU_SSU_HOST_ETS_TC_HICREDIT_31_REG            (PPE_SSU_BASE + 0xBACC)
#define PPE_SSU_SSU_HOST_ETS_TC_LOCREDIT_0_REG             (PPE_SSU_BASE + 0xBB50)
#define PPE_SSU_SSU_HOST_ETS_TC_LOCREDIT_1_REG             (PPE_SSU_BASE + 0xBB54)
#define PPE_SSU_SSU_HOST_ETS_TC_LOCREDIT_2_REG             (PPE_SSU_BASE + 0xBB58)
#define PPE_SSU_SSU_HOST_ETS_TC_LOCREDIT_3_REG             (PPE_SSU_BASE + 0xBB5C)
#define PPE_SSU_SSU_HOST_ETS_TC_LOCREDIT_4_REG             (PPE_SSU_BASE + 0xBB60)
#define PPE_SSU_SSU_HOST_ETS_TC_LOCREDIT_5_REG             (PPE_SSU_BASE + 0xBB64)
#define PPE_SSU_SSU_HOST_ETS_TC_LOCREDIT_6_REG             (PPE_SSU_BASE + 0xBB68)
#define PPE_SSU_SSU_HOST_ETS_TC_LOCREDIT_7_REG             (PPE_SSU_BASE + 0xBB6C)
#define PPE_SSU_SSU_HOST_ETS_TC_LOCREDIT_8_REG             (PPE_SSU_BASE + 0xBB70)
#define PPE_SSU_SSU_HOST_ETS_TC_LOCREDIT_9_REG             (PPE_SSU_BASE + 0xBB74)
#define PPE_SSU_SSU_HOST_ETS_TC_LOCREDIT_10_REG            (PPE_SSU_BASE + 0xBB78)
#define PPE_SSU_SSU_HOST_ETS_TC_LOCREDIT_11_REG            (PPE_SSU_BASE + 0xBB7C)
#define PPE_SSU_SSU_HOST_ETS_TC_LOCREDIT_12_REG            (PPE_SSU_BASE + 0xBB80)
#define PPE_SSU_SSU_HOST_ETS_TC_LOCREDIT_13_REG            (PPE_SSU_BASE + 0xBB84)
#define PPE_SSU_SSU_HOST_ETS_TC_LOCREDIT_14_REG            (PPE_SSU_BASE + 0xBB88)
#define PPE_SSU_SSU_HOST_ETS_TC_LOCREDIT_15_REG            (PPE_SSU_BASE + 0xBB8C)
#define PPE_SSU_SSU_HOST_ETS_TC_LOCREDIT_16_REG            (PPE_SSU_BASE + 0xBB90)
#define PPE_SSU_SSU_HOST_ETS_TC_LOCREDIT_17_REG            (PPE_SSU_BASE + 0xBB94)
#define PPE_SSU_SSU_HOST_ETS_TC_LOCREDIT_18_REG            (PPE_SSU_BASE + 0xBB98)
#define PPE_SSU_SSU_HOST_ETS_TC_LOCREDIT_19_REG            (PPE_SSU_BASE + 0xBB9C)
#define PPE_SSU_SSU_HOST_ETS_TC_LOCREDIT_20_REG            (PPE_SSU_BASE + 0xBBA0)
#define PPE_SSU_SSU_HOST_ETS_TC_LOCREDIT_21_REG            (PPE_SSU_BASE + 0xBBA4)
#define PPE_SSU_SSU_HOST_ETS_TC_LOCREDIT_22_REG            (PPE_SSU_BASE + 0xBBA8)
#define PPE_SSU_SSU_HOST_ETS_TC_LOCREDIT_23_REG            (PPE_SSU_BASE + 0xBBAC)
#define PPE_SSU_SSU_HOST_ETS_TC_LOCREDIT_24_REG            (PPE_SSU_BASE + 0xBBB0)
#define PPE_SSU_SSU_HOST_ETS_TC_LOCREDIT_25_REG            (PPE_SSU_BASE + 0xBBB4)
#define PPE_SSU_SSU_HOST_ETS_TC_LOCREDIT_26_REG            (PPE_SSU_BASE + 0xBBB8)
#define PPE_SSU_SSU_HOST_ETS_TC_LOCREDIT_27_REG            (PPE_SSU_BASE + 0xBBBC)
#define PPE_SSU_SSU_HOST_ETS_TC_LOCREDIT_28_REG            (PPE_SSU_BASE + 0xBBC0)
#define PPE_SSU_SSU_HOST_ETS_TC_LOCREDIT_29_REG            (PPE_SSU_BASE + 0xBBC4)
#define PPE_SSU_SSU_HOST_ETS_TC_LOCREDIT_30_REG            (PPE_SSU_BASE + 0xBBC8)
#define PPE_SSU_SSU_HOST_ETS_TC_LOCREDIT_31_REG            (PPE_SSU_BASE + 0xBBCC)

/* ETS HOST侧 TC node 级的CBS使能配置 */
#define PPE_SSU_SSU_HOST_ETS_TC_CBS_EN_0_REG               (PPE_SSU_BASE + 0xBC50)
#define PPE_SSU_SSU_HOST_ETS_TC_CBS_EN_1_REG               (PPE_SSU_BASE + 0xBC54)
#define PPE_SSU_SSU_HOST_ETS_TC_CBS_EN_2_REG               (PPE_SSU_BASE + 0xBC58)
#define PPE_SSU_SSU_HOST_ETS_TC_CBS_EN_3_REG               (PPE_SSU_BASE + 0xBC5C)
#define PPE_SSU_SSU_HOST_ETS_TC_CBS_EN_4_REG               (PPE_SSU_BASE + 0xBC60)
#define PPE_SSU_SSU_HOST_ETS_TC_CBS_EN_5_REG               (PPE_SSU_BASE + 0xBC64)
#define PPE_SSU_SSU_HOST_ETS_TC_CBS_EN_6_REG               (PPE_SSU_BASE + 0xBC68)
#define PPE_SSU_SSU_HOST_ETS_TC_CBS_EN_7_REG               (PPE_SSU_BASE + 0xBC6C)
#define PPE_SSU_SSU_HOST_ETS_TC_CBS_EN_8_REG               (PPE_SSU_BASE + 0xBC70)
#define PPE_SSU_SSU_HOST_ETS_TC_CBS_EN_9_REG               (PPE_SSU_BASE + 0xBC74)
#define PPE_SSU_SSU_HOST_ETS_TC_CBS_EN_10_REG              (PPE_SSU_BASE + 0xBC78)
#define PPE_SSU_SSU_HOST_ETS_TC_CBS_EN_11_REG              (PPE_SSU_BASE + 0xBC7C)
#define PPE_SSU_SSU_HOST_ETS_TC_CBS_EN_12_REG              (PPE_SSU_BASE + 0xBC80)
#define PPE_SSU_SSU_HOST_ETS_TC_CBS_EN_13_REG              (PPE_SSU_BASE + 0xBC84)
#define PPE_SSU_SSU_HOST_ETS_TC_CBS_EN_14_REG              (PPE_SSU_BASE + 0xBC88)
#define PPE_SSU_SSU_HOST_ETS_TC_CBS_EN_15_REG              (PPE_SSU_BASE + 0xBC8C)
#define PPE_SSU_SSU_HOST_ETS_TC_CBS_EN_16_REG              (PPE_SSU_BASE + 0xBC90)
#define PPE_SSU_SSU_HOST_ETS_TC_CBS_EN_17_REG              (PPE_SSU_BASE + 0xBC94)
#define PPE_SSU_SSU_HOST_ETS_TC_CBS_EN_18_REG              (PPE_SSU_BASE + 0xBC98)
#define PPE_SSU_SSU_HOST_ETS_TC_CBS_EN_19_REG              (PPE_SSU_BASE + 0xBC9C)
#define PPE_SSU_SSU_HOST_ETS_TC_CBS_EN_20_REG              (PPE_SSU_BASE + 0xBCA0)
#define PPE_SSU_SSU_HOST_ETS_TC_CBS_EN_21_REG              (PPE_SSU_BASE + 0xBCA4)
#define PPE_SSU_SSU_HOST_ETS_TC_CBS_EN_22_REG              (PPE_SSU_BASE + 0xBCA8)
#define PPE_SSU_SSU_HOST_ETS_TC_CBS_EN_23_REG              (PPE_SSU_BASE + 0xBCAC)
#define PPE_SSU_SSU_HOST_ETS_TC_CBS_EN_24_REG              (PPE_SSU_BASE + 0xBCB0)
#define PPE_SSU_SSU_HOST_ETS_TC_CBS_EN_25_REG              (PPE_SSU_BASE + 0xBCB4)
#define PPE_SSU_SSU_HOST_ETS_TC_CBS_EN_26_REG              (PPE_SSU_BASE + 0xBCB8)
#define PPE_SSU_SSU_HOST_ETS_TC_CBS_EN_27_REG              (PPE_SSU_BASE + 0xBCBC)
#define PPE_SSU_SSU_HOST_ETS_TC_CBS_EN_28_REG              (PPE_SSU_BASE + 0xBCC0)
#define PPE_SSU_SSU_HOST_ETS_TC_CBS_EN_29_REG              (PPE_SSU_BASE + 0xBCC4)
#define PPE_SSU_SSU_HOST_ETS_TC_CBS_EN_30_REG              (PPE_SSU_BASE + 0xBCC8)
#define PPE_SSU_SSU_HOST_ETS_TC_CBS_EN_31_REG              (PPE_SSU_BASE + 0xBCCC)

/* ETS HOST侧 TC node 级的对应MAC端口速率配置 */
#define PPE_SSU_SSU_HOST_ETS_TC_SPEED_SEL_0_REG            (PPE_SSU_BASE + 0xBD50)
#define PPE_SSU_SSU_HOST_ETS_TC_SPEED_SEL_1_REG            (PPE_SSU_BASE + 0xBD54)
#define PPE_SSU_SSU_HOST_ETS_TC_SPEED_SEL_2_REG            (PPE_SSU_BASE + 0xBD58)
#define PPE_SSU_SSU_HOST_ETS_TC_SPEED_SEL_3_REG            (PPE_SSU_BASE + 0xBD5C)
#define PPE_SSU_SSU_HOST_ETS_TC_SPEED_SEL_4_REG            (PPE_SSU_BASE + 0xBD60)
#define PPE_SSU_SSU_HOST_ETS_TC_SPEED_SEL_5_REG            (PPE_SSU_BASE + 0xBD64)
#define PPE_SSU_SSU_HOST_ETS_TC_SPEED_SEL_6_REG            (PPE_SSU_BASE + 0xBD68)
#define PPE_SSU_SSU_HOST_ETS_TC_SPEED_SEL_7_REG            (PPE_SSU_BASE + 0xBD6C)
#define PPE_SSU_SSU_HOST_ETS_TC_SPEED_SEL_8_REG            (PPE_SSU_BASE + 0xBD70)
#define PPE_SSU_SSU_HOST_ETS_TC_SPEED_SEL_9_REG            (PPE_SSU_BASE + 0xBD74)
#define PPE_SSU_SSU_HOST_ETS_TC_SPEED_SEL_10_REG           (PPE_SSU_BASE + 0xBD78)
#define PPE_SSU_SSU_HOST_ETS_TC_SPEED_SEL_11_REG           (PPE_SSU_BASE + 0xBD7C)
#define PPE_SSU_SSU_HOST_ETS_TC_SPEED_SEL_12_REG           (PPE_SSU_BASE + 0xBD80)
#define PPE_SSU_SSU_HOST_ETS_TC_SPEED_SEL_13_REG           (PPE_SSU_BASE + 0xBD84)
#define PPE_SSU_SSU_HOST_ETS_TC_SPEED_SEL_14_REG           (PPE_SSU_BASE + 0xBD88)
#define PPE_SSU_SSU_HOST_ETS_TC_SPEED_SEL_15_REG           (PPE_SSU_BASE + 0xBD8C)
#define PPE_SSU_SSU_HOST_ETS_TC_SPEED_SEL_16_REG           (PPE_SSU_BASE + 0xBD90)
#define PPE_SSU_SSU_HOST_ETS_TC_SPEED_SEL_17_REG           (PPE_SSU_BASE + 0xBD94)
#define PPE_SSU_SSU_HOST_ETS_TC_SPEED_SEL_18_REG           (PPE_SSU_BASE + 0xBD98)
#define PPE_SSU_SSU_HOST_ETS_TC_SPEED_SEL_19_REG           (PPE_SSU_BASE + 0xBD9C)
#define PPE_SSU_SSU_HOST_ETS_TC_SPEED_SEL_20_REG           (PPE_SSU_BASE + 0xBDA0)
#define PPE_SSU_SSU_HOST_ETS_TC_SPEED_SEL_21_REG           (PPE_SSU_BASE + 0xBDA4)
#define PPE_SSU_SSU_HOST_ETS_TC_SPEED_SEL_22_REG           (PPE_SSU_BASE + 0xBDA8)
#define PPE_SSU_SSU_HOST_ETS_TC_SPEED_SEL_23_REG           (PPE_SSU_BASE + 0xBDAC)
#define PPE_SSU_SSU_HOST_ETS_TC_SPEED_SEL_24_REG           (PPE_SSU_BASE + 0xBDB0)
#define PPE_SSU_SSU_HOST_ETS_TC_SPEED_SEL_25_REG           (PPE_SSU_BASE + 0xBDB4)
#define PPE_SSU_SSU_HOST_ETS_TC_SPEED_SEL_26_REG           (PPE_SSU_BASE + 0xBDB8)
#define PPE_SSU_SSU_HOST_ETS_TC_SPEED_SEL_27_REG           (PPE_SSU_BASE + 0xBDBC)
#define PPE_SSU_SSU_HOST_ETS_TC_SPEED_SEL_28_REG           (PPE_SSU_BASE + 0xBDC0)
#define PPE_SSU_SSU_HOST_ETS_TC_SPEED_SEL_29_REG           (PPE_SSU_BASE + 0xBDC4)
#define PPE_SSU_SSU_HOST_ETS_TC_SPEED_SEL_30_REG           (PPE_SSU_BASE + 0xBDC8)
#define PPE_SSU_SSU_HOST_ETS_TC_SPEED_SEL_31_REG           (PPE_SSU_BASE + 0xBDCC)

/* ETS HOST侧 queue node 级的队列FLUSH的请求信号 */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_FLUSH_REQ_REG           (PPE_SSU_BASE + 0xBE50)
/* ETS HOST侧 queue node 级的队列FLUSH的完成信号 */
#define PPE_SSU_SSU_HOST_ETS_QUEUE_FLUSH_DONE_REG          (PPE_SSU_BASE + 0xBE54)

#define PPE_SSU_SSU_TX_OQ_DROP_THD_0_REG                   (PPE_SSU_BASE + 0xC000) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_THD_1_REG                   (PPE_SSU_BASE + 0xC004) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_THD_2_REG                   (PPE_SSU_BASE + 0xC008) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_THD_3_REG                   (PPE_SSU_BASE + 0xC00C) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_THD_4_REG                   (PPE_SSU_BASE + 0xC010) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_THD_5_REG                   (PPE_SSU_BASE + 0xC014) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_THD_6_REG                   (PPE_SSU_BASE + 0xC018) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_THD_7_REG                   (PPE_SSU_BASE + 0xC01C) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_THD_8_REG                   (PPE_SSU_BASE + 0xC100) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_THD_9_REG                   (PPE_SSU_BASE + 0xC104) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_THD_10_REG                  (PPE_SSU_BASE + 0xC108) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_THD_11_REG                  (PPE_SSU_BASE + 0xC10C) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_THD_12_REG                  (PPE_SSU_BASE + 0xC110) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_THD_13_REG                  (PPE_SSU_BASE + 0xC114) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_THD_14_REG                  (PPE_SSU_BASE + 0xC118) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_THD_15_REG                  (PPE_SSU_BASE + 0xC11C) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_THD_16_REG                  (PPE_SSU_BASE + 0xC200) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_THD_17_REG                  (PPE_SSU_BASE + 0xC204) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_THD_18_REG                  (PPE_SSU_BASE + 0xC208) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_THD_19_REG                  (PPE_SSU_BASE + 0xC20C) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_THD_20_REG                  (PPE_SSU_BASE + 0xC210) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_THD_21_REG                  (PPE_SSU_BASE + 0xC214) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_THD_22_REG                  (PPE_SSU_BASE + 0xC218) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_THD_23_REG                  (PPE_SSU_BASE + 0xC21C) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_THD_24_REG                  (PPE_SSU_BASE + 0xC300) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_THD_25_REG                  (PPE_SSU_BASE + 0xC304) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_THD_26_REG                  (PPE_SSU_BASE + 0xC308) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_THD_27_REG                  (PPE_SSU_BASE + 0xC30C) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_THD_28_REG                  (PPE_SSU_BASE + 0xC310) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_THD_29_REG                  (PPE_SSU_BASE + 0xC314) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_THD_30_REG                  (PPE_SSU_BASE + 0xC318) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_THD_31_REG                  (PPE_SSU_BASE + 0xC31C) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_THD_0_REG                   (PPE_SSU_BASE + 0xC020) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_THD_1_REG                   (PPE_SSU_BASE + 0xC024) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_THD_2_REG                   (PPE_SSU_BASE + 0xC028) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_THD_3_REG                   (PPE_SSU_BASE + 0xC02C) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_THD_4_REG                   (PPE_SSU_BASE + 0xC030) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_THD_5_REG                   (PPE_SSU_BASE + 0xC034) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_THD_6_REG                   (PPE_SSU_BASE + 0xC038) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_THD_7_REG                   (PPE_SSU_BASE + 0xC03C) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_THD_8_REG                   (PPE_SSU_BASE + 0xC120) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_THD_9_REG                   (PPE_SSU_BASE + 0xC124) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_THD_10_REG                  (PPE_SSU_BASE + 0xC128) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_THD_11_REG                  (PPE_SSU_BASE + 0xC12C) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_THD_12_REG                  (PPE_SSU_BASE + 0xC130) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_THD_13_REG                  (PPE_SSU_BASE + 0xC134) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_THD_14_REG                  (PPE_SSU_BASE + 0xC138) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_THD_15_REG                  (PPE_SSU_BASE + 0xC13C) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_THD_16_REG                  (PPE_SSU_BASE + 0xC220) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_THD_17_REG                  (PPE_SSU_BASE + 0xC224) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_THD_18_REG                  (PPE_SSU_BASE + 0xC228) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_THD_19_REG                  (PPE_SSU_BASE + 0xC22C) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_THD_20_REG                  (PPE_SSU_BASE + 0xC230) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_THD_21_REG                  (PPE_SSU_BASE + 0xC234) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_THD_22_REG                  (PPE_SSU_BASE + 0xC238) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_THD_23_REG                  (PPE_SSU_BASE + 0xC23C) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_THD_24_REG                  (PPE_SSU_BASE + 0xC320) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_THD_25_REG                  (PPE_SSU_BASE + 0xC324) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_THD_26_REG                  (PPE_SSU_BASE + 0xC328) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_THD_27_REG                  (PPE_SSU_BASE + 0xC32C) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_THD_28_REG                  (PPE_SSU_BASE + 0xC330) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_THD_29_REG                  (PPE_SSU_BASE + 0xC334) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_THD_30_REG                  (PPE_SSU_BASE + 0xC338) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_THD_31_REG                  (PPE_SSU_BASE + 0xC33C) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_CELL_THD_0_REG              (PPE_SSU_BASE + 0xC040) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_CELL_THD_1_REG              (PPE_SSU_BASE + 0xC044) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_CELL_THD_2_REG              (PPE_SSU_BASE + 0xC048) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_CELL_THD_3_REG              (PPE_SSU_BASE + 0xC04C) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_CELL_THD_4_REG              (PPE_SSU_BASE + 0xC050) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_CELL_THD_5_REG              (PPE_SSU_BASE + 0xC054) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_CELL_THD_6_REG              (PPE_SSU_BASE + 0xC058) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_CELL_THD_7_REG              (PPE_SSU_BASE + 0xC05C) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_CELL_THD_8_REG              (PPE_SSU_BASE + 0xC140) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_CELL_THD_9_REG              (PPE_SSU_BASE + 0xC144) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_CELL_THD_10_REG             (PPE_SSU_BASE + 0xC148) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_CELL_THD_11_REG             (PPE_SSU_BASE + 0xC14C) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_CELL_THD_12_REG             (PPE_SSU_BASE + 0xC150) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_CELL_THD_13_REG             (PPE_SSU_BASE + 0xC154) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_CELL_THD_14_REG             (PPE_SSU_BASE + 0xC158) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_CELL_THD_15_REG             (PPE_SSU_BASE + 0xC15C) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_CELL_THD_16_REG             (PPE_SSU_BASE + 0xC240) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_CELL_THD_17_REG             (PPE_SSU_BASE + 0xC244) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_CELL_THD_18_REG             (PPE_SSU_BASE + 0xC248) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_CELL_THD_19_REG             (PPE_SSU_BASE + 0xC24C) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_CELL_THD_20_REG             (PPE_SSU_BASE + 0xC250) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_CELL_THD_21_REG             (PPE_SSU_BASE + 0xC254) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_CELL_THD_22_REG             (PPE_SSU_BASE + 0xC258) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_CELL_THD_23_REG             (PPE_SSU_BASE + 0xC25C) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_CELL_THD_24_REG             (PPE_SSU_BASE + 0xC340) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_CELL_THD_25_REG             (PPE_SSU_BASE + 0xC344) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_CELL_THD_26_REG             (PPE_SSU_BASE + 0xC348) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_CELL_THD_27_REG             (PPE_SSU_BASE + 0xC34C) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_CELL_THD_28_REG             (PPE_SSU_BASE + 0xC350) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_CELL_THD_29_REG             (PPE_SSU_BASE + 0xC354) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_CELL_THD_30_REG             (PPE_SSU_BASE + 0xC358) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_OQ_DROP_CELL_THD_31_REG             (PPE_SSU_BASE + 0xC35C) /* TX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_CELL_THD_0_REG              (PPE_SSU_BASE + 0xC060) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_CELL_THD_1_REG              (PPE_SSU_BASE + 0xC064) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_CELL_THD_2_REG              (PPE_SSU_BASE + 0xC068) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_CELL_THD_3_REG              (PPE_SSU_BASE + 0xC06C) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_CELL_THD_4_REG              (PPE_SSU_BASE + 0xC070) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_CELL_THD_5_REG              (PPE_SSU_BASE + 0xC074) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_CELL_THD_6_REG              (PPE_SSU_BASE + 0xC078) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_CELL_THD_7_REG              (PPE_SSU_BASE + 0xC07C) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_CELL_THD_8_REG              (PPE_SSU_BASE + 0xC160) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_CELL_THD_9_REG              (PPE_SSU_BASE + 0xC164) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_CELL_THD_10_REG             (PPE_SSU_BASE + 0xC168) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_CELL_THD_11_REG             (PPE_SSU_BASE + 0xC16C) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_CELL_THD_12_REG             (PPE_SSU_BASE + 0xC170) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_CELL_THD_13_REG             (PPE_SSU_BASE + 0xC174) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_CELL_THD_14_REG             (PPE_SSU_BASE + 0xC178) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_CELL_THD_15_REG             (PPE_SSU_BASE + 0xC17C) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_CELL_THD_16_REG             (PPE_SSU_BASE + 0xC260) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_CELL_THD_17_REG             (PPE_SSU_BASE + 0xC264) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_CELL_THD_18_REG             (PPE_SSU_BASE + 0xC268) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_CELL_THD_19_REG             (PPE_SSU_BASE + 0xC26C) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_CELL_THD_20_REG             (PPE_SSU_BASE + 0xC270) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_CELL_THD_21_REG             (PPE_SSU_BASE + 0xC274) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_CELL_THD_22_REG             (PPE_SSU_BASE + 0xC278) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_CELL_THD_23_REG             (PPE_SSU_BASE + 0xC27C) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_CELL_THD_24_REG             (PPE_SSU_BASE + 0xC360) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_CELL_THD_25_REG             (PPE_SSU_BASE + 0xC364) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_CELL_THD_26_REG             (PPE_SSU_BASE + 0xC368) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_CELL_THD_27_REG             (PPE_SSU_BASE + 0xC36C) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_CELL_THD_28_REG             (PPE_SSU_BASE + 0xC370) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_CELL_THD_29_REG             (PPE_SSU_BASE + 0xC374) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_CELL_THD_30_REG             (PPE_SSU_BASE + 0xC378) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_RX_OQ_DROP_CELL_THD_31_REG             (PPE_SSU_BASE + 0xC37C) /* RX出口各队列的丢弃水线 */
#define PPE_SSU_SSU_TX_DUP_OT_PORT_DROP_THD_0_REG          (PPE_SSU_BASE + 0xC080) /* 复制后多播到各MAC的报文水线 */
#define PPE_SSU_SSU_TX_DUP_OT_PORT_DROP_THD_1_REG          (PPE_SSU_BASE + 0xC180) /* 复制后多播到各MAC的报文水线 */
#define PPE_SSU_SSU_TX_DUP_OT_PORT_DROP_THD_2_REG          (PPE_SSU_BASE + 0xC280) /* 复制后多播到各MAC的报文水线 */
#define PPE_SSU_SSU_TX_DUP_OT_PORT_DROP_THD_3_REG          (PPE_SSU_BASE + 0xC380) /* 复制后多播到各MAC的报文水线 */
#define PPE_SSU_SSU_RX_DUP_OT_PORT_DROP_THD_0_REG          (PPE_SSU_BASE + 0xC084) /* 复制后多播到各HOST的报文水线 */
#define PPE_SSU_SSU_RX_DUP_OT_PORT_DROP_THD_1_REG          (PPE_SSU_BASE + 0xC184) /* 复制后多播到各HOST的报文水线 */
#define PPE_SSU_SSU_RX_DUP_OT_PORT_DROP_THD_2_REG          (PPE_SSU_BASE + 0xC284) /* 复制后多播到各HOST的报文水线 */
#define PPE_SSU_SSU_RX_DUP_OT_PORT_DROP_THD_3_REG          (PPE_SSU_BASE + 0xC384) /* 复制后多播到各HOST的报文水线 */
#define PPE_SSU_SSU_TX_OT_PORT_DROP_THD_0_REG              (PPE_SSU_BASE + 0xC088) /* TX出口各端口丢弃水线 */
#define PPE_SSU_SSU_TX_OT_PORT_DROP_THD_1_REG              (PPE_SSU_BASE + 0xC188) /* TX出口各端口丢弃水线 */
#define PPE_SSU_SSU_TX_OT_PORT_DROP_THD_2_REG              (PPE_SSU_BASE + 0xC288) /* TX出口各端口丢弃水线 */
#define PPE_SSU_SSU_TX_OT_PORT_DROP_THD_3_REG              (PPE_SSU_BASE + 0xC388) /* TX出口各端口丢弃水线 */
#define PPE_SSU_SSU_RX_OT_PORT_DROP_THD_0_REG              (PPE_SSU_BASE + 0xC08C) /* RX出口各端口丢弃水线 */
#define PPE_SSU_SSU_RX_OT_PORT_DROP_THD_1_REG              (PPE_SSU_BASE + 0xC18C) /* RX出口各端口丢弃水线 */
#define PPE_SSU_SSU_RX_OT_PORT_DROP_THD_2_REG              (PPE_SSU_BASE + 0xC28C) /* RX出口各端口丢弃水线 */
#define PPE_SSU_SSU_RX_OT_PORT_DROP_THD_3_REG              (PPE_SSU_BASE + 0xC38C) /* RX出口各端口丢弃水线 */
#define PPE_SSU_SSU_TX_DUP_OT_PORT_DROP_CELL_THD_0_REG     (PPE_SSU_BASE + 0xC090) /* 复制后多播到各MAC的CELL水线 */
#define PPE_SSU_SSU_TX_DUP_OT_PORT_DROP_CELL_THD_1_REG     (PPE_SSU_BASE + 0xC190) /* 复制后多播到各MAC的CELL水线 */
#define PPE_SSU_SSU_TX_DUP_OT_PORT_DROP_CELL_THD_2_REG     (PPE_SSU_BASE + 0xC290) /* 复制后多播到各MAC的CELL水线 */
#define PPE_SSU_SSU_TX_DUP_OT_PORT_DROP_CELL_THD_3_REG     (PPE_SSU_BASE + 0xC390) /* 复制后多播到各MAC的CELL水线 */
#define PPE_SSU_SSU_RX_DUP_OT_PORT_DROP_CELL_THD_0_REG     (PPE_SSU_BASE + 0xC094) /* 复制后多播到各HOST的CELL水线 */
#define PPE_SSU_SSU_RX_DUP_OT_PORT_DROP_CELL_THD_1_REG     (PPE_SSU_BASE + 0xC194) /* 复制后多播到各HOST的CELL水线 */
#define PPE_SSU_SSU_RX_DUP_OT_PORT_DROP_CELL_THD_2_REG     (PPE_SSU_BASE + 0xC294) /* 复制后多播到各HOST的CELL水线 */
#define PPE_SSU_SSU_RX_DUP_OT_PORT_DROP_CELL_THD_3_REG     (PPE_SSU_BASE + 0xC394) /* 复制后多播到各HOST的CELL水线 */
#define PPE_SSU_SSU_TX_OT_PORT_CELL_DROP_THD_0_REG         (PPE_SSU_BASE + 0xC098) /* TX出口各端口丢弃水线 */
#define PPE_SSU_SSU_TX_OT_PORT_CELL_DROP_THD_1_REG         (PPE_SSU_BASE + 0xC198) /* TX出口各端口丢弃水线 */
#define PPE_SSU_SSU_TX_OT_PORT_CELL_DROP_THD_2_REG         (PPE_SSU_BASE + 0xC298) /* TX出口各端口丢弃水线 */
#define PPE_SSU_SSU_TX_OT_PORT_CELL_DROP_THD_3_REG         (PPE_SSU_BASE + 0xC398) /* TX出口各端口丢弃水线 */
#define PPE_SSU_SSU_RX_OT_PORT_CELL_DROP_THD_0_REG         (PPE_SSU_BASE + 0xC09C) /* RX出口各端口丢弃水线 */
#define PPE_SSU_SSU_RX_OT_PORT_CELL_DROP_THD_1_REG         (PPE_SSU_BASE + 0xC19C) /* RX出口各端口丢弃水线 */
#define PPE_SSU_SSU_RX_OT_PORT_CELL_DROP_THD_2_REG         (PPE_SSU_BASE + 0xC29C) /* RX出口各端口丢弃水线 */
#define PPE_SSU_SSU_RX_OT_PORT_CELL_DROP_THD_3_REG         (PPE_SSU_BASE + 0xC39C) /* RX出口各端口丢弃水线 */
#define PPE_SSU_SSU_RX_REV_PD_THD_0_REG                    (PPE_SSU_BASE + 0xC0A0) /* 接收方向预留PD配置水线寄存器 */
#define PPE_SSU_SSU_RX_REV_PD_THD_1_REG                    (PPE_SSU_BASE + 0xC1A0) /* 接收方向预留PD配置水线寄存器 */
#define PPE_SSU_SSU_RX_REV_PD_THD_2_REG                    (PPE_SSU_BASE + 0xC2A0) /* 接收方向预留PD配置水线寄存器 */
#define PPE_SSU_SSU_RX_REV_PD_THD_3_REG                    (PPE_SSU_BASE + 0xC3A0) /* 接收方向预留PD配置水线寄存器 */
#define PPE_SSU_SSU_TX_REV_PD_THD_0_REG                    (PPE_SSU_BASE + 0xC0A4) /* 发送方向预留PD配置水线寄存器 */
#define PPE_SSU_SSU_TX_REV_PD_THD_1_REG                    (PPE_SSU_BASE + 0xC1A4) /* 发送方向预留PD配置水线寄存器 */
#define PPE_SSU_SSU_TX_REV_PD_THD_2_REG                    (PPE_SSU_BASE + 0xC2A4) /* 发送方向预留PD配置水线寄存器 */
#define PPE_SSU_SSU_TX_REV_PD_THD_3_REG                    (PPE_SSU_BASE + 0xC3A4) /* 发送方向预留PD配置水线寄存器 */

#endif // __REG_SSU_OFFSET_H__
