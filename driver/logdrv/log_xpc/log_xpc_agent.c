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
#include "log_xpc_agent.h"
#include "devdrv_functional_cqsq_api.h"
#ifndef AOS_LLVM_BUILD
#include "tsdrv_hwinfo.h"
#endif

#ifndef LOG_UT
void log_agent_send_cq(unsigned int type, struct log_xpc_cq_head *head, const u8 *cq_buf)
{
    int ret, chl_id;
    struct log_xpc_cq_st cq;

    if (type >= XPC_CHANNEL_TYPE_MAX) {
        return;
    }

    cq.head.device_id = head->device_id;
    cq.head.tsid = head->tsid;
    cq.head.ret = head->ret;

    ret = memcpy_s(&cq.data, sizeof(struct log_cq_scheduler), cq_buf, sizeof(struct log_cq_scheduler));
    if (ret != 0) {
        slog_drv_err("memcpy err, ret = %d\n", ret);
        return;
    }
    chl_id = log_xpc_get_chl_id(type);
    ret = xpcshm_write(chl_id, (u8 *)&cq, sizeof(cq));
    if (ret != 0) {
        slog_drv_err("xpc send cq0 err, devid=%u, tsid=%u, ret=%d\n", head->device_id, head->tsid, ret);
    }
}

void log_agent_send_cq0_report(u32 device_id, u32 tsid, const u8 *cq_buf, u8 *sq_buf)
{
    struct log_xpc_cq_head head = {LOG_RET_OK, device_id, tsid};
    log_agent_send_cq(XPC_CHANNEL_TYPE_CQ0, &head, cq_buf);
}

void log_agent_send_cq1_callback(u32 device_id, u32 tsid, const u8 *cq_buf, u8 *sq_buf)
{
    struct log_xpc_cq_head head = {LOG_RET_OK, device_id, tsid};
    log_agent_send_cq(XPC_CHANNEL_TYPE_CQ1, &head, cq_buf);
}

void log_agent_send_cmd_to_ts(struct log_xpc_sq_cmd *cmd)
{
    int ret;
    ret = devdrv_functional_send_sq(cmd->device_id, cmd->tsid, cmd->sqcq_idx,
        (u8 *)(&cmd->sq_info), sizeof(struct log_sq_scheduler));
    if (ret != 0) {
        slog_drv_err("send sq err, dev_id=%u, tsid=%u, idx=%u\n", cmd->device_id, cmd->tsid, cmd->sqcq_idx);
    }
}

void log_agent_parse_cmd(unsigned char *data, unsigned int len)
{
    struct log_xpc_sq_cmd *cmd = (struct log_xpc_sq_cmd *)data;
    if (len != sizeof(struct log_xpc_sq_cmd)) {
        slog_drv_err("size is invalid, len=%u, size=%lu\n", len, sizeof(struct log_xpc_sq_cmd));
    }
    log_agent_send_cmd_to_ts(cmd);
}

struct log_sqcq_id_addr_pair {
    u32 idx;
    u64 addr;
};

enum log_drv_cqsq_num {
    LOG_SQ_0 = 0,
    LOG_CQ_0,
    LOG_CQ_1,
    LOG_CQSQ_NUM
};

void log_agent_cqsq_init_mailbox(u32 device_id, u32 tsid,
    struct devdrv_mailbox_cqsq *mb, struct log_sqcq_id_addr_pair *pair)
{
    mb->cq_irq = (u16)tsdrv_get_dfx_cq_irq_vector(device_id, tsid);
    mb->cmd_type = LOG_CQSQ_CREATE;
    mb->sq_index = (u16)pair[LOG_SQ_0].idx;
    mb->sq_addr = pair[LOG_SQ_0].addr;
    mb->cq0_index = (u16)pair[LOG_CQ_0].idx;
    mb->cq0_addr = pair[LOG_CQ_0].addr;
    mb->cq1_index = (u16)pair[LOG_CQ_1].idx;
    mb->cq1_addr = pair[LOG_CQ_1].addr;
}

void log_agent_cqsq_init_result(struct log_sqcq_id_addr_pair *pair, struct log_xpc_cqcq_init_reult *result)
{
    result->sq_0_index = pair[LOG_SQ_0].idx;
    result->cq_0_index = pair[LOG_CQ_0].idx;
    result->cq_1_index = pair[LOG_CQ_1].idx;
}

int log_agent_cqsq_init_to_ts(u32 device_id, u32 tsid, struct log_xpc_cqcq_init_reult *result)
{
    s32 ret;
    struct log_sqcq_id_addr_pair pair[LOG_CQSQ_NUM];
    struct devdrv_mailbox_cqsq mailbox_cqsq = {};

    ret = devdrv_create_functional_sq(device_id, tsid,
        LOG_SQ_BUF_LEN, &pair[LOG_SQ_0].idx, (u64 *)&pair[LOG_SQ_0].addr);
    if (ret != 0) {
        slog_drv_err("logdrv create sq failed, tsid=%u\n", tsid);
        return ret;
    }
    /* the first cq length is 128 Byte; and the other cqs are 32 Byte; */
    ret = devdrv_create_functional_cq(device_id, tsid, LOG_SQ_BUF_LEN, report_cq,
        log_agent_send_cq0_report, &pair[LOG_CQ_0].idx, &pair[LOG_CQ_0].addr);
    if (ret != 0) {
        slog_drv_err("logdrv create cq0 failed, tsid=%u\n", tsid);
        devdrv_destroy_functional_sq((u32)device_id, tsid, pair[LOG_SQ_0].idx);
        return ret;
    }

    ret = devdrv_create_functional_cq((u32)device_id, tsid, LOG_CQ_BUF_LEN, callback_cq,
        log_agent_send_cq1_callback, &pair[LOG_CQ_1].idx, (u64 *)&pair[LOG_CQ_1].addr);
    if (ret != 0) {
        slog_drv_err("logdrv create cq1 failed, tsid=%u\n", tsid);
        devdrv_destroy_functional_sq((u32)device_id, tsid, pair[LOG_SQ_0].idx);
        devdrv_destroy_functional_cq((u32)device_id, tsid, pair[LOG_CQ_0].idx);
        return ret;
    }

    log_agent_cqsq_init_mailbox(device_id, tsid, &mailbox_cqsq, pair);

    ret = devdrv_mailbox_send_cqsq((u32)device_id, tsid, &mailbox_cqsq);
    if (ret != 0) {
        slog_drv_err("logdrv mailbox send cqsq failed, tsid=%u\n", tsid);
        devdrv_destroy_functional_sq((u32)device_id, tsid, pair[LOG_SQ_0].idx);
        devdrv_destroy_functional_cq((u32)device_id, tsid, pair[LOG_CQ_0].idx);
        devdrv_destroy_functional_cq((u32)device_id, tsid, pair[LOG_CQ_1].idx);
        return ret;
    }

    log_agent_cqsq_init_result(pair, result);
    LOG_CQSQ_INFO_SHOW(tsid, cqsq_info);
    slog_drv_info("logdrv cqsq create succeeded. (device_id=%d, tsid=%u)\n", device_id, tsid);
    return ret;
}

void log_agent_cqsq_uninit_to_ts(u32 device_id, u32 tsid, u32 sq_0_index, u32 cq_0_index, u32 cq_1_index)
{
    struct devdrv_mailbox_cqsq mailbox_cqsq = {};
    s32 ret;

    mailbox_cqsq.cmd_type = LOG_CQSQ_RELEASE;
    mailbox_cqsq.sq_index = sq_0_index;
    mailbox_cqsq.cq0_index = (u16)cq_0_index;
    mailbox_cqsq.cq1_index = (u16)cq_1_index;

    ret = devdrv_mailbox_send_cqsq(device_id, tsid, &mailbox_cqsq);
    if (ret != (s32)LOG_RET_OK) {
        slog_drv_err("logdrv uninit send cqsq failed. (device_id=%u, ret=%d)\n", device_id, ret);
    }

    devdrv_destroy_functional_sq(device_id, tsid, sq_0_index);
    devdrv_destroy_functional_cq(device_id, tsid, cq_0_index);
    devdrv_destroy_functional_cq(device_id, tsid, cq_1_index);
    slog_drv_info("logdrv uninit success. (device_id=%u, tsid=%u)\n", device_id, tsid);

    return;
}

void log_agent_cqsq_uninit(struct log_xpc_cqcq_init_arg *cmd)
{
    log_agent_cqsq_uninit_to_ts(cmd->device_id, cmd->tsid, cmd->sq_0_index, cmd->cq_0_index, cmd->cq_1_index);
}

void log_agent_cqsq_init(struct log_xpc_cqcq_init_arg *cmd)
{
    int ret, chl_id;
    struct log_xpc_cqcq_init_reult result = {};
    ret = log_agent_cqsq_init_to_ts(cmd->device_id, cmd->tsid, &result);
    if (ret != 0) {
        slog_drv_err("xpc cqsq init failed, dev_id=%u, tsid=%u\n", cmd->device_id, cmd->tsid);
    }
    result.ret = ret;
    result.device_id = cmd->device_id;
    result.tsid = cmd->tsid;
    chl_id = log_xpc_get_chl_id(XPC_CHANNEL_TYPE_INIT_REPLY);
    ret = xpcshm_write(chl_id, (char *)&result, sizeof(result));
    if (ret != 0) {
        slog_drv_err("xpc cqsq init send xpc cmd failed, dev_id=%u, tsid=%u\n", cmd->device_id, cmd->tsid);
    }
    return;
}

void log_agent_parse_init_cmd(unsigned char *data, unsigned int len)
{
    struct log_xpc_cqcq_init_arg *cmd = (struct log_xpc_cqcq_init_arg *)data;
    if (len != sizeof(struct log_xpc_cqcq_init_arg)) {
        slog_drv_err("size is invalid, len=%u, size=%lu\n", len, sizeof(struct log_xpc_cqcq_init_arg));
        return;
    }
    if (cmd->type == LOG_CQSQ_INIT) {
        log_agent_cqsq_init(cmd);
    } else if (cmd->type == LOG_CQSQ_UNINIT) {
        log_agent_cqsq_uninit(cmd);
    } else {
        slog_drv_err("type is invalid, type=%d\n", cmd->type);
    }
}
#else
void log_agent_parse_cmd(unsigned char *data, unsigned int len)
{
    return;
}

#endif
