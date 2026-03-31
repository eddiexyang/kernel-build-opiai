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
* Create: 2022-08-25
*/

#ifndef PROF_HWTS_INIT_H
#define PROF_HWTS_INIT_H

#define PROF_DEV_IS_VM             1
#define PROF_DEV_IS_PHY            2

void prof_set_device_flag_vm_or_phy(u32 device_id, u32 flag);
int prof_refresh_rw_ptr(struct prof_sub_channel_info *sub_channel_info, int refresh_type);

int prof_init_almost_full_irq(u32 device_id, u32 tsid);
void prof_uninit_almost_full_irq(u32 device_id, u32 tsid);

#endif
