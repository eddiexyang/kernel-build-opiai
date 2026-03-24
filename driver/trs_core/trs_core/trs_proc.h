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

#ifndef TRS_PROC_H
#define TRS_PROC_H

#include <linux/types.h>
#include <linux/mutex.h>
#include <linux/interrupt.h>
#include <linux/proc_fs.h>

struct trs_core_ts_inst;

struct trs_proc_shm_ctx {
    struct mutex mutex;
    int chan_id;
    u32 cqid;
    u64 cq_pa;
    u32 cq_irq;
    unsigned long shm_sq_va;
};

struct trs_proc_ts_ctx {
    struct mutex mutex;
    atomic_t thread_bind_irq_num;
    atomic_t current_id_num[TRS_CORE_MAX_ID_TYPE];
    struct trs_proc_shm_ctx shm_ctx;
};

#define TASK_COMM_LEN 16

#define TRS_PROC_STATUS_NORMAL 0
#define TRS_PROC_STATUS_EXIT 1

struct trs_proc_ctx {
    struct list_head node;
    char name[TASK_COMM_LEN];
    u32 task_id;
    int pid;
    int status;
    int cp_ssid;
    u32 devid;
    struct workqueue_struct *work_queue;
    struct proc_dir_entry *entry;
    struct trs_proc_ts_ctx ts_ctx[TRS_TS_MAX_NUM];
};

static inline int trs_get_proc_res_num(struct trs_proc_ctx *proc_ctx, u32 tsid, int res_type)
{
    return atomic_read(&proc_ctx->ts_ctx[tsid].current_id_num[res_type]);
}

bool trs_proc_has_res(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, int res_type, u32 res_id);
bool trs_proc_has_res_with_pid(struct trs_core_ts_inst *ts_inst, int pid, int res_type, u32 res_id);
int trs_proc_add_res(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, int res_type, u32 res_id);
int trs_proc_del_res(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, int res_type, u32 res_id);
int trs_res_get(struct trs_core_ts_inst *ts_inst, int pid, int res_type, u32 res_id);
int trs_res_put(struct trs_core_ts_inst *ts_inst, int res_type, u32 res_id);
bool trs_is_proc_res_limited(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, int res_type);

struct trs_proc_ctx *trs_proc_ctx_find(struct trs_core_ts_inst *ts_inst, int pid);
int trs_proc_wait_for_exit(struct trs_core_ts_inst *ts_inst, int pid);
struct trs_proc_ctx *trs_proc_ctx_create(struct trs_core_ts_inst *ts_inst);
int trs_proc_release(struct trs_proc_ctx *proc_ctx);
int trs_proc_recycle(struct trs_proc_ctx *proc_ctx);
void trs_proc_ctx_destroy(struct trs_proc_ctx *proc_ctx);

#define TRS_MAP_TYPE_MEM 0 /* normal memory, kernal and user share */
#define TRS_MAP_TYPE_DEV_MEM 1 /* device prop memory, hardware and user share */
#define TRS_MAP_TYPE_RO_DEV_MEM 2 /* device prop memory, hardware and user share, user is readonly */
#define TRS_MAP_TYPE_REG 3 /* device register, hardware and user share */
#define TRS_MAP_TYPE_RO_REG 4 /* device register, hardware and user share, user is readonly */

struct trs_mem_map_para {
    int type;
    unsigned long va;
    unsigned long pa;
    unsigned long len;
};

struct trs_mem_unmap_para {
    int type;
    unsigned long va;
    unsigned long len;
};

static inline void trs_remap_fill_para(struct trs_mem_map_para *para,
    int type, unsigned long va, unsigned long pa, unsigned long len)
{
    para->type = type;
    para->va = va;
    para->pa = pa;
    para->len = len;
}

static inline void trs_unmap_fill_para(struct trs_mem_unmap_para *para, int type, unsigned long va, unsigned long len)
{
    para->type = type;
    para->va = va;
    para->len = len;
}

int trs_remap_sq(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, struct trs_mem_map_para *para);
int trs_unmap_sq(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, struct trs_mem_unmap_para *para);

#endif

