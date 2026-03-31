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
#include "lpm_init.h"
#include "drv_ras_common.h"
#include "dms_interface.h"
#include "dms_node_type.h"
#include "dms_sensor_type.h"
#include "lpm_fault_report.h"
#include "lpm_fault_config.h"
#include "lpm_fault_common.h"
#include "lpm_fault_aosub_ras.h"
#include "lpm_fault_aosub_safety.h"

#if defined(CFG_SOC_PLATFORM_MDC_V11)
#define AOSUB_AA_REG_LEN         40U
#define AOSUB_SMMU_REG_LEN       64U
#endif

#define AOSUB_DISP_REG_LEN       64U
#define AOSUB_DISP_STATUS_RAS_OFST    0x2010U

/* fault ras aosub disp report handler func */
STATIC void lpm_fault_ras_disp_report_event(const uint32_t *const register_array, const uint32_t array_size)
{
	int32_t ret;
	uint32_t i;
	struct lpm_inner_fault_event event = {0};

	static const struct ao_event_cfg_info {
		uint32_t offset;
		struct ao_event_info event_info;
	} mask_table[] = {
		/* dispatch fault */
		{0x4, {DISP_AO_SBECC_MASK,  DMS_SEN_TYPE_RAS_SENSOR, LPM_FAULT_EVENT_SBECCOVERTHOLD,    0x1,
				AOSUB_DISP_STATUS_RAS_OFST, DMS_DEV_TYPE_AOSUB_DISP, 0, 0x1}},
		{0x4, {DISP_AO_MBECC_MASK, DMS_SEN_TYPE_RAS_SENSOR,  LPM_FAULT_EVENT_MBECC,             0x1,
				AOSUB_DISP_STATUS_RAS_OFST, DMS_DEV_TYPE_AOSUB_DISP, 0, 0x2}},
		{0x4, {DISP_AO_INPUT_ERR_MASK, DMS_SEN_TYPE_RAS_SENSOR, LPM_FAULT_EVENT_INPUT_ERROR,    0x1,
				AOSUB_DISP_STATUS_RAS_OFST, DMS_DEV_TYPE_AOSUB_DISP, 0, 0x2}},
		{0x4, {DISP_AO_WRITE_ERR_MASK, DMS_SEN_TYPE_RAS_SENSOR, LPM_FAULT_EVENT_INPUT_ERROR,    0x1,
				AOSUB_DISP_STATUS_RAS_OFST, DMS_DEV_TYPE_AOSUB_DISP, 0, 0x2}},
		{0x4, {DISP_AO_CONFIG_MASK, DMS_SEN_TYPE_RAS_SENSOR, LPM_FAULT_EVENT_CONFIG_ERR,        0x1,
				AOSUB_DISP_STATUS_RAS_OFST, DMS_DEV_TYPE_AOSUB_DISP, 0, 0x2}}
	};

	if (array_size != AOSUB_DISP_REG_LEN) {
		lpm_log_err("array size=%u is not equal to aosub disp reg len=%u\n", array_size, AOSUB_DISP_REG_LEN);
		return;
	}

	event.dev_id = (uint8_t)0;
	event.node_type = DMS_DEV_TYPE_AOSUB_DISP;

	for (i = 0; i < (uint32_t)ARRAY_SIZE(mask_table); ++i) {
		if ((register_array[mask_table[i].offset] & 0xFFU) == mask_table[i].event_info.mask) {
			event.sensor_type = mask_table[i].event_info.sensor_type;
			event.event_type = mask_table[i].event_info.event_type;
			event.assertion = mask_table[i].event_info.event_assertion;
			ret = lpm_handle_fault(&event);
			if (ret != 0) {
				lpm_log_err("lpm report disp fault of aosubsys failed, i=%u, ret=%d\n", i, ret);
				return;
			}
		}
	}
}

#if defined(CFG_SOC_PLATFORM_MDC_V11)
// error code defined in lpm_fault_aosub_ras.h
STATIC int32_t lpm_fault_aosub_report_event(
	const uint32_t *const register_array, const struct ras_event_info *mask_table, uint32_t table_size)
{
	int32_t ret;
	uint32_t i;
	struct lpm_inner_fault_event event = {0};

	event.dev_id = (uint32_t)0U;

	for (i = 0; i < table_size; ++i) {
		if ((register_array[mask_table[i].offset] & mask_table[i].filter_mask) == mask_table[i].err_mask) {
			event.node_inner_id = mask_table[i].node_id;
			event.node_type     = mask_table[i].node_type;
			event.sensor_type   = mask_table[i].sensor_type;
			event.event_type    = mask_table[i].event_type;
			event.assertion     = mask_table[i].assertion;
			ret = lpm_handle_fault(&event);
			if (ret != 0) {
				lpm_log_err("lpm report aosubsys fault failed, ret=%d, reg_val[%u]=0x%x, "
					"node_id=%u, node_type=0x%x, sensor_type=0x%x, event_type=0x%x, assertion=%u\n",
					ret, mask_table[i].offset, register_array[mask_table[i].offset],
					mask_table[i].node_id, mask_table[i].node_type, mask_table[i].sensor_type,
					mask_table[i].event_type, mask_table[i].assertion);
				return FAULT_EVENT_REPORT_FAIL;
			}
			return 0;
		}
	}

	// not record log in this function, whether logging is required is decided by the caller
	// Do not modify this special return value
	return FAULT_EVENT_NOT_FOUND;
}

STATIC void lpm_fault_ras_aa_report_event(const uint32_t *const register_array, const uint32_t array_size)
{
	int32_t ret;
	static const struct ras_event_info aa_mask_table[] = {
		{0x0U, 0x0U, DMS_EVENT_TYPE_OCCUR, 0x0, AA_AO_ERR_FILTER_MASK, AA_AO_DEC_MISS,
			DMS_DEV_TYPE_AOSUB_AA, DMS_SEN_TYPE_RAS_SENSOR, LPM_FAULT_EVENT_CONFIG_ERR},
		{0x0U, 0x0U, DMS_EVENT_TYPE_OCCUR, 0x0, AA_AO_ERR_FILTER_MASK, AA_AO_DAW_OVERLAP,
			DMS_DEV_TYPE_AOSUB_AA, DMS_SEN_TYPE_RAS_SENSOR, LPM_FAULT_EVENT_CONFIG_ERR},
		{0x0U, 0x0U, DMS_EVENT_TYPE_OCCUR, 0x0, AA_AO_ERR_FILTER_MASK, AA_AO_MSD_OVERLAP,
			DMS_DEV_TYPE_AOSUB_AA, DMS_SEN_TYPE_RAS_SENSOR, LPM_FAULT_EVENT_CONFIG_ERR}
	};

	if (array_size != AOSUB_AA_REG_LEN) {
		lpm_log_err("array size=%u is not equal to aosub aa reg len=%u\n", array_size, AOSUB_AA_REG_LEN);
		return;
	}

	ret = lpm_fault_aosub_report_event(register_array, &aa_mask_table[0], (uint32_t)ARRAY_SIZE(aa_mask_table));
	if ((ret != 0) && (ret != FAULT_EVENT_NOT_FOUND)) {
		lpm_log_err("lpm report aa fault of aosubsys failed, ret=%d\n", ret);
		return;
	}
}

STATIC int32_t lpm_fault_ras_smmu_ierr_report_event(const uint32_t *const register_array)
{
	int32_t ret;
	static const struct ras_event_info smmu_ierr_mask_table[] = {
		{0x4U, 0x0U, DMS_EVENT_TYPE_OCCUR, 0x0, SMMU_AO_IERR_SERR_FILTER_MASK, SMMU_AO_STE_FETCH_ERR_MASK,
			DMS_DEV_TYPE_AOSUB_SMMU, DMS_SEN_TYPE_RAS_SENSOR, LPM_FAULT_EVENT_CONFIG_ERR},
		{0x4U, 0x0U, DMS_EVENT_TYPE_OCCUR, 0x0, SMMU_AO_IERR_SERR_FILTER_MASK, SMMU_AO_CD_FETCH_ERR_MASK,
			DMS_DEV_TYPE_AOSUB_SMMU, DMS_SEN_TYPE_RAS_SENSOR, LPM_FAULT_EVENT_CONFIG_ERR},
		{0x4U, 0x0U, DMS_EVENT_TYPE_OCCUR, 0x0, SMMU_AO_IERR_SERR_FILTER_MASK, SMMU_AO_WALK_EABT_ERR_MASK,
			DMS_DEV_TYPE_AOSUB_SMMU, DMS_SEN_TYPE_RAS_SENSOR, LPM_FAULT_EVENT_CONFIG_ERR},
		{0x4U, 0x0U, DMS_EVENT_TYPE_OCCUR, 0x0, SMMU_AO_IERR_SERR_FILTER_MASK, SMMU_AO_CMDQ_FETCH_ERR_MASK,
			DMS_DEV_TYPE_AOSUB_SMMU, DMS_SEN_TYPE_RAS_SENSOR, LPM_FAULT_EVENT_CONFIG_ERR},
		{0x4U, 0x0U, DMS_EVENT_TYPE_OCCUR, 0x0, SMMU_AO_IERR_SERR_FILTER_MASK, SMMU_AO_WRITE_EVENTQ_ABORD_ERR_MASK,
			DMS_DEV_TYPE_AOSUB_SMMU, DMS_SEN_TYPE_RAS_SENSOR, LPM_FAULT_EVENT_CONFIG_ERR},
		{0x4U, 0x0U, DMS_EVENT_TYPE_OCCUR, 0x0, SMMU_AO_IERR_SERR_FILTER_MASK, SMMU_AO_WRITE_PRIQ_ABORD_ERR_MASK,
			DMS_DEV_TYPE_AOSUB_SMMU, DMS_SEN_TYPE_RAS_SENSOR, LPM_FAULT_EVENT_CONFIG_ERR},
		{0x4U, 0x0U, DMS_EVENT_TYPE_OCCUR, 0x0, SMMU_AO_IERR_SERR_FILTER_MASK, SMMU_AO_AXI_TIMEOUT_ERR_MASK,
			DMS_DEV_TYPE_AOSUB_SMMU, DMS_SEN_TYPE_RAS_SENSOR, LPM_FAULT_EVENT_CONFIG_ERR},
		{0x4U, 0x0U, DMS_EVENT_TYPE_OCCUR, 0x0, SMMU_AO_IERR_SERR_FILTER_MASK, SMMU_AO_SKYROS_TIMEOUT_ERR_MASK,
			DMS_DEV_TYPE_AOSUB_SMMU, DMS_SEN_TYPE_RAS_SENSOR, LPM_FAULT_EVENT_CONFIG_ERR},
		{0x4U, 0x0U, DMS_EVENT_TYPE_OCCUR, 0x0, SMMU_AO_IERR_SERR_FILTER_MASK, SMMU_AO_VMS_FETCH_ERR_MASK,
			DMS_DEV_TYPE_AOSUB_SMMU, DMS_SEN_TYPE_RAS_SENSOR, LPM_FAULT_EVENT_CONFIG_ERR},
	};

	ret = lpm_fault_aosub_report_event(
		register_array, &smmu_ierr_mask_table[0], (uint32_t)ARRAY_SIZE(smmu_ierr_mask_table));
	if ((ret != 0) && (ret != FAULT_EVENT_NOT_FOUND)) {
		lpm_log_err("lpm report smmu ierr fault of aosubsys failed, ret=%d\n", ret);
	}

	// report smmu ierr error done
	return ret;
}

STATIC int32_t lpm_fault_ras_smmu_serr_report_event(const uint32_t *const register_array)
{
	int32_t ret;
	static const struct ras_event_info smmu_serr_mask_table[] = {
		{0x4U, 0x0U, DMS_EVENT_TYPE_OCCUR, 0x0, SMMU_AO_ECC_FILTER_MASK, SMMU_AO_SBECC_TRANSIENT_MASK,
			DMS_DEV_TYPE_AOSUB_SMMU, DMS_SEN_TYPE_RAS_SENSOR, LPM_FAULT_EVENT_SBECCOVERTHOLD},
		{0x4U, 0x0U, DMS_EVENT_TYPE_OCCUR, 0x0, SMMU_AO_ECC_FILTER_MASK, SMMU_AO_SBECC_ERR_MASK,
			DMS_DEV_TYPE_AOSUB_SMMU, DMS_SEN_TYPE_RAS_SENSOR, LPM_FAULT_EVENT_SBECCOVERTHOLD},
		{0x4U, 0x0U, DMS_EVENT_TYPE_OCCUR, 0x0, SMMU_AO_ECC_FILTER_MASK, SMMU_AO_SBECC_PERSISTENT_MASK,
			DMS_DEV_TYPE_AOSUB_SMMU, DMS_SEN_TYPE_RAS_SENSOR, LPM_FAULT_EVENT_SBECCOVERTHOLD},
		{0x4U, 0x0U, DMS_EVENT_TYPE_OCCUR, 0x0, SMMU_AO_ECC_FILTER_MASK, SMMU_AO_MBECC_DE_MASK,
			DMS_DEV_TYPE_AOSUB_SMMU, DMS_SEN_TYPE_RAS_SENSOR, LPM_FAULT_EVENT_MBECC},
		{0x4U, 0x0U, DMS_EVENT_TYPE_OCCUR, 0x0, SMMU_AO_ECC_FILTER_MASK, SMMU_AO_MBECC_UE_MASK,
			DMS_DEV_TYPE_AOSUB_SMMU, DMS_SEN_TYPE_RAS_SENSOR, LPM_FAULT_EVENT_MBECC},
	};

	if ((register_array[0x4] & SMMU_AO_IERR_SERR_FILTER_MASK) != SMMU_AO_ECC_MASK) {
		// not 1bit(CE) or 2bit(UEU), not report fault
		return 0;
	}

	// could be 1bit(CE) or 2bit(UEU), UEU or CE can tell from UE(bit29)/CE(bit25~24)/DE(Bit23)
	ret = lpm_fault_aosub_report_event(
		register_array, &smmu_serr_mask_table[0], (uint32_t)ARRAY_SIZE(smmu_serr_mask_table));
	if ((ret != 0) && (ret != FAULT_EVENT_NOT_FOUND)) {
		lpm_log_err("lpm report smmu serr fault of aosubsys failed, ret=%d\n", ret);
	}

	// report smmu ecc error done
	return ret;
}

STATIC void lpm_fault_ras_smmu_report_event(const uint32_t *const register_array, const uint32_t array_size)
{
	int32_t ret;

	if (array_size != AOSUB_SMMU_REG_LEN) {
		lpm_log_err("array size=%u is not equal to aosub smmu reg len=%u\n", array_size, AOSUB_SMMU_REG_LEN);
		return;
	}

	ret = lpm_fault_ras_smmu_ierr_report_event(register_array);
	if (ret != FAULT_EVENT_NOT_FOUND) {
		return;
	}

	ret = lpm_fault_ras_smmu_serr_report_event(register_array);
	if (ret != FAULT_EVENT_NOT_FOUND) {
		return;
	}
}
#endif

/* fault ras aosub handler func */
STATIC void lpm_fault_ras_aosub_handler(const struct notify_data *pdata)
{
	uint32_t i;
	hisi_common_error_info *hisi_error = NULL;
	static const struct func_node handler_table[] = {
		{HISI_MODULE_DISP, lpm_fault_ras_disp_report_event}, // module id 7
#if defined(CFG_SOC_PLATFORM_MDC_V11)
		{HISI_MODULE_AA,   lpm_fault_ras_aa_report_event},   // module id 3
		{HISI_MODULE_SMMU, lpm_fault_ras_smmu_report_event}  // module id 13
#endif
	};
	if ((pdata == NULL) || (pdata->origin_data == NULL)) {
		lpm_log_err("input pdata is NULL or origin data is NULL\n");
		return;
	}
	/* find module id handler */
	hisi_error = (hisi_common_error_info *)pdata->origin_data;
	for (i = 0; i < (uint32_t)ARRAY_SIZE(handler_table); ++i) {
		if (hisi_error->module_id == handler_table[i].module_id) {
			handler_table[i].handler(hisi_error->register_array, hisi_error->register_array_size);
			return;
		}
	}
	lpm_log_err("lpm_fault_ras_aosub_handler failed to parse module id from pdata, module_id=%u\n",
		(uint32_t)hisi_error->module_id);
}

/* aosub ras init */
int32_t lpm_fault_ras_aosub_init(void)
{
	/* register fpdc func handler */
	int32_t ret = fpdc_register_fault_notifier(DMS_DEV_TYPE_AO_SUBSYS, lpm_fault_ras_aosub_handler);
	if (ret != 0) {
		lpm_log_err("failed to register handler for aosub, ret=%d\n", ret);
		return ret;
	}
	return 0;
}

/* aosub ras uninit */
int32_t lpm_fault_ras_aosub_uninit(void)
{
	/* unregister fpdc func handler */
	int32_t ret = fpdc_unregister_fault_notifier(DMS_DEV_TYPE_AO_SUBSYS);
	if (ret != 0) {
		lpm_log_err("failed to unregister handler for aosub, ret=%d\n", ret);
		return ret;
	}
	return 0;
}