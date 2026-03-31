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

#ifndef _ICM_CORE_H_
#define _ICM_CORE_H_

#include "drv_log.h"
#include "drv_ipc.h"
#include "icm_interface.h"

#define module_icm "icm"

#ifdef STATIC_SKIP
#define STATIC
#else
#define STATIC static
#endif

#define icm_err(fmt, ...) do { \
    drv_err(module_icm, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__); \
} while (0)
#define icm_warn(fmt, ...) do { \
    drv_warn(module_icm, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__); \
} while (0)
#define icm_info(fmt, ...) do { \
    drv_info(module_icm, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__); \
} while (0)

#define ICMDRV_ALLOC 1
#define ICMDRV_FREE 0

#define MAX_CHIP_NUM 4

#ifdef CFG_SOC_PLATFORM_ESL_FPGA
#define ICM_WAIT_TIMEOUT 30000
#else
#define ICM_WAIT_TIMEOUT 3000
#endif

extern struct icmdrv_ctrl *icm_ctrl;
extern struct icmdrv_chan_adapter icm_adapter;


typedef enum {
    ICM_CHAN_TYPE_IPC = 0,
    ICM_CHAN_TYPE_MAX
}icm_chan_type;

struct icmdrv_chan_adapter {
    u32 chan_type;
    u32 chan_num;
    void (*init) (void);
    void (*uninit) (void);
    ICM_HANDLE (*alloc) (u32 dev_id, u32 peer);
    void (*free) (ICM_HANDLE fd);
    int (*send_async) (ICM_HANDLE fd, u32 *msg, u32 send_len);
    int (*fast_send_async) (ICM_HANDLE fd, u32 *msg, u32 send_len);
    int (*send_sync) (ICM_HANDLE fd, u32 *msg, u32 send_len, u32 *ack_buf, u32 ack_len);
    int (*rx_register) (u32 dev_id, u32 chan_id, u32 peer, struct notifier_block *nb);
    int (*rx_unregister) (u32 dev_id, u32 chan_id, u32 peer, struct notifier_block *nb);
};

struct icmdrv_ctrl {
    struct icmdrv_chan_adapter *adapter;
    struct icmdrv_msg_chan *msg_chan[MAX_CHIP_NUM];
};

#endif
