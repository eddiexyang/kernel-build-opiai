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
*/
#ifndef LPM_DEVMNG_IPC_H
#define LPM_DEVMNG_IPC_H

#include <linux/types.h>
#include <linux/time64.h>
#include "icm_interface.h"
#include "drv_ipc.h"
#include "lpm_devmng_common.h"

#if defined(LPM_IPC_V2) || defined(LPM_IPC_V3) || defined(LPM_IPC_V1)
#ifdef CFG_SOC_PLATFORM_MINIV3
enum lpm_devmng_ipc_send_type {
	LPM_IPC_SET_IDLE,
	LPM_IPC_NOTIFY_IDLE,
	LPM_IPC_SET_PROFILE,
	LPM_IPC_NOTIFY_SUSPEND,
	LPM_IPC_NOTIFY_RESUME,
	LPM_IPC_NOTIFY_VRD_UPGRADE,
	LPM_IPC_QUERY_LP_ACG,
	LPM_IPC_SET_LP_TEST,
	LPM_IPC_SEND_TYPE_MAX
};
#elif defined(CFG_SOC_PLATFORM_CLOUD_V2)
enum lpm_devmng_ipc_send_type {
	LPM_IPC_SET_IDLE,
	LPM_IPC_SET_LP_TEST,
	LPM_IPC_SET_TURBO,
	LPM_IPC_SET_MAX_AIC_FREQ,
	LPM_IPC_SET_SOC_STRESS,
	LPM_IPC_GET_ACG_CPM,
	LPM_IPC_SEND_TYPE_MAX
};
#elif defined(CFG_SOC_PLATFORM_MDC_V51)
enum lpm_devmng_ipc_send_type {
	LPM_IPC_QUERY_SOC_TEMPERATURE,
	LPM_IPC_QUERY_DDR_FREQ,
	LPM_IPC_QUERY_CTRLCPU_FREQ,
	LPM_IPC_QUERY_AICORE_FREQ,
	LPM_IPC_QUERY_VECTOR_FREQ,
	LPM_IPC_SEND_TYPE_MAX
};
#elif defined(CFG_SOC_PLATFORM_MDC_V51_LITE)
enum lpm_devmng_ipc_send_type {
	LPM_IPC_SET_LP_TEST,
	LPM_IPC_SEND_TYPE_MAX
};
#endif

struct lpm_devmng_ipc_msg {
	enum lpm_devmng_ipc_send_type send_type;
	// input buffer
	void *in;
	// buffer len for input
	uint32_t in_len;
	// out buffer
	void *out;
	// ack message offset
	uint32_t out_pos;
	// buffer len for output
	uint32_t out_len;
};

#if defined(LPM_IPC_V3)
#define LPM_IPC_DATA_SIZE ICM_MSG_DATA_LENGTH

// the same with struct icmdrv_ipc_msg
struct lpm_devmng_ipc_send_msg {
	uint8_t sub_cmd;
	uint8_t cmd;
	uint8_t cmd_dest;
	uint8_t cmd_src;
	uint8_t msg_type;
	uint8_t len;
	uint8_t data[LPM_IPC_DATA_SIZE];
};

// the same with struct icmdrv_ipc_msg_info
struct lpm_devmng_ipc_ack_msg {
	uint8_t sub_cmd;
	uint8_t cmd;
	uint8_t cmd_dest;
	uint8_t cmd_src;
	uint16_t msg_seq;
	uint8_t msg_type : 1;
	uint8_t reserved : 3;
	uint8_t version  : 4;
	uint8_t len;
	uint8_t data[LPM_IPC_DATA_SIZE];
	uint16_t crc16;
};

#elif defined(LPM_IPC_V2)

#define LPM_IPC_DATA_SIZE 28

struct lpm_devmng_ipc_send_msg {
	uint8_t sub_cmd;
	uint8_t cmd;
	uint8_t cmd_dest;
	uint8_t cmd_src;
	uint8_t data[LPM_IPC_DATA_SIZE];
};

struct lpm_devmng_ipc_ack_msg {
	uint8_t sub_cmd;
	uint8_t cmd;
	uint8_t cmd_dest;
	uint8_t cmd_src;
	uint8_t data[LPM_IPC_DATA_SIZE];
};

#elif defined(LPM_IPC_V1)

#define LPM_IPC_DATA_SIZE 20

struct lpm_devmng_ipc_send_msg {
	uint8_t sub_cmd;
	uint8_t cmd;
	uint8_t cmd_dest;
	uint8_t cmd_src;
	uint8_t data[LPM_IPC_DATA_SIZE];
};

struct lpm_devmng_ipc_ack_msg {
	uint8_t sub_cmd;
	uint8_t cmd;
	uint8_t cmd_dest;
	uint8_t cmd_src;
	uint8_t data[LPM_IPC_DATA_SIZE];
};

#endif

struct lpm_devmng_ipc_head {
	uint8_t sub_cmd;
	uint8_t cmd;
	uint8_t cmd_dest;
	uint8_t cmd_src;
};

typedef bool (*fn_ipc_check_ack_msg)(
	uint32_t dev_id, enum lpm_devmng_ipc_send_type send_type, struct lpm_devmng_ipc_ack_msg *ack_msg);

struct lpm_devmng_ipc_fn_hook {
	// check ack msg format, or cache data/ try send again
	fn_ipc_check_ack_msg fn_check_ack;
};

struct lpm_devmng_ipc_tx_cfg {
	// ipc mailbox channel
	uint32_t chan_id;
	// retry times after ipc message sending failure
	uint32_t retry_times;

	// ipc head info
	struct lpm_devmng_ipc_head head;
	struct lpm_devmng_ipc_fn_hook fn_hook;
};

struct lpm_devmng_ipc_priv {
	struct lpm_devmng_ipc_tx_cfg *tx_cfg;
};

int32_t lpm_ipc_send_sync_msg(uint32_t dev_id, struct lpm_devmng_ipc_msg *msg_info);
int32_t lpm_ipc_send_async_msg(uint32_t dev_id, struct lpm_devmng_ipc_msg *msg_info);

int32_t lpm_ipc_probe(uint64_t *param, uint32_t param_num);
int32_t lpm_ipc_remove(uint64_t *param, uint32_t param_num);

#else

static inline int32_t lpm_ipc_probe(uint64_t *param, uint32_t param_num)
{
	(void)param;
	(void)param_num;
	return 0;
}

static inline int32_t lpm_ipc_remove(uint64_t *param, uint32_t param_num)
{
	(void)param;
	(void)param_num;
	return 0;
}

#endif
#endif
