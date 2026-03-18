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
* Create: 2023-8-21
*/
#include "trs_pub_def.h"
#include "trs_ts_status.h"
#include "trs_device_notice.h"
int trs_set_ts_hb_status(u32 devid, u32 tsid, u32 status)
{
    struct trs_adapt_notice_ops *notice_ops = trs_adapt_get_notice_ops();
    struct trs_id_inst inst;
    int ret;

    trs_id_inst_pack(&inst, devid, tsid);
    ret = trs_set_ts_status(&inst, status);
    if (ret != 0) {
        trs_err("Set ts status failed. (devid=%u; tsid=%u; status=%u; ret=%d)\n", inst.devid, inst.tsid, status, ret);
        return ret;
    }

    if (notice_ops->set_ts_status != NULL) {
        return notice_ops->set_ts_status(devid, tsid, status);
    }

    return 0;
}
EXPORT_SYMBOL(trs_set_ts_hb_status);