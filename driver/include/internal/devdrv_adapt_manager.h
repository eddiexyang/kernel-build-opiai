/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
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
* Create: 2019-10-15
*/

#ifndef __DEVDRV_ADAPT_MANAGER_LOAD_H
#define __DEVDRV_ADAPT_MANAGER_LOAD_H
#include <linux/of_address.h>

extern struct devdrv_manager_info *dev_manager_info;

void devdrv_free_tsfw_mem(struct device *dev, int size, void *addr, dma_addr_t ts_dma_handle);
unsigned long devdrv_manager_get_wait_time(struct devdrv_info *dev_info);

#endif /* __DEVDRV_ADAPT_MANAGER_LOAD_H */
