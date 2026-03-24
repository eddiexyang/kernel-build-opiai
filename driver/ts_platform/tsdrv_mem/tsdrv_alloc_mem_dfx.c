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

#include <linux/slab.h>
#include <linux/vmalloc.h>
#ifndef AOS_LLVM_BUILD
#include <linux/debugfs.h>
#endif
#include "securec.h"
#include "devdrv_common.h"
#include "tsdrv_log.h"
#include "tsdrv_dev_dfx.h"

#include "tsdrv_ts_alloc_mem.h"
#include "tsdrv_alloc_mem_dfx.h"

#define TSDRV_DFX_ALLOC_MEM_LEN 64

#ifndef CFG_TRS_REFACTOR_FEATURE
STATIC int alloc_mem_dfx_open(struct inode *inode, struct file *filp)
{
    u32 dfx_id = (u32)(uintptr_t)inode->i_private;
    u32 devid = tsdrv_dfx_id_to_devid(dfx_id);
    u32 tsid = tsdrv_dfx_id_to_tsid(dfx_id);
    u32 fid = tsdrv_dfx_id_to_fid(dfx_id);

    TSDRV_PRINT_DEBUG("devid(%u) fid(%u) tsid(%u)\n", devid, fid, tsid);
    filp->private_data = (void *)(uintptr_t)dfx_id;
    return 0;
}

STATIC ssize_t alloc_mem_dfx_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos)
{
    u32 dfx_id = (u32)(uintptr_t)file->private_data;
    u32 devid = tsdrv_dfx_id_to_devid(dfx_id);
    u32 tsid = tsdrv_dfx_id_to_tsid(dfx_id);
    u32 fid = tsdrv_dfx_id_to_fid(dfx_id);
    char buf[TSDRV_DFX_ALLOC_MEM_LEN] = {0};
    int buf_size = TSDRV_DFX_ALLOC_MEM_LEN;
    int len;
    char *str = buf;
    size_t output_len = 0;

    DRV_CHECK_EXP_ACT(devid >= TSDRV_MAX_DAVINCI_NUM, return 0, "Device id is invalid. (devid=%u)\n", devid);
    DRV_CHECK_EXP_ACT(tsid >= DEVDRV_MAX_TS_NUM, return 0, "Ts id is invalid. (tsid=%u)\n", tsid);
    DRV_CHECK_EXP_ACT(fid != TSDRV_PM_FID, return 0, "Only support physical. (fid=%u)\n", fid);

    len = snprintf_s(str, buf_size, buf_size - 1, "node_num=%u\n",
        tsdrv_get_isolation_mem_num(devid, tsid));
    DRV_CHECK_EXP_ACT(len < 0, return 0, "Snprintf failed. (len=%d)\n", len);
    str += len;
    buf_size -= len;
    output_len += len;

    len = snprintf_s(str, buf_size, buf_size - 1, "alloc_mem=%llu\n",
        tsdrv_get_allocated_mem_size(devid, tsid));
    DRV_CHECK_EXP_ACT(len < 0, return 0, "Snprintf failed. (len=%d)\n", len);
    str += len;
    buf_size -= len;
    output_len += len;

    return simple_read_from_buffer_safe(user_buf, count, ppos, (const void *)buf, output_len);
}

STATIC const struct file_operations alloc_mem_dfx_ops = {
    .open = alloc_mem_dfx_open,
    .read = alloc_mem_dfx_read,
    .release = NULL,
};

static int tsdrv_alloc_mem_dfx_create(u32 devid, u32 tsnum, u32 fid, struct dentry *parent)
{
    u32 tsid;

    for (tsid = 0; tsid < tsnum; tsid++) {
        char name[TSDRV_DFX_NAME_LEN] = {0};
        struct dentry *event_id = NULL;
        u32 dfx_id;
        int ret;

        ret = snprintf_s(name, TSDRV_DFX_NAME_LEN, TSDRV_DFX_NAME_LEN - 1, "mem_alloc_ts%u", tsid);
        if (ret < 0) {
            return -EFAULT;
        }

        dfx_id = tsdrv_pack_dfx_id(devid, fid, tsid);
        event_id = debugfs_create_file(name, S_IRUGO, parent, (void *)(uintptr_t)dfx_id, &alloc_mem_dfx_ops);
        if (event_id == NULL) {
            return -ENOMEM;
        }
    }

    return 0;
}

int tsdrv_alloc_mem_dfx_file_create(u32 devid, u32 tsnum, u32 fid)
{
    struct tsdrv_dfx_fid *dfx_fid = tsdrv_get_fid_dfx(devid, fid);
    struct dentry *mem_dir = NULL;
    int ret;

    if (dfx_fid->fid_dentry == NULL) {
        TSDRV_PRINT_ERR("Fid entry is null. (devid=%u; fid=%u)\n", devid, fid);
        return -ENODEV;
    }

    mem_dir = debugfs_create_dir("mem", dfx_fid->fid_dentry);
    if (mem_dir == NULL) {
        TSDRV_PRINT_ERR("Create mem dfx dir failed. (devid=%u; fid=%u)\n", devid, fid);
        return -EFAULT;
    }

    ret = tsdrv_alloc_mem_dfx_create(devid, tsnum, fid, mem_dir);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Create alloc mem dfx failed. (ret=%d)\n", ret);
        return ret;
    }
    return 0;
}
#endif

