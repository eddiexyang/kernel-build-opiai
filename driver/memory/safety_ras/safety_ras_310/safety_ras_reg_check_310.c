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
 * Create: 2023-08-31
 */
#include "dms_dev_node.h"

#include "safety_ras_ipc.h"
#include "safety_ras_init_310.h"
#include "safety_ras_config_310.h"
#include "safety_ras_node.h"

#include <linux/notifier.h>
#include <linux/suspend.h>
#include <linux/delay.h>
#include "safety_ras_reg_check_310.h"
#ifdef AOS_LLVM_BUILD
#include <aos_dlib.h>
#endif

#define DDR_READ_CHECK_DEFAULT_VALUE    0xFFFFFFFFU

STATIC void __iomem *g_ddr_reg_read_check_vir_addr[MEMORY_NODE_ID_MAX] = {NULL};
STATIC struct notifier_block g_memory_sr_nb;

STATIC void release_iomap_space(uint32_t dev_num)
{
	uint32_t dev_id;

	for (dev_id = 0; dev_id < memory_dev_num(dev_num); dev_id++) {
		if (g_ddr_reg_read_check_vir_addr[dev_id] != NULL) {
			iounmap(g_ddr_reg_read_check_vir_addr[dev_id]);
			g_ddr_reg_read_check_vir_addr[dev_id] = NULL;
		}
	}
}

STATIC int32_t init_iomap_space(uint32_t dev_num)
{
	uint32_t dev_id;

	for (dev_id = 0; dev_id < memory_dev_num(dev_num); dev_id++) {
		g_ddr_reg_read_check_vir_addr[dev_id] = ioremap(REG_READ_CHECK_SHAREMEM_BASE_ADDR, IO_REMAP_SIZE);
		if (g_ddr_reg_read_check_vir_addr[dev_id] == NULL) {
			memory_drv_err("ddr info ioremap fail\n");
			release_iomap_space(dev_num);
			return -1;
		}
	}

	return 0;
}

STATIC int32_t report_event_to_dms(uint32_t dev_id)
{
	uint8_t que_index;
	struct memory_fault_event *ras_event = NULL;
	struct memory_error_list *error_new = NULL;
#ifndef DRV_MEM_GTEST
	error_new = kzalloc(sizeof(struct memory_error_list), GFP_KERNEL | __GFP_ACCOUNT);
	if (error_new == NULL) {
		memory_drv_err("new_event kzalloc failed.\n");
		return -ENOMEM;
	}

	ras_event = &error_new->error;
	ras_event->device_id = (uint8_t)dev_id;
	ras_event->sensor_id = MEMORY_SENSOR_DDRC;
	ras_event->sensor_type = DMS_SEN_TYPE_EXTEND_SENSOR;
	ras_event->assertion = MEMORY_EVENT_OCCUR;
	ras_event->detail_reason = DDR_REG_READ_CHECK_FAIL_REASON;
	ras_event->event_type = DDR_REG_READ_CHECK_EVENT_TYPE;

	memory_drv_info("sensor_type: 0x%x, event_type: %d\n", ras_event->sensor_type, ras_event->event_type);
#endif
	que_index = memory_get_queue_index(DMS_SEN_TYPE_EXTEND_SENSOR, 0U);
	return memory_add_fault_event(error_new, que_index);
}

STATIC void check_return_value(int32_t ret)
{
	if (ret != 0) {
		if (ret == EEXIST) {
			memory_drv_info("Fault event already exist.\n");
		} else {
			memory_drv_err("fail to report event.\n");
		}
	} else {
		memory_drv_info("report event OK\n");
	}
}

STATIC void ddr_reg_read_check(uint32_t dev_num)
{
	int32_t ret;
	uint32_t dev_id;
	uint32_t result;

	for (dev_id = 0; dev_id < memory_dev_num(dev_num); dev_id++) {
		result = readl(g_ddr_reg_read_check_vir_addr[dev_id] + DDR_REG_REAG_CHECK_RESULT_OFFSET);
		if ((result != 0U) && (result != DDR_READ_CHECK_DEFAULT_VALUE)) {
			memory_drv_event("DDR key reg read check fail, check fail chan mask: 0x%x\n", result);
			ret = report_event_to_dms(dev_id);	// 校验不通过时通知故障管理框架
			check_return_value(ret);
		} else {
			memory_drv_info("DDR key reg read check OK\n");
		}
		writel(0x0U, g_ddr_reg_read_check_vir_addr[dev_id] + DDR_REG_REAG_CHECK_RESULT_OFFSET);    // 此次检查回读校验结果后清除
	}
}

STATIC int32_t memory_sr_notofier(struct notifier_block *nb, unsigned long action, void *data)
{
	int32_t ret;
	(void)nb;
	(void)data;

	switch (action) {
	case PM_POST_HIBERNATION:
	case PM_POST_SUSPEND:
		ddr_reg_read_check(g_memory_devs_ptr->dev_num);
		ret = NOTIFY_OK;
		break;
	default:
		ret = NOTIFY_DONE;
		break;
	}

	return ret;
}

int32_t init_memory_pm_notifier(void)
{
	int32_t ret;

	// 注册睡眠唤醒时回调接口，唤醒后对DDR寄存器进行回读校验
	g_memory_sr_nb.notifier_call = memory_sr_notofier;
	g_memory_sr_nb.priority = -1;
	ret = register_pm_notifier(&g_memory_sr_nb);
	if (ret != 0) {
		memory_drv_err("register memory sr notifier fail, ret: %d\n", ret);
		return ret;
	}

	memory_drv_info("register memory sr notifier OK\r\n");
	return 0;
}

int32_t deinit_memory_pm_notifier(void)
{
	(void)unregister_pm_notifier(&g_memory_sr_nb);

	return 0;
}

int32_t init_check_ddr_reg_read_result(void)
{
	int32_t ret;

	// 检查初始化后DDR关键寄存器回读校验结果
	ret = init_iomap_space(g_memory_devs_ptr->dev_num);
	if (ret != 0) {
		memory_drv_err("init_iomap_space failed. (ret=%d)\n", ret);
		return ret;
	}
	ddr_reg_read_check(g_memory_devs_ptr->dev_num);

	return 0;
}

int32_t deinit_check_ddr_reg_read_result(void)
{
	// 释放申请的ioremap资源，该资源用于DDR寄存器回读校验结果
	release_iomap_space(g_memory_devs_ptr->dev_num);

	return 0;
}
