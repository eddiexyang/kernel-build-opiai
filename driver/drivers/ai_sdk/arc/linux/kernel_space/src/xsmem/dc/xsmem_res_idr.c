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
 * Create: 2022-08-13
 */
#include <linux/types.h>
#include <linux/printk.h>
#include <linux/idr.h>

#include "xsmem_framework.h"
#include "xsmem_res_dispatch.h"

#define XSMEM_MAX_ID (1UL << 14)
DEFINE_IDR(xsmem_idr);
static int xsmem_idr_alloc_id(struct xsm_pool *xp, int *id_in)
{
    static int next_xsmem_id = 0;
    int id;
    id = idr_alloc(&xsmem_idr, xp, next_xsmem_id, XSMEM_MAX_ID, GFP_KERNEL | __GFP_ACCOUNT);
    if (id < 0) {
        if (next_xsmem_id != 0) {
            id = idr_alloc(&xsmem_idr, xp, 0, next_xsmem_id, GFP_KERNEL | __GFP_ACCOUNT);
        }

        if (id < 0) {
            pr_err("idr alloc failed\n");
            return id;
        }
    }

    next_xsmem_id = (int)((u32)(id + 1) & (XSMEM_MAX_ID - 1));
    *id_in = id;
    return 0;
}

static void xsmem_idr_delete_id(int id)
{
    (void)idr_remove(&xsmem_idr, (unsigned long)id);
}

static struct xsm_pool *xsmem_idr_get_xp_by_id(int id)
{
    return idr_find(&xsmem_idr, (u32)id);
}

static int xsmem_idr_id_for_each(int (*fn)(int id, void *p, void *data), void *data)
{
    return idr_for_each(&xsmem_idr, fn, data);
}

struct xsm_id_to_xp xsm_idr_swith = {
    .alloc = xsmem_idr_alloc_id,
    .del = xsmem_idr_delete_id,
    .get = xsmem_idr_get_xp_by_id,
    .loop = xsmem_idr_id_for_each
};
