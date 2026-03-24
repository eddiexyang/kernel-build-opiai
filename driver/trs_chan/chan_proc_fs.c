/*
* Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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
* Create: 2022-10-15
*/

#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/version.h>

#ifdef CFG_BUILD_DEBUG
#include <linux/uaccess.h>
#include <linux/cred.h>
#include "uda.h"
#endif

#include "securec.h"
#include "chan_rxtx.h"

#define PROC_FS_NAME_LEN 32
#define PROC_FS_MODE 0400
#define BUFF_LEN 512

static struct proc_dir_entry *chan_top_entry = NULL;

#ifdef CFG_BUILD_DEBUG
static int chan_trace_show(struct seq_file *seq, void *offset)
{
    struct trs_chan_ts_inst *ts_inst = (struct trs_chan_ts_inst *)(uintptr_t)seq->private;

    seq_printf(seq, "%d\n", ts_inst->trace_enable);
    return 0;
}

ssize_t chan_trace_ops_write(struct file *filp, const char __user *ubuf, size_t count, loff_t *ppos)
{
    struct inode *inode = file_inode(filp);
    struct trs_chan_ts_inst *ts_inst = (struct trs_chan_ts_inst *)(uintptr_t)PDE_DATA(inode);
    char ch[2] = {0}; /* 2 bytes long */
    long val;

    if ((ppos == NULL) || (*ppos != 0) || (count != sizeof(ch)) || (ubuf == NULL)) {
        return -EINVAL;
    }

    if (!uda_can_access_udevid(ts_inst->inst.devid)) {
        return -EACCES;
    }

    if (copy_from_user(ch, ubuf, count)) {
        return -ENOMEM;
    }

    ch[count - 1] = '\0';
    if (kstrtol(ch, 10, &val)) {
        return -EFAULT;
    }
    ts_inst->trace_enable = (val == 0) ? false : true;

    return (ssize_t)count;
}

int chan_trace_ops_open(struct inode *inode, struct file *file)
{
    return single_open(file, chan_trace_show, PDE_DATA(inode));
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 5, 0)
static const struct file_operations chan_trace_ops = {
    .owner = THIS_MODULE,
    .open = chan_trace_ops_open,
    .read = seq_read,
    .write = chan_trace_ops_write,
    .llseek  = seq_lseek,
    .release = single_release,
};
#else
static const struct proc_ops chan_trace_ops = {
    .proc_open    = chan_trace_ops_open,
    .proc_read    = seq_read,
    .proc_write   = chan_trace_ops_write,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};
#endif
#endif

static int chan_sum_show(struct seq_file *seq, void *offset)
{
    struct trs_chan *chan = (struct trs_chan *)seq->private;
    char buff[BUFF_LEN];

    seq_printf(seq, "id %d ref %u flag 0x%x irq %u type %u subtype %u\n",
        chan->id, kref_read(&chan->ref), chan->flag, chan->irq, chan->types.type, chan->types.sub_type);

    if (_trs_chan_to_string(chan, buff, BUFF_LEN) > 0) {
        seq_printf(seq, "%s\n", buff);
    }
    return 0;
}

int chan_sum_ops_open(struct inode *inode, struct file *file)
{
    return single_open(file, chan_sum_show, PDE_DATA(inode));
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 5, 0)
static const struct file_operations chan_sum_ops = {
    .owner = THIS_MODULE,
    .open    = chan_sum_ops_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};
#else
static const struct proc_ops chan_sum_ops = {
    .proc_open    = chan_sum_ops_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};
#endif

static void proc_fs_format_ts_inst_dir_name(struct trs_chan_ts_inst *ts_inst, char *name, int len)
{
    if (sprintf_s(name, len, "dev%u-ts%u", ts_inst->inst.devid, ts_inst->inst.tsid) <= 0) {
        trs_warn("Sprintf_s failed. (devid=%u; tsid=%u)\n", ts_inst->inst.devid, ts_inst->inst.tsid);
    }
}

static struct proc_dir_entry *proc_fs_mk_ts_inst_dir(struct trs_chan_ts_inst *ts_inst, struct proc_dir_entry *parent)
{
    char name[PROC_FS_NAME_LEN];

    proc_fs_format_ts_inst_dir_name(ts_inst, name, PROC_FS_NAME_LEN);
    return proc_mkdir((const char *)name, parent);
}

static void proc_fs_rm_ts_inst_dir(struct trs_chan_ts_inst *ts_inst, struct proc_dir_entry *parent)
{
    char name[PROC_FS_NAME_LEN];

    proc_fs_format_ts_inst_dir_name(ts_inst, name, PROC_FS_NAME_LEN);
    (void)remove_proc_subtree((const char *)name, parent);
}

void chan_proc_fs_add_ts_inst(struct trs_chan_ts_inst *ts_inst)
{
    ts_inst->entry = proc_fs_mk_ts_inst_dir(ts_inst, chan_top_entry);
    if (ts_inst->entry == NULL) {
        trs_warn("Create entry failed. (devid=%u; tsid=%u)\n", ts_inst->inst.devid, ts_inst->inst.tsid);
        return;
    }

#ifdef CFG_BUILD_DEBUG
    (void)proc_create_data("trace_enable", PROC_FS_MODE, ts_inst->entry, &chan_trace_ops, ts_inst);
#endif
}

void chan_proc_fs_del_ts_inst(struct trs_chan_ts_inst *ts_inst)
{
    proc_fs_rm_ts_inst_dir(ts_inst, chan_top_entry);
}

static void proc_fs_format_chan_file_name(struct trs_chan *chan, char *name, int len)
{
    if (sprintf_s(name, len, "chan%d-%s", chan->id, trs_chan_type_to_name(&chan->types)) <= 0) {
        trs_warn("Sprintf_s failed. (id=%d)\n", chan->id);
    }
}

void chan_proc_fs_add_chan(struct trs_chan *chan)
{
    char name[PROC_FS_NAME_LEN];

    proc_fs_format_chan_file_name(chan, name, PROC_FS_NAME_LEN);
    chan->entry = proc_create_data(name, PROC_FS_MODE, chan->ts_inst->entry, &chan_sum_ops, chan);
}

void chan_proc_fs_del_chan(struct trs_chan *chan)
{
    proc_remove(chan->entry);
}

void chan_proc_fs_init(void)
{
    chan_top_entry = proc_mkdir("trs_chan", NULL);
    if (chan_top_entry == NULL) {
        trs_err("create top entry dir failed\n");
    }
}

void chan_proc_fs_uninit(void)
{
    (void)remove_proc_subtree("trs_chan", NULL);
}

