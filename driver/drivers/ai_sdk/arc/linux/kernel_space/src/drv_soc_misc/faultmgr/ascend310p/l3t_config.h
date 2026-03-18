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

#ifndef L3T_CONFIG_H
#define L3T_CONFIG_H

#include "soc_misc_dms_def.h"
#include "l3t_dms_init.h"

/* node numbers */
#define L3T_NODE_NUM (4U)

/* dms sensor */
#define L3T_DMS_SENSORS {  \
    SOC_MISC_DMS_SENSOR(SOC_MISC_NODE_0, 0, DMS_SEN_TYPE_RAS_SENSOR, "L3T", l3t_sensor_scan), \
}

#endif /* L3T_CONFIG_H */
