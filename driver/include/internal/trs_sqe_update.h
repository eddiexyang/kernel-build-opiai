/*
* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
* Create: 2023-8-5
*/
#ifndef TRS_SQE_UPDATE_H
#define TRS_SQE_UPDATE_H

#include "drv_type.h"

struct trs_dma_desc_addr_info {
    u64 src_va; /* dev_va */
    u32 passid; /* use for find src addr */

    u32 sqid;   /* use for find dst addr */
    u32 sqeid;  /* use for find dst addr */
    u16 offset; /* use for find dst addr */

    u16 size;
    u32 pid;
};

struct trs_dma_desc {
    void *sq_addr;
    u32 sq_tail;    /* means the cnt of sq_addr */
};

/* sqid sqeid --key */
int hal_kernel_sqe_update_desc_create(u32 devid, u32 tsid, struct trs_dma_desc_addr_info *addr_info,
    struct trs_dma_desc *dma_desc);
void hal_kernel_sqe_update_desc_destroy(u32 devid, u32 tsid, u32 sqid);

int hal_kernel_trs_get_ssid(u32 devid, u32 tsid, int pid, u32 *passid);

void trs_sqe_update_init(u32 devid);

#endif

