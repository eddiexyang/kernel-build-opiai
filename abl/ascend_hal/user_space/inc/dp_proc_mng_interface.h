/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
 * Create: 2023-10-18
 */

#ifndef DP_PROC_MNG_INTERFACE_H
#define DP_PROC_MNG_INTERFACE_H

#include <stdint.h>

#include "drv_profile.h"

int dp_proc_mng_prof_sample_fun(struct prof_sample_para *para);
void dp_proc_mng_module_used_size_update(uint32_t devid, uint32_t module_id, uint64_t size);

#endif

