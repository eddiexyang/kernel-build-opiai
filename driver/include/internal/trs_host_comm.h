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
#ifndef TRS_HOST_COMM_H
#define TRS_HOST_COMM_H

#include <linux/types.h>

#include "trs_pub_def.h"
#include "trs_msg.h"

int trs_host_get_ssid(struct trs_id_inst *inst, int *ssid);
int trs_host_ts_adapt_abnormal_proc(u32 devid, struct trs_msg_data *msg);
int trs_host_set_ts_status(u32 devid, struct trs_msg_data *data);
int trs_host_flush_id(u32 devid, struct trs_msg_data *data);
int trs_host_res_id_check(struct trs_id_inst *inst, int id_type, u32 res_id);

#endif /* TRS_HOST_COMM_H */
