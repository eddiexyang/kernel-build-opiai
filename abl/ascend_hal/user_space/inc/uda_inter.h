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
* Create: 2023-5-16
*/

#ifndef UDA_INTER_H
#define UDA_INTER_H

#include <stdint.h>

int uda_get_udevid_by_devid(uint32_t devid, uint32_t *udevid);
int uda_get_devid_by_udevid(uint32_t udevid, uint32_t *devid);

#endif
