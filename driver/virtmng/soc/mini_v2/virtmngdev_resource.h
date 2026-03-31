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

#ifndef __VIRTMNGDEV_RESOURCE_H__
#define __VIRTMNGDEV_RESOURCE_H__

#include "virtmngdev_res_common.h"

#define VMNGD_DVPP_ENGINE_NUM 6
#define VMNGD_DVPP_VENC_INDEX 5
#define VMNGD_DVPP_PNGD_INDEX 4

enum vmng_vfg_status {
    VMNG_VFG_FREE = 0,
    VMGN_VFG_OCCUPIED
};

int vmngd_res_ops_init(struct vmngd_resource_ops *ops);
int vmngd_res_ops_uninit(struct vmngd_resource_ops *ops);

#endif
