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
 * Create: 2022-08-13
 */

#ifndef LOG_DRV_RES_COMMON_H
#define LOG_DRV_RES_COMMON_H
#include "log_drv_dev.h"
#include "log_drv_sqcq.h"

struct log_channel_desc *log_get_channel_desc(void);
int log_get_channel_num(void);
int log_is_channel_valid(int channel_id);
void log_init_channel_desc(void);
int log_channel_id_shift(u32 device_id, unsigned int tsid, int channel_id, int *set_level_channel_type);
int log_shift_cmd_to_ts(u32 device_id, struct log_sq_scheduler *sq_info, int *set_level_channel_type);

#endif
