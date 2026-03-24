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
#include "devdrv_common.h"
#ifndef TSDRV_KERNEL_UT
#include "tsdrv_pci_chan.h"
#include "tsdrv_d2h_chan.h"
#include "tsdrv_log.h"

int tsdrv_d2h_chan_setup(void)
{
    const struct tsdrv_d2h_chan_ops *g_d2h_chan_ops = tsdrv_get_d2h_chan_ops();

    if (g_d2h_chan_ops->d2h_chan_setup != NULL) {
        return g_d2h_chan_ops->d2h_chan_setup();
    }
    TSDRV_PRINT_ERR("d2h_chan_setup is NULL\n");
    return -EINVAL;
}

void tsdrv_d2h_chan_cleanup(void)
{
    const struct tsdrv_d2h_chan_ops *g_d2h_chan_ops = tsdrv_get_d2h_chan_ops();

    if (g_d2h_chan_ops->d2h_chan_cleanup == NULL) {
        TSDRV_PRINT_ERR("d2h_chan_cleanup is NULL\n");
        return;
    }
    g_d2h_chan_ops->d2h_chan_cleanup();
}

bool tsdrv_d2h_slow_chan_ready(u32 devid)
{
    const struct tsdrv_d2h_chan_ops *g_d2h_chan_ops = tsdrv_get_d2h_chan_ops();

    if (g_d2h_chan_ops->d2h_slow_chan_ready != NULL) {
        return g_d2h_chan_ops->d2h_slow_chan_ready(devid);
    }
    return false;
}

int tsdrv_d2h_slow_msg_send(u32 devid, void *tx, size_t tx_size)
{
    const struct tsdrv_d2h_chan_ops *g_d2h_chan_ops = tsdrv_get_d2h_chan_ops();

    if (g_d2h_chan_ops->d2h_slow_msg_send != NULL) {
        return g_d2h_chan_ops->d2h_slow_msg_send(devid, tx, tx_size);
    }
    TSDRV_PRINT_ERR("d2h_slow_msg_send is NULL, devid=%u\n", devid);
    return -EINVAL;
}
#else
int tsdrv_d2h_fast_chan_init(u32 devid)
{
    return 0;
}
#endif
