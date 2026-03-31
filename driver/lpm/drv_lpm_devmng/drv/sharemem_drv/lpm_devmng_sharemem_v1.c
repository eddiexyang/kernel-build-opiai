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
*/
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include "lpm_devmng_sharemem.h"
#include "lpm_devmng_sharemem_v1.h"

int32_t lpm_read_aic_freq_adj_from_sharemem(uint32_t dev_id, struct lpm_aic_freq_adj_info *adj_info)
{
	return lpm_sharemem_get_value(dev_id, LPM_SHAREMEM_AIC_FREQ_ADJ_OFFSET,
		(uint8_t *)adj_info, sizeof(struct lpm_aic_freq_adj_info));
}
