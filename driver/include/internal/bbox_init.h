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

#ifndef BBOX_INIT_H
#define BBOX_INIT_H

#include "device/bbox_types.h"
#include "bootparam/bbox_boot_param.h"
#include "config/bbox_config.h"

s32 bbox_register_init(void);
void bbox_register_exit(void);

s32 bbox_bootcheck_init(void);
void bbox_bootcheck_exit(void);

s32 bbox_rdr_init(void);
void bbox_rdr_exit(void);

s32 bbox_adapter_init(void);
void bbox_adapter_exit(void);

s32 bbox_communication_init(void);
void bbox_communication_exit(void);

s32 bbox_proxy_init(void);
void bbox_proxy_exit(void);

s32 bbox_xpc_init(void);
void bbox_xpc_exit(void);
#endif

