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
*/

#include <linux/seq_file.h>
#include <linux/securec.h>
#include <linux/uaccess.h>
#include "lpm_devmng_sharemem_drv.h"
#include "lpm_devmng_power.h"

int32_t lpm_power_get_info(uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len)
{
	unsigned long cpy_len;
	int32_t ret;
	struct lpm_power_info power_info = {0};

	ret = lpm_check_dsmi_get_param(dev_id, in, in_len, sizeof(struct lpm_power_info), out_len);
	if (ret != 0) {
		lpm_log_err("Get power info, check param failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
		return ret;
	}

	ret = lpm_get_power_from_sharemem(dev_id, LPM_DEVMNG_POWER_LIMIT, &power_info.soc_rated_power);
	if (ret != 0) {
		lpm_log_err("Get soc rated power from share memory failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
		return ret;
	}

	cpy_len = copy_to_user(in, (void *)&power_info, sizeof(struct lpm_power_info));
	if (cpy_len != 0) {
		lpm_log_err("Get power info, copy to user failed. (dev_id=%u; cpy_len=%lu)\n", dev_id, cpy_len);
		return -EIO;
	}

	*out_len = (uint32_t)sizeof(struct lpm_power_info);
	return 0;
}
