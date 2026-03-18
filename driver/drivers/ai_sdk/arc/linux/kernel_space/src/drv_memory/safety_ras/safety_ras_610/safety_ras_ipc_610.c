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
#include <linux/slab.h>
#include <linux/notifier.h>

#include "drv_ipc.h"

#include "safety_ras_ipc.h"
#include "safety_ras_init_610.h"
#include "safety_ras_report_610.h"

#ifdef AOS_LLVM_BUILD
#define MEMORY_RPROC_RX_MBX   HISI_RPROC_LP_Q_RX_RPID3
#else
#define MEMORY_RPROC_RX_MBX   HISI_RPROC_LP_Q_RX_RPID2
#endif

struct ipc_cmd_parse {
	uint8_t cmd_type;
	uint8_t cmd;
	uint8_t cmd_obj;
	uint8_t cmd_src;
	uint8_t cmd_para0[SIZE_CMDPARA];
	uint8_t cmd_para1[SIZE_CMDPARA];
	uint8_t cmd_para2[SIZE_CMDPARA];
	uint8_t cmd_para3[SIZE_CMDPARA];
	uint8_t cmd_para4[SIZE_CMDPARA];
	uint8_t cmd_para5[SIZE_CMDPARA];
	uint8_t cmd_para6[SIZE_CMDPARA];
};

int32_t memory_ipc_rx_notifier(struct notifier_block *nb, unsigned long len, void *data);
uint8_t memory_get_sensor_type_from_event_id(uint32_t event_id);
uint16_t memory_get_event_type_from_event_id(uint32_t event_id);
uint8_t memory_get_queue_index(uint8_t senseor_type, uint8_t chn);

STATIC struct memory_ipc_info g_ipc_info[MEMORY_NODE_ID_MAX] = {0};

STATIC bool memory_check_ipc_msg(const struct ipc_cmd_parse *msg)
{
	if (msg->cmd_obj != TAISHAN_CPU_ID) {
		memory_drv_debug("dest cpuId(%u) not for ddr\n", msg->cmd_obj);
		return false;
	}

	if (msg->cmd_src != LP_CPU_ID) {
		memory_drv_debug("source cpuId(%u) not for ddr\n", msg->cmd_src);
		return false;
	}

	if (msg->cmd_type != CMD_TYPE_HEALTH_STATE) {
		memory_drv_debug("ipc cmd_type. cmd type(%u) not for ddr\n", msg->cmd_type);
		return false;
	}

	if (msg->cmd != CMD_NOTIFY) {
		memory_drv_debug("ipc cmd. cmd(%u) not for ddr\n", msg->cmd);
		return false;
	}

	return true;
}

STATIC uint8_t memory_get_assertion_from_event_id(uint32_t event_id)
{
	return (uint8_t)((event_id >> EVENT_ID_ASSERTION_BIT) & 0x03U);
}

#ifndef DRV_MEM_GTEST
STATIC int32_t memory_msg_handle_event(uint32_t cmd_para0, uint32_t cmd_para1, uint32_t cmd_para2, int32_t dev_id)
{
	int32_t ret = 0;
	uint8_t que_index, chn;
	struct memory_fault_event *ras_event = NULL;
	struct memory_error_list *error_new = NULL;

	/* LP: 0   DDR: 1 */
	if (cmd_para1 == 0x0) {
		memory_drv_info("ipc msg is not mydeal.cmd_para1 : %d, just return.\n", cmd_para1);
		return 0;
	}

	error_new = kzalloc(sizeof(struct memory_error_list), GFP_KERNEL | __GFP_ACCOUNT);
	if (error_new == NULL) {
		memory_drv_err("new_event kzalloc failed.\n");
		return -ENOMEM;
	}

	chn = (uint8_t)cmd_para2 & MEMORY_MASK_8BIT;

	ras_event = &error_new->error;
	ras_event->device_id = (uint8_t)dev_id;
	ras_event->sensor_id = MEMORY_SENSOR_DDRC;
	ras_event->sensor_type = memory_get_sensor_type_from_event_id(cmd_para0);
	ras_event->assertion = memory_get_assertion_from_event_id(cmd_para0);
	ras_event->detail_reason = (uint8_t)((cmd_para2 >> MEMORY_OFFSET_8BIT) & MEMORY_MASK_8BIT);
	ras_event->event_type = memory_get_event_type_from_event_id(cmd_para0);

	memory_drv_info("sensor_type: 0x%x, assertion:%d, event_type: %d, chn: %d, reasson: %d\n",
					ras_event->sensor_type, ras_event->assertion, ras_event->event_type, chn, ras_event->detail_reason);

	que_index = memory_get_queue_index(ras_event->sensor_type, chn);
	if (ras_event->assertion == MEMORY_EVENT_RESUME) {
		ret = memory_del_fault_event(error_new, que_index);
	} else if (ras_event->assertion == MEMORY_EVENT_OCCUR) {
		ret = memory_add_fault_event(error_new, que_index);
	} else {
		ret = memory_add_fault_event(error_new, que_index);
	}
	return ret;
}
#endif

int32_t memory_msg_handle(int32_t dev_id, void *data)
{
	int32_t ret = 0;
	uint32_t cmd_para0, cmd_para1, cmd_para2;
	const struct ipc_cmd_parse *msg = NULL;

	msg = (const struct ipc_cmd_parse *)data;
	if (!memory_check_ipc_msg(msg)) {
		memory_drv_debug("ipc msg check not for ddr\n");
		return ret;
	}

	cmd_para0 = *(uint32_t *)(uintptr_t)(msg->cmd_para0);
	cmd_para1 = *(uint32_t *)(uintptr_t)(msg->cmd_para1);
	cmd_para2 = *(uint32_t *)(uintptr_t)(msg->cmd_para2);

	ret = memory_msg_handle_event(cmd_para0, cmd_para1, cmd_para2, dev_id);
	if (ret != 0)
		memory_drv_err("memory add/del ras event failed. (ret=%d)\n", ret);

	return ret;
}

STATIC int32_t ipc_unregister_process(int32_t dev_id)
{
	int32_t ret;

	g_ipc_info[dev_id].dev_id = (uint32_t)dev_id;
	g_ipc_info[dev_id].ipc_notifier.notifier_call = memory_ipc_rx_notifier;

	ret = rproc_rx_unregister(dev_id, MEMORY_RPROC_RX_MBX, &(g_ipc_info[dev_id].ipc_notifier));

	return ret;
}

STATIC void release_ipc_source(int32_t dev_num)
{
	int32_t i;

	for (i = dev_num; i >= 0; i--) {
		int32_t result;
		result = ipc_unregister_process(i);
		if (result != 0)
			memory_drv_err("unregister fail. (dev_id=%d; result=%d)\n", i, result);
	}
}

int32_t memory_ipc_notifier_unregister(void)
{
	uint32_t dev_id;
	const struct drv_memory_devices *memory_devs = memory_get_devices();

	for (dev_id = 0; dev_id < memory_dev_num(memory_devs->dev_num); dev_id++) {
		int32_t ret;
		ret = ipc_unregister_process((int32_t)dev_id);
		if (ret != 0)
			memory_drv_err("rproc_rx_unregister. (dev_id=%d; ret=%d)\n", dev_id, ret);
	}

	return 0;
}

int32_t memory_ipc_notifier_register(void)
{
	uint32_t dev_id;
	int32_t ret;
	const struct drv_memory_devices *memory_devs = memory_get_devices();

	for (dev_id = 0; dev_id < memory_dev_num(memory_devs->dev_num); dev_id++) {
		g_ipc_info[dev_id].dev_id = dev_id;
		g_ipc_info[dev_id].ipc_notifier.notifier_call = memory_ipc_rx_notifier;

		ret = rproc_rx_register((int32_t)dev_id, MEMORY_RPROC_RX_MBX, &(g_ipc_info[dev_id].ipc_notifier));
		if (ret != 0) {
			memory_drv_err("rproc_rx_register. (dev_id=%d; ret=%d)\n", dev_id, ret);
			goto out_release_source;
		}
	}
	return 0;
out_release_source:
	release_ipc_source((int32_t)dev_id - 1);
	return ret;
}
