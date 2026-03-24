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

#ifndef __HISI_RPROC_H__
#define __HISI_RPROC_H__

#include <linux/errno.h>

#include "hisi_mailbox.h"
#include "drv_ipc.h"
#include "hisi_rproc_adapt.h"

int rproc_xfer_async(int ipc_id, rproc_id_t rproc_id, rproc_msg_t *msg, rproc_msg_len_t len);
int fast_rproc_xfer_async(int ipc_id, rproc_id_t rproc_id, rproc_msg_t *msg, rproc_msg_len_t len);

/* register & unregister function should be called in pair.
 * DO NOT register multiple times for same rproc_id and nb
 */
int rproc_rx_register(int dev_id, rproc_id_t rproc_id, struct notifier_block *nb);
int rproc_rx_unregister(int dev_id, rproc_id_t rproc_id, struct notifier_block *nb);
int hs_rproc_put(int ipc_id, rproc_id_t rproc_id);
int rproc_flush_tx(int ipc_id, rproc_id_t rproc_id);
struct hisi_rproc_info *find_rproc(int ipc_id, rproc_id_t rproc_id);
extern struct hisi_mbox_task *g_TxTaskBuffer[MAX_IPCDEV_NUM];
extern void sync_ipc_clear_lpi(struct hisi_rproc_info *rproc);
#if (defined(CFG_SOC_PLATFORM_MDC_V51) || defined(CFG_SOC_PLATFORM_MINIV2)) && !defined(CFG_SOC_PLATFORM_MINIV3)
extern struct hisi_rproc_info* g_rproc_table[MAX_IPCDEV_NUM];
#else
extern struct hisi_rproc_info g_rproc_table[MAX_IPCDEV_NUM][HISI_RPROC_MAX];
#endif

int rproc_init(int ipc_id, int mdev_num);
void hisi_rproc_exit(int ipc_id);
#endif /* __HISI_RPROC_H__ */
