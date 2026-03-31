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

#ifndef TRS_SQCQ_CTX_H
#define TRS_SQCQ_CTX_H

#include <linux/mutex.h>
#include "ascend_hal_define.h"
#include "trs_pub_def.h"
#include "securec.h"
struct trs_sq_map_addr {
    u64 uva;
    void *kva;
    u64 len;
    int mem_type;
    int cp_proc_state;
};

enum sq_send_mode_type {
    SEND_MODE_KIO,
    SEND_MODE_UIO_T,
    SEND_MODE_UIO_D,
    SEND_MODE_IO_MAX
};

struct trs_sq_ctx {
    struct trs_id_inst inst;
    drvSqCqType_t type;
    u32 flag;
    u32 status;
    u32 sqid;
    u32 cqid;
    u32 stream_id;
    int chan_id;
    u32 sq_depth;
    u32 sqe_size;
    u32 sq_tail;
    u32 mode;
    u32 cqe_status;
    u64 send_fail;
    struct mutex mutex;
    struct trs_sq_map_addr que_mem;
    struct trs_sq_map_addr head;
    struct trs_sq_map_addr tail;
    struct trs_sq_map_addr db;
    struct trs_sq_map_addr head_reg;
    struct trs_sq_map_addr tail_reg;
    spinlock_t shr_info_lock;
    struct trs_sq_map_addr shr_info;
    struct trs_sq_map_addr reg_mem; /* RtSq reg va for stars */
};

struct trs_cq_stat {
    u64 rx;
    u64 rx_enque;
    u64 rx_drop;
    u64 rx_enque_fail;
};

struct trs_cq_ctx {
    int chan_id;
    u32 logic_cqid;
    struct trs_cq_stat stat;
};

static inline bool trs_cq_is_need_show(struct trs_cq_ctx *cq_ctx)
{
    return (cq_ctx->stat.rx_drop > 0 || cq_ctx->stat.rx_enque_fail > 0);
}

static inline void trs_sq_ctx_init(struct trs_id_inst *inst, struct trs_sq_ctx *sq_ctx, struct halSqCqInputInfo *para,
    u32 stream_id, int chan_id)
{
    sq_ctx->inst = *inst;
    sq_ctx->sqid = para->sqId;
    sq_ctx->cqid = para->cqId;
    sq_ctx->stream_id = stream_id;
    sq_ctx->chan_id = chan_id;
    sq_ctx->sq_depth = para->sqeDepth;
    sq_ctx->sqe_size = para->sqeSize;
    sq_ctx->sq_tail = 0;
    sq_ctx->send_fail = 0;
    sq_ctx->type = para->type;
    sq_ctx->flag = para->flag;
}

static inline void trs_sq_ctx_uninit(struct trs_sq_ctx *sq_ctx)
{
    sq_ctx->chan_id = -1;
}

static inline void trs_set_sq_status(struct trs_sq_ctx *sq_ctx, u32 status)
{
    mutex_lock(&sq_ctx->mutex);
    sq_ctx->status = status;
    mutex_unlock(&sq_ctx->mutex);
}

static inline void trs_cq_ctx_init(struct trs_cq_ctx *cq_ctx, u32 logic_cqid, int chan_id)
{
    cq_ctx->logic_cqid = logic_cqid;
    cq_ctx->chan_id = chan_id;
    (void)memset_s(&cq_ctx->stat, sizeof(struct trs_cq_stat), 0, sizeof(struct trs_cq_stat));
}

static inline void trs_cq_ctx_uninit(struct trs_cq_ctx *cq_ctx)
{
    cq_ctx->chan_id = -1;
}

static inline void trs_sq_ctxs_init(struct trs_sq_ctx *sq_ctx, u32 sq_num)
{
    u32 i;

    for (i = 0; i < sq_num; i++) {
        mutex_init(&sq_ctx[i].mutex);
        spin_lock_init(&sq_ctx[i].shr_info_lock);
    }
}
#endif

