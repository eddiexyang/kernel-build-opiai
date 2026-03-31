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

#include <linux/version.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/mm.h>
#include <linux/spinlock_types.h>
#include <asm/pgtable.h>

#include "kernel_version_adapt.h"
#include "securec.h"

#include "trs_proc_fs.h"
#include "trs_ts_inst.h"
#include "trs_proc.h"

static bool trs_is_id_pid_match(struct trs_res_ids *id, int pid, u32 task_id)
{
    return ((id->pid == pid) && (id->task_id == task_id) && (id->status == RES_STATUS_NORMAL));
}

static bool trs_is_res_pid_match(struct trs_core_ts_inst *ts_inst, int pid, u32 task_id, int res_type, u32 res_id)
{
    struct trs_res_mng *res_mng = &ts_inst->res_mng[res_type];
    bool ret = false;

    if (res_id >= res_mng->max_id) {
        trs_err("Invalid id. (devid=%u; tsid=%u; res_type=%d; id=%u; max_num=%u)\n",
            ts_inst->inst.devid, ts_inst->inst.tsid, res_type, res_id, res_mng->max_id);
        return false;
    }

    ret = trs_is_id_pid_match(&res_mng->ids[res_id], pid, task_id);
    if (!ret) {
        /*
         * Do not change log level.
         * Shr id will check ipc notify is opend by this proc.
         */
        trs_debug("Match failed. (devid=%u; tsid=%u; res_type=%d; res_id=%u; id_pid=%d; pid=%u; "
            "id_task_id=%u; task_id=%u; id_status=%u)\n", ts_inst->inst.devid, ts_inst->inst.tsid,
            res_type, res_id, res_mng->ids[res_id].pid, pid, res_mng->ids[res_id].task_id, task_id,
            res_mng->ids[res_id].status);
    }
    return ret;
}

bool trs_proc_has_res(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, int res_type, u32 res_id)
{
    return trs_is_res_pid_match(ts_inst, proc_ctx->pid, proc_ctx->task_id, res_type, res_id);
}

static int trs_get_proc_task_id(struct trs_core_ts_inst *ts_inst, int pid, u32 *task_id)
{
    struct trs_core_ts_inst *tmp_ts_inst = ts_inst;
    struct trs_proc_ctx *proc_ctx = NULL;

    if (ts_inst->inst.tsid != 0) {
        tmp_ts_inst = trs_core_inst_get(ts_inst->inst.devid, 0); /* proc info stored in tsid 0 inst */
        if (tmp_ts_inst == NULL) {
            return -EINVAL;
        }
    }

    down_read(&ts_inst->sem);
    proc_ctx = trs_proc_ctx_find(tmp_ts_inst, pid);
    if (proc_ctx != NULL) {
        *task_id = proc_ctx->task_id;
    }
    up_read(&ts_inst->sem);

    if (tmp_ts_inst != ts_inst) {
        trs_core_inst_put(tmp_ts_inst);
    }

    return (proc_ctx == NULL) ? -EINVAL : 0;
}

int trs_core_get_ssid(struct trs_id_inst *inst, int pid, u32 *passid)
{
    struct trs_core_ts_inst *ts_inst = NULL;
    struct trs_proc_ctx *proc_ctx = NULL;

    ts_inst = trs_core_inst_get(inst->devid, inst->tsid);
    if (ts_inst == NULL) {
        return -EINVAL;
    }

    down_read(&ts_inst->sem);
    proc_ctx = trs_proc_ctx_find(ts_inst, pid);
    up_read(&ts_inst->sem);
    if (proc_ctx == NULL) {
        trs_core_inst_put(ts_inst);
        return -ENOENT;
    }

    *passid = proc_ctx->cp_ssid;
    trs_core_inst_put(ts_inst);

    return 0;
}
EXPORT_SYMBOL(trs_core_get_ssid);

bool trs_proc_has_res_with_pid(struct trs_core_ts_inst *ts_inst, int pid, int res_type, u32 res_id)
{
    u32 task_id;
    int ret;

    ret = trs_get_proc_task_id(ts_inst, pid, &task_id);
    if (ret != 0) {
        /*
         * Do not change log level.
         * Shr id will check ipc notify is opend by this proc.
         */
        trs_debug("Invalid value. (pid=%d)\n", pid);
        return false;
    }

    return trs_is_res_pid_match(ts_inst, pid, task_id, res_type, res_id);
}

int trs_res_get(struct trs_core_ts_inst *ts_inst, int pid, int res_type, u32 res_id)
{
    struct trs_res_mng *res_mng = &ts_inst->res_mng[res_type];
    u32 task_id;
    int ret;

    ret = trs_get_proc_task_id(ts_inst, pid, &task_id);
    if (ret != 0) {
        trs_err("Invalid value. (pid=%d)\n", pid);
        return ret;
    }

    mutex_lock(&res_mng->mutex);
    if (!trs_is_res_pid_match(ts_inst, pid, task_id, res_type, res_id)) {
        mutex_unlock(&res_mng->mutex);
        return -EINVAL;
    }

    res_mng->ids[res_id].ref++;
    mutex_unlock(&res_mng->mutex);

    return 0;
}

int trs_res_put(struct trs_core_ts_inst *ts_inst, int res_type, u32 res_id)
{
    struct trs_res_mng *res_mng = &ts_inst->res_mng[res_type];
    int ref;

    if (res_id >= res_mng->max_id) {
        return -EINVAL;
    }

    mutex_lock(&res_mng->mutex);
    if (res_mng->ids[res_id].ref == 0) {
        trs_err("Res not valid. (devid=%u; tsid=%u; res_type=%d; id=%u)\n",
            ts_inst->inst.devid, ts_inst->inst.tsid, res_type, res_id);
        mutex_unlock(&res_mng->mutex);
        return -EINVAL;
    }

    res_mng->ids[res_id].ref--;
    if (res_mng->ids[res_id].ref <= 0) {
        res_mng->ids[res_id].pid = 0;
        res_mng->use_num--;
    }
    ref = res_mng->ids[res_id].ref;
    mutex_unlock(&res_mng->mutex);

    return ref;
}

int trs_proc_add_res(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, int res_type, u32 res_id)
{
    struct trs_res_mng *res_mng = &ts_inst->res_mng[res_type];
    struct trs_id_inst *inst = &ts_inst->inst;

    if (res_id >= res_mng->max_id) {
        trs_err("Invalid id. (devid=%u; tsid=%u; res_type=%d; id=%u; max_num=%u)\n",
            inst->devid, inst->tsid, res_type, res_id, res_mng->max_id);
        return -EINVAL;
    }

    mutex_lock(&res_mng->mutex);
    if (res_mng->ids[res_id].pid != 0) {
        trs_err("Repeat add res. (devid=%u; tsid=%u; res_type=%d; id=%u; owner_pid=%u)\n",
            inst->devid, inst->tsid, res_type, res_id, res_mng->ids[res_id].pid);
        mutex_unlock(&res_mng->mutex);
        return -EINVAL;
    }

    res_mng->ids[res_id].task_id = proc_ctx->task_id;
    res_mng->ids[res_id].pid = proc_ctx->pid;
    res_mng->ids[res_id].ref = 1;
    res_mng->ids[res_id].status = RES_STATUS_NORMAL;
    res_mng->use_num++;
    mutex_unlock(&res_mng->mutex);

    trs_debug("Add res. (devid=%u; tsid=%u; res_type=%d; id=%u; owner_pid=%u)\n",
        inst->devid, inst->tsid, res_type, res_id, res_mng->ids[res_id].pid);

    atomic_inc(&proc_ctx->ts_ctx[inst->tsid].current_id_num[res_type]);

    return 0;
}

bool trs_is_proc_res_limited(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, int res_type)
{
    struct trs_res_mng *res_mng = &ts_inst->res_mng[res_type];
    struct trs_id_inst *inst = &ts_inst->inst;
    bool flag = false;

    mutex_lock(&res_mng->mutex);
    if ((res_mng->id_num - res_mng->use_num) < ts_inst->surport_proc_num) {
        if (trs_get_proc_res_num(proc_ctx, inst->tsid, res_type) > 0) {
            trs_warn("Proc res limited. (devid=%u; tsid=%u; res_type=%d; id_num=%u; use_num=%u; surport_proc_num=%u)\n",
                inst->devid, inst->tsid, res_type, res_mng->id_num, res_mng->use_num, ts_inst->surport_proc_num);
            flag = true;
        }
    }
    mutex_unlock(&res_mng->mutex);

    return flag;
}

int trs_proc_del_res(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, int res_type, u32 res_id)
{
    struct trs_res_mng *res_mng = &ts_inst->res_mng[res_type];
    struct trs_id_inst *inst = &ts_inst->inst;

    mutex_lock(&res_mng->mutex);
    if (!trs_proc_has_res(proc_ctx, ts_inst, res_type, res_id)) {
        mutex_unlock(&res_mng->mutex);
        trs_err("Not proc res. (devid=%u; tsid=%u; res_type=%d; id=%d)\n", inst->devid, inst->tsid, res_type, res_id);
        return -EINVAL;
    }

    res_mng->ids[res_id].status = RES_STATUS_DEL;
    mutex_unlock(&res_mng->mutex);

    trs_debug("Del res. (devid=%u; tsid=%u; res_type=%d; id=%u\n", inst->devid, inst->tsid, res_type, res_id);

    atomic_dec(&proc_ctx->ts_ctx[inst->tsid].current_id_num[res_type]);

    return trs_res_put(ts_inst, res_type, res_id);
}

struct trs_proc_ctx *trs_proc_ctx_find(struct trs_core_ts_inst *ts_inst, int pid)
{
    struct trs_proc_ctx *proc_ctx = NULL;

    list_for_each_entry(proc_ctx, &ts_inst->proc_list_head, node) {
        if (proc_ctx->pid == pid) {
            return proc_ctx;
        }
    }

    return NULL;
}

#define TRS_MAX_WAIT_PROC_EXIT_CNT 100U // 100 try times
int trs_proc_wait_for_exit(struct trs_core_ts_inst *ts_inst, int pid)
{
    int loop = 0;

    do {
        struct trs_proc_ctx *proc_ctx = NULL;

        down_read(&ts_inst->sem);
        proc_ctx = trs_proc_ctx_find(ts_inst, pid);
        up_read(&ts_inst->sem);
        if (proc_ctx == NULL) {
            return 0;
        }
        usleep_range(USEC_PER_MSEC - 100, USEC_PER_MSEC); /* 1ms, range 100 us */
    } while (++loop < TRS_MAX_WAIT_PROC_EXIT_CNT);
    return -EBUSY;
}

struct trs_proc_ctx *trs_proc_ctx_create(struct trs_core_ts_inst *ts_inst)
{
    struct trs_proc_ctx *proc_ctx = NULL;
    int i;

    proc_ctx = kzalloc(sizeof(struct trs_proc_ctx), GFP_KERNEL | __GFP_ACCOUNT);
    if (proc_ctx == NULL) {
        trs_err("Mem alloc failed. (size=%lx)\n", sizeof(struct trs_proc_ctx));
        return NULL;
    }

    for (i = 0; i < TRS_TS_MAX_NUM; i++) {
        mutex_init(&proc_ctx->ts_ctx[i].mutex);
        mutex_init(&proc_ctx->ts_ctx[i].shm_ctx.mutex);
        proc_ctx->ts_ctx[i].shm_ctx.chan_id = -1;
    }

    proc_ctx->devid = ts_inst->inst.devid;
    proc_ctx->task_id = ts_inst->cur_task_id++;
    proc_ctx->pid = current->tgid;
    if (strncpy_s(proc_ctx->name, TASK_COMM_LEN, current->comm, strlen(current->comm)) != 0) {
        trs_warn("Strcpy failed. (pid=%d)\n", proc_ctx->pid);
    }
    proc_ctx->name[TASK_COMM_LEN - 1] = '\0';

    proc_fs_add_pid(proc_ctx);

    return proc_ctx;
}

void trs_proc_ctx_destroy(struct trs_proc_ctx *proc_ctx)
{
    proc_fs_del_pid(proc_ctx);
    kfree(proc_ctx);
}

static int trs_vma_flag_check(struct trs_core_ts_inst *ts_inst, struct vm_area_struct *vma, int type)
{
    if ((vma->vm_flags & VM_HUGETLB) != 0) {
        trs_err("Invalid vm_flags. (vm_flags=0x%lx)\n", vma->vm_flags);
        return -EINVAL;
    }

    if ((type == TRS_MAP_TYPE_RO_DEV_MEM) || (type == TRS_MAP_TYPE_RO_REG)) {
        if ((vma->vm_flags & VM_WRITE) != 0) {
            trs_err("Invalid vm_flags. (vm_flags=0x%lx)\n", vma->vm_flags);
            return -EINVAL;
        }
    }

    return 0;
}

static int trs_check_va_range(struct vm_area_struct *vma, unsigned long addr, unsigned long size)
{
    unsigned long end = addr + PAGE_ALIGN(size);
    if (((addr & (PAGE_SIZE - 1)) != 0) || (addr < vma->vm_start) || (addr > vma->vm_end) ||
        (end > vma->vm_end) || (addr >= end)) {
        trs_err("Invalid para. (addr=0x%lx; size=0x%lx; end=0x%lx; vm_start=0x%lx; vm_end=0x%lx)\n",
            addr, size, end, vma->vm_start, vma->vm_end);
        return -EINVAL;
    }

    return 0;
}

static int trs_check_va_map(struct vm_area_struct *vma, unsigned long addr, unsigned long size)
{
    unsigned long end = addr + PAGE_ALIGN(size);
    unsigned long va_check, pfn;

    for (va_check = addr; va_check < end; va_check += PAGE_SIZE) {
        if (follow_pfn(vma, va_check, &pfn) == 0) {
            trs_err("Check va is map. (addr=0x%lx; size=0x%lx; va_check=0x%lx)\n", addr, size, va_check);
            return -EINVAL;
        }
    }

    return 0;
}

static int trs_check_va_unmap(struct vm_area_struct *vma, unsigned long addr, unsigned long size)
{
    unsigned long end = addr + PAGE_ALIGN(size);
    unsigned long va_check, pfn;

    for (va_check = addr; va_check < end; va_check += PAGE_SIZE) {
        if (follow_pfn(vma, va_check, &pfn) != 0) {
            trs_err("Check va is unmap. (addr=0x%lx; size=0x%lx; va_check=0x%lx)\n", addr, size, va_check);
            return -EINVAL;
        }
    }

    return 0;
}

static int trs_get_remap_prot(struct trs_core_ts_inst *ts_inst, int type, pgprot_t vm_page_prot, pgprot_t *prot)
{
    if ((type == TRS_MAP_TYPE_REG) || (type == TRS_MAP_TYPE_RO_REG)) {
        *prot = pgprot_device(vm_page_prot);
    } else if (type == TRS_MAP_TYPE_MEM) {
        *prot = vm_page_prot;
    } else { /* TRS_MAP_TYPE_DEV_MEM TRS_MAP_TYPE_DEV_RD_MEM */
#ifdef CONFIG_ARM64
        *prot = pgprot_writecombine(vm_page_prot);
#else
        *prot = vm_page_prot;
#endif
    }

    return 0;
}

static void trs_zap_vma_ptes(struct vm_area_struct *vma, unsigned long addr, size_t len)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 18, 0)
    int ret;
    ret = zap_vma_ptes(vma, addr, PAGE_ALIGN(len));
    if (ret != 0) {
        trs_err("Unmap va failed. (vma_start=0x%lx; end=0x%lx; addr=0x%lx; len=0x%lx)\n",
            vma->vm_start, vma->vm_end, addr, len);
    }
#else
    zap_vma_ptes(vma, addr, PAGE_ALIGN(len));
#endif
}

static int trs_remap_pfn(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst,
    struct trs_mem_map_para *para, struct vm_area_struct *vma)
{
    pgprot_t prot;
    int ret;

    ret = trs_get_remap_prot(ts_inst, para->type, vma->vm_page_prot, &prot);
    if (ret != 0) {
        return ret;
    }

    ret = remap_pfn_range(vma, para->va, __phys_to_pfn(para->pa), para->len, prot);
    if (ret != 0) {
        trs_err("Remap va failed. (vma_start=0x%lx; end=0x%lx; addr=0x%lx; len=0x%lx)\n",
            vma->vm_start, vma->vm_end, para->va, para->len);
    }

    return ret;
}

static int trs_remap_sq_mem(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst,
    struct trs_mem_map_para *para)
{
    struct vm_area_struct *vma = NULL;
    int ret;

    vma = find_vma(current->mm, para->va);
    if (vma == NULL) {
        trs_err("Find vma failed. (va=%lx)\n", para->va);
        return -EINVAL;
    }

    if (vma->vm_private_data != proc_ctx) {
        trs_err("Invalid vm private data.\n");
        return -EINVAL;
    }

    ret = trs_vma_flag_check(ts_inst, vma, para->type);
    if (ret != 0) {
        return ret;
    }

    ret = trs_check_va_range(vma, para->va, para->len);
    if (ret != 0) {
        return ret;
    }

    ret = trs_check_va_map(vma, para->va, para->len);
    if (ret != 0) {
        return ret;
    }

    return trs_remap_pfn(proc_ctx, ts_inst, para, vma);
}

int trs_remap_sq(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, struct trs_mem_map_para *para)
{
    int ret;

    down_write(get_mmap_sem(current->mm));
    ret = trs_remap_sq_mem(proc_ctx, ts_inst, para);
    up_write(get_mmap_sem(current->mm));

    return ret;
}

static int trs_unmap_sq_mem(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst,
    struct trs_mem_unmap_para *para)
{
    struct vm_area_struct *vma = NULL;
    int ret;

    vma = find_vma(current->mm, para->va);
    if (vma == NULL) {
        trs_err("Find vma failed. (va=%lx)\n", para->va);
        return -EINVAL;
    }

    if (vma->vm_private_data != proc_ctx) {
        trs_err("Invalid vm private data.\n");
        return -EINVAL;
    }

    ret = trs_check_va_range(vma, para->va, para->len);
    if (ret != 0) {
        return ret;
    }

    ret = trs_check_va_unmap(vma, para->va, para->len);
    if (ret != 0) {
        return ret;
    }

    trs_zap_vma_ptes(vma, para->va, para->len);

    return 0;
}

int trs_unmap_sq(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, struct trs_mem_unmap_para *para)
{
    int ret;

    down_write(get_mmap_sem(current->mm));
    ret = trs_unmap_sq_mem(proc_ctx, ts_inst, para);
    up_write(get_mmap_sem(current->mm));

    return ret;
}

static int trs_proc_release_msg_send(struct trs_core_ts_inst *ts_inst, int pid, u16 cmd_type)
{
    struct recycle_proc_msg msg;

    trs_mbox_init_header(&msg.header, cmd_type);
    msg.proc_info.app_cnt = 1;
    msg.proc_info.pid[0] = pid;

    /* adapt fill: plat_type, fid */
    return trs_core_notice_ts(ts_inst, (u8 *)&msg, sizeof(msg));
}

static int trs_proc_release_check_ts(struct trs_proc_ctx *proc_ctx, u32 tsid)
{
    int i, ret;

    for (i = 0; i < 300; i++) { /* retry 300 times */
        struct trs_core_ts_inst *ts_inst = trs_core_inst_get(proc_ctx->devid, tsid);
        if (ts_inst == NULL) {
            return 0;
        }

        trs_debug("Send check. (devid=%u; tsid=%u; name=%s; pid=%d; task_id=%d)\n",
            ts_inst->inst.devid, tsid, proc_ctx->name, proc_ctx->pid, proc_ctx->task_id);
        ret = trs_proc_release_msg_send(ts_inst, proc_ctx->pid, TRS_MBOX_RECYCLE_CHECK);
        if (ret == 0) {
            trs_core_inst_put(ts_inst);
            return 0;
        }

        trs_core_inst_put(ts_inst);

        (i < 10) ? msleep(100) : msleep(1000); /* fisrt 10 times, sleep 100 ms, then sleep 1000 ms */
    }

    return -ETIMEDOUT;
}

static void trs_proc_release_notice_ts(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, int cnt)
{
    struct trs_id_inst *inst = &ts_inst->inst;
    int i, ret;

    for (i = 0; i < cnt; i++) {
        ret = trs_proc_release_msg_send(ts_inst, proc_ctx->pid, TRS_MBOX_RECYCLE_PID);
        if (ret == 0) {
            break;
        }

        trs_warn("Notice ts failed. (devid=%u; tsid=%u; i=%d; ret=%d)\n", inst->devid, inst->tsid, i, ret);
    }
}

static void trs_for_each_proc_res_id(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, int res_type,
    void (*recycle_func)(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, int res_type, u32 res_id))
{
    struct trs_res_mng *res_mng = &ts_inst->res_mng[res_type];
    struct trs_id_inst *inst = &ts_inst->inst;
    u32 i;
    u32 recycle_num = 0;

    if (trs_get_proc_res_num(proc_ctx, inst->tsid, res_type) == 0) {
        return;
    }

    for (i = 0; i < res_mng->max_id; i++) {
        if (trs_is_id_pid_match(&res_mng->ids[i], proc_ctx->pid, proc_ctx->task_id)) {
            recycle_num++;
            recycle_func(proc_ctx, ts_inst, res_type, i);
            if (trs_get_proc_res_num(proc_ctx, ts_inst->inst.tsid, res_type) == 0) {
                break;
            }
        }
    }

    trs_debug("Recycle res. (devid=%u; tsid=%u; res_type=%d; recycle_num=%u; proc_res_num=%u)\n",
        inst->devid, inst->tsid, res_type, recycle_num, trs_get_proc_res_num(proc_ctx, inst->tsid, res_type));
}

static void trs_proc_leak_res_show(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst)
{
    struct trs_id_inst *inst = &ts_inst->inst;
    struct trs_res_mng *res_mng = NULL;
    int i, j, num;

    for (i = 0; i < TRS_CORE_MAX_ID_TYPE; i++) {
        res_mng = &ts_inst->res_mng[i];
        num = 0;
        for (j = 0; (u32)j < res_mng->max_id; j++) {
            if (num == trs_get_proc_res_num(proc_ctx, inst->tsid, i)) {
                break;
            }
            if (trs_is_id_pid_match(&res_mng->ids[i], proc_ctx->pid, proc_ctx->task_id)) {
                num++;
                trs_warn("Leak res. (devid=%u; tsid=%u; res_type=%d; res_id=%d; proc_res_num=%d)\n",
                    inst->devid, inst->tsid, i, j, trs_get_proc_res_num(proc_ctx, inst->tsid, i));
            }
        }
    }
}

static int trs_proc_ts_res_recycle(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst)
{
    struct trs_id_inst *inst = &ts_inst->inst;
    int i, ret;

    ret = trs_proc_release_check_ts(proc_ctx, inst->tsid);
    if (ret != 0) {
        trs_warn("Recycle failed. (devid=%u; tsid=%u; name=%s; pid=%d; task_id=%d)\n",
            inst->devid, inst->tsid, proc_ctx->name, proc_ctx->pid, proc_ctx->task_id);
        trs_proc_leak_res_show(proc_ctx, ts_inst);
        return -EBUSY;
    }

    if (proc_ctx->ts_ctx[inst->tsid].shm_ctx.chan_id >= 0) {
        trs_shm_sqcq_recycle(proc_ctx, ts_inst);
    }

    trs_for_each_proc_res_id(proc_ctx, ts_inst, TRS_HW_SQ, trs_hw_sqcq_recycle);
    trs_for_each_proc_res_id(proc_ctx, ts_inst, TRS_HW_CQ, trs_hw_sqcq_recycle);
    trs_for_each_proc_res_id(proc_ctx, ts_inst, TRS_SW_SQ, trs_sw_sqcq_recycle);
    trs_for_each_proc_res_id(proc_ctx, ts_inst, TRS_SW_CQ, trs_sw_sqcq_recycle);

    for (i = 0; i < TRS_HW_SQ; i++) {
        trs_for_each_proc_res_id(proc_ctx, ts_inst, i, trs_res_id_recycle);
    }

    trs_debug("Recycle success. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);

    return 0;
}

static bool trs_proc_is_res_leak(struct trs_proc_ctx *proc_ctx, u32 tsid)
{
    int i;

    if (proc_ctx->ts_ctx[tsid].shm_ctx.chan_id >= 0) {
        return true;
    }

    for (i = 0; i < TRS_CORE_MAX_ID_TYPE; i++) {
        if (trs_get_proc_res_num(proc_ctx, tsid, i) > 0) {
            return true;
        }
    }

    return false;
}

int trs_proc_release(struct trs_proc_ctx *proc_ctx)
{
    int i;
    int ret = 0;

    proc_ctx->status = TRS_PROC_STATUS_EXIT;

    for (i = 0; i < TRS_TS_MAX_NUM; i++) {
        struct trs_core_ts_inst *ts_inst = NULL;

        ts_inst = trs_core_inst_get(proc_ctx->devid, i);
        if (ts_inst != NULL) {
            if (ts_inst->featur_mode == TRS_INST_PART_FEATUR_MODE) {
                continue;
            }
            trs_for_each_proc_res_id(proc_ctx, ts_inst, TRS_LOGIC_CQ, trs_logic_cq_recycle);
            trs_for_each_proc_res_id(proc_ctx, ts_inst, TRS_CB_CQ, trs_cb_sqcq_recycle);
            trs_for_each_proc_res_id(proc_ctx, ts_inst, TRS_CB_SQ, trs_cb_sqcq_recycle);
            if (trs_proc_is_res_leak(proc_ctx, i)) {
                trs_warn("Res leak. (pid=%d; devid=%u; tsid=%u)\n",
                    proc_ctx->pid, ts_inst->inst.devid, ts_inst->inst.tsid);
                ret = -EBUSY;
                trs_proc_release_notice_ts(proc_ctx, ts_inst, 1); /* retry 1 times */
            } else {
                trs_proc_release_notice_ts(proc_ctx, ts_inst, 3); /* retry 3 times */
            }
            trs_core_inst_put(ts_inst);
        }
    }

    return ret;
}

int trs_proc_recycle(struct trs_proc_ctx *proc_ctx)
{
    int ret = 0;
    int i;

    trs_debug("Begin recycle. (pid=%d)\n", proc_ctx->pid);

    for (i = 0; i < TRS_TS_MAX_NUM; i++) {
        struct trs_core_ts_inst *ts_inst = trs_core_inst_get(proc_ctx->devid, i);
        if (ts_inst != NULL) {
            if (trs_proc_is_res_leak(proc_ctx, i)) {
                ret |= trs_proc_ts_res_recycle(proc_ctx, ts_inst);
            }
            trs_core_inst_put(ts_inst);
        }
    }

    return ret;
}

