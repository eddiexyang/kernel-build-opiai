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

#ifndef CDQM_PROC_H
#define CDQM_PROC_H

#include <linux/types.h>
#include "cdqm_core.h"

struct cdqm_proc_ctx {
    int pid;
    u32 devid;
    u32 tsnum;

    struct hlist_node link; /* hash task link */
};

struct cdqm_proc_ctx *cdqm_proc_ctx_create(u32 devid);
void cdqm_proc_ctx_destroy(struct cdqm_proc_ctx *proc_ctx);

struct cdqm_proc_ctx *cdqm_proc_ctx_find(struct cdqm_dev_manage *cdq_dev, int pid);
void cdqm_proc_ctx_add(struct cdqm_dev_manage *cdq_dev, struct cdqm_proc_ctx *proc_ctx);
void cdqm_proc_ctx_del(struct cdqm_proc_ctx *proc_ctx);

#endif

