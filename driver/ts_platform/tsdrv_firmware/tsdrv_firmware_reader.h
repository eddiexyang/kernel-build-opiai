/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
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

#ifndef TSDRV_FIRMWARE_READER_H
#define TSDRV_FIRMWARE_READER_H

#include "devdrv_common.h"

size_t get_file_size(const char *path);
bool copy_firmware(struct file *fp, size_t fsize, loff_t pos, void *firmware_dst_addr);
#ifdef AOS_LLVM_BUILD
int tsdrv_get_firmware(const char *path, void *dst_addr, size_t dest_max);
#endif
int tsdrv_firmware_read(const char *path, void **fw_src_addr, size_t *file_size);
int tsdrv_firmware_write(struct devdrv_info *dev_info);
int tsdrv_ffts_read(const char *path, void **ffts_src_addr, size_t *file_size);
int tsdrv_ffts_write(struct devdrv_info *dev_info, void *src, u32 src_size, void *dst, u32 dst_size);
#endif

