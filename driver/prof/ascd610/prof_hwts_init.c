/*
* Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
* Create: 2022-08-25
*/
#ifndef PROF_UNIT_TEST

#include <linux/types.h>
#include <linux/irq.h>
#include <linux/interrupt.h>

#include "soc_res.h"

#include "prof_drv_dev.h"
#include "prof_hwts_init.h"

#define HWTS_DFX_PROFILE_PTR_REG_ADDR                             0x98108
#define HWTS_DFX_LOG_PTR_REG_ADDR                                 0x9A108
#define PTR_REG_SIZE (32 * 0x20)

#define HWTS_AIC_BUF_UNIT            128
#define HWTS_LOG_BUF_UNIT             64

#define STRIDE_PER_VIRTUAL_MACHINE  0x20
#define HWTS_RPTR_SHIFT_NUM           16
#define HWTS_WPTR_MASK            0xFFFF

struct hwts_int_context {
    u64 irq_count;
    u32 irq_id;
    u32 device_id;
    u32 channel_id;
    u32 tsid;
    u32 buf_unit;
    spinlock_t lock;
    void __iomem *ptr_reg_vaddr;
    struct tasklet_struct prof_irq_task;
};

static struct hwts_int_context aicore_int_ctx[PROF_DEVICE_NUM_VALUE];
static struct hwts_int_context hwts_log_int_ctx[PROF_DEVICE_NUM_VALUE];

static u32 device_flag[PROF_DEVICE_NUM_VALUE];

void prof_set_device_flag_vm_or_phy(u32 device_id, u32 flag)
{
    prof_debug("Set vm_or_phy flag. (device_id=%u; flag=%u).\n", device_id, flag);
    device_flag[device_id] = flag;
}

static int prof_remap_hwts_reg(u32 devid, u32 tsid, struct hwts_int_context *pctx)
{
    struct res_inst_info inst = {.devid = devid, .sub_type = TS_SUBSYS, .subid = tsid};
    struct soc_reg_base_info hwts_reg_base_info;
    u32 channel_id = pctx->channel_id;
    void __iomem *io_base = NULL;
    u64 paddr;
    int ret;

    if (pctx->ptr_reg_vaddr != NULL) {
        prof_err("It has been mapped. (devid=%u; channel_id=%u; tsid=%u)\n", devid, channel_id, tsid);
        return PROF_ERROR;
    }

    ret = soc_resmng_get_reg_base(&inst, "TS_HWTS_REG", &hwts_reg_base_info);
    if (ret != 0) {
        prof_err("Failed to get hwts reg base. (devid=%u; channel_id=%u; tsid=%u)\n", devid, channel_id, tsid);
        return ret;
    }

    if (channel_id == CHANNEL_AICORE) {
        paddr = hwts_reg_base_info.io_base + HWTS_DFX_PROFILE_PTR_REG_ADDR;
    } else if (channel_id == CHANNEL_HWTS_LOG) {
        paddr = hwts_reg_base_info.io_base + HWTS_DFX_LOG_PTR_REG_ADDR;
    }
    paddr += (devid * PROF_CHIP_ADDR_OFFSET);

    io_base = ioremap(paddr, PTR_REG_SIZE);
    if (io_base == NULL) {
        prof_err("Invoke ioremap failed. (devid=%u; channel_id=%u; tsid=%u)\n", devid, channel_id, tsid);
        return -ENOMEM;
    }
    pctx->ptr_reg_vaddr = io_base;
    return 0;
}

static void prof_unmap_hwts_reg(struct hwts_int_context *pctx)
{
    void *base_va = NULL;

    spin_lock_bh(&pctx->lock);
    if (pctx->ptr_reg_vaddr != NULL) {
        base_va = pctx->ptr_reg_vaddr;
        pctx->ptr_reg_vaddr = NULL;
    }
    spin_unlock_bh(&pctx->lock);

    if (base_va != NULL) {
        mb();
        iounmap(base_va);
    }
    prof_info("Unmap reg success. (device_id=%u; channel_id=%u; tsid=%u).\n",
        pctx->device_id, pctx->channel_id, pctx->tsid);
}

static void get_rptr_wptr(u64 reg_val, u16 *rptr, u16 *wptr)
{
    *rptr = reg_val >> HWTS_RPTR_SHIFT_NUM;
    *wptr = reg_val & HWTS_WPTR_MASK;
}

static void prof_almost_full_irq_task_func(unsigned long data)
{
    struct hwts_int_context *pctx = (struct hwts_int_context *)((uintptr_t)data);
    struct prof_cq_scheduler cq_scheduler;
    u32 device_id = pctx->device_id;
    u32 channel_id = pctx->channel_id;
    u32 offset, actual_vm_num;
    u64 rwptr_addr, value;
    u16 rptr, wptr;

    pctx->irq_count++;
    cq_scheduler.device_id = device_id;
    cq_scheduler.channel_id = channel_id;

    if (device_flag[device_id] == PROF_DEV_IS_PHY) {
        actual_vm_num = 1;
    } else {
        actual_vm_num = PROF_VFID_NUM_MAX - 1;
    }
    prof_debug("Recv one interrupt. (device_id=%u; channel_id=%u; vm_num=%u; irq count=%llu).\n",
        device_id, channel_id, actual_vm_num, pctx->irq_count);

    for (offset = 0; offset < actual_vm_num; offset++) {
        spin_lock_bh(&pctx->lock);
        if (pctx->ptr_reg_vaddr == NULL) {
            spin_unlock_bh(&pctx->lock);
            return;
        }
        rwptr_addr = (u64)pctx->ptr_reg_vaddr + offset * STRIDE_PER_VIRTUAL_MACHINE;
        value = readq((void __iomem *)rwptr_addr);
        spin_unlock_bh(&pctx->lock);

        get_rptr_wptr(value, (u16 *)&rptr, (u16 *)&wptr);
        if (wptr != rptr) {
            if (device_flag[device_id] == PROF_DEV_IS_PHY) {
                cq_scheduler.vfid = 0;
            } else {
                cq_scheduler.vfid = offset + 1;
            }
            cq_scheduler.sub_channel_id = cq_scheduler.vfid;
            prof_cq_callback_profile(device_id, pctx->tsid, (unsigned char *)&cq_scheduler, NULL);
        }
    }
}

static void prof_update_hwts_rptr(struct prof_sub_channel_info *sub_channel_info)
{
    struct prof_data_head *data_head = (struct prof_data_head *)sub_channel_info->vir_addr;
    u32 device_id = sub_channel_info->device_id;
    u32 channel_id = sub_channel_info->channel_id;
    u32 offset = sub_channel_info->vfid;
    struct hwts_int_context *pctx;
    u64 rwptr_addr, new_ptr, value;
    u16 rptr, wptr;

    if (channel_id == CHANNEL_AICORE) {
        pctx = &aicore_int_ctx[device_id];
    } else {
        pctx = &hwts_log_int_ctx[device_id];
    }

    if (offset > PROF_PHYSICAL_MACHINE_VFID) {
        offset -= 1;
    }

    spin_lock_bh(&pctx->lock);
    if (pctx->ptr_reg_vaddr == NULL) {
        spin_unlock_bh(&pctx->lock);
        return;
    }
    rwptr_addr = (u64)pctx->ptr_reg_vaddr + offset * STRIDE_PER_VIRTUAL_MACHINE;
    value = readq((void __iomem *)rwptr_addr);
    get_rptr_wptr(value, &rptr, &wptr);

    rptr = (u16)(data_head->read_ptr / pctx->buf_unit);
    new_ptr = ((u64)rptr) << HWTS_RPTR_SHIFT_NUM;
    new_ptr |= ((u64)wptr);
    writeq(new_ptr, (void __iomem *)rwptr_addr);
    spin_unlock_bh(&pctx->lock);

    prof_debug("Debug. (vaddr=0x%pK; rwptr_addr=0x%pK; read_ptr=%u; unit=%u; vfid=%u; offset=%u; value=0x%llx).\n",
        (void *)pctx->ptr_reg_vaddr, (void *)rwptr_addr, data_head->read_ptr, pctx->buf_unit,
        sub_channel_info->vfid, offset, value);
}

int prof_refresh_rw_ptr(struct prof_sub_channel_info *sub_channel_info, int refresh_type)
{
    u32 device_id = sub_channel_info->device_id;
    u32 vfid = sub_channel_info->vfid;
    u32 channel_id = sub_channel_info->channel_id;
    u32 tsid = sub_channel_info->ts_channel.tsid;

    prof_debug("Refresh read or write pointer. (device_id=%u; vfid=%u; channel_id=%u; tsid=%u; type=%d).\n",
        device_id, vfid, channel_id, tsid, refresh_type);
    if (refresh_type == TS_SYNC_READ_PTR) {
        prof_update_hwts_rptr(sub_channel_info);
    }

    return 0;
}

static irqreturn_t prof_almost_full_irq_handler(int irq, void *data)
{
    struct hwts_int_context *int_context = NULL;
    unsigned long flags;

    local_irq_save(flags);
    int_context = (struct hwts_int_context *)data;
    tasklet_schedule(&int_context->prof_irq_task);
    local_irq_restore(flags);

    return IRQ_HANDLED;
}

static int prof_almost_full_register_irq(u32 devid, u32 tsid, struct hwts_int_context *pctx)
{
    struct res_inst_info inst = {.devid = devid, .sub_type = TS_SUBSYS, .subid = tsid};
    u32 channel_id = pctx->channel_id;
    u32 cpu_id, irq_id, irq_type;
    int ret;

    if (channel_id == CHANNEL_AICORE) {
        irq_type = TS_PROF_AICORE_IRQ;
    } else if (channel_id == CHANNEL_HWTS_LOG) {
        irq_type = TS_PROF_HWTS_LOG_IRQ;
    }
    ret = soc_resmng_get_irq_by_index(&inst, irq_type, 0, &irq_id);
    if (ret != 0) {
        prof_err("Get irq failed. (devid=%u; channel_id=%u; ret=%d)\n", devid, channel_id, ret);
        return ret;
    }

    tasklet_init(&pctx->prof_irq_task, prof_almost_full_irq_task_func, (unsigned long)(uintptr_t)pctx);
    if (channel_id == CHANNEL_AICORE) {
        ret = request_irq(irq_id, prof_almost_full_irq_handler, 0, "prof_aicore", (void *)pctx);
    } else if (channel_id == CHANNEL_HWTS_LOG) {
        ret = request_irq(irq_id, prof_almost_full_irq_handler, 0, "prof_hwts_log", (void *)pctx);
    }
    if (ret != 0) {
        tasklet_kill(&pctx->prof_irq_task);
        prof_err("Register irq failed. (devid=%u; channel_id=%u; ret=%d)\n", devid, channel_id, ret);
        return ret;
    }

    cpu_id = prof_get_affinity_cpuid(devid);
    ret = prof_set_irq_affinity(irq_id, get_cpu_mask(cpu_id));
    if (ret != 0) {
        prof_err("Set irq affinity failed. (devid=%u; channel_id=%u; ret=%d)\n", devid, channel_id, ret);
        free_irq(irq_id, (void *)pctx);
        tasklet_kill(&pctx->prof_irq_task);
        return ret;
    }

    pctx->irq_id = irq_id;
    return 0;
}

static void prof_hwts_unregister_irq(struct hwts_int_context *pctx)
{
    if (pctx->irq_id != 0) {
        (void)prof_clear_irq_affinity(pctx->irq_id);
        free_irq(pctx->irq_id, (void *)pctx);
        tasklet_kill(&pctx->prof_irq_task);
        pctx->irq_id = 0;
        prof_info("Unregister irq success. (device_id=%u; channel_id=%u; tsid=%u).\n",
            pctx->device_id, pctx->channel_id, pctx->tsid);
    }
}

static int prof_init_hwts_int_context(u32 device_id, u32 tsid, u32 channel_id)
{
    struct hwts_int_context *pctx = NULL;
    int ret;

    if (channel_id == CHANNEL_AICORE) {
        pctx = &aicore_int_ctx[device_id];
        pctx->channel_id = CHANNEL_AICORE;
        pctx->buf_unit = HWTS_AIC_BUF_UNIT;
    } else if (channel_id == CHANNEL_HWTS_LOG) {
        pctx = &hwts_log_int_ctx[device_id];
        pctx->channel_id = CHANNEL_HWTS_LOG;
        pctx->buf_unit = HWTS_LOG_BUF_UNIT;
    }
    pctx->device_id = device_id;
    pctx->tsid = tsid;
    spin_lock_init(&pctx->lock);

    ret = prof_remap_hwts_reg(device_id, tsid, pctx);
    if (ret != 0) {
        prof_err("Remap hwts reg fail. (device_id=%u; channel_id=%u; tsid=%u).\n", device_id, channel_id, tsid);
        return ret;
    }

    ret = prof_almost_full_register_irq(device_id, tsid, pctx);
    if (ret != 0) {
        prof_unmap_hwts_reg(pctx);
        prof_err("Register hwts irq fail. (device_id=%u; channel_id=%u; tsid=%u).\n", device_id, channel_id, tsid);
        return ret;
    }

    prof_info("Init hwts int ctx success. (device_id=%u; channel_id=%u; tsid=%u).\n", device_id, channel_id, tsid);
    return 0;
}

static void prof_uninit_hwts_int_context(u32 device_id, u32 tsid, u32 channel_id)
{
    struct hwts_int_context *pctx = NULL;

    if (channel_id == CHANNEL_AICORE) {
        pctx = &aicore_int_ctx[device_id];
    } else if (channel_id == CHANNEL_HWTS_LOG) {
        pctx = &hwts_log_int_ctx[device_id];
    }

    prof_hwts_unregister_irq(pctx);
    prof_unmap_hwts_reg(pctx);
}

void prof_uninit_almost_full_irq(u32 device_id, u32 tsid)
{
    prof_uninit_hwts_int_context(device_id, tsid, CHANNEL_AICORE);
    prof_uninit_hwts_int_context(device_id, tsid, CHANNEL_HWTS_LOG);
    prof_info("Uninit irq success. (device_id=%u; tsid=%u).\n", device_id, tsid);
}

int prof_init_almost_full_irq(u32 device_id, u32 tsid)
{
    int ret;

    ret = prof_init_hwts_int_context(device_id, tsid, CHANNEL_AICORE);
    if (ret != 0) {
        prof_err("Init aicore context fail. (device_id=%u; tsid=%u).\n", device_id, tsid);
        return ret;
    }

    ret = prof_init_hwts_int_context(device_id, tsid, CHANNEL_HWTS_LOG);
    if (ret != 0) {
        prof_uninit_hwts_int_context(device_id, tsid, CHANNEL_AICORE);
        prof_err("Init hwts log context fail. (device_id=%u; tsid=%u).\n", device_id, tsid);
        return ret;
    }

    prof_info("Init irq success. (device_id=%u; tsid=%u).\n", device_id, tsid);
    return 0;
}

#else
int prof_hwts_init_ut_test(void)
{
    return 0;
}
#endif
