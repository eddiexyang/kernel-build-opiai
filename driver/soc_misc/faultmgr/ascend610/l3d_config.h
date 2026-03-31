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

#ifndef L3D_CONFIG_H
#define L3D_CONFIG_H

#ifndef CFG_SOC_PLATFORM_MDC_V51
#  error "This header can only include in ascend610 or bs9sx1a"
#endif

#include "soc_misc_dms_def.h"
#include "l3d_dms_init.h"

/* node numbers */
#define L3D_NODE_NUM (4U)

/* l3d reg base */
#define CPU_CLUSTER0_L3D_REG_BASE (0x81130000U)
#define CPU_CLUSTER1_L3D_REG_BASE (0x81140000U)
#define CPU_CLUSTER2_L3D_REG_BASE (0x81150000U)
#define CPU_CLUSTER3_L3D_REG_BASE (0x81160000U)

/* dms sensor */
#define L3D_DMS_SENSORS { \
    SOC_MISC_DMS_SENSOR(SOC_MISC_NODE_0, 0, DMS_SEN_TYPE_RAS_SENSOR, "L3D", l3d_sensor_scan),    \
    SOC_MISC_DMS_SENSOR(SOC_MISC_NODE_0, 0, DMS_SEN_TYPE_SAFETY_SENSOR, "L3D", l3d_sensor_scan), \
    SOC_MISC_DMS_SENSOR(SOC_MISC_NODE_0, 0, DMS_SEN_TYPE_CHECK_SENSOR, "L3D", l3d_sensor_scan),  \
}

#endif /* L3D_CONFIG_H */
