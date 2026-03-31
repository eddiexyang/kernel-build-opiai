/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2023. All rights reserved.
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

#ifndef _CAN_DRV_DMS_INIT_H__
#define _CAN_DRV_DMS_INIT_H__

#include "dfm_report.h"

#define CAN_CONVERAGE_NUM 4

int mttcan_register_dms_node(u32 dev_id);
struct dfm_struct *can_get_drv_dms(u32 dev_id);
void can_unregister_dms(u32 devid);
#endif