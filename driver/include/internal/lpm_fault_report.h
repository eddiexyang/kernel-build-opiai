/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2022. All rights reserved.
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
#ifndef LPM_FAULT_REPORT_H
#define LPM_FAULT_REPORT_H

#include <linux/types.h>
#include <linux/list.h>
#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
#include <linux/seq_file.h>
#endif
#include <linux/spinlock.h>
#include "dms_sensor.h"
#include "dms_define.h"
#include "icm_interface.h"
#include "lpm_fault_common.h"

struct lpm_inner_fault_event {
	uint32_t dev_id;
	uint32_t node_type;
	uint32_t node_inner_id;
	uint32_t sensor_type;
	uint32_t event_type;
	uint32_t assertion;  // DMS_EVENT_TYPE_RESUME, DMS_EVENT_TYPE_OCCUR
};

#ifdef LPM_REPORT

#define LPM_FAULT_ASSERT_MASK 0
#define LPM_FAULT_DEASSERT_MASK 1

struct lpm_fault_sensor_cfg {
	uint32_t sensor_type_id;
	struct dms_sensor_object_cfg sensors;
	// 0:resume, 1:occur, 2:one time
	uint32_t event_status[DMS_MAX_SENSOR_EVENT_COUNT];
	spinlock_t alarm_lock;
};

struct lpm_fault_node_cfg {
	struct dms_node dms_node; // node type信息
	struct dms_node_operations node_ops;
	uint32_t sensor_num;
	struct lpm_fault_sensor_cfg *sensors_cfg; // sensor信息
};

// specify dev_id, specify node_type, the number of nodes
struct lpm_fault_dms_node {
	uint32_t node_num;
	struct lpm_fault_node_cfg *node_cfg;
};

struct lpm_fault_node_num_cfg {
	uint32_t node_start_id;
	uint32_t node_cfg_offset;
	uint32_t sensor_cfg_offset;
};

struct lpm_fault_dev_priv_cfg {
	// node types num configuration information
	struct lpm_fault_node_num_cfg num_cfg_all[LPM_FAULT_NODE_TYPE_NUM];

	// node configuration information for all node types
	struct lpm_fault_node_cfg node_cfg_all[LPM_FAULT_NODE_NUM];

	// sensor configuration information for all node types
	struct lpm_fault_sensor_cfg sensor_cfg_all[LPM_FAULT_SENSOR_NUM];

	struct lpm_fault_dms_node node_info[LPM_FAULT_NODE_TYPE_NUM];
};

typedef struct lpm_fault_event_desc* (*fn_lpm_fault_get_event_info)(uint32_t *event_num);

struct lpm_fault_sensor_obj_cfg {
	uint8_t sensor_name[DMS_SENSOR_DESCRIPT_LENGTH]; // 20
	uint32_t sensor_type;
	// node type mask supported by the sensor
	uint32_t support_node_mask;
	uint32_t assert_mask[LPM_FAULT_NODE_TYPE_NUM];
	uint32_t deassert_mask[LPM_FAULT_NODE_TYPE_NUM];
	fn_lpm_fault_get_event_info fn_get_event;
};

struct lpm_fault_sensor_obj_table {
	uint32_t sensor_num;
	struct lpm_fault_sensor_obj_cfg sensor_cfg[LPM_FAULT_SENSOR_TYPE_NUM];
};

typedef int32_t (*fn_lpm_dms_node_ops_init)(struct dms_node *node);
typedef void (*fn_lpm_dms_node_ops_uninit)(struct dms_node *node);

struct lpm_fault_node_obj_cfg {
	char node_name[DMS_MAX_DEV_NAME_LEN];
	uint32_t node_type;
	uint32_t owner_node_type;
	uint32_t node_num;
	uint32_t max_sensor;
	uint32_t sensor_num;
	uint32_t support_sensor[LPM_FAULT_SENSOR_TYPE_NUM];
	fn_lpm_dms_node_ops_init fn_init;
	fn_lpm_dms_node_ops_uninit fn_uninit;
};

struct lpm_fault_node_obj_table {
	uint32_t node_type_num;
	struct lpm_fault_node_obj_cfg node_type_cfg[LPM_FAULT_NODE_TYPE_NUM];
};

struct lpm_fault_priv {
	// all node_type supportrd
	struct lpm_fault_node_obj_table node_type_table;

	// all sensor supported
	struct lpm_fault_sensor_obj_table sensor_table;

	struct lpm_fault_dev_priv_cfg dev_priv[LPM_DMS_NODE_MAX_NUM];
};

struct lpm_fault_event_desc {
	uint32_t event_type;
	uint8_t describe[DMS_MAX_EVENT_DATA_LENGTH];
};

struct lpm_fault_scan_priv_data {
	uint32_t sensor_inner_id;
	uint32_t node_inner_id;
	uint32_t node_type_id;
	uint32_t dev_id;
};

struct lpm_fault_event_data {
	uint32_t dev_id;
	uint32_t node_type_id;
	uint32_t node_inner_id;
	uint32_t sensor_type;
	uint32_t event_type;
	// 0:resume, 1:occur, 2:one time
	uint32_t event_status;
};


typedef uint32_t *(*fn_lpm_fault_get_sensor_list)(uint32_t *table_num);
typedef int32_t (*fn_lpm_fault_init_node_table)(
	uint32_t dev_id, struct dms_node *node, uint32_t node_id);
typedef uint32_t (*fn_lpm_fault_get_event_mask)(uint32_t node_type);

struct lpm_fault_node_obj_register {
	char node_name[DMS_MAX_DEV_NAME_LEN];
	uint32_t node_type;
	uint32_t owner_node_type;
	uint32_t node_num;
	uint32_t max_sensor;
	fn_lpm_dms_node_ops_init fn_init;
	fn_lpm_dms_node_ops_uninit fn_uninit;
};

struct lpm_fault_sensor_obj_register {
	uint8_t sensor_name[DMS_SENSOR_DESCRIPT_LENGTH]; // 20
	uint32_t sensor_type;
	// node type mask supported by the sensor
	uint32_t support_node_mask;
	fn_lpm_fault_get_event_mask fn_assert_mask;
	fn_lpm_fault_get_event_mask fn_deassert_mask;
	fn_lpm_fault_get_event_info fn_get_event;
};

int32_t lpm_fault_dev_node_exit(uint32_t dev_num);
int32_t lpm_fault_dev_node_init(uint32_t dev_num);
int32_t lpm_fault_register_sensor_table(struct lpm_fault_sensor_obj_register *sensor_cfg);
int32_t lpm_fault_register_node_table(struct lpm_fault_node_obj_register *node_cfg);
int32_t lpm_handle_fault(const struct lpm_inner_fault_event *event);

#else
static inline int32_t lpm_fault_dev_node_exit(uint32_t dev_num)
{
	(void)dev_num;
	return 0;
}
static inline int32_t lpm_fault_dev_node_init(uint32_t dev_num)
{
	(void)dev_num;
	return 0;
}

static inline int32_t lpm_handle_fault(const struct lpm_inner_fault_event *event)
{
	(void)event;
	return 0;
}

#endif

#endif
