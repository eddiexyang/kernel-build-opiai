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
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/errno.h>
#include <linux/list.h>
#include <linux/platform_device.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>
#ifdef AOS_LLVM_BUILD
#include <linux/time64.h>
#include <linux/iommu.h>
#include <aos_dma.h>
#endif

#include "kernel_cgroup_mem_adapt.h"
#include "securec.h"
#include "devdrv_manager_comm.h"
#include "devdrv_driver_pm.h"
#include "devdrv_interface.h"
#include "tsdrv_drvops.h"
#include "tsdrv_device.h"
#include "devdrv_devinit.h"
#include "devdrv_id.h"
#include "tsdrv_osal_mm.h"
#include "tsdrv_id.h"
#include "tsdrv_sync.h"
#include "tsdrv_ctx.h"
#include "devdrv_cb.h"
#include "tsdrv_get_ssid.h"
#include "tsdrv_common.h"
#include "tsdrv_vsq.h"
#include "devdrv_cqsq.h"
#include "devdrv_common.h"
#include "logic_cq.h"

#ifndef CFG_SOC_PLATFORM_MDC_V51
#include "hvtsdrv_cqsq.h"
#include "hvtsdrv_tsagent.h"
#endif
#include "hvtsdrv_id.h"

#ifndef page_to_virt
#define page_to_virt(page) __va(page_to_pfn(page) << PAGE_SHIFT)
#endif

#define SQCQ_UMAX           0xFFFFFFFF
#define SQ_REUSE_SUCCESS    0
#define KERNEL_CHAN_TYPE    0
#define USER_CHAN_TYPE      1
#define NO_RES              (-14)

struct tsdrv_cqsq_mem_alloc_free_para {
    u32 dev_id;
    u32 tsid;
    u32 id;
    u32 id_type;
    u32 size;
    u32 addr_side;
    phys_addr_t virt_addr;
    phys_addr_t phy_addr;
    phys_addr_t bar_addr;
};

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
static inline void *dma_zalloc_coherent(struct device *dev, size_t size, dma_addr_t *dma_handle, gfp_t flag)
{
    void *ret = dma_alloc_coherent(dev, size, dma_handle, flag | __GFP_ZERO);
    return ret;
}
#endif

static void tsdrv_get_sq_head(u32 devid, u32 tsid, u32 sqid, u32 *sq_head);

STATIC int devdrv_cq_init(u32 devid, u32 tsid, u32 num_cq)
{
    struct devdrv_cq_sub_info *cq_sub_info = NULL;
    struct devdrv_ts_cq_info *cq_info = NULL;
    struct tsdrv_ts_resource *ts_res = NULL;
    struct tsdrv_id_info *cq_id_info = NULL;
    phys_addr_t info_mem_addr;
    unsigned long size;
    u32 cq_min, cq_max;
    int err;
    u32 i;

    ts_res = tsdrv_get_ts_resoruce(devid, TSDRV_PM_FID, tsid);
    if (ts_res == NULL) {
        return -ENODEV;
    }
    if (!list_empty_careful(&ts_res->id_res[TSDRV_CQ_ID].id_available_list)) {
        return -ENODEV;
    }

    err = tsdrv_get_ts_id_range(devid, tsid, TSDRV_CQ_ID, &cq_min, &cq_max);
    if (err != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("get ts id range fail, devid(%u) tsid(%u)\n", devid, tsid);
        return -ENODEV;
#endif
    }
    TSDRV_PRINT_DEBUG("devid(%u) tsid(%u) cq_min(%u) cq_max(%u) num_cq(%u)\n", devid, tsid, cq_min, cq_max, num_cq);

    info_mem_addr = ts_res->mem_info[DEVDRV_INFO_MEM].virt_addr;
    ts_res->id_res[TSDRV_CQ_ID].id_available_num = 0;

    size = (long)(unsigned)sizeof(struct devdrv_cq_sub_info) * num_cq;
    cq_sub_info = vzalloc(size);
    if (cq_sub_info == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("malloc failed.\n");
        return -ENOMEM;
#endif
    }

    cq_id_info = kzalloc(sizeof(struct tsdrv_id_info) * num_cq, GFP_KERNEL);
    if (cq_id_info == NULL) {
#ifndef TSDRV_UT
        vfree(cq_sub_info);
        TSDRV_PRINT_ERR("malloc failed.\n");
        return -ENOMEM;
#endif
    }

    ts_res->id_res[TSDRV_CQ_ID].pm_id = kzalloc(num_cq * sizeof(u32), GFP_KERNEL);
    if (ts_res->id_res[TSDRV_CQ_ID].pm_id == NULL) {
#ifndef TSDRV_UT
        vfree(cq_sub_info);
        kfree(cq_id_info);
        TSDRV_PRINT_ERR("malloc failed.\n");
        return -ENOMEM;
#endif
    }

    ts_res->cq_sub_addr = (void *)cq_sub_info;
    ts_res->id_res[TSDRV_CQ_ID].id_addr = cq_id_info;

    for (i = 0; i < num_cq; i++) {
        cq_info = devdrv_calc_cq_info(info_mem_addr, i);
        cq_info->head = 0;
        cq_info->tail = 0;
        cq_info->phase = 1;
        cq_info->index = i;
        cq_info->slot_size = DEVDRV_CQ_SLOT_SIZE;
        cq_info->depth = DEVDRV_MAX_CQ_DEPTH;
        cq_info->count_report = 0;
        cq_info->vfid = TSDRV_PM_FID;
        cq_info->uio_fd = DEVDRV_INVALID_FD_OR_NUM;
        cq_info->cq_sub = (void *)(cq_sub_info + i);
        cq_info->alloc_status = SQCQ_INACTIVE;
        cq_info->receive_count = 0;

        cq_sub_info[i].ctx = NULL;
        cq_sub_info[i].index = cq_info->index;
        cq_sub_info[i].virt_addr = (phys_addr_t)NULL;
        cq_sub_info[i].phy_addr = (phys_addr_t)NULL;

        cq_id_info[i].ctx = NULL;
        cq_sub_info[i].chan = NULL;
        cq_sub_info[i].complete_handle = NULL;
        cq_id_info[i].devid = devid;
        cq_id_info[i].phy_id = i;
        cq_id_info[i].id = cq_id_info[i].phy_id;
        ts_res->id_res[TSDRV_CQ_ID].pm_id[i] = cq_id_info[i].phy_id;
        atomic_set(&cq_id_info[i].ref, 0);
        spin_lock_init(&cq_id_info[i].spinlock);
        INIT_LIST_HEAD(&cq_sub_info[i].list_sq);

        /* The ID resources are initialized on the device side. Therefore, the value of max
         * and min on the host side is 0.
         */
        if ((i >= cq_min) && (i < cq_max)) {
            /* IDs 334 to 351 are reserved for callback */
            if (i < (DEVDRV_MAX_CQ_NUM - CALLBACK_MAX_CQ_NUM)) {
                list_add_tail(&cq_id_info[i].list, &ts_res->id_res[TSDRV_CQ_ID].id_available_list);
                ts_res->id_res[TSDRV_CQ_ID].id_available_num++;
            }
        } else if (i >= (DEVDRV_MAX_CQ_NUM - CALLBACK_MAX_CQ_NUM)) {
            /* init callback cq_info */
            cq_info->slot_size = DEVDRV_MAX_CBCQ_SIZE;
            if (i != (DEVDRV_MAX_CQ_NUM - 1)) {
                cq_info->vfid = DEVDRV_MAX_CQ_NUM - i - CALLBACK_PHYSICAL_CQ_NUM;
                cq_sub_info[i].callback_sq_index = DEVDRV_INVALID_CB_SQ_ID;
            } else {
                cq_info->vfid = TSDRV_PM_FID;
            }
            TSDRV_PRINT_DEBUG("cbcq_id=%d vfid=%u.\n", cq_id_info[i].phy_id, cq_info->vfid);
        }
        spin_lock_init(&cq_sub_info[i].spinlock);
    }

    return 0;
}

int devdrv_cqsq_init(u32 devid, u32 tsid, u32 num_sq, u32 num_cq)
{
    struct devdrv_sq_sub_info *sq_sub_info = NULL;
    struct devdrv_ts_sq_info *sq_info = NULL;
    struct tsdrv_id_info *sq_id_info = NULL;
    struct tsdrv_ts_resource *ts_res = NULL;
    phys_addr_t info_mem_addr;
    u32 sq_min, sq_max;
    unsigned long size;
    int err;
    u32 i;

    ts_res = tsdrv_get_ts_resoruce(devid, TSDRV_PM_FID, tsid);
    if (ts_res == NULL) {
        return -ENODEV;
    }
    if (!list_empty_careful(&ts_res->id_res[TSDRV_SQ_ID].id_available_list)) {
        return -ENODEV;
    }
    err = tsdrv_get_ts_id_range(devid, tsid, TSDRV_SQ_ID, &sq_min, &sq_max);
    if (err != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("get ts id range fail, devid(%u) tsid(%u)\n", devid, tsid);
        return -ENODEV;
#endif
    }
    TSDRV_PRINT_DEBUG("devid(%u) tsid(%u) sq_min(%u) sq_max(%u)\n", devid, tsid, sq_min, sq_max);

    info_mem_addr = ts_res->mem_info[DEVDRV_INFO_MEM].virt_addr;

    ts_res->id_res[TSDRV_SQ_ID].id_available_num = 0;
    size = (long)(unsigned)sizeof(struct devdrv_sq_sub_info) * num_sq;
    sq_sub_info = vzalloc(size);
    if (sq_sub_info == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("malloc failed.\n");
        return -ENOMEM;
#endif
    }

    sq_id_info = kzalloc(sizeof(struct tsdrv_id_info) * num_sq, GFP_KERNEL);
    if (sq_id_info == NULL) {
#ifndef TSDRV_UT
        vfree(sq_sub_info);
        TSDRV_PRINT_ERR("malloc failed.\n");
        return -ENOMEM;
#endif
    }

    ts_res->id_res[TSDRV_SQ_ID].pm_id = kzalloc(num_sq * sizeof(u32), GFP_KERNEL);
    if (ts_res->id_res[TSDRV_SQ_ID].pm_id == NULL) {
#ifndef TSDRV_UT
        vfree(sq_sub_info);
        kfree(sq_id_info);
        TSDRV_PRINT_ERR("malloc failed.\n");
        return -ENOMEM;
#endif
    }

    for (i = 0; i < num_sq; i++) {
        sq_info = devdrv_calc_sq_info(info_mem_addr, i);
        sq_info->index = i;
        sq_info->head = 0;
        sq_info->tail = 0;
        sq_info->send_count = 0;
        sq_info->depth = DEVDRV_MAX_SQ_DEPTH;
        sq_info->alloc_status = SQCQ_INACTIVE;
        sq_info->uio_fd = DEVDRV_INVALID_FD_OR_NUM;
        sq_info->sq_sub = (void *)(sq_sub_info + i);
        sq_info->bind_cqid = DEVDRV_MAX_CQ_NUM;
        sq_sub_info[i].index = sq_info->index;
        sq_sub_info[i].vaddr = 0;
        sq_sub_info[i].phy_addr = (phys_addr_t)NULL;
        sq_sub_info[i].bar_addr = (phys_addr_t)NULL;
        sq_id_info[i].id = i;
        sq_id_info[i].phy_id = i;
        sq_id_info[i].devid = devid;
        sq_id_info[i].ctx = NULL;
        ts_res->id_res[TSDRV_SQ_ID].pm_id[i] = sq_id_info[i].phy_id;
        atomic_set(&sq_id_info[i].ref, 0);
        spin_lock_init(&sq_id_info[i].spinlock);
        if ((i >= sq_min) && (i < sq_max)) {
            list_add_tail(&sq_id_info[i].list, &ts_res->id_res[TSDRV_SQ_ID].id_available_list);
            ts_res->id_res[TSDRV_SQ_ID].id_available_num++;
        }
    }

    err = devdrv_cq_init(devid, tsid, num_cq);
    if (err != 0) {
#ifndef TSDRV_UT
        vfree(sq_sub_info);
        kfree(sq_id_info);
        kfree(ts_res->id_res[TSDRV_SQ_ID].pm_id);
        ts_res->id_res[TSDRV_SQ_ID].pm_id = NULL;
        TSDRV_PRINT_ERR("malloc failed.\n");
        return -ENODEV;
#endif
    }

    ts_res->sq_sub_addr = (void *)sq_sub_info;
    ts_res->id_res[TSDRV_SQ_ID].id_addr = sq_id_info;

    return 0;
}

void devdrv_cqsq_destroy(u32 devid, u32 tsid)
{
    struct tsdrv_ts_resource *ts_res = NULL;

    ts_res = tsdrv_get_ts_resoruce(devid, TSDRV_PM_FID, tsid);

    INIT_LIST_HEAD(&ts_res->id_res[TSDRV_SQ_ID].id_available_list);
    INIT_LIST_HEAD(&ts_res->id_res[TSDRV_CQ_ID].id_available_list);

    vfree(ts_res->sq_sub_addr);
    ts_res->sq_sub_addr = NULL;
    vfree(ts_res->cq_sub_addr);
    ts_res->cq_sub_addr = NULL;

    kfree(ts_res->id_res[TSDRV_SQ_ID].id_addr);
    ts_res->id_res[TSDRV_SQ_ID].id_addr = NULL;
    kfree(ts_res->id_res[TSDRV_CQ_ID].id_addr);
    ts_res->id_res[TSDRV_CQ_ID].id_addr = NULL;

    kfree(ts_res->id_res[TSDRV_SQ_ID].pm_id);
    ts_res->id_res[TSDRV_SQ_ID].pm_id = NULL;
    kfree(ts_res->id_res[TSDRV_CQ_ID].pm_id);
    ts_res->id_res[TSDRV_CQ_ID].pm_id = NULL;

    return;
}

STATIC u32 devdrv_get_available_cq_num(struct tsdrv_ts_resource *ts_res)
{
    u32 cq_num;

    spin_lock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
    cq_num = ts_res->id_res[TSDRV_CQ_ID].id_available_num;
    spin_unlock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);

    return cq_num;
}

void devdrv_calc_irq_cq_range(u32 irq_id, u32 irq_num, u32 cq_num, u32 *first_cq_index, u32 *last_cq_index)
{
    if (tsdrv_get_env_type() == TSDRV_ENV_OFFLINE) {
        *first_cq_index = irq_id * DEVDRV_CQ_PER_IRQ;
#ifdef CFG_SOC_PLATFORM_MINIV2
        *last_cq_index = *first_cq_index + DEVDRV_CQ_PER_IRQ - 1;
#else
        *last_cq_index = *first_cq_index + DEVDRV_CQ_PER_IRQ - 1;
#endif
    } else {
        if (irq_num == 0) {
            TSDRV_PRINT_ERR("irq number is zero\n");
            return;
        }
        *first_cq_index = (cq_num / irq_num) * irq_id;
        *last_cq_index = ((cq_num / irq_num) * (irq_id + 1)) - 1;

        /* The last interrupt needs to process all remaining CQs */
        if (irq_id == (irq_num - 1)) {
            *last_cq_index = cq_num - 1;
        }
    }

    TSDRV_PRINT_FPGA("first(%u), last(%u)\n", *first_cq_index, *last_cq_index);
}

void devdrv_calc_cq_irq_id(u32 cq_index, u32 irq_num, u32 cq_num, u16 *irq_id)
{
    u32 cq_irq_num = 0;

    if (tsdrv_get_env_type() == TSDRV_ENV_OFFLINE) {
        *irq_id = cq_index / DEVDRV_CQ_PER_IRQ;
    } else {
#if defined(CFG_SOC_PLATFORM_MINI) && !defined(CFG_SOC_PLATFORM_MINIV2) && !defined(CFG_SOC_PLATFORM_MINIV3)
        /* in ascend310, reserve the last irq for cbcq */
        cq_irq_num = irq_num - 1;
#else
        cq_irq_num = irq_num;
#endif

        *irq_id = cq_index / (cq_num / cq_irq_num);
        if (*irq_id >= cq_irq_num) {
            *irq_id = cq_irq_num - 1;
        }
    }
}

void tsdrv_trigger_phy_cq_scan(u32 devid, u32 tsid, u32 phy_cqid)
{
#ifndef TSDRV_UT
    struct tsdrv_ts_resource *ts_res = tsdrv_get_ts_resoruce(devid, TSDRV_PM_FID, tsid);
    struct tsdrv_cq_hwinfo *cq_hwinfo;
    u16 index; // physical cq irq index;

    cq_hwinfo = tsdrv_get_cq_hwinfo(devid, tsid);
    devdrv_calc_cq_irq_id(phy_cqid, cq_hwinfo->cq_irq_num, DEVDRV_MAX_CQ_NUM, &index);

    if (index >= DEVDRV_CQ_IRQ_NUM) {
        TSDRV_PRINT_DEBUG("Get invalid index. (devid=%u; tsid=%u; phy_cqid=%u; cq_irq_num=%u; index=%u; "
            "DEVDRV_MAX_CQ_NUM=%u)\n", devid, tsid, phy_cqid, cq_hwinfo->cq_irq_num, (u32)index,
            (u32)DEVDRV_MAX_CQ_NUM);
        return;
    }
    tasklet_schedule(&ts_res->int_context[index].find_cq_task);
    TSDRV_PRINT_DEBUG("trigger phy_cq_scan. (devid=%u; tsid=%u; phy_cqid=%u; cq_irq_num=%u; index=%u; "
        "DEVDRV_MAX_CQ_NUM=%u)\n", devid, tsid, phy_cqid, cq_hwinfo->cq_irq_num, (u32)index,
        (u32)DEVDRV_MAX_CQ_NUM);
#endif
}

bool devdrv_is_ctx_has_normal_cq(struct tsdrv_ctx *ctx, u32 devid, u32 tsid)
{
    struct tsdrv_ts_resource *ts_res = tsdrv_ctx_to_ts_res(ctx, tsid);
    struct tsdrv_ts_ctx *ts_ctx = &ctx->ts_ctx[tsid];
    struct devdrv_ts_cq_info *cq_info = NULL;
    struct tsdrv_id_info *cq_id_info = NULL;
    struct tsdrv_id_info *n = NULL;

    spin_lock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
    if (list_empty_careful(&ts_ctx->id_ctx[TSDRV_CQ_ID].id_list)) {
        spin_unlock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
        return false;
    }

    list_for_each_entry_safe(cq_id_info, n, &ts_ctx->id_ctx[TSDRV_CQ_ID].id_list, list) {
        cq_info = devdrv_calc_cq_info(ts_res->mem_info[DEVDRV_INFO_MEM].virt_addr, cq_id_info->phy_id);
        if (cq_info->type == NORMAL_SQCQ_TYPE) {
            spin_unlock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
            TSDRV_PRINT_ERR("Context already has a normal cq. (devid=%u; cqid=%u)\n", devid, cq_id_info->phy_id);
            return true;
        }
    }

    spin_unlock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
    return false;
}

struct devdrv_ts_cq_info *devdrv_get_new_cq(u32 devid, u32 tsid, struct tsdrv_ctx *ctx)
{
    int ret;
    u32 cq_num;
    phys_addr_t info_mem_addr;
    struct tsdrv_ts_ctx *ts_ctx = NULL;
    struct tsdrv_ts_resource *ts_res = NULL;
    struct tsdrv_id_info *cq_id_info = NULL;
    struct devdrv_ts_cq_info *cq_info = NULL;
    struct devdrv_cq_sub_info *cq_sub = NULL;

    if ((devid >= TSDRV_MAX_DAVINCI_NUM) || (tsid >= DEVDRV_MAX_TS_NUM) || (ctx == NULL)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("invalid param, devid(%u) tsid(%u)\n", devid, tsid);
        return NULL;
#endif
    }

    ts_res = tsdrv_ctx_to_ts_res(ctx, tsid);
    ts_ctx = &ctx->ts_ctx[tsid];
    mutex_lock(&ts_res->id_res[TSDRV_CQ_ID].id_mutex_t);
    if (devdrv_is_ctx_has_normal_cq(ctx, devid, tsid)) {
        mutex_unlock(&ts_res->id_res[TSDRV_CQ_ID].id_mutex_t);
        return NULL;
    }
    cq_num = devdrv_get_available_cq_num(ts_res);
    if (cq_num == 0) {
        TSDRV_PRINT_DEBUG("NO cq, try to get cq from opposite side!!\n");
        if (tsdrv_is_in_pm(devid)) {
            ret = tsdrv_msg_alloc_sync_cq(devid, tsid);
            if (ret != 0) {
                mutex_unlock(&ts_res->id_res[TSDRV_CQ_ID].id_mutex_t);
                TSDRV_PRINT_ERR("[dev_id=%u]:get cq from opposite side failed\n", devid);
                return NULL;
            }
        } else {
#ifndef TSDRV_UT
            mutex_unlock(&ts_res->id_res[TSDRV_CQ_ID].id_mutex_t);
            TSDRV_PRINT_WARN("[dev_id=%u]:no cq available!\n", devid);
            return NULL;
#endif
        }
    }
    spin_lock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
    cq_id_info = list_first_entry(&ts_res->id_res[TSDRV_CQ_ID].id_available_list,
        struct tsdrv_id_info, list);
    list_del(&cq_id_info->list);
    list_add(&cq_id_info->list, &ts_ctx->id_ctx[TSDRV_CQ_ID].id_list);
    ts_res->id_res[TSDRV_CQ_ID].id_available_num--;
    ts_ctx->id_ctx[TSDRV_CQ_ID].id_num++;

    info_mem_addr = ts_res->mem_info[DEVDRV_INFO_MEM].virt_addr;
    cq_info = devdrv_calc_cq_info(info_mem_addr, cq_id_info->phy_id);
    cq_sub = cq_info->cq_sub;
    spin_lock_irq(&cq_sub->spinlock);
    cq_sub->ctx = ctx;
    spin_unlock_irq(&cq_sub->spinlock);
    spin_unlock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
    mutex_unlock(&ts_res->id_res[TSDRV_CQ_ID].id_mutex_t);

    TSDRV_PRINT_DEBUG("Alloc Cq succeed. (devid=%u; tsid=%u; phy_cqid=%d; virt_cqid=%u; cq_index=%u)\n",
        devid, tsid, cq_id_info->phy_id, cq_id_info->virt_id, cq_sub->index);
    return cq_info;
}

void devdrv_free_cq(struct tsdrv_ts_resource *ts_res, struct tsdrv_ctx *ctx, struct devdrv_ts_cq_info *cq_info)
{
    struct devdrv_cq_sub_info *cq_sub = cq_info->cq_sub;
#ifndef CFG_SOC_PLATFORM_MDC_V51
    u32 devid = tsdrv_get_devid_by_ctx(ctx);
#endif
    struct tsdrv_id_info *cq_id_info = NULL;
    struct tsdrv_ts_ctx *ts_ctx = NULL;

    ts_ctx = &ctx->ts_ctx[ts_res->tsid];

    spin_lock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
    if (list_empty_careful(&ts_ctx->id_ctx[TSDRV_CQ_ID].id_list) != 0) {
#ifndef TSDRV_UT
        spin_unlock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
        TSDRV_PRINT_ERR("cce_context cq_list empty.\n");
        return;
#endif
    }
    cq_id_info = list_first_entry(&ts_ctx->id_ctx[TSDRV_CQ_ID].id_list, struct tsdrv_id_info, list);

    if (cq_id_info->phy_id != cq_info->index) {
        spin_unlock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
        return;
    }

    if (list_empty_careful(&cq_sub->list_sq) != 0) {
        list_del(&cq_id_info->list);
        list_add(&cq_id_info->list, &ts_res->id_res[TSDRV_CQ_ID].id_available_list);

        ts_ctx->receive_count += cq_info->receive_count;
        ts_res->id_res[TSDRV_CQ_ID].id_available_num++;
        ts_ctx->id_ctx[TSDRV_CQ_ID].id_num--;

        spin_lock_irq(&cq_sub->spinlock);
        cq_info->type = NORMAL_SQCQ_TYPE;
        cq_sub->ctx = NULL;
        cq_info->alloc_status = SQCQ_INACTIVE;
        cq_info->head = 0;
        cq_info->tail = 0;
        cq_info->count_report = 0;
        cq_info->depth = DEVDRV_MAX_CQ_DEPTH;
        cq_info->receive_count = 0;
        cq_info->release_head = 0;
        cq_info->phase = DEVDRV_PHASE_STATE_0;
        cq_info->slot_size = DEVDRV_CQ_SLOT_SIZE;
        cq_info->uio_fd = DEVDRV_INVALID_FD_OR_NUM;
        spin_unlock_irq(&cq_sub->spinlock);
#ifndef CFG_SOC_PLATFORM_MDC_V51
        if (tsdrv_is_in_vm(devid)) {
            hvtsdrv_clear_vcq_info(ts_res, cq_id_info->virt_id);
        }
#endif
    }
    spin_unlock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
}

void tsdrv_free_sq(struct tsdrv_ts_resource *ts_res, struct tsdrv_ctx *ctx, struct devdrv_ts_sq_info *sq_info)
{
#ifndef CFG_SOC_PLATFORM_MDC_V51
    u32 devid = tsdrv_get_devid_by_ctx(ctx);
#endif
    struct tsdrv_id_info *sq_id_info = NULL;
    u32 fid;

    spin_lock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
    if (list_empty_careful(&ctx->ts_ctx[ts_res->tsid].id_ctx[TSDRV_SQ_ID].id_list) != 0) {
#ifndef TSDRV_UT
        spin_unlock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
        TSDRV_PRINT_ERR("cce_context sq list empty.\n");
        return;
#endif
    }

    sq_id_info = devdrv_find_one_id(&ts_res->id_res[TSDRV_SQ_ID], (int)sq_info->index);
    if (sq_id_info == NULL) {
        spin_unlock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
        TSDRV_PRINT_ERR("id(%u).\n", sq_info->index);
        return;
    }
    list_del(&sq_id_info->list);
    list_add(&sq_id_info->list, &ts_res->id_res[TSDRV_SQ_ID].id_available_list);
    ts_res->id_res[TSDRV_SQ_ID].id_available_num++;
    ctx->ts_ctx[ts_res->tsid].send_count += sq_info->send_count;
    ctx->ts_ctx[ts_res->tsid].id_ctx[TSDRV_SQ_ID].id_num--;

    sq_info->alloc_status = SQCQ_INACTIVE;
    sq_info->type = NORMAL_SQCQ_TYPE;
    sq_info->head = 0;
    sq_info->tail = 0;
    sq_info->depth = DEVDRV_MAX_SQ_DEPTH;
    sq_info->send_count = 0;
    sq_info->uio_fd = DEVDRV_INVALID_FD_OR_NUM;

    fid = tsdrv_get_fid_by_ctx(ctx);
#ifndef CFG_SOC_PLATFORM_MDC_V51
    if (!tsdrv_is_in_pm(devid)) {
        hvtsdrv_clear_vsq_info(ts_res, sq_id_info->virt_id);
    }
#endif
    spin_unlock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
}

struct devdrv_ts_cq_info *devdrv_get_cq_exist(struct tsdrv_ts_resource *ts_res,
    struct tsdrv_ts_ctx *ts_ctx)
{
    struct tsdrv_id_info *cq_id_info = NULL;
    phys_addr_t info_mem_addr;

    spin_lock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
    if (list_empty_careful(&ts_ctx->id_ctx[TSDRV_CQ_ID].id_list) != 0) {
        spin_unlock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
        return NULL;
    }
    info_mem_addr = ts_res->mem_info[DEVDRV_INFO_MEM].virt_addr;
    cq_id_info = list_first_entry(&ts_ctx->id_ctx[TSDRV_CQ_ID].id_list, struct tsdrv_id_info, list);
    spin_unlock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);

    return devdrv_calc_cq_info(info_mem_addr, cq_id_info->phy_id);
}

STATIC struct devdrv_ts_sq_info *devdrv_get_new_sq(struct tsdrv_ts_resource *ts_res, struct tsdrv_ts_ctx *ts_ctx)
{
    struct tsdrv_id_info *sq_id_info = NULL;
    phys_addr_t info_mem_addr;

    sq_id_info = list_first_entry(&ts_res->id_res[TSDRV_SQ_ID].id_available_list,
        struct tsdrv_id_info, list);
    list_del(&sq_id_info->list);
    list_add(&sq_id_info->list, &ts_ctx->id_ctx[TSDRV_SQ_ID].id_list);
    ts_res->id_res[TSDRV_SQ_ID].id_available_num--;
    ts_ctx->id_ctx[TSDRV_SQ_ID].id_num++;

    info_mem_addr = ts_res->mem_info[DEVDRV_INFO_MEM].virt_addr;

    return devdrv_calc_sq_info(info_mem_addr, sq_id_info->phy_id);
}

static bool is_sq_belong_to_proc(struct tsdrv_ctx *ctx, u32 tsid, u32 sqid)
{
    struct tsdrv_id_info *sq_id_info = NULL;

    if (!list_empty_careful(&ctx->ts_ctx[tsid].id_ctx[TSDRV_SQ_ID].id_list)) {
        list_for_each_entry(sq_id_info, &ctx->ts_ctx[tsid].id_ctx[TSDRV_SQ_ID].id_list, list) {
            if (sq_id_info->id == sqid) {
                return true;
            }
        }
    }
    return false;
}

static enum phy_sqcq_type tsdrv_get_sq_type(u32 devid, u32 fid, u32 tsid, u32 sqid)
{
    struct tsdrv_ts_resource *ts_res = tsdrv_get_ts_resoruce(devid, fid, tsid);
    struct devdrv_ts_sq_info *sq_info = NULL;
    phys_addr_t info_mem_addr;

    info_mem_addr = ts_res->mem_info[DEVDRV_INFO_MEM].virt_addr;
    sq_info = devdrv_calc_sq_info(info_mem_addr, ts_res->id_res[TSDRV_SQ_ID].pm_id[sqid]);

    return sq_info->type;
}

static int tsdrv_proc_sq_exist_check(struct tsdrv_ts_resource *ts_res, struct tsdrv_ctx *ctx,
    u32 sqId, u32 sq_type)
{
    u32 devid = tsdrv_get_devid_by_ctx(ctx);
    u32 fid = tsdrv_get_fid_by_ctx(ctx);
    u32 tsid = ts_res->tsid;

    spin_lock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
    if (!is_sq_belong_to_proc(ctx, tsid, sqId)) {
        spin_unlock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
        TSDRV_PRINT_ERR("Sqid is not belong to this ctx. (tsid=%u; sqid=%u)\n", tsid, sqId);
        return -ENODEV;
    }
    if ((enum phy_sqcq_type)sq_type != tsdrv_get_sq_type(devid, fid, tsid, sqId)) {
        spin_unlock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
        TSDRV_PRINT_ERR("Sq type mismatch. (sqid=%u; sq_type=%u)\n", sqId, sq_type);
        return -EFAULT;
    }
    spin_unlock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
    return 0;
}

int tsdrv_sq_exist_check(struct tsdrv_ts_resource *ts_res, struct tsdrv_ctx *ctx, u32 sqId)
{
    return tsdrv_proc_sq_exist_check(ts_res, ctx, sqId, NORMAL_SQCQ_TYPE);
}

int tsdrv_cq_exist_check(struct tsdrv_ts_resource *ts_res, struct tsdrv_ctx *ctx, u32 cqId)
{
    struct tsdrv_id_info *cq_id_info = NULL;
    struct list_head *pos = NULL;
    struct list_head *n = NULL;

    spin_lock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
    if (list_empty_careful(&ctx->ts_ctx[ts_res->tsid].id_ctx[TSDRV_CQ_ID].id_list) != 0) {
        spin_unlock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
        TSDRV_PRINT_ERR("cq_list is empty.\n");
        return -EINVAL;
    }

    list_for_each_safe(pos, n, &ctx->ts_ctx[ts_res->tsid].id_ctx[TSDRV_CQ_ID].id_list) {
        cq_id_info = list_entry(pos, struct tsdrv_id_info, list);
        if (cq_id_info->id == cqId) {
            spin_unlock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
            return 0;
        }
    }

    spin_unlock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
    TSDRV_PRINT_ERR("the cqid(%u) is invalid.\n", cqId);
    return -EINVAL;
}

#ifndef VM_ST_TEST

bool tsdrv_is_sq_belong_to_proc(struct tsdrv_ctx *ctx, u32 tsId, u32 sqId)
{
    struct tsdrv_ts_resource *ts_res = tsdrv_ctx_to_ts_res(ctx, tsId);

    spin_lock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
    if (!is_sq_belong_to_proc(ctx, tsId, sqId)) {
        spin_unlock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
        TSDRV_PRINT_ERR("Sqid is not belong to this ctx. (tsid=%u; sqid=%u)\n", tsId, sqId);
        return false;
    }
    spin_unlock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
    return true;
}

bool tsdrv_is_cq_belong_to_proc(struct tsdrv_ctx *ctx, u32 tsId, u32 cqId)
{
    struct tsdrv_ts_resource *ts_res = tsdrv_ctx_to_ts_res(ctx, tsId);
    bool ret;

    mutex_lock(&ts_res->id_res[TSDRV_CQ_ID].id_mutex_t);
    ret = (tsdrv_cq_exist_check(tsdrv_ctx_to_ts_res(ctx, tsId), ctx, cqId) == 0);
    mutex_unlock(&ts_res->id_res[TSDRV_CQ_ID].id_mutex_t);

    return ret;
}

int tsdrv_get_phy_sq_mem_phy_addr(struct tsdrv_ctx *ctx, u32 tsId, u32 sqId, struct tsdrv_phy_addr_get *info)
{
    struct tsdrv_ts_resource *ts_res = tsdrv_ctx_to_ts_res(ctx, tsId);
    struct devdrv_sq_sub_info *sq_sub = NULL;
    phys_addr_t paddr;

    sq_sub = tsdrv_get_sq_sub_info(ts_res, sqId);
    if ((info->offset + info->len) != (PAGE_ALIGN(sq_sub->size * sq_sub->depth))) {
        TSDRV_PRINT_ERR("Para is error. (sqId=%u; offset=%u; len=%u; sq_size=%lu)\n",
            sqId, info->offset, info->len, sq_sub->size * sq_sub->depth);
        return -EINVAL;
    }

    if (tsdrv_get_env_type() == TSDRV_ENV_OFFLINE) {
        paddr = sq_sub->phy_addr;
    } else {
        if (sq_sub->addr_side == TSDRV_MEM_ON_HOST_SIDE) {
            paddr = (phys_addr_t)virt_to_phys((void *)(uintptr_t)sq_sub->vaddr);
        } else {
            paddr = sq_sub->bar_addr;
        }
    }

    info->paddr = paddr;
    info->paddr_len = info->len;
    return 0;
}

int tsdrv_get_phy_cq_mem_phy_addr(struct tsdrv_ctx *ctx, u32 tsId, u32 cqId, struct tsdrv_phy_addr_get *info)
{
    struct tsdrv_ts_resource *ts_res = tsdrv_ctx_to_ts_res(ctx, tsId);
    struct tsdrv_device *ts_dev = tsdrv_res_to_dev(ts_res);
    struct devdrv_cq_sub_info *cq_sub = NULL;
    phys_addr_t paddr;
    u32 phy_cqId = cqId;

    if (cqId >= DEVDRV_MAX_CQ_NUM) {
        TSDRV_PRINT_ERR("Input pararmeter is error. (cqid=%u)\n", cqId);
        return -EINVAL;
    }

    if (!tsdrv_is_cq_belong_to_proc(ctx, tsId, cqId)) {
        TSDRV_PRINT_ERR("Not self cq. (cqid=%u)\n", cqId);
        return -EINVAL;
    }

    if (!tsdrv_is_in_pm(ts_dev->devid)) {
        phy_cqId = tsdrv_vrit_to_physic_id(&ts_res->id_res[TSDRV_CQ_ID], cqId);
    }

    cq_sub = tsdrv_get_cq_sub_info(ts_res, phy_cqId);
    if ((info->offset + info->len) != PAGE_ALIGN(cq_sub->size)) {
        TSDRV_PRINT_ERR("Para is error. (cqId=%u; offset=%u; len=%u; cq_size=%u)\n",
            phy_cqId, info->offset, info->len, cq_sub->size);
        return -EINVAL;
    }

    if (tsdrv_get_env_type() == TSDRV_ENV_OFFLINE) {
#if defined(CFG_SOC_PLATFORM_MINIV2) && !defined(CFG_SOC_PLATFORM_MINIV3)
        paddr = (phys_addr_t)CQ_RESERVE_MEM_BASE + ((phys_addr_t)tsId * CQ_RESERVE_MEM_SIZE) +
            ((phys_addr_t)cq_sub->index * cq_sub->size);
#else
        paddr = (phys_addr_t)virt_to_phys((void *)(uintptr_t)cq_sub->virt_addr);
#endif /* CFG_SOC_PLATFORM_MINIV2 */
    } else {
#ifdef CFG_SOC_PLATFORM_MINIV3
        paddr = cq_sub->bar_addr;
#else
        paddr = (phys_addr_t)virt_to_phys((void *)(uintptr_t)cq_sub->virt_addr);
#endif
    }

    info->paddr = paddr;
    info->paddr_len = info->len;
    TSDRV_PRINT_DEBUG("cq_id=%u, paddr=0x%pK, len=%u\n",
        cqId, (void *)(uintptr_t)info->paddr, info->paddr_len);
    return 0;
}
#ifndef AOS_LLVM_BUILD
int tsdrv_get_sq_mem_phy_addr(struct tsdrv_ctx *ctx, u32 tsId, u32 sqId, u32 sq_type, struct tsdrv_phy_addr_get *info)
{
    struct tsdrv_ts_resource *ts_res = tsdrv_ctx_to_ts_res(ctx, tsId);
    struct tsdrv_device *ts_dev = tsdrv_res_to_dev(ts_res);
    int ret;

    if (sqId >= DEVDRV_MAX_SQ_NUM) {
        TSDRV_PRINT_ERR("Input pararmeter is error. (sqid=%u)\n", sqId);
        return -EINVAL;
    }

    if ((tsdrv_is_in_container(ts_dev->devid) && (sq_type == MEM_MAP_SUBTYPE_SHM))) {
        return tsdrv_get_phy_sq_mem_phy_addr(ctx, tsId, sqId, info);
    }

    if (!tsdrv_is_sq_belong_to_proc(ctx, tsId, sqId)) {
        TSDRV_PRINT_ERR("Not self sq. (sqid=%u; sq_type=%u)\n", sqId, sq_type);
        return -EINVAL;
    }

    if (tsdrv_is_in_pm(ts_dev->devid)) {
#ifndef CFG_FEATURE_SUPPORT_VSQ_MNG
        ret = tsdrv_get_phy_sq_mem_phy_addr(ctx, tsId, sqId, info);
#else
        ret = tsdrv_get_vsq_phy_addr(ctx, tsId, sqId, info);
#endif
    } else {
#ifndef CFG_SOC_PLATFORM_MDC_V51
        ret = hvtsdrv_get_vsq_mem_phy_addr(ctx, tsId, sqId, info);
#else
        ret = -EINVAL;
#endif
    }

    return ret;
}

int tsdrv_get_cq_mem_phy_addr(struct tsdrv_ctx *ctx, u32 tsId, u32 cqId, u32 cq_type, struct tsdrv_phy_addr_get *info)
{
    struct tsdrv_ts_resource *ts_res = tsdrv_ctx_to_ts_res(ctx, tsId);
    struct tsdrv_device *ts_dev = tsdrv_res_to_dev(ts_res);
    int ret;

    if (cq_type == MEM_MAP_SUBTYPE_CB) {
        ret = tsdrv_get_cb_cq_mem_phy_addr(ts_dev->devid, tsId, cqId, info);
    } else {
        ret = tsdrv_get_phy_cq_mem_phy_addr(ctx, tsId, cqId, info);
    }

    return ret;
}

int tsdrv_get_sq_mem_map_prot(struct tsdrv_ctx *ctx, u32 tsId, u32 sq_type, pgprot_t vm_page_prot, pgprot_t *prot)
{
#ifdef CONFIG_ARM64
    u32 devid = tsdrv_get_devid_by_ctx(ctx);
    if ((!tsdrv_is_in_pm(devid)) && (sq_type == MEM_MAP_SUBTYPE_NORMAL)) {
        *prot = vm_page_prot;
    } else {
        *prot = pgprot_device(vm_page_prot);
    }
#else
    *prot = vm_page_prot;
#endif

    return 0;
}

int tsdrv_get_cq_mem_map_prot(struct tsdrv_ctx *ctx, u32 tsId, struct tsdrv_mem_map_para *map_para,
    pgprot_t vm_page_prot, pgprot_t *prot)
{
    if ((tsdrv_get_env_type() == TSDRV_ENV_ONLINE) && !devdrv_is_need_invalid_cache()) {
#if defined(CONFIG_ARM64)
        *prot = (map_para->addr_side == TSDRV_MEM_ON_DEVICE_SIDE) ? pgprot_device(vm_page_prot) : vm_page_prot;
#else
        *prot = vm_page_prot;
#endif
    } else {
        *prot = pgprot_noncached(vm_page_prot);
    }

    return 0;
}
#endif
#endif

STATIC int tsdrv_reuse_input_sq(struct tsdrv_ts_resource *ts_res, struct tsdrv_ctx *ctx,
    struct normal_alloc_sqcq_para *reused_sqcq, struct devdrv_normal_cqsq_mailbox *cqsq_mailbox)
{
#ifndef TSDRV_UT
    struct devdrv_sq_sub_info *sq_sub = NULL;

    /* Check if the sqId to be reused is legal */
    if (tsdrv_sq_exist_check(ts_res, ctx, reused_sqcq->sqId) != 0) {
        TSDRV_PRINT_ERR("the sqid(%u) that will be reused is invalid.\n", reused_sqcq->sqId);
        return -EINVAL;
    }

    /* sq_addr only informs ts once */
    cqsq_mailbox->sq_index = tsdrv_vrit_to_physic_id(&ts_res->id_res[TSDRV_SQ_ID], reused_sqcq->sqId);
    cqsq_mailbox->sq_addr = 0;

    sq_sub = tsdrv_get_sq_sub_info(ts_res, cqsq_mailbox->sq_index);
    cqsq_mailbox->sq_cq_side |= (sq_sub->addr_side << (int)TSDRV_SQCQ_SIDE_SQ_BIT);
#endif
    return 0;
}

STATIC struct devdrv_ts_sq_info *tsdrv_alloc_new_sq(struct tsdrv_ts_resource *ts_res, struct tsdrv_ctx *ctx)
{
    struct tsdrv_ts_ctx *ts_ctx = &ctx->ts_ctx[ts_res->tsid];
    struct devdrv_ts_sq_info *sq_info = NULL;

    sq_info = devdrv_get_new_sq(ts_res, ts_ctx);
    sq_info->alloc_status = SQCQ_ACTIVE;
    sq_info->type = NORMAL_SQCQ_TYPE;
    sq_info->sqDbVaddr = 0;
    sq_info->head = 0;
    sq_info->tail = 0;
    sq_info->tgid = current->tgid;  /* using global pid to get task */
    return sq_info;
}

static int tsdrv_map_sq_db(struct tsdrv_ctx *ctx, u32 tsid, struct normal_alloc_sqcq_para *sqcq_alloc,
    struct devdrv_ts_sq_info *sq_info)
{
#ifndef CFG_SOC_PLATFORM_MDC
    u32 devid = tsdrv_get_devid_by_ctx(ctx);
    struct tsdrv_db_hwinfo *db_hwinfo = tsdrv_get_db_hwinfo(devid, tsid);
    int ret = -ENODEV;

    if ((tsdrv_is_in_container(devid)) || (TSDRV_USERMAP_DB_SIZE != PAGE_SIZE)) {
        TSDRV_PRINT_DEBUG("Do not map sq db. (page_size=0x%lx)\n", (unsigned long)PAGE_SIZE);
        return 0;
    }

    if (db_hwinfo != NULL) {
        struct vm_area_struct *vma = NULL;

        down_write(get_mmap_sem(current->mm));
        vma = find_vma(current->mm, sqcq_alloc->sqDbVaddr);
        if (vma != NULL) {
            unsigned long pa = db_hwinfo->db_paddr + TSDRV_USERMAP_DB_SIZE * sq_info->index;
#ifndef TSDRV_UT
            ret = tsdrv_check_va_range(ctx, vma, sqcq_alloc->sqDbVaddr, TSDRV_USERMAP_DB_SIZE);
            if (ret != 0) {
                up_write(get_mmap_sem(current->mm));
                return ret;
            }

            ret = tsdrv_check_va_map(vma, sqcq_alloc->sqDbVaddr, TSDRV_USERMAP_DB_SIZE);
            if (ret != 0) {
                up_write(get_mmap_sem(current->mm));
                return ret;
            }
#endif
            ret = remap_pfn_range(vma, sqcq_alloc->sqDbVaddr, __phys_to_pfn(pa), (unsigned long)TSDRV_USERMAP_DB_SIZE,
                pgprot_device(vma->vm_page_prot));
            if (ret != 0) {
                up_write(get_mmap_sem(current->mm));
                TSDRV_PRINT_ERR("Remap pfn fail. (devid=%u; tsid=%u; sqid=%u; ret=%d)\n",
                    devid, tsid, sq_info->index, ret);
                return -ENODEV;
            }
            sq_info->sqDbVaddr = sqcq_alloc->sqDbVaddr;
            sqcq_alloc->IsSqDbMap = 1;
        }
        up_write(get_mmap_sem(current->mm));
    }
    return ret;
#else
    return 0;
#endif
}

int tsdrv_unmap_sq_db(struct tsdrv_ctx *ctx, u32 tsid, struct devdrv_ts_sq_info *sq_info)
{
#ifndef AOS_LLVM_BUILD
    struct vm_area_struct *vma = NULL;
    int ret;

    if (sq_info->sqDbVaddr == 0) {
        return 0;
    }
    down_write(get_mmap_sem(current->mm));

    vma = find_vma(current->mm, sq_info->sqDbVaddr);
    if (vma == NULL) {
        up_write(get_mmap_sem(current->mm));
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Find vma failed unmap.\n");
#endif
        return -EINVAL;
    }
#ifndef TSDRV_UT
    ret = tsdrv_check_va_range(ctx, vma, sq_info->sqDbVaddr, TSDRV_USERMAP_DB_SIZE);
    if (ret != 0) {
        up_write(get_mmap_sem(current->mm));
        return ret;
    }

    ret = tsdrv_check_va_unmap(vma, sq_info->sqDbVaddr, TSDRV_USERMAP_DB_SIZE);
    if (ret != 0) {
        up_write(get_mmap_sem(current->mm));
        return ret;
    }
#endif
    tsdrv_zap_vma_ptes(vma, sq_info->sqDbVaddr, (unsigned long)TSDRV_USERMAP_DB_SIZE);
    sq_info->sqDbVaddr = 0;
    up_write(get_mmap_sem(current->mm));

    return 0;
#endif
}

STATIC int tsdrv_init_new_sq(struct tsdrv_ctx *ctx, struct tsdrv_ts_resource *ts_res,
    struct devdrv_ts_sq_info *sq_info, struct normal_alloc_sqcq_para *sqcq_alloc,
    struct devdrv_normal_cqsq_mailbox *cqsq_mailbox)
{
    struct devdrv_sq_sub_info *sq_sub = NULL;
    struct tsdrv_id_info *id_info = NULL;
#ifndef AOS_LLVM_BUILD
    struct tsdrv_mem_map_para map_para;
#endif
    u32 devid = tsdrv_get_devid_by_ctx(ctx);
    int ret;

    id_info = devdrv_find_one_id(&ts_res->id_res[TSDRV_SQ_ID], sq_info->index);
    if (id_info == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("sq_id(%u) is invalid.\n", sq_info->index);
#endif
        return -EINVAL;
    }

    sq_info->depth = sqcq_alloc->sqeDepth;
    TSDRV_PRINT_DEBUG("sqid(%u), depth(%u)\n", sq_info->index, sq_info->depth);

    sqcq_alloc->sqId = id_info->id;
    TSDRV_PRINT_DEBUG("sqid(%d), vsqid(%d)\n", id_info->phy_id, id_info->id);
    sq_sub = (struct devdrv_sq_sub_info *)sq_info->sq_sub;
    sq_sub->size = sqcq_alloc->sqeSize;
    sq_sub->depth = sq_info->depth;
#ifndef AOS_LLVM_BUILD
    sq_sub->map_va = sqcq_alloc->sqMapVaddr;
#endif
    if (devdrv_init_sq(ts_res, ctx, sq_sub, sq_info->depth * DEVDRV_SQ_SLOT_SIZE) != 0) {
        TSDRV_PRINT_ERR("devdrv_init_uio sq failed!\n");
        return -EINVAL;
    }

#ifndef CFG_SOC_PLATFORM_MDC_V51
    if (!tsdrv_is_in_pm(devid)) {
        hvtsdrv_clear_vsq_info(ts_res, id_info->id);
    }
#endif
    if (!tsdrv_is_in_vm(devid)) {
#ifndef AOS_LLVM_BUILD
        ret = tsdrv_map_sq_db(ctx, ts_res->tsid, sqcq_alloc, sq_info);
        if (ret != 0) {
#ifndef TSDRV_UT
            goto err_remap_sqdb;
#endif
        }

        map_para.id_type = TSDRV_SQ_ID;
        map_para.sub_type = MEM_MAP_SUBTYPE_NORMAL;
        map_para.id = id_info->id;
        map_para.va = sq_sub->map_va;
        map_para.len = sq_sub->depth * sq_sub->size;
        ret = tsdrv_remap_va(ctx, ts_res->tsid, &map_para);
        if (ret != 0) {
#ifndef TSDRV_UT
            goto err_remap_sq;
#endif
        }
#else
        ret = tsdrv_get_user_va(sq_sub->vaddr, sq_sub->depth * sq_sub->size, &sqcq_alloc->sqMapVaddr);
        if (ret != 0) {
            TSDRV_PRINT_ERR("Failed to get sq user virt addr. (id=%d; tsid=%u; devid=%u; ret=%d)\n",
                            id_info->id, ts_res->tsid, devid, ret);
            sq_sub->vaddr = 0;
#ifndef TSDRV_UT
            goto err_remap_sq;
#endif
        }
        sq_sub->map_va = sqcq_alloc->sqMapVaddr;
#endif
    }
    /* get the sq_index and sq_addr that will be notified to ts through the mailbox */
    cqsq_mailbox->sq_index = sq_info->index;
    cqsq_mailbox->sq_addr = ((struct devdrv_sq_sub_info *)sq_info->sq_sub)->phy_addr;
    cqsq_mailbox->sq_cq_side |= (sq_sub->addr_side << (int)TSDRV_SQCQ_SIDE_SQ_BIT);
    return 0;
#ifndef TSDRV_UT
err_remap_sq:
    (void)tsdrv_unmap_sq_db(ctx, ts_res->tsid, sq_info);
err_remap_sqdb:
    if (sq_sub->addr_side == TSDRV_MEM_ON_HOST_SIDE) {
        struct tsdrv_device *tsdrv_dev = tsdrv_res_to_dev(ts_res);
        dma_unmap_single(tsdrv_dev->dev, sq_sub->phy_addr, sq_sub->size, DMA_BIDIRECTIONAL);
        tsdrv_free_pages_exact((void *)(uintptr_t)sq_sub->vaddr, sq_info->depth * DEVDRV_SQ_SLOT_SIZE);
        sq_sub->phy_addr = 0;
        sq_sub->vaddr = 0;
    }
    return -ENODEV;
#endif
}

/* attention: the caller has been locked currently, when use this interface needed to add lock */
static bool tsdrv_is_ctx_has_ctrl_sq(struct tsdrv_ctx *ctx, u32 devid, u32 tsid)
{
    struct tsdrv_ts_resource *ts_res = tsdrv_ctx_to_ts_res(ctx, tsid);
    struct tsdrv_ts_ctx *ts_ctx = &ctx->ts_ctx[tsid];
    struct devdrv_ts_sq_info *sq_info = NULL;
    struct tsdrv_id_info *sq_id_info = NULL;
    struct list_head *pos = NULL;
    struct list_head *n = NULL;

    if (list_empty_careful(&ts_ctx->id_ctx[TSDRV_SQ_ID].id_list)) {
        return false;
    }

    list_for_each_safe(pos, n, &ts_ctx->id_ctx[TSDRV_SQ_ID].id_list) {
        sq_id_info = list_entry(pos, struct tsdrv_id_info, list);
        sq_info = devdrv_calc_sq_info(ts_res->mem_info[DEVDRV_INFO_MEM].virt_addr, sq_id_info->phy_id);
        if (sq_info->type == CTRL_SQCQ_TYPE) {
            return true;
        }
    }

    return false;
}

STATIC int tsdrv_reuse_ctx_sq(struct tsdrv_ctx *ctx, u32 devid, struct tsdrv_ts_resource *ts_res,
    struct normal_alloc_sqcq_para *sqcq_alloc)
{
    struct tsdrv_ts_ctx *ts_ctx = &ctx->ts_ctx[ts_res->tsid];
    u32 thre_val = 0;

    if ((tsdrv_ctx_is_nothread_type(ctx)) && (tsdrv_get_fid_by_ctx(ctx) == TSDRV_PM_FID)) {
        thre_val++;
    }
    if (tsdrv_is_ctx_has_ctrl_sq(ctx, devid, ts_res->tsid)) {
        thre_val++; /* one ctx must has a normal sq and ctrl sq. */
    }

    if (ts_ctx->id_ctx[TSDRV_SQ_ID].id_num <= thre_val) {
        return -EPERM;
    }

    sqcq_alloc->sqId = SQCQ_UMAX;
    sqcq_alloc->flag |= TSDRV_SQ_REUSE;

    return SQ_REUSE_SUCCESS;
}

static u32 tsdrv_get_sq_thresh_num(u32 devid, u32 fid)
{
    u32 thresh_num;
    u32 thresh_capacity;

    if (tsdrv_is_in_pm(devid) == true) {
        thresh_num = DEVDRV_SQ_FLOOR * 2; /* 2 means: one for normal, another for ctrl */
    } else {
        struct tsdrv_dev_resource *dev_res = NULL;
        dev_res = tsdrv_get_dev_resource(devid, fid);
        thresh_capacity = (u32)((dev_res->cap_num * DEVDRV_CORE_MULTIPLE_CAPACITY) / dev_res->aicore_num);

        if (thresh_capacity < DEVDRV_CORE_VF_FLOOR_CAPACITY) {
            thresh_num = DEVDRV_SQ_VF_FLOOR_MIN;
        } else {
            thresh_num = DEVDRV_SQ_VF_FLOOR;
        }
    }

    return thresh_num;
}

/**
 * If the local ID resources are insufficient, synchronize data from the opposite side.
 * After successfully synchronized, alloc sq id again.
 */
STATIC int tsdrv_sync_sq_from_opposite(u32 devid, u32 tsid)
{
    if (!tsdrv_is_in_pm(devid)) {
        return -EINVAL;
    }

    if (tsdrv_msg_alloc_sync_sq(devid, tsid) != 0) {
        TSDRV_PRINT_ERR("[dev_id=%u]:get sq from opposite side failed !!!!!\n", devid);
        return -EINVAL;
    }

    return 0;
}

static bool tsdrv_sq_need_reuse(struct tsdrv_ts_resource *ts_res, struct tsdrv_ctx *ctx,
    u32 local_available_num)
{
    /* To ensure that each process has at least one SQ. If the number of local SQ
     * is less than or equal to proc_num, the processes reuse there existing SQ.
     */
    u32 devid = tsdrv_get_devid_by_ctx(ctx);
    u32 fid = tsdrv_get_fid_by_ctx(ctx);
    u32 local_cur_num, remain_sq_num;
    u32 thresh_num;

    thresh_num = tsdrv_get_sq_thresh_num(devid, fid);

    if (tsdrv_get_fid_by_ctx(ctx) != TSDRV_PM_FID) {
        /* In container or vm, the sq is in host before init instance.
         * Just compare avail sq num with thresh_num.
         */
        return (local_available_num < thresh_num) ? true : false;
    } else {
        if (tsdrv_get_ts_id_cur_num(devid, ts_res->tsid, TSDRV_SQ_ID, &local_cur_num)) {
            return true;
        }
        remain_sq_num = DEVDRV_MAX_SQ_NUM - (local_cur_num - local_available_num) - 1; /* 1 for offline cb */
        return (remain_sq_num < thresh_num) ? true : false;
    }
}

STATIC int tsdrv_sq_alloc(struct tsdrv_ts_resource *ts_res, struct tsdrv_ctx *ctx,
    struct normal_alloc_sqcq_para *sqcq_alloc, struct devdrv_normal_cqsq_mailbox *tmp_cqsq)
{
    struct devdrv_ts_sq_info *sq_info = NULL;
    u32 devid = tsdrv_get_devid_by_ctx(ctx);
    u32 local_available_num;

    if ((sqcq_alloc->flag & TSDRV_SQ_REUSE) != 0) {
        return tsdrv_reuse_input_sq(ts_res, ctx, sqcq_alloc, tmp_cqsq);
    }

sq_alloc_retry:

    spin_lock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
    local_available_num = ts_res->id_res[TSDRV_SQ_ID].id_available_num;

    if (tsdrv_sq_need_reuse(ts_res, ctx, local_available_num)) {
        if (tsdrv_reuse_ctx_sq(ctx, devid, ts_res, sqcq_alloc) == SQ_REUSE_SUCCESS) {
            spin_unlock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
            return -EAGAIN;
        }
    }

    /* If the local SQ resources are sufficient or the process applies
     * for the SQ for the first time, obtain the new SQ.
     */
    if (local_available_num > 0) {
        sq_info = tsdrv_alloc_new_sq(ts_res, ctx);
        spin_unlock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
        return tsdrv_init_new_sq(ctx, ts_res, sq_info, sqcq_alloc, tmp_cqsq);
    }
    spin_unlock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);

    TSDRV_PRINT_WARN("no sq available, sync from opposite side!\n");
    if (tsdrv_sync_sq_from_opposite(devid, ts_res->tsid) != 0) {
#ifndef TSDRV_UT
        /* Sq id exhausted. return -EAGAIN */
        sqcq_alloc->sqId = SQCQ_UMAX;
        return -EAGAIN;
#endif /* TSDRV_UT */
    }
    goto sq_alloc_retry;
}

STATIC int tsdrv_cq_alloc_new_one(struct tsdrv_ts_resource *ts_res, struct tsdrv_ctx *ctx,
    struct normal_alloc_sqcq_para *sqcq_alloc, struct devdrv_normal_cqsq_mailbox *tmp_cqsq)
{
    struct devdrv_ts_cq_info *cq_info = NULL;
    struct tsdrv_id_info *id_info = NULL;
    struct devdrv_cq_sub_info *cq_sub = NULL;
    u32 devid = tsdrv_get_devid_by_ctx(ctx);
    int ret;

    cq_info = devdrv_get_new_cq(devid, ts_res->tsid, ctx);
    if (cq_info == NULL) {
        TSDRV_PRINT_ERR("alloc persistent cq for cce_context failed.\n");
        return -ENOMEM;
    }

    id_info = devdrv_find_one_id(&ts_res->id_res[TSDRV_CQ_ID], cq_info->index);
    if (id_info == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("cq_id(%u) is invalid.\n", cq_info->index);
        devdrv_free_cq(ts_res, ctx, cq_info);
        return -EINVAL;
#endif
    }

    cq_info->slot_size = sqcq_alloc->cqeSize;
    cq_info->depth = sqcq_alloc->cqeDepth;
    cq_info->type = NORMAL_SQCQ_TYPE;
    cq_info->head = 0;
    cq_info->tail = 0;

    sqcq_alloc->cqId = id_info->id;
    TSDRV_PRINT_DEBUG("Get cqid. (phy_id=%u; id=%u; depth=%u)\n", id_info->phy_id, id_info->id, cq_info->depth);
    cq_info->alloc_status = SQCQ_ACTIVE;
    cq_info->phase = DEVDRV_PHASE_STATE_1;
    cq_sub = (struct devdrv_cq_sub_info *)cq_info->cq_sub;
#ifndef AOS_LLVM_BUILD
    cq_sub->map_va = sqcq_alloc->cqMapVaddr;
#endif
    ret = devdrv_init_cq(ctx, ts_res->tsid, cq_sub, cq_info->depth * cq_info->slot_size);
    if (ret != 0) {
        TSDRV_PRINT_ERR("init uio for persistent cq failed.\n");
        devdrv_free_cq(ts_res, ctx, cq_info);
        return -ENOMEM;
    }
#ifndef CFG_SOC_PLATFORM_MDC_V51
    if (!tsdrv_is_in_pm(devid)) {
        hvtsdrv_clear_vcq_info(ts_res, id_info->id);
    }
#endif
    if (!tsdrv_is_in_vm(devid)) {
#ifndef AOS_LLVM_BUILD
        struct tsdrv_mem_map_para map_para;
        map_para.id_type = TSDRV_CQ_ID;
        map_para.sub_type = MEM_MAP_SUBTYPE_NORMAL;
        map_para.id = id_info->id;
        map_para.addr_side = cq_sub->addr_side;
        map_para.va = cq_sub->map_va;
        map_para.len = cq_sub->size;
        ret = tsdrv_remap_va(ctx, ts_res->tsid, &map_para);
        if (ret != 0) {
#ifndef TSDRV_UT
            TSDRV_PRINT_ERR("Failed to mmap cq. (id=%d; tsid=%u; devid=%u)\n", id_info->id, ts_res->tsid, devid);
            if (tsdrv_get_env_type() == TSDRV_ENV_ONLINE) {
                struct tsdrv_device *tsdrv_dev = tsdrv_res_to_dev(ts_res);
                dma_unmap_single(tsdrv_dev->dev, cq_sub->phy_addr, cq_sub->size, DMA_BIDIRECTIONAL);
                tsdrv_free_pages_exact((void *)(uintptr_t)cq_sub->virt_addr, cq_sub->size);
            } else {
#ifdef CFG_SOC_PLATFORM_MINIV2
                iounmap((void *)(uintptr_t)cq_sub->virt_addr);
#else
                tsdrv_free_pages_exact((void *)(uintptr_t)cq_sub->virt_addr, cq_sub->size);
#endif /* CFG_SOC_PLATFORM_MINIV2 */
            }
            spin_lock_irq(&cq_sub->spinlock);
            cq_sub->virt_addr = 0;
            cq_sub->phy_addr = 0;
            spin_unlock_irq(&cq_sub->spinlock);
            devdrv_free_cq(ts_res, ctx, cq_info);
            return -EFAULT;
#endif
        }
#else
        ret = tsdrv_get_user_va(cq_sub->virt_addr, cq_info->depth * cq_info->slot_size, &sqcq_alloc->cqMapVaddr);
        if (ret != 0) {
            TSDRV_PRINT_ERR("Failed to get cq user virt addr. (id=%d; tsid=%u; devid=%u; ret=%d)\n",
                            id_info->id, ts_res->tsid, devid, ret);
            spin_lock_irq(&cq_sub->spinlock);
            iounmap((void *)(uintptr_t)cq_sub->virt_addr);
            cq_sub->virt_addr = 0;
            cq_sub->phy_addr = 0;
            spin_unlock_irq(&cq_sub->spinlock);
            devdrv_free_cq(ts_res, ctx, cq_info);
            return ret;
        }
        cq_sub->map_va = sqcq_alloc->cqMapVaddr;
#endif
    }

    tmp_cqsq->cq0_index = cq_info->index;
    tmp_cqsq->cq0_addr = cq_sub->phy_addr;
    tmp_cqsq->sq_cq_side |= (cq_sub->addr_side << (int)TSDRV_SQCQ_SIDE_CQ_BIT);

    return 0;
}

STATIC int tsdrv_get_nothread_cq(struct tsdrv_ts_resource *ts_res, struct tsdrv_ctx *ctx)
{
    struct devdrv_ts_cq_info *cq_info = NULL;
    struct tsdrv_id_info *cq_id_info = NULL;
    u32 devid = tsdrv_get_devid_by_ctx(ctx);
    struct list_head *pos = NULL;
    struct list_head *n = NULL;

    spin_lock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
    if (list_empty_careful(&ctx->ts_ctx[ts_res->tsid].id_ctx[TSDRV_CQ_ID].id_list) != 0) {
#ifndef TSDRV_UT
        spin_unlock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
        TSDRV_PRINT_ERR("The cq_list is empty.\n");
        return -EINVAL;
#endif
    }

    list_for_each_safe(pos, n, &ctx->ts_ctx[ts_res->tsid].id_ctx[TSDRV_CQ_ID].id_list) {
        cq_id_info = list_entry(pos, struct tsdrv_id_info, list);
        cq_info = tsdrv_get_cq_info(devid, tsdrv_get_fid_by_ctx(ctx), ts_res->tsid, cq_id_info->phy_id);
        if (cq_info->type == SHM_SQCQ_TYPE) {
            spin_unlock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
            return cq_id_info->phy_id;
        }
    }

    spin_unlock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
    TSDRV_PRINT_ERR("Has not alloced shm cq.\n");
    return -EINVAL;
}

STATIC int tsdrv_cq_alloc(struct tsdrv_ts_resource *ts_res, struct tsdrv_ctx *ctx,
    struct normal_alloc_sqcq_para *sqcq_alloc, struct devdrv_normal_cqsq_mailbox *tmp_cqsq)
{
    struct devdrv_cq_sub_info *cq_sub = NULL;
    u32 devid = tsdrv_get_devid_by_ctx(ctx);
    struct tsdrv_cq_hwinfo *cq_hwinfo = tsdrv_get_cq_hwinfo(devid, ts_res->tsid);
    u16 irq_id;
    int ret;

    if (tsdrv_ctx_is_nothread_type(ctx)) {
        int cqid;
        cqid = tsdrv_get_nothread_cq(ts_res, ctx);
        if (cqid < 0) {
            TSDRV_PRINT_ERR("Get nothread cq failed. (cqid=%d)\n", cqid);
            return -EINVAL;
        }
        /* cq_addr only informs ts once */
        sqcq_alloc->cqId = cqid;
        tmp_cqsq->cq0_index = cqid;
        tmp_cqsq->cq0_addr = 0;
        cq_sub = tsdrv_get_cq_sub_info(ts_res, tmp_cqsq->cq0_index);
        tmp_cqsq->sq_cq_side |= (cq_sub->addr_side << (int)TSDRV_SQCQ_SIDE_CQ_BIT);
        sqcq_alloc->flag |= TSDRV_CQ_REUSE;
        TSDRV_PRINT_DEBUG("Reuse cq. (virt_cqid=%u; phy_cqid=%u)\n", sqcq_alloc->cqId, (u32)tmp_cqsq->cq0_index);
    } else {
        if ((sqcq_alloc->flag & TSDRV_CQ_REUSE) == 0) {
            ret = tsdrv_cq_alloc_new_one(ts_res, ctx, sqcq_alloc, tmp_cqsq);
            if (ret != 0) {
                TSDRV_PRINT_ERR("cq alloc new one failed, ret(%d).\n", ret);
                return ret;
            }
        } else {
            ret = tsdrv_cq_exist_check(ts_res, ctx, sqcq_alloc->cqId);
            if (ret != 0) {
                TSDRV_PRINT_ERR("the cqid(%u) that will be reused is invalid.\n", sqcq_alloc->cqId);
                return -EINVAL;
            }
            /* sq_addr only informs ts once */
            tmp_cqsq->cq0_index = tsdrv_vrit_to_physic_id(&ts_res->id_res[TSDRV_CQ_ID], sqcq_alloc->cqId);
            tmp_cqsq->cq0_addr = 0;

            cq_sub = tsdrv_get_cq_sub_info(ts_res, tmp_cqsq->cq0_index);
            tmp_cqsq->sq_cq_side |= (cq_sub->addr_side << (int)TSDRV_SQCQ_SIDE_CQ_BIT);
        }
    }

    devdrv_calc_cq_irq_id(tmp_cqsq->cq0_index, cq_hwinfo->cq_irq_num, DEVDRV_MAX_CQ_NUM, &irq_id);
    TSDRV_PRINT_FPGA("cqid(%u), irq_id(%u).\n", sqcq_alloc->cqId, irq_id);
    tmp_cqsq->cq_irq = cq_hwinfo->cq_irq[irq_id];

    return 0;
}

STATIC int devdrv_cqsq_alloc(struct tsdrv_ts_resource *ts_res, struct tsdrv_ctx *ctx,
    struct normal_alloc_sqcq_para *sqcq_alloc, struct devdrv_normal_cqsq_mailbox *tmp_cqsq)
{
    struct devdrv_ts_sq_info *sq_info = NULL;
    phys_addr_t info_mem_addr;
    int ret;

    ret = tsdrv_sq_alloc(ts_res, ctx, sqcq_alloc, tmp_cqsq);
    /* ret is nor equal to 0 while applying sq failed or noticing runtime to reuse sq */
    if (ret != 0) {
        return ret;
    }

    ret = tsdrv_cq_alloc(ts_res, ctx, sqcq_alloc, tmp_cqsq);
    if (ret != 0) {
        if ((sqcq_alloc->flag & TSDRV_SQ_REUSE) == 0) {
            info_mem_addr = ts_res->mem_info[DEVDRV_INFO_MEM].virt_addr;
            sq_info = devdrv_calc_sq_info(info_mem_addr, ts_res->id_res[TSDRV_SQ_ID].pm_id[sqcq_alloc->sqId]);
            (void)devdrv_exit_sq(ts_res, ctx, sq_info);
            tsdrv_free_sq(ts_res, ctx, sq_info);
        }
        TSDRV_PRINT_ERR("tsdrv_cq_alloc alloc failed, ret(%d).\n", ret);
        return -EINVAL;
    }

    return 0;
}

STATIC void devdrv_cqsq_free_id(struct tsdrv_ts_resource *ts_res, struct tsdrv_ctx *ctx, u32 sqId, u32 cqId, u32 flag)
{
    struct devdrv_ts_cq_info *cq_tmp = NULL;
    struct devdrv_ts_sq_info *sq_tmp = NULL;
    phys_addr_t info_mem_addr;
    int ret;

    info_mem_addr = ts_res->mem_info[DEVDRV_INFO_MEM].virt_addr;

    if ((flag & TSDRV_CQ_REUSE) == 0) {
        cq_tmp = devdrv_calc_cq_info(info_mem_addr, ts_res->id_res[TSDRV_CQ_ID].pm_id[cqId]);
        ret = devdrv_exit_cq(ctx, ts_res->tsid, cq_tmp->cq_sub);
        if (ret != 0) {
            return;
        }
        devdrv_free_cq(ts_res, ctx, cq_tmp);
    }
    if ((flag & TSDRV_SQ_REUSE) == 0) {
        sq_tmp = devdrv_calc_sq_info(info_mem_addr, ts_res->id_res[TSDRV_SQ_ID].pm_id[sqId]);
        ret = devdrv_exit_sq(ts_res, ctx, sq_tmp);
        if (ret != 0) {
            return;
        }
        tsdrv_free_sq(ts_res, ctx, sq_tmp);
    }

    return;
}

STATIC int devdrv_mailbox_cqsq_inform(struct devdrv_mailbox *mailbox, struct tsdrv_ctx *ctx, u16 cmd,
    struct devdrv_normal_cqsq_mailbox *tmp_cqsq)
{
    struct devdrv_normal_cqsq_mailbox *cqsq = NULL;
    struct devdrv_ts_cq_info *cq_info = NULL;
    struct tsdrv_ts_resource *ts_res = NULL;
#ifndef CFG_SOC_PLATFORM_MDC_V51
    struct hvtsdrv_trans_mailbox_ctx trans_mbox;
    u32 disable_thread = (tsdrv_ctx_is_nothread_type(ctx) == true) ? 1 : 0;
#endif
    phys_addr_t info_mem_addr;
    int result = 0;
    u32 len;
    int ret;

    ts_res = container_of(mailbox, struct tsdrv_ts_resource, mailbox);
    cqsq = kzalloc(sizeof(struct devdrv_normal_cqsq_mailbox), GFP_KERNEL);
    if (cqsq == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("kmalloc failed.\n");
        return -ENOMEM;
#endif
    }

    ret = memcpy_s(cqsq, sizeof(struct devdrv_normal_cqsq_mailbox), tmp_cqsq,
        sizeof(struct devdrv_normal_cqsq_mailbox));
    if (ret != EOK) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("memcpy_s failed, ret = %d\n", ret);
        kfree(cqsq);
        cqsq = NULL;
        return -EINVAL;
#endif
    }

    /* add message header */
    cqsq->valid = DEVDRV_MAILBOX_MESSAGE_VALID;
    cqsq->cmd_type = cmd;
    cqsq->result = 0;

#if defined(CFG_SOC_PLATFORM_MINI) && !defined(CFG_SOC_PLATFORM_MINIV2) && !defined(CFG_SOC_PLATFORM_MINIV3)
    cqsq->app_flag = (u8)tsdrv_get_env_type();
#else
    cqsq->app_type = (u8)tsdrv_get_env_type();
    cqsq->fid = tsdrv_get_fid_by_ctx(ctx);
#endif

    cqsq->pid = ctx->tgid;
    cqsq->ssid = ctx->ssid;

    if (tmp_cqsq->cq0_index < DEVDRV_MAX_CQ_NUM) {
        info_mem_addr = ts_res->mem_info[DEVDRV_INFO_MEM].virt_addr;
        cq_info = devdrv_calc_cq_info(info_mem_addr, cqsq->cq0_index);
        cqsq->cqesize = cq_info->slot_size;
    }

    TSDRV_PRINT_DEBUG("(side=%u, cmd=%u, plat=%d, cq=%u, cq_addr=%pK, irq=%u, sq=%u, sq_addr=%pK, stream_id=%u, "
        "ssid=%u)\n", (u32)cqsq->sq_cq_side, (u32)cqsq->cmd_type, (int)tsdrv_get_env_type(), (u32)cqsq->cq0_index,
        (void *)(uintptr_t)cqsq->cq0_addr, (u32)cqsq->cq_irq, (u32)cqsq->sq_index, (void *)(uintptr_t)cqsq->sq_addr,
        cqsq->info[0], (u32)cqsq->ssid);

#ifndef CFG_SOC_PLATFORM_MDC_V51
    TSDRV_PRINT_DEBUG("trans normal: valid=%u; cmd=%u; disable_thread=%u.\n", (u32)cqsq->valid, (u32)cqsq->cmd_type,
        disable_thread);
    hvtsdrv_fill_trans_info_mbox(tsdrv_get_devid_by_ctx(ctx), tsdrv_get_fid_by_ctx(ctx), ts_res->tsid,
        disable_thread, &trans_mbox);
    ret = hvtsdrv_trans_info_msg(&trans_mbox, (void *)cqsq, sizeof(struct devdrv_normal_cqsq_mailbox));
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to translate runtime info. (ret=%d; tsid=%u)\n", ret, ts_res->tsid);
        kfree(cqsq);
        cqsq = NULL;
        return ret;
    }
#endif

    len = sizeof(struct devdrv_normal_cqsq_mailbox);
    ret = devdrv_mailbox_kernel_sync_no_feedback(mailbox, (u8 *)cqsq, len, &result);
    kfree(cqsq);
    cqsq = NULL;

    if (ret == 0) {
        ret = result;
    }
    return ret;
}

STATIC int tsdrv_sqcq_alloc_para_check(struct normal_alloc_sqcq_para *sqcq_para)
{
    if (sqcq_para->sqeDepth < DEVDRV_MIN_SQ_DEPTH || sqcq_para->sqeDepth > DEVDRV_MAX_SQ_DEPTH ||
        sqcq_para->cqeDepth < DEVDRV_MIN_CQ_DEPTH || sqcq_para->cqeDepth > DEVDRV_MAX_CQ_DEPTH ||
        sqcq_para->cqeSize > DEVDRV_MAX_CQE_SIZE) {
        TSDRV_PRINT_ERR("invalid para: sqeSize(%u), sqeDepth(%u), cqeSize(%u), cqeDepth(%u).\n",
            sqcq_para->sqeSize, sqcq_para->sqeDepth, sqcq_para->cqeSize, sqcq_para->cqeDepth);
        return -EINVAL;
    }

    TSDRV_PRINT_DEBUG("sqeSize(%u), sqeDepth(%u), cqeSize(%u), cqeDepth(%u).\n",
        sqcq_para->sqeSize, sqcq_para->sqeDepth, sqcq_para->cqeSize, sqcq_para->cqeDepth);

    return 0;
}

STATIC int tsdrv_phy_sqcq_unmap(struct tsdrv_ctx *ctx, u32 tsid, u32 sq_id, u32 cq_id)
{
    struct tsdrv_mem_unmap_para sq_unmap_para = {0};
    struct tsdrv_mem_unmap_para cq_unmap_para = {0};
    u32 devid = tsdrv_get_devid_by_ctx(ctx);
    u32 fid = tsdrv_get_fid_by_ctx(ctx);
    struct tsdrv_ts_resource *ts_res = tsdrv_get_ts_resoruce(devid, fid, tsid);
    struct devdrv_sq_sub_info *sq_sub = tsdrv_get_sq_sub_info(ts_res, sq_id);
    struct devdrv_cq_sub_info *cq_sub = tsdrv_get_cq_sub_info(ts_res, cq_id);
    int ret;

    sq_unmap_para.va = sq_sub->map_va;
    sq_unmap_para.len = sq_sub->size * sq_sub->depth;
    ret = tsdrv_unmap_va(ctx, tsid, &sq_unmap_para);
    if (ret != 0) {
        TSDRV_PRINT_ERR("normal sq unmap fail. (devid=%u; tsid=%u; sqid=%u)\n", devid, tsid, sq_id);
        return ret;
    }

    cq_unmap_para.va = cq_sub->map_va;
    cq_unmap_para.len = cq_sub->slot_size * cq_sub->depth;
    ret = tsdrv_unmap_va(ctx, tsid, &cq_unmap_para);
    if (ret != 0) {
        TSDRV_PRINT_ERR("normal cq unmap fail. (devid=%u; tsid=%u; cq_id=%u)\n", devid, tsid, cq_id);
        return ret;
    }

    TSDRV_PRINT_DEBUG("Ummap normal sq cq success. (sqid=%u; va=0x%pK; len=0x%lx; cqid=%u;"
        " va=0x%pK; len=0x%lx; tgid=%d)\n",
        sq_id, (void *)(uintptr_t)sq_unmap_para.va, sq_unmap_para.len, cq_id,
        (void *)(uintptr_t)cq_unmap_para.va, cq_unmap_para.len, ctx->tgid);
    return 0;
}

STATIC int tsdrv_phy_sqcq_mmap(struct tsdrv_ctx *ctx, u32 tsid, struct normal_alloc_sqcq_para *arg)
{
    u32 devid = tsdrv_get_devid_by_ctx(ctx);
    struct tsdrv_mem_map_para sq_map_para = {0};
    struct tsdrv_mem_map_para cq_map_para = {0};
    struct tsdrv_mem_unmap_para sq_unmap_para = {0};
    u32 fid = tsdrv_get_fid_by_ctx(ctx);
    struct tsdrv_ts_resource *ts_res = tsdrv_get_ts_resoruce(devid, fid, tsid);
    struct devdrv_sq_sub_info *sq_sub = tsdrv_get_sq_sub_info(ts_res, arg->sqId);
    struct devdrv_cq_sub_info *cq_sub = tsdrv_get_cq_sub_info(ts_res, arg->cqId);
    int ret;

    sq_map_para.id = arg->sqId;
    sq_map_para.id_type = TSDRV_SQ_ID;
    sq_map_para.sub_type = MEM_MAP_SUBTYPE_NORMAL;
    sq_map_para.va = arg->sqMapVaddr;
    sq_map_para.len = arg->sqeSize * arg->sqeDepth;
    ret = tsdrv_remap_va(ctx, tsid, &sq_map_para);
    if (ret != 0) {
        TSDRV_PRINT_ERR("normal sq map fail. (devid=%u; tsid=%u; sqid=%u)\n", devid, tsid, arg->sqId);
        return ret;
    }
    sq_sub->map_va = sq_map_para.va;

    cq_map_para.id = arg->cqId;
    cq_map_para.id_type = TSDRV_CQ_ID;
    cq_map_para.sub_type = MEM_MAP_SUBTYPE_NORMAL;
    cq_map_para.va = arg->cqMapVaddr;
    cq_map_para.len = arg->cqeSize * arg->cqeDepth;
    ret = tsdrv_remap_va(ctx, tsid, &cq_map_para);
    if (ret != 0) {
#ifndef TSDRV_UT
        sq_unmap_para.va = sq_map_para.va;
        sq_unmap_para.len = sq_map_para.len;
        (void)tsdrv_unmap_va(ctx, tsid, &sq_unmap_para);
        TSDRV_PRINT_ERR("normal sq map fail. (devid=%u; tsid=%u; sqid=%u)\n", devid, tsid, arg->cqId);
        return ret;
#endif
    }
    cq_sub->map_va = cq_map_para.va;

    TSDRV_PRINT_DEBUG("Mmap normal sq cq success. (sqid=%u; va=0x%pK; len=0x%lx; cqid=%u;"
        " va=0x%pK; len=0x%lx; tgid=%d)\n",
        arg->sqId, (void *)(uintptr_t)sq_sub->map_va, sq_map_para.len, arg->cqId,
        (void *)(uintptr_t)cq_sub->map_va, cq_map_para.len, ctx->tgid);
    return 0;
}

STATIC int tsdrv_free_phy_sqcq(struct tsdrv_ctx *ctx, u32 tsid, u32 sq_id, u32 cq_id)
{
    u32 devid = tsdrv_get_devid_by_ctx(ctx);
    struct phy_sq_free_para sq_free_para;
    struct phy_cq_free_para cq_free_para;
    int ret;

    sq_free_para.sq_id = sq_id;
    ret = tsdrv_phy_sq_free(ctx, tsid, &sq_free_para);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to free sq. (device_id=%u; tsid=%u; sq_id=%u)", devid, tsid, sq_id);
        return ret;
    }

    cq_free_para.cq_id = cq_id;
    ret = tsdrv_phy_cq_free(ctx, tsid, &cq_free_para);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to free cq. (device_id=%u; tsid=%u; cq_id=%u)", devid, tsid, cq_id);
        return ret;
    }

    return 0;
}

STATIC int tsdrv_alloc_phy_sqcq(struct tsdrv_ctx *ctx, u32 tsid, struct normal_alloc_sqcq_para *sqcq_alloc)
{
    struct phy_sq_alloc_para sq_alloc_para = {0};
    struct phy_cq_alloc_para cq_alloc_para = {0};
    struct phy_sq_free_para sq_free_para = {0};
    u32 devid = tsdrv_get_devid_by_ctx(ctx);
    u32 fid = tsdrv_get_fid_by_ctx(ctx);
    struct devdrv_ts_sq_info *sq_info = NULL;
    struct devdrv_ts_cq_info *cq_info = NULL;
    int ret;

    sq_alloc_para.type = CTRL_SQCQ_TYPE;
    sq_alloc_para.sq_size = sqcq_alloc->sqeSize;
    sq_alloc_para.sq_depth = sqcq_alloc->sqeDepth;
    ret = tsdrv_phy_sq_alloc(ctx, tsid, &sq_alloc_para);
    if (ret != 0) {
        if (ret == NO_RES) {
            sqcq_alloc->sqId = SQCQ_UMAX;
        }
        TSDRV_PRINT_ERR("Failed to alloc sq. (device_id=%u; tsid=%u; ret=%d)\n", devid, tsid, ret);
        return ret;
    }

    cq_alloc_para.type = CTRL_SQCQ_TYPE;
    cq_alloc_para.cq_size = sqcq_alloc->cqeSize;
    cq_alloc_para.cq_depth = sqcq_alloc->cqeDepth;
    ret = tsdrv_phy_cq_alloc(ctx, tsid, &cq_alloc_para);
    if (ret != 0) {
        sq_free_para.sq_id = sq_alloc_para.sq_id;
        (void)tsdrv_phy_sq_free(ctx, tsid, &sq_free_para);
        TSDRV_PRINT_ERR("Failed to alloc cq. (device_id=%u; tsid=%u)", devid, tsid);
        return ret;
    }

    sqcq_alloc->sqId = sq_alloc_para.sq_id;
    sqcq_alloc->cqId = cq_alloc_para.cq_id;
    sq_info = tsdrv_get_sq_info(devid, fid, tsid, sqcq_alloc->sqId);
    sq_info->bind_cqid = sqcq_alloc->cqId;

    cq_info = tsdrv_get_cq_info(devid, fid, tsid, sqcq_alloc->cqId);
    cq_info->bind_logic_cqid = sqcq_alloc->info[0];

    TSDRV_PRINT_DEBUG("alloc cqsq id success. (device_id=%u; tsid=%u; sq_id=%u; cq_id=%u)",
        devid, tsid, sqcq_alloc->sqId, sqcq_alloc->cqId);

    return 0;
}

int tsdrv_alloc_phy_cqsq_mbox_send(struct tsdrv_ctx *ctx, u32 tsid, struct normal_alloc_sqcq_para *sqcq_alloc)
{
    struct devdrv_normal_cqsq_mailbox cqsq_mailbox = {0};
    u32 devid = tsdrv_get_devid_by_ctx(ctx);
    u32 fid = tsdrv_get_fid_by_ctx(ctx);
    struct tsdrv_ts_resource *pm_ts_resource = tsdrv_get_ts_resoruce(devid, TSDRV_PM_FID, tsid);
    struct tsdrv_cq_hwinfo *cq_hwinfo = tsdrv_get_cq_hwinfo(devid, tsid);
    struct tsdrv_ts_resource *ts_res = tsdrv_get_ts_resoruce(devid, fid, tsid);
    struct devdrv_sq_sub_info *sq_sub = tsdrv_get_sq_sub_info(ts_res, sqcq_alloc->sqId);
    struct devdrv_cq_sub_info *cq_sub = tsdrv_get_cq_sub_info(ts_res, sqcq_alloc->cqId);
    u16 irq_index;
    int ret;

    cqsq_mailbox.sq_index = sqcq_alloc->sqId;
    cqsq_mailbox.sq_addr = sq_sub->phy_addr;
    cqsq_mailbox.sqesize = (u8)sqcq_alloc->sqeSize;
    cqsq_mailbox.sqdepth = (u16)sqcq_alloc->sqeDepth;
    cqsq_mailbox.sq_cq_side |= (sq_sub->addr_side << (int)TSDRV_SQCQ_SIDE_SQ_BIT);
#if !defined(CFG_SOC_PLATFORM_MINI) || defined(CFG_SOC_PLATFORM_MINIV2)
    if (sqcq_alloc->isCtrlType == 1) {
        cqsq_mailbox.sw_reg_flag = 1;
    }
#endif

    cqsq_mailbox.cq0_index = sqcq_alloc->cqId;
    cqsq_mailbox.cq0_addr = cq_sub->phy_addr;
    cqsq_mailbox.cqesize = (u8)sqcq_alloc->cqeSize;
    cqsq_mailbox.cqdepth = (u16)sqcq_alloc->cqeDepth;
    cqsq_mailbox.sq_cq_side |= (cq_sub->addr_side << (int)TSDRV_SQCQ_SIDE_CQ_BIT);
    devdrv_calc_cq_irq_id(sqcq_alloc->cqId, cq_hwinfo->cq_irq_num, DEVDRV_MAX_CQ_NUM, &irq_index);
    cqsq_mailbox.cq_irq = cq_hwinfo->cq_irq[irq_index];

    ret = memcpy_s(cqsq_mailbox.info, sizeof(u32) * SQCQ_RTS_INFO_LENGTH, sqcq_alloc->info,
        sizeof(u32) * SQCQ_RTS_INFO_LENGTH);
    if (ret != EOK) {
        TSDRV_PRINT_ERR("memcpy_s failed, ret = %d\n", ret);
        return ret;
    }

    TSDRV_PRINT_DEBUG("Start to send Msg to Ts. (tgid=%d; devid=%u, fid=%u; tsid=%u;"
        " phy_sqid=%u; sq_pa=0x%pK; sqesize=%u; sqdepth=%u;"
        " phy_cqid=%u; cq_pa=0x%pK; cqesize=%u; cqdepth=%u; cq_irq=%u; sq_cq_side=%u)\n", ctx->tgid, devid, fid, tsid,
        (u32)cqsq_mailbox.sq_index, (void *)(uintptr_t)cqsq_mailbox.sq_addr, (u32)cqsq_mailbox.sqesize,
        (u32)cqsq_mailbox.sqdepth, (u32)cqsq_mailbox.cq0_index,
        (void *)(uintptr_t)cqsq_mailbox.cq0_addr, (u32)cqsq_mailbox.cqesize, (u32)cqsq_mailbox.cqdepth,
        (u32)cqsq_mailbox.cq_irq, (u32)cqsq_mailbox.sq_cq_side);

    ret = devdrv_mailbox_cqsq_inform(&pm_ts_resource->mailbox, ctx, DEVDRV_MAILBOX_CREATE_CTRL_CQSQ, &cqsq_mailbox);
    if (ret != 0) {
        TSDRV_PRINT_ERR("devdrv_mailbox_cqsq_inform failed, (ret=%d, devid=%u; tsid=%u).\n", ret, devid, tsid);
        return ret;
    }

    return 0;
}

int tsdrv_alloc_one_pair_phy_sqcq(struct tsdrv_ctx *ctx, u32 tsid,
    struct normal_alloc_sqcq_para *sqcq_alloc)
{
    struct devdrv_ts_sq_info *sq_info = NULL;
    u32 devid = tsdrv_get_devid_by_ctx(ctx);
    u32 fid = tsdrv_get_fid_by_ctx(ctx);
    int ret;

    if ((sqcq_alloc->flag & TSDRV_SQ_REUSE) != 0 || (sqcq_alloc->flag & TSDRV_CQ_REUSE) != 0) {
        TSDRV_PRINT_ERR("sq or cq reuse is not support. (device_id=%u; tsid=%u; flag=%u)\n",
            devid, tsid, sqcq_alloc->flag);
        return -EINVAL;
    }

    ret = tsdrv_alloc_phy_sqcq(ctx, tsid, sqcq_alloc);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to alloc sqcq id. (device_id=%u; tsid=%u)\n", devid, tsid);
        return ret;
    }

    sq_info = tsdrv_get_sq_info(devid, fid, tsid, sqcq_alloc->sqId);
    if (!tsdrv_is_in_vm(devid)) {
        ret = tsdrv_map_sq_db(ctx, tsid, sqcq_alloc, sq_info);
        if (ret != 0) {
            TSDRV_PRINT_ERR("Failed to map sq doorbell. (devid=%u; sqid=%u)\n", devid, sqcq_alloc->sqId);
            goto err_map_db;
        }

        ret = tsdrv_phy_sqcq_mmap(ctx, tsid, sqcq_alloc);
        if (ret != 0) {
            TSDRV_PRINT_ERR("Failed to mmap sqcq space. (device_id=%u; tsid=%u)\n", devid, tsid);
            goto err_shm_sq_map;
        }
    }

    ret = tsdrv_alloc_phy_cqsq_mbox_send(ctx, tsid, sqcq_alloc);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to send mbox. (device_id=%u; tsid=%u)\n", devid, tsid);
        goto err_send_mbox;
    }

    TSDRV_PRINT_EVENT("alloc one pair cqsq success. (device_id=%u; tsid=%u; sq_id=%u; cq_id=%u; logic_cqid=%u)\n",
        devid, tsid, sqcq_alloc->sqId, sqcq_alloc->cqId, sqcq_alloc->info[0]);

    return 0;
err_send_mbox:
    if (!tsdrv_is_in_vm(devid)) {
        (void)tsdrv_phy_sqcq_unmap(ctx, tsid, sqcq_alloc->sqId, sqcq_alloc->cqId);
    }
err_shm_sq_map:
    if (!tsdrv_is_in_vm(devid)) {
        (void)tsdrv_unmap_sq_db(ctx, tsid, sq_info);
    }
err_map_db:
    (void)tsdrv_free_phy_sqcq(ctx, tsid, sqcq_alloc->sqId, sqcq_alloc->cqId);
    return ret;
}

STATIC int tsdrv_free_one_pair_phy_sqcq_check(struct tsdrv_ctx *ctx, u32 tsid,
    struct normal_free_sqcq_para *sqcq_free)
{
    struct tsdrv_ts_resource *ts_resource = tsdrv_ctx_to_ts_res(ctx, tsid);
    struct devdrv_ts_sq_info *sq_info = NULL;
    u32 devid = tsdrv_get_devid_by_ctx(ctx);
    u32 fid = tsdrv_get_fid_by_ctx(ctx);
    int ret;

    if ((sqcq_free->flag & TSDRV_SQ_REUSE) != 0 || (sqcq_free->flag & TSDRV_CQ_REUSE) != 0) {
        TSDRV_PRINT_ERR("sq or cq reuse is not support. (device_id=%u; tsid=%u; flag=%u)\n",
            devid, tsid, sqcq_free->flag);
        return -EINVAL;
    }

    ret = tsdrv_proc_sq_exist_check(ts_resource, ctx, sqcq_free->sqId, CTRL_SQCQ_TYPE);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Sq id not exist. (device_id=%u; tsid=%u; sq_id=%u)\n", devid, tsid, sqcq_free->sqId);
        return -EINVAL;
    }

    sq_info = tsdrv_get_sq_info(devid, fid, tsid, sqcq_free->sqId);
    if (sqcq_free->cqId != sq_info->bind_cqid) {
        TSDRV_PRINT_ERR("The cqid is not matched with sqid. (devid=%u; fid=%u; free_cqid=%u; bind_cqid=%u)\n",
            devid, fid, sqcq_free->cqId, sq_info->bind_cqid);
        return -EINVAL;
    }
    ret = tsdrv_cq_exist_check(ts_resource, ctx, sqcq_free->cqId);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Cq id not exist. (device_id=%u; tsid=%u; cq_id=%u)\n", devid, tsid, sqcq_free->cqId);
        return -EINVAL;
    }

    return 0;
}

int tsdrv_free_phy_cqsq_mbox_send(struct tsdrv_ctx *ctx, u32 tsid, struct normal_free_sqcq_para *sqcq_free)
{
    struct devdrv_normal_cqsq_mailbox cqsq_mailbox = { 0 };
    u32 devid = tsdrv_get_devid_by_ctx(ctx);
    struct tsdrv_ts_resource *pm_ts_resource = tsdrv_get_ts_resoruce(devid, TSDRV_PM_FID, tsid);
    int ret;

    cqsq_mailbox.cq0_addr = 0;
    cqsq_mailbox.sq_addr = 0;
    cqsq_mailbox.cq0_index = sqcq_free->cqId;
    cqsq_mailbox.sq_index = sqcq_free->sqId;

    pm_ts_resource = tsdrv_get_ts_resoruce(devid, TSDRV_PM_FID, tsid);
    ret = devdrv_mailbox_cqsq_inform(&pm_ts_resource->mailbox, ctx, DEVDRV_MAILBOX_RELEASE_CTRL_CQSQ, &cqsq_mailbox);
    if (ret != 0) {
        TSDRV_PRINT_ERR("devdrv_mailbox_cqsq_inform failed, ret(%d), tsid(%u).\n", ret, tsid);
        return ret;
    }

    return 0;
}

STATIC int tsdrv_free_one_pair_phy_sqcq(struct tsdrv_ctx *ctx, u32 tsid,
    struct normal_free_sqcq_para *sqcq_free)
{
    u32 devid = tsdrv_get_devid_by_ctx(ctx);
    u32 fid = tsdrv_get_fid_by_ctx(ctx);
    int ret;

    ret = tsdrv_free_one_pair_phy_sqcq_check(ctx, tsid, sqcq_free);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to free_one_pair_phy_sqcq_check. (device_id=%u; tsid=%u)\n",
            devid, tsid);
        return ret;
    }

    if (!tsdrv_is_in_vm(devid)) {
        struct devdrv_ts_sq_info *sq_info = tsdrv_get_sq_info(devid, fid, tsid, sqcq_free->sqId);
        ret = tsdrv_phy_sqcq_unmap(ctx, tsid, sqcq_free->sqId, sqcq_free->cqId);
        if (ret != 0) {
            TSDRV_PRINT_ERR("Failed to tsdrv_phy_sqcq_unmap. (device_id=%u; tsid=%u; sq_id=%u; cq_id=%u)\n",
                devid, tsid, sqcq_free->sqId, sqcq_free->cqId);
            return ret;
        }

        ret = tsdrv_unmap_sq_db(ctx, tsid, sq_info);
        if (ret != 0) {
#ifndef TSDRV_UT
            TSDRV_PRINT_ERR("Failed to sq db unmap. (device_id=%u; tsid=%u; sq_id=%u)\n",
                devid, tsid, sqcq_free->sqId);
#endif
            return ret;
        }
    }

    ret = tsdrv_free_phy_cqsq_mbox_send(ctx, tsid, sqcq_free);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to tsdrv_alloc_phy_cqsq_mbox_send. (device_id=%u; tsid=%u)\n",
            devid, tsid);
        return ret;
    }

    ret = tsdrv_free_phy_sqcq(ctx, tsid, sqcq_free->sqId, sqcq_free->cqId);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to tsdrv_free_phy_sqcq. (device_id=%u; tsid=%u; sq_id=%u; cq_id=%u)\n",
            devid, tsid, sqcq_free->sqId, sqcq_free->cqId);
        return ret;
    }

    TSDRV_PRINT_EVENT("Free sqcq success. (devid=%u; tsid=%u; sqid=%u; cqid=%u)\n",
        devid, tsid, sqcq_free->sqId, sqcq_free->cqId);
    return 0;
}

bool tsdrv_is_support_ctrl_sqcq(u32 devid)
{
    u32 chip_type = devdrv_get_dev_chip_type(devid);
    if ((chip_type == HISI_MINI_V1) || (!tsdrv_is_in_pm(devid))) {
        return false;
    }
    return true;
}

static int tsdrv_ctrl_sqcq_alloc(struct tsdrv_ctx *ctx, u32 tsid, struct normal_alloc_sqcq_para *sqcq_alloc)
{
    struct tsdrv_ts_resource *ts_resource = tsdrv_ctx_to_ts_res(ctx, tsid);
    struct logic_sqcq_ts_ctx *logic_ctx = NULL;
    u32 devid = tsdrv_get_devid_by_ctx(ctx);
    u32 fid = tsdrv_get_fid_by_ctx(ctx);
    int ret;

    if (!tsdrv_is_support_ctrl_sqcq(devid)) {
        TSDRV_PRINT_DEBUG("Not support.\n");
        return -EOPNOTSUPP;
    }
    logic_ctx = (struct logic_sqcq_ts_ctx *)ctx->ts_ctx[tsid].logic_sqcq_ctx;
    if (logic_ctx == NULL) {
        TSDRV_PRINT_ERR("Logic ctx is NULL. (devid=%u; fid=%u; tsid=%u; tgid=%d)\n",
            devid, fid, tsid, ctx->tgid);
        return -ENODEV;
    }
    mutex_lock(&logic_ctx->lock);
    ret = logic_cq_is_belong_to_proc(devid, fid, tsid, sqcq_alloc->info[0],
        ctx->ts_ctx[tsid].logic_sqcq_ctx);
    if (ret != 0) {
        mutex_unlock(&logic_ctx->lock);
        TSDRV_PRINT_ERR("Logic cq is not belong to this process."
            "(devid=%u; fid=%u; tsid=%u; logic_cqid=%u; ret=%d)\n",
            devid, fid, tsid, sqcq_alloc->info[0], ret);
        return ret;
    }
    mutex_unlock(&logic_ctx->lock);
    mutex_lock(&ts_resource->id_res[TSDRV_SQ_ID].id_mutex_t);
    ret = tsdrv_alloc_one_pair_phy_sqcq(ctx, tsid, sqcq_alloc);
    if (ret != 0) {
        mutex_unlock(&ts_resource->id_res[TSDRV_SQ_ID].id_mutex_t);
        TSDRV_PRINT_ERR("Failed to alloc ctrl sqcq. (devid=%u; fid=%u; tsid=%u; tgid=%d)\n",
            devid, fid, tsid, ctx->tgid);
        return ret;
    }
    mutex_unlock(&ts_resource->id_res[TSDRV_SQ_ID].id_mutex_t);

    return 0;
}

static int tsdrv_ctrl_sqcq_free(struct tsdrv_ctx *ctx, u32 tsid, struct normal_free_sqcq_para *sqcq_free)
{
    struct tsdrv_ts_resource *ts_resource = tsdrv_ctx_to_ts_res(ctx, tsid);
    u32 devid = tsdrv_get_devid_by_ctx(ctx);
    u32 fid = tsdrv_get_fid_by_ctx(ctx);
    int ret;

    if (!tsdrv_is_support_ctrl_sqcq(devid)) {
        TSDRV_PRINT_DEBUG("Not support.\n");
        return -EOPNOTSUPP;
    }
    mutex_lock(&ts_resource->id_res[TSDRV_SQ_ID].id_mutex_t);
    ret = tsdrv_free_one_pair_phy_sqcq(ctx, tsid, sqcq_free);
    if (ret != 0) {
        mutex_unlock(&ts_resource->id_res[TSDRV_SQ_ID].id_mutex_t);
        TSDRV_PRINT_ERR("Failed to free ctrl sqcq. (devid=%u; fid=%u; tsid=%u; tgid=%d)\n",
            devid, fid, tsid, ctx->tgid);
        return ret;
    }
    mutex_unlock(&ts_resource->id_res[TSDRV_SQ_ID].id_mutex_t);

    return 0;
}

#define VM_PAGE_SIZE_4K  4096U
#define VM_PAGE_SIZE_64K 65536U
int devdrv_ioctl_sqcq_alloc(struct tsdrv_ctx *ctx, struct devdrv_ioctl_arg *arg)
{
    struct devdrv_normal_cqsq_mailbox tmp_sqcq = { 0 };
    struct normal_alloc_sqcq_para *sqcq_alloc = NULL;
    struct tsdrv_ts_resource *pm_ts_resource = NULL;
    struct tsdrv_ts_resource *ts_resource = NULL;
    struct devdrv_ts_cq_info *cq_info = NULL;
    struct devdrv_ts_sq_info *sq_info = NULL;
    struct tsdrv_stream_info *stream_info = NULL;
    u32 devid = tsdrv_get_devid_by_ctx(ctx);
    u32 fid = tsdrv_get_fid_by_ctx(ctx);
    struct tsdrv_id_inst id_inst;
    phys_addr_t info_mem_addr;
    int ret;
#ifdef CFG_SOC_PLATFORM_MDC_V51
    int type = tsdrv_get_chip_type();
#else
    int type = CHIP_TYPE_NOT_SET;
#endif

    ts_resource = tsdrv_ctx_to_ts_res(ctx, arg->tsid);
    sqcq_alloc = &arg->sqcq_alloc_para;
    ret = tsdrv_sqcq_alloc_para_check(sqcq_alloc);
    if (ret != 0) {
        return ret;
    }

    sqcq_alloc->sqeSize = DEVDRV_SQ_SLOT_SIZE;
    if (sqcq_alloc->isCtrlType == 1) {
        return tsdrv_ctrl_sqcq_alloc(ctx, arg->tsid, sqcq_alloc);
    }

    if (type != CHIP_TYPE_MDC_BS9SX1A) {
        tsdrv_pack_id_inst(devid, fid, ts_resource->tsid, &id_inst);
        ret = tsdrv_id_is_belong_to_proc(&id_inst, ctx->tgid, sqcq_alloc->info[0], TSDRV_STREAM_ID);
        if (ret != 0) {
            TSDRV_PRINT_ERR("StreamId is not belong to this process. (devid=%u; fid=%u; tsid=%u; streamId=%u; "
                "ret=%d)\n", id_inst.devid, id_inst.fid, id_inst.tsid, sqcq_alloc->info[0], ret);
            return ret;
        }
    }

    mutex_lock(&ts_resource->id_res[TSDRV_SQ_ID].id_mutex_t);
    /* while sqId is not enough, ret is -EAGAIN, the function devdrv_ioctl_sqcq_alloc will return 0 */
    ret = devdrv_cqsq_alloc(ts_resource, ctx, sqcq_alloc, &tmp_sqcq);
    if (ret != 0) {
        mutex_unlock(&ts_resource->id_res[TSDRV_SQ_ID].id_mutex_t);
        /* sq is not enough, runtime needs to consider reusing sq */
        if (ret == -EAGAIN) {
            return 0;
        } else {
            TSDRV_PRINT_ERR("Failed to alloc sqcq. (ret=%d; tsid=%u; tgid=%d)\n", ret, ts_resource->tsid,
                ctx->tgid);
            return -EINVAL;
        }
    }
    /* sqid bind streamid is for cb normal task security. */
    stream_info = &ts_resource->stream_info[sqcq_alloc->info[0]];
    stream_info->sq = (u32)tmp_sqcq.sq_index;

    /* The variables cqesize and cqdepth are not used by drv and ts, so no judgment is made */
    tmp_sqcq.sqesize = (u8)sqcq_alloc->sqeSize;
    tmp_sqcq.cqesize = (u8)sqcq_alloc->cqeSize;
    tmp_sqcq.sqdepth = (u16)sqcq_alloc->sqeDepth;
    tmp_sqcq.cqdepth = (u16)sqcq_alloc->cqeDepth;
    ret = memcpy_s(tmp_sqcq.info, sizeof(u32) * SQCQ_RTS_INFO_LENGTH, sqcq_alloc->info,
        sizeof(u32) * SQCQ_RTS_INFO_LENGTH);
    if (ret != EOK) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("memcpy_s failed, ret = %d\n", ret);
        goto ERROR;
#endif
    }

    TSDRV_PRINT_DEBUG("Send Msg to Ts. (tgid=%d; phy_sqid=%u; phy_cqid=%u)\n", ctx->tgid, (u32)tmp_sqcq.sq_index,
        (u32)tmp_sqcq.cq0_index);
    pm_ts_resource = tsdrv_get_ts_resoruce(devid, TSDRV_PM_FID, arg->tsid);
    ret = devdrv_mailbox_cqsq_inform(&pm_ts_resource->mailbox, ctx, DEVDRV_MAILBOX_CREATE_CQSQ_CALC, &tmp_sqcq);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("devdrv_mailbox_cqsq_inform failed, ret(%d), tsid(%u).\n", ret, ts_resource->tsid);
        goto ERROR;
#endif
    }
    if (tsdrv_is_in_vm(devid)) {
        struct tsdrv_dev_resource *dev_res = tsdrv_get_dev_resource(devid, fid);
        if ((sqcq_alloc->res[0] != VM_PAGE_SIZE_4K) && (sqcq_alloc->res[0] != VM_PAGE_SIZE_64K)) {
#ifndef TSDRV_UT
            TSDRV_PRINT_ERR("Vm page size is invalid. (page_size=%u)\n", sqcq_alloc->res[0]);
            goto ERROR;
#endif
        }
        dev_res->page_size = sqcq_alloc->res[0];
        TSDRV_PRINT_DEBUG("Set vm page size. (devid=%u; fid=%u; page_size=%u)\n", devid, fid, dev_res->page_size);
    }

    mutex_unlock(&ts_resource->id_res[TSDRV_SQ_ID].id_mutex_t);
    TSDRV_PRINT_DEBUG("Alloc sqcq success. (devid=%u; sqid=%u; cqid=%u)\n",
        devid, sqcq_alloc->sqId, sqcq_alloc->cqId);
    return 0;
#ifndef TSDRV_UT
ERROR:
    info_mem_addr = ts_resource->mem_info[DEVDRV_INFO_MEM].virt_addr;
    if ((sqcq_alloc->flag & TSDRV_SQ_REUSE) == 0) {
        sq_info = devdrv_calc_sq_info(info_mem_addr, ts_resource->id_res[TSDRV_SQ_ID].pm_id[sqcq_alloc->sqId]);
        (void)devdrv_exit_sq(ts_resource, ctx, sq_info);
        tsdrv_free_sq(ts_resource, ctx, sq_info);
    }
    if ((sqcq_alloc->flag & TSDRV_CQ_REUSE) == 0) {
        cq_info = devdrv_calc_cq_info(info_mem_addr, ts_resource->id_res[TSDRV_CQ_ID].pm_id[sqcq_alloc->cqId]);
        (void)devdrv_exit_cq(ctx, ts_resource->tsid, cq_info->cq_sub);
        devdrv_free_cq(ts_resource, ctx, cq_info);
    }
    mutex_unlock(&ts_resource->id_res[TSDRV_SQ_ID].id_mutex_t);
    return -EINVAL;
#endif
}

STATIC int tsdrv_free_sqcq_check(struct tsdrv_ts_resource *ts_res, struct tsdrv_ctx *ctx,
    struct normal_free_sqcq_para *sqcq_free)
{
    int ret;

    if ((sqcq_free->flag & TSDRV_CQ_REUSE) == 0) {
        /* Check if the cqId to be released is legal */
        ret = tsdrv_cq_exist_check(ts_res, ctx, sqcq_free->cqId);
        if (ret != 0) {
            return -EINVAL;
        }
    }
    /* Check if the sqId to be released is legal */
    ret = tsdrv_sq_exist_check(ts_res, ctx, sqcq_free->sqId);
    if (ret != 0) {
        return -EINVAL;
    }
    return 0;
}

int devdrv_ioctl_sqcq_free(struct tsdrv_ctx *ctx, struct devdrv_ioctl_arg *arg)
{
    struct devdrv_normal_cqsq_mailbox tmp_sqcq = { 0 };
    struct normal_free_sqcq_para *sqcq_free = &arg->sqcq_free_para;
    struct tsdrv_ts_resource *pm_ts_resource = NULL;
    struct tsdrv_ts_resource *ts_resource = NULL;
    u16 tmp_cqId = DEVDRV_MAILBOX_INVALID_INDEX;
    u32 devid = tsdrv_get_devid_by_ctx(ctx);
    int ret;

    if (sqcq_free->isCtrlType == 1) {
        return tsdrv_ctrl_sqcq_free(ctx, arg->tsid, sqcq_free);
    }

    ts_resource = tsdrv_ctx_to_ts_res(ctx, arg->tsid);

    mutex_lock(&ts_resource->id_res[TSDRV_SQ_ID].id_mutex_t);
    ret = tsdrv_free_sqcq_check(ts_resource, ctx, sqcq_free);
    if (ret != 0) {
        mutex_unlock(&ts_resource->id_res[TSDRV_SQ_ID].id_mutex_t);
        TSDRV_PRINT_ERR("free_sqcq_check failed, ret(%d), tsid(%u), flag(%u).\n", ret, ts_resource->tsid,
            arg->sqcq_free_para.flag);
        return ret;
    }

    if ((sqcq_free->flag & TSDRV_CQ_REUSE) == 0) {
        tmp_cqId = (u16)tsdrv_vrit_to_physic_id(&ts_resource->id_res[TSDRV_CQ_ID], sqcq_free->cqId);
    }

    tmp_sqcq.cq0_addr = 0;
    tmp_sqcq.sq_addr = 0;
    tmp_sqcq.cq0_index = tmp_cqId;
    tmp_sqcq.sq_index = (u16)tsdrv_vrit_to_physic_id(&ts_resource->id_res[TSDRV_SQ_ID], sqcq_free->sqId);

    pm_ts_resource = tsdrv_get_ts_resoruce(devid, TSDRV_PM_FID, arg->tsid);
    if ((sqcq_free->flag & TSDRV_SQ_REUSE) == 0) {
        ret = devdrv_mailbox_cqsq_inform(&pm_ts_resource->mailbox, ctx, DEVDRV_MAILBOX_RELEASE_CQSQ_CALC, &tmp_sqcq);
        if (ret != 0) {
#ifndef TSDRV_UT
            mutex_unlock(&ts_resource->id_res[TSDRV_SQ_ID].id_mutex_t);
            TSDRV_PRINT_ERR("devdrv_mailbox_cqsq_inform failed, ret(%d), tsid(%u).\n", ret, ts_resource->tsid);
            return ret;
#endif
        }
    }

    devdrv_cqsq_free_id(ts_resource, ctx, sqcq_free->sqId, sqcq_free->cqId, sqcq_free->flag);
    mutex_unlock(&ts_resource->id_res[TSDRV_SQ_ID].id_mutex_t);

    TSDRV_PRINT_DEBUG("Free sqcq success. (devid=%u; sqid=%u; cqid=%u)\n", devid, sqcq_free->sqId, sqcq_free->cqId);
    return 0;
}

int tsdrv_ioctl_get_sq_head(struct tsdrv_ctx *ctx, struct devdrv_ioctl_arg *arg)
{
#ifndef TSDRV_UT
    struct tsdrv_ts_resource *ts_resource = NULL;
    struct devdrv_ts_sq_info *sq_info = NULL;
    phys_addr_t info_mem_addr;
    u32 sq_id;
    int ret;
#ifdef CFG_FEATURE_STARS
    u32 devid = tsdrv_get_devid_by_ctx(ctx);
    u32 tsid = arg->tsid;
#endif
    sq_id = arg->sqcq_data_para.id;
    if (sq_id >= DEVDRV_MAX_SQ_NUM) {
        TSDRV_PRINT_ERR("invalid para, sq_id(%u).\n", sq_id);
        return -EINVAL;
    }

    ts_resource = tsdrv_ctx_to_ts_res(ctx, arg->tsid);
    mutex_lock(&ts_resource->id_res[TSDRV_SQ_ID].id_mutex_t);
    ret = tsdrv_sq_exist_check(ts_resource, ctx, sq_id);
    if (ret != 0) {
        mutex_unlock(&ts_resource->id_res[TSDRV_SQ_ID].id_mutex_t);
        TSDRV_PRINT_ERR("sq_id(%u) is not exist.\n", sq_id);
        return -EINVAL;
    }

    info_mem_addr = ts_resource->mem_info[DEVDRV_INFO_MEM].virt_addr;
    sq_info = devdrv_calc_sq_info(info_mem_addr, sq_id);

#ifdef CFG_FEATURE_SUPPORT_VSQ_MNG
    tsdrv_get_sq_head(devid, tsid, sq_id, &sq_info->head);
    tsdrv_update_vsq_head(ts_resource, sq_id, sq_info->head);
    TSDRV_PRINT_DEBUG("Update vs head by read reg. (sqid=%u, sqHead=%u\n)", sq_id, sq_info->head);
    arg->sqcq_data_para.val = tsdrv_get_vsq_head(ts_resource, sq_id);
#else
    arg->sqcq_data_para.val = sq_info->head;
    TSDRV_PRINT_DEBUG("Update sq head. (sqid=%u, sq_info->head=%u, arg->sqcq_data_para.val=%u\n)",
        sq_id, sq_info->head, arg->sqcq_data_para.val);
#endif
    mutex_unlock(&ts_resource->id_res[TSDRV_SQ_ID].id_mutex_t);
#endif
    return 0;
}


int tsdrv_sync_id_maping_mailbox(struct devdrv_mailbox *mailbox,
    struct tsdrv_ctx *ctx,
    struct resource_mapping_notice_mailbox_t *msg)
{
#ifdef CFG_FEATURE_ID_MAPING
    int result = 0;
    u32 len;
    int ret;

    msg->vf_id = tsdrv_get_fid_by_ctx(ctx);
    if (msg->vf_id == 0) {
        return 0;
    }
    /* add message header */
    msg->head.valid = DEVDRV_MAILBOX_MESSAGE_VALID;
    msg->head.cmd_type = TSDRV_MBOX_RESOURCE_MAPPING_NOTICE;
    msg->head.result = 0;
    msg->host_pid = ctx->tgid;
    len = sizeof(struct resource_mapping_notice_mailbox_t);
    ret = devdrv_mailbox_kernel_sync_no_feedback(mailbox, (u8 *)msg, len, &result);
    if (ret == 0) {
        ret = result;
    }
    return ret;
#else
    return 0;
#endif
}

int tsdrv_ioctl_sq_msg_send(struct tsdrv_ctx *ctx, struct devdrv_ioctl_arg *arg)
{
    struct tsdrv_ts_resource *ts_res = NULL;
    struct tsdrv_db_hwinfo_t *db_hwinfo = NULL;
    struct devdrv_ts_sq_info *sq_info = NULL;
    struct tsdrv_id_info *sq_id_info = NULL;
    struct tsdrv_device *tsdrv_dev = NULL;
    struct devdrv_sq_sub_info *sq_sub = NULL;
    struct list_head *pos = NULL;
    struct list_head *n = NULL;
    phys_addr_t info_mem_addr;
    bool sq_exist = false;
    u32 *doorbell = NULL;
    u32 sq_id, sq_tail;

#ifdef CFG_FEATURE_SUPPORT_VSQ_MNG
    return tsdrv_vsq_msg_send(ctx, arg);
#endif

    sq_id = arg->sqcq_data_para.id;
    sq_tail = arg->sqcq_data_para.val;
    if ((sq_id >= DEVDRV_MAX_SQ_NUM) || (sq_tail >= DEVDRV_MAX_SQ_DEPTH)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("invalid para, sq_id(%u), sq_tail(%u).\n", sq_id, sq_tail);
        return -EINVAL;
#endif
    }

    ts_res = tsdrv_ctx_to_ts_res(ctx, arg->tsid);
    tsdrv_dev = tsdrv_res_to_dev(ts_res);
    spin_lock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
    if (list_empty_careful(&ctx->ts_ctx[ts_res->tsid].id_ctx[TSDRV_SQ_ID].id_list) != 0) {
#ifndef TSDRV_UT
        spin_unlock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
        TSDRV_PRINT_ERR("sq_list is empty.\n");
        return -EINVAL;
#endif
    }
    list_for_each_safe(pos, n, &ctx->ts_ctx[ts_res->tsid].id_ctx[TSDRV_SQ_ID].id_list) {
        sq_id_info = list_entry(pos, struct tsdrv_id_info, list);
        if (sq_id_info->id == sq_id) {
            sq_exist = true;
            break;
        }
    }
    if (sq_exist != true) {
#ifndef TSDRV_UT
        spin_unlock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
        TSDRV_PRINT_ERR("sq_id=%u doesn't exist\n", sq_id);
        return -ENODEV;
#endif
    }

    info_mem_addr = ts_res->mem_info[DEVDRV_INFO_MEM].virt_addr;
    sq_info = devdrv_calc_sq_info(info_mem_addr, sq_id);
    if (sq_tail >= sq_info->depth) {
#ifndef TSDRV_UT
        spin_unlock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
        TSDRV_PRINT_ERR("invalid para, sqid(%u), depth(%u), tail(%u)\n", sq_id, sq_info->depth, sq_tail);
        return -EINVAL;
#endif
    }

    sq_sub = tsdrv_get_sq_sub_info(ts_res, sq_id);
    if (sq_sub->addr_side == TSDRV_MEM_ON_HOST_SIDE) {
        dma_sync_single_for_device(tsdrv_dev->dev, sq_sub->phy_addr, sq_sub->queue_size, DMA_TO_DEVICE);
    }

    db_hwinfo = tsdrv_get_db_hwinfo_t(tsdrv_dev->devid, arg->tsid, TSDRV_SQ_DB);
    doorbell = db_hwinfo->dbs + ((long)(unsigned)sq_id * db_hwinfo->db_stride) + DEVDRV_SQ_TAIL_OFFSET;
    writel(sq_tail, (void __iomem *)doorbell);

    sq_info->tail = sq_tail;
    arg->sqcq_data_para.val = sq_info->head;

    spin_unlock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);

    TSDRV_PRINT_DEBUG("send sq msg, tsid(%u), sqid(%u), depth(%u), head(%u), tail(%u)\n",
        arg->tsid, sq_id, sq_info->depth, sq_info->head, sq_tail);

    return 0;
}

static void tsdrv_get_sq_status(u32 devid, u32 tsid, u32 sqid, u32 *sq_status)
{
    struct tsdrv_db_hwinfo_t *db_hwinfo = tsdrv_get_db_hwinfo_t(devid, tsid, TSDRV_SQ_DB);
    void __iomem *sq_info = NULL;

    sq_info = db_hwinfo->dbs + ((long)(unsigned)sqid * db_hwinfo->db_stride) + DEVDRV_SQ_STATUS_OFFSET;
    *sq_status = readl(sq_info);
}

static void tsdrv_get_sq_head(u32 devid, u32 tsid, u32 sqid, u32 *sq_head)
{
    struct tsdrv_db_hwinfo_t *db_hwinfo = tsdrv_get_db_hwinfo_t(devid, tsid, TSDRV_SQ_DB);
    void __iomem *sq_info = NULL;
    sq_info = db_hwinfo->dbs + ((long)(unsigned)sqid * db_hwinfo->db_stride) + DEVDRV_SQ_HEAD_OFFSET;
    *sq_head = readl(sq_info);
}

static void tsdrv_get_sq_tail(u32 devid, u32 tsid, u32 sqid, u32 *sq_tail)
{
    struct tsdrv_db_hwinfo_t *db_hwinfo = tsdrv_get_db_hwinfo_t(devid, tsid, TSDRV_SQ_DB);
    void __iomem *sq_info = NULL;

    sq_info = db_hwinfo->dbs + ((long)(unsigned)sqid * db_hwinfo->db_stride) + DEVDRV_SQ_TAIL_OFFSET;
    *sq_tail = readl(sq_info);
}

#if defined(CFG_SOC_PLATFORM_CLOUD_V2) || defined(CFG_SOC_PLATFORM_MINIV3)
static bool tsdrv_sq_enabled(u32 devid, u32 tsid, u32 sqid)
{
    struct tsdrv_db_hwinfo_t *db_hwinfo = tsdrv_get_db_hwinfo_t(devid, tsid, TSDRV_SQ_DB);
    void __iomem *sq_cfg = NULL;
    u32 reg_value;

    sq_cfg = db_hwinfo->dbs + ((long)(unsigned)sqid * db_hwinfo->db_stride) + DEVDRV_SQ_STATUS_OFFSET;
    reg_value = readl(sq_cfg);
    if ((reg_value & 0x1) == 1) {
        return true;
    }
    return false;
}

static void tsdrv_set_sq_head(u32 devid, u32 tsid, u32 sqid, u32 value)
{
    struct tsdrv_db_hwinfo_t *db_hwinfo = tsdrv_get_db_hwinfo_t(devid, tsid, TSDRV_SQ_DB);
    void __iomem *sq_cfg = NULL;

    sq_cfg = db_hwinfo->dbs + ((long)(unsigned)sqid * db_hwinfo->db_stride) + DEVDRV_SQ_HEAD_OFFSET;

    writel(value, sq_cfg);
}

static void tsdrv_sq_enable_disable(u32 devid, u32 tsid, u32 sqid, int sq_enable)
{
    struct tsdrv_db_hwinfo_t *db_hwinfo = NULL;
    void __iomem *sq_cfg = NULL;
    u32 reg_value;

    db_hwinfo = tsdrv_get_db_hwinfo_t(devid, tsid, TSDRV_SQ_DB);
    sq_cfg = db_hwinfo->dbs + ((long)(unsigned)sqid * db_hwinfo->db_stride) + DEVDRV_SQ_STATUS_OFFSET;
    reg_value = readl(sq_cfg);

    if (sq_enable == 0) {
        /* Disable */
        if ((reg_value & 0x1) == 0) {
            return;
        }
        reg_value &= ~0x1;
    } else {
        /* Enable */
        if ((reg_value & 0x1) == 1) {
            return;
        }
        reg_value |= 0x1;
    }
    writel(reg_value, sq_cfg);
}

static int tsdrv_ioctl_sqcq_set_status(struct tsdrv_ctx *ctx, u32 tsid, u32 sqid, u32 value)
{
    struct tsdrv_ts_resource *ts_res = tsdrv_ctx_to_ts_res(ctx, tsid);
    u32 devid = tsdrv_get_devid_by_ctx(ctx);

    spin_lock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
    if (!is_sq_belong_to_proc(ctx, tsid, sqid)) {
        spin_unlock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
        TSDRV_PRINT_ERR("Sqid is not belong to this proc. (devid=%u; tsid=%u; sqid=%u; value=%u)\n",
            devid, tsid, sqid, value);
        return -ENODEV;
    }
    tsdrv_sq_enable_disable(devid, tsid, sqid, value);
    spin_unlock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
    return 0;
}

static int tsdrv_ioctl_sq_enable_wait(struct tsdrv_ctx *ctx, u32 tsid, u32 sqid, unsigned long timeout)
{
    struct tsdrv_ts_resource *ts_res = tsdrv_ctx_to_ts_res(ctx, tsid);
    u32 devid = tsdrv_get_devid_by_ctx(ctx);
    const unsigned long max_timeout = 5000;
    unsigned long sleep_time = 1; // sleep 1ms for each loop in begining
    unsigned long total_sleep_time = 0;
    const u32 sleep_time_adjust = 10;
    u32 cnt = 0;
    int ret = 0;

    if (timeout > max_timeout) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Invalid timeout. (timeout=%lums; max_timeout=%lums)\n", timeout, max_timeout);
#endif
        return -ENODEV;
    }

    spin_lock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
    while (true) {
        if (is_sq_belong_to_proc(ctx, tsid, sqid) == false) {
            spin_unlock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
            TSDRV_PRINT_ERR("Sqid is not belong to this proc. (devid=%u; tsid=%u; sqid=%u)\n", devid, tsid, sqid);
            return -ENODEV;
        }
        if (tsdrv_sq_enabled(devid, tsid, sqid) == false) {
            tsdrv_sq_enable_disable(devid, tsid, sqid, 0x1);
            ret = 0;
            break;
        }
        if (((++cnt) % sleep_time_adjust) == 0) { /* Adjust sleep time every 10 loop count */
            sleep_time = sleep_time * sleep_time_adjust;
        }
        total_sleep_time += sleep_time;
        if (time_after(total_sleep_time, timeout)) {
            ret = -ETIMEDOUT;
            break;
        }
        spin_unlock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
        if (sleep_time < 10) { /* sleep time smaller than 10ms, use usleep_range */
            usleep_range(sleep_time * USEC_PER_MSEC, sleep_time * USEC_PER_MSEC + 1);
        } else {
            (void)msleep_interruptible(sleep_time);
        }
        spin_lock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
    }
    spin_unlock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);

    if (ret == -ETIMEDOUT) {
        TSDRV_PRINT_WARN("Wait for sq enable timeout. "
            "(devid=%u; tsid=%u; sqid=%u; timeout=%lums; total_sleep_time=%lums; "
            "cnt=%u; sleep_time=%lums)\n", devid, tsid, sqid, timeout, total_sleep_time, cnt, sleep_time);
    }

    return ret;
}

static int tsdrv_ioctl_sqcq_set_sq_head(struct tsdrv_ctx *ctx, u32 tsid, u32 sqid, u32 value)
{
    struct tsdrv_ts_resource *ts_res = tsdrv_ctx_to_ts_res(ctx, tsid);
    struct devdrv_ts_sq_info *sq_info = NULL;
    u32 devid = tsdrv_get_devid_by_ctx(ctx);
    u32 fid = tsdrv_get_fid_by_ctx(ctx);

    spin_lock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
    if (!is_sq_belong_to_proc(ctx, tsid, sqid)) {
        spin_unlock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
        TSDRV_PRINT_ERR("Sqid is not belong to this ctx. (devid=%u; tsid=%u; sqid=%u)\n", devid, tsid, sqid);
        return -ENODEV;
    }

    sq_info = tsdrv_get_sq_info(devid, fid, tsid, sqid);
    if (value >= sq_info->depth) {
#ifndef TSDRV_UT
        spin_unlock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
        TSDRV_PRINT_ERR("Invalid value. (devid=%u; tsid=%u; sqid=%u; value=%u; sq_depth=%u)\n",
            devid, tsid, sqid, value, sq_info->depth);
        return -EINVAL;
#endif
    }

    if (tsdrv_sq_enabled(devid, tsid, sqid) == true) {
        spin_unlock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
        TSDRV_PRINT_DEBUG("Sq is enabled, do not set head ptr. (devid=%u; tsid=%u; sqid=%u)\n", devid, tsid, sqid);
        return -EFAULT;
    }

    tsdrv_set_sq_head(devid, tsid, sqid, value);
    spin_unlock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);

    return 0;
}
#endif

int tsdrv_ioctl_sqcq_set(struct tsdrv_ctx *ctx, struct devdrv_ioctl_arg *arg)
{
#if !defined(CFG_SOC_PLATFORM_CLOUD_V2) && !defined(CFG_SOC_PLATFORM_MINIV3)
    return -EINVAL;
#else
    enum tsdrv_sqcq_set_prop prop;
    u32 sqid, value;
    int ret = 0;

    sqid = arg->sqcq_set.sq_id;
    prop = (enum tsdrv_sqcq_set_prop)arg->sqcq_set.prop;
    value = arg->sqcq_set.value;
    if ((sqid >= DEVDRV_MAX_SQ_NUM) || (prop >= TSDRV_SQCQ_SET_PROP_MAX)) {
        TSDRV_PRINT_ERR("Invalid para. (sqid=%u; prop=%d)\n", sqid, (int)prop);
        return -EINVAL;
    }

    switch (prop) {
        case TSDRV_SQCQ_SET_PROP_STATUS:
            ret = tsdrv_ioctl_sqcq_set_status(ctx, arg->tsid, sqid, value);
            break;
        case TSDRV_SQCQ_SET_PROP_SQ_HEAD:
            ret = tsdrv_ioctl_sqcq_set_sq_head(ctx, arg->tsid, sqid, value);
            break;
        case TSDRV_SQCQ_SQ_DISABLE_TO_ENABLE:
            ret = tsdrv_ioctl_sq_enable_wait(ctx, arg->tsid, sqid, value);
            break;
        default:
            TSDRV_PRINT_ERR("Unknown prop. (prop=%d)\n", (int)prop);
            break;
    }
    return ret;
#endif
}

int tsdrv_ioctl_sqcq_query(struct tsdrv_ctx *ctx, struct devdrv_ioctl_arg *arg)
{
    struct tsdrv_sqcq_query_para *para = NULL;
    u32 devid, tsid, sqid, sq_status, sq_head, sq_tail;

    para = (struct tsdrv_sqcq_query_para*)&arg->sqcq_query_para;
    devid = tsdrv_get_devid_by_ctx(ctx);
    tsid = arg->tsid;
    sqid = para->sqid;

    if (!tsdrv_is_sq_belong_to_proc(ctx, tsid, sqid)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Sq is not belong this ctx. (devid=%u, tsid=%u, sqid=%u)\n",
            devid, tsid, sqid);
#endif
        return -EINVAL;
    }

    switch (para->prop) {
        case TSDRV_SQCQ_QUERY_PROP_SQ_STATUS:
            tsdrv_get_sq_status(devid, tsid, sqid, &sq_status);
            arg->sqcq_query_para.value = sq_status;
            break;
        case TSDRV_SQCQ_QUERY_PROP_SQ_HEAD:
            tsdrv_get_sq_head(devid, tsid, sqid, &sq_head);
            arg->sqcq_query_para.value = sq_head;
            break;
        case TSDRV_SQCQ_QUERY_PROP_SQ_TAIL:
            tsdrv_get_sq_tail(devid, tsid, sqid, &sq_tail);
            arg->sqcq_query_para.value = sq_tail;
            break;
        default:
#ifndef TSDRV_UT
            TSDRV_PRINT_ERR("Invalid query prop. (devid=%u, sqid=%u, prop=%u)\n",
                devid, sqid, para->prop);
#endif
            return -EINVAL;
    }

    return 0;
}

STATIC int tsdrv_ioctl_set_stream_sq_status(struct tsdrv_ctx *ctx, struct devdrv_ioctl_arg *arg, u32 status)
{
#if !defined(CFG_SOC_PLATFORM_CLOUD_V2) && !defined(CFG_SOC_PLATFORM_MINIV3)
    return -EINVAL;
#else
    struct tsdrv_ts_resource *ts_res = NULL;
    u32 devid = tsdrv_get_devid_by_ctx(ctx);
    enum devdrv_process_type cp_type;
    u32 chip_id, vfid;
    u32 host_pid;
    u32 stream_id;
    int ret;

    if (tsdrv_get_env_type() != TSDRV_ENV_OFFLINE) {
        TSDRV_PRINT_ERR("Do not suuport\n");
        return -ENODEV;
    }

    stream_id = arg->id_para.res_id;
    if (stream_id >= tsdrv_get_stream_id_max_num(devid, arg->tsid)) {
        TSDRV_PRINT_ERR("invalid para, stream_id(%u).\n", stream_id);
        return -EINVAL;
    }
    ts_res = tsdrv_ctx_to_ts_res(ctx, arg->tsid);
    if (ts_res->stream_info[stream_id].valid == 0) {
        TSDRV_PRINT_ERR("stream_id %u not valid.\n", stream_id);
        return -EINVAL;
    }

    ret = devdrv_query_process_host_pid(ctx->tgid, &chip_id, &vfid, &host_pid, &cp_type);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Query host pid fail. (ret=%d)\n", ret);
        return ret;
    }

    if (host_pid != (u32)ts_res->stream_info[stream_id].remote_tgid) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Query host pid fail. (host_pid=%u; remote_tgid=%d)\n", host_pid,
            ts_res->stream_info[stream_id].remote_tgid);
#endif
        return -ENODEV;
    }

    tsdrv_sq_enable_disable(devid, arg->tsid, ts_res->stream_info[stream_id].sq, status);
    return 0;
#endif
}

int tsdrv_ioctl_enable_stream(struct tsdrv_ctx *ctx, struct devdrv_ioctl_arg *arg)
{
    return tsdrv_ioctl_set_stream_sq_status(ctx, arg, 1);
}

int tsdrv_ioctl_disable_stream(struct tsdrv_ctx *ctx, struct devdrv_ioctl_arg *arg)
{
    return tsdrv_ioctl_set_stream_sq_status(ctx, arg, 0);
}

int tsdrv_ioctl_cq_report_release(struct tsdrv_ctx *ctx, struct devdrv_ioctl_arg *arg)
{
    struct tsdrv_ts_resource *ts_resource = tsdrv_ctx_to_ts_res(ctx, arg->tsid);
    struct tsdrv_device *tsdrv_dev = tsdrv_res_to_dev(ts_resource);
    struct devdrv_ts_cq_info *cq_info = NULL;
    phys_addr_t info_mem_addr;
    u32 cq_id, release_head;
    int ret;

    cq_id = arg->sqcq_data_para.id;
    release_head = arg->sqcq_data_para.val;
    if ((cq_id >= DEVDRV_MAX_CQ_NUM) || (release_head >= DEVDRV_MAX_CQ_DEPTH)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("invalid para, cq_id(%u), release_head(%u).\n", cq_id, release_head);
        return -EINVAL;
#endif
    }

    mutex_lock(&ts_resource->id_res[TSDRV_SQ_ID].id_mutex_t);
    ret = tsdrv_cq_exist_check(ts_resource, ctx, cq_id);
    if (ret != 0) {
#ifndef TSDRV_UT
        mutex_unlock(&ts_resource->id_res[TSDRV_SQ_ID].id_mutex_t);
        TSDRV_PRINT_ERR("cq_id(%u) is not exist.\n", cq_id);
        return -EINVAL;
#endif
    }

    info_mem_addr = ts_resource->mem_info[DEVDRV_INFO_MEM].virt_addr;
    cq_info = devdrv_calc_cq_info(info_mem_addr, cq_id);
    if (release_head >= cq_info->depth) {
#ifndef TSDRV_UT
        mutex_unlock(&ts_resource->id_res[TSDRV_SQ_ID].id_mutex_t);
        TSDRV_PRINT_ERR("invalid para, cqid(%u), depth(%u), head(%u).\n", cq_id, cq_info->depth, release_head);
        return -EINVAL;
#endif
    }

    tsdrv_set_cq_doorbell(tsdrv_dev->devid, arg->tsid, cq_id, release_head);
    tsdrv_update_cq_head(ts_resource, cq_id, release_head);

    TSDRV_PRINT_DEBUG("cq_id(%u) head(%u) offset(%d).\n", cq_id, release_head, DEVDRV_CQ_HEAD_OFFSET);

    mutex_unlock(&ts_resource->id_res[TSDRV_SQ_ID].id_mutex_t);
    return 0;
}

int devdrv_alloc_sq_msg_chan(u32 devid, u32 tsid, struct tsdrv_msg_resource_id *dev_msg_resource_id)
{
    struct tsdrv_id_info *sq_id_info = NULL;
    struct tsdrv_ts_resource *ts_res = NULL;
    u32 req_id_num;
    u32 j = 0;
    u32 i;

    req_id_num = dev_msg_resource_id->req_id_num;
    if (req_id_num > DEVDRV_MANAGER_MSG_ID_NUM_MAX) {
        TSDRV_PRINT_ERR("info input err, req id num = %u.\n", req_id_num);
        return -EINVAL;
    }

    ts_res = tsdrv_get_ts_resoruce(devid, TSDRV_PM_FID, tsid);

    spin_lock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
    if (ts_res->id_res[TSDRV_SQ_ID].id_available_num < req_id_num) {
        req_id_num = ts_res->id_res[TSDRV_SQ_ID].id_available_num;
    }

    for (i = 0; i < req_id_num; i++) {
        sq_id_info = list_first_entry(&ts_res->id_res[TSDRV_SQ_ID].id_available_list,
            struct tsdrv_id_info, list);
        list_del(&sq_id_info->list);
        ts_res->id_res[TSDRV_SQ_ID].id_available_num--;
        dev_msg_resource_id->id[j++] = sq_id_info->id;
        dev_msg_resource_id->ret_id_num++;
        TSDRV_PRINT_DEBUG("sq_id = %d, sq_num = %u, ret_id_num = %u\n", sq_id_info->id,
            ts_res->id_res[TSDRV_SQ_ID].id_available_num,
            dev_msg_resource_id->ret_id_num);
    }
    spin_unlock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
    return 0;
}

int devdrv_alloc_cq_msg_chan(u32 devid, u32 tsid, struct tsdrv_msg_resource_id *dev_msg_resource_id)
{
    struct tsdrv_id_info *cq_id_info = NULL;
    struct tsdrv_ts_resource *ts_res = NULL;
    u32 req_id_num;
    u32 i;
    u32 j = 0;

    if ((dev_msg_resource_id == NULL) || (dev_msg_resource_id->req_id_num > DEVDRV_MANAGER_MSG_ID_NUM_MAX)) {
        TSDRV_PRINT_ERR("info input err, id = %pK.\n", dev_msg_resource_id);
        return -EINVAL;
    }
    ts_res = tsdrv_get_ts_resoruce(devid, TSDRV_PM_FID, tsid);
    req_id_num = dev_msg_resource_id->req_id_num;

    spin_lock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
    if (ts_res->id_res[TSDRV_CQ_ID].id_available_num < req_id_num) {
        req_id_num = ts_res->id_res[TSDRV_CQ_ID].id_available_num;
    }

    for (i = 0; i < req_id_num; i++) {
        cq_id_info = list_first_entry(&ts_res->id_res[TSDRV_CQ_ID].id_available_list,
            struct tsdrv_id_info, list);
        list_del(&cq_id_info->list);
        ts_res->id_res[TSDRV_CQ_ID].id_available_num--;
        dev_msg_resource_id->id[j++] = cq_id_info->phy_id;
        dev_msg_resource_id->ret_id_num++;
        TSDRV_PRINT_DEBUG("devid=%u tsid=%u cq_id=%d, cq_num=%u, ret_id_num=%u\n", devid, tsid,
            cq_id_info->phy_id, ts_res->id_res[TSDRV_CQ_ID].id_available_num,
            dev_msg_resource_id->ret_id_num);
    }
    spin_unlock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
    return 0;
}

int devdrv_add_sq_msg_chan(u32 devid, u32 tsid, struct tsdrv_msg_resource_id *dev_msg_resource_id)
{
    struct tsdrv_id_info *sq_id_info = NULL;
    struct tsdrv_ts_resource *ts_res = NULL;
    u32 i;

    if ((dev_msg_resource_id->ret_id_num == 0) || (dev_msg_resource_id->ret_id_num > DEVDRV_MANAGER_MSG_ID_NUM_MAX)) {
        TSDRV_PRINT_ERR("info input err, id=%u.\n", dev_msg_resource_id->ret_id_num);
        return -EINVAL;
    }
    ts_res = tsdrv_get_ts_resoruce(devid, TSDRV_PM_FID, tsid);

    spin_lock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
    for (i = 0; i < dev_msg_resource_id->ret_id_num; i++) {
        sq_id_info = devdrv_find_one_id(&ts_res->id_res[TSDRV_SQ_ID], dev_msg_resource_id->id[i]);
        if (sq_id_info == NULL) {
#ifndef TSDRV_UT
            spin_unlock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
            TSDRV_PRINT_ERR("id(%u %u) is invalid.\n", i, (u32)dev_msg_resource_id->id[i]);
            return -EINVAL;
#endif
        }
        ts_res->id_res[TSDRV_SQ_ID].pm_id[sq_id_info->id] = sq_id_info->phy_id;
        list_add_tail(&sq_id_info->list, &ts_res->id_res[TSDRV_SQ_ID].id_available_list);
        ts_res->id_res[TSDRV_SQ_ID].id_available_num++;
        TSDRV_PRINT_DEBUG("sq_id = %d, sq_num = %u, ret_id_num = %u\n", sq_id_info->phy_id,
            ts_res->id_res[TSDRV_SQ_ID].id_available_num,
            dev_msg_resource_id->ret_id_num);
    }
    spin_unlock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);

    return 0;
}

int devdrv_add_cq_msg_chan(u32 devid, u32 tsid, struct tsdrv_msg_resource_id *dev_msg_resource_id)
{
    struct tsdrv_id_info *cq_id_info = NULL;
    struct tsdrv_ts_resource *ts_res = NULL;
    u32 i;

    if ((dev_msg_resource_id == NULL) || (dev_msg_resource_id->ret_id_num > DEVDRV_MANAGER_MSG_ID_NUM_MAX)) {
        TSDRV_PRINT_ERR("info input err, id=%pK.\n", dev_msg_resource_id);
        return -EINVAL;
    }
    ts_res = tsdrv_get_ts_resoruce(devid, TSDRV_PM_FID, tsid);

    spin_lock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
    for (i = 0; i < dev_msg_resource_id->ret_id_num; i++) {
        cq_id_info = devdrv_find_one_id(&ts_res->id_res[TSDRV_CQ_ID], dev_msg_resource_id->id[i]);
        if (cq_id_info == NULL) {
#ifndef TSDRV_UT
            spin_unlock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
            TSDRV_PRINT_ERR("id(%u %u) is invalid.\n", i, (u32)dev_msg_resource_id->id[i]);
            return -EINVAL;
#endif
        }
        ts_res->id_res[TSDRV_CQ_ID].pm_id[cq_id_info->id] = cq_id_info->phy_id;
        list_add_tail(&cq_id_info->list, &ts_res->id_res[TSDRV_CQ_ID].id_available_list);
        ts_res->id_res[TSDRV_CQ_ID].id_available_num++;
        TSDRV_PRINT_DEBUG("devid=%u tsid=%u cq_id=%d, sq_num=%u, ret_id_num = %u\n", devid, tsid, cq_id_info->phy_id,
            ts_res->id_res[TSDRV_CQ_ID].id_available_num,
            dev_msg_resource_id->ret_id_num);
    }
    spin_unlock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);

    return 0;
}

struct devdrv_ts_sq_info *tsdrv_get_sq_info(u32 devid, u32 fid, u32 tsid, u32 sq_id)
{
    struct tsdrv_ts_resource *ts_res = NULL;
    struct devdrv_ts_sq_info *sq_info = NULL;
    phys_addr_t phy_sq_base;

    /* info mem is stored in fid0 currently */
    ts_res = tsdrv_get_ts_resoruce(devid, fid, tsid);
    if (!ts_res->mem_info[DEVDRV_INFO_MEM].virt_addr) {
        TSDRV_PRINT_ERR("pm info res is NULL, devid=%u fid=%u tsid=%u\n", devid, fid, tsid);
        return NULL;
    }
    phy_sq_base = ts_res->mem_info[DEVDRV_INFO_MEM].virt_addr;
    sq_info = devdrv_calc_sq_info(phy_sq_base, sq_id);
    return sq_info;
}

int tsdrv_phy_sq_head_update(u32 devid, u32 fid, u32 tsid, u32 sq_id, u32 sq_head)
{
#ifndef TSDRV_UT
    struct devdrv_ts_sq_info *sq_info = NULL;
    struct tsdrv_id_info *sq_id_info = NULL;
    struct tsdrv_ts_resource *ts_res = NULL;

    sq_info = tsdrv_get_sq_info(devid, TSDRV_PM_FID, tsid, sq_id);
    if (sq_info == NULL) {
        TSDRV_PRINT_ERR("get sq info failed. sq_id=%u, devid=%u, tsid=%u\n", sq_id, devid, tsid);
        return -ENODEV;
    }
    rmb();
    if (sq_info->alloc_status != SQCQ_ACTIVE || sq_head >= sq_info->depth) {
        TSDRV_PRINT_WARN("PhySq is not allocated, abandon this cqe.(sq_id=%u status=%d devid=%u tsid=%u)\n", sq_id,
            (int)sq_info->alloc_status, devid, tsid);
        /*
         * When phy sq id has been freed, cqe needs to be abandoned to ensure that
         * it does not block other sq head updates
         */
        return 0;
    }
    sq_info->head = sq_head;

    if (tsdrv_is_in_pm(devid) == false) {
        ts_res = tsdrv_get_ts_resoruce(devid, fid, tsid);

        sq_id_info = devdrv_find_one_id(&ts_res->id_res[TSDRV_SQ_ID], sq_info->index);
        if (sq_id_info == NULL) {
            return -ENODEV;
        }
        TSDRV_PRINT_DEBUG("Update vsq head. (devid=%u; fid=%u; sqid=%u; vsqid=%d)\n",
            devid, fid, sq_info->index, sq_id_info->virt_id);
#ifndef CFG_SOC_PLATFORM_MDC_V51
        hvtsdrv_update_vsq_head(ts_res, sq_id_info->virt_id, sq_head);
#endif
    }

    TSDRV_PRINT_DEBUG("[SQ INFO]:tgid=%d, head=%u, tail=%u, depth=%u, index=%u\n",
        sq_info->tgid,
        sq_info->head,
        sq_info->tail,
        sq_info->depth,
        sq_info->index);
#endif /* TSDRV_UT */
    return 0;
}

/* get phyical sqId info from freelist */
int tsrdv_get_phy_sqid(u32 devid, u32 fid, u32 tsid, struct devdrv_sq_sub_info **sq_sub)
{
    struct tsdrv_ts_resource *ts_res = tsdrv_get_ts_resoruce(devid, fid, tsid);
    struct devdrv_ts_sq_info *ts_sq_info = NULL;
    struct tsdrv_id_info *sq_id_info = NULL;
    phys_addr_t info_mem_addr;

    spin_lock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
    if (ts_res->id_res[TSDRV_SQ_ID].id_available_num == 0) {
        if (tsdrv_is_in_pm(devid)) {
            spin_unlock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
            (void)tsdrv_msg_alloc_sync_sq(devid, tsid);
            spin_lock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
        }
        if (ts_res->id_res[TSDRV_SQ_ID].id_available_num == 0) {
            spin_unlock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
            TSDRV_PRINT_WARN("no enough sq, devid=%u fid=%u tsid=%u\n", devid, fid, tsid);
            *sq_sub = NULL;
            return NO_RES;
        }
    }
    sq_id_info = list_first_entry(&ts_res->id_res[TSDRV_SQ_ID].id_available_list,
        struct tsdrv_id_info, list);
    list_del(&sq_id_info->list);
    ts_res->id_res[TSDRV_SQ_ID].id_available_num--;

    info_mem_addr = ts_res->mem_info[DEVDRV_INFO_MEM].virt_addr;
    ts_sq_info = devdrv_calc_sq_info(info_mem_addr, sq_id_info->phy_id);
    spin_unlock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);

    *sq_sub = (struct devdrv_sq_sub_info *)ts_sq_info->sq_sub;
    return 0;
}

/* put phyical sqId info back to freelist */
STATIC_INLINE void tsrdv_put_phy_sqid(struct tsdrv_ts_resource *ts_res,
    struct devdrv_sq_sub_info *sq_sub)
{
    struct tsdrv_id_info *sq_id_info = NULL;
    spin_lock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
    sq_id_info = devdrv_find_one_id(&ts_res->id_res[TSDRV_SQ_ID], (int)sq_sub->index);
    if (sq_id_info == NULL) {
        spin_unlock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
        return;
    }

    list_add(&sq_id_info->list, &ts_res->id_res[TSDRV_SQ_ID].id_available_list);
    ts_res->id_res[TSDRV_SQ_ID].id_available_num++;
    spin_unlock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
}

int tsdrv_ring_phy_cq_doorbell(u32 devid, u32 tsid, u32 cq_id, u32 cq_head)
{
#ifndef TSDRV_UT
    struct tsdrv_db_hwinfo_t *db_hwinfo = NULL;
    void __iomem *cq_db = NULL;

    db_hwinfo = tsdrv_get_db_hwinfo_t(devid, tsid, TSDRV_CQ_DB);
    cq_db = db_hwinfo->dbs + ((long)(unsigned)cq_id * db_hwinfo->db_stride) + DEVDRV_CQ_HEAD_OFFSET;
    writel(cq_head, cq_db);
#endif
    return 0;
}

int tsdrv_host_alloc_dma_sqcqmem(struct tsdrv_cqsq_mem_alloc_free_para *para)
{
    para->size = PAGE_ALIGN(para->size);
    para->virt_addr = (phys_addr_t)(uintptr_t)tsdrv_alloc_pages_exact(para->size, GFP_KERNEL);
    if (para->virt_addr == 0) {
        TSDRV_PRINT_ERR("kzalloc fail, no mem for rtsq. (size=%u)\n", para->size);
        return -ENOMEM;
    }

    para->phy_addr = dma_map_single(tsdrv_get_dev_p(para->dev_id), (void *)(uintptr_t)para->virt_addr,
        para->size, DMA_BIDIRECTIONAL);
    if (dma_mapping_error(tsdrv_get_dev_p(para->dev_id), para->phy_addr) != 0) {
        TSDRV_PRINT_ERR("dev_id(%u) dma map single error !\n", para->dev_id);
        tsdrv_free_pages_exact((void *)(uintptr_t)para->virt_addr, para->size);
        return -ENOMEM;
    }

    para->addr_side = TSDRV_MEM_ON_HOST_SIDE;
    TSDRV_PRINT_DEBUG("alloc dma addr ok, id=%u id_type=%u, size=%u, virt_addr=0x%pK,"
        "phy_addr=0x%pK, addr_side=%u\n",
        para->id, para->id_type, para->size, (void *)(uintptr_t)para->virt_addr,
        (void *)(uintptr_t)para->phy_addr, para->addr_side);
    return 0;
}

void tsdrv_host_free_dma_sqcqmem(struct tsdrv_cqsq_mem_alloc_free_para *para)
{
    dma_unmap_single(tsdrv_get_dev_p(para->dev_id), para->phy_addr, para->size, DMA_BIDIRECTIONAL);
    tsdrv_free_pages_exact((void *)(uintptr_t)para->virt_addr, para->size);
}

int tsdrv_host_alloc_bar_sqcqmem(struct tsdrv_cqsq_mem_alloc_free_para *para)
{
#ifndef AOS_LLVM_BUILD
    u32 chip_type = devdrv_get_dev_chip_type(para->dev_id);
#else
    u32 chip_type = HISI_MINI_V2;
#endif
    struct tsdrv_sq_hwinfo *sq_hwinfo =
        tsdrv_get_ts_sq_hwinfo(para->dev_id, para->tsid);
    struct tsdrv_ts_resource *pm_ts_res =
        tsdrv_get_ts_resoruce(para->dev_id, TSDRV_PM_FID, para->tsid);

    if (chip_type == HISI_MINI_V3 && para->id_type == DEVDRV_CQ_MEM) {
        para->bar_addr = sq_hwinfo->bar_addr + DEVDRV_RESERVE_CQ_MEM_OFFSET +
            ((unsigned long)para->id *
            (unsigned long)DEVDRV_CQ_SLOT_SIZE *
            (unsigned long)DEVDRV_MAX_CQ_DEPTH);

        para->phy_addr = DEVDRV_RESERVE_MEM_BASE + DEVDRV_RESERVE_CQ_MEM_OFFSET +
            (long)(unsigned)para->dev_id * CHIP_BASEADDR_PA_OFFSET +
            (unsigned long)para->id *
            (unsigned long)DEVDRV_CQ_SLOT_SIZE *
            (unsigned long)DEVDRV_MAX_CQ_DEPTH;

        para->virt_addr = pm_ts_res->mem_info[DEVDRV_SQ_MEM].virt_addr +
            DEVDRV_RESERVE_CQ_MEM_OFFSET +
            (unsigned long)para->id *
            (unsigned long)DEVDRV_CQ_SLOT_SIZE *
            (unsigned long)DEVDRV_MAX_CQ_DEPTH;
    } else {
        sq_hwinfo = tsdrv_get_ts_sq_hwinfo(para->dev_id, para->tsid);
        para->phy_addr = sq_hwinfo->paddr +
            ((unsigned long)para->id * para->size);
        para->bar_addr = sq_hwinfo->bar_addr +
            ((unsigned long)para->id * para->size);
        para->virt_addr = pm_ts_res->mem_info[DEVDRV_SQ_MEM].virt_addr +
            ((unsigned long)para->id * para->size);
    }
    memset_io((void *)(uintptr_t)(para->virt_addr), 0, para->size);

    para->addr_side = TSDRV_MEM_ON_DEVICE_SIDE;
    TSDRV_PRINT_DEBUG("alloc bar addr ok, id=%u id_type=%u, size=%u, bar_addr=%pK, "
        "virt_addr=%pK, phy_addr=%pK, addr_side=%u\n",
        para->id, para->id_type, para->size, (void *)(uintptr_t)para->bar_addr,
        (void *)(uintptr_t)para->virt_addr, (void *)(uintptr_t)para->phy_addr, para->addr_side);
    return 0;
}

void tsdrv_host_free_bar_sqcqmem(struct tsdrv_cqsq_mem_alloc_free_para *para)
{
    para->phy_addr = 0;
    para->bar_addr = 0;
    para->virt_addr = 0;
}

int tsdrv_dev_alloc_ts_node_sqcqmem(struct tsdrv_cqsq_mem_alloc_free_para *para)
{
    para->size = PAGE_ALIGN(para->size);
#ifndef AOS_LLVM_BUILD
    para->virt_addr = (phys_addr_t)(uintptr_t)tsdrv_alloc_pages_exact_nid(
        devdrv_manager_devid_to_nid(para->dev_id, (u32)DEVDRV_TS_NODE_DDR_MEM),
        para->size, GFP_HIGHUSER_MOVABLE | __GFP_THISNODE);
#else
    para->virt_addr = kmalloc(para->size, GFP_USER);
#endif
    if (para->virt_addr == 0) {
        TSDRV_PRINT_ERR("[dev_id=%u]:kzalloc fail, no mem for sq\n", para->dev_id);
        return -ENOMEM;
    }
    tsdrv_get_drv_ops()->flush_cache(para->virt_addr, para->size);
    para->phy_addr = virt_to_phys((void *)(uintptr_t)para->virt_addr);
    para->addr_side = TSDRV_MEM_ON_DEVICE_SIDE;

    TSDRV_PRINT_DEBUG("alloc ts_node addr ok, id=%u id_type=%u, size=%u, "
        "virt_addr=%pK, phy_addr=%pK, addr_side=%u\n",
        para->id, para->id_type, para->size, (void *)(uintptr_t)para->virt_addr,
        (void *)(uintptr_t)para->phy_addr, para->addr_side);

    return 0;
}

void tsdrv_dev_free_ts_node_sqcqmem(struct tsdrv_cqsq_mem_alloc_free_para *para)
{
    tsdrv_free_pages_exact((void *)(uintptr_t)para->virt_addr, para->size);
}

int tsdrv_dev_alloc_reserve_sqcqmem(struct tsdrv_cqsq_mem_alloc_free_para *para)
{
#ifndef AOS_LLVM_BUILD
    u32 chip_type = devdrv_get_dev_chip_type(para->dev_id);
#else
    u32 chip_type = HISI_MINI_V2;
#endif
    struct tsdrv_sq_hwinfo *sq_hwinfo =
        tsdrv_get_ts_sq_hwinfo(para->dev_id, para->tsid);
    struct tsdrv_ts_resource *pm_ts_res =
        tsdrv_get_ts_resoruce(para->dev_id, TSDRV_PM_FID, para->tsid);
    int err;

    if (chip_type == HISI_MINI_V2 && para->id_type == DEVDRV_CQ_MEM) {
        if (para->size > CQ_RESERVE_MEM_CQ_OFFSET) {
            TSDRV_PRINT_ERR("cq que_size=0x%x larger than cq_offset=0x%lx, devid=%u tsid=%u.\n", para->size,
                CQ_RESERVE_MEM_CQ_OFFSET, para->dev_id, para->tsid);
            return -EFAULT;
        }
        para->phy_addr = (phys_addr_t)CQ_RESERVE_MEM_BASE + ((phys_addr_t)para->dev_id * CHIP_BASEADDR_PA_OFFSET) +
                   ((phys_addr_t)para->tsid * (phys_addr_t)CQ_RESERVE_MEM_SIZE) +
                   ((phys_addr_t)para->id * (phys_addr_t)CQ_RESERVE_MEM_CQ_OFFSET);
        para->virt_addr = (phys_addr_t)(uintptr_t)ioremap_wc(para->phy_addr, para->size);
        if (para->virt_addr == 0) {
            TSDRV_PRINT_ERR("ioremap fail, no mem for cq\n");
            return -ENOMEM;
        }
        err = memset_s((void *)(uintptr_t)para->virt_addr, para->size, 0, para->size);
        if (err != 0) {
            TSDRV_PRINT_ERR("cq uio memset failed.\n");
            iounmap((void *)(uintptr_t)para->virt_addr);
            return -EINVAL;
        }
        wmb();
    } else {
        sq_hwinfo = tsdrv_get_ts_sq_hwinfo(para->dev_id, para->tsid);
        para->phy_addr = sq_hwinfo->paddr + ((unsigned long)para->id * para->size);
        para->virt_addr = pm_ts_res->mem_info[DEVDRV_SQ_MEM].virt_addr + ((unsigned long)para->id * para->size);
    }

    para->addr_side = TSDRV_MEM_ON_DEVICE_SIDE;
    TSDRV_PRINT_DEBUG("alloc reserve addr ok, id=%u id_type=%u, size=%u, virt_addr=%pK,"
        "phy_addr=%pK, addr_side=%u\n",
        para->id, para->id_type, para->size, (void *)(uintptr_t)para->virt_addr,
        (void *)(uintptr_t)para->phy_addr, para->addr_side);
    return 0;
}

void tsdrv_dev_free_reserve_sqcqmem(struct tsdrv_cqsq_mem_alloc_free_para *para)
{
#ifdef CFG_SOC_PLATFORM_MINIV2
    if (para->id_type == DEVDRV_CQ_MEM) {
        iounmap((void __iomem *)(uintptr_t)para->virt_addr);
    }
#endif
    para->phy_addr = 0;
    para->virt_addr = 0;
}

int tsrdv_phy_sq_dev_mem_init(struct tsdrv_cqsq_mem_alloc_free_para *para)
{
    int err;

#ifdef CFG_SOC_PLATFORM_CLOUD_V2
    err = tsdrv_dev_alloc_ts_node_sqcqmem(para);
    if (err != 0) {
        TSDRV_PRINT_ERR("[dev_id=%u,tsid=%u,sqid=%u]:tsdrv_alloc_ts_node_sqcqmem fail, no mem for sq\n",
            para->dev_id, para->tsid, para->id);
        return err;
    }
#else
    err = tsdrv_dev_alloc_reserve_sqcqmem(para);
    if (err != 0) {
        TSDRV_PRINT_ERR("[dev_id=%u,tsid=%u,sqid=%u]:tsdrv_dev_alloc_reserve_sqcqmem fail, no mem for sq\n",
            para->dev_id, para->tsid, para->id);
        return err;
    }
#endif

    return 0;
}

int tsrdv_phy_sq_host_mem_init(struct tsdrv_cqsq_mem_alloc_free_para *para)
{
    int err;

#ifdef CFG_SOC_PLATFORM_CLOUD_V2
    err = tsdrv_host_alloc_dma_sqcqmem(para);
    if (err != 0) {
        TSDRV_PRINT_ERR("tsdrv_alloc_dma_mem fail, devid=%u tsid=%u sqid=%u\n",
            para->dev_id, para->tsid, para->id);
        return err;
    }
#else
    err = tsdrv_host_alloc_bar_sqcqmem(para);
    if (err != 0) {
        TSDRV_PRINT_ERR("tsdrv_alloc_dma_mem fail, devid=%u tsid=%u sqid=%u\n",
            para->dev_id, para->tsid, para->id);
        return err;
    }
#endif

    return 0;
}

STATIC int tsrdv_phy_sq_mem_init(u32 devid, u32 tsid, struct devdrv_sq_sub_info *sq_sub,
    struct phy_sq_alloc_para *para)
{
    enum tsdrv_env_type env_type = tsdrv_get_env_type();
    struct tsdrv_cqsq_mem_alloc_free_para alloc_para = {0};
    int err;

    alloc_para.dev_id = devid;
    alloc_para.tsid = tsid;
    alloc_para.size = para->sq_size * para->sq_depth;
    alloc_para.id= sq_sub->index;
    alloc_para.id_type = DEVDRV_SQ_MEM;
    switch (env_type) {
        case TSDRV_ENV_OFFLINE:
            err = tsrdv_phy_sq_dev_mem_init(&alloc_para);
            if (err != 0) {
                TSDRV_PRINT_ERR("tsrdv_phy_sq_dev_mem_init fail, devid=%u tsid=%u sqid=%u\n",
                    devid, tsid, sq_sub->index);
                return err;
            }
            break;
        case TSDRV_ENV_ONLINE:
            err = tsrdv_phy_sq_host_mem_init(&alloc_para);
            if (err != 0) {
                TSDRV_PRINT_ERR("tsrdv_phy_sq_host_mem_init fail, devid=%u tsid=%u sqid=%u\n",
                    devid, tsid, sq_sub->index);
                return err;
            }
            break;
        default:
#ifndef TSDRV_UT
            TSDRV_PRINT_ERR("invalid env_type=%d, devid=%u tsid=%u]\n", (int)env_type, devid, tsid);
#endif
            return -ENODEV;
    }

    sq_sub->bar_addr = alloc_para.bar_addr;
    sq_sub->phy_addr = alloc_para.phy_addr;
    sq_sub->vaddr = alloc_para.virt_addr;
    sq_sub->addr_side = alloc_para.addr_side;
    sq_sub->queue_size = alloc_para.size;
    sq_sub->depth = para->sq_depth;
    sq_sub->size = para->sq_size;
    TSDRV_PRINT_DEBUG("alloc sq mem success. (sq_id=%u; bar_addr=%pK; phy_addr=%pK;"
        "vaddr=%pK; addr_side=%u; queue_size=%u; slot_size=%lu; depth=%u)\n",
        sq_sub->index, (void *)(uintptr_t)sq_sub->bar_addr, (void *)(uintptr_t)sq_sub->phy_addr,
        (void *)(uintptr_t)sq_sub->vaddr, sq_sub->addr_side,
        sq_sub->queue_size, sq_sub->size, sq_sub->depth);

    return err;
}

void tsrdv_phy_sq_dev_mem_exit(struct tsdrv_cqsq_mem_alloc_free_para *para)
{
#ifdef CFG_SOC_PLATFORM_CLOUD_V2
    tsdrv_dev_free_ts_node_sqcqmem(para);
#else
    tsdrv_dev_free_reserve_sqcqmem(para);
#endif
}

void tsrdv_phy_sq_host_mem_exit(struct tsdrv_cqsq_mem_alloc_free_para *para)
{
#ifdef CFG_SOC_PLATFORM_CLOUD_V2
    tsdrv_host_free_dma_sqcqmem(para);
#else
    tsdrv_host_free_bar_sqcqmem(para);
#endif
}

STATIC void tsdrv_phy_sq_mem_exit(u32 devid, u32 tsid, struct devdrv_sq_sub_info *sq_sub)
{
    enum tsdrv_env_type env_type = tsdrv_get_env_type();
    struct tsdrv_cqsq_mem_alloc_free_para free_para = {0};

    free_para.dev_id = devid;
    free_para.tsid = tsid;
    free_para.size = sq_sub->queue_size;
    free_para.id= sq_sub->index;
    free_para.id_type = DEVDRV_SQ_MEM;
    free_para.virt_addr = sq_sub->vaddr;
    free_para.phy_addr = sq_sub->phy_addr;
    free_para.bar_addr = sq_sub->bar_addr;
    switch (env_type) {
        case TSDRV_ENV_OFFLINE:
            tsrdv_phy_sq_dev_mem_exit(&free_para);
            break;
        case TSDRV_ENV_ONLINE:
            tsrdv_phy_sq_host_mem_exit(&free_para);
            break;
        default:
            TSDRV_PRINT_ERR("invalid env_type=%d, devid=%u tsid=%u]\n", (int)env_type, devid, tsid);
            return;
    }

    sq_sub->depth = 0;
    sq_sub->size = 0;
    sq_sub->bar_addr = 0;
    sq_sub->phy_addr = 0;
    sq_sub->vaddr = 0;
    sq_sub->queue_size = 0;
}

/* Add physical sqId to current context and init it */
STATIC int tsdrv_phy_sqid_init(struct tsdrv_ts_resource *ts_res, struct tsdrv_ctx *ctx,
    struct devdrv_sq_sub_info *sq_sub, struct phy_sq_alloc_para *para)
{
    struct tsdrv_id_info *sq_id_info = NULL;
    struct devdrv_ts_sq_info *sq_info = NULL;
    u32 devid = tsdrv_get_devid_by_ctx(ctx);
    struct tsdrv_ts_ctx *ts_ctx = NULL;

    ts_ctx = &ctx->ts_ctx[ts_res->tsid];
    /* use pm sq info address */
    sq_info = tsdrv_get_sq_info(devid, TSDRV_PM_FID, ts_res->tsid, sq_sub->index);
    if (sq_info == NULL) {
        return -ENOMEM;
    }
    spin_lock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
    sq_id_info = devdrv_find_one_id(&ts_res->id_res[TSDRV_SQ_ID], sq_info->index);
    if (sq_id_info == NULL) {
#ifndef TSDRV_UT
        spin_unlock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
        return -ENOMEM;
#endif
    }

    list_add(&sq_id_info->list, &ts_ctx->id_ctx[TSDRV_SQ_ID].id_list);

    ts_ctx->id_ctx[TSDRV_SQ_ID].id_num++;
    sq_info->tgid = ctx->tgid;
    sq_info->type = para->type;
    sq_info->head = 0;
    sq_info->tail = 0;
    sq_info->alloc_status = SQCQ_ACTIVE;
    wmb();

    sq_info->depth = para->sq_depth;
    spin_unlock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
    TSDRV_PRINT_DEBUG("type=%d, sqid=%u\n", (int)sq_info->type, sq_info->index);

    return 0;
}

int tsdrv_cq_type_check(u32 devid, u32 fid, u32 tsid, u32 cqid, enum phy_sqcq_type type)
{
    struct devdrv_ts_cq_info *cq_info = tsdrv_get_cq_info(devid, fid, tsid, cqid);
    if (cq_info == NULL) {
        return -EINVAL;
    }
    if (cq_info->type != type) {
        TSDRV_PRINT_ERR("Invalid id type. (devid=%u; fid=%u; tsid=%u; cqid=%u; type=%d; cq_type=%d)\n",
            devid, fid, tsid, cqid, (int)type, (int)cq_info->type);
        return -EFAULT;
    }
    return 0;
}

int tsdrv_phy_cqid_exist_chk(struct tsdrv_ctx *ctx, u32 tsid, u32 cqid, enum phy_sqcq_type type)
{
    struct tsdrv_id_info *cq_id_info = NULL;
    struct tsdrv_ts_resource *ts_res = NULL;
    struct tsdrv_ts_ctx *ts_ctx = NULL;
    struct list_head *pos = NULL;
    struct list_head *n = NULL;
    u32 devid = tsdrv_get_devid_by_ctx(ctx);
    u32 fid = tsdrv_get_fid_by_ctx(ctx);
    int ret;

    ts_res = tsdrv_ctx_to_ts_res(ctx, tsid);
    ts_ctx = &ctx->ts_ctx[ts_res->tsid];
    spin_lock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
    if (!list_empty_careful(&ts_ctx->id_ctx[TSDRV_CQ_ID].id_list)) {
        list_for_each_safe(pos, n, &ts_ctx->id_ctx[TSDRV_CQ_ID].id_list) {
            cq_id_info = list_entry(pos, struct tsdrv_id_info, list);
            if (cq_id_info->phy_id == cqid) {
                ret = tsdrv_cq_type_check(devid, fid, tsid, cqid, type);
                if (ret != 0) {
                    spin_unlock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
                    TSDRV_PRINT_ERR("Failed to check cq type. (cqid=%u; type=%d; ret=%d)\n", cqid, (int)type, ret);
                    return ret;
                }
                spin_unlock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
                return 0;
            }
        }
    }
    spin_unlock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
    return -EINVAL;
}

int tsdrv_sq_type_check(u32 devid, u32 fid, u32 tsid, u32 sqid, enum phy_sqcq_type type)
{
    struct devdrv_ts_sq_info *sq_info = tsdrv_get_sq_info(devid, fid, tsid, sqid);
    if (sq_info == NULL) {
        return -EINVAL;
    }
    if (sq_info->type != type) {
        TSDRV_PRINT_ERR("Invalid id type. (devid=%u; fid=%u; tsid=%u; sqid=%u; type=%d; sq_type=%d)\n",
            devid, fid, tsid, sqid, (int)type, (int)sq_info->type);
        return -EFAULT;
    }
    return 0;
}

int tsdrv_phy_sqid_exist_chk(struct tsdrv_ctx *ctx, u32 tsid, u32 sqid, enum phy_sqcq_type type)
{
    struct tsdrv_ts_resource *ts_res = NULL;
    struct tsdrv_id_info *id_info = NULL;
    struct tsdrv_ts_ctx *ts_ctx = NULL;
    struct list_head *pos = NULL;
    struct list_head *n = NULL;
    u32 devid = tsdrv_get_devid_by_ctx(ctx);
    u32 fid = tsdrv_get_fid_by_ctx(ctx);
    int ret;

    ts_res = tsdrv_ctx_to_ts_res(ctx, tsid);
    ts_ctx = &ctx->ts_ctx[ts_res->tsid];
    spin_lock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
    if (!list_empty_careful(&ts_ctx->id_ctx[TSDRV_SQ_ID].id_list)) {
        list_for_each_safe(pos, n, &ts_ctx->id_ctx[TSDRV_SQ_ID].id_list) {
            id_info = list_entry(pos, struct tsdrv_id_info, list);
            if (id_info->id == sqid) {
                ret = tsdrv_sq_type_check(devid, fid, tsid, sqid, type);
                if (ret != 0) {
                    spin_unlock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
                    TSDRV_PRINT_ERR("Failed to check sq type. (sqid=%u; type=%d; ret=%d)\n", sqid, (int)type, ret);
                    return ret;
                }
                spin_unlock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
                return 0;
            }
        }
    }
    spin_unlock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
    return -EINVAL;
}

/* Remove physical sqId from current context and return sq_sub */
struct devdrv_sq_sub_info *tsdrv_phy_sqid_exit(struct tsdrv_ts_resource *ts_res,
    struct tsdrv_ctx *ctx, u32 sqid)
{
    struct tsdrv_id_info *sq_id_info = NULL;
    struct devdrv_ts_sq_info *sq_info = NULL;
    struct devdrv_sq_sub_info *sq_sub = NULL;
    u32 devid = tsdrv_get_devid_by_ctx(ctx);
    struct tsdrv_ts_ctx *ts_ctx = NULL;

    ts_ctx = &ctx->ts_ctx[ts_res->tsid];
    sq_info = tsdrv_get_sq_info(devid, TSDRV_PM_FID, ts_res->tsid, sqid);
    if ((sq_info == NULL) || (sq_info->sq_sub == NULL)) {
        TSDRV_PRINT_ERR("get pm sq info base fail, devid=%u tsid=%u\n", devid, ts_res->tsid);
        return NULL;
    }
    sq_sub = sq_info->sq_sub;
    spin_lock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
    sq_info->tgid = 0;
    sq_info->type = NORMAL_SQCQ_TYPE; // default type
    sq_info->head = 0;
    sq_info->tail = 0;
    sq_info->alloc_status = SQCQ_INACTIVE;
    wmb();

    sq_info->depth = DEVDRV_MAX_SQ_DEPTH; // set default value
    sq_info->bind_cqid = DEVDRV_MAX_CQ_NUM;
    sq_id_info = devdrv_find_one_id(&ts_res->id_res[TSDRV_SQ_ID], (int)sq_info->index);
    if (sq_id_info == NULL) {
        spin_unlock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
        return NULL;
    }
    list_del(&sq_id_info->list);
    ts_ctx->id_ctx[TSDRV_SQ_ID].id_num--;
    spin_unlock(&ts_res->id_res[TSDRV_SQ_ID].spinlock);
    return sq_sub;
}

/* handle alloc sq mbox send fail */
void tsdrv_phy_sq_alloc_restore(struct tsdrv_ctx *ctx, u32 tsid, u32 sqid)
{
#ifndef TSDRV_UT
    struct devdrv_sq_sub_info *sq_sub = NULL;
    struct tsdrv_ts_resource *ts_res = NULL;
    u32 devid = tsdrv_get_devid_by_ctx(ctx);
    u32 fid = tsdrv_get_fid_by_ctx(ctx);

    if (tsdrv_is_in_pm(devid) != true) {
        return;
    }

    ts_res = tsdrv_ctx_to_ts_res(ctx, tsid);
    sq_sub = tsdrv_phy_sqid_exit(ts_res, ctx, sqid);
    if (sq_sub == NULL) {
        TSDRV_PRINT_ERR("sqId=%u is not allocated by tgid=%d, devid=%u fid=%u tsid=%u\n", sqid, ctx->tgid, devid,
            fid, tsid);
        return;
    }
    tsdrv_phy_sq_mem_exit(devid, tsid, sq_sub);
    tsrdv_put_phy_sqid(ts_res, sq_sub);
#endif
}

int tsdrv_phy_sq_alloc(struct tsdrv_ctx *ctx, u32 tsid, struct phy_sq_alloc_para *para)
{
    struct devdrv_sq_sub_info *sq_sub = NULL;
    struct tsdrv_ts_resource *ts_res = NULL;
    u32 devid = tsdrv_get_devid_by_ctx(ctx);
    u32 fid = tsdrv_get_fid_by_ctx(ctx);
    int err;

    TSDRV_PRINT_DEBUG("enter devid = %u, tsid = %u, fid = %u.\n", devid, tsid, fid);

    ts_res = tsdrv_ctx_to_ts_res(ctx, tsid);
    err = tsrdv_get_phy_sqid(devid, fid, tsid, &sq_sub);
    if (err != 0) {
        return err;
    }
    err = tsrdv_phy_sq_mem_init(devid, tsid, sq_sub, para);
    if (err != 0) {
        tsrdv_put_phy_sqid(ts_res, sq_sub);
        return -EINVAL;
    }
    err = tsdrv_phy_sqid_init(ts_res, ctx, sq_sub, para);
    if (err != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("add sq to ctx fail, devid=%u fid=%u tsid=%u\n", devid, fid, tsid);
        tsdrv_phy_sq_mem_exit(devid, tsid, sq_sub);
        tsrdv_put_phy_sqid(ts_res, sq_sub);
        return -EFAULT;
#endif
    }

#ifdef CFG_FEATURE_SUPPORT_VSQ_MNG
    err = tsdrv_vsq_mem_create(ts_res, sq_sub->index, para->sq_size, para->sq_depth);
    if (err != 0) {
        TSDRV_PRINT_ERR("devdrv_init_uio vsq failed!\n");
        (void)tsdrv_phy_sqid_exit(ts_res, ctx, sq_sub->index);
        tsdrv_phy_sq_mem_exit(devid, tsid, sq_sub);
        tsrdv_put_phy_sqid(ts_res, sq_sub);
        return err;
    }
#endif

    para->sq_id = sq_sub->index;
    para->sq_bar_addr = sq_sub->bar_addr;
    para->sq_paddr = sq_sub->phy_addr;
    para->sq_vaddr = NULL; // it's not used for now

    TSDRV_PRINT_DEBUG("exit devid = %u, tsid = %u, fid = %u, sq_id = %u.\n",
        devid, tsid, fid, para->sq_id);
    return 0;
}

int tsdrv_phy_sq_free(struct tsdrv_ctx *ctx, u32 tsid, struct phy_sq_free_para *para)
{
    struct devdrv_sq_sub_info *sq_sub = NULL;
    struct tsdrv_ts_resource *ts_res = NULL;
    u32 devid = tsdrv_get_devid_by_ctx(ctx);
    u32 fid = tsdrv_get_fid_by_ctx(ctx);

    TSDRV_PRINT_DEBUG("enter devid = %u, tsid = %u, fid = %u.\n", devid, tsid, fid);

    ts_res = tsdrv_ctx_to_ts_res(ctx, tsid);

#ifdef CFG_FEATURE_SUPPORT_VSQ_MNG
    tsdrv_vsq_mem_destroy(ts_res, para->sq_id);
    tsdrv_clear_vsq_info(ts_res, para->sq_id);
#endif

    sq_sub = tsdrv_phy_sqid_exit(ts_res, ctx, para->sq_id);
    if (sq_sub == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("sqId=%u is not allocated by tgid=%d, devid=%u fid=%u tsid=%u\n", para->sq_id, ctx->tgid,
            devid, fid, tsid);
        return -EINVAL;
#endif
    }
    tsdrv_phy_sq_mem_exit(devid, tsid, sq_sub);
    tsrdv_put_phy_sqid(ts_res, sq_sub);
    TSDRV_PRINT_DEBUG("exit devid = %u, tsid = %u, fid = %u.\n", devid, tsid, fid);
    return 0;
}

struct devdrv_ts_cq_info *tsdrv_get_cq_info(u32 devid, u32 fid, u32 tsid, u32 cq_id)
{
    struct devdrv_ts_cq_info *cq_info = NULL;
    struct tsdrv_ts_resource *ts_res = NULL;
    phys_addr_t phy_cq_base;

    /* info mem is stored in fid0 currently */
    ts_res = tsdrv_get_ts_resoruce(devid, fid, tsid);
    if (!ts_res->mem_info[DEVDRV_INFO_MEM].virt_addr) {
        TSDRV_PRINT_ERR("pm info res is NULL, devid=%u fid=%u tsid=%u\n", devid, fid, tsid);
        return NULL;
    }
    phy_cq_base = ts_res->mem_info[DEVDRV_INFO_MEM].virt_addr;
    cq_info = devdrv_calc_cq_info(phy_cq_base, cq_id);
    return cq_info;
}

/* get phyical cqId info from freelist */
int tsrdv_get_phy_cqid(u32 devid, u32 fid, u32 tsid, struct devdrv_cq_sub_info **cq_sub)
{
    struct tsdrv_ts_resource *ts_res = tsdrv_get_ts_resoruce(devid, fid, tsid);
    struct tsdrv_id_info *cq_id_info = NULL;
    struct devdrv_ts_cq_info *cq_info = NULL;
    phys_addr_t info_mem_addr;

    mutex_lock(&ts_res->id_res[TSDRV_CQ_ID].id_mutex_t);
    spin_lock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
    if (ts_res->id_res[TSDRV_CQ_ID].id_available_num == 0) {
        if (tsdrv_is_in_pm(devid)) {
            spin_unlock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
            if (tsdrv_msg_alloc_sync_cq(devid, tsid) != 0) {
                mutex_unlock(&ts_res->id_res[TSDRV_CQ_ID].id_mutex_t);
#ifndef TSDRV_UT
                *cq_sub = NULL;
#endif
                return -ENOMSG;
            }
            spin_lock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
        }
        if (ts_res->id_res[TSDRV_CQ_ID].id_available_num == 0) {
            spin_unlock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
            mutex_unlock(&ts_res->id_res[TSDRV_CQ_ID].id_mutex_t);
#ifndef TSDRV_UT
            TSDRV_PRINT_WARN("no enough cq, devid=%u fid=%u tsid=%u\n", devid, fid, tsid);
            *cq_sub = NULL;
#endif
            return NO_RES;
        }
    }
    cq_id_info = list_first_entry(&ts_res->id_res[TSDRV_CQ_ID].id_available_list,
        struct tsdrv_id_info, list);
    list_del(&cq_id_info->list);
    ts_res->id_res[TSDRV_CQ_ID].id_available_num--;
    info_mem_addr = ts_res->mem_info[DEVDRV_INFO_MEM].virt_addr;
    cq_info = devdrv_calc_cq_info(info_mem_addr, cq_id_info->phy_id);
    *cq_sub = cq_info->cq_sub;
    spin_unlock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
    mutex_unlock(&ts_res->id_res[TSDRV_CQ_ID].id_mutex_t);
    TSDRV_PRINT_DEBUG("Alloc Cq succeed. (devid=%u; tsid=%u; phy_cqid=%d; cq_index=%u)\n", devid, tsid,
        cq_id_info->phy_id, (*cq_sub)->index);
    return 0;
}

STATIC int tsdrv_phy_cq_device_mem_init(struct tsdrv_cqsq_mem_alloc_free_para *para)
{
    int err;

#ifdef CFG_SOC_PLATFORM_MINIV2
    err = tsdrv_dev_alloc_reserve_sqcqmem(para);
    if (err != 0) {
        TSDRV_PRINT_ERR("tsdrv_dev_alloc_reserve_sqcqmem fail, devid=%u tsid=%u que_size=0x%x\n",
            para->dev_id, para->tsid, para->size);
        return -ENOMEM;
    }
#else
    err = tsdrv_dev_alloc_ts_node_sqcqmem(para);
    if (err != 0) {
        TSDRV_PRINT_ERR("tsdrv_dev_alloc_reserve_sqcqmem fail, devid=%u tsid=%u que_size=0x%x\n",
            para->dev_id, para->tsid, para->size);
        return -ENOMEM;
    }
#endif

    return 0;
}

STATIC void tsrdv_phy_cq_device_mem_exit(struct tsdrv_cqsq_mem_alloc_free_para *para)
{
#ifdef CFG_SOC_PLATFORM_MINIV2
    tsdrv_dev_free_reserve_sqcqmem(para);
#else
    tsdrv_dev_free_ts_node_sqcqmem(para);
#endif
}

STATIC int tsdrv_phy_cq_host_mem_init(struct tsdrv_cqsq_mem_alloc_free_para *para)
{
    int err;

#ifdef CFG_SOC_PLATFORM_MINIV3
    err = tsdrv_host_alloc_bar_sqcqmem(para);
    if (err != 0) {
        TSDRV_PRINT_ERR("[dev_id=%u,tsid=%u,sqid=%u]:tsdrv_host_alloc_bar_sqcqmem fail, no mem for cq\n",
            para->dev_id, para->tsid, para->id);
        return err;
    }
#else
    err = tsdrv_host_alloc_dma_sqcqmem(para);
    if (err != 0) {
        TSDRV_PRINT_ERR("[dev_id=%u,tsid=%u,sqid=%u]:tsdrv_host_alloc_dma_sqcqmem fail, no mem for cq\n",
            para->dev_id, para->tsid, para->id);
        return err;
    }
#endif

    return 0;
}

STATIC void tsrdv_phy_cq_host_mem_exit(struct tsdrv_cqsq_mem_alloc_free_para *para,
    struct devdrv_cq_sub_info *cq_sub)
{
    spin_lock_irq(&cq_sub->spinlock);
#ifdef CFG_SOC_PLATFORM_MINIV3
    tsdrv_host_free_bar_sqcqmem(para);
#else
    tsdrv_host_free_dma_sqcqmem(para);
#endif
    spin_unlock_irq(&cq_sub->spinlock);
}

STATIC int tsrdv_phy_cq_mem_init(u32 devid, u32 tsid, struct devdrv_cq_sub_info *cq_sub,
    struct phy_cq_alloc_para *para)
{
    int err;
    enum tsdrv_env_type env_type = tsdrv_get_env_type();
    struct tsdrv_cqsq_mem_alloc_free_para alloc_para = {0};

    alloc_para.dev_id = devid;
    alloc_para.tsid = tsid;
    alloc_para.id = cq_sub->index;
    alloc_para.id_type = DEVDRV_CQ_MEM;
    alloc_para.size = para->cq_size * para->cq_depth;
    switch (env_type) {
        case TSDRV_ENV_OFFLINE:
            err = tsdrv_phy_cq_device_mem_init(&alloc_para);
            break;
        case TSDRV_ENV_ONLINE:
            err = tsdrv_phy_cq_host_mem_init(&alloc_para);
            break;
        default:
#ifndef TSDRV_UT
            TSDRV_PRINT_ERR("invalid env type=%d, devid=%u tsid=%u\n", (int)env_type, devid, tsid);
#endif
            err = -ENODEV;
            break;
    }

    spin_lock_irq(&cq_sub->spinlock);
    cq_sub->bar_addr = alloc_para.bar_addr;
    cq_sub->virt_addr = alloc_para.virt_addr;
    cq_sub->phy_addr = alloc_para.phy_addr;
    cq_sub->size = alloc_para.size;
    cq_sub->slot_size = para->cq_size;
    cq_sub->depth = para->cq_depth;
    cq_sub->addr_side = alloc_para.addr_side;
    cq_sub->dev = tsdrv_get_dev_p(devid);
    spin_unlock_irq(&cq_sub->spinlock);

    TSDRV_PRINT_DEBUG("alloc cq mem success. (cq_id=%u; bar_addr=%pK; phy_addr=%pK;"
        "vaddr=%pK; addr_side=%u; queue_size=%u; slot_size=%lu; depth=%u)\n",
        cq_sub->index, (void *)(uintptr_t)cq_sub->bar_addr, (void *)(uintptr_t)cq_sub->phy_addr,
        (void *)(uintptr_t)cq_sub->virt_addr, cq_sub->addr_side, cq_sub->size, cq_sub->slot_size,
        cq_sub->depth);

    return err;
}

STATIC void tsrdv_phy_cq_mem_exit(u32 devid, u32 tsid, struct devdrv_cq_sub_info *cq_sub)
{
    enum tsdrv_env_type env_type = tsdrv_get_env_type();
    struct tsdrv_cqsq_mem_alloc_free_para free_para = {0};

    free_para.dev_id = devid;
    free_para.tsid = tsid;
    free_para.id_type = DEVDRV_CQ_MEM;
    spin_lock_irq(&cq_sub->spinlock);
    free_para.id = cq_sub->index;
    free_para.size = cq_sub->size;
    free_para.addr_side = cq_sub->addr_side;
    free_para.virt_addr = cq_sub->virt_addr;
    free_para.phy_addr = cq_sub->phy_addr;
    free_para.bar_addr = cq_sub->bar_addr;
    cq_sub->virt_addr = 0;
    cq_sub->phy_addr = 0;
    cq_sub->size = 0;
    cq_sub->slot_size = 0;
    cq_sub->depth = 0;
    spin_unlock_irq(&cq_sub->spinlock);

    switch (env_type) {
        case TSDRV_ENV_OFFLINE:
            tsrdv_phy_cq_device_mem_exit(&free_para);
            break;
        case TSDRV_ENV_ONLINE:
            tsrdv_phy_cq_host_mem_exit(&free_para, cq_sub);
            break;
        default:
            TSDRV_PRINT_ERR("invalid env type=%d, devid=%u tsid=%u\n", (int)env_type, devid, tsid);
            break;
    }
}

/* put phyical sqId info back to freelist */
STATIC void tsrdv_put_phy_cqid(struct tsdrv_ts_resource *ts_res,
    struct devdrv_cq_sub_info *cq_sub)
{
    struct tsdrv_id_info *cq_id_info = NULL;
    spin_lock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
    cq_id_info = devdrv_find_one_id(&ts_res->id_res[TSDRV_CQ_ID], cq_sub->index);
    if (cq_id_info == NULL) {
        spin_unlock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
        return;
    }

    list_add(&cq_id_info->list, &ts_res->id_res[TSDRV_CQ_ID].id_available_list);
    ts_res->id_res[TSDRV_CQ_ID].id_available_num++;
    spin_unlock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);

    TSDRV_PRINT_DEBUG("tsid=%u cqid=%d cq index=%u\n", ts_res->tsid, cq_id_info->id, cq_sub->index);
}

/* Add physical sqId to current context and init it */
STATIC int tsdrv_phy_cqid_init(struct tsdrv_ts_resource *ts_res, struct tsdrv_ctx *ctx,
    struct devdrv_cq_sub_info *cq_sub, struct phy_cq_alloc_para *para)
{
    struct tsdrv_id_info *cq_id_info = NULL;
    struct devdrv_ts_cq_info *cq_info = NULL;
    u32 devid = tsdrv_get_devid_by_ctx(ctx);
    struct tsdrv_ts_ctx *ts_ctx = NULL;

    ts_ctx = &ctx->ts_ctx[ts_res->tsid];
    /* use pm sq info address */
    cq_info = tsdrv_get_cq_info(devid, TSDRV_PM_FID, ts_res->tsid, cq_sub->index);
    if (cq_info == NULL) {
        return -ENOMEM;
    }

    spin_lock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
    cq_id_info = devdrv_find_one_id(&ts_res->id_res[TSDRV_CQ_ID], (int)cq_info->index);
    if (cq_id_info == NULL) {
        spin_unlock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
        return -ENOMEM;
    }

    list_add(&cq_id_info->list, &ts_ctx->id_ctx[TSDRV_CQ_ID].id_list);

    ts_ctx->id_ctx[TSDRV_CQ_ID].id_num++;
    spin_lock_irq(&cq_sub->spinlock);
    cq_info->alloc_status = SQCQ_ACTIVE;
    cq_info->phase = 1;
    cq_info->type = para->type;
    cq_info->tgid = ctx->tgid;
    cq_info->head = 0;
    cq_info->tail = 0;
    cq_info->depth = para->cq_depth;
    cq_info->slot_size = para->cq_size;
    cq_sub->ctx = ctx;
    cq_sub->complete_handle = NULL;
    spin_unlock_irq(&cq_sub->spinlock);
    spin_unlock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
    TSDRV_PRINT_DEBUG("(type=%d; cqid=%u; cq_info->phase=%u; cqe_size=%u)\n",
        (int)cq_info->type, cq_info->index, cq_info->phase, (u32)cq_info->slot_size);

    return 0;
}

int tsdrv_phy_cq_alloc(struct tsdrv_ctx *ctx, u32 tsid, struct phy_cq_alloc_para *para)
{
    struct tsdrv_ts_resource *ts_res = NULL;
    u32 devid = tsdrv_get_devid_by_ctx(ctx);
    struct devdrv_cq_sub_info *cq_sub = NULL;
    u32 fid = tsdrv_get_fid_by_ctx(ctx);
    struct tsdrv_cq_hwinfo *cq_hwinfo = NULL;
    u16 irq_id;
    int err;

    TSDRV_PRINT_DEBUG("Enter (devid=%u; tsid=%u; fid=%u).\n", devid, tsid, fid);

    ts_res = tsdrv_ctx_to_ts_res(ctx, tsid);
    if (ts_res == NULL) {
        TSDRV_PRINT_ERR("(devid=%u; tsid=%u; fid=%u).\n", devid, tsid, fid);
        return -ENODEV;
    }
    err = tsrdv_get_phy_cqid(devid, fid, tsid, &cq_sub);
    if (err != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("(devid=%u; tsid=%u; fid=%u; err=%d).\n", devid, tsid, fid, err);
#endif
        return err;
    }
    err = tsrdv_phy_cq_mem_init(devid, tsid, cq_sub, para);
    if (err != 0) {
#ifndef TSDRV_UT
        tsrdv_put_phy_cqid(ts_res, cq_sub);
        return err;
#endif
    }
    err = tsdrv_phy_cqid_init(ts_res, ctx, cq_sub, para);
    if (err != 0) {
#ifndef TSDRV_UT
        tsrdv_phy_cq_mem_exit(devid, tsid, cq_sub);
        tsrdv_put_phy_cqid(ts_res, cq_sub);
        return err;
#endif
    }
    cq_hwinfo = tsdrv_get_cq_hwinfo(devid, tsid);
    devdrv_calc_cq_irq_id(cq_sub->index, cq_hwinfo->cq_irq_num, DEVDRV_MAX_CQ_NUM, &irq_id);
    para->cq_irq = cq_hwinfo->cq_irq[irq_id];
    para->cq_id = cq_sub->index;
    para->cq_paddr = cq_sub->phy_addr;
    para->cq_vaddr = (void *)(uintptr_t)cq_sub->virt_addr;

    TSDRV_PRINT_DEBUG("exit devid = %u, tsid = %u, fid = %u, cq_id = %u, cq_irq=%u.\n",
        devid, tsid, fid, para->cq_id, (u32)para->cq_irq);

    return 0;
}

/* Remove physical sqId from current context and return sq_sub */
struct devdrv_cq_sub_info *tsdrv_phy_cqid_exit(struct tsdrv_ts_resource *ts_res,
    struct tsdrv_ctx *ctx, u32 cqid)
{
    struct tsdrv_id_info *cq_id_info = NULL;
    struct devdrv_ts_cq_info *cq_info = NULL;
    struct devdrv_cq_sub_info *cq_sub = NULL;
    u32 devid = tsdrv_get_devid_by_ctx(ctx);
    struct tsdrv_ts_ctx *ts_ctx = NULL;

    ts_ctx = &ctx->ts_ctx[ts_res->tsid];
    cq_info = tsdrv_get_cq_info(devid, TSDRV_PM_FID, ts_res->tsid, cqid);
    if ((cq_info == NULL) || (cq_info->cq_sub == NULL)) {
        TSDRV_PRINT_ERR("get pm sq info base fail, devid=%u tsid=%u\n", devid, ts_res->tsid);
        return NULL;
    }

    cq_sub = cq_info->cq_sub;
    spin_lock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
    spin_lock_irq(&cq_sub->spinlock);
    cq_sub->ctx = NULL;
    cq_info->type = NORMAL_SQCQ_TYPE; // default type
    cq_info->vfid = 0;
    cq_info->tgid = 0;
    cq_info->tail = 0;
    cq_info->head = 0;
    cq_info->alloc_status = SQCQ_INACTIVE;
    cq_info->count_report = 0;
    cq_info->depth = DEVDRV_MAX_CQ_DEPTH;
    cq_info->receive_count = 0;
    cq_info->release_head = 0;
    cq_info->phase = DEVDRV_PHASE_STATE_0;
    cq_info->slot_size = DEVDRV_CQ_SLOT_SIZE;
    cq_info->uio_fd = DEVDRV_INVALID_FD_OR_NUM;
    spin_unlock_irq(&cq_sub->spinlock);

    cq_id_info = devdrv_find_one_id(&ts_res->id_res[TSDRV_CQ_ID], cqid);
    if (cq_id_info == NULL) {
        spin_unlock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
        return NULL;
    }

    list_del(&cq_id_info->list);
    ts_ctx->id_ctx[TSDRV_CQ_ID].id_num--;
    spin_unlock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
    return cq_sub;
}

int tsdrv_phy_cq_free(struct tsdrv_ctx *ctx, u32 tsid, struct phy_cq_free_para *para)
{
    struct devdrv_cq_sub_info *cq_sub = NULL;
    struct tsdrv_ts_resource *ts_res = NULL;
    u32 devid = tsdrv_get_devid_by_ctx(ctx);
    u32 fid = tsdrv_get_fid_by_ctx(ctx);

    TSDRV_PRINT_DEBUG("enter devid = %u, tsid = %u, fid = %u.\n", devid, tsid, fid);
    ts_res = tsdrv_ctx_to_ts_res(ctx, tsid);
    cq_sub = tsdrv_phy_cqid_exit(ts_res, ctx, para->cq_id);
    if (cq_sub == NULL) {
        TSDRV_PRINT_ERR("cqId=%u is not allocated by tgid=%d, devid=%u fid=%u tsid=%u\n", para->cq_id, ctx->tgid,
            devid, fid, tsid);
        return -EINVAL;
    }
    tsrdv_phy_cq_mem_exit(devid, tsid, cq_sub);
    tsrdv_put_phy_cqid(ts_res, cq_sub);

    TSDRV_PRINT_DEBUG("exit devid = %u, tsid = %u, fid = %u.\n", devid, tsid, fid);
    return 0;
}

int tsdrv_kernel_alloc_sq(u32 devid, u32 vfid, u32 tsid, u32 sqe_size, u32 depth, enum phy_sqcq_type type)
{
    struct devdrv_sq_sub_info *sq_sub = NULL;
    struct devdrv_ts_sq_info *sq_info = NULL;
    struct page *page = NULL;
    int ret;

    ret = tsrdv_get_phy_sqid(devid, vfid, tsid, &sq_sub);
    if (ret != 0) {
        TSDRV_PRINT_ERR("devid %u vfid %u tsid %u alloc sq failed\n", devid, vfid, tsid);
        return ret;
    }
    sq_info = tsdrv_get_sq_info(devid, vfid, tsid, sq_sub->index);
    if (sq_info == NULL) {
        TSDRV_PRINT_ERR("devid %u vfid %u tsid %u sq %u get sq_info failed\n", devid, vfid, tsid, sq_sub->index);
        return -1;
    }

    sq_info->type = type;
    sq_info->head = 0;
    sq_info->tail = 0;
    sq_info->alloc_status = SQCQ_ACTIVE;
    sq_info->depth = depth;
    if (type == TS_SQCQ_TYPE) {
        sq_sub->vaddr = 0;
        return sq_sub->index;
    }

    sq_sub->size = sqe_size;
    sq_sub->depth = depth;

    if (tsdrv_get_env_type() == TSDRV_ENV_ONLINE) {
#ifdef CFG_FEATURE_SQ_HOST_MEM
        sq_sub->vaddr = (uintptr_t)dma_zalloc_coherent(tsdrv_get_dev(devid)->dev, sq_sub->size * sq_sub->depth,
            (dma_addr_t *)&sq_sub->phy_addr, GFP_KERNEL);
        if (sq_sub->vaddr == 0) {
            goto err_alloc;
        }
        sq_sub->bar_addr = sq_sub->phy_addr;
#else
        struct tsdrv_ts_resource *ts_res = tsdrv_get_ts_resoruce(devid, TSDRV_PM_FID, tsid);
        sq_sub->phy_addr = ts_res->mem_info[DEVDRV_SQ_MEM].phy_addr + sq_sub->index * sq_sub->size * sq_sub->depth;
        sq_sub->bar_addr = ts_res->mem_info[DEVDRV_SQ_MEM].bar_addr + sq_sub->index * sq_sub->size * sq_sub->depth;
        sq_sub->vaddr = ts_res->mem_info[DEVDRV_SQ_MEM].virt_addr + sq_sub->index * sq_sub->size * sq_sub->depth;
#endif
    } else {
        page = ka_alloc_pages_node(devdrv_manager_devid_to_nid(devid, (u32)DEVDRV_TS_NODE_DDR_MEM),
            __GFP_ZERO | __GFP_THISNODE | GFP_HIGHUSER_MOVABLE, (u32)get_order(sq_sub->size * sq_sub->depth));
        if (page == NULL) {
            goto err_alloc;
        }
        sq_sub->vaddr = (uintptr_t)page_to_virt(page);
        sq_sub->phy_addr = (phys_addr_t)virt_to_phys((void *)(uintptr_t)sq_sub->vaddr);
    }

    return sq_sub->index;
err_alloc:
    TSDRV_PRINT_ERR("devid %u vfid %u tsid %u sq %u alloc sq mem failed\n", devid, vfid, tsid, sq_sub->index);
    tsrdv_put_phy_sqid(tsdrv_get_ts_resoruce(devid, vfid, tsid), sq_sub);
    return -1;
}

void tsdrv_kernel_free_sq(u32 devid, u32 vfid, u32 tsid, u32 sqid)
{
    struct devdrv_ts_sq_info *sq_info = NULL;
    struct devdrv_sq_sub_info *sq_sub = NULL;

    sq_info = tsdrv_get_sq_info(devid, vfid, tsid, sqid);
    if ((sq_info == NULL) || (sq_info->sq_sub == NULL)) {
        return;
    }

    sq_sub = sq_info->sq_sub;
    sq_sub->phy_addr = 0;

    if (tsdrv_get_env_type() == TSDRV_ENV_ONLINE) {
#ifdef CFG_FEATURE_SQ_HOST_MEM
        struct tsdrv_device *tsdrv_dev = tsdrv_get_dev(devid);
        if (sq_sub->vaddr != 0) {
            dma_free_coherent(tsdrv_dev->dev, sq_sub->size * sq_sub->depth,
                (void *)(uintptr_t)sq_sub->vaddr, (dma_addr_t)sq_sub->phy_addr);
            sq_sub->vaddr = 0;
        }
#endif
    } else {
        if (sq_sub->vaddr != 0) {
            free_pages((unsigned long)sq_sub->vaddr, (u32)get_order(sq_sub->size * sq_sub->depth));
            sq_sub->vaddr = 0;
        }
    }

    tsrdv_put_phy_sqid(tsdrv_get_ts_resoruce(devid, vfid, tsid), sq_sub);
}

int tsdrv_kernel_alloc_cq(u32 devid, u32 vfid, u32 tsid, u32 cqe_size, u32 depth, enum phy_sqcq_type type)
{
    struct devdrv_cq_sub_info *cq_sub = NULL;
    struct devdrv_ts_cq_info *cq_info = NULL;
    phys_addr_t vaddr;
    phys_addr_t paddr;
    int ret;
#ifdef CFG_FEATURE_CQ_HOST_MEM
    struct tsdrv_device *tsdrv_dev = NULL;
#else
    struct tsdrv_ts_resource *ts_res = NULL;
#endif

    ret = tsrdv_get_phy_cqid(devid, vfid, tsid, &cq_sub);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Alloc cq failed. (devid=%u; vfid=%u; tsid=%u; ret=%d).\n", devid, vfid, tsid, ret);
#endif
        return ret;
    }

    cq_info = tsdrv_get_cq_info(devid, vfid, tsid, cq_sub->index);
    if (cq_info == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("devid %u vfid %u tsid %u cq %u get cq_info failed\n",
            devid, vfid, tsid, cq_sub->index);
#endif
        return -1;
    }

    cq_info->type = type;
    cq_info->phase = 1;
    cq_info->head = 0;
    cq_info->tail = 0;
    cq_info->slot_size = cqe_size;
    cq_info->alloc_status = SQCQ_ACTIVE;
    if (type == TS_SQCQ_TYPE) {
        cq_sub->virt_addr = 0;
        return cq_sub->index;
    }

    cq_sub->size = depth * cqe_size;
    cq_sub->size = PAGE_ALIGN(cq_sub->size);
    cq_sub->depth = depth;
    cq_sub->slot_size = cqe_size;

    if (tsdrv_get_env_type() == TSDRV_ENV_ONLINE) {
#ifdef CFG_FEATURE_CQ_HOST_MEM
        tsdrv_dev = tsdrv_get_dev(devid);
        vaddr = (uintptr_t)dma_zalloc_coherent(tsdrv_dev->dev, cq_sub->size,
            (dma_addr_t *)&paddr, GFP_KERNEL);
#else
        ts_res = tsdrv_get_ts_resoruce(devid, TSDRV_PM_FID, tsid);
        paddr = DEVDRV_RESERVE_MEM_BASE + DEVDRV_RESERVE_CQ_MEM_OFFSET +
            (long)(unsigned)devid * CHIP_BASEADDR_PA_OFFSET +
            (unsigned long)cq_sub->index *
            (unsigned long)cqe_size *
            (unsigned long)depth;
        vaddr = ts_res->mem_info[DEVDRV_SQ_MEM].virt_addr + DEVDRV_RESERVE_CQ_MEM_OFFSET +
            (unsigned long)cq_sub->index *
            (unsigned long)cqe_size *
            (unsigned long)depth;
        memset_io((void *)(uintptr_t)(vaddr), 0, cqe_size * depth);
#endif
    } else {
        vaddr = (uintptr_t)tsdrv_alloc_pages_exact_nid(devdrv_manager_devid_to_nid(devid, (u32)DEVDRV_TS_NODE_DDR_MEM),
            cq_sub->size, __GFP_THISNODE | GFP_HIGHUSER_MOVABLE);
        paddr = (phys_addr_t)virt_to_phys((void *)(uintptr_t)vaddr);
    }

    if (vaddr == 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("devid %u vfid %u tsid %u sq %u alloc sq mem failed\n", devid, vfid, tsid,
            cq_sub->index);
        tsrdv_put_phy_cqid(tsdrv_get_ts_resoruce(devid, vfid, tsid), cq_sub);
        return -1;
#endif
    }

    spin_lock_irq(&cq_sub->spinlock);
    cq_sub->virt_addr = vaddr;
    cq_sub->phy_addr = paddr;
    cq_sub->ctx = NULL;
    spin_unlock_irq(&cq_sub->spinlock);

    return cq_sub->index;
}

void tsdrv_kernel_free_cq(u32 devid, u32 vfid, u32 tsid, u32 cqid)
{
#ifdef CFG_FEATURE_CQ_HOST_MEM
    struct tsdrv_device *tsdrv_dev = NULL;
#endif
    struct devdrv_ts_cq_info *cq_info = NULL;
    struct devdrv_cq_sub_info *cq_sub = NULL;
    phys_addr_t vaddr;
    phys_addr_t paddr;

    cq_info = tsdrv_get_cq_info(devid, vfid, tsid, cqid);
    if ((cq_info == NULL) || (cq_info->cq_sub == NULL)) {
        return;
    }

    cq_info->vfid = TSDRV_PM_FID;
    cq_sub = cq_info->cq_sub;
    vaddr = cq_sub->virt_addr;
    paddr = cq_sub->phy_addr;
    spin_lock_irq(&cq_sub->spinlock);
    cq_sub->virt_addr = 0;
    cq_sub->phy_addr = 0;
    cq_sub->ctx = NULL;
    cq_sub->complete_handle = NULL;
    cq_sub->chan = NULL;
    spin_unlock_irq(&cq_sub->spinlock);

    if (tsdrv_get_env_type() == TSDRV_ENV_ONLINE) {
#ifdef CFG_FEATURE_CQ_HOST_MEM
        tsdrv_dev = tsdrv_get_dev(devid);
        if (vaddr != 0) {
            dma_free_coherent(tsdrv_dev->dev, cq_sub->size, (void *)(uintptr_t)vaddr, (dma_addr_t)paddr);
        }
#endif
    } else {
        if (vaddr != 0) {
            tsdrv_free_pages_exact((void *)(uintptr_t)vaddr, cq_sub->size);
        }
    }

    tsrdv_put_phy_cqid(tsdrv_get_ts_resoruce(devid, vfid, tsid), cq_sub);
}

STATIC int tsdrv_submit_task_chan_check(struct tsdrv_task_submit_chan *chan)
{
    enum tsdrv_dev_status status;

    if (chan == NULL) {
        TSDRV_PRINT_ERR("null ptr\n");
        return -EINVAL;
    }

    if ((chan->devid >= TSDRV_MAX_DAVINCI_NUM) || (chan->vfid >= TSDRV_MAX_FID_NUM) ||
        (chan->tsid >= DEVDRV_MAX_TS_NUM) || (chan->sqid >= DEVDRV_MAX_SQ_NUM) ||
        (chan->cqid >= DEVDRV_MAX_CQ_NUM)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("devid %u vfid %u tsid %u sq %u cq %u invalid\n",
            chan->devid, chan->vfid, chan->tsid, chan->sqid, chan->cqid);
#endif
        return -EINVAL;
    }

    status = tsdrv_get_dev_status(chan->devid, chan->vfid);
    if (status != TSDRV_DEV_ACTIVE) {
#ifndef TSDRV_UT
        TSDRV_PRINT_WARN("tsdrv dev(%u) fid(%u), invalid status(%d).\n", chan->devid, chan->vfid, (int)status);
#endif
        return -EINVAL;
    }

    return 0;
}

STATIC void tsdrv_set_sq_doorbell(u32 devid, u32 tsid, u32 sqid, u32 sq_tail)
{
    struct tsdrv_db_hwinfo_t *db_hwinfo = tsdrv_get_db_hwinfo_t(devid, tsid, TSDRV_SQ_DB);
    void __iomem *doorbell = db_hwinfo->dbs + ((long)(unsigned)sqid * db_hwinfo->db_stride) + DEVDRV_SQ_TAIL_OFFSET;
    writel(sq_tail, doorbell);
}

void tsdrv_set_cq_doorbell(u32 devid, u32 tsid, u32 cqid, u32 cq_head)
{
    struct tsdrv_db_hwinfo_t *db_hwinfo = tsdrv_get_db_hwinfo_t(devid, tsid, TSDRV_CQ_DB);
    void __iomem *doorbell = db_hwinfo->dbs + ((long)(unsigned)cqid * db_hwinfo->db_stride) + DEVDRV_CQ_HEAD_OFFSET;
    writel(cq_head, doorbell);
}

int tsdrv_submit_task(void *handle, const void *sqe, u32 timeout)
{
    struct tsdrv_task_submit_chan *chan = (struct tsdrv_task_submit_chan *)handle;
    struct devdrv_ts_sq_info *sq_info = NULL;
    struct devdrv_sq_sub_info *sq_sub = NULL;
    int ret;

    (void)timeout;  /* surport timeout latter */

    ret = tsdrv_submit_task_chan_check(chan);
    if (ret != 0) {
        TSDRV_PRINT_ERR("chan invalid\n");
        return ret;
    }

    if (sqe == NULL) {
        TSDRV_PRINT_ERR("Sqe is null. (devid=%u; vfid=%u; tsid=%u; sqid=%u)\n",
            chan->devid, chan->vfid, chan->tsid, chan->sqid);
        return -EINVAL;
    }

    sq_info = tsdrv_get_sq_info(chan->devid, chan->vfid, chan->tsid, chan->sqid);
    if ((sq_info == NULL) || (sq_info->sq_sub == NULL)) {
        TSDRV_PRINT_ERR("Sq_info is null. (devid=%u; vfid=%u; tsid=%u; sq=%u)\n",
            chan->devid, chan->vfid, chan->tsid, chan->sqid);
        return -EINVAL;
    }

    sq_sub = sq_info->sq_sub;

    spin_lock_bh(&chan->lock);

    /* sq is full */
    if (((sq_info->tail + 1) % sq_sub->depth) == sq_info->head) {
        spin_unlock_bh(&chan->lock);
        TSDRV_PRINT_ERR("Sq is full. (devid=%u; vfid=%u; tsid=%u; sqid=%u)\n",
            chan->devid, chan->vfid, chan->tsid, chan->sqid);
        return -ENOSPC;
    }

    memcpy_toio((void *)(uintptr_t)(sq_sub->vaddr + (sq_info->tail * DEVDRV_SQ_SLOT_SIZE)), sqe, DEVDRV_SQ_SLOT_SIZE);

    tsdrv_get_drv_ops()->flush_cache((sq_sub->vaddr + (sq_info->tail * DEVDRV_SQ_SLOT_SIZE)), DEVDRV_SQ_SLOT_SIZE);
    sq_info->tail = (sq_info->tail + 1) % sq_sub->depth;

    wmb();

    tsdrv_set_sq_doorbell(chan->devid, chan->tsid, chan->sqid, sq_info->tail);

    spin_unlock_bh(&chan->lock);

    TSDRV_PRINT_INFO("Esched sbumit task. (devid=%u, tsid=%u, sqid=%u, cqid=%u)\n",
        chan->devid, chan->tsid, chan->sqid, chan->cqid);

    return 0;
}
TSDRV_EXPORT_SYMBOL(tsdrv_submit_task);

void tsdrv_set_chan_complete_handle(void *handle,
    void(*report_handle)(void *report, u32 report_count))
{
    struct tsdrv_task_submit_chan *chan = (struct tsdrv_task_submit_chan *)handle;
    int ret;

    ret = tsdrv_submit_task_chan_check(chan);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Chan invalid.\n");
        return;
    }

    chan->report_handle = report_handle;
}
TSDRV_EXPORT_SYMBOL(tsdrv_set_chan_complete_handle);

STATIC void tsdrv_task_complete(struct devdrv_ts_cq_info *cq_info)
{
#ifndef TSDRV_UT
    struct devdrv_cq_sub_info *cq_sub = cq_info->cq_sub;
    struct tsdrv_task_submit_chan *chan = (struct tsdrv_task_submit_chan *)cq_sub->chan;
    struct devdrv_ts_sq_info *sq_info = tsdrv_get_sq_info(chan->devid, chan->vfid, chan->tsid, chan->sqid);
    struct devdrv_report *report = NULL;
    struct devdrv_report tmp_report;
    int ret;

    if (sq_info == NULL) {
        TSDRV_PRINT_ERR("Sq_info is null. (devid=%u; vfid=%u; tsid=%u; sqid=%u)\n", chan->devid, chan->vfid,
            chan->tsid, chan->sqid);
        return;
    }
    do {
        report = (struct devdrv_report *)((uintptr_t)((unsigned long)cq_sub->virt_addr +
            ((unsigned long)cq_info->slot_size * cq_info->head)));

        if (devdrv_report_get_phase(report) != cq_info->phase) {
            break;
        }

        if (tsdrv_report_get_sq_id(report) != chan->sqid) {
            TSDRV_PRINT_ERR("devid %u vfid %u tsid %u sq %u cq %u complete sq %u cq %d not match\n",
                chan->devid, chan->vfid, chan->tsid, chan->sqid, chan->cqid,
                cq_info->index, tsdrv_report_get_sq_id(report));
            continue;
        }

        TSDRV_PRINT_DEBUG("cq_index %u sq index %u\n", chan->cqid, tsdrv_report_get_sq_id(report));
        TSDRV_PRINT_FPGA("cq_index %u sq index %u\n", chan->cqid, tsdrv_report_get_sq_id(report));

        sq_info->head = tsdrv_report_get_sq_head(report);

        wmb();
        cq_info->head = (cq_info->head + 1) % cq_sub->depth;
        if (chan->report_handle != NULL) {
            TSDRV_PRINT_DEBUG("report handle. (devid=%u; vfid=%u; cq_index=%u)\n",
                chan->devid, chan->vfid, chan->cqid);
            ret = memcpy_s(&tmp_report, sizeof(struct devdrv_report), report, sizeof(struct devdrv_report));
            if (ret != 0) {
                TSDRV_PRINT_ERR("memcpy failed. (devid=%u; vfid=%u; cq_index=%u)\n",
                    chan->devid, chan->vfid, chan->cqid);
            } else {
                chan->report_handle(&tmp_report, 1);
            }
        }
        tsdrv_set_cq_doorbell(chan->devid, chan->tsid, chan->cqid, cq_info->head);

        if (cq_info->head == 0) {
            cq_info->phase = (cq_info->phase == 0) ? 1 : 0;
        }
    } while (1);
#endif
}

STATIC int tsdrv_sqcq_inform_ts(u32 devid, u32 vfid, u32 tsid, u32 sqid, u32 cqid, u32 pool_id, u16 cmd_type, u32 pri)
{
#define CUR_SQCQ_MAILBOX_MSG_ZERO 0
#define CUR_SQCQ_MAILBOX_MSG_ONE  1
#define CUR_SQCQ_MAILBOX_MSG_TWO  2
#define CUR_SQCQ_MAILBOX_MSG_FOUR 4

    struct devdrv_normal_cqsq_mailbox msg;
    struct devdrv_ts_sq_info *sq_info = NULL;
    struct devdrv_sq_sub_info *sq_sub = NULL;
    struct devdrv_ts_cq_info *cq_info = NULL;
    struct devdrv_cq_sub_info *cq_sub = NULL;
    struct tsdrv_ts_resource *ts_res = NULL;
    struct tsdrv_cq_hwinfo *cq_hwinfo = NULL;
    int ret, result, ssid = 0;
    u32 phy_devid, fid;
    u16 irq_id;
    pid_t vnr = 0;

    if (cmd_type == TSDRV_MBOX_CREATE_KERNEL_SQCQ) {
#ifndef CFG_MANAGER_HOST_ENV
#ifndef AOS_LLVM_BUILD
        vnr = task_tgid_vnr(current);
        ssid = svm_get_pasid(vnr, devid);
#else
        vnr = current->tgid;
        ssid = AOS_SVAGetCurrentPasid();
#endif
        if (unlikely(ssid < 0)) {
            TSDRV_PRINT_ERR("Get ssid failed. (devid=%u; pid=%d; ssid=%d)\n", devid, vnr, ssid);
            return TSDRV_INVALID_SSID;
        }
#endif
    }

    sq_info = tsdrv_get_sq_info(devid, vfid, tsid, sqid);
    cq_info = tsdrv_get_cq_info(devid, vfid, tsid, cqid);
    if ((sq_info == NULL) || (cq_info == NULL) || (sq_info->sq_sub == NULL) ||
        (cq_info->cq_sub == NULL)) {
        TSDRV_PRINT_ERR("Get sq cq info failed. (devid=%u; vfid=%u; tsid=%u)\n", devid, vfid, tsid);
        return -ENOMEM;
    }
    sq_sub = sq_info->sq_sub;
    cq_sub = cq_info->cq_sub;

    msg.sq_index = (u16)sqid;
    msg.sq_addr = (u64)sq_sub->phy_addr;
    msg.cq0_index = (u16)cqid;
    msg.cq0_addr = (u64)cq_sub->phy_addr;

    msg.sqesize = DEVDRV_SQ_SLOT_SIZE;
    msg.cqesize = DEVDRV_CQ_SLOT_SIZE;
    msg.sqdepth = (u16)sq_sub->depth;
    msg.cqdepth = (u16)cq_sub->depth;

    msg.valid = DEVDRV_MAILBOX_MESSAGE_VALID;
    msg.cmd_type = cmd_type;
    msg.result = 0;

    if (tsdrv_get_env_type() == TSDRV_ENV_ONLINE) {
#if (defined(CFG_FEATURE_SQ_HOST_MEM) && defined(CFG_FEATURE_CQ_HOST_MEM))
        msg.sq_cq_side = (0x1 << 0) | (0x1 << 1);
#else
        msg.sq_cq_side = 0;
#endif
    } else {
        msg.sq_cq_side = 0;
    }
    cq_hwinfo = tsdrv_get_cq_hwinfo(devid, tsid);
    devdrv_calc_cq_irq_id(cqid, cq_hwinfo->cq_irq_num, DEVDRV_MAX_CQ_NUM, &irq_id);
    msg.cq_irq = cq_hwinfo->cq_irq[irq_id];
    TSDRV_PRINT_FPGA("sqid(%u), cqid(%u), addr side(%u), cq_irq(%u)\n", sqid, cqid, msg.sq_cq_side, msg.cq_irq);

    if (tsdrv_get_pfvf_type_by_devid(devid) == DEVDRV_SRIOV_TYPE_PF) {
        phy_devid = devid;
        fid = vfid;
    } else {
#ifndef AOS_LLVM_BUILD
        ret = devdrv_get_pfvf_id_by_devid(devid, &phy_devid, &fid);
        if (ret != 0) {
            TSDRV_PRINT_ERR("Failed to get physic devid and fid. (devid=%u)\n", devid);
            return ret;
        }
        fid = fid - 1;
#endif
    }
#if (defined(CFG_SOC_PLATFORM_MINI) && (!defined(CFG_SOC_PLATFORM_MINIV2)) && (!defined(CFG_SOC_PLATFORM_MINIV3)))
    msg.app_flag = (u8)tsdrv_get_env_type();
#else
    msg.app_type = (u8)tsdrv_get_env_type();
    msg.fid = fid;
#endif

    msg.pid = vnr;
    msg.ssid = ssid;
    msg.info[CUR_SQCQ_MAILBOX_MSG_ZERO] = 0xffff;   /* streamid */
    msg.info[CUR_SQCQ_MAILBOX_MSG_ONE] = pri;       /* rtsq pri */
    msg.info[CUR_SQCQ_MAILBOX_MSG_TWO] = 0;         /* ssid */
                                                    /* tid */
    msg.info[CUR_SQCQ_MAILBOX_MSG_FOUR] = pool_id;  /* poolid */

    ts_res = tsdrv_get_ts_resoruce(phy_devid, vfid, tsid);

    ret = devdrv_mailbox_kernel_sync_no_feedback(&ts_res->mailbox, (u8 *)&msg, sizeof(msg), &result);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Inform ts failed. (devid=%u; vfid=%u; tsid=%u)\n", phy_devid, fid, tsid);
    }

    TSDRV_PRINT_INFO("Kernel sqcq mbox info. (devid=%u; vfid=%u; phy_devid=%u; fid=%u; tsid=%u; sqid=%u; cqid=%u;"
        "pid=%d; ssid=%d; pool_id=%u; mb_cmd_type=%d)\n",
        devid, vfid, phy_devid, fid, tsid, sqid, cqid, vnr, ssid, pool_id, msg.cmd_type);

    return ret;
}

static void tsdrv_reserved_sqcq_store(u32 devid, u32 vfid, u32 tsid, struct tsdrv_reserved_sqcq_info *sqcq_info)
{
    struct devdrv_ts_sq_info *sq_info = tsdrv_get_sq_info(devid, vfid, tsid, sqcq_info->sqid);
    struct devdrv_ts_cq_info *cq_info = tsdrv_get_cq_info(devid, vfid, tsid, sqcq_info->cqid);
    struct tsdrv_ts_resource *ts_res = tsdrv_get_ts_resoruce(devid, vfid, tsid);
    int ret;

    if (ts_res->sqcq_reserved_num < TSDRV_SQCQ_RESEARVED_MAX_NUM) {
        ret = memcpy_s(&ts_res->reserved_sqcq[ts_res->sqcq_reserved_num].sq_info,
            sizeof(struct devdrv_ts_sq_info), sq_info, sizeof(struct devdrv_ts_sq_info));
        ret |= memcpy_s(&ts_res->reserved_sqcq[ts_res->sqcq_reserved_num].sq_sub,
            sizeof(struct devdrv_sq_sub_info), sq_info->sq_sub, sizeof(struct devdrv_sq_sub_info));
        ret |= memcpy_s(&ts_res->reserved_sqcq[ts_res->sqcq_reserved_num].cq_info,
            sizeof(struct devdrv_ts_cq_info), cq_info, sizeof(struct devdrv_ts_cq_info));
        ret |= memcpy_s(&ts_res->reserved_sqcq[ts_res->sqcq_reserved_num].cq_sub,
            sizeof(struct devdrv_cq_sub_info), cq_info->cq_sub, sizeof(struct devdrv_cq_sub_info));
        if (ret != 0) {
            TSDRV_PRINT_ERR("Memcpy failed. (ret=%d)\n", ret);
            return;
        }
        ts_res->reserved_sqcq[ts_res->sqcq_reserved_num].pool_id = sqcq_info->pool_id;
        ts_res->reserved_sqcq[ts_res->sqcq_reserved_num].pri = sqcq_info->pri;
        ts_res->sqcq_reserved_num++;
        TSDRV_PRINT_DEBUG("(devid=%u; vfid=%u; tsid=%u; reserved_num=%u; sqid=%u; cqid=%u)\n", devid, vfid, tsid,
            ts_res->sqcq_reserved_num, sqcq_info->sqid, sqcq_info->cqid);
    }
}

void tsdrv_reserved_sqcq_restore(u32 devid, u32 vfid, u32 tsid)
{
    struct tsdrv_ts_resource *ts_res = tsdrv_get_ts_resoruce(devid, TSDRV_PM_FID, tsid);
    struct devdrv_ts_sq_info *sq_info = NULL;
    struct devdrv_ts_cq_info *cq_info = NULL;
    u16 cmd_type;
    int ret;
    u32 id;

    for (id = 0; id < ts_res->sqcq_reserved_num; id++) {
        sq_info = tsdrv_get_sq_info(devid, TSDRV_PM_FID, tsid, ts_res->reserved_sqcq[id].sq_info.index);
        cq_info = tsdrv_get_cq_info(devid, TSDRV_PM_FID, tsid, ts_res->reserved_sqcq[id].cq_info.index);
        ret = memcpy_s(sq_info, sizeof(struct devdrv_ts_sq_info),
            &ts_res->reserved_sqcq[id].sq_info, sizeof(struct devdrv_ts_sq_info));
        ret |= memcpy_s(sq_info->sq_sub, sizeof(struct devdrv_sq_sub_info),
            &ts_res->reserved_sqcq[id].sq_sub, sizeof(struct devdrv_sq_sub_info));
        ret |= memcpy_s(cq_info, sizeof(struct devdrv_ts_cq_info),
            &ts_res->reserved_sqcq[id].cq_info, sizeof(struct devdrv_ts_cq_info));
        ret |= memcpy_s(cq_info->cq_sub, sizeof(struct devdrv_cq_sub_info),
            &ts_res->reserved_sqcq[id].cq_sub, sizeof(struct devdrv_cq_sub_info));
        if (ret != 0) {
            TSDRV_PRINT_ERR("Memcpy failed. (ret=%d)\n", ret);
            return;
        }
        if (sq_info->type == TS_SQCQ_TYPE) {
            struct devdrv_notice_ts_sqcq_mailbox notice_msg;
            int result = 0;
            notice_msg.head.valid = DEVDRV_MAILBOX_MESSAGE_VALID;
            notice_msg.head.cmd_type = TSDRV_MBOX_NOTICE_TS_SQCQ_CREATE;
            notice_msg.head.result = 0;

            notice_msg.sq_id = sq_info->index;
            notice_msg.cq_id = cq_info->index;
            notice_msg.vfid = TSDRV_PM_FID;

            ret = devdrv_mailbox_kernel_sync_no_feedback(&ts_res->mailbox, (u8 *)&notice_msg,
                sizeof(notice_msg), &result);
            TSDRV_PRINT_DEBUG("(phy_devid=%u; fid=%u; sq_id=%u; cq_id=%u; ret=%d; result=%d)\n",
                devid, vfid, sq_info->index, cq_info->index, ret, result);
        }
        if (sq_info->type == KERNEL_SQCQ_TYPE) {
            cmd_type = TSDRV_MBOX_NOTICE_KERNER_SQCQ;
            ret = tsdrv_sqcq_inform_ts(devid, TSDRV_PM_FID, tsid, sq_info->index, cq_info->index,
                ts_res->reserved_sqcq[id].pool_id, cmd_type, ts_res->reserved_sqcq[id].pri);
        }

        TSDRV_PRINT_DEBUG("Restore researve sqcq. (devid=%u; sqid=%u; cqid=%u)\n", devid, sq_info->index,
            cq_info->index);
    }
}

static int tsdrv_task_submit_chan_para_check(u32 devid, u32 vfid, u32 tsid, u32 sq_depth, u32 cq_depth)
{
    if ((devid >= TSDRV_MAX_DAVINCI_NUM) || (vfid >= TSDRV_MAX_FID_NUM) ||
        (tsid >= DEVDRV_MAX_TS_NUM) || (sq_depth >= TSDRV_MAX_SQ_DEPTH) ||
        (cq_depth >= TSDRV_MAX_CQ_DEPTH)) {
        return -EINVAL;
    }

    return 0;
}
struct tsdrv_id_info *tsdrv_kernel_alloc_stream(u32 devid, u32 vfid, u32 tsid)
{
    struct tsdrv_ts_resource *ts_resource = tsdrv_get_ts_resoruce(devid, vfid, tsid);
    struct tsdrv_id_info *stream_info = NULL;
    int ret, no_stream;

    ret =  devdrv_alloc_stream_check(ts_resource, &no_stream);
    if (ret != 0) {
        TSDRV_PRINT_ERR("alloc stream check failed, ret(%d).\n", ret);
        return NULL;
    }

    spin_lock(&ts_resource->id_res[TSDRV_STREAM_ID].spinlock);
    stream_info = devdrv_get_one_stream_id(ts_resource);
    if (stream_info == NULL) {
        spin_unlock(&ts_resource->id_res[TSDRV_STREAM_ID].spinlock);
        TSDRV_PRINT_ERR("alloc stream id failed. (devid=%u; vfid=%u; tsid=%u)\n",
            devid, vfid, tsid);
        return NULL;
    }
    spin_unlock(&ts_resource->id_res[TSDRV_STREAM_ID].spinlock);
    return stream_info;
}

void tsdrv_kernel_free_stream(u32 devid, u32 vfid, u32 tsid, u32 stream_id)
{
    struct tsdrv_ts_resource *ts_resource = NULL;
    struct tsdrv_id_info *stream_info = NULL;

    ts_resource = tsdrv_get_ts_resoruce(devid, vfid, tsid);
    spin_lock(&ts_resource->id_res[TSDRV_STREAM_ID].spinlock);
    stream_info = (struct tsdrv_id_info *)(ts_resource->id_res[TSDRV_STREAM_ID].id_addr +
        ((long)sizeof(struct tsdrv_id_info) * stream_id));
    list_add(&stream_info->list, &ts_resource->id_res[TSDRV_STREAM_ID].id_available_list);
    ts_resource->id_res[TSDRV_STREAM_ID].id_available_num++;
    stream_info->ctx = NULL;
    spin_unlock(&ts_resource->id_res[TSDRV_STREAM_ID].spinlock);
}

void tsdrv_pack_reserved_sqcq_info(u32 sqid, u32 cqid, u32 pool_id, u32 pri,
    struct tsdrv_reserved_sqcq_info *sqcq_info)
{
    sqcq_info->sqid = sqid;
    sqcq_info->cqid = cqid;
    sqcq_info->pool_id = pool_id;
    sqcq_info->pri = pri;
}

void *tsdrv_create_task_topic_sched_submit_chan(u32 devid, u32 vfid, u32 tsid, int type,
    u32 sq_depth, u32 cq_depth, u32 pool_id, u32 pri)
{
    struct tsdrv_task_submit_chan *chan = NULL;
    struct devdrv_ts_cq_info *cq_info = NULL;
    struct devdrv_cq_sub_info *cq_sub = NULL;
    struct devdrv_ts_sq_info *sq_info = NULL;
    enum tsdrv_dev_status status;
    enum phy_sqcq_type sqcq_type;
    u16 cmd_type;
    int ret, sqid, cqid;

    ret = tsdrv_task_submit_chan_para_check(devid, vfid, tsid, sq_depth, cq_depth);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Task submit chan param invalid. (devid=%u; vfid=%u; tsid=%u; sq_depth=%u; "
            "cq_depth=%u)\n", devid, vfid, tsid, sq_depth, cq_depth);
        return NULL;
#endif
    }

    status = tsdrv_get_dev_status(devid, vfid);
    if (status != TSDRV_DEV_ACTIVE) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Dev status invalid. (devid=%u; vfid=%u; status=%d)\n", devid, vfid, (int)status);
        return NULL;
#endif
    }

    TSDRV_PRINT_FPGA("---\n");
    chan = vmalloc(sizeof(struct tsdrv_task_submit_chan));
    if (chan == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Alloc chan mem failed. (devid=%u; vfid=%u; tsid=%u)\n", devid, vfid, tsid);
        return NULL;
#endif
    }
    (void)memset_s((void *)chan, sizeof(struct tsdrv_task_submit_chan), 0, sizeof(struct tsdrv_task_submit_chan));

    sqcq_type = (type == USER_CHAN_TYPE) ? NORMAL_SQCQ_TYPE : KERNEL_SQCQ_TYPE;

    sqid = tsdrv_kernel_alloc_sq(devid, vfid, tsid, DEVDRV_SQ_SLOT_SIZE, sq_depth, sqcq_type);
    if (sqid < 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Alloc sq failed. (devid=%u; vfid=%u; tsid=%u)\n", devid, vfid, tsid);
        goto free_chan_mem;
#endif
    }

    cqid = tsdrv_kernel_alloc_cq(devid, vfid, tsid, DEVDRV_CQ_SLOT_SIZE, cq_depth, sqcq_type);
    if (cqid < 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Alloc cq failed. (devid=%u; vfid=%u; tsid=%u)\n", devid, vfid, tsid);
        goto free_sq;
#endif
    }

    cq_info = tsdrv_get_cq_info(devid, vfid, tsid, cqid);
    if (cq_info == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Get cq_info failed. (devid=%u; vfid=%u; tsid=%u; cqid=%d)\n",
            devid, vfid, tsid, cqid);
        goto free_cq;
#endif
    }
    cq_sub = cq_info->cq_sub;

    chan->devid = devid;
    chan->vfid = vfid;
    chan->tsid = tsid;
    chan->sqid = (u32)sqid;
    chan->cqid = (u32)cqid;
    chan->report_handle = NULL;

    TSDRV_PRINT_INFO("Chan info. (devid=%u; vfid=%u; tsid=%u; phy_sqid=%d; phy_cqid=%d)\n",
        devid, vfid, tsid, sqid, cqid);

    spin_lock_irq(&cq_sub->spinlock);
    cq_sub->ctx = (void *)chan;
    cq_sub->chan = (void *)chan;
    cq_sub->complete_handle = tsdrv_task_complete;
    spin_unlock_irq(&cq_sub->spinlock);
    spin_lock_init(&chan->lock);

    cmd_type = (type == USER_CHAN_TYPE) ? TSDRV_MBOX_CREATE_KERNEL_SQCQ : TSDRV_MBOX_NOTICE_KERNER_SQCQ;

    ret = tsdrv_sqcq_inform_ts(devid, vfid, tsid, (u32)sqid, (u32)cqid, pool_id, cmd_type, pri);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Inform ts failed. (devid=%u; vfid=%u; tsid=%u)\n", devid, vfid, tsid);
        goto free_cq;
#endif
    }

    sq_info = tsdrv_get_sq_info(devid, vfid, tsid, (u32)sqid);
    sq_info->bind_cqid = (u32)cqid;

    if (tsdrv_get_pfvf_type_by_devid(devid) == DEVDRV_SRIOV_TYPE_PF) {
        struct tsdrv_reserved_sqcq_info sqcq_info;
        tsdrv_pack_reserved_sqcq_info((u32)sqid, (u32)cqid, pool_id, pri, &sqcq_info);
        tsdrv_reserved_sqcq_store(devid, vfid, tsid, &sqcq_info);
    }

    TSDRV_PRINT_FPGA("---\n");
    return (void *)chan;
#ifndef TSDRV_UT
free_cq:
    tsdrv_kernel_free_cq(devid, vfid, tsid, (u32)cqid);

free_sq:
    tsdrv_kernel_free_sq(devid, vfid, tsid, (u32)sqid);

free_chan_mem:
    vfree(chan);

    return NULL;
#endif
}
TSDRV_EXPORT_SYMBOL(tsdrv_create_task_topic_sched_submit_chan);

void *tsdrv_task_submit_chan_create(u32 devid, u32 vfid, u32 tsid,
    int type, u32 sq_depth, u32 cq_depth)
{
    return tsdrv_create_task_topic_sched_submit_chan(devid, vfid, tsid, type, sq_depth, cq_depth, 0, 0);
}
TSDRV_EXPORT_SYMBOL(tsdrv_task_submit_chan_create);

void tsdrv_destroy_task_submit_chan(void *handle, int type)
{
    struct tsdrv_task_submit_chan *chan = (struct tsdrv_task_submit_chan *)handle;
    struct devdrv_ts_sq_info *sq_info = NULL;
    u32 bind_cqid;
    int ret;

    ret = tsdrv_submit_task_chan_check(chan);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Chan invalid.\n");
        return;
    }

    if (type == USER_CHAN_TYPE) {
        ret = tsdrv_sqcq_inform_ts(chan->devid, chan->vfid, chan->tsid, chan->sqid, chan->cqid,
            0, TSDRV_MBOX_RELEASE_KERNEL_SQCQ, 0);
        if (ret != 0) {
            TSDRV_PRINT_ERR("Send kernel sqcq release info to ts failed."
                "(devid=%u; vfid=%u; tsid=%u; sqid=%u; cqid=%u)\n",
                chan->devid, chan->vfid, chan->tsid, chan->sqid, chan->cqid);
            return;
        }
    }

    sq_info = tsdrv_get_sq_info(chan->devid, chan->vfid, chan->tsid, chan->sqid);
    bind_cqid = sq_info->bind_cqid;
    sq_info->bind_cqid = DEVDRV_MAX_CQ_NUM;
    tsdrv_kernel_free_sq(chan->devid, chan->vfid, chan->tsid, chan->sqid);
    tsdrv_kernel_free_cq(chan->devid, chan->vfid, chan->tsid, bind_cqid);

    TSDRV_PRINT_DEBUG("Destroy task submit channel success. (devid=%u)\n", chan->devid);
    vfree(chan);
    chan = NULL;
}
TSDRV_EXPORT_SYMBOL(tsdrv_destroy_task_submit_chan);

/* alloc sq for ts submit task */
int tsdrv_alloc_ts_sqcq(u32 devid, u32 vfid, u32 tsid)
{
#if !defined(CFG_SOC_PLATFORM_CLOUD_V2) && !defined(CFG_SOC_PLATFORM_MINIV3)
    return 0;
#else

    struct tsdrv_ts_resource *ts_res = NULL;
    struct tsdrv_ts_resource *vf_ts_res = NULL;
    struct devdrv_notice_ts_sqcq_mailbox notice_msg;
    struct devdrv_ts_sq_info *sq_info = NULL;
    int ret;
    int result = 0;
    int sqid, cqid;
    u32 phy_devid, fid;

    /* only device alloc sqcq for ts */
    if (tsdrv_get_env_type() == TSDRV_ENV_ONLINE) {
        return 0;
    }

    if (tsdrv_get_pfvf_type_by_devid(devid) == DEVDRV_SRIOV_TYPE_PF) {
        phy_devid = devid;
        fid = vfid;
    } else {
#ifndef AOS_LLVM_BUILD
        ret = devdrv_get_pfvf_id_by_devid(devid, &phy_devid, &fid);
        if (ret != 0) {
            TSDRV_PRINT_ERR("Failed to get physic devid and fid. (devid=%u)\n", devid);
            return ret;
        }
        fid = fid - 1;
#endif
    }
    ts_res = tsdrv_get_ts_resoruce(phy_devid, vfid, tsid);

    sqid = tsdrv_kernel_alloc_sq(devid, vfid, tsid, DEVDRV_SQ_SLOT_SIZE, 0, TS_SQCQ_TYPE);
    if (sqid < 0) {
        TSDRV_PRINT_ERR("devid %u vfid %u tsid %u alloc sq failed\n", devid, vfid, tsid);
        return -EFAULT;
    }

    cqid = tsdrv_kernel_alloc_cq(devid, vfid, tsid, DEVDRV_CQ_SLOT_SIZE, 0, TS_SQCQ_TYPE);
    if (cqid < 0) {
        tsdrv_kernel_free_sq(devid, vfid, tsid, (u32)sqid);
        TSDRV_PRINT_ERR("devid %u vfid %u tsid %u alloc cq failed\n", devid, vfid, tsid);
        return -EFAULT;
    }

    notice_msg.head.valid = DEVDRV_MAILBOX_MESSAGE_VALID;
    notice_msg.head.cmd_type = TSDRV_MBOX_NOTICE_TS_SQCQ_CREATE;
    notice_msg.head.result = 0;

    notice_msg.sq_id = (u32)sqid;
    notice_msg.cq_id = (u32)cqid;
    notice_msg.vfid = fid;

    ret = devdrv_mailbox_kernel_sync_no_feedback(&ts_res->mailbox, (u8 *)&notice_msg, sizeof(notice_msg), &result);
    TSDRV_PRINT_DEBUG("(devid=%u; vfid=%u; phy_devid=%u; fid=%u; sq_id=%u; cq_id=%u; ret=%d; result=%d)\n",
        devid, vfid, phy_devid, fid, sqid, cqid, ret, result);

    if (tsdrv_get_pfvf_type_by_devid(devid) == DEVDRV_SRIOV_TYPE_PF) {
        struct tsdrv_reserved_sqcq_info sqcq_info;
        tsdrv_pack_reserved_sqcq_info((u32)sqid, (u32)cqid, 0, 0, &sqcq_info);
        tsdrv_reserved_sqcq_store(devid, vfid, tsid, &sqcq_info);
    } else {
        vf_ts_res = tsdrv_get_ts_resoruce(devid, vfid, tsid);
        vf_ts_res->reserved_sqcq[vf_ts_res->sqcq_reserved_num].sq_info.index = sqid;
        vf_ts_res->reserved_sqcq[vf_ts_res->sqcq_reserved_num].cq_info.index = cqid;
        vf_ts_res->sqcq_reserved_num++;
        TSDRV_PRINT_DEBUG("(devid=%u; reserved_num=%u; sqid=%d; cqid=%d)\n", devid,
            vf_ts_res->sqcq_reserved_num, sqid, cqid);
    }

    sq_info = tsdrv_get_sq_info(devid, vfid, tsid, sqid);
    sq_info->bind_cqid = cqid;

    return 0;
#endif
}

STATIC void tsrdv_phy_cq_device_mem_recycle(struct devdrv_cq_sub_info *cq_sub)
{
#ifndef TSDRV_UT
    phys_addr_t virt_addr;

    spin_lock_irq(&cq_sub->spinlock);
    if (cq_sub->virt_addr == 0) {
        spin_unlock_irq(&cq_sub->spinlock);
        return;
    }

    virt_addr = cq_sub->virt_addr;
    cq_sub->virt_addr = 0;
    spin_unlock_irq(&cq_sub->spinlock);

#ifdef CFG_SOC_PLATFORM_MINIV2
    iounmap((void __iomem *)(uintptr_t)virt_addr);
#else
    tsdrv_free_pages_exact((void *)(uintptr_t)virt_addr, cq_sub->size);
#endif

    cq_sub->phy_addr = 0;
    cq_sub->size = 0;
    cq_sub->slot_size = 0;
    cq_sub->depth = 0;
#endif
}

STATIC void tsrdv_phy_cq_host_mem_recycle(u32 devid, u32 tsid, struct devdrv_cq_sub_info *cq_sub)
{
#ifndef TSDRV_UT
    struct device *pci_dev = NULL;
    phys_addr_t virt_addr;

    pci_dev = tsdrv_get_dev_p(devid);
    if (pci_dev == NULL) {
        TSDRV_PRINT_ERR("get device instance fail, devid=%u tsid=%u\n", devid, tsid);
        return;
    }
    if (cq_sub->phy_addr != 0) {
        dma_unmap_single(pci_dev, cq_sub->phy_addr, cq_sub->size, DMA_BIDIRECTIONAL);
        cq_sub->phy_addr = 0;
    }

    spin_lock_irq(&cq_sub->spinlock);
    if (cq_sub->virt_addr == 0) {
        spin_unlock_irq(&cq_sub->spinlock);
        return;
    }

    virt_addr = cq_sub->virt_addr;
    cq_sub->virt_addr = 0;
    spin_unlock_irq(&cq_sub->spinlock);
    tsdrv_free_pages_exact((void *)(uintptr_t)virt_addr, cq_sub->size);
    cq_sub->slot_size = 0;
    cq_sub->depth = 0;
    cq_sub->size = 0;
#endif
}

int tsdrv_phy_cq_recycle(struct tsdrv_ctx *ctx, u32 tsid, struct phy_cq_free_para *para)
{
    struct devdrv_ts_cq_info *cq_info = NULL;
    struct devdrv_cq_sub_info *cq_sub = NULL;
    struct tsdrv_ts_resource *ts_res = NULL;
    struct tsdrv_id_info *cq_id_info = NULL;
    u32 devid = tsdrv_get_devid_by_ctx(ctx);
    struct tsdrv_ts_ctx *ts_ctx = NULL;
    u32 fid = tsdrv_get_fid_by_ctx(ctx);

    TSDRV_PRINT_INFO("enter devid=%u, tsid=%u, fid=%u.\n", devid, tsid, fid);
    ts_res = tsdrv_ctx_to_ts_res(ctx, tsid);
    ts_ctx = &ctx->ts_ctx[tsid];
    cq_info = tsdrv_get_cq_info(devid, TSDRV_PM_FID, ts_res->tsid, para->cq_id);
    if ((cq_info == NULL) || (cq_info->cq_sub == NULL)) {
        TSDRV_PRINT_ERR("get pm sq info base fail, devid=%u tsid=%u\n", devid, ts_res->tsid);
        return -EFAULT;
    }
    cq_sub = cq_info->cq_sub;
    spin_lock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
    spin_lock_irq(&cq_sub->spinlock);
    cq_info->tgid = 0;
    cq_info->head = 0;
    cq_info->tail = 0;
    cq_info->alloc_status = SQCQ_INACTIVE;
    cq_sub->ctx = NULL;
    cq_info->type = NORMAL_SQCQ_TYPE; // default type
    spin_unlock_irq(&cq_sub->spinlock);
    cq_id_info = devdrv_find_one_id(&ts_res->id_res[TSDRV_CQ_ID], para->cq_id);
    if (cq_id_info == NULL) {
        spin_unlock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
        return -ENODEV;
    }
    list_del(&cq_id_info->list);
    ts_ctx->id_ctx[TSDRV_CQ_ID].id_num--;
    spin_unlock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);

    if (tsdrv_get_env_type() == TSDRV_ENV_OFFLINE) {
        tsrdv_phy_cq_device_mem_recycle(cq_sub);
    } else {
        tsrdv_phy_cq_host_mem_recycle(devid, tsid, cq_sub);
    }
    spin_lock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
    list_add(&cq_id_info->list, &ts_res->id_res[TSDRV_CQ_ID].id_available_list);
    ts_res->id_res[TSDRV_CQ_ID].id_available_num++;
    spin_unlock(&ts_res->id_res[TSDRV_CQ_ID].spinlock);
    TSDRV_PRINT_INFO("exit devid = %u, tsid = %u, fid = %u.\n", devid, tsid, fid);
    return 0;
}

void tsdrv_update_cq_head(struct tsdrv_ts_resource *ts_res, u32 cqid, u32 cq_head)
{
    struct devdrv_ts_cq_info *cq_info = NULL;
    phys_addr_t info_mem_addr;

    info_mem_addr = ts_res->mem_info[DEVDRV_INFO_MEM].virt_addr;
    cq_info = devdrv_calc_cq_info(info_mem_addr, cqid);
    cq_info->head = cq_head;
}

void tsdrv_dma_sync_cpu(u32 devid, dma_addr_t dma_addr, size_t size, enum dma_data_direction dir)
{
#ifndef TSDRV_UT
    enum tsdrv_env_type env_type = tsdrv_get_env_type();
    if (env_type == TSDRV_ENV_ONLINE) {
        struct device *pci_dev = NULL;
        pci_dev = tsdrv_get_dev_p(devid);
        if (pci_dev != NULL) {
            dma_sync_single_for_cpu(pci_dev, dma_addr, size, dir);
        }
    }
#endif
}
