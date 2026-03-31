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

#ifndef BBOX_AGENT_OS_H
#define BBOX_AGENT_OS_H

#include "config/bbox_config.h"
#include "common/bbox_common.h"

struct module_core_map {
    const char *name;
    u8 coreid;
};

#define EXCEPID_AP_PANIC   0xA8460001U
#define RDR_PRODUCT_RELATION_LEN 16


const char *bbox_agent_os_get_model_name(u32 excepid);
const char *bbox_agent_os_get_exception_core(u8 coreid);
u8 bbox_agent_os_get_exception_coreid(const char *name);
s32 bbox_agent_get_areainfo(u8 coreid, u64 *addr, u32 *len);
s32 bbox_agent_os_init(void);
void bbox_agent_os_exit(void);
s32 bbox_agent_os_panic_notify(struct notifier_block *nb, bbox_event_t event, bbox_arg_t *args);

#endif
