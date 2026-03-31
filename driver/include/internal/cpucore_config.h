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
* Create: 2023-08-28
*/

#ifndef CPUCORE_CONFIG_H
#define CPUCORE_CONFIG_H

#include "dms_define.h"
#include "soc_misc_cpucore_comcfg.h"

#define SOC_MISC_CPUCORE_NODE_NUM  (4U)
#define SOC_MISC_MAX_DEV_NUM (1U)

struct dms_node g_soc_misc_cpucore_dms_nodes[SOC_MISC_MAX_DEV_NUM][SOC_MISC_CPUCORE_NODE_NUM] = {
    {
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(0, 0, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(0, 1, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(0, 2, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(0, 3, &g_soc_misc_cpucore_ops)
    }
};

#endif /* CPUCORE_CONFIG_H */