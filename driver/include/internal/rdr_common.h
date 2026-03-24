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

#ifndef RDR_COMMON_H
#define RDR_COMMON_H

#include <linux/types.h>

#include "device/bbox_types.h"
#include "common/bbox_common.h"

struct reboot_reason_map {
    const char *name;
    u8 num;
};

struct module_core_map {
    const char *name;
    u8 coreid;
};

void *rdr_map(u32 devid, phys_addr_t paddr, size_t size);
void rdr_unmap(const void *vaddr);

const char *rdr_get_model_name(u32 excepid);
u8 rdr_get_etype(const char *reason);
void rdr_set_saving_state(s32 state);
u8 rdr_get_exception_coreid(const char *name);
const char *rdr_get_exception_core(u8 coreid);
s32 rdr_get_suspend_state(void);
s32 rdr_common_init(void);
void rdr_common_exit(void);

#endif
