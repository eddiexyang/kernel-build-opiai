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
#include <linux/types.h>
#include <linux/slab.h>

#include "dms_dev_node.h"

#include "memory_log.h"
#include "safety_ras_node.h"

struct drv_memory_devices g_memory_devs;

uint32_t memory_dev_num(uint32_t dev_num)
{
	return (dev_num < (uint32_t)MEMORY_NODE_ID_MAX) ? (dev_num) : ((uint32_t)MEMORY_NODE_ID_MAX);
}

struct drv_memory_devices *memory_get_devices(void)
{
	return &g_memory_devs;
}

int32_t memory_dms_node_destroy(void)
{
	uint32_t i, j;

	for (i = 0; i < memory_dev_num(g_memory_devs.dev_num); i++) {
		for (j = 0; j < g_memory_devs.m_dev[i][MEMORY_SENSOR_DDRC].sensor_obj_num; j++)
			mutex_destroy(&g_memory_devs.m_dev[i][MEMORY_SENSOR_DDRC].sensor_event_queue[j].mutex);
	}

	return 0;
}

int32_t memory_register_one_node(struct memory_dev *m_dev)
{
	uint32_t i;
	int32_t ret;

	if (m_dev == NULL) {
		memory_drv_err("Invalid para, h_dev is NULL.\n");
		return -EINVAL;
	}

	ret = dms_register_dev_node(m_dev->dev_node);
	if (ret != 0) {
		memory_drv_err("register dev_node failed. (ret=%d.)\n", ret);
		return ret;
	}

	for (i = 0; i < m_dev->sensor_obj_num; i++) {
		ret = (int32_t)dms_sensor_register(m_dev->dev_node, &m_dev->sensor_obj_table[i]);     /* register sensor */
		if (ret != 0) {
			memory_drv_err("register sensor failed. (sensor_idx=%u; ret=%d.)\n", i, ret);
			goto out;
		}
	}

	return 0;
out:
	if (i > 0)
		(void)dms_sensor_node_unregister(m_dev->dev_node);

	(void)dms_unregister_dev_node(m_dev->dev_node);
	return ret;
}

void memory_unregister_one_node(struct memory_dev *m_dev)
{
	uint32_t ret;

	if (m_dev == NULL) {
		memory_drv_err("Invalid para, m_dev is NULL.\n");
		return;
	}
	ret = dms_sensor_node_unregister(m_dev->dev_node);
	if (ret != 0) {
		memory_drv_warn("unregister sensor node failed, (ret=%u.)\n", ret);
	}

	ret = (uint32_t)dms_unregister_dev_node(m_dev->dev_node);
	if (ret != 0) {
		memory_drv_warn("unregister dev node failed, (ret=%u.)\n", ret);
	}

	return;
}

int32_t memory_dev_node_register(void)
{
	int32_t i;
	uint32_t j, sensor_id;
	uint32_t dev_id;
	uint32_t successed_sensor_num;
	struct memory_dev *m_dev = NULL;
	struct drv_memory_devices *memory_devs = memory_get_devices();
	int32_t ret;

	for (dev_id = 0; dev_id < memory_dev_num(memory_devs->dev_num); dev_id++) {
		for (sensor_id = 0; sensor_id < (uint32_t)MEMORY_SENSOR_ID_MAX; sensor_id++) {
			m_dev = &memory_devs->m_dev[dev_id][sensor_id];    /* get each device each sensor dev_node */
			ret = memory_register_one_node(m_dev);
			if (ret != 0) {
				memory_drv_err("register one node failed. (dev_id=%d; s_id=%d; ret=%d)\n", dev_id, sensor_id, ret);
				goto out;
			}
		}
	}

	return 0;
out:
	for (i = (int32_t)dev_id; i >= 0; i--) {
		successed_sensor_num = ((i == (int32_t)dev_id) ? sensor_id : (uint32_t)MEMORY_SENSOR_ID_MAX);
		for (j = 0; j < successed_sensor_num; j++) {
			memory_unregister_one_node(&memory_devs->m_dev[i][j]);
		}
	}
	return ret;
}

void memory_fault_event_free(struct memory_event *event_queue)
{
	struct memory_error_list *pos = NULL;
	struct memory_error_list *n = NULL;

	if (event_queue == NULL) {
		memory_drv_err("Invalid para, event_queue is NULL\n");
		return;
	}
#ifndef DRV_MEM_GTEST
	mutex_lock(&event_queue->mutex);
	list_for_each_entry_safe(pos, n, &event_queue->error_list.list, list) {
		list_del(&pos->list);
		kfree(pos);
		pos = NULL;
		event_queue->error_num--;
	}
	mutex_unlock(&event_queue->mutex);
#endif
}

int32_t memory_dev_node_uregister(void)
{
	int32_t j;
	uint32_t i, k;
	struct drv_memory_devices *memory_devs = memory_get_devices();

	for (i = 0; i < memory_dev_num(memory_devs->dev_num); i++) {
		for (j = 0; j < (int32_t)MEMORY_SENSOR_ID_MAX; j++) {
			(void)dms_sensor_node_unregister(memory_devs->m_dev[i][j].dev_node);
			(void)dms_unregister_dev_node(memory_devs->m_dev[i][j].dev_node);
			for (k = 0; k < memory_devs->m_dev[i][j].sensor_obj_num; k++)
				memory_fault_event_free(&memory_devs->m_dev[i][j].sensor_event_queue[k]);
		}
	}

	return 0;
}