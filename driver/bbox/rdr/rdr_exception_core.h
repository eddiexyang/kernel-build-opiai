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

#ifndef RDR_EXCEPTION_CORE_H
#define RDR_EXCEPTION_CORE_H

#include <linux/list.h>
#include <linux/types.h>
#include "device/bbox_pub.h"

#define RDR_EXCEPTION_REENTRANT_NUM_MAX     100

struct rdr_exception_node {
    struct list_head e_list;
    struct bbox_exception_info info;
    u32 reference;
    s32 pause;
};

struct bbox_exception_info *rdr_exception_get_info(u32 excepid);
u8 rdr_exception_get_etype(u32 excepid);
void rdr_exception_print_info(struct bbox_exception_info *e);
void rdr_exception_callback(const struct bbox_exception_info *p_exce_info, u32 excepid);

s32 rdr_exception_inc_reference(u32 excepid);
void rdr_exception_dec_reference(u32 excepid);

s32 rdr_exception_init(void);
void rdr_exception_exit(void);

#endif

