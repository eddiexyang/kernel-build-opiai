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

#ifndef __HISI_RPROC_ADAPT_H__
#define __HISI_RPROC_ADAPT_H__

#include <linux/notifier.h>
#include <linux/mutex.h>

#include "drv_ipc.h"

#define MAX_IPCDEV_NUM              4
#define HISI_MBX_SEGMENT_SIZE_BIT   7   /* 128 */
#define MBOX_CHAN_DATA_SIZE 8
#define MAX_BUFFER_LEN   32
#define MAX_FIFO_LEN     128

/* lp mbx */
/* ao lp normal mbx */
#define IPC_LP_MBX0_RPID0_RX_ACPU0 "AO_IPC_LP_MBX0_RPID0_RX_ACPU0"
#define IPC_LP_MBX1_RPID1_RX_ACPU1 "AO_IPC_LP_MBX1_RPID1_RX_ACPU1"
#define IPC_LP_MBX2_RPID2_RX_ACPU2 "AO_IPC_LP_MBX2_RPID2_RX_ACPU2"
#define IPC_LP_MBX3_RPID3_RX_ACPU3 "AO_IPC_LP_MBX3_RPID3_RX_ACPU3"
#define IPC_LP_MBX5_RPID4_TX_ACPU0 "AO_IPC_LP_MBX5_RPID5_TX_ACPU0"
#define IPC_LP_MBX6_RPID4_TX_ACPU1 "AO_IPC_LP_MBX6_RPID6_TX_ACPU1"
#define IPC_LP_MBX7_RPID4_TX_ACPU2 "AO_IPC_LP_MBX7_RPID7_TX_ACPU2"
#define IPC_LP_MBX8_RPID4_TX_ACPU3 "AO_IPC_LP_MBX8_RPID8_TX_ACPU3"
#define IPC_LP_MBX9_RPID4_TX_ACPU "AO_IPC_LP_MBX9_RPID4_TX_ACPU"
#define IPC_LP_MBX10_RPID4_TX_ACPU "AO_IPC_LP_MBX10_RPID4_TX_ACPU"

/* stars ts normal mbx */
#define IPC_TS_MBX0_RPID0_TX_ACPU0 "AO_IPC_TS_MBX0_RPID0_TX_ACPU0"
#define IPC_TS_MBX1_RPID1_TX_ACPU1 "AO_IPC_TS_MBX1_RPID1_TX_ACPU1"
#define IPC_TS_MBX2_RPID2_TX_ACPU2 "AO_IPC_TS_MBX2_RPID2_TX_ACPU2"
#define IPC_TS_MBX3_RPID3_TX_ACPU3 "AO_IPC_TS_MBX3_RPID3_TX_ACPU3"
#define IPC_TS_MBX8_RPID8_RX_ACPU0 "AO_IPC_TS_MBX8_RPID8_RX_ACPU0"
#define IPC_TS_MBX9_RPID9_RX_ACPU1 "AO_IPC_TS_MBX9_RPID9_RX_ACPU1"
#define IPC_TS_MBX10_RPID10_RX_ACPU2 "AO_IPC_TS_MBX10_RPID10_RX_ACPU2"
#define IPC_TS_MBX11_RPID11_RX_ACPU3 "AO_IPC_TS_MBX11_RPID11_RX_ACPU3"

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
/* TS rprocess id is not the same */
typedef enum {
    ACPU0 = 0,
    ACPU1,
    ACPU2,
    ACPU3,
    TS_ACPU0 = 8,
    TS_ACPU1,
    TS_ACPU2,
    TS_ACPU3,
    UNCERTAIN_REMOTE_PROCESSOR
} remote_processor_type_t;

void mbox_clear_mbix_eoi_irq(u32 irq);

#endif
