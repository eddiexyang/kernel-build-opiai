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
#include <linux/types.h>
#include <linux/wait.h>
#include "log_drv_dev.h"
#include "log_xpc.h"
#include "log_xpc_agent.h"
#include "log_xpc_mng.h"

#ifndef LOG_UT
#define AGENT_POLL_CHL_NUM 2
#define AGENT_POLL_TIME_OUT 3000  // 3000 ms
#define AGENT_READ_BUF_LEN 256
s32 log_agent_work_thread(void *arg)
{
    int ret, res_num, i;
    unsigned int read_len, pack_type;
    unsigned char out_buf[AGENT_READ_BUF_LEN];
    struct chl_poll_ret *poll_chl = NULL;
    struct chl_poll_ret poll_ret[AGENT_POLL_CHL_NUM] = {};
    int poll_chl_id[AGENT_POLL_CHL_NUM] = {};

    poll_chl_id[0] = log_xpc_get_chl_id(XPC_CHANNEL_TYPE_CMD);
    poll_chl_id[1] = log_xpc_get_chl_id(XPC_CHANNEL_TYPE_INIT);
    slog_drv_info("xpc thread work, chl=%d, %d\n", poll_chl_id[0], poll_chl_id[1]);
    while (!kthread_should_stop()) {
        res_num = 0;
        ret = xpcshm_poll(poll_chl_id, AGENT_POLL_CHL_NUM, poll_ret, &res_num, AGENT_POLL_TIME_OUT);
        if (ret < 0) {
            if (ret != XPC_ERR_POLL_TIMEOUT && ret != XPC_ERR_POLL_CANCELED_BY_OTHERS) {
                slog_drv_err("xpc shm poll err, ret=%d\n", ret);
            }
            continue;
        }
        for (i = 0; i < res_num; i++) {
            poll_chl = &poll_ret[i];
            read_len = AGENT_READ_BUF_LEN;
            ret = xpcshm_read(poll_chl->id, out_buf, &read_len, &pack_type);
            if (ret < 0) {
                slog_drv_err("xpc shm read err, ret=%d\n", ret);
                continue;
            }
            if (poll_chl->id == log_xpc_get_chl_id(XPC_CHANNEL_TYPE_CMD)) {
                log_agent_parse_cmd(out_buf, read_len);
            } else if (poll_chl->id == log_xpc_get_chl_id(XPC_CHANNEL_TYPE_INIT)) {
                log_agent_parse_init_cmd(out_buf, read_len);
            }
        }
    }
}
#else
s32 log_agent_work_thread(void *arg)
{
    return 0;
}
#endif
