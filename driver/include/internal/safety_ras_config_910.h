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

#ifndef SAFETY_RAS_CONFIG_910_H
#define SAFETY_RAS_CONFIG_910_H
#include <linux/string.h>
#include <linux/notifier.h>
#include <linux/cper.h>
#include <linux/uuid.h>
#include <acpi/ghes.h>
#include <acpi/apei.h>

#include "memory_fault_init_cfg.h"
#include "hbm_isolation_common.h"

#define SCAN_FUNC memory_fault_event_scan

#define AST_MASK 0xFFF /* DMS_SEN_TYPE_RAS_SENSOR assert_event_mask */
#define DST_MASK 0xFBF /* DMS_SEN_TYPE_RAS_SENSOR deassert_event_mask */
#define M_AST_MASK 0x3FFF /* DMS_SEN_TYPE_MEMORY deassert_event_mask */
#define M_DST_MASK 0x3DDF /* DMS_SEN_TYPE_MEMORY deassert_event_mask */
#define T_AST_MASK 0xFFF /* DMS_SEN_TYPE_TEMPERATURE deassert_event_mask */
#define T_DST_MASK 0xDFF /* DMS_SEN_TYPE_TEMPERATURE deassert_event_mask */

#define HBMC_MODULE 0x2EU
#define HBMA_MODULE 0x14U

STATIC struct dms_node_operations g_hbm_ops = {
	.init = hbm_ops_init,
	.uninit = hbm_ops_exit,
	.get_info_list = hbm_get_dev_info_list,
	.get_state = hbm_get_dev_state,
	.get_capacity = hbm_get_dev_capacity,
	.set_power_state = hbm_set_dev_power_state,
	.scan = NULL,
	.fault_diag = NULL,
	.event_notify = NULL,
	.get_link_state = NULL,
	.set_link_state = NULL
};

STATIC struct dms_node_operations g_ddr_ops = {
	.init = ddr_ops_init,
	.uninit = ddr_ops_exit,
	.get_info_list = ddr_get_dev_info_list,
	.get_state = ddr_get_dev_state,
	.get_capacity = ddr_get_dev_capacity,
	.set_power_state = ddr_set_dev_power_state,
	.scan = NULL,
	.fault_diag = NULL,
	.event_notify = NULL,
	.get_link_state = NULL,
	.set_link_state = NULL
};

STATIC struct dms_node_operations g_hha_ops = {
	.init = hha_ops_init,
	.uninit = hha_ops_exit,
	.get_info_list = hha_get_dev_info_list,
	.get_state = hha_get_dev_state,
	.get_capacity = hha_get_dev_capacity,
	.set_power_state = hha_set_dev_power_state,
	.scan = NULL,
	.fault_diag = NULL,
	.event_notify = NULL,
	.get_link_state = NULL,
	.set_link_state = NULL
};

STATIC struct dms_node_operations g_dha_ops = {
	.init = dha_ops_init,
	.uninit = dha_ops_exit,
	.get_info_list = dha_get_dev_info_list,
	.get_state = dha_get_dev_state,
	.get_capacity = dha_get_dev_capacity,
	.set_power_state = dha_set_dev_power_state,
	.scan = NULL,
	.fault_diag = NULL,
	.event_notify = NULL,
	.get_link_state = NULL,
	.set_link_state = NULL
};

STATIC struct dms_node g_memory_dev_node_table[MEMORY_NODE_ID_MAX][MEMORY_SENSOR_ID_MAX] = {
	{ /* dev_node 0 */
		memory_node_def(DMS_DEV_TYPE_HBM, "hbm", MEMORY_NODE_0, MEMORY_SENSOR_HBMC, g_hbm_ops),
		memory_node_def(DMS_DEV_TYPE_DDR, "ddr", MEMORY_NODE_0, MEMORY_SENSOR_DDRC, g_ddr_ops),
		memory_node_def(DMS_DEV_TYPE_HBMA, "hha", MEMORY_NODE_0, MEMORY_SENSOR_HHA, g_hha_ops),
		memory_node_def(DMS_DEV_TYPE_DDRA, "dha", MEMORY_NODE_0, MEMORY_SENSOR_DHA, g_dha_ops),
	},
	{ /* dev_node 1 */
		memory_node_def(DMS_DEV_TYPE_HBM, "hbm", MEMORY_NODE_1, MEMORY_SENSOR_HBMC, g_hbm_ops),
		memory_node_def(DMS_DEV_TYPE_DDR, "ddr", MEMORY_NODE_1, MEMORY_SENSOR_DDRC, g_ddr_ops),
		memory_node_def(DMS_DEV_TYPE_HBMA, "hha", MEMORY_NODE_1, MEMORY_SENSOR_HHA, g_hha_ops),
		memory_node_def(DMS_DEV_TYPE_DDRA, "dha", MEMORY_NODE_1, MEMORY_SENSOR_DHA, g_dha_ops),
	},
	{ /* dev_node 2 */
		memory_node_def(DMS_DEV_TYPE_HBM, "hbm", MEMORY_NODE_2, MEMORY_SENSOR_HBMC, g_hbm_ops),
		memory_node_def(DMS_DEV_TYPE_DDR, "ddr", MEMORY_NODE_2, MEMORY_SENSOR_DDRC, g_ddr_ops),
		memory_node_def(DMS_DEV_TYPE_HBMA, "hha", MEMORY_NODE_2, MEMORY_SENSOR_HHA, g_hha_ops),
		memory_node_def(DMS_DEV_TYPE_DDRA, "dha", MEMORY_NODE_2, MEMORY_SENSOR_DHA, g_dha_ops),
	},
	{ /* dev_node 3 */
		memory_node_def(DMS_DEV_TYPE_HBM, "hbm", MEMORY_NODE_3, MEMORY_SENSOR_HBMC, g_hbm_ops),
		memory_node_def(DMS_DEV_TYPE_DDR, "ddr", MEMORY_NODE_3, MEMORY_SENSOR_DDRC, g_ddr_ops),
		memory_node_def(DMS_DEV_TYPE_HBMA, "hha", MEMORY_NODE_3, MEMORY_SENSOR_HHA, g_hha_ops),
		memory_node_def(DMS_DEV_TYPE_DDRA, "dha", MEMORY_NODE_3, MEMORY_SENSOR_DHA, g_dha_ops),
	}
};

/* ddr sensor table */
STATIC struct dms_sensor_object_cfg g_ddr_sensor_table[MEMORY_NODE_ID_MAX][DDR_SENSOR_NUM] = {
	{ /* dev_node 0 */
		memory_sensor_def(TYPE_RAS, "ddr_0_0", 0UL, MEMORY_SENSOR_DDRC, S_IDX0, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_RAS, "ddr_0_1", 0UL, MEMORY_SENSOR_DDRC, S_IDX1, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_MEM, "ddr_0_2", 0UL, MEMORY_SENSOR_DDRC, S_IDX2, M_AST_MASK, M_DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_MEM, "ddr_0_3", 0UL, MEMORY_SENSOR_DDRC, S_IDX3, M_AST_MASK, M_DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
	},
	{ /* dev_node 1 */
		memory_sensor_def(TYPE_RAS, "ddr_1_0", 1UL, MEMORY_SENSOR_DDRC, S_IDX0, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_RAS, "ddr_1_1", 1UL, MEMORY_SENSOR_DDRC, S_IDX1, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_MEM, "ddr_1_2", 1UL, MEMORY_SENSOR_DDRC, S_IDX2, M_AST_MASK, M_DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_MEM, "ddr_1_3", 1UL, MEMORY_SENSOR_DDRC, S_IDX3, M_AST_MASK, M_DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
	},
	{ /* dev_node 2 */
		memory_sensor_def(TYPE_RAS, "ddr_2_0", 2UL, MEMORY_SENSOR_DDRC, S_IDX0, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_RAS, "ddr_2_1", 2UL, MEMORY_SENSOR_DDRC, S_IDX1, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_MEM, "ddr_2_2", 2UL, MEMORY_SENSOR_DDRC, S_IDX2, M_AST_MASK, M_DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_MEM, "ddr_2_3", 2UL, MEMORY_SENSOR_DDRC, S_IDX3, M_AST_MASK, M_DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
	},
	{ /* dev_node 3 */
		memory_sensor_def(TYPE_RAS, "ddr_3_0", 3UL, MEMORY_SENSOR_DDRC, S_IDX0, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_RAS, "ddr_3_1", 3UL, MEMORY_SENSOR_DDRC, S_IDX1, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_MEM, "ddr_3_2", 3UL, MEMORY_SENSOR_DDRC, S_IDX2, M_AST_MASK, M_DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_MEM, "ddr_3_3", 3UL, MEMORY_SENSOR_DDRC, S_IDX3, M_AST_MASK, M_DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
	}
};

/* hha sensor table */
STATIC struct dms_sensor_object_cfg g_hha_sensor_table[MEMORY_NODE_ID_MAX][HHA_SENSOR_NUM] = {
	{ /* dev_node 0 */
		memory_sensor_def(TYPE_RAS, "hha_0_0", 0UL, MEMORY_SENSOR_HHA, S_IDX0, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_RAS, "hha_0_1", 0UL, MEMORY_SENSOR_HHA, S_IDX1, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_RAS, "hha_0_2", 0UL, MEMORY_SENSOR_HHA, S_IDX2, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_RAS, "hha_0_3", 0UL, MEMORY_SENSOR_HHA, S_IDX3, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_RAS, "hha_0_4", 0UL, MEMORY_SENSOR_HHA, S_IDX4, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_RAS, "hha_0_5", 0UL, MEMORY_SENSOR_HHA, S_IDX5, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_RAS, "hha_0_6", 0UL, MEMORY_SENSOR_HHA, S_IDX6, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_RAS, "hha_0_7", 0UL, MEMORY_SENSOR_HHA, S_IDX7, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_MEM, "hha_memroy_0_0", 0UL, MEMORY_SENSOR_HHA, S_IDX8, M_AST_MASK, M_DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
	},
	{ /* dev_node 1 */
		memory_sensor_def(TYPE_RAS, "hha_1_0", 1UL, MEMORY_SENSOR_HHA, S_IDX0, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_RAS, "hha_1_1", 1UL, MEMORY_SENSOR_HHA, S_IDX1, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_RAS, "hha_1_2", 1UL, MEMORY_SENSOR_HHA, S_IDX2, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_RAS, "hha_1_3", 1UL, MEMORY_SENSOR_HHA, S_IDX3, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_RAS, "hha_1_4", 1UL, MEMORY_SENSOR_HHA, S_IDX4, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_RAS, "hha_1_5", 1UL, MEMORY_SENSOR_HHA, S_IDX5, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_RAS, "hha_1_6", 1UL, MEMORY_SENSOR_HHA, S_IDX6, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_RAS, "hha_1_7", 1UL, MEMORY_SENSOR_HHA, S_IDX7, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_MEM, "hha_memroy_1_0", 1UL, MEMORY_SENSOR_HHA, S_IDX8, M_AST_MASK, M_DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
	},
	{ /* dev_node 2 */
		memory_sensor_def(TYPE_RAS, "hha_2_0", 2UL, MEMORY_SENSOR_HHA, S_IDX0, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_RAS, "hha_2_1", 2UL, MEMORY_SENSOR_HHA, S_IDX1, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_RAS, "hha_2_2", 2UL, MEMORY_SENSOR_HHA, S_IDX2, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_RAS, "hha_2_3", 2UL, MEMORY_SENSOR_HHA, S_IDX3, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_RAS, "hha_2_4", 2UL, MEMORY_SENSOR_HHA, S_IDX4, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_RAS, "hha_2_5", 2UL, MEMORY_SENSOR_HHA, S_IDX5, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_RAS, "hha_2_6", 2UL, MEMORY_SENSOR_HHA, S_IDX6, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_RAS, "hha_2_7", 2UL, MEMORY_SENSOR_HHA, S_IDX7, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_MEM, "hha_memroy_2_0", 2UL, MEMORY_SENSOR_HHA, S_IDX8, M_AST_MASK, M_DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
	},
	{ /* dev_node 3 */
		memory_sensor_def(TYPE_RAS, "hha_3_0", 3UL, MEMORY_SENSOR_HHA, S_IDX0, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_RAS, "hha_3_1", 3UL, MEMORY_SENSOR_HHA, S_IDX1, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_RAS, "hha_3_2", 3UL, MEMORY_SENSOR_HHA, S_IDX2, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_RAS, "hha_3_3", 3UL, MEMORY_SENSOR_HHA, S_IDX3, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_RAS, "hha_3_4", 3UL, MEMORY_SENSOR_HHA, S_IDX4, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_RAS, "hha_3_5", 3UL, MEMORY_SENSOR_HHA, S_IDX5, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_RAS, "hha_3_6", 3UL, MEMORY_SENSOR_HHA, S_IDX6, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_RAS, "hha_3_7", 3UL, MEMORY_SENSOR_HHA, S_IDX7, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_MEM, "hha_memroy_3_0", 3UL, MEMORY_SENSOR_HHA, S_IDX8, M_AST_MASK, M_DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
	}
};

/* dha sensor table */
STATIC struct dms_sensor_object_cfg g_dha_sensor_table[MEMORY_NODE_ID_MAX][DHA_SENSOR_NUM] = {
	{ /* dev_node 0 */
		memory_sensor_def(TYPE_RAS, "dha_0_0", 0UL, MEMORY_SENSOR_DHA, S_IDX0, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
	},
	{ /* dev_node 1 */
		memory_sensor_def(TYPE_RAS, "dha_1_0", 1UL, MEMORY_SENSOR_DHA, S_IDX0, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
	},
	{ /* dev_node 2 */
		memory_sensor_def(TYPE_RAS, "dha_2_0", 2UL, MEMORY_SENSOR_DHA, S_IDX0, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
	},
	{ /* dev_node 3 */
		memory_sensor_def(TYPE_RAS, "dha_3_0", 3UL, MEMORY_SENSOR_DHA, S_IDX0, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
	}
};

/* hbm sensor table */
STATIC struct dms_sensor_object_cfg g_hbm_sensor_table[MEMORY_NODE_ID_MAX][HBM_SENSOR_NUM] = {
	{ /* dev_node 0 */
		memory_sensor_def(TYPE_RAS, "hbm_0_0", 0UL, MEMORY_SENSOR_HBMC, S_IDX0, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_RAS, "hbm_0_1", 0UL, MEMORY_SENSOR_HBMC, S_IDX1, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_RAS, "hbm_0_2", 0UL, MEMORY_SENSOR_HBMC, S_IDX2, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_RAS, "hbm_0_3", 0UL, MEMORY_SENSOR_HBMC, S_IDX3, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_MEM, "hbm_0_4", 0UL, MEMORY_SENSOR_HBMC, S_IDX4, M_AST_MASK, M_DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_MEM, "hbm_0_5", 0UL, MEMORY_SENSOR_HBMC, S_IDX5, M_AST_MASK, M_DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_MEM, "hbm_0_6", 0UL, MEMORY_SENSOR_HBMC, S_IDX6, M_AST_MASK, M_DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_MEM, "hbm_0_7", 0UL, MEMORY_SENSOR_HBMC, S_IDX7, M_AST_MASK, M_DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_MEM, "memory_0_0", 0UL, HBM_MEMORY_SENSOR, S_IDX8, 0xFF, 0xFF,
			HBM_ECC_SENSOR_SCAN_INTERVAL, hbm_unisolated_multi_ecc_event_scan),
		memory_sensor_def(DMS_SEN_TYPE_MEMORY_ERR_RECORD, "mem_err_rec_0_0", 0UL, HBM_MEMORY_ERROR_RECORD_SENSOR,
			S_IDX9, 0xFF, 0xFE, HBM_ECC_SENSOR_SCAN_INTERVAL, hbm_multi_ecc_fault_event_scan),
		memory_sensor_def(DMS_SEN_TYPE_TEMPERATURE, "hbm_temp_0", 0UL, MEMORY_SENSOR_HBMC, S_IDX10, T_AST_MASK,
			T_DST_MASK, DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
	},
	{ /* dev_node 1 */
		memory_sensor_def(TYPE_RAS, "hbm_1_0", 1UL, MEMORY_SENSOR_HBMC, S_IDX0, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_RAS, "hbm_1_1", 1UL, MEMORY_SENSOR_HBMC, S_IDX1, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_RAS, "hbm_1_2", 1UL, MEMORY_SENSOR_HBMC, S_IDX2, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_RAS, "hbm_1_3", 1UL, MEMORY_SENSOR_HBMC, S_IDX3, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_MEM, "hbm_1_4", 1UL, MEMORY_SENSOR_HBMC, S_IDX4, M_AST_MASK, M_DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_MEM, "hbm_1_5", 1UL, MEMORY_SENSOR_HBMC, S_IDX5, M_AST_MASK, M_DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_MEM, "hbm_1_6", 1UL, MEMORY_SENSOR_HBMC, S_IDX6, M_AST_MASK, M_DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_MEM, "hbm_1_7", 1UL, MEMORY_SENSOR_HBMC, S_IDX7, M_AST_MASK, M_DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_MEM, "memory_1_0", 1UL, HBM_MEMORY_SENSOR, S_IDX8, 0xFF, 0xFF,
			HBM_ECC_SENSOR_SCAN_INTERVAL, hbm_unisolated_multi_ecc_event_scan),
		memory_sensor_def(DMS_SEN_TYPE_MEMORY_ERR_RECORD, "mem_err_rec_1_0", 1UL, HBM_MEMORY_ERROR_RECORD_SENSOR,
			S_IDX9, 0xFF, 0xFE, HBM_ECC_SENSOR_SCAN_INTERVAL, hbm_multi_ecc_fault_event_scan),
		memory_sensor_def(DMS_SEN_TYPE_TEMPERATURE, "hbm_temp_0", 0UL, MEMORY_SENSOR_HBMC, S_IDX10, T_AST_MASK,
			T_DST_MASK, DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
	},
	{ /* dev_node 2 */
		memory_sensor_def(TYPE_RAS, "hbm_2_0", 2UL, MEMORY_SENSOR_HBMC, S_IDX0, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_RAS, "hbm_2_1", 2UL, MEMORY_SENSOR_HBMC, S_IDX1, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_RAS, "hbm_2_2", 2UL, MEMORY_SENSOR_HBMC, S_IDX2, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_RAS, "hbm_2_3", 2UL, MEMORY_SENSOR_HBMC, S_IDX3, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_MEM, "hbm_2_4", 2UL, MEMORY_SENSOR_HBMC, S_IDX4, M_AST_MASK, M_DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_MEM, "hbm_2_5", 2UL, MEMORY_SENSOR_HBMC, S_IDX5, M_AST_MASK, M_DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_MEM, "hbm_2_6", 2UL, MEMORY_SENSOR_HBMC, S_IDX6, M_AST_MASK, M_DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_MEM, "hbm_2_7", 2UL, MEMORY_SENSOR_HBMC, S_IDX7, M_AST_MASK, M_DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_MEM, "memory_2_0", 2UL, HBM_MEMORY_SENSOR, S_IDX8, 0xFF, 0xFF,
			HBM_ECC_SENSOR_SCAN_INTERVAL, hbm_unisolated_multi_ecc_event_scan),
		memory_sensor_def(DMS_SEN_TYPE_MEMORY_ERR_RECORD, "mem_err_rec_2_0", 2UL, HBM_MEMORY_ERROR_RECORD_SENSOR,
			S_IDX9, 0xFF, 0xFE, HBM_ECC_SENSOR_SCAN_INTERVAL, hbm_multi_ecc_fault_event_scan),
		memory_sensor_def(DMS_SEN_TYPE_TEMPERATURE, "hbm_temp_0", 0UL, MEMORY_SENSOR_HBMC, S_IDX10, T_AST_MASK,
			T_DST_MASK, DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
	},
	{ /* dev_node 3 */
		memory_sensor_def(TYPE_RAS, "hbm_3_0", 3UL, MEMORY_SENSOR_HBMC, S_IDX0, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_RAS, "hbm_3_1", 3UL, MEMORY_SENSOR_HBMC, S_IDX1, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_RAS, "hbm_3_2", 3UL, MEMORY_SENSOR_HBMC, S_IDX2, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_RAS, "hbm_3_3", 3UL, MEMORY_SENSOR_HBMC, S_IDX3, AST_MASK, DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_MEM, "hbm_3_4", 3UL, MEMORY_SENSOR_HBMC, S_IDX4, M_AST_MASK, M_DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_MEM, "hbm_3_5", 3UL, MEMORY_SENSOR_HBMC, S_IDX5, M_AST_MASK, M_DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_MEM, "hbm_3_6", 3UL, MEMORY_SENSOR_HBMC, S_IDX6, M_AST_MASK, M_DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_MEM, "hbm_3_7", 3UL, MEMORY_SENSOR_HBMC, S_IDX7, M_AST_MASK, M_DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(TYPE_MEM, "memory_3_0", 3UL, HBM_MEMORY_SENSOR, S_IDX8, 0xFF, 0xFF,
			HBM_ECC_SENSOR_SCAN_INTERVAL, hbm_unisolated_multi_ecc_event_scan),
		memory_sensor_def(DMS_SEN_TYPE_MEMORY_ERR_RECORD, "mem_err_rec_3_0", 3UL, HBM_MEMORY_ERROR_RECORD_SENSOR,
			S_IDX9, 0xFF, 0xFE, HBM_ECC_SENSOR_SCAN_INTERVAL, hbm_multi_ecc_fault_event_scan),
		memory_sensor_def(DMS_SEN_TYPE_TEMPERATURE, "hbm_temp_0", 0UL, MEMORY_SENSOR_HBMC, S_IDX10, T_AST_MASK,
			T_DST_MASK, DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
	}
};

#endif /* SAFETY_RAS_CONFIG_910_H */
