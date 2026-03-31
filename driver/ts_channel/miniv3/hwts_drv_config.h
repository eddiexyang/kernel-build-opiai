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

#define CHIP_NUM_MAX     4
#define AICPU_MAX_NUM    16
#define AICPU_MAILBOX_MAX_NUM (AICPU_MAX_NUM)

#define AICPU_PLATFORM_FPGA 0
#define AICPU_PLATFORM_EMU 1
#define AICPU_PLATFORM_ESL 2
#define AICPU_PLATFORM_ASIC 3
#define AICPU_PLATFORM_CHIP_ID_OFFSET 4
#define AICPU_PLATFORM_CHIP_TYPE_OFFSET 16

#define CPU_NUM_EACH_NODE_OF_FPGA  2
#define SOC_GIC_LPI_TABLE_BASE (0xAB500000)
#define TS_GICR_PROPBASER_OFFSET (0x70)
#define OS_HWI_LPI_MIN (8192)
#define LPI_PROP_ENABLED (1 << 0)
#define TS_GICR_INVLPIR_OFFSET (0xA0)
#define AICPU_CACHELINE_SIZE (64)
#define AICPU_CACHELINE_MASK (AICPU_CACHELINE_SIZE - 1)

#define MAX_TS_NUM       1
#define MAILBOX_SIZE     400
#define RET_MAILBOX_SIZE 64

#define IRQ_AICPU_TO_TS_OFFSET 16

#define TS_READY_STATE_REG_OFFSET 0xbe0ULL
#define TS_READY_MIN_VALUE 3
#define TS_READY_WAIT_COUNT 12000

#define TS_GICR_OFFSET 0x500000
#define TS_GICR_SIZE PAGE_SIZE
#define MAX_OFFLINE_PROCESS 32

#define KERNEL_TYPE_CCE            0
#define KERNEL_TYPE_TF             1
#define KERNEL_TYPE_CTL            2
#define KERNEL_TYPE_CUSTOMAICPU    4

#define HWTS_MAILBOX_READY_UDELAY_TIME 100
#define HWTS_MAILBOX_READY_UDELAY_NUM 10000

#ifdef __aarch64__
#define HWTS_DRV_FLUSH_CACHE(addr)    asm volatile("DC CIVAC ,%x0" ::"r"(addr))
#else
#define HWTS_DRV_FLUSH_CACHE(addr)
#endif

struct drv_hwts_cce_kernel {
    unsigned long long kernelName;
    unsigned long long kernelSo;
    unsigned long long paramBase;
    unsigned long long l2VaddrBase;
    unsigned int blockId;
    unsigned int blockNum;
    unsigned int l2Size;
    unsigned int l2InMain;
    unsigned int streamID;
    unsigned int taskID : 26;
    unsigned int vfid : 6;
    unsigned int reserved;
};

struct drv_hwts_fwk_kernel {
    unsigned long long kernel;
    unsigned int size;
    unsigned int streamID;
    unsigned int taskID : 26;
    unsigned int vfid : 6;
    unsigned int reserved;
};

struct drv_hwts_ts_kernel {
    unsigned int kernelType;
    union {
        struct drv_hwts_cce_kernel cce_kernel;
        struct drv_hwts_fwk_kernel fwk_kernel;
    } kernelBase;
};

struct drv_hwts_mailbox {
    volatile unsigned int valid;
    pid_t pid;
    volatile unsigned long long serial_no;
    struct drv_hwts_ts_kernel kernel_info;
};

struct drv_hwts_int_context {
    unsigned int mailbox_index;
    unsigned int ts_index;
    cpumask_t cpumask;
    unsigned long long tick_ah;
    int node_id;
    struct tasklet_struct find_pid_then_wakeup_task;
};

struct drv_hwts_raw_irq_info {
    unsigned int irq_aicpu_to_ts[AICPU_MAX_NUM];
    unsigned int irq_ts_to_aicpu[AICPU_MAX_NUM];
    int irq_ts_to_aicpu_os_proxy[AICPU_MAX_NUM];
    unsigned int aicpu_id_base;
    unsigned int aicpu_num;
    unsigned int ts_int_start_id;
};

struct drv_hwts_raw_chip_info {
    unsigned int chip_id;
    unsigned int chip_type;
    unsigned int chip_version;

    unsigned int cpu_cluster_num;
    unsigned int cpu_num_per_cluster;
};

struct drv_hwts_gicd_info {
    unsigned long long gicd_pa_base;
    unsigned long long gicd_pa_size;
    unsigned int gic_multichip_off;
};

struct drv_hwts_sram_info {
    unsigned long long sram_pa_base;
    unsigned long long sram_pa_size;
};

struct drv_hwts_ts_status_info {
    unsigned long long ts_aicpu_status_base;
    unsigned long long ts_aicpu_status_size;
};

struct drv_hwts_ts_info {
    unsigned int ts_index;
    unsigned long long flag;
    struct drv_hwts_ts_status_info ts_status_info;
    struct drv_hwts_sram_info sram_info;
    struct drv_hwts_gicd_info gicd_info;
    struct drv_hwts_raw_chip_info chip_info;
    struct drv_hwts_raw_irq_info irq_info;
    void __iomem *sram_va_base;
    void __iomem *ts_aicpu_va_base;
    void __iomem *gicd_va_base;
    void __iomem *ts_gicr_va_base;
    struct drv_hwts_int_context int_context[AICPU_MAX_NUM];
};

#define RESERVED_ARRAY_SIZE         11
struct drv_hwts_task_response {
    volatile unsigned int valid;
    volatile unsigned int state;
    volatile unsigned long long serial_no;
    volatile unsigned int reserved[RESERVED_ARRAY_SIZE];
};

#define MAILBOX_CONFIRM (0x59595959)
#define MAILBOX_VALID (0x5a5a5a5a)
#define MAILBOX_INVALID (0)
/* Redistributor registers:write irq to GICR in 64bit to trigger ts int */
#define GICR_SETLPIR 0x0040
void tsdrv_writel_relaxed(void *reg, unsigned int val);
#endif
