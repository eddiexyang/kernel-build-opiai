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
#include <linux/notifier.h>
#include "icm_interface.h"
#include "dms_node_type.h"

#include "lpm_fault_common.h"
#include "lpm_init.h"
#include "lpm_fault_report.h"
#include "lpm_fault_heartbeat.h"
#include "lpm_fault_ipc.h"

STATIC struct lpm_ipc_info g_lpm_ipc_notify_info = {0};

STATIC struct lpm_ipc_info *lpm_ipc_priv_info(void)
{
	return &g_lpm_ipc_notify_info;
}

STATIC struct lpm_ipc_dev_info *lpm_ipc_dev_priv_info(uint32_t dev_id)
{
	return &g_lpm_ipc_notify_info.dev_data[dev_id];
}

STATIC int32_t lpm_ipc_check_and_get_rx_chan_type(
	enum lpm_fault_ipc_rx_type rx_type, uint32_t chan_id, uint32_t *chan_type)
{
	uint32_t i;
	uint32_t j;
	struct lpm_ipc_info *ipc_priv = lpm_ipc_priv_info();

	if (rx_type >= LPM_IPC_RX_TYPE_MAX) {
		lpm_log_err("set rx channel failed, rx_type=%u is out of range\n", (uint32_t)rx_type);
		return -1;
	}

	for (i = 0; i < ipc_priv->channel_cfg.rx_chan_num; i++) {
		// check chan_id is repeat
		if (ipc_priv->channel_cfg.rx_ipc[i].chan_id != chan_id) {
			continue;
		}

		// check rx_type is repeat
		for (j = 0; j < ipc_priv->channel_cfg.rx_ipc[i].rx_type_cnt; j++) {
			if (ipc_priv->channel_cfg.rx_ipc[i].rx_type[j] == rx_type) {
				lpm_log_err("set rx channel repeat, rx_type=%u, chan_id=%u, rx_chan_type=%u\n",
					(uint32_t)rx_type, chan_id, i);
				return -1;
			}
		}
		*chan_type = i;
		return 0;
	}

	if (ipc_priv->channel_cfg.rx_chan_num >= LPM_FAULT_IPC_RPROC_NUM) {
		lpm_log_err("set rx channel full, rx_type=%u, chan_id=%u\n", (uint32_t)rx_type, chan_id);
		return -1;
	}

	*chan_type = ipc_priv->channel_cfg.rx_chan_num;
	return 0;
}

STATIC bool lpm_ipc_check_tx_channel_param(enum lpm_fault_ipc_tx_type tx_type)
{
	struct lpm_ipc_info *ipc_priv = lpm_ipc_priv_info();

	if (tx_type >= LPM_IPC_TX_TYPE_MAX) {
		lpm_log_err("set tx channel failed, tx_type=%u is out of range\n", (uint32_t)tx_type);
		return false;
	}
	if (ipc_priv->channel_cfg.tx_type_num >= (uint32_t)LPM_IPC_TX_TYPE_MAX) {
		lpm_log_err("set tx channel failed, tx ipc is full, tx_type=%u\n", (uint32_t)tx_type);
		return false;
	}
	return true;
}

int32_t lpm_fault_ipc_set_rx_channel(enum lpm_fault_ipc_rx_type rx_type, uint32_t chan_id)
{
	int32_t ret;
	uint32_t rx_chan_type;
	struct lpm_ipc_info *ipc_priv = lpm_ipc_priv_info();
	struct lpm_ipc_rx_channel_info *rx_channel = NULL;

	ret = lpm_ipc_check_and_get_rx_chan_type(rx_type, chan_id, &rx_chan_type);
	if (ret != 0) {
		lpm_log_err("set rx channel failed, rx_type=%u, chan_id=%u\n", (uint32_t)rx_type, chan_id);
		return -1;
	}

	// maintain the mapping relationship between rx_type and rx_chan_type
	ipc_priv->channel_cfg.rx_chan_type[(uint32_t)rx_type] = (uint8_t)rx_chan_type;

	// save rx chan type info
	rx_channel = &ipc_priv->channel_cfg.rx_ipc[rx_chan_type];
	if (rx_channel->rx_type_cnt == 0) {
		ipc_priv->channel_cfg.rx_chan_num++;
	}
	rx_channel->chan_id = (uint8_t)chan_id;
	rx_channel->rx_type[rx_channel->rx_type_cnt] = rx_type;
	rx_channel->rx_type_cnt++;

	lpm_log_info("set rx channel success, rx_chan_type=%u, rx_type=%u, chan_id=%u, "
		"rx_type_cnt=%u, rx_chan_num=%u\n",
		rx_chan_type, (uint32_t)rx_type, chan_id,
		rx_channel->rx_type_cnt, ipc_priv->channel_cfg.rx_chan_num);
	return 0;
}

int32_t lpm_fault_ipc_set_tx_channel(
	enum lpm_fault_ipc_tx_type tx_type, uint32_t chan_id, struct lpm_fault_ipc_head_cfg *ipc_head)
{
	int32_t ret;
	struct lpm_ipc_info *ipc_priv = lpm_ipc_priv_info();

	if (!lpm_ipc_check_tx_channel_param(tx_type)) {
		lpm_log_err("check param failed when set tx channel, tx_type=%u, chan_id=%u\n",
			(uint32_t)tx_type, chan_id);
		return -1;
	}

	ret = memcpy_s(&ipc_priv->channel_cfg.tx_ipc[(uint32_t)tx_type].head_cfg,
		sizeof(struct lpm_fault_ipc_head_cfg), ipc_head, sizeof(struct lpm_fault_ipc_head_cfg));
	if (ret != 0) {
		lpm_log_err("set tx channel failed, tx_type=%u, chan_id=%u, copy ipc head config failed\n",
			(uint32_t)tx_type, chan_id);
		return -1;
	}

	// tx is maintained based on the send type dimension
	// various module can use same ipc mailbox
	ipc_priv->channel_cfg.tx_ipc[(uint32_t)tx_type].chan_id = (uint8_t)chan_id;
	ipc_priv->channel_cfg.tx_type_num++;

	lpm_log_info("set tx channel success, tx_type=%u chan_id=%u, tx_type_num=%u\n",
		(uint32_t)tx_type, chan_id, ipc_priv->channel_cfg.tx_type_num);
	return 0;
}

STATIC uint8_t lpm_ipc_fill_send_msg_head(
	struct lpm_fault_ipc_msg *msg_info, struct lpm_fault_ipc_send_msg *send_msg)
{
	struct lpm_ipc_info *ipc_priv = lpm_ipc_priv_info();
	struct lpm_ipc_tx_channel_info *tx_channel = &ipc_priv->channel_cfg.tx_ipc[(uint32_t)msg_info->send_type];

	send_msg->sub_cmd  = tx_channel->head_cfg.sub_cmd;
	send_msg->cmd      = tx_channel->head_cfg.cmd;
	send_msg->cmd_dest = tx_channel->head_cfg.cmd_dest;
	send_msg->cmd_src  = tx_channel->head_cfg.cmd_src;

#if defined(LPM_IPC_V3)
	// old version not support ipc msg len
	send_msg->len      = (uint8_t)msg_info->in_len;
#endif

	return tx_channel->chan_id;
}

STATIC bool lpm_ipc_check_and_fill_in_param(
	struct lpm_fault_ipc_msg *msg_info, struct lpm_fault_ipc_send_msg *send_msg, uint8_t *chan_id)
{
	int32_t ret;

	if (msg_info->send_type >= LPM_IPC_TX_TYPE_MAX) {
		lpm_log_err("ipc param check, send_type=%u should be less than %u\n",
			(uint32_t)msg_info->send_type, (uint32_t)LPM_IPC_TX_TYPE_MAX);
		return false;
	}

	if ((msg_info->in == NULL) || (msg_info->in_len > LPM_IPC_DATA_SIZE)) {
		lpm_log_err("ipc param check, input char is NULL or in_len is wrong, "
			"in_len=%u, max_in_len=%u\n",
			msg_info->in_len, LPM_IPC_DATA_SIZE);
		return false;
	}

	ret = memcpy_s(send_msg->data, sizeof(send_msg->data), msg_info->in, msg_info->in_len);
	if (ret != 0) {
		lpm_log_err("ipc param check, copy in msg failed, ret=%d, in_len=%u, send_type=%u\n",
			ret, msg_info->in_len, (uint32_t)msg_info->send_type);
		return false;
	}

	*chan_id = lpm_ipc_fill_send_msg_head(msg_info, send_msg);
	return true;
}

#ifdef CFG_SOC_PLATFORM_MDC_V51
STATIC bool lpm_ipc_check_out_param(struct lpm_fault_ipc_msg *msg_info)
{
	if ((msg_info->out == NULL) || (msg_info->out_len > LPM_IPC_DATA_SIZE)) {
		lpm_log_err("ipc param check, output char is NULL or out_len is wrong, "
			"out_len=%u, max_out_len=%u\n",
			msg_info->out_len, LPM_IPC_DATA_SIZE);
		return false;
	}
	return true;
}

STATIC bool lpm_fault_ipc_check_ack_head(
	struct lpm_fault_ipc_send_msg *send_msg, struct lpm_fault_ipc_ack_msg *ack_msg)
{
	if (ack_msg->sub_cmd != send_msg->sub_cmd) {
		lpm_log_err("check ack head: sub_cmd not match, send=%u, ack=%u\n",
			send_msg->sub_cmd, ack_msg->sub_cmd);
		return false;
	}
	if (ack_msg->cmd != send_msg->cmd) {
		lpm_log_err("check ack head: cmd not match, send=%u, ack=%u\n",
			send_msg->cmd, ack_msg->cmd);
		return false;
	}
	if (ack_msg->cmd_dest != send_msg->cmd_src) {
		lpm_log_err("check ack head: src and dest not match, send_src=%u, ack_dest=%u\n",
			send_msg->cmd_src, ack_msg->cmd_dest);
		return false;
	}
	if (ack_msg->cmd_src != send_msg->cmd_dest) {
		lpm_log_err("check ack head: src and dest not match, send_dest=%u, ack_src=%u\n",
			send_msg->cmd_dest, ack_msg->cmd_src);
		return false;
	}

	return true;
}

int32_t lpm_fault_ipc_send_sync_msg(
	uint32_t dev_id, struct lpm_fault_ipc_msg *msg_info)
{
	int32_t ret;
	uint8_t chan_id;
	struct lpm_fault_ipc_send_msg send_msg = {0};
	struct lpm_fault_ipc_ack_msg ack_msg   = {0};

	if (!lpm_ipc_check_and_fill_in_param(msg_info, &send_msg, &chan_id)) {
		lpm_log_err("send sync ipc, in param invalid\n");
		return -1;
	}

	if (!lpm_ipc_check_out_param(msg_info)) {
		lpm_log_err("send sync ipc, out param invalid\n");
		return -1;
	}

	ret = icm_msg_send_sync(ICM_FD_BUILD(dev_id, chan_id),
		(rproc_msg_t *)(void *)&send_msg, (uint32_t)(sizeof(send_msg) / sizeof(rproc_msg_t)),
		(rproc_msg_t *)(void *)&ack_msg, (uint32_t)(sizeof(ack_msg) / sizeof(rproc_msg_t)));
	if (ret != 0) {
		lpm_log_err("send sync ipc failed, send_type=%u, ret=%d\n",
			(uint32_t)msg_info->send_type, ret);
		return ret;
	}

	if (!lpm_fault_ipc_check_ack_head(&send_msg, &ack_msg)) {
		lpm_log_err("send sync ipc, invalid ack head\n");
		return -1;
	}
	// only need to copy the specified part
	ret = memcpy_s(msg_info->out, msg_info->out_len, ack_msg.data, msg_info->out_len);
	if (ret != 0) {
		lpm_log_err("send sync ipc, copy ack msg failed, ret=%d, out_len=%u, send_type=%u\n",
			ret, msg_info->out_len, (uint32_t)msg_info->send_type);
		return ret;
	}

	return 0;
}
#endif /* CFG_SOC_PLATFORM_MDC_V51 */

int32_t lpm_fault_ipc_send_async_msg(
	uint32_t dev_id, struct lpm_fault_ipc_msg *msg_info)
{
	int32_t ret;
	uint8_t chan_id;
	struct lpm_fault_ipc_send_msg send_msg = {0};

	if (!lpm_ipc_check_and_fill_in_param(msg_info, &send_msg, &chan_id)) {
		lpm_log_err("async sync ipc failed, in param invalid\n");
		return -1;
	}

	ret = icm_msg_send_async(ICM_FD_BUILD(dev_id, chan_id),
		(rproc_msg_t *)(void *)&send_msg, (uint32_t)(sizeof(send_msg) / sizeof(rproc_msg_t)));
	if (ret != 0) {
		lpm_log_err("send async ipc failed, send_type=%u, ret=%d\n",
			(uint32_t)msg_info->send_type, ret);
		return ret;
	}

	return 0;
}

STATIC int32_t lpm_handle_event_from_ipc(struct notifier_block *nb, unsigned long len, void *data)
{
	uint32_t i;
	const struct lpm_fault_ipc_rx_cfg *monitor = NULL;

	(void)len;
	if ((nb == NULL) || (data == NULL)) {
		lpm_log_err("ipc nb or data null ptr\n");
		return NOTIFY_DONE;
	}

	monitor = container_of(nb, struct lpm_fault_ipc_rx_cfg, ipc_monitor);
	if (monitor == NULL) {
		lpm_log_err("monitor is null\n");
		return NOTIFY_DONE;
	}
	if (!lpm_common_check_dev_id(monitor->dev_id)) {
		lpm_log_err("dev_id=%u is out of range, rx_chan_type=%u\n", monitor->dev_id, monitor->rx_chan_type);
		return NOTIFY_DONE;
	}
	if (monitor->rx_chan_type >= LPM_FAULT_IPC_RPROC_NUM) {
		lpm_log_err("rx_chan_type=%u should less than support_rx_ipc_num=%u, dev_id=%u\n",
			monitor->rx_chan_type, LPM_FAULT_IPC_RPROC_NUM, monitor->dev_id);
		return NOTIFY_DONE;
	}

	for (i = 0; i < monitor->notify_cnt; i++) {
		monitor->fn_notify[i](monitor->dev_id, (uint8_t *)data);
	}

	// this function does not return an error, only records exceptions,
	// otherwise it will affect the processing of other fields
	return NOTIFY_DONE;
}

STATIC void lpm_ipc_rx_unregister(uint32_t dev_id)
{
	uint32_t chan_type;
	int32_t ret;
	struct lpm_ipc_info *ipc_priv = lpm_ipc_priv_info();

	for (chan_type = 0; chan_type < LPM_FAULT_IPC_RPROC_NUM; chan_type++) {
		ret = icm_rx_unregister(dev_id, ipc_priv->channel_cfg.rx_ipc[chan_type].chan_id,
			IPC_OBJ_LP, &ipc_priv->dev_data[dev_id].rx_ipc_cfg[chan_type].ipc_monitor);
		if (ret != 0) {
			lpm_log_err("ipc rx unregister failed, dev_id=%u, chan_type=%u, chan_id=%u\n",
				dev_id, chan_type, ipc_priv->channel_cfg.rx_ipc[chan_type].chan_id);
		}
	}
}

STATIC int32_t lpm_ipc_rx_register(uint32_t dev_id)
{
	uint32_t chan_type;
	uint32_t i;
	int32_t ret;
	struct lpm_ipc_info *ipc_priv = lpm_ipc_priv_info();

	for (chan_type = 0; chan_type < LPM_FAULT_IPC_RPROC_NUM; chan_type++) {
		ipc_priv->dev_data[dev_id].rx_ipc_cfg[chan_type].dev_id = dev_id;
		ipc_priv->dev_data[dev_id].rx_ipc_cfg[chan_type].rx_chan_type = chan_type;
		ipc_priv->dev_data[dev_id].rx_ipc_cfg[chan_type].ipc_monitor.notifier_call = lpm_handle_event_from_ipc;

		ret = icm_rx_register(dev_id, ipc_priv->channel_cfg.rx_ipc[chan_type].chan_id,
			IPC_OBJ_LP, &ipc_priv->dev_data[dev_id].rx_ipc_cfg[chan_type].ipc_monitor);
		if (ret != 0) {
			lpm_log_err("ipc_rx_register failed, dev_id=%u, ret=%d\n", dev_id, ret);
			goto ipc_register_failed;
		} else {
			lpm_log_info("ipc rx register succ, dev_id=%u, chan_type=%u, chan_id=%u\n",
				dev_id, chan_type, ipc_priv->channel_cfg.rx_ipc[chan_type].chan_id);
		}
	}

	return 0;

ipc_register_failed:
	for (i = 0; i < chan_type; i++) {
		ret = icm_rx_unregister(dev_id, ipc_priv->channel_cfg.rx_ipc[i].chan_id,
			IPC_OBJ_LP, &ipc_priv->dev_data[dev_id].rx_ipc_cfg[i].ipc_monitor);
		if (ret != 0) {
			lpm_log_err("ipc rx rollback register failed, dev_id=%u, chan_type=%u, chan_id=%u\n",
				dev_id, i, ipc_priv->channel_cfg.rx_ipc[i].chan_id);
		}
	}

	return -1;
}

STATIC bool lpm_ipc_check_chan_cfg_info(uint32_t dev_num)
{
	uint32_t i;
	uint32_t dev_id;
	struct lpm_ipc_info *ipc_priv = lpm_ipc_priv_info();
	struct lpm_ipc_dev_info *dev_priv = NULL;

	if (ipc_priv->channel_cfg.tx_type_num < (uint32_t)LPM_IPC_TX_TYPE_MAX) {
		lpm_log_err("tx ipc registration is incomplete, tx_type_num=%u\n",
			ipc_priv->channel_cfg.tx_type_num);
		return false;
	}
	if (ipc_priv->channel_cfg.rx_chan_num != LPM_FAULT_IPC_RPROC_NUM) {
		lpm_log_err("rx ipc registration is incomplete, rx_chan_num=%u, support_rx_ipc_num=%u\n",
			ipc_priv->channel_cfg.rx_chan_num, LPM_FAULT_IPC_RPROC_NUM);
		return false;
	}

	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		dev_priv = lpm_ipc_dev_priv_info(dev_id);
		for (i = 0; i < ipc_priv->channel_cfg.rx_chan_num; i++) {
			if (dev_priv->rx_ipc_cfg[i].notify_cnt != ipc_priv->channel_cfg.rx_ipc[i].rx_type_cnt) {
				lpm_log_err("rx_chan=%u ipc notify registration is incomplete, notify_cnt=%u, rx_type_cnt=%u\n",
					i, dev_priv->rx_ipc_cfg[i].notify_cnt,
					ipc_priv->channel_cfg.rx_ipc[i].rx_type_cnt);
				return false;
			}
		}
	}

	return true;
}

int32_t lpm_ipc_receiver_init(uint32_t dev_num)
{
	int32_t ret = 0;
	uint32_t dev_id;
	uint32_t i;
	struct lpm_ipc_info *ipc_priv = lpm_ipc_priv_info();

	if (dev_num > LPM_DMS_NODE_MAX_NUM) {
		lpm_log_err("lpm ipc receiver init failed, dev_num=%u is out of range\n",
			dev_num);
		return -1;
	}

	if (!lpm_ipc_check_chan_cfg_info(dev_num)) {
		lpm_log_err("ipc check channel config failed\n");
		return -1;
	}
	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		ret = lpm_ipc_rx_register(dev_id);
		if (ret != 0) {
			lpm_log_err("ipc rx register failed, dev_id=%u, ret=%d\n", dev_id, ret);
			goto ipc_init_failed;
		}
	}
	ipc_priv->dev_num = dev_num;

	lpm_log_info("lpm ipc receiver init success, dev_num=%u\n", dev_num);
	return 0;

ipc_init_failed:
	for (i = 0; i < dev_id; i++) {
		lpm_ipc_rx_unregister(i);
	}
	lpm_log_err("lpm ipc receiver init failed, dev_num=%u\n", dev_num);
	return -1;
}

int32_t lpm_ipc_receiver_exit(uint32_t dev_num)
{
	uint32_t dev_id;

	if (dev_num > LPM_DMS_NODE_MAX_NUM) {
		lpm_log_err("lpm ipc receiver exit failed, dev_num=%u is out of range\n",
			dev_num);
		return -1;
	}

	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		lpm_ipc_rx_unregister(dev_id);
	}

	lpm_log_info("lpm ipc receiver exit done, dev_num=%u\n", dev_num);
	return 0;
}

STATIC uint32_t lpm_ipc_find_notify_idx(enum lpm_fault_ipc_rx_type rx_type)
{
	uint32_t notify_idx;
	struct lpm_ipc_info *ipc_priv = lpm_ipc_priv_info();
	uint32_t chan_type = ipc_priv->channel_cfg.rx_chan_type[(uint32_t)rx_type];
	struct lpm_ipc_rx_channel_info *notify_cfg = &ipc_priv->channel_cfg.rx_ipc[chan_type];

	for (notify_idx = 0; notify_idx < notify_cfg->rx_type_cnt; notify_idx++) {
		if (notify_cfg->rx_type[notify_idx] == rx_type) {
			return notify_idx;
		}
	}

	return LPM_FAULT_IPC_MAX_NOTITY_NUM;
}

STATIC bool lpm_ipc_check_register_notify_param(
	uint32_t dev_id, enum lpm_fault_ipc_rx_type rx_type, fn_fault_lpm_ipc_notify fn_notify)
{
	uint32_t chan_type;
	uint32_t notify_idx;
	struct lpm_ipc_info *ipc_priv = lpm_ipc_priv_info();
	struct lpm_fault_ipc_rx_cfg *rx_cfg = NULL;

	if (!lpm_common_check_dev_id(dev_id)) {
		lpm_log_err("lpm ipc register check notify param failed, dev_id=%u is out of range, rx_type=%u\n",
			dev_id, (uint32_t)rx_type);
		return false;
	}

	if (rx_type >= LPM_IPC_RX_TYPE_MAX) {
		lpm_log_err("lpm ipc register check notify param failed, rx_type=%u is out of range\n",
			(uint32_t)rx_type);
		return false;
	}

	if (fn_notify == NULL) {
		lpm_log_err("lpm ipc register check rx_type=%u notify param failed, fn_notify is null\n",
			(uint32_t)rx_type);
		return false;
	}

	chan_type = ipc_priv->channel_cfg.rx_chan_type[(uint32_t)rx_type];
	rx_cfg = &ipc_priv->dev_data[dev_id].rx_ipc_cfg[chan_type];
	if (rx_cfg->notify_cnt >= LPM_FAULT_IPC_MAX_NOTITY_NUM) {
		lpm_log_err("lpm ipc register check chan_type=%u rx_type=%u notify param failed, notify_cnt=%u is full\n",
			chan_type, (uint32_t)rx_type, LPM_FAULT_IPC_MAX_NOTITY_NUM);
		return false;
	}

	notify_idx = lpm_ipc_find_notify_idx(rx_type);
	if (notify_idx == LPM_FAULT_IPC_MAX_NOTITY_NUM) {
		lpm_log_err("lpm ipc register check chan_type=%u notify param failed, not find rx_type=%u\n",
			chan_type, (uint32_t)rx_type);
		return false;
	}

	if (rx_cfg->fn_notify[notify_idx] != NULL) {
		lpm_log_err("lpm ipc register check chan_type=%u notify param failed, rx_type=%u is repeat\n",
			chan_type, (uint32_t)rx_type);
		return false;
	}

	return true;
}

// internal function interface, which needs to be called during initialization
// preemption scenarios are not considered here
int32_t lpm_ipc_register_rx_notify_func(
	uint32_t dev_id, enum lpm_fault_ipc_rx_type rx_type, fn_fault_lpm_ipc_notify fn_notify)
{
	uint32_t chan_type;
	uint32_t notify_idx;
	struct lpm_ipc_info *ipc_priv = lpm_ipc_priv_info();
	struct lpm_fault_ipc_rx_cfg *rx_cfg = NULL;

	if (!lpm_ipc_check_register_notify_param(dev_id, rx_type, fn_notify)) {
		lpm_log_err("lpm ipc register notify param check failed, rx_type=%u.\n",
			(uint32_t)rx_type);
		return -1;
	}

	chan_type = ipc_priv->channel_cfg.rx_chan_type[(uint32_t)rx_type];
	notify_idx = lpm_ipc_find_notify_idx(rx_type);
	rx_cfg = &ipc_priv->dev_data[dev_id].rx_ipc_cfg[chan_type];
	rx_cfg->fn_notify[notify_idx] = fn_notify;
	rx_cfg->notify_cnt++;

	lpm_log_info("lpm ipc register notify success, chan_type=%u, rx_type=%u, notify_cnt=%u\n",
		chan_type, (uint32_t)rx_type, rx_cfg->notify_cnt);
	return 0;
}

int32_t lpm_ipc_unregister_notify_func(uint32_t dev_id, enum lpm_fault_ipc_rx_type rx_type)
{
	uint32_t chan_type;
	uint32_t notify_idx;
	struct lpm_ipc_info *ipc_priv = lpm_ipc_priv_info();
	struct lpm_fault_ipc_rx_cfg *rx_cfg = NULL;

	if (!lpm_common_check_dev_id(dev_id)) {
		lpm_log_err("lpm ipc unregister check notify param failed, dev_id=%u is out of range, rx_type=%u\n",
			dev_id, (uint32_t)rx_type);
		return -1;
	}

	if (rx_type >= LPM_IPC_RX_TYPE_MAX) {
		lpm_log_err("lpm ipc unregister check notify param failed, rx_type=%u is out of range\n",
			(uint32_t)rx_type);
		return -1;
	}

	notify_idx = lpm_ipc_find_notify_idx(rx_type);
	if (notify_idx == LPM_FAULT_IPC_MAX_NOTITY_NUM) {
		lpm_log_info("lpm ipc unregister notify not found, rx_type=%u\n", (uint32_t)rx_type);
		return 0;
	}

	chan_type = ipc_priv->channel_cfg.rx_chan_type[(uint32_t)rx_type];

	rx_cfg = &ipc_priv->dev_data[dev_id].rx_ipc_cfg[chan_type];
	if (rx_cfg->fn_notify[notify_idx] != NULL) {
		rx_cfg->fn_notify[notify_idx] = NULL;
		rx_cfg->notify_cnt--;
	}

	lpm_log_info("lpm ipc unregister notify success, chan_type=%u, rx_type=%u, notify_cnt=%u\n",
		chan_type, (uint32_t)rx_type, rx_cfg->notify_cnt);
	return 0;
}
