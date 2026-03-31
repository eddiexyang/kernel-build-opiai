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
 * Create: 2023-10-23
 */
#ifndef SAFETY_RAS_COMMON_H
#define SAFETY_RAS_COMMON_H
#include <linux/types.h>

#define SAFETY_RAS_SUPPORTED      0
#define SAFETY_RAS_NOT_SUPPORTED  1U

uint32_t safety_ras_supported(void);
#endif