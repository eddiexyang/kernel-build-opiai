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
 * Create: 2022-01-20
 */

#include <linux/slab.h>
#include "dms_sensor.h"
#include "dms_sensor_type.h"
#include "dms_define.h"

#ifdef CFG_SOC_PLATFORM_MINIV3
#include "safety_ras_310/safety_ras_converge_310.h"
#else
#include "safety_ras_610/safety_ras_converge_610.h"
#endif

STATIC uint32_t g_recover_table[] = {
	MEMORY_SBRAM_1ERR,
	MEMORY_RBUF_SERR,
	MEMORY_WBUF_SERR,
	MEMORY_IECC_SERR,
	MEMORY_WSRAM_SERR,
	MEMORY_SBRAM_SERR,
	MEMORY_RPRAM_SERR,
	MEMORY_PHYRAM_SERR,
	MEMORY_UCERAM_SERR,
#ifndef CFG_SOC_PLATFORM_MINIV3
	DMC_UCERAM_SERR,
	DMC_PHYRAM_SERR,
	DMC_RPRAM_SERR,
	DMC_SBRAM_SERR,
	DMC_WSRAM_SERR,
	DMC_LECC_SERR,
	IECC_IECC_SERR,
	IECC_ADDRRAM_SERR,
	IECC_SBRAM_SERR,
	IECC_RBUF_SERR,
	IECC_WBUF_SERR,
	IECC_IECC_SERR_ADDR,
#endif
};

STATIC struct ras_fault_converge_item g_memory_fault_converge_table[] = {
	/* temputer fault converge */
	DMS_FAULT_CONVERGE_CONFIG(DMS_DEV_TYPE_DDR, MEMORY_SENSOR_DDRC, RAS_SEC_GENERIC, MEM_ERR_STATUS,
		DMS_SEN_TYPE_TEMPERATURE, TEMP_ERROR_TYPE_L1H_OCCUR, "Upper non-critical going high")
	DMS_FAULT_CONVERGE_CONFIG(DMS_DEV_TYPE_DDR, MEMORY_SENSOR_DDRC, RAS_SEC_GENERIC, MEM_ERR_STATUS,
		DMS_SEN_TYPE_TEMPERATURE, TEMP_ERROR_TYPE_L1L_OCCUR, "Upper non-critical going low") /* for 310 */
	/* soc_event fault converge */
	DMS_FAULT_CONVERGE_CONFIG(DMS_DEV_TYPE_DDR, MEMORY_SENSOR_DDRC, RAS_SEC_OEM, MEM_ERR_STATUS,
		DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_ERROR, "module error")
	DMS_FAULT_CONVERGE_CONFIG(DMS_DEV_TYPE_DDR, MEMORY_SENSOR_DDRC, RAS_SEC_OEM, MEM_ERR_STATUS,
		DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_ERROR_NF, "module error can not be fixed")
	DMS_FAULT_CONVERGE_CONFIG(DMS_DEV_TYPE_DDR, MEMORY_SENSOR_DDRC, RAS_SEC_OEM, MEM_ERR_STATUS,
		DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_INPUT_ERR, "input error")
	DMS_FAULT_CONVERGE_CONFIG(DMS_DEV_TYPE_DDR, MEMORY_SENSOR_DDRC, RAS_SEC_OEM, MEM_ERR_STATUS,
		DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR, "internal config error")
	DMS_FAULT_CONVERGE_CONFIG(DMS_DEV_TYPE_DDR, MEMORY_SENSOR_DDRC, RAS_SEC_OEM, MEM_ERR_STATUS,
		DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_CFG_ERR, "config error")
	DMS_FAULT_CONVERGE_CONFIG(DMS_DEV_TYPE_DDR, MEMORY_SENSOR_DDRC, RAS_SEC_OEM, MEM_ERR_STATUS,
		DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_PARITY, "parity error")
	DMS_FAULT_CONVERGE_CONFIG(DMS_DEV_TYPE_DDR, MEMORY_SENSOR_DDRC, RAS_SEC_OEM, MEM_ERR_STATUS,
		DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_SBECCOverThold, "secc error exc the threshold")
	DMS_FAULT_CONVERGE_CONFIG(DMS_DEV_TYPE_DDR, MEMORY_SENSOR_DDRC, RAS_SEC_OEM, MEM_ERR_STATUS,
		DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_SBECC_NC, "secc error not corrected")
	DMS_FAULT_CONVERGE_CONFIG(DMS_DEV_TYPE_DDR, MEMORY_SENSOR_DDRC, RAS_SEC_OEM, MEM_ERR_STATUS,
		DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_MBECC, "mutiple bit ecc error")
	DMS_FAULT_CONVERGE_CONFIG(DMS_DEV_TYPE_DDR, MEMORY_SENSOR_DDRC, RAS_SEC_OEM, MEM_ERR_STATUS,
		DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_BUS_ERR, "bus error")
	/* memory fault converge*/
	DMS_FAULT_CONVERGE_CONFIG(DMS_DEV_TYPE_DDR, MEMORY_SENSOR_DDRC, RAS_SEC_MEM, MEM_ERR_STATUS,
		DMS_SEN_TYPE_MEMORY, MEMORY_ERROR_TYPE_UCEECC, "Uncorrectable memory error")
	DMS_FAULT_CONVERGE_CONFIG(DMS_DEV_TYPE_DDR, MEMORY_SENSOR_DDRC, RAS_SEC_MEM, MEM_ERR_STATUS,
		DMS_SEN_TYPE_MEMORY, MEMORY_ERROR_TYPE_CECC_OVER_THOLD, "cecc logging limit reached")
	/* safety fault converge*/
	DMS_FAULT_CONVERGE_CONFIG(DMS_DEV_TYPE_DDR, MEMORY_SENSOR_DDRC, RAS_SEC_OEM, MEM_ERR_STATUS,
		DMS_SEN_TYPE_SAFETY_SENSOR, SOC_SAFETY_LOCKSTEP_ERR, "lockstep error")
	DMS_FAULT_CONVERGE_CONFIG(DMS_DEV_TYPE_DDR, MEMORY_SENSOR_DDRC, RAS_SEC_OEM, MEM_ERR_STATUS,
		DMS_SEN_TYPE_SAFETY_SENSOR, SOC_SAFETY_REPORT_TIMEOUT_MAJOR, "report timeout(major)")
	/* extend fault converge*/
	DMS_FAULT_CONVERGE_CONFIG(DMS_DEV_TYPE_DDR, MEMORY_SENSOR_DDRC, RAS_SEC_OEM, MEM_ERR_STATUS,
		DMS_SEN_TYPE_EXTEND_SENSOR, EXTEND_ERROR_TYPE_REG_ERR_READ_AFTRE_WRITER, "reg err after readback compare")
};

STATIC struct memory_event *memory_get_sensor_event_queue(uint32_t device_id, uint32_t sensor_id, uint32_t que_index)
{
	struct memory_dev *node = NULL;
	struct memory_event *event_queue = NULL;
	struct drv_memory_devices *memory_devs = memory_get_devices();

	if ((device_id >= MEMORY_NODE_ID_MAX) || (sensor_id >= MEMORY_SENSOR_ID_MAX)) {
		memory_drv_err("invalid params. device id(%u), sensor id(%u)\n", device_id, sensor_id);
		return NULL;
	}

	node = &memory_devs->m_dev[device_id][sensor_id];
	event_queue = &node->sensor_event_queue[que_index];
	return event_queue;
}

#ifndef DRV_MEM_GTEST
int32_t memory_del_fault_event(struct memory_error_list *error_new, uint8_t que_index)
{
	struct memory_error_list *n = NULL;
	struct memory_error_list *pos = NULL;
	struct memory_event *event_queue = NULL;

	event_queue = memory_get_sensor_event_queue(error_new->error.device_id, error_new->error.sensor_id, que_index);
	if (event_queue == NULL) {
		kfree(error_new);
		memory_drv_err("get sensor event queue failed\n");
		return -EFAULT;
	}

	mutex_lock(&event_queue->mutex);
	list_for_each_entry_safe(pos, n, &event_queue->error_list.list, list) {
		if (pos->error.event_type == error_new->error.event_type) {
			list_del(&pos->list);                /* delete event from queue */
			kfree(pos);
			kfree(error_new);
			pos = NULL;
			event_queue->error_num--;
			mutex_unlock(&event_queue->mutex);
			return EEXIST;
		}
	}

	kfree(error_new);
	mutex_unlock(&event_queue->mutex);
	return 0;
}

int32_t memory_add_fault_event(struct memory_error_list *error_new, uint8_t que_index)
{
	struct memory_error_list *n = NULL;
	struct memory_error_list *pos = NULL;
	struct memory_event *event_queue = NULL;

	event_queue = memory_get_sensor_event_queue(error_new->error.device_id, error_new->error.sensor_id, que_index);
	if (event_queue == NULL) {
		memory_drv_err("get sensor event queue failed\n");
		kfree(error_new);
		return -EFAULT;
	}

	mutex_lock(&event_queue->mutex);
	list_for_each_entry_safe(pos, n, &event_queue->error_list.list, list) {
		if (pos->error.event_type == error_new->error.event_type) {
			mutex_unlock(&event_queue->mutex);
			kfree(error_new);
			return EEXIST;
		}
	}

	list_add(&error_new->list, &event_queue->error_list.list);      /* add new event to queue */
	event_queue->error_num++;
	mutex_unlock(&event_queue->mutex);
	return 0;
}

STATIC struct ras_fault_converge_item *memory_get_converge_node(const struct memory_fault_event *event)
{
	uint32_t i;

	if (event == NULL) {
		memory_drv_err("Invalid para, event is NULL\n");
		return NULL;
	}

	for (i = 0; i < (sizeof(g_memory_fault_converge_table) / sizeof(g_memory_fault_converge_table[0])); i++) {
		if ((event->sensor_type == g_memory_fault_converge_table[i].sensor_type) &&
		    (event->event_type == g_memory_fault_converge_table[i].error_type))
				return &g_memory_fault_converge_table[i];
	}
	return NULL;
}

STATIC bool memory_is_corrected_fault_event(const struct memory_fault_event *event)
{
	uint32_t i, len;
	bool is_corrected = false;

	if (event == NULL) {
		memory_drv_err("Invalid para, fault_info is NULL\n");
		return is_corrected;
	}
	len = sizeof(g_recover_table) / sizeof(g_recover_table[0]);
	for (i = 0; i < len; i++) {
		if (event->detail_reason == g_recover_table[i]) {
			is_corrected = true;
			break;            /* true means found fault event type in record table, need clear */
		}
	}
	return is_corrected;
}

int32_t memory_scan_para_check(struct dms_sensor_event_data *data, struct drv_memory_devices *memory_devs, \
							   uint32_t dev_id, uint32_t sensor_id, uint32_t queue_idx)
{
	if ((data == NULL) || (memory_devs == NULL) || (dev_id >= (uint32_t)memory_dev_num(memory_devs->dev_num)) ||
		(sensor_id >= (uint32_t)MEMORY_SENSOR_ID_MAX) || (queue_idx >= (uint32_t)S_IDX_MAX)) {
		memory_drv_err("Invalid para. (data=%d; dev_id=%u; s_id=%u; queue_idx=%u)\n",
						(data == NULL), dev_id, sensor_id, queue_idx);
		return -EINVAL;
	}

	return 0;
}

int32_t memory_event_scan(unsigned long long private_data, struct dms_sensor_event_data *data)
{
	int32_t ret;
	uint32_t dev_id, sensor_id, queue_idx;
	struct memory_error_list *pos = NULL;
	struct memory_error_list *n = NULL;
	struct memory_event *event_queue = NULL;
	struct drv_memory_devices *memory_devs = memory_get_devices();
	struct ras_fault_converge_item *fault_info = NULL;

	dev_id = (private_data >> DEVNODE_OFFSET_32BIT) & MEMORY_MASK_32BIT;
	sensor_id = (private_data & MEMORY_MASK_32BIT) >> SENSOR_OFFSET_16BIT;
	queue_idx = (uint32_t)(private_data & MEMORY_MASK_16BIT);

	ret = memory_scan_para_check(data, memory_devs, dev_id, sensor_id, queue_idx);
	if (ret != 0) {
		return ret;
	}

	event_queue = &memory_devs->m_dev[dev_id][sensor_id].sensor_event_queue[queue_idx];
	data->event_count = 0;
	mutex_lock(&event_queue->mutex);

	/* get every event of the sensor event queue */
	list_for_each_entry_safe(pos, n, &event_queue->error_list.list, list) {
		fault_info = memory_get_converge_node(&pos->error);
		if (fault_info == NULL) {
			continue;
		}

		data->sensor_data[data->event_count].current_value = fault_info->error_type;
		data->sensor_data[data->event_count].data_size = sizeof(fault_info->describe);
		ret = memcpy_s(data->sensor_data[data->event_count].event_data, DMS_MAX_EVENT_DATA_LENGTH,
						fault_info->describe, sizeof(fault_info->describe));
		if (ret != 0) {
			continue;
		}

		if (memory_is_corrected_fault_event(&pos->error)) {
			list_del(&pos->list);
			kfree(pos);
			pos = NULL;
			event_queue->error_num--;
		}
		data->event_count++;
		if (data->event_count == DMS_MAX_SENSOR_EVENT_COUNT) {
			break;
		}
	}
	mutex_unlock(&event_queue->mutex);
	return 0;
}
#endif