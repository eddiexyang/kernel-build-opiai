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

#include <linux/stat.h>
#ifndef AOS_LLVM_BUILD
#include <linux/debugfs.h>
#endif
#include <linux/slab.h>
#include <linux/atomic.h>

#include "securec.h"
#include "tsdrv_ctx_dfx.h"
#include "tsdrv_dev_dfx.h"
#include "tsdrv_device.h"
#include "tsdrv_ctx.h"

#define CTX_DFX_BUF_LEN        (10 * 1024)
#define CTX_DFX_BUF_MIN_LEN    30
#define CTX_DFX_MAX_OPEN_NUM 128

STATIC const char *ctx_status_str[TSDRV_CTX_STATUS_MAX] = {
    [TSDRV_CTX_INVALID] = "INVALID",
    [TSDRV_CTX_VALID] = "VALID",
    [TSDRV_CTX_RUN]  = "RUN",
    [TSDRV_CTX_WAIT] = "WAIT",
    [TSDRV_CTX_RELEASE] = "RELEASE",
    [TSDRV_CTX_RECYCLE] = "RECYCLE",
    [TSDRV_CTX_RECYCLE_ERR] = "RECYCLE_ERR",
    [TSDRV_CTX_STOP_RECYCLE] = "STOP_RECYCLE",
};

STATIC atomic_t g_ctx_dfx_open_cnt = ATOMIC_INIT(0);

struct ctx_dfx_private_data {
    u32 dfx_id;
    char *buff;
#ifdef AOS_LLVM_BUILD
    u32 finish_flag;
#endif
};

STATIC int tsdrv_ctx_dfx_open(struct inode *inode, struct file *filp)
{
    struct ctx_dfx_private_data *data = NULL;

    if (atomic_inc_return(&g_ctx_dfx_open_cnt) > CTX_DFX_MAX_OPEN_NUM) {
        atomic_dec(&g_ctx_dfx_open_cnt);
        TSDRV_PRINT_ERR("over max ctx dfx open num=%d\n", CTX_DFX_MAX_OPEN_NUM);
        return -ENODEV;
    }
#ifndef TSDRV_UT
    data = kzalloc(sizeof(struct ctx_dfx_private_data), GFP_KERNEL | __GFP_ACCOUNT);
    if (data == NULL) {
        TSDRV_PRINT_ERR("data kzalloc failed\n");
        atomic_dec(&g_ctx_dfx_open_cnt);
        return -ENODEV;
    }
#ifndef AOS_LLVM_BUILD
    data->dfx_id = (u32)(uintptr_t)inode->i_private;
#else
    data->dfx_id =  (u32)(uintptr_t)pde_data(inode);
    data->finish_flag = 0;
#endif
    data->buff = kzalloc(CTX_DFX_BUF_LEN, GFP_KERNEL | __GFP_ACCOUNT);
    if (data->buff == NULL) {
        TSDRV_PRINT_ERR("data->buff kzalloc failed\n");
        kfree(data);
        atomic_dec(&g_ctx_dfx_open_cnt);
        return -ENODEV;
    }
    filp->private_data = (void *)data;
#endif
    return 0;
}

STATIC ssize_t tsdrv_ctx_dfx_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos)
{
#ifndef TSDRV_UT
    u32 tsid;
    u32 fid;
    u32 devid;
    struct tsdrv_device *tsdrv_dev = NULL;
    int buf_size = CTX_DFX_BUF_LEN;
    struct tsdrv_ctx *ctx = NULL;
    enum tsdrv_ctx_status status;
    char *str = NULL;
    size_t output_len = 0;
    u32 ctx_num, idx, i;
    pid_t tgid, pid;
    int len;
    struct ctx_dfx_private_data *data = (struct ctx_dfx_private_data *)file->private_data;
    if (data == NULL || data->buff == NULL) {
        TSDRV_PRINT_ERR("Input para is invalid.\n");
        return 0;
    }

    tsid = (data->dfx_id >> (TSDRV_DFX_DEV_OFFSET + TSDRV_DFX_FID_OFFSET)) & TSDRV_DFX_TS_MASK;
    fid = (data->dfx_id >> TSDRV_DFX_DEV_OFFSET) & TSDRV_DFX_FID_MASK;
    devid = data->dfx_id & TSDRV_DFX_DEV_MASK;
    str = data->buff;

#ifdef AOS_LLVM_BUILD
    if (data->finish_flag) {
        TSDRV_PRINT_INFO("read finish. return 0\n");
        return 0;
    }
    data->finish_flag = 1;
#endif
    if ((devid >= TSDRV_MAX_DAVINCI_NUM) || (tsid >= DEVDRV_MAX_TS_NUM) || (fid >= TSDRV_MAX_FID_NUM)) {
        return 0;
    }
    ctx_num = tsdrv_dev_get_ctx_num(devid, fid);
    len = snprintf_s(str, buf_size, buf_size - 1, "total:%3u        use:%3u         free:%3u \n\n",
        TSDRV_MAX_CTX_NUM, ctx_num, TSDRV_MAX_CTX_NUM - ctx_num);
    DRV_CHECK_EXP_ACT(len < 0, return 0, "snprintf_s fail, len(%d)\n", len);
    str += len;
    buf_size -= len;
    output_len += len;
    DRV_CHECK_EXP_ACT(buf_size < CTX_DFX_BUF_MIN_LEN, return 0, "buf_size(%d) err,line:%d\n", buf_size, __LINE__);

    len = snprintf_s(str, buf_size, buf_size - 1, "idx            tgid            pid            status\n");
    DRV_CHECK_EXP_ACT(len < 0, return 0, "snprintf_s fail, len(%d)\n", len);
    str += len;
    buf_size -= len;
    output_len += len;
    DRV_CHECK_EXP_ACT(buf_size < CTX_DFX_BUF_MIN_LEN, return 0, "buf_size(%d) err,line:%d\n", buf_size, __LINE__);

    tsdrv_dev = tsdrv_get_dev(devid);
    ctx = tsdrv_dev->dev_ctx[fid].ctx;

    for (i = 0; i < TSDRV_MAX_CTX_NUM; i++) {
        spin_lock_bh(&ctx->ctx_lock);
        status = atomic_read(&ctx[i].status);
        idx = ctx[i].ctx_index;
        tgid = ctx[i].tgid;
        pid = ctx[i].pid;
        spin_unlock_bh(&ctx->ctx_lock);
        if ((status == TSDRV_CTX_INVALID) || (status >= TSDRV_CTX_STATUS_MAX)) {
            continue;
        }
        len = snprintf_s(str, buf_size, buf_size - 1, "%5u%16d%15d%18s\n", idx, tgid, pid, ctx_status_str[status]);
        DRV_CHECK_EXP_ACT(len < 0, return 0, "snprintf_s fail, len(%d)\n", len);
        str += len;
        buf_size -= len;
        output_len += len;
        DRV_CHECK_EXP_ACT(buf_size < CTX_DFX_BUF_MIN_LEN, return 0, "buf_size(%d) err,line:%d\n", buf_size, __LINE__);
    }
#ifndef AOS_LLVM_BUILD
    return simple_read_from_buffer_safe(user_buf, count, ppos, data->buff, output_len);
#else
    return simple_read_from_buffer_safe(user_buf, data->buff, output_len);
#endif
#endif
}

STATIC int tsdrv_ctx_dfx_release(struct inode *inode, struct file *filep)
{
    struct ctx_dfx_private_data *data = (struct ctx_dfx_private_data *)filep->private_data;

    if (data == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("data is NULL.\n");
        return -ENOMEM;
#endif
    } else {
        if (data->buff != NULL) {
            kfree(data->buff);
        }
        kfree(data);
        atomic_dec(&g_ctx_dfx_open_cnt);
        filep->private_data = NULL;
        return 0;
    }
}
#ifndef AOS_LLVM_BUILD
STATIC const struct file_operations g_ctx_dbgfs_ops = {
    .open = tsdrv_ctx_dfx_open,
    .read = tsdrv_ctx_dfx_read,
    .release = tsdrv_ctx_dfx_release,
};

int tsdrv_ctx_dfx_file_create(u32 devid, u32 fid)
{
    struct tsdrv_dfx_fid *dfx_fid = tsdrv_get_fid_dfx(devid, fid);
    struct dentry *ts_id_dentry[DEVDRV_MAX_TS_NUM] = {NULL};
    u32 dfx_id = (fid << TSDRV_DFX_DEV_OFFSET) | devid;
    u32 tsnum = tsdrv_get_dev_tsnum(devid);
    char name[TSDRV_DFX_NAME_LEN] = {0};
    struct dentry *ctx_dir = NULL;
    u32 tsid, i;
    int err;

    if (dfx_fid->fid_dentry == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("fid entry is NULL, devid(%u) fid(%u)\n", devid, fid);
        return -ENODEV;
#endif
    }

    ctx_dir = debugfs_create_dir("ctx_status", dfx_fid->fid_dentry);
    if (ctx_dir == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("ctx_status dir create fail, devid=%u fid=%u\n", devid, fid);
        return -EFAULT;
#endif
    }

    for (tsid = 0; tsid < tsnum; tsid++) {
        err = snprintf_s(name, TSDRV_DFX_NAME_LEN, TSDRV_DFX_NAME_LEN - 1, "ctx_ts%u", tsid);
        if (err < 0) {
#ifndef TSDRV_UT
            TSDRV_PRINT_ERR("copy dfx name fail, err(%d)\n", err);
            goto ERR;
#endif
        }

        dfx_id = (tsid << (TSDRV_DFX_DEV_OFFSET + TSDRV_DFX_FID_OFFSET)) | (fid << TSDRV_DFX_DEV_OFFSET) | devid;
        ts_id_dentry[tsid] = debugfs_create_file(name, S_IRUGO, ctx_dir, (void *)(uintptr_t)dfx_id, &g_ctx_dbgfs_ops);
        if (ts_id_dentry[tsid] == NULL) {
            TSDRV_PRINT_ERR("create status fail, devid(%u) fid(%u)", devid, fid);
            goto ERR;
        }
    }
    return 0;
ERR:
    for (i = 0; i < tsid; i++) {
        if (ts_id_dentry[i] != NULL) {
            debugfs_remove_recursive(ts_id_dentry[i]);
        }
    }

    return -ENODEV;
}
#else
ssize_t tsdrv_ctx_proc_dfx_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    return tsdrv_ctx_dfx_read(file, buf, count, NULL);
}

STATIC const struct file_operations g_ctx_proc_ops = {
    .open = tsdrv_ctx_dfx_open,
    .read = tsdrv_ctx_proc_dfx_read,
    .release = tsdrv_ctx_dfx_release,
};

int tsdrv_ctx_dfx_file_create(u32 devid, u32 fid)
{
    struct tsdrv_proc_dfx_fid *dfx_fid = tsdrv_get_fid_dfx(devid, fid);
    struct proc_dir_entry *ts_id_dentry[DEVDRV_MAX_TS_NUM] = {NULL};
    u32 dfx_id = (fid << TSDRV_DFX_DEV_OFFSET) | devid;
    u32 tsnum = tsdrv_get_dev_tsnum(devid);
    char name[TSDRV_DFX_NAME_LEN] = {0};
    struct proc_dir_entry *ctx_dir = NULL;
    u32 tsid, i;
    int err;

    if (dfx_fid->fid_dentry == NULL) {
        TSDRV_PRINT_ERR("fid entry is NULL, devid(%u) fid(%u)\n", devid, fid);
        return -ENODEV;
    }

    ctx_dir = proc_mkdir("ctx_status", dfx_fid->fid_dentry);
    if (ctx_dir == NULL) {
        TSDRV_PRINT_ERR("ctx_status dir create fail, devid=%u fid=%u\n", devid, fid);
        return -EFAULT;
    }

    for (tsid = 0; tsid < tsnum; tsid++) {
        err = snprintf_s(name, TSDRV_DFX_NAME_LEN, TSDRV_DFX_NAME_LEN - 1, "ctx_ts%u", tsid);
        if (err < 0) {
            TSDRV_PRINT_ERR("copy dfx name fail, err(%d)\n", err);
            goto ERR;
        }
        dfx_id = (tsid << (TSDRV_DFX_DEV_OFFSET + TSDRV_DFX_FID_OFFSET)) | (fid << TSDRV_DFX_DEV_OFFSET) | devid;
        ts_id_dentry[tsid] = proc_create_data(name, TS_DRV_ATTR_RD, ctx_dir, &g_ctx_proc_ops,
                                              (void *)(uintptr_t)dfx_id);
        if (ts_id_dentry[tsid] == NULL) {
            TSDRV_PRINT_ERR("create status fail, devid(%u) fid(%u)", devid, fid);
            goto ERR;
        }
    }
    return 0;
ERR:
    for (i = 0; i < tsid; i++) {
        if (ts_id_dentry[i] != NULL) {
            proc_remove(ts_id_dentry[i]);
        }
    }
    proc_remove(ctx_dir);
    return -ENODEV;
}
#endif
