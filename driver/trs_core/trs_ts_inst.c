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

#include <linux/slab.h>
#include <linux/vmalloc.h>

#include "trs_proc_fs.h"
#include "trs_proc.h"
#include "trs_ts_inst.h"

struct mutex core_ts_inst_mutex;
static rwlock_t core_ts_inst_lock;

struct trs_core_ts_inst *core_ts_inst[TRS_TS_INST_MAX_NUM] = {NULL, };

static int trs_core_ts_inst_init(struct trs_core_ts_inst *ts_inst)
{
    struct trs_id_inst *inst = &ts_inst->inst;
    int ret;

    kref_init(&ts_inst->ref);
    init_rwsem(&ts_inst->sem);
    INIT_LIST_HEAD(&ts_inst->proc_list_head);
    INIT_LIST_HEAD(&ts_inst->exit_proc_list_head);
    ts_inst->sq_work_retry_time = 0;

    ts_inst->surport_proc_num = 0;
    if (ts_inst->ops.get_res_surport_proc_num != NULL) {
        (void)ts_inst->ops.get_res_surport_proc_num(inst, &ts_inst->surport_proc_num);
    }

    ret = trs_res_mng_init(ts_inst);
    if (ret != 0) {
        trs_err("Res mng init failed. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        return ret;
    }

    ret = trs_cb_sqcq_init(ts_inst);
    if (ret != 0) {
        trs_err("Cb sqcq init failed. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        goto res_mng_uninit;
    }

    ret = trs_logic_cq_init(ts_inst);
    if (ret != 0) {
        trs_err("Logic cq init failed. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        goto cb_sqcq_uninit;
    }

    ret = trs_shm_sqcq_init(ts_inst);
    if (ret != 0) {
        trs_err("Logic cq init failed. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        goto logic_cq_uninit;
    }

    ret = trs_hw_sqcq_init(ts_inst);
    if (ret != 0) {
        trs_err("Hw sqcq init failed. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        goto shm_sqcq_uninit;
    }

    ret = trs_sw_sqcq_init(ts_inst);
    if (ret != 0) {
        trs_err("Sw sqcq init failed. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        goto hw_sqcq_uninit;
    }

    return 0;

hw_sqcq_uninit:
    trs_hw_sqcq_uninit(ts_inst);
shm_sqcq_uninit:
    trs_shm_sqcq_uninit(ts_inst);
logic_cq_uninit:
    trs_logic_cq_uninit(ts_inst);
cb_sqcq_uninit:
    trs_cb_sqcq_uninit(ts_inst);
res_mng_uninit:
    trs_res_mng_uninit(ts_inst);
    return ret;
}

static void trs_core_ts_inst_uninit(struct trs_core_ts_inst *ts_inst)
{
    trs_sw_sqcq_uninit(ts_inst);
    trs_hw_sqcq_uninit(ts_inst);
    trs_shm_sqcq_uninit(ts_inst);
    trs_logic_cq_uninit(ts_inst);
    trs_cb_sqcq_uninit(ts_inst);
    trs_res_mng_uninit(ts_inst);
}

static int trs_core_ts_inst_create(struct trs_id_inst *inst, int hw_type, struct trs_core_adapt_ops *ops)
{
    u32 ts_inst_id = trs_id_inst_to_ts_inst(inst);
    struct trs_core_ts_inst *ts_inst = NULL;
    int ret;

    if (core_ts_inst[ts_inst_id] != NULL) {
        trs_err("Repeat register. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        return -EINVAL;
    }

    ts_inst = vzalloc(sizeof(struct trs_core_ts_inst));
    if (ts_inst == NULL) {
        trs_err("Mem alloc failed. (devid=%u; tsid=%u; size=%lx)\n",
            inst->devid, inst->tsid, sizeof(struct trs_core_ts_inst));
        return -ENOMEM;
    }

    ts_inst->inst = *inst;
    ts_inst->hw_type = hw_type;
    ts_inst->ops = *ops;
    ts_inst->featur_mode = TRS_INST_ALL_FEATUR_MODE;
    ret = trs_core_ts_inst_init(ts_inst);
    if (ret != 0) {
        vfree(ts_inst);
        trs_err("Ts inst create failed. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        return ret;
    }

    proc_fs_add_ts_inst(ts_inst);
    core_ts_inst[ts_inst_id] = ts_inst;

    trs_debug("Ts inst create success. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);

    return 0;
}

static void trs_core_ts_inst_release(struct kref *kref)
{
    struct trs_core_ts_inst *ts_inst = container_of(kref, struct trs_core_ts_inst, ref);

    trs_info("Ts inst release success. (devid=%u; tsid=%u)\n", ts_inst->inst.devid, ts_inst->inst.tsid);

    proc_fs_del_ts_inst(ts_inst);
    trs_core_ts_inst_uninit(ts_inst);
    vfree(ts_inst);
}

static void trs_core_ts_inst_destroy(struct trs_id_inst *inst)
{
    u32 ts_inst_id = trs_id_inst_to_ts_inst(inst);
    struct trs_core_ts_inst *ts_inst = core_ts_inst[ts_inst_id];

    if (core_ts_inst[ts_inst_id] == NULL) {
        trs_err("Repeat unregister. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        return;
    }

    write_lock_bh(&core_ts_inst_lock);
    core_ts_inst[ts_inst_id] = NULL; /* set inst invalid, so other thread will not get it */
    write_unlock_bh(&core_ts_inst_lock);

    trs_info("Ts inst destroy success. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);

    trs_hw_sq_trigger_irq_hw_res_uninit(ts_inst);
    trs_thread_bind_irq_hw_res_uninit(ts_inst, ts_inst->logic_cq_ctx.intr_mng.irq_num);

    kref_put(&ts_inst->ref, trs_core_ts_inst_release);
}

static int trs_core_check_ts_inst_ops(struct trs_core_adapt_ops *ops)
{
    if (ops == NULL) {
        return -EINVAL;
    }

    if ((ops->notice_ts == NULL) || (ops->ssid_query == NULL) || (ops->get_res_reg_offset == NULL) ||
        (ops->get_res_reg_total_size == NULL) || (ops->owner == NULL)) {
        return -EINVAL;
    }

    return 0;
}

int trs_core_ts_inst_register(struct trs_id_inst *inst, int hw_type, struct trs_core_adapt_ops *ops)
{
    int ret;

    ret = trs_id_inst_check(inst);
    if (ret != 0) {
        return ret;
    }

    ret = trs_core_check_ts_inst_ops(ops);
    if (ret != 0) {
        trs_err("Invalid ops. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        return ret;
    }

    mutex_lock(&core_ts_inst_mutex);
    ret = trs_core_ts_inst_create(inst, hw_type, ops);
    mutex_unlock(&core_ts_inst_mutex);

    return ret;
}
EXPORT_SYMBOL(trs_core_ts_inst_register);

void trs_core_ts_inst_unregister(struct trs_id_inst *inst)
{
    int ret;

    ret = trs_id_inst_check(inst);
    if (ret != 0) {
        return;
    }

    mutex_lock(&core_ts_inst_mutex);
    trs_core_ts_inst_destroy(inst);
    mutex_unlock(&core_ts_inst_mutex);
}
EXPORT_SYMBOL(trs_core_ts_inst_unregister);

struct trs_core_ts_inst *trs_core_ts_inst_get(struct trs_id_inst *inst)
{
    struct trs_core_ts_inst *ts_inst = NULL;

    if (trs_id_inst_check(inst) != 0) {
        return NULL;
    }

    read_lock_bh(&core_ts_inst_lock);
    ts_inst = core_ts_inst[trs_id_inst_to_ts_inst(inst)];
    if (ts_inst != NULL) {
        kref_get(&ts_inst->ref);
    }
    read_unlock_bh(&core_ts_inst_lock);
    return ts_inst;
}

void trs_core_ts_inst_put(struct trs_core_ts_inst *ts_inst)
{
    kref_put(&ts_inst->ref, trs_core_ts_inst_release);
}

static void trs_core_destroy_all_proc(struct trs_core_ts_inst *ts_inst)
{
    struct trs_proc_ctx *proc_ctx = NULL;
    struct trs_proc_ctx *tmp = NULL;

    down_write(&ts_inst->sem);

    list_for_each_entry_safe(proc_ctx, tmp, &ts_inst->proc_list_head, node) {
        trs_info("Recycle proc. (pid=%d, name=%s)", proc_ctx->pid, proc_ctx->name);
        list_del(&proc_ctx->node);
        trs_proc_ctx_destroy(proc_ctx);
    }

    list_for_each_entry_safe(proc_ctx, tmp, &ts_inst->exit_proc_list_head, node) {
        trs_info("Recycle proc. (pid=%d, name=%s)", proc_ctx->pid, proc_ctx->name);
        list_del(&proc_ctx->node);
        trs_proc_ctx_destroy(proc_ctx);
    }

    up_write(&ts_inst->sem);
}

void trs_core_inst_init(void)
{
    u32 i;

    mutex_init(&core_ts_inst_mutex);
    rwlock_init(&core_ts_inst_lock);

    for (i = 0; i < TRS_TS_INST_MAX_NUM; i++) {
        core_ts_inst[i] = NULL;
    }
}

void trs_core_inst_uninit(void)
{
    u32 i;

    for (i = 0; i < TRS_TS_INST_MAX_NUM; i++) {
        struct trs_core_ts_inst *ts_inst = core_ts_inst[i];
        if (ts_inst != NULL) {
            trs_core_destroy_all_proc(ts_inst);
            trs_core_ts_inst_unregister(&ts_inst->inst);
        }
    }
    mutex_destroy(&core_ts_inst_mutex);
}

bool trs_still_has_proc(struct trs_core_ts_inst *ts_inst)
{
    if (list_empty(&ts_inst->proc_list_head) && list_empty(&ts_inst->exit_proc_list_head)) {
        return false;
    }
    return true;
}

int trs_set_ts_inst_feature_mode(struct trs_id_inst *inst, u32 mode, u32 force)
{
    struct trs_core_ts_inst *ts_inst = NULL;
    int ret;

    ts_inst = trs_core_ts_inst_get(inst);
    if (ts_inst == NULL) {
        trs_err("Invalid para. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        return -EINVAL;
    }

    if (mode == TRS_INST_PART_FEATUR_MODE) {
        if (force == 0) {
            bool has_proc;

            down_read(&ts_inst->sem);
            has_proc = trs_still_has_proc(ts_inst);
            up_read(&ts_inst->sem);
            if (has_proc) {
                trs_core_inst_put(ts_inst);
                trs_err("Pf device still has proc, can not to mia. (devid=%u)\n", inst->devid);
                return -EPERM;
            }
        }

        if (ts_inst->ops.get_sq_trigger_irq != NULL) {
            if (ts_inst->work_queue != NULL) {
                (void)cancel_delayed_work_sync(&ts_inst->sq_trigger_work);
            }
            trs_hw_sq_send_thread_destroy(ts_inst);
        }
    }

    if (mode == TRS_INST_ALL_FEATUR_MODE) {
        if (ts_inst->ops.get_sq_trigger_irq != NULL) {
            ret = trs_hw_sq_send_thread_create(ts_inst);
            if (ret != 0) {
                trs_core_inst_put(ts_inst);
                trs_err("Failed to create hw sq send thread. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
                return ret;
            }
        }
    }

    ts_inst->featur_mode = mode;
    trs_core_inst_put(ts_inst);

    trs_info("Set device run status. (devid=%u; status=%u)\n", inst->devid, mode);
    return 0;
}
EXPORT_SYMBOL(trs_set_ts_inst_feature_mode);

