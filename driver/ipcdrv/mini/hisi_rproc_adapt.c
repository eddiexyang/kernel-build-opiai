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
    { /* tx channels */
        {
            .rproc_id = HISI_RPROC_TX_TS,
            .mbox_rp = HISI_MAILBOX_RP_TX_TS,
        },
        {
            .rproc_id = HISI_RPROC_TX_LPM3,
            .mbox_rp = HISI_MAILBOX_RP_TX_LPM3,
        },
        /* rx channels */
        {
            .rproc_id = HISI_RPROC_RX_TS_MBX4,
            .mbox_rp = HISI_MAILBOX_RP_RX_TS_MBX4,
        },
        {
            .rproc_id = HISI_RPROC_RX_LPM3_MBX5,
            .mbox_rp = HISI_MAILBOX_RP_RX_M3_MBX5,
        },
        {
            .rproc_id = HISI_RPROC_RX_TS_MBX6,
            .mbox_rp = HISI_MAILBOX_RP_RX_TS_MBX6,
        },
        {
            .rproc_id = HISI_RPROC_RX_LPM3_MBX7,
            .mbox_rp = HISI_MAILBOX_RP_RX_M3_MBX7,
        },
        {
            .rproc_id = HISI_RPROC_RX_TS_MBX8,
            .mbox_rp = HISI_MAILBOX_RP_RX_TS_MBX8,
        },
        {
            .rproc_id = HISI_RPROC_RX_LPM3_MBX9,
            .mbox_rp = HISI_MAILBOX_RP_RX_M3_MBX9,
        },
        {
            .rproc_id = HISI_RPROC_RX_TS_MBX10,
            .mbox_rp = HISI_MAILBOX_RP_RX_TS_MBX10,
        },
        {
            .rproc_id = HISI_RPROC_RX_LPM3_MBX11,
            .mbox_rp = HISI_MAILBOX_RP_RX_M3_MBX11,
        }
    }
};
