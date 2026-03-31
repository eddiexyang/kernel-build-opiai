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
#ifndef LOG_DRV_SQCQ_H
#define LOG_DRV_SQCQ_H

#include "log_drv_dev.h"

enum cq_info_type {
    callback_cq = 0,
    report_cq
};

/* sq info; size: 48 Byte */
#define LOG_SQ_SCHEDULER_REV 4
struct log_sq_scheduler {
    u32 cmd_verify;
    u32 channel_id;
    u32 channel_cmd;
    u32 buf_len; /* the actual buffer length for data */
    uintptr_t phy_addr;
    u32 log_level;
    s32 reserved[LOG_SQ_SCHEDULER_REV];
};

/* cq info; size: 32 Byte */
#define LOG_CQ_SCHEDULER_REV 2
struct log_cq_scheduler {
    u32 cmd_verify;
    u32 channel_id;
    u32 channel_cmd;
    u32 ret_val;
    u32 device_id;
    u32 tsid;
    s32 reserved[LOG_CQ_SCHEDULER_REV];
};

#define LOG_DEBUG_SQSLOT(s, tsid) \
    slog_drv_debug("DEBUG_SQSLOT. (cmd_verify=%u; channel_id=%u; tsid=%u; " \
        "channel_cmd=%u; buf_len=%u; phy_addr=%pK; log_level=%d)\n", \
        (s)->cmd_verify, (s)->channel_id, (tsid), (s)->channel_cmd, \
        (s)->buf_len, (void *)(uintptr_t)(s)->phy_addr, (s)->log_level)

void log_init_conn_func_sqcq(struct log_channel_desc *log_chan_desc);
#ifdef CFG_SOC_PLATFORM_MDC_V51
bool log_is_ts_valid(void);
#endif
#endif /* LOG_DRV_SQCQ_H */

