/*
 * Copyright     :  Copyright (C) 2022, Huawei Technologies Co. Ltd.
 * File name     :  perf_stat_reg_offset.h
 * Project line  :  
 * Department    :  
 * Author        :  x198079
 * Version       :  V110
 * Date          :  Tester
 * Description   :  项目描述信息
 * Others        :  Generated automatically by nManager V5.1 
 * History       :  x198079 2022/03/04 09:31:54 Create file
 */

#ifndef __PERF_STAT_REG_OFFSET_H__
#define __PERF_STAT_REG_OFFSET_H__

/* PERF_STAT Base address of Module's Register */
#define PERF_STAT_BASE                       (0x10000)

/******************************************************************************/
/*                      HiDMSS PERF_STAT Registers' Definitions                            */
/******************************************************************************/

#define PERF_STAT_CLK_GATE_REG                  (PERF_STAT_BASE + 0x0)   /* 时钟gate寄存器。 */
#define PERF_STAT_CMD_START_REG                 (PERF_STAT_BASE + 0x8)   /* 启动寄存器。 */
#define PERF_STAT_SAMPLE_CNT_REG_REG            (PERF_STAT_BASE + 0x10)  /* 采样统计周期寄存器。 */
#define PERF_STAT_SAMPLE_NUM_REG_REG            (PERF_STAT_BASE + 0x18)  /* 采样次数寄存器。 */
#define PERF_STAT_SAMPLE_STOP_REG               (PERF_STAT_BASE + 0x20)  /* 采样停止寄存器。 */
#define PERF_STAT_REAL_PORT_NUM_REG             (PERF_STAT_BASE + 0x28)  /* 监控端口个数寄存器。 */
#define PERF_STAT_SEQ_ADDR_LEN_REG              (PERF_STAT_BASE + 0x40)  /* 连续模式写地址区间寄存器。 */
#define PERF_STAT_STAT_RST_CNT_REG              (PERF_STAT_BASE + 0x58)  /* 复位后计数寄存器。 */
#define PERF_STAT_INT_EN_REG                    (PERF_STAT_BASE + 0x60)  /* ST中断屏蔽控制寄存器。 */
#define PERF_STAT_INT_CLR_REG                   (PERF_STAT_BASE + 0x68)  /* ST中断清除寄存器。 */
#define PERF_STAT_RAW_INT_STAT_REG              (PERF_STAT_BASE + 0x70)  /* ST原始中断状态寄存器。 */
#define PERF_STAT_MASK_INT_STAT_REG             (PERF_STAT_BASE + 0x78)  /* ST屏蔽后中断状态寄存器。 */
#define PERF_STAT_ALL_SAMPLE_NUM_REG            (PERF_STAT_BASE + 0x80)  /* 总采样次数寄存器。 */
#define PERF_STAT_OVERTIME_CFG_CNT_REG          (PERF_STAT_BASE + 0x90)  /* 超时计数器。 */
#define PERF_STAT_DEBUG_FIFO_FULL_REG           (PERF_STAT_BASE + 0x98)  /* 查询fifo满状态寄存器。 */
#define PERF_STAT_DEBUG_MONITOR_REG             (PERF_STAT_BASE + 0xA0)  /* 查询monitor阻塞寄存器。 */
#define PERF_STAT_MONITOR_RESET_REG             (PERF_STAT_BASE + 0xA8)  /* monitor复位寄存器。 */
#define PERF_STAT_SEQ_ADDR_LOW_REG              (PERF_STAT_BASE + 0xB0)  /* 连续模式写地址低32位配置寄存器。 */
#define PERF_STAT_SEQ_ADDR_HIGH_REG             (PERF_STAT_BASE + 0xB8)  /* 连续模式写地址高32位配置寄存器。 */
#define PERF_STAT_DESCRIPTOR_ADDR_LOW_REG       (PERF_STAT_BASE + 0xC0)  /* 链表地址读地址低32位配置寄存器。 */
#define PERF_STAT_DESCRIPTOR_ADDR_HIGH_REG      (PERF_STAT_BASE + 0xC8)  /* 链表地址读地址高32位配置寄存器。 */
#define PERF_STAT_LAST_DESC_ADDR_LOW_REG        (PERF_STAT_BASE + 0xD0)  /* 当前链表地址低32位寄存器。 */
#define PERF_STAT_LAST_DESC_ADDR_HIGH_REG       (PERF_STAT_BASE + 0xD8)  /* 当前链表地址高32位寄存器。 */
#define PERF_STAT_LAST_SAMPLE_AXI_ADDR_LOW_REG  (PERF_STAT_BASE + 0xE0)  /* 采样地址低32位寄存器。 */
#define PERF_STAT_LAST_SAMPLE_AXI_ADDR_HIGH_REG (PERF_STAT_BASE + 0xE8)  /* 采样地址高32位寄存器。 */
#define PERF_STAT_PDRST_TMO_CNT_CFG_REG         (PERF_STAT_BASE + 0xF0)  /* 下电/复位触发超时阈值寄存器。 */
#define PERF_STAT_RS_ENABLE1_REG                (PERF_STAT_BASE + 0xF8)  /* RS 一拍配置寄存器。 */
#define PERF_STAT_RS_ENABLE2_REG                (PERF_STAT_BASE + 0x100) /* RS 二拍配置寄存器。 */
#define PERF_STAT_RS_ENABLE3_REG                (PERF_STAT_BASE + 0x108) /* RS 三拍配置寄存器。 */
#define PERF_STAT_RS_ENABLE4_REG                (PERF_STAT_BASE + 0x110) /* RS 四拍配置寄存器。 */
#define PERF_STAT_RS_ENABLE5_REG                (PERF_STAT_BASE + 0x118) /* RS 五拍配置寄存器。 */
#define PERF_STAT_RS_ENABLE6_REG                (PERF_STAT_BASE + 0x120) /* RS 六拍配置寄存器。 */
#define PERF_STAT_HISTOGRAM_CFG0_REG            (PERF_STAT_BASE + 0x128) /* 直方图配置0 */
#define PERF_STAT_HISTOGRAM_CFG1_REG            (PERF_STAT_BASE + 0x130) /* 直方图配置1 */
#define PERF_STAT_HISTOGRAM_CFG2_REG            (PERF_STAT_BASE + 0x138) /* 直方图配置2 */
#define PERF_STAT_HISTOGRAM_CFG3_REG            (PERF_STAT_BASE + 0x140) /* 直方图配置3 */
#define PERF_STAT_HISTOGRAM_CFG4_REG            (PERF_STAT_BASE + 0x148) /* 直方图配置4 */
#define PERF_STAT_HISTOGRAM_CFG5_REG            (PERF_STAT_BASE + 0x150) /* 直方图配置5 */
#define PERF_STAT_HISTOGRAM_CFG6_REG            (PERF_STAT_BASE + 0x158) /* 直方图配置6 */
#define PERF_STAT_HISTOGRAM_ST0_REG             (PERF_STAT_BASE + 0x160) /* 直方图状态 */
#define PERF_STAT_HISTOGRAM_STS0_REG            (PERF_STAT_BASE + 0x168) /* 直方图统计数据 */
#define PERF_STAT_HISTOGRAM_STS1_REG            (PERF_STAT_BASE + 0x170) /* 直方图统计数据 */
#define PERF_STAT_HISTOGRAM_STS2_REG            (PERF_STAT_BASE + 0x178) /* 直方图统计数据 */
#define PERF_STAT_HISTOGRAM_STS3_REG            (PERF_STAT_BASE + 0x180) /* 直方图统计数据 */
#define PERF_STAT_HISTOGRAM_STS4_REG            (PERF_STAT_BASE + 0x188) /* 直方图统计数据 */
#define PERF_STAT_HISTOGRAM_STS5_REG            (PERF_STAT_BASE + 0x190) /* 直方图统计数据 */
#define PERF_STAT_HISTOGRAM_CFG7_REG            (PERF_STAT_BASE + 0x198) /* 直方图配置7 */
#define PERF_STAT_EVENT_STAT_CTRL0_REG          (PERF_STAT_BASE + 0x1A0) /* 事件统计控制寄存器 */
#define PERF_STAT_LAST_READ_ADDR_LOW_REG        (PERF_STAT_BASE + 0x1A8) /* 当前 read 地址低32位寄存器。 */
#define PERF_STAT_LAST_READ_ADDR_HIGH_REG       (PERF_STAT_BASE + 0x1B0) /* 当前 read 地址高32位寄存器。 */
#define PERF_STAT_AXI_BRESP_ST_REG              (PERF_STAT_BASE + 0x1B8) /* AXI BRESP status */
#define PERF_STAT_LAST_READ_ST_REG              (PERF_STAT_BASE + 0x1C0) /* last read status */

#endif // __PERF_STAT_REG_OFFSET_H__
