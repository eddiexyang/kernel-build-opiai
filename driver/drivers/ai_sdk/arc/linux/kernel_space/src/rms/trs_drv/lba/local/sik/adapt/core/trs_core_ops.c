/*
* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
* Create: 2023-8-15
*/
#include <linux/sched.h>
#include <linux/mman.h>

#include "trs_pm_adapt.h"
#include "trs_device_core.h"
#include "trs_mailbox_def.h"
#include "trs_core.h"
#include "trs_chip_def.h"
#include "trs_device_mbox.h"
#include "trs_msg.h"
#include "trs_chan_mem.h"
#include "trs_id.h"
#include "soc_res.h"
#include "trs_core_ops.h"
#include "trs_chan_mbox.h"
#include "kernel_version_adapt.h"

static int trs_core_ops_recycle_check_msg_fill(struct trs_id_inst *inst, u8 *msg, u32 len)
{
    struct recycle_proc_msg *tmp_msg = (struct recycle_proc_msg *)msg;

    tmp_msg->proc_info.plat_type = TRS_MBOX_SEND_FROM_DEVICE;
    tmp_msg->proc_info.fid = 0;

    return 0;
}

static int (* const trs_core_ops_fill_mbox_msg[TRS_MBOX_CMD_MAX])(struct trs_id_inst *inst, u8 *msg, u32 len) = {
    [TRS_MBOX_RECYCLE_PID] = trs_core_ops_recycle_check_msg_fill,
    [TRS_MBOX_RECYCLE_CHECK] = trs_core_ops_recycle_check_msg_fill,
};

int trs_core_ops_notice_ts(struct trs_id_inst *inst, u8 *msg, u32 len)
{
    struct trs_mb_header *header = (struct trs_mb_header *)msg;
    int ret;

    if (trs_core_ops_fill_mbox_msg[header->cmd_type] == NULL) {
        return 0;
    }

    ret = trs_core_ops_fill_mbox_msg[header->cmd_type](inst, msg, len);
    if (ret != 0) {
        return ret;
    }

    return trs_mbox_send(inst, 0, (void *)msg, (size_t)len, 0);
}

static int trs_core_ops_get_event_offset(struct trs_id_inst *inst, u32 id, u32 *offset)
{
    *offset = trs_get_event_offset(id);
    return 0;
}

static int trs_core_ops_get_notify_offset(struct trs_id_inst *inst, u32 id, u32 *offset)
{
    *offset = trs_get_notify_offset(id);
    return 0;
}

static int trs_core_ops_get_notify_total_size(struct trs_id_inst *inst, u32 *total_size)
{
    size_t notify_size = trs_get_notify_size();
    u32 start, end;
    int ret;

    ret = trs_id_get_range(inst, TRS_NOTIFY_ID, &start, &end);
    if (ret != 0) {
        return ret;
    }

    *total_size = (end - start) * notify_size;

    return 0;
}

int trs_core_ops_get_res_reg_offset(struct trs_id_inst *inst, int type, u32 id, u32 *offset)
{
    if (type == TRS_NOTIFY_ID) {
        return trs_core_ops_get_notify_offset(inst, id, offset);
    }
    if (type == TRS_EVENT_ID) {
        return trs_core_ops_get_event_offset(inst, id, offset);
    }

    return -ENODEV;
}
EXPORT_SYMBOL(trs_core_ops_get_res_reg_offset);

int trs_core_ops_get_res_reg_total_size(struct trs_id_inst *inst, int type, u32 *total_size)
{
    if (type == TRS_NOTIFY_ID) {
        return trs_core_ops_get_notify_total_size(inst, total_size);
    }

    return -ENODEV;
}
EXPORT_SYMBOL(trs_core_ops_get_res_reg_total_size);

int trs_core_ops_get_support_proc_num(struct trs_id_inst *inst, u32 *proc_num)
{
    *proc_num = TRS_SUPPORT_PROC_NUM;
    return 0;
}
EXPORT_SYMBOL(trs_core_ops_get_support_proc_num);

int trs_core_ops_get_ts_inst_status(struct trs_id_inst *inst, u32 *status)
{
    struct res_inst_info res_inst;
    int ret;

    soc_resmng_inst_pack(&res_inst, inst->devid, TS_SUBSYS, inst->tsid);
    ret = soc_resmng_get_ts_status(&res_inst, status);
    if (ret != 0) {
        trs_err("Get ts status failed. (devid=%u; ret=%d)\n", inst->devid, ret);
    }

    return ret;
}
EXPORT_SYMBOL(trs_core_ops_get_ts_inst_status);

