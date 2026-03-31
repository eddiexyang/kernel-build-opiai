/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2023. All rights reserved.
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

#include "tsdrv_ipc.h"
#include "tsdrv_ipc_dfx.h"

#define TSDRV_DFX_IPC_LEN 1024

#ifndef CFG_TRS_REFACTOR_FEATURE
#ifndef AOS_LLVM_BUILD
STATIC int tsdrv_ipc_dfx_open(struct inode *inode, struct file *filp)
{
    u32 dfx_id = (u32)(uintptr_t)inode->i_private;
    u32 devid = tsdrv_dfx_id_to_devid(dfx_id);
    u32 tsid = tsdrv_dfx_id_to_tsid(dfx_id);
    u32 fid = tsdrv_dfx_id_to_fid(dfx_id);

    TSDRV_PRINT_INFO("devid(%u) fid(%u) tsid(%u)\n", devid, fid, tsid);
    filp->private_data = (void *)(uintptr_t)dfx_id;
    return 0;
}

STATIC ssize_t tsdrv_ipc_dfx_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos)
{
    u32 dfx_id = (u32)(uintptr_t)file->private_data;
    u32 devid = tsdrv_dfx_id_to_devid(dfx_id);
    u32 fid = tsdrv_dfx_id_to_fid(dfx_id);
    struct tsdrv_ipc_chan *ipc_chan = NULL;
    char buf[TSDRV_DFX_IPC_LEN] = {0};
    int buf_size = TSDRV_DFX_IPC_LEN;
    int len;
    char *str = buf;
    size_t output_len = 0;
    u32 idx;

    DRV_CHECK_EXP_ACT(devid >= TSDRV_MAX_DAVINCI_NUM, return 0, "Device id is invalid. (devid=%u)\n", devid);
    DRV_CHECK_EXP_ACT(fid != TSDRV_PM_FID, return 0, "Only support physical. (fid=%u)\n", fid);

    len = snprintf_s(str, buf_size, buf_size - 1, "devid     tsid     tx     rx\n");
    DRV_CHECK_EXP_ACT(len < 0, return 0, "Snprintf fail. (len=%d)\n", len);
    str += len;
    buf_size -= len;
    output_len += len;
    DRV_CHECK_EXP_ACT(buf_size <= 0, return 0, "Buffer are not enough.\n");

    ipc_chan = tsdrv_get_ipc_chan_table(devid);
    for (idx = 0; idx < HISI_RPROC_MAX; idx++) {
        if (ipc_chan[idx].nb_name == NULL) {
            break;
        }

        len = snprintf_s(str, buf_size, buf_size - 1, "%5u%9u%7u%7u\n",
            ipc_chan[idx].devid, ipc_chan[idx].tsid,
            ipc_chan[idx].tx_rproc_id, ipc_chan[idx].rx_rproc_id);
        DRV_CHECK_EXP_ACT(len < 0, return 0, "Snprintf failed. (len=%d)\n", len);
        str += len;
        buf_size -= len;
        output_len += len;
        DRV_CHECK_EXP_ACT(buf_size <= 0, return 0, "Buffer are not enough.\n");
    }

    output_len += len;
    return simple_read_from_buffer_safe(user_buf, count, ppos, (const void *)buf, output_len);
}

static const struct file_operations ipc_dfx_ops = {
    .open = tsdrv_ipc_dfx_open,
    .read = tsdrv_ipc_dfx_read,
    .release = NULL,
};

static int tsdrv_ipc_dfx_create(u32 devid, u32 fid, struct dentry *parent)
{
    char name[TSDRV_DFX_NAME_LEN] = {0};
    struct dentry *event_id = NULL;
    u32 dfx_id;
    int ret;

    ret = snprintf_s(name, TSDRV_DFX_NAME_LEN, TSDRV_DFX_NAME_LEN - 1, "ipc_info");
    if (ret < 0) {
        return -EFAULT;
    }

    dfx_id = tsdrv_pack_dfx_id(devid, fid, 0);
    event_id = debugfs_create_file(name, S_IRUGO, parent, (void *)(uintptr_t)dfx_id, &ipc_dfx_ops);
    if (event_id == NULL) {
        return -ENOMEM;
    }

    return 0;
}

int tsdrv_ipc_dfx_file_create(u32 devid, u32 fid)
{
    struct tsdrv_dfx_fid *dfx_fid = tsdrv_get_fid_dfx(devid, fid);
    struct dentry *ipc_dir = NULL;
    int ret;

    if (dfx_fid->fid_dentry == NULL) {
        TSDRV_PRINT_ERR("Fid entry is null. (devid=%u; fid=%u)\n", devid, fid);
        return -ENODEV;
    }

    ipc_dir = debugfs_create_dir("ipc", dfx_fid->fid_dentry);
    if (ipc_dir == NULL) {
        TSDRV_PRINT_ERR("Create ipc dfx dir failed. (devid=%u; fid=%u)\n", devid, fid);
        return -EFAULT;
    }

    ret = tsdrv_ipc_dfx_create(devid, fid, ipc_dir);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Create ipc dfx failed. (ret=%d)\n", ret);
        return ret;
    }

    return 0;
}
#else
STATIC int tsdrv_ipc_proc_dfx_open(struct inode *inode, struct file *filp)
{
    struct proc_dfx_private_data *data = kzalloc(sizeof(struct proc_dfx_private_data), GFP_KERNEL);
    if (data == NULL) {
        TSDRV_PRINT_ERR("data kzalloc failed\n");
        return -ENODEV;
    }

    data->dfx_id = (u32)(uintptr_t)PDE_DATA(inode);
    u32 devid = tsdrv_dfx_id_to_devid(data->dfx_id);
    u32 tsid = tsdrv_dfx_id_to_tsid(data->dfx_id);
    u32 fid = tsdrv_dfx_id_to_fid(data->dfx_id);

    TSDRV_PRINT_INFO("devid(%u) fid(%u) tsid(%u)\n", devid, fid, tsid);
    data->finish_flag = 0;
    filp->private_data = (void *)(uintptr_t)data;
    return 0;
}

STATIC ssize_t tsdrv_ipc_proc_dfx_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos)
{
    (void)ppos;
    struct proc_dfx_private_data *data = (struct proc_dfx_private_data *)file->private_data;
    u32 dfx_id = data->dfx_id;
    u32 devid = tsdrv_dfx_id_to_devid(dfx_id);
    u32 fid = tsdrv_dfx_id_to_fid(dfx_id);
    struct tsdrv_ipc_chan *ipc_chan = NULL;
    char buf[TSDRV_DFX_IPC_LEN] = {0};
    int buf_size = TSDRV_DFX_IPC_LEN;
    int len;
    char *str = buf;
    size_t output_len = 0;
    u32 idx;

    if (data->finish_flag) {
        TSDRV_PRINT_INFO("read finish. return 0\n");
        return 0;
    }
    data->finish_flag = 1;

    DRV_CHECK_EXP_ACT(devid >= TSDRV_MAX_DAVINCI_NUM, return 0, "Device id is invalid. (devid=%u)\n", devid);
    DRV_CHECK_EXP_ACT(fid != TSDRV_PM_FID, return 0, "Only support physical. (fid=%u)\n", fid);

    len = snprintf_s(str, buf_size, buf_size - 1, "devid     tsid     tx     rx\n");
    DRV_CHECK_EXP_ACT(len < 0, return 0, "Snprintf fail. (len=%d)\n", len);
    str += len;
    buf_size -= len;
    output_len += len;
    DRV_CHECK_EXP_ACT(buf_size <= 0, return 0, "Buffer are not enough.\n");

    ipc_chan = tsdrv_get_ipc_chan_table(devid);
    for (idx = 0; idx < HISI_RPROC_MAX; idx++) {
        if (ipc_chan[idx].nb_name == NULL) {
            break;
        }

        len = snprintf_s(str, buf_size, buf_size - 1, "%5u%9u%7u%7u\n",
            ipc_chan[idx].devid, ipc_chan[idx].tsid,
            ipc_chan[idx].tx_rproc_id, ipc_chan[idx].rx_rproc_id);
        DRV_CHECK_EXP_ACT(len < 0, return 0, "Snprintf failed. (len=%d)\n", len);
        str += len;
        buf_size -= len;
        output_len += len;
        DRV_CHECK_EXP_ACT(buf_size <= 0, return 0, "Buffer are not enough.\n");
    }

    output_len += len;
    return simple_read_from_buffer_safe(user_buf, (const void *)buf, output_len);
}
STATIC int tsdrv_ipc_proc_dfx_release(struct inode *inode, struct file *filep)
{
    struct proc_dfx_private_data *data = (struct proc_dfx_private_data *)filep->private_data;
    TSDRV_PRINT_INFO("release");
    if (data == NULL) {
        TSDRV_PRINT_ERR("data is NULL.\n");
        return -ENOMEM;
    } else {
        kfree(data);
        filep->private_data = NULL;
        return 0;
    }
}
static const struct file_operations ipc_dfx_ops = {
    .open = tsdrv_ipc_proc_dfx_open,
    .read = tsdrv_ipc_proc_dfx_read,
    .release = tsdrv_ipc_proc_dfx_release,
};

static int tsdrv_ipc_dfx_create(u32 devid, u32 fid, struct proc_dir_entry *parent)
{
    char name[TSDRV_DFX_NAME_LEN] = {0};
    struct proc_dir_entry *event_id = NULL;
    u32 dfx_id;
    int ret;

    ret = snprintf_s(name, TSDRV_DFX_NAME_LEN, TSDRV_DFX_NAME_LEN - 1, "ipc_info");
    if (ret < 0) {
        return -EFAULT;
    }

    dfx_id = tsdrv_pack_dfx_id(devid, fid, 0);
    event_id = proc_create_data(name, TS_DRV_ATTR_RD, parent, &ipc_dfx_ops, (void *)(uintptr_t)dfx_id);
    if (event_id == NULL) {
        return -ENOMEM;
    }

    return 0;
}

int tsdrv_ipc_dfx_file_create(u32 devid, u32 fid)
{
    struct tsdrv_proc_dfx_fid *dfx_fid = tsdrv_get_fid_dfx(devid, fid);
    struct proc_dir_entry *ipc_dir = NULL;
    int ret;

    if (dfx_fid->fid_dentry == NULL) {
        TSDRV_PRINT_ERR("Fid entry is null. (devid=%u; fid=%u)\n", devid, fid);
        return -ENODEV;
    }

    ipc_dir = proc_mkdir("ipc", dfx_fid->fid_dentry);
    if (ipc_dir == NULL) {
        TSDRV_PRINT_ERR("Create ipc dfx dir failed. (devid=%u; fid=%u)\n", devid, fid);
        return -EFAULT;
    }

    ret = tsdrv_ipc_dfx_create(devid, fid, ipc_dir);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Create ipc dfx failed. (ret=%d)\n", ret);
        return ret;
    }

    return 0;
}
#endif
#endif
