/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2023. All rights reserved.
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

#ifndef ESCHED_DRV_MIA_H
#define ESCHED_DRV_MIA_H

struct esched_mia_dev {
    u32 valid;
    u32 dev_id;
    u32 chip_id;
    u32 vfid;
    u32 dtype;
    u32 pool_id;
    u32 sched_cpu_mask;
    u32 slot_mask;
    u64 host_sched_cpu_mask;
    u32 rtsq_num;
};

struct esched_mia_dev *esched_drv_get_mia_dev(u32 dev_id);

#endif

