/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
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

#ifndef _DEV_UPGRADE_SSD_H
#define _DEV_UPGRADE_SSD_H

#include <linux/types.h>
#include "dev_upgrade_public.h"

int dev_upgrade_ssd_read(unsigned int flags, u64 offset, char *data, u64 bytes);
int dev_upgrade_ssd_write(unsigned int flags, u64 offset, const char *data, u64 bytes);
#ifdef CFG_SOC_PLATFORM_MDC_V11
int dev_upgrade_emmc_read(unsigned int flags, u64 offset, char *data, u64 bytes);
int dev_upgrade_emmc_write(unsigned int flags, u64 offset, const char *data, u64 bytes);
#endif
#endif

