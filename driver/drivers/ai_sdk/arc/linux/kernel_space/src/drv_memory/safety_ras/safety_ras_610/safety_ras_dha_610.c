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
 * Create: 2022-10-27
 */
#include "safety_ras_dha_610.h"

#include <linux/slab.h>
#include <linux/of.h>
#include <linux/io.h>
#include <linux/platform_device.h>

#include "drv_systime.h"
#include "safety_ras_init_610.h"
#include "dfm_dev_register.h"
#include "dms_sensor_type.h"
#include "dfm_safety_report.h"
#include "dfm_report.h"
#include "safety_ras_common_610.h"

/*
* @brief dha dms node opreations init
* @param [in] device: dms node
* @return 0: success, else: fail
*/
STATIC int ddra_dha_ops_init(struct dms_node *device)
{
	memory_drv_debug("DEBUG (node_name=%s).\n", device->node_name);
	return 0;
}

/*
* @brief dha dms node opreations uinit
* @param [in] device: dms node
* @return 0: success, else: fail
*/
STATIC void ddra_dha_ops_uninit(struct dms_node *device)
{
	memory_drv_debug("DEBUG (node_name=%s).\n", device->node_name);
	return;
}

STATIC struct dms_node_operations g_ddra_dha_ops = {
	.init = ddra_dha_ops_init,
	.uninit = ddra_dha_ops_uninit,
	.scan = NULL,
	.fault_diag = NULL,
	.event_notify = NULL,
	.get_link_state = NULL,
	.set_link_state = NULL,
};

STATIC struct dms_node g_ddra_dha_dms_nodes[DDRA_NODE_ID_MAX] = {
	ddra_dha_dms_node_define(0, &g_ddra_dha_ops)
};

struct dfm_struct ddra_dha_dms;

STATIC struct safety_irq_hw_info g_safety_ddra_dha_hwinfo[2][3] = {
	{
		/* 右通道safety_irq_hw_info定义 */
		ddra_dha_interrupt_err_hwinfo(DHA_RIGHT_CH_PADDR, 26U, DHA_SAFETY_ERR_STATUS_REG_OFFSET,\
			DHA_SAFETY_ERR_CLEAR_REG_OFFSET, DHA_SAFETY_ERR_MASK_REG_OFFSET),
		ddra_dha_interrupt_int_hwinfo(DHA_RIGHT_CH_PADDR, 10U, DHA_SAFETY_STATUS_REG_OFFSET,\
			DHA_SAFETY_CLEAR_REG_OFFSET, DHA_SAFETY_MASK_REG_OFFSET),
		ddra_dha_interrupt_ras_safety_hwinfo(DHA_RIGHT_CH_PADDR, DHA_RAS_SAFETY_STATUS_REG_OFFSET,\
			DHA_RAS_SAFETY_CLEAR_REG_OFFSET, DHA_RAS_SAFETY_MASK_REG_OFFSET)
	},
	{
		/* 左通道safety_irq_hw_info定义 */
		ddra_dha_interrupt_err_hwinfo(DHA_LEFT_CH_PADDR, 27U, DHA_SAFETY_ERR_STATUS_REG_OFFSET,\
			DHA_SAFETY_ERR_CLEAR_REG_OFFSET, DHA_SAFETY_ERR_MASK_REG_OFFSET),
		ddra_dha_interrupt_int_hwinfo(DHA_LEFT_CH_PADDR, 11U, DHA_SAFETY_STATUS_REG_OFFSET,\
			DHA_SAFETY_CLEAR_REG_OFFSET, DHA_SAFETY_MASK_REG_OFFSET),
		ddra_dha_interrupt_ras_safety_hwinfo(DHA_LEFT_CH_PADDR, DHA_RAS_SAFETY_STATUS_REG_OFFSET,\
			DHA_RAS_SAFETY_CLEAR_REG_OFFSET, DHA_RAS_SAFETY_MASK_REG_OFFSET)
	}
};

STATIC struct safety_irq_info g_safety_ddra_dha_info[2] = {
	ddra_dha_interrupt_info("int_ras_safety_dha_right", 0),
	ddra_dha_interrupt_info("int_ras_safety_dha_left", 1U)
};

STATIC struct ras_irq_hw_info g_ras_eri_ddra_dha_hwinfo[RAS_DDRA_DHA_INT_NUM] = {
	ras_misc1h_ddra_dha_hw_info(DHA_LEFT_CH_PADDR),
	ras_misc1h_ddra_dha_hw_info(DHA_RIGHT_CH_PADDR)
};

STATIC struct ras_irq_hw_info g_ras_fhi_ddra_dha_hwinfo[RAS_DDRA_DHA_INT_NUM] = {
	ras_misc1h_ddra_dha_hw_info(DHA_LEFT_CH_PADDR),
	ras_misc1h_ddra_dha_hw_info(DHA_RIGHT_CH_PADDR)
};

STATIC struct ras_irq_info g_ras_fhi_ddra_dha_info[RAS_DDRA_DHA_INT_NUM] = {
	ras_fhi_dha_interrupt_info("int_fhi_dha_left", 0),
	ras_fhi_dha_interrupt_info("int_fhi_dha_right", 1U)
};

STATIC struct ras_irq_info g_ras_eri_ddra_dha_info[RAS_DDRA_DHA_INT_NUM] = {
	ras_eri_dha_interrupt_info("int_eri_dha_left", 0),
	ras_eri_dha_interrupt_info("int_eri_dha_right", 1U)
};

/**
 * @brief ddra_scan_dha_event: node id&event check
 */
STATIC int ddra_scan_dha_event(unsigned long long private_data, struct dms_sensor_event_data *data)
{
	uint32_t node_id;
	uint32_t sensor_type;

	node_id = (uint32_t)((private_data >> DEV_NODE_OFFSET) & (DEV_NODE_MASK));
	sensor_type = (uint32_t)((private_data >> SENSOR_NODE_OFFSET) & (SENSOR_NODE_MASK));
	/* check node_id */
	if (node_id >= ddra_dha_dms.node_num) {
		memory_drv_err("invalid node id (node_id=%d)!\n", node_id);
		return -EINVAL;
	}
	/* dfm scan events, check the params */
	if (dfm_scan_events(&ddra_dha_dms, node_id, (unsigned char)sensor_type, data) != 0) {
		memory_drv_err("dfm scan events failed!\n");
		return -EFAULT;
	}

	return 0;
}

/**
 * @brief ddra_dha_register_dms_node: dms_node注册
 */
STATIC int ddra_dha_register_dms_node(void)
{
	uint32_t i, sensor_idx;

	/* sensor定义 */
	STATIC struct dms_sensor_object_cfg sensor_cfg[] = {
		ddra_dha_sensor_obj(DMS_SEN_TYPE_RAS_SENSOR, "dha-0-0", DMS_DISCRETE_SENSOR_CLASS,
			DMS_SENSOR_ATTRIB_THRES_NONE, 0, DMS_SENSOR_CHECK_INTERVAL_TIME, DMS_SENSOR_PROC_ENABLE_FLAG,
			DMS_SENSOR_ENABLE_FALG, ddra_scan_dha_event, sensor_priv_data(0U, (uint32_t)DMS_SEN_TYPE_RAS_SENSOR),
			0XFFFF, 0XFFBF), /* ras */
		ddra_dha_sensor_obj(DMS_SEN_TYPE_SAFETY_SENSOR, "dha-0-1", DMS_DISCRETE_SENSOR_CLASS,
			DMS_SENSOR_ATTRIB_THRES_NONE, 0, DMS_SENSOR_CHECK_INTERVAL_TIME, DMS_SENSOR_PROC_ENABLE_FLAG,
			DMS_SENSOR_ENABLE_FALG, ddra_scan_dha_event, sensor_priv_data(0U, (uint32_t)DMS_SEN_TYPE_SAFETY_SENSOR),
			0XFFFF, 0XFFFF) /* safety */
	};

	uint32_t sensor_num = (uint32_t)sizeof(sensor_cfg) / (uint32_t)sizeof(struct dms_sensor_object_cfg);
	ddra_dha_dms.dev_id = 0;
	/* init ddra_dha_dms */
	if (dfm_struct_init(&ddra_dha_dms, DDRA_NODE_ID_MAX, sensor_num) != 0) {
		memory_drv_err("init ddra dha's dms data failed!\n");
		goto _fail;
	}

	memory_drv_debug("init ddra dha's dms data success!\n");
	/* check dev_nodes */
	if (ddra_dha_dms.dev_nodes == NULL) {
		memory_drv_err("ddra_dha_dms dev_nodes is null pointer!\n");
		goto _fail;
	}

	/* dev_node定义 */
	for (i = 0; i < (uint32_t)DDRA_NODE_ID_MAX; i++) {
		ddra_dha_dms.dev_nodes[i].node = &g_ddra_dha_dms_nodes[i];
		ddra_dha_dms.dev_nodes[i].post_proc = NULL;
		ddra_dha_dms.dev_nodes[i].fpdc_notify = NULL;
		ddra_dha_dms.dev_nodes[i].get_converage_node = ddra_dha_safety_coverage;
		for (sensor_idx = 0; sensor_idx < sensor_num; sensor_idx++)
			ddra_dha_dms.dev_nodes[i].sensors[sensor_idx].dms_sensor = sensor_cfg[sensor_idx];
	}
	/* register to dfm */
	if (dfm_register_nodes(&ddra_dha_dms) != 0) {
		memory_drv_err("register ddra dha dms node failed!\n");
		goto _fail;
	}

	memory_drv_debug("register ddra dha dms node success!\n");

	return 0;

_fail:
	dfm_struct_final(&ddra_dha_dms, DDRA_NODE_ID_MAX, 0);
	return -EFAULT;
}

/**
 * @brief ddra_dha_register_safety_irq: safety_irq注册
 */
int32_t ddra_dha_register_safety_irq(void)
{
	int32_t ret = 0;

	if (safety_ras_supported() != SAFETY_RAS_SUPPORTED) {
		memory_drv_info("don not support safety ras!\n");
		return ret;
	}

	/* register ddra dha device node */
	if (ddra_dha_register_dms_node() != 0) {
		memory_drv_err("register ddra dha device node failed!\n");
		return -EFAULT;
	}

	memory_drv_debug("register ddra dha device node success!\n");
	/* register safety irq */
	if (dfm_register_safety_irq_hwinfo(0, g_safety_ddra_dha_info, 2U) != 0) {
		memory_drv_err("register safety irq info failed!\n");
		ret = -EFAULT;
		goto out_release_dms_node;
	}

	memory_drv_debug("register safety irq info success!\n");
	/* register eri ras irq info */
	if (dfm_register_ras_irq_hwinfo(0, g_ras_eri_ddra_dha_info, 2U) != 0) {
		memory_drv_err("register eri ras irq info failed!\n");
		ret = -EFAULT;
		goto out_unregister_safety_irq;
	}

	memory_drv_debug("register eri ras irq info success!\n");
	/* register fhi ras irq info */
	if (dfm_register_ras_irq_hwinfo(0, g_ras_fhi_ddra_dha_info, 2U) != 0) {
		memory_drv_err("register fhi ras irq info failed!\n");
		ret = -EFAULT;
		goto out_unregister_ras_eri_irq;
	}

	if (platform_driver_register(&g_dha_ras_driver) != 0) {
		memory_drv_err("register dha ras driver failed!\n");
		ret = -EFAULT;
		goto out_unregister_dha_ras_dirver;
	}
	memory_drv_debug("register fhi ras irq info success!\n");
	return 0;

out_unregister_dha_ras_dirver:
	platform_driver_unregister(&g_dha_ras_driver);
out_unregister_ras_eri_irq:
	dfm_unregister_ras_irq_hwinfo(0, g_ras_eri_ddra_dha_info, 2U); /* unregister ras */
out_unregister_safety_irq:
	dfm_unregister_safety_irq_hwinfo(0, g_safety_ddra_dha_info, 2U); /* unregister safety */
out_release_dms_node:
	dfm_unregister_nodes(&ddra_dha_dms);
	dfm_struct_final(&ddra_dha_dms, DDRA_NODE_ID_MAX, 0);
	return ret;
}

/**
 * @brief ddra_dha_unregister_safety_irq: safety_irq取消注册
 */
int32_t ddra_dha_unregister_safety_irq(void)
{
	if (safety_ras_supported() != SAFETY_RAS_SUPPORTED) {
		memory_drv_info("don not support safety ras!\n");
		return 0;
	}

	platform_driver_unregister(&g_dha_ras_driver);
	dfm_unregister_safety_irq_hwinfo(0, g_safety_ddra_dha_info, 2U); /* unregister safety irq info */
	dfm_unregister_ras_irq_hwinfo(0, g_ras_eri_ddra_dha_info, 2U); /* unregister eri ras irq info */
	dfm_unregister_ras_irq_hwinfo(0, g_ras_fhi_ddra_dha_info, 2U); /* unregister fhi ras irq info */
	dfm_unregister_nodes(&ddra_dha_dms);
	dfm_struct_final(&ddra_dha_dms, DDRA_NODE_ID_MAX, 0);
	return 0;
}
