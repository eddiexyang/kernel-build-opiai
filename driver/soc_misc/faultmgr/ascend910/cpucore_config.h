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

#define SOC_MISC_CPUCORE_NODE_NUM  (16U)
#define SOC_MISC_MAX_DEV_NUM (4U)

static struct dms_node g_soc_misc_cpucore_dms_nodes[SOC_MISC_MAX_DEV_NUM][SOC_MISC_CPUCORE_NODE_NUM] = {
    {
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(0, 0, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(0, 1, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(0, 2, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(0, 3, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(0, 4, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(0, 5, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(0, 6, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(0, 7, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(0, 8, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(0, 9, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(0, 10, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(0, 11, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(0, 12, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(0, 13, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(0, 14, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(0, 15, &g_soc_misc_cpucore_ops)
    }, {
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(1, 0, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(1, 1, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(1, 2, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(1, 3, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(1, 4, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(1, 5, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(1, 6, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(1, 7, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(1, 8, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(1, 9, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(1, 10, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(1, 11, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(1, 12, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(1, 13, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(1, 14, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(1, 15, &g_soc_misc_cpucore_ops)
    }, {
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(2, 0, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(2, 1, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(2, 2, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(2, 3, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(2, 4, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(2, 5, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(2, 6, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(2, 7, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(2, 8, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(2, 9, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(2, 10, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(2, 11, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(2, 12, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(2, 13, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(2, 14, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(2, 15, &g_soc_misc_cpucore_ops)
    }, {
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(3, 0, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(3, 1, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(3, 2, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(3, 3, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(3, 4, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(3, 5, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(3, 6, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(3, 7, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(3, 8, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(3, 9, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(3, 10, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(3, 11, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(3, 12, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(3, 13, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(3, 14, &g_soc_misc_cpucore_ops),
    SOC_MISC_CPUCORE_DMS_NODE_DEFINE(3, 15, &g_soc_misc_cpucore_ops)
    }
};

#endif /* CPUCORE_CONFIG_H */