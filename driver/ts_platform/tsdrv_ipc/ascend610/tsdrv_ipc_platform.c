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

#include "devdrv_common.h"
#include "drv_ipc.h"
#include "tsdrv_ipc.h"

static struct tsdrv_ipc_chan g_ipc_chan_table[DEVDRV_MAX_DAVINCI_NUM][HISI_RPROC_MAX] = {
    {
        {0, 0, "TSC_RX_RPID4", HISI_RPROC_TSC_TX_RPID0, HISI_RPROC_TSC_RX_RPID4},
        {0, 1, "TSV_RX_RPID4", HISI_RPROC_TSV_TX_RPID0, HISI_RPROC_TSV_RX_RPID4},

        /* end of array. */
        {0, 0, NULL, HISI_RPROC_MAX, HISI_RPROC_MAX},
    },
    {
        {1, 0, "TSC_RX_RPID4", HISI_RPROC_TSC_TX_RPID0, HISI_RPROC_TSC_RX_RPID4},
        {1, 1, "TSV_RX_RPID4", HISI_RPROC_TSV_TX_RPID0, HISI_RPROC_TSV_RX_RPID4},

        /* end of array. */
        {0, 0, NULL, HISI_RPROC_MAX, HISI_RPROC_MAX},
    },
};

struct tsdrv_ipc_chan *tsdrv_get_ipc_chan_table(u32 devid)
{
    return g_ipc_chan_table[devid];
}
