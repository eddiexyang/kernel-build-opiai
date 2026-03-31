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

#ifndef _ICM_IPC_H_
#define _ICM_IPC_H_

#include "icm_core.h"
#include "icm_interface.h"

#define MAX_IPCDRV_MSG_LENGTH 32
#define MAX_IPCDRV_MSG_HEAD 4

#define CMD_DMP_MSG_HEAD_LEN 4
/* 24 bytes: ipc mail size 32 bytes, 4 bytes ipc cmd head, 4 bytes dmp ipc msg head */
#define CMD_DMP_MSG_MAX_LEN (MAX_IPCDRV_MSG_LENGTH - MAX_IPCDRV_MSG_HEAD - CMD_DMP_MSG_HEAD_LEN)

#ifdef CFG_SOC_PLATFORM_ESL_FPGA
#define IPC_TIME_OUT 30000
#else
#ifdef CFG_SOC_PLATFORM_MDC_V51
#define IPC_TIME_OUT 3000
#else
#define IPC_TIME_OUT 100
#endif
#endif
#define ICM_TIMEOUT_TIMES 3

#define ICM_RX_CONTINUE 1
#define ICM_RX_FINISH 0

#define ICM_TS_CMD_MASK 0xfe

enum icm_ipc_usage {
    ICM_IPC_RESERVE,
    ICM_IPC_TX,
    ICM_IPC_RX,
    ICM_DIREC_MAX
};

struct icmdrv_ipc_res {
    int status;
    int direction;
    int peer;
    rproc_id_t id;
    int msg_version;
    u64 used_cnt;
};

void icm_ipc_res_init(void);
void icm_ipc_res_uninit(void);
ICM_HANDLE icm_ipc_msg_chan_alloc(u32 dev_id, u32 peer);
void icm_ipc_msg_chan_free(ICM_HANDLE fd);
int icm_ipc_msg_send_async(ICM_HANDLE fd, u32 *msg, u32 send_len);
int icm_ipc_msg_fast_send_async(ICM_HANDLE fd, u32 *msg, u32 send_len);
int icm_ipc_msg_send_sync(ICM_HANDLE fd, u32 *msg, u32 send_len, u32 *ack_buf, u32 ack_len);
int icm_ipc_rx_register(u32 dev_id, u32 chan_id, u32 peer, struct notifier_block *nb);
int icm_ipc_rx_unregister(u32 dev_id, u32 chan_id, u32 peer, struct notifier_block *nb);

#endif
