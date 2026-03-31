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
* Create: 2022-7-15
*/
#ifndef TRS_CHAN_MEM_H
#define TRS_CHAN_MEM_H

#include <linux/types.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/gfp.h>
#include <linux/dma-mapping.h>
#include <linux/numa.h>

#include "trs_pub_def.h"

enum trs_chan_mem_type {
    TRS_CHAN_MEM_RSV = 0,
    TRS_CHAN_MEM_DDR,
    TRS_CHAN_MEM_DMA,
    TRS_CHAN_MEM_DDR_RSV,
    TRS_CHAN_MEM_RSV_DDR,
    TRS_CHAN_MEM_MAX
};

void *trs_chan_mem_alloc_ddr(struct trs_id_inst *inst, int nid, size_t size, phys_addr_t *paddr);
void trs_chan_mem_free_ddr(struct trs_id_inst *inst, void *vaddr, size_t size);
void *trs_chan_mem_alloc_rsv(struct trs_id_inst *inst, int type,
    size_t size, phys_addr_t *paddr, u32 flag);

void trs_chan_mem_free_rsv(struct trs_id_inst *inst, int type, void *sq_addr, size_t size);
int trs_chan_rsv_mem_init(struct trs_id_inst *inst, int type, bool cacheable);
void trs_chan_rsv_mem_uninit(struct trs_id_inst *inst, int type);

#endif /* TRS_CHAN_MEM_H */
