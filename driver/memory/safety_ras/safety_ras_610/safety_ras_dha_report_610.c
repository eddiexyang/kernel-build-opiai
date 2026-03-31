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
 *
 * Description:
 * Author: huawei
 * Create: 2022-10-27
 */
#include <linux/types.h>
#include <linux/slab.h>

#include "drv_systime.h"
#include "dfm_safety_report.h"
#include "dfm_report.h"
#include "dms_define.h"
#include "dms_sensor_type.h"

#include "memory_log.h"
#include "safety_ras_dha_610.h"

/**
 * @brief ddra_safety_coverge: ras_fault返回
 */
const struct ras_fault_converge_item *ddra_safety_coverge(const struct ras_fault_converge_item *items,
	uint32_t item_num, unsigned long long ras_code)
{
	uint32_t i;
	/* find right ras_code */
	for (i = 0; i < item_num; i++) {
		if (ras_code == items[i].ras_code.err_status)
			return &items[i];
	}
	return NULL;
}

/*
* @brief get ras_fault_converge_item by section_type and ras_code
* @param [in] section_type: section type
* @param [in] ras_code: ras_code
* @return ras_fault_converge_item
*/
STATIC const struct ras_fault_converge_item *get_ras_coverage_node(uint32_t section_type, uint64_t ras_code)
{
	/* ras table */
	STATIC struct ras_fault_converge_item ddra_dha_ras_converge_tab[] = {
		dha_safety_coverage_item(DHA_RAS_ERR_REQ_MIRREQERR, "dha_ras_err_req_mirreqerr", DMS_SEN_TYPE_RAS_SENSOR,
			RAS_ERROR_TYPE_IN_CFG_ERR), /* dha_ras_err_req_mirreqerr */
		dha_safety_coverage_item(DHA_RAS_ERR_REQ_DMCADDRERR, "dha_ras_err_req_dmcaddrerr", DMS_SEN_TYPE_RAS_SENSOR,
			RAS_ERROR_TYPE_IN_CFG_ERR), /* dha_ras_err_req_dmcaddrerr */
		dha_safety_coverage_item(DHA_RAS_ERR_REQ_MEEVOIDERR, "dha_ras_err_req_meevoiderr", DMS_SEN_TYPE_RAS_SENSOR,
			RAS_ERROR_TYPE_IN_CFG_ERR), /* dha_ras_err_req_meevoiderr */
		dha_safety_coverage_item(DHA_RAS_ERR_REQ_DEFAULT, "dha_ras_err_req_default", DMS_SEN_TYPE_RAS_SENSOR,
			RAS_ERROR_TYPE_IN_CFG_ERR), /* dha_ras_err_req_default */
		dha_safety_coverage_item(DHA_RAS_ERR_REQ_NONSECERR, "dha_ras_err_req_nonsecerr", DMS_SEN_TYPE_RAS_SENSOR,
			RAS_ERROR_TYPE_IN_CFG_ERR), /* dha_ras_err_req_nonsecerr */
		dha_safety_coverage_item(DHA_RAS_ERR_REQ_SECERR, "dha_ras_err_req_secerr", DMS_SEN_TYPE_RAS_SENSOR,
			RAS_ERROR_TYPE_IN_CFG_ERR), /* dha_ras_err_req_secerr */
		dha_safety_coverage_item(DHA_RAS_ERR_REQ_ERRREQ, "dha_ras_err_req_errreq", DMS_SEN_TYPE_RAS_SENSOR,
			RAS_ERROR_TYPE_IN_CFG_ERR), /* dha_ras_err_req_errreq */
		dha_safety_coverage_item(DHA_RAS_ERR_REQ_MSDMISS, "dha_ras_err_req_msdmiss", DMS_SEN_TYPE_RAS_SENSOR,
			RAS_ERROR_TYPE_IN_CFG_ERR), /* dha_ras_err_req_msdmiss */
		dha_safety_coverage_item(DHA_RAS_ERR_REQ_MSDOVERLAP, "dha_ras_err_req_msdoverlap", DMS_SEN_TYPE_RAS_SENSOR,
			RAS_ERROR_TYPE_IN_CFG_ERR), /* dha_ras_err_req_msdoverlap */
		dha_safety_coverage_item(DHA_RAS_ERR_REQ_MSDINVERT, "dha_ras_err_req_msdinvert", DMS_SEN_TYPE_RAS_SENSOR,
			RAS_ERROR_TYPE_IN_CFG_ERR), /* dha_ras_err_req_msdinvert */
		dha_safety_coverage_item(DHA_RAS_ERR_REQ_MSD_CCIX_HIT, "dha_ras_err_req_msd_ccix_hit", DMS_SEN_TYPE_RAS_SENSOR,
			RAS_ERROR_TYPE_IN_CFG_ERR), /* dha_ras_err_req_msd_ccix_hit */
	};
	/* get max items_num */
	uint32_t items_num = (uint32_t)sizeof(ddra_dha_ras_converge_tab) / (uint32_t)sizeof(ddra_dha_ras_converge_tab[0]);
	/* find  ras_fault_converge_item by section_type and ras_code */
	return ddra_safety_coverge(ddra_dha_ras_converge_tab, items_num, ras_code);
}

/**
 * @brief ddra_dha_safety_coverage: ras_fault定义
 */
const struct ras_fault_converge_item *ddra_dha_safety_coverage(uint32_t section_type, unsigned long long ras_code)
{
	/* 故障bit位定义 */
	STATIC struct ras_fault_converge_item ddra_dha_coverage_tab[] = {
		dha_safety_coverage_item(DHA_RAS_SKYROS_SIGNALS_INTR, "dha_ras_skyros_signals_intr", DMS_SEN_TYPE_RAS_SENSOR,
			RAS_ERROR_TYPE_BUS_ERR), /* dha_ras_skyros_signals_intr */
		dha_safety_coverage_item(DHA_RAS_SKYROS_PAYLAOD_INTR, "dha_ras_skyros_payload_intr", DMS_SEN_TYPE_RAS_SENSOR,
			RAS_ERROR_TYPE_BUS_ERR), /* dha_ras_skyros_payload_intr */
		dha_safety_coverage_item(DHA_RAS_INGRESS_INTR, "dha_ras_ingress_intr", DMS_SEN_TYPE_SAFETY_SENSOR,
			SOC_SAFETY_LOCKSTEP_ERR), /* dha_ras_ingress_intr */
		dha_safety_coverage_item(DHA_RAS_PQBANK_MIRROR_INTR, "dha_ras_pqbank_mirror_intr", DMS_SEN_TYPE_SAFETY_SENSOR,
			SOC_SAFETY_UNAUTHORIZED_ACCESS_ERR), /* dha_ras_pqbank_mirror_intr */
		dha_safety_coverage_item(DHA_RAS_PQBANK_LOCKSTEP_IBTR, "dha_ras_pqbank_lockstep_intr",
			DMS_SEN_TYPE_SAFETY_SENSOR, SOC_SAFETY_LOCKSTEP_ERR), /* dha_ras_pqbank_lockstep_intr */
		dha_safety_coverage_item(DHA_RAS_PQBANK_TIMEOUT_INTR, "dha_ras_pqbank_timeout_intr", DMS_SEN_TYPE_RAS_SENSOR,
			RAS_ERROR_TYPE_TIMEOUT_ERR), /* dha_ras_pqbank_timeout_intr */
		dha_safety_coverage_item(DHA_RAS_MEMORY_ADDR_INTR, "dha_ras_memory_addr_intr", DMS_SEN_TYPE_RAS_SENSOR,
			RAS_ERROR_TYPE_PARITY), /* dha_ras_memory_addr_intr */
		dha_safety_coverage_item(DHA_RAS_DMC_CRC_INTR, "dha_ras_dmc_crc_intr", DMS_SEN_TYPE_RAS_SENSOR,
			RAS_ERROR_TYPE_BUS_ERR), /* dha_ras_dmc_crc_intr */
		dha_safety_coverage_item(DHA_RAS_DMC_CNT_INTR, "dha_ras_dmc_cnt_intr", DMS_SEN_TYPE_RAS_SENSOR,
			RAS_ERROR_TYPE_BUS_ERR), /* dha_ras_dmc_cnt_intr */
		dha_safety_coverage_item(DHA_RAS_SDMAA_ISO_INTR, "dha_ras_sdmaa_iso_intr", DMS_SEN_TYPE_SAFETY_SENSOR,
			SOC_SAFETY_UNAUTHORIZED_ACCESS_ERR), /* dha_ras_sdmaa_iso_intr */
		dha_safety_coverage_item(DHA_RAS_SDMAA_LOCKSTEP_INTR, "dha_ras_sdmaa_lockstep_intr", DMS_SEN_TYPE_SAFETY_SENSOR,
			SOC_SAFETY_LOCKSTEP_ERR), /* dha_ras_sdmaa_lockstep_intr */
		dha_safety_coverage_item(DHA_RAS_SDMAA_CRC_CNT_INTR, "dha_ras_sdmaa_crc_cnt_intr", DMS_SEN_TYPE_RAS_SENSOR,
			RAS_ERROR_TYPE_BUS_ERR), /* dha_ras_sdmaa_crc_cnt_intr */
		dha_safety_coverage_item(DHA_RAS_CBUF_LOCKSTEP_INTR, "dha_ras_cbuf_lockstep_intr", DMS_SEN_TYPE_SAFETY_SENSOR,
			SOC_SAFETY_LOCKSTEP_ERR), /* dha_ras_cbuf_lockstep_intr */
		dha_safety_coverage_item(DHA_RAS_SECC_INTR, "dha_ras_secc_intr", DMS_SEN_TYPE_RAS_SENSOR,
			RAS_ERROR_TYPE_SBECCOverThold), /* dha_ras_secc_intr */
		dha_safety_coverage_item(DHA_RAS_MECC_INTR, "dha_ras_mecc_intr", DMS_SEN_TYPE_RAS_SENSOR,
			RAS_ERROR_TYPE_MBECC), /* dha_ras_mecc_intr */
		dha_safety_coverage_item(DHA_RAS_REG_INTR, "dha_ras_reg_intr", DMS_SEN_TYPE_RAS_SENSOR,
			RAS_ERROR_TYPE_PARITY) /* dha_ras_reg_intr */
	};
	/* get max items_num */
	uint32_t items_num = (uint32_t)sizeof(ddra_dha_coverage_tab) / (uint32_t)sizeof(ddra_dha_coverage_tab[0]);
	/* find  ras_fault_converge_item by items_num and ras_code */
	return ddra_safety_coverge(ddra_dha_coverage_tab, items_num, ras_code);
}

/*
* @brief dha check ras param
* @param [in] module_info: moduleinfo
* @param [in] fault_status: fault status
* @return 0: success, else: fail
*/
STATIC int ddra_dha_check_ras_param(const struct dfm_safety_module *module_info,
	const struct safety_fault_status *fault_status)
{
	/* check module info, it could be null */
	if (module_info == NULL) {
		memory_drv_err("module info is null\n");
		return -EINVAL;
	}
	/* check fault_status, it could be null */
	if (fault_status == NULL) {
		memory_drv_err("safety fault is null\n");
		return -EINVAL;
	}
	return 0;
}

/*
* @brief report ras fault
* @param [in] module_info: moduleinfo
* @param [in] fault_status: fault status
* @param [in] safety_bit: safety bit
* @return 0: success, else: fail
*/
STATIC int ddra_dha_report_ras_fault(const struct dfm_safety_module *module_info,
	const struct safety_fault_status *fault_status, uint32_t safety_bit)
{
	const struct ras_fault_converge_item *coverage_node = NULL;
	int ret;
	/* check ras param */
	ret = ddra_dha_check_ras_param(module_info, fault_status);
	if (ret != 0) {
		return -EINVAL;
	}
	/* find ras_fault_converge_item */
	if (fault_status->fault_status_offset == DHA_RAS_SAFETY_STATUS_REG_OFFSET) {
		coverage_node = ddra_dha_safety_coverage(module_info->section_type, safety_bit);
	} else {
		coverage_node = get_ras_coverage_node(module_info->section_type, safety_bit);
	}
	if (coverage_node == NULL) {
		memory_drv_err("get safety code failed.(bit id=%u)\n", safety_bit);
		return -EINVAL;
	}
	/* report ras fault */
	ret = dfm_add_event(&ddra_dha_dms, module_info->node_id, coverage_node);
	if ((ret != 0) && (ret != -EEXIST)) {
		memory_drv_err("Process fault event info failed.(ret=%d)\n", ret);
		return ret;
	}

	return 0;
}

/**
 * @brief ddra_dha_ras_safety_handler: dha ras_safety故障回调接口
 */
STATIC int ddra_dha_ras_safety_handler(const struct dfm_safety_module *module_info,
	const struct safety_fault_status *fault_status)
{
	uint32_t status_val, safety_bit;

	memory_drv_info("here we receive ras_safety_ddra_dha_hwinfo!\n");
	/* check ras param */
	if (ddra_dha_check_ras_param(module_info, fault_status) != 0) {
		return -EINVAL;
	}

	status_val = fault_status->fault_status;
	while (status_val > 0) {
		safety_bit = ffs(status_val) - 1;
		/* report res fault to dfm */
		if (ddra_dha_report_ras_fault(module_info, fault_status, safety_bit) != 0) {
			memory_drv_err("report ras code failed.(status id=%u)\n", safety_bit);
		}
		status_val &= ~(((uint32_t)1U) << safety_bit);
	}

	memory_drv_info("ddra dha ras safety handler success!\n");
	return 0;
}

STATIC int ddra_dha_safety_irq_preprocess(struct safety_fault_info *safety_fault,
	unsigned int *event_num, struct safety_event **event_list)
{
	/* check safety_fault */
	if (safety_fault == NULL) {
		memory_drv_err("safety fault is null!\n");
		return -EINVAL;
	}

	/* check event_num */
	if (event_num == NULL) {
		memory_drv_err("event_num is null!\n");
		return -EINVAL;
	}

	*event_num = 0;
	/* check event_list */
	if (event_list == NULL) {
		memory_drv_err("event_list is null ptr!\n");
		return -EINVAL;
	}
	/* memory alloc for *event_list */
	*event_list = (struct safety_event *)kmalloc(sizeof(struct safety_event) * DHA_SAFETY_MAX_NUM, GFP_KERNEL);
	if ((*event_list) == NULL) {
		memory_drv_err("kmalloc safety event list failed!\n");
		return -ENOMEM;
	}

	return 0;
}

/**
 * @brief ddra_dha_safety_irq_handler: dha safety故障回调接口
 */
int ddra_dha_safety_irq_handler(struct safety_fault_info *safety_fault,
	unsigned int *event_num, struct safety_event **event_list)
{
	uint64_t end_time;
	struct dfm_safety_module module_info;
	uint32_t node_id = 0;
	uint32_t i;
	int ret;

	memory_drv_info("here we receive safety_ddra_dha_hwinfo!\n");

	ret = ddra_dha_safety_irq_preprocess(safety_fault, event_num, event_list);
	if (ret != 0) {
		return ret;
	}

	module_info.dev_id = (uint8_t)safety_fault->dev_id;
	module_info.node_type = DMS_DEV_TYPE_DDRA;
	module_info.max_event = DHA_SAFETY_MAX_NUM;

	for (i = 0; i < safety_fault->fault_reg_num; i++) {
		module_info.node_id = 0;
		module_info.dev_node = &ddra_dha_dms.dev_nodes[node_id];
		module_info.section_type = 0;
		if (safety_fault->fault_status_list[i].fault_status_offset == DHA_RAS_SAFETY_STATUS_REG_OFFSET) {
			/* add ras */
			ret = ddra_dha_ras_safety_handler(&module_info, &safety_fault->fault_status_list[i]);
			if (ret != 0) {
				memory_drv_err("ddra dha ras safety handler failed.\n");
			}
			continue;
		}
		/* report ras */
		ret = dfm_safety_handler(&module_info, &safety_fault->fault_status_list[i], event_num, *event_list);
		if (ret != 0) {
			memory_drv_err("ddra dha safety handler failed.\n");
		}
	}
	/* calculate the function execution time */
	end_time = get_syscnt();
	memory_drv_info("SysCnt: ddra dha safety handler success(%llu)!\n", end_time);

	return 0;
}

int ddra_dha_ras_irq_handler(struct safety_fault_info *safety_fault,
	unsigned int *event_num, struct safety_event **event_list)
{
	struct dfm_safety_module module_info;
	uint32_t i;

	memory_drv_info("here we receive ras_ddra_dha_hwinfo!\n");
	/* check safety_fault */
	if (safety_fault == NULL) {
		memory_drv_err("safety fault is null!\n");
		return -EINVAL;
	}
	/* check event_num */
	if (event_num == NULL) {
		memory_drv_err("event_num is null!\n");
		return -EINVAL;
	}
	/* check event_list */
	if (event_list == NULL) {
		memory_drv_err("event_list is null ptr!\n");
		return -EINVAL;
	}
	/* fill data */
	module_info.dev_id = (uint8_t)safety_fault->dev_id;
	module_info.node_type = DMS_DEV_TYPE_DDRA;
	module_info.node_id = 0;
	module_info.section_type = 0;
	/* report ras */
	for (i = 0; i < safety_fault->fault_reg_num; i++) {
		if (ddra_dha_ras_safety_handler(&module_info, &safety_fault->fault_status_list[i]) != 0) {
			memory_drv_err("ddra dha ras handler failed.\n");
		}
	}
	/* dha ras dfx */
	ddra_dha_ras_dfx();

	memory_drv_info("ddra dha ras handler success!\n");
	return 0;
}