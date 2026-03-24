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
* Create: 2022-10-12
*/
#ifndef TRS_MIA_DEVICE_H
#define TRS_MIA_DEVICE_H

#include <linux/types.h>

int trs_mia_device_init(u32 devid);
void trs_mia_device_uninit(u32 devid);

#endif /* TRS_MIA_DEVICE_H */

