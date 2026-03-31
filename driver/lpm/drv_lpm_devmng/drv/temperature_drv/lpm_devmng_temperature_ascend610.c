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
#include <linux/securec.h>
#include <linux/uaccess.h>
#include <linux/types.h>
#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
#include "lpm_devmng_debugfs.h"
#endif
#include "lpm_devmng_common.h"
#include "lpm_devmng_ipc.h"
#include "lpm_devmng_temperature_drv.h"
#include "lpm_devmng_temperature_ascend610.h"

#define LPM_DEVMNG_PERCENT_UNIT ((int32_t)100)
#define LPM_DEVMNG_INTER_MEDIATE_DATE ((int32_t)50)

STATIC uint32_t lpm_get_soc_max_temperature(struct lpm_query_temp_ipc *temp_info)
{
	int32_t temp;

	// query result has checked in ipc module, no need check repeat
	// soc_max_temp get from ipc = Celsius * 100
	// Celsius = soc_max_temp get from ipc / 100
	temp = *(int32_t *)&temp_info->soc_max_temp;
	if (temp < 0) {
		temp = (temp - LPM_DEVMNG_INTER_MEDIATE_DATE) / LPM_DEVMNG_PERCENT_UNIT;
	} else {
		temp = (temp + LPM_DEVMNG_INTER_MEDIATE_DATE) / LPM_DEVMNG_PERCENT_UNIT;
	}

	return (uint32_t)temp;
}

STATIC int32_t lpm_get_soc_temperature_from_ipc(uint32_t dev_id, uint32_t *temperature)
{
	int32_t ret;
	struct lpm_query_temp_ipc temp_info = {0};
	struct lpm_devmng_ipc_msg msg_info;

	// no need suppress
	msg_info.send_type = LPM_IPC_QUERY_SOC_TEMPERATURE;
	msg_info.in        = NULL;
	msg_info.in_len    = 0;
	msg_info.out       = (void *)&temp_info;
	msg_info.out_pos   = 0;
	msg_info.out_len   = (uint32_t)sizeof(temp_info);

	// query result has checked in ipc module, no need check repeat
	ret = lpm_ipc_send_sync_msg(dev_id, &msg_info);
	if (ret != 0) {
		lpm_log_err("failed to send soc max temperature ipc, dev_id=%u, ret=%d\n",
			dev_id, ret);
		return ret;
	}

	*temperature = lpm_get_soc_max_temperature(&temp_info);
	return 0;
}

int32_t lpm_temperature_drv_get_soc_temp(
	uint32_t dev_id, enum lpm_devmng_core_id core_id, uint32_t *temperature)
{
	int32_t ret;

	ret = lpm_get_soc_temperature_from_ipc(dev_id, temperature);
	if (ret != 0) {
		lpm_log_err("dev_id=%u get soc max temperature from ipc failed, ret=%d\n",
			dev_id, ret);
		return ret;
	}

	(void)core_id;
	return 0;
}

int32_t lpm_temperature_drv_probe(uint64_t *param, uint32_t param_num)
{
	(void)param;
	(void)param_num;
	lpm_log_info("lpm temperature drv probe success\n");
	return 0;
}

int32_t lpm_temperature_drv_remove(uint64_t *param, uint32_t param_num)
{
	(void)param;
	(void)param_num;
	lpm_log_info("lpm temperature drv remove success\n");
	return 0;
}

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)

int32_t lpm_temperature_drv_debugfs_show(struct seq_file *seq, void *v)
{
	(void)seq;
	(void)v;
	return 0;
}
#endif
