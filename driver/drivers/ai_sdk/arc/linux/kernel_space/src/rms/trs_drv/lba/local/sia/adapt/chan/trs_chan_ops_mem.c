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
* Create: 2022-12-15
*/
#include <linux/oom.h>
#include <linux/opiai_vendor_compat.h>

#include "trs_chan_mem.h"
#include "trs_device_comm.h"
#include "trs_chip_def.h"
#include "dbl/chip_config.h"

static int trs_chan_ops_get_sq_mem_type(struct trs_chan_type *types)
{
    int type = TRS_CHAN_MEM_MAX;

    switch (types->type) {
        case CHAN_TYPE_HW:
            type = trs_get_hw_sq_mem_type();
            break;
        case CHAN_TYPE_SW:
        case CHAN_TYPE_TASK_SCHED:
            type = trs_get_sw_sq_mem_type();
            break;
        case CHAN_TYPE_MAINT:
            type = trs_get_maint_sq_mem_type();
            break;
        default:
            break;
    }

    return type;
}

static int trs_chan_ops_get_cq_mem_type(struct trs_chan_type *types)
{
    int type = TRS_CHAN_MEM_MAX;

    switch (types->type) {
        case CHAN_TYPE_HW:
            type = trs_get_hw_cq_mem_type();
            break;
        case CHAN_TYPE_SW:
        case CHAN_TYPE_TASK_SCHED:
            type = trs_get_sw_cq_mem_type();
            break;
        case CHAN_TYPE_MAINT:
            type = trs_get_maint_cq_mem_type();
            break;
        default:
            break;
    }

    return type;
}

static int trs_chan_ops_get_rsv_mem_type(struct trs_chan_type *types)
{
    return (types->type == CHAN_TYPE_MAINT) ? trs_get_maint_sqcq_rsv_mem_type() : trs_get_hw_sqcq_rsv_mem_type();
}

static void *trs_chan_mem_alloc_nids_ddr(struct trs_id_inst *inst, size_t size, u64 *phy_addr)
{
    u32 nids[DBL_NUMA_ID_MAX_NUM];
    int node_num, i;

    node_num = dbl_get_ts_nid(inst->devid, nids, DBL_NUMA_ID_MAX_NUM);
    if ((node_num <= 0) || (node_num >= DBL_NUMA_ID_MAX_NUM)) {
        trs_err("Invalid node_num. (devid=%u; node_num=%d)\n", inst->devid, node_num);
        return NULL;
    }

    for (i = 0; i < node_num; i++) {
        void *vaddr = trs_chan_mem_alloc_ddr(inst, nids[i], size, phy_addr);
        if (vaddr != NULL) {
            return vaddr;
        }
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
    (void)oom_type_notifier_call(OOM_TYPE_CGROUP, NULL);
#else
    (void)hisi_oom_notifier_call(HISI_OOM_TYPE_CGROUP, NULL);
#endif

    return NULL;
}

void *trs_chan_ops_sq_mem_alloc(struct trs_id_inst *inst, struct trs_chan_type *types,
    struct trs_chan_sq_para *sq_para, u64 *phy_addr, u32 *attr)
{
    size_t size = sq_para->sqe_size * sq_para->sq_depth;
    void *vaddr = NULL;

    switch (trs_chan_ops_get_sq_mem_type(types)) {
        case TRS_CHAN_MEM_RSV:
            vaddr = trs_chan_mem_alloc_rsv(inst, trs_chan_ops_get_rsv_mem_type(types), size, phy_addr, 0);
            break;
        case TRS_CHAN_MEM_DDR:
            vaddr = trs_chan_mem_alloc_nids_ddr(inst, size, phy_addr);
            break;
        default:
            trs_err("Unknown type. (type=%d; sub_type=%d)\n", types->type, types->sub_type);
            break;
    }
    *attr |= (0x1 << CHAN_MEM_FLAG_LOCAL);
    return vaddr;
}
EXPORT_SYMBOL(trs_chan_ops_sq_mem_alloc);

void trs_chan_ops_sq_mem_free(struct trs_id_inst *inst, struct trs_chan_type *types,
    struct trs_chan_sq_para *sq_para, void *sq_addr, u64 phy_addr)
{
    size_t size = sq_para->sqe_size * sq_para->sq_depth;

    switch (trs_chan_ops_get_sq_mem_type(types)) {
        case TRS_CHAN_MEM_RSV:
            trs_chan_mem_free_rsv(inst, trs_chan_ops_get_rsv_mem_type(types), sq_addr, size);
            break;
        case TRS_CHAN_MEM_DDR:
            trs_chan_mem_free_ddr(inst, sq_addr, size);
            break;
        default:
            trs_err("Unknown type. (type=%d; sub_type=%d)\n", types->type, types->sub_type);
            break;
    }
}
EXPORT_SYMBOL(trs_chan_ops_sq_mem_free);

void *trs_chan_ops_cq_mem_alloc(struct trs_id_inst *inst, struct trs_chan_type *types,
    struct trs_chan_cq_para *cq_para, u64 *phy_addr, u32 *attr)
{
    size_t size = cq_para->cqe_size * cq_para->cq_depth;
    void *vaddr = NULL;

    switch (trs_chan_ops_get_cq_mem_type(types)) {
        case TRS_CHAN_MEM_RSV:
            vaddr = trs_chan_mem_alloc_rsv(inst, trs_chan_ops_get_rsv_mem_type(types), size, phy_addr,
                TRS_RSV_MEM_OP_ZERO);
            break;
        case TRS_CHAN_MEM_DDR:
            vaddr = trs_chan_mem_alloc_nids_ddr(inst, size, phy_addr);
            break;
        default:
            trs_err("Unknown type. (type=%d; sub_type=%d)\n", types->type, types->sub_type);
            break;
    }
    *attr |= (0x1 << CHAN_MEM_FLAG_LOCAL);
    return vaddr;
}
EXPORT_SYMBOL(trs_chan_ops_cq_mem_alloc);

void trs_chan_ops_cq_mem_free(struct trs_id_inst *inst, struct trs_chan_type *types,
    struct trs_chan_cq_para *cq_para, void *cq_addr, u64 phy_addr)
{
    size_t size = cq_para->cqe_size * cq_para->cq_depth;

    switch (trs_chan_ops_get_cq_mem_type(types)) {
        case TRS_CHAN_MEM_RSV:
            trs_chan_mem_free_rsv(inst, trs_chan_ops_get_rsv_mem_type(types), cq_addr, size);
            break;
        case TRS_CHAN_MEM_DDR:
            trs_chan_mem_free_ddr(inst, cq_addr, size);
            break;
        default:
            trs_err("Unknown type. (type=%d; sub_type=%d)\n", types->type, types->sub_type);
            break;
    }
}
EXPORT_SYMBOL(trs_chan_ops_cq_mem_free);

void trs_chan_ops_flush_sqe_cache(struct trs_id_inst *inst,
    struct trs_chan_type *types, void *addr, u64 pa, u32 len)
{
    if (trs_chan_ops_get_sq_mem_type(types) == TRS_CHAN_MEM_DDR) {
        trs_flush_cache((u64)addr, len);
    }
}
EXPORT_SYMBOL(trs_chan_ops_flush_sqe_cache);

void trs_chan_ops_invalid_cqe_cache(struct trs_id_inst *inst,
    struct trs_chan_type *types, void *addr, u64 pa, u32 len)
{
    if (trs_chan_ops_get_cq_mem_type(types) == TRS_CHAN_MEM_DDR) {
        trs_invalid_cache((u64)addr, len);
    }
}
EXPORT_SYMBOL(trs_chan_ops_invalid_cqe_cache);
