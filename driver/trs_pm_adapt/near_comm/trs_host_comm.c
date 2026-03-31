/*
* Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
* Create: 2022-8-15
*/
#include <linux/sched.h>

#include "trs_pub_def.h"
#include "trs_msg.h"
#include "trs_host_msg.h"
#include "trs_chan.h"
#include "trs_abnormal_info.h"
#include "trs_ts_status.h"
#include "trs_id.h"
#include "trs_host_comm.h"

int trs_host_get_ssid(struct trs_id_inst *inst, int *ssid)
{
    struct trs_msg_sync_ssid *ssid_msg = NULL;
    struct trs_msg_data msg;
    int ret;

    msg.header.tsid = inst->tsid;
    msg.header.valid = TRS_MSG_SEND_MAGIC;
    msg.header.cmdtype = TRS_MSG_GET_SSID;
    msg.header.result = 0;

    ssid_msg = (struct trs_msg_sync_ssid *)msg.payload;
    ssid_msg->vfid = 0;
    ssid_msg->hpid = current->tgid;

    ret = trs_host_msg_send(inst->devid, &msg, sizeof(struct trs_msg_data));
    if ((ret != 0) || (msg.header.result != 0)) {
        trs_err("Msg send fail. (devid=%u; tsid=%u; ret=%d; result=%d)\n",
            inst->devid, inst->tsid, ret, msg.header.result);
        return -ENODEV;
    }
    *ssid = ssid_msg->ssid;
    return 0;
}

int trs_host_ts_adapt_abnormal_proc(u32 devid, struct trs_msg_data *msg)
{
    struct stars_abnormal_info *abnormal_info = (struct stars_abnormal_info *)msg->payload;
    struct trs_id_inst inst;

    if (abnormal_info == NULL) {
        trs_err("Data is NULL. (devid=%u; tsid=%u)\n", devid, msg->header.tsid);
        return -EFAULT;
    }

    trs_id_inst_pack(&inst, devid, msg->header.tsid);
    return trs_chan_abnormal_proc(&inst, abnormal_info);
}

int trs_host_set_ts_status(u32 devid, struct trs_msg_data *data)
{
    u32 status = data->payload[0];
    struct trs_id_inst inst;

    if ((status != TRS_INST_STATUS_NORMAL) && (status != TRS_INST_STATUS_ABNORMAL)) {
        trs_err("Invalid para. (devid=%u; tsid=%u; status=%u)\n", devid, data->header.tsid, status);
        return -EINVAL;
    }

    trs_id_inst_pack(&inst, devid, data->header.tsid);
    return trs_set_ts_status(&inst, status);
}

int trs_host_flush_id(u32 devid, struct trs_msg_data *data)
{
    struct trs_msg_id_sync_head *msg_info = (struct trs_msg_id_sync_head *)data->payload;
    struct trs_id_inst inst;

    trs_id_inst_pack(&inst, devid, data->header.tsid);
    return trs_id_free_batch_specific(&inst, msg_info->type);
}

int trs_host_res_id_check(struct trs_id_inst *inst, int id_type, u32 res_id)
{
#ifndef EMU_ST
    struct trs_msg_res_id_check *id_msg = NULL;
    struct trs_msg_data msg;
    int ret;

    msg.header.tsid = inst->tsid;
    msg.header.valid = TRS_MSG_SEND_MAGIC;
    msg.header.cmdtype = TRS_MSG_RES_ID_CHECK;
    msg.header.result = 0;

    id_msg = (struct trs_msg_res_id_check *)msg.payload;
    id_msg->hpid = (u32)current->tgid;
    id_msg->id_type = id_type;
    id_msg->res_id = res_id;

    ret = trs_host_msg_send(inst->devid, &msg, sizeof(struct trs_msg_data));
    if ((ret != 0) || (msg.header.result != 0)) {
        trs_err("Msg send fail. (devid=%u; tsid=%u; ret=%d; result=%d)\n",
            inst->devid, inst->tsid, ret, msg.header.result);
        return -ENODEV;
    }
#endif
    return 0;
}

