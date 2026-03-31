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

#include <linux/stat.h>
#ifndef AOS_LLVM_BUILD
#include <linux/debugfs.h>
#endif
#include "securec.h"
#include "tsdrv_mailbox_dfx.h"
#include "tsdrv_dev_dfx.h"
#include "tsdrv_device.h"
#include "devdrv_mailbox.h"

#define TSDRV_DFX_MBOX_BUF_LEN        512U
#define TSDRV_DFX_MBOX_BUF_MIN_LEN    30

struct tsdrv_mailbox_dfx mbox_dfx_record[DEVDRV_MAX_DAVINCI_NUM][DEVDRV_MAX_TS_NUM];

struct tsdrv_mailbox_dfx *tsdrv_get_mbox_dfx_addr(u32 devid, u32 tsid)
{
    return &mbox_dfx_record[devid][tsid];
}
#ifndef AOS_LLVM_BUILD
STATIC int tsdrv_dfx_mbox_open(struct inode *inode, struct file *filp)
{
    u32 dfx_id = (u32)(uintptr_t)inode->i_private;
    u32 devid = dfx_id & TSDRV_DFX_DEV_MASK;
    u32 tsid = (dfx_id >> (TSDRV_DFX_DEV_OFFSET + TSDRV_DFX_FID_OFFSET)) & TSDRV_DFX_TS_MASK;
    u32 fid = (dfx_id >> TSDRV_DFX_DEV_OFFSET) & TSDRV_DFX_FID_MASK;

    TSDRV_PRINT_INFO("devid(%u) fid(%u) tsid(%u)\n", devid, fid, tsid);

    filp->private_data = (void *)(uintptr_t)dfx_id;

    return 0;
}

STATIC ssize_t tsdrv_dfx_mbox_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos)
{
    u32 dfx_id = (u32)(uintptr_t)file->private_data;
    u32 tsid = (dfx_id >> (TSDRV_DFX_DEV_OFFSET + TSDRV_DFX_FID_OFFSET)) & TSDRV_DFX_TS_MASK;
    u32 fid = (dfx_id >> TSDRV_DFX_DEV_OFFSET) & TSDRV_DFX_FID_MASK;
    struct tsdrv_mailbox_dfx *mbox_dfx = NULL;
    u32 devid = dfx_id & TSDRV_DFX_DEV_MASK;
    char buf[TSDRV_DFX_MBOX_BUF_LEN] = {0};
    s32 buf_size = TSDRV_DFX_MBOX_BUF_LEN;
    size_t output_len = 0;
    char *str = buf;
    int len, i, tmp;

    TSDRV_PRINT_INFO("devid(%u) fid(%u) tsid(%u)\n", devid, fid, tsid);

    if ((devid >= DEVDRV_MAX_DAVINCI_NUM) || (tsid >= DEVDRV_MAX_TS_NUM)) {
        return 0;
    }

    mbox_dfx = tsdrv_get_mbox_dfx_addr(devid, tsid);
    if (fid == 0) {
        len = snprintf_s(str, buf_size, buf_size - 1, "mailbox_dfx: total send(%lu),rec(%lu),lost(%lu),"
            "irqnum(%lu),upnum(%lu),free(%u)\n",
            mbox_dfx->send_num, mbox_dfx->receive_num, mbox_dfx->lost_num,
            mbox_dfx->irq_num, mbox_dfx->up_num, mbox_dfx->free_num);
        DRV_CHECK_EXP_ACT(len < 0, return 0, "snprintf_s fail, len(%d)\n", len);
        str += len;
        buf_size -= len;
        output_len += len;
        DRV_CHECK_EXP_ACT(buf_size < TSDRV_DFX_MBOX_BUF_MIN_LEN, return 0, "buf_size err,line:%d, buf_size=(%d)\n",
            __LINE__, buf_size);

        tmp = (mbox_dfx->free_num <= MAILBOX_DFX_COUNT) ? mbox_dfx->free_num : MAILBOX_DFX_COUNT;
        for (i = 0; i < tmp; i++) {
#ifndef TSDRV_UT
            len = snprintf_s(str, buf_size, buf_size - 1, "index:%d [%ld s %ld us],free_type(%u),free_valid(%u)\n",
                i, mbox_dfx->free_time[i].tv_sec, mbox_dfx->free_time[i].tv_usec,
                mbox_dfx->free_type[i], mbox_dfx->free_valid[i]);
#endif
            DRV_CHECK_EXP_ACT(len < 0, return 0, "snprintf_s fail, len(%d)\n", len);
            str += len;
            buf_size -= len;
            output_len += len;
            DRV_CHECK_EXP_ACT(buf_size < TSDRV_DFX_MBOX_BUF_MIN_LEN, return 0, "buf_size err,line:%d, buf_size=(%d)\n",
                __LINE__, buf_size);
        }
    }
    return simple_read_from_buffer_safe(user_buf, count, ppos, buf, output_len);
}


STATIC const struct file_operations g_mailbox_dfx_ops = {
    .open = tsdrv_dfx_mbox_open,
    .read = tsdrv_dfx_mbox_read,
};

int tsdrv_mailbox_dfx_file_create(u32 devid, u32 fid)
{
    struct tsdrv_dfx_fid *dfx_fid = tsdrv_get_fid_dfx(devid, fid);
    struct dentry *ts_id_dentry[DEVDRV_MAX_TS_NUM] = {NULL};
    u32 dfx_id = (fid << TSDRV_DFX_DEV_OFFSET) | devid;
    u32 tsnum = tsdrv_get_dev_tsnum(devid);
    char name[TSDRV_DFX_NAME_LEN] = {0};
    struct dentry *mbox_dir = NULL;
    u32 tsid, i;
    int err;

    if (dfx_fid->fid_dentry == NULL) {
        TSDRV_PRINT_ERR("fid entry is NULL, devid(%u) fid(%u)\n", devid, fid);
        return -ENODEV;
    }

    mbox_dir = debugfs_create_dir("mailbox", dfx_fid->fid_dentry);
    if (mbox_dir == NULL) {
        TSDRV_PRINT_ERR("mailbox dir create fail, devid=%u fid=%u\n", devid, fid);
        return -EFAULT;
    }

    for (tsid = 0; tsid < tsnum; tsid++) {
        err = snprintf_s(name, TSDRV_DFX_NAME_LEN, TSDRV_DFX_NAME_LEN - 1, "mailbox_ts%u", tsid);
        if (err < 0) {
            TSDRV_PRINT_ERR("copy dfx mbox name fail, err(%d)\n", err);
            goto ERR;
        }

        dfx_id = (tsid << (TSDRV_DFX_DEV_OFFSET + TSDRV_DFX_FID_OFFSET)) | (fid << TSDRV_DFX_DEV_OFFSET) | devid;
        ts_id_dentry[tsid] = debugfs_create_file(name, S_IRUGO, mbox_dir, (void *)(uintptr_t)dfx_id,
            &g_mailbox_dfx_ops);
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
STATIC int mailbox_proc_dfx_open(struct inode *inode, struct file *filp)
{
    struct proc_dfx_private_data *data = kzalloc(sizeof(struct proc_dfx_private_data), GFP_KERNEL | __GFP_ACCOUNT);
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

ssize_t mailbox_proc_dfx_read(struct file *file, char __user *user_buf, size_t count, loff_t *ppos)
{
    (void)ppos;
    struct proc_dfx_private_data *data = (struct proc_dfx_private_data *)file->private_data;
    u32 dfx_id = data->dfx_id;
    u32 tsid = (dfx_id >> (TSDRV_DFX_DEV_OFFSET + TSDRV_DFX_FID_OFFSET)) & TSDRV_DFX_TS_MASK;
    u32 fid = (dfx_id >> TSDRV_DFX_DEV_OFFSET) & TSDRV_DFX_FID_MASK;
    struct tsdrv_mailbox_dfx *mbox_dfx = NULL;
    u32 devid = dfx_id & TSDRV_DFX_DEV_MASK;
    char buf[TSDRV_DFX_MBOX_BUF_LEN] = {0};
    s32 buf_size = TSDRV_DFX_MBOX_BUF_LEN;
    size_t output_len = 0;
    char *str = buf;
    int len, i, tmp;

    if (data->finish_flag) {
        TSDRV_PRINT_INFO("read finish. return 0\n");
        return 0;
    }
    data->finish_flag = 1;

    TSDRV_PRINT_INFO("devid(%u) fid(%u) tsid(%u)\n", devid, fid, tsid);

    if ((devid >= DEVDRV_MAX_DAVINCI_NUM) || (tsid >= DEVDRV_MAX_TS_NUM)) {
        return 0;
    }

    mbox_dfx = tsdrv_get_mbox_dfx_addr(devid, tsid);
    if (fid == 0) {
        len = snprintf_s(str, buf_size, buf_size - 1, "mailbox_dfx: total send(%lu),rec(%lu),lost(%lu),"
            "irqnum(%lu),upnum(%lu),free(%u)\n",
            mbox_dfx->send_num, mbox_dfx->receive_num, mbox_dfx->lost_num,
            mbox_dfx->irq_num, mbox_dfx->up_num, mbox_dfx->free_num);
        DRV_CHECK_EXP_ACT(len < 0, return 0, "snprintf_s fail, len(%d)\n", len);
        str += len;
        buf_size -= len;
        output_len += len;
        DRV_CHECK_EXP_ACT(buf_size < TSDRV_DFX_MBOX_BUF_MIN_LEN, return 0, "buf_size err,line:%d, buf_size=(%d)\n",
            __LINE__, buf_size);

        tmp = (mbox_dfx->free_num <= MAILBOX_DFX_COUNT) ? mbox_dfx->free_num : MAILBOX_DFX_COUNT;
        for (i = 0; i < tmp; i++) {
#ifndef TSDRV_UT
            len = snprintf_s(str, buf_size, buf_size - 1, "index:%d [%ld s %ld us],free_type(%u),free_valid(%u)\n",
                i, mbox_dfx->free_time[i].tv_sec, mbox_dfx->free_time[i].tv_usec,
                mbox_dfx->free_type[i], mbox_dfx->free_valid[i]);
#endif
            DRV_CHECK_EXP_ACT(len < 0, return 0, "snprintf_s fail, len(%d)\n", len);
            str += len;
            buf_size -= len;
            output_len += len;
            DRV_CHECK_EXP_ACT(buf_size < TSDRV_DFX_MBOX_BUF_MIN_LEN, return 0, "buf_size err,line:%d, buf_size=(%d)\n",
                __LINE__, buf_size);
        }
    }

    return simple_read_from_buffer_safe(user_buf, buf, output_len);
}

STATIC int mailbox_proc_dfx_release(struct inode *inode, struct file *filep)
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

STATIC const struct file_operations g_mailbox_dfx_proc_ops = {
    .open = mailbox_proc_dfx_open,
    .read = mailbox_proc_dfx_read,
    .release = mailbox_proc_dfx_release,
};

int tsdrv_mailbox_dfx_file_create(u32 devid, u32 fid)
{
    struct tsdrv_proc_dfx_fid *dfx_fid = tsdrv_get_fid_dfx(devid, fid);
    struct proc_dir_entry *ts_id_dentry[DEVDRV_MAX_TS_NUM] = {NULL};
    u32 tsnum = tsdrv_get_dev_tsnum(devid);
    u32 dfx_id = (fid << TSDRV_DFX_DEV_OFFSET) | devid;
    char name[TSDRV_DFX_NAME_LEN] = {0};
    struct proc_dir_entry *mbox_dir = NULL;
    u32 tsid, i;
    int err;

    if (dfx_fid->fid_dentry == NULL) {
        TSDRV_PRINT_ERR("fid entry is NULL, devid(%u) fid(%u)\n", devid, fid);
        return -ENODEV;
    }

    mbox_dir = proc_mkdir("mailbox", dfx_fid->fid_dentry);
    if (mbox_dir == NULL) {
        TSDRV_PRINT_ERR("mailbox dir create fail, devid=%u fid=%u\n", devid, fid);
        return -EFAULT;
    }

    for (tsid = 0; tsid < tsnum; tsid++) {
        err = snprintf_s(name, TSDRV_DFX_NAME_LEN, TSDRV_DFX_NAME_LEN - 1, "mailbox_ts%u", tsid);
        if (err < 0) {
            TSDRV_PRINT_ERR("copy dfx mbox name fail, err(%d)\n", err);
            goto ERR;
        }
        dfx_id = (tsid << (TSDRV_DFX_DEV_OFFSET + TSDRV_DFX_FID_OFFSET)) | (fid << TSDRV_DFX_DEV_OFFSET) | devid;
        ts_id_dentry[tsid] = proc_create_data(name, TS_DRV_ATTR_RD, mbox_dir, &g_mailbox_dfx_proc_ops,
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
    proc_remove(mbox_dir);
    return -ENODEV;
}

#endif
