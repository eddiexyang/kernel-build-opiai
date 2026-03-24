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
* Create: 2023-02-22
*/
#ifndef __VIRTMNGDEV_MIA_CFG_H__
#define __VIRTMNGDEV_MIA_CFG_H__

#include "dbl/uda.h"
#include "virtmngdev_res_mng.h"

int vmngd_add_mia_uda(u32 dev_id, u32 vfid, enum uda_dev_prop prop);
int vmngd_remove_mia_uda(u32 dev_id, u32 vfid, enum uda_dev_prop prop);
int vmngd_config_soc_res(u32 dev_id, u32 vfid);

#endif