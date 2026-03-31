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
 *
 * Description:
 * Author: huawei
 * Create: 2023-05-17
 */
#include <linux/slab.h>

#include "memory_log.h"
#include "safety_ras_mata_310.h"
#include "safety_ras_mata_config_310.h"
#include "safety_ras_init_310.h"

STATIC uint32_t g_ras_event_recover_type_table[] = {
	RAS_ERROR_TYPE_SBECCOverThold
};

STATIC uint32_t g_mata_ras_ce_type_table[] = {
	SINGLE_BIT_ECC_INGRESS,
	SINGLE_BIT_ECC_SDIR,
	SINGLE_BIT_ECC_DATA_BUFF,
	SINGLE_BIT_ECC_RDQ_BUFF,
	SINGLE_BIT_ECC_SHARE_BUFF
};

int32_t mata_ops_init(struct dms_node *device)
{
	if (device != NULL)
		memory_drv_debug("DEBUG (node_name=%s)\n", device->node_name);

	return 0;
}

void mata_ops_exit(struct dms_node *device)
{
	if (device != NULL)
		memory_drv_debug("DEBUG (node_name=%s)\n", device->node_name);

	return;
}

/*
 * @brief: Check whether the event is in the event table that can be processed
 * @in[event]: event
 * @return: NULL-event ont in table, others-event in table
 */
struct ras_fault_converge_item *mata_parse_fault_table(struct memory_fault_event *event)
{
	uint32_t i;

	if (event == NULL) {
		memory_drv_err("Invalid para, event is NULL.\n");
		return NULL;
	}

	for (i = 0; i < (sizeof(g_mata_fault_converge_table) / sizeof(g_mata_fault_converge_table[0])); i++) {
		if ((event->event_type == g_mata_fault_converge_table[i].error_type) &&
			(event->error_code == (g_mata_fault_converge_table[i].ras_code.err_status & MEMORY_MASK_16BIT)))
			return &g_mata_fault_converge_table[i];
	}

	return NULL;
}

/*
 * @brief: Check whether an event can be recovered.
 * @in[fault_info]: pointer of event info
 * @return: true-event can be recovered, false-event can not be recovered
 */
STATIC bool mata_event_clear_check(struct ras_fault_converge_item *fault_info)
{
	int32_t i, len;

	len = (int32_t)sizeof(g_ras_event_recover_type_table) / (int32_t)sizeof(g_ras_event_recover_type_table[0]);
	for (i = 0; i < len; i++) {
		if (fault_info->error_type == g_ras_event_recover_type_table[i])
			return true;	/* true means found fault event type in record table, need clear */
	}

	return false;
}

int32_t mata_scan_para_check(struct dms_sensor_event_data *data, struct drv_memory_devices *mata_devs, \
							   uint32_t dev_id, uint32_t sensor_id, uint32_t queue_idx)
{
	if ((data == NULL) || (mata_devs == NULL) || (dev_id >= mata_dev_num(mata_devs->dev_num)) ||
		(sensor_id >= (uint32_t)MEMORY_SENSOR_ID_MAX) || (queue_idx >= MATA_S_IDX_MAX)) {
		memory_drv_err("Invalid para. (data=%d; dev_id=%u; s_id=%u; queue_idx=%u)\n",
			(data == NULL), dev_id, sensor_id, queue_idx);
		return -EINVAL;
	}

	return 0;
}

/*
 * @brief: scan event func, called by dms
 * @in[private_data]: private_data of event
 * @out[data]: store data of event and return to dms
 * @return: 0-success, others-fail
 */
int32_t mata_fault_event_scan(unsigned long long private_data, struct dms_sensor_event_data *data)
{
	int32_t ret;
	uint32_t dev_id, sensor_id, queue_idx;
	struct memory_error_list *pos = NULL;
	struct memory_error_list *n = NULL;
	struct memory_event *event_queue = NULL;
	struct drv_memory_devices *mata_devs = mata_get_devices();
	struct ras_fault_converge_item *fault_info = NULL;

	dev_id = (uint32_t)((private_data >> MEMORY_OFFSET_32BIT) & MEMORY_MASK_32BIT);
	sensor_id = (uint32_t)((private_data & MEMORY_MASK_32BIT) >> MEMORY_OFFSET_16BIT);
	queue_idx = (uint32_t)private_data & (uint32_t)MEMORY_MASK_16BIT;

	ret = mata_scan_para_check(data, mata_devs, dev_id, sensor_id, queue_idx);
	if (ret != 0) {
		return ret;
	}

#ifndef DRV_MEM_GTEST
	event_queue = &mata_devs->m_dev[dev_id][sensor_id].sensor_event_queue[queue_idx];
	data->event_count = 0;
	mutex_lock(&event_queue->mutex);
	/* get every event of the sensor event queue */
	list_for_each_entry_safe(pos, n, &event_queue->error_list.list, list) {
		fault_info = mata_parse_fault_table(&pos->error);
		if (fault_info == NULL) {
			continue;
		}
		data->sensor_data[data->event_count].current_value = fault_info->error_type;
		data->sensor_data[data->event_count].data_size = (unsigned short)sizeof(fault_info->describe);
		ret = memcpy_s(data->sensor_data[data->event_count].event_data, DMS_MAX_EVENT_DATA_LENGTH,
			fault_info->describe, sizeof(fault_info->describe));
		if (ret != 0) {
			continue;
		}
		if (mata_event_clear_check(fault_info)) {
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
#endif
	return 0;
}

int32_t get_mata_event_type_by_error_code(uint32_t error_code, uint16_t *event_type)
{
	uint32_t i;

	if (event_type == NULL) {
		memory_drv_err("Invalid para, event_type is NULL.\n");
		return -1;
	}

	for (i = 0; i < (sizeof(g_mata_fault_converge_table) / sizeof(g_mata_fault_converge_table[0])); i++) {
		if (error_code == (g_mata_fault_converge_table[i].ras_code.err_status & MEMORY_MASK_16BIT)) {
			*event_type = g_mata_fault_converge_table[i].error_type;
			return 0;
		}
	}

	return -1;
}

/*
 * @brief: add event to list of sensor
 * @in[error_new]: event that needs to be added to sensor list
 * @in[event_queue]: sensor
 * @return: 0-success, others-fail
 */
int32_t mata_add_fault_event(struct memory_error_list *error_new, struct memory_event *event_queue)
{
	struct memory_error_list *pos = NULL;
	struct memory_error_list *n = NULL;

	if ((event_queue == NULL) || (error_new == NULL)) {
		memory_drv_err("Invalid para. (event_queue NULL=%d)\n", (event_queue == NULL));
		return -EINVAL;
	}
#ifndef DRV_MEM_GTEST
	mutex_lock(&event_queue->mutex);
	list_for_each_entry_safe(pos, n, &event_queue->error_list.list, list) {
		if ((pos->error.event_type == error_new->error.event_type)) {
			mutex_unlock(&event_queue->mutex);
			return EEXIST;	/* it means found fault event already in event list, doesn't need add */
		}
	}
	list_add(&error_new->list, &event_queue->error_list.list);	/* add new event to list */
	event_queue->error_num++;
	mutex_unlock(&event_queue->mutex);
#endif
	return 0;
}

/*
 * @brief: Check whether the fault is a CE fault
 * @in[error_code]: error code of the fault
 * @return: true-CE fault, false-UE fault
 */
STATIC bool is_mata_ce_ras(uint32_t error_code)
{
	uint32_t i;
	uint32_t num = (uint32_t)sizeof(g_mata_ras_ce_type_table) / (uint32_t)sizeof(g_mata_ras_ce_type_table[0]);
	uint32_t err_type = error_code & (uint32_t)RAS_MATA_CE_MASK;
	for (i = 0; i < num; i++) {
		if (err_type == g_mata_ras_ce_type_table[i])
			return true;
	}
	return false;
}

/*
 * @brief: unmask mata irq
 * @in[devid]: id of device
 * @in[sensor_id]: id of sensor
 * @in[error_code]: error code of the fault
 * @return: NA
 */
void unmask_mata_irq_process(uint32_t devid, uint32_t sensor_id, uint32_t error_code)
{
	struct drv_memory_devices *mata_devs = mata_get_devices();
	struct memory_dev *dev = &mata_devs->m_dev[devid][sensor_id];
	struct delayed_work *work;

	g_mata_ras_ce_type = is_mata_ce_ras(error_code);
	if (g_mata_ras_ce_type) {
		work = &dev->unmask_ce_irq_work;
	} else {
		work = &dev->unmask_ue_irq_work;
	}
#ifndef DRV_MEM_GTEST
	if (delayed_work_pending(work))
		(void)cancel_delayed_work_sync(work);

	(void)schedule_delayed_work(work, msecs_to_jiffies(SCHEDULE_UNMASK_RAS_IRQ_PERIOD));
#endif
}