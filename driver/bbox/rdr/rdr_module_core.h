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

#ifndef RDR_MODULE_CORE_H
#define RDR_MODULE_CORE_H

#include <linux/list.h>
#include <linux/types.h>
#include "device/bbox_pub.h"
#include "common/bbox_common.h"

#define RDR_MODULE_EXCEPTION_NUM_MAX     500

struct rdr_module_ops_s {
    struct list_head s_list;
    struct bbox_module_info s_ops;
    u32 reference;
    s32 pause;
};

u8 rdr_check_curr_time(const excep_time *tm);
u8 rdr_check_curr_coreid(u8 coreid);
u8 rdr_check_curr_excepid(u32 excepid);

void bbox_dump_done(const struct bbox_dump_done_ops_info *info);
s32 rdr_module_ddr_dump(const struct bbox_dump_done_ops_info *info, u32 excepid, const char *addr, u32 len);
s32 rdr_module_log_dump(const struct bbox_dump_done_ops_info *info, u32 excepid, const char *addr, u32 len);
s32 rdr_module_get_vaddr(u32 devid, u8 coreid, u64 *vaddr);
void rdr_module_free_vaddr(u64 vaddr);
s32 rdr_module_inc_reference(u8 coreid);
void rdr_module_dec_reference(u8 coreid);

u64 rdr_notify_module_dump(u32 devid, u32 excepid, const struct bbox_time *tm, u32 arg,
                           const struct bbox_exception_info *e_info);
u64 rdr_get_dump_result(u32 excepid);
void rdr_notify_module_reset(u32 devid, u32 excepid, const struct bbox_exception_info *e_info);

s32 rdr_module_init(void);
void rdr_module_exit(void);

#endif

