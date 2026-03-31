/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
 * Create: 2022-01-15
 */
#ifndef _AGENTDRV_FLR_H_
#define _AGENTDRV_FLR_H_

#include <linux/workqueue.h>

#include "agentdrv_platform.h"

void agentdrv_handle_pcie_flr_mask(struct agentdrv_platform_dev *p_dev);
void agentdrv_handle_pcie_flr_unmask(struct agentdrv_platform_dev *p_dev);
void agentdrv_flr_clear_status(struct agentdrv_platform_dev *p_dev);
void agentdrv_flr_task(struct work_struct *work);

#endif
