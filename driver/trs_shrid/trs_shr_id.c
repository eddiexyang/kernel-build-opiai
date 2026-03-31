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
 * Create: 2022-07-28
 */
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/hashtable.h>
#include <linux/delay.h>

#include "ascend_hal_define.h"
#include "securec.h"
#include "trs_core.h"

#include "trs_shr_id_node.h"
#include "trs_shr_id.h"

static DEFINE_RWLOCK(proc_lock);

#define PROC_HASH_TABLE_BIT     4
#define PROC_HASH_TABLE_MASK    ((1 << PROC_HASH_TABLE_BIT) - 1)
static DECLARE_HASHTABLE(proc_htable, PROC_HASH_TABLE_BIT);

struct shr_id_proc_node {
    char name[SHR_ID_NSM_NAME_SIZE];
    int res_type;
    int type;
    struct list_head list;
    struct trs_id_inst inst;
    int id;
    u32 ref;
    u32 flag; /* remote id */
    u32 opened_devid;
};

static int shr_id_type_trans[SHR_ID_TYPE_MAX] = {
    [SHR_ID_NOTIFY_TYPE] = TRS_NOTIFY,
    [SHR_ID_EVENT_TYPE] = TRS_EVENT,
};

static void shr_id_proc_num_inc(struct shr_id_proc_ctx *proc_ctx, int op, u32 num)
{
    (op == 0) ? (proc_ctx->create_node_num += num) : (proc_ctx->open_node_num += num);
}

static void shr_id_proc_num_dec(struct shr_id_proc_ctx *proc_ctx, int op, u32 num)
{
    (op == 0) ? (proc_ctx->create_node_num -= num) : (proc_ctx->open_node_num -= num);
}

static void _shr_id_proc_add(struct shr_id_proc_ctx *proc_ctx)
{
    int key = proc_ctx->pid & PROC_HASH_TABLE_MASK;

    hash_add(proc_htable, &proc_ctx->link, key);
}

static void _shr_id_proc_del(struct shr_id_proc_ctx *proc_ctx)
{
    write_lock(&proc_lock);
    hash_del(&proc_ctx->link);
    write_unlock(&proc_lock);
}

struct shr_id_proc_ctx *shr_id_proc_create(pid_t pid)
{
    size_t ctx_size = sizeof(struct shr_id_proc_ctx);
    struct shr_id_proc_ctx *proc_ctx = NULL;

    proc_ctx = (struct shr_id_proc_ctx *)kzalloc(ctx_size, GFP_KERNEL | __GFP_ACCOUNT);
    if (proc_ctx == NULL) {
        trs_err("Kmalloc fail. (size=%ld)\n", ctx_size);
        return NULL;
    }

    proc_ctx->pid = current->tgid;
    proc_ctx->start_time = current->group_leader->start_time;
    kref_init(&proc_ctx->ref);
    INIT_LIST_HEAD(&proc_ctx->create_list_head);
    INIT_LIST_HEAD(&proc_ctx->open_list_head);
    rwlock_init(&proc_ctx->lock);
    mutex_init(&proc_ctx->mutex);
    return proc_ctx;
}

void shr_id_proc_destroy(struct shr_id_proc_ctx *proc_ctx)
{
    kfree(proc_ctx);
}

static struct shr_id_proc_ctx *shr_id_find_proc(pid_t pid)
{
    struct shr_id_proc_ctx *proc_ctx = NULL;
    int key = pid & PROC_HASH_TABLE_MASK;

    hash_for_each_possible(proc_htable, proc_ctx, link, key) {
        if (proc_ctx->pid == pid) {
            return proc_ctx;
        }
    }
    return NULL;
}

static struct shr_id_proc_node *shr_id_creat_proc_node(const char *name, struct shr_id_node_op_attr *attr)
{
    struct shr_id_proc_node *proc_node = NULL;
    int i;

    proc_node = kvzalloc(sizeof(struct shr_id_proc_node), GFP_KERNEL | __GFP_ACCOUNT);
    if (proc_node == NULL) {
        return NULL;
    }

    proc_node->res_type = attr->res_type;
    proc_node->inst = attr->inst;
    proc_node->id = attr->id;
    proc_node->type = attr->type;
    for (i = 0; i < SHR_ID_NSM_NAME_SIZE; i++) {
        proc_node->name[i] = name[i];
    }
    proc_node->ref = 1;

    return proc_node;
}

static void shr_id_destory_proc_node(struct shr_id_proc_node *proc_node)
{
    if (proc_node != NULL) {
        kvfree(proc_node);
    }
}

static void shr_id_recycle_opened(struct shr_id_proc_ctx *proc_ctx)
{
    struct shr_id_proc_node *proc_node = NULL, *n = NULL;
    int ret = 0;
    u32 i;

    trs_debug("Opened start recycle. (pid=%d; open_num=%u)\n", proc_ctx->pid, proc_ctx->open_node_num);

    list_for_each_entry_safe(proc_node, n, &proc_ctx->open_list_head, list) {
        for (i = 0; i < proc_node->ref; i++) {
            ret |= shr_id_node_close(proc_node->name, proc_node->type, proc_ctx->pid);

            if (need_resched()) {
                cond_resched();
            }
        }

        if (ret == 0) {
            shr_id_proc_num_dec(proc_ctx, 1, proc_node->ref);
        } else {
            trs_warn("Opened recycle failed. (name=%s; type=%d; ret=%d)\n", proc_node->name, proc_node->type, ret);
        }
        list_del(&proc_node->list);
        shr_id_destory_proc_node(proc_node);
        trs_debug("Opened recycle. (pid=%d; open_num=%u)\n", proc_ctx->pid, proc_ctx->open_node_num);
    }
}

static void shr_id_recycle_created(struct shr_id_proc_ctx *proc_ctx)
{
    struct shr_id_proc_node *proc_node = NULL, *n = NULL;

    trs_debug("Create start recycle. (pid=%d; create_num=%u)\n", proc_ctx->pid, proc_ctx->create_node_num);

    list_for_each_entry_safe(proc_node, n, &proc_ctx->create_list_head, list) {
        int ret = shr_id_node_destroy(proc_node->name, proc_node->type, proc_ctx->pid);
        if (ret == 0) {
            shr_id_proc_num_dec(proc_ctx, 0, proc_node->ref);
        } else {
            trs_warn("Create recycle failed. (name=%s; type=%d; ret=%d)\n", proc_node->name, proc_node->type, ret);
        }
        list_del(&proc_node->list);
        shr_id_destory_proc_node(proc_node);
    }
}

static void shr_id_node_recycle(struct shr_id_proc_ctx *proc_ctx)
{
    shr_id_recycle_opened(proc_ctx);
    shr_id_recycle_created(proc_ctx);
}

static void shr_id_proc_release(struct kref *kref)
{
    struct shr_id_proc_ctx *proc_ctx = container_of(kref, struct shr_id_proc_ctx, ref);

    /*
     * After the recovery is complete, delete the process context from the hash table
     * This sequence cooperates with shr_id_wait_for_proc_exit to ensure that
     * only one process with the same pid exists at the same time
     */
    shr_id_node_recycle(proc_ctx);
    _shr_id_proc_del(proc_ctx);
    shr_id_proc_destroy(proc_ctx);
}

static struct shr_id_proc_ctx *shr_id_proc_get(pid_t pid)
{
    struct shr_id_proc_ctx *proc_ctx = NULL;

    read_lock(&proc_lock);
    proc_ctx = shr_id_find_proc(pid);
    if (proc_ctx != NULL) {
        if (kref_get_unless_zero(&proc_ctx->ref) == 0) {
            read_unlock(&proc_lock);
            return NULL;
        }
    }
    read_unlock(&proc_lock);
    return proc_ctx;
}

static void shr_id_proc_put(struct shr_id_proc_ctx *proc_ctx)
{
    kref_put(&proc_ctx->ref, shr_id_proc_release);
}

static int shr_id_name_generate(struct trs_id_inst *inst, int pid, int id_type, u32 shr_id, char *name)
{
    int offset = snprintf_s(name, SHR_ID_NSM_NAME_SIZE, SHR_ID_NSM_NAME_SIZE - 1, "%08x%08x%08x%08x%08x",
        pid, inst->devid, inst->tsid, id_type, shr_id);
    if (offset < 0) {
        trs_err("Snprintf failed. (offset=%d)\n", offset);
        return -EINVAL;
    }
    name[offset] = '\0';

    return 0;
}

static int shr_id_get_type_by_name(const char *name, int *id_type)
{
    u32 tgid, devid, tsid;
    size_t name_len;
    int ret;

    name_len = strnlen(name, SHR_ID_NSM_NAME_SIZE);
    if ((name_len == 0) || (name_len >= SHR_ID_NSM_NAME_SIZE)) {
        trs_err("Length out of range. (name_len=%lu)\n", name_len);
        return -EINVAL;
    }

    ret = sscanf_s(name, "%08x %08x %08x %08x", &tgid, &devid, &tsid, id_type);
    if (ret != 4) { /* 4 for parse nums */
        trs_err("sscanf failed. (ret=%d)\n", ret);
        return -EINVAL;
    }

    if ((*id_type >= SHR_ID_TYPE_MAX) || (*id_type < 0)) {
        trs_err("Para invalid. (id_type=%u)\n", *id_type);
        return -EINVAL;
    }
    return 0;
}

static struct list_head *shr_id_get_list_head(struct shr_id_proc_ctx *proc_ctx, int op)
{
    return (op == 0) ? &proc_ctx->create_list_head : &proc_ctx->open_list_head;
}

static struct shr_id_proc_node *shr_id_find_proc_node(struct shr_id_proc_ctx *proc_ctx, const char *name, int op)
{
    struct shr_id_proc_node *proc_node = NULL, *n = NULL;
    struct list_head *head;

    head = shr_id_get_list_head(proc_ctx, op);
    read_lock(&proc_ctx->lock);
    list_for_each_entry_safe(proc_node, n, head, list) {
        if (strcmp(proc_node->name, name) == 0) {
            read_unlock(&proc_ctx->lock);
            return proc_node;
        }
    }
    read_unlock(&proc_ctx->lock);

    return NULL;
}

static int shr_id_add_to_proc(struct shr_id_proc_ctx *proc_ctx, struct shr_id_ioctl_info *ioctl_info,
    struct shr_id_node_op_attr *attr, int op)
{
    struct shr_id_proc_node *proc_node = NULL;

    mutex_lock(&proc_ctx->mutex);
    proc_node = shr_id_find_proc_node(proc_ctx, ioctl_info->name, op);
    if (proc_node != NULL) {
        if ((ioctl_info->flag & TSDRV_FLAG_REMOTE_ID) != 0) {
            ioctl_info->flag &= ~TSDRV_FLAG_REMOTE_ID;
        }
        proc_node->ref++;
        shr_id_proc_num_inc(proc_ctx, op, 1);
        mutex_unlock(&proc_ctx->mutex);
        return 0;
    }

    proc_node = shr_id_creat_proc_node(ioctl_info->name, attr);
    if (proc_node == NULL) {
        mutex_unlock(&proc_ctx->mutex);
        return -ENOMEM;
    }

    if ((ioctl_info->flag & TSDRV_FLAG_REMOTE_ID) != 0) {
        proc_node->opened_devid = ioctl_info->opened_devid;
        proc_node->flag |= TSDRV_FLAG_REMOTE_ID;
        ioctl_info->flag |= TSDRV_FLAG_REMOTE_ID;
    }

    trs_debug("Add info. (devid=%u; tsid=%u; type=%d; id=%u; flag=%u; node_flag=%u; op=%d)\n",
        attr->inst.devid, attr->inst.tsid, attr->res_type, attr->id, ioctl_info->flag, proc_node->flag, op);

    write_lock(&proc_ctx->lock);
    list_add_tail(&proc_node->list, shr_id_get_list_head(proc_ctx, op));
    write_unlock(&proc_ctx->lock);
    shr_id_proc_num_inc(proc_ctx, op, 1);
    mutex_unlock(&proc_ctx->mutex);

    return 0;
}

static void shr_id_del_from_proc(struct shr_id_proc_ctx *proc_ctx,
    struct shr_id_ioctl_info *ioctl_info, int op)
{
    struct shr_id_proc_node *proc_node = NULL;

    mutex_lock(&proc_ctx->mutex);
    proc_node = shr_id_find_proc_node(proc_ctx, ioctl_info->name, op);
    if (proc_node == NULL) {
        mutex_unlock(&proc_ctx->mutex);
        return;
    }

    trs_debug("Del info. (devid=%u; tsid=%u; type=%d; id=%u; flag=%u; node_flag=%u; op=%d)\n",
        proc_node->inst.devid, proc_node->inst.devid, proc_node->type, proc_node->id,
        ioctl_info->flag, proc_node->flag, op);

    shr_id_proc_num_dec(proc_ctx, op, 1);
    proc_node->ref--;
    if (proc_node->ref == 0) {
        if ((proc_node->flag & TSDRV_FLAG_REMOTE_ID) != 0) {
            ioctl_info->opened_devid = proc_node->opened_devid;
            ioctl_info->flag |= TSDRV_FLAG_REMOTE_ID;
            ioctl_info->devid = proc_node->inst.devid;
            ioctl_info->tsid = proc_node->inst.tsid;
#ifndef EMU_ST
            ioctl_info->id_type = (u32)proc_node->type;
            ioctl_info->shr_id = (u32)proc_node->id;
#endif
        }
        /* shr id close don't need check shr id. */
        write_lock(&proc_ctx->lock);
        list_del(&proc_node->list);
        write_unlock(&proc_ctx->lock);
        shr_id_destory_proc_node(proc_node);
    }
    mutex_unlock(&proc_ctx->mutex);
}

int shr_id_wait_for_proc_exit(pid_t pid)
{
    int loop = 0;

    do {
        /*
         * Fisrt 100 times, sleep 1 ms, then sleep 1000 ms
         * The waiting time must be greater than or equal to the recycle time
         */
        unsigned long timeout = (loop < 100) ? 1 : 1000;
        struct shr_id_proc_ctx *proc_ctx = NULL;

        read_lock(&proc_lock);
        proc_ctx = shr_id_find_proc(pid);
        read_unlock(&proc_lock);
        if (proc_ctx == NULL) {
            return 0;
        }
        if (timeout <= 1) {
            usleep_range(timeout * USEC_PER_MSEC, timeout * USEC_PER_MSEC + 100); /* range 100 us */
        } else {
            if (msleep_interruptible(timeout) != 0) {
                return -EINTR;
            }
        }
    } while (++loop < 400); /* retry 400 times, 300 s */

    return -EEXIST;
}

int shr_id_proc_add(struct shr_id_proc_ctx *proc_ctx)
{
    write_lock(&proc_lock);
    if (shr_id_find_proc(proc_ctx->pid) != NULL) {
        write_unlock(&proc_lock);
        trs_err("Proc add fail. (pid=%d)\n", proc_ctx->pid);
        return -EINVAL;
    }
    _shr_id_proc_add(proc_ctx);
    write_unlock(&proc_lock);
    return 0;
}

void shr_id_proc_del(struct shr_id_proc_ctx *proc_ctx)
{
    shr_id_proc_put(proc_ctx);
}

bool shr_id_is_belong_to_proc(struct trs_id_inst *inst, int pid, int res_type, u32 res_id)
{
    struct shr_id_proc_node *proc_node = NULL, *n = NULL;
    struct shr_id_proc_ctx *proc_ctx = NULL;
    bool is_belong = false;

    proc_ctx = shr_id_proc_get(pid);
    if (proc_ctx == NULL) {
        return false;
    }

    read_lock(&proc_ctx->lock);
    list_for_each_entry_safe(proc_node, n, &proc_ctx->open_list_head, list) {
        if ((proc_node->inst.devid == inst->devid) && (proc_node->inst.tsid == inst->tsid) &&
            (proc_node->res_type == res_type) && (proc_node->id == res_id)) {
            is_belong = true;
            break;
        }
    }

    /* for mc2 feature, shr id alloced by device cp, but host app need notify wait,
     * so host app need call halShridCreate.
     */
#ifndef EMU_ST
    list_for_each_entry_safe(proc_node, n, &proc_ctx->create_list_head, list) {
        if ((proc_node->inst.devid == inst->devid) && (proc_node->inst.tsid == inst->tsid) &&
            (proc_node->res_type == res_type) && (proc_node->id == res_id)) {
            is_belong = true;
            break;
        }
    }
#endif

    read_unlock(&proc_ctx->lock);
    shr_id_proc_put(proc_ctx);

    trs_debug("Id info. (devid=%u; tsid=%u; pid=%d; type=%d; shrid=%u; is=%u)\n",
        inst->devid, inst->tsid, current->tgid, res_type, res_id, is_belong);

    return is_belong;
}

/* 后续显式包含对应头文件 */
int devdrv_manager_container_check_devid_in_container_ns(u32 devid, struct task_struct *tsk);
int shr_id_create(struct shr_id_proc_ctx *proc_ctx, struct shr_id_ioctl_info *ioctl_info)
{
    struct shr_id_node_op_attr attr;
    u32 type = ioctl_info->id_type;
    int ret;

    ret = devdrv_manager_container_check_devid_in_container_ns(ioctl_info->devid, current);
    if (ret != 0) {
        trs_err("Device is not in container. (devid=%u)\n", ioctl_info->devid);
        return -EINVAL;
    }

    if (type >= SHR_ID_TYPE_MAX) {
        trs_err("Para invalid. (devid=%u; tsid=%u; idtype=%u)\n",
            ioctl_info->devid, ioctl_info->tsid, type);
        return -EINVAL;
    }
    attr.res_type = shr_id_type_trans[type];
    attr.type = type;
    attr.id = ioctl_info->shr_id;
    trs_id_inst_pack(&attr.inst, ioctl_info->devid, ioctl_info->tsid);
    ret = shr_id_name_generate(&attr.inst, proc_ctx->pid, type, ioctl_info->shr_id, ioctl_info->name);
    if (ret != 0) {
        return ret;
    }
    ret = shr_id_node_create(ioctl_info->name, proc_ctx->pid, &attr, ioctl_info->flag);
    if (ret != 0) {
        return ret;
    }
    ret = shr_id_add_to_proc(proc_ctx, ioctl_info, &attr, 0);
    if (ret != 0) {
        /* res_id will be put when shr_id_node destroyed */
        (void)shr_id_node_destroy(ioctl_info->name, type, proc_ctx->pid);
    }
    return ret;
}

int shr_id_destroy(struct shr_id_proc_ctx *proc_ctx, struct shr_id_ioctl_info *ioctl_info)
{
    int ret, type;

    ret = shr_id_get_type_by_name(ioctl_info->name, &type);
    if (ret != 0) {
        return ret;
    }

    ret = shr_id_node_destroy(ioctl_info->name, type, proc_ctx->pid);
    if (ret != 0) {
        trs_err("Destroy shr id node fail. (type=%d; ret=%d)\n", type, ret);
        return ret;
    }
    shr_id_del_from_proc(proc_ctx, ioctl_info, 0);

    return 0;
}

/* open func passed to the user the physical id */
int shr_id_open(struct shr_id_proc_ctx *proc_ctx, struct shr_id_ioctl_info *ioctl_info)
{
    struct shr_id_node_op_attr attr;
    int ret, type;

    ret = shr_id_get_type_by_name(ioctl_info->name, &type);
    if (ret != 0) {
        return ret;
    }

    attr.res_type = shr_id_type_trans[type];
    attr.type = type;
    ret = shr_id_node_open(ioctl_info->name, proc_ctx->pid, proc_ctx->start_time, &attr);
    if (ret != 0) {
        return ret;
    }

    ret = shr_id_add_to_proc(proc_ctx, ioctl_info, &attr, 1);
    if (ret != 0) {
        (void)shr_id_node_close(ioctl_info->name, type, proc_ctx->pid);
        return ret;
    }

    ioctl_info->devid = attr.inst.devid;
    ioctl_info->tsid = attr.inst.tsid;
    ioctl_info->id_type = (u32)attr.type;
    ioctl_info->shr_id = (u32)attr.id;

    return 0;
}

int shr_id_close(struct shr_id_proc_ctx *proc_ctx, struct shr_id_ioctl_info *ioctl_info)
{
    int ret, type;

    ret = shr_id_get_type_by_name(ioctl_info->name, &type);
    if (ret != 0) {
        return ret;
    }

    ret = shr_id_node_close(ioctl_info->name, type, proc_ctx->pid);
    if (ret != 0) {
        return ret;
    }
    shr_id_del_from_proc(proc_ctx, ioctl_info, 1);
    return 0;
}

int shr_id_set_pid(struct shr_id_proc_ctx *proc_ctx, struct shr_id_ioctl_info *ioctl_info)
{
    int ret, type;

    ret = shr_id_get_type_by_name(ioctl_info->name, &type);
    if (ret != 0) {
        return ret;
    }
    return shr_id_node_set_pids(ioctl_info->name, type, proc_ctx->pid, ioctl_info->pid, SHR_ID_PID_MAX_NUM);
}

