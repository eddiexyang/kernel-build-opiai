/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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
 * Create: 2022-02-16
 */

#ifndef __VIRTMNGDEV_CLIENT_ONLINE_H__
#define __VIRTMNGDEV_CLIENT_ONLINE_H__

#include "virtmngdev_res_mng.h"

unsigned int get_res_num(vmng_resource_list *list, enum soc_mia_res_type type);
unsigned long get_res_bitmap(vmng_resource_list *list, enum soc_mia_res_type type);
void vmngd_print_ts_cfg(const vmng_vf_cfg_t *cfg);
void vmngd_set_resource_to_vdev_ctrl(u32 dev_id, u32 vfid, vmng_vf_cfg_t *cfg);

#endif
