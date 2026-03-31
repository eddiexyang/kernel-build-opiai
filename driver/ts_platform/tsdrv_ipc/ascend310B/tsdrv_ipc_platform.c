/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
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
 * Create: 2021-12-20
 */
#include "devdrv_common.h"
#include "drv_ipc.h"
#include "tsdrv_ipc.h"

static struct tsdrv_ipc_chan g_ipc_chan_table[DEVDRV_MAX_DAVINCI_NUM][HISI_RPROC_MAX] = {
    {
        {0, 0, "RX_TS_MBX8", HISI_RPROC_TX_TS_ACPU0, HISI_RPROC_RX_TS_ACPU0},
        {0, 0, "RX_TS_ACPU1", HISI_RPROC_TX_TS_ACPU1, HISI_RPROC_RX_TS_ACPU1},

        /* end of array. */
        {0, 0, NULL, HISI_RPROC_MAX, HISI_RPROC_MAX},
    },
};

struct tsdrv_ipc_chan *tsdrv_get_ipc_chan_table(u32 devid)
{
    return g_ipc_chan_table[devid];
}
