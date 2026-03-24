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
#include "dms_dev_node.h"

#include "safety_ras_ipc.h"
#include "safety_ras_init_310.h"
#include "safety_ras_config_310.h"

#include "safety_ras_mata_310.h"
#ifdef CFG_SOC_PLATFORM_MDC_V11
#include "safety_ras_reg_check_310.h"
#endif

struct drv_memory_devices *g_memory_devs_ptr = NULL;

typedef int32_t (* drv_step)(void);
typedef struct {
	drv_step action;          // 驱动初始化正向步骤
	drv_step rbkaction;       // 异常场景回滚动作
} mem_drv;

int32_t ddr_ops_init(struct dms_node *device)
{
	if (device != NULL)
		memory_drv_debug("DEBUG (node_name=%s)\n", device->node_name);

	return 0;
}

void ddr_ops_exit(struct dms_node *device)
{
	if (device != NULL)
		memory_drv_debug("DEBUG (node_name=%s)\n", device->node_name);

	return;
}

STATIC struct dms_node_operations g_ddr_ops = {
	.init = ddr_ops_init,
	.uninit = ddr_ops_exit,
	.scan = NULL,
	.fault_diag = NULL,
	.event_notify = NULL,
	.get_link_state = NULL,
	.set_link_state = NULL
};

/* memory dev node table */
STATIC struct dms_node g_memory_dev_node_table[MEMORY_NODE_ID_MAX][MEMORY_SENSOR_ID_MAX] = {
	{ /* dev_node 0 */
		memory_node_def(DMS_DEV_TYPE_DDR, "ddr_0", MEMORY_NODE_0, MEMORY_SENSOR_DDRC, g_ddr_ops),
	}
};

/* ddr sensor table */
struct dms_sensor_object_cfg g_ddr_sensor_table[MEMORY_NODE_ID_MAX][S_IDX_MAX] = {
	{ /* dev_node 0 */
		memory_sensor_def(DMS_SEN_TYPE_RAS_SENSOR, "ddr_0_0", 0UL, MEMORY_SENSOR_DDRC, 0UL,
			R_AST_MASK, R_DST_MASK, DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(DMS_SEN_TYPE_RAS_SENSOR, "ddr_0_1", 0UL, MEMORY_SENSOR_DDRC, 1UL,
			R_AST_MASK, R_DST_MASK, DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(DMS_SEN_TYPE_RAS_SENSOR, "ddr_0_2", 0UL, MEMORY_SENSOR_DDRC, 2UL,
			R_AST_MASK, R_DST_MASK, DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(DMS_SEN_TYPE_RAS_SENSOR, "ddr_0_3", 0UL, MEMORY_SENSOR_DDRC, 3UL,
			R_AST_MASK, R_DST_MASK, DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(DMS_SEN_TYPE_RAS_SENSOR, "ddr_0_4", 0UL, MEMORY_SENSOR_DDRC, 4UL,
			R_AST_MASK, R_DST_MASK, DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(DMS_SEN_TYPE_RAS_SENSOR, "ddr_0_5", 0UL, MEMORY_SENSOR_DDRC, 5UL,
			R_AST_MASK, R_DST_MASK, DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),

		memory_sensor_def(DMS_SEN_TYPE_MEMORY, "ddr_0_6", 0UL, MEMORY_SENSOR_DDRC, 6UL,
			M_AST_MASK, M_DST_MASK, DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(DMS_SEN_TYPE_MEMORY, "ddr_0_7", 0UL, MEMORY_SENSOR_DDRC, 7UL,
			M_AST_MASK, M_DST_MASK, DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(DMS_SEN_TYPE_MEMORY, "ddr_0_8", 0UL, MEMORY_SENSOR_DDRC, 8UL,
			M_AST_MASK, M_DST_MASK, DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(DMS_SEN_TYPE_MEMORY, "ddr_0_9", 0UL, MEMORY_SENSOR_DDRC, 9UL,
			M_AST_MASK, M_DST_MASK, DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(DMS_SEN_TYPE_MEMORY, "ddr_0_10", 0UL, MEMORY_SENSOR_DDRC, 10UL,
			M_AST_MASK, M_DST_MASK, DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
		memory_sensor_def(DMS_SEN_TYPE_MEMORY, "ddr_0_11", 0UL, MEMORY_SENSOR_DDRC, 11UL,
			M_AST_MASK, M_DST_MASK, DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),

		memory_sensor_def(DMS_SEN_TYPE_TEMPERATURE, "ddr_0_12", 0UL, MEMORY_SENSOR_DDRC, 12UL,
			T_AST_MASK, T_DST_MASK, DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),

		memory_sensor_def(DMS_SEN_TYPE_SAFETY_SENSOR, "ddr_0_13", 0UL, MEMORY_SENSOR_DDRC, 13UL,
			S_AST_MASK, S_DST_MASK, DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),

		memory_sensor_def(DMS_SEN_TYPE_EXTEND_SENSOR, "ddr_0_14", 0UL, MEMORY_SENSOR_DDRC, 14UL,
			E_AST_MASK, E_DST_MASK, DMS_SENSOR_SCAN_INTERVAL, SCAN_FUNC),
	}
};

int32_t memory_dms_node_init(void)
{
	int32_t ret;
	uint32_t i, j;

	g_memory_devs_ptr = memory_get_devices();

	ret = devdrv_get_devnum(&g_memory_devs_ptr->dev_num);
	if (ret != 0) {
		memory_drv_err("Get dev_num failed. (ret=%d)\n", ret);
		return ret;
	}
	memory_drv_info("dev num: %u\n", g_memory_devs_ptr->dev_num);

	for (i = 0; i < memory_dev_num(g_memory_devs_ptr->dev_num); i++) {
		g_memory_devs_ptr->m_dev[i][MEMORY_SENSOR_DDRC].die_id = i;
		g_memory_devs_ptr->m_dev[i][MEMORY_SENSOR_DDRC].dev_node = &g_memory_dev_node_table[i][MEMORY_SENSOR_DDRC];
		g_memory_devs_ptr->m_dev[i][MEMORY_SENSOR_DDRC].sensor_obj_num =
			(uint32_t)(sizeof(g_ddr_sensor_table[i]) / sizeof(struct dms_sensor_object_cfg));
		g_memory_devs_ptr->m_dev[i][MEMORY_SENSOR_DDRC].sensor_obj_table = g_ddr_sensor_table[i];
		for (j = 0; j < g_memory_devs_ptr->m_dev[i][MEMORY_SENSOR_DDRC].sensor_obj_num; j++) {
			mutex_init(&g_memory_devs_ptr->m_dev[i][MEMORY_SENSOR_DDRC].sensor_event_queue[j].mutex);
			INIT_LIST_HEAD(&g_memory_devs_ptr->m_dev[i][MEMORY_SENSOR_DDRC].sensor_event_queue[j].error_list.list);
		}
	}
	return 0;
}

STATIC mem_drv g_mem_drv_table[] = {
	{memory_dms_node_init,	memory_dms_node_destroy},
	{memory_dev_node_register,	memory_dev_node_uregister},
	{memory_ipc_notifier_register,	memory_ipc_notifier_unregister},
#ifdef CFG_SOC_PLATFORM_MDC_V11
	{init_check_ddr_reg_read_result, deinit_check_ddr_reg_read_result},
	{init_memory_pm_notifier, deinit_memory_pm_notifier},
#endif
	{mata_dms_node_init, mata_dms_node_destroy},
	{mata_dev_node_register, mata_dev_node_uregister},
	{mata_fpdc_notifier_register, mata_fpdc_notifier_unregister}
};

int32_t memory_safety_ras_init(void)
{
	int32_t step, steps, rbkstep;

	steps = (int32_t)(sizeof(g_mem_drv_table) / sizeof(g_mem_drv_table[0]));
	for (step = 0; step < steps; step++) {
		int32_t ret = g_mem_drv_table[step].action();
		if (ret != 0) {
			rbkstep = step - 1;
			while (rbkstep >= 0) {
				(void)g_mem_drv_table[rbkstep].rbkaction();
				rbkstep--;
			}
			memory_drv_err("drv init faild in step: %d, ret: %d\n", step, ret);
			return ret;
		}
	}

	memory_drv_info("driver init success.\n");

	return 0;
}

void memory_safety_ras_exit(void)
{
	int32_t rbkstep, rbksteps;

	rbksteps = (int32_t)(sizeof(g_mem_drv_table) / sizeof(g_mem_drv_table[0]));
	for (rbkstep = rbksteps - 1; rbkstep >= 0; rbkstep--) {
		(void)g_mem_drv_table[rbkstep].rbkaction();
	}

	memory_drv_info("driver exit success.\n");
}