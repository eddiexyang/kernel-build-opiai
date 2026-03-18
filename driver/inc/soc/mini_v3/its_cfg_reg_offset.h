/*
 * Copyright     :  Copyright (C) 2021, Huawei Technologies Co. Ltd.
 * File name     :  its_cfg_reg_offset.h
 * Project line  :  Platform And Key Technologies Development
 * Department    :  CAD Development Department
 * Author        :  xxx
 * Version       :  1
 * Date          :  2013/3/10
 * Description   :  The description of xxx project
 * Others        :  Generated automatically by nManager V5.1 
 * History       :  xxx 2021/10/25 08:35:56 Create file
 */

#ifndef __ITS_CFG_REG_OFFSET_H__
#define __ITS_CFG_REG_OFFSET_H__

/* ITS_CFG Base address of Module's Register */
#define SOC_ITS_CFG_BASE                       (0x10000000)

/******************************************************************************/
/*                      SOC ITS_CFG Registers' Definitions                            */
/******************************************************************************/

#define SOC_ITS_CFG_GITS_CTLR_REG               (SOC_ITS_CFG_BASE + 0x0)     /* GITS控制寄存器 */
#define SOC_ITS_CFG_GITS_IIDR_REG               (SOC_ITS_CFG_BASE + 0x4)     /* GITS版本寄存器 */
#define SOC_ITS_CFG_GITS_TYPER_REG              (SOC_ITS_CFG_BASE + 0x8)     /* GITS配置信息寄存器 */
#define SOC_ITS_CFG_GITS_CBASER_L_REG           (SOC_ITS_CFG_BASE + 0x80)    /* GITS命令队列基地址寄存器[31:0] */
#define SOC_ITS_CFG_GITS_CBASER_H_REG           (SOC_ITS_CFG_BASE + 0x84)    /* GITS命令队列基地址寄存器[63:32] */
#define SOC_ITS_CFG_GITS_CWRITER_L_REG          (SOC_ITS_CFG_BASE + 0x88)    /* 软件写下一个命令的偏移地址寄存器[31:0] */
#define SOC_ITS_CFG_GITS_CWRITER_H_REG          (SOC_ITS_CFG_BASE + 0x8C)    /* 软件写下一个命令的偏移地址寄存器[63:32] */
#define SOC_ITS_CFG_GITS_CREADR_L_REG           (SOC_ITS_CFG_BASE + 0x90)    /* GITS读下一个命令的偏移地址寄存器[31:0] */
#define SOC_ITS_CFG_GITS_CREADR_H_REG           (SOC_ITS_CFG_BASE + 0x94)    /* GITS读下一个命令的偏移地址寄存器[63:32] */
#define SOC_ITS_CFG_GITS_BASER0_L_REG           (SOC_ITS_CFG_BASE + 0x100)   /* 外设/VCPU表基地址寄存器[31:0]。n{7,0} */
#define SOC_ITS_CFG_GITS_BASER0_H_REG           (SOC_ITS_CFG_BASE + 0x104)   /* 外设/VCPU表基地址寄存器[63:32]。n{7,0} */
#define SOC_ITS_CFG_GITS_BASER1_L_REG           (SOC_ITS_CFG_BASE + 0x108)   /* 外设/VCPU表基地址寄存器[31:0]。n{7,0} */
#define SOC_ITS_CFG_GITS_BASER1_H_REG           (SOC_ITS_CFG_BASE + 0x10C)   /* 外设/VCPU表基地址寄存器[63:32]。n{7,0} */
#define SOC_ITS_CFG_GITS_BASER2_L_REG           (SOC_ITS_CFG_BASE + 0x110)   /* 外设/VCPU表基地址寄存器[31:0]。n{7,0} */
#define SOC_ITS_CFG_GITS_BASER2_H_REG           (SOC_ITS_CFG_BASE + 0x114)   /* 外设/VCPU表基地址寄存器[63:32]。n{7,0} */
#define SOC_ITS_CFG_GITS_CIDR0_REG              (SOC_ITS_CFG_BASE + 0xFFF0)  /* GITS外设ID寄存器0 */
#define SOC_ITS_CFG_GITS_CIDR1_REG              (SOC_ITS_CFG_BASE + 0xFFF4)  /* GITS外设ID寄存器1 */
#define SOC_ITS_CFG_GITS_CIDR2_REG              (SOC_ITS_CFG_BASE + 0xFFF8)  /* GITS外设ID寄存器2 */
#define SOC_ITS_CFG_GITS_CIDR3_REG              (SOC_ITS_CFG_BASE + 0xFFFC)  /* GITS外设ID寄存器3 */
#define SOC_ITS_CFG_GITS_PIDR0_REG              (SOC_ITS_CFG_BASE + 0xFFE0)  /* GITS架构版本寄存器 */
#define SOC_ITS_CFG_GITS_PIDR1_REG              (SOC_ITS_CFG_BASE + 0xFFE4)  /* GITS架构版本寄存器 */
#define SOC_ITS_CFG_GITS_PIDR2_REG              (SOC_ITS_CFG_BASE + 0xFFE8)  /* GITS架构版本寄存器 */
#define SOC_ITS_CFG_GITS_PIDR3_REG              (SOC_ITS_CFG_BASE + 0xFFEC)  /* GITS架构版本寄存器 */
#define SOC_ITS_CFG_GITS_PIDR4_REG              (SOC_ITS_CFG_BASE + 0xFFD0)  /* GITS架构版本寄存器 */
#define SOC_ITS_CFG_GITS_PIDR5_REG              (SOC_ITS_CFG_BASE + 0xFFD4)  /* GITS架构版本寄存器 */
#define SOC_ITS_CFG_GITS_PIDR6_REG              (SOC_ITS_CFG_BASE + 0xFFD8)  /* GITS架构版本寄存器 */
#define SOC_ITS_CFG_GITS_PIDR7_REG              (SOC_ITS_CFG_BASE + 0xFFDC)  /* GITS架构版本寄存器 */
#define SOC_ITS_CFG_GITS_TRANSLATER_L_REG       (SOC_ITS_CFG_BASE + 0x10040) /* GITS中断接收寄存器 */
#define SOC_ITS_CFG_GITS_TRANSLATER_H_REG       (SOC_ITS_CFG_BASE + 0x10044) /* GITS中断接收寄存器 */
#define SOC_ITS_CFG_GITS_DFX_AXIERR_REG         (SOC_ITS_CFG_BASE + 0x20000) /* 访问总线错误指示 */
#define SOC_ITS_CFG_GITS_CFG_CNT_CLR_CE_REG     (SOC_ITS_CFG_BASE + 0x20004) /* CNT_CYC类型寄存器读清控制信号 */
#define SOC_ITS_CFG_GITS_DFX_AXIWR_CNT_REG      (SOC_ITS_CFG_BASE + 0x2000C) /* GITS总线操作计数器 */
#define SOC_ITS_CFG_GITS_DFX_SYS_ERR0_REG       (SOC_ITS_CFG_BASE + 0x20010) /* GITS系统错误状态寄存器0 */
#define SOC_ITS_CFG_GITS_DFX_SYS_ERR1_REG       (SOC_ITS_CFG_BASE + 0x20014) /* GITS系统错误状态寄存器1 */
#define SOC_ITS_CFG_GITS_DFX_SYS_ERR2_REG       (SOC_ITS_CFG_BASE + 0x20018) /* GITS系统错误状态寄存器2 */
#define SOC_ITS_CFG_GITS_DFX_ST_REG             (SOC_ITS_CFG_BASE + 0x2001C) /* DFX GITS状态机 */
#define SOC_ITS_CFG_GITS_DFX_CFIFO_ST_REG       (SOC_ITS_CFG_BASE + 0x20020) /* DFX CMDQ FIFO状态寄存器 */
#define SOC_ITS_CFG_GITS_AXIM_USER_REG          (SOC_ITS_CFG_BASE + 0x20040) /* AXIM USER控制寄存器 */
#define SOC_ITS_CFG_GITS_DFX_LAST_ID_REG        (SOC_ITS_CFG_BASE + 0x20044) /* DFX接收的中断号记录寄存器 */
#define SOC_ITS_CFG_GITS_DFX_LAST_DID_REG       (SOC_ITS_CFG_BASE + 0x20048) /* DFX GITS接收的最后的命令中的设备号 */
#define SOC_ITS_CFG_GITS_DFX_CACHE_CNT_CS_REG   (SOC_ITS_CFG_BASE + 0x2004C) /* CACHE统计类型选择寄存器 */
#define SOC_ITS_CFG_GITS_DFX_CACHE_MISS_CNT_REG (SOC_ITS_CFG_BASE + 0x20050) /* CACHE不命中统计计数器 */
#define SOC_ITS_CFG_GITS_PLANB_CFG_REG          (SOC_ITS_CFG_BASE + 0x20058) /* 自研芯片版本号配置寄存器 */
#define SOC_ITS_CFG_GITS_DFX_LPI_CNT_REG        (SOC_ITS_CFG_BASE + 0x20060) /* 接收到的LPI中断统计计数器 */
#define SOC_ITS_CFG_GITS_DFX_DISCARD_CNT_REG    (SOC_ITS_CFG_BASE + 0x20064) /* 丢失的LPI中断统计计数器 */
#define SOC_ITS_CFG_GITS_DFX_LPITX_CNT_REG      (SOC_ITS_CFG_BASE + 0x20068) /* 发送LPI中断统计计数器 */
#define SOC_ITS_CFG_GITS_FUNC_EN_REG            (SOC_ITS_CFG_BASE + 0x20080) /* GITS功能使能开关 */
#define SOC_ITS_CFG_GITS_DFX_VECTOR_CS_REG      (SOC_ITS_CFG_BASE + 0x20084) /* 被维测中断向量配置信号 */
#define SOC_ITS_CFG_GITS_DFX_INF0_REG           (SOC_ITS_CFG_BASE + 0x20088) /* 被维测中断路由信息 */
#define SOC_ITS_CFG_GITS_DFX_INF1_REG           (SOC_ITS_CFG_BASE + 0x2008C) /* 被维测中断路由信息 */
#define SOC_ITS_CFG_GITS_DFX_INF2_REG           (SOC_ITS_CFG_BASE + 0x20090) /* 被维测中断路由信息 */
#define SOC_ITS_CFG_GITS_DFX_INF3_REG           (SOC_ITS_CFG_BASE + 0x20094) /* 被维测中断路由信息 */
#define SOC_ITS_CFG_GITS_DFX_INF4_REG           (SOC_ITS_CFG_BASE + 0x20098) /* 被维测中断路由信息 */
#define SOC_ITS_CFG_GITS_DFX_INF5_REG           (SOC_ITS_CFG_BASE + 0x2009C) /* 被维测中断路由信息 */
#define SOC_ITS_CFG_GITS_SYS_ERR_MASK0_REG      (SOC_ITS_CFG_BASE + 0x200A0) /* 系统错误中断MASK0 */
#define SOC_ITS_CFG_GITS_SYS_ERR_MASK1_REG      (SOC_ITS_CFG_BASE + 0x200A4) /* 系统错误中断MASK1 */
#define SOC_ITS_CFG_GITS_SYS_ERR_MASK2_REG      (SOC_ITS_CFG_BASE + 0x200A8) /* 系统错误中断MASK2 */
#define SOC_ITS_CFG_GITS_SYS_ERR_INJECT0_REG    (SOC_ITS_CFG_BASE + 0x200AC) /* 系统错误中断INJECT0 */
#define SOC_ITS_CFG_GITS_SYS_ERR_INJECT1_REG    (SOC_ITS_CFG_BASE + 0x200B0) /* 系统错误中断INJECT1 */
#define SOC_ITS_CFG_GITS_SYS_ERR_INJECT2_REG    (SOC_ITS_CFG_BASE + 0x200B4) /* 系统错误中断INJECT2 */
#define SOC_ITS_CFG_GITS_SYNC_TIMEOUT_REG       (SOC_ITS_CFG_BASE + 0x200B8) /* SYNC/vSYNC 定时配置寄存器 */
#define SOC_ITS_CFG_GITS_SL3_OTSD_CNT_REG       (SOC_ITS_CFG_BASE + 0x200BC) /* SL3写操作OUTSTANDING计数寄存器 */
#define SOC_ITS_CFG_GITS_DFX_AXIM_WDATA_0_REG   (SOC_ITS_CFG_BASE + 0x200C0) /* MBIX总线写数据记录寄存器[31:0] */
#define SOC_ITS_CFG_GITS_DFX_AXIM_WDATA_1_REG   (SOC_ITS_CFG_BASE + 0x200C4) /* MBIX总线写数据记录寄存器[63:32] */
#define SOC_ITS_CFG_GITS_DFX_AXIM_WDATA_2_REG   (SOC_ITS_CFG_BASE + 0x200C8) /* MBIX总线写数据记录寄存器[95:33] */
#define SOC_ITS_CFG_GITS_DFX_AXIM_WDATA_3_REG   (SOC_ITS_CFG_BASE + 0x200CC) /* MBIX总线写数据记录寄存器[127:96] */
#define SOC_ITS_CFG_GITS_DFX_AXIM_ADDR_L_REG    (SOC_ITS_CFG_BASE + 0x200D0) /* MBIX总线写地址记录寄存器[31:0] */
#define SOC_ITS_CFG_GITS_DFX_AXIM_ADDR_H_REG    (SOC_ITS_CFG_BASE + 0x200D4) /* MBIX总线写地址记录寄存器[47:32] */

#endif // __ITS_CFG_REG_OFFSET_H__
