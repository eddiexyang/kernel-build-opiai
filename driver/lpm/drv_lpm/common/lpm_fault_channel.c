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
#include <linux/of.h>
#include <linux/kernel.h>
#include <linux/notifier.h>
#include "lpm_fault_common.h"
#include "lpm_init.h"
#include "lpm_fault_ipc.h"
#include "lpm_fault_channel.h"

STATIC int32_t lpm_fault_send_rx_to_lp(uint32_t dev_num, struct lpm_fault_send_ipc_data *send_data)
{
#if defined(CFG_SOC_PLATFORM_MDC_V51)
	uint32_t dev_id;
	uint8_t out_data = 0;
	struct lpm_fault_ipc_msg msg_info = {
		.send_type = LPM_IPC_SET_FAULT_CHAN,
		.in        = (void *)send_data,
		.in_len    = (uint32_t)sizeof(struct lpm_fault_send_ipc_data),
		.out       = (void *)&out_data,
		.out_len   = (uint32_t)sizeof(out_data)
	};

	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		if (lpm_fault_ipc_send_sync_msg(dev_id, &msg_info) != 0) {
			lpm_log_err("lpm fault ipc send rx to lp failed, "
				"dev_id=%u, rx_fault_ipc=%u, rx_hb_ipc=%u\n",
				dev_id, send_data->rx_fault_ipc, send_data->rx_hb_ipc);
			return -1;
		}
		if (out_data != 0) {
			lpm_log_err("lpm fault ipc set rx to lp failed, "
				"dev_id=%u, out=%u, rx_fault_ipc=%u, rx_hb_ipc=%u\n",
				dev_id, out_data, send_data->rx_fault_ipc, send_data->rx_hb_ipc);
			return -1;
		}
	}
#else
	(void)dev_num;
	(void)send_data;
#endif
	return 0;
}

STATIC struct lpm_fault_ipc_tx_type_cfg *lpm_fault_get_tx_ipc_cfg(uint32_t *table_num)
{
	static struct lpm_fault_ipc_tx_type_cfg tx_ipc_cfg[(uint32_t)LPM_IPC_TX_TYPE_MAX] = {
		// ts_property_name, lpm_fault_ipc_tx_type, default chan_id, ipc_head_info
		{
#if defined(CFG_SOC_PLATFORM_MDC_V51)
			{"tx_fault_ipc", (uint8_t)LPM_IPC_SET_FAULT_CHAN, LPM_FAULT_IPC_TPROC_ID_1},
			{LPM_FAULT_SUB_CMD_SET_CHAN, LPM_FAULT_CMD_SETTING, LP_CPU_ID_FAULT, TAISHAN_CPU_ID_FAULT}
		}, {
#endif
			{"tx_hb_ipc", (uint8_t)LPM_IPC_NOTIFY_HB, LPM_FAULT_IPC_TPROC_ID_0},
			{LPM_FAULT_SUB_CMD_SET_HB, LPM_FAULT_CMD_INQUIRY, LP_CPU_ID_HB, TAISHAN_CPU_ID_HB}
		}
	};

	*table_num = (uint32_t)ARRAY_SIZE(tx_ipc_cfg);
	return &tx_ipc_cfg[0];
}

STATIC struct lpm_fault_ipc_type_cfg *lpm_fault_get_rx_ipc_cfg(uint32_t *table_num)
{
	static struct lpm_fault_ipc_type_cfg rx_ipc_cfg[(uint32_t)LPM_IPC_RX_TYPE_MAX] = {
		// ts_property_name, lpm_fault_ipc_rx_type, default chan_id, ipc_head_info
#if defined(CFG_SOC_PLATFORM_CLOUD) || defined(CFG_SOC_PLATFORM_MDC_V51) || \
	defined(CFG_SOC_PLATFORM_MINIV3) || defined(CFG_SOC_PLATFORM_CLOUD_V2)
		{"rx_fault_ipc", (uint8_t)LPM_IPC_RX_FAULT, LPM_FAULT_IPC_RPROC_ID_1},
#endif
		{"rx_hb_ipc",    (uint8_t)LPM_IPC_RX_HB,    LPM_FAULT_IPC_RPROC_ID_0}
	};

	*table_num = (uint32_t)ARRAY_SIZE(rx_ipc_cfg);
	return &rx_ipc_cfg[0];
}

#if defined(CFG_SOC_PLATFORM_MDC_V51)
STATIC int32_t lpm_fault_parse_channel(
	const struct device_node *np, const char *cfg_name, uint32_t *ipc_mail)
{
	uint32_t ipc_mailbox = 0;

	// read mailbox from dts
	if (of_property_read_u32(np, cfg_name, &ipc_mailbox) != 0) {
		lpm_log_err("read %s from dtsi failed\n", cfg_name);
		return -1;
	}
	*ipc_mail = ipc_mailbox;
	lpm_log_info("lpm fault parse %s from dts done, mailbox index=%u\n", cfg_name, ipc_mailbox);
	return 0;
}

STATIC int32_t lpm_fault_parse_dts(struct lpm_fault_ipc_channel *ipc_channel)
{
	int32_t ret;
	uint32_t i;
	uint32_t tx_table_num = 0;
	uint32_t rx_table_num = 0;
	struct lpm_fault_ipc_tx_type_cfg *tx_ipc_table = lpm_fault_get_tx_ipc_cfg(&tx_table_num);
	struct lpm_fault_ipc_type_cfg *rx_ipc_table = lpm_fault_get_rx_ipc_cfg(&rx_table_num);
	const struct device_node *np = of_find_compatible_node(NULL, NULL, MODULE_LPM_MATCH);

	if (np == NULL) {
		lpm_log_err("read dtsi failed, device node not found\n");
		return -1;
	}

	// inner function
	// tx_table_num equal to LPM_IPC_TX_TYPE_MAX, rx_table_num equal to LPM_IPC_RX_TYPE_MAX
	for (i = 0; i < tx_table_num; i++) {
		ret = lpm_fault_parse_channel(np, tx_ipc_table[i].ipc_cfg.cfg_name, &ipc_channel->tx_ipc[i]);
		if (ret != 0) {
			lpm_log_err("lpm fault parse %s from dts fail\n", tx_ipc_table[i].ipc_cfg.cfg_name);
			return -1;
		}
	}
	for (i = 0; i < rx_table_num; i++) {
		ret = lpm_fault_parse_channel(np, rx_ipc_table[i].cfg_name, &ipc_channel->rx_ipc[i]);
		if (ret != 0) {
			lpm_log_err("lpm fault parse %s from dts fail\n", rx_ipc_table[i].cfg_name);
			return -1;
		}
	}
	return 0;
}

STATIC int32_t lpm_fault_get_tx_chan_id(uint32_t tx_ipc, uint32_t *tx_chan)
{
	uint32_t i;
	uint32_t ipc_tx_chan_table[][0x2] = {
		// mailbox index, ipc channel id
		{8,  LPM_FAULT_IPC_TPROC_ID_1}, // for linux fault report
		{11, LPM_FAULT_IPC_TPROC_ID_2}, // for aos_core fault report and heartbeat
		{9,  LPM_FAULT_IPC_TPROC_ID_0}, // for linux heartbeat
	};
	uint32_t table_num = (uint32_t)ARRAY_SIZE(ipc_tx_chan_table);

	for (i = 0; i < table_num; ++i) {
		if (tx_ipc == ipc_tx_chan_table[i][0]) {
			*tx_chan = ipc_tx_chan_table[i][0x1];
			return 0;
		}
	}
	return -1;
}

STATIC int32_t lpm_fault_get_rx_chan_id(uint32_t rx_ipc, uint32_t *rx_chan)
{
	uint32_t i;
	uint32_t ipc_rx_chan_table[][0x2] = {
		// mailbox index, ipc channel id
		{2,  LPM_FAULT_IPC_RPROC_ID_1}, // for linux fault report
		{3,  LPM_FAULT_IPC_RPROC_ID_2}, // for aos_core fault report and heartbeat
		{1,  LPM_FAULT_IPC_RPROC_ID_0}, // for linux heartbeat
	};
	uint32_t table_num = (uint32_t)ARRAY_SIZE(ipc_rx_chan_table);

	for (i = 0; i < table_num; ++i) {
		if (rx_ipc == ipc_rx_chan_table[i][0]) {
			*rx_chan = ipc_rx_chan_table[i][0x1];
			return 0;
		}
	}
	return -1;
}

STATIC int32_t lpm_fault_trans_chan_id(
	struct lpm_fault_ipc_channel *ipc_chan_mail, struct lpm_fault_ipc_channel *ipc_chan_id)
{
	int32_t ret;
	uint32_t i;

	for (i = 0; i < (uint32_t)LPM_IPC_TX_TYPE_MAX; i++) {
		ret = lpm_fault_get_tx_chan_id(ipc_chan_mail->tx_ipc[i], &ipc_chan_id->tx_ipc[i]);
		if (ret != 0) {
			lpm_log_err("lpm ipc match tx_type=%u channel failed, mailbox=%u\n",
				i, ipc_chan_mail->tx_ipc[i]);
			return ret;
		}
		lpm_log_info("tx_type=%u channel %u mapped to tx_channel %u on table\n",
			i, ipc_chan_mail->tx_ipc[i], ipc_chan_id->tx_ipc[i]);
	}
	for (i = 0; i < (uint32_t)LPM_IPC_RX_TYPE_MAX; i++) {
		ret = lpm_fault_get_rx_chan_id(ipc_chan_mail->rx_ipc[i], &ipc_chan_id->rx_ipc[i]);
		if (ret != 0) {
			lpm_log_err("lpm ipc match rx_type=%u channel failed, mailbox=%u\n",
				i, ipc_chan_mail->rx_ipc[i]);
			return ret;
		}
		lpm_log_info("rx_type=%u channel %u mapped to rx_channel %u on table\n",
			i, ipc_chan_mail->rx_ipc[i], ipc_chan_id->rx_ipc[i]);
	}

	return 0;
}
#endif

STATIC int32_t lpm_fault_init_ipc_cfg(struct lpm_fault_ipc_channel *ipc_chan_id)
{
	uint32_t i;
	uint32_t tx_table_num = 0;
	uint32_t rx_table_num = 0;
	struct lpm_fault_ipc_tx_type_cfg *tx_ipc_table = lpm_fault_get_tx_ipc_cfg(&tx_table_num);
	struct lpm_fault_ipc_type_cfg *rx_ipc_table = lpm_fault_get_rx_ipc_cfg(&rx_table_num);

	if (tx_table_num != (uint32_t)LPM_IPC_TX_TYPE_MAX) {
		lpm_log_err("lpm channel init ipc config failed, tx_table_num=%u should equal to %u\n",
			tx_table_num, (uint32_t)LPM_IPC_TX_TYPE_MAX);
		return -1;
	}
	if (rx_table_num != (uint32_t)LPM_IPC_RX_TYPE_MAX) {
		lpm_log_err("lpm channel init ipc config failed, rx_table_num=%u should equal to %u\n",
			rx_table_num, (uint32_t)LPM_IPC_RX_TYPE_MAX);
		return -1;
	}

	for (i = 0; i < tx_table_num; i++) {
		ipc_chan_id->tx_ipc[i] = tx_ipc_table[i].ipc_cfg.chan_id;
	}
	for (i = 0; i < rx_table_num; i++) {
		ipc_chan_id->rx_ipc[i] = rx_ipc_table[i].chan_id;
	}
	return 0;
}

STATIC int32_t lpm_fault_set_ipc_channel(struct lpm_fault_ipc_channel *ipc_chan_id)
{
	int32_t ret;
	uint32_t i;
	uint32_t tx_table_num = 0;
	uint32_t rx_table_num = 0;
	struct lpm_fault_ipc_tx_type_cfg *tx_ipc_table = lpm_fault_get_tx_ipc_cfg(&tx_table_num);
	struct lpm_fault_ipc_type_cfg *rx_ipc_table = lpm_fault_get_rx_ipc_cfg(&rx_table_num);

	// inner function
	// this should use ipc_chan_id, chan_id in tx_ipc_table is default config
	// tx_table_num equal to LPM_IPC_TX_TYPE_MAX, rx_table_num equal to LPM_IPC_RX_TYPE_MAX
	for (i = 0; i < tx_table_num; i++) {
		ret = lpm_fault_ipc_set_tx_channel(
			(enum lpm_fault_ipc_tx_type)tx_ipc_table[i].ipc_cfg.module_type,
			ipc_chan_id->tx_ipc[i], &tx_ipc_table[i].ipc_head);
		if (ret != 0) {
			lpm_log_err("lpm channel set tx channel failed, ret=%d, i=%u, tx_type=%u, chan_id=%u\n",
				ret, i, tx_ipc_table[i].ipc_cfg.module_type, ipc_chan_id->tx_ipc[i]);
			return ret;
		}
	}
	for (i = 0; i < rx_table_num; i++) {
		ret = lpm_fault_ipc_set_rx_channel(
			(enum lpm_fault_ipc_rx_type)rx_ipc_table[i].module_type, ipc_chan_id->rx_ipc[i]);
		if (ret != 0) {
			lpm_log_err("lpm channel set rx channel failed, ret=%d, i=%u, rx_type=%u, chan_id=%u\n",
				ret, i, rx_ipc_table[i].module_type, ipc_chan_id->rx_ipc[i]);
			return ret;
		}
	}
	return 0;
}

STATIC int32_t lpm_fault_get_mailbox_from_dts(
	struct lpm_fault_ipc_channel *ipc_chan_id, struct lpm_fault_send_ipc_data *send_data)
{
#if defined(CFG_SOC_PLATFORM_MDC_V51)
	struct lpm_fault_ipc_channel ipc_chan_mail = {0};

	if (lpm_fault_parse_dts(&ipc_chan_mail) != 0) {
		lpm_log_err("lpm channel parse dts failed\n");
		return -1;
	}

	if (lpm_fault_trans_chan_id(&ipc_chan_mail, ipc_chan_id) != 0) {
		lpm_log_err("lpm channel map to txrx failed\n");
		return -1;
	}

	// send original ipc mailbox id to lp by ipc if needed
	send_data->rx_fault_ipc = (uint8_t)ipc_chan_mail.rx_ipc[(uint32_t)LPM_IPC_RX_FAULT];
	send_data->rx_hb_ipc = (uint8_t)ipc_chan_mail.rx_ipc[(uint32_t)LPM_IPC_RX_HB];
#else
	// no use
	send_data->rx_fault_ipc = 0;
	send_data->rx_hb_ipc = 0;
#endif
	return 0;
}

STATIC int32_t lpm_fault_get_channel(struct lpm_fault_send_ipc_data *send_data)
{
	int32_t ret;
	struct lpm_fault_ipc_channel ipc_chan_id = {0};

	// step1: set default ipc channel value
	ret = lpm_fault_init_ipc_cfg(&ipc_chan_id);
	if (ret != 0) {
		lpm_log_err("lpm channel init ipc config failed\n");
		return ret;
	}

	// step2: read ipc channel value from dts if needed
	ret = lpm_fault_get_mailbox_from_dts(&ipc_chan_id, send_data);
	if (ret != 0) {
		lpm_log_err("lpm channel read ipc channel info from dts failed\n");
		return -1;
	}

	// step4: save ipc channel id to ipc module
	ret = lpm_fault_set_ipc_channel(&ipc_chan_id);
	if (ret != 0) {
		lpm_log_err("lpm channel set ipc failed, ret=%d\n", ret);
		return ret;
	}

	return 0;
}

int32_t lpm_fault_channel_init(uint32_t dev_num)
{
	struct lpm_fault_send_ipc_data send_data = {0};

	if (dev_num > LPM_DMS_NODE_MAX_NUM) {
		lpm_log_err("lpm channel init failed, dev_num=%u is out of range\n",
			dev_num);
		return -1;
	}

	if (lpm_fault_get_channel(&send_data) != 0) {
		lpm_log_err("lpm fault get channel failed\n");
		return -1;
	}

	if (lpm_fault_send_rx_to_lp(dev_num, &send_data) != 0) {
		lpm_log_err("lpm fault channel send RX to LP failed\n");
		return -1;
	}

	lpm_log_info("lpm channel init success, dev_num=%u\n", dev_num);
	return 0;
}

int32_t lpm_fault_channel_exit(uint32_t dev_num)
{
	if (dev_num > LPM_DMS_NODE_MAX_NUM) {
		lpm_log_err("lpm fault channel exit failed, dev_num=%u is out of range\n",
			dev_num);
		return -1;
	}

	lpm_log_info("lpm fault channel exit done, dev_num=%u\n", dev_num);
	return 0;
}
