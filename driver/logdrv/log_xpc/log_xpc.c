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
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include "log_xpc.h"
#include "devdrv_functional_cqsq_api.h"

#ifndef LOG_UT
struct chl_info g_log_xpc_chl[XPC_CHANNEL_TYPE_MAX] = {
    [XPC_CHANNEL_TYPE_CMD] = {"log_drv_cmd", 0, 0, 0},
    [XPC_CHANNEL_TYPE_INIT] = {"log_drv_init", 0, 0, 0},
    [XPC_CHANNEL_TYPE_INIT_REPLY] = {"log_drv_init_reply", 0, 0, 0},
    [XPC_CHANNEL_TYPE_CQ0] = {"log_drv_cq0", 0, 0, 0},
    [XPC_CHANNEL_TYPE_CQ1] = {"log_drv_cq1", 0, 0, 0},
};

int g_log_xpc_chl_id[XPC_CHANNEL_TYPE_MAX] = {};

struct chl_info *log_xpc_get_chl_info(int type)
{
    return &g_log_xpc_chl[type];
}

int log_xpc_get_chl_id(int type)
{
    return g_log_xpc_chl_id[type];
}

void log_xpc_set_chl_id(int type, int chl_id)
{
    g_log_xpc_chl_id[type] = chl_id;
}

void log_xpc_init_all_channel(void)
{
    int i;
    for (i = 0; i < XPC_CHANNEL_TYPE_MAX; i++) {
        log_xpc_set_chl_id(i, INVALID_XPC_CHL_ID);
    }
}
void log_xpc_close_channel(int type)
{
    xpcshm_close_channel(log_xpc_get_chl_id(type));
    log_xpc_set_chl_id(type, INVALID_XPC_CHL_ID);
}

#define LOG_XPC_WAIT_OPEN_INTERVAL 10 // 10ms
#define LOG_XPC_WAIT_MAX_CNT 1000
int log_xpc_open_all_channel(void)
{
    int i, j, cnt, chl_id;
    struct chl_info *info = NULL;
    bool connet_flag = false;

    log_xpc_init_all_channel();
    for (i = 0; i < XPC_CHANNEL_TYPE_MAX; i++) {
        info = log_xpc_get_chl_info(i);
        chl_id = xpcshm_open_channel(info, CHL_O_CREATE);
        if (chl_id < 0) {
            slog_drv_err("xpc shm open chl err, type=%d, ret=%d\n", i, chl_id);
            for (j = 0; j < i; j++) {
                log_xpc_close_channel(j);
            }
            return LOG_RET_ERROR;
        } else {
            slog_drv_info("xpc shm open chl success, type=%d, chl_id=%d\n", i, chl_id);
            log_xpc_set_chl_id(i, chl_id);
        }
    }
    slog_drv_info("log xpc create all channel success\n");
    cnt = 0;
    while (connet_flag == false && cnt < LOG_XPC_WAIT_MAX_CNT) {
        connet_flag = true;
        for (i = 0; i < XPC_CHANNEL_TYPE_MAX; i++) {
            if (xpcshm_get_chl_state(g_log_xpc_chl_id[i]) != XPC_CHL_STATE_OPEN) {
                connet_flag = false;
                break;
            }
        }
        msleep(LOG_XPC_WAIT_OPEN_INTERVAL);
        cnt++;
    }
    if (cnt == LOG_XPC_WAIT_MAX_CNT) {
        slog_drv_err("log xpc wait open fail\n");
        return LOG_RET_ERROR;
    }
    slog_drv_info("log xpc open all channel success\n");
    return LOG_RET_OK;
}

void log_xpc_close_all_channel(void)
{
    int i;
    int chl_id;
    for (i = 0; i < XPC_CHANNEL_TYPE_MAX; i++) {
        chl_id = log_xpc_get_chl_id(i);
        if (chl_id != INVALID_XPC_CHL_ID) {
            log_xpc_close_channel(i);
        }
    }
    slog_drv_info("log xpc close all channel success\n");
}
#else
void log_xpc_close_all_channel(void)
{
    return;
}
#endif
