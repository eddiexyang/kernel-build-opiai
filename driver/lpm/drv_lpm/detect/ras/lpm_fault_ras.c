/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2024. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/io.h>
#include "lpm_fault_common.h"
#include "lpm_fault_aosub_ras.h"
#include "lpm_fault_lpm_ras.h"
#include "lpm_fault_ras.h"

/* fault ras init */
int32_t lpm_fault_ras_init(uint32_t dev_num)
{
	int32_t ret;

	(void)dev_num;

	/* init aosub ras */
	ret = lpm_fault_ras_aosub_init();
	if (ret != 0) {
		lpm_log_err("lpm init aosub ras failed, ret=%d\n", ret);
		return ret;
	}

	/* init lpm ras */
	ret = lpm_fault_ras_lpm_init();
	if (ret != 0) {
		lpm_log_err("lpm init lpm ras failed, ret=%d\n", ret);
		goto lpm_init_fail;
	}

	lpm_log_info("lpm fault ras init success\n");
	return 0;

lpm_init_fail:
	(void)lpm_fault_ras_aosub_uninit();
	return -1;
}

/* fault ras exit */
int32_t lpm_fault_ras_exit(uint32_t dev_num)
{
	int32_t ret;
	int32_t final_ret = 0;

	(void)dev_num;
	/* uninit aosub ras */
	ret = lpm_fault_ras_aosub_uninit();
	if (ret != 0) {
		final_ret = -1;
		lpm_log_err("lpm uninit aosub ras failed, ret=%d\n", ret);
	}

	/* uninit lpm ras */
	ret = lpm_fault_ras_lpm_uninit();
	if (ret != 0) {
		final_ret = -1;
		lpm_log_err("lpm uninit lpm ras failed, ret=%d\n", ret);
	}

	lpm_log_info("lpm fault ras uninit finish, final_ret=%d\n", final_ret);
	return final_ret;
}