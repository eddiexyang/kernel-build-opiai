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

#ifndef TSDRV_IPC_H
#define TSDRV_IPC_H

#include <linux/types.h>
#include <linux/notifier.h>

#include "drv_ipc.h"

#ifdef __cplusplus
extern "C" {
#endif

struct tsdrv_ipc_chan {
    u32 devid;
    u32 tsid;
    char *nb_name;
    rproc_id_t tx_rproc_id;
    rproc_id_t rx_rproc_id;
    struct notifier_block nb;
};

#ifdef CFG_FEATURE_TS_IPC
void tsdrv_ipc_register(u32 devid);
void tsdrv_ipc_unregister(u32 devid);
int tsdrv_ipc_msg_send(struct tsdrv_ipc_chan *ipc_chan, rproc_msg_t *resp_msg);
int tsdrv_ipc_handler_register(u32 cmd_type,
    void (* ipc_handlers)(struct notifier_block *nb, unsigned long len, void *data));
void tsdrv_ipc_handler_unregister(u32 cmd_type);
struct tsdrv_ipc_chan *tsdrv_get_ipc_chan_table(u32 devid);
#else
static inline void tsdrv_ipc_register(u32 devid)
{
}

static inline void tsdrv_ipc_unregister(u32 devid)
{
}

static inline int tsdrv_ipc_handler_register(u32 cmd_type,
    void (* ipc_handlers)(struct notifier_block *nb, unsigned long len, void *data))
{
    return 0;
}

static inline void tsdrv_ipc_handler_unregister(u32 cmd_type)
{
}

static inline int tsdrv_ipc_msg_send(struct tsdrv_ipc_chan *ipc_chan, rproc_msg_t *resp_msg)
{
    return 0;
}
#endif

static inline struct tsdrv_ipc_chan *notifier_block_to_ipc_chan(struct notifier_block *nb)
{
    return (struct tsdrv_ipc_chan *)container_of(nb, struct tsdrv_ipc_chan, nb);
}

#ifdef __cplusplus
};
#endif

#endif
