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
#include "hbm_isolation_common.h"
#include <linux/slab.h>
#include <linux/mutex.h>
#include "linux/securec.h"
#include <linux/errno.h>
#include <linux/io.h>
#include "devdrv_dfm.h"
#include "memory_fault_init_cfg.h"
#include "drvmem_base_info.h"
#include "memory_log.h"
#include "dms_define.h"

STATIC struct ras_fault_converge_item g_hbm_multi_ecc_fault_table[] = {
	{
		.subsys_id = DMS_DEV_TYPE_HBM,
		.module_id = HBM_MEMORY_ERROR_RECORD_SENSOR,
		.section_type = RAS_SEC_MEM,
		.ras_code.err_type = STANDARD_ERR_TYPE_SCRUB_UNCORRECT,
		.describe = "ecc",
		.sensor_type = DMS_SEN_TYPE_MEMORY_ERR_RECORD,
		.error_type = HBM_NEW_MULTI_BITS_ECC
	},
	{
		.subsys_id = DMS_DEV_TYPE_HBM,
		.module_id = HBM_MEMORY_ERROR_RECORD_SENSOR,
		.section_type = RAS_SEC_MEM,
		.ras_code.err_type = STANDARD_ERR_TYPE_SCRUB_UNCORRECT,
		.describe = "multiple bits record exceeds 16",
		.sensor_type = DMS_SEN_TYPE_MEMORY_ERR_RECORD,
		.error_type = HBM_NOTICE_ECC_COUNT
	},
	{
		.subsys_id = DMS_DEV_TYPE_HBM,
		.module_id = HBM_MEMORY_ERROR_RECORD_SENSOR,
		.section_type = RAS_SEC_MEM,
		.ras_code.err_type = STANDARD_ERR_TYPE_SCRUB_UNCORRECT,
		.describe = "multiple bits record exceeds 64",
		.sensor_type = DMS_SEN_TYPE_MEMORY_ERR_RECORD,
		.error_type = HBM_UPPER_LIMITS_ECC_COUNT
	}
};

int hbm_add_ecc_event(struct new_multi_ecc_list *new_ecc_event)
{
	struct new_multi_ecc_list *pos = NULL;
	struct new_multi_ecc_list *n = NULL;
	struct ecc_config_tool_s *ecc_config_tool = hbm_get_ecc_config_tool();

	mutex_lock(&ecc_config_tool->multi_ecc_event_queue.mutex);
	if (new_ecc_event->error_type != HBM_NEW_MULTI_BITS_ECC) {
		list_for_each_entry_safe(pos, n, &ecc_config_tool->multi_ecc_event_queue.ecc_list.list, list) {
			if (pos->error_type == new_ecc_event->error_type) {
				mutex_unlock(&ecc_config_tool->multi_ecc_event_queue.mutex);
				return -EEXIST;
			}
		}

		list_add(&new_ecc_event->list, &ecc_config_tool->multi_ecc_event_queue.ecc_list.list);
	} else {
		list_add_tail(&new_ecc_event->list, &ecc_config_tool->multi_ecc_event_queue.ecc_list.list);
	}

	ecc_config_tool->multi_ecc_event_queue.error_num++;
	mutex_unlock(&ecc_config_tool->multi_ecc_event_queue.mutex);
	return OK;
}

STATIC struct ras_fault_converge_item *hbm_ecc_to_fault_table(uint32_t scan_die_id,
	struct new_multi_ecc_list *ecc_error)
{
	unsigned int i;
	struct ras_fault_converge_item *fault_info = NULL;
	if (ecc_error->dev_id != scan_die_id) {
		return NULL;
	}

	for (i = 0; i < sizeof(g_hbm_multi_ecc_fault_table) / sizeof(struct ras_fault_converge_item); i++) {
		if (ecc_error->error_type == g_hbm_multi_ecc_fault_table[i].error_type) {
			fault_info = &g_hbm_multi_ecc_fault_table[i];
			break;
		}
	}
	return fault_info;
}

int hbm_unisolated_multi_ecc_event_scan(unsigned long long private_data, struct dms_sensor_event_data *data)
{
	unsigned int die_id, sensor_id;
	int ret = 0;
	struct ecc_config_tool_s *ecc_config_tool = hbm_get_ecc_config_tool();

	die_id = (private_data >> MEMORY_OFFSET_32BIT) & MEMORY_MASK_32BIT;
	sensor_id = (private_data & MEMORY_MASK_32BIT) >> MEMORY_OFFSET_16BIT;
	if ((data == NULL) || (die_id >= MAX_OS_DEVICE_COUNT) || (sensor_id != HBM_MEMORY_SENSOR)) {
		memory_drv_err("Invalid para. (data=%d; die_id=%u; s_id=%u)\n", (data == NULL), die_id, sensor_id);
		return -EINVAL;
	}

	data->event_count = 0;
	mutex_lock(&ecc_config_tool->un_online_isolated_pfn_queue[die_id].mutex);
	if (!list_empty(&ecc_config_tool->un_online_isolated_pfn_queue[die_id].pfns.list)) {
		data->sensor_data[data->event_count].current_value = HBM_MEMORY_ERROR_UE_ECC_TYPE;
		ret = strncpy_s(data->sensor_data[data->event_count].event_data, DMS_MAX_EVENT_DATA_LENGTH,
			"appread new multi-bits ecc", DMS_MAX_EVENT_DATA_LENGTH - 1);
		if (ret != 0) {
			mutex_unlock(&ecc_config_tool->un_online_isolated_pfn_queue[die_id].mutex);
			memory_drv_warn("Memcpy failed. (ret=%d)\n", ret);
			return ret;
		}
		data->sensor_data[data->event_count].data_size = sizeof(data->sensor_data[data->event_count].event_data);
		data->event_count++;
	}
	mutex_unlock(&ecc_config_tool->un_online_isolated_pfn_queue[die_id].mutex);
	return ret;
}

STATIC void hbm_add_sensor_event_data(uint32_t dev_id, struct dms_sensor_event_data *data)
{
	int ret;
	struct new_multi_ecc_list *pos = NULL;
	struct new_multi_ecc_list *n = NULL;
	struct ras_fault_converge_item *fault_info = NULL;
	struct ecc_config_tool_s *ecc_config_tool = hbm_get_ecc_config_tool();

	data->event_count = 0;
	mutex_lock(&ecc_config_tool->multi_ecc_event_queue.mutex);
	/* get every event of the sensor event queue */
	list_for_each_entry_safe(pos, n, &ecc_config_tool->multi_ecc_event_queue.ecc_list.list, list) {
		fault_info = hbm_ecc_to_fault_table(dev_id, pos);
		if (fault_info == NULL) {
			continue;
		}

		data->sensor_data[data->event_count].current_value = fault_info->error_type;
		if (fault_info->error_type == HBM_NEW_MULTI_BITS_ECC) {
			ret = snprintf_s(data->sensor_data[data->event_count].event_data, DMS_MAX_EVENT_DATA_LENGTH,
				DMS_MAX_EVENT_DATA_LENGTH - 1, "%s 0x%016llx-%u", fault_info->describe, pos->physical_addr,
				pos->occupied_pid);
			if (ret < 0) {
				memory_drv_warn("Snprintf ecc data failed. (ret=%d; physical_addr=0x%llx; pid=%d\n",
					ret, pos->physical_addr, pos->occupied_pid);
				continue;
			}
			list_del(&pos->list);
			hbm_kfree((void **)&pos);
			ecc_config_tool->multi_ecc_event_queue.error_num--;
		} else {
			ret = memcpy_s(data->sensor_data[data->event_count].event_data, DMS_MAX_EVENT_DATA_LENGTH,
			fault_info->describe, sizeof(fault_info->describe));
			if (ret != 0) {
				memory_drv_warn("Memcpy failed. (ret=%d)\n", ret);
				continue;
			}
		}

		data->sensor_data[data->event_count].data_size = sizeof(data->sensor_data[data->event_count].event_data);
		data->event_count++;
		if (data->event_count == DMS_MAX_SENSOR_EVENT_COUNT) {
			break;
		}
	}
	mutex_unlock(&ecc_config_tool->multi_ecc_event_queue.mutex);
}

int hbm_multi_ecc_fault_event_scan(unsigned long long private_data, struct dms_sensor_event_data *data)
{
	unsigned int die_id, sensor_id;

	die_id = (private_data >> MEMORY_OFFSET_32BIT) & MEMORY_MASK_32BIT;
	sensor_id = (private_data & MEMORY_MASK_32BIT) >> MEMORY_OFFSET_16BIT;
	if ((data == NULL) || (die_id >= MAX_OS_DEVICE_COUNT) || (sensor_id != HBM_MEMORY_ERROR_RECORD_SENSOR)) {
		memory_drv_err("Invalid para. (data=%d; die_id=%u; s_id=%u)\n", (data == NULL), die_id, sensor_id);
		return -EINVAL;
	}

	hbm_add_sensor_event_data(die_id, data);
	return 0;
}

void hbm_add_statistics_event(uint32_t dev_id, uint32_t ecc_recorded_count, uint32_t except_id)
{
	int ret;
	struct new_multi_ecc_list *new_ecc_event = NULL;

	new_ecc_event = (struct new_multi_ecc_list *)kzalloc(sizeof(struct new_multi_ecc_list), GFP_KERNEL | __GFP_ACCOUNT);
	if (new_ecc_event == NULL) {
		memory_drv_err("new_event kzalloc failed.\n");
		return;
	}

	new_ecc_event->dev_id = dev_id;
	switch (except_id) {
		case ECC_REACH_64_WARNING:
			new_ecc_event->error_type = HBM_UPPER_LIMITS_ECC_COUNT;
			break;
		case ECC_EXCEED_16_WARNING:
			new_ecc_event->error_type = HBM_NOTICE_ECC_COUNT;
			break;
		default:
			memory_drv_info("device_id-%u current multi ecc recorded length is %u\n", dev_id, ecc_recorded_count);
			hbm_kfree((void **)&new_ecc_event);
			return;
	}

	ret = hbm_add_ecc_event(new_ecc_event);
	if (ret) {
		memory_drv_warn("Add new statistics event failed. (ret=%d; dev_id=%u; except_id=%u)\n",
			ret, dev_id, except_id);
		hbm_kfree((void **)&new_ecc_event);
	}
}

#ifdef CFG_HBM_FLASH
void hbm_check_statistic_to_bbox(uint8_t *ecc_config_data, uint32_t device_count, uint16_t statistics_info_offset)
{
	uint32_t device_id;
	struct ecc_isolated_statistics_record *multi_statistics_record = NULL;
	uint32_t recorded_count;

	for (device_id = 0; device_id < device_count; device_id++) {
		multi_statistics_record = (struct ecc_isolated_statistics_record *)(ecc_config_data +
		statistics_info_offset + device_id * ECC_FLASH_DIE_SIZE);  /* get multi_statistics_record */
		recorded_count = multi_statistics_record->hbm_multi_isolated_page_count;	/* update recorded_count */
		if (recorded_count >= NOTICE_MULTI_ECC_COUNT) {
			hbm_add_statistics_event(device_id, recorded_count, ECC_EXCEED_16_WARNING); /* add warning if reach 16 */
		}
		if (recorded_count >= ERROR_MULTI_ECC_COUNT) {
			hbm_add_statistics_event(device_id, recorded_count, ECC_REACH_64_WARNING);  /* add critical if reach 64 */
		}
	}
}
#else
void hbm_check_statistic_to_bbox(struct ecc_isolated_statistics_record *statistics_record, uint32_t device_count)
{
	uint32_t device_id;
	uint32_t total_recorded_count = 0;

	for (device_id = 0; device_id < device_count; device_id++) {
		uint32_t recorded_count = statistics_record[device_id].ddr_multi_isolated_page_count +
			statistics_record[device_id].hbm_multi_isolated_page_count;
		total_recorded_count += recorded_count;
		if (recorded_count >= NOTICE_MULTI_ECC_COUNT) {
			hbm_add_statistics_event(device_id, recorded_count, ECC_EXCEED_16_WARNING);
		}
	}

	if (total_recorded_count >= ERROR_MULTI_ECC_COUNT) {
		hbm_add_statistics_event(0, total_recorded_count, ECC_REACH_64_WARNING);
	}
}
#endif

int read_reg32(uint64_t addr, uint32_t *data)
{
	void *vir_addr = NULL;

	vir_addr = ioremap(addr, sizeof(uint32_t));
	if (vir_addr == NULL) {
		memory_drv_err("ioremap failed.\n");
		return -EINVAL;
	}
	*data = readl(vir_addr);
	iounmap(vir_addr);
	return OK;
}


int write_reg32(uint64_t addr, uint32_t data)
{
	void *vir_addr = NULL;

	vir_addr = ioremap(addr, sizeof(uint32_t));
	if (vir_addr == NULL) {
		memory_drv_err("ioremap failed.\n");
		return -EINVAL;
	}
	writel(data, vir_addr);
	iounmap(vir_addr);
	return OK;
}
