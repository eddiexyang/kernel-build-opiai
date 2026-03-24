/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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
 * Create: 2022-08-13
 */

#include "bbox_init.h"
#include <linux/module.h>
#include <linux/init.h>
#include "bbox_platform.h"
#include "device/bbox_types.h"
#include "common/bbox_print.h"


/*
 * @brief       : bbox init
 * @return      : <0 failure; =0 success
 */
STATIC s32 INIT bbox_init(void)
{
    s32 ret;

    BB_PRINT_INFO("bbox init.\n");

    // read the config, from dts or acpi
    ret = bbox_config_init();
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("get config failed with %d.\n", ret);
        return BBOX_FAILURE;
    }

    // read the boot param, from ddr or cmdline
    ret = bbox_boot_param_init();
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("get boot param failed with %d.\n", ret);
        return BBOX_FAILURE;
    }

    // init register, sctrl or pmu
    ret = bbox_register_init();
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("register init failed with %d.\n", ret);
        return BBOX_FAILURE;
    }

    ret = bbox_rdr_init();
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("rdr init failed with %d.\n", ret);
        return BBOX_FAILURE;
    }

    ret = bbox_bootcheck_init();
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("bootcheck init failed with %d.\n", ret);
        return BBOX_FAILURE;
    }

    ret = bbox_communication_init();
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("communication init failed with %d.\n", ret);
        return BBOX_FAILURE;
    }

    ret = bbox_proxy_init();
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("proxy init failed with %d.\n", ret);
    }

    ret = bbox_adapter_init();
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("os adapter failed with %d.\n", ret);
        return BBOX_FAILURE;
    }

#if (defined BBOX_SOC_PLATFORM_MDC) && (!defined BBOX_UT)
    ret = bbox_xpc_init();
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("xpc init failed with %d.\n", ret);
        return BBOX_FAILURE;
    }
#endif

    BB_PRINT_INFO("bbox initialized.\n");
    return BBOX_SUCCESS;
}

/*
 * @brief       : bbox exit function
 * @return      : NA
 */
STATIC EXIT void bbox_exit(void)
{
    BB_PRINT_INFO("bbox exit start.\n");
#if (defined BBOX_SOC_PLATFORM_MDC) && (!defined BBOX_UT)
    bbox_xpc_exit();
#endif
    bbox_adapter_exit();
    bbox_proxy_exit();
    bbox_bootcheck_exit();
    bbox_rdr_exit();
    bbox_communication_exit();
    bbox_register_exit();
    BB_PRINT_INFO("bbox exit end.\n");
}

module_init(bbox_init);
module_exit(bbox_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("bbox");
