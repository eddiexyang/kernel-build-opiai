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

#include <linux/delay.h>
#include <asm/uaccess.h>
#include <linux/time.h>
#include "devdrv_manager.h"
#include "devdrv_manager_common.h"
#include "dms_define.h"
#include "memory_log.h"
#include "memory_ecc_cfg.h"
#include "memory_ddr_cfg.h"
#include "memory_ddr.h"
#include "memory_scan.h"

#ifdef CFG_MEMORY_ECC_INFO_FROM_LP
int get_ecc_statistics_from_lp(struct memory_info *memory_arg)
{
	int ret;
	struct ioctl_arg user_arg = {0};

	user_arg.dev_id = memory_arg->dev_id;
	user_arg.type = memory_arg->type;
	user_arg.data3 = ECC_ERROR_TYPE;

	ret = devdrv_get_ecc_statistics((unsigned long)&user_arg);
	if (ret) {
		memory_drv_err("Get ecc statistics failed. (ret=%d)\n", ret);
		return ret;
	}

	memory_arg->ecc_sigle_bit = user_arg.data1;
	memory_arg->ecc_double_bit = user_arg.data3;

	memory_drv_debug("Get ecc statistic form lp success. (dev_id=%u; type=%u; sigle_bit=%u; double_bit=%u)\n",
		memory_arg->dev_id, memory_arg->type, memory_arg->ecc_sigle_bit, memory_arg->ecc_double_bit);
	return 0;
}
#endif

#ifdef CFG_MEMORY_DDR_INFO_FROM_SHAREMEM
int get_ddr_ecc_statistics_from_sharemem(struct memory_info *memory_arg)
{
	int ret;
	unsigned int ecc_bit = 0;

	if (memory_arg == NULL) {
		memory_drv_err("Invalid parameter, memory_arg is NULL.\n");
		return -EINVAL;
	}

	ret = memory_get_ddr_info_from_sharemem(ECC_SINGLE_BIT_OFFSET, &ecc_bit, sizeof(unsigned int));
	if (ret != 0) {
		memory_drv_err("Get ddr ecc single bit failed. (dev_id=%d, ret=%d)\n", memory_arg->dev_id, ret);
		return ret;
	}
	memory_arg->ecc_sigle_bit = ecc_bit;

	ret = memory_get_ddr_info_from_sharemem(ECC_DOBIT_OFFSET, &ecc_bit, sizeof(unsigned int));
	if (ret != 0) {
		memory_drv_err("Get ddr ecc Dobit failed. (dev_id=%d, ret=%d)\n", memory_arg->dev_id, ret);
		return ret;
	}
	memory_arg->ecc_double_bit = ecc_bit;

	return 0;
}
#endif

STATIC int32_t query_ecc_statics_info(struct memory_scan_list *item, struct memory_info *user_arg, uint32_t device_id)
{
	int32_t ret = 0;

	if (item->scan_flag == 0) {
		ret = GET_ECC_STATISTISCS(user_arg);
		if (ret != 0) {
			memory_drv_err("Get ddr ecc statistics failed.(dev_id=%u; ret=%d)\n", ret, device_id);
			return ret;
		}
	} else {
		*user_arg =
			(user_arg->type == MEMORY_DDR) ? item->value[device_id].ddr_mem_info : item->value[device_id].hbm_mem_info;
	}

	return ret;
}

STATIC int32_t para_valid_check(char **in, uint32_t in_len, struct memory_info *user_arg, uint32_t *device_id)
{
	int32_t ret = 0;

	ret = memcpy_s((void *)user_arg, sizeof(struct memory_info), (void *)(*in), in_len);
	if (ret != 0) {
		memory_drv_err("Memcpy_s failed. (ret=%d)\n", ret);
		return ret;
	}

	*device_id = user_arg->dev_id;
	if (dms_is_devid_valid(*device_id) == false) {
		memory_drv_err("Invalid parameter. (dev_id=%u)\n", *device_id);
		return -EINVAL;
	}

	return ret;
}

int memory_get_ecc_statistics(void *feature, char *in, unsigned int in_len, char *out, unsigned int out_len)
{
	int ret;
	unsigned int device_id;
	struct memory_info user_arg = { 0 };
	struct memory_scan_list *item = NULL;

	if ((in == NULL) || (in_len != sizeof(struct memory_info)) || (out == NULL) || (out_len != in_len)) {
		memory_drv_err("Invalid parameter. (in=%pK; in_len=%u; out=%pK; out_len=%u)\n", in, in_len, out, out_len);
		return -EINVAL;
	}

	ret = para_valid_check(&in, in_len, &user_arg, &device_id);
	if (ret != 0) {
		memory_drv_err("para_valid_check failed. (ret=%d)\n", ret);
		return ret;
	}

	item = find_operate_info_str(MEMORY_ECC_STATIC, user_arg.type);
	if (item == NULL) {
		memory_drv_err("Get ecc static handle struct failed. (dev_id=%u)\n", device_id);
		return -EINVAL;
	}

	ret = query_ecc_statics_info(item, &user_arg, device_id);
	if (ret != 0) {
		memory_drv_err("query_ecc_statics_info failed.(dev_id=%u; ret=%d)\n", ret, device_id);
		return ret;
	}

	ret = memcpy_s((void *)out, out_len, (void *)&user_arg, sizeof(struct memory_info));
	if (ret) {
		memory_drv_debug("Memcpy_s failed. (ret=%d)\n", ret);
		return ret;
	}
	return 0;
}
