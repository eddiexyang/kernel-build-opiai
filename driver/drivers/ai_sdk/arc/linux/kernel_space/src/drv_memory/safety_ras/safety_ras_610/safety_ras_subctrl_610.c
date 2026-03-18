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

#include "safety_ras_subctrl_610.h"

#include <linux/io.h>
#include <linux/slab.h>
#include <linux/of.h>

#include "safety_ras_init_610.h"

#include "dfm_dev_register.h"
#include "dms_sensor_type.h"
#include "dfm_safety_report.h"
#include "dfm_report.h"
#include "drv_systime.h"
#include "safety_ras_common_610.h"

STATIC int ddr_subctrl_ops_init(struct dms_node *device)
{
	memory_drv_debug("ddr_subctrl_ops_init (node_name=%s).\n", device->node_name);
	return 0;
}

STATIC void ddr_subctrl_ops_uninit(struct dms_node *device)
{
	memory_drv_debug("ddr_subctrl_ops_uninit (node_name=%s).\n", device->node_name);
	return;
}

STATIC struct dms_node_operations g_ddr_subctrl_ops = {
	.init = ddr_subctrl_ops_init,
	.uninit = ddr_subctrl_ops_uninit,
	.scan = NULL,
	.fault_diag = NULL,
	.event_notify = NULL,
	.get_link_state = NULL,
	.set_link_state = NULL,
};

STATIC struct dfm_struct g_ddr_subctrl_dms;

STATIC struct dms_node g_ddr_subctrl_dms_nodes[DDR_SUBCTRL_NODE_MAX] = {
	ddr_subctrl_dms_node_define(0, &g_ddr_subctrl_ops)
};

STATIC int32_t ddr_subctrl_safety_irq_clear(unsigned long long base_addr)
{
	void __iomem *vir_addr;
	u32 reg_val;

	vir_addr = ioremap(base_addr, DDR_SUBCTRL_BASE_ADDR_LEN);
	if (vir_addr == NULL) {
		memory_drv_err("ddr subctrl ioremap fail\n");
		return -ENOMEM;
	}

	reg_val = readl(vir_addr + DDR_SUBCTRL_CLEAR_REG_OFFSET);
	reg_val |= 0x1U;
	writel(reg_val, vir_addr + DDR_SUBCTRL_CLEAR_REG_OFFSET);

	iounmap(vir_addr);

	memory_drv_debug("ddr_subctrl_safety_irq_clear success!\n");

	return 0;
}

STATIC int32_t ddr_subctrl_safety_irq_preprocess(
	struct safety_fault_info *safety_fault, unsigned int *event_num, struct safety_event **event_list)
{
	if (safety_fault == NULL) {
		memory_drv_err("safety_fault is null ptr!\n");
		return -EINVAL;
	}

	if (event_num == NULL) {
		memory_drv_err("event_num is null ptr!\n");
		return -EINVAL;
	}

	if (event_list == NULL) {
		memory_drv_err("event_list is null ptr!\n");
		return -EINVAL;
	}

	*event_num = 0;

	*event_list = (struct safety_event *)kmalloc(sizeof(struct safety_event) * DDR_SUBCTRL_EVENT_MAX_NUM, GFP_KERNEL);
	if ((*event_list) == NULL) {
		memory_drv_err("ddr subctrl kmalloc safety event list failed!\n");
		return -ENOMEM;
	}

	return 0;
}

STATIC int32_t ddr_subctrl_safety_irq_handler(
	struct safety_fault_info *safety_fault, unsigned int *event_num, struct safety_event **event_list)
{
	uint64_t end_time;
	int32_t ret = 0;
	uint32_t i;
	struct dfm_safety_module module_info = {0};

	memory_drv_debug("ddr_subctrl_safety_irq occurs!\n");

	ret = ddr_subctrl_safety_irq_preprocess(safety_fault, event_num, event_list);
	if (ret != 0) {
		return ret;
	}

	module_info.dev_id = (uint8_t)safety_fault->dev_id;
	module_info.node_type = DMS_DEV_TYPE_DDR;
	module_info.max_event = DDR_SUBCTRL_EVENT_MAX_NUM;

	for (i = 0; i < safety_fault->fault_reg_num; i++) {
		module_info.node_id = 0;
		module_info.dev_node = &g_ddr_subctrl_dms.dev_nodes[0];
		module_info.section_type = 0;

		/* 故障上报 */
		ret = dfm_safety_handler(&module_info, &safety_fault->fault_status_list[i], event_num, *event_list);
		if (ret != 0) {
			memory_drv_err("ddr subctrl dfm_safety_handler failed. (ret = %d)\n", ret);
		}

		ret = ddr_subctrl_safety_irq_clear(safety_fault->fault_status_list[i].base_paddr);
		if (ret != 0) {
			memory_drv_err("ddr_subctrl_safety_irq_clear failed. (ret = %d)\n", ret);
		}
	}
	end_time = get_syscnt();
	memory_drv_info("SysCnt: subctrl end(%llu)\n", end_time);

	return ret;
}

STATIC struct safety_irq_hw_info g_left_subctrl1_safety_hwinfo = {
	.emu_id = PERI_SUB_EMU_ID,
	.src_id = 2,			  /* 1 + 43 / 32 */
	.bit_id = 11,			  /* 43 % 32 */
	.base_paddr = DDR_SUBCTRL_LEFT_CH_PADDR, /* interrupt reg base addr */
	.base_size = DDR_SUBCTRL_BASE_ADDR_LEN,	 /* interrupt reg base size */
	.status = {DDR_SUBCTRL_STATUS_REG_OFFSET, 0x1U},  /* interrupt status reg */
	.clear = {DDR_SUBCTRL_CLEAR_REG_OFFSET, 0x1U},   /* interrupt clear reg */
	.mask = {DDR_SUBCTRL_ERR_MASK_REG_OFFSET, 0x1U},	/* interrupt mask reg */
	.check_mask[0] = {DRVFAULT_FILED_VALID, DDR_SUBCTRL_FUN_INT_MASK_REG_OFFSET, 0xFFFFFFFEU},
};

STATIC struct safety_irq_info g_left_subctrl1_safety_irq_info = {
	.dev_id = 0,
	.irq_type = SAFETY_IRQ_TYPE_LPI,
	.irq_name = "int_ras_safety_lpddr_left_subctrl1",
	.safety_irq_func = ddr_subctrl_safety_irq_handler,
	.irq_hwinfo_num = 1,
	.irq_hwinfo_list = &g_left_subctrl1_safety_hwinfo,
};

STATIC struct safety_irq_hw_info g_right_subctrl0_safety_hwinfo = {
	.emu_id = HAC_SUB_EMU_ID,
	.src_id = 1,			  /* 1 + 31 / 32 */
	.bit_id = 31,			 /* 31 % 32 */
	.base_paddr = DDR_SUBCTRL_RIGHT_CH_PADDR, /* interrupt reg base addr */
	.base_size = DDR_SUBCTRL_BASE_ADDR_LEN,	 /* interrupt reg base size */
	.status = {DDR_SUBCTRL_STATUS_REG_OFFSET, 0x1U},  /* interrupt status reg */
	.clear = {DDR_SUBCTRL_CLEAR_REG_OFFSET, 0x1U},   /* interrupt clear reg */
	.mask = {DDR_SUBCTRL_ERR_MASK_REG_OFFSET, 0x1U},	/* interrupt mask reg */
	.check_mask[0] = {DRVFAULT_FILED_VALID, DDR_SUBCTRL_FUN_INT_MASK_REG_OFFSET, 0xFFFFFFFEU},
};

STATIC struct safety_irq_info g_right_subctrl0_safety_irq_info = {
	.dev_id = 0,
	.irq_type = SAFETY_IRQ_TYPE_LPI,
	.irq_name = "int_ras_safety_lpddr_right_subctrl0",
	.safety_irq_func = ddr_subctrl_safety_irq_handler,
	.irq_hwinfo_num = 1,
	.irq_hwinfo_list = &g_right_subctrl0_safety_hwinfo,
};

STATIC int ddr_subctrl_scan_event(unsigned long long private_data, struct dms_sensor_event_data *data)
{
	uint32_t node_id;
	uint8_t sensor_type;

	node_id = (uint32_t)((private_data >> DDR_SUBCTRL_NODE_OFFSET) & (DDR_SUBCTRL_NODE_MASK));
	sensor_type = (uint8_t)((private_data >> DDR_SUBCTRL_SENSOR_NODE_OFFSET) & (DDR_SUBCTRL_SENSOR_NODE_MASK));

	if (node_id >= g_ddr_subctrl_dms.node_num) {
		memory_drv_err("ddr subctrl invalid node id (node_id=%d)!\n", node_id);
		return -EINVAL;
	}

	if (dfm_scan_events(&g_ddr_subctrl_dms, node_id, sensor_type, data) != 0) {
		memory_drv_err("ddr subctrl dfm scan events failed!\n");
		return -EFAULT;
	}

	return 0;
}

STATIC const struct ras_fault_converge_item *ddr_subctrl_safety_item(const struct ras_fault_converge_item *items,
	uint32_t item_num, unsigned long long ras_code)
{
	uint32_t i;

	for (i = 0; i < item_num; i++) {
		if (ras_code == items[i].ras_code.err_status) {
			return &items[i];
		}
	}

	return NULL;
}

STATIC const struct ras_fault_converge_item *ddr_subctrl_safety_coverage(uint32_t section_type,
	unsigned long long ras_code)
{
	/* 故障bit位定义 */
	STATIC struct ras_fault_converge_item ddr_subctrl_coverage_tab[] = {
		ddr_subctrl_safety_coverage_item(DDR_SUBCTRL_PARITY_ERROR, "ddr_subctrl_parity_err", DMS_SEN_TYPE_RAS_SENSOR,
			SOC_SAFETY_REG_VAL_UNEXPECT)
	};

	uint32_t items_num = (uint32_t)(sizeof(ddr_subctrl_coverage_tab) / sizeof(ddr_subctrl_coverage_tab[0]));

	return ddr_subctrl_safety_item(ddr_subctrl_coverage_tab, items_num, ras_code);
}

STATIC int ddr_subctrl_register_dms_node(void)
{
	uint8_t i;
	uint32_t sensor_idx;

	/* sensor定义 */
	STATIC struct dms_sensor_object_cfg sensor_cfg[] = {
		ddr_subctrl_sensor_obj(DMS_SEN_TYPE_RAS_SENSOR, "ddr-subctrl", DMS_DISCRETE_SENSOR_CLASS,
			DMS_SENSOR_ATTRIB_THRES_NONE, 0, DMS_SENSOR_CHECK_INTERVAL_TIME, DMS_SENSOR_PROC_ENABLE_FLAG,
			DMS_SENSOR_ENABLE_FALG, ddr_subctrl_scan_event, ddr_subctrl_sensor_priv_data(0, DMS_SEN_TYPE_RAS_SENSOR),
			0XFFFF, 0XFFFF)
	};

	uint32_t sensor_num = (uint32_t)(sizeof(sensor_cfg) / sizeof(struct dms_sensor_object_cfg));
	g_ddr_subctrl_dms.dev_id = 0;

	if (dfm_struct_init(&g_ddr_subctrl_dms, DDR_SUBCTRL_NODE_MAX, sensor_num) != 0) {
		memory_drv_err("ddr subctrl dfm_struct_init failed!\n");
		goto _fail;
	}

	if (g_ddr_subctrl_dms.dev_nodes == NULL) {
		memory_drv_err("g_ddr_subctrl_dms dev_nodes is null pointer!\n");
		goto _fail;
	}

	/* dev_node定义 */
	for (i = 0; i < (uint8_t)DDR_SUBCTRL_NODE_MAX; i++) {
		g_ddr_subctrl_dms.dev_nodes[i].node = &g_ddr_subctrl_dms_nodes[i];
		g_ddr_subctrl_dms.dev_nodes[i].post_proc = NULL;
		g_ddr_subctrl_dms.dev_nodes[i].fpdc_notify = NULL;
		g_ddr_subctrl_dms.dev_nodes[i].get_converage_node = ddr_subctrl_safety_coverage;
		for (sensor_idx = 0; sensor_idx < sensor_num; sensor_idx++) {
			g_ddr_subctrl_dms.dev_nodes[i].sensors[sensor_idx].dms_sensor = sensor_cfg[sensor_idx];
		}
	}

	if (dfm_register_nodes(&g_ddr_subctrl_dms) != 0) {
		memory_drv_err("ddr subctrl dfm_register_nodes failed!\n");
		goto _fail;
	}

	memory_drv_debug("ddr subctrl nodes register success!\n");

	return 0;

_fail:
	dfm_struct_final(&g_ddr_subctrl_dms, DDR_SUBCTRL_NODE_MAX, 0);
	return -EFAULT;
}

int32_t ddr_subctrl_safety_irq_init(void)
{
	int32_t ret = 0;

	if (safety_ras_supported() != SAFETY_RAS_SUPPORTED) {
		memory_drv_info("don not support safety ras!\n");
		return ret;
	}

	if (ddr_subctrl_register_dms_node() != 0) {
		memory_drv_err("register ddr subctrl dms node failed!\n");
		return -EFAULT;
	}

	ret = drvfault_register_safety_irq_info(&g_right_subctrl0_safety_irq_info);
	if (ret != 0) {
		memory_drv_err("right_subctrl0 drvfault_register_safety_irq_info return error : %d!\n", ret);
		goto out_release_dms_node;
	}

	ret = drvfault_register_safety_irq_info(&g_left_subctrl1_safety_irq_info);
	if (ret != 0) {
		memory_drv_err("left_subctrl1 drvfault_register_safety_irq_info return error : %d!\n", ret);
		goto out_unregister_safety_irq;
	}

	return ret;
out_unregister_safety_irq:
	(void)drvfault_unregister_safety_irq_info(0, g_right_subctrl0_safety_irq_info.irq_name);
out_release_dms_node:
	dfm_unregister_nodes(&g_ddr_subctrl_dms);
	dfm_struct_final(&g_ddr_subctrl_dms, DDR_SUBCTRL_NODE_MAX, 0);
	return ret;
}

int32_t ddr_subctrl_safety_irq_destroy(void)
{
	int32_t ret = 0;

	if (safety_ras_supported() != SAFETY_RAS_SUPPORTED) {
		memory_drv_info("don not support safety ras!\n");
		return ret;
	}

	ret = drvfault_unregister_safety_irq_info(0, g_right_subctrl0_safety_irq_info.irq_name);
	if (ret != 0) {
		memory_drv_err("right_subctrl0 drvfault_unregister_safety_irq_info return error : %d!\n", ret);
	}

	ret = drvfault_unregister_safety_irq_info(0, g_left_subctrl1_safety_irq_info.irq_name);
	if (ret != 0) {
		memory_drv_err("left_subctrl1 drvfault_unregister_safety_irq_info return error : %d!\n", ret);
	}

	dfm_unregister_nodes(&g_ddr_subctrl_dms);
	dfm_struct_final(&g_ddr_subctrl_dms, DDR_SUBCTRL_NODE_MAX, 0);

	return ret;
}