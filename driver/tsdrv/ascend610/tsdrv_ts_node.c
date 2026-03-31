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
* Create: 2022-08-16
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/of.h>
#include <linux/fs.h>

#include "tsdrv_log.h"

#ifdef CFG_SOC_PLATFORM_MDC_V51
STATIC int g_ts_node_num = 0;
int devdrv_init_ts_node_num(void)
{
    int ts_num = 0;
    struct device_node *node = NULL;
    struct device_node *son = NULL;

    node = of_find_compatible_node(NULL, NULL, "hisi,mini-devdrv-device");
    if (node == NULL) {
        TSDRV_PRINT_INFO("fail to find mini-devdrv-device\n");
        return 0;
    }

    son = of_find_node_by_name(node, "ts-0");
    if (son != NULL) {
        TSDRV_PRINT_INFO("sucess to find ts-0\n");
        ts_num++;
    }

    son = of_find_node_by_name(node, "ts-1");
    if (son != NULL) {
        TSDRV_PRINT_INFO("sucess to find ts-1\n");
        ts_num++;
    }
    g_ts_node_num = ts_num;
    TSDRV_PRINT_INFO("find ts (%d)\n", ts_num);
    return ts_num;
}

int devdrv_get_ts_node_num(void)
{
    return g_ts_node_num;
}

EXPORT_SYMBOL(devdrv_get_ts_node_num);

#endif
