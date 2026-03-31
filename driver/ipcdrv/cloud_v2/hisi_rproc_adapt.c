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
    {   /* ts rx channels */
        {
            .rproc_id = HISI_RPROC_RX_TS_MBX8,
            .mbox_rp = HISI_MAILBOX_RP_MBX8,
            .symmetry_id = HISI_RPROC_MAX,
        },
        {
            .rproc_id = HISI_RPROC_RX_TS_MBX9,
            .mbox_rp = HISI_MAILBOX_RP_MBX9,
            .symmetry_id = HISI_RPROC_MAX,
        },
        {
            .rproc_id = HISI_RPROC_RX_TS_MBX10,
            .mbox_rp = HISI_MAILBOX_RP_MBX10,
            .symmetry_id = HISI_RPROC_MAX,
        },
        {
            .rproc_id = HISI_RPROC_RX_TS_MBX11,
            .mbox_rp = HISI_MAILBOX_RP_MBX11,
            .symmetry_id = HISI_RPROC_MAX,
        },
        /* ts tx channels */
        {
            .rproc_id = HISI_RPROC_TX_TS_MBX2,
            .mbox_rp = HISI_MAILBOX_RP_MBX2,
            .symmetry_id = HISI_RPROC_MAX,
        },
        {
            .rproc_id = HISI_RPROC_TX_TS_MBX3,
            .mbox_rp = HISI_MAILBOX_RP_MBX3,
            .symmetry_id = HISI_RPROC_MAX,
        },
        {
            .rproc_id = HISI_RPROC_TX_TS_MBX4,
            .mbox_rp = HISI_MAILBOX_RP_MBX4,
            .symmetry_id = HISI_RPROC_MAX,
        },
        {
            .rproc_id = HISI_RPROC_TX_TS_MBX5,
            .mbox_rp = HISI_MAILBOX_RP_MBX5,
            .symmetry_id = HISI_RPROC_MAX,
        },

        /* lp rx channels */
        {
            .rproc_id = HISI_RPROC_RX_IMU_MBX20,
            .mbox_rp = HISI_MAILBOX_RP_MBX20,
            .symmetry_id = HISI_RPROC_MAX,
        },
        {
            .rproc_id = HISI_RPROC_RX_IMU_MBX21,
            .mbox_rp = HISI_MAILBOX_RP_MBX21,
            .symmetry_id = HISI_RPROC_MAX,
        },
        {
            .rproc_id = HISI_RPROC_RX_IMU_MBX22,
            .mbox_rp = HISI_MAILBOX_RP_MBX22,
            .symmetry_id = HISI_RPROC_MAX,
        },
        {
            .rproc_id = HISI_RPROC_RX_IMU_MBX23,
            .mbox_rp = HISI_MAILBOX_RP_MBX23,
            .symmetry_id = HISI_RPROC_MAX,
        },

        /* lp tx channels */
        {
            .rproc_id = HISI_RPROC_TX_IMU_MBX28,
            .mbox_rp = HISI_MAILBOX_RP_MBX28,
            .symmetry_id = HISI_RPROC_MAX,
        },
        {
            .rproc_id = HISI_RPROC_TX_IMU_MBX29,
            .mbox_rp = HISI_MAILBOX_RP_MBX29,
            .symmetry_id = HISI_RPROC_MAX,
        }
    },
    {   /* ts rx channels */
        {
            .rproc_id = HISI_RPROC_RX_TS_MBX8,
            .mbox_rp = HISI_MAILBOX_RP_MBX8,
            .symmetry_id = HISI_RPROC_MAX,
        },
        {
            .rproc_id = HISI_RPROC_RX_TS_MBX9,
            .mbox_rp = HISI_MAILBOX_RP_MBX9,
            .symmetry_id = HISI_RPROC_MAX,
        },
        {
            .rproc_id = HISI_RPROC_RX_TS_MBX10,
            .mbox_rp = HISI_MAILBOX_RP_MBX10,
            .symmetry_id = HISI_RPROC_MAX,
        },
        {
            .rproc_id = HISI_RPROC_RX_TS_MBX11,
            .mbox_rp = HISI_MAILBOX_RP_MBX11,
            .symmetry_id = HISI_RPROC_MAX,
        },
        /* ts tx channels */
        {
            .rproc_id = HISI_RPROC_TX_TS_MBX2,
            .mbox_rp = HISI_MAILBOX_RP_MBX2,
            .symmetry_id = HISI_RPROC_MAX,
        },
        {
            .rproc_id = HISI_RPROC_TX_TS_MBX3,
            .mbox_rp = HISI_MAILBOX_RP_MBX3,
            .symmetry_id = HISI_RPROC_MAX,
        },
        {
            .rproc_id = HISI_RPROC_TX_TS_MBX4,
            .mbox_rp = HISI_MAILBOX_RP_MBX4,
            .symmetry_id = HISI_RPROC_MAX,
        },
        {
            .rproc_id = HISI_RPROC_TX_TS_MBX5,
            .mbox_rp = HISI_MAILBOX_RP_MBX5,
            .symmetry_id = HISI_RPROC_MAX,
        },

        /* lp rx channels */
        {
            .rproc_id = HISI_RPROC_RX_IMU_MBX20,
            .mbox_rp = HISI_MAILBOX_RP_MBX20,
            .symmetry_id = HISI_RPROC_MAX,
        },
        {
            .rproc_id = HISI_RPROC_RX_IMU_MBX21,
            .mbox_rp = HISI_MAILBOX_RP_MBX21,
            .symmetry_id = HISI_RPROC_MAX,
        },
        {
            .rproc_id = HISI_RPROC_RX_IMU_MBX22,
            .mbox_rp = HISI_MAILBOX_RP_MBX22,
            .symmetry_id = HISI_RPROC_MAX,
        },
        {
            .rproc_id = HISI_RPROC_RX_IMU_MBX23,
            .mbox_rp = HISI_MAILBOX_RP_MBX23,
            .symmetry_id = HISI_RPROC_MAX,
        },

        /* lp tx channels */
        {
            .rproc_id = HISI_RPROC_TX_IMU_MBX28,
            .mbox_rp = HISI_MAILBOX_RP_MBX28,
            .symmetry_id = HISI_RPROC_MAX,
        },
        {
            .rproc_id = HISI_RPROC_TX_IMU_MBX29,
            .mbox_rp = HISI_MAILBOX_RP_MBX29,
            .symmetry_id = HISI_RPROC_MAX,
        }
    }
};

#if defined(UT_TEST) || defined(ST_TEST)
int for_ut_test(void)
{
    return 0;
}
#endif

