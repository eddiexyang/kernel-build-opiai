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

#include <linux/wait.h>
#include "hisi_rproc_adapt.h"

struct hisi_rproc_info g_rproc_table[MAX_IPCDEV_NUM][HISI_RPROC_MAX] = {
    { /* rx channels */
        {
            .rproc_id = HISI_RPROC_RX_IMU_MBX0,
            .mbox_rp = HISI_MAILBOX_RP_MBX0,
            .symmetry_id = HISI_RPROC_MAX,
        },
        {
            .rproc_id = HISI_RPROC_RX_IMU_MBX1,
            .mbox_rp = HISI_MAILBOX_RP_MBX1,
            .symmetry_id = HISI_RPROC_MAX,
        },
        {
            .rproc_id = HISI_RPROC_RX_TS_MBX2,
            .mbox_rp = HISI_MAILBOX_RP_MBX2,
            .symmetry_id = HISI_RPROC_MAX,
        },
        {
            .rproc_id = HISI_RPROC_RX_IMU_MBX3,
            .mbox_rp = HISI_MAILBOX_RP_MBX3,
            .symmetry_id = HISI_RPROC_MAX,
        },

        /* tx channels */
        {
            .rproc_id = HISI_RPROC_TX_TS_MBX17,
            .mbox_rp = HISI_MAILBOX_RP_MBX17,
            .symmetry_id = HISI_RPROC_RX_TS_MBX2,
        },
        {
            .rproc_id = HISI_RPROC_TX_IMU_MBX24,
            .mbox_rp = HISI_MAILBOX_RP_MBX24,
            .symmetry_id = HISI_RPROC_RX_IMU_MBX0,
        },
        {
            .rproc_id = HISI_RPROC_TX_IMU_MBX25,
            .mbox_rp = HISI_MAILBOX_RP_MBX25,
            .symmetry_id = HISI_RPROC_RX_IMU_MBX1,
        }
    },
    { /* rx channels */
        {
            .rproc_id = HISI_RPROC_RX_IMU_MBX0,
            .mbox_rp = HISI_MAILBOX_RP_MBX0,
            .symmetry_id = HISI_RPROC_MAX,

        },
        {
            .rproc_id = HISI_RPROC_RX_IMU_MBX1,
            .mbox_rp = HISI_MAILBOX_RP_MBX1,
            .symmetry_id = HISI_RPROC_MAX,
        },
        {
            .rproc_id = HISI_RPROC_RX_TS_MBX2,
            .mbox_rp = HISI_MAILBOX_RP_MBX2,
            .symmetry_id = HISI_RPROC_MAX,
        },
        {
            .rproc_id = HISI_RPROC_RX_IMU_MBX3,
            .mbox_rp = HISI_MAILBOX_RP_MBX3,
            .symmetry_id = HISI_RPROC_MAX,
        },
        /* tx channels */
        {
            .rproc_id = HISI_RPROC_TX_TS_MBX17,
            .mbox_rp = HISI_MAILBOX_RP_MBX17,
            .symmetry_id = HISI_RPROC_RX_TS_MBX2,
        },
        {
            .rproc_id = HISI_RPROC_TX_IMU_MBX24,
            .mbox_rp = HISI_MAILBOX_RP_MBX24,
            .symmetry_id = HISI_RPROC_RX_IMU_MBX0,
        },
        {
            .rproc_id = HISI_RPROC_TX_IMU_MBX25,
            .mbox_rp = HISI_MAILBOX_RP_MBX25,
            .symmetry_id = HISI_RPROC_RX_IMU_MBX1,
        }
    },
    { /* rx channels */
        {
            .rproc_id = HISI_RPROC_RX_IMU_MBX0,
            .mbox_rp = HISI_MAILBOX_RP_MBX0,
            .symmetry_id = HISI_RPROC_MAX,
        },
        {
            .rproc_id = HISI_RPROC_RX_IMU_MBX1,
            .mbox_rp = HISI_MAILBOX_RP_MBX1,
            .symmetry_id = HISI_RPROC_MAX,
        },
        {
            .rproc_id = HISI_RPROC_RX_TS_MBX2,
            .mbox_rp = HISI_MAILBOX_RP_MBX2,
            .symmetry_id = HISI_RPROC_MAX,
        },
        {
            .rproc_id = HISI_RPROC_RX_IMU_MBX3,
            .mbox_rp = HISI_MAILBOX_RP_MBX3,
            .symmetry_id = HISI_RPROC_MAX,
        },
        /* tx channels */
        {
            .rproc_id = HISI_RPROC_TX_TS_MBX17,
            .mbox_rp = HISI_MAILBOX_RP_MBX17,
            .symmetry_id = HISI_RPROC_RX_TS_MBX2,
        },
        {
            .rproc_id = HISI_RPROC_TX_IMU_MBX24,
            .mbox_rp = HISI_MAILBOX_RP_MBX24,
            .symmetry_id = HISI_RPROC_RX_IMU_MBX0,
        },
        {
            .rproc_id = HISI_RPROC_TX_IMU_MBX25,
            .mbox_rp = HISI_MAILBOX_RP_MBX25,
            .symmetry_id = HISI_RPROC_RX_IMU_MBX1,
        }
    },
    { /* rx channels */
        {
            .rproc_id = HISI_RPROC_RX_IMU_MBX0,
            .mbox_rp = HISI_MAILBOX_RP_MBX0,
            .symmetry_id = HISI_RPROC_MAX,
        },
        {
            .rproc_id = HISI_RPROC_RX_IMU_MBX1,
            .mbox_rp = HISI_MAILBOX_RP_MBX1,
            .symmetry_id = HISI_RPROC_MAX,
        },
        {
            .rproc_id = HISI_RPROC_RX_TS_MBX2,
            .mbox_rp = HISI_MAILBOX_RP_MBX2,
            .symmetry_id = HISI_RPROC_MAX,
        },
        {
            .rproc_id = HISI_RPROC_RX_IMU_MBX3,
            .mbox_rp = HISI_MAILBOX_RP_MBX3,
            .symmetry_id = HISI_RPROC_MAX,
        },
        /* tx channels */
        {
            .rproc_id = HISI_RPROC_TX_TS_MBX17,
            .mbox_rp = HISI_MAILBOX_RP_MBX17,
            .symmetry_id = HISI_RPROC_RX_TS_MBX2,
        },
        {
            .rproc_id = HISI_RPROC_TX_IMU_MBX24,
            .mbox_rp = HISI_MAILBOX_RP_MBX24,
            .symmetry_id = HISI_RPROC_RX_IMU_MBX0,
        },
        {
            .rproc_id = HISI_RPROC_TX_IMU_MBX25,
            .mbox_rp = HISI_MAILBOX_RP_MBX25,
            .symmetry_id = HISI_RPROC_RX_IMU_MBX1,
        }
    }
};
