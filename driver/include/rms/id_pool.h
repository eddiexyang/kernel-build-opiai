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
#ifndef ID_POOL_H
#define ID_POOL_H

#include <linux/types.h>
#include "trs_pub_def.h"

#ifdef __cplusplus
extern "C" {
#endif

struct id_pool_inst {
    u32 phy_devid;
    u32 sub_id;
};

struct id_pool_attr {
    u32 node_tunable;
    u32 id_start;
    u32 id_total_num;
};

int id_pool_get_total_num(struct id_pool_inst *inst, int type, u32 *id_total_num);

int id_pool_alloc(struct id_pool_inst *inst, int type, u32 *id);
void id_pool_free(struct id_pool_inst *inst, int type, u32 id);

int id_pool_alloc_by_range(struct id_pool_inst *inst, int type, u32 start, u32 end, u32 *id);

int id_pool_register(struct id_pool_inst *inst, int type, struct id_pool_attr *attr);
void id_pool_unregister(struct id_pool_inst *inst, int type, struct id_pool_attr *attr);

int id_pool_get_range(struct id_pool_inst *inst, int type, u32 *start, u32 *end);
int id_pool_get_total_num_by_range(struct id_pool_inst *inst, int type, u32 start, u32 end, u32 *total_num);
int id_pool_get_tunable_range_by_range(struct id_pool_inst *inst, int type, u32 *start, u32 *end);
int id_pool_get_avail_num(struct id_pool_inst *inst, int type, u32 *avail_num);

static inline void id_pool_inst_pack(struct id_pool_inst *inst, u32 devid, u32 sub_id)
{
    inst->phy_devid = devid;
    inst->sub_id = sub_id;
}

static inline void id_pool_attr_pack(struct id_pool_attr *attr, bool tunable, u32 id_start, u32 id_total_num)
{
    attr->node_tunable = tunable;
    attr->id_start = id_start;
    attr->id_total_num = id_total_num;
}

#ifdef __cplusplus
}
#endif

#endif /* ID_POOL_H */
