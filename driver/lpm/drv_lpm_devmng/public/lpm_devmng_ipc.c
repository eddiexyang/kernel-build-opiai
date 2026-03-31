/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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

#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/time64.h>
#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
#include "lpm_devmng_debugfs.h"
#endif
#include "lpm_devmng_common.h"
#include "lpm_devmng_ipc.h"

STATIC struct lpm_devmng_ipc_priv *lpm_ipc_get_priv_info(void)
{
	static struct lpm_devmng_ipc_priv lpm_ipc_priv_info = {0};
	return &lpm_ipc_priv_info;
}

STATIC struct lpm_devmng_ipc_tx_cfg *lpm_ipc_get_tx_table(enum lpm_devmng_ipc_send_type send_type)
{
	struct lpm_devmng_ipc_priv *ipc_priv = lpm_ipc_get_priv_info();
	return &ipc_priv->tx_cfg[(uint32_t)send_type];
}

#ifdef CFG_SOC_PLATFORM_MINIV3
STATIC struct lpm_devmng_ipc_tx_cfg *lpm_ipc_get_channel_cfg_table(void)
{
	static struct lpm_devmng_ipc_tx_cfg lpm_ipc_cfg[(uint32_t)LPM_IPC_SEND_TYPE_MAX] = {
		{
			LPM_IPC_TX_LP_ACPU0, 0,
			{ ICM_SUB_CMD_IDLE, ICM_MAIN_CMD_LP_SET, LPM_DEV_OBJ_LP, LPM_DEV_OBJ_TAISHAN0 },
			{ NULL }
		}, {
			LPM_IPC_TX_LP_ACPU0, 0,
			{ ICM_SUB_CMD_IDLE, ICM_MAIN_CMD_LP_NOTIFY, LPM_DEV_OBJ_LP, LPM_DEV_OBJ_TAISHAN0 },
			{ NULL }
		}, {
			LPM_IPC_TX_LP_ACPU0, 0,
			{ ICM_SUB_CMD_PROFILE_LIMIT, ICM_MAIN_CMD_LP_SET, LPM_DEV_OBJ_LP, LPM_DEV_OBJ_TAISHAN0 },
			{ NULL }
		}, {
			LPM_IPC_TX_LP_ACPU0, 0,
			{ ICM_SUB_CMD_SUSPEND, ICM_MAIN_CMD_LP_NOTIFY, LPM_DEV_OBJ_LP, LPM_DEV_OBJ_TAISHAN0 },
			{ NULL }
		}, {
			LPM_IPC_TX_LP_ACPU0, 0,
			{ ICM_SUB_CMD_RESUME, ICM_MAIN_CMD_LP_NOTIFY, LPM_DEV_OBJ_LP, LPM_DEV_OBJ_TAISHAN0 },
			{ NULL }
		}, {
			LPM_IPC_TX_LP_ACPU0, 0,
			{ ICM_SUB_CMD_SET_VRD_INFO, ICM_MAIN_CMD_LP_NOTIFY, LPM_DEV_OBJ_LP, LPM_DEV_OBJ_TAISHAN0 },
			{ NULL }
		}, {
			LPM_IPC_TX_LP_ACPU0, 0,
			{ ICM_SUB_CMD_QUERY_LP_ACG, ICM_MAIN_CMD_LP_REQ, LPM_DEV_OBJ_LP, LPM_DEV_OBJ_TAISHAN0 },
			{ NULL }
		}, {
			LPM_IPC_TX_LP_ACPU0, 0,
			{ ICM_SUB_CMD_SMOKE, ICM_MAIN_CMD_LP_REQ, LPM_DEV_OBJ_LP, LPM_DEV_OBJ_TAISHAN0 },
			{ NULL }
		}
	};
	return &lpm_ipc_cfg[0];
}

#elif defined(CFG_SOC_PLATFORM_CLOUD_V2)
STATIC struct lpm_devmng_ipc_tx_cfg *lpm_ipc_get_channel_cfg_table(void)
{
	static struct lpm_devmng_ipc_tx_cfg lpm_ipc_cfg[(uint32_t)LPM_IPC_SEND_TYPE_MAX] = {
		{
			LPM_IPC_TX_LP_ACPU0, 0,
			{ ICM_SUB_CMD_IDLE, ICM_MAIN_CMD_LP_SET, LPM_DEV_OBJ_LP, LPM_DEV_OBJ_TAISHAN0 },
			{ NULL }
		}, {
			LPM_IPC_TX_LP_ACPU0, 0,
			{ ICM_SUB_CMD_SMOKE, ICM_MAIN_CMD_LP_REQ, LPM_DEV_OBJ_LP, LPM_DEV_OBJ_TAISHAN0 },
			{ NULL }
		}, {
			LPM_IPC_TX_LP_ACPU0, 0,
			{ ICM_SUB_CMD_TURBO_ONOFF, ICM_MAIN_CMD_LP_SET, LPM_DEV_OBJ_LP, LPM_DEV_OBJ_TAISHAN0 },
			{ NULL }
		}, {
			LPM_IPC_TX_LP_ACPU0, 0,
			{ ICM_SUB_CMD_AIC_FREQ, ICM_MAIN_CMD_LP_SET, LPM_DEV_OBJ_LP, LPM_DEV_OBJ_TAISHAN0 },
			{ NULL }
		}, {
			LPM_IPC_TX_LP_ACPU0, 0,
			{ ICM_SUB_CMD_LIMIT, ICM_MAIN_CMD_LP_SET, LPM_DEV_OBJ_LP, LPM_DEV_OBJ_TAISHAN0 },
			{ NULL }
		}, {
			LPM_IPC_TX_LP_ACPU0, 0,
			{ ICM_SUB_CMD_QUERY_LP_ACG, ICM_MAIN_CMD_LP_NOTIFY, LPM_DEV_OBJ_LP, LPM_DEV_OBJ_TAISHAN0 },
			{ NULL }
		}
	};
	return &lpm_ipc_cfg[0];
}

#elif defined(CFG_SOC_PLATFORM_MDC_V51)
STATIC bool lpm_ipc_check_ack_head(
	uint32_t dev_id, enum lpm_devmng_ipc_send_type send_type, struct lpm_devmng_ipc_ack_msg *ack_msg)
{
	struct lpm_devmng_ipc_tx_cfg *ipc_cfg = lpm_ipc_get_tx_table(send_type);

	if (ipc_cfg->head.sub_cmd != ack_msg->sub_cmd) {
		lpm_log_err("ipc ack message sub_cmd error, dev_id=%u, send_type=%u, "
			"expect sub_cmd=%u, ack sub_cmd=%u\n",
			dev_id, (uint32_t)send_type, ipc_cfg->head.sub_cmd, ack_msg->sub_cmd);
		return false;
	}

	if (ipc_cfg->head.cmd != ack_msg->cmd) {
		lpm_log_err(" ipc ack message cmd error, dev_id=%u, send_type=%u, "
			"expect cmd=%u, ack cmd=%u\n",
			dev_id, (uint32_t)send_type, ipc_cfg->head.cmd, ack_msg->cmd);
		return false;
	}

	return true;
}

STATIC bool lpm_ipc_check_ack_result(
	uint32_t dev_id, enum lpm_devmng_ipc_send_type send_type, struct lpm_devmng_ipc_ack_msg *ack_msg)
{
	uint32_t ipc_result;

	// for result
	if (!lpm_ipc_check_ack_head(dev_id, send_type, ack_msg)) {
		lpm_log_err(" ipc check ack head fail, dev_id=%u, send_type=%u\n", dev_id, (uint32_t)send_type);
		return false;
	}

	ipc_result = *(uint32_t *)&(ack_msg->data[0]);
	if (ipc_result != 0) {
		lpm_log_err(" ipc check ipc result fail, dev_id=%u, send_type=%u, ipc_result=%u\n",
			dev_id, (uint32_t)send_type, ipc_result);
		return false;
	}

	return true;
}

STATIC struct lpm_devmng_ipc_tx_cfg *lpm_ipc_get_channel_cfg_table(void)
{
	static struct lpm_devmng_ipc_tx_cfg lpm_ipc_cfg[(uint32_t)LPM_IPC_SEND_TYPE_MAX] = {
		// 0:chan_id, 1:retry_times, 2:head(sub_cmd, cmd, cmdObj, cmdSrc), 3:fn_hook(fn_check_ack)
		{
			LPM_IPC_TX_LP_ACPU0, 1,
			{ LPM_DEV_SUB_CMD_QUERY_SOC_TEMP, LPM_DEV_CMD_INQUIRY, LPM_DEV_OBJ_LP, LPM_DEV_OBJ_TAISHAN0 },
			{ lpm_ipc_check_ack_result }
		}, {
			LPM_IPC_TX_LP_ACPU0, 0,
			{ LPM_DEV_SUB_CMD_QUERY_DDR_FREQ,     LPM_DEV_CMD_INQUIRY, LPM_DEV_OBJ_LP, LPM_DEV_OBJ_TAISHAN0 },
			{ lpm_ipc_check_ack_result }
		}, {
			LPM_IPC_TX_LP_ACPU0, 0,
			{ LPM_DEV_SUB_CMD_QUERY_CTRLCPU_FREQ, LPM_DEV_CMD_INQUIRY, LPM_DEV_OBJ_LP, LPM_DEV_OBJ_TAISHAN0 },
			{ lpm_ipc_check_ack_result }
		}, {
			LPM_IPC_TX_LP_ACPU0, 0,
			{ LPM_DEV_SUB_CMD_QUERY_AICORE_FREQ,  LPM_DEV_CMD_INQUIRY, LPM_DEV_OBJ_LP, LPM_DEV_OBJ_TAISHAN0 },
			{ lpm_ipc_check_ack_result }
		}, {
			LPM_IPC_TX_LP_ACPU0, 0,
			{ LPM_DEV_SUB_CMD_QUERY_VECTOR_FREQ,  LPM_DEV_CMD_INQUIRY, LPM_DEV_OBJ_LP, LPM_DEV_OBJ_TAISHAN0 },
			{ lpm_ipc_check_ack_result }
		}
	};
	return &lpm_ipc_cfg[0];
}
#elif defined(CFG_SOC_PLATFORM_MDC_V51_LITE)
STATIC struct lpm_devmng_ipc_tx_cfg *lpm_ipc_get_channel_cfg_table(void)
{
	static struct lpm_devmng_ipc_tx_cfg lpm_ipc_cfg[(uint32_t)LPM_IPC_SEND_TYPE_MAX] = {
		{
			LPM_IPC_TX_LP_ACPU0, 0,
			{ ICM_SUB_CMD_SMOKE, ICM_MAIN_CMD_LP_REQ, LPM_DEV_OBJ_LP, LPM_DEV_OBJ_TAISHAN0 },
			{ NULL }
		},
	};
	return &lpm_ipc_cfg[0];
}
#endif

STATIC int32_t lpm_ipc_init_priv_data(void)
{
	struct lpm_devmng_ipc_priv *ipc_priv = lpm_ipc_get_priv_info();

	ipc_priv->tx_cfg = lpm_ipc_get_channel_cfg_table();
	return 0;
}

STATIC int32_t lpm_ipc_fill_send_msg(
	struct lpm_devmng_ipc_msg *msg_info, struct lpm_devmng_ipc_send_msg *send_msg)
{
	int32_t ret;
	struct lpm_devmng_ipc_tx_cfg *ipc_cfg = lpm_ipc_get_tx_table(msg_info->send_type);

	// inner function, no need check param
	// fill ipc send msg and return chan_id
	send_msg->sub_cmd  = ipc_cfg->head.sub_cmd;
	send_msg->cmd      = ipc_cfg->head.cmd;
	send_msg->cmd_dest = ipc_cfg->head.cmd_dest;
	send_msg->cmd_src  = ipc_cfg->head.cmd_src;
#if defined(LPM_IPC_V3)
	send_msg->len      = (uint8_t)msg_info->in_len;
#endif

	if (msg_info->in_len == 0) {
		return 0;
	}

	ret = memcpy_s(send_msg->data, sizeof(send_msg->data), msg_info->in, msg_info->in_len);
	if (ret != 0) {
		lpm_log_err("ipc param check, copy in msg failed, ret=%d, in_len=%u, send_type=%u\n",
			ret, msg_info->in_len, (uint32_t)msg_info->send_type);
		return -EIO;
	}

	return 0;
}

STATIC bool lpm_ipc_check_in_param(struct lpm_devmng_ipc_msg *msg_info)
{
	if (msg_info->send_type >= LPM_IPC_SEND_TYPE_MAX) {
		lpm_log_err("ipc param check, send_type=%u should less than %u\n",
			(uint32_t)msg_info->send_type, (uint32_t)LPM_IPC_SEND_TYPE_MAX);
		return false;
	}

	if (msg_info->in_len > LPM_IPC_DATA_SIZE) {
		lpm_log_err("ipc param check, input in_len is wrong, in_len=%u, max_in_len=%u\n",
			msg_info->in_len, LPM_IPC_DATA_SIZE);
		return false;
	}

	if ((msg_info->in == NULL) && (msg_info->in_len != 0)) {
		lpm_log_err("ipc param check, input is null, but in_len not zero\n");
		return false;
	}

	if ((msg_info->in != NULL) && (msg_info->in_len == 0)) {
		lpm_log_err("ipc param check, input not null, but in_len is zero\n");
		return false;
	}
	return true;
}

STATIC bool lpm_ipc_check_out_param(struct lpm_devmng_ipc_msg *msg_info)
{
	if (msg_info->out == NULL) {
		lpm_log_err("ipc param check, output char is NULL\n");
		return false;
	}
	if ((msg_info->out_len == 0) || (msg_info->out_len > LPM_IPC_DATA_SIZE)) {
		lpm_log_err("ipc param check, out_len is wrong, out_len=%u, max_out_len=%u\n",
			msg_info->out_len, LPM_IPC_DATA_SIZE);
		return false;
	}
	if (msg_info->out_pos >= LPM_IPC_DATA_SIZE) {
		lpm_log_err("ipc param check, out_pos is wrong, out_pos=%u\n", msg_info->out_pos);
		return false;
	}
	if ((msg_info->out_len + msg_info->out_pos) > LPM_IPC_DATA_SIZE) {
		lpm_log_err("ipc param check, out len plus out pos is out of range, "
			"out_len=%u, out_pos=%u, max_out_len=%u\n",
			msg_info->out_len, msg_info->out_pos, LPM_IPC_DATA_SIZE);
		return false;
	}
	return true;
}

STATIC int32_t lpm_ipc_sync_send_with_ack(
	uint32_t dev_id, struct lpm_devmng_ipc_msg *msg_info,
	struct lpm_devmng_ipc_send_msg *send_msg, bool *succ_flag)
{
	int32_t ret;
	struct lpm_devmng_ipc_ack_msg ack_msg = {0};
	struct lpm_devmng_ipc_tx_cfg *ipc_cfg = lpm_ipc_get_tx_table(msg_info->send_type);

	*succ_flag = false;
	ret = icm_msg_send_sync(ICM_FD_BUILD(dev_id, ipc_cfg->chan_id),
		(rproc_msg_t *)send_msg, (uint32_t)(sizeof(*send_msg) / sizeof(rproc_msg_t)),
		(rproc_msg_t *)&ack_msg, (uint32_t)(sizeof(ack_msg) / sizeof(rproc_msg_t)));
	if (ret != 0) {
		lpm_log_err("failed to send sync ipc message, dev_id=%u, chan_id=%u, send_type=%u, ret=%d\n",
			dev_id, ipc_cfg->chan_id, (uint32_t)msg_info->send_type, ret);
		return ret;
	}

	if (ipc_cfg->fn_hook.fn_check_ack != NULL) {
		if (!ipc_cfg->fn_hook.fn_check_ack(dev_id, msg_info->send_type, &ack_msg)) {
			return 0;
		}
	}

	// only need to copy the specified part
	ret = memcpy_s(msg_info->out, msg_info->out_len, &ack_msg.data[msg_info->out_pos], msg_info->out_len);
	if (ret != 0) {
		lpm_log_err("send sync ipc, copy ack msg failed, ret=%d, "
			"dev_id=%u, chan_id=%u, out_len=%u, out_pos=%u, send_type=%u\n",
			ret, dev_id, ipc_cfg->chan_id, msg_info->out_len, msg_info->out_pos, (uint32_t)msg_info->send_type);
		return -EIO;
	}

	*succ_flag = true;
	return 0;
}

STATIC int32_t lpm_ipc_sync_send_with_retry(
	uint32_t dev_id, struct lpm_devmng_ipc_msg *msg_info)
{
	int32_t ret;
	uint32_t retry_cnt;
	bool succ_flag = false;
	struct lpm_devmng_ipc_send_msg send_msg = {0};
	struct lpm_devmng_ipc_tx_cfg *ipc_cfg = lpm_ipc_get_tx_table(msg_info->send_type);

	// fill ipc head and ipc message
	ret = lpm_ipc_fill_send_msg(msg_info, &send_msg);
	if (ret != 0) {
		lpm_log_err("send sync ipc: fill send message failed, ret=%d\n", ret);
		return ret;
	}

	// send ipc message with retry, and get ack message
	for (retry_cnt = 0; retry_cnt <= ipc_cfg->retry_times; retry_cnt++) {
		ret = lpm_ipc_sync_send_with_ack(dev_id, msg_info, &send_msg, &succ_flag);
		if (ret != 0) {
			lpm_log_err("failed to send sync ipc message for the %uth time, send_type=%u, ret=%d\n",
				retry_cnt, (uint32_t)msg_info->send_type, ret);
			return ret;
		}
		if (succ_flag) {
			return 0;
		}
	}

	lpm_log_err("failed to send sync ipc message after %u attempts, send_type=%u, ret=%d\n",
		ipc_cfg->retry_times, (uint32_t)msg_info->send_type, ret);
	return -ETIMEDOUT;
}

int32_t lpm_ipc_send_sync_msg(
	uint32_t dev_id, struct lpm_devmng_ipc_msg *msg_info)
{
	int32_t ret;

	// check param
	if (!lpm_ipc_check_in_param(msg_info)) {
		lpm_log_err("send sync ipc failed, in param invalid\n");
		return -EINVAL;
	}

	if (!lpm_ipc_check_out_param(msg_info)) {
		lpm_log_err("send sync ipc failed, out param invalid\n");
		return -EINVAL;
	}

	// send ipc message
	ret = lpm_ipc_sync_send_with_retry(dev_id, msg_info);
	if (ret != 0) {
		lpm_log_err("send sync ipc with retry failed, ret=%d\n", ret);
		return ret;
	}

	return 0;
}

int32_t lpm_ipc_send_async_msg(
	uint32_t dev_id, struct lpm_devmng_ipc_msg *msg_info)
{
	int32_t ret;
	struct lpm_devmng_ipc_send_msg send_msg = {0};
	struct lpm_devmng_ipc_tx_cfg *ipc_cfg = NULL;

	// check param
	if (!lpm_ipc_check_in_param(msg_info)) {
		lpm_log_err("send async ipc failed, in param invalid\n");
		return -EINVAL;
	}

	// fill ipc head and ipc message
	ret = lpm_ipc_fill_send_msg(msg_info, &send_msg);
	if (ret != 0) {
		lpm_log_err("send async ipc: fill send message failed, ret=%d\n", ret);
		return ret;
	}

	// send ipc message
	ipc_cfg = lpm_ipc_get_tx_table(msg_info->send_type);
	ret = icm_msg_send_async(ICM_FD_BUILD(dev_id, ipc_cfg->chan_id),
		(rproc_msg_t *)&send_msg, (uint32_t)(sizeof(send_msg) / sizeof(rproc_msg_t)));
	if (ret != 0) {
		lpm_log_err("send async ipc failed, send_type=%u, ret=%d\n",
			(uint32_t)msg_info->send_type, ret);
		return ret;
	}

	return 0;
}

int32_t lpm_ipc_probe(uint64_t *param, uint32_t param_num)
{
	int32_t ret;
	uint32_t dev_num = lpm_common_get_dev_num();

	ret = lpm_ipc_init_priv_data();
	if (ret != 0) {
		lpm_log_err("lpm ipc init priv data failed, ret=%d\n", ret);
		return ret;
	}

	(void)param;
	(void)param_num;
	lpm_log_info("lpm ipc probe success, dev_num=%u\n", dev_num);
	return 0;
}

int32_t lpm_ipc_remove(uint64_t *param, uint32_t param_num)
{
	uint32_t dev_num = lpm_common_get_dev_num();

	(void)param;
	(void)param_num;
	lpm_log_info("lpm ipc remove success, dev_num=%u\n", dev_num);
	return 0;
}
