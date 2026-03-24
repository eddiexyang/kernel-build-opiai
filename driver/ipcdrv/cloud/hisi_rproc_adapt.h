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

#define MAX_IPCDEV_NUM 4
#define HISI_MBX_SEGMENT_SIZE_BIT 6 // 64
/* mailbox's channel size, does  portland use the same? */
#define MBOX_CHAN_DATA_SIZE 8 // 24 in minv2
#define MAX_BUFFER_LEN   32
#define MAX_FIFO_LEN     128

/* recv mbx */
#define HISI_MAILBOX_RP_MBX0 "HI_CLOUD_IMU_0"
#define HISI_MAILBOX_RP_MBX1 "HI_CLOUD_IMU_1"
#define HISI_MAILBOX_RP_MBX2 "HI_CLOUD_TS"
#define HISI_MAILBOX_RP_MBX3 "HI_CLOUD_IMU_3"

/* send mbx */
#define HISI_MAILBOX_RP_MBX17 "HI_CLOUD_TX_TS_MBX17"
#define HISI_MAILBOX_RP_MBX24 "HI_CLOUD_TX_IMU_MBX24"
#define HISI_MAILBOX_RP_MBX25 "HI_CLOUD_TX_IMU_MBX25"

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

#define HISI_MAX_LPI_IRQ_NUM 45
#define HISI_RX_IMU_MBX0_IRQ_IDX 0 /* imu to core0 */
#define HISI_RX_IMU_MBX1_IRQ_IDX 1 /* imu to core1 */
#define HISI_RX_IMU_MBX2_IRQ_IDX 2 /* ts to core2 */

#define HISI_ACPU0_ACK_IRQ_IDX 25 /* aicpu_0 ipc_rcv ack */
#define HISI_ACPU1_ACK_IRQ_IDX 26 /* aicpu_1 ipc_rcv ack */
#define HISI_ACPU2_ACK_IRQ_IDX 27 /* aicpu_2 ipc_rcv ack */
#define HISI_ACPU3_ACK_IRQ_IDX 28 /* aicpu_3 ipc_rcv ack */

#define HISI_ACPU2_TO_TSCORE0_IDX 17

#define HISI_TS_ACPU0_TO_IMU_MBX24_IRQ 109
#define HISI_TS_ACPU1_TO_IMU_MBX25_IRQ 110

/*
 * Table for available remote processors. DTS sub-node, "remote_processor_type",
 * of node, "hisi_mdev", is configured according to the table.
 *
 * If the table was modified, DTS configiuration should be updated accordingly.
 */
typedef enum {
    ACPU0 = 0,
    ACPU1,
    ACPU2,
    ACPU3,
    ACPU4,
    ACPU5,
    ACPU6,
    ACPU7,
    TS0 = 16,
    TS1,
    TS2,
    TS3,
    IMU = 20,
    UNCERTAIN_REMOTE_PROCESSOR
} remote_processor_type_t;

extern int rproc_init(int ipc_id, int mdev_num);
extern void hisi_rproc_exit(int ipc_id);

#endif
