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
* Create: 2023-06-10
*/

#include "chan_init.h"
#include "trs_abnormal_info.h"
#include "trs_chan.h"

trs_chan_abnormal_handle trs_chan_sq_abnormal_handle[ABNORMAL_TASK_TYPE_MAX] = {NULL};

int trs_chan_register_abnormal_handle(u32 task_type, trs_chan_abnormal_handle handle)
{
    if (task_type >= ABNORMAL_TASK_TYPE_MAX) {
        trs_err("Invalid para. (task_type=%u; max=%u)\n", task_type, ABNORMAL_TASK_TYPE_MAX);
        return -EINVAL;
    }

    if (trs_chan_sq_abnormal_handle[task_type] != NULL) {
        trs_err("Abnormal handle has already been registered. (task_type=%u)\n", task_type);
        return -EINVAL;
    }

    trs_chan_sq_abnormal_handle[task_type] = handle;

    return 0;
}
EXPORT_SYMBOL(trs_chan_register_abnormal_handle);

int trs_chan_unregister_abnormal_handle(u32 task_type)
{
    if (task_type >= ABNORMAL_TASK_TYPE_MAX) {
        trs_err("Invalid para. (task_type=%u; max=%u)\n", task_type, ABNORMAL_TASK_TYPE_MAX);
        return -EINVAL;
    }

    trs_chan_sq_abnormal_handle[task_type] = NULL;

    return 0;
}
EXPORT_SYMBOL(trs_chan_unregister_abnormal_handle);

static int trs_task_abnormal_proc(struct trs_id_inst *inst, struct trs_chan *chan, u8 task_type, void* info)
{
    int ret = 0;

    if (trs_chan_sq_abnormal_handle[task_type] != NULL) {
        trs_info("Process chan abnormal task. (devid=%u; tsid=%u; chan_id=%d; sqid=%u; task_type=%u)\n",
            inst->devid, inst->tsid, chan->id, chan->sq.sqid, task_type);
        ret = trs_chan_sq_abnormal_handle[task_type](inst, chan->sq.sqid, NULL, info);
        if (ret != 0) {
            trs_err("Failed to handle aicpu sq abnormal task. (chan_id=%d, task_type=%d)\n", chan->id, task_type);
        }
    }

    return ret;
}

static int _trs_chan_abnormal_proc(struct trs_id_inst *inst, struct trs_chan *chan, u8 err_type)
{
    int ret = 0;

    if (chan->ops.abnormal_proc != NULL) {
        ret = chan->ops.abnormal_proc(inst, chan->id, err_type);
        if (ret != 0) {
            trs_err("Chan abnormal proc failed. (devid=%u; tsid=%u; chan_id=%d; sqid=%u; err_type=%u)\n",
                inst->devid, inst->tsid, chan->id, chan->sq.sqid, err_type);
        }
    }

    return ret;
}

int trs_chan_abnormal_proc(struct trs_id_inst *inst, struct stars_abnormal_info *abnormal_info)
{
    struct trs_chan_ts_inst *ts_inst = NULL;
    struct trs_chan *chan = NULL;
    u8 task_type = abnormal_info->task_type;
    u8 err_type = abnormal_info->err_type;
    int ret, chan_id;

    if ((err_type >= ABNORMAL_ERR_TYPE_MAX) || (task_type >= ABNORMAL_TASK_TYPE_MAX)) {
        trs_err("Invalid para. (err_type=%u; task_type=%u)\n", err_type, task_type);
        return -EINVAL;
    }

    ts_inst = trs_chan_ts_inst_get(inst);
    if (ts_inst == NULL) {
        trs_warn("Invalid para. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        return -EINVAL;
    }

    chan_id = trs_chan_sq_to_chan_id(ts_inst, abnormal_info->sqid);
    if (chan_id == -1) {
        trs_chan_ts_inst_put(ts_inst);
        return -EINVAL;
    }

    chan = trs_chan_get(inst, (u32)chan_id);
    if (chan == NULL) {
        trs_chan_ts_inst_put(ts_inst);
        return -EINVAL;
    }

    trs_info("Abnormal info. (devid=%u; tsid=%u; chan_id=%d; task_type=%u; err_type=%u)\n",
        inst->devid, inst->tsid, chan->id, task_type, err_type);

    ret = trs_task_abnormal_proc(inst, chan, task_type, (void*)abnormal_info->abnormal_data);
    if (ret != 0) {
        goto exit;
    }

    ret = _trs_chan_abnormal_proc(inst, chan, err_type);
exit:
    trs_chan_put(chan);
    trs_chan_ts_inst_put(ts_inst);

    return ret;
}
EXPORT_SYMBOL(trs_chan_abnormal_proc);
