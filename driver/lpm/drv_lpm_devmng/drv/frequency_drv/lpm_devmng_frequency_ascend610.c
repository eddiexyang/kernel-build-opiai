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
#include <linux/types.h>
#include "lpm_devmng_common.h"
#include "lpm_devmng_ipc.h"
#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
#include "lpm_devmng_debugfs.h"
#endif
#include "lpm_devmng_frequency_drv.h"
#include "lpm_devmng_frequency_ascend610.h"

STATIC struct lpm_frequency_drv_priv *lpm_frequency_ipc_get_priv_info(void)
{
	static struct lpm_frequency_drv_priv ipc_priv = {0};
	return &ipc_priv;
}

STATIC struct lpm_frequency_query_ipc_list *lpm_frequency_ipc_get_query_list(void)
{
	struct lpm_frequency_drv_priv *ipc_priv = lpm_frequency_ipc_get_priv_info();
	return &ipc_priv->query_list;
}

STATIC struct lpm_frequency_query_ipc_cfg *lpm_frequency_get_support_ipc_cfg(uint32_t *support_num)
{
	static struct lpm_frequency_query_ipc_cfg ipc_cfg[] = {
		// core_id, send_type, data_pos
		{LPM_CLUSTER_ID, LPM_IPC_QUERY_CTRLCPU_FREQ, 0x0},
		{LPM_DDR_ID,     LPM_IPC_QUERY_DDR_FREQ,     0x0},
		{LPM_AICORE0_ID, LPM_IPC_QUERY_AICORE_FREQ,  0x0},
		{LPM_AICORE1_ID, LPM_IPC_QUERY_AICORE_FREQ,  0x1},
		{LPM_VECTOR_ID,  LPM_IPC_QUERY_VECTOR_FREQ,  0x0},
	};
	*support_num = (uint32_t)ARRAY_SIZE(ipc_cfg);
	return &ipc_cfg[0];
}

STATIC int32_t lpm_frequency_init_query_ipc_list(void)
{
	uint32_t support_num = 0;
	struct lpm_frequency_query_ipc_list *query_list = lpm_frequency_ipc_get_query_list();
	struct lpm_frequency_query_ipc_cfg *ipc_cfg = lpm_frequency_get_support_ipc_cfg(&support_num);

	if (support_num > (uint32_t)LPM_INVALID_ID) {
		lpm_log_err("support_num=%u should not exceed %u\n", support_num, (uint32_t)LPM_INVALID_ID);
		return -1;
	}

	query_list->support_num = support_num;
	query_list->ipc_cfg = ipc_cfg;

	lpm_log_info("lpm frequency init ipc query list success, support_num=%u\n", support_num);
	return 0;
}

STATIC int32_t lpm_frequency_drv_init_priv_data(void)
{
	struct lpm_frequency_drv_priv *ipc_priv = lpm_frequency_ipc_get_priv_info();

	ipc_priv->query_list.support_num = 0;
	ipc_priv->query_list.ipc_cfg = NULL;

	return lpm_frequency_init_query_ipc_list();
}

STATIC int32_t lpm_get_frequency_from_ipc(
	uint32_t dev_id, struct lpm_frequency_query_ipc_cfg *ipc_cfg, uint32_t *frequency)
{
	int32_t ret;
	struct lpm_frequency_result_info freq_info = {0};
	struct lpm_devmng_ipc_msg msg_info;

	// no need suppress
	msg_info.send_type = ipc_cfg->send_type;
	msg_info.in        = NULL;
	msg_info.in_len    = 0;
	msg_info.out       = (void *)&freq_info;
	msg_info.out_pos   = 0;
	msg_info.out_len   = (uint32_t)sizeof(freq_info);

	ret = lpm_ipc_send_sync_msg(dev_id, &msg_info);
	if (ret != 0) {
		lpm_log_err("lpm frequency send sync ipc failed, dev_id=%u, ret=%d\n", dev_id, ret);
		return ret;
	}

	// check ack
	if (freq_info.result != 0) {
		lpm_log_err("lpm frequency check ipc result fail, dev_id=%u, send_type=%u, ipc_result=%u\n",
			dev_id, (uint32_t)ipc_cfg->send_type, freq_info.result);
		return -EINVAL;
	}

	*frequency = freq_info.data[ipc_cfg->data_pos];

	return 0;
}

STATIC struct lpm_frequency_query_ipc_cfg *lpm_frequency_get_ipc_cfg_by_core_id(uint32_t core_id)
{
	uint32_t i;
	struct lpm_frequency_query_ipc_list *query_list = lpm_frequency_ipc_get_query_list();

	for (i = 0; i < query_list->support_num; i++) {
		if (core_id == (uint32_t)query_list->ipc_cfg[i].core_id) {
			return &query_list->ipc_cfg[i];
		}
	}

	return NULL;
}

int32_t lpm_frequency_drv_query(uint32_t dev_id, uint32_t core_id, uint32_t *frequency)
{
	int32_t ret;
	struct lpm_frequency_query_ipc_cfg *ipc_cfg = lpm_frequency_get_ipc_cfg_by_core_id(core_id);
	if (ipc_cfg == NULL) {
		return -EOPNOTSUPP;
	}

	ret = lpm_get_frequency_from_ipc(dev_id, ipc_cfg, frequency);
	if (ret != 0) {
		lpm_log_err("lpm query frequency from ipc failed, dev_id=%u, core_id=%u, ret=%d\n",
			dev_id, core_id, ret);
		return ret;
	}

	return 0;
}

int32_t lpm_frequency_drv_probe(uint64_t *param, uint32_t param_num)
{
	int32_t ret = lpm_frequency_drv_init_priv_data();
	if (ret != 0) {
		lpm_log_err("lpm frequency init priv data failed, ret=%d\n", ret);
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

int32_t lpm_frequency_drv_debugfs_show(struct seq_file *seq, void *v)
{
	uint32_t i;
	uint32_t dev_id;
	uint32_t dev_num = lpm_common_get_dev_num();
	struct lpm_frequency_query_ipc_list *query_list = lpm_frequency_ipc_get_query_list();

	seq_printf(seq, "Frequency type that lpm devmng support to query:\n");
	seq_printf(seq, " support_num=%u, dev_num=%u\n", query_list->support_num, dev_num);

	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		seq_printf(seq, " [dev_id=%u]\n", dev_id);

		for (i = 0; i < query_list->support_num; i++) {
			seq_printf(seq, "   [%02u](from ipc) core_id=%u, send_type=%u, data_pos=%u\n",
				i, (uint32_t)query_list->ipc_cfg[i].core_id,
				(uint32_t)query_list->ipc_cfg[i].send_type, query_list->ipc_cfg[i].data_pos);
		}
	}

	(void)v;
	return 0;
}
#endif
