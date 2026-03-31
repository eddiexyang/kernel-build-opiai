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

#ifndef CAN_DRV_FEATURE_H
#define CAN_DRV_FEATURE_H

#include "drv_type.h"

#define DMS_CAN_CMD_NAME "DMS_CAN"

int mttcan_get_can_status(void *feature, char *in, unsigned int in_len, char *out, unsigned int out_len);
int mttcan_dmsi_set_can_config(void *feature, char *in, unsigned int in_len, char *out, unsigned int out_len);
int mttcan_dmsi_get_can_config(void *feature, char *in, unsigned int in_len, char *out, unsigned int out_len);

int can_drv_feature_init(void);
void can_drv_feature_uninit(void);
#endif