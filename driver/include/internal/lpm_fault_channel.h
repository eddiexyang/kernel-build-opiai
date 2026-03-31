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
#ifndef LPM_FAULT_CHANNEL_H
#define LPM_FAULT_CHANNEL_H
#include <linux/types.h>

#if defined(LPM_IPC_V2) || defined(LPM_IPC_V3) || defined(LPM_IPC_V1)
#include <lpm_fault_ipc.h>

struct lpm_fault_ipc_type_cfg {
	const char *cfg_name;      // dts config name
	uint8_t module_type; // module index
	uint8_t chan_id;     // default channel id
};

struct lpm_fault_ipc_tx_type_cfg {
	struct lpm_fault_ipc_type_cfg ipc_cfg;
	struct lpm_fault_ipc_head_cfg ipc_head;
};

struct lpm_fault_ipc_channel {
	uint32_t tx_ipc[(uint32_t)LPM_IPC_TX_TYPE_MAX];
	uint32_t rx_ipc[(uint32_t)LPM_IPC_RX_TYPE_MAX];
};

struct lpm_fault_send_ipc_data {
	uint8_t rx_fault_ipc;
	uint8_t rx_hb_ipc;
};

int32_t lpm_fault_channel_init(uint32_t dev_num);

int32_t lpm_fault_channel_exit(uint32_t dev_num);

#else

static inline int32_t lpm_fault_channel_init(uint32_t dev_num)
{
	(void)dev_num;
	return 0;
}

static inline int32_t lpm_fault_channel_exit(uint32_t dev_num)
{
	(void)dev_num;
	return 0;
}

#endif /* #if defined(LPM_IPC_V2) || defined(LPM_IPC_V3) || defined(LPM_IPC_V1) */

#endif /* LPM_FAULT_CHANNEL_H */
