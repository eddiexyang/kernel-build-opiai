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

#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/notifier.h>
#include "drv_ipc.h"
#include "devdrv_platform.h"
#include "devdrv_manager.h"
#include "tsdrv_log.h"
#include "tsdrv_pdata.h"

#include "tsdrv_ipc.h"
#include "tsdrv_ipc_dfx.h"
#include "icm_interface.h"
#include "tsmng_interface.h"

#ifndef CFG_FEATURE_USE_ICM_CHAN
    static void (*ts_ipc_rx_handlers[IPCDRV_TS_MAX])(struct notifier_block *nb, unsigned long len, void *data);
#else
    static void (*ts_ipc_rx_handlers[ICM_MAIN_CMD_MAX])(struct notifier_block *nb, unsigned long len, void *data);
#endif

static int tsdrv_rproc_rx_register(struct tsdrv_ipc_chan *ipc_chan)
{
    return rproc_rx_register(ipc_chan->devid, ipc_chan->rx_rproc_id, &ipc_chan->nb);
}

static void tsdrv_rproc_rx_unregister(struct tsdrv_ipc_chan *ipc_chan)
{
    (void)rproc_rx_unregister(ipc_chan->devid, ipc_chan->rx_rproc_id, &ipc_chan->nb);
}

int tsdrv_ipc_handler_register(u32 cmd_type,
    void (* ipc_handlers)(struct notifier_block *nb, unsigned long len, void *data))
{
#ifndef CFG_FEATURE_USE_ICM_CHAN
    u32 max_cmd_type = IPCDRV_TS_MAX;
#else
    u32 max_cmd_type = ICM_MAIN_CMD_MAX;
#endif

    if (cmd_type >= max_cmd_type) {
        TSDRV_PRINT_ERR("Cmd type is invalid. (cmd_type=%u)\n", cmd_type);
        return -EINVAL;
    }

    if (ts_ipc_rx_handlers[cmd_type] != NULL) {
        TSDRV_PRINT_ERR("Ipc handler has been register. (cmd_type=%u)\n", cmd_type);
        return -EINVAL;
    }

    ts_ipc_rx_handlers[cmd_type] = ipc_handlers;
    return 0;
}

void tsdrv_ipc_handler_unregister(u32 cmd_type)
{
#ifndef CFG_FEATURE_USE_ICM_CHAN
    u32 max_cmd_type = IPCDRV_TS_MAX;
#else
    u32 max_cmd_type = ICM_MAIN_CMD_MAX;
#endif

    if (cmd_type >= max_cmd_type) {
        TSDRV_PRINT_ERR("Cmd type is invalid. (cmd_type=%u)\n", cmd_type);
        return;
    }

    ts_ipc_rx_handlers[cmd_type] = NULL;
    return;
}

int tsdrv_ipc_msg_send(struct tsdrv_ipc_chan *ipc_chan, rproc_msg_t *resp_msg)
{
    int ret;

    ret = rproc_xfer_async(ipc_chan->devid, ipc_chan->tx_rproc_id,
        resp_msg, IPCDRV_RPROC_MSG_LENGTH);
    if (ret != 0) {
        TSDRV_PRINT_DEBUG("Send rproc msg failed. (devid=%u; tsid=%u)\n",
            ipc_chan->devid, ipc_chan->tsid);
    }

    return ret;
}

static int tsdrv_ipc_chan_proc(struct notifier_block *nb, unsigned long len, void *data)
{
#ifndef CFG_FEATURE_USE_ICM_CHAN
    struct ipcdrv_message *ipc_msg = NULL;
    size_t expect_len = sizeof(struct ipcdrv_message) / sizeof(u32);
#else
    struct icmdrv_ipc_msg_info *icm_ipc_msg = NULL;
    size_t expect_len = sizeof(struct icmdrv_ipc_msg_info) / sizeof(u32);
#endif
    u32 cmd_type, max_cmd_type;

    if ((nb == NULL) || (data == NULL)) {
        TSDRV_PRINT_ERR("Notifier block or data is null.\n");
        return NOTIFY_DONE;
    }

    if (len != (unsigned long)expect_len) {
        TSDRV_PRINT_ERR("Ipc message len is invalid. (len=0x%lx; expect_len=0x%lx)\n",
            len, (unsigned long)expect_len);
        return NOTIFY_DONE;
    }

#ifndef CFG_FEATURE_USE_ICM_CHAN
    ipc_msg = (struct ipcdrv_message *)data;
    cmd_type = ipc_msg->ipc_msg_header.cmd_type;
    max_cmd_type = IPCDRV_TS_MAX;
#else
    icm_ipc_msg = (struct icmdrv_ipc_msg_info *)data;
    cmd_type = icm_ipc_msg->cmd;
    max_cmd_type = ICM_MAIN_CMD_MAX;
#endif

    if (cmd_type >= max_cmd_type) {
        TSDRV_PRINT_ERR("Cmd type is invalid. (cmd=%u; max=%u)\n", cmd_type, max_cmd_type - 1);
        return NOTIFY_DONE;
    }

    if (ts_ipc_rx_handlers[cmd_type] == NULL) {
        TSDRV_PRINT_DEBUG("Ipc notifier handler does not register. (cmd_type=%u)\n", cmd_type);
        return NOTIFY_DONE;
    }

    TSDRV_PRINT_DEBUG("Ipc notifier process. (cmd_type=%u)\n", cmd_type);
    ts_ipc_rx_handlers[cmd_type](nb, len, data);
    return NOTIFY_DONE;
}

static int tsdrv_ts_ipc_register(u32 devid)
{
    int ret;
    u32 idx;
    u32 tmp_num;
    struct tsdrv_ipc_chan *ipc_chan = NULL;

    /* cannot be null. */
    ipc_chan = tsdrv_get_ipc_chan_table(devid);
    for (idx = 0; idx < HISI_RPROC_MAX; idx++) {
        if (ipc_chan[idx].nb_name == NULL) {
            break;
        }

        TSDRV_PRINT_INFO("Register ipc notifier. (devid=%u; name=%s)\n",
            ipc_chan[idx].devid, ipc_chan[idx].nb_name);
        ipc_chan[idx].nb.notifier_call = tsdrv_ipc_chan_proc;
        ipc_chan[idx].nb.next = NULL;
        ipc_chan[idx].nb.priority = 0;

        ret = tsdrv_rproc_rx_register(&ipc_chan[idx]);
        if (ret != 0) {
            TSDRV_PRINT_ERR("Register rproc rx failed. (devid=%u)\n", devid);
            goto register_fail;
        }
    }
    return 0;

register_fail:
    tmp_num = idx;
    for (idx = 0; idx < tmp_num; idx++) {
        tsdrv_rproc_rx_unregister(&ipc_chan[idx]);
    }

    return ret;
}

static void tsdrv_ts_ipc_unregister(u32 devid)
{
    u32 idx;
    struct tsdrv_ipc_chan *ipc_chan = NULL;

    /* cannot be null. */
    ipc_chan = tsdrv_get_ipc_chan_table(devid);
    for (idx = 0; idx < HISI_RPROC_MAX; idx++) {
        if (ipc_chan[idx].nb_name == NULL) {
            break;
        }

        tsdrv_rproc_rx_unregister(&ipc_chan[idx]);
    }
}

void tsdrv_ipc_register(u32 devid)
{
    int ret;

    if (devid >= DEVDRV_MAX_DAVINCI_NUM) {
        TSDRV_PRINT_ERR("Device id is invalid. (devid=%u)\n", devid);
        return;
    }

    ret = tsdrv_ts_ipc_register(devid);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Register ipc failed. (devid=%u)\n", devid);
    }
}

void tsdrv_ipc_unregister(u32 devid)
{
    if (devid >= DEVDRV_MAX_DAVINCI_NUM) {
        TSDRV_PRINT_ERR("Device id is invalid. (devid=%u)\n", devid);
        return;
    }

    tsdrv_ts_ipc_unregister(devid);
}
