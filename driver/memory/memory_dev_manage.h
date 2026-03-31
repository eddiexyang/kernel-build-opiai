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
* Create: 2023-05-25
*/

#ifndef MEMORY_DEV_MANAGE_H
#define MEMORY_DEV_MANAGE_H

#if defined(CFG_FEATURE_LP_ENABLE)
int32_t memory_get_ddr_device_info(uint32_t dev_id, uint32_t sub_cmd, char *in, uint32_t in_len, uint32_t *out_len);
#endif

#endif
