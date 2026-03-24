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
 * Create: 2023-05-06
 */

#ifndef __TSMNG_MEM_H__
#define __TSMNG_MEM_H__

int tsmng_ts_req_mem(void *data, u32 devid);
void tsmng_ts_free_mem(void *data, u32 devid);

#endif  /* __TSMNG_MEM_H__ */
