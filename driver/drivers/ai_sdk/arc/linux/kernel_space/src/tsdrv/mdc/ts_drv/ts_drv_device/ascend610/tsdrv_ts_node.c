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
#ifdef CFG_SOC_MDC_V51_LITE_FPGA
#include <linux/io.h>
#endif
#include "tsdrv_log.h"
#include "tsdrv_ts_node.h"

#ifdef CFG_SOC_PLATFORM_MDC_V51

#ifdef CFG_SOC_MDC_V51_LITE_FPGA
#define SUPPORT_TS_PLAT_REG_VALUE_V1   0x1001
#define SUPPORT_TS_PLAT_REG_VALUE_V5   0x1005
#define SYSCTL_REG_BASE_ADDR        0x80000000U
#define DEVDRV_PLAT_TYPE_REG_OFFSET 0xFFFC
STATIC int devdrv_check_plat_type_support_ts(void)
{
    void __iomem *reg_addr = NULL;
    int reg_value;

    reg_addr = ioremap(SYSCTL_REG_BASE_ADDR + DEVDRV_PLAT_TYPE_REG_OFFSET, sizeof(int));
    if (reg_addr == NULL) {
        TSDRV_PRINT_ERR("ioremap failed.\n");
        return -EINVAL;
    }

    reg_value = readl(reg_addr);
    iounmap(reg_addr);
    if ((reg_value != SUPPORT_TS_PLAT_REG_VALUE_V1) && (reg_value != SUPPORT_TS_PLAT_REG_VALUE_V5)) {
        TSDRV_PRINT_INFO("plat_type do not support ts, (reg_value=%#x).\n", reg_value);
        return -EINVAL;
    }

    TSDRV_PRINT_INFO("plat_type support ts.\n");
    return 0;
}
#endif

STATIC int g_ts_node_num = 0;
int devdrv_init_ts_node_num(void)
{
    int ts_num = 0;
    struct device_node *node = NULL;
    struct device_node *son = NULL;

    node = of_find_compatible_node(NULL, NULL, "hisi,mini-devdrv-device");
    if (node == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_INFO("The mini-devdrv-device is not configured in dts.\n");
#endif
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

#ifdef CFG_SOC_MDC_V51_LITE_FPGA
    if (devdrv_check_plat_type_support_ts() != 0) {
        ts_num = 0;
    }
#endif

    g_ts_node_num = ts_num;
    TSDRV_PRINT_INFO("find ts (%d)\n", ts_num);
    return ts_num;
}

int devdrv_get_ts_node_num(void)
{
    return g_ts_node_num;
}

TSDRV_EXPORT_SYMBOL(devdrv_get_ts_node_num);

#endif
