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
#ifdef LPM_FAULT_AO_SAFETY

#include <linux/slab.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include "lpm_fault_aosub_safety.h"
#include "lpm_fault_report.h"
#include "lpm_fault_config.h"
#include "drv_ras_common.h"
#include "dms_sensor_type.h"
#include "dms_interface.h"
#include "dfm_safety_report.h"

#define AOSUB_SRC_ID             1U
#define AOSUB_SMMU_INT_BIT_ID    0U
#define AOSUB_SMMU_ERR_BIT_ID    16U
#define AOSUB_DISP_INT_BIT_ID    1U
#define AOSUB_DISP_ERR_BIT_ID    17U
#define AOSUB_SCHE_INT_BIT_ID    2U
#define AOSUB_SCHE_ERR_BIT_ID    18U

#define AO_SUB_MAX_REG_NUM       0x3U

#define AOSUB_AA_MODULE_ID       (uint8_t)3U
#define AOSUB_DISP_MODULE_ID     (uint8_t)7U
#define AOSUB_SMMU_MODULE_ID     (uint8_t)13U
#define AOSUB_SCHE_MODULE_ID     (uint8_t)33U

// these data should be provided by bios
#define AOSUB_SMMU_REG_LEN       40U
#define AOSUB_DISP_REG_LEN       104U
#define AOSUB_SCHE_REG_LEN       48U
#define AO_MASK_SIZE             6U

STATIC struct module_info {
	uintptr_t phy_addr;
	void __iomem *vaddr;
	uint32_t size;
	uint32_t mask[AO_MASK_SIZE];
	uint32_t val[AO_MASK_SIZE];
} g_ao_module_info = {0x80000000u, (void *)0, 0x10000u, {0x2C04, 0x2C08, 0x2C0C, 0x2C24, 0x2C28, 0x2C2C},
						{0xFFE188CCu, 0xFFFEF7FFu, 0xFFE080CCu, 0xFFF9FFF9u, 0xFFF9FFF9u, 0xFFF9FFF9u}};

STATIC int32_t lpm_fault_aosubsys_init_mem(void)
{
	g_ao_module_info.vaddr = (void __iomem *)ioremap(g_ao_module_info.phy_addr, g_ao_module_info.size);
	if (g_ao_module_info.vaddr == NULL) {
		lpm_log_err("lpm ioremap aosubsys memory failed, size=0x%x\n", g_ao_module_info.size);
		return -1;
	}

	return 0;
}

STATIC void lpm_fault_aosubsys_uninit_mem(void)
{
	if (g_ao_module_info.vaddr != NULL) {
		iounmap(g_ao_module_info.vaddr);
		g_ao_module_info.vaddr = NULL;
	}
}

STATIC void lpm_fault_aosubsys_safety_enable(void)
{
	uintptr_t reg_offset;
	uint32_t i;
	if (g_ao_module_info.vaddr != NULL) {
		for (i = 0; i < AO_MASK_SIZE; i++) {
			reg_offset = (uintptr_t)g_ao_module_info.vaddr + (uintptr_t)g_ao_module_info.mask[i];
			writel(g_ao_module_info.val[i], (void __iomem *)reg_offset);
		}
	}
}

STATIC void lpm_fault_aosubsys_safety_disable(void)
{
	uintptr_t reg_offset;
	uint32_t i;
	if (g_ao_module_info.vaddr != NULL) {
		for (i = 0; i < AO_MASK_SIZE; i++) {
			reg_offset = (uintptr_t)g_ao_module_info.vaddr + (uintptr_t)g_ao_module_info.mask[i];
			writel(0xFFFFFFFFu, (void __iomem *)reg_offset);
		}
	}
}

void lpm_fault_safety_aosub_suspend(void)
{
	lpm_fault_aosubsys_safety_disable();
}

void lpm_fault_safety_aosub_resume(void)
{
	lpm_fault_aosubsys_safety_enable();
}

STATIC void lpm_fault_report_to_sils(const struct lpm_inner_fault_event *event, const struct ao_safety_info *para_list)
{
	uint32_t event_num = 0x1U;
	struct safety_event fault_list = { 0 };
	fault_list.emu_id = (uint8_t)para_list->emu_id;
	fault_list.src_id = (uint8_t)para_list->src_id;
	fault_list.bit_id = (uint8_t)para_list->bit_id;
	fault_list.node_type = (uint8_t)para_list->owner_node_type;
	fault_list.sub_node_type = (uint8_t)event->node_type;
	fault_list.sensor_type = (uint8_t)event->sensor_type;
	fault_list.event_type = (uint8_t)event->event_type;
	fault_list.node_id = para_list->node_id;
	fault_list.event_severity = para_list->event_severity;
	fault_list.event_assertion = (uint8_t)event->assertion;
	fault_list.event_serial_num = 0; // this num should be the same when fault occur and resume

	drvfault_send_safety_info_to_sils(event_num, &fault_list);
}

STATIC void lpm_fault_smmu_report_event(const uint32_t *const register_array, const uint32_t array_size)
{
	int32_t ret;
	uint32_t i;
	struct lpm_inner_fault_event event = {0};
	struct ao_safety_info para_list = {
		.emu_id = AO_SUB_EMU_ID,
		.src_id = AOSUB_SRC_ID,
		.bit_id = AOSUB_SMMU_ERR_BIT_ID,
		.owner_node_type = DMS_DEV_TYPE_AO_SUBSYS,
		.node_id = (uint8_t)0
	};
	static const struct ao_event_info mask_table[] = {
		{SMMU_PARITY_MASK,  DMS_SEN_TYPE_RAS_SENSOR,    LPM_FAULT_EVENT_PARITY_ERR,   0x1, AO_SUBSYS_STATUS_OFST,
		 DMS_DEV_TYPE_AO_SUBSYS, 0, 0x3},
		{SMMU_LKSTEP_MASK,  DMS_SEN_TYPE_SAFETY_SENSOR, LPM_FAULT_EVENT_LOCKSTEP_ERR, 0x1, AO_SUBSYS_STATUS_OFST,
		 DMS_DEV_TYPE_AO_SUBSYS, 0, 0x3},
		{SMMU_TIMEOUT_MASK, DMS_SEN_TYPE_RAS_SENSOR,    LPM_FAULT_EVENT_TIMEOUT_ERR,  0x1, AO_SUBSYS_STATUS_OFST,
		 DMS_DEV_TYPE_AO_SUBSYS, 0, 0x3},
		{SMMU_MBECC_MASK,   DMS_SEN_TYPE_RAS_SENSOR,    LPM_FAULT_EVENT_MBECC,        0x1, AO_SUBSYS_STATUS_OFST,
		 DMS_DEV_TYPE_AO_SUBSYS, 0, 0x3}
	};

	if (array_size != AOSUB_SMMU_REG_LEN) {
		lpm_log_err("array size=%u is not equal to aosub smmu reg len=%u\n", array_size, AOSUB_SMMU_REG_LEN);
		return;
	}

	event.dev_id = (uint8_t)0;
	event.node_type = DMS_DEV_TYPE_AOSUB_SMMU;

	for (i = 0; i < (uint32_t)ARRAY_SIZE(mask_table); ++i) {
		// safety src of sche exist in register_array[0x1]
		if ((register_array[0x2] & mask_table[i].mask) != 0) {
			event.sensor_type = mask_table[i].sensor_type;
			event.event_type = mask_table[i].event_type;
			event.assertion = mask_table[i].event_assertion;
			para_list.event_severity = mask_table[i].event_severity;
			ret = lpm_handle_fault(&event);
			if (ret != 0) {
				lpm_log_err("lpm report smmu fault of aosubsys failed, i=%u, ret=%d\n", i, ret);
				return;
			}

			lpm_fault_report_to_sils(&event, &para_list);
		}
	}
}

STATIC void lpm_fault_disp_report_event(const uint32_t *const register_array, const uint32_t array_size)
{
	int32_t ret;
	uint32_t i;
	struct lpm_inner_fault_event event = {0};
	struct ao_safety_info para_list = {
		.emu_id = AO_SUB_EMU_ID,
		.src_id = AOSUB_SRC_ID,
		.bit_id = AOSUB_DISP_ERR_BIT_ID,
		.owner_node_type = DMS_DEV_TYPE_AO_SUBSYS,
		.node_id = (uint8_t)0
	};
	static const struct ao_event_cfg_info {
		uint32_t offset;
		struct ao_event_info event_info;
	} mask_table[] = {
		{0x4, {DISP_AO_PARITY_MASK,  DMS_SEN_TYPE_RAS_SENSOR,    LPM_FAULT_EVENT_PARITY_ERR,      0x1,
		 AO_SUBSYS_STATUS_OFST, DMS_DEV_TYPE_AO_SUBSYS, 0, 0x3}},
		{0x4, {DISP_AO_LKSTEP_MASK1, DMS_SEN_TYPE_SAFETY_SENSOR, LPM_FAULT_EVENT_LOCKSTEP_ERR,    0x1,
		 AO_SUBSYS_STATUS_OFST, DMS_DEV_TYPE_AO_SUBSYS, 0, 0x3}},
		{0x4, {DISP_AO_TIMEOUT_MASK, DMS_SEN_TYPE_RAS_SENSOR,    LPM_FAULT_EVENT_TIMEOUT_ERR,     0x1,
		 AO_SUBSYS_STATUS_OFST, DMS_DEV_TYPE_AO_SUBSYS, 0, 0x3}},
		{0x4, {DISP_AO_CRC_MASK,     DMS_SEN_TYPE_CHECK_SENSOR,  LPM_FAULT_EVENT_CRC_CHECK_FAIL,  0x1,
		 AO_SUBSYS_STATUS_OFST, DMS_DEV_TYPE_AO_SUBSYS, 0, 0x3}},
		{0x5, {DISP_AO_LKSTEP_MASK2, DMS_SEN_TYPE_SAFETY_SENSOR, LPM_FAULT_EVENT_LOCKSTEP_ERR,    0x1,
		 AO_SUBSYS_STATUS_OFST, DMS_DEV_TYPE_AO_SUBSYS, 0, 0x3}}
	};

	if (array_size != AOSUB_DISP_REG_LEN) {
		lpm_log_err("array size=%u is not equal to aosub disp reg len=%u\n", array_size, AOSUB_DISP_REG_LEN);
		return;
	}

	event.dev_id = (uint8_t)0;
	event.node_type = DMS_DEV_TYPE_AOSUB_DISP;

	for (i = 0; i < (uint32_t)ARRAY_SIZE(mask_table); ++i) {
		if ((register_array[mask_table[i].offset] & mask_table[i].event_info.mask) != 0) {
			event.sensor_type = mask_table[i].event_info.sensor_type;
			event.event_type = mask_table[i].event_info.event_type;
			event.assertion = mask_table[i].event_info.event_assertion;
			para_list.event_severity = mask_table[i].event_info.event_severity;
			ret = lpm_handle_fault(&event);
			if (ret != 0) {
				lpm_log_err("lpm report disp fault of aosubsys failed, i=%u, ret=%d\n", i, ret);
				return;
			}

			lpm_fault_report_to_sils(&event, &para_list);
		}
	}
}

STATIC void lpm_fault_sche_report_event(const uint32_t *const register_array, const uint32_t array_size)
{
	int32_t ret;
	uint32_t i;
	struct lpm_inner_fault_event event = {0};
	struct ao_safety_info para_list = {
		.emu_id = AO_SUB_EMU_ID,
		.src_id = AOSUB_SRC_ID,
		.bit_id = AOSUB_SCHE_ERR_BIT_ID,
		.owner_node_type = DMS_DEV_TYPE_AO_SUBSYS,
		.node_id = (uint8_t)0
	};
	static const struct ao_event_info mask_table[] = {
		{SCH_PARTY_MASK,  DMS_SEN_TYPE_RAS_SENSOR,    LPM_FAULT_EVENT_PARITY_ERR,   0x1,
		 AO_SUBSYS_STATUS_OFST, DMS_DEV_TYPE_AO_SUBSYS, 0, 0x3},
		{SCH_LKSTEP_MASK, DMS_SEN_TYPE_SAFETY_SENSOR, LPM_FAULT_EVENT_LOCKSTEP_ERR, 0x1,
		 AO_SUBSYS_STATUS_OFST, DMS_DEV_TYPE_AO_SUBSYS, 0, 0x3}
	};

	if (array_size != AOSUB_SCHE_REG_LEN) {
		lpm_log_err("array size=%u is not equal to aosub sche reg len=%u\n", array_size, AOSUB_SCHE_REG_LEN);
		return;
	}

	event.dev_id = (uint8_t)0;
	event.node_type = DMS_DEV_TYPE_AOSUB_SCHE;

	for (i = 0; i < (uint32_t)ARRAY_SIZE(mask_table); ++i) {
		// safety error status of sche exist in register_array[0x1]
		if ((register_array[0x2] & mask_table[i].mask) != 0) {
			event.sensor_type = mask_table[i].sensor_type;
			event.event_type = mask_table[i].event_type;
			event.assertion = mask_table[i].event_assertion;
			para_list.event_severity = mask_table[i].event_severity;
			ret = lpm_handle_fault(&event);
			if (ret != 0) {
				lpm_log_err("lpm report sche fault of aosubsys failed, i=%u, ret=%d\n", i, ret);
				return;
			}

			lpm_fault_report_to_sils(&event, &para_list);
		}
	}
}

STATIC void lpm_fault_aosub_handler(const struct notify_data *pdata)
{
	uint32_t i;
	hisi_common_error_info *hisi_error = NULL;
	static const struct func_node handler_table[] = {
		{AOSUB_DISP_MODULE_ID, lpm_fault_disp_report_event},
		{AOSUB_SCHE_MODULE_ID, lpm_fault_sche_report_event},
		{AOSUB_SMMU_MODULE_ID, lpm_fault_smmu_report_event}
	};
	if ((pdata == NULL) || (pdata->origin_data == NULL)) {
		lpm_log_err("input pdata is NULL or origin data is NULL\n");
		return;
	}

	hisi_error = (hisi_common_error_info *)pdata->origin_data;
	for (i = 0; i < (uint32_t)ARRAY_SIZE(handler_table); ++i) {
		if (hisi_error->module_id == handler_table[i].module_id) {
			handler_table[i].handler(hisi_error->register_array, hisi_error->register_array_size);
			return;
		}
	}
	lpm_log_err("failed to parse module id from pdata, module_id=%u\n", (uint32_t)hisi_error->module_id);
}

int32_t lpm_fault_safety_aosub_init(void)
{
	int32_t ret = fpdc_register_fault_notifier(DMS_DEV_TYPE_AO_SUBSYS, lpm_fault_aosub_handler);
	if (ret != 0) {
		lpm_log_err("failed to register handler for aosub, ret=%d\n", ret);
		return ret;
	}

	ret = lpm_fault_aosubsys_init_mem();
	if (ret != 0) {
		(void)fpdc_unregister_fault_notifier(DMS_DEV_TYPE_AO_SUBSYS);
		return ret;
	}

	return 0;
}

int32_t lpm_fault_safety_aosub_uninit(void)
{
	int32_t ret = fpdc_unregister_fault_notifier(DMS_DEV_TYPE_AO_SUBSYS);
	if (ret != 0) {
		lpm_log_err("failed to unregister handler for aosub, ret=%d\n", ret);
	}

	lpm_fault_aosubsys_uninit_mem();
	return ret;
}

STATIC int32_t lpm_safety_report_aosubsys_fault(uint32_t dev_id, uint32_t sensor_type,
	uint32_t event_type, uint32_t node_type, uint32_t node_id)
{
	struct lpm_inner_fault_event event = {0};
	int32_t ret;

	event.dev_id = (uint8_t)dev_id;
	event.assertion = DMS_EVENT_TYPE_OCCUR;
	event.node_type = node_type;
	event.node_inner_id = node_id;
	event.sensor_type = sensor_type;
	event.event_type = event_type;

	ret = lpm_handle_fault(&event);
	if (ret != 0) {
		lpm_log_err("safety report aosubsys fault err, dev_id=%u, sensor_type=%u, event_type=%u, ret=%d\n",
			dev_id, sensor_type, event_type, ret);
		return ret;
	}
	return 0;
}

STATIC bool lpm_safety_aosub_check_irq_param_valid(struct safety_fault_info *safety_fault,
	uint32_t *event_num, struct safety_event **event_list)
{
	if (safety_fault == NULL) {
		lpm_log_err("safety fault is null\n");
		return false;
	}

	if (event_num == NULL) {
		lpm_log_err("event_num is null\n");
		return false;
	}

	if (event_list == NULL) {
		lpm_log_err("event_list is null\n");
		return false;
	}

	if (safety_fault->fault_status_list == NULL) {
		lpm_log_err("fault status list is null\n");
		return false;
	}

	return true;
}

STATIC void lpm_safety_aosbub_update_data_to_sils(struct safety_event **event_list,
	const struct ao_event_info *mask_table, struct safety_fault_info *safety_fault, uint32_t *event_num, uint32_t idx)
{
	struct safety_event *fault_event = NULL;
	fault_event = &(*event_list)[*event_num];
	fault_event->node_type = (uint8_t)mask_table->node_type;
	fault_event->sensor_type = (uint8_t)mask_table->sensor_type;
	fault_event->event_type = (unsigned short)mask_table->event_type;
	// the index of devices
	fault_event->node_id = mask_table->node_id;
	fault_event->sub_node_type = (uint8_t)0;
	fault_event->sub_node_id = (uint8_t)0;
	fault_event->event_severity = mask_table->event_severity;
	// 0:RESUME 1:OCCUR 2:ONE_TIME
	fault_event->event_assertion = (uint8_t)mask_table->event_assertion;
	fault_event->emu_id = safety_fault->fault_status_list[idx].emu_id;
	fault_event->src_id = safety_fault->fault_status_list[idx].src_id;
	fault_event->bit_id = safety_fault->fault_status_list[idx].bit_id;
	*event_num = (*event_num) + (uint32_t)1;
}

STATIC int32_t lpm_safety_proc_aosubsys_event_data(struct safety_fault_info *safety_fault,
	uint32_t *event_num, struct safety_event **event_list)
{
	uint32_t i;
	uint32_t j;
	uint32_t fault_status;
	uint32_t fault_status_offset;
	static const struct ao_event_info mask_table[] = {
		{AOSUB_PARITY_MASK,     DMS_SEN_TYPE_RAS_SENSOR,     LPM_FAULT_EVENT_PARITY_ERR,      0x1,
		 AO_SUBSYS_STATUS_OFST, DMS_DEV_TYPE_AO_SUBSYS,           0,             0x2},
		{AOSUB_TSEN_EXCEP_MASK, DMS_SEN_TYPE_MODULE_SENSOR2, LPM_FAULT_EVENT_TSENSOR_EXCEP,   0x1,
		 AO_SUBSYS_STATUS_OFST, DMS_DEV_TYPE_AO_SUBSYS,           0,             0x2},
		{AOSUB_LKSTEP_MASK,     DMS_SEN_TYPE_SAFETY_SENSOR,  LPM_FAULT_EVENT_LOCKSTEP_ERR,    0x1,
		 AO_SUBSYS_STATUS_OFST, DMS_DEV_TYPE_AO_SUBSYS,           0,             0x3},
		{AOSUB_UNACESS_MASK,    DMS_SEN_TYPE_SAFETY_SENSOR,  LPM_FAULT_EVENT_UN_ACCESS_NOTIF, 0x2,
		 AO_SUBSYS_STATUS_OFST, DMS_DEV_TYPE_AO_SUBSYS,           0,             0x0},
		{AOSUB_BUS_MASK,        DMS_SEN_TYPE_RAS_SENSOR,     LPM_FAULT_EVENT_BUS_ERR,         0x1,
		 AO_SUBSYS_STATUS_OFST, DMS_DEV_TYPE_AO_SUBSYS,           0,             0x3},
		{AOSUB_UV_MASK,         DMS_SEN_TYPE_VOLTAGE,        LPM_FAULT_EVENT_VOLTAGE_UNDER,   0x1,
		 PG_SENSOR_STATUS_OFST, DMS_DEV_TYPE_POWER_GLITCH_SENSOR, 0x1,           0x2},
		{AOSUB_OV_MASK,         DMS_SEN_TYPE_VOLTAGE,        LPM_FAULT_EVENT_VOLTAGE_OVER,    0x1,
		 PG_SENSOR_STATUS_OFST, DMS_DEV_TYPE_POWER_GLITCH_SENSOR, 0x1,           0x2}
	};

	uint32_t event_list_size = ((uint32_t)sizeof(struct safety_event) * (uint32_t)ARRAY_SIZE(mask_table)) *
		(uint32_t)AO_SUB_MAX_REG_NUM;

	*event_num = 0;
	// memory will be freed by other module
	*event_list = (struct safety_event *)kzalloc(event_list_size, GFP_KERNEL);
	if ((*event_list) == NULL) {
		lpm_log_err("kmalloc lpm safety event list is null\n");
		return -1;
	}

	if (safety_fault->fault_reg_num < AO_SUB_MAX_REG_NUM) {
		lpm_log_err("get regs for aosubsys form dfm is less than 2\n");
		kfree(event_list);
		*event_list = NULL;
		return -1;
	}

	for (i = 0; i < AO_SUB_MAX_REG_NUM; ++i) {
		fault_status = safety_fault->fault_status_list[i].fault_status;
		fault_status_offset = safety_fault->fault_status_list[i].fault_status_offset;
		for (j = 0; j < (uint32_t)ARRAY_SIZE(mask_table); ++j) {
			if (((fault_status & mask_table[j].mask) == 0) ||
			    (fault_status_offset != mask_table[j].fault_status_offset)) {
				continue;
			}
			lpm_safety_aosbub_update_data_to_sils(event_list, &mask_table[j], safety_fault, event_num, i);
			if (lpm_safety_report_aosubsys_fault(0, mask_table[j].sensor_type, mask_table[j].event_type,
			    mask_table[j].node_type, mask_table[j].node_id) != 0) {
				lpm_log_err("report safety fault failed, fault status=%u, idx=%u\n", fault_status, j);
			}
		}
	}

	return 0;
}

int32_t lpm_safety_irq_aosubsys_handler(struct safety_fault_info *safety_fault,
	uint32_t *event_num, struct safety_event **event_list)
{
	int32_t ret;

	if (!lpm_safety_aosub_check_irq_param_valid(safety_fault, event_num, event_list)) {
		return -1;
	}

	ret = lpm_safety_proc_aosubsys_event_data(safety_fault, event_num, event_list);
	if (ret != 0) {
		lpm_log_err("process event data for aosubsys failed, ret=%d\n", ret);
		return ret;
	}

	return 0;
}
#endif
