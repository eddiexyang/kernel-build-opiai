//******************************************************************************
// Copyright     :  Copyright (C) 2021, Hisilicon Technologies Co., Ltd.
// File name     :  sysctrl_cfg_reg_offset.h
// Author        :  xxx
// Version       :  1.0
// Date          :  2021-06-02
// Description   :  Define all registers/tables for xxx
// Others        :  Generated automatically by nManager V4.0
// History       :  xxx 2021-06-02 Create file
//******************************************************************************

#ifndef __SYSCTRL_CFG_REG_OFFSET_H__
#define __SYSCTRL_CFG_REG_OFFSET_H__

/* SYSCTRL_CFG Base address of Module's Register */
#define xxx_SYSCTRL_CFG_BASE                       (0x0)

/******************************************************************************/
/*                      xxx SYSCTRL_CFG Registers' Definitions                            */
/******************************************************************************/

#define xxx_SYSCTRL_CFG_SC_PLLCTRL_REG                    (xxx_SYSCTRL_CFG_BASE + 0xB0)   /* SC_PLLCTRL为PLL控制寄存器。 */
#define xxx_SYSCTRL_CFG_SC_SYSMODE_CTRL_REG               (xxx_SYSCTRL_CFG_BASE + 0xB4)   /* SC_SYSMODE_CTRL为CDIE_CRG系统时钟工作模式控制寄存器 */
#define xxx_SYSCTRL_CFG_SC_ALL_SCAN_CTRL_REG              (xxx_SYSCTRL_CFG_BASE + 0x150)  /* SC_ALL_SCAN_CTRL为ALL_SCAN模式OCC时钟关断控制寄存器。 */
#define xxx_SYSCTRL_CFG_SC_SYSSOFTRST_CTRL_REG            (xxx_SYSCTRL_CFG_BASE + 0x200)  /* SC_SYSSOFTRST_CTRL为TOTEM CRG系统软复位寄存器（无线产品应用中，请勿配置）。 */
#define xxx_SYSCTRL_CFG_SC_WDOG_RESET_CFG_REG             (xxx_SYSCTRL_CFG_BASE + 0x210)  /* 看门狗溢出复位范围控制寄存器（无线产品应用中，请勿配置）。 */
#define xxx_SYSCTRL_CFG_SC_PROBE_CLK_EN_REG               (xxx_SYSCTRL_CFG_BASE + 0x480)  /* PROBE时钟使能寄存器 */
#define xxx_SYSCTRL_CFG_SC_PROBE_CLK_DIS_REG              (xxx_SYSCTRL_CFG_BASE + 0x484)  /* PROBE时钟禁止寄存器 */
#define xxx_SYSCTRL_CFG_SC_DJTAG_CLK_EN_REG               (xxx_SYSCTRL_CFG_BASE + 0x488)  /* DJTAG时钟使能寄存器 */
#define xxx_SYSCTRL_CFG_SC_DJTAG_CLK_DIS_REG              (xxx_SYSCTRL_CFG_BASE + 0x48C)  /* DJTAG时钟禁止寄存器 */
#define xxx_SYSCTRL_CFG_SC_FUNC_MBIST_ICG_EN_REG          (xxx_SYSCTRL_CFG_BASE + 0x490)  /* FUNC_MBIST时钟使能寄存器 */
#define xxx_SYSCTRL_CFG_SC_FUNC_MBIST_ICG_DIS_REG         (xxx_SYSCTRL_CFG_BASE + 0x494)  /* FUNC_MBIST时钟禁止寄存器 */
#define xxx_SYSCTRL_CFG_SC_SYSCNT_CLK_EN_REG              (xxx_SYSCTRL_CFG_BASE + 0x630)  /* SC_SYSCNT_CLK_EN是clk_syscnt的时钟使能寄存器。 */
#define xxx_SYSCTRL_CFG_SC_SYSCNT_CLK_DIS_REG             (xxx_SYSCTRL_CFG_BASE + 0x634)  /* SC_SYSCNT_CLK_DIS是clk_syscnt时钟禁止寄存器。 */
#define xxx_SYSCTRL_CFG_SC_STATUS_RESET_REQ_REG           (xxx_SYSCTRL_CFG_BASE + 0xC50)  /* STATUS软复位请求信号(CPU外所有ras软复位请求）。 */
#define xxx_SYSCTRL_CFG_SC_STATUS_RESET_DREQ_REG          (xxx_SYSCTRL_CFG_BASE + 0xC54)  /* STATUS软复位去请求信号。 */
#define xxx_SYSCTRL_CFG_SC_DJTAG_RESET_REQ_REG            (xxx_SYSCTRL_CFG_BASE + 0xD18)  /* DJTAG逻辑复位请求寄存器 */
#define xxx_SYSCTRL_CFG_SC_DJTAG_RESET_DREQ_REG           (xxx_SYSCTRL_CFG_BASE + 0xD1C)  /* DJTAG逻辑复位去请求寄存器 */
#define xxx_SYSCTRL_CFG_SC_FUNC_MBIST_RESET_REQ_REG       (xxx_SYSCTRL_CFG_BASE + 0xD20)  /* FUNC_MBIST软复位请求控制寄存器 */
#define xxx_SYSCTRL_CFG_SC_FUNC_MBIST_RESET_DREQ_REG      (xxx_SYSCTRL_CFG_BASE + 0xD24)  /* FUNC_MBIST软复位去请求控制寄存器 */
#define xxx_SYSCTRL_CFG_SC_SYSCNT_RESET_REQ_REG           (xxx_SYSCTRL_CFG_BASE + 0xD30)  /* SC_SYSCNT_SRST_REQ表示SYSCNT逻辑复位请求寄存器 */
#define xxx_SYSCTRL_CFG_SC_SYSCNT_RESET_DREQ_REG          (xxx_SYSCTRL_CFG_BASE + 0xD34)  /* SC_SYSCNT_SRST_DREQ表示SYSCNT逻辑复位去请求寄存器 */
#define xxx_SYSCTRL_CFG_SC_FUNC_MBIST_CLK_SEL_REG         (xxx_SYSCTRL_CFG_BASE + 0x2000) /* DJTAG_FUNC时钟选择寄存器。 */
#define xxx_SYSCTRL_CFG_SC_DJTAG_FUNC_CLK_SEL_REG         (xxx_SYSCTRL_CFG_BASE + 0x2004) /* DJTAG_FUNC时钟选择寄存器。 */
#define xxx_SYSCTRL_CFG_SC_SIOE_IO_MIRROR_CFG_REG         (xxx_SYSCTRL_CFG_BASE + 0x2300) /* SIOE MIRROR配置寄存器 */
#define xxx_SYSCTRL_CFG_SC_DBGACK_CTRL_REG                (xxx_SYSCTRL_CFG_BASE + 0x20FC) /* DBGACK 控制寄存器 */
#define xxx_SYSCTRL_CFG_SC_SYSCNT_CTRL_REG                (xxx_SYSCTRL_CFG_BASE + 0x2144) /* SYSCNT控制寄存器 */
#define xxx_SYSCTRL_CFG_SC_MBIST_CPUI_ENABLE_REG          (xxx_SYSCTRL_CFG_BASE + 0x3500) /* CPU启动MBIST选择信号；芯片电路测试使用。 */
#define xxx_SYSCTRL_CFG_SC_MBIST_CPUI_DATAIN_REG          (xxx_SYSCTRL_CFG_BASE + 0x3504) /* CPU启动MBIST，TMB电路数据和指令输入接口寄存器 */
#define xxx_SYSCTRL_CFG_SC_MBIST_CPUI_WRITE_EN_REG        (xxx_SYSCTRL_CFG_BASE + 0x3508) /* DFT MBIST模式下TMB电路CPU2TAP接口配置寄存器；芯片电路测试使用。 */
#define xxx_SYSCTRL_CFG_SC_MBIST_CPUI_SMS_FUNC_RESET_REG  (xxx_SYSCTRL_CFG_BASE + 0x350C) /* CPU启动MBIST复位信号 */
#define xxx_SYSCTRL_CFG_SC_DJTAG_SRC_INT_REG              (xxx_SYSCTRL_CFG_BASE + 0x4008) /* DJTAG中断源寄存器 */
#define xxx_SYSCTRL_CFG_SC_DJTAG_INT_MASK_REG             (xxx_SYSCTRL_CFG_BASE + 0x400C) /* DJTAG中断屏蔽寄存器 */
#define xxx_SYSCTRL_CFG_SC_XTAL_CTRL_REG                  (xxx_SYSCTRL_CFG_BASE + 0x4104) /* 晶振控制寄存器 */
#define xxx_SYSCTRL_CFG_SC_ITCR_REG                       (xxx_SYSCTRL_CFG_BASE + 0x4108) /* 状态机测试使能寄存器 */
#define xxx_SYSCTRL_CFG_SC_ITIR0_REG                      (xxx_SYSCTRL_CFG_BASE + 0x410C) /* 测试模式输入控制寄存器 */
#define xxx_SYSCTRL_CFG_SC_ITOR_REG                       (xxx_SYSCTRL_CFG_BASE + 0x4110) /* 测试模式输出控制寄存器 */
#define xxx_SYSCTRL_CFG_SC_CNT_DATA_CFG_REG               (xxx_SYSCTRL_CFG_BASE + 0x4114) /* PLL切换COUNTER的初始值配置寄存器 */
#define xxx_SYSCTRL_CFG_SC_CNT_STEP_RSV_CFG_REG           (xxx_SYSCTRL_CFG_BASE + 0x4118) /* PLL切换COUNTER的递增配置寄存器 */
#define xxx_SYSCTRL_CFG_SC_CNT_CTRL_REG                   (xxx_SYSCTRL_CFG_BASE + 0x411C) /* CNT配置寄存器 */
#define xxx_SYSCTRL_CFG_SC_IM_CTRL_REG                    (xxx_SYSCTRL_CFG_BASE + 0x4120) /* Intterrupt模式控制寄存器 */
#define xxx_SYSCTRL_CFG_SC_IM_STAT_REG                    (xxx_SYSCTRL_CFG_BASE + 0x4124) /* Intterrupt模式状态寄存器 */
#define xxx_SYSCTRL_CFG_SC_PROBE_SYSTEM_COUNTER_VALUE_REG (xxx_SYSCTRL_CFG_BASE + 0x4580) /* System Counter的DFX寄存器，可产生System Counter脉冲，用于多片间计数值的比对。 */
#define xxx_SYSCTRL_CFG_SC_PROBE_SYSTEM_COUNTER_EN_REG    (xxx_SYSCTRL_CFG_BASE + 0x4584) /* System Counter的DFX寄存器，用于打开多片间计数值的比对功能。 */
#define xxx_SYSCTRL_CFG_SC_BROADCAST_DIE0_ADDR0_REG       (xxx_SYSCTRL_CFG_BASE + 0x4600) /* SC_BROADCAST_DIE0_ADDR0表示CPU CORE EVENT广播到DIE0的地址配置 */
#define xxx_SYSCTRL_CFG_SC_BROADCAST_DIE0_ADDR1_REG       (xxx_SYSCTRL_CFG_BASE + 0x4604) /* SC_BROADCAST_DIE0_ADDR1表示CPU CORE EVENT广播到DIE0的地址配置 */
#define xxx_SYSCTRL_CFG_SC_BROADCAST_DIE1_ADDR0_REG       (xxx_SYSCTRL_CFG_BASE + 0x4610) /* SC_BROADCAST_DIE1_ADDR0表示CPU CORE EVENT广播到DIE1的地址配置 */
#define xxx_SYSCTRL_CFG_SC_BROADCAST_DIE1_ADDR1_REG       (xxx_SYSCTRL_CFG_BASE + 0x4614) /* SC_BROADCAST_DIE1_ADDR1表示CPU CORE EVENT广播到DIE1的地址配置 */
#define xxx_SYSCTRL_CFG_SC_BROADCAST_DIE2_ADDR0_REG       (xxx_SYSCTRL_CFG_BASE + 0x4620) /* SC_BROADCAST_DIE2_ADDR0表示CPU CORE EVENT广播到DIE2的地址配置 */
#define xxx_SYSCTRL_CFG_SC_BROADCAST_DIE2_ADDR1_REG       (xxx_SYSCTRL_CFG_BASE + 0x4624) /* SC_BROADCAST_DIE2_ADDR1表示CPU CORE EVENT广播到DIE2的地址配置 */
#define xxx_SYSCTRL_CFG_SC_BROADCAST_DIE3_ADDR0_REG       (xxx_SYSCTRL_CFG_BASE + 0x4630) /* SC_BROADCAST_DIE3_ADDR0表示CPU CORE EVENT广播到DIE3的地址配置 */
#define xxx_SYSCTRL_CFG_SC_BROADCAST_DIE3_ADDR1_REG       (xxx_SYSCTRL_CFG_BASE + 0x4634) /* SC_BROADCAST_DIE3_ADDR1表示CPU CORE EVENT广播到DIE3的地址配置 */
#define xxx_SYSCTRL_CFG_SC_BROADCAST_DIE4_ADDR0_REG       (xxx_SYSCTRL_CFG_BASE + 0x4640) /* SC_BROADCAST_DIE4_ADDR0表示CPU CORE EVENT广播到DIE4的地址配置 */
#define xxx_SYSCTRL_CFG_SC_BROADCAST_DIE4_ADDR1_REG       (xxx_SYSCTRL_CFG_BASE + 0x4644) /* SC_BROADCAST_DIE4_ADDR1表示CPU CORE EVENT广播到DIE4的地址配置 */
#define xxx_SYSCTRL_CFG_SC_BROADCAST_DIE5_ADDR0_REG       (xxx_SYSCTRL_CFG_BASE + 0x4650) /* SC_BROADCAST_DIE5_ADDR0表示CPU CORE EVENT广播到DIE5的地址配置 */
#define xxx_SYSCTRL_CFG_SC_BROADCAST_DIE5_ADDR1_REG       (xxx_SYSCTRL_CFG_BASE + 0x4654) /* SC_BROADCAST_DIE5_ADDR1表示CPU CORE EVENT广播到DIE5的地址配置 */
#define xxx_SYSCTRL_CFG_SC_BROADCAST_DIE6_ADDR0_REG       (xxx_SYSCTRL_CFG_BASE + 0x4660) /* SC_BROADCAST_DIE6_ADDR0表示CPU CORE EVENT广播到DIE6的地址配置 */
#define xxx_SYSCTRL_CFG_SC_BROADCAST_DIE6_ADDR1_REG       (xxx_SYSCTRL_CFG_BASE + 0x4664) /* SC_BROADCAST_DIE6_ADDR1表示CPU CORE EVENT广播到DIE6的地址配置 */
#define xxx_SYSCTRL_CFG_SC_BROADCAST_DIE7_ADDR0_REG       (xxx_SYSCTRL_CFG_BASE + 0x4670) /* SC_BROADCAST_DIE7_ADDR0表示CPU CORE EVENT广播到DIE7的地址配置 */
#define xxx_SYSCTRL_CFG_SC_BROADCAST_DIE7_ADDR1_REG       (xxx_SYSCTRL_CFG_BASE + 0x4674) /* SC_BROADCAST_DIE7_ADDR1表示CPU CORE EVENT广播到DIE7的地址配置 */
#define xxx_SYSCTRL_CFG_SC_BROADCAST_DIE8_ADDR0_REG       (xxx_SYSCTRL_CFG_BASE + 0x4680) /* SC_BROADCAST_DIE8_ADDR0表示CPU CORE EVENT广播到DIE8的地址配置 */
#define xxx_SYSCTRL_CFG_SC_BROADCAST_DIE8_ADDR1_REG       (xxx_SYSCTRL_CFG_BASE + 0x4684) /* SC_BROADCAST_DIE8_ADDR1表示CPU CORE EVENT广播到DIE8的地址配置 */
#define xxx_SYSCTRL_CFG_SC_BROADCAST_DIE9_ADDR0_REG       (xxx_SYSCTRL_CFG_BASE + 0x4690) /* SC_BROADCAST_DIE9_ADDR0表示CPU CORE EVENT广播到DIE9的地址配置 */
#define xxx_SYSCTRL_CFG_SC_BROADCAST_DIE9_ADDR1_REG       (xxx_SYSCTRL_CFG_BASE + 0x4694) /* SC_BROADCAST_DIE9_ADDR1表示CPU CORE EVENT广播到DIE9的地址配置 */
#define xxx_SYSCTRL_CFG_SC_BROADCAST_DIE10_ADDR0_REG      (xxx_SYSCTRL_CFG_BASE + 0x46A0) /* SC_BROADCAST_DIE10_ADDR0表示CPU CORE EVENT广播到DIE10的地址配置 */
#define xxx_SYSCTRL_CFG_SC_BROADCAST_DIE10_ADDR1_REG      (xxx_SYSCTRL_CFG_BASE + 0x46A4) /* SC_BROADCAST_DIE10_ADDR1表示CPU CORE EVENT广播到DIE10的地址配置 */
#define xxx_SYSCTRL_CFG_SC_BROADCAST_DIE11_ADDR0_REG      (xxx_SYSCTRL_CFG_BASE + 0x46B0) /* SC_BROADCAST_DIE11_ADDR0表示CPU CORE EVENT广播到DIE11的地址配置 */
#define xxx_SYSCTRL_CFG_SC_BROADCAST_DIE11_ADDR1_REG      (xxx_SYSCTRL_CFG_BASE + 0x46B4) /* SC_BROADCAST_DIE11_ADDR1表示CPU CORE EVENT广播到DIE11的地址配置 */
#define xxx_SYSCTRL_CFG_SC_BROADCAST_DIE12_ADDR0_REG      (xxx_SYSCTRL_CFG_BASE + 0x46C0) /* SC_BROADCAST_DIE12_ADDR0表示CPU CORE EVENT广播到DIE12的地址配置 */
#define xxx_SYSCTRL_CFG_SC_BROADCAST_DIE12_ADDR1_REG      (xxx_SYSCTRL_CFG_BASE + 0x46C4) /* SC_BROADCAST_DIE12_ADDR1表示CPU CORE EVENT广播到DIE12的地址配置 */
#define xxx_SYSCTRL_CFG_SC_BROADCAST_DIE13_ADDR0_REG      (xxx_SYSCTRL_CFG_BASE + 0x46D0) /* SC_BROADCAST_DIE13_ADDR0表示CPU CORE EVENT广播到DIE13的地址配置 */
#define xxx_SYSCTRL_CFG_SC_BROADCAST_DIE13_ADDR1_REG      (xxx_SYSCTRL_CFG_BASE + 0x46D4) /* SC_BROADCAST_DIE13_ADDR1表示CPU CORE EVENT广播到DIE13的地址配置 */
#define xxx_SYSCTRL_CFG_SC_BROADCAST_DIE14_ADDR0_REG      (xxx_SYSCTRL_CFG_BASE + 0x46E0) /* SC_BROADCAST_DIE14_ADDR0表示CPU CORE EVENT广播到DIE14的地址配置 */
#define xxx_SYSCTRL_CFG_SC_BROADCAST_DIE14_ADDR1_REG      (xxx_SYSCTRL_CFG_BASE + 0x46E4) /* SC_BROADCAST_DIE14_ADDR1表示CPU CORE EVENT广播到DIE14的地址配置 */
#define xxx_SYSCTRL_CFG_SC_BROADCAST_DIE15_ADDR0_REG      (xxx_SYSCTRL_CFG_BASE + 0x46F0) /* SC_BROADCAST_DIE15_ADDR0表示CPU CORE EVENT广播到DIE15的地址配置 */
#define xxx_SYSCTRL_CFG_SC_BROADCAST_DIE15_ADDR1_REG      (xxx_SYSCTRL_CFG_BASE + 0x46F4) /* SC_BROADCAST_DIE15_ADDR1表示CPU CORE EVENT广播到DIE15的地址配置 */
#define xxx_SYSCTRL_CFG_SC_BROADCAST_EN_REG               (xxx_SYSCTRL_CFG_BASE + 0x4700) /* SC_BROADCAST_EN表示CPU CORE EVENT_EN的broadcast使能 */
#define xxx_SYSCTRL_CFG_SC_RECEIVE_BROADCAST_EVENT_REG    (xxx_SYSCTRL_CFG_BASE + 0x4710) /* SC_RECEIVE_BROADCAST表示SYSCTRL接收来自其他CPU CORE的 EVENT广播 */
#define xxx_SYSCTRL_CFG_SC_POE_EVENT_EN_REG               (xxx_SYSCTRL_CFG_BASE + 0x4720) /* POE的Event选择使能 */
#define xxx_SYSCTRL_CFG_SC_PLLCTRL_ST_REG                 (xxx_SYSCTRL_CFG_BASE + 0x5004) /* SC_PLLCTRL_ST为PLL状态寄存器。 */
#define xxx_SYSCTRL_CFG_SC_SYSMODE_STATUS_REG             (xxx_SYSCTRL_CFG_BASE + 0x50B4) /* SC_CTRL为系统时钟工作状态寄存器 */
#define xxx_SYSCTRL_CFG_SC_DDRC_WARM_RST_ACKED_REG        (xxx_SYSCTRL_CFG_BASE + 0x5200) /* SC_DDRC_WARM_RST_ACKED表示CRG检测到非上电的系统复位的warn_rst_req请求的DDRC握手信号 */
#define xxx_SYSCTRL_CFG_SC_PROBE_ICG_ST_REG               (xxx_SYSCTRL_CFG_BASE + 0x5480) /* PROBE时钟状态寄存器 */
#define xxx_SYSCTRL_CFG_SC_DJTAG_ICG_ST_REG               (xxx_SYSCTRL_CFG_BASE + 0x5488) /* DJTAG时钟状态寄存器 */
#define xxx_SYSCTRL_CFG_SC_FUNC_MBIST_ICG_ST_REG          (xxx_SYSCTRL_CFG_BASE + 0x5490) /* FUNC_MBIST时钟状态寄存器 */
#define xxx_SYSCTRL_CFG_SC_SYSCNT_CLK_ST_REG              (xxx_SYSCTRL_CFG_BASE + 0x5630) /* SC_SYSCNT_CLK_ST表示CDIE_CRG内syscnt_clk的时钟状态。 */
#define xxx_SYSCTRL_CFG_SC_STATUS_TRESET_ST_REG           (xxx_SYSCTRL_CFG_BASE + 0x5C50) /* STATUS的RST_REQ信号的状态。 */
#define xxx_SYSCTRL_CFG_SC_DJTAG_RESET_ST_REG             (xxx_SYSCTRL_CFG_BASE + 0x5D18) /* DJTAG相关逻辑复位状态 */
#define xxx_SYSCTRL_CFG_SC_FUNC_MBIST_RESET_ST_REG        (xxx_SYSCTRL_CFG_BASE + 0x5D20) /* FUNC_MBIST软复位状态寄存器 */
#define xxx_SYSCTRL_CFG_SC_SYSCNT_RESET_ST_REG            (xxx_SYSCTRL_CFG_BASE + 0x5D30) /* SC_SYSCNT_SRST_ST表示DJTAG相关逻辑复位状态 */
#define xxx_SYSCTRL_CFG_SC_MBIST_CPUI_DATAOUT_REG         (xxx_SYSCTRL_CFG_BASE + 0x7500) /* DFT MBIST模式下读数据寄存器 */
#define xxx_SYSCTRL_CFG_SC_DJTAG_INT_STATUS_REG           (xxx_SYSCTRL_CFG_BASE + 0x8008) /* DJTAG中断状态寄存器 */
#define xxx_SYSCTRL_CFG_SC_XTAL_ST_REG                    (xxx_SYSCTRL_CFG_BASE + 0x8104)
#define xxx_SYSCTRL_CFG_SC_CNT_ST_REG                     (xxx_SYSCTRL_CFG_BASE + 0x8108)
#define xxx_SYSCTRL_CFG_SC_ITIR0_TEST_REG                 (xxx_SYSCTRL_CFG_BASE + 0x810C)
#define xxx_SYSCTRL_CFG_SC_ITOR_TEST_REG                  (xxx_SYSCTRL_CFG_BASE + 0x8110)
#define xxx_SYSCTRL_CFG_SC_CNT_DATA_REG                   (xxx_SYSCTRL_CFG_BASE + 0x8114) /* PLL切换COUNTER的值状态寄存器 */
#define xxx_SYSCTRL_CFG_SC_AXI_ACC_ST_REG                 (xxx_SYSCTRL_CFG_BASE + 0x8504)
#define xxx_SYSCTRL_CFG_SC_DJTAG_SEC_ACC_EN_REG           (xxx_SYSCTRL_CFG_BASE + 0xD800) /* DJTAG访问各模块内安全世界寄存器的安全访问控制使能。（注意：1，此寄存器空间必须是安全访问才能成功。） */
#define xxx_SYSCTRL_CFG_SC_DJTAG_MSTR_ADDR_REG            (xxx_SYSCTRL_CFG_BASE + 0xD810) /* SC_DJTAG_MSTR_ADDR为DJTAG Master配置访问模块的偏移地址寄存器 */
#define xxx_SYSCTRL_CFG_SC_DJTAG_MSTR_DATA_REG            (xxx_SYSCTRL_CFG_BASE + 0xD814) /* SC_DJTAG_MSTR_DATA为DJTAG Master的写数据 */
#define xxx_SYSCTRL_CFG_SC_DJTAG_MSTR_CFG_REG             (xxx_SYSCTRL_CFG_BASE + 0xD818) /* SC_DJTAG_MSTR_CFG为DJTAG Master的配置寄存器 */
#define xxx_SYSCTRL_CFG_SC_DJTAG_MSTR_START_EN_REG        (xxx_SYSCTRL_CFG_BASE + 0xD81C) /* SC_DJTAG_MSTR_START_EN为DJTAG Master访问chain链的start使能 */
#define xxx_SYSCTRL_CFG_SC_DJTAG_MSTR_PIPE_CFG_REG        (xxx_SYSCTRL_CFG_BASE + 0xD820) /* DJTAG_MSTR PIPELINE配置寄存器 */
#define xxx_SYSCTRL_CFG_SC_DJTAG_TMOUT_REG                (xxx_SYSCTRL_CFG_BASE + 0xD840) /* SC_DJTAG_TMOUT为IDIE中DJTAG Master的状态机超时阈值 */
#define xxx_SYSCTRL_CFG_SC_DJTAG_SEC_MODE_REG             (xxx_SYSCTRL_CFG_BASE + 0xD844) /* DJTAG安全访问模式寄存器。 */
#define xxx_SYSCTRL_CFG_SC_AXI_MSTR_SEC_ACC_EN_REG        (xxx_SYSCTRL_CFG_BASE + 0xDA00) /* sysctrl内AXI Master访问DDR安全地址空间的安全访问控制使能。（注意：1，此寄存器空间必须是安全访问才能成功。） */
#define xxx_SYSCTRL_CFG_SC_AXI_MSTR_USER_REG              (xxx_SYSCTRL_CFG_BASE + 0xDA04) /* SC_AXI_MSTR_USER表示SYS_CTRL内AXI Master的虚拟机ID */
#define xxx_SYSCTRL_CFG_SC_AXI_MSTR_DATA0_REG             (xxx_SYSCTRL_CFG_BASE + 0xDA08) /* SC_AXI_MSTR_DATA0为SYS_CTRL内AXI Master的访问数据bit[31:0] */
#define xxx_SYSCTRL_CFG_SC_AXI_MSTR_DATA1_REG             (xxx_SYSCTRL_CFG_BASE + 0xDA0C) /* SC_AXI_MSTR_DATA1为SYS_CTRL内AXI Master的访问数据bit[63:32] */
#define xxx_SYSCTRL_CFG_SC_AXI_MSTR_DATA2_REG             (xxx_SYSCTRL_CFG_BASE + 0xDA10) /* SC_AXI_MSTR_DATA2为SYS_CTRL内AXI Master的访问数据bit[95:64] */
#define xxx_SYSCTRL_CFG_SC_AXI_MSTR_DATA3_REG             (xxx_SYSCTRL_CFG_BASE + 0xDA14) /* SC_AXI_MSTR_DATA3为SYS_CTRL内AXI Master的访问数据bit[127:96] */
#define xxx_SYSCTRL_CFG_SC_AXI_MSTR_WR_USER_REG           (xxx_SYSCTRL_CFG_BASE + 0xDA18) /* SC_AXI_MSTR_WR_USER为SYS_CTRL内AXI Master读写使能 */
#define xxx_SYSCTRL_CFG_SC_AXI_MSTR_ADDR0_REG             (xxx_SYSCTRL_CFG_BASE + 0xDA1C) /* SC_AXI_MSTR_ADDR0为SYS_CTRL内AXI Master的访问地址bit[31:0] */
#define xxx_SYSCTRL_CFG_SC_AXI_MSTR_ADDR1_REG             (xxx_SYSCTRL_CFG_BASE + 0xDA20) /* SC_AXI_MSTR_ADDR1为SYS_CTRL内AXI Master的访问地址bit[63:32] */
#define xxx_SYSCTRL_CFG_SC_AXI_MSTR_START_EN_REG          (xxx_SYSCTRL_CFG_BASE + 0xDA24) /* SC_AXI_MSTR_START_EN表示SYS_CTRL内AXI Master的start使能 */
#define xxx_SYSCTRL_CFG_SC_EFUSE_SECURE_INFO_REG          (xxx_SYSCTRL_CFG_BASE + 0xE050) /* EFUSE中的安全访问查询 */
#define xxx_SYSCTRL_CFG_SC_LAST_RST_STATUS_REG            (xxx_SYSCTRL_CFG_BASE + 0xE090) /* SC_LAST_RST_STATUS表示芯片最后一次复位的复位状态 */
#define xxx_SYSCTRL_CFG_SC_JTAG_AUTH_RESULT_REG           (xxx_SYSCTRL_CFG_BASE + 0xE100) /* 鉴权结果寄存器 */
#define xxx_SYSCTRL_CFG_CFG_VERSION_REG                   (xxx_SYSCTRL_CFG_BASE + 0xE0A0) /* VERSION寄存器 */
#define xxx_SYSCTRL_CFG_CFG_MAGIC_WORD_REG                (xxx_SYSCTRL_CFG_BASE + 0xE0A4) /* 版本号锁定寄存器 */
#define xxx_SYSCTRL_CFG_SYSCTRL_CFG_ECO_CFG0_REG          (xxx_SYSCTRL_CFG_BASE + 0xE0A8) /* 用于ECO的冗余配置寄存器0 */
#define xxx_SYSCTRL_CFG_SYSCTRL_CFG_ECO_CFG1_REG          (xxx_SYSCTRL_CFG_BASE + 0xE0AC) /* 用于ECO的冗余配置寄存器1 */
#define xxx_SYSCTRL_CFG_SYSCTRL_CFG_ECO_CFG2_REG          (xxx_SYSCTRL_CFG_BASE + 0xE0B0) /* 用于ECO的冗余配置寄存器2 */
#define xxx_SYSCTRL_CFG_SYSCTRL_CFG_ECO_CFG3_REG          (xxx_SYSCTRL_CFG_BASE + 0xE0B4) /* 用于ECO的冗余配置寄存器3 */
#define xxx_SYSCTRL_CFG_SC_DJTAG_RD_DATA0_REG             (xxx_SYSCTRL_CFG_BASE + 0xE800) /* DJTAG扫描链上第一个模块读数据寄存器 */
#define xxx_SYSCTRL_CFG_SC_DJTAG_RD_DATA1_REG             (xxx_SYSCTRL_CFG_BASE + 0xE804) /* DJTAG扫描链上第二个模块读数据寄存器 */
#define xxx_SYSCTRL_CFG_SC_DJTAG_RD_DATA2_REG             (xxx_SYSCTRL_CFG_BASE + 0xE808) /* DJTAG扫描链上第三个模块读数据寄存器 */
#define xxx_SYSCTRL_CFG_SC_DJTAG_RD_DATA3_REG             (xxx_SYSCTRL_CFG_BASE + 0xE80C) /* DJTAG扫描链上第四个模块读数据寄存器 */
#define xxx_SYSCTRL_CFG_SC_DJTAG_RD_DATA4_REG             (xxx_SYSCTRL_CFG_BASE + 0xE810) /* DJTAG扫描链上第五个模块读数据寄存器 */
#define xxx_SYSCTRL_CFG_SC_DJTAG_RD_DATA5_REG             (xxx_SYSCTRL_CFG_BASE + 0xE814) /* DJTAG扫描链上第六个模块读数据寄存器 */
#define xxx_SYSCTRL_CFG_SC_DJTAG_RD_DATA6_REG             (xxx_SYSCTRL_CFG_BASE + 0xE818) /* DJTAG扫描链上第七个模块读数据寄存器 */
#define xxx_SYSCTRL_CFG_SC_DJTAG_RD_DATA7_REG             (xxx_SYSCTRL_CFG_BASE + 0xE81C) /* DJTAG扫描链上第八个模块读数据寄存器 */
#define xxx_SYSCTRL_CFG_SC_DJTAG_RD_DATA8_REG             (xxx_SYSCTRL_CFG_BASE + 0xE820) /* DJTAG扫描链上第九个模块读数据寄存器 */
#define xxx_SYSCTRL_CFG_SC_DJTAG_RD_DATA9_REG             (xxx_SYSCTRL_CFG_BASE + 0xE824) /* DJTAG扫描链上第十个模块读数据寄存器 */
#define xxx_SYSCTRL_CFG_SC_DJTAG_OP_ST_REG                (xxx_SYSCTRL_CFG_BASE + 0xE828) /* DJTAG访问状态寄存器 */
#define xxx_SYSCTRL_CFG_SC_AXI_RD_DATA0_REG               (xxx_SYSCTRL_CFG_BASE + 0xEA00) /* AXI访问DDR的读数据寄存器 */
#define xxx_SYSCTRL_CFG_SC_AXI_RD_DATA1_REG               (xxx_SYSCTRL_CFG_BASE + 0xEA04) /* AXI访问DDR的读数据寄存器 */
#define xxx_SYSCTRL_CFG_SC_AXI_RD_DATA2_REG               (xxx_SYSCTRL_CFG_BASE + 0xEA08) /* AXI访问DDR的读数据寄存器 */
#define xxx_SYSCTRL_CFG_SC_AXI_RD_DATA3_REG               (xxx_SYSCTRL_CFG_BASE + 0xEA0C) /* AXI访问DDR的读数据寄存器 */
#define xxx_SYSCTRL_CFG_SC_AXI_MSTR_ST_REG                (xxx_SYSCTRL_CFG_BASE + 0xEA10) /* AXI访问DDR返回状态寄存器 */
#define xxx_SYSCTRL_CFG_SC_NORESET_0_REG                  (xxx_SYSCTRL_CFG_BASE + 0xF000) /* 安全启动记录寄存器0（寄存器无复位端）。 */
#define xxx_SYSCTRL_CFG_SC_NORESET_1_REG                  (xxx_SYSCTRL_CFG_BASE + 0xF004) /* 安全启动记录寄存器1（寄存器无复位端）。 */
#define xxx_SYSCTRL_CFG_SC_NORESET_2_REG                  (xxx_SYSCTRL_CFG_BASE + 0xF008) /* 安全启动记录寄存器2（寄存器无复位端）。 */
#define xxx_SYSCTRL_CFG_SC_NORESET_3_REG                  (xxx_SYSCTRL_CFG_BASE + 0xF00C) /* 安全启动记录寄存器3（寄存器无复位端）。 */
#define xxx_SYSCTRL_CFG_SC_NORESET_4_REG                  (xxx_SYSCTRL_CFG_BASE + 0xF010) /* 安全启动记录寄存器4（寄存器无复位端）。 */
#define xxx_SYSCTRL_CFG_SC_NORESET_5_REG                  (xxx_SYSCTRL_CFG_BASE + 0xF014) /* 安全启动记录寄存器5（寄存器无复位端）。 */
#define xxx_SYSCTRL_CFG_SC_NORESET_6_REG                  (xxx_SYSCTRL_CFG_BASE + 0xF018) /* 安全启动记录寄存器6（寄存器无复位端）。 */
#define xxx_SYSCTRL_CFG_SC_NORESET_7_REG                  (xxx_SYSCTRL_CFG_BASE + 0xF01C) /* 无复位端寄存器7 */
#define xxx_SYSCTRL_CFG_SC_NORESET_8_REG                  (xxx_SYSCTRL_CFG_BASE + 0xF020) /* 无复位端寄存器8 */
#define xxx_SYSCTRL_CFG_SC_NORESET_9_REG                  (xxx_SYSCTRL_CFG_BASE + 0xF024) /* 无复位端寄存器9 */
#define xxx_SYSCTRL_CFG_SC_NORESET_10_REG                 (xxx_SYSCTRL_CFG_BASE + 0xF028) /* 无复位端寄存器10 */
#define xxx_SYSCTRL_CFG_SC_NORESET_11_REG                 (xxx_SYSCTRL_CFG_BASE + 0xF02C) /* 无复位端寄存器11 */
#define xxx_SYSCTRL_CFG_SC_NORESET_12_REG                 (xxx_SYSCTRL_CFG_BASE + 0xF030) /* 无复位端寄存器12 */
#define xxx_SYSCTRL_CFG_SC_NORESET_13_REG                 (xxx_SYSCTRL_CFG_BASE + 0xF034) /* 无复位端寄存器13 */
#define xxx_SYSCTRL_CFG_SC_NORESET_14_REG                 (xxx_SYSCTRL_CFG_BASE + 0xF038) /* 无复位端寄存器14 */
#define xxx_SYSCTRL_CFG_SC_NORESET_15_REG                 (xxx_SYSCTRL_CFG_BASE + 0xF03C) /* 无复位端寄存器15 */
#define xxx_SYSCTRL_CFG_SC_SYSCTRL_LOCK_REG               (xxx_SYSCTRL_CFG_BASE + 0xF100) /* 系统控制器锁定寄存器。（注意：1，此寄存器空间必须是安全访问才能成功） */
#define xxx_SYSCTRL_CFG_SC_SYSCTRL_UNLOCK_REG             (xxx_SYSCTRL_CFG_BASE + 0xF110) /* 系统控制器解锁寄存器。（注意：1，此寄存器空间必须是安全访问才能成功） */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED0_REG               (xxx_SYSCTRL_CFG_BASE + 0xF800) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED1_REG               (xxx_SYSCTRL_CFG_BASE + 0xF804) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED2_REG               (xxx_SYSCTRL_CFG_BASE + 0xF808) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED3_REG               (xxx_SYSCTRL_CFG_BASE + 0xF80C) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED4_REG               (xxx_SYSCTRL_CFG_BASE + 0xF810) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED5_REG               (xxx_SYSCTRL_CFG_BASE + 0xF814) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED6_REG               (xxx_SYSCTRL_CFG_BASE + 0xF818) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED7_REG               (xxx_SYSCTRL_CFG_BASE + 0xF81C) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED8_REG               (xxx_SYSCTRL_CFG_BASE + 0xF820) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED9_REG               (xxx_SYSCTRL_CFG_BASE + 0xF824) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED10_REG              (xxx_SYSCTRL_CFG_BASE + 0xF828) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED11_REG              (xxx_SYSCTRL_CFG_BASE + 0xF82C) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED12_REG              (xxx_SYSCTRL_CFG_BASE + 0xF830) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED13_REG              (xxx_SYSCTRL_CFG_BASE + 0xF834) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED14_REG              (xxx_SYSCTRL_CFG_BASE + 0xF838) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED15_REG              (xxx_SYSCTRL_CFG_BASE + 0xF83C) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED16_REG              (xxx_SYSCTRL_CFG_BASE + 0xF840) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED17_REG              (xxx_SYSCTRL_CFG_BASE + 0xF844) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED18_REG              (xxx_SYSCTRL_CFG_BASE + 0xF848) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED19_REG              (xxx_SYSCTRL_CFG_BASE + 0xF84C) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED20_REG              (xxx_SYSCTRL_CFG_BASE + 0xF850) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED21_REG              (xxx_SYSCTRL_CFG_BASE + 0xF854) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED22_REG              (xxx_SYSCTRL_CFG_BASE + 0xF858) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED23_REG              (xxx_SYSCTRL_CFG_BASE + 0xF85C) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED24_REG              (xxx_SYSCTRL_CFG_BASE + 0xF860) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED25_REG              (xxx_SYSCTRL_CFG_BASE + 0xF864) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED26_REG              (xxx_SYSCTRL_CFG_BASE + 0xF868) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED27_REG              (xxx_SYSCTRL_CFG_BASE + 0xF86C) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED28_REG              (xxx_SYSCTRL_CFG_BASE + 0xF870) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED29_REG              (xxx_SYSCTRL_CFG_BASE + 0xF874) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED30_REG              (xxx_SYSCTRL_CFG_BASE + 0xF878) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED31_REG              (xxx_SYSCTRL_CFG_BASE + 0xF87C) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED32_REG              (xxx_SYSCTRL_CFG_BASE + 0xF880) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED33_REG              (xxx_SYSCTRL_CFG_BASE + 0xF884) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED34_REG              (xxx_SYSCTRL_CFG_BASE + 0xF888) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED35_REG              (xxx_SYSCTRL_CFG_BASE + 0xF88C) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED36_REG              (xxx_SYSCTRL_CFG_BASE + 0xF890) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED37_REG              (xxx_SYSCTRL_CFG_BASE + 0xF894) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED38_REG              (xxx_SYSCTRL_CFG_BASE + 0xF898) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED39_REG              (xxx_SYSCTRL_CFG_BASE + 0xF89C) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED40_REG              (xxx_SYSCTRL_CFG_BASE + 0xF8A0) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED41_REG              (xxx_SYSCTRL_CFG_BASE + 0xF8A4) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED42_REG              (xxx_SYSCTRL_CFG_BASE + 0xF8A8) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED43_REG              (xxx_SYSCTRL_CFG_BASE + 0xF8AC) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED44_REG              (xxx_SYSCTRL_CFG_BASE + 0xF8B0) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED45_REG              (xxx_SYSCTRL_CFG_BASE + 0xF8B4) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED46_REG              (xxx_SYSCTRL_CFG_BASE + 0xF8B8) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED47_REG              (xxx_SYSCTRL_CFG_BASE + 0xF8BC) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED48_REG              (xxx_SYSCTRL_CFG_BASE + 0xF8C0) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED49_REG              (xxx_SYSCTRL_CFG_BASE + 0xF8C4) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED50_REG              (xxx_SYSCTRL_CFG_BASE + 0xF8C8) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED51_REG              (xxx_SYSCTRL_CFG_BASE + 0xF8CC) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED52_REG              (xxx_SYSCTRL_CFG_BASE + 0xF8D0) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED53_REG              (xxx_SYSCTRL_CFG_BASE + 0xF8D4) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED54_REG              (xxx_SYSCTRL_CFG_BASE + 0xF8D8) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED55_REG              (xxx_SYSCTRL_CFG_BASE + 0xF8DC) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED56_REG              (xxx_SYSCTRL_CFG_BASE + 0xF8E0) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED57_REG              (xxx_SYSCTRL_CFG_BASE + 0xF8E4) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED58_REG              (xxx_SYSCTRL_CFG_BASE + 0xF8E8) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED59_REG              (xxx_SYSCTRL_CFG_BASE + 0xF8EC) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED60_REG              (xxx_SYSCTRL_CFG_BASE + 0xF8F0) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED61_REG              (xxx_SYSCTRL_CFG_BASE + 0xF8F4) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED62_REG              (xxx_SYSCTRL_CFG_BASE + 0xF8F8) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED63_REG              (xxx_SYSCTRL_CFG_BASE + 0xF8FC) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED64_REG              (xxx_SYSCTRL_CFG_BASE + 0xF900) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED65_REG              (xxx_SYSCTRL_CFG_BASE + 0xF904) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED66_REG              (xxx_SYSCTRL_CFG_BASE + 0xF908) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED67_REG              (xxx_SYSCTRL_CFG_BASE + 0xF90C) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED68_REG              (xxx_SYSCTRL_CFG_BASE + 0xF910) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED69_REG              (xxx_SYSCTRL_CFG_BASE + 0xF914) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED70_REG              (xxx_SYSCTRL_CFG_BASE + 0xF918) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED71_REG              (xxx_SYSCTRL_CFG_BASE + 0xF91C) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED72_REG              (xxx_SYSCTRL_CFG_BASE + 0xF920) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED73_REG              (xxx_SYSCTRL_CFG_BASE + 0xF924) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED74_REG              (xxx_SYSCTRL_CFG_BASE + 0xF928) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED75_REG              (xxx_SYSCTRL_CFG_BASE + 0xF92C) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED76_REG              (xxx_SYSCTRL_CFG_BASE + 0xF930) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED77_REG              (xxx_SYSCTRL_CFG_BASE + 0xF934) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED78_REG              (xxx_SYSCTRL_CFG_BASE + 0xF938) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED79_REG              (xxx_SYSCTRL_CFG_BASE + 0xF93C) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED80_REG              (xxx_SYSCTRL_CFG_BASE + 0xF940) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED81_REG              (xxx_SYSCTRL_CFG_BASE + 0xF944) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED82_REG              (xxx_SYSCTRL_CFG_BASE + 0xF948) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED83_REG              (xxx_SYSCTRL_CFG_BASE + 0xF94C) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED84_REG              (xxx_SYSCTRL_CFG_BASE + 0xF950) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED85_REG              (xxx_SYSCTRL_CFG_BASE + 0xF954) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED86_REG              (xxx_SYSCTRL_CFG_BASE + 0xF958) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED87_REG              (xxx_SYSCTRL_CFG_BASE + 0xF95C) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED88_REG              (xxx_SYSCTRL_CFG_BASE + 0xF960) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED89_REG              (xxx_SYSCTRL_CFG_BASE + 0xF964) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED90_REG              (xxx_SYSCTRL_CFG_BASE + 0xF968) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED91_REG              (xxx_SYSCTRL_CFG_BASE + 0xF96C) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED92_REG              (xxx_SYSCTRL_CFG_BASE + 0xF970) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED93_REG              (xxx_SYSCTRL_CFG_BASE + 0xF974) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED94_REG              (xxx_SYSCTRL_CFG_BASE + 0xF978) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_SW_RESERVED95_REG              (xxx_SYSCTRL_CFG_BASE + 0xF97C) /* 给软件预留的寄存器 */
#define xxx_SYSCTRL_CFG_SC_ECO_RSV0_REG                   (xxx_SYSCTRL_CFG_BASE + 0xFF00) /* ECO 寄存器0 */
#define xxx_SYSCTRL_CFG_SC_ECO_RSV1_REG                   (xxx_SYSCTRL_CFG_BASE + 0xFF04) /* ECO 寄存器1 */
#define xxx_SYSCTRL_CFG_SC_ECO_RSV2_REG                   (xxx_SYSCTRL_CFG_BASE + 0xFF08) /* ECO 寄存器2 */
#define xxx_SYSCTRL_CFG_SC_ECO_RSV3_REG                   (xxx_SYSCTRL_CFG_BASE + 0xFF0C) /* ECO 寄存器3 */
#define xxx_SYSCTRL_CFG_SC_ECO_RSV4_REG                   (xxx_SYSCTRL_CFG_BASE + 0xFF10) /* ECO 寄存器4 */
#define xxx_SYSCTRL_CFG_SC_ECO_RSV5_REG                   (xxx_SYSCTRL_CFG_BASE + 0xFF14) /* ECO 寄存器5 */
#define xxx_SYSCTRL_CFG_SC_MONITOR_TEST0_REG              (xxx_SYSCTRL_CFG_BASE + 0xFFF0) /* 测试寄存器 */
#define xxx_SYSCTRL_CFG_SC_MONITOR_TEST1_REG              (xxx_SYSCTRL_CFG_BASE + 0xFFF4) /* 测试寄存器 */
#define xxx_SYSCTRL_CFG_SC_CHIP_INF_REG                   (xxx_SYSCTRL_CFG_BASE + 0xFFF8) /* 芯片信息寄存器 */
#define xxx_SYSCTRL_CFG_SC_VER_NUM_REG                    (xxx_SYSCTRL_CFG_BASE + 0xFFFC) /* 版本寄存器 */

#endif // __SYSCTRL_CFG_REG_OFFSET_H__
