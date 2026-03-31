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
#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
#include "lpm_devmng_debugfs.h"
#endif
#include "lpm_devmng_common.h"
#include "lpm_devmng_temperature_drv.h"
#include "lpm_devmng_temperature_sharemem_v2.h"

STATIC struct lpm_temperature_drv_sharemem_priv *lpm_temperature_sharemem_get_priv_info(void)
{
	static struct lpm_temperature_drv_sharemem_priv lpm_temperature_drv_priv = {0};
	return &lpm_temperature_drv_priv;
}

STATIC struct lpm_temperature_drv_query_list *lpm_temperature_sharemem_get_query_list(void)
{
	struct lpm_temperature_drv_sharemem_priv *temp_sharemem_info = lpm_temperature_sharemem_get_priv_info();
	return &temp_sharemem_info->query_list;
}

STATIC void lpm_temperature_drv_init_priv(void)
{
	struct lpm_temperature_drv_sharemem_priv *temp_sharemem_info = lpm_temperature_sharemem_get_priv_info();

	// init query list
	temp_sharemem_info->query_list.support_num = 0;
	temp_sharemem_info->query_list.support_cfg = NULL;
}

STATIC int32_t lpm_temperature_drv_init_query_list(void)
{
	uint32_t support_num;
	struct lpm_temperature_drv_query_list *query_list = lpm_temperature_sharemem_get_query_list();
	static struct lpm_temperature_drv_support_cfg support_cfg[] = {
		{LPM_SOC_ID, LPM_DEVMNG_SOC_MAX_TEMP}
	};

	support_num = (uint32_t)ARRAY_SIZE(support_cfg);
	if (support_num > (uint32_t)LPM_INVALID_ID) {
		lpm_log_err("support_num=%u should not large than %u\n",
			support_num, (uint32_t)LPM_INVALID_ID);
		return -1;
	}

	query_list->support_num = support_num;
	query_list->support_cfg = &support_cfg[0];

	lpm_log_info("register temperature query list success, support_num=%u\n", support_num);
	return 0;
}

STATIC int32_t lpm_temperature_drv_get_type_by_core_id(
	enum lpm_devmng_core_id core_id, enum lpm_devmng_temperature_type *temp_type)
{
	uint32_t i;
	struct lpm_temperature_drv_query_list *query_list = lpm_temperature_sharemem_get_query_list();

	for (i = 0; i < query_list->support_num; i++) {
		if (query_list->support_cfg[i].core_id == core_id) {
			*temp_type = query_list->support_cfg[i].temp_type;
			return 0;
		}
	}
	// can not find temperature type by core id
	return -1;
}

int32_t lpm_temperature_drv_get_soc_temp(
	uint32_t dev_id, enum lpm_devmng_core_id core_id, uint32_t *temperature)
{
	int32_t ret;
	enum lpm_devmng_temperature_type temp_type = LPM_DEVMNG_SHARE_MEM_TEMP_NUM;

	ret = lpm_temperature_drv_get_type_by_core_id(core_id, &temp_type);
	if (ret != 0) {
		lpm_log_err("find temperature type failed, dev_id=%u, core_id=%u, ret=%d\n",
			dev_id, (uint32_t)core_id, ret);
		return ret;
	}

	ret = lpm_get_temperature_from_sharemem(dev_id, temp_type, temperature);
	if (ret != 0) {
		lpm_log_err("get temperature failed, dev_id=%u, core_id=%u, temp_type=%u, ret=%d\n",
			dev_id, (uint32_t)core_id, (uint32_t)temp_type, ret);
		return ret;
	}

	return 0;
}

int32_t lpm_temperature_drv_probe(uint64_t *param, uint32_t param_num)
{
	int32_t ret;

	// should init sharemem module first before init query list
	lpm_temperature_drv_init_priv();

	ret = lpm_temperature_drv_init_query_list();
	if (ret != 0) {
		lpm_log_err("lpm temperature drv init query list failed, ret=%d\n", ret);
		return ret;
	}

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
STATIC void lpm_temperature_drv_show_query_list(struct seq_file *seq)
{
	uint32_t i;
	int32_t ret;
	uint32_t dev_id;
	uint32_t temperature = 0;
	uint32_t dev_num = lpm_common_get_dev_num();
	struct lpm_temperature_drv_query_list *query_list = lpm_temperature_sharemem_get_query_list();

	seq_printf(seq, "Temperature information queried from shared memory:\n");
	seq_printf(seq, " support_num=%u, dev_num=%u\n", query_list->support_num, dev_num);

	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		seq_printf(seq, " [dev_id=%u]\n", dev_id);

		for (i = 0; i < query_list->support_num; i++) {
			ret = lpm_temperature_drv_get_soc_temp(
				dev_id, query_list->support_cfg[i].core_id, &temperature);
			if (ret != 0) {
				seq_printf(seq, "   [%02u] core_id=%u, temperature_type=%u query failed, ret=%d\n",
					i, (uint32_t)query_list->support_cfg[i].core_id,
					(uint32_t)query_list->support_cfg[i].temp_type, ret);
			} else {
				seq_printf(seq, "   [%02u] core_id=%u, temperature_type=%u, temperature=%u\n",
					i, (uint32_t)query_list->support_cfg[i].core_id,
					(uint32_t)query_list->support_cfg[i].temp_type, temperature);
			}
		}
	}
}

int32_t lpm_temperature_drv_debugfs_show(struct seq_file *seq, void *v)
{
	lpm_temperature_drv_show_query_list(seq);

	(void)v;
	return 0;
}

#endif