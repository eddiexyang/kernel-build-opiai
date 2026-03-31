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
#include <linux/types.h>
#include <linux/notifier.h>
#include <linux/kernel.h>

#include "dms_sensor_type.h"

#include "memory_log.h"
#include "safety_ras_node.h"
#include "safety_ras_ipc.h"

uint32_t memory_dev_num(uint32_t dev_num);
int32_t memory_msg_handle(int32_t dev_id, void *data);
struct drv_memory_devices *memory_get_devices(void);

uint16_t memory_get_event_type_from_event_id(uint32_t event_id)
{
	return (uint16_t)(event_id & 0x1FFU);
}

uint8_t memory_get_sensor_type_from_event_id(uint32_t event_id)
{
	return (uint8_t)((event_id >> EVENT_ID_SENSOR_BIT) & 0xFFU);
}

uint8_t memory_get_queue_index(uint8_t senseor_type, uint8_t chn)
{
	/* 0~23 ras; 24~47 mem; 48 temputer; 49 safety */
	if (senseor_type == (uint8_t)DMS_SEN_TYPE_RAS_SENSOR) {
		return chn;
	} else if (senseor_type == (uint8_t)DMS_SEN_TYPE_MEMORY) {
		return chn + IDX_CHN_MAX;
	} else if (senseor_type == (uint8_t)DMS_SEN_TYPE_TEMPERATURE) {
		return 2U * IDX_CHN_MAX;
	} else if (senseor_type == (uint8_t)DMS_SEN_TYPE_SAFETY_SENSOR) {
		return (2U * IDX_CHN_MAX) + 0x1U;
	} else {
		return (2U * IDX_CHN_MAX) + 0x2U;
	}
}

int32_t memory_ipc_rx_notifier(struct notifier_block *nb, unsigned long len, void *data)
{
	int32_t ret;
	struct memory_ipc_info *ipc_info = NULL;

	if ((nb == NULL) || (data == NULL) || (len != sizeof(struct memory_ipc_info) / sizeof(uint32_t))) {
		memory_drv_err("The parameter is invailed. (nb=%d; data=%d; len=%lu)\n", nb == NULL, data == NULL, len);
		return NOTIFY_DONE;
	}

	ipc_info = container_of(nb, struct memory_ipc_info, ipc_notifier);
	if (ipc_info->dev_id > memory_dev_num(memory_get_devices()->dev_num)) {
		memory_drv_err("The device id is invalid. (dev_id=%d)\n", ipc_info->dev_id);
		return NOTIFY_DONE;
	}

	ret = memory_msg_handle((int32_t)ipc_info->dev_id, data);
	if (ret != 0) {
		memory_drv_err("memory_msg_handle. (ret=%d)\n", ret);
	}

	return NOTIFY_OK;
}
