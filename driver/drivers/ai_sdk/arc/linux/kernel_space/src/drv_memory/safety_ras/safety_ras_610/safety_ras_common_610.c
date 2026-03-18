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
 * Create: 2023-10-23
 */

#include "safety_ras_common_610.h"
#include <linux/of.h>
#ifndef AOS_LLVM_BUILD
#include "devdrv_manager_comm.h"
#endif
#include "memory_log.h"

#ifndef AOS_LLVM_BUILD
static uint32_t get_safety_ras_supported_by_chip(void)
{
	int ret;
	int32_t chip_type = 0;
	ret = devdrv_manager_get_chip_type(&chip_type);
	if (ret != 0) {
		memory_drv_err("get chip_type failed, ret(%d).\n", ret);
		return SAFETY_RAS_NOT_SUPPORTED;
	}
	/* ASCEND_V1 do not support safety ras */
	if (chip_type == CHIP_TYPE_ASCEND_V1) {
		memory_drv_info("not support safety ras feature by chip type!\n");
		return SAFETY_RAS_NOT_SUPPORTED;
	}
	return SAFETY_RAS_SUPPORTED;
}
#endif
/*
* @brief if need register safety or ras interrupt
* @return 0: need, else: not need
*/
uint32_t safety_ras_supported(void)
{
	uint32_t ret = SAFETY_RAS_SUPPORTED;
	struct device_node *np = NULL;
	np = of_find_node_by_name(NULL, "davici_devfault_ddr_subctrl");
	if (np == NULL) {
		memory_drv_info("no ddr subctrl node in dts\n");
		return SAFETY_RAS_NOT_SUPPORTED;
	}
#ifndef AOS_LLVM_BUILD
	ret = get_safety_ras_supported_by_chip();
#endif
	return ret;
}