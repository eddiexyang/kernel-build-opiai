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
* Create: 2022-7-15
*/

#include "trs_chan_mbox.h"
#include "trs_device_comm.h"

void trs_chan_update_ssid(struct trs_id_inst *inst, struct trs_chan_info *chan_info)
{
    if ((chan_info->types.type == CHAN_TYPE_HW) && (chan_info->types.sub_type == CHAN_SUB_TYPE_HW_DVPP)) {
        int ssid;
        int ret = trs_device_get_ssid(inst, &ssid);
        if (ret != 0) {
            trs_warn("Get ssid fail. (devid=%u; ret=%d)\n", inst->devid, ret);
        }
        chan_info->ssid = (u16)ssid;
    }
}
EXPORT_SYMBOL(trs_chan_update_ssid);

int trs_chan_ops_mbox_send(struct trs_id_inst *inst, struct trs_chan_info *chan_info)
{
    struct trs_chan_adapt_info adapt;

    adapt.app_type = TRS_MBOX_SEND_FROM_DEVICE;
    adapt.sq_side = 0;
    adapt.cq_side = 0;
    adapt.fid = 0;

    trs_chan_update_ssid(inst, chan_info);

    return trs_chan_mbox_send(inst, chan_info, &adapt);
}
