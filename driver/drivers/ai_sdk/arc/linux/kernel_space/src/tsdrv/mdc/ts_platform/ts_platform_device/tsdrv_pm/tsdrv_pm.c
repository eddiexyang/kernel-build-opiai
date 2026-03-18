
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

#include <linux/delay.h>
#include <linux/limits.h>
#include <linux/kernel.h>

#include "devdrv_manager.h"
#include "drv_ipc.h"
#include "tsdrv_log.h"
#include "devdrv_ipc.h"
#include "tsdrv_ipc.h"
#include "tsdrv_pm.h"
#include "icm_interface.h"
#include "tsmng_interface.h"


#ifndef CFG_FEATURE_USE_ICM_CHAN
static void tsdrv_ts_suspend_handler(struct notifier_block *nb, unsigned long len, void *data)
{
    struct ipcdrv_message *ipc_msg = NULL;
    struct ipcdrv_msg_payload *payload = NULL;
    struct tsdrv_ipc_chan *ipc_chan = NULL;

    ipc_msg = (struct ipcdrv_message *)data;
    payload = (struct ipcdrv_msg_payload *)ipc_msg->ipcdrv_payload;
    ipc_chan = notifier_block_to_ipc_chan(nb);

    if (tsmng_chk_ipc_crc16(ipc_msg) != 0) {
        TSDRV_PRINT_ERR("crc mismatch. (devid=%u; tsid=%u)\n", ipc_chan->devid, ipc_chan->tsid);
        return;
    }

    TSDRV_PRINT_INFO("Receive TS suspend ack. (devid=%u; tsid=%u; result=%u)\n",
        ipc_chan->devid, ipc_chan->tsid, (u32)payload->result);
    tsdrv_ts_suspend_ready(ipc_chan->devid, ipc_chan->tsid, payload->result);
}
#else
static void tsdrv_ts_suspend_handler(struct notifier_block *nb, unsigned long len, void *data)
{
    struct tsdrv_ipc_chan *ipc_chan = NULL;
    struct icmdrv_ipc_msg_info *ack_data = NULL;

    ack_data = (struct icmdrv_ipc_msg_info *)data;
    ipc_chan = notifier_block_to_ipc_chan(nb);

    TSDRV_PRINT_INFO("Receive TS suspend ack. (devid=%u; tsid=%u; crc=%u; result=%u)\n",
        ipc_chan->devid, ipc_chan->tsid, (u32)ack_data->crc16, (u32)ack_data->data[0]);
    tsdrv_ts_suspend_ready(ipc_chan->devid, ipc_chan->tsid, ack_data->data[0]);
}
#endif

void tsdrv_pm_init(void)
{
#ifndef CFG_FEATURE_USE_ICM_CHAN
    u32 cmd_type = IPCDRV_TS_SUSPEND_READY;
#else
    u32 cmd_type = ICM_MAIN_CMD_TS_REQ;
#endif

    int ret = tsdrv_ipc_handler_register(cmd_type, tsdrv_ts_suspend_handler);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Register ts suspend handler failed.\n");
    }
}

void tsdrv_pm_exit(void)
{
#ifndef CFG_FEATURE_USE_ICM_CHAN
    u32 cmd_type = IPCDRV_TS_SUSPEND_READY;
#else
    u32 cmd_type = ICM_MAIN_CMD_TS_REQ;
#endif
    tsdrv_ipc_handler_unregister(cmd_type);
}
