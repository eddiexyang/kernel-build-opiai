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

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/securec.h>

#include "dev_upgrade_public.h"
#include "dev_upgrade_def.h"
#include "dev_upgrade_ssd.h"

#define SSD_DEV_PATH "/dev/nvme0n1"


#ifdef CFG_SOC_PLATFORM_MDC_V11
#define EMMC_DEV_PATH "/dev/mmcblk0"

static int dev_upgrade_commom_read(struct file *fp,  u64 offset, char *data, u64 bytes)
{
    ssize_t ret;
    loff_t read_offset;

    if (IS_ERR(fp)) {
        dev_upgrade_err("filp_open failed\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_OPEN);
    }

    read_offset = vfs_llseek(fp, (loff_t)offset, SEEK_SET);
    if (read_offset < 0) {
        dev_upgrade_err("vfs_llseek fail, read offset=%llu.\n", read_offset);
        filp_close(fp, NULL);
        fp = NULL;
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_SEEK);
    }

    ret = kernel_read(fp, (void *)data, (size_t)bytes, &read_offset);
    if (ret != bytes) {
        dev_upgrade_err("kernel_read fail, ret = %lu.\n", ret);
        filp_close(fp, NULL);
        fp = NULL;
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_READ);
    }

    filp_close(fp, NULL);
    fp = NULL;
    return OK;
}

static int dev_upgrade_commom_write(struct file *fp,  u64 offset, const char *data, u64 bytes)
{
    ssize_t ret;
    loff_t write_offset;

    if (IS_ERR(fp)) {
        dev_upgrade_err("filp_open failed\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_OPEN);
    }

    write_offset = vfs_llseek(fp, (loff_t)offset, SEEK_SET);
    if (write_offset < 0) {
        dev_upgrade_err("vfs_llseek fail, read offset=%llu.\n", write_offset);
        filp_close(fp, NULL);
        fp = NULL;
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_SEEK);
    }

    ret = kernel_write(fp, (const void *)data, (size_t)bytes, &write_offset);
    if (ret != bytes) {
        dev_upgrade_err("kernel_write fail, ret = %lu.\n", ret);
        filp_close(fp, NULL);
        fp = NULL;
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_WRITE);
    }

    filp_close(fp, NULL);
    fp = NULL;
    return OK;
}

int dev_upgrade_ssd_read(unsigned int flags, u64 offset, char *data, u64 bytes)
{
    struct file *fp = NULL;
    fp = filp_open(SSD_DEV_PATH, O_RDONLY | O_LARGEFILE | O_SYNC, 0);
    return dev_upgrade_commom_read(fp, offset, data, bytes);
}

int dev_upgrade_ssd_write(unsigned int flags, u64 offset, const char *data, u64 bytes)
{
    struct file *fp = NULL;
    fp = filp_open(SSD_DEV_PATH, O_RDWR | O_LARGEFILE | O_SYNC, 0);
    return dev_upgrade_commom_write(fp, offset, data, bytes);
}

int dev_upgrade_emmc_read(unsigned int flags, u64 offset, char *data, u64 bytes)
{
    struct file *fp = NULL;
    fp = filp_open(EMMC_DEV_PATH, O_RDONLY | O_LARGEFILE | O_SYNC, 0);
    return dev_upgrade_commom_read(fp, offset, data, bytes);
}

int dev_upgrade_emmc_write(unsigned int flags, u64 offset, const char *data, u64 bytes)
{
    struct file *fp = NULL;
    fp = filp_open(EMMC_DEV_PATH, O_RDWR | O_LARGEFILE | O_SYNC, 0);
    return dev_upgrade_commom_write(fp, offset, data, bytes);
}

#else
int dev_upgrade_ssd_read(unsigned int flags, u64 offset, char *data, u64 bytes)
{
    ssize_t ret;
    struct file *fp = NULL;
    loff_t read_offset;

    fp = filp_open(SSD_DEV_PATH, O_RDONLY | O_LARGEFILE | O_SYNC, 0);
    if (IS_ERR(fp)) {
        dev_upgrade_err("filp_open %s failed\n", SSD_DEV_PATH);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_OPEN);
    }

    read_offset = vfs_llseek(fp, (loff_t)offset, SEEK_SET);
    if (read_offset < 0) {
        dev_upgrade_err("vfs_llseek fail, read offset=%llu.\n", read_offset);
        filp_close(fp, NULL);
        fp = NULL;
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_SEEK);
    }

    ret = kernel_read(fp, (void *)data, (size_t)bytes, &read_offset);
    if (ret != bytes) {
        dev_upgrade_err("kernel_read fail, ret = %lu.\n", ret);
        filp_close(fp, NULL);
        fp = NULL;
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_READ);
    }

    filp_close(fp, NULL);
    fp = NULL;
    return OK;
}

int dev_upgrade_ssd_write(unsigned int flags, u64 offset, const char *data, u64 bytes)
{
    ssize_t ret;
    struct file *fp = NULL;
    loff_t write_offset;

    fp = filp_open(SSD_DEV_PATH, O_RDWR | O_LARGEFILE | O_SYNC, 0);
    if (IS_ERR(fp)) {
        dev_upgrade_err("filp_open %s failed\n", SSD_DEV_PATH);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_OPEN);
    }

    write_offset = vfs_llseek(fp, (loff_t)offset, SEEK_SET);
    if (write_offset < 0) {
        dev_upgrade_err("vfs_llseek fail, read offset=%llu.\n", write_offset);
        filp_close(fp, NULL);
        fp = NULL;
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_SEEK);
    }

    ret = kernel_write(fp, (const void *)data, (size_t)bytes, &write_offset);
    if (ret != bytes) {
        dev_upgrade_err("kernel_write fail, ret = %lu.\n", ret);
        filp_close(fp, NULL);
        fp = NULL;
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_WRITE);
    }

    filp_close(fp, NULL);
    fp = NULL;
    return OK;
}
#endif
