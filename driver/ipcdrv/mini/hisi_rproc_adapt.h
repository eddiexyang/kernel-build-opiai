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

#include <linux/notifier.h>
#include "drv_ipc.h"

#define MAX_IPCDEV_NUM 1
#define HISI_MBX_SEGMENT_SIZE_BIT 6 // 64
/* mailbox's channel size, does  portland use the same? */
#define MBOX_CHAN_DATA_SIZE 8 // 24 in minv2
#define MAX_BUFFER_LEN   32
#define MAX_FIFO_LEN     128

#define HISI_MAILBOX_RP_TX_TS "MINI_TS_MBX0"
#define HISI_MAILBOX_RP_TX_LPM3 "MINI_LPM3_MBX2"
#define HISI_MAILBOX_RP_RX_TS_MBX4 "MINI_TS_MBX4"
#define HISI_MAILBOX_RP_RX_M3_MBX5 "MINI_LPM3_MBX5"
#define HISI_MAILBOX_RP_RX_TS_MBX6 "MINI_TS_MBX6"
#define HISI_MAILBOX_RP_RX_M3_MBX7 "MINI_LPM3_MBX7"
#define HISI_MAILBOX_RP_RX_TS_MBX8 "MINI_TS_MBX8"
#define HISI_MAILBOX_RP_RX_M3_MBX9 "MINI_LPM3_MBX9"
#define HISI_MAILBOX_RP_RX_TS_MBX10 "MINI_TS_MBX10"
#define HISI_MAILBOX_RP_RX_M3_MBX11 "MINI_LPM3_MBX11"

struct hisi_rproc_info {
    rproc_id_t rproc_id;
    const char *mbox_rp;
    struct atomic_notifier_head notifier;
    struct notifier_block nb;
    struct hisi_mbox *mbox;
    struct mutex rproc_mutex;
    u32 register_flag;
};

#define HISI_MAX_LPI_IRQ_NUM 4

#define HISI_ACPU0_ACK_IRQ_IDX 0 /* aicpu_0 ipc_rcv ack */
#define HISI_ACPU1_ACK_IRQ_IDX 1 /* aicpu_1 ipc_rcv ack */
#define HISI_ACPU2_ACK_IRQ_IDX 2 /* aicpu_2 ipc_rcv ack */
#define HISI_ACPU3_ACK_IRQ_IDX 3 /* aicpu_3 ipc_rcv ack */

/*
 * Table for available remote processors. DTS sub-node, "remote_processor_type",
 * of node, "hisi_mdev", is configured according to the table.
 *
 * If the table was modified, DTS configiuration should be updated accordingly.
 */
typedef enum {
    TS = 0,
    LPM3,
    ACPU0,
    ACPU1,
    ACPU2,
    ACPU3,
    ACPU4,
    ACPU5,
    ACPU6,
    ACPU7,
    UNCERTAIN_REMOTE_PROCESSOR
} remote_processor_type_t;

extern int rproc_init(int ipc_id, int mdev_num);
extern void hisi_rproc_exit(int ipc_id);
#endif
