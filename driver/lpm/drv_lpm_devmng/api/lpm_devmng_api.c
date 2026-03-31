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

#include <linux/of.h>
#include <linux/io.h>
#include <linux/export.h>
#include "uda.h"
#include "lpm_devmng_aic_freq.h"
#include "lpm_devmng_api.h"


#ifdef LPM_AIC_FREQ_SYSCNT
/*
 * return value:
 * = 0 ：no valid data or no new data
 * > 0 ： the length of filled buff data.
 * < 0 ： profiling data failed
 */
int32_t lpm_devmng_prof_sample_syscnt_handle(struct prof_peri_para para)
{
	int32_t ret;
	uint32_t result_len = 0;
	struct uda_mia_dev_para mia_para = {0};

	if (uda_is_phy_dev(para.device_id)) {
		mia_para.phy_devid = para.device_id;
		mia_para.sub_devid = 0;
	} else {
		ret = uda_udevid_to_mia_devid(para.device_id, &mia_para);
		if (ret != 0) {
			lpm_log_err("lpm devmng prof sample syscnt: change udevid to physical devid failed, "
				"ret=%d, device_id=%u\n", ret, para.device_id);
			return ret;
		}
	}

	ret = lpm_aic_freq_get_syscnt_info(mia_para.phy_devid, para.buff, para.buff_len, &result_len);
	if (ret != 0) {
		lpm_log_err("lpm devmng prof sample syscnt handle failed, "
			"ret=%d, para.device_id=%u, mia_para.phy_devid=%u, mia_para.sub_devid=%u\n",
			ret, para.device_id, mia_para.phy_devid, mia_para.sub_devid);
		return ret;
	}

	return result_len;
}

EXPORT_SYMBOL(lpm_devmng_prof_sample_syscnt_handle);

#endif