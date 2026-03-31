/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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
#include <asm/barrier.h>
#include "devdrv_manager_comm.h"
#include "dms_interface.h"
#include "dms_sensor_type.h"
#include "dms_dev_node.h"
#include "dms_sensor.h"
#include "dms_node_type.h"
#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
#include "lpm_debugfs_base.h"
#include "lpm_fault_debugfs.h"
#endif
#include "lpm_fault_report.h"

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
STATIC int32_t lpm_alarm_debugfs_init(void);
#endif

// for private_data
#define LPM_FAULT_DEV_ID_WIDTH           7U
#define LPM_FAULT_NODE_TYPE_ID_WIDTH     4U
#define LPM_FAULT_NODE_INNER_ID_WIDTH    6U
#define LPM_FAULT_SENSOR_INNER_ID_WIDTH  6U

// for sensor scan interval
#define LPM_SENSOR_SCAN_DEFAULT_INTERVAL 100

STATIC struct lpm_fault_priv *lpm_fault_get_priv(void)
{
	static struct lpm_fault_priv lpm_fault_priv_data = {0};
	return &lpm_fault_priv_data;
}

STATIC struct dms_node *lpm_fault_get_dms_node(uint32_t node_type, uint32_t dev_id)
{
	uint32_t node_type_id;
	struct lpm_fault_priv *fault_priv = lpm_fault_get_priv();
	struct lpm_fault_dms_node *node_info = &fault_priv->dev_priv[dev_id].node_info[0];

	for (node_type_id = 0; node_type_id < LPM_FAULT_NODE_TYPE_NUM; ++node_type_id) {
		if ((node_info[node_type_id].node_cfg != NULL) &&
			(node_info[node_type_id].node_cfg->dms_node.node_type == (int32_t)node_type)) {
			return &node_info[node_type_id].node_cfg->dms_node;
		}
	}
	return NULL;
}

// private_data: 64 bit
// |63--------23-22------16-15--------12-11----------6-5-------------0|
// |     41bit  |   7bit   |    4bit    |     6bit    |      6bit     |
// |  reserved  |  dev_id  |node_type_id|node_inner_id|sensor_inner_id|
// |------------------------------------------------------------------|
STATIC void lpm_fault_parse_private_data(
	uint64_t private_data, struct lpm_fault_scan_priv_data *priv_data)
{
	uint32_t i;
	uint64_t offset = 0;
	uint64_t mask;
	uint32_t *data = (uint32_t *)priv_data;
	uint64_t width[] = {
		0x0U,
		LPM_FAULT_SENSOR_INNER_ID_WIDTH, LPM_FAULT_NODE_INNER_ID_WIDTH,
		LPM_FAULT_NODE_TYPE_ID_WIDTH, LPM_FAULT_DEV_ID_WIDTH
	};

	for (i = 1; i < (uint32_t)ARRAY_SIZE(width); i++) {
		offset += width[i - 0x1U];
		mask = ((uint64_t)0x1U << width[i]) - 0x1U;
		data[i - 0x1U] = (uint32_t)((private_data >> offset) & mask);
	}
}

STATIC uint64_t lpm_fault_set_private_data(struct lpm_fault_scan_priv_data *priv_data)
{
	uint32_t i;
	uint64_t offset = 0;
	uint64_t mask;
	uint64_t private_data = 0;
	uint32_t *data = (uint32_t *)priv_data;
	uint64_t width[] = {
		0x0U,
		LPM_FAULT_SENSOR_INNER_ID_WIDTH, LPM_FAULT_NODE_INNER_ID_WIDTH,
		LPM_FAULT_NODE_TYPE_ID_WIDTH, LPM_FAULT_DEV_ID_WIDTH
	};

	for (i = 1; i < (uint32_t)ARRAY_SIZE(width); i++) {
		offset += width[i - 0x1U];
		mask = ((uint64_t)0x1U << width[i]) - 0x1U;
		private_data |= ((uint64_t)data[i - 0x1U] & mask) << offset;
	}

	return private_data;
}

STATIC bool lpm_fault_check_private_data(const struct lpm_fault_scan_priv_data *priv_data)
{
	struct lpm_fault_priv *fault_priv = lpm_fault_get_priv();
	struct lpm_fault_node_obj_cfg *node_type_cfg = NULL;

	if (!lpm_common_check_dev_id(priv_data->dev_id)) {
		return false;
	}

	if (priv_data->node_type_id >= fault_priv->node_type_table.node_type_num) {
		return false;
	}

	node_type_cfg = &fault_priv->node_type_table.node_type_cfg[priv_data->node_type_id];

	if (priv_data->node_inner_id >= node_type_cfg->node_num) {
		return false;
	}

	if (priv_data->sensor_inner_id >= node_type_cfg->sensor_num) {
		return false;
	}
	return true;
}

STATIC struct lpm_fault_sensor_obj_cfg *lpm_fault_get_sensor_cfg(uint32_t sensor_type_id)
{
	struct lpm_fault_priv *fault_priv = lpm_fault_get_priv();

	if (sensor_type_id >= fault_priv->sensor_table.sensor_num) {
		return NULL;
	}
	return &fault_priv->sensor_table.sensor_cfg[sensor_type_id];
}

// get event type, fill to sensor data
STATIC uint32_t lpm_fault_fill_event_info(
	struct dms_sensor_event_data *data, struct lpm_fault_sensor_cfg *sensors_cfg,
	struct lpm_fault_sensor_obj_cfg *sensor_table)
{
	uint32_t event_num = 0;
	uint32_t event_id;
	uint32_t event_type;
	uint32_t alarm_mask = 0;
	int32_t ret;
	struct lpm_fault_event_desc *event_desc = NULL;
	struct dms_sensor_event_data_item *sensor_data = NULL;

	// Notice:
	// this function is protected by the spin_lock, so the error log cannot be printed
	// use different return value to distinguish error-return branches
	event_desc = sensor_table->fn_get_event(&event_num);
	data->event_count = 0;

	for (event_id = 0; event_id < event_num; event_id++) {
		event_type = event_desc[event_id].event_type;
		if (sensors_cfg->event_status[event_type] == (uint32_t)DMS_EVENT_TYPE_RESUME) {
			continue;
		}

		sensor_data = &data->sensor_data[data->event_count];
		sensor_data->current_value = (int32_t)event_type;
		sensor_data->data_size     = (uint16_t)sizeof(event_desc[event_id].describe);
		ret = memcpy_s(sensor_data->event_data, DMS_MAX_EVENT_DATA_LENGTH,
			event_desc[event_id].describe, DMS_MAX_EVENT_DATA_LENGTH);
		if (ret != 0) {
			alarm_mask |= ((uint32_t)0x1U << event_type);
			continue;
		}

		// one time alarm change event_status from occur to recovery
		if ((sensors_cfg->sensors.deassert_event_mask & ((uint32_t)0x1U << event_type)) == 0x0U) {
			sensors_cfg->event_status[event_type] = (uint32_t)DMS_EVENT_TYPE_RESUME;
		}
		data->event_count++;
	}

	return alarm_mask;
}

STATIC int32_t lpm_fault_event_scan(unsigned long long private_data, struct dms_sensor_event_data *data)
{
	uint32_t alarm_mask;
	unsigned long irq_flags;
	struct lpm_fault_scan_priv_data priv_data = {0};
	struct lpm_fault_priv *fault_priv = lpm_fault_get_priv();
	struct lpm_fault_node_cfg *node_cfg = NULL;
	struct lpm_fault_sensor_cfg *sensors_cfg = NULL;
	struct lpm_fault_sensor_obj_cfg *sensor_table = NULL;

	if (data == NULL) {
		lpm_log_err("lpm fault scan event failed, null ptr, private_data=%llu\n", private_data);
		return -EINVAL;
	}

	lpm_fault_parse_private_data((uint64_t)private_data, &priv_data);
	if (!lpm_fault_check_private_data(&priv_data)) {
		lpm_log_err("lpm fault scan event failed, private_data=%llu illegal\n", private_data);
		return -1;
	}

	node_cfg = fault_priv->dev_priv[priv_data.dev_id].node_info[priv_data.node_type_id].node_cfg;
	sensors_cfg = &node_cfg[priv_data.node_inner_id].sensors_cfg[priv_data.sensor_inner_id];
	sensor_table = lpm_fault_get_sensor_cfg(sensors_cfg->sensor_type_id);
	if (sensor_table == NULL) {
		lpm_log_err("lpm fault scan event failed, sensor_type_id=%u illegal, private_data=%llu\n",
			sensors_cfg->sensor_type_id, private_data);
		return -1;
	}

	if (sensor_table->fn_get_event == NULL) {
		lpm_log_err("lpm fault scan event failed, sensor_type_id=%u fn_get_event is null, private_data=%llu\n",
			sensors_cfg->sensor_type_id, private_data);
		return -1;
	}

	spin_lock_irqsave(&sensors_cfg->alarm_lock, irq_flags);
	alarm_mask = lpm_fault_fill_event_info(data, sensors_cfg, sensor_table);
	spin_unlock_irqrestore(&sensors_cfg->alarm_lock, irq_flags);

	if (alarm_mask != 0) {
		lpm_log_err("lpm scan sensor data failed, "
			"private_data=%llu, alarm_mask=0x%x, sensor_type_id=%u\n",
			private_data, alarm_mask, sensors_cfg->sensor_type_id);
	}
	return 0;
}

STATIC int32_t lpm_fault_fill_sensor_table(struct dms_sensor_object_cfg *sensor_obj,
	uint32_t sensor_type_id, struct lpm_fault_scan_priv_data *priv_data)
{
	int32_t ret;
	struct lpm_fault_sensor_obj_cfg *sensor_cfg = lpm_fault_get_sensor_cfg(sensor_type_id);
	if (sensor_cfg == NULL) {
		lpm_log_err("can not found sensor, sensor_type_id=%u, node_type_id=%u, "
			"node_inner_id=%u, sensor_inner_id=%u, dev_id=%u\n",
			sensor_type_id, priv_data->node_type_id,
			priv_data->node_inner_id, priv_data->sensor_inner_id, priv_data->dev_id);
		return -1;
	}

	ret = sprintf_s((char *)sensor_obj->sensor_name, sizeof(sensor_obj->sensor_name),
		(const char *)sensor_cfg->sensor_name);
	if (ret <= 0) {
		lpm_log_err("sprintf_s %s sensor_name failed, ret=%d, sensor_type=0x%x, node_type_id=%u, "
			"node_inner_id=%u, sensor_inner_id=%u, dev_id=%u\n",
			sensor_cfg->sensor_name, ret, sensor_cfg->sensor_type, priv_data->node_type_id,
			priv_data->node_inner_id, priv_data->sensor_inner_id, priv_data->dev_id);
		return ret;
	}

	sensor_obj->sensor_type         = sensor_cfg->sensor_type;
	sensor_obj->sensor_class        = DMS_DISCRETE_SENSOR_CLASS; // Report when fault come
	sensor_obj->sensor_class_cfg.discrete_sensor.attribute = DMS_SENSOR_ATTRIB_THRES_NONE;
	sensor_obj->sensor_class_cfg.discrete_sensor.debounce_time = 0;
	sensor_obj->pf_scan_func        = lpm_fault_event_scan;
	sensor_obj->private_data        = lpm_fault_set_private_data(priv_data);
	sensor_obj->scan_interval       = LPM_SENSOR_SCAN_DEFAULT_INTERVAL;
	sensor_obj->scan_module         = DMS_SERSOR_SCAN_NOTIFY;
	sensor_obj->proc_flag           = DMS_SENSOR_PROC_ENABLE_FLAG;
	sensor_obj->enable_flag         = DMS_SENSOR_ENABLE_FALG;

	// same sensor_type under diffrent node_type, can support different event_type
	sensor_obj->assert_event_mask   = sensor_cfg->assert_mask[priv_data->node_type_id];
	sensor_obj->deassert_event_mask = sensor_cfg->deassert_mask[priv_data->node_type_id];
	return 0;
}

STATIC int32_t lpm_fault_init_node_obj(
	struct lpm_fault_node_cfg *node_cfg, uint32_t node_type_id, uint32_t node_id, uint32_t dev_id)
{
	int32_t ret;
	struct lpm_fault_priv *fault_priv = lpm_fault_get_priv();
	struct lpm_fault_node_obj_cfg *node_type_cfg = &fault_priv->node_type_table.node_type_cfg[node_type_id];
	struct dms_node *owner_node = NULL;

	if (node_type_cfg->owner_node_type != 0) {
		owner_node = lpm_fault_get_dms_node(node_type_cfg->owner_node_type, dev_id);
		if (owner_node == NULL) {
			lpm_log_err("please ensure owner_node=0x%x registered before subnode=0x%x, dev_id=%u\n",
				node_type_cfg->owner_node_type, node_type_cfg->node_type, dev_id);
			return -1;
		}
	}

	ret = sprintf_s(node_cfg->dms_node.node_name, DMS_MAX_DEV_NAME_LEN, "%s-%u",
		node_type_cfg->node_name, node_id);
	if (ret <= 0) {
		lpm_log_err("sprintf_s lpm node name failed, ret=%d, "
			"node_type_id=%u, node_type=0x%x, node_name=%s, node_id=%u\n",
			ret, node_type_id, node_type_cfg->node_type, node_type_cfg->node_name, node_id);
		return -1;
	}

	node_cfg->node_ops.init           = node_type_cfg->fn_init;
	node_cfg->node_ops.uninit         = node_type_cfg->fn_uninit;
	node_cfg->node_ops.scan           = NULL;
	node_cfg->node_ops.fault_diag     = NULL;
	node_cfg->node_ops.event_notify   = NULL;
	node_cfg->node_ops.get_link_state = NULL;
	node_cfg->node_ops.set_link_state = NULL;

	node_cfg->dms_node.node_type    = node_type_cfg->node_type;
	node_cfg->dms_node.node_id      = (int32_t)node_id;
	node_cfg->dms_node.capacity     = 0x01;
	node_cfg->dms_node.permission   = 0x01;
	node_cfg->dms_node.owner_devid  = dev_id;
	node_cfg->dms_node.owner_device = owner_node;
	node_cfg->dms_node.ops          = &node_cfg->node_ops;
	return 0;
}

STATIC int32_t lpm_fault_init_sensor_obj(struct lpm_fault_sensor_cfg *sensor_cfg,
	uint32_t dev_id, uint32_t node_type_id, uint32_t node_inner_id)
{
	int32_t ret;
	uint32_t sensor_inner_id;
	struct lpm_fault_sensor_cfg *sensor_instance = NULL;
	struct lpm_fault_priv *fault_priv = lpm_fault_get_priv();
	struct lpm_fault_node_obj_cfg *node_type_cfg = &fault_priv->node_type_table.node_type_cfg[node_type_id];
	struct lpm_fault_scan_priv_data priv_data;

	priv_data.node_inner_id = node_inner_id;
	priv_data.node_type_id  = node_type_id;
	priv_data.dev_id        = dev_id;

	for (sensor_inner_id = 0; sensor_inner_id < node_type_cfg->sensor_num; sensor_inner_id++) {
		sensor_instance = &sensor_cfg[sensor_inner_id];
		(void)memset_s(sensor_instance, sizeof(struct lpm_fault_sensor_cfg),
			0, sizeof(struct lpm_fault_sensor_cfg));

		priv_data.sensor_inner_id = sensor_inner_id;
		ret = lpm_fault_fill_sensor_table(&sensor_instance->sensors,
			node_type_cfg->support_sensor[sensor_inner_id], &priv_data);
		if (ret != 0) {
			lpm_log_err("fill lpm sensor table error, ret=%d, dev_id=%u, node_inner_id=%u, "
				"sensor_type_id=%u, sensor_inner_id=%u, sensor_num=%u.\n",
				ret, dev_id, node_inner_id, node_type_cfg->support_sensor[sensor_inner_id],
				sensor_inner_id, node_type_cfg->sensor_num);
			return ret;
		}

		sensor_instance->sensor_type_id = node_type_cfg->support_sensor[sensor_inner_id];
		spin_lock_init(&sensor_instance->alarm_lock);
	}
	return 0;
}

STATIC int32_t lpm_fault_init_node(
	struct lpm_fault_dev_priv_cfg *cfg_info, uint32_t dev_id, uint32_t node_type_id)
{
	int32_t ret;
	uint32_t node_inner_id;
	struct lpm_fault_priv *fault_priv = lpm_fault_get_priv();
	struct lpm_fault_node_num_cfg *num_cfg = &cfg_info->num_cfg_all[node_type_id];
	struct lpm_fault_dms_node *node_info = &cfg_info->node_info[node_type_id];
	struct lpm_fault_node_obj_cfg *node_type_cfg = &fault_priv->node_type_table.node_type_cfg[node_type_id];
	uint32_t sensor_cfg_offset = num_cfg->sensor_cfg_offset;

	node_info->node_num = node_type_cfg->node_num;
	node_info->node_cfg = &cfg_info->node_cfg_all[num_cfg->node_cfg_offset];

	for (node_inner_id = 0; node_inner_id < node_info->node_num; node_inner_id++) {
		// init each node instance in same node_type
		ret = lpm_fault_init_node_obj(&node_info->node_cfg[node_inner_id],
			node_type_id, node_inner_id + num_cfg->node_start_id, dev_id);
		if (ret != 0) {
			lpm_log_err("node init failed, ret=%d, dev_id=%u, "
				"node_type_id=%u, node_inner_id=%u, node_start_id=%u\n",
				ret, dev_id, node_type_id, node_inner_id, num_cfg->node_start_id);
			return -1;
		}

		// init each sensor instance for each node instance
		node_info->node_cfg[node_inner_id].sensor_num  = node_type_cfg->sensor_num;
		node_info->node_cfg[node_inner_id].sensors_cfg = &cfg_info->sensor_cfg_all[sensor_cfg_offset];
		sensor_cfg_offset += node_type_cfg->sensor_num;

		ret = lpm_fault_init_sensor_obj(node_info->node_cfg[node_inner_id].sensors_cfg,
			dev_id, node_type_id, node_inner_id);
		if (ret != 0) {
			lpm_log_err("sensor init failed, ret=%d, dev_id=%u, node_type_id=%u, node_inner_id=%u\n",
				ret, dev_id, node_type_id, node_inner_id);
			return -1;
		}
	}

	return 0;
}

STATIC int32_t lpm_fault_init_num_cfg(
	uint32_t node_type_id, struct lpm_fault_node_num_cfg *num_cfg_all, uint32_t *start_node_id,
	uint32_t *node_cfg_offset, uint32_t *sensor_cfg_offset)
{
	struct lpm_fault_priv *fault_priv = lpm_fault_get_priv();
	struct lpm_fault_node_obj_cfg *node_type_cfg =
		&fault_priv->node_type_table.node_type_cfg[node_type_id];

	if ((node_type_cfg->sensor_num == 0) || (node_type_cfg->node_num == 0)) {
		lpm_log_err("node_type_id=%u has no sensor or node, sensor_num=%u, node_num=%u\n",
			node_type_id, node_type_cfg->sensor_num, node_type_cfg->node_num);
		return -1;
	}
	if (node_type_cfg->sensor_num != node_type_cfg->max_sensor) {
		lpm_log_err("node_type_id=%u some sensor not register, sensor_num=%u, max_sensor=%u\n",
			node_type_id, node_type_cfg->sensor_num, node_type_cfg->max_sensor);
		return -1;
	}

	num_cfg_all[node_type_id].node_start_id     = start_node_id[node_type_id];
	num_cfg_all[node_type_id].node_cfg_offset   = *node_cfg_offset;
	num_cfg_all[node_type_id].sensor_cfg_offset = *sensor_cfg_offset;

	// all devs are serially numbered
	start_node_id[node_type_id] += node_type_cfg->node_num;

	// independent numbering in dev
	*node_cfg_offset += node_type_cfg->node_num;
	*sensor_cfg_offset += (node_type_cfg->node_num * node_type_cfg->sensor_num);
	return 0;
}

STATIC int32_t lpm_fault_fill_dms_node_table(uint32_t dev_id, uint32_t *start_node_id)
{
	int32_t ret;
	uint32_t node_type_id;
	uint32_t node_cfg_offset = 0;
	uint32_t sensor_cfg_offset = 0;
	struct lpm_fault_priv *fault_priv = lpm_fault_get_priv();
	struct lpm_fault_dev_priv_cfg *cfg_info = &fault_priv->dev_priv[dev_id];

	for (node_type_id = 0; node_type_id < fault_priv->node_type_table.node_type_num; node_type_id++) {
		ret = lpm_fault_init_num_cfg(node_type_id, cfg_info->num_cfg_all, start_node_id,
			&node_cfg_offset, &sensor_cfg_offset);
		if (ret != 0) {
			lpm_log_err("init node num cfg failed, ret=%d, dev_id=%u\n", ret, dev_id);
			return ret;
		}

		ret = lpm_fault_init_node(cfg_info, dev_id, node_type_id);
		if (ret != 0) {
			lpm_log_err("init node table failed, ret=%d, dev_id=%u, node_type_id=%u\n",
				ret, dev_id, node_type_id);
			return ret;
		}
	}

	return 0;
}

STATIC bool lpm_fault_check_node_sensor_cfg(void)
{
	struct lpm_fault_priv *fault_priv = lpm_fault_get_priv();

	if (fault_priv->node_type_table.node_type_num == 0) {
		lpm_log_err("init fault priv failed, no node_type has been registered\n");
		return false;
	}
	if (fault_priv->sensor_table.sensor_num == 0) {
		lpm_log_err("init fault priv failed, no sensor has been registered\n");
		return false;
	}
	return true;
}

STATIC int32_t lpm_fault_init_fault_priv(uint32_t dev_num)
{
	int32_t ret;
	uint32_t dev_id;
	uint32_t start_node_id[LPM_FAULT_NODE_TYPE_NUM] = {0};

	if (!lpm_fault_check_node_sensor_cfg()) {
		return -1;
	}

	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		ret = lpm_fault_fill_dms_node_table(dev_id, start_node_id);
		if (ret != 0) {
			lpm_log_err("init fault priv failed, ret=%d, dev_num=%u, dev_id=%u\n",
				ret, dev_num, dev_id);
			return ret;
		}
	}
	return 0;
}

STATIC int32_t lpm_fault_unregister_one_node(struct lpm_fault_node_cfg *node_cfg)
{
	int32_t s_ret;
	uint32_t u_ret;
	int32_t total_ret = 0;

	u_ret = dms_sensor_node_unregister(&node_cfg->dms_node);
	if (u_ret != 0) {
		total_ret = -1;
		lpm_log_err("unregister sensor failed, ret=%u\n", u_ret);
	}

	s_ret = dms_unregister_dev_node(&node_cfg->dms_node);
	if (s_ret != 0) {
		total_ret = -1;
		lpm_log_err("unregister node failed, ret=%d\n", s_ret);
	}
	return total_ret;
}

STATIC int32_t lpm_fault_register_one_node(struct lpm_fault_node_cfg *node_cfg)
{
	int32_t s_ret;
	uint32_t u_ret;
	uint32_t sensor_inner_id;

	s_ret = dms_register_dev_node(&node_cfg->dms_node);
	if (s_ret != 0) {
		lpm_log_err("register lpm node failed, ret=%d.\n", s_ret);
		return s_ret;
	}

	for (sensor_inner_id = 0; sensor_inner_id < node_cfg->sensor_num; sensor_inner_id++) {
		u_ret = dms_sensor_register(&node_cfg->dms_node, &node_cfg->sensors_cfg[sensor_inner_id].sensors);
		if (u_ret != 0) {
			lpm_log_err("register sensor failed, ret=%d, sensor_inner_id=%u\n", u_ret, sensor_inner_id);
			goto register_sensor_failed;
		}
	}

	return 0;

register_sensor_failed:
	if (sensor_inner_id > 0) {
		u_ret = dms_sensor_node_unregister(&node_cfg->dms_node);
		if (u_ret != 0) {
			lpm_log_err("unregister sensor failed, ret=%u\n", u_ret);
		}
	}
	s_ret = dms_unregister_dev_node(&node_cfg->dms_node);
	if (s_ret != 0) {
		lpm_log_err("unregister node failed, ret=%d\n", s_ret);
	}
	return -1;
}

STATIC int32_t lpm_fault_unregister_one_node_type(
	uint32_t dev_id, struct lpm_fault_dms_node *node_info)
{
	uint32_t node_inner_id;
	int32_t ret;
	int32_t total_ret = 0;

	for (node_inner_id = 0; node_inner_id < node_info->node_num; node_inner_id++) {
		ret = lpm_fault_unregister_one_node(&node_info->node_cfg[node_inner_id]);
		if (ret != 0) {
			total_ret = -1;
			lpm_log_err("lpm fault unregister one node type failed, ret=%d, dev_id=%u, node_inner_id=%u\n",
				ret, dev_id, node_inner_id);
		}
	}

	return total_ret;
}

STATIC int32_t lpm_fault_register_one_node_type(
	uint32_t dev_id, struct lpm_fault_dms_node *node_info)
{
	uint32_t node_inner_id;
	uint32_t node_id;
	int32_t ret;

	for (node_inner_id = 0; node_inner_id < node_info->node_num; node_inner_id++) {
		 ret = lpm_fault_register_one_node(&node_info->node_cfg[node_inner_id]);
		 if (ret != 0) {
			lpm_log_err("lpm fault register one node failed, ret=%d, dev_id=%u, node_inner_id=%u\n",
				ret, dev_id, node_inner_id);
			goto node_register_failed;
		}
	}

	return 0;

node_register_failed:
	for (node_id = 0; node_id < node_inner_id; node_id++) {
		ret = lpm_fault_unregister_one_node(&node_info->node_cfg[node_id]);
		if (ret != 0) {
			lpm_log_err("lpm fault rollback register node failed, ret=%d, dev_id=%u, node_id=%u\n",
				ret, dev_id, node_id);
		}
	}
	return -1;
}

STATIC int32_t lpm_fault_unregister_dev_node(
	uint32_t dev_id, struct lpm_fault_dev_priv_cfg *dev_priv)
{
	uint32_t node_typd_id;
	int32_t ret;
	int32_t total_ret = 0;
	struct lpm_fault_priv *fault_priv = lpm_fault_get_priv();

	for (node_typd_id = 0; node_typd_id < fault_priv->node_type_table.node_type_num; node_typd_id++) {
		ret = lpm_fault_unregister_one_node_type(dev_id, &dev_priv->node_info[node_typd_id]);
		if (ret != 0) {
			total_ret = -1;
			lpm_log_err("lpm fault unregister dev node failed, ret=%d, dev_id=%u, node_typd_id=%u\n",
				ret, dev_id, node_typd_id);
		}
	}

	return total_ret;
}

STATIC int32_t lpm_fault_register_dev_node(
	uint32_t dev_id, struct lpm_fault_dev_priv_cfg *dev_priv)
{
	uint32_t node_typd_id;
	uint32_t node_id;
	int32_t ret;
	struct lpm_fault_priv *fault_priv = lpm_fault_get_priv();

	for (node_typd_id = 0; node_typd_id < fault_priv->node_type_table.node_type_num; node_typd_id++) {
		 ret = lpm_fault_register_one_node_type(dev_id, &dev_priv->node_info[node_typd_id]);
		 if (ret != 0) {
			lpm_log_err("lpm fault register one node failed, ret=%d, dev_id=%u, node_typd_id=%u\n",
				ret, dev_id, node_typd_id);
			goto node_register_failed;
		}
	}

	return 0;

node_register_failed:
	for (node_id = 0; node_id < node_typd_id; node_id++) {
		ret = lpm_fault_unregister_one_node_type(dev_id, &dev_priv->node_info[node_id]);
		if (ret != 0) {
			lpm_log_err("lpm fault rollback register one node failed, ret=%d, dev_id=%u, node_typd_id=%u\n",
				ret, dev_id, node_id);
		}
	}
	return -1;
}

STATIC int32_t lpm_fault_unregister_node(uint32_t dev_num)
{
	int32_t ret;
	int32_t total_ret = 0;
	uint32_t dev_id;
	struct lpm_fault_priv *fault_priv = lpm_fault_get_priv();

	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		ret = lpm_fault_unregister_dev_node(dev_id, &fault_priv->dev_priv[dev_id]);
		if (ret != 0) {
			total_ret = -1;
			lpm_log_err("lpm unregister node failed, ret=%d, dev_id=%u\n", ret, dev_id);
		}
	}
	return total_ret;
}

STATIC int32_t lpm_fault_register_node(uint32_t dev_num)
{
	uint32_t dev_id;
	uint32_t i;
	int32_t ret;
	struct lpm_fault_priv *fault_priv = lpm_fault_get_priv();

	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		 ret = lpm_fault_register_dev_node(dev_id, &fault_priv->dev_priv[dev_id]);
		 if (ret != 0) {
			lpm_log_err("lpm register node failed, ret=%d, dev_id=%u\n", ret, dev_id);
			goto node_register_failed;
		}
	}

	return 0;

node_register_failed:
	for (i = 0; i < dev_id; i++) {
		ret = lpm_fault_unregister_dev_node(i, &fault_priv->dev_priv[i]);
		if (ret != 0) {
			lpm_log_err("lpm rollback register node failed, ret=%d, dev_id=%u\n", ret, i);
		}
	}
	return -1;
}

STATIC bool lpm_fault_check_is_report(
	struct lpm_fault_sensor_cfg *sensors_cfg, uint32_t event_type, uint32_t event_status)
{
	bool report_flag = false;
	unsigned long irq_flags;

	spin_lock_irqsave(&sensors_cfg->alarm_lock, irq_flags);

	if (sensors_cfg->event_status[event_type] != event_status) {
		// alarm status change, need to report alarm
		report_flag = true;
		sensors_cfg->event_status[event_type] = event_status;
	} else {
		report_flag = false;
	}

	spin_unlock_irqrestore(&sensors_cfg->alarm_lock, irq_flags);

	return report_flag;
}

STATIC struct lpm_fault_sensor_cfg* lpm_fault_get_sensor_cfg_by_type(
	struct lpm_fault_event_data *event_data)
{
	uint32_t sensor_inner_id;
	struct lpm_fault_priv *fault_priv = lpm_fault_get_priv();
	struct lpm_fault_node_cfg *node_cfg = NULL;
	struct lpm_fault_sensor_cfg *sensors_cfg = NULL;

	node_cfg = fault_priv->dev_priv[event_data->dev_id].node_info[event_data->node_type_id].node_cfg;

	for (sensor_inner_id = 0; sensor_inner_id < node_cfg->sensor_num; sensor_inner_id++) {
		sensors_cfg = &node_cfg[event_data->node_inner_id].sensors_cfg[sensor_inner_id];
		if (sensors_cfg->sensors.sensor_type == event_data->sensor_type) {
			return sensors_cfg;
		}
	}
	return NULL;
}

STATIC void lpm_fault_event_report_immediately(
	struct lpm_fault_sensor_cfg *sensors_cfg, struct lpm_fault_event_data *event_data)
{
	int32_t ret;

	if (sensors_cfg->sensors.scan_module != (uint32_t)DMS_SERSOR_SCAN_NOTIFY) {
		return;
	}

	// trigger to report immediately
	ret = dms_sensor_event_notify(event_data->dev_id, &sensors_cfg->sensors);
	if (ret != 0) {
		lpm_log_err("report alarm immediately failed, ret=%d, node_type_id=%u, "
			"node_inner_id=%u, sensor_type=0x%x, event_type=0x%x, event_status=%u\n",
			ret, event_data->node_type_id, event_data->node_inner_id, event_data->sensor_type,
			event_data->event_type, event_data->event_status);
	} else {
		lpm_log_info("report alarm immediately, node_type_id=%u, "
			"node_inner_id=%u, sensor_type=0x%x, event_type=0x%x, event_status=%u\n",
			event_data->node_type_id, event_data->node_inner_id, event_data->sensor_type,
			event_data->event_type, event_data->event_status);
	}
}

STATIC bool lpm_fault_check_event_type(
	struct lpm_fault_sensor_cfg *sensors_cfg, struct lpm_fault_event_data *event_data)
{
	struct lpm_fault_sensor_obj_cfg *sensor_cfg = lpm_fault_get_sensor_cfg(sensors_cfg->sensor_type_id);
	uint32_t assert_mask;

	if (sensor_cfg == NULL) {
		return false;
	}

	assert_mask = sensor_cfg->assert_mask[event_data->node_type_id];
	if ((assert_mask & ((uint32_t)0x1 << event_data->event_type)) != 0) {
		return true;
	}
	return false;
}

STATIC int32_t lpm_fault_report_event(struct lpm_fault_event_data *event_data)
{
	bool report_flag = false;
	struct lpm_fault_sensor_cfg *sensors_cfg = NULL;

	sensors_cfg = lpm_fault_get_sensor_cfg_by_type(event_data);
	if (sensors_cfg == NULL) {
		lpm_log_err("failed to find sensor when report, node_type_id=%u, "
			"node_inner_id=%u, sensor_type=0x%x, event_type=0x%x, event_status=%u\n",
			event_data->node_type_id, event_data->node_inner_id, event_data->sensor_type,
			event_data->event_type, event_data->event_status);
		return -1;
	}

	if (!lpm_fault_check_event_type(sensors_cfg, event_data)) {
		lpm_log_err("event_type=0x%x not support, node_type_id=%u, "
			"node_inner_id=%u, sensor_type=0x%x, event_status=%u\n",
			event_data->event_type, event_data->node_type_id, event_data->node_inner_id,
			event_data->sensor_type, event_data->event_status);
		return -1;
	}

	// check is need report, filter repeat reports
	report_flag = lpm_fault_check_is_report(
		sensors_cfg, event_data->event_type, event_data->event_status);
	if (!report_flag) {
		return 0;
	}

	// if the notification fails to be reported immediately,
	// it can be reported through periodic scanning, which only affects the reporting period
	lpm_fault_event_report_immediately(sensors_cfg, event_data);
	return 0;
}

STATIC uint32_t lpm_fault_get_node_type_id(uint32_t node_type)
{
	uint32_t node_type_id;
	struct lpm_fault_priv *fault_priv = lpm_fault_get_priv();

	for (node_type_id = 0; node_type_id < fault_priv->node_type_table.node_type_num; node_type_id++) {
		if (fault_priv->node_type_table.node_type_cfg[node_type_id].node_type == node_type) {
			return node_type_id;
		}
	}

	// not found
	return LPM_FAULT_NODE_TYPE_NUM;
}

static bool lpm_fault_check_owner_node_registered(uint32_t node_type, struct lpm_fault_node_obj_table *node_type_table)
{
	uint32_t i;
	if (node_type == 0) {
		return true;
	}
	for (i = 0; i < node_type_table->node_type_num; ++i) {
		if (node_type_table->node_type_cfg[i].node_type == node_type) {
			return true;
		}
	}
	return false;
}

STATIC bool lpm_fault_check_register_node_table(struct lpm_fault_node_obj_register *node_cfg)
{
	uint32_t node_type_id;
	struct lpm_fault_priv *fault_priv = lpm_fault_get_priv();
	struct lpm_fault_node_obj_table *node_type_table = &fault_priv->node_type_table;

	if (node_cfg == NULL) {
		lpm_log_err("lpm register node table failed, node_cfg is null\n");
		return false;
	}

	if (node_cfg->node_num == 0) {
		lpm_log_err("lpm register node table failed, node_num is zero, node_type=0x%x\n",
			node_cfg->node_type);
		return false;
	}

	if (node_cfg->fn_init == NULL) {
		lpm_log_err("lpm register node table failed, fn_init is null, node_type=0x%x\n",
			node_cfg->node_type);
		return false;
	}

	if (node_cfg->fn_uninit == NULL) {
		lpm_log_err("lpm register node table failed, fn_uninit is null, node_type=0x%x\n",
			node_cfg->node_type);
		return false;
	}

	if (strlen(node_cfg->node_name) == 0) {
		lpm_log_err("lpm register node table failed, node_name is empty, node_type=0x%x\n",
			node_cfg->node_type);
		return false;
	}

	if ((node_cfg->max_sensor == 0) || (node_cfg->max_sensor > LPM_FAULT_SENSOR_TYPE_NUM)) {
		lpm_log_err("lpm register node table failed, max_sensor=%u out of range, node_type=0x%x\n",
			node_cfg->max_sensor, node_cfg->node_type);
		return false;
	}

	if (node_type_table->node_type_num >= LPM_FAULT_NODE_TYPE_NUM) {
		lpm_log_err("lpm register node table failed, node_type_num is full, node_type=0x%x\n",
			node_cfg->node_type);
		return false;
	}

	if (!lpm_fault_check_owner_node_registered(node_cfg->owner_node_type, node_type_table)) {
		lpm_log_err("lpm register node table failed, owner_node_type=%u hasn't been registered before node_type=%u\n",
			node_cfg->owner_node_type, node_cfg->node_type);
		return false;
	}
	// check repeat register
	for (node_type_id = 0; node_type_id < node_type_table->node_type_num; node_type_id++) {
		if (node_type_table->node_type_cfg[node_type_id].node_type == node_cfg->node_type) {
			lpm_log_err("lpm register node table repeated, node_type_id=%u, node_type=0x%x, node_num=%u\n",
				node_type_id, node_cfg->node_type, node_cfg->node_num);
			return false;
		}
	}

	return true;
}

STATIC bool lpm_fault_check_register_sensor_table(struct lpm_fault_sensor_obj_register *sensor_cfg)
{
	uint32_t sensor_id;
	struct lpm_fault_priv *fault_priv = lpm_fault_get_priv();
	struct lpm_fault_sensor_obj_table *sensor_table = &fault_priv->sensor_table;

	if (sensor_cfg == NULL) {
		lpm_log_err("lpm register sensor table failed, sensor_cfg is null\n");
		return false;
	}

	if (sensor_cfg->support_node_mask == 0) {
		lpm_log_err("lpm register sensor table failed, sensor_type=0x%x has no node type support\n",
			sensor_cfg->sensor_type);
		return false;
	}

	if (sensor_cfg->fn_get_event == NULL) {
		lpm_log_err("lpm register sensor table failed, sensor_type=0x%x fn_get_event is null\n",
			sensor_cfg->sensor_type);
		return false;
	}

	if (sensor_cfg->fn_assert_mask == NULL) {
		lpm_log_err("lpm register sensor table failed, sensor_type=0x%x fn_assert_mask is null\n",
			sensor_cfg->sensor_type);
		return false;
	}

	if (sensor_cfg->fn_deassert_mask == NULL) {
		lpm_log_err("lpm register sensor table failed, sensor_type=0x%x fn_deassert_mask is null\n",
			sensor_cfg->sensor_type);
		return false;
	}

	if (strlen((const char *)sensor_cfg->sensor_name) == 0) {
		lpm_log_err("lpm register sensor table failed, sensor_name is null, sensor_type=0x%x\n",
			sensor_cfg->sensor_type);
		return false;
	}
	if (sensor_table->sensor_num >= LPM_FAULT_SENSOR_TYPE_NUM) {
		lpm_log_err("lpm register sensor table failed, sensor_num=%u is full, sensor_type=0x%x\n",
			sensor_table->sensor_num, sensor_cfg->sensor_type);
		return false;
	}

	for (sensor_id = 0; sensor_id < sensor_table->sensor_num; sensor_id++) {
		if (sensor_table->sensor_cfg[sensor_id].sensor_type == sensor_cfg->sensor_type) {
			lpm_log_err("lpm register sensor table repeated, sensor_id=%u, sensor_type=0x%x\n",
				sensor_id, sensor_cfg->sensor_type);
			return false;
		}
	}

	if (fault_priv->node_type_table.node_type_num == 0) {
		lpm_log_err("lpm register sensor table failed, no node_type registered, sensor_type=0x%x\n",
			sensor_cfg->sensor_type);
		return false;
	}
	return true;
}

STATIC int32_t lpm_fault_set_support_sensor_list(uint32_t sensor_type_id,
	struct lpm_fault_sensor_obj_register *sensor_cfg, struct lpm_fault_sensor_obj_cfg *sensor_type_cfg)
{
	uint32_t node_type_id;
	struct lpm_fault_priv *fault_priv = lpm_fault_get_priv();
	struct lpm_fault_node_obj_cfg *node_type_cfg = NULL;

	for (node_type_id = 0; node_type_id < fault_priv->node_type_table.node_type_num; node_type_id++) {
		if ((sensor_cfg->support_node_mask & ((uint32_t)0x1 << node_type_id)) == 0) {
			continue;
		}

		node_type_cfg = &fault_priv->node_type_table.node_type_cfg[node_type_id];
		if (node_type_cfg->sensor_num >= node_type_cfg->max_sensor) {
			lpm_log_err("node_type=0x%x register sensor_num=%u should not large than %u\n",
				node_type_cfg->node_type, node_type_cfg->sensor_num, node_type_cfg->max_sensor);
			return -1;
		}

		node_type_cfg->support_sensor[node_type_cfg->sensor_num] = sensor_type_id;
		node_type_cfg->sensor_num++;

		sensor_type_cfg->assert_mask[node_type_id] = sensor_cfg->fn_assert_mask(node_type_cfg->node_type);
		sensor_type_cfg->deassert_mask[node_type_id] = sensor_cfg->fn_deassert_mask(node_type_cfg->node_type);
	}
	return 0;
}


int32_t lpm_fault_dev_node_exit(uint32_t dev_num)
{
	int32_t ret;

	ret = lpm_fault_unregister_node(dev_num);
	if (ret != 0) {
		lpm_log_err("lpm unregister node failed, ret=%d, dev_num=%u\n",
			ret, dev_num);
		return -1;
	}
	lpm_log_info("lpm dev node exit success, dev_num=%u\n", dev_num);
	return 0;
}

int32_t lpm_fault_dev_node_init(uint32_t dev_num)
{
	int32_t ret;

	if (dev_num > LPM_DMS_NODE_MAX_NUM) {
		lpm_log_err("lpm init node failed, dev_num=%u shoud not large than %u\n",
			dev_num, LPM_DMS_NODE_MAX_NUM);
		return -1;
	}

	ret = lpm_fault_init_fault_priv(dev_num);
	if (ret != 0) {
		lpm_log_err("lpm init priv data failed, ret=%d, dev_num=%u\n", ret, dev_num);
		return -1;
	}

	ret = lpm_fault_register_node(dev_num);
	if (ret != 0) {
		lpm_log_err("lpm register node failed, ret=%d, dev_num=%u\n", ret, dev_num);
		return -1;
	}
#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
	ret = lpm_alarm_debugfs_init();
	if (ret != 0) {
		(void)lpm_fault_unregister_node(dev_num);
		return ret;
	}
#endif
	lpm_log_info("lpm dev node init success, dev_num=%u\n", dev_num);
	return 0;
}

// should register before sensor register
int32_t lpm_fault_register_node_table(struct lpm_fault_node_obj_register *node_cfg)
{
	int32_t ret;
	struct lpm_fault_priv *fault_priv = lpm_fault_get_priv();
	struct lpm_fault_node_obj_table *node_type_table = &fault_priv->node_type_table;
	struct lpm_fault_node_obj_cfg *node_type_cfg = NULL;

	if (!lpm_fault_check_register_node_table(node_cfg)) {
		return -1;
	}

	node_type_cfg = &fault_priv->node_type_table.node_type_cfg[node_type_table->node_type_num];

	(void)memset_s(node_type_cfg, sizeof(struct lpm_fault_node_obj_cfg),
		0, sizeof(struct lpm_fault_node_obj_cfg));

	ret = memcpy_s(node_type_cfg->node_name, DMS_MAX_DEV_NAME_LEN, node_cfg->node_name, DMS_MAX_DEV_NAME_LEN);
	if (ret != 0) {
		lpm_log_err("lpm register node table failed for copy, node_type=0x%x, node_name=%s\n",
			node_cfg->node_type, node_cfg->node_name);
		return -1;
	}

	node_type_cfg->node_type       = node_cfg->node_type;
	node_type_cfg->owner_node_type = node_cfg->owner_node_type;
	node_type_cfg->node_num        = node_cfg->node_num;
	node_type_cfg->fn_init         = node_cfg->fn_init;
	node_type_cfg->fn_uninit       = node_cfg->fn_uninit;
	node_type_cfg->sensor_num      = 0;
	node_type_cfg->max_sensor      = node_cfg->max_sensor;

	lpm_log_info("lpm register node table success, node_type_id=%u, node_name=%s, node_type=0x%x,"
		"owner_node_type_id=0x%x, node_num=%u, max_sensor=%u\n",
		node_type_table->node_type_num, node_cfg->node_name, node_cfg->node_type, node_cfg->owner_node_type,
		node_cfg->node_num, node_cfg->max_sensor);

	// should set after log
	node_type_table->node_type_num++;
	return 0;
}

// this function needs to be executed before the report module initialization starts
int32_t lpm_fault_register_sensor_table(struct lpm_fault_sensor_obj_register *sensor_cfg)
{
	int32_t ret;
	struct lpm_fault_priv *fault_priv = lpm_fault_get_priv();
	struct lpm_fault_sensor_obj_table *sensor_table = &fault_priv->sensor_table;
	struct lpm_fault_sensor_obj_cfg *sensor_type_cfg = NULL;

	if (!lpm_fault_check_register_sensor_table(sensor_cfg)) {
		return -1;
	}

	sensor_type_cfg = &sensor_table->sensor_cfg[sensor_table->sensor_num];

	ret = memcpy_s(sensor_type_cfg->sensor_name,
		DMS_SENSOR_DESCRIPT_LENGTH, sensor_cfg->sensor_name, DMS_SENSOR_DESCRIPT_LENGTH);
	if (ret != 0) {
		lpm_log_err("lpm register sensor table failed for copy, sensor_type=0x%x\n",
			sensor_cfg->sensor_type);
		return -1;
	}

	ret = lpm_fault_set_support_sensor_list(sensor_table->sensor_num, sensor_cfg, sensor_type_cfg);
	if (ret != 0) {
		lpm_log_err("set support sensor list for node type failed, sensor_type=0x%x\n",
			sensor_cfg->sensor_type);
		return -1;
	}

	sensor_type_cfg->sensor_type       = sensor_cfg->sensor_type;
	sensor_type_cfg->support_node_mask = sensor_cfg->support_node_mask;
	sensor_type_cfg->fn_get_event      = sensor_cfg->fn_get_event;

	lpm_log_info("lpm register sensor table success, sensor_type_id=%u, "
		"sensor_type=0x%x, support_node_mask=0x%x\n",
		sensor_table->sensor_num, sensor_cfg->sensor_type, sensor_cfg->support_node_mask);

	// should add in last
	sensor_table->sensor_num++;
	return 0;
}

STATIC unsigned long long lpm_get_sys_cnt(void)
{
	unsigned long long syscnt;
#ifndef LPM_UT_TEST
	isb();
	asm volatile("mrs %0, CNTPCT_EL0" : "=r"(syscnt)::"memory");
#endif
	return syscnt;
}

int32_t lpm_handle_fault(const struct lpm_inner_fault_event *event)
{
	int32_t ret;
	uint32_t node_type_id;
	struct lpm_fault_event_data event_data;

	if (event == NULL) {
		lpm_log_err("lpm handle fault failed, event is null\n");
		return -EINVAL;
	}

	if (!lpm_common_check_dev_id(event->dev_id)) {
		lpm_log_err("dev_id=%u is out of range, "
			"node_type=0x%x, sensor_type=0x%x, event_type=0x%x, assertion=%u\n",
			event->dev_id, event->node_type, event->sensor_type, event->event_type, event->assertion);
		return -1;
	}

	node_type_id = lpm_fault_get_node_type_id(event->node_type);
	if (node_type_id == LPM_FAULT_NODE_TYPE_NUM) {
		lpm_log_err("can not found node type id, dev_id=%u, "
			"node_type=0x%x, sensor_type=0x%x, event_type=0x%x, assertion=%u\n",
			event->dev_id, event->node_type, event->sensor_type, event->event_type, event->assertion);
		return -1;
	}

	event_data.dev_id         = event->dev_id;
	event_data.node_type_id   = node_type_id;
	event_data.node_inner_id  = event->node_inner_id;
	event_data.sensor_type    = event->sensor_type;
	event_data.event_type     = event->event_type;
	event_data.event_status   = event->assertion;

	ret = lpm_fault_report_event(&event_data);
	lpm_log_info("fault handle finish, ret=%d, dev_id=%u, node_type=0x%x, sensor_type=0x%x, "
		"event_type=0x%x, event_status=%u, node_type_id=%u, node_inner_id=%u, sysCnt=%llu\n",
		ret, event_data.dev_id, event->node_type, event_data.sensor_type, event_data.event_type,
		event_data.event_status, event_data.node_type_id, event_data.node_inner_id, lpm_get_sys_cnt());

	return ret;
}

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
STATIC void lpm_fault_debugfs_show_event_info(
	struct seq_file *seq, struct lpm_fault_sensor_obj_cfg *sensor_cfg, uint32_t event_type, bool is_one_time)
{
	uint32_t event_id;
	uint32_t event_num = 0;
	char *ont_time_desc[] = {" is one_time event", "not one_time event"};
	struct lpm_fault_event_desc *event_desc = sensor_cfg->fn_get_event(&event_num);

	for (event_id = 0; event_id < event_num; event_id++) {
		if (event_desc[event_id].event_type == event_type) {
			seq_printf(seq, "          event_type=0x%x, %s, describe=%s\n",
				event_desc[event_id].event_type,
				is_one_time ? ont_time_desc[0] : ont_time_desc[1],
				event_desc[event_id].describe);
			return;
		}
	}

	seq_printf(seq, "   [ERROR]can not find event_type=0x%x info\n", event_type);
}

STATIC void lpm_fault_debugfs_show_event_list(
	struct seq_file *seq, struct lpm_fault_sensor_obj_cfg *sensor_cfg, uint32_t node_type_id)
{
	uint32_t assert_mask;
	uint32_t deassert_mask;
	uint32_t event_type;
	uint32_t event_cnt = 0;
	bool is_one_time = false;

	assert_mask   = sensor_cfg->assert_mask[node_type_id];
	deassert_mask = sensor_cfg->deassert_mask[node_type_id];

	for (event_type = 0; event_type < DMS_MAX_SENSOR_EVENT_COUNT; event_type++) {
		if ((assert_mask & ((uint32_t)0x1 << event_type)) == 0) {
			if ((deassert_mask & ((uint32_t)0x1 << event_type)) != 0) {
				seq_printf(seq, "         [WARN]not support event_type=0x%x, but set deassert_mask\n", event_type);
			}
			continue;
		}
		event_cnt++;

		if ((deassert_mask & ((uint32_t)0x1 << event_type)) == 0) {
			is_one_time = true;
		}
		lpm_fault_debugfs_show_event_info(seq, sensor_cfg, event_type, is_one_time);
	}

	if (event_cnt == 0) {
		seq_printf(seq, "         [ERROR]not support any event_type, assert_mask=0x%x, deassert_mask=0x%x\n",
			assert_mask, deassert_mask);
	}
}

STATIC void lpm_fault_debugfs_show_node_in_sensor(
	struct seq_file *seq, struct lpm_fault_sensor_obj_cfg *sensor_cfg)
{
	uint32_t node_type_id;
	uint32_t node_cnt = 0;
	struct lpm_fault_node_obj_cfg *node_type_cfg = NULL;
	struct lpm_fault_priv *fault_priv = lpm_fault_get_priv();

	seq_printf(seq, "     node_type detail:\n");

	for (node_type_id = 0; node_type_id < fault_priv->node_type_table.node_type_num; node_type_id++) {
		if ((sensor_cfg->support_node_mask & ((uint32_t)0x1 << node_type_id)) == 0) {
			continue;
		}

		node_cnt++;
		node_type_cfg = &fault_priv->node_type_table.node_type_cfg[node_type_id];
		seq_printf(seq, "       node_name=%s, event_type list:\n", node_type_cfg->node_name);
		lpm_fault_debugfs_show_event_list(seq, sensor_cfg, node_type_id);
	}

	if (node_cnt == 0) {
		seq_printf(seq, "       [ERROR]not support any node_type\n");
	}
}

STATIC void lpm_fault_debugfs_show_all_sensor(struct seq_file *seq)
{
	uint32_t sensor_type_id;
	struct lpm_fault_sensor_obj_cfg *sensor_cfg = NULL;
	struct lpm_fault_priv *fault_priv = lpm_fault_get_priv();

	seq_printf(seq, "[all sensor list, sensor_num=%u]\n", fault_priv->sensor_table.sensor_num);

	if (fault_priv->sensor_table.sensor_num != LPM_FAULT_SENSOR_TYPE_NUM) {
		seq_printf(seq, "   [ERROR]sensor_num=%u should equal to %u, some sensor has not been registered\n",
			fault_priv->sensor_table.sensor_num, LPM_FAULT_SENSOR_TYPE_NUM);
	}

	for (sensor_type_id = 0; sensor_type_id < fault_priv->sensor_table.sensor_num; sensor_type_id++) {
		sensor_cfg = &fault_priv->sensor_table.sensor_cfg[sensor_type_id];
		seq_printf(seq, "   id=%u, name=%s, type=0x%x, support_node_mask=0x%x\n",
			sensor_type_id, sensor_cfg->sensor_name, sensor_cfg->sensor_type, sensor_cfg->support_node_mask);
		lpm_fault_debugfs_show_node_in_sensor(seq, sensor_cfg);
	}
}

STATIC void lpm_fault_degbufs_check_sensor_cfg(
	struct seq_file *seq, uint32_t dev_id, uint32_t node_type_id, uint32_t node_id)
{
	struct lpm_fault_priv *fault_priv = lpm_fault_get_priv();
	struct lpm_fault_dms_node *node_info = &fault_priv->dev_priv[dev_id].node_info[node_type_id];
	struct lpm_fault_node_obj_cfg *node_type_cfg = &fault_priv->node_type_table.node_type_cfg[node_type_id];
	struct lpm_fault_node_cfg *node_cfg = &node_info->node_cfg[node_id];
	uint32_t sensor_id;
	uint32_t sensor_num = (node_cfg->sensor_num <= node_type_cfg->sensor_num) ?
		node_cfg->sensor_num : node_type_cfg->sensor_num;

	if (node_cfg->sensor_num != node_type_cfg->sensor_num) {
		seq_printf(seq, " [ERROR]dev_id=%u, node_type_id=%u, node_id=%u: sensor_num=%u should be %u\n",
			dev_id, node_type_id, node_id,
			node_cfg->sensor_num, node_type_cfg->sensor_num);
	}

	for (sensor_id = 0; sensor_id < sensor_num; sensor_id++) {
		if (node_cfg->sensors_cfg[sensor_id].sensor_type_id != node_type_cfg->support_sensor[sensor_id]) {
			seq_printf(seq, " [ERROR]dev_id=%u, node_type_id=%u, node_id=%u, sensor_id=%u:"
				"sensor_type_id=%u in node_cfg should be %u\n",
				dev_id, node_type_id, node_id, sensor_id,
				node_cfg->sensors_cfg[sensor_id].sensor_type_id, node_type_cfg->support_sensor[sensor_id]);
		}
	}
}

STATIC void lpm_fault_degbufs_check_node_id_cfg(
	struct seq_file *seq, uint32_t dev_id, uint32_t node_type_id)
{
	struct lpm_fault_priv *fault_priv = lpm_fault_get_priv();
	struct lpm_fault_dms_node *node_info = &fault_priv->dev_priv[dev_id].node_info[node_type_id];
	struct lpm_fault_node_obj_cfg *node_type_cfg = &fault_priv->node_type_table.node_type_cfg[node_type_id];
	struct lpm_fault_node_cfg *node_cfg = NULL;
	uint32_t node_id;

	if (node_info->node_num != node_type_cfg->node_num) {
		seq_printf(seq, " [ERROR]dev_id=%u, node_type_id=%u: node_num=%u should be %u\n",
			dev_id, node_type_id, node_info->node_num, node_type_cfg->node_num);
	}

	for (node_id = 0; node_id < node_info->node_num; node_id++) {
		node_cfg = &node_info->node_cfg[node_id];

		if (node_cfg->dms_node.node_type != (int32_t)node_type_cfg->node_type) {
			seq_printf(seq, " [ERROR]dev_id=%u, node_type_id=%u, node_id=%u: node_type=0x%x should be %u\n",
				dev_id, node_type_id, node_id, node_cfg->dms_node.node_type, node_type_cfg->node_type);
		}

		if (node_cfg->sensor_num == 0) {
			seq_printf(seq, " [ERROR]dev_id=%u, node_type_id=%u, node_id=%u:"
				"sensor_num is zero, should be %u\n",
				dev_id, node_type_id, node_id, node_type_cfg->sensor_num);
			continue;
		}

		lpm_fault_degbufs_check_sensor_cfg(seq, dev_id, node_type_id, node_id);
	}
}

STATIC void lpm_fault_degbufs_check_one_node_type(
	struct seq_file *seq, uint32_t dev_id, uint32_t node_type_id)
{
	struct lpm_fault_priv *fault_priv = lpm_fault_get_priv();
	struct lpm_fault_dms_node *node_info = &fault_priv->dev_priv[dev_id].node_info[node_type_id];
	struct lpm_fault_node_obj_cfg *node_type_cfg = &fault_priv->node_type_table.node_type_cfg[node_type_id];

	if (node_info->node_num == 0) {
		seq_printf(seq, " [ERROR]dev_id=%u, node_type_id=%u: node_num is zero should be %u\n",
			dev_id, node_type_id, node_type_cfg->node_num);
		return;
	}

	lpm_fault_degbufs_check_node_id_cfg(seq, dev_id, node_type_id);
}

STATIC void lpm_fault_degbufs_check_node_cfg(struct seq_file *seq)
{
	uint32_t dev_id;
	uint32_t node_type_id;
	uint32_t dev_num = lpm_common_get_dev_num();
	struct lpm_fault_priv *fault_priv = lpm_fault_get_priv();

	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		for (node_type_id = 0; node_type_id < fault_priv->node_type_table.node_type_num; node_type_id++) {
			lpm_fault_degbufs_check_one_node_type(seq, dev_id, node_type_id);
		}
	}
}

STATIC void lpm_fault_debugfs_show_node_cfg(struct seq_file *seq, uint32_t node_type_id)
{
	struct lpm_fault_priv *fault_priv = lpm_fault_get_priv();
	struct lpm_fault_node_obj_cfg *node_type_cfg = &fault_priv->node_type_table.node_type_cfg[node_type_id];

	seq_printf(seq, "  id=%u, node_name=%s, node_type=0x%x, node_num=%u, sensor_num=%u\n",
		node_type_id, node_type_cfg->node_name, node_type_cfg->node_type,
		node_type_cfg->node_num, node_type_cfg->sensor_num);

	lpm_fault_degbufs_check_node_cfg(seq);
}

STATIC void lpm_fault_debugfs_show_all_node(struct seq_file *seq)
{
	uint32_t node_type_id;
	struct lpm_fault_priv *fault_priv = lpm_fault_get_priv();

	seq_printf(seq, "[all node list, node_type_num=%u]\n", fault_priv->node_type_table.node_type_num);
	for (node_type_id = 0; node_type_id < fault_priv->node_type_table.node_type_num; node_type_id++) {
		lpm_fault_debugfs_show_node_cfg(seq, node_type_id);
	}
}

STATIC void lpm_fault_debugfs_show_node_sensor(struct seq_file *seq)
{
	// show all node_type supported
	lpm_fault_debugfs_show_all_node(seq);

	seq_printf(seq, "\n");

	// show all sensor_type_supported
	lpm_fault_debugfs_show_all_sensor(seq);
}

STATIC ssize_t lpm_fault_debugfs_alarm_write(
	struct file *file, const char __user *buf, size_t len, loff_t *pos)
{
	int32_t ret;
	uint32_t user_cfg[LPM_DEBUGFS_MAX_USER_INPUT_NUM] = {0};
	struct lpm_inner_fault_event event = {0};

	// dev_id, node_type, sensor_type, event_type, assertion
	ret = lpm_debugfs_get_input(buf, len, user_cfg, 0x5);
	if (ret != 0) {
		lpm_log_err("debugfs get alarm input failed, ret=%d\n", ret);
		return -1;
	}

	event.dev_id      = user_cfg[0x0];
	event.node_type   = user_cfg[0x1];
	event.sensor_type = user_cfg[0x2];
	event.event_type  = user_cfg[0x3];
	event.assertion   = user_cfg[0x4];
	ret = lpm_handle_fault(&event);
	if (ret != 0) {
		lpm_log_err("fault inject failed, ret=%d\n", ret);
	}

	(void)pos;
	(void)file;
	return (ssize_t)len;
}

STATIC int32_t lpm_fault_debugfs_alarm_show(struct seq_file *seq, void *v)
{
	(void)v;
	lpm_fault_debugfs_show_node_sensor(seq);
	return 0;
}

STATIC int32_t lpm_fault_debugfs_alarm_open(struct inode *inode, struct file *file)
{
	return single_open(file, lpm_fault_debugfs_alarm_show, inode->i_private);
}
STATIC int32_t lpm_alarm_debugfs_init(void)
{
	int32_t ret;
	struct lpm_debugfs_init_cfg init_cfg = {
		.name = "alarm",
		.fn_open = lpm_fault_debugfs_alarm_open,
		.fn_write = lpm_fault_debugfs_alarm_write
	};
	ret = lpm_debugfs_register_module(&init_cfg);
	if (ret != 0) {
		lpm_log_err("lpm alarm register debugfs failed, ret=%d\n", ret);
		return ret;
	}
	return 0;
}
#endif