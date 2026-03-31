/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
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
* Create: 2019-10-15
*/

#include "devdrv_common.h"
#include "tsdrv_device.h"
#include "devdrv_cb.h"
#include "devdrv_cqsq.h"

#include "hvtsdrv_cqsq.h"

void hvtsdrv_cb_update_cq_report(struct work_struct *work)
{
}

void hvtsdrv_cb_cq_handler(u32 devid, u32 fid, u32 tsid, u32 cq_id)
{
}

s32 hvtsdrv_cb_inform_tsagent_vsq_tail(u32 devid, u32 fid, u32 vsqid, u32 tail)
{
    return 0;
}

