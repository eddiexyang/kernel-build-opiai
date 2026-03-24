// ******************************************************************************
// Copyright     :  Copyright (C) 2021, Hisilicon Technologies Co. Ltd.
// File name     :  gicr_dfx_regs_reg_offset.h
// Project line  :  Platform And Key Technologies Development
// Department    :  CAD Development Department
// Author        :  xxx
// Version       :  1
// Date          :  2013/3/10
// Description   :  The description of xxx project
// Others        :  Generated automatically by nManager V5.1
// History       :  xxx 2021/06/02 10:30:30 Create file
// ******************************************************************************

#ifndef __GICR_DFX_REGS_REG_OFFSET_H__
#define __GICR_DFX_REGS_REG_OFFSET_H__

/* GICR_DFX_REGS Base address of Module's Register */
#define SOC_GICR_DFX_REGS_BASE                       (0x30000000)

/******************************************************************************/
/*                      SOC GICR_DFX_REGS Registers' Definitions                            */
/******************************************************************************/

#define SOC_GICR_DFX_REGS_GICR_AXUSER_CFG_REG                    (SOC_GICR_DFX_REGS_BASE + 0x1004) /* GICR访问总线USER信号配置 */
#define SOC_GICR_DFX_REGS_GICR_MODIFY_REG                        (SOC_GICR_DFX_REGS_BASE + 0x1008) /* 自研芯片版本号配置寄存器 */
#define SOC_GICR_DFX_REGS_GICD_DIEEN_CROSS_REG                   (SOC_GICR_DFX_REGS_BASE + 0x1040) /* GICR CROSSDIE使能寄存器 */
#define SOC_GICR_DFX_REGS_GICD_DIEEN_CROSS_SPI_12N_REG           (SOC_GICR_DFX_REGS_BASE + 0x1044) /* GICR CROSSDIE使能寄存器 */
#define SOC_GICR_DFX_REGS_GICD_DIEEN_CROSS_SPI_N2N_REG           (SOC_GICR_DFX_REGS_BASE + 0x1048) /* GICR CROSSDIE使能寄存器 */
#define SOC_GICR_DFX_REGS_GICD_DIEEN_CROSS_SGI_N2N_REG           (SOC_GICR_DFX_REGS_BASE + 0x104C) /* GICR CROSSDIE使能寄存器 */
#define SOC_GICR_DFX_REGS_GICD_CROSSDIE_BASEADDR_0_REG           (SOC_GICR_DFX_REGS_BASE + 0x1060) /* GICR跨DIE中断产生和清除寄存器的基地址（[31:0]）。e{0,15} */
#define SOC_GICR_DFX_REGS_GICD_CROSSDIE_BASEADDR_1_REG           (SOC_GICR_DFX_REGS_BASE + 0x1064) /* GICR跨DIE中断产生和清除寄存器的基地址（[31:0]）。e{0,15} */
#define SOC_GICR_DFX_REGS_GICD_CROSSDIE_BASEADDR_2_REG           (SOC_GICR_DFX_REGS_BASE + 0x1068) /* GICR跨DIE中断产生和清除寄存器的基地址（[31:0]）。e{0,15} */
#define SOC_GICR_DFX_REGS_GICD_CROSSDIE_BASEADDR_3_REG           (SOC_GICR_DFX_REGS_BASE + 0x106C) /* GICR跨DIE中断产生和清除寄存器的基地址（[31:0]）。e{0,15} */
#define SOC_GICR_DFX_REGS_GICD_CROSSDIE_BASEADDR_4_REG           (SOC_GICR_DFX_REGS_BASE + 0x1070) /* GICR跨DIE中断产生和清除寄存器的基地址（[31:0]）。e{0,15} */
#define SOC_GICR_DFX_REGS_GICD_CROSSDIE_BASEADDR_5_REG           (SOC_GICR_DFX_REGS_BASE + 0x1074) /* GICR跨DIE中断产生和清除寄存器的基地址（[31:0]）。e{0,15} */
#define SOC_GICR_DFX_REGS_GICD_CROSSDIE_BASEADDR_6_REG           (SOC_GICR_DFX_REGS_BASE + 0x1078) /* GICR跨DIE中断产生和清除寄存器的基地址（[31:0]）。e{0,15} */
#define SOC_GICR_DFX_REGS_GICD_CROSSDIE_BASEADDR_7_REG           (SOC_GICR_DFX_REGS_BASE + 0x107C) /* GICR跨DIE中断产生和清除寄存器的基地址（[31:0]）。e{0,15} */
#define SOC_GICR_DFX_REGS_GICD_CROSSDIE_BASEADDR_8_REG           (SOC_GICR_DFX_REGS_BASE + 0x1080) /* GICR跨DIE中断产生和清除寄存器的基地址（[31:0]）。e{0,15} */
#define SOC_GICR_DFX_REGS_GICD_CROSSDIE_BASEADDR_9_REG           (SOC_GICR_DFX_REGS_BASE + 0x1084) /* GICR跨DIE中断产生和清除寄存器的基地址（[31:0]）。e{0,15} */
#define SOC_GICR_DFX_REGS_GICD_CROSSDIE_BASEADDR_10_REG          (SOC_GICR_DFX_REGS_BASE + 0x1088) /* GICR跨DIE中断产生和清除寄存器的基地址（[31:0]）。e{0,15} */
#define SOC_GICR_DFX_REGS_GICD_CROSSDIE_BASEADDR_11_REG          (SOC_GICR_DFX_REGS_BASE + 0x108C) /* GICR跨DIE中断产生和清除寄存器的基地址（[31:0]）。e{0,15} */
#define SOC_GICR_DFX_REGS_GICD_CROSSDIE_BASEADDR_12_REG          (SOC_GICR_DFX_REGS_BASE + 0x1090) /* GICR跨DIE中断产生和清除寄存器的基地址（[31:0]）。e{0,15} */
#define SOC_GICR_DFX_REGS_GICD_CROSSDIE_BASEADDR_13_REG          (SOC_GICR_DFX_REGS_BASE + 0x1094) /* GICR跨DIE中断产生和清除寄存器的基地址（[31:0]）。e{0,15} */
#define SOC_GICR_DFX_REGS_GICD_CROSSDIE_BASEADDR_14_REG          (SOC_GICR_DFX_REGS_BASE + 0x1098) /* GICR跨DIE中断产生和清除寄存器的基地址（[31:0]）。e{0,15} */
#define SOC_GICR_DFX_REGS_GICD_CROSSDIE_BASEADDR_15_REG          (SOC_GICR_DFX_REGS_BASE + 0x109C) /* GICR跨DIE中断产生和清除寄存器的基地址（[31:0]）。e{0,15} */
#define SOC_GICR_DFX_REGS_DFX_CTRL_REG                           (SOC_GICR_DFX_REGS_BASE + 0x3500) /* 中断统计计数器统计选择控制寄存器。 */
#define SOC_GICR_DFX_REGS_GICR_REGS_CNT_CLR_CE_REG               (SOC_GICR_DFX_REGS_BASE + 0x3504) /* GICD中CNT_CYC类型寄存器读清控制信号 */
#define SOC_GICR_DFX_REGS_DFX_LPI_CACHE_DATA_0_REG               (SOC_GICR_DFX_REGS_BASE + 0x3550) /* LPI CACHE地址数据寄存器（0~7） */
#define SOC_GICR_DFX_REGS_DFX_LPI_CACHE_DATA_1_REG               (SOC_GICR_DFX_REGS_BASE + 0x3554) /* LPI CACHE地址数据寄存器（0~7） */
#define SOC_GICR_DFX_REGS_DFX_LPI_CACHE_DATA_2_REG               (SOC_GICR_DFX_REGS_BASE + 0x3558) /* LPI CACHE地址数据寄存器（0~7） */
#define SOC_GICR_DFX_REGS_DFX_LPI_CACHE_DATA_3_REG               (SOC_GICR_DFX_REGS_BASE + 0x355C) /* LPI CACHE地址数据寄存器（0~7） */
#define SOC_GICR_DFX_REGS_DFX_LPI_CACHE_DATA_4_REG               (SOC_GICR_DFX_REGS_BASE + 0x3560) /* LPI CACHE地址数据寄存器（0~7） */
#define SOC_GICR_DFX_REGS_DFX_LPI_CACHE_DATA_5_REG               (SOC_GICR_DFX_REGS_BASE + 0x3564) /* LPI CACHE地址数据寄存器（0~7） */
#define SOC_GICR_DFX_REGS_DFX_LPI_CACHE_DATA_6_REG               (SOC_GICR_DFX_REGS_BASE + 0x3568) /* LPI CACHE地址数据寄存器（0~7） */
#define SOC_GICR_DFX_REGS_DFX_LPI_CACHE_DATA_7_REG               (SOC_GICR_DFX_REGS_BASE + 0x356C) /* LPI CACHE地址数据寄存器（0~7） */
#define SOC_GICR_DFX_REGS_DFX_LPI_CACHE_STATUS0_REG              (SOC_GICR_DFX_REGS_BASE + 0x3570) /* LPI CACHE状态 */
#define SOC_GICR_DFX_REGS_DFX_LPI_CACHE_STATUS1_REG              (SOC_GICR_DFX_REGS_BASE + 0x3574) /* LPI CACHE数据状态 */
#define SOC_GICR_DFX_REGS_DFX_VLPI_CACHE_DATA_0_REG              (SOC_GICR_DFX_REGS_BASE + 0x3580) /* VLPI CACHE地址数据寄存器（0~7） */
#define SOC_GICR_DFX_REGS_DFX_VLPI_CACHE_DATA_1_REG              (SOC_GICR_DFX_REGS_BASE + 0x3584) /* VLPI CACHE地址数据寄存器（0~7） */
#define SOC_GICR_DFX_REGS_DFX_VLPI_CACHE_DATA_2_REG              (SOC_GICR_DFX_REGS_BASE + 0x3588) /* VLPI CACHE地址数据寄存器（0~7） */
#define SOC_GICR_DFX_REGS_DFX_VLPI_CACHE_DATA_3_REG              (SOC_GICR_DFX_REGS_BASE + 0x358C) /* VLPI CACHE地址数据寄存器（0~7） */
#define SOC_GICR_DFX_REGS_DFX_VLPI_CACHE_DATA_4_REG              (SOC_GICR_DFX_REGS_BASE + 0x3590) /* VLPI CACHE地址数据寄存器（0~7） */
#define SOC_GICR_DFX_REGS_DFX_VLPI_CACHE_DATA_5_REG              (SOC_GICR_DFX_REGS_BASE + 0x3594) /* VLPI CACHE地址数据寄存器（0~7） */
#define SOC_GICR_DFX_REGS_DFX_VLPI_CACHE_DATA_6_REG              (SOC_GICR_DFX_REGS_BASE + 0x3598) /* VLPI CACHE地址数据寄存器（0~7） */
#define SOC_GICR_DFX_REGS_DFX_VLPI_CACHE_DATA_7_REG              (SOC_GICR_DFX_REGS_BASE + 0x359C) /* VLPI CACHE地址数据寄存器（0~7） */
#define SOC_GICR_DFX_REGS_DFX_VLPI_CACHE_STATUS0_REG             (SOC_GICR_DFX_REGS_BASE + 0x35A0) /* VLPI CACHE状态 */
#define SOC_GICR_DFX_REGS_DFX_VLPI_CACHE_STATUS1_REG             (SOC_GICR_DFX_REGS_BASE + 0x35A4) /* VLPI CACHE数据状态 */
#define SOC_GICR_DFX_REGS_DFX_GICD_CROSSDIE_STATUS_REG           (SOC_GICR_DFX_REGS_BASE + 0x3600) /* 跨DIE中断发送状态 */
#define SOC_GICR_DFX_REGS_DFX_GICD_CROSSDIE_CNT_REG              (SOC_GICR_DFX_REGS_BASE + 0x3640) /* GICR产生和清除跨DIE中断统计 */
#define SOC_GICR_DFX_REGS_DFX_LPI_CTRL_STATUS_REG                (SOC_GICR_DFX_REGS_BASE + 0x3700) /* LPI CTRL状态机状态指示。 */
#define SOC_GICR_DFX_REGS_DFX_LPI_ADD_CTRL_STATUS_REG            (SOC_GICR_DFX_REGS_BASE + 0x3704) /* LPI CTRL ADD状态机状态指示。 */
#define SOC_GICR_DFX_REGS_DFX_LPI_CTRL_DATA_ID_REG               (SOC_GICR_DFX_REGS_BASE + 0x3708) /* LPI CTRL处理中断ID寄存器。 */
#define SOC_GICR_DFX_REGS_DFX_LPI_CTRL_DATA_TA_REG               (SOC_GICR_DFX_REGS_BASE + 0x370C) /* LPI CTRL处理中断TA寄存器。 */
#define SOC_GICR_DFX_REGS_DFX_LPI_CMD_LPI_ACP_CNT_REG            (SOC_GICR_DFX_REGS_BASE + 0x3710) /* LPI中断命令被接收数量计算器。 */
#define SOC_GICR_DFX_REGS_DFX_LPI_CMD_LPI_INVALL_DONE_CNT_REG    (SOC_GICR_DFX_REGS_BASE + 0x3714) /* LPI中断INVALL命令完成数量计算器。 */
#define SOC_GICR_DFX_REGS_DFX_LPI_CMD_LPI_SET_SAMEID_CNT_REG     (SOC_GICR_DFX_REGS_BASE + 0x3718) /* LPI接收重复ID中断计算器。 */
#define SOC_GICR_DFX_REGS_DFX_LPI_CMD_LPI_MOVMEMORY_DONE_CNT_REG (SOC_GICR_DFX_REGS_BASE + 0x371C) /* LPI中断命令MOV MEMORY完成数量计算器。 */
#define SOC_GICR_DFX_REGS_DFX_LPI_CMD_LPI_MOVALL_DONE_CNT_REG    (SOC_GICR_DFX_REGS_BASE + 0x3720) /* LPI中断命令MOVALL命令完成数量计算器。 */
#define SOC_GICR_DFX_REGS_DFX_LPI_CMD_LPI_SINGLE_DONE_CNT_REG    (SOC_GICR_DFX_REGS_BASE + 0x3724) /* LPI中断命令SET&CLR&INV&MOV完成数量计算器。 */
#define SOC_GICR_DFX_REGS_DFX_LPI_ADD_ENABLE_REG                 (SOC_GICR_DFX_REGS_BASE + 0x3728) /* ADD命令被使能次数统计计数器。 */
#define SOC_GICR_DFX_REGS_DFX_LPI_ADD_SEARCH_DONE_REG            (SOC_GICR_DFX_REGS_BASE + 0x372C) /* ADD命令搜索完成数量统计计数器。 */
#define SOC_GICR_DFX_REGS_DFX_LPI_ADD_MOVBRESP_ACK_REG           (SOC_GICR_DFX_REGS_BASE + 0x3730) /* ADD状态机执行MOV MEMORY操作收到bresp响应数量统计计数器。 */
#define SOC_GICR_DFX_REGS_DFX_VLPI_CTRL_STATUS_REG               (SOC_GICR_DFX_REGS_BASE + 0x3800) /* LPI CTRL状态机状态指示。 */
#define SOC_GICR_DFX_REGS_DFX_VLPI_ADD_CTRL_STATUS_REG           (SOC_GICR_DFX_REGS_BASE + 0x3804) /* LPI CTRL ADD状态机状态指示。 */
#define SOC_GICR_DFX_REGS_DFX_VLPI_CTRL_DATA_ID_REG              (SOC_GICR_DFX_REGS_BASE + 0x3808) /* LPI CTRL处理中断ID寄存器。 */
#define SOC_GICR_DFX_REGS_DFX_VLPI_CTRL_DATA_TA_REG              (SOC_GICR_DFX_REGS_BASE + 0x380C) /* LPI CTRL处理中断TA寄存器。 */
#define SOC_GICR_DFX_REGS_DFX_VLPI_CTRL_DATA_VPT_REG             (SOC_GICR_DFX_REGS_BASE + 0x3810) /* LPI CTRL处理中断VPT寄存器。 */
#define SOC_GICR_DFX_REGS_DFX_VLPI_CMD_LPI_ACP_CNT_REG           (SOC_GICR_DFX_REGS_BASE + 0x3814) /* LPI中断命令被接收数量计算器。 */
#define SOC_GICR_DFX_REGS_DFX_VLPI_CMD_LPI_INVALL_DONE_CNT_REG   (SOC_GICR_DFX_REGS_BASE + 0x3818) /* LPI中断INVALL命令完成数量计算器。 */
#define SOC_GICR_DFX_REGS_DFX_VLPI_CMD_LPI_MOVALL_DONE_CNT_REG   (SOC_GICR_DFX_REGS_BASE + 0x381C) /* LPI中断命令MOVALL命令完成数量计算器。 */
#define SOC_GICR_DFX_REGS_DFX_VLPI_CMD_LPI_SINGLE_DONE_CNT_REG   (SOC_GICR_DFX_REGS_BASE + 0x3820) /* LPI中断命令SET&CLR&INV&MOV完成数量计算器。 */
#define SOC_GICR_DFX_REGS_DFX_VLPI_CMD_LPI_SET_SAMEID_CNT_REG    (SOC_GICR_DFX_REGS_BASE + 0x3824) /* LPI接收重复ID中断计算器。 */
#define SOC_GICR_DFX_REGS_DFX_VLPI_ADD_ENABLE_REG                (SOC_GICR_DFX_REGS_BASE + 0x3828) /* ADD命令被使能次数统计计数器。 */
#define SOC_GICR_DFX_REGS_DFX_VLPI_ADD_SEARCH_DONE_REG           (SOC_GICR_DFX_REGS_BASE + 0x382C) /* ADD命令搜索完成数量统计计数器。 */
#define SOC_GICR_DFX_REGS_DFX_LPI_CTLR_REG                       (SOC_GICR_DFX_REGS_BASE + 0x3900) /* DFX控制寄存器 */
#define SOC_GICR_DFX_REGS_DFX_LPI_MONITOR_CTRL0_REG              (SOC_GICR_DFX_REGS_BASE + 0x3904) /* 维测读写总线地址寄存器。 */
#define SOC_GICR_DFX_REGS_DFX_LPI_MONITOR_CTRL_1_REG             (SOC_GICR_DFX_REGS_BASE + 0x3908) /* 维测LPI与ADD命令写内存pending状态地址寄存器。 */
#define SOC_GICR_DFX_REGS_DFX_LPI_MONITOR_STATUS_0_WR_REG        (SOC_GICR_DFX_REGS_BASE + 0x390C) /* 维测写总线地址寄存器。 */
#define SOC_GICR_DFX_REGS_DFX_LPI_MONITOR_STATUS_0_RD_REG        (SOC_GICR_DFX_REGS_BASE + 0x3910) /* 维测读总线地址寄存器。 */
#define SOC_GICR_DFX_REGS_DFX_LPI_MONITOR_STATUS_1_REG           (SOC_GICR_DFX_REGS_BASE + 0x3914) /* LPI中断写内存中断pending状态错误记录寄存器。 */
#define SOC_GICR_DFX_REGS_DFX_LPI_MONITOR_STATUS_2_REG           (SOC_GICR_DFX_REGS_BASE + 0x3918) /* LPI、ADD状态机同时启动错误和系统状态寄存器。 */
#define SOC_GICR_DFX_REGS_DFX_LPI_RD_INFO_REG                    (SOC_GICR_DFX_REGS_BASE + 0x391C) /* 记录最近一次上报LPI中断ID对应的在内存中的中断配置与中断状态信息寄存器。 */
#define SOC_GICR_DFX_REGS_DFX_LPI_MOV_STATUS_REG                 (SOC_GICR_DFX_REGS_BASE + 0x3920) /* MOV命令执行状态寄存器。 */
#define SOC_GICR_DFX_REGS_DFX_LPI_CMD_LPI_SET_DONE_CNT_REG       (SOC_GICR_DFX_REGS_BASE + 0x3A00) /* LPI中断命令SET完成数量计算器。 */
#define SOC_GICR_DFX_REGS_DFX_LPI_CMD_LPI_CLR_DONE_CNT_REG       (SOC_GICR_DFX_REGS_BASE + 0x3A04) /* LPI中断命令CLR完成数量计算器。 */
#define SOC_GICR_DFX_REGS_DFX_LPI_CMD_LPI_INV_DONE_CNT_REG       (SOC_GICR_DFX_REGS_BASE + 0x3A08) /* LPI中断命令INV完成数量计算器。 */
#define SOC_GICR_DFX_REGS_DFX_LPI_CMD_LPI_MOV_DONE_CNT_REG       (SOC_GICR_DFX_REGS_BASE + 0x3A0C) /* LPI中断命令MOV完成数量计算器。 */
#define SOC_GICR_DFX_REGS_DFX_LPI_CMD_LPI_ID_CNT_REG             (SOC_GICR_DFX_REGS_BASE + 0x3A10) /* LPI命令所带ID在GICR接收数量统计。 */
#define SOC_GICR_DFX_REGS_GICR_DFX_GICR_INFO0_REG                (SOC_GICR_DFX_REGS_BASE + 0x3C00) /* 被维测的中断接收统计 */
#define SOC_GICR_DFX_REGS_GICR_DFX_GICR_INFO1_REG                (SOC_GICR_DFX_REGS_BASE + 0x3C04) /* 被维测的中断被ACTIVE统计 */
#define SOC_GICR_DFX_REGS_GICR_DFX_GICR_INFO2_REG                (SOC_GICR_DFX_REGS_BASE + 0x3C08) /* 被维测的中断被CLR统计 */
#define SOC_GICR_DFX_REGS_GICR_DFX_GICR_INFO3_REG                (SOC_GICR_DFX_REGS_BASE + 0x3C0C) /* 被维测的中断被合并的统计 */
#define SOC_GICR_DFX_REGS_GICR_SYS_ERR_REG                       (SOC_GICR_DFX_REGS_BASE + 0x3C10) /* 系统错误中断指示 */
#define SOC_GICR_DFX_REGS_GIC_AXI_AWQOS_REG                      (SOC_GICR_DFX_REGS_BASE + 0x3C14) /* GIC AXI MST AWQOS配置寄存器 */
#define SOC_GICR_DFX_REGS_GIC_AXI_RWQOS_REG                      (SOC_GICR_DFX_REGS_BASE + 0x3C18) /* GIC AXI MST RWQOS配置寄存器 */
#define SOC_GICR_DFX_REGS_GICR_SYS_ERR_MASK_REG                  (SOC_GICR_DFX_REGS_BASE + 0x3C1C) /* 系统错误中断MASK */
#define SOC_GICR_DFX_REGS_GICR_SYS_ERR_INJECT_REG                (SOC_GICR_DFX_REGS_BASE + 0x3C20) /* 系统错误中断INJECT */

#endif // __GICR_DFX_REGS_REG_OFFSET_H__
