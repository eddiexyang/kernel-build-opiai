/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
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
* Create: 2019-10-15
*/

#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <asm/io.h>
#include <linux/workqueue.h>
#include <linux/idr.h>
#include <linux/list.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>
#include <linux/smp.h>

#include "memory_log.h"
#include "safety_ras_config_910.h"
#include "memory_fault_converge.h"
#include "devdrv_common.h"
#include "memory_ddr.h"
#ifdef SUPPORT_HBM_ISOLATION
#include "hbm_multi.h"
#endif
#include "fpdc_ras_receiver.h"
#include "dms_template.h"
#include "urd_feature.h"
#include "dms_cmd_def.h"
#include "urd_acc_ctrl.h"
#include "drvmem_base_info.h"
#include "drv_ras_common.h"

#define RAS_DDRC_UNMASK_REMAP_SIZE 0x4
#define RAS_DDRC_UNMASK_VAL 0x51DU
#define DEVICE_OFFSET 0x200000000000ULL
#define RAS_DDRC0_BASS_ADDR 0x80102000U
#define RAS_DDRC1_BASS_ADDR 0x80112000U
#define RAS_DDRC_UNMASK_OFFSET 0x8U
#define RAS_DDRC_NUM 2U

uint32_t memory_dev_num(uint32_t dev_num);
struct drv_memory_devices *memory_get_devices(void);
void memory_fault_event_free(struct memory_event *event_queue);
int32_t memory_register_one_node(struct memory_dev *m_dev);
void memory_unregister_one_node(struct memory_dev *m_dev);

struct drv_memory_devices *g_memory_devs_ptr = NULL;

STATIC unsigned int g_fault_event_record_ras_type_table[] = {
	RAS_ERROR_TYPE_ERROR,
	RAS_ERROR_TYPE_IN_CFG_ERR,
	RAS_ERROR_TYPE_PARITY,
	RAS_ERROR_TYPE_MBECC,
	RAS_ERROR_TYPE_BUS_ERR,
	RAS_ERROR_TYPE_TIMEOUT_ERR,
};

STATIC unsigned int g_fault_event_record_mem_type_table[] = {
	MEMORY_ERROR_TYPE_MIRROR_SUB_ERROR
};

STATIC unsigned int g_fault_event_record_tem_type_table[] = {
	TEMP_ERROR_TYPE_L3H_OCCUR
};

STATIC void memory_kfree(struct memory_error_list **ptr)
{
	if (*ptr != NULL) {
		kfree(*ptr);
		*ptr = NULL;
	}
}

STATIC void write_ddrc_reg32(unsigned long long reg_phy_addr, unsigned int dev_id)
{
	void __iomem *reg_virt_addr = NULL;

	reg_virt_addr = ioremap(reg_phy_addr, RAS_DDRC_UNMASK_REMAP_SIZE);
	if (reg_virt_addr == NULL) {
		memory_drv_err("Failed to ioremap ddrc unmask irq register. (devid=%u)\n", dev_id);
		return;
	}

	writel(RAS_DDRC_UNMASK_VAL, (volatile void *)reg_virt_addr);
	iounmap(reg_virt_addr);
	reg_virt_addr = NULL;

	memory_drv_event("Unmask ddrc local ras irq success. (devid=%u)\n", dev_id);
}

void memory_ddrc_fault_unmask_task(struct work_struct *work)
{
	struct memory_dev *m_dev = container_of(work, struct memory_dev, unmask_irq_work.work);
	unsigned int ddrc_base_addr_arg[RAS_DDRC_NUM] = {RAS_DDRC0_BASS_ADDR, RAS_DDRC1_BASS_ADDR};
	unsigned long long reg_phy_addr;
	unsigned int i;

	for (i = 0; i < RAS_DDRC_NUM; i++) {
		if (m_dev->work_data[i] == 1) {
			reg_phy_addr = DEVICE_OFFSET * m_dev->die_id + ddrc_base_addr_arg[i] + RAS_DDRC_UNMASK_OFFSET;
			write_ddrc_reg32(reg_phy_addr, m_dev->die_id);
			m_dev->work_data[i] = 0;
		}
	}
}

void ddrc_relieve_suppresion(struct memory_fault_event *event)
{
#ifdef CFG_SOC_PLATFORM_CLOUD
	struct memory_dev *m_dev = NULL;
	m_dev = &g_memory_devs_ptr->m_dev[event->device_id][MEMORY_SENSOR_DDRC];

	if (event->sensor_id == MEMORY_SENSOR_DDRC) {
		if (((event->sec_type == RAS_SEC_MEM) && (event->ras_err_type != 2)) || /* 2 – Single-bit ECC */
			(event->sec_type == RAS_SEC_OEM)) {
			if (event->submodule_id < MEMORY_UNMASK_IRQ_WORK_DATA) { /* DDRC0_SUB_MODULE ~ DDRC1_SUB_MODULE */
				m_dev->work_data[event->submodule_id] = 1;
				(void)schedule_delayed_work(&m_dev->unmask_irq_work, msecs_to_jiffies(300000)); /* 300000:5 minutes */
			}
		}
	}
#endif
}

int hbm_relieve_suppresion(uint32_t dev_id, uint32_t hbmc_id)
{
#ifdef NEED_RECOVER_HBM_INTERRUPT
	int ret;
	uint64_t base;
	uint32_t rev_val, hbm_stack_i, hbm_pc_i;

	hbm_stack_i = (hbmc_id - ECC_MODULE_HBM_MIN) / MAX_HBM_PC_PER_STACK;
	hbm_pc_i = (hbmc_id - ECC_MODULE_HBM_MIN) % MAX_HBM_PC_PER_STACK;
	ret = get_hbm_stack_base(dev_id, hbm_stack_i, &base);
	if (ret != 0) {
		memory_drv_err("get stack base fail.ret=%d\n", ret);
		return ret;
	}
	base += HBM_HBMC_BASE_ADDR + HBM_HBMC_ARER_ADDR + HBM_HBMC_PC_OFFSET * hbm_pc_i +
		ARER_ERR_CTLR_L_OFFSET;

	ret = read_reg32(base, &rev_val);
	if (ret) {
		memory_drv_err("read dev-%u hbm_stack-%u hbm_pc-%u ctlr reg failed, ret: %d.\n",
			dev_id, hbm_stack_i, hbm_pc_i, ret);
		return ret;
	}
	rev_val |= INHIBIT_INTERRUPT_VAL;

	ret = write_reg32(base, rev_val);
	if (ret) {
		memory_drv_err("write dev-%u hbm_stack-%u hbm_pc-%u ctlr reg failed, ret: %d.\n",
			dev_id, hbm_stack_i, hbm_pc_i, ret);
		return ret;
	}
#else
	(void)dev_id;
	(void)hbmc_id;
#endif
	return 0;
}

STATIC int memory_add_fault_event(struct memory_error_list *error_new, struct memory_event *event_queue)
{
	struct memory_error_list *pos = NULL;
	struct memory_error_list *n = NULL;

	if ((event_queue == NULL) || (error_new == NULL)) {
		memory_drv_err("Invalid para. (event_queue NULL=%d)\n", (event_queue == NULL));
		return -EINVAL;
	}

	mutex_lock(&event_queue->mutex);
	list_for_each_entry_safe(pos, n, &event_queue->error_list.list, list) {
		if ((pos->error.sec_type == error_new->error.sec_type) &&
			(pos->error.ras_err_type == error_new->error.ras_err_type)) {
			mutex_unlock(&event_queue->mutex);
			return 1; /* 1 means found fault event already in event list, don't need add */
		}
	}
	list_add(&error_new->list, &event_queue->error_list.list); /* add new event to queue */
	event_queue->error_num++;
	mutex_unlock(&event_queue->mutex);

	return 0;
}

STATIC unsigned int *memory_get_fault_table(struct ras_fault_converge_item *fault_info, int *len)
{
	if (fault_info->sensor_type == DMS_SEN_TYPE_MEMORY) {
		*len = sizeof(g_fault_event_record_mem_type_table) / sizeof(unsigned int);
		return g_fault_event_record_mem_type_table;
	} else if (fault_info->sensor_type == DMS_SEN_TYPE_RAS_SENSOR) {
		*len = sizeof(g_fault_event_record_ras_type_table) / sizeof(unsigned int);
		return g_fault_event_record_ras_type_table;
	} else if (fault_info->sensor_type == DMS_SEN_TYPE_TEMPERATURE) {
		*len = sizeof(g_fault_event_record_tem_type_table) / sizeof(unsigned int);
		return g_fault_event_record_tem_type_table;
	} else {
		memory_drv_err("Invalid para. (sensor_type=%u)\n", fault_info->sensor_type);
		return NULL;
	}
}

STATIC int memory_clear_fault_event_check(struct ras_fault_converge_item *fault_info)
{
	int i, len;
	unsigned int *p_table = NULL;

	if (fault_info == NULL) {
		memory_drv_err("Invalid para, fault_info is NULL.\n");
		return -EINVAL;
	}

	p_table = memory_get_fault_table(fault_info, &len);
	if (p_table == NULL) {
		return -EINVAL;
	}

	for (i = 0; i < len; i++) {
		if (fault_info->error_type == p_table[i]) {
			return 1; /* 1 means found fault event type in record table, don't need clear */
		}
	}

	return 0;
}

STATIC unsigned int memory_get_section_ras_code(struct ras_fault_converge_item *fault_info)
{
	unsigned int ras_code;
	switch (fault_info->section_type) {
		case RAS_SEC_OEM:
			ras_code = fault_info->ras_code.err_status & MEMORY_MASK_32BIT;
			break;
		case RAS_SEC_MEM:
		case RAS_SEC_GENERIC:
			ras_code = fault_info->ras_code.err_type;
			break;
		default:
			ras_code = fault_info->ras_code.int_status;
			break;
	}

	return ras_code;
}

STATIC struct ras_fault_converge_item *memory_get_memory_table(unsigned int idx, struct memory_fault_event *event)
{
	if ((event->sec_type == g_memory_fault_converge_table[idx].section_type) &&
		(event->sensor_id == g_memory_fault_converge_table[idx].module_id)) {
		if (event->ras_err_type == memory_get_section_ras_code(&g_memory_fault_converge_table[idx])) {
			return &g_memory_fault_converge_table[idx];
		}
	}

	return NULL;
}

STATIC struct ras_fault_converge_item *memory_parse_fault_table(struct memory_fault_event *event)
{
	unsigned int i;
	struct ras_fault_converge_item *fault_info = NULL;

	if (event == NULL) {
		memory_drv_err("Invalid para, event is NULL.\n");
		return NULL;
	}

	for (i = 0; i < sizeof(g_memory_fault_converge_table) / sizeof(struct ras_fault_converge_item); i++) {
		fault_info = memory_get_memory_table(i, event);
		if (fault_info != NULL) {
			return fault_info;
		}
	}
	return NULL;
}

STATIC int memory_check_nullptr(struct dms_sensor_event_data *data)
{
	if ((data == NULL) || (g_memory_devs_ptr == NULL)) {
		memory_drv_err("Invalid para. (data=%d; g_memory_devs_ptr=%d)\n", (data == NULL), (g_memory_devs_ptr == NULL));
		return -EINVAL;
	}

	return 0;
}

STATIC int memory_check_sensor_id(unsigned int sensor_id)
{
	if (sensor_id >= MEMORY_SENSOR_ID_MAX) {
		memory_drv_err("Invalid para. (s_id=%u)\n", sensor_id);
		return -EINVAL;
	}

	return 0;
}

STATIC int memory_check_hbmc_sensor(unsigned int sensor_id, unsigned int idx)
{
	if ((sensor_id == MEMORY_SENSOR_HBMC) && (idx >= HBM_SENSOR_NUM)) {
		memory_drv_err("Invalid para. (s_id=%u; idx=%u)\n", sensor_id, idx);
		return -EINVAL;
	}

	return 0;
}

STATIC int memory_check_ddrc_sensor(unsigned int sensor_id, unsigned int idx)
{
	if ((sensor_id == MEMORY_SENSOR_DDRC) && (idx >= DDR_SENSOR_NUM)) {
		memory_drv_err("Invalid para. (s_id=%u; idx=%u)\n", sensor_id, idx);
		return -EINVAL;
	}

	return 0;
}

STATIC int memory_check_hha_sensor(unsigned int sensor_id, unsigned int idx)
{
	if ((sensor_id == MEMORY_SENSOR_HHA) && (idx >= HHA_SENSOR_NUM)) {
		memory_drv_err("Invalid para. (s_id=%u; idx=%u)\n", sensor_id, idx);
		return -EINVAL;
	}

	return 0;
}

STATIC int memory_check_sensor_idx(unsigned int sensor_id, unsigned int idx)
{
	int ret;

	ret = memory_check_hbmc_sensor(sensor_id, idx);
	if (ret != 0) {
		return ret;
	}

	ret = memory_check_ddrc_sensor(sensor_id, idx);
	if (ret != 0) {
		return ret;
	}

	ret = memory_check_hha_sensor(sensor_id, idx);
	if (ret != 0) {
		return ret;
	}

	return 0;
}

STATIC int memory_fault_event_check_param(unsigned int sensor_id, unsigned int idx, struct dms_sensor_event_data *data)
{
	int ret;

	ret = memory_check_nullptr(data);
	if (ret != 0) {
		return -EINVAL;
	}

	ret = memory_check_sensor_id(sensor_id);
	if (ret != 0) {
		return -EINVAL;
	}

	ret = memory_check_sensor_idx(sensor_id, idx);
	if (ret != 0) {
		return -EINVAL;
	}

	return 0;
}

STATIC int memory_get_dms_sensor_event(struct ras_fault_converge_item *fault_info, struct dms_sensor_event_data *data)
{
	int ret;

	data->sensor_data[data->event_count].current_value = fault_info->error_type;
	data->sensor_data[data->event_count].data_size = sizeof(fault_info->describe);

	ret = memcpy_s(data->sensor_data[data->event_count].event_data, DMS_MAX_EVENT_DATA_LENGTH,
		fault_info->describe, sizeof(fault_info->describe));
	if (ret != 0) {
		memory_drv_warn("Memcpy_s failed, continue. (ret=%d)\n", ret);
		return ret;
	}

	return 0;
}

STATIC int memory_fault_event_parse(struct memory_error_list *pos, struct dms_sensor_event_data *data,
	struct memory_event *event_queue)
{
	int ret;
	struct ras_fault_converge_item *fault_info = NULL;

	fault_info = memory_parse_fault_table(&pos->error);
	if (fault_info == NULL) {
		return -EINVAL;
	}

	ret = memory_get_dms_sensor_event(fault_info, data);
	if (ret != 0) {
		return ret;
	}

	ret = memory_clear_fault_event_check(fault_info);
	if (ret == 0) {
		list_del(&pos->list);
		kfree(pos);
		pos = NULL;
		event_queue->error_num--;
	}

	return 0;
}

int memory_fault_event_scan(unsigned long long private_data, struct dms_sensor_event_data *data)
{
	int ret;
	unsigned int die_id, sensor_id, idx;
	struct memory_error_list *pos = NULL;
	struct memory_error_list *n = NULL;
	struct memory_event *event_queue = NULL;
	die_id = (private_data >> MEMORY_OFFSET_32BIT) & MEMORY_MASK_32BIT;
	sensor_id = (private_data & MEMORY_MASK_32BIT) >> MEMORY_OFFSET_16BIT;
	idx = private_data & MEMORY_MASK_16BIT;

	ret = memory_fault_event_check_param(sensor_id, idx, data);
	if (ret != 0) {
		return -EINVAL;
	}

 	/* get hbm|ddr|hha sensor event queue */
	event_queue = &g_memory_devs_ptr->m_dev[die_id][sensor_id].sensor_event_queue[idx];
	data->event_count = 0;
	mutex_lock(&event_queue->mutex);
	/* get every event of the sensor event queue */
	list_for_each_entry_safe(pos, n, &event_queue->error_list.list, list) {
		ret = memory_fault_event_parse(pos, data, event_queue);
		if (ret != 0) {
			continue;
		}

		data->event_count++;
		if (data->event_count == DMS_MAX_SENSOR_EVENT_COUNT) {
			break;
		}
	}
	mutex_unlock(&event_queue->mutex);

	return 0;
}

STATIC void memory_set_ddr_hha_sensor_id(struct memory_fault_event *event)
{
	event->sensor_id = MEMORY_SENSOR_DHA;
	event->sensor_idx = event->submodule_id;
}

STATIC void memory_set_hbm_hha_sensor_id(struct memory_fault_event *event)
{
	event->sensor_id = MEMORY_SENSOR_HHA;
	event->sensor_idx = (event->sec_type == RAS_SEC_MEM) ? S_IDX8 : (event->submodule_id - 0x01);
}

STATIC int memory_set_hha_sensor_id(struct memory_fault_event *event)
{
	if (event->submodule_id == 0) { /* HHA_DDR:0 */
		memory_set_ddr_hha_sensor_id(event);
	} else if ((event->submodule_id >= 0x01) && (event->submodule_id <= 0x08)) { /* HBM_HHA0~HHA7:0x01~0x08 */
		memory_set_hbm_hha_sensor_id(event);
	} else {
		memory_drv_err("Invalid hha submodule_id. (submodule_id=%u)\n", event->submodule_id);
		return -EINVAL;
	}

	return 0;
}

STATIC void memory_set_ddr_sensor_id(struct memory_fault_event *event)
{
	unsigned char idx;

	event->sensor_id = MEMORY_SENSOR_DDRC;
	idx = event->submodule_id;
	event->sensor_idx = (event->sec_type == RAS_SEC_MEM) ? (idx + DDR_SENSOR_MEM_START_IDX) : (idx);
}

STATIC void memory_set_hbm_sensor_id(struct memory_fault_event *event)
{
	unsigned char idx;

	event->sensor_id = MEMORY_SENSOR_HBMC;
	idx = (event->submodule_id - HBM0_HBMC00_SUB_MODULE) / 16; /* 16 submodule/hbmc */
	event->sensor_idx = (event->sec_type == RAS_SEC_MEM) ? (idx + HBM_SENSOR_MEM_START_IDX) : (idx);
#ifdef CFG_HBM_FAULT_EVENT
	if ((event->ras_err_type == RAS_UEU_FATAL_HIGH_TEMPER_INT) ||
		(event->ras_err_type == RAS_UEU_HIGH_TEMPER_INT)) {
		event->sensor_idx = S_IDX10;
	}
#endif
}

STATIC int memory_set_hbm_ddr_sensor_id(struct memory_fault_event *event)
{
	if (event->submodule_id <= DDRC1_SUB_MODULE) {
		memory_set_ddr_sensor_id(event);
	} else if (event->submodule_id <= HBM3_HBMC15_SUB_MODULE) {
		memory_set_hbm_sensor_id(event);
	} else {
		memory_drv_err("Invalid hbm/ddr submodule_id. (submodule_id=%u)\n", event->submodule_id);
		return -EINVAL;
	}

	return 0;
}

STATIC int memory_set_sensor_id(struct memory_fault_event *event)
{
	if (event->module_id == RAS_HHA_MODULE) {
		return memory_set_hha_sensor_id(event);
	} else {
		return memory_set_hbm_ddr_sensor_id(event);
	}
}

STATIC void memory_ras_parse_hisi_common(const struct notify_data *data, struct memory_fault_event *event)
{
	hisi_common_error_info *pdata_common = NULL;

	pdata_common = (hisi_common_error_info *)data->origin_data;
	if (pdata_common->module_id == HBMC_MODULE) {
		event->module_id = RAS_MEMORY_MODULE;
		event->submodule_id = get_common_mem_submodule(pdata_common->sub_sys_num, pdata_common->device_or_core_id);
	} else if (pdata_common->module_id == HBMA_MODULE) {
		event->module_id = RAS_HHA_MODULE;
		event->submodule_id = pdata_common->sub_module_id + 0x01;
	}

	/* IERR(bits 8~15 are vaild) will be uploaded as the 4th member of the register list */
	event->ras_err_type = (pdata_common->register_array[4] & MEMORY_MASK_16BIT) >> 8;
	event->sec_type = RAS_SEC_GENERIC;

	/* voluntarily modify event's sec_type to match sensor_type When IERR is 0x17(dmc_rdata_mirror_error) */
#ifdef CFG_HBM_FAULT_EVENT
	if (event->ras_err_type == RAS_DMC_RDATA_MIRROR_ERROR) {
		event->sec_type = RAS_SEC_MEM;
	}
#endif
}

STATIC int memory_ras_parse_section_type(const struct notify_data *data, struct memory_fault_event *event)
{
	struct cper_sec_mem_err *pdata_mem = NULL;
	struct sec_oem_error *pdata_oem = NULL;

	if (guid_equal(data->section_type, &CPER_SEC_PLATFORM_MEM)) {
		pdata_mem = (struct cper_sec_mem_err *)data->origin_data;
		event->module_id = RAS_MEMORY_MODULE;
		event->submodule_id = pdata_mem->module;
		event->ras_err_type = pdata_mem->error_type;
		event->sec_type = RAS_SEC_MEM;
	} else if (guid_equal(data->section_type, &CPER_SEC_HISI_OEM_2)) {
		pdata_oem = (struct sec_oem_error *)data->origin_data;
		event->module_id = pdata_oem->module_id;
		event->submodule_id = pdata_oem->submodule_id;
		event->ras_err_type = pdata_oem->err_status_l & MEMORY_MASK_16BIT; /* err_status: 0~15 bit */
		event->sec_type = RAS_SEC_OEM;
	} else if (guid_equal(data->section_type, &CPER_SEC_HISI_COMMON)) {
		memory_ras_parse_hisi_common(data, event);
	} else {
		memory_drv_warn("Invalid sec_type.\n");
		return -EINVAL;
	}

	return 0;
}

STATIC int memory_ras_data_parse(const struct notify_data *data, struct memory_fault_event *event)
{
	int ret;

	ret = memory_ras_parse_section_type(data, event);
	if (ret != 0) {
		memory_drv_warn("Unknown sec_type.\n");
		return -EINVAL;
	}

	event->device_id = data->chip_id;

	if (memory_set_sensor_id(event) != 0) {
		memory_drv_err("Unknow fault, (device_id=%u; submodule_id=%u; sec_type=%u; ras_err_type=0x%x)\n",
			event->device_id, event->submodule_id, event->sec_type, event->ras_err_type);
		return -EINVAL;
	}

	if (memory_parse_fault_table(event) == NULL) {
		memory_drv_warn("Fault not in table, (device_id=%u; sensor_id=%u; sec_type=%u; ras_err_type=0x%x)\n",
			event->device_id, event->sensor_id, event->sec_type, event->ras_err_type);
		return -EINVAL;
	}

	return 0;
}

int memory_oem_fault_event_handler(struct memory_fault_event *event)
{
	int ret;
	struct memory_dev *node = NULL;
	struct memory_event *event_queue = NULL;
	struct memory_error_list *error_new = NULL;

	node = &g_memory_devs_ptr->m_dev[event->device_id][event->sensor_id];
	event_queue = &node->sensor_event_queue[event->sensor_idx]; /* get event queue list head */

	error_new = kzalloc(sizeof(struct memory_error_list), GFP_KERNEL | __GFP_ACCOUNT);
	if (error_new == NULL) {
		memory_drv_err("New_event kzalloc failed.\n");
		return -ENOMEM;
	}

	ret = memcpy_s(&error_new->error, sizeof(struct memory_fault_event), event, sizeof(struct memory_fault_event));
	if (ret != 0) {
		memory_kfree(&error_new);
		memory_drv_err("Memcpy_s failed. (ret=%d; die_id=%u; sensor_id=%u; sec_type=%u; ras_err_type=0x%x)\n",
			ret, event->device_id, event->sensor_id, event->sec_type, event->ras_err_type);
		return ret;
	}

	ret = memory_add_fault_event(error_new, event_queue);
	if (ret != 0) {
		memory_kfree(&error_new);
		memory_drv_event("Fault event already exist. (die_id=%u; sensor_id=%u; sec_type=%u; ras_err_type=0x%x)\n",
			event->device_id, event->sensor_id, event->sec_type, event->ras_err_type);
		return ret;
	}

	return 0;
}

STATIC int memory_is_hbm_fault(struct memory_fault_event *event)
{
	if (event->module_id == RAS_MEMORY_MODULE) {
		if ((event->submodule_id >= HBM0_HBMC00_SUB_MODULE) && (event->submodule_id <= HBM3_HBMC15_SUB_MODULE)) {
			return 1;
		}
	}

	return 0;
}

STATIC int memory_handler_check_param(const struct notify_data *pdata)
{
	if ((pdata == NULL) || (g_memory_devs_ptr == NULL) || (pdata->chip_id >= memory_dev_num(g_memory_devs_ptr->dev_num))) {
		memory_drv_err("Invalid para. (data=%d; hbm_ctrl=%d)\n", (pdata == NULL), (g_memory_devs_ptr == NULL));
		return -EINVAL;
	}

	return 0;
}

#ifdef SUPPORT_HBM_ISOLATION
STATIC void memory_handler_multi_ecc_record(const struct notify_data *pdata)
{
	if (guid_equal(pdata->section_type, &CPER_SEC_PLATFORM_MEM)) {
		if (pdata->data_len >= sizeof(struct cper_sec_mem_err)) {
			int ret = hbm_multi_ecc_record((struct cper_sec_mem_err *)pdata->origin_data);
			if (ret != 0) {
				memory_drv_err("HBM handler mem data failed, (ret=%d).\n", ret);
			}
		}
	}
}
#endif

void memory_fault_event_handler(const struct notify_data *pdata)
{
	int ret;
	struct memory_fault_event event = {0};

	ret = memory_handler_check_param(pdata);
	if (ret != 0) {
		return;
	}

	if (memory_ras_data_parse(pdata, &event) == 0) {
		(void)memory_oem_fault_event_handler(&event);
	}

#ifdef SUPPORT_HBM_ISOLATION
	memory_handler_multi_ecc_record(pdata);
#endif

	if (memory_is_hbm_fault(&event)) {
		(void)hbm_relieve_suppresion(pdata->chip_id, event.submodule_id);
	}

#ifdef CFG_MEMORY_DDR
	(void)ddrc_relieve_suppresion(&event);
#endif
}

int hbm_get_dev_capacity(struct dms_node *device, unsigned long long *capacity)
{
	if ((device == NULL) || (capacity == NULL)) {
		memory_drv_err("Invalid para. (device=%d; capacity=%d)\n", (device == NULL), (capacity == NULL));
		return -EINVAL;
	}

	*capacity = device->capacity;

	return 0;
}

int hbm_get_dev_info_list(struct dms_node *device, struct dms_dev_data_attr *info_list)
{
	return 0;
}

int hbm_get_dev_state(struct dms_node *device, unsigned int *state)
{
	if ((device == NULL) || (state == NULL)) {
		memory_drv_err("Invalid para. (device=%d; state=%d)\n", (device == NULL), (state == NULL));
		return -EINVAL;
	}

	*state = device->state;

	return 0;
}

int hbm_set_dev_power_state(struct dms_node *device, DSMI_POWER_STATE power_state)
{
	return 0;
}

int ddr_get_dev_capacity(struct dms_node *device, unsigned long long *capacity)
{
	if ((device == NULL) || (capacity == NULL)) {
		memory_drv_err("Invalid para. (device=%d; capacity=%d)\n", (device == NULL), (capacity == NULL));
		return -EINVAL;
	}

	*capacity = device->capacity;

	return 0;
}

int ddr_get_dev_info_list(struct dms_node *device, struct dms_dev_data_attr *info_list)
{
	return 0;
}

int ddr_get_dev_state(struct dms_node *device, unsigned int *state)
{
	if ((device == NULL) || (state == NULL)) {
		memory_drv_err("Invalid para. (device=%d; state=%d)\n", (device == NULL), (state == NULL));
		return -EINVAL;
	}

	*state = device->state;

	return 0;
}

int ddr_set_dev_power_state(struct dms_node *device, DSMI_POWER_STATE power_state)
{
	return 0;
}

int hbm_ops_init(struct dms_node *device)
{
	return 0;
}

void hbm_ops_exit(struct dms_node *device)
{
	return;
}

int ddr_ops_init(struct dms_node *device)
{
	return 0;
}

void ddr_ops_exit(struct dms_node *device)
{
	return;
}

int dha_get_dev_info_list(struct dms_node *device, struct dms_dev_data_attr *info_list)
{
	return 0;
}

int dha_get_dev_state(struct dms_node *device, unsigned int *state)
{
	if ((device == NULL) || (state == NULL)) {
		memory_drv_err("Invalid para. (device=%d; state=%d)\n", (device == NULL), (state == NULL));
		return -EINVAL;
	}

	*state = device->state;

	return 0;
}

int dha_get_dev_capacity(struct dms_node *device, unsigned long long *capacity)
{
	if ((device == NULL) || (capacity == NULL)) {
		memory_drv_err("Invalid para. (device=%d; capacity=%d)\n", (device == NULL), (capacity == NULL));
		return -EINVAL;
	}

	*capacity = device->capacity;

	return 0;
}

int dha_set_dev_power_state(struct dms_node *device, DSMI_POWER_STATE power_state)
{
	return 0;
}

int dha_ops_init(struct dms_node *device)
{
	return 0;
}

void dha_ops_exit(struct dms_node *device)
{
	return;
}

int hha_get_dev_state(struct dms_node *device, unsigned int *state)
{
	if ((device == NULL) || (state == NULL)) {
		memory_drv_err("Invalid para. (device=%d; state=%d)\n", (device == NULL), (state == NULL));
		return -EINVAL;
	}

	*state = device->state;

	return 0;
}

int hha_get_dev_capacity(struct dms_node *device, unsigned long long *capacity)
{
	if ((device == NULL) || (capacity == NULL)) {
		memory_drv_err("Invalid para. (device=%d; capacity=%d)\n", (device == NULL), (capacity == NULL));
		return -EINVAL;
	}

	*capacity = device->capacity;

	return 0;
}

int hha_get_dev_info_list(struct dms_node *device, struct dms_dev_data_attr *info_list)
{
	return 0;
}

int hha_set_dev_power_state(struct dms_node *device, DSMI_POWER_STATE power_state)
{
	return 0;
}

int hha_ops_init(struct dms_node *device)
{
	return 0;
}

void hha_ops_exit(struct dms_node *device)
{
	return;
}

void memory_sensor_obj_init(struct memory_dev *dev)
{
	unsigned int i;

	for (i = 0; i < dev->sensor_obj_num; i++) {
		mutex_init(&dev->sensor_event_queue[i].mutex);
		INIT_LIST_HEAD(&dev->sensor_event_queue[i].error_list.list);
	}

	return;
}

STATIC int memory_ctrl_init(void)
{
	int ret;
	unsigned int i;
	unsigned int len = sizeof(struct dms_sensor_object_cfg);

	g_memory_devs_ptr = memory_get_devices();

	ret = devdrv_get_devnum(&g_memory_devs_ptr->dev_num);
	if (ret != 0) {
		memory_drv_err("Get dev_num failed. (ret=%d)\n", ret);
		return ret;
	}

	for (i = 0; i < memory_dev_num(g_memory_devs_ptr->dev_num); i++) {
		g_memory_devs_ptr->m_dev[i][MEMORY_SENSOR_HBMC].die_id = i;
		g_memory_devs_ptr->m_dev[i][MEMORY_SENSOR_HBMC].dev_node = &g_memory_dev_node_table[i][MEMORY_SENSOR_HBMC];
		g_memory_devs_ptr->m_dev[i][MEMORY_SENSOR_HBMC].sensor_obj_num = sizeof(g_hbm_sensor_table[i]) / len;
		g_memory_devs_ptr->m_dev[i][MEMORY_SENSOR_HBMC].sensor_obj_table = g_hbm_sensor_table[i];
		g_memory_devs_ptr->m_dev[i][MEMORY_SENSOR_HBMC].notify_func = memory_fault_event_handler;
		INIT_DELAYED_WORK(&g_memory_devs_ptr->m_dev[i][MEMORY_SENSOR_HBMC].unmask_irq_work, NULL);
		memory_sensor_obj_init(&g_memory_devs_ptr->m_dev[i][MEMORY_SENSOR_HBMC]);

		g_memory_devs_ptr->m_dev[i][MEMORY_SENSOR_DDRC].die_id = i;
		g_memory_devs_ptr->m_dev[i][MEMORY_SENSOR_DDRC].dev_node = &g_memory_dev_node_table[i][MEMORY_SENSOR_DDRC];
		g_memory_devs_ptr->m_dev[i][MEMORY_SENSOR_DDRC].sensor_obj_num = sizeof(g_ddr_sensor_table[i]) / len;
		g_memory_devs_ptr->m_dev[i][MEMORY_SENSOR_DDRC].sensor_obj_table = g_ddr_sensor_table[i];
		g_memory_devs_ptr->m_dev[i][MEMORY_SENSOR_DDRC].notify_func = memory_fault_event_handler;
#ifdef CFG_MEMORY_DDR
		INIT_DELAYED_WORK(&g_memory_devs_ptr->m_dev[i][MEMORY_SENSOR_DDRC].unmask_irq_work, memory_ddrc_fault_unmask_task);
#else
		INIT_DELAYED_WORK(&g_memory_devs_ptr->m_dev[i][MEMORY_SENSOR_DDRC].unmask_irq_work, NULL);
#endif
		memory_sensor_obj_init(&g_memory_devs_ptr->m_dev[i][MEMORY_SENSOR_DDRC]);

		g_memory_devs_ptr->m_dev[i][MEMORY_SENSOR_HHA].die_id = i;
		g_memory_devs_ptr->m_dev[i][MEMORY_SENSOR_HHA].dev_node = &g_memory_dev_node_table[i][MEMORY_SENSOR_HHA];
		g_memory_devs_ptr->m_dev[i][MEMORY_SENSOR_HHA].sensor_obj_num = sizeof(g_hha_sensor_table[i]) / len;
		g_memory_devs_ptr->m_dev[i][MEMORY_SENSOR_HHA].sensor_obj_table = g_hha_sensor_table[i];
		g_memory_devs_ptr->m_dev[i][MEMORY_SENSOR_HHA].notify_func = memory_fault_event_handler;
		INIT_DELAYED_WORK(&g_memory_devs_ptr->m_dev[i][MEMORY_SENSOR_HHA].unmask_irq_work, NULL);
		memory_sensor_obj_init(&g_memory_devs_ptr->m_dev[i][MEMORY_SENSOR_HHA]);

		g_memory_devs_ptr->m_dev[i][MEMORY_SENSOR_DHA].die_id = i;
		g_memory_devs_ptr->m_dev[i][MEMORY_SENSOR_DHA].dev_node = &g_memory_dev_node_table[i][MEMORY_SENSOR_DHA];
		g_memory_devs_ptr->m_dev[i][MEMORY_SENSOR_DHA].sensor_obj_num = sizeof(g_dha_sensor_table[i]) / len;
		g_memory_devs_ptr->m_dev[i][MEMORY_SENSOR_DHA].sensor_obj_table = g_dha_sensor_table[i];
		g_memory_devs_ptr->m_dev[i][MEMORY_SENSOR_DHA].notify_func = memory_fault_event_handler;
		INIT_DELAYED_WORK(&g_memory_devs_ptr->m_dev[i][MEMORY_SENSOR_DHA].unmask_irq_work, NULL);
		memory_sensor_obj_init(&g_memory_devs_ptr->m_dev[i][MEMORY_SENSOR_DHA]);
	}

	return 0;
}

STATIC void memory_ctrl_exit(void)
{
	unsigned int i, j;

	for (i = 0; i < memory_dev_num(g_memory_devs_ptr->dev_num); i++) {
		for (j = 0; j < g_memory_devs_ptr->m_dev[i][MEMORY_SENSOR_HBMC].sensor_obj_num; j++) {
			mutex_destroy(&g_memory_devs_ptr->m_dev[i][MEMORY_SENSOR_HBMC].sensor_event_queue[j].mutex);
		}
		for (j = 0; j < g_memory_devs_ptr->m_dev[i][MEMORY_SENSOR_DDRC].sensor_obj_num; j++) {
			mutex_destroy(&g_memory_devs_ptr->m_dev[i][MEMORY_SENSOR_DDRC].sensor_event_queue[j].mutex);
		}
		for (j = 0; j < g_memory_devs_ptr->m_dev[i][MEMORY_SENSOR_HHA].sensor_obj_num; j++) {
			mutex_destroy(&g_memory_devs_ptr->m_dev[i][MEMORY_SENSOR_HHA].sensor_event_queue[j].mutex);
		}
		for (j = 0; j < g_memory_devs_ptr->m_dev[i][MEMORY_SENSOR_DHA].sensor_obj_num; j++) {
			mutex_destroy(&g_memory_devs_ptr->m_dev[i][MEMORY_SENSOR_DHA].sensor_event_queue[j].mutex);
		}
	}

	return;
}

STATIC int memory_dev_node_init(void)
{
	int i, k, ret;
	unsigned int max_support_type;
	struct memory_dev *m_dev = NULL;

	max_support_type = sizeof(support_fault_type) / sizeof(enum memory_sensor_id);
	for (i = 0; i < memory_dev_num(g_memory_devs_ptr->dev_num); i++) {
		for (k = 0; k < max_support_type; k++) {
			m_dev = &g_memory_devs_ptr->m_dev[i][support_fault_type[k]]; // get each device each sensor dev_node
			ret = memory_register_one_node(m_dev);
			if (ret != 0) {
				memory_drv_err("MEM register one node failed. (devnode=%d; s_id=%d; ret=%d)\n", i, k, ret);
				goto out;
			}
		}
	}

	return 0;
out:
	while (i >= 0) {
		while ((--k) >= 0) {
			memory_unregister_one_node(&g_memory_devs_ptr->m_dev[i][support_fault_type[k]]);
		}
		i--;
	}
	return ret;
}

STATIC void memory_dev_node_exit(void)
{
	unsigned int i, j, k;
	unsigned int max_support_type;

	max_support_type = sizeof(support_fault_type) / sizeof(enum memory_sensor_id);
	for (i = 0; i < memory_dev_num(g_memory_devs_ptr->dev_num); i++) {
		for (k = 0; k < max_support_type; k++) {
			(void)dms_sensor_node_unregister(g_memory_devs_ptr->m_dev[i][support_fault_type[k]].dev_node);
			(void)dms_unregister_dev_node(g_memory_devs_ptr->m_dev[i][support_fault_type[k]].dev_node);
			for (j = 0; j < g_memory_devs_ptr->m_dev[i][support_fault_type[k]].sensor_obj_num; j++) {
				memory_fault_event_free(&g_memory_devs_ptr->m_dev[i][support_fault_type[k]].sensor_event_queue[j]);
			}
		}
	}

	return;
}

STATIC int memory_register_fpdc(void)
{
	int ret;

	ret = fpdc_register_fault_notifier(DMS_DEV_TYPE_HBM, memory_fault_event_handler);
	if (ret != 0) {
		memory_drv_err("MEM register hbm fpdc fault notifier failed, (ret=%d)\n", ret);
		return ret;
	}
	ret = fpdc_register_fault_notifier(DMS_DEV_TYPE_HBMA, memory_fault_event_handler);
	if (ret != 0) {
		memory_drv_err("MEM register hha fpdc fault notifier failed, (ret=%d)\n", ret);
		goto ERR_REG_HBMA;
	}
#ifdef CFG_MEMORY_DDR
	ret = fpdc_register_fault_notifier(DMS_DEV_TYPE_DDR, memory_fault_event_handler);
	if (ret != 0) {
		memory_drv_err("MEM register ddr fpdc fault notifier failed, (ret=%d)\n", ret);
		goto ERR_REG_DDR;
	}
	ret = fpdc_register_fault_notifier(DMS_DEV_TYPE_DDRA, memory_fault_event_handler);
	if (ret != 0) {
		memory_drv_err("MEM register dha fpdc fault notifier failed, (ret=%d)\n", ret);
		goto ERR_REG_DDRA;
	}
#endif
	return 0;
#ifdef CFG_MEMORY_DDR
ERR_REG_DDRA:
	(void)fpdc_unregister_fault_notifier(DMS_DEV_TYPE_DDR);
ERR_REG_DDR:
	(void)fpdc_unregister_fault_notifier(DMS_DEV_TYPE_HBMA);
#endif
ERR_REG_HBMA:
	(void)fpdc_unregister_fault_notifier(DMS_DEV_TYPE_HBM);
	return ret;
}

STATIC void memory_unregister_fpdc(void)
{
	int ret;

	ret = fpdc_unregister_fault_notifier(DMS_DEV_TYPE_DDRA);
	if (ret != 0) {
		memory_drv_warn("MEM unregister dha fpdc fault notifier failed, (ret=%d)\n", ret);
	}

	ret = fpdc_unregister_fault_notifier(DMS_DEV_TYPE_HBMA);
	if (ret != 0) {
		memory_drv_warn("MEM unregister hha fpdc fault notifier failed, (ret=%d)\n", ret);
	}

	ret = fpdc_unregister_fault_notifier(DMS_DEV_TYPE_DDR);
	if (ret != 0) {
		memory_drv_warn("MEM unregister ddr fpdc fault notifier failed, (ret=%d)\n", ret);
	}

	ret = fpdc_unregister_fault_notifier(DMS_DEV_TYPE_HBM);
	if (ret != 0) {
		memory_drv_warn("MEM unregister hbm fpdc fault notifier failed, (ret=%d)\n", ret);
	}

	return;
}

STATIC int memory_init_notifier(struct notifier_block *nb, unsigned long mode, void *data)
{
	return 0;
}

STATIC struct notifier_block g_memory_notifier = {
	.notifier_call = memory_init_notifier,
};
int memory_safety_ras_init(void)
{
	int ret;

	ret = memory_ctrl_init();
	if (ret != 0) {
		memory_drv_err("MEM ctrl init failed. (ret=%d)\n", ret);
		return ret;
	}

#ifdef SUPPORT_HBM_ISOLATION
	ret = hbm_ecc_isolation_init();
	if (ret != 0) {
		memory_drv_err("HBM ecc isolation init failed. (ret=%d)\n", ret);
		goto CTRL_EXIT;
	}
#endif

	ret = dms_register_notifier(&g_memory_notifier);
	if (ret != 0) {
		memory_drv_err("MEM register dms notifier failed. (ret=%d)\n", ret);
		goto HBM_ISOLATION_EXIT;
	}

	ret = memory_dev_node_init();
	if (ret != 0) {
		memory_drv_err("MEM dev_node init failed. (ret=%d)\n", ret);
		goto NOTIFIER_EXIT;
	}

	ret = memory_register_fpdc();
	if (ret != 0) {
		memory_drv_err("MEM register fpdc fault notifier failed, (ret=%d)\n", ret);
		goto FPDC_EXIT;
	}

	memory_drv_info("MEM driver init success. (subsys_id=0x%x)\n", g_memory_fault_converge_table[0].subsys_id);

	return 0;
FPDC_EXIT:
	memory_dev_node_exit();
NOTIFIER_EXIT:
	(void)dms_unregister_notifier(&g_memory_notifier);
HBM_ISOLATION_EXIT:
#ifdef SUPPORT_HBM_ISOLATION
	hbm_ecc_isolation_exit();
CTRL_EXIT:
#endif
	memory_ctrl_exit();
	return ret;
}

void memory_safety_ras_exit(void)
{
	memory_unregister_fpdc();
	memory_dev_node_exit();
	(void)dms_unregister_notifier(&g_memory_notifier);
#ifdef SUPPORT_HBM_ISOLATION
	hbm_ecc_isolation_exit();
#endif
	memory_ctrl_exit();

	memory_drv_info("MEM event driver exit success. (subsys_id=0x%x)\n", g_memory_fault_converge_table[0].subsys_id);
}

