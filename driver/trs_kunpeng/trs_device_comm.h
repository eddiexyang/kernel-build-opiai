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
* Create: 2023-8-3
*/

#include <linux/types.h>

#define ALL_FEATURE_MODE    0
#define PART_FEATURE_MODE   1

void trs_invalid_cache(u64 base, u32 len);
void trs_flush_cache(u64 base, u32 len);

void trs_set_feature_mode(void);
u32 trs_get_feature_mode(void);

