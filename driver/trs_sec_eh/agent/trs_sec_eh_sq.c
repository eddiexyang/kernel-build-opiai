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
* Create: 2022-11-10
*/
#include <linux/io.h>

#include "securec.h"

#include "trs_chan_near_ops_mem.h"
#include "trs_chan_update.h"

#include "trs_sec_eh_id.h"
#include "trs_sec_eh_sq.h"

#define TRS_SEC_EH_MAX_SQE_SIZE 128
#define TRS_SEC_EH_MAX_SQDEPTH_SIZE (64 * 1024)

void trs_sec_eh_sq_ctx_init(struct trs_sec_eh_ts_inst *sec_eh_cfg, struct trs_sec_eh_sq_ctx_info *ctx)
{
    mutex_lock(&sec_eh_cfg->mutex);
    sec_eh_cfg->sq_ctx[ctx->sqid].sqe_size = ctx->sqesize;
    sec_eh_cfg->sq_ctx[ctx->sqid].sq_depth = ctx->sqdepth;
    sec_eh_cfg->sq_ctx[ctx->sqid].d_addr = ctx->sq_addr;
    sec_eh_cfg->sq_ctx[ctx->sqid].sq_paddr = ctx->sq_paddr;
    sec_eh_cfg->sq_ctx[ctx->sqid].pid = ctx->pid;
    mutex_unlock(&sec_eh_cfg->mutex);
}

void trs_sec_eh_sq_ctx_uninit(struct trs_sec_eh_ts_inst *sec_eh_cfg, u32 sqid)
{
    mutex_lock(&sec_eh_cfg->mutex);
    sec_eh_cfg->sq_ctx[sqid].sqe_size = 0;
    sec_eh_cfg->sq_ctx[sqid].sq_depth = 0;
    sec_eh_cfg->sq_ctx[sqid].d_addr = NULL;
    sec_eh_cfg->sq_ctx[sqid].sq_paddr = 0;
    sec_eh_cfg->sq_ctx[sqid].pid = 0;
    mutex_unlock(&sec_eh_cfg->mutex);
}

u64 trs_sec_eh_alloc_sq_mem(struct trs_sec_eh_ts_inst *sec_eh_cfg, struct trs_sec_eh_sq_ctx_info *ctx)
{
    u64 size = ctx->sqesize * ctx->sqdepth;
    u32 attr;

    if ((ctx->sqesize > TRS_SEC_EH_MAX_SQE_SIZE) || (size == 0) || (ctx->sqdepth > TRS_SEC_EH_MAX_SQDEPTH_SIZE)) {
        trs_err("Invalid. (devid=%u; tsid=%u; sqid=%u; offset=0x%pK)\n",
            sec_eh_cfg->inst.devid, sec_eh_cfg->inst.tsid, ctx->sqid, (void *)ctx->addr_offset);
        return -EINVAL;
    }

    ctx->sq_addr = trs_chan_sq_mem_alloc(&sec_eh_cfg->inst, ctx->sqesize, ctx->sqdepth, &ctx->sq_paddr, &attr);
    if (ctx->sq_addr == NULL) {
        return -ENOMEM;
    }

    return 0;
}

void trs_sec_eh_free_sq_mem(struct trs_sec_eh_ts_inst *sec_eh_cfg, u32 sqid)
{
    struct trs_sec_eh_sq_ctx *sq_ctx = sec_eh_cfg->sq_ctx;

    mutex_lock(&sec_eh_cfg->mutex);
    if (sq_ctx[sqid].d_addr != NULL) {
        trs_chan_sq_mem_free(&sec_eh_cfg->inst, sq_ctx[sqid].sqe_size, sq_ctx[sqid].sq_depth,
            sq_ctx[sqid].d_addr, sq_ctx[sqid].sq_paddr);
        sq_ctx[sqid].d_addr = NULL;
    }
    mutex_unlock(&sec_eh_cfg->mutex);
}

void trs_sec_eh_free_sq_mem_all(struct trs_id_inst *inst)
{
    struct trs_sec_eh_ts_inst *sec_eh_cfg = NULL;
    u32 num = 0;
    u32 sqid;

    sec_eh_cfg = trs_sec_eh_ts_inst_get(inst);
    if (sec_eh_cfg != NULL) {
        for (sqid = 0; sqid < sec_eh_cfg->id_info[TRS_HW_SQ].end; sqid++) {
            trs_sec_eh_free_sq_mem(sec_eh_cfg, sqid);
            num++;
        }

        trs_info("Kernel free sq success. (devid=%u; num=%u)\n", sec_eh_cfg->inst.devid, num);
        trs_sec_eh_ts_inst_put(sec_eh_cfg);
    }
}

int _trs_sec_eh_sqe_update(struct trs_id_inst *inst, int pid, u32 sqid, u32 sqeid, u8 data[])
{
    struct trs_sec_eh_ts_inst *sec_eh_cfg = NULL;
    u8 *s_addr = (void *)data;
    u8 *d_addr = NULL;
    u64 sqe_size, sqe_pa;
    int ret = -EINVAL;

    sec_eh_cfg = trs_sec_eh_ts_inst_get(inst);
    if (sec_eh_cfg != NULL) {
        if (!trs_sec_eh_id_is_belong_to_vf(&sec_eh_cfg->id_info[TRS_HW_SQ], sqid)) {
            trs_sec_eh_ts_inst_put(sec_eh_cfg);
            trs_debug("Invalid. (devid=%u; type=%d; id=%u)\n", inst->devid, TRS_HW_SQ, sqid);
            return -EACCES;
        }

        if (sqeid >= sec_eh_cfg->sq_ctx[sqid].sq_depth) {
            trs_sec_eh_ts_inst_put(sec_eh_cfg);
            return -EINVAL;
        }

        mutex_lock(&sec_eh_cfg->mutex);
        if (sec_eh_cfg->sq_ctx[sqid].d_addr == NULL) {
            trs_sec_eh_ts_inst_put(sec_eh_cfg);
            mutex_unlock(&sec_eh_cfg->mutex);
            return -EINVAL;
        }

        sqe_size = sec_eh_cfg->sq_ctx[sqid].sqe_size;
        d_addr = sec_eh_cfg->sq_ctx[sqid].d_addr + sqe_size * sqeid;
        mutex_unlock(&sec_eh_cfg->mutex);

        ret = trs_chan_ops_sqe_update(&sec_eh_cfg->inst, pid, s_addr);
        if (ret != 0) {
            trs_sec_eh_ts_inst_put(sec_eh_cfg);
            return ret;
        }

        memcpy_toio(d_addr, s_addr, sqe_size);
        sqe_pa = sec_eh_cfg->sq_ctx[sqid].sq_paddr + sqe_size * sqeid;
        trs_chan_flush_sqe_cache(&sec_eh_cfg->inst, sqe_pa, sqe_size);
        trs_sec_eh_ts_inst_put(sec_eh_cfg);
    }

    return ret;
}

