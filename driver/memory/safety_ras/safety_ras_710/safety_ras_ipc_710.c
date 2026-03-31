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
#include <linux/cper.h>
#include "drv_ipc.h"
#include "memory_log.h"
#include "safety_ras_ipc.h"
#include "safety_ras_init_710.h"
#include "safety_ras_isolation_710.h"

#define DDR_MULTI_ECC_FAULT		0xA8193248U
#define MEMORY_RPROC_RX_MBX		HISI_RPROC_LP_Q_RX_RPID2

#define mem_check_act_dbg_log(expr, action, fmt, ...)			\
	if (expr) {													\
		memory_drv_debug(fmt, ##__VA_ARGS__);					\
		action;													\
	}

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

STATIC struct memory_ipc_info g_ipc_info[MEMORY_NODE_ID_MAX] = {0};

STATIC bool memory_check_ipc_msg(const struct ipc_cmd_parse *msg)
{
	// 1. check dst
	mem_check_act_dbg_log(msg->cmd_obj != TAISHAN_CPU_ID,
		return false, "dest cpuId(%u) not for ddr\n", msg->cmd_obj);

	// 2. check src
	mem_check_act_dbg_log(msg->cmd_src != LP_CPU_ID,
		return false, "source cpuId(%u) not for ddr\n", msg->cmd_src);

	// 3. check cmd type
	mem_check_act_dbg_log(msg->cmd_type != 0x7U,
		return false, "ipc cmd_type. cmd type(%u) not for ddr\n", msg->cmd_type);

	// 4. check cmd
	mem_check_act_dbg_log(msg->cmd != CMD_NOTIFY,
		return false, "ipc cmd. cmd(%u) not for ddr\n", msg->cmd);

	return true;
}

#ifndef DRV_MEM_GTEST
int32_t memory_msg_handle(int32_t dev_id, void *data)
{
	int32_t ret = 0;
	uint32_t exception_id = 0;
	const struct ipc_cmd_parse *msg = NULL;

	msg = (const struct ipc_cmd_parse *)data;

	// 1. sanity check
	if (!memory_check_ipc_msg(msg)) {
		memory_drv_debug("ipc msg check not for ddr\n");
		return ret;
	}

	exception_id = *(uint32_t *)(uintptr_t)(msg->cmd_para0);
	mem_check_act_dbg_log(exception_id != DDR_MULTI_ECC_FAULT,
		return ret, "exception id(0x%x) not for ddr uecc\n", exception_id);
	memory_drv_info("recve exception id(0x%x)\n", exception_id);

	// 2. uecc error post handle
	memory_failure_handle(dev_id);

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
#endif
