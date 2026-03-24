
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2023. All rights reserved.
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
 * Create: 2020-4-20
 */

#ifndef MMZ_EXT_H
#define MMZ_EXT_H

#include "hi_type.h"

// mmz对外对外提供接口，由osal层封装调用
hi_s32 mmz_init(hi_void);
hi_s32 mmz_exit(hi_void);
hi_u64 mmz_malloc(const hi_char *mmz_name, const hi_char *buf_name, hi_ulong size);
hi_void mmz_free(hi_u64 phy_addr, hi_void *vir_addr);
hi_s32 mmz_is_valid(const hi_char *mmz_name, hi_u64 phy_addr);

#endif