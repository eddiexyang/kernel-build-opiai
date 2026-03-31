/*
 * Copyright     :  Copyright (C) 2021, Huawei Technologies Co. Ltd.
 * File name     :  mata_reg_offset.h
 * Project line  :  Platform And crux Technologies Development
 * Department    :  CAD Development Department
 * Author        :  xxx
 * Version       :  1.0
 * Date          :  2019/05/01
 * Description   :  The description of xxx project
 * Others        :  Generated automatically by nManager V5.1
 * History       :  xxx 2021/11/05 18:21:39 Create file
 */

#ifndef __MATA_REG_OFFSET_H__
#define __MATA_REG_OFFSET_H__

/* MATA Base address of Module's Register */
#define MATA_BASE                       (0x0)

/******************************************************************************/
/*                      P688 MATA Registers' Definitions                            */
/******************************************************************************/

#define MATA_MATA_CTRL_REG               (MATA_BASE + 0x0)    /* MATA控制寄存器 */
#define MATA_MATA_CC_CTRL_REG            (MATA_BASE + 0x4)    /* MATA一致性控制寄存器 */
#define MATA_MATA_DIR_CTRL_REG           (MATA_BASE + 0x8)    /* MATA Dir控制寄存器 */
#define MATA_MATA_FUNC_DIS_REG           (MATA_BASE + 0xC)    /* MATA功能关闭寄存器 */
#define MATA_MATA_TRANSBUS_CTRL_REG      (MATA_BASE + 0x10)   /* MATA和L4D之间通路的调度控制寄存器 */
#define MATA_MATA_LUC_CTRL_REG           (MATA_BASE + 0x14)   /* LUC控制寄存器（静态配置） */
#define MATA_MATA_ATOMIC_CTRL_REG        (MATA_BASE + 0x1C)   /* MATA内Atomic行为控制寄存器 */
#define MATA_MATA_INTLV_REG              (MATA_BASE + 0x20)   /* MATA内部交织配置寄存器 */
#define MATA_MATA_LUC_PERSIST_WAYL_REG   (MATA_BASE + 0x24)   /* MATA内LUC的way分配给persist的配置寄存器 */
#define MATA_MATA_LUC_PERSIST_WAYH_REG   (MATA_BASE + 0x28)   /* MATA内LUC的way分配给persist的配置寄存器 */
#define MATA_MATA_CXLDAW_EN_REG          (MATA_BASE + 0x2C)   /* CXL DAW窗口使能寄存器。(Hi1980CV100未使用） */
#define MATA_MATA_CXLDAW0_ADDR_REG       (MATA_BASE + 0x30)   /* CXL DAW0起始地址配置寄存器。(Hi1980CV100未使用） */
#define MATA_MATA_CXLDAW0_SIZEID_REG     (MATA_BASE + 0x34)   /* CXL DAW0窗口大小和目的配置寄存器。(Hi1980CV100未使用） */
#define MATA_MATA_CXLDAW1_ADDR_REG       (MATA_BASE + 0x38)   /* CXL DAW1起始地址配置寄存器。 */
#define MATA_MATA_CXLDAW1_SIZEID_REG     (MATA_BASE + 0x3C)   /* CXL DAW1窗口大小和目的配置寄存器。(Hi1980CV100未使用） */
#define MATA_MATA_CXLDAW2_ADDR_REG       (MATA_BASE + 0x40)   /* CXL DAW2起始地址配置寄存器。(Hi1980CV100未使用） */
#define MATA_MATA_CXLDAW2_SIZEID_REG     (MATA_BASE + 0x44)   /* CXL DAW2窗口大小和目的配置寄存器。 */
#define MATA_MATA_CXLDAW3_ADDR_REG       (MATA_BASE + 0x48)   /* CXL DAW3起始地址配置寄存器。(Hi1980CV100未使用） */
#define MATA_MATA_CXLDAW3_SIZEID_REG     (MATA_BASE + 0x4C)   /* CXL DAW3窗口大小和目的配置寄存器。(Hi1980CV100未使用） */
#define MATA_MATA_DMABW_CTRL0_REG        (MATA_BASE + 0x50)   /* SDMA第一级带宽限制配置寄存器 */
#define MATA_MATA_DMABW_CTRL1_REG        (MATA_BASE + 0x54)   /* SDMA第二级带宽限制配置寄存器 */
#define MATA_MATA_TRANSTXREQ_CTRL_REG    (MATA_BASE + 0x58)   /* sdma以及CXL/PG的请求汇聚到txreq通道的调度控制器 */
#define MATA_MATA_EXTER_IOCACHE_EN_REG   (MATA_BASE + 0x5C)   /* MATA片外IOCACHE使能寄存器 */
#define MATA_MATA_MBIST_CTRL_REG         (MATA_BASE + 0x80)   /* MATA_MBIST_CTRL为配置MBIST_REQ的寄存器器 */
#define MATA_MATA_INIT_CTRL_REG          (MATA_BASE + 0x404)  /* MATA初始化控制寄存器 */
#define MATA_MATA_FLOW_MODE_REG          (MATA_BASE + 0x408)  /* MATA流控配置寄存器 */
#define MATA_MATA_SEC_CTRL_REG           (MATA_BASE + 0x500)  /* MATA安全控制寄存器地址交织配置寄存器 */
#define MATA_MATA_SEC_ADDR0_REG          (MATA_BASE + 0x510)  /* MATA安全空间地址寄存器地址交织配置寄存器 */
#define MATA_MATA_SEC_ADDR1_REG          (MATA_BASE + 0x514)  /* MATA安全空间地址寄存器地址交织配置寄存器 */
#define MATA_MATA_SEC_ADDR2_REG          (MATA_BASE + 0x518)  /* MATA安全空间地址寄存器地址交织配置寄存器 */
#define MATA_MATA_SEC_ADDR3_REG          (MATA_BASE + 0x51C)  /* MATA安全空间地址寄存器地址交织配置寄存器 */
#define MATA_MATA_SEC_ADDR4_REG          (MATA_BASE + 0x520)  /* MATA安全空间地址寄存器地址交织配置寄存器 */
#define MATA_MATA_SEC_ADDR5_REG          (MATA_BASE + 0x524)  /* MATA安全空间地址寄存器地址交织配置寄存器 */
#define MATA_MATA_SEC_ADDR6_REG          (MATA_BASE + 0x528)  /* MATA安全空间地址寄存器地址交织配置寄存器 */
#define MATA_MATA_SEC_ADDR7_REG          (MATA_BASE + 0x52C)  /* MATA安全空间地址寄存器地址交织配置寄存器 */
#define MATA_MATA_SEC_SIZE0_REG          (MATA_BASE + 0x530)  /* MATA安全空间范围寄存器地址交织配置寄存器 */
#define MATA_MATA_SEC_SIZE1_REG          (MATA_BASE + 0x534)  /* MATA安全空间范围寄存器地址交织配置寄存器 */
#define MATA_MATA_SEC_SIZE2_REG          (MATA_BASE + 0x538)  /* MATA安全空间范围寄存器地址交织配置寄存器 */
#define MATA_MATA_SEC_SIZE3_REG          (MATA_BASE + 0x53C)  /* MATA安全空间范围寄存器地址交织配置寄存器 */
#define MATA_MATA_SEC_SIZE4_REG          (MATA_BASE + 0x540)  /* MATA安全空间范围寄存器地址交织配置寄存器 */
#define MATA_MATA_SEC_SIZE5_REG          (MATA_BASE + 0x544)  /* MATA安全空间范围寄存器地址交织配置寄存器 */
#define MATA_MATA_SEC_SIZE6_REG          (MATA_BASE + 0x548)  /* MATA安全空间范围寄存器地址交织配置寄存器 */
#define MATA_MATA_SEC_SIZE7_REG          (MATA_BASE + 0x54C)  /* MATA安全空间范围寄存器地址交织配置寄存器 */
#define MATA_MATA_TXDAT_DET_TIME_REG     (MATA_BASE + 0x588)  /* 流量检查对应时间配置寄存器 */
#define MATA_MATA_TXDAT_PUSH_TIME_REG    (MATA_BASE + 0x58C)  /* 流量PUSH对应时间配置寄存器 */
#define MATA_MATA_DDR_LEVEL_REG          (MATA_BASE + 0x590)  /* MATA中DDR相关水线配置寄存器 */
#define MATA_MATA_ATOMIC_TH0_REG         (MATA_BASE + 0x598)  /* MATA Atomic monitor水线配置寄存器 */
#define MATA_MATA_SPILL_TH_REG           (MATA_BASE + 0x59C)  /* MATA Spill outstanding配置寄存器 */
#define MATA_MATA_UTL_TH_REG             (MATA_BASE + 0x5A0)  /* MATA队列利用率水线配置寄存器 */
#define MATA_MATA_DDR_BW_REG             (MATA_BASE + 0x5A4)  /* MATA DDR带宽控制寄存器 */
#define MATA_MATA_CMDSCH_TH_REG          (MATA_BASE + 0x5A8)  /* MATA命令调度水线配置寄存器 */
#define MATA_MATA_ATOMIC_TH1_REG         (MATA_BASE + 0x5AC)  /* MATA Atomic monitor水线配置寄存器 */
#define MATA_MATA_COMPDBID_COMB_REG      (MATA_BASE + 0x5B0)  /* MATA的comp和dbid合并配置寄存器 */
#define MATA_MATA_MPAM_CTRL_REG          (MATA_BASE + 0x5B4)  /* MATA MPAM控制配置寄存器(静态配置) */
#define MATA_MATA_RDATA_BYP_CTRL_REG     (MATA_BASE + 0x5B8)  /* DMC ACTIVE & BYPASS控制配置寄存器(静态配置) */
#define MATA_MATA_LUC_MST_PORTID_REG     (MATA_BASE + 0x5BC)  /* 可使用 L2 CACHE 的master的PORTID的配置寄存器（Hi1980CV100未使用此寄存器） */
#define MATA_MATA_ATOMIC_CALC_CTRL_REG   (MATA_BASE + 0x5DC)  /* MATA atomic运算控制寄存器 */
#define MATA_MATA_ECC_INJECT_REG         (MATA_BASE + 0x600)  /* MATA ECC错误注入寄存器 */
#define MATA_MATA_QOS_CTRL_REG           (MATA_BASE + 0x604)  /* MATA的QOS的控制寄存器（AUTOQOS相关寄存器在Hi1980CV100未使用） */
#define MATA_MATA_QOS_CFG_REG            (MATA_BASE + 0x608)  /* MATA的AUTOQOS的配置寄存器（AUTOQOS相关寄存器在Hi1980CV100未使用） */
#define MATA_MATA_SCH_CTRL_REG           (MATA_BASE + 0x60C)  /* MATA中IQ入口处调度控制配置寄存器 */
#define MATA_MATA_SCH_RETRY_TH_REG       (MATA_BASE + 0x610)  /* MATA中调度的retry水线值配置寄存器 */
#define MATA_MATA_SCH_PGNT_REG           (MATA_BASE + 0x614)  /* MATA内PGNT调度配置寄存器 */
#define MATA_MATA_SCH_TH_REG             (MATA_BASE + 0x618)  /* MATA内调度水线配置寄存器 */
#define MATA_MATA_SCH_TIMEOUT_THL_REG    (MATA_BASE + 0x620)  /* MATA内timeout调度水线寄存器低位 */
#define MATA_MATA_SCH_TIMEOUT_THH_REG    (MATA_BASE + 0x624)  /* MATA内timeout调度水线寄存器高位 */
#define MATA_MATA_PGNT_TIMEOUT_THL_REG   (MATA_BASE + 0x628)  /* MATA内timeout的pgnt水线寄存器低位 */
#define MATA_MATA_PGNT_TIMEOUT_THH_REG   (MATA_BASE + 0x62C)  /* MATA内timeout的pgnt水线寄存器高位 */
#define MATA_MATA_LUCMAINT_CTRL_REG      (MATA_BASE + 0x640)  /* MATA内l4maintain控制寄存器 */
#define MATA_MATA_LUCMAINT_START_L_REG   (MATA_BASE + 0x648)  /* MATA_L4MAINT_START_L为MATA按地址段Maintain操作起始地址低bit位。 */
#define MATA_MATA_LUCMAINT_START_H_REG   (MATA_BASE + 0x64C)  /* MATA_L4MAINT_START_L为MATA按地址段Maintain操作起始地址高bit位。 */
#define MATA_MATA_LUCMAINT_LEN_L_REG     (MATA_BASE + 0x650)  /* MATA_L4MAINT_LEN_L为MATA按地址段Maintain操作地址区间区间长度的低bit位。 */
#define MATA_MATA_LUCMAINT_LEN_H_REG     (MATA_BASE + 0x654)  /* MATA_L4MAINT_LEN_L为MATA按地址段Maintain操作地址区间区间长度的高bit位。 */
#define MATA_MATA_LUCMAINT_WAY_L_REG     (MATA_BASE + 0x658)  /* MATA_L4MAINT_WAY为MATA按WAY Maintain操作的低 32 WAY配置值 */
#define MATA_MATA_LUCMAINT_WAY_H_REG     (MATA_BASE + 0x65C)  /* MATA_L4MAINT_WAY为MATA按WAY Maintain操作的高2 WAY配置值 */
#define MATA_MATA_LUCALLOC_TH_REG        (MATA_BASE + 0x660)  /* MATA LUC Allocate水线相关配置寄存器 */
#define MATA_MATA_MAGIC_WORD_REG         (MATA_BASE + 0x7F0)  /* MATA版本修改寄存器 */
#define MATA_MATA_PERF_INTR_REG          (MATA_BASE + 0x800)  /* 统计事件溢出原始中断状态寄存器 */
#define MATA_MATA_PERF_INTM_REG          (MATA_BASE + 0x804)  /* 统计事件溢出中断屏蔽寄存器 */
#define MATA_MATA_PERF_INTS_REG          (MATA_BASE + 0x808)  /* 统计事件溢出中断状态寄存器 */
#define MATA_MATA_PERF_INTC_REG          (MATA_BASE + 0x80C)  /* 统计事件溢出中断清除寄存器 */
#define MATA_MATA_SKT_DOMAIN_REG         (MATA_BASE + 0x900)  /* 本socket far/near domain内其他socket vec寄存器 */
#define MATA_MATA_SKT0_DT_CANUM_REG      (MATA_BASE + 0x910)  /* socket0中TotemA/TotemB CANUM寄存器 */
#define MATA_MATA_SKT1_DT_CANUM_REG      (MATA_BASE + 0x914)  /* socket1中TotemA/TotemB CANUM寄存器 */
#define MATA_MATA_SKT2_DT_CANUM_REG      (MATA_BASE + 0x918)  /* socket2中TotemA/TotemB CANUM寄存器 */
#define MATA_MATA_SKT3_DT_CANUM_REG      (MATA_BASE + 0x91C)  /* socket3中TotemA/TotemB CANUM寄存器 */
#define MATA_MATA_SKT4_DT_CANUM_REG      (MATA_BASE + 0x920)  /* socket4中TotemA/TotemB CANUM寄存器 */
#define MATA_MATA_SKT5_DT_CANUM_REG      (MATA_BASE + 0x924)  /* socket5中TotemA/TotemB CANUM寄存器 */
#define MATA_MATA_SKT6_DT_CANUM_REG      (MATA_BASE + 0x928)  /* socket6中TotemA/TotemB CANUM寄存器 */
#define MATA_MATA_SKT7_DT_CANUM_REG      (MATA_BASE + 0x92C)  /* socket7中TotemA/TotemB CANUM寄存器 */
#define MATA_MATA_SKT8_DT_CANUM_REG      (MATA_BASE + 0x930)  /* socket8中TotemA/TotemB CANUM寄存器 */
#define MATA_MATA_SKT9_DT_CANUM_REG      (MATA_BASE + 0x934)  /* socket9中TotemA/TotemB CANUM寄存器 */
#define MATA_MATA_SKT10_DT_CANUM_REG     (MATA_BASE + 0x938)  /* socket10中TotemA/TotemB CANUM寄存器 */
#define MATA_MATA_SKT11_DT_CANUM_REG     (MATA_BASE + 0x93C)  /* socket11中TotemA/TotemB CANUM寄存器 */
#define MATA_MATA_SKT12_DT_CANUM_REG     (MATA_BASE + 0x940)  /* socket12中TotemA/TotemB CANUM寄存器 */
#define MATA_MATA_SKT13_DT_CANUM_REG     (MATA_BASE + 0x944)  /* socket13中TotemA/TotemB CANUM寄存器 */
#define MATA_MATA_SKT14_DT_CANUM_REG     (MATA_BASE + 0x948)  /* socket14中TotemA/TotemB CANUM寄存器 */
#define MATA_MATA_SKT15_DT_CANUM_REG     (MATA_BASE + 0x94C)  /* socket15中TotemA/TotemB CANUM寄存器 */
#define MATA_MATA_ECO0_REG               (MATA_BASE + 0xC00)  /* ECO寄存器0 */
#define MATA_MATA_ECO1_REG               (MATA_BASE + 0xC04)  /* ECO寄存器1 */
#define MATA_MATA_ECO2_REG               (MATA_BASE + 0xC08)  /* ECO寄存器2 */
#define MATA_MATA_ECO3_REG               (MATA_BASE + 0xC0C)  /* ECO寄存器3 */
#define MATA_MATA_DISABLE0_REG           (MATA_BASE + 0xC10)  /* 保留寄存器0 */
#define MATA_MATA_DISABLE1_REG           (MATA_BASE + 0xC14)  /* 保留寄存器1 */
#define MATA_MATA_DISABLE2_REG           (MATA_BASE + 0xC18)  /* 保留寄存器2 */
#define MATA_MATA_DISABLE3_REG           (MATA_BASE + 0xC1C)  /* 保留寄存器3 */
#define MATA_MATA_MIRROR_CTRL_REG        (MATA_BASE + 0xD00)  /* MATA mirror功能配置寄存器 */
#define MATA_MATA_MIRROR_ADDR0_REG       (MATA_BASE + 0xD04)  /* MATA mirror空间地址配置寄存器0 */
#define MATA_MATA_MIRROR_ADDR1_REG       (MATA_BASE + 0xD08)  /* MATA mirror空间地址配置寄存器1 */
#define MATA_MATA_MIRROR_ADDR2_REG       (MATA_BASE + 0xD0C)  /* MATA mirror空间地址配置寄存器2 */
#define MATA_MATA_MIRROR_ADDR3_REG       (MATA_BASE + 0xD10)  /* MATA mirror空间地址配置寄存器3 */
#define MATA_MATA_MIRROR_ADDR4_REG       (MATA_BASE + 0xD14)  /* MATA mirror空间地址配置寄存器4 */
#define MATA_MATA_MIRROR_ADDR5_REG       (MATA_BASE + 0xD18)  /* MATA mirror空间地址配置寄存器5 */
#define MATA_MATA_MIRROR_ADDR6_REG       (MATA_BASE + 0xD1C)  /* MATA mirror空间地址配置寄存器6 */
#define MATA_MATA_MIRROR_ADDR7_REG       (MATA_BASE + 0xD20)  /* MATA mirror空间地址配置寄存器7 */
#define MATA_MATA_MIRROR_SIZE0_REG       (MATA_BASE + 0xD24)  /* MATA mirror空间范围配置寄存器0 */
#define MATA_MATA_MIRROR_SIZE1_REG       (MATA_BASE + 0xD28)  /* MATA mirror空间范围配置寄存器1 */
#define MATA_MATA_MIRROR_SIZE2_REG       (MATA_BASE + 0xD2C)  /* MATA mirror空间范围配置寄存器2 */
#define MATA_MATA_MIRROR_SIZE3_REG       (MATA_BASE + 0xD30)  /* MATA mirror空间范围配置寄存器3 */
#define MATA_MATA_MIRROR_SIZE4_REG       (MATA_BASE + 0xD34)  /* MATA mirror空间范围配置寄存器4 */
#define MATA_MATA_MIRROR_SIZE5_REG       (MATA_BASE + 0xD38)  /* MATA mirror空间范围配置寄存器5 */
#define MATA_MATA_MIRROR_SIZE6_REG       (MATA_BASE + 0xD3C)  /* MATA mirror空间范围配置寄存器6 */
#define MATA_MATA_MIRROR_SIZE7_REG       (MATA_BASE + 0xD40)  /* MATA mirror空间范围配置寄存器7 */
#define MATA_MATA_PAW0_CTRL_REG          (MATA_BASE + 0xD44)  /* PAW0窗口寄存器 */
#define MATA_MATA_PAW1_CTRL_REG          (MATA_BASE + 0xD48)  /* PAW1窗口寄存器 */
#define MATA_MATA_PAW2_CTRL_REG          (MATA_BASE + 0xD4C)  /* PAW2窗口寄存器 */
#define MATA_MATA_PAW3_CTRL_REG          (MATA_BASE + 0xD50)  /* PAW3窗口寄存器 */
#define MATA_MATA_PAW4_CTRL_REG          (MATA_BASE + 0xD54)  /* PAW4窗口寄存器 */
#define MATA_MATA_PAW5_CTRL_REG          (MATA_BASE + 0xD58)  /* PAW5窗口寄存器 */
#define MATA_MATA_PAW6_CTRL_REG          (MATA_BASE + 0xD5C)  /* PAW6窗口寄存器 */
#define MATA_MATA_PAW7_CTRL_REG          (MATA_BASE + 0xD60)  /* PAW7窗口寄存器 */
#define MATA_MATA_PAW0_ADDR_REG          (MATA_BASE + 0xD64)  /* PAW0地址寄存器 */
#define MATA_MATA_PAW1_ADDR_REG          (MATA_BASE + 0xD68)  /* PAW1地址寄存器 */
#define MATA_MATA_PAW2_ADDR_REG          (MATA_BASE + 0xD6C)  /* PAW2地址寄存器 */
#define MATA_MATA_PAW3_ADDR_REG          (MATA_BASE + 0xD70)  /* PAW3地址寄存器 */
#define MATA_MATA_PAW4_ADDR_REG          (MATA_BASE + 0xD74)  /* PAW4地址寄存器 */
#define MATA_MATA_PAW5_ADDR_REG          (MATA_BASE + 0xD78)  /* PAW5地址寄存器 */
#define MATA_MATA_PAW6_ADDR_REG          (MATA_BASE + 0xD7C)  /* PAW6地址寄存器 */
#define MATA_MATA_PAW7_ADDR_REG          (MATA_BASE + 0xD80)  /* PAW7地址寄存器 */
#define MATA_MATA_STATUS1_REG            (MATA_BASE + 0xFFC)  /* MATA状态寄存器(记录longatomic计算原始中断) */
#define MATA_MATA_STATUS_REG             (MATA_BASE + 0x1000) /* MATA状态寄存器 */
#define MATA_MATA_OVERFLOW_REG           (MATA_BASE + 0x1004) /* MATA buffer溢出状态寄存器 */
#define MATA_MATA_STATUS_IF_REG          (MATA_BASE + 0x1008) /* MATA Interface接口状态寄存器 */
#define MATA_MATA_STATUS_DMCIF_REG       (MATA_BASE + 0x100C) /* MATA DMC Interface接口状态寄存器 */
#define MATA_MATA_PROBE_CTRL_REG         (MATA_BASE + 0x1010) /* MATA状态查询控制寄存器 */
#define MATA_MATA_PROBE_INFOL_REG        (MATA_BASE + 0x1020) /* MATA状态寄存器0 */
#define MATA_MATA_PROBE_INFOH_REG        (MATA_BASE + 0x1024) /* MATA状态寄存器1 */
#define MATA_MATA_FULLWREMPTYRD_REG      (MATA_BASE + 0x1028) /* 寄存器的满写空读状态寄存器 */
#define MATA_MATA_DIRSIDEINFO_CTRL_REG   (MATA_BASE + 0x1030) /* MATA dirsideinfo控制寄存器 */
#define MATA_MATA_DIRSIDEINFO_ENTRYH_REG (MATA_BASE + 0x103C) /* MATA dirsideinfo TAG寄存器 */
#define MATA_MATA_DIRSIDEINFO_ENTRYM_REG (MATA_BASE + 0x1040) /* MATA dirsideinfo TAG寄存器 */
#define MATA_MATA_DIRSIDEINFO_ENTRYL_REG (MATA_BASE + 0x1044) /* MATA dirsideinfo PATTERN寄存器 */
#define MATA_MATA_ERR_MASKL_REG          (MATA_BASE + 0x1048) /* MATA error mask寄存器 */
#define MATA_MATA_ERR_MASKH_REG          (MATA_BASE + 0x104C) /* MATA error mask寄存器 */
#define MATA_MATA_DIREVENT_ENABLE_REG    (MATA_BASE + 0x1050) /* 目录事件统计使能计数器 */
#define MATA_MATA_SDIR_WAY_SHUTDOWNL_REG (MATA_BASE + 0x1058) /* MATA关sdir WAY寄存器低位 */
#define MATA_MATA_SDIR_WAY_SHUTDOWNH_REG (MATA_BASE + 0x105C) /* MATA关sdir WAY寄存器高位 */
#define MATA_MATA_BUFF_DEPTH0_REG        (MATA_BASE + 0x1060) /* MATA BUFFER 水线配置寄存器0 */
#define MATA_MATA_DDR_LEVEL_PMU_REG      (MATA_BASE + 0x1064) /* MATA访问DDR的请求超过水线的配置寄存器&DDR size容量配置 */
#define MATA_MATA_BUFF_DEPTH1_REG        (MATA_BASE + 0x1068) /* MATA BUFFER 水线配置寄存器 */
#define MATA_MATA_DMA_CTRL_REG           (MATA_BASE + 0x106C) /* MATA DMA控制寄存器 */
#define MATA_MATA_DMA_THRES_REG          (MATA_BASE + 0x1070) /* MATA DMA水线配置寄存器 */
#define MATA_MATA_OUTSTAND_TH_REG        (MATA_BASE + 0x1074) /* MATA OUTSTANDING的水线配置寄存器 */
#define MATA_MATA_LUC_WAY_SHUTDOWNL_REG  (MATA_BASE + 0x1078) /* MATA关luc WAY寄存器低位 */
#define MATA_MATA_LUC_WAY_SHUTDOWNH_REG  (MATA_BASE + 0x107C) /* MATA关luc WAY寄存器高位 */
#define MATA_MATA_DMC_SIZE_GROUP0_REG    (MATA_BASE + 0x1080) /* MATA配置dmc0/dmc1容量寄存器 */
#define MATA_MATA_DMC_SIZE_GROUP1_REG    (MATA_BASE + 0x1084) /* MATA配置dmc2/dmc3容量寄存器 */
#define MATA_MATA_DMC_SIZE_GROUP2_REG    (MATA_BASE + 0x1088) /* MATA配置dmc4/dmc5容量寄存器 */
#define MATA_MATA_DMC_SIZE_GROUP3_REG    (MATA_BASE + 0x108C) /* MATA配置dmc6/dmc7容量寄存器 */
#define MATA_MATA_DMC_SIZE_CTRL_PRE_REG  (MATA_BASE + 0x1090) /* MATA配置DMC容量控制寄存器 */
#define MATA_MATA_OUTSTAND_TH1_REG       (MATA_BASE + 0x1094) /* MATA OUTSTANDING的水线配置寄存器 */
#define MATA_MATA_BUFF_DEPTH2_REG        (MATA_BASE + 0x1098) /* MATA BUFFER水线配置寄存器2 */
#define MATA_MATA_SENTRY_CNT_REG         (MATA_BASE + 0x1120) /* MATA single pattern有效目录entry计数器 */
#define MATA_MATA_DENTRY_CNT_REG         (MATA_BASE + 0x1124) /* MATA double pattern有效目录entry计数器 */
#define MATA_MATA_MEMCTRL_HISI_REG       (MATA_BASE + 0x1B00) /* MATA memory控制寄存器 */
#define MATA_MATA_EFUSE_MEM_CTRL_REG     (MATA_BASE + 0x1B80) /* MATA EFUSE MEMORY下电控制寄存器 */
#define MATA_MATA_DATSRC_CTRL_REG        (MATA_BASE + 0x1BF0) /* MATA DATSRC控制寄存器（Hi1980CV100不使用此寄存器）*/
#define MATA_MATA_PMU_VERSION_REG        (MATA_BASE + 0x1CF0) /* MATA模块PMU版本号 */
#define MATA_MATA_PERF_ENABLE_REG        (MATA_BASE + 0x1E00) /* MATA统计事件全局使能寄存器 */
#define MATA_MATA_EVENT_CTRL_REG         (MATA_BASE + 0x1E04) /* MATA的事件统计控制寄存器 */
#define MATA_MATA_SUBEVENT_CTRL0_REG     (MATA_BASE + 0x1E08) /* MATA latency统计控制寄存器 */
#define MATA_MATA_SUBEVENT_CTRL1_REG     (MATA_BASE + 0x1E0C) /* MATA latency统计控制寄存器 */
#define MATA_MATA_EVENT_REGION0_REG      (MATA_BASE + 0x1E10) /* DDR统计地址段配置寄存器0 */
#define MATA_MATA_EVENT_REGION1_REG      (MATA_BASE + 0x1E14) /* DDR统计地址段配置寄存器1 */
#define MATA_MATA_EVENT_FEATURE_REG      (MATA_BASE + 0x1E18) /* MATA事件功能配置寄存器 */
#define MATA_MATA_TRACETAG_CTRL0_REG     (MATA_BASE + 0x1E1C) /* MATA SNOOP的TRACETAG控制寄存器 */
#define MATA_MATA_TRACETAG_CTRL1_REG     (MATA_BASE + 0x1E20) /* MATA MEDIATED的TRACETAG控制寄存器 */
#define MATA_MATA_EVENT_FLT_CTRL_REG     (MATA_BASE + 0x1E24) /* MATA EVENT筛选控制寄存器 */
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
#define MATA_MATA_ERR_FRL_REG            (MATA_BASE + 0x2000) /* 错误记录特性寄存器低位 */
#define MATA_MATA_ERR_FRH_REG            (MATA_BASE + 0x2004) /* 错误记录特性寄存器高位 */
#define MATA_MATA_ERR_CTRLL_REG          (MATA_BASE + 0x2008) /* 错误记录控制寄存器低位 */
#define MATA_MATA_ERR_CTRLH_REG          (MATA_BASE + 0x200C) /* 错误记录控制寄存器高位 */
#define MATA_MATA_ERR_STATUSL_REG        (MATA_BASE + 0x2010) /* 错误记录原始状态寄存器低位 */
#define MATA_MATA_ERR_STATUSH_REG        (MATA_BASE + 0x2014) /* 错误记录原始状态寄存器高位 */
#define MATA_MATA_ERR_ADDRL_REG          (MATA_BASE + 0x2018) /* 错误记录地址寄存器低位 */
#define MATA_MATA_ERR_ADDRH_REG          (MATA_BASE + 0x201C) /* 错误记录地址寄存器高位 */
#define MATA_MATA_ERR_MISC0L_REG         (MATA_BASE + 0x2020) /* 错误记录信息寄存器低位 */
#define MATA_MATA_ERR_MISC0H_REG         (MATA_BASE + 0x2024) /* 错误记录信息寄存器高位 */
#define MATA_MATA_ERR_MISC1L_REG         (MATA_BASE + 0x2028) /* 错误记录信息寄存器低位 */
#define MATA_MATA_ERR_MISC1H_REG         (MATA_BASE + 0x202C) /* 错误记录信息寄存器高位 */
#define MATA_MATA_ERR_MISC2L_REG         (MATA_BASE + 0x2030) /* 错误记录信息寄存器低位 */
#define MATA_MATA_ERR_MISC2H_REG         (MATA_BASE + 0x2034) /* 错误记录信息寄存器高位 */
#define MATA_MATA_ERR_MISC3L_REG         (MATA_BASE + 0x2038) /* 错误记录信息寄存器高位 */
#define MATA_MATA_ERR_MISC3H_REG         (MATA_BASE + 0x203C) /* 错误记录信息寄存器高位 */
#define MATA_MATA_RAS_PFGF_L_REG         (MATA_BASE + 0x2800) /* 故障注入特征寄存器低位 */
#define MATA_MATA_RAS_PFGF_H_REG         (MATA_BASE + 0x2804) /* 故障注入特征寄存器高位 */
#define MATA_MATA_RAS_PFGCTL_L_REG       (MATA_BASE + 0x2808) /* 故障注入控制寄存器低位 */
#define MATA_MATA_RAS_PFGCTL_H_REG       (MATA_BASE + 0x280C) /* 故障注入控制寄存器高位 */
#define MATA_MATA_RAS_PFGCDN_L_REG       (MATA_BASE + 0x2810) /* 故障注入计数寄存器低位 */
#define MATA_MATA_RAS_PFGCDN_H_REG       (MATA_BASE + 0x2814) /* 故障注入计数寄存器高位 */
#define MATA_MATA_RAS_ERRGSR_L_REG       (MATA_BASE + 0x2E00) /* AS_ERGSR寄存器，记录组数，现在只有一组寄存器 */
#define MATA_MATA_RAS_ERRGSR_H_REG       (MATA_BASE + 0x2E04) /* AS_ERGSR寄存器，记录组数，现在只有一组寄存器 */
#define MATA_MATA_RAS_ERRDEVARCH_REG     (MATA_BASE + 0x2FBC) /* ARCHITECTURE寄存器 */
#define MATA_MATA_RAS_ERRIDR_REG         (MATA_BASE + 0x2FC8) /* ERROR RECORD ID寄存器 */
#define MATA_MATA_RAS_CONTAIN_CTRL_REG   (MATA_BASE + 0x3000) /* RAS控制寄存器 */
#define MATA_MATA_RAS_TIMEOUT_CTRL_REG   (MATA_BASE + 0x3004) /* RAS的timeout控制寄存器 */
#define MATA_MATA_DMA_SPEC_SIZE_ID0_REG  (MATA_BASE + 0x3030) /* SDMAA小size拆分的ID0配置寄存器 */
#define MATA_MATA_DMA_SPEC_SIZE_ID1_REG  (MATA_BASE + 0x3034) /* SDMAA小size拆分的ID1配置寄存器 */
#define MATA_MATA_DMA_SPEC_SIZE_ID2_REG  (MATA_BASE + 0x3038) /* SDMAA小size拆分的ID2配置寄存器 */
#define MATA_MATA_DMA_SPEC_SIZE_ID3_REG  (MATA_BASE + 0x303C) /* SDMAA小size拆分的ID3配置寄存器 */
#define MATA_MATA_DMA_PCIE0_ID_REG       (MATA_BASE + 0x3040) /* SDMAA使用的PCIE0的ID */
#define MATA_MATA_DMA_PCIE1_ID_REG       (MATA_BASE + 0x3044) /* SDMAA使用的PCIE1的ID */
#define MATA_MATA_DMA_PCIE_SPLIT_REG     (MATA_BASE + 0x3048) /* SDMAA的PCIe请求拆分配置寄存器 */
#define MATA_MATA_LP_ICG_MODULE_EN_REG   (MATA_BASE + 0x4200) /* 模块门控使能寄存器 */
#define MATA_MATA_MIRROR_INJ_ERR_REG     (MATA_BASE + 0x4204) /* MATA 注入MIRROR 错误寄存器 */
#define MATA_MATA_BP_CONFIG0_REG         (MATA_BASE + 0x4208) /* MATA 模块内部反压配置寄存器0 */
#define MATA_MATA_BP_CONFIG1_REG         (MATA_BASE + 0x420C) /* MATA 模块内部反压配置寄存器1 */
#define MATA_MATA_BP_CONFIG2_REG         (MATA_BASE + 0x4210) /* MATA 模块内部反压配置寄存器2 */
#define MATA_MATA_BP_CONFIG3_REG         (MATA_BASE + 0x4214) /* MATA 模块内部反压配置寄存器3 */
#define MATA_MATA_BP_CONFIG4_REG         (MATA_BASE + 0x4218) /* MATA 模块内部反压配置寄存器4 */

#endif // __MATA_REG_OFFSET_H__
