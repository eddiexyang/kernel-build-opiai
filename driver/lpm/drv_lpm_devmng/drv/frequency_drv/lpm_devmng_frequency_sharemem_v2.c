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
#include "lpm_devmng_common.h"
#include "lpm_devmng_sharemem_drv.h"
#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
#include "lpm_devmng_debugfs.h"
#endif
#include "lpm_devmng_frequency_drv.h"
#include "lpm_devmng_frequency_sharemem_v2.h"

STATIC struct lpm_frequency_drv_sharemem_priv *lpm_frequency_sharemem_get_priv_info(void)
{
	static struct lpm_frequency_drv_sharemem_priv sharemem_priv = {0};
	return &sharemem_priv;
}

STATIC struct lpm_frequency_query_sharemem_list *lpm_frequency_sharemem_get_query_list(void)
{
	struct lpm_frequency_drv_sharemem_priv *sharemem_priv = lpm_frequency_sharemem_get_priv_info();
	return &sharemem_priv->query_list;
}

STATIC struct lpm_frequency_sharemem_cfg *lpm_frequency_get_sharemem_cfg(uint32_t *support_num)
{
	static struct lpm_frequency_sharemem_cfg sharemem_cfg[] = {
		// core_id, freq_type
		{LPM_CLUSTER_ID, LPM_DEVMNG_CPU_CURRENT_FREQ},
		{LPM_AICORE0_ID, LPM_DEVMNG_AIC_CURRENT_FREQ},
		{LPM_AICORE1_ID, LPM_DEVMNG_AIC_RATE_FREQ},
	};
	*support_num = (uint32_t)ARRAY_SIZE(sharemem_cfg);
	return &sharemem_cfg[0];
}

STATIC int32_t lpm_frequency_init_query_sharemem_list(void)
{
	uint32_t support_num = 0;
	struct lpm_frequency_query_sharemem_list *query_list = lpm_frequency_sharemem_get_query_list();
	struct lpm_frequency_sharemem_cfg *sharemem_cfg = lpm_frequency_get_sharemem_cfg(&support_num);

	if (support_num > (uint32_t)LPM_INVALID_ID) {
		lpm_log_err("support_num=%u should not exceed %u\n", support_num, (uint32_t)LPM_INVALID_ID);
		return -1;
	}

	query_list->support_num  = support_num;
	query_list->sharemem_cfg = sharemem_cfg;

	lpm_log_info("lpm frequency init sharemem query list success, support_num=%u\n", support_num);
	return 0;
}

STATIC int32_t lpm_frequency_drv_init_priv_data(void)
{
	struct lpm_frequency_drv_sharemem_priv *sharemem_priv = lpm_frequency_sharemem_get_priv_info();

	sharemem_priv->query_list.support_num  = 0;
	sharemem_priv->query_list.sharemem_cfg = NULL;

	return lpm_frequency_init_query_sharemem_list();
}

STATIC int32_t lpm_frequency_get_type_by_core_id(
	uint32_t core_id, enum lpm_devmng_frequency_type *freq_type)
{
	uint32_t i;
	struct lpm_frequency_query_sharemem_list *query_list = lpm_frequency_sharemem_get_query_list();

	for (i = 0; i < query_list->support_num; i++) {
		if (core_id == (uint32_t)query_list->sharemem_cfg[i].core_id) {
			*freq_type = query_list->sharemem_cfg[i].freq_type;
			return 0;
		}
	}
	return -EOPNOTSUPP;
}

int32_t lpm_frequency_drv_query(uint32_t dev_id, uint32_t core_id, uint32_t *frequency)
{
	int32_t ret;
	enum lpm_devmng_frequency_type freq_type = LPM_DEVMNG_SHARE_MEM_FREQ_NUM;

	ret = lpm_frequency_get_type_by_core_id(core_id, &freq_type);
	if (ret != 0) {
		return ret; // -EOPNOTSUPP
	}

	ret = lpm_get_frequency_from_sharemem(dev_id, freq_type, frequency);
	if (ret != 0) {
		lpm_log_err("lpm query frequency from sharemem failed, "
			"dev_id=%u, core_id=%u, freq_type=%u, ret=%d\n",
			dev_id, core_id, (uint32_t)freq_type, ret);
		return ret;
	}

	return 0;
}

#ifdef LPM_TURBO
int32_t lpm_frequency_get_aic_freq_range(uint32_t dev_id, uint32_t *min_freq, uint32_t *max_freq)
{
	int32_t ret;

	ret = lpm_get_frequency_from_sharemem(dev_id, LPM_DEVMNG_AIC_MIN_FREQ, min_freq);
	if (ret != 0) {
		lpm_log_err("lpm freq get min aic freq failed, dev_id=%u, ret=%d\n", dev_id, ret);
		return ret;
	}
	ret = lpm_get_frequency_from_sharemem(dev_id, LPM_DEVMNG_AIC_RATE_FREQ, max_freq);
	if (ret != 0) {
		lpm_log_err("lpm freq get max aic freq failed, dev_id=%u, ret=%d\n", dev_id, ret);
		return ret;
	}

	if (*min_freq > *max_freq) {
		lpm_log_err("lpm freq get aic freq range failed, dev_id=%u, min_freq=%u large than max_freq=%u\n",
			dev_id, *min_freq, *max_freq);
		return -EINVAL;
	}

	return 0;
}
#endif

int32_t lpm_frequency_drv_probe(uint64_t *param, uint32_t param_num)
{
	int32_t ret = lpm_frequency_drv_init_priv_data();
	if (ret != 0) {
		lpm_log_err("lpm frequency drv init priv data failed, ret=%d\n", ret);
		return ret;
	}

	(void)param;
	(void)param_num;
	lpm_log_info("lpm frequency probe success\n");
	return 0;
}

int32_t lpm_frequency_drv_remove(uint64_t *param, uint32_t param_num)
{
	(void)param;
	(void)param_num;
	lpm_log_info("lpm frequency remove success\n");
	return 0;
}

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)

STATIC void lpm_frequency_drv_debugfs_freq_show(struct seq_file *seq)
{
	int32_t ret;
	uint32_t dev_id;
	uint32_t frequency = 0;
	uint32_t dev_num = lpm_common_get_dev_num();

	seq_printf(seq, "Frequency value read from sharemem:\n");
	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		seq_printf(seq, " dev_id=%u\n", dev_id);

		ret = lpm_get_frequency_from_sharemem(dev_id, LPM_DEVMNG_AIC_SET_FREQ, &frequency);
		if (ret != 0) {
			seq_printf(seq, "  get aic set freq failed, ret=%d\n", ret);
		} else {
			seq_printf(seq, "  aic set freq=%u\n", frequency);
		}
	}
}

int32_t lpm_frequency_drv_debugfs_show(struct seq_file *seq, void *v)
{
	uint32_t i;
	uint32_t dev_id;
	uint32_t dev_num = lpm_common_get_dev_num();
	struct lpm_frequency_query_sharemem_list *query_list = lpm_frequency_sharemem_get_query_list();

	seq_printf(seq, "Frequency type that lpm devmng support to query:\n");
	seq_printf(seq, " support_num=%u, dev_num=%u\n", query_list->support_num, dev_num);

	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		seq_printf(seq, " [dev_id=%u]\n", dev_id);

		for (i = 0; i < query_list->support_num; i++) {
			seq_printf(seq, "   [%02u](from sharemem) core_id=%u, frequency_type=%u\n",
				i, (uint32_t)query_list->sharemem_cfg[i].core_id,
				(uint32_t)query_list->sharemem_cfg[i].freq_type);
		}
	}

	lpm_frequency_drv_debugfs_freq_show(seq);

	(void)v;
	return 0;
}

#endif
