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
#ifndef TRS_RCV_MEM_H
#define TRS_RCV_MEM_H

#include <linux/types.h>

#include "trs_pub_def.h"
#define TRS_RSV_MEM_OP_ZERO     (1 << 0)
#ifdef __cplusplus
extern "C" {
#endif
#define TRS_RSV_MEM_FLAG_DEVICE (1 << 0)
struct trs_rsv_mem_attr {
    void __iomem *vaddr;
    phys_addr_t paddr;
    size_t total_size;
    u32 flag;
};

enum {
    RSV_MEM_HW_SQCQ = 0,
    RSV_MEM_MAINT_SQCQ,
    RSV_MEM_MAX
};

void *trs_rsv_mem_alloc(struct trs_id_inst *inst, int type, size_t size, u32 flag);
void trs_rsv_mem_free(struct trs_id_inst *inst, int type, void *vaddr, size_t size);
int trs_rsv_mem_v2p(struct trs_id_inst *inst, int type,
    void *vaddr, phys_addr_t *phy_addr);
int trs_rsv_mem_init(struct trs_id_inst *inst, int type,
    struct trs_rsv_mem_attr *attr);
void trs_rsv_mem_uninit(struct trs_id_inst *inst, int type);

bool trs_rsv_mem_is_in_range(struct trs_id_inst *inst, int type, void *vaddr);
int trs_rsv_mem_get_meminfo(struct trs_id_inst *inst, int type, size_t *alloc_size, size_t *total_size);

#ifdef __cplusplus
}
#endif

#endif /* TRS_RCV_MEM_H */
