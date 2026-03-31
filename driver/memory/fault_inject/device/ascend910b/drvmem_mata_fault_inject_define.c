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
* Description: mata fault inject declaration
* Author: Huawei Technologies Co.Ltd
* Create: 2023-11-1
*/

#include <linux/slab.h>
#include "drvmem_mata_fault_inject_define.h"
#include "drvmem_base_info.h"
#include "drvmem_fault_inject_common.h"
#include "memory_log.h"

/*
* @brief mata correctable error inject
* @param [in] base: base address
* @return 0: success, else: fail
*/
int32_t drvmem_mata_ce_inject(uint64_t base)
{
	size_t length;
	struct fault_inject_process proc[] = {
		{MATA_BASE_ADDR(0) + MATA_RAS_PFGCTL_H, 0x208} /* error address, error value */
	};
	(void)base;

	memory_drv_info("************** hbma ce fault inject *************\n");
	length = sizeof(proc) / sizeof(proc[0]);

	return drvmem_fault_inject_write_reg(base, length, proc);
}

/*
* @brief uncorrected error inject
* @param [in] base: base address
* @return 0: success, else: fail
*/
int32_t drvmem_mata_ue_inject(uint64_t base)
{
	size_t length;
	struct fault_inject_process proc[] = {
		{MATA_BASE_ADDR(1) + MATA_ERR_STATUSL, 0x702}, /* error address, error value */
		{MATA_BASE_ADDR(1) + MATA_RAS_PFGCTL_L, 0x80000008}  /* error address, error value */
	};
	(void)base;

	memory_drv_info("************** hbma ue fault inject *************\n");
	length = sizeof(proc) / sizeof(proc[0]);

	return drvmem_fault_inject_write_reg(base, length, proc);
}

/*
* @brief mata cfg error inject
* @param [in] base: base address
* @return 0: success, else: fail
*/
int32_t drvmem_mata_cfg_err_inject(uint64_t base)
{
	size_t length;
	struct fault_inject_process proc[] = {
		{MATA_BASE_ADDR(2) + MATA_ERR_STATUSL, 0xc0e},
		{MATA_BASE_ADDR(2) + MATA_RAS_PFGCTL_L, 0x80000010}
	};
	(void)base;

	memory_drv_info("************* hbma cfg fault inject *************\n");
	length = sizeof(proc) / sizeof(proc[0]);

	return drvmem_fault_inject_write_reg(base, length, proc);
}

/*
* @brief mata mirror error inject
* @param [in] base: base address
* @return 0: success, else: fail
*/
int32_t drvmem_mata_mirror_err_inject(uint64_t base)
{
	size_t length;
	struct fault_inject_process proc[] = {
		{MATA_BASE_ADDR(3) + MATA_ERR_STATUSL, 0x170c},
		{MATA_BASE_ADDR(3) + MATA_RAS_PFGCTL_L, 0x80000040}
	};
	(void)base;

	memory_drv_info("************* hbma mir fault inject *************\n");
	length = sizeof(proc) / sizeof(proc[0]);

	return drvmem_fault_inject_write_reg(base, length, proc);
}