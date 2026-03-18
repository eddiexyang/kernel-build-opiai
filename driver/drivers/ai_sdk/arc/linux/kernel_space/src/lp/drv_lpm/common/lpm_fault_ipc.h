/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
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
#ifndef LPM_FAULT_IPC_H
#define LPM_FAULT_IPC_H
#include <linux/types.h>
#include "lpm_fault_common.h"

#if defined(LPM_IPC_V2) || defined(LPM_IPC_V3) || defined(LPM_IPC_V1)
#include <linux/notifier.h>

// each ipc rx mailbox supports up to 2 module monitoring
#define LPM_FAULT_IPC_MAX_NOTITY_NUM 2

typedef void (*fn_fault_lpm_ipc_notify)(uint32_t dev_id, uint8_t *ipc_data);

struct lpm_fault_ipc_notify_list {
	enum lpm_fault_ipc_rx_type rx_type;
	fn_fault_lpm_ipc_notify fn_notify;
};

struct lpm_fault_ipc_head_cfg {
	uint8_t sub_cmd;
	uint8_t cmd;
	uint8_t cmd_dest;
	uint8_t cmd_src;
};

struct lpm_ipc_tx_channel_info {
	uint8_t chan_id;
	struct lpm_fault_ipc_head_cfg head_cfg;
};

struct lpm_ipc_rx_channel_info {
	uint8_t chan_id;
	// the expected number of modules that need to monitor and change mailboxes
	uint8_t rx_type_cnt;
	enum lpm_fault_ipc_rx_type rx_type[LPM_FAULT_IPC_MAX_NOTITY_NUM];
};

struct lpm_ipc_channel_cfg {
	// tx is maintained based on the send type dimension
	uint32_t tx_type_num;
	struct lpm_ipc_tx_channel_info tx_ipc[(uint32_t)LPM_IPC_TX_TYPE_MAX];

	// maintain the mapping relationship between rx_type and rx_chan_type
	uint8_t rx_chan_type[(uint32_t)LPM_IPC_RX_TYPE_MAX];

	// rx is maintained based on the rx channel dimension
	uint32_t rx_chan_num;
	struct lpm_ipc_rx_channel_info rx_ipc[LPM_FAULT_IPC_RPROC_NUM];
};

struct lpm_fault_ipc_rx_cfg {
	uint32_t dev_id;
	uint32_t rx_chan_type;
	struct notifier_block ipc_monitor;
	// the number of modules actually registered for monitor
	uint8_t notify_cnt;
	fn_fault_lpm_ipc_notify fn_notify[LPM_FAULT_IPC_MAX_NOTITY_NUM];
};

struct lpm_ipc_dev_info {
	struct lpm_fault_ipc_rx_cfg rx_ipc_cfg[LPM_FAULT_IPC_RPROC_NUM];
};

struct lpm_ipc_info {
	uint32_t dev_num;
	struct lpm_ipc_channel_cfg channel_cfg;
	struct lpm_ipc_dev_info dev_data[LPM_DMS_NODE_MAX_NUM];
};

struct lpm_fault_ipc_msg {
	enum lpm_fault_ipc_tx_type send_type;
	void *in;
	uint32_t in_len;
	void *out;
	uint32_t out_len;
};

#if defined(LPM_IPC_V3)
#define LPM_IPC_DATA_SIZE ICM_MSG_DATA_LENGTH

// the same with struct icmdrv_ipc_msg
struct lpm_fault_ipc_send_msg {
	uint8_t sub_cmd;
	uint8_t cmd;
	uint8_t cmd_dest;
	uint8_t cmd_src;
	uint8_t msg_type;
	uint8_t len;
	uint8_t data[LPM_IPC_DATA_SIZE];
};

// the same with struct icmdrv_ipc_msg_info
struct lpm_fault_ipc_ack_msg {
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

struct lpm_fault_ipc_send_msg {
	uint8_t sub_cmd;
	uint8_t cmd;
	uint8_t cmd_dest;
	uint8_t cmd_src;
	uint8_t data[LPM_IPC_DATA_SIZE];
};

struct lpm_fault_ipc_ack_msg {
	uint8_t sub_cmd;
	uint8_t cmd;
	uint8_t cmd_dest;
	uint8_t cmd_src;
	uint8_t data[LPM_IPC_DATA_SIZE];
};

#elif defined(LPM_IPC_V1)
#define LPM_IPC_DATA_SIZE 20

struct lpm_fault_ipc_send_msg {
	uint8_t sub_cmd;
	uint8_t cmd;
	uint8_t cmd_dest;
	uint8_t cmd_src;
	uint8_t data[LPM_IPC_DATA_SIZE];
};

struct lpm_fault_ipc_ack_msg {
	uint8_t sub_cmd;
	uint8_t cmd;
	uint8_t cmd_dest;
	uint8_t cmd_src;
	uint8_t data[LPM_IPC_DATA_SIZE];
};
#endif /* #if defined(LPM_IPC_V3) #elif defined(LPM_IPC_V2) #elif defined(LPM_IPC_V1) */

int32_t lpm_fault_ipc_set_rx_channel(enum lpm_fault_ipc_rx_type rx_type, uint32_t chan_id);
int32_t lpm_fault_ipc_set_tx_channel(
	enum lpm_fault_ipc_tx_type tx_type, uint32_t chan_id, struct lpm_fault_ipc_head_cfg *ipc_head);

#ifdef CFG_SOC_PLATFORM_MDC_V51
int32_t lpm_fault_ipc_send_sync_msg(uint32_t dev_id, struct lpm_fault_ipc_msg *msg_info);
#endif

int32_t lpm_fault_ipc_send_async_msg(uint32_t dev_id, struct lpm_fault_ipc_msg *msg_info);

int32_t lpm_ipc_register_rx_notify_func(uint32_t dev_id,
	enum lpm_fault_ipc_rx_type rx_type, fn_fault_lpm_ipc_notify fn_notify);
int32_t lpm_ipc_unregister_notify_func(uint32_t dev_id, enum lpm_fault_ipc_rx_type rx_type);

/*
 * Function Name: lpm_ipc_receiver_init
 * Decription: register callback to ipc
 * Parameters: dev_num
 * Return: 0 is success, otherwise failed
 */
int32_t lpm_ipc_receiver_init(uint32_t dev_num);

/*
 * Function Name: lpm_ipc_receiver_exit
 * Decription: unregister callback to ipc
 * Parameters: dev_num
 * Return: 0 is success, otherwise failed
 */
int32_t lpm_ipc_receiver_exit(uint32_t dev_num);

#else

static inline int32_t lpm_ipc_receiver_init(uint32_t dev_num)
{
	(void)dev_num;
	return 0;
}

static inline int32_t lpm_ipc_receiver_exit(uint32_t dev_num)
{
	(void)dev_num;
	return 0;
}

#endif /* #if defined(LPM_IPC_V2) || defined(LPM_IPC_V3)  || defined(LPM_IPC_V1) */
#endif /* LPM_FAULT_IPC_H */
