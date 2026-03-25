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
#ifndef TRS_STARS_H
#define TRS_STARS_H

#include <linux/types.h>

#include "trs_pub_def.h"
#include "trs_stars_reg_def.h"

#ifdef __cplusplus
extern "C" {
#endif

struct trs_stars_cqint {
    void __iomem *base;
    size_t size;

    u32 group;
    u32 cq_num;
    u32 cq_grp_num;
};

enum {
    TRS_STARS_SCHED = 0,
    TRS_STARS_CQINT,
    TRS_STARS_MAX,
};

typedef void (* trs_stars_set_cq_l1_mask_t)(struct trs_stars_cqint *cqint, int val);
typedef int (* trs_stars_get_valid_cq_list_t)(struct trs_stars_cqint *cqint, u32 cqid[], u32 num, u32 *valid_num);

struct trs_stars_attr {
    phys_addr_t paddr;
    size_t size;
    u32 stride;
    u32 cq_num;
    u32 cq_grp_num;

    trs_stars_set_cq_l1_mask_t set_cq_l1_mask;
    trs_stars_get_valid_cq_list_t get_valid_cq_list;
};

int trs_stars_set_sq_head(struct trs_id_inst *inst, u32 sqid, u32 head);
int trs_stars_sq_enable(struct trs_id_inst *inst, u32 sqid);
int trs_stars_get_sq_head(struct trs_id_inst *inst, u32 sqid, u32 *head);
int trs_stars_init(struct trs_id_inst *inst, int type, struct trs_stars_attr *attr);
void trs_stars_uninit(struct trs_id_inst *inst, int type);
int trs_stars_get_valid_cq_list(struct trs_id_inst *inst, u32 group, u32 cqid[], u32 num, u32 *valid_num);
int trs_stars_set_sq_tail(struct trs_id_inst *inst, u32 sqid, u32 tail);
int trs_stars_get_sq_tail(struct trs_id_inst *inst, u32 sqid, u32 *tail);

int trs_stars_set_cq_head(struct trs_id_inst *inst, u32 cqid, u32 head);
int trs_stars_get_cq_head(struct trs_id_inst *inst, u32 cqid, u32 *head);
int trs_stars_get_cq_tail(struct trs_id_inst *inst, u32 cqid, u32 *tail);

int trs_stars_get_sq_head_paddr(struct trs_id_inst *inst, u32 sqid, u64 *paddr);
int trs_stars_get_sq_tail_paddr(struct trs_id_inst *inst, u32 sqid, u64 *paddr);

int trs_stars_set_sq_status(struct trs_id_inst *inst, u32 sqid, u32 val);
int trs_stars_get_sq_status(struct trs_id_inst *inst, u32 sqid, u32 *val);

int trs_stars_set_cq_l1_mask(struct trs_id_inst *inst, u32 val, u32 group);
int trs_stars_get_cq_affinity_group(struct trs_id_inst *inst, u32 cq_id, u32 *group);

int trs_stars_test_bit(u32 nr, u32 val);
int trs_stars_get_rtsq_paddr(struct trs_id_inst *inst, u32 sqid, phys_addr_t *paddr, size_t *size);

#ifdef __cplusplus
}
#endif

#endif /* TRS_STARS_H */
