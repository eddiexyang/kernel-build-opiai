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

#ifndef __HISI_RPROC_ADAPT_H__
#define __HISI_RPROC_ADAPT_H__

#include <uapi/linux/sched/types.h>
#include <linux/notifier.h>

#include "drv_ipc.h"

#define MAX_IPCDEV_NUM 2
#define HISI_MBX_SEGMENT_SIZE_BIT 7

#define MBOX_CHAN_DATA_SIZE 24 // 24 in milan
#define MAX_BUFFER_LEN   96
#define MAX_FIFO_LEN     384

/* recv mbx */
#define HISI_MAILBOX_RP_MBX8 "HI_CLOUD_TS_0"
#define HISI_MAILBOX_RP_MBX9 "HI_CLOUD_TS_1"
#define HISI_MAILBOX_RP_MBX10 "HI_CLOUD_TS_2"
#define HISI_MAILBOX_RP_MBX11 "HI_CLOUD_TS_3"

#define HISI_MAILBOX_RP_MBX20 "HI_CLOUD_IMU_0"
#define HISI_MAILBOX_RP_MBX21 "HI_CLOUD_IMU_1"
#define HISI_MAILBOX_RP_MBX22 "HI_CLOUD_IMU_2"
#define HISI_MAILBOX_RP_MBX23 "HI_CLOUD_IMU_3"

/* send mbx */
#define HISI_MAILBOX_RP_MBX2 "HI_CLOUD_TX_TS_MBX2"
#define HISI_MAILBOX_RP_MBX3 "HI_CLOUD_TX_TS_MBX3"
#define HISI_MAILBOX_RP_MBX4 "HI_CLOUD_TX_TS_MBX4"
#define HISI_MAILBOX_RP_MBX5 "HI_CLOUD_TX_TS_MBX5"

#define HISI_MAILBOX_RP_MBX28 "HI_CLOUD_TX_IMU_MBX28"
#define HISI_MAILBOX_RP_MBX29 "HI_CLOUD_TX_IMU_MBX29"

struct hisi_rproc_info {
    rproc_id_t rproc_id;
    const char *mbox_rp;
    struct atomic_notifier_head notifier;
    struct notifier_block nb;
    struct hisi_mbox *mbox;
    rproc_id_t symmetry_id;  // we  need the symmetry id to clear lpi interrupts in sync send
    struct mutex rproc_mutex;
    u32 register_flag;
};

#define HISI_MAX_LPI_IRQ_NUM 12

#define HISI_ACPU0_ACK_IRQ_IDX 8 /* aicpu_0 ipc_rcv ack */
#define HISI_ACPU1_ACK_IRQ_IDX 9 /* aicpu_1 ipc_rcv ack */
#define HISI_ACPU2_ACK_IRQ_IDX 10 /* aicpu_2 ipc_rcv ack */
#define HISI_ACPU3_ACK_IRQ_IDX 11 /* aicpu_3 ipc_rcv ack */
/*
 * Table for available remote processors. DTS sub-node, "remote_processor_type",
 * of node, "hisi_mdev", is configured according to the table.
 *
 * If the table was modified, DTS configiuration should be updated accordingly.
 */
typedef enum {
    TS_ACPU0 = 8,
    TS_ACPU1,
    TS_ACPU2,
    TS_ACPU3,
    IMU_ACPU0 = 20,
    IMU_ACPU1,
    IMU_ACPU2,
    IMU_ACPU3,
    UNCERTAIN_REMOTE_PROCESSOR
} remote_processor_type_t;

extern int rproc_init(int ipc_id, int mdev_num);
extern void hisi_rproc_exit(int ipc_id);

#endif
