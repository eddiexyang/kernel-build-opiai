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
#include "trs_chan.h"
#include "trs_device_chan.h"
#include "trs_chip_def.h"

int trs_chan_config(struct trs_id_inst *inst)
{
    int ret;

    ret = trs_chan_ops_init(inst);
    if (ret != 0) {
        return ret;
    }

    ret = trs_chan_ts_inst_register(inst, trs_get_hw_type(), trs_chan_get_adapt_ops());
    if (ret != 0) {
        trs_chan_ops_uninit(inst);
    }
    return ret;
}

void trs_chan_deconfig(struct trs_id_inst *inst)
{
    trs_chan_ts_inst_unregister(inst);
    trs_chan_ops_uninit(inst);
}

