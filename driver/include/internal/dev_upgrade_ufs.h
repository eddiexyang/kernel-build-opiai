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

#ifndef DEV_UPGRADE_UFS_H
#define DEV_UPGRADE_UFS_H

#include <linux/types.h>
#include <linux/fs.h>
#include "dev_upgrade_public.h"

#define MAX_BUFFER_SIZE (16*1024*1024) /* 16MB */

typedef struct tag_dev_upgrade_core_ctrl_st dev_upgrade_core_ctrl;

/**
 * UFS driver API
 */
extern int ufs_read(unsigned int flags, u64 offset, void *data, u64 bytes);
extern int ufs_write(unsigned int flags, u64 offset, const void *data, u64 bytes);
extern void ufs_dfx_data_check(const u8 *addr, uint32_t size);
/**
 * open_mdc_package锛? open a MDC upgrade component package
 * @full_name:  package path and file name
 *
 * open a MDC upgrade package in read only mode.
 */
struct file *open_mdc_package(const char *full_name);

/**
 * read_mdc_package_content: read a block from upgrade package
 * @fp : struct file pointer returned by open_mdc_package
 * @buff: buffer to store the content of file, length limit in @len
 * @len:  max size to read
 * @read_len: the real length has read in file.
 *
 * Return
 *   0-READ_END: read to end successfully
 *   1-READ_CONTINUE: read will be continued
 *  -1-READ_ERROR: error occured since read
 *
 * read the content from file, the offset is store in fp, every time,
 * the start position is the end of last read.
 */
int read_mdc_package_content(struct file *fp, char *buff, u32 len, int *read_len);

/**
 * close_mdc_package: close file point
 * @fp: file point which return by open_mdc_package funciton
 *
 * close package file point.
 */
void close_mdc_package(struct file *fp);

/**
 * dev_upgrade_ufs_read: read ufs data
 * @flags: phy partition lu
 * @offset: read offset
 * @data: read data pointer
 * @bytes: read data count
 *
 * return: read result.
 */
int dev_upgrade_ufs_read(unsigned int flags, u64 offset, char *data, u64 bytes);

/**
 * dev_upgrade_ufs_write: write ufs data
 * @flags: phy partition lu
 * @offset: write offset
 * @data: write data pointer
 * @bytes: write data count
 *
 * return: write result.
 */
int dev_upgrade_ufs_write(unsigned int flags, u64 offset, const char *data, u64 bytes);

#endif

