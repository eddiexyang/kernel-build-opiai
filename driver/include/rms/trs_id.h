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
* Create: 2022-10-15
*/
#ifndef TRS_ID_H
#define TRS_ID_H

#include <linux/types.h>
#include <linux/module.h>

#include "trs_pub_def.h"

#ifdef __cplusplus
extern "C" {
#endif

struct trs_id_attr {
    u32 id_start;
    u32 id_end;
    u32 id_num;
    u32 batch_num;
    u32 split;      // id larger than spli will be added to tail, otherwise add to head
};

typedef int (*trs_id_alloc_batch)(struct trs_id_inst *inst, int type, u32 id[], u32 id_num, u32 *real_id_num);
typedef int (*trs_id_free_batch)(struct trs_id_inst *inst, int type, u32 id[], u32 id_num);
typedef int (*trs_id_trans)(struct trs_id_inst *inst, int type, u32 id, u32 *phy_id);
typedef int (*trs_id_res_avail_query)(struct trs_id_inst *inst, int type, u32 *num);
typedef bool (*trs_id_is_non_cache_type)(int type);

struct trs_id_ops {
    struct module *owner;
    trs_id_alloc_batch alloc_batch;
    trs_id_free_batch free_batch;
    trs_id_res_avail_query avail_query;
    trs_id_trans trans;
    trs_id_is_non_cache_type is_non_cache_type;
};

struct trs_id_stat {
    u32 alloc;
    u32 allocatable;
};

/* Stream Id, Event id, etc., initialized by adapt */
int trs_id_register(struct trs_id_inst *inst, int type, struct trs_id_attr *attr, struct trs_id_ops *ops);
int trs_id_unregister(struct trs_id_inst *inst, int type);

int trs_id_get_total_num(struct trs_id_inst *inst, int type, u32 *total_num);
int trs_id_get_avail_num(struct trs_id_inst *inst, int type, u32 *avail_num);
int trs_id_get_avail_num_in_pool(struct trs_id_inst *inst, int type, u32 *avail_num);
int trs_id_get_used_num(struct trs_id_inst *inst, int type, u32 *used_num);
int trs_id_get_split(struct trs_id_inst *inst, int type, u32 *split);
int trs_id_get_stat(struct trs_id_inst *inst, int type, struct trs_id_stat *stat);

int trs_id_get_max_id(struct trs_id_inst *inst, int type, u32 *max_id);

int trs_id_alloc(struct trs_id_inst *inst, int type, u32 *id);
int trs_id_free(struct trs_id_inst *inst, int type, u32 id);
int trs_id_free_batch_specific(struct trs_id_inst *inst, int type);
int trs_id_flush_to_pool(struct trs_id_inst *inst);
int trs_id_to_string(struct trs_id_inst *inst, int type, u32 id, char *msg, u32 msg_len);
int trs_id_get_range(struct trs_id_inst *inst, int type, u32 *start, u32 *end);

#ifdef __cplusplus
}
#endif

#endif /* TRS_ID_H */
