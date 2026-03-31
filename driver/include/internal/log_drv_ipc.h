/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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
 * Create: 2022-08-13
 */
#ifndef LOG_DRV_IPC_H
#define LOG_DRV_IPC_H

#include "log_drv_dev.h"
#include "devdrv_manager.h"
#include "icm_interface.h"

#define LOG_DEBUG_IPCMSG(i) \
    slog_drv_debug("DEBUG_IPCMSG. (cmd_type1=%u; cmd_type0=%u; " \
        "target_id=%u; source_id=%u; cmd_para0=%u; cmd_para1=%u; " \
        "cmd_para2=%u; cmd_para3=%u)\n", \
        (i)->cmd_type1, (i)->cmd_type0, (i)->target_id, (i)->source_id, \
        (i)->cmd_para0, (i)->cmd_para1, (i)->cmd_para2, (i)->cmd_para3)

union log_ipc_msg {
        struct devdrv_ipc_lpm3 log_devdrv_ipc_msg;
        struct icmdrv_ipc_msg log_icm_ipc_msg;
    };

struct log_ipc_cfg {
    bool is_support;
    union log_ipc_msg msg;
    u32 (*get_ipc_len)(void);
    u32 (*get_ipc_fd)(u8 devid, int mailbox);
    void (*set_log_level)(union log_ipc_msg *ipc_msg, u8 level);
};

void log_init_conn_func_ipc(struct log_channel_desc *log_chan_desc);

#endif // LOG_DRV_IPC_H
