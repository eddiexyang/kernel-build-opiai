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
* Create: 2023-08-24
*/
#include <securec.h>

#include "trs_msg.h"
#include "trs_agent.h"
#include "trs_d2h_msg_ops.h"
int trs_send_set_ts_status_d2h_msg(u32 devid, u32 tsid, u32 status)
{
    struct trs_msg_data data;
    int ret;

    data.header.cmdtype = TRS_MSG_SET_TS_STATUS;
    data.header.devid = devid;
    data.header.tsid = tsid;
    data.header.result = TRS_MSG_INVALID_RESULT;
    data.header.valid = TRS_MSG_SEND_MAGIC;
    data.payload[0] = (u8)status;

    ret = trs_agent_non_trans_msg_send(devid, (void*)&data, sizeof(struct trs_msg_data));
    if ((ret != 0) || (data.header.result != 0) || (data.header.valid != TRS_MSG_RCV_MAGIC)) {
        trs_err("Msg chan send failed. (devid=%u; result=%u; valid = 0x%x; ret=%d)\n",
            devid, data.header.result, data.header.valid, ret);
    }

    return ret;
}

static int trs_ts_agent_abnormal_proc(u32 devid, u32 tsid, void *data)
{
    struct trs_msg_data msg_head = { 0 };
    struct stars_abnormal_info *abnormal_info = (struct stars_abnormal_info *)msg_head.payload;
    int ret;

    if (data == NULL) {
        trs_err("Data is NULL. (devid=%u; tsid=%u)\n", devid, tsid);
        return -EFAULT;
    }

    msg_head.header.devid = devid;
    msg_head.header.tsid = tsid;
    msg_head.header.cmdtype = TRS_MSG_CHAN_ABNORMAL;
    msg_head.header.valid = TRS_MSG_SEND_MAGIC;
    msg_head.header.result = TRS_MSG_INVALID_RESULT;

    (void)memcpy_s(abnormal_info, sizeof(struct stars_abnormal_info), data, sizeof(struct stars_abnormal_info));

    ret = trs_agent_non_trans_msg_send(devid, (void*)&msg_head, sizeof(struct trs_msg_data));
    if ((ret != 0) || (msg_head.header.result != 0) || (msg_head.header.valid != TRS_MSG_RCV_MAGIC)) {
        trs_err("Msg chan send failed. (devid=%u; ret=%d; result=%u; valid=0x%x)\n",
            devid, ret, (u32)msg_head.header.result, (u32)msg_head.header.valid);
        return -EFAULT;
    }

    return 0;
}

static int trs_sync_id_proc(u32 devid, u32 tsid, int type)
{
    struct trs_msg_data data = {0};
    struct trs_msg_id_sync_head *msg_info = (struct trs_msg_id_sync_head *)data.payload;
    int ret;

    data.header.cmdtype = TRS_MSG_FLUSH_RES_ID;
    data.header.devid = devid;
    data.header.tsid = tsid;
    data.header.result = TRS_MSG_INVALID_RESULT;
    data.header.valid = TRS_MSG_SEND_MAGIC;
    msg_info->type = type;

    ret = trs_agent_non_trans_msg_send(devid, (void*)&data, sizeof(struct trs_msg_data));
    if ((ret != 0) || (data.header.result != 0) || (data.header.valid != TRS_MSG_RCV_MAGIC)) {
        trs_err("Msg chan send failed. (devid=%u; result=%u; valid = 0x%x; ret=%d)\n",
            devid, data.header.result, data.header.valid, ret);
    }

    return ret;
}

struct trs_adapt_notice_ops ops = {
    .set_ts_status = trs_send_set_ts_status_d2h_msg,
    .abnormal_proc = trs_ts_agent_abnormal_proc,
    .sync_id_proc = trs_sync_id_proc,
};

struct trs_adapt_notice_ops *trs_agent_get_adapt_notice_ops(void)
{
    return &ops;
}
