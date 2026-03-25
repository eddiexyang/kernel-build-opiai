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
* Create: 2023-1-11
*/
#include "trs_core.h"
#include "soc_adapt.h"
#include "trs_host_msg.h"
#include "trs_id.h"
#include "trs_ts_status.h"

int trs_core_ops_get_proc_num(struct trs_id_inst *inst, u32 *proc_num)
{
    struct trs_msg_proc_num *proc_num_msg = NULL;
    struct trs_msg_data msg;
    int ret;

    msg.header.tsid = inst->tsid;
    msg.header.valid = TRS_MSG_SEND_MAGIC;
    msg.header.cmdtype = TRS_MSG_GET_PROC_NUM;
    msg.header.result = 0;

    ret = trs_host_msg_send(inst->devid, &msg, sizeof(struct trs_msg_data));
    if ((ret != 0) || (msg.header.result != 0)) {
        trs_err("Msg send fail. (devid=%u; tsid=%u; ret=%d; result=%d)\n",
            inst->devid, inst->tsid, ret, msg.header.result);
        return -ENODEV;
    }
    proc_num_msg = (struct trs_msg_proc_num *)msg.payload;
    *proc_num = proc_num_msg->proc_num;
    return 0;
}

int trs_core_ops_get_res_reg_offset(struct trs_id_inst *inst, int type, u32 id, u32 *offset)
{
    if (type == TRS_NOTIFY_ID) {
        return trs_soc_get_notify_offset(inst, id, offset);
    }
    return -ENODEV;
}

static int trs_core_ops_get_notify_reg_total_size(struct trs_id_inst *inst, u32 *total_size)
{
    size_t notify_size;
    u32 start, end;
    int ret;

    ret = trs_id_get_range(inst, TRS_NOTIFY_ID, &start, &end);
    if (ret != 0) {
        return ret;
    }

    ret = trs_soc_get_notify_size(inst, &notify_size);
    if (ret != 0) {
        return ret;
    }

    *total_size = (end - start) * notify_size;
    return 0;
}

int trs_core_ops_get_res_reg_total_size(struct trs_id_inst *inst, int type, u32 *total_size)
{
    if (type == TRS_NOTIFY_ID) {
        return trs_core_ops_get_notify_reg_total_size(inst, total_size);
    }

    return -ENODEV;
}

int trs_core_ops_get_ts_inst_status(struct trs_id_inst *inst, u32 *status)
{
    return trs_get_ts_status(inst, status);
}
