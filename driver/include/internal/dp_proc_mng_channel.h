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
#ifndef __DP_PROC_MNG_CHANNEL_H__
#define __DP_PROC_MNG_CHANNEL_H__
#include "dp_proc_mng_cmd.h"
#include "dp_proc_mng_proc_info.h"

#define DP_PROC_MNG_CHANNEL_MASK 0x00000FFF

struct dp_proc_mng_chan_msg_head {
    struct dp_proc_mng_process_id process_id;
    u32 msg_id;
    short result;
    u32 res;
};

enum dp_proc_mng_h2d_cmd {
    DP_PROC_MNG_CHAN_QUERY_PROCESS_STATUS_H2D_ID = 0,
    DP_PROC_MNG_CHAN_H2D_MAX_ID
};

struct dp_proc_mng_chan_process_status {
    struct dp_proc_mng_chan_msg_head head;
    processStatus_t pid_status;
};

enum dp_proc_mng_d2h_cmd {
    DP_PROC_MNG_CHAN_PROCESS_STATUS_REPORT_D2H_ID = 0,
    DP_PROC_MNG_CHAN_CHECK_PROCESS_SIGN_D2H,
    DP_PROC_MNG_CHAN_D2H_MAX_ID
};


struct dp_proc_mng_chan_check_process_sign {
    struct dp_proc_mng_chan_msg_head head;
    char sign[PROCESS_SIGN_LENGTH];
    char resv[PROCESS_RESV_LENGTH];
};

struct dp_proc_mng_chan_handlers_st {
    int (*const chan_msg_processes)(void *msg, u32 *ack_len);
    u32 msg_size;
    u32 msg_bitmap;
};

int dp_proc_mng_chan_msg_dispatch(void *msg, u32 in_data_len, u32 out_data_len, u32 *ack_len,
    const struct dp_proc_mng_chan_handlers_st *msg_process);

extern struct dp_proc_mng_chan_handlers_st dp_proc_mng_agent_msg_processes[DP_PROC_MNG_CHAN_H2D_MAX_ID];
extern struct dp_proc_mng_chan_handlers_st dp_proc_mng_master_msg_processes[DP_PROC_MNG_CHAN_D2H_MAX_ID];

#endif /* __DP_PROC_MNG_CHANNEL_H__ */
