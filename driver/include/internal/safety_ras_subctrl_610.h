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
#ifndef SAFETY_RAS_SUBCTRL_610_H
#define SAFETY_RAS_SUBCTRL_610_H

#include <linux/types.h>

#define DDR_SUBCTRL_SAFETY_EVENT_ID                 0x90DF8005U
#define DDR_SUBCTRL_SAFETY_CH_LEFT                  0U
#define DDR_SUBCTRL_SAFETY_CH_RIGHT                 1U

#define DDR_SUBCTRL_LEFT_CH_PADDR                   0x81320000ULL
#define DDR_SUBCTRL_RIGHT_CH_PADDR                  0x81310000ULL
#define DDR_SUBCTRL_BASE_ADDR_LEN                   0x10000U
#define DDR_SUBCTRL_STATUS_REG_OFFSET               0x2C00
#define DDR_SUBCTRL_CLEAR_REG_OFFSET                0x2C00
#define DDR_SUBCTRL_INT_MASK_REG_OFFSET             0x2C04
#define DDR_SUBCTRL_ERR_MASK_REG_OFFSET             0x2C08
#define DDR_SUBCTRL_FUN_INT_MASK_REG_OFFSET         0x2C0C

#define DDR_SUBCTRL_EVENT_MAX_NUM                   2U

#define DDR_SUBCTRL_NODE_OFFSET                     16U
#define DDR_SUBCTRL_NODE_MASK                       0xFFFFU
#define DDR_SUBCTRL_SENSOR_NODE_OFFSET              0x0U
#define DDR_SUBCTRL_SENSOR_NODE_MASK                0xFFFFU

#define ddr_subctrl_sensor_priv_data(_node_id, _sensor_type) \
	(((uint64_t)((uint32_t)(_node_id) & DDR_SUBCTRL_NODE_MASK) << DDR_SUBCTRL_NODE_OFFSET) | \
	((uint64_t)((uint32_t)(_sensor_type) & DDR_SUBCTRL_SENSOR_NODE_MASK) << DDR_SUBCTRL_SENSOR_NODE_OFFSET))

#define ddr_subctrl_sensor_obj(_type, _name, _class, _attr, _debounce, _interval, \
	_proc, _enable, _func, _data, _assert, _deassert) {                        \
	.sensor_type = (_type),              \
	.sensor_name = (_name),              \
	.sensor_class = (_class),            \
	.sensor_class_cfg = {              \
		.discrete_sensor = {           \
			.attribute = (_attr),        \
			.debounce_time = (_debounce) \
		}                              \
	},                                 \
	.scan_interval = (_interval),        \
	.proc_flag = (_proc),                \
	.enable_flag =  (_enable),           \
	.pf_scan_func = (_func),             \
	.private_data = (_data),             \
	.assert_event_mask = (_assert),      \
	.deassert_event_mask = (_deassert)   \
}

#define ddr_subctrl_dms_node_define(_id, _ops) { \
	.node_type = DMS_DEV_TYPE_DDR,            \
	.node_id = (_id),                         \
	.node_name = "ddr-subctrl",               \
	.capacity = 0x1,                          \
	.permission = 0x1,                        \
	.owner_devid = 0,                         \
	.ops = (_ops)                             \
}

#define ddr_subctrl_safety_coverage_item(_err_status, _describe, _sensor_type, _err_type) { \
	.subsys_id = DMS_DEV_TYPE_DDR,          \
	.module_id = 0,                         \
	.section_type = 0,                      \
	.ras_code.err_status = (_err_status),     \
	.describe = (_describe),                  \
	.sensor_type = (_sensor_type),            \
	.error_type = (_err_type)                 \
}

enum ddr_subctrl_node {
	DDR_SUBCTRL_NODE_0 = 0U,
	DDR_SUBCTRL_NODE_MAX
};

enum ddr_subctrl_safety_err_bit {
	DDR_SUBCTRL_PARITY_ERROR  = 0U,
	DDR_SUBCTRL_ERROR_MAX
};

#endif