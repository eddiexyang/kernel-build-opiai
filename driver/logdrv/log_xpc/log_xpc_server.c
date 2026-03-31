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
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include "devdrv_functional_cqsq_api.h"
#include "tsdrv_hwinfo.h"
#include "log_xpc_server.h"

#ifndef LOG_UT
void log_cq0_report(u32 device_id, u32 tsid, const u8 *cq_buf, u8 *sq_buf);
void log_cq1_callback(u32 device_id, u32 tsid, const u8 *cq_buf, u8 *sq_buf);
int log_xpc_send_cmd(u32 device_id, u32 tsid, u32 sqcq_idx, struct log_sq_scheduler *sq_info)
{
    int ret, chl_id;
    struct log_xpc_sq_cmd cmd;
    cmd.device_id = device_id;
    cmd.tsid = tsid;
    cmd.sqcq_idx = sqcq_idx;
    ret = memcpy_s(&cmd.sq_info, sizeof(struct log_sq_scheduler), sq_info, sizeof(struct log_sq_scheduler));
    if (ret != 0) {
        slog_drv_err("memcpy err, ret = %d\n", ret);
        return LOG_RET_ERROR;
    }
    chl_id = log_xpc_get_chl_id(XPC_CHANNEL_TYPE_CMD);
    ret = xpcshm_write(chl_id, (char *)&cmd, sizeof(cmd));
    if (ret != 0) {
        slog_drv_err("xpc send cmd err, devid=%u, tsid=%u, idx=%u, ret=%d\n", device_id, tsid, sqcq_idx, ret);
        return LOG_RET_ERROR;
    }
    return 0;
}

int log_xpc_send_init_cmd(u32 device_id, u32 tsid)
{
    int ret, chl_id;
    struct log_xpc_cqcq_init_arg cmd;
    cmd.device_id = device_id;
    cmd.tsid = tsid;
    cmd.type = LOG_CQSQ_INIT;
    chl_id = log_xpc_get_chl_id(XPC_CHANNEL_TYPE_INIT);
    ret = xpcshm_write(chl_id, (char *)&cmd, sizeof(cmd));
    if (ret != 0) {
        slog_drv_err("xpc send init cmd err, devid=%u, tsid=%u, ret=%d\n", device_id, tsid, ret);
        return LOG_RET_ERROR;
    }
    return 0;
}

void log_xpc_send_uninit_cmd(u32 device_id, u32 tsid, struct log_cqsq_info *cqsq_info)
{
    int ret, chl_id;
    struct log_xpc_cqcq_init_arg cmd;
    cmd.device_id = device_id;
    cmd.tsid = tsid;
    cmd.type = LOG_CQSQ_UNINIT;
    cmd.sq_0_index = cqsq_info->sq_0_index;
    cmd.cq_0_index = cqsq_info->cq_0_index;
    cmd.cq_1_index = cqsq_info->cq_1_index;
    chl_id = log_xpc_get_chl_id(XPC_CHANNEL_TYPE_INIT);
    ret = xpcshm_write(chl_id, (char *)&cmd, sizeof(cmd));
    if (ret != 0) {
        slog_drv_err("xpc send uninit cmd err, devid=%u, tsid=%u, ret=%d\n", device_id, tsid, ret);
    }
}


int log_xpc_init_parse_result(unsigned char *buf, int len, struct log_cqsq_info *cqsq_info)
{
    struct log_xpc_cqcq_init_reult *result = NULL;
    if (len != sizeof(struct log_xpc_cqcq_init_reult)) {
        slog_drv_err("xpc init parse err, target len=%lu, now=%d\n", sizeof(struct log_xpc_cqcq_init_reult), len);
        return LOG_RET_ERROR;
    }
    result = (struct log_xpc_cqcq_init_reult *)buf;
    if (result->ret != 0) {
        slog_drv_err("xpc init agent exec err, ret=%d\n", result->ret);
        return result->ret;
    }
    cqsq_info->sq_0_index = result->sq_0_index;
    cqsq_info->cq_0_index = result->cq_0_index;
    cqsq_info->cq_1_index = result->cq_1_index;
    LOG_CQSQ_INFO_SHOW(tsid, cqsq_info);
    return LOG_RET_OK;
}

void log_xpc_parse_cq_data(unsigned char *buf, int len, int type)
{
    struct log_xpc_cq_st *cq;
    if (len != sizeof(struct log_xpc_cq_st)) {
        slog_drv_err("xpc cq data parse err, target len=%lu, now=%d\n", sizeof(struct log_xpc_cq_st), len);
        return;
    }
    cq = (struct log_xpc_cq_st *)buf;
    if (type == XPC_CHANNEL_TYPE_CQ0) {
        log_cq0_report(cq->head.device_id, cq->head.tsid, (const u8 *)&cq->data, NULL);
    } else if (type == XPC_CHANNEL_TYPE_CQ1) {
        log_cq1_callback(cq->head.device_id, cq->head.tsid, (const u8 *)&cq->data, NULL);
    } else {
        slog_drv_err("xpc cq data parse err, type=%d undefiend\n", type);
    }
}

#define CQSQ_INIT_READ_BUF_LEN 256
#define CQSQ_INIT_POLL_TIMEOUT 3000 // 3000ms
#define CQSQ_INIT_POLL_NUM 1
int log_xpc_cqsq_init(struct log_device_ctx *device_ctx, u32 tsid)
{
    int ret, chl_id;
    int res_num;
    int read_len = CQSQ_INIT_READ_BUF_LEN;
    unsigned int pack_type, device_id;
    unsigned char out_buf[CQSQ_INIT_READ_BUF_LEN];
    struct chl_poll_ret poll_ret[CQSQ_INIT_POLL_NUM] = {};
    struct log_device_info *device_info = NULL;
    struct log_cqsq_info *cqsq_info = NULL;
    if (device_ctx == NULL) {
        slog_drv_info("Device_ctx is NULL.\n");
        return LOG_RET_ERROR;
    }

    device_info = &device_ctx->device_info;
    device_id = device_info->device_id;
    cqsq_info = &device_info->cqsq_info[tsid];
    if (device_info->device_state[tsid] == DEV_USED) {
        slog_drv_info("Device has initialized sqcq.\n");
        return LOG_RET_OK;
    }
    device_info->device_state[tsid] = DEV_USED;
    ret = log_xpc_send_init_cmd(device_id, tsid);
    if (ret != 0) {
        return ret;
    }
    chl_id = log_xpc_get_chl_id(XPC_CHANNEL_TYPE_INIT_REPLY);
    ret = xpcshm_poll(&chl_id, CQSQ_INIT_POLL_NUM, poll_ret, &res_num, CQSQ_INIT_POLL_TIMEOUT);
    if (ret != 0) {
        device_info->device_state[tsid] = DEV_UNUSED;
        slog_drv_err("init cqsq poll failed, chl_id=%d, timeout=%d, ret=%d\n", chl_id, CQSQ_INIT_POLL_TIMEOUT, ret);
        return LOG_RET_ERROR;
    }
    ret = xpcshm_read(chl_id, out_buf, &read_len, &pack_type); // 根据接口说明，xpcshm_read 有四类返回值
    if (ret != 0) {
        device_info->device_state[tsid] = DEV_UNUSED;
        slog_drv_err("init cqsq read failed, chl_id=%d, ret=%d\n", chl_id, ret);
        return LOG_RET_ERROR;
    }
    ret = log_xpc_init_parse_result(out_buf, read_len, cqsq_info);
    if (ret != 0) {
        device_info->device_state[tsid] = DEV_UNUSED;
        slog_drv_err("init cqsq read failed, chl_id=%d, ret=%d\n", chl_id, ret);
    }
    LOG_CQSQ_INFO_SHOW(tsid, cqsq_info);
    slog_drv_info("log cqsq xpc init success.\n");
    return ret;
}

void log_xpc_cqsq_uninit(struct log_device_ctx *device_ctx, u32 tsid)
{
    struct log_device_info *device_info = NULL;
    struct log_cqsq_info *cqsq_info = NULL;
    s32 device_id;

    if (device_ctx == NULL) {
        slog_drv_info("Device_ctx is NULL.\n");
        return;
    }

    device_info = &device_ctx->device_info;
    device_id = device_info->device_id;
    cqsq_info = &device_info->cqsq_info[tsid];
    if (device_id >= LOG_DEVICE_ID_MAX) {
        slog_drv_err("Device_id is invalid. (device_id=%d)\n", device_id);
        return;
    }

    if (device_info->device_state[tsid] == DEV_UNUSED) {
        slog_drv_err("Device state is DEV_UNUSED. (device_id=%d; DEV_UNUSED=%d)\n", device_id, DEV_UNUSED);
        return;
    }

    device_info->device_state[tsid] = DEV_UNUSED;
    log_xpc_send_uninit_cmd(device_id, tsid, cqsq_info);
    slog_drv_info("log cqsq xpc uninit success.\n");
    return;
}
#else
int log_xpc_cqsq_init(struct log_device_ctx *device_ctx, u32 tsid)
{
    return 0;
}
#endif
