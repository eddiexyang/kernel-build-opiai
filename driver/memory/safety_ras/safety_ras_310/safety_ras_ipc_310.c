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
#include "safety_ras_init_310.h"
#include "safety_ras_report_310.h"

#include <linux/io.h>
#include "safety_ras_converge_310.h"
#include "icm_interface.h"
#include "../icm/icm_ipc.h"

#define MEMORY_RPROC_RX_MBX   HISI_RPROC_LP_Q_RX_RPID2

#define MEMORY_EVENT_INFO_SHARED_ADDR    0x903172C0U
#define EVENT_FROM_MEM                   0x1U
#define EVENT_ASSERTION_OFFSET           0x0U
#define EVENT_REASON_OFFSET              0x4U
#define EVENT_CHANNEL_OFFSET             0x8U

int32_t memory_ipc_rx_notifier(struct notifier_block *nb, unsigned long len, void *data);
uint8_t memory_get_sensor_type_from_event_id(uint32_t event_id);
uint16_t memory_get_event_type_from_event_id(uint32_t event_id);
uint8_t memory_get_queue_index(uint8_t senseor_type, uint8_t chn);

STATIC void __iomem *g_vir_addr[MEMORY_NODE_ID_MAX] = {NULL};
STATIC struct memory_ipc_info g_ipc_info[MEMORY_NODE_ID_MAX] = {0};

#ifndef DRV_MEM_GTEST
STATIC int32_t memory_msg_handle_event(uint32_t cmd_para0, uint32_t cmd_para1, uint32_t cmd_para2, int32_t dev_id)
{
	int32_t ret = 0;
	uint8_t que_index, chn;
	struct memory_fault_event *ras_event = NULL;
	struct memory_error_list *error_new = NULL;
	uint32_t assertion, reason, channel;

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

	assertion = readl(g_vir_addr[dev_id] + EVENT_ASSERTION_OFFSET);
	reason = readl(g_vir_addr[dev_id] + EVENT_REASON_OFFSET);
	channel = readl(g_vir_addr[dev_id] + EVENT_CHANNEL_OFFSET);
	chn = (uint8_t)channel;

	ras_event = &error_new->error;
	ras_event->device_id = (uint8_t)dev_id;
	ras_event->sensor_id = MEMORY_SENSOR_DDRC;
	ras_event->sensor_type = memory_get_sensor_type_from_event_id(cmd_para0);
	ras_event->assertion = (uint8_t)assertion;
	ras_event->detail_reason = (uint8_t)reason;
	ras_event->event_type = memory_get_event_type_from_event_id(cmd_para0);

	memory_drv_info("sensor_type: 0x%x, assertion:%d, event_type: %d, chn: %d, reasson: %d\n",
					ras_event->sensor_type, ras_event->assertion, ras_event->event_type, chn, ras_event->detail_reason);

	que_index = memory_get_queue_index(ras_event->sensor_type, chn);
	if (ras_event->assertion == MEMORY_EVENT_RESUME) {
		ret = memory_del_fault_event(error_new, que_index);
	} else {
		ret = memory_add_fault_event(error_new, que_index);
	}
	return ret;
}
#endif

STATIC int32_t ddr_complete_event_id(uint32_t ori_event_id, uint32_t excep_id, uint32_t *ret_event_id)
{
	uint32_t event_id = ori_event_id;
	uint32_t node_type;

	// assertion and node_type fields contain in excep_id
	if ((excep_id & (uint32_t)0x0000ff00) == ((uint32_t)0x22 << 0x8)) { // BBOX_EXCEP_DDR_FATAL
		node_type = (uint32_t)DMS_DEV_TYPE_DDR << EVENT_ID_MODULE_BIT;
	} else {
		return -1;
	}
	event_id = ((excep_id & (uint32_t)0x30000000) == ((uint32_t)0x1 << 0x1C)) ?
		event_id : (event_id | ((uint32_t)0x1 << EVENT_ID_ASSERTION_BIT));
	event_id = event_id | node_type;
	// bit16 use the black box to reserve space for sensor type
	event_id |= (excep_id & ((uint32_t)0x10000));

	*ret_event_id = event_id;
	return 0;
}

int32_t memory_msg_handle(int32_t dev_id, void *data)
{
	int32_t ret;
	uint32_t event_id = 0;
	const struct icmdrv_ipc_msg_info *msg = (const struct icmdrv_ipc_msg_info *)data;    // 解析为IPC消息格式数据
	struct ddr_event_data *event_data = NULL;

	event_data = (struct ddr_event_data *)msg->data;    // data字段即Bbox填充字段
	ret = ddr_complete_event_id(event_data->event_id, event_data->excep_id, &event_id);
	if (ret != 0) {
		memory_drv_info("node_type is not DDR, drv_memory won't deal with this ipc message and return\r\n");
		return 0;
	}

	ret = memory_msg_handle_event(event_id, EVENT_FROM_MEM, 0, dev_id);
	if ((ret != 0) && (ret != EEXIST))
		memory_drv_err("memory add/del ras event failed. (ret=%d)\n", ret);

	return 0;
}

STATIC void release_iomap_source(void)
{
	uint32_t dev_id;
	const struct drv_memory_devices *memory_devs = memory_get_devices();

	for (dev_id = 0; dev_id < memory_dev_num(memory_devs->dev_num); dev_id++) {
		if (g_vir_addr[dev_id] != NULL) {
			iounmap(g_vir_addr[dev_id]);
			g_vir_addr[dev_id] = NULL;
		}
	}
}

STATIC int32_t ipc_unregister_process(int32_t dev_id)
{
	int32_t ret;

	g_ipc_info[dev_id].dev_id = (uint32_t)dev_id;
	g_ipc_info[dev_id].ipc_notifier.notifier_call = memory_ipc_rx_notifier;
	ret = icm_ipc_rx_unregister((uint32_t)dev_id, HISI_RPROC_RX_LP_ACPU2, IPC_OBJ_LP, &(g_ipc_info[dev_id].ipc_notifier));

	return ret;
}

STATIC void release_ipc_source(int32_t dev_num)
{
	int32_t i;

	release_iomap_source();

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

	release_iomap_source();

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
		g_vir_addr[dev_id] = ioremap(MEMORY_EVENT_INFO_SHARED_ADDR, 0x10);
		if (g_vir_addr[dev_id] == NULL) {
			memory_drv_err("ddr event info ioremap fail\n");
			ret = -ENOMEM;
			goto out_release_source;
		}

		g_ipc_info[dev_id].dev_id = dev_id;
		g_ipc_info[dev_id].ipc_notifier.notifier_call = memory_ipc_rx_notifier;

		ret = icm_ipc_rx_register(dev_id, HISI_RPROC_RX_LP_ACPU2, IPC_OBJ_LP, &(g_ipc_info[dev_id].ipc_notifier));
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
