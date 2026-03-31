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
 * Create: 2022-08-15
 */
#ifndef TSDRV_FILE_CHECK_H
#define TSDRV_FILE_CHECK_H

#include "devdrv_common.h"
int devdrv_tsfw_bin_check(struct devdrv_info *dev_info, const char *firmware_path);
int tsdrv_ffts_check(u32 dev_id, void *file_src_addr, u32 file_size);
#endif

