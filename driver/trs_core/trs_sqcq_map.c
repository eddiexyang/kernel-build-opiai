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
* Create: 2022-12-23
*/
#include <linux/mm.h>
#include <linux/io.h>

#include "kernel_cgroup_mem_adapt.h"

#include "trs_ts_inst.h"
#include "trs_sqcq_map.h"

static void trs_sq_mem_free(struct trs_sq_ctx *sq_ctx)
{
    if (sq_ctx->que_mem.kva != NULL) {
        free_pages_exact(sq_ctx->que_mem.kva, sq_ctx->que_mem.len);
        sq_ctx->que_mem.kva = NULL;
    }

    if (sq_ctx->head.kva != NULL) {
        free_pages_exact(sq_ctx->head.kva, sq_ctx->head.len);
        sq_ctx->head.kva = NULL;
    }

    if (sq_ctx->tail.kva != NULL) {
        free_pages_exact(sq_ctx->tail.kva, sq_ctx->tail.len);
        sq_ctx->tail.kva = NULL;
    }
}

static int trs_sq_mem_alloc(struct trs_sq_ctx *sq_ctx, unsigned long que_mem_len)
{
    sq_ctx->que_mem.kva = ka_alloc_pages_exact(que_mem_len, GFP_KERNEL | __GFP_ZERO | __GFP_ACCOUNT |
        __GFP_RETRY_MAYFAIL);
    if (sq_ctx->que_mem.kva == NULL) {
        return -ENOMEM;
    }
    sq_ctx->que_mem.len = que_mem_len;

    sq_ctx->head.kva = ka_alloc_pages_exact(PAGE_SIZE, GFP_KERNEL | __GFP_ZERO | __GFP_ACCOUNT | __GFP_RETRY_MAYFAIL);
    if (sq_ctx->head.kva == NULL) {
        trs_sq_mem_free(sq_ctx);
        return -ENOMEM;
    }
    sq_ctx->head.len = PAGE_SIZE;

    sq_ctx->tail.kva = ka_alloc_pages_exact(PAGE_SIZE, GFP_KERNEL | __GFP_ZERO | __GFP_ACCOUNT | __GFP_RETRY_MAYFAIL);
    if (sq_ctx->tail.kva == NULL) {
        trs_sq_mem_free(sq_ctx);
        return -ENOMEM;
    }
    sq_ctx->tail.len = PAGE_SIZE;

    return 0;
}

static int trs_sq_que_remap(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst,
    struct trs_sq_ctx *sq_ctx, struct trs_mem_map_para *map_para)
{
    int ret;

    ret = trs_remap_sq(proc_ctx, ts_inst, map_para);
    if (ret == 0) {
        sq_ctx->que_mem.uva = map_para->va;
        sq_ctx->que_mem.len = map_para->len;
        sq_ctx->que_mem.mem_type = map_para->type;
    }

    return ret;
}

static void trs_sq_que_unmap(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, struct trs_sq_ctx *sq_ctx)
{
    struct trs_mem_unmap_para unmap_para;

    if (sq_ctx->que_mem.uva != 0) {
        trs_unmap_fill_para(&unmap_para, sq_ctx->que_mem.mem_type, sq_ctx->que_mem.uva, sq_ctx->que_mem.len);
        (void)trs_unmap_sq(proc_ctx, ts_inst, &unmap_para);
        sq_ctx->que_mem.uva = 0;
    }
}

static int trs_sq_head_remap(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst,
    struct trs_sq_ctx *sq_ctx, struct trs_mem_map_para *map_para)
{
    int ret;

    if (map_para->pa == 0) {
        return -EFAULT;
    }

    ret = trs_remap_sq(proc_ctx, ts_inst, map_para);
    if (ret == 0) {
        sq_ctx->head.uva = map_para->va;
        sq_ctx->head.len = map_para->len;
        sq_ctx->head.mem_type = map_para->type;
    }

    return ret;
}

static void trs_sq_head_unmap(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst,
    struct trs_sq_ctx *sq_ctx)
{
    struct trs_mem_unmap_para unmap_para;

    if (sq_ctx->head.uva != 0) {
        trs_unmap_fill_para(&unmap_para, sq_ctx->head.mem_type, sq_ctx->head.uva, sq_ctx->head.len);
        (void)trs_unmap_sq(proc_ctx, ts_inst, &unmap_para);
        sq_ctx->head.uva = 0;
    }
}

static int trs_sq_tail_remap(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst,
    struct trs_sq_ctx *sq_ctx, struct trs_mem_map_para *map_para)
{
    int ret;

    if (map_para->pa == 0) {
        return -EFAULT;
    }

    ret = trs_remap_sq(proc_ctx, ts_inst, map_para);
    if (ret == 0) {
        sq_ctx->tail.uva = map_para->va;
        sq_ctx->tail.len = map_para->len;
        sq_ctx->tail.mem_type = map_para->type;
    }

    return ret;
}

static void trs_sq_tail_unmap(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst,
    struct trs_sq_ctx *sq_ctx)
{
    struct trs_mem_unmap_para unmap_para;

    if (sq_ctx->tail.uva != 0) {
        trs_unmap_fill_para(&unmap_para, sq_ctx->tail.mem_type, sq_ctx->tail.uva, sq_ctx->tail.len);
        (void)trs_unmap_sq(proc_ctx, ts_inst, &unmap_para);
        sq_ctx->tail.uva = 0;
    }
}

static int trs_sq_db_remap(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst,
    struct trs_sq_ctx *sq_ctx, struct trs_mem_map_para *map_para)
{
    int ret;

    ret = trs_remap_sq(proc_ctx, ts_inst, map_para);
    if (ret == 0) {
        sq_ctx->db.uva = map_para->va;
        sq_ctx->db.len = map_para->len;
        sq_ctx->db.mem_type = map_para->type;
    }

    return ret;
}

static void trs_sq_db_unmap(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, struct trs_sq_ctx *sq_ctx)
{
    struct trs_mem_unmap_para unmap_para;

    if (sq_ctx->db.uva != 0) {
        trs_unmap_fill_para(&unmap_para, sq_ctx->db.mem_type, sq_ctx->db.uva, sq_ctx->db.len);
        (void)trs_unmap_sq(proc_ctx, ts_inst, &unmap_para);
        sq_ctx->db.uva = 0;
    }
}

static void trs_refill_sq_info(struct trs_sq_ctx *sq_ctx, struct trs_chan_sq_info *sq_info)
{
    sq_info->sq_phy_addr = (u64)virt_to_phys(sq_ctx->que_mem.kva);
    sq_info->head_addr = (u64)virt_to_phys(sq_ctx->head.kva);
    sq_info->tail_addr = (u64)virt_to_phys(sq_ctx->tail.kva);
}

static void trs_sq_shr_info_mem_free(struct trs_sq_ctx *sq_ctx)
{
    if (sq_ctx->shr_info.kva != NULL) {
        spin_lock_bh(&sq_ctx->shr_info_lock);
        free_pages_exact(sq_ctx->shr_info.kva, sq_ctx->shr_info.len);
        sq_ctx->shr_info.kva = NULL;
        spin_unlock_bh(&sq_ctx->shr_info_lock);
    }
}

static void trs_sq_shr_info_remap(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst,
    struct trs_uio_info *uio_info, struct trs_sq_ctx *sq_ctx, struct trs_chan_sq_info *sq_info)
{
    struct trs_mem_map_para map_para;
    int ret;

    sq_ctx->shr_info.kva = ka_alloc_pages_exact(PAGE_SIZE, GFP_KERNEL | __GFP_ZERO | __GFP_ACCOUNT |
        __GFP_RETRY_MAYFAIL);
    if (sq_ctx->shr_info.kva == NULL) {
        return;
    }

    trs_remap_fill_para(&map_para, TRS_MAP_TYPE_MEM, uio_info->sq_ctrl_addr[TRS_UIO_SHR_INFO],
        virt_to_phys(sq_ctx->shr_info.kva), PAGE_SIZE);
    ret = trs_remap_sq(proc_ctx, ts_inst, &map_para);
    if (ret != 0) {
        trs_sq_shr_info_mem_free(sq_ctx);
        uio_info->sq_ctrl_addr[TRS_UIO_SHR_INFO] = 0;
        return;
    }

    sq_ctx->shr_info.len = PAGE_SIZE;
    sq_ctx->shr_info.uva = uio_info->sq_ctrl_addr[TRS_UIO_SHR_INFO];
    sq_ctx->shr_info.mem_type = TRS_MAP_TYPE_MEM;
}

static void trs_sq_shr_info_unmap(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst,
    struct trs_sq_ctx *sq_ctx)
{
    if (sq_ctx->shr_info.uva != 0) {
        struct trs_mem_unmap_para unmap_para;
        trs_unmap_fill_para(&unmap_para, sq_ctx->shr_info.mem_type, sq_ctx->shr_info.uva, sq_ctx->shr_info.len);
        (void)trs_unmap_sq(proc_ctx, ts_inst, &unmap_para);
        sq_ctx->shr_info.uva = 0;
    }

    trs_sq_shr_info_mem_free(sq_ctx);
}

static void trs_sq_reg_remap(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst,
    struct trs_uio_info *uio_info, struct trs_sq_ctx *sq_ctx, struct trs_chan_sq_info *sq_info)
{
    struct trs_mem_map_para map_para;
    int ret;

    /* head reg addr not page align */
    trs_remap_fill_para(&map_para, TRS_MAP_TYPE_RO_REG,
        uio_info->sq_ctrl_addr[TRS_UIO_HEAD_REG], ALIGN_DOWN(sq_info->head_addr, PAGE_SIZE), PAGE_SIZE);
    ret = trs_remap_sq(proc_ctx, ts_inst, &map_para);
    if (ret == 0) {
        sq_ctx->head_reg.uva = map_para.va;
        sq_ctx->head_reg.len = map_para.len;
        sq_ctx->head_reg.mem_type = TRS_MAP_TYPE_RO_REG;
        uio_info->sq_ctrl_addr[TRS_UIO_HEAD_REG] += sq_info->head_addr % map_para.len;
    } else {
        uio_info->sq_ctrl_addr[TRS_UIO_HEAD_REG] = 0;
    }

    /* tail reg addr not page align */
    trs_remap_fill_para(&map_para, TRS_MAP_TYPE_RO_REG,
        uio_info->sq_ctrl_addr[TRS_UIO_TAIL_REG], ALIGN_DOWN(sq_info->tail_addr, PAGE_SIZE), PAGE_SIZE);
    ret = trs_remap_sq(proc_ctx, ts_inst, &map_para);
    if (ret == 0) {
        sq_ctx->tail_reg.uva = map_para.va;
        sq_ctx->tail_reg.len = map_para.len;
        sq_ctx->tail_reg.mem_type = TRS_MAP_TYPE_RO_REG;
        uio_info->sq_ctrl_addr[TRS_UIO_TAIL_REG] += sq_info->tail_addr % map_para.len;
    } else {
        uio_info->sq_ctrl_addr[TRS_UIO_TAIL_REG] = 0;
    }
}

static void trs_sq_reg_unmap(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst,
    struct trs_sq_ctx *sq_ctx)
{
    struct trs_mem_unmap_para unmap_para;

    if (sq_ctx->head_reg.uva != 0) {
        trs_unmap_fill_para(&unmap_para, sq_ctx->head_reg.mem_type, sq_ctx->head_reg.uva, sq_ctx->head_reg.len);
        (void)trs_unmap_sq(proc_ctx, ts_inst, &unmap_para);
        sq_ctx->head_reg.uva = 0;
    }

    if (sq_ctx->tail_reg.uva != 0) {
        trs_unmap_fill_para(&unmap_para, sq_ctx->tail_reg.mem_type, sq_ctx->tail_reg.uva, sq_ctx->tail_reg.len);
        (void)trs_unmap_sq(proc_ctx, ts_inst, &unmap_para);
        sq_ctx->tail_reg.uva = 0;
    }
}

int trs_sq_remap(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst,
    struct halSqCqInputInfo *para, struct trs_sq_ctx *sq_ctx, struct trs_chan_sq_info *sq_info)
{
    struct trs_id_inst *inst = &ts_inst->inst;
    struct trs_alloc_para *alloc_para = get_alloc_para_addr(para);
    struct trs_uio_info *uio_info = alloc_para->uio_info;
    struct trs_mem_map_para map_para;
    unsigned long que_mem_len = PAGE_ALIGN(sq_info->sq_para.sq_depth * sq_info->sq_para.sqe_size);
    int sq_mem_type = TRS_MAP_TYPE_DEV_MEM;
    int sq_reg_type = TRS_MAP_TYPE_REG;
    int ret;

    if ((sq_info->sq_phy_addr == 0) || (sq_info->db_addr == 0) || (uio_info->sq_que_addr == 0)) {
        ret = 0;
        goto out;
    }

    trs_sq_shr_info_remap(proc_ctx, ts_inst, uio_info, sq_ctx, sq_info);

    uio_info->soft_que_flag = 0;
    if ((ts_inst->ops.get_sq_trigger_irq != NULL) && (para->type == DRV_NORMAL_TYPE)) {
        uio_info->soft_que_flag = 1;

        trs_sq_reg_remap(proc_ctx, ts_inst, uio_info, sq_ctx, sq_info);

        ret = trs_sq_mem_alloc(sq_ctx, que_mem_len);
        if (ret != 0) {
            goto fail;
        }
        trs_refill_sq_info(sq_ctx, sq_info);
        sq_mem_type = TRS_MAP_TYPE_MEM;
        sq_reg_type = TRS_MAP_TYPE_MEM;
    }

    trs_remap_fill_para(&map_para, sq_mem_type, uio_info->sq_que_addr, sq_info->sq_phy_addr, que_mem_len);
    ret = trs_sq_que_remap(proc_ctx, ts_inst, sq_ctx, &map_para);
    if (ret != 0) {
        goto fail;
    }

    /* head addr not page align */
    trs_remap_fill_para(&map_para, sq_reg_type, uio_info->sq_ctrl_addr[TRS_UIO_HEAD],
        ALIGN_DOWN(sq_info->head_addr, PAGE_SIZE), PAGE_SIZE);
    uio_info->sq_ctrl_addr[TRS_UIO_HEAD] += sq_info->head_addr % PAGE_SIZE;
    ret = trs_sq_head_remap(proc_ctx, ts_inst, sq_ctx, &map_para);
    if (ret != 0) {
        uio_info->sq_ctrl_addr[TRS_UIO_HEAD] = 0;
    }

    /* tail addr not page align */
    trs_remap_fill_para(&map_para, sq_reg_type, uio_info->sq_ctrl_addr[TRS_UIO_TAIL],
        ALIGN_DOWN(sq_info->tail_addr, PAGE_SIZE), PAGE_SIZE);
    uio_info->sq_ctrl_addr[TRS_UIO_TAIL] += sq_info->tail_addr % PAGE_SIZE;
    ret = trs_sq_tail_remap(proc_ctx, ts_inst, sq_ctx, &map_para);
    if (ret != 0) {
        uio_info->sq_ctrl_addr[TRS_UIO_TAIL] = 0;
    }

    /* db addr not page align */
    trs_remap_fill_para(&map_para, TRS_MAP_TYPE_REG, uio_info->sq_ctrl_addr[TRS_UIO_DB],
        ALIGN_DOWN(sq_info->db_addr, PAGE_SIZE), PAGE_SIZE);
    uio_info->sq_ctrl_addr[TRS_UIO_DB] += sq_info->db_addr % PAGE_SIZE;
    ret = trs_sq_db_remap(proc_ctx, ts_inst, sq_ctx, &map_para);
    if (ret != 0) {
        goto fail;
    }

    uio_info->uio_flag = 1;
    if (uio_info->soft_que_flag == 1) {
        sq_ctx->mode = SEND_MODE_UIO_T;
    } else {
        sq_ctx->mode = SEND_MODE_UIO_D;
    }

    trs_debug("Sq send use uio mode. (devid=%u; tsid=%u; sqid=%u)\n", inst->devid, inst->tsid, para->sqId);

    return 0;

fail:
    trs_sq_unmap(proc_ctx, ts_inst, sq_ctx);
out:
    uio_info->uio_flag = 0;
    uio_info->soft_que_flag = 0;
    sq_ctx->mode = SEND_MODE_KIO;
    trs_debug("Sq send use kernel mode. (devid=%u; tsid=%u; sqid=%u; type=%u; flag=%u, ret=%d)\n",
        inst->devid, inst->tsid, para->sqId, para->type, para->flag, ret);

    return ret;
}

void trs_sq_unmap(struct trs_proc_ctx *proc_ctx, struct trs_core_ts_inst *ts_inst, struct trs_sq_ctx *sq_ctx)
{
    trs_sq_que_unmap(proc_ctx, ts_inst, sq_ctx);
    trs_sq_head_unmap(proc_ctx, ts_inst, sq_ctx);
    trs_sq_tail_unmap(proc_ctx, ts_inst, sq_ctx);
    trs_sq_db_unmap(proc_ctx, ts_inst, sq_ctx);
    trs_sq_reg_unmap(proc_ctx, ts_inst, sq_ctx);
    trs_sq_shr_info_unmap(proc_ctx, ts_inst, sq_ctx);

    trs_sq_mem_free(sq_ctx);
}

void trs_sq_clear_map_info(struct trs_sq_ctx *sq_ctx)
{
    sq_ctx->que_mem.uva = 0;
    sq_ctx->head.uva = 0;
    sq_ctx->tail.uva = 0;
    sq_ctx->db.uva = 0;
    sq_ctx->head_reg.uva = 0;
    sq_ctx->tail_reg.uva = 0;
    sq_ctx->shr_info.uva = 0;
}

void trs_sq_ctx_mem_free(struct trs_sq_ctx *sq_ctx)
{
    trs_sq_shr_info_mem_free(sq_ctx);
    trs_sq_mem_free(sq_ctx);
}
