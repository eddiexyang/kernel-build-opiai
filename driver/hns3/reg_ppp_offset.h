/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: reg_ppp_offset
 * Author: huawei
 * Create: 2021-12-28
 */

#ifndef __REG_PPP_OFFSET_H__
#define __REG_PPP_OFFSET_H__

/* PPE_PPP Base address of Module's Register */
#define PPE_PPP_BASE                       (0x0)

/******************************************************************************/
/*                      xxx PPE_PPP Registers' Definitions                    */
/******************************************************************************/

#define PPE_PPP_PPP_PART_INT_ENABLE_REG                  (PPE_PPP_BASE + 0x4)    /* PPP模块除MEM外的中断使能寄存器 */
#define PPE_PPP_PPP_MEM_SBIT_ECC_ENABLE_REG              (PPE_PPP_BASE + 0x8)    /* MEM单bit ECC中断使能寄存器 */
#define PPE_PPP_PPP_MEM_MBIT_ECC_ENABLE_REG              (PPE_PPP_BASE + 0xC)    /* MEM多bit ECC中断使能寄存器 */
#define PPE_PPP_PPP_PART_INT_STATUS_REG                  (PPE_PPP_BASE + 0x404)  /* PPP模块除MEM外的原始中断状态寄存器 */
#define PPE_PPP_PPP_MEM_SBIT_ECC_INT_STATUS_REG          (PPE_PPP_BASE + 0x408)  /* MEM单bit ECC原始中断状态寄存器 */
#define PPE_PPP_PPP_MEM_MBIT_ECC_INT_STATUS_REG          (PPE_PPP_BASE + 0x40C)  /* MEM多bit ECC原始中断状态寄存器 */
#define PPE_PPP_PPP_PART_INT_SET_REG                     (PPE_PPP_BASE + 0x804)  /* PPP模块除MEM外的中断注入寄存器 */
#define PPE_PPP_PPP_MEM_SBIT_ECC_REQ_REG                 (PPE_PPP_BASE + 0x808)  /* MEM单bit ECC中断注入寄存器 */
#define PPE_PPP_PPP_MEM_MBIT_ECC_REQ_REG                 (PPE_PPP_BASE + 0x80C)  /* MEM多bit ECC中断注入寄存器 */
/* PPP模块除MEM外的原始中断寄存器（含注入，按bit写1清） */
#define PPE_PPP_PPP_PART_INT_SOURCE_REG                  (PPE_PPP_BASE + 0xC04)
#define PPE_PPP_PPP_MEM_SBIT_ECC_INT_SOURCE_REG          (PPE_PPP_BASE + 0xC08)  /* MEM单bit ECC中断注入寄存器 */
#define PPE_PPP_PPP_MEM_MBIT_ECC_INT_SOURCE_REG          (PPE_PPP_BASE + 0xC0C)  /* MEM多bit ECC中断注入寄存器 */
#define PPE_PPP_PPP_FIFO_UF_INT_INFO_REG                 (PPE_PPP_BASE + 0x1000) /* FIFO下溢时的编号 */
#define PPE_PPP_PPP_FIFO_OF_INT_INFO_REG                 (PPE_PPP_BASE + 0x1004) /* FIFO上溢时的编号 */
#define PPE_PPP_PPP_MEM_SBIT_ERR_INFO_REG                (PPE_PPP_BASE + 0x1008) /* MEM发生1bit ecc错误的错误信息记录寄存器 */
#define PPE_PPP_PPP_MEM_MBIT_ERR_INFO_REG                (PPE_PPP_BASE + 0x100C) /* MEM发生多bit ecc错误的错误信息记录寄存器 */
#define PPE_PPP_PPP_INT_TYPE_REG                         (PPE_PPP_BASE + 0x1400) /* 各种中断的类型 */
#define PPE_PPP_PPP_PART_RINT_CE_REG                     (PPE_PPP_BASE + 0x1C04) /* PPP模块除MEM外的CE中断寄存器 */
#define PPE_PPP_PPP_MEM_SBIT_ECC_RINT_CE_REG             (PPE_PPP_BASE + 0x1C08) /* MEM单bit ECC的CE中断寄存器 */
#define PPE_PPP_PPP_MEM_MBIT_ECC_RINT_CE_REG             (PPE_PPP_BASE + 0x1C0C) /* MEM多bit ECC的CE中断寄存器 */
#define PPE_PPP_PPP_PART_RINT_NFE_REG                    (PPE_PPP_BASE + 0x2004) /* PPP模块除MEM外的NFE中断寄存器 */
#define PPE_PPP_PPP_MEM_SBIT_ECC_RINT_NFE_REG            (PPE_PPP_BASE + 0x2008) /* MEM单bit ECC的NFE中断寄存器 */
#define PPE_PPP_PPP_MEM_MBIT_ECC_RINT_NFE_REG            (PPE_PPP_BASE + 0x200C) /* MEM多bit ECC的NFE中断寄存器 */
#define PPE_PPP_PPP_PART_RINT_FE_REG                     (PPE_PPP_BASE + 0x2404) /* PPP模块除MEM外的FE中断寄存器 */
#define PPE_PPP_PPP_MEM_SBIT_ECC_RINT_FE_REG             (PPE_PPP_BASE + 0x2408) /* MEM单bit ECC的FE中断寄存器 */
#define PPE_PPP_PPP_MEM_MBIT_ECC_RINT_FE_REG             (PPE_PPP_BASE + 0x240C) /* MEM多bit ECC的FE中断寄存器 */
#define PPE_PPP_PPP_MEM_SBIT_ECC_THD_REG                 (PPE_PPP_BASE + 0x2800) /* Mem发生sbit错误时，上报ECC错误中断的阀值 */
#define PPE_PPP_PPP_MEM_SBIT_ERR_CNT_REG                 (PPE_PPP_BASE + 0x2804) /* 单bit ECC统计寄存器 */
#define PPE_PPP_PPP_MEM_MBIT_ERR_CNT_REG                 (PPE_PPP_BASE + 0x2808) /* 2bit ECC统计寄存器 */
#define PPE_PPP_PPP_MEM_INIT_CTRL_REG                    (PPE_PPP_BASE + 0x3000) /* PPP内部MEMORY初始化控制寄存器 */
#define PPE_PPP_PPP_MEM_INIT_STATUS_REG                  (PPE_PPP_BASE + 0x3004) /* PPP内部MEMORY初始化状态寄存器 */
#define PPE_PPP_PPP_SPMEM_CTRL_CFG_REG                   (PPE_PPP_BASE + 0x3008) /* PPP memory控制寄存器 */
#define PPE_PPP_PPP_TPMEM_CTRL_CFG_REG                   (PPE_PPP_BASE + 0x300C) /* PPP memory控制寄存器 */
#define PPE_PPP_PPP_MEM_POWER_CTRL_CFG_REG               (PPE_PPP_BASE + 0x3010) /* PPP memory控制寄存器 */
#define PPE_PPP_PPP_TCAM_CFG_REG                         (PPE_PPP_BASE + 0x3014) /* PPP TCAM配置寄存器 */
#define PPE_PPP_PPP_MODE_CFG_REG                         (PPE_PPP_BASE + 0x3100) /* PPP工作模式寄存器 */
#define PPE_PPP_PPP_CNT_CLR_CE_REG                       (PPE_PPP_BASE + 0x3110) /* 读清和快照使能 */
#define PPE_PPP_PPP_AUTO_GATE_EN_REG                     (PPE_PPP_BASE + 0x3114) /* The Clock Gating enable of PPP */
/* 统计门控打开的窗口大小，单位是64K个该模块工作时钟。 */
#define PPE_PPP_PPP_GATING_WINDOW_REG                    (PPE_PPP_BASE + 0x3118)
/* 统计门控打开的窗口中打开时钟的周期，单位是64K个该模块工作时钟的。 */
#define PPE_PPP_PPP_GATING_STS_REG                       (PPE_PPP_BASE + 0x311C)
#define PPE_PPP_PPP_REQ_FIFO_CFG_REG                     (PPE_PPP_BASE + 0x3120) /* PPP 入口FIFO水线配置 */
#define PPE_PPP_PPP_REQ_WRR_WEIGHT_REG                   (PPE_PPP_BASE + 0x3124) /* PPP 入口MAC和CPU查表请求仲裁权重配置 */
#define PPE_PPP_PPP_GLB_MISC_CFG_REG                     (PPE_PPP_BASE + 0x3130) /* 全局的兼容性配置 */
/* 解决PPP中使用tuple_data生成key的undriven问题 */
#define PPE_PPP_PPP_TUPLE_DATA_DRIVEN_REG                (PPE_PPP_BASE + 0x3134)
#define PPE_PPP_PPP_FD_TCAM_BS_PERIOD_CFG_REG            (PPE_PPP_BASE + 0x3200) /* PPP  FD背景扫描周期 */
#define PPE_PPP_PPP_FD_TCAM_BS_CFG_REG                   (PPE_PPP_BASE + 0x3204) /* PPP  FD背景扫描周期 */
#define PPE_PPP_PPP_TCAM_MODE_REG                        (PPE_PPP_BASE + 0x321C) /* 流表的模式配置 */
#define PPE_PPP_PPP_RSS_TC_MODE_DATA_REG                 (PPE_PPP_BASE + 0x3508) /* RSS TC MODE配置的数据接口 */
#define PPE_PPP_PPP_RSS_TC_MODE_CFG_REG                  (PPE_PPP_BASE + 0x350C) /* RSS TC MODE配置的命令接口 */
#define PPE_PPP_PPP_PKT_TC_OVRD_DATA0_REG                (PPE_PPP_BASE + 0x3510) /* RSS时PKT TC OVRD配置的数据接口 */
#define PPE_PPP_PPP_PKT_TC_OVRD_DATA1_REG                (PPE_PPP_BASE + 0x3514) /* RSS时PKT TC OVRD配置的数据接口 */
#define PPE_PPP_PPP_RSS_IDT_ALLOC_DATA_REG               (PPE_PPP_BASE + 0x3518) /* RSS间接表的分配数据接口 */
#define PPE_PPP_PPP_PKT_TC_OVRD_CFG_REG                  (PPE_PPP_BASE + 0x351C) /* PKT TC OVRD配置的命令接口 */
#define PPE_PPP_PPP_RSS_IDT_DATA_0_REG                   (PPE_PPP_BASE + 0x3540) /* 间接表表配置的数据接口 */
#define PPE_PPP_PPP_RSS_IDT_DATA_1_REG                   (PPE_PPP_BASE + 0x3544) /* 间接表表配置的数据接口 */
#define PPE_PPP_PPP_RSS_IDT_DATA_2_REG                   (PPE_PPP_BASE + 0x3548) /* 间接表表配置的数据接口 */
#define PPE_PPP_PPP_RSS_IDT_DATA_3_REG                   (PPE_PPP_BASE + 0x354C) /* 间接表表配置的数据接口 */
#define PPE_PPP_PPP_RSS_IDT_DATA_4_REG                   (PPE_PPP_BASE + 0x3550) /* 间接表表配置的数据接口 */
#define PPE_PPP_PPP_RSS_IDT_DATA_5_REG                   (PPE_PPP_BASE + 0x3554) /* 间接表表配置的数据接口 */
#define PPE_PPP_PPP_RSS_IDT_DATA_6_REG                   (PPE_PPP_BASE + 0x3558) /* 间接表表配置的数据接口 */
#define PPE_PPP_PPP_RSS_IDT_DATA_7_REG                   (PPE_PPP_BASE + 0x355C) /* 间接表表配置的数据接口 */
#define PPE_PPP_PPP_RSS_IDT_DATA_8_REG                   (PPE_PPP_BASE + 0x3560) /* 间接表表配置的数据接口 */
#define PPE_PPP_PPP_RSS_IDT_DATA_9_REG                   (PPE_PPP_BASE + 0x3564) /* 间接表表配置的数据接口 */
#define PPE_PPP_PPP_RSS_IDT_DATA_10_REG                  (PPE_PPP_BASE + 0x3568) /* 间接表表配置的数据接口 */
#define PPE_PPP_PPP_RSS_IDT_DATA_11_REG                  (PPE_PPP_BASE + 0x356C) /* 间接表表配置的数据接口 */
#define PPE_PPP_PPP_RSS_IDT_DATA_12_REG                  (PPE_PPP_BASE + 0x3570) /* 间接表表配置的数据接口 */
#define PPE_PPP_PPP_RSS_IDT_DATA_13_REG                  (PPE_PPP_BASE + 0x3574) /* 间接表表配置的数据接口 */
#define PPE_PPP_PPP_RSS_IDT_DATA_14_REG                  (PPE_PPP_BASE + 0x3578) /* 间接表表配置的数据接口 */
#define PPE_PPP_PPP_RSS_IDT_DATA_15_REG                  (PPE_PPP_BASE + 0x357C) /* 间接表表配置的数据接口 */
#define PPE_PPP_PPP_RSS_IDT_CFG_REG                      (PPE_PPP_BASE + 0x3580) /* 间接表表配置的命令接口 */
/* PPP RSS_TYPE 非IP写数据，此寄存器与PPP_RSS_HASH_CFG相关 */
#define PPE_PPP_PPP_RSS_TYPE_NIP_REG                     (PPE_PPP_BASE + 0x3590)
/* PPP RSS_TYPE IPV4 写数据，此寄存器与PPP_RSS_HASH_CFG相关 */
#define PPE_PPP_PPP_RSS_TYPE_IPV4_REG                    (PPE_PPP_BASE + 0x3594)
/* PPP RSS_TYPE IPV6 写数据，此寄存器与PPP_RSS_HASH_CFG相关 */
#define PPE_PPP_PPP_RSS_TYPE_IPV6_REG                    (PPE_PPP_BASE + 0x3598)

/* RSS HASH key种子配置，此寄存器与PPP_RSS_HASH_CFG相关 */
#define PPE_PPP_PPP_RSS_HASH_KEY_0_REG                   (PPE_PPP_BASE + 0x359C)
#define PPE_PPP_PPP_RSS_HASH_KEY_1_REG                   (PPE_PPP_BASE + 0x35A0)
#define PPE_PPP_PPP_RSS_HASH_KEY_2_REG                   (PPE_PPP_BASE + 0x35A4)
#define PPE_PPP_PPP_RSS_HASH_KEY_3_REG                   (PPE_PPP_BASE + 0x35A8)
#define PPE_PPP_PPP_RSS_HASH_KEY_4_REG                   (PPE_PPP_BASE + 0x35AC)
#define PPE_PPP_PPP_RSS_HASH_KEY_5_REG                   (PPE_PPP_BASE + 0x35B0)
#define PPE_PPP_PPP_RSS_HASH_KEY_6_REG                   (PPE_PPP_BASE + 0x35B4)
#define PPE_PPP_PPP_RSS_HASH_KEY_7_REG                   (PPE_PPP_BASE + 0x35B8)
#define PPE_PPP_PPP_RSS_HASH_KEY_8_REG                   (PPE_PPP_BASE + 0x35BC)
#define PPE_PPP_PPP_RSS_HASH_KEY_9_REG                   (PPE_PPP_BASE + 0x35C0)
/* HASH algorithm selection，此寄存器与PPP_RSS_HASH_CFG相关 */
#define PPE_PPP_PPP_RSS_HASH_ALGO_REG                    (PPE_PPP_BASE + 0x35C4)
#define PPE_PPP_PPP_RSS_HASH_CFG_REG                     (PPE_PPP_BASE + 0x35C8) /* RSS HASH 操作 */
#define PPE_PPP_PPP_FD_KEY_TEMPL_DATA_0_REG              (PPE_PPP_BASE + 0x3900) /* 模板匹配表配置key数据 */
#define PPE_PPP_PPP_FD_KEY_TEMPL_DATA_1_REG              (PPE_PPP_BASE + 0x3904) /* 模板匹配表配置key数据 */
#define PPE_PPP_PPP_FD_KEY_TEMPL_DATA_2_REG              (PPE_PPP_BASE + 0x3908) /* 模板匹配表配置key数据 */
#define PPE_PPP_PPP_FD_KEY_TEMPL_DATA_3_REG              (PPE_PPP_BASE + 0x390C) /* 模板匹配表配置关联数据 */
#define PPE_PPP_PPP_FD_KEY_TEMPL_CFG_REG                 (PPE_PPP_BASE + 0x3910) /* PPP  FD KEY模板匹配表操作命令 */
#define PPE_PPP_PPP_FD_KEY_SEL_TDATA_0_REG               (PPE_PPP_BASE + 0x3A00) /* PPP  FD 每个模板的KEY选择表数据 */
#define PPE_PPP_PPP_FD_KEY_SEL_TDATA_1_REG               (PPE_PPP_BASE + 0x3A04) /* PPP  FD 每个模板的KEY选择表数据 */
#define PPE_PPP_PPP_FD_KEY_SEL_MDATA_REG                 (PPE_PPP_BASE + 0x3A14) /* PPP  FD 每个模板的KEY选择表数据 */
#define PPE_PPP_PPP_FD_KEY_SEL_CFG_REG                   (PPE_PPP_BASE + 0x3A18) /* 模板key配置操作 */
#define PPE_PPP_PPP_FD_KEY_DATA_0_REG                    (PPE_PPP_BASE + 0x3B00) /* PPP  FD TCAM数据 */
#define PPE_PPP_PPP_FD_KEY_DATA_1_REG                    (PPE_PPP_BASE + 0x3B04) /* PPP  FD TCAM数据 */
#define PPE_PPP_PPP_FD_KEY_DATA_2_REG                    (PPE_PPP_BASE + 0x3B08) /* PPP  FD TCAM数据 */
#define PPE_PPP_PPP_FD_KEY_DATA_3_REG                    (PPE_PPP_BASE + 0x3B0C) /* PPP  FD TCAM数据 */
#define PPE_PPP_PPP_FD_KEY_DATA_4_REG                    (PPE_PPP_BASE + 0x3B10) /* PPP  FD TCAM数据 */
#define PPE_PPP_PPP_FD_KEY_DATA_5_REG                    (PPE_PPP_BASE + 0x3B14) /* PPP  FD TCAM数据 */
#define PPE_PPP_PPP_FD_KEY_DATA_6_REG                    (PPE_PPP_BASE + 0x3B18) /* PPP  FD TCAM数据 */
#define PPE_PPP_PPP_FD_KEY_DATA_7_REG                    (PPE_PPP_BASE + 0x3B1C) /* PPP  FD TCAM数据 */
#define PPE_PPP_PPP_FD_KEY_DATA_8_REG                    (PPE_PPP_BASE + 0x3B20) /* PPP  FD TCAM数据 */
#define PPE_PPP_PPP_FD_KEY_DATA_9_REG                    (PPE_PPP_BASE + 0x3B24) /* PPP  FD TCAM数据 */
#define PPE_PPP_PPP_FD_KEY_DATA_10_REG                   (PPE_PPP_BASE + 0x3B28) /* PPP  FD TCAM数据 */
#define PPE_PPP_PPP_FD_KEY_DATA_11_REG                   (PPE_PPP_BASE + 0x3B2C) /* PPP  FD TCAM数据 */
#define PPE_PPP_PPP_FD_KEY_DATA_MSB_REG                  (PPE_PPP_BASE + 0x3B30) /* PPP  FD TCAM数据最高16bit */
#define PPE_PPP_PPP_FD_KEY_CFG_REG                       (PPE_PPP_BASE + 0x3B34) /* PPP  FD TCAM配置操作命令 */
#define PPE_PPP_PPP_FD_STATUS_REG                        (PPE_PPP_BASE + 0x3B38) /* 流表操作后的状态 */
#define PPE_PPP_PPP_ING_VLAN_DATA0_REG                   (PPE_PPP_BASE + 0x4200) /* LSW 入口处理配置 */
#define PPE_PPP_PPP_ING_VLAN_DATA1_REG                   (PPE_PPP_BASE + 0x4204) /* LSW 入口处理配置 */
#define PPE_PPP_PPP_ING_VLAN_CFG_REG                     (PPE_PPP_BASE + 0x4210) /* LSW入口端口属性操作命令 */
#define PPE_PPP_PPP_UP_MAPPING_DATA_0_REG                (PPE_PPP_BASE + 0x4240) /* CPU侧到端口的UP_MAPPING配置 */
#define PPE_PPP_PPP_UP_MAPPING_DATA_1_REG                (PPE_PPP_BASE + 0x4244) /* CPU侧到端口的UP_MAPPING配置 */
#define PPE_PPP_PPP_UP_MAPPING_DATA_2_REG                (PPE_PPP_BASE + 0x4248) /* CPU侧到端口的UP_MAPPING配置 */
#define PPE_PPP_PPP_UP_MAPPING_DATA_3_REG                (PPE_PPP_BASE + 0x424C) /* CPU侧到端口的UP_MAPPING配置 */
#define PPE_PPP_PPP_UP_MAPPING_DATA_4_REG                (PPE_PPP_BASE + 0x4250) /* CPU侧到端口的UP_MAPPING配置 */
#define PPE_PPP_PPP_UP_MAPPING_DATA_5_REG                (PPE_PPP_BASE + 0x4254) /* CPU侧到端口的UP_MAPPING配置 */
#define PPE_PPP_PPP_UP_MAPPING_DATA_6_REG                (PPE_PPP_BASE + 0x4258) /* CPU侧到端口的UP_MAPPING配置 */
#define PPE_PPP_PPP_UP_MAPPING_DATA_7_REG                (PPE_PPP_BASE + 0x425C) /* CPU侧到端口的UP_MAPPING配置 */
#define PPE_PPP_PPP_UP_MAPPING_CFG_REG                   (PPE_PPP_BASE + 0x4260) /* CPU侧到端口的UP_MAPPING配置操作命令 */
#define PPE_PPP_PPP_UP_I2O_MAPPING_DATA_0_REG            (PPE_PPP_BASE + 0x4270) /* 芯片内部到外部的UP_MAPPING配置 */
#define PPE_PPP_PPP_UP_I2O_MAPPING_DATA_1_REG            (PPE_PPP_BASE + 0x4274) /* 芯片内部到外部的UP_MAPPING配置 */
#define PPE_PPP_PPP_UP_I2O_MAPPING_DATA_2_REG            (PPE_PPP_BASE + 0x4278) /* 芯片内部到外部的UP_MAPPING配置 */
#define PPE_PPP_PPP_UP_I2O_MAPPING_DATA_3_REG            (PPE_PPP_BASE + 0x427C) /* 芯片内部到外部的UP_MAPPING配置 */
#define PPE_PPP_PPP_PPS_CAR_CFG_0_0_REG                  (PPE_PPP_BASE + 0x4300) /* PPS 桶速率的配置。 */
#define PPE_PPP_PPP_PPS_CAR_CFG_0_1_REG                  (PPE_PPP_BASE + 0x4304) /* PPS 桶速率的配置。 */
#define PPE_PPP_PPP_PPS_CAR_CFG_0_2_REG                  (PPE_PPP_BASE + 0x4308) /* PPS 桶速率的配置。 */
#define PPE_PPP_PPP_PPS_CAR_CFG_0_3_REG                  (PPE_PPP_BASE + 0x4380) /* PPS 桶速率的配置。 */
#define PPE_PPP_PPP_PPS_CAR_CFG_0_4_REG                  (PPE_PPP_BASE + 0x4384) /* PPS 桶速率的配置。 */
#define PPE_PPP_PPP_PPS_CAR_CFG_0_5_REG                  (PPE_PPP_BASE + 0x4388) /* PPS 桶速率的配置。 */
#define PPE_PPP_PPP_PPS_CAR_CFG_0_6_REG                  (PPE_PPP_BASE + 0x4400) /* PPS 桶速率的配置。 */
#define PPE_PPP_PPP_PPS_CAR_CFG_0_7_REG                  (PPE_PPP_BASE + 0x4404) /* PPS 桶速率的配置。 */
#define PPE_PPP_PPP_PPS_CAR_CFG_0_8_REG                  (PPE_PPP_BASE + 0x4408) /* PPS 桶速率的配置。 */
#define PPE_PPP_PPP_PPS_CAR_CFG_0_9_REG                  (PPE_PPP_BASE + 0x4480) /* PPS 桶速率的配置。 */
#define PPE_PPP_PPP_PPS_CAR_CFG_0_10_REG                 (PPE_PPP_BASE + 0x4484) /* PPS 桶速率的配置。 */
#define PPE_PPP_PPP_PPS_CAR_CFG_0_11_REG                 (PPE_PPP_BASE + 0x4488) /* PPS 桶速率的配置。 */
#define PPE_PPP_PPP_PPS_CAR_CFG_1_0_REG                  (PPE_PPP_BASE + 0x4310) /* PPS 桶深度的配置。 */
#define PPE_PPP_PPP_PPS_CAR_CFG_1_1_REG                  (PPE_PPP_BASE + 0x4314) /* PPS 桶深度的配置。 */
#define PPE_PPP_PPP_PPS_CAR_CFG_1_2_REG                  (PPE_PPP_BASE + 0x4318) /* PPS 桶深度的配置。 */
#define PPE_PPP_PPP_PPS_CAR_CFG_1_3_REG                  (PPE_PPP_BASE + 0x4390) /* PPS 桶深度的配置。 */
#define PPE_PPP_PPP_PPS_CAR_CFG_1_4_REG                  (PPE_PPP_BASE + 0x4394) /* PPS 桶深度的配置。 */
#define PPE_PPP_PPP_PPS_CAR_CFG_1_5_REG                  (PPE_PPP_BASE + 0x4398) /* PPS 桶深度的配置。 */
#define PPE_PPP_PPP_PPS_CAR_CFG_1_6_REG                  (PPE_PPP_BASE + 0x4410) /* PPS 桶深度的配置。 */
#define PPE_PPP_PPP_PPS_CAR_CFG_1_7_REG                  (PPE_PPP_BASE + 0x4414) /* PPS 桶深度的配置。 */
#define PPE_PPP_PPP_PPS_CAR_CFG_1_8_REG                  (PPE_PPP_BASE + 0x4418) /* PPS 桶深度的配置。 */
#define PPE_PPP_PPP_PPS_CAR_CFG_1_9_REG                  (PPE_PPP_BASE + 0x4490) /* PPS 桶深度的配置。 */
#define PPE_PPP_PPP_PPS_CAR_CFG_1_10_REG                 (PPE_PPP_BASE + 0x4494) /* PPS 桶深度的配置。 */
#define PPE_PPP_PPP_PPS_CAR_CFG_1_11_REG                 (PPE_PPP_BASE + 0x4498) /* PPS 桶深度的配置。 */
#define PPE_PPP_PPP_PPS_CAR_CFG_2_0_REG                  (PPE_PPP_BASE + 0x4320) /* PPS中操作配置 */
#define PPE_PPP_PPP_PPS_CAR_CFG_2_1_REG                  (PPE_PPP_BASE + 0x4324) /* PPS中操作配置 */
#define PPE_PPP_PPP_PPS_CAR_CFG_2_2_REG                  (PPE_PPP_BASE + 0x4328) /* PPS中操作配置 */
#define PPE_PPP_PPP_PPS_CAR_CFG_2_3_REG                  (PPE_PPP_BASE + 0x43A0) /* PPS中操作配置 */
#define PPE_PPP_PPP_PPS_CAR_CFG_2_4_REG                  (PPE_PPP_BASE + 0x43A4) /* PPS中操作配置 */
#define PPE_PPP_PPP_PPS_CAR_CFG_2_5_REG                  (PPE_PPP_BASE + 0x43A8) /* PPS中操作配置 */
#define PPE_PPP_PPP_PPS_CAR_CFG_2_6_REG                  (PPE_PPP_BASE + 0x4420) /* PPS中操作配置 */
#define PPE_PPP_PPP_PPS_CAR_CFG_2_7_REG                  (PPE_PPP_BASE + 0x4424) /* PPS中操作配置 */
#define PPE_PPP_PPP_PPS_CAR_CFG_2_8_REG                  (PPE_PPP_BASE + 0x4428) /* PPS中操作配置 */
#define PPE_PPP_PPP_PPS_CAR_CFG_2_9_REG                  (PPE_PPP_BASE + 0x44A0) /* PPS中操作配置 */
#define PPE_PPP_PPP_PPS_CAR_CFG_2_10_REG                 (PPE_PPP_BASE + 0x44A4) /* PPS中操作配置 */
#define PPE_PPP_PPP_PPS_CAR_CFG_2_11_REG                 (PPE_PPP_BASE + 0x44A8) /* PPS中操作配置 */
#define PPE_PPP_PPP_EGR_PORT_ATTR_REG                    (PPE_PPP_BASE + 0x4A00) /* 出口属性配置 */
#define PPE_PPP_PPP_EGR_PORT_ATTR_CFG_REG                (PPE_PPP_BASE + 0x4A04) /* 出口属性配置命令操作 */
#define PPE_PPP_PPP_MPORT_MISC_CFG_0_REG                 (PPE_PPP_BASE + 0x5600) /* MAC侧端口杂项配置 */
#define PPE_PPP_PPP_MPORT_MISC_CFG_1_REG                 (PPE_PPP_BASE + 0x5604) /* MAC侧端口杂项配置 */
#define PPE_PPP_PPP_MPORT_MISC_CFG_2_REG                 (PPE_PPP_BASE + 0x5608) /* MAC侧端口杂项配置 */
#define PPE_PPP_PPP_MPORT_MISC_CFG_3_REG                 (PPE_PPP_BASE + 0x560C) /* MAC侧端口杂项配置 */
#define PPE_PPP_PPP_UP_MAP_CFG_0_REG                     (PPE_PPP_BASE + 0x5800) /* up_mapping的选择信号配置 */
#define PPE_PPP_PPP_UP_MAP_CFG_1_REG                     (PPE_PPP_BASE + 0x5804) /* up_mapping的选择信号配置 */
#define PPE_PPP_PPP_UP_MAP_CFG_2_REG                     (PPE_PPP_BASE + 0x5808) /* up_mapping的选择信号配置 */
#define PPE_PPP_PPP_UP_MAP_CFG_3_REG                     (PPE_PPP_BASE + 0x580C) /* up_mapping的选择信号配置 */
#define PPE_PPP_PPP_TNL_MISC_CFG_0_REG                   (PPE_PPP_BASE + 0x5A00) /* CPU侧端口杂项配置（POE模式only） */
#define PPE_PPP_PPP_TNL_MISC_CFG_1_REG                   (PPE_PPP_BASE + 0x5A04) /* CPU侧端口杂项配置（POE模式only） */
#define PPE_PPP_PPP_TNL_MISC_CFG_2_REG                   (PPE_PPP_BASE + 0x5A08) /* CPU侧端口杂项配置（POE模式only） */
#define PPE_PPP_PPP_TNL_MISC_CFG_3_REG                   (PPE_PPP_BASE + 0x5A0C) /* CPU侧端口杂项配置（POE模式only） */
#define PPE_PPP_PPP_ECO0_REG                             (PPE_PPP_BASE + 0x6000) /* PPP ECO预留寄存器 */
#define PPE_PPP_PPP_ECO1_REG                             (PPE_PPP_BASE + 0x6004) /* PPP ECO预留寄存器 */
#define PPE_PPP_PPP_ECO2_REG                             (PPE_PPP_BASE + 0x6008) /* PPP ECO预留寄存器 */
#define PPE_PPP_PPP_ECO3_REG                             (PPE_PPP_BASE + 0x600C) /* PPP ECO预留寄存器 */
#define PPE_PPP_PPP_ING_RX_PKT_CNT_REG                   (PPE_PPP_BASE + 0x7000) /* 入口SSU到PPP的RX侧统计 */
#define PPE_PPP_PPP_ING_TX_PKT_CNT_REG                   (PPE_PPP_BASE + 0x7008) /* 入口SSU到PPP的TX侧统计 */
#define PPE_PPP_PPP_LSW_ING_VLAN_HANDLE_DROP_PKT_CNT_REG (PPE_PPP_BASE + 0x71B0) /* 入口vlan处理丢弃报文 */
#define PPE_PPP_PPP_FD_CNT_CFG_REG                       (PPE_PPP_BASE + 0x7224) /* 流表命中的流统计 */
#define PPE_PPP_PPP_FD_CNT_REG                           (PPE_PPP_BASE + 0x7228) /* 流表命中的流统计 */
#define PPE_PPP_PPP_DROP_CNT_REG                         (PPE_PPP_BASE + 0x7400) /* 被car过滤的报文统计 */
#define PPE_PPP_PPP_FW_FD_BYPASS_PKT_CNT_REG             (PPE_PPP_BASE + 0x7530) /* 分发流表bypass统计 */
#define PPE_PPP_PPP_FW_FD_STG_1_HIT_PKT_CNT_REG          (PPE_PPP_BASE + 0x7538) /* 分发流表1级命中统计 */
#define PPE_PPP_PPP_FW_FD_STG_1_MISS_PKT_CNT_REG         (PPE_PPP_BASE + 0x7548) /* 分发流表1级不命中统计 */
#define PPE_PPP_PPP_FW_FD_STG_1_HIT_DROP_PKT_CNT_REG     (PPE_PPP_BASE + 0x7558) /* 分发流表1级命中结果为丢弃个数 */
#define PPE_PPP_PPP_ING_FIFO_STATUS_REG                  (PPE_PPP_BASE + 0x7704) /* LSW 入口FIFO状态 */
#define PPE_PPP_PPP_RLT_STATUS_CFG_REG                   (PPE_PPP_BASE + 0x7720) /* PPP送SSU最后一次查表结果状态 */
#define PPE_PPP_PPP_RLT_STATUS_REG                       (PPE_PPP_BASE + 0x7724) /* PPP送SSU最后一次查表结果状态 */
#define PPE_PPP_PPP_FW_FD_STATUS_CFG_REG                 (PPE_PPP_BASE + 0x7764) /* 分发流表状态配置 */
#define PPE_PPP_PPP_FW_FD_STATUS_REG                     (PPE_PPP_BASE + 0x7768) /* 分发流表的key */
#define PPE_PPP_PPP_FW_FD_HIT_STATUS_REG                 (PPE_PPP_BASE + 0x776C) /* 分发流表命中的entry */
#define PPE_PPP_PPP_FW_TNL_CNT_CFG_REG                   (PPE_PPP_BASE + 0x7804) /* 选择哪个FW_TNL计数 */
#define PPE_PPP_PPP_FW_TNL_PKT_CNT_REG                   (PPE_PPP_BASE + 0x7810) /* 配置统计的FW_TNL收到的报文 */

#endif // __REG_PPP_OFFSET_H__
