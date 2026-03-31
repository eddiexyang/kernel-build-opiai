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
*/
#include <linux/notifier.h>
#include "icm_interface.h"
#include "dms_node_type.h"

#include "lpm_fault_common.h"
#include "lpm_init.h"
#include "lpm_fault_report.h"
#include "lpm_fault_heartbeat.h"
#include "lpm_fault_ipc.h"
#include "lpm_fault_from_ipc.h"

#ifdef LPM_IPC_V2
STATIC bool lpm_is_ipc_msg_check_ok(const struct lpm_fault_ipc_send_msg *msg)
{
	uint32_t from_lpm = 0;

	if (msg->cmd_dest != TAISHAN_CPU_ID_FAULT) {
		lpm_log_info("invalid dest cpuId=%u\n", msg->cmd_dest);
		return false;
	}

	if (msg->cmd_src != LP_CPU_ID_FAULT) {
		lpm_log_info("invalid source cpuId=%u\n", msg->cmd_src);
		return false;
	}
#ifdef CFG_SOC_PLATFORM_MDC_V51
	if (msg->cmd != LPM_FAULT_CMD_NOTIFY) {
		// not fault report ipc, ignore
		return false;
	}

	if (msg->sub_cmd != LPM_FAULT_SUB_CMD_HEALTH_STATE) {
		// not fault report ipc, ignore
		return false;
	}
#endif
	from_lpm = *(uint32_t *)(uintptr_t)(&msg->data[0x4]);
	if (from_lpm > 0) {
		lpm_log_info("discard msg, not from lpm, from_lpm=%u\n", from_lpm);
		return false;
	}

	return true;
}
#else

STATIC bool lpm_is_ipc_msg_check_ok(const struct icmdrv_ipc_msg_info *msg)
{
	uint16_t crc16;

	if ((msg->cmd != (uint8_t)ICM_MAIN_CMD_CCPU_NOTIFY) ||
		(msg->sub_cmd != (uint8_t)ICM_SUB_CMD_HEALTH_EVENT) ||
		(msg->cmd_dest != (uint8_t)OBJ_CMD_CCPU) ||
		(msg->cmd_src != (uint8_t)OBJ_CMD_LP) ||
		(msg->msg_type != ICM_IPC_MSG_REQ)) {
		lpm_log_err("ipc msg cmd=%u sub_cmd=%u cmd_dest=%u cmd_src=%u msg_type=%u\n",
			msg->cmd, msg->sub_cmd, msg->cmd_dest, msg->cmd_src, msg->msg_type);
		return false;
	}

	if (msg->len < sizeof(struct lp_event_data)) {
		lpm_log_err("ipc msg len=%u not valid\n", msg->len);
		return false;
	}

	crc16 = lpm_common_crc16((uint8_t *)msg, (uint16_t)msg->len + (uint16_t)ICM_IPC_MSG_HEAD_LEN);
	if (crc16 != msg->crc16) {
		lpm_log_err("crc16 check failed (cmd_src=%d; cmd_dest=%d)\n", msg->cmd_src, msg->cmd_dest);
		return false;
	}

	return true;
}

STATIC int32_t lpm_complete_event_id(uint32_t ori_event_id, uint32_t excep_id, uint32_t *ret_event_id)
{
	uint32_t event_id = ori_event_id;
	uint32_t node_type;

	// bit16 use the black box to reserve space for sensor type
	event_id |= (excep_id & ((uint32_t)0x10000));

	if (event_id == 0) {
		// The bbox and fault reporting are the same IPC
		// If event_id is equal to 0, it means only the bbox is reported and the fault is not reported
		// Because sensor_type=0 is not a legal sensor
		return -1;
	}

	// assertion and node_type fields contain in excep_id
	if ((excep_id & (uint32_t)0x0000ff00) == ((uint32_t)0x32 << 0x8)) { // BBOX_EXCEP_LPM
		node_type = (uint32_t)DMS_DEV_TYPE_LPM << EVENT_ID_MODULE_BIT;
	} else if ((excep_id & (uint32_t)0x0000ff00) == ((uint32_t)0x20 << 0x8)) { // BBOX_EXCEP_PMU
		node_type = (uint32_t)DMS_DEV_TYPE_PMU << EVENT_ID_MODULE_BIT;
	} else if ((excep_id & (uint32_t)0x0000ff00) == ((uint32_t)0x21 << 0x8)) { // BBOX_EXCEP_VR
		node_type = (uint32_t)DMS_DEV_TYPE_VR << EVENT_ID_MODULE_BIT;
	} else {
		return -1;
	}
	event_id = ((excep_id & (uint32_t)0x30000000) == ((uint32_t)0x1 << 0x1C)) ?
		event_id : (event_id | ((uint32_t)0x1 << EVENT_ID_ASSERTION_BIT));
	event_id = event_id | node_type;

	*ret_event_id = event_id;
	return 0;
}
#endif

STATIC void lpm_parse_event_id(
	struct lpm_inner_fault_event *event, uint32_t dev_id, uint32_t event_id)
{
	uint32_t ori_event_id;
	uint64_t rx_timestamp;

	event->dev_id      = dev_id;
	event->node_type   = lpm_parse_module_from_event_id(event_id);
	event->sensor_type = lpm_parse_sensor_from_event_id(event_id);
	event->event_type  = lpm_parse_errcode_from_event_id(event_id);
	event->assertion   = lpm_parse_assertion_from_event_id(event_id);

	ori_event_id = event_id & (~((uint32_t)0x1U << EVENT_ID_ASSERTION_BIT));
	rx_timestamp = lpm_common_syscount_get_timestamp(dev_id);
	lpm_log_info("receive fault=0x%X, timestamp=%llu us\n", ori_event_id, rx_timestamp);
}

STATIC void lpm_handle_fault_ipc(uint32_t dev_id, uint8_t *data)
{
	int32_t ret;
	struct lpm_inner_fault_event event = {0};
	uint32_t event_id = 0;
#ifdef LPM_IPC_V2
	const struct lpm_fault_ipc_send_msg *msg = (const struct lpm_fault_ipc_send_msg *)data;
#else
	const struct icmdrv_ipc_msg_info *msg = (const struct icmdrv_ipc_msg_info *)data;
	struct lp_event_data *event_data = NULL;
#endif

	// inner function, not check in param
	if (!lpm_is_ipc_msg_check_ok(msg)) {
		return;
	}

#ifdef LPM_IPC_V2
	event_id = *(uint32_t *)(uintptr_t)(&msg->data[0x0]);
#else
	event_data = (struct lp_event_data *)msg->data;
	ret = lpm_complete_event_id(event_data->event_id, event_data->excep_id, &event_id);
	if (ret != 0) {
		return;
	}
#endif

	lpm_parse_event_id(&event, dev_id, event_id);
	ret = lpm_handle_fault(&event);
	lpm_log_err("lpm recv fault ipc, ret=%d, dev_id=%u, event_id=0x%X\n", ret, dev_id, event_id);
}

STATIC int32_t lpm_fault_unregister_fault_ipc(uint32_t dev_num)
{
	int32_t final_ret = 0;
	int32_t ret;
	uint32_t dev_id;

	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		ret = lpm_ipc_unregister_notify_func(dev_id, LPM_IPC_RX_FAULT);
		if (ret != 0) {
			lpm_log_err("lpm unregister fault handle failed, dev_id=%u, ret=%d\n", dev_id, ret);
			final_ret = -1;
		}
	}

	return final_ret;
}

STATIC int32_t lpm_fault_register_fault_ipc(uint32_t dev_num)
{
	int32_t ret;
	uint32_t dev_id;

	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		ret = lpm_ipc_register_rx_notify_func(dev_id, LPM_IPC_RX_FAULT, lpm_handle_fault_ipc);
		if (ret != 0) {
			lpm_log_err("lpm register fault handle failed, dev_id=%u, ret=%d\n", dev_id, ret);
			goto register_fail;
		}
	}

	return 0;

register_fail:
	(void)lpm_fault_unregister_fault_ipc(dev_id);
	return -1;
}

int32_t lpm_fault_from_ipc_init(uint32_t dev_num)
{
	int32_t ret;

	if (dev_num > LPM_DMS_NODE_MAX_NUM) {
		lpm_log_err("lpm fault from ipc init failed, dev_num=%u is out of range\n",
			dev_num);
		return -1;
	}

	ret = lpm_fault_register_fault_ipc(dev_num);
	if (ret != 0) {
		lpm_log_err("lpm register fault handle failed, ret=%d\n", ret);
		return ret;
	}

	lpm_log_info("lpm fault from ipc init success\n");
	return 0;
}

int32_t lpm_fault_from_ipc_exit(uint32_t dev_num)
{
	int32_t ret;

	if (dev_num > LPM_DMS_NODE_MAX_NUM) {
		lpm_log_err("lpm fault from ipc exit failed, dev_num=%u is out of range\n",
			dev_num);
		return -1;
	}

	ret = lpm_fault_unregister_fault_ipc(dev_num);
	if (ret != 0) {
		lpm_log_err("lpm unregister fault handle failed, ret=%d\n", ret);
		return ret;
	}

	lpm_log_info("lpm fault from ipc exit success\n");
	return 0;
}