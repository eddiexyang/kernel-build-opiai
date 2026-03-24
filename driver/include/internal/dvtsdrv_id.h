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
 * Create: 2022-05-21
 */
#include "virtmng_interface.h"
#include "tsdrv_device.h"
#include "drv_type.h"

void dvtsdrv_res_ids_restore(u32 devid, u32 fid, u32 tsid);
void dvtsdrv_free_reserved_ids(u32 devid, u32 fid, u32 tsid);
void tsdrv_vdev_set_id_range(u32 devid, u32 tsnum, vf_ac_info_t *ts_ac_info);
int dvtsdrv_ids_init(u32 devid, u32 fid, u32 tsnum, vf_ac_info_t *ts_ac_info);
void dvtsdrv_ids_destroy(u32 devid, u32 fid, u32 tsnum);
int dvtsdrv_basic_module_init(u32 devid, u32 fid, u32 tsnum, vf_ac_info_t *ts_ac_info);
void dvtsdrv_basic_module_uninit(u32 devid, u32 fid, u32 tsnum);

