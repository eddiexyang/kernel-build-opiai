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
#ifndef TRS_DEVICE_COMM_H
#define TRS_DEVICE_COMM_H

#include <linux/types.h>

#include "trs_pub_def.h"

#define ALL_FEATURE_MODE    0
#define PART_FEATURE_MODE   1

int trs_device_get_remote_ssid(u32 devid, u32 vfid, pid_t hpid);

int trs_tscpu_chan_create(struct trs_id_inst *inst);
void trs_tscpu_chan_destroy(struct trs_id_inst *inst);

void trs_invalid_cache(u64 base, u32 len);
void trs_flush_cache(u64 base, u32 len);

int trs_get_ts_nid(u32 devid);

int trs_ts_adapt_abnormal_proc(u32 udevid, u32 tsid, void *data);

void trs_set_feature_mode(void);
u32 trs_get_feature_mode(void);

#endif /* TRS_DEVICE_COMM_H */
