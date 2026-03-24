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

#include "tsdrv_ioctl.h"
#include "devdrv_cqsq.h"
#include "hvtsdrv_shm_sqcq.h"

int hvtsdrv_shm_sqcq_alloc(u32 devid, u32 fid, pid_t tgid, struct devdrv_ioctl_arg *arg)
{
    return 0;
}

int hvtsdrv_shm_sqcq_free(u32 devid, u32 fid, pid_t tgid, struct devdrv_ioctl_arg *arg)
{
    return 0;
}

int hvtsdrv_shm_offset_get(u32 devid, u32 tsid, u32 fid, pid_t tgid, u8 *msg)
{
    return 0;
}

int hvtsdrv_get_shm_phy_sq(u32 devid, u32 fid, u32 tsid, struct phy_sq_alloc_para *phy_sq)
{
    return 0;
}

