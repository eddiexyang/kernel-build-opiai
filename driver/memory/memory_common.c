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
*
* Description:
* Author: huawei
* Create: 2023-05-25
*/

#include <linux/delay.h>
#include <asm/uaccess.h>
#include <linux/time.h>
#include "devdrv_manager.h"
#include "dms_define.h"
#include "memory_log.h"
#include "memory_common.h"
#include "icm_interface.h"
#include "devdrv_manager_common.h"
#include "devdrv_user_common.h"
#include "devdrv_ipc.h"

int32_t msg_from_lp_para_check(uint32_t dev_id, struct devdrv_ipc_imu *ipc, struct devdrv_ipc_imu *msg_ret)
{
	if ((dev_id >= DEVDRV_MANGER_MAX_DEVICE_NUM) || (ipc == NULL) || (msg_ret == NULL)) {
		memory_drv_err("invalid device id. (id=%u; ipc=%d; msg_ret=%d)\n", dev_id, ipc != NULL, msg_ret != NULL);
		return -EINVAL;
	}

	return 0;
}

int32_t query_information_lp_module(uint32_t dev_id, struct devdrv_ipc_imu *ipc, struct devdrv_ipc_imu *msg_ret)
{
	int32_t ret = 0;

	ret = icm_msg_send_sync(ICM_FD_BUILD(dev_id, HISI_RPROC_LP_Q_TX_RPID4_ACPU0),
		(rproc_msg_t *)ipc, sizeof(struct devdrv_ipc_imu) / sizeof(rproc_msg_t),
		(rproc_msg_t *)msg_ret, IPCDRV_RPROC_MSG_LENGTH);
	if (ret) {
		memory_drv_err("icm_msg_send_sync failed, ret=%d, ipc.cmd_type0 = %u, ipc.cmd_type1 = %u\n",
			ret, ipc->cmd_type0, ipc->cmd_type1);
		return ret;
	}

	return ret;
}

void drv_memory_show_logs(struct devdrv_ipc_imu *ipc, struct devdrv_ipc_imu *msg_ret)
{
	memory_drv_err("get invalid ipc message. "
		"ack.cmd_type0 = %u, ipc.cmd_type0 = %u, "
		"ack.cmd_type1 = %u, ipc.cmd_type1 = %u.\n",
		msg_ret->cmd_type0, ipc->cmd_type0, msg_ret->cmd_type1, ipc->cmd_type1);
	memory_drv_err("ack.cmd_type1(%u), ack.cmd_type0(%u), ack.target_id(%u), ack.source_id(%d), "
		"ack.cmd_para0(%u), ack.cmd_para1(%u), ack.cmd_para2(%u), ack.cmd_para3(%u), "
		"ack.cmd_data0(%u), ack.cmd_data1(%u), ack.cmd_data2(%u), ack.cmd_data3(%u), "
		"ack.data1(%u), ack.data2(%u), ack.data3(%u), ack.data4(%u), ack.data5(%u).\n",
		msg_ret->cmd_type1, msg_ret->cmd_type0, msg_ret->target_id, msg_ret->source_id,
		msg_ret->cmd_para0, msg_ret->cmd_para1, msg_ret->cmd_para2, msg_ret->cmd_para3,
		msg_ret->cmd_data0, msg_ret->cmd_data1, msg_ret->cmd_data2, msg_ret->cmd_data3,
		msg_ret->data1, msg_ret->data2, msg_ret->data3, msg_ret->data4, msg_ret->data5);
}

int32_t memory_dev_get_msg_from_lp(uint32_t dev_id, struct devdrv_ipc_imu *ipc, struct devdrv_ipc_imu *msg_ret,
	uint32_t msg_len)
{
	int32_t ret = 0;

	ret = msg_from_lp_para_check(dev_id, ipc, msg_ret);
	if (ret != 0) {
		return ret;
	}

	ret = query_information_lp_module(dev_id, ipc, msg_ret);
	if (ret != 0) {
		return ret;
	}

	if ((msg_ret->cmd_type0 != ipc->cmd_type0) || (msg_ret->cmd_type1 != ipc->cmd_type1)) {
// Query information from the LP module again when the cmd_types are inconsistent.
#if defined (CFG_SOC_PLATFORM_MDC_V51) || defined (CFG_SOC_PLATFORM_MDC_V51LITE)
		ret = query_information_lp_module(dev_id, ipc, msg_ret);
		if (ret != 0) {
			return ret;
		}
		if ((msg_ret->cmd_type0 == ipc->cmd_type0) && (msg_ret->cmd_type1 == ipc->cmd_type1)) {
			return ret;
		}
#endif
		drv_memory_show_logs(ipc, msg_ret);
		return -EINVAL;
	}

	return ret;
}

int32_t memory_dev_check_lp_msg_validity(struct devdrv_ipc_imu *lp_msg)
{
	uint32_t lp_error_code;

	/*
	LP_ERRCODE_QUERY_SUCCESS			0
	LP_ERRCODE_QUERY_FAIL				1
	LP_ERRCODE_QUERY_TIMEOUT			2
	LP_ERRCODE_QUERY_PARA_ERR			3
	LP_ERRCODE_QUERY_NONSUPPORT			4
	32-bit error code. The first 16 bits indicate the voltage error code and the last 16 bits
	indicate the current error code.
	*/
	lp_error_code = *(uint32_t *)(&(lp_msg->cmd_para0));
	if (lp_error_code != 0) {
		memory_drv_err("get low_power module message failed. ack.cmd_type0 = %u, ack.cmd_type1 = %u"
			"lp_err_code = 0x%x\n", lp_msg->cmd_type0, lp_msg->cmd_type1, lp_error_code);
		return -EINVAL;
	}

	return 0;
}
