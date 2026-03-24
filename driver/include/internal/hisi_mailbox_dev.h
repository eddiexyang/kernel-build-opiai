/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2023. All rights reserved.
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

#ifndef __HISI_MAILBOX_DEV_H__
#define __HISI_MAILBOX_DEV_H__

#include <linux/of_address.h>
#include "hisi_rproc_adapt.h"

#define IPCBITMASK(n)           (1u << (n))
#define IPCMBxSOURCE(mdev)      (((mdev) << HISI_MBX_SEGMENT_SIZE_BIT) + 0x00) /* MBX_SOURCE */
#define IPCMBxDSET(mdev)        (((mdev) << HISI_MBX_SEGMENT_SIZE_BIT) + 0x04) /* MBX_DEST */
#define IPCMBxDCLR(mdev)        (((mdev) << HISI_MBX_SEGMENT_SIZE_BIT) + 0x08) /* MBX_DCLEAR */
#define IPCMBxDSTATUS(mdev)     (((mdev) << HISI_MBX_SEGMENT_SIZE_BIT) + 0x0C) /* MBX_DSTATUS */
#define IPCMBxMODE(mdev)        (((mdev) << HISI_MBX_SEGMENT_SIZE_BIT) + 0x10) /* MBX_MODE */
#define IPCMBxIMASK(mdev)       (((mdev) << HISI_MBX_SEGMENT_SIZE_BIT) + 0x14) /* MBX_IMASK */
#define IPCMBxICLR(mdev)        (((mdev) << HISI_MBX_SEGMENT_SIZE_BIT) + 0x18) /* MBX_ICLR */
#define IPCMBxSEND(mdev)        (((mdev) << HISI_MBX_SEGMENT_SIZE_BIT) + 0x1C) /* MBX_SEND */
#define IPCMBxDATA(mdev, index) (((mdev) << HISI_MBX_SEGMENT_SIZE_BIT) + 0x20 + ((index) << 2)) /* MBX_LOCK */

#define IPCCPUxIMST2(cpu, n) (((cpu) << 3) + ((n) << 8) + 0x8000)
#define IPCCPUxIRST2(cpu, n) (((cpu) << 3) + ((n) << 8) + 0x8004)

#define IPCCPUxIMST(cpu) (((cpu) << 3) + 0x800)
#define IPCCPUxIRST(cpu) (((cpu) << 3) + 0x804)

#if defined(CFG_SOC_PLATFORM_CLOUD_V2)
#define IPCLOCK() (0xA000)
#elif defined(CFG_SOC_PLATFORM_MINIV2)
#define IPCLOCK() (0xA000)

#define HISI_MINIV2_V1 1
#define HISI_MINIV2_V2 2
u32 hisi_mdev_get_chip_type(void);
#elif defined(CFG_SOC_PLATFORM_MINIV3)
#define IPCLOCK() (0xA000)
#else
#define IPCLOCK() (0xA00)
#endif

#define FAST_MBOX           (1 << 0)
#define COMM_MBOX           (1 << 1)
#define SOURCE_MBOX         (1 << 2)
#define DESTINATION_MBOX    (1 << 3)

#define IPC_LOCKED 0x00000001
#define IPC_UNLOCKED 0x00000000
#define IPCACKMSG 0x00000000
#define COMM_MBOX_IRQ (-2)
#define AUTOMATIC_ACK_CONFIG (1 << 0)
#define NO_FUNC_CONFIG (0 << 0)

/* Optimize interrupts assignment */
#define IPC_IRQ_AFFINITY_CPU (1)

#define SYS_RPROC_NUMBER 0x8
#define ISP_RPROC_NUMBER 0x2
#define STATE_NUMBER 0x4

#define MAILBOX_ASYNC_TIMEOUT_CNT (1060)
#define MAILBOX_ASYNC_UDELAY_CNT (1000)

#define MBIX_INT_CLR_REG (0xA068)  // int_group7 addr
#define MIIX_PIN_BASE (56)
#define MBIX_IPC_INT_OFFSET (25)

#define IPC_CPU_AFFINITY_DEFAULT 0xFF

#define IPC_WRITE_CHECK_RETRY_TIMES 3

enum {
    RX_BUFFER_TYPE = 0,
    ACK_BUFFER_TYPE,
    MBOX_BUFFER_TYPE_MAX,
};

struct hisi_remote_processor {
    const char *name;
    remote_processor_type_t type;
};

struct hisi_common_mbox_info {
    int gic_1_irq_requested;
    int gic_2_irq_requested;
    int gic_3_irq_requested;
    int gic_4_irq_requested;
    int cmbox_gic_1_irq;
    int cmbox_gic_2_irq;
    int cmbox_gic_3_irq;
    int cmbox_gic_4_irq;
    int cmbox_gic_5_irq;
    struct hisi_mbox_device *cmdev;
};

struct hisi_ipc_device {
    void __iomem *base;
    void __iomem *lpi_base;
    u32 unlock;
    mbox_msg_t *buf_pool;
    struct hisi_common_mbox_info *cmbox_info;
    struct hisi_mbox_device **mdev_res;
    int rp_num;
};

struct hisi_mbox_device_priv {
    const char *src_name;
    const char *dse_name; /* Do not change its name, codemars warning */
    remote_processor_type_t src;
    remote_processor_type_t des;
    u32 index;
    u32 irq_affinity;
    int irq;
    int capability;
    struct hisi_ipc_device *idev;
    u8 func;
};

struct hisi_mbox_irq_info {
    int irq_no;
    void *priv_data;
    struct list_head list;
};

void sync_ipc_clear_lpi(struct hisi_rproc_info *rproc);

#endif /* __HISI_MAILBOX_DEV_H__ */
