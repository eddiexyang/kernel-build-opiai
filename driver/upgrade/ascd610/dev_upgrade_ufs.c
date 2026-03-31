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

/**
 * brief description about this document.
 * points to focus on.
 */
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/printk.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/securec.h>
#include <linux/vmalloc.h>

#include "dev_upgrade_ufs.h"
#include "dev_upgrade_public.h"
#include "dev_upgrade_def.h"
#include "dev_upgrade_mdccore.h"

/**
 * open mdc package in read only mode.
 */
struct file *open_mdc_package(const char *full_name)
{
    struct file *fp = filp_open(full_name, O_RDONLY | O_LARGEFILE, 0);
    if (IS_ERR(fp)) {
        dev_upgrade_err("%s: open file %s failed\n", __func__, full_name);
        return NULL;
    }

    return fp;
}

/**
 * read package file.
 */
int read_mdc_package_content(struct file *fp, char *buff, u32 len, int *read_len)
{
    ssize_t ret;
    loff_t pos = fp->f_pos;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
    ret = kernel_read(fp, buff, len, &pos);
#else
    mm_segment_t old_fs;
    old_fs = get_fs();
    set_fs(KERNEL_DS);
    ret = vfs_read(fp, buff, len, &pos);
    set_fs(old_fs);
#endif

    if (ret < 0) {
        return READ_ERROR;
    }

    *read_len = ret;
    if (ret < len) {
        ret = READ_END;
    } else {
        ret = READ_CONTINUE;
    }

    fp->f_pos = pos;
    return ret;
}

/**
 * close package file point.
 */
void close_mdc_package(struct file *fp)
{
    if (fp != NULL) {
        if (filp_close(fp, NULL)) {
            dev_upgrade_err("%s: close file error.\n", __func__);
        }
    }
}

#ifdef CFG_SOC_PLATFORM_MDC_LITE_ESL
// esl don't support tee
int ufs_read(unsigned int flags, u64 offset, void *data, u64 bytes)
{
    dev_upgrade_err("ufs_read error, esl don't support ufs");
    return 0;
}

int ufs_write(unsigned int flags, u64 offset, const void *data, u64 bytes)
{
    dev_upgrade_err("ufs_read error, esl don't support ufs");
    return 0;
}
#endif

int dev_upgrade_ufs_read(unsigned int flags, u64 offset, char *data, u64 bytes)
{
#ifdef CFG_SOC_PLATFORM_MDC_V11
    return 0;
#else
    return ufs_read(flags, offset, (void *)data, bytes);
#endif
}

int dev_upgrade_ufs_write(unsigned int flags, u64 offset, const char *data, u64 bytes)
{
#ifdef CFG_SOC_PLATFORM_MDC_V11
    return 0;
#else
    return ufs_write(flags, offset, (const void *)data, bytes);
#endif
}
