/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * Description:
 * Author: huawei
 * Create: 2019-10-15
 */
#ifndef HWTS_DRV_CONFIG_H
#define HWTS_DRV_CONFIG_H

#include <linux/interrupt.h>
#define CHIP_NUM_MAX    2
#define AICPU_MAX_NUM   (16)  /* each chip */
#define AICPU_MAILBOX_MAX_NUM 16

#ifdef CFG_SOC_PLATFORM_MDC_V51
#define HWTS_IRQ_AFFINITY_CPU_ID 14
#define MAX_TS_NUM 2
#else
#define MAX_TS_NUM 1
#endif

#define IPC_MONITOR_NUM  2
#define TSC_ID           0
#define TSV_ID           1

#define HWTS_SYSTEM_CONFIG_PLAT_BASE       (0x9fe000ULL) /* 10M - 8K */
#define HWTS_CHIP_BASEADDR_PA_OFFSET       (0x8000000000ULL)
#define HWTS_SYSTEM_CONFIG_BASE(node_id)   (HWTS_SYSTEM_CONFIG_PLAT_BASE + HWTS_CHIP_BASEADDR_PA_OFFSET * (node_id))
#define HWTS_SYSTEM_CONFIG_SIZE            (4 * 1024)
#define HWTS_SYSTEM_CONFIG_FLAG            (0x5a5aa5a55a5aa5a5ULL)

#define MAILBOX_SIZE     (64)
#define MAILBOX_SHIFT    (6)

#define RET_MAILBOX_SIZE  64

#define STATUS_IDLE    (0x5aUL)
#define STATUS_BUSY    (0x6bUL)
#define STATUS_ERROR   (0xFFUL)
#define STATUS_PENDING (STATUS_ERROR)

#define HWTS_KERNEL_TYPE    10
#define MAILBOX_LEN         16 /* 4 bytes as unit */
#define MAILBOX_REG_SHIFT      0x20
#define AICPU_STATUS_EXCEPTION 0x4
#define AICPU_STATUS_END_OF_SEQUENCE 0x8

#define AE_STATUS_END_OF_SEQUENCE 6

#define KERNEL_TYPE_CUSTOMAICPU    4

struct drv_hwts_mailbox {
    pid_t pid;
    unsigned short kernel_type;
    unsigned short reserved;
    unsigned long long kernelName;
    unsigned long long kernelSo;
    unsigned long long paramBase;
    unsigned long long l2VaddrBase;
    unsigned long long l2Ctrl;
    unsigned short blockId;
    unsigned short blockNum;
    unsigned int l2InMain;
    unsigned int taskID : 26;
    unsigned int vfid : 6;
    unsigned int streamID;
};

struct drv_hwts_ts_status_info {
    unsigned long long ts_aicpu_status_base;
    unsigned long long ts_aicpu_status_size;
};

struct drv_hwts_sram_info {
    unsigned long long sram_pa_base;
    unsigned long long sram_pa_size;
    void __iomem *sram_va_base;
};

struct drv_hwts_gicd_info {
    unsigned long long gicd_pa_base;
    unsigned long long gicd_pa_size;
    unsigned int gic_multichip_off;
    void __iomem *gicd_va_base;
    void __iomem *ts_gicr_va_base;
};

struct drv_hwts_raw_irq_info {
    unsigned int irq_aicpu_to_ts[AICPU_MAX_NUM];
    unsigned int irq_ts_to_aicpu[AICPU_MAX_NUM];
    int irq_ts_to_aicpu_os_proxy[AICPU_MAX_NUM];
    unsigned int aicpu_id_base;
    unsigned int aicpu_num;
    unsigned int ts_int_start_id;
    unsigned int com_irq_ts_to_aicpu;
};

struct drv_hwts_int_context {
    unsigned int mailbox_index;
    unsigned int ts_index;
    cpumask_t cpumask;
    unsigned long long tick_ah;
    int node_id;
    struct tasklet_struct find_pid_then_submit_task;
};

struct drv_hwts_ts_info {
    unsigned int ts_index;
    unsigned long long flag;
    struct drv_hwts_sram_info sram_info;
    void __iomem *ts_aicpu_va_base;
    struct drv_hwts_raw_irq_info irq_info;
    struct drv_hwts_gicd_info gicd_info;
    struct drv_hwts_ts_status_info ts_status_info;
    struct drv_hwts_int_context int_context[AICPU_MAX_NUM];
};

#endif
