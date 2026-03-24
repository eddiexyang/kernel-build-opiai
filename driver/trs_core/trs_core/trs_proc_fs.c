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
* Create: 2022-7-15
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
#include "trs_chan.h"
#include "trs_id.h"
#include "trs_ts_inst.h"
#include "trs_proc.h"
#include "trs_timestamp.h"

#define PROC_FS_NAME_LEN 32
#define PROC_FS_MODE 0400
#define BUFF_LEN 512

static struct proc_dir_entry *top_entry = NULL;
static struct proc_dir_entry *ts_inst_entry = NULL;
static struct proc_dir_entry *proc_entry = NULL;

static char *res_name[TRS_CORE_MAX_ID_TYPE] = {
    "stream", "event", "notify", "model", "cmo", "hw_sq", "hw_cq", "sw_sq", "sw_cq", "cb_sq", "cb_cq", "logic_cq"};

#ifdef CFG_BUILD_DEBUG
static int proc_trace_show(struct seq_file *seq, void *offset)
{
    struct trs_core_ts_inst *ts_inst = (struct trs_core_ts_inst *)(uintptr_t)seq->private;

    seq_printf(seq, "%d\n", ts_inst->trace_enable);
    return 0;
}

int proc_trace_ops_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_trace_show, PDE_DATA(inode));
}

ssize_t proc_trace_ops_write(struct file *filp, const char __user *ubuf, size_t count, loff_t *ppos)
{
    struct inode *inode = file_inode(filp);
    struct trs_core_ts_inst *ts_inst = (struct trs_core_ts_inst *)(uintptr_t)PDE_DATA(inode);
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


#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 5, 0)
static const struct file_operations proc_trace_ops = {
    .owner = THIS_MODULE,
    .open = proc_trace_ops_open,
    .read = seq_read,
    .write = proc_trace_ops_write,
    .llseek  = seq_lseek,
    .release = single_release,
};
#else
static const struct proc_ops proc_trace_ops = {
    .proc_open    = proc_trace_ops_open,
    .proc_read    = seq_read,
    .proc_write   = proc_trace_ops_write,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};
#endif
#endif

static void trs_proc_show(struct trs_proc_ctx *proc_ctx, struct seq_file *seq)
{
    seq_printf(seq, "pid %d(%s) task_id %u devid %d status %d(0:normal, 1:exit) ssid %u\n",
        proc_ctx->pid, proc_ctx->name, proc_ctx->task_id, proc_ctx->devid, proc_ctx->status, proc_ctx->cp_ssid);
}

static void ts_inst_proc_show(struct trs_core_ts_inst *ts_inst, struct seq_file *seq)
{
    struct trs_proc_ctx *proc_ctx = NULL;

    down_read(&ts_inst->sem);
    seq_printf(seq, "\nproc list:\n");
    list_for_each_entry(proc_ctx, &ts_inst->proc_list_head, node) {
        trs_proc_show(proc_ctx, seq);
    }

    seq_printf(seq, "\nexit proc list:\n");
    list_for_each_entry(proc_ctx, &ts_inst->exit_proc_list_head, node) {
        trs_proc_show(proc_ctx, seq);
    }
    up_read(&ts_inst->sem);
}

static void ts_inst_res_show(struct trs_core_ts_inst *ts_inst, struct seq_file *seq)
{
    int i;

    seq_printf(seq, "\nres list:\n");

    for (i = 0; i < TRS_CORE_MAX_ID_TYPE; i++) {
        struct trs_res_mng *res_mng = &ts_inst->res_mng[i];
        seq_printf(seq, "idx %d res %s id_num %u max_id %u use_num %u\n",
            i, res_name[i], res_mng->id_num, res_mng->max_id, res_mng->use_num);
    }

    seq_printf(seq, "\nid_allocator res list:\n");
    for (i = 0; i < TRS_ID_TYPE_MAX; i++) {
        struct trs_id_stat stat;
        int ret;

        ret = trs_id_get_stat(&ts_inst->inst, i, &stat);
        if (ret == 0) {
            seq_printf(seq, "idx %d res %s use_num %u allocatable %u\n",
                i, trs_id_type_to_name(i), stat.alloc, stat.allocatable);
        }
    }

    seq_printf(seq, "\nid_pool res list:\n");
    for (i = 0; i < TRS_ID_TYPE_MAX; i++) {
        u32 avail_num;
        int ret;

        ret = trs_id_get_avail_num_in_pool(&ts_inst->inst, i, &avail_num);
        if (ret == 0) {
            seq_printf(seq, "idx %d res %s avail_num %u\n", i, trs_id_type_to_name(i), avail_num);
        }
    }
}

static int ts_inst_info_show(struct seq_file *seq, void *offset)
{
    struct trs_core_ts_inst *ts_inst = (struct trs_core_ts_inst *)seq->private;

    seq_printf(seq, "devid %d tsid %d hw_type %d(0:tscpu, 1:stars) surport_proc_num %u(0:no_limit) ref %u\n",
        ts_inst->inst.devid, ts_inst->inst.tsid, ts_inst->hw_type, ts_inst->surport_proc_num, kref_read(&ts_inst->ref));
    if (ts_inst->inst.tsid == 0) {
        ts_inst_proc_show(ts_inst, seq);
    }
    ts_inst_res_show(ts_inst, seq);

    return 0;
}

int ts_inst_sum_open(struct inode *inode, struct file *file)
{
    return single_open(file, ts_inst_info_show, PDE_DATA(inode));
}

static void trs_chan_show(struct trs_core_ts_inst *ts_inst, struct seq_file *seq, int chan_id)
{
    char buff[BUFF_LEN];
    if (trs_chan_to_string(&ts_inst->inst, chan_id, buff, BUFF_LEN) > 0) {
        seq_printf(seq, "%s\n", buff);
    }
}

static void trs_id_show(struct trs_core_ts_inst *ts_inst, struct seq_file *seq, int type, u32 id)
{
    char buff[BUFF_LEN];
    if (trs_id_to_string(&ts_inst->inst, type, id, buff, BUFF_LEN) > 0) {
        seq_printf(seq, "    %s\n", buff);
    }
}

static void ts_inst_cb_phy_sqcq_show(struct trs_core_ts_inst *ts_inst, struct seq_file *seq)
{
    struct trs_cb_ctx *cb_ctx = &ts_inst->cb_ctx;
    if (cb_ctx->phy_sqcq.chan_id >= 0) {
        seq_printf(seq, "cb cq_num %u phy sqid %u cqid %u chan %d\n",
            cb_ctx->cq_num, cb_ctx->phy_sqcq.sqid, cb_ctx->phy_sqcq.cqid, cb_ctx->phy_sqcq.chan_id);
        trs_id_show(ts_inst, seq, TRS_SW_SQ_ID, cb_ctx->phy_sqcq.sqid);
        trs_id_show(ts_inst, seq, TRS_SW_CQ_ID, cb_ctx->phy_sqcq.cqid);
        trs_chan_show(ts_inst, seq, cb_ctx->phy_sqcq.chan_id);
    }
}

static void ts_inst_logic_phy_sqcq_show(struct trs_core_ts_inst *ts_inst, struct seq_file *seq)
{
    struct trs_logic_cq_ctx *logic_cq_ctx = &ts_inst->logic_cq_ctx;
    if (logic_cq_ctx->phy_cq.chan_id >= 0) {
        seq_printf(seq, "logic cq_num %u phy cqid %u chan %d\n",
            logic_cq_ctx->cq_num, logic_cq_ctx->phy_cq.cqid, logic_cq_ctx->phy_cq.chan_id);
        trs_id_show(ts_inst, seq, TRS_SW_CQ_ID, logic_cq_ctx->phy_cq.cqid);
        trs_chan_show(ts_inst, seq, logic_cq_ctx->phy_cq.chan_id);
    }
}

static void res_stream_detail_show(struct trs_core_ts_inst *ts_inst, u32 id, struct seq_file *seq)
{
    struct trs_stream_ctx *stream_ctx = &ts_inst->stream_ctx[id];
    seq_printf(seq, "    host_pid %d sq %d cq %d logic_cq %d\n",
        stream_ctx->host_pid, stream_ctx->sq, stream_ctx->cq, stream_ctx->logic_cq);
}

static int trs_hw_sq_to_string(struct trs_core_ts_inst *ts_inst, u32 id, char *buff, u32 buff_len)
{
    struct trs_sq_ctx *sq_ctx = &ts_inst->sq_ctx[id];
    int len = sprintf_s(buff, buff_len, "    mode %s type %u flag %u cqid %u stream_id %u chan %d cp_proc %s\n",
        (sq_ctx->mode == 0) ? "kio" : ((sq_ctx->mode == 1) ? "uio_t" : "uio_d"), sq_ctx->type, sq_ctx->flag,
        sq_ctx->cqid, sq_ctx->stream_id, sq_ctx->chan_id, (sq_ctx->reg_mem.cp_proc_state == 0) ? "exist" : "not exist");

    spin_lock_bh(&sq_ctx->shr_info_lock);
    if ((len > 0) && (sq_ctx->mode == 1) && (sq_ctx->shr_info.kva != NULL)) {
        struct trs_sq_shr_info *shr_info = (struct trs_sq_shr_info *)sq_ctx->shr_info.kva;
        len += sprintf_s(buff + len, buff_len - len, "    uio send ok %llu send full %llu send fail %llu\n",
            shr_info->send_ok, shr_info->send_full, sq_ctx->send_fail);
    }
    spin_unlock_bh(&sq_ctx->shr_info_lock);

    if (len > 0) {
        len += trs_chan_to_string(&ts_inst->inst, sq_ctx->chan_id, buff + len, buff_len - len);
    }

    return len;
}

void trs_hw_sq_show(struct trs_core_ts_inst *ts_inst, u32 sqid)
{
    char buff[BUFF_LEN];
    if (trs_hw_sq_to_string(ts_inst, sqid, buff, BUFF_LEN) > 0) {
        trs_debug("%s", buff);
    }
}

static void res_hw_sq_detail_show(struct trs_core_ts_inst *ts_inst, u32 id, struct seq_file *seq)
{
    char buff[BUFF_LEN];
    if (trs_hw_sq_to_string(ts_inst, id, buff, BUFF_LEN) > 0) {
        seq_printf(seq, "%s\n", buff);
    }
}

static int trs_hw_cq_stat_to_string(struct trs_cq_ctx *cq_ctx, char *buff, u32 buff_len)
{
    int len = 0;

    len = sprintf_s(buff, buff_len, "    chan %d, logic_cqid %d\n", cq_ctx->chan_id,  cq_ctx->logic_cqid);
    len += sprintf_s(buff + len, buff_len - len, "    stat: rx %llu rx_enque %llu drop %llu logic_enque_failed %llu\n",
        cq_ctx->stat.rx, cq_ctx->stat.rx_enque, cq_ctx->stat.rx_drop, cq_ctx->stat.rx_enque_fail);
    return len;
}

void trs_hw_cq_show(struct trs_core_ts_inst *ts_inst, u32 cqid)
{
    struct trs_cq_ctx *cq_ctx = &ts_inst->cq_ctx[cqid];
    struct trs_id_inst *inst = &ts_inst->inst;
    char buff[BUFF_LEN];

    trs_info("Hw cq stat show. (devid=%u; tsid=%u; id=%u)\n", inst->devid, inst->tsid, cqid);
    if (trs_hw_cq_stat_to_string(cq_ctx, buff, BUFF_LEN) > 0) {
        trs_info("%s", buff);
    }
}

static void res_hw_cq_detail_show(struct trs_core_ts_inst *ts_inst, u32 id, struct seq_file *seq)
{
    struct trs_cq_ctx *cq_ctx = &ts_inst->cq_ctx[id];
    char buff[BUFF_LEN];

    if (trs_hw_cq_stat_to_string(cq_ctx, buff, BUFF_LEN) > 0) {
        seq_printf(seq, "%s", buff);
    }

    if (!trs_is_stars_inst(ts_inst)) {
        trs_chan_show(ts_inst, seq, cq_ctx->chan_id);
    }
}

static void res_sw_sq_detail_show(struct trs_core_ts_inst *ts_inst, u32 id, struct seq_file *seq)
{
    struct trs_sq_ctx *sq_ctx = &ts_inst->sw_sq_ctx[id];
    seq_printf(seq, "    cqid %u chan %d\n", sq_ctx->cqid, sq_ctx->chan_id);
    trs_chan_show(ts_inst, seq, sq_ctx->chan_id);
}

static void res_sw_cq_detail_show(struct trs_core_ts_inst *ts_inst, u32 id, struct seq_file *seq)
{
    struct trs_cq_ctx *cq_ctx = &ts_inst->sw_cq_ctx[id];
    seq_printf(seq, "    chan %d logic_cqid %u\n", cq_ctx->chan_id, cq_ctx->logic_cqid);
    if (!trs_is_stars_inst(ts_inst)) {
        trs_chan_show(ts_inst, seq, cq_ctx->chan_id);
    }
}

static void res_cb_cq_detail_show(struct trs_core_ts_inst *ts_inst, u32 id, struct seq_file *seq)
{
    struct trs_cb_cq *cb_cq = &ts_inst->cb_ctx.cq[id];
    seq_printf(seq, "    valid %u pid %d cq_depth %u cqe_size %u grpid %u\n",
        cb_cq->valid, cb_cq->pid, cb_cq->cq_depth, cb_cq->cqe_size, cb_cq->grpid);
}

static int trs_logic_cq_to_string(struct trs_logic_cq *cq, char *buff, u32 buff_len)
{
    u64 timestamp = trs_get_s_timestamp();
    struct trs_logic_cqe *cqe;
    int len;

    len = sprintf_s(buff, buff_len,
        "    valid %u head %u tail %u cq_depth %u cqe_size %u thread_bind_irq %d wakeup_num %d wait_thread_num %d\n"
        "    enque %llu full_drop %llu wakeup %llu recv_in %llu recv %llu timeout %llu \n",
        cq->valid, cq->head, cq->tail, cq->cq_depth, cq->cqe_size, cq->thread_bind_irq,
        atomic_read(&cq->wakeup_num), atomic_read(&cq->wait_thread_num),
        cq->stat.enque, cq->stat.full_drop, cq->stat.wakeup, cq->stat.recv_in, cq->stat.recv, cq->stat.timeout);
    if ((len < 0) || (cq->head == cq->tail)) {
        return len;
    }

    cqe = (struct trs_logic_cqe *)(cq->addr + (unsigned long)cq->head * cq->cqe_size);
    return sprintf_s(buff + len, buff_len - len,
        "    stranded cqe: pos=%u match_flag=%u timestamp=%llu(s) enque_timestamp=%llu(s) enque_time=%llu(s)"
        " stream_id=%u task_id=%u sqeType=%u sqId=%u sqHead=%u\n",
        cq->head, cqe->match_flag, timestamp, cqe->enque_timestamp, timestamp - cqe->enque_timestamp,
        cqe->stream_id, cqe->task_id, cqe->sqeType, cqe->sqId, cqe->sqHead);
}

void trs_logic_cq_show(struct trs_core_ts_inst *ts_inst, u32 cqid)
{
    struct trs_logic_cq *cq = &ts_inst->logic_cq_ctx.cq[cqid];
    struct trs_id_inst *inst = &ts_inst->inst;
    char buff[BUFF_LEN];

    trs_info("Logic cq stat show. (devid=%u; tsid=%u; id=%u; flag=%u)\n",
        inst->devid, inst->tsid, cqid, cq->flag);
    if (trs_logic_cq_to_string(cq, buff, BUFF_LEN) > 0) {
        trs_info("%s", buff);
    }
}

static void res_logic_cq_detail_show(struct trs_core_ts_inst *ts_inst, u32 id, struct seq_file *seq)
{
    struct trs_logic_cq *cq = &ts_inst->logic_cq_ctx.cq[id];
    char buff[BUFF_LEN];
    if (trs_logic_cq_to_string(cq, buff, BUFF_LEN) > 0) {
        seq_printf(seq, "%s\n", buff);
    }
}

static void (*const res_detail_handles[TRS_CORE_MAX_ID_TYPE])(struct trs_core_ts_inst *ts_inst,
    u32 id, struct seq_file *seq) = {
    [TRS_STREAM] = res_stream_detail_show,
    [TRS_HW_SQ] = res_hw_sq_detail_show,
    [TRS_HW_CQ] = res_hw_cq_detail_show,
    [TRS_SW_SQ] = res_sw_sq_detail_show,
    [TRS_SW_CQ] = res_sw_cq_detail_show,
    [TRS_CB_CQ] = res_cb_cq_detail_show,
    [TRS_LOGIC_CQ] = res_logic_cq_detail_show
};

static int res_detail_show(struct trs_res_mng *res_mng, int pid, struct seq_file *seq)
{
    u32 i;

    for (i = 0; i < res_mng->max_id; i++) {
        struct trs_res_ids *id = &res_mng->ids[i];
        if ((id->ref != 0) && ((pid == id->pid) || (pid == 0))) {
            int res_type = trs_res_replace_res_type(res_mng->ts_inst, res_mng->res_type);
            seq_printf(seq, "id %u ref %d status %d pid %d\n", i, id->ref, id->status, id->pid);
            trs_id_show(res_mng->ts_inst, seq, trs_res_get_id_type(res_type), i);
            if (res_detail_handles[res_mng->res_type] != NULL) {
                res_detail_handles[res_mng->res_type](res_mng->ts_inst, i, seq);
            }
        }
    }

    return 0;
}

static int ts_inst_res_detail_show(struct seq_file *seq, void *offset)
{
    struct trs_res_mng *res_mng = (struct trs_res_mng *)seq->private;

    if (res_mng->res_type == TRS_CB_CQ) {
        ts_inst_cb_phy_sqcq_show(res_mng->ts_inst, seq);
    } else if (res_mng->res_type == TRS_LOGIC_CQ) {
        ts_inst_logic_phy_sqcq_show(res_mng->ts_inst, seq);
    } else {
        /* do nothing */
    }

    res_detail_show(res_mng, 0, seq);

    return 0;
}

int ts_inst_res_detail_open(struct inode *inode, struct file *file)
{
    return single_open(file, ts_inst_res_detail_show, PDE_DATA(inode));
}

static void proc_res_show(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, struct seq_file *seq)
{
    int i;

    for (i = 0; i < TRS_CORE_MAX_ID_TYPE; i++) {
        int res_num = trs_get_proc_res_num(proc_ctx, ts_inst->inst.tsid, i);
        if (res_num != 0) {
            seq_printf(seq, "tsid %u res %s res_num %u\n", ts_inst->inst.tsid, res_name[i], res_num);
            res_detail_show(&ts_inst->res_mng[i], proc_ctx->pid, seq);
        }
    }
}

static void proc_ts_shm_show(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, struct seq_file *seq)
{
    struct trs_shm_ctx *shm_ctx = &ts_inst->shm_ctx;
    u32 tsid = ts_inst->inst.tsid;

    if (shm_ctx->chan_id >= 0) {
        seq_printf(seq, "tsid %u shm sqid %u chan %d\n", tsid, shm_ctx->sqid, shm_ctx->chan_id);
        trs_chan_show(ts_inst, seq, shm_ctx->chan_id);
    }
}

static void proc_logic_cq_show(struct trs_proc_ctx *proc_ctx, u32 tsid, struct seq_file *seq)
{
    int thread_bind_irq_num = atomic_read(&proc_ctx->ts_ctx[tsid].thread_bind_irq_num);
    if (thread_bind_irq_num != 0) {
        seq_printf(seq, "tsid %u thread_bind_irq_num %d\n", tsid, thread_bind_irq_num);
    }
}

static int proc_sum_show(struct seq_file *seq, void *offset)
{
    struct trs_proc_ctx *proc_ctx = (struct trs_proc_ctx *)seq->private;
    u32 i;

    trs_proc_show(proc_ctx, seq);
    for (i = 0; i < TRS_TS_MAX_NUM; i++) {
        struct trs_core_ts_inst *ts_inst = trs_core_inst_get(proc_ctx->devid, i);
        if (ts_inst != NULL) {
            proc_res_show(proc_ctx, ts_inst, seq);
            proc_ts_shm_show(proc_ctx, ts_inst, seq);
            proc_logic_cq_show(proc_ctx, i, seq);
            trs_core_inst_put(ts_inst);
        }
    }

    return 0;
}

int proc_sum_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_sum_show, PDE_DATA(inode));
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 5, 0)
static const struct file_operations ts_inst_sum_ops = {
    .owner = THIS_MODULE,
    .open    = ts_inst_sum_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

static const struct file_operations ts_inst_res_detail_ops = {
    .owner = THIS_MODULE,
    .open    = ts_inst_res_detail_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};

static const struct file_operations proc_sum_ops = {
    .owner = THIS_MODULE,
    .open    = proc_sum_open,
    .read    = seq_read,
    .llseek  = seq_lseek,
    .release = single_release,
};
#else
static const struct proc_ops ts_inst_sum_ops = {
    .proc_open    = ts_inst_sum_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};

static const struct proc_ops ts_inst_res_detail_ops = {
    .proc_open    = ts_inst_res_detail_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};

static const struct proc_ops proc_sum_ops = {
    .proc_open    = proc_sum_open,
    .proc_read    = seq_read,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};
#endif

static void proc_fs_format_ts_inst_dir_name(struct trs_core_ts_inst *ts_inst, char *name, int len)
{
    if (sprintf_s(name, len, "dev%u-ts%u", ts_inst->inst.devid, ts_inst->inst.tsid) <= 0) {
        trs_warn("Sprintf_s failed. (devid=%u; tsid=%u)\n", ts_inst->inst.devid, ts_inst->inst.tsid);
    }
}

static struct proc_dir_entry *proc_fs_mk_ts_inst_dir(struct trs_core_ts_inst *ts_inst, struct proc_dir_entry *parent)
{
    char name[PROC_FS_NAME_LEN];

    proc_fs_format_ts_inst_dir_name(ts_inst, name, PROC_FS_NAME_LEN);
    return proc_mkdir((const char *)name, parent);
}

static void proc_fs_rm_ts_inst_dir(struct trs_core_ts_inst *ts_inst, struct proc_dir_entry *parent)
{
    char name[PROC_FS_NAME_LEN];

    proc_fs_format_ts_inst_dir_name(ts_inst, name, PROC_FS_NAME_LEN);
    (void)remove_proc_subtree((const char *)name, parent);
}

void proc_fs_add_ts_inst(struct trs_core_ts_inst *ts_inst)
{
    int i;

    ts_inst->entry = proc_fs_mk_ts_inst_dir(ts_inst, ts_inst_entry);
    if (ts_inst->entry == NULL) {
        trs_warn("Create entry failed. (devid=%u; tsid=%u)\n", ts_inst->inst.devid, ts_inst->inst.tsid);
        return;
    }

#ifdef CFG_BUILD_DEBUG
    (void)proc_create_data("trace_enable", PROC_FS_MODE, ts_inst->entry, &proc_trace_ops, ts_inst);
#endif
    (void)proc_create_data("summary", PROC_FS_MODE, ts_inst->entry, &ts_inst_sum_ops, ts_inst);

    for (i = 0; i < TRS_CORE_MAX_ID_TYPE; i++) {
        struct trs_res_mng *res_mng = &ts_inst->res_mng[i];
        (void)proc_create_data(res_name[i], PROC_FS_MODE, ts_inst->entry, &ts_inst_res_detail_ops, res_mng);
    }
}

void proc_fs_del_ts_inst(struct trs_core_ts_inst *ts_inst)
{
    proc_fs_rm_ts_inst_dir(ts_inst, ts_inst_entry);
}

static void proc_fs_format_pid_dir_name(struct trs_proc_ctx *proc_ctx, char *name, int len)
{
    if (sprintf_s(name, len, "%d-%d-dev-%d", proc_ctx->pid, proc_ctx->task_id, proc_ctx->devid) <= 0) {
        trs_warn("Sprintf_s failed. (pid=%d; task_id=%u)\n", proc_ctx->pid, proc_ctx->task_id);
    }
}

static struct proc_dir_entry *proc_fs_mk_pid_dir(struct trs_proc_ctx *proc_ctx, struct proc_dir_entry *parent)
{
    char name[PROC_FS_NAME_LEN];

    proc_fs_format_pid_dir_name(proc_ctx, name, PROC_FS_NAME_LEN);
    return proc_mkdir((const char *)name, parent);
}

static void proc_fs_rm_pid_dir(struct trs_proc_ctx *proc_ctx, struct proc_dir_entry *parent)
{
    char name[PROC_FS_NAME_LEN];

    proc_fs_format_pid_dir_name(proc_ctx, name, PROC_FS_NAME_LEN);
    (void)remove_proc_subtree((const char *)name, parent);
}

void proc_fs_add_pid(struct trs_proc_ctx *proc_ctx)
{
    proc_ctx->entry = proc_fs_mk_pid_dir(proc_ctx, proc_entry);
    if (proc_ctx->entry == NULL) {
        trs_warn("Create entry failed. (pid=%d)\n", proc_ctx->pid);
        return;
    }

    (void)proc_create_data("summary", PROC_FS_MODE, proc_ctx->entry, &proc_sum_ops, proc_ctx);
}

void proc_fs_del_pid(struct trs_proc_ctx *proc_ctx)
{
    proc_fs_rm_pid_dir(proc_ctx, proc_entry);
}

void trs_proc_fs_init(void)
{
    top_entry = proc_mkdir("trs_core", NULL);
    if (top_entry == NULL) {
        trs_err("create top entry dir failed\n");
        return;
    }

    ts_inst_entry = proc_mkdir("ts_inst", top_entry);
    if (ts_inst_entry == NULL) {
        trs_err("create ts_inst entry dir failed\n");
        return;
    }

    proc_entry = proc_mkdir("proc", top_entry);
    if (proc_entry == NULL) {
        trs_err("create proc entry dir failed\n");
        return;
    }

    return;
}

void trs_proc_fs_uninit(void)
{
    (void)remove_proc_subtree("trs_core", NULL);
}

