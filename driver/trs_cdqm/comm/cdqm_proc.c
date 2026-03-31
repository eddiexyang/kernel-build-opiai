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
* Create: 2022-7-9
*/
#include <linux/slab.h>

#include "trs_pub_def.h"
#include "soc_res.h"
#include "cdqm_proc.h"

struct cdqm_proc_ctx *cdqm_proc_ctx_create(u32 devid)
{
    struct cdqm_proc_ctx *proc_ctx = NULL;
    int ret;

    proc_ctx = kzalloc(sizeof(struct cdqm_proc_ctx), GFP_KERNEL | __GFP_ACCOUNT);
    if (proc_ctx == NULL) {
        trs_err("Mem alloc failed. (size=%lx)\n", sizeof(struct cdqm_proc_ctx));
        return NULL;
    }

    proc_ctx->devid = devid;
    proc_ctx->pid = current->tgid;

    ret = soc_resmng_subsys_get_num(proc_ctx->devid, TS_SUBSYS, &proc_ctx->tsnum);
    if (ret != 0) {
        trs_err("Get tsnum failed. (devid=%u)\n", proc_ctx->devid);
        kfree(proc_ctx);
        return NULL;
    }

    return proc_ctx;
}

void cdqm_proc_ctx_destroy(struct cdqm_proc_ctx *proc_ctx)
{
    if (proc_ctx != NULL) {
        kfree(proc_ctx);
    }
}

#define CTX_HASH_TABLE_MASK ((0x1 << CTX_HASH_TABLE_BIT) - 1)
struct cdqm_proc_ctx *cdqm_proc_ctx_find(struct cdqm_dev_manage *cdq_dev, int pid)
{
    struct cdqm_proc_ctx *proc_ctx = NULL;
    int key = pid & CTX_HASH_TABLE_MASK;

    hash_for_each_possible(cdq_dev->proc_hash_table, proc_ctx, link, key) {
        if (proc_ctx->pid == pid) {
            return proc_ctx;
        }
    }

    return NULL;
}

void cdqm_proc_ctx_add(struct cdqm_dev_manage *cdq_dev, struct cdqm_proc_ctx *proc_ctx)
{
    int key = proc_ctx->pid & CTX_HASH_TABLE_MASK;

    hash_add(cdq_dev->proc_hash_table, &proc_ctx->link, key);
}

void cdqm_proc_ctx_del(struct cdqm_proc_ctx *proc_ctx)
{
    hash_del(&proc_ctx->link);
}
