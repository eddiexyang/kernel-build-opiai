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
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/io.h>
#include "fpdc.h"
#include "dms_interface.h"
#include "dms_node_type.h"
#include "dms_sensor_type.h"
#include "lpm_fault_report.h"
#include "lpm_fault_config.h"
#include "lpm_fault_common.h"
#include "lpm_fault_lpm_ras.h"

/* report pll ras event */
STATIC void lpm_fault_ras_pll_report_event(const hisi_common_error_info *hisi_error)
{
	int32_t ret;
	struct lpm_inner_fault_event event = {0};
	static uint32_t report_mask = 0; // for log suppress, each pll occupies 1 bit, 0: not report, 1:report

	if (hisi_error->register_array_size != 0) {
		lpm_log_err("array size=%u is not equal to pll reg len\n", hisi_error->register_array_size);
		return;
	}

	if (hisi_error->device_or_core_id >= PLL_NUM_MAX) {
		lpm_log_err("pll ras failed to report for pll[%u] illegal\n", hisi_error->device_or_core_id);
		return;
	}

	/* record error info, log suppress */
	if ((report_mask & ((uint32_t)0x1 << hisi_error->device_or_core_id)) == 0) {
		report_mask |= ((uint32_t)0x1 << hisi_error->device_or_core_id);
		lpm_log_err("pll[%u] unlock occur.\n", hisi_error->device_or_core_id);
	}

	event.dev_id        = (uint32_t)0U;
	event.node_type     = (uint32_t)DMS_DEV_TYPE_LPM;
	event.node_inner_id = (uint32_t)0U;
	event.sensor_type   = (uint32_t)DMS_SEN_TYPE_RAS_SENSOR;
	event.event_type    = (uint32_t)LPM_FAULT_EVENT_ERROR_CANNOT_FIXED;
	event.assertion     = (uint32_t)DMS_EVENT_TYPE_OCCUR; // error occur
	ret = lpm_handle_fault(&event);
	if (ret != 0) {
		lpm_log_err("lpm report pll ras fault failed, pll=%u, ret=%d\n", hisi_error->device_or_core_id, ret);
		return;
	}
}

/* handle ras events which node_type is LPM */
STATIC void lpm_fault_ras_lpm_handler(const struct notify_data *pdata)
{
	uint32_t i;
	hisi_common_error_info *hisi_error = NULL;
	static const struct common_ras_node handler_table[] = {
		{HISI_MODULE_PLL, lpm_fault_ras_pll_report_event}
	};

	if ((pdata == NULL) || (pdata->origin_data == NULL)) {
		lpm_log_err("lpm ras input pdata is NULL or origin data is NULL\n");
		return;
	}

	/* find module handler */
	hisi_error = (hisi_common_error_info *)pdata->origin_data;
	for (i = 0; i < (uint32_t)ARRAY_SIZE(handler_table); ++i) {
		if (hisi_error->module_id == handler_table[i].module_id) {
			handler_table[i].handler(hisi_error);
			return;
		}
	}

	lpm_log_err("lpm ras failed to parse module id from pdata, module_id=%u\n",
		(uint32_t)hisi_error->module_id);
}

/* lpm ras init */
int32_t lpm_fault_ras_lpm_init(void)
{
	/* register fpdc func handler */
	int32_t ret = fpdc_register_fault_notifier(DMS_DEV_TYPE_LPM, lpm_fault_ras_lpm_handler);
	if (ret != 0) {
		lpm_log_err("failed to register handler for lpm ras, ret=%d\n", ret);
		return ret;
	}
	return 0;
}

/* lpm ras uninit */
int32_t lpm_fault_ras_lpm_uninit(void)
{
	/* unregister fpdc func handler */
	int32_t ret = fpdc_unregister_fault_notifier(DMS_DEV_TYPE_LPM);
	if (ret != 0) {
		lpm_log_err("failed to unregister handler for lpm ras, ret=%d\n", ret);
		return ret;
	}
	return 0;
}