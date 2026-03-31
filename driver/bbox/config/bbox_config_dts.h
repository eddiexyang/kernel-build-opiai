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

#ifndef BBOX_CONFIG_DTS_H
#define BBOX_CONFIG_DTS_H

#include "device/bbox_types.h"

#define RDR_FLASH_TYPE_CHIP 0
#define RDR_FLASH_TYPE_FPGA 1
#define RDR_FLASH_TYPE_ESL  2
#define RDR_FLASH_TYPE_MAX  3

s32 bbox_config_init(void);

#endif