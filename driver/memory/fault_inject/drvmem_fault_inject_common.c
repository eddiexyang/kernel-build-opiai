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
* Description: fault inject common functions
* Author: Huawei Technologies Co.Ltd
* Create: 2022-11-22
*/

#include <drvmem_fault_inject_common.h>
#include <linux/types.h>
#include <linux/io.h>
#include <linux/errno.h>
#include <memory_log.h>

/*
* @brief write register
* @param [in] addr: register address
* @param [in] val: register value
* @return 0: success, else: fail
*/
int32_t drvmem_write_reg(uint64_t addr, uint32_t val)
{
	int32_t ret;
	void __iomem *reg = NULL;

	reg = ioremap(addr, sizeof(uint32_t));
	if (IS_ERR_OR_NULL(reg)) {
		ret = PTR_ERR(reg);
		memory_drv_err("ioremap failed, size = %lu,ret=%d\n", sizeof(uint32_t), ret);
		return ret;
	}

	writel(val, reg);
	iounmap(reg);

	return 0;
}

/*
* @brief read register
* @param [in] addr: register address
* @param [out] val: register value read
* @return 0: success, else: fail
*/
int32_t drvmem_read_reg(uint64_t addr, uint32_t *val)
{
	int32_t ret;
	void __iomem *reg = NULL;

	reg = ioremap(addr, sizeof(uint32_t));
	if (IS_ERR_OR_NULL(reg)) {
		ret = PTR_ERR(reg);
		memory_drv_err("ioremap failed, size = %lu,ret=%d\n", sizeof(uint32_t), ret);
		return ret;
	}

	*val = readl(reg);
	iounmap(reg);

	return 0;
}

int32_t drvmem_fault_inject_write_reg(uint64_t base, size_t proc_length, struct fault_inject_process *proc)
{
	size_t i;
	int32_t ret;

	for (i = 0; i < proc_length; i++) {
		ret = drvmem_write_reg(base + proc[i].addr, proc[i].val);
		if (ret != 0) {
			memory_drv_err("write reg failed, addr = 0x%llx, val = %u\n",
				base + proc[i].addr, proc[i].val);
			return ret;
		}
	}

	return 0;
}