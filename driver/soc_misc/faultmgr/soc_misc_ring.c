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

#include "soc_misc_ring.h"
#include "soc_misc_err_info.h"
#include "soc_misc_init.h"
#include "dms_node_type.h"

int soc_misc_ring_ops_init(struct dms_node *device)
{
    soc_misc_drv_debug("DEBUG (node_name=%.*s)\n", DMS_MAX_DEV_NAME_LEN, device->node_name);
    return 0;
}

void soc_misc_ring_ops_uninit(struct dms_node *device)
{
    soc_misc_drv_debug("DEBUG (node_name=%.*s)\n", DMS_MAX_DEV_NAME_LEN, device->node_name);
    return;
}

void soc_misc_ring_fault_handler(const struct notify_data *pdata)
{
    return;
}

int soc_misc_sllc_sensor_scan(unsigned long long private_data, struct dms_sensor_event_data *data)
{
    data->event_count = 0;
    return 0;
}
