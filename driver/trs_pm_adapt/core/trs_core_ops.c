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
#include <linux/sched.h>
#include <linux/mman.h>

#include "virtmng_interface.h"
#include "devdrv_manager_comm.h"

#include "trs_pm_adapt.h"
#include "trs_device_core.h"
#include "trs_mailbox_def.h"
#include "trs_core.h"
#include "trs_chip_def.h"
#include "trs_device_mbox.h"
#include "trs_device_comm.h"
#include "trs_msg.h"
#include "trs_chan_mem.h"
#include "trs_id.h"
#include "soc_res.h"
#include "trs_core_ops.h"
#include "trs_chan_mbox.h"
#include "kernel_version_adapt.h"

#define TRS_CORE_MBOX_TIMEOUT   3000

static int trs_core_ops_cbcq_msg_fill(struct trs_id_inst *inst, u8 *msg, u32 len)
{
    struct trs_cb_cq_mbox *tmp_msg  = (struct trs_cb_cq_mbox *)msg;

    tmp_msg->plat_type = TRS_MBOX_SEND_FROM_DEVICE;

    if (tmp_msg->header.cmd_type == TRS_MBOX_RELEASE_CB_CQ) {
        return 0;
    }

    return trs_chan_ops_get_hw_irq(inst, TS_CQ_UPDATE_IRQ, tmp_msg->cq_irq, &tmp_msg->cq_irq);
}

static int trs_core_ops_shm_sqcq_msg_fill(struct trs_id_inst *inst, u8 *msg, u32 len)
{
    struct trs_shm_sqcq_mbox *tmp_msg  = (struct trs_shm_sqcq_mbox *)msg;

    tmp_msg->app_type = TRS_MBOX_SEND_FROM_DEVICE;
    tmp_msg->sq_cq_side = (TRS_CHAN_DEV_MEM << TRS_CHAN_SQ_MEM_OFFSET) |
        (TRS_CHAN_DEV_MEM << TRS_CHAN_CQ_MEM_OFFSET);
    tmp_msg->fid = 0;

    if (tmp_msg->header.cmd_type == TRS_MBOX_SHM_SQCQ_FREE) {
        return 0;
    }

    return trs_chan_ops_get_hw_irq(inst, TS_CQ_UPDATE_IRQ, tmp_msg->cq_irq, &tmp_msg->cq_irq);
}

static int trs_core_ops_sqcq_msg_fill(struct trs_id_inst *inst, u8 *msg, u32 len)
{
    struct trs_normal_cqsq_mailbox *tmp_msg = (struct trs_normal_cqsq_mailbox *)msg;

    tmp_msg->app_type = TRS_MBOX_SEND_FROM_DEVICE;
    tmp_msg->sq_cq_side = (TRS_CHAN_DEV_MEM << TRS_CHAN_SQ_MEM_OFFSET) |
        (TRS_CHAN_DEV_MEM << TRS_CHAN_CQ_MEM_OFFSET);
    tmp_msg->fid = 0;

    if ((tmp_msg->header.cmd_type == TRS_MBOX_RELEASE_CQSQ_CALC) || (tmp_msg->cq_irq == 0)) { /* free or reuse cq */
        return 0;
    } else {
        u32 cq_irq;
        int ret = trs_chan_ops_get_hw_irq(inst, TS_CQ_UPDATE_IRQ, (u32)tmp_msg->cq_irq, &cq_irq);
        tmp_msg->cq_irq = (u16)cq_irq;
        return ret;
    }
}

static int trs_core_ops_logic_cq_alloc_msg_fill(struct trs_id_inst *inst, u8 *msg, u32 len)
{
    struct trs_logic_cq_mbox *tmp_msg = (struct trs_logic_cq_mbox *)msg;
    u32 cq_irq;
    int ret = trs_chan_ops_get_hw_irq(inst, TS_CQ_UPDATE_IRQ, (u32)tmp_msg->mb_alloc.cq_irq, &cq_irq);

    tmp_msg->mb_alloc.cq_irq = (u16)cq_irq;
    tmp_msg->mb_alloc.app_flag = TRS_MBOX_SEND_FROM_DEVICE;
    tmp_msg->mb_alloc.vfid = 0;

    return ret;
}

static int trs_core_ops_logic_cq_free_msg_fill(struct trs_id_inst *inst, u8 *msg, u32 len)
{
    struct trs_logic_cq_mbox *tmp_msg = (struct trs_logic_cq_mbox *)msg;

    tmp_msg->mb_free.vfid = 0;

    return 0;
}

static int trs_core_ops_recycle_check_msg_fill(struct trs_id_inst *inst, u8 *msg, u32 len)
{
    struct recycle_proc_msg *tmp_msg = (struct recycle_proc_msg *)msg;

    tmp_msg->proc_info.plat_type = TRS_MBOX_SEND_FROM_DEVICE;
    tmp_msg->proc_info.fid = 0;

    return 0;
}

static int trs_core_ops_res_map_msg_fill(struct trs_id_inst *inst, u8 *msg, u32 len)
{
    struct trs_res_map_msg *tmp_msg = (struct trs_res_map_msg *)msg;

    tmp_msg->vf_id = 0;
    tmp_msg->phy_id = tmp_msg->id;

    return 0;
}

static int trs_core_ops_reset_notify(struct trs_id_inst *inst, u8 *msg, u32 len)
{
    struct trs_notify_msg *tmp_msg = (struct trs_notify_msg *)msg;

    tmp_msg->fid = 0;
    tmp_msg->plat_type = TRS_MBOX_SEND_FROM_DEVICE;
    tmp_msg->phy_notifyId = tmp_msg->notifyId;

    return 0;
}

static int (* const trs_core_ops_fill_mbox_msg[TRS_MBOX_CMD_MAX])(struct trs_id_inst *inst, u8 *msg, u32 len) = {
    [TRS_MBOX_CREATE_CB_CQ] = trs_core_ops_cbcq_msg_fill,
    [TRS_MBOX_RELEASE_CB_CQ] = trs_core_ops_cbcq_msg_fill,
    [TRS_MBOX_SHM_SQCQ_ALLOC] = trs_core_ops_shm_sqcq_msg_fill,
    [TRS_MBOX_SHM_SQCQ_FREE] = trs_core_ops_shm_sqcq_msg_fill,
    [TRS_MBOX_CREATE_CQSQ_CALC] = trs_core_ops_sqcq_msg_fill,
    [TRS_MBOX_RELEASE_CQSQ_CALC] = trs_core_ops_sqcq_msg_fill,
    [TRS_MBOX_LOGIC_CQ_ALLOC] = trs_core_ops_logic_cq_alloc_msg_fill,
    [TRS_MBOX_LOGIC_CQ_FREE] = trs_core_ops_logic_cq_free_msg_fill,
    [TRS_MBOX_RECYCLE_PID] = trs_core_ops_recycle_check_msg_fill,
    [TRS_MBOX_RECYCLE_CHECK] = trs_core_ops_recycle_check_msg_fill,
    [TRS_MBOX_RES_MAP] = trs_core_ops_res_map_msg_fill,
    [TRS_MBOX_RESET_NOTIFY] = trs_core_ops_reset_notify
};

int trs_core_ops_notice_ts(struct trs_id_inst *inst, u8 *msg, u32 len)
{
    struct trs_mb_header *header = (struct trs_mb_header *)msg;
    int ret;

    if (trs_core_ops_fill_mbox_msg[header->cmd_type] != NULL) {
        ret = trs_core_ops_fill_mbox_msg[header->cmd_type](inst, msg, len);
        if (ret != 0) {
            return ret;
        }
    }

    return trs_mbox_send(inst, 0, (void *)msg, (size_t)len, TRS_CORE_MBOX_TIMEOUT);
}

static int trs_core_ops_get_notify_offset(struct trs_id_inst *inst, u32 id, u32 *offset)
{
    *offset = trs_get_notify_offset(id);
    return 0;
}

static int trs_core_ops_get_notify_total_size(struct trs_id_inst *inst, u32 *total_size)
{
    size_t notify_size = trs_get_notify_size();
    u32 start, end;
    int ret;

    ret = trs_id_get_range(inst, TRS_NOTIFY_ID, &start, &end);
    if (ret != 0) {
        return ret;
    }

    *total_size = (end - start) * notify_size;

    return 0;
}

int trs_core_ops_get_res_reg_offset(struct trs_id_inst *inst, int type, u32 id, u32 *offset)
{
    if (type == TRS_NOTIFY_ID) {
        return trs_core_ops_get_notify_offset(inst, id, offset);
    }

    return -ENODEV;
}
EXPORT_SYMBOL(trs_core_ops_get_res_reg_offset);

int trs_core_ops_get_res_reg_total_size(struct trs_id_inst *inst, int type, u32 *total_size)
{
    if (type == TRS_NOTIFY_ID) {
        return trs_core_ops_get_notify_total_size(inst, total_size);
    }

    return -ENODEV;
}
EXPORT_SYMBOL(trs_core_ops_get_res_reg_total_size);

char ts_reg_name[][SOC_RESMNG_MAX_NAME_LEN] = {
    [TRS_NOTIFY_ID] = "TS_STARS_NOTIFY_TBL_REG",
    [TRS_EVENT_ID] = "TS_STARS_EVENT_TBL_NS_REG",
};

static int trs_core_ops_res_id_query_addr(struct trs_id_inst *inst, u32 type, u32 id, u64 *value)
{
    struct soc_reg_base_info io_base;
    struct res_inst_info res_inst;
    int ret;

    if ((type != TRS_NOTIFY_ID) && ((type != TRS_EVENT_ID))) {
        return -ENODEV;
    }

    soc_resmng_inst_pack(&res_inst, inst->devid, TS_SUBSYS, inst->tsid);
    ret = soc_resmng_get_reg_base(&res_inst, ts_reg_name[type], &io_base);
    if (ret != 0) {
        trs_err("Failed to get res id reg base. (devid=%u; tsid=%u; type=%u)\n", inst->devid, inst->tsid, type);
        return ret;
    }

    if (type == TRS_NOTIFY_ID) {
        *value = io_base.io_base + trs_get_notify_offset(id);
    } else {
        *value = io_base.io_base + trs_get_event_offset(id);
    }

    return 0;
}

#ifndef EMU_ST
bool trs_is_support_uio_d(void)
{
    enum devdrv_process_type cp_type;
    u32 chip_id, vfid, host_pid;
    int ret;

    if (trs_get_feature_mode() == PART_FEATURE_MODE) {
        ret = devdrv_query_process_host_pid(current->tgid, &chip_id, &vfid, &host_pid, &cp_type);
        if (ret != 0) {
            trs_debug("no cp. (pid=%d; ret=%d)\n", current->tgid, ret);
            return false;
        }

        if (cp_type == DEVDRV_PROCESS_CP1) {
            trs_debug("Cp info. (pid=%d; cp_type=%d)\n", current->tgid, cp_type);
            return true;
        }
    }

    return false;
}
#endif

int trs_core_ops_res_id_query(struct trs_id_inst *inst, u32 type, u32 id, u32 cmd, u64 *value)
{
    if (cmd == TRS_RES_QUERY_ADDR) {
#ifndef EMU_ST
        if (!trs_is_support_uio_d()) {
            *value = 0;
            return 0;
        }
#endif
        return trs_core_ops_res_id_query_addr(inst, type, id, value);
    }

    return -ENODEV;
}

int trs_core_ops_get_support_proc_num(struct trs_id_inst *inst, u32 *proc_num)
{
    *proc_num = TRS_SUPPORT_PROC_NUM;
    return 0;
}
EXPORT_SYMBOL(trs_core_ops_get_support_proc_num);

#ifndef EMU_ST
static unsigned long trs_core_ops_vm_mmap(struct mm_struct *mm, unsigned long addr,
    unsigned long len, unsigned long prot,
    unsigned long flag, unsigned long pgoff)
{
    unsigned long populate;
    LIST_HEAD(uf);
    /* __do_mmap_mm stubbed */ return -ENOTSUPP;
}

static int trs_core_ops_vm_munmap(struct mm_struct *mm, unsigned long start, size_t len)
{
    LIST_HEAD(uf);
#ifdef CFG_SUPPORT_AOS_KERNEL
    return do_munmap(mm, start, len, &uf);
#else
    /* __do_munmap stubbed */ return do_munmap(mm, start, len, NULL);
#endif
}

static struct task_struct *trs_core_ops_find_get_task(pid_t pid)
{
    struct task_struct *tsk = NULL;

    rcu_read_lock();
    for_each_process(tsk) {
        if (tsk->tgid == pid) {
            get_task_struct(tsk);
            rcu_read_unlock();
            return tsk;
        }
    }
    rcu_read_unlock();

    return NULL;
}

static pgprot_t trs_core_ops_pgprot(u32 flg)
{
    u64 prot_val = (pgprot_val(PAGE_SHARED_EXEC) & (~PTE_SHARED)) | PTE_DIRTY;

    return __pgprot(prot_val);
}

void trs_core_ops_zap_vma_ptes(struct vm_area_struct *vma, unsigned long vaddr, unsigned long size)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 18, 0)
    int ret;

    ret = zap_vma_ptes(vma, vaddr, size);
    if (ret != 0) {
        devmm_drv_err("Zap_vma_ptes fail. (va=0x%lx; ret=%d; flags=0x%lx; start=0x%lx; end=0x%lx)\n",
                      vaddr, ret, vma->vm_flags, vma->vm_start, vma->vm_end);
    }
#else
    zap_vma_ptes(vma, vaddr, size);
#endif
}

static int trs_core_ops_mmap_remote(pid_t pid, phys_addr_t paddr, size_t size, u64 *va)
{
    struct vm_area_struct *vma = NULL;
    struct task_struct *tsk = NULL;
    u64 addr;
    int ret;

    tsk = trs_core_ops_find_get_task(pid);
    if (tsk == NULL) {
        trs_err("Find task fail. (pid=%d)\n", pid);
        return -ENODEV;
    }

    if (tsk->mm == NULL) {
        put_task_struct(tsk);
        trs_err("Task mm is NULL. (pid=%d; tsk->tgid=%d)\n", pid, tsk->tgid);
        return -ENOMEM;
    }
    down_write(get_mmap_sem(tsk->mm));
    current->mm = tsk->mm;
    addr = trs_core_ops_vm_mmap(tsk->mm, 0, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0);
    current->mm = NULL;
    ret = IS_ERR((void *)(uintptr_t)(addr));
    if (ret != 0) {
        up_write(get_mmap_sem(tsk->mm));
        put_task_struct(tsk);
        trs_err("Vm mmap fail. (ret=%d)\n", ret);
        return ret;
    }
    vma = find_vma(tsk->mm, addr);
    if (vma == NULL) {
        trs_core_ops_vm_munmap(tsk->mm, addr, size);
        up_write(get_mmap_sem(tsk->mm));
        put_task_struct(tsk);
        return -ENOMEM;
    }
    vm_flags_set(vma, VM_LOCKED);
    vm_flags_set(vma, VM_DONTEXPAND);
    vm_flags_set(vma, VM_PFNMAP);
    vm_flags_set(vma, VM_DONTDUMP);
    vm_flags_set(vma, VM_DONTCOPY);

    ret = remap_pfn_range(vma, addr, PFN_DOWN(paddr), size, trs_core_ops_pgprot(0));
    if (ret != 0) {
        trs_core_ops_vm_munmap(tsk->mm, addr, size);
        up_write(get_mmap_sem(tsk->mm));
        put_task_struct(tsk);
        return ret;
    }
    up_write(get_mmap_sem(tsk->mm));
    put_task_struct(tsk);

    if (va != NULL) {
        *va = addr;
    }
    return 0;
}

static int trs_core_ops_mmap_local(pid_t pid, phys_addr_t paddr, size_t size, u64 *va)
{
    struct vm_area_struct *vma = NULL;
    u64 addr;
    int ret;

    down_write(get_mmap_sem(current->mm));
    addr = trs_core_ops_vm_mmap(current->mm, 0, size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0);
    ret = IS_ERR((void *)(uintptr_t)(addr));
    if (ret != 0) {
        up_write(get_mmap_sem(current->mm));
        trs_err("Vm mmap fail. (ret=%d)\n", ret);
        return ret;
    }
    vma = find_vma(current->mm, addr);
    if (vma == NULL) {
        trs_core_ops_vm_munmap(current->mm, addr, size);
        up_write(get_mmap_sem(current->mm));
        return -ENOMEM;
    }
    vm_flags_set(vma, VM_LOCKED);
    vm_flags_set(vma, VM_DONTEXPAND);
    vm_flags_set(vma, VM_PFNMAP);
    vm_flags_set(vma, VM_DONTDUMP);
    vm_flags_set(vma, VM_DONTCOPY);

    ret = remap_pfn_range(vma, addr, PFN_DOWN(paddr), size, trs_core_ops_pgprot(0));
    if (ret != 0) {
        trs_core_ops_vm_munmap(current->mm, addr, size);
        up_write(get_mmap_sem(current->mm));
        return ret;
    }
    up_write(get_mmap_sem(current->mm));

    if (va != NULL) {
        *va = addr;
    }

    return 0;
}

static int trs_core_ops_unmap_remote(pid_t pid, u64 va, size_t size)
{
    struct task_struct *tsk = trs_core_ops_find_get_task(pid);
    struct vm_area_struct *vma = NULL;
    int ret;

    if (tsk == NULL) {
        trs_err("Find task fail. (pid=%d)\n", pid);
        return -ENODEV;
    }

    if (tsk->mm == NULL) {
        put_task_struct(tsk);
        trs_err("Task mm is NULL\n");
        return -ENOMEM;
    }

    down_write(get_mmap_sem(tsk->mm));
    vma = find_vma(tsk->mm, va);
    if ((vma == NULL) || (vma->vm_start > va)) {
        up_write(get_mmap_sem(tsk->mm));
        put_task_struct(tsk);
        trs_err("Find vma fail.\n");
        return -ENOMEM;
    }
    trs_core_ops_zap_vma_ptes(vma, va, size);
    ret = trs_core_ops_vm_munmap(tsk->mm, va, size);
    if (ret != 0) {
        up_write(get_mmap_sem(tsk->mm));
        put_task_struct(tsk);
        trs_err("Unmap vm fail. (ret=%d)\n", ret);
        return ret;
    }
    up_write(get_mmap_sem(tsk->mm));
    put_task_struct(tsk);

    return ret;
}

static int trs_core_ops_unmap_local(pid_t pid, u64 va, size_t size)
{
    struct vm_area_struct *vma = NULL;
    int ret;

    down_write(get_mmap_sem(current->mm));
    vma = find_vma(current->mm, va);
    if ((vma == NULL) || (vma->vm_start > va)) {
        up_write(get_mmap_sem(current->mm));
        trs_err("Find vma fail.\n");
        return -ENOMEM;
    }
    trs_core_ops_zap_vma_ptes(vma, va, size);
    ret = trs_core_ops_vm_munmap(current->mm, va, size);
    if (ret != 0) {
        up_write(get_mmap_sem(current->mm));
        trs_err("Unmap vm fail. (ret=%d)\n", ret);
        return ret;
    }
    up_write(get_mmap_sem(current->mm));

    return ret;
}

static inline bool trs_core_ops_is_kthread(void)
{
    return (current->mm == NULL) ? true : false;
}

#endif

int trs_core_ops_mmap(pid_t pid, phys_addr_t paddr, size_t size, u64 *va)
{
#ifndef EMU_ST
    bool is_kthread = trs_core_ops_is_kthread();
    int ret;

    if (is_kthread == true) {
        ret = trs_core_ops_mmap_remote(pid, paddr, size, va);
    } else {
        ret = trs_core_ops_mmap_local(pid, paddr, size, va);
    }

    return ret;
#else
    return 0;
#endif
}
EXPORT_SYMBOL(trs_core_ops_mmap);

int trs_core_ops_unmap(pid_t pid, u64 va, size_t size)
{
#ifndef EMU_ST
    bool is_kthread = trs_core_ops_is_kthread();
    int ret;

    if (is_kthread == true) {
        ret = trs_core_ops_unmap_remote(pid, va, size);
    } else {
        ret = trs_core_ops_unmap_local(pid, va, size);
    }
    return ret;
#else
    return 0;
#endif
}
EXPORT_SYMBOL(trs_core_ops_unmap);

int trs_core_ops_get_ts_inst_status(struct trs_id_inst *inst, u32 *status)
{
    struct res_inst_info res_inst;
    int ret;

    soc_resmng_inst_pack(&res_inst, inst->devid, TS_SUBSYS, inst->tsid);
    ret = soc_resmng_get_ts_status(&res_inst, status);
    if (ret != 0) {
        trs_err("Get ts status failed. (devid=%u; ret=%d)\n", inst->devid, ret);
    }

    return ret;
}
EXPORT_SYMBOL(trs_core_ops_get_ts_inst_status);
