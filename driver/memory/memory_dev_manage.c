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
#include <linux/uaccess.h>
#include <asm/uaccess.h>
#include <linux/time.h>
#include "devdrv_manager.h"
#include "devdrv_manager_common.h"
#include "dms_define.h"
#include "memory_log.h"
#include "memory_common.h"
#include "memory_ddr.h"
#include "devdrv_ipc.h"
#include "icm_interface.h"
#include "memory_dev_manage.h"
#include "devdrv_user_common.h"

#if defined(CFG_FEATURE_LP_ENABLE)
// DDR INFO第一次查询后缓存，后续查询直接读取缓存，不再发IPC从小核查询
static bool g_ddr_manu_info_query_flag = false;
static bool g_ddr_base_info_query_flag = false;
static struct devdrv_ddr_base_info g_ddr_base_info_buffer = {0};
static unsigned char g_ddr_manufactures_code_buffer[DDR_MANUFACTURE_CODE_LEN] = {0};

STATIC int32_t parse_info_out_size_by_cmd(uint32_t cmd, uint32_t *out_size)
{
	*out_size = sizeof(uint32_t);
	switch (cmd) {
		case DMS_SUBCMD_MEMORY_TYPE:
		case DMS_SUBCMD_MEMORY_CAPACITY:
		case DMS_SUBCMD_MEMORY_CHANNEL:
		case DMS_SUBCMD_MEMORY_RANK_NUM:
		case DMS_SUBCMD_MEMORY_ECC_ENABLE:
			break;
		case DMS_SUBCMD_MEMORY_MANUFACTURES:
			*out_size = sizeof(unsigned char) * DDR_MAX_CHAN_NUM;
			break;
		default:
			memory_drv_err("Not support command. (cmd=%d)", cmd);
			return -EOPNOTSUPP;
	}

	return 0;
}

STATIC int32_t get_ddr_base_info_from_buffer(uint32_t cmd, void *buff)
{
	switch (cmd) {
		case DMS_SUBCMD_MEMORY_TYPE:
			*(uint32_t *)buff = g_ddr_base_info_buffer.ddr_type;
			break;
		case DMS_SUBCMD_MEMORY_CAPACITY:
			*(uint32_t *)buff = g_ddr_base_info_buffer.ddr_capacity;
			break;
		case DMS_SUBCMD_MEMORY_CHANNEL:
			*(uint32_t *)buff = g_ddr_base_info_buffer.ddr_channel;
			break;
		case DMS_SUBCMD_MEMORY_RANK_NUM:
			*(uint32_t *)buff = g_ddr_base_info_buffer.ddr_rank_num;
			break;
		case DMS_SUBCMD_MEMORY_ECC_ENABLE:
			*(uint32_t *)buff = g_ddr_base_info_buffer.ddr_ecc_enable;
			break;
		default:
			memory_drv_err("Not support command. (cmd=%d)", cmd);
			return -EOPNOTSUPP;
	}

	return 0;
}

STATIC int32_t get_ddr_base_info_result(uint32_t cmd, void *buff, struct devdrv_ipc_imu *ack_msg)
{
	g_ddr_base_info_buffer.ddr_type = *((uint32_t *)&ack_msg->cmd_data0);
	g_ddr_base_info_buffer.ddr_capacity = ack_msg->data1;
	g_ddr_base_info_buffer.ddr_channel = ack_msg->data2;
	g_ddr_base_info_buffer.ddr_rank_num = ack_msg->data3;
	g_ddr_base_info_buffer.ddr_ecc_enable = ack_msg->data4;

	return get_ddr_base_info_from_buffer(cmd, buff);
}

STATIC void get_ddr_manu_info_from_buffer(void *buff, uint32_t size)
{
	uint32_t i;

	for (i = 0; i < size; i++) {
		*(unsigned char *)buff = g_ddr_manufactures_code_buffer[i];
		buff++;
	}
}

STATIC int32_t get_ddr_manu_info_result(void *buff, uint32_t size, struct devdrv_ipc_imu *ack_msg)
{
	int32_t ret;

	ret = memcpy_s(g_ddr_manufactures_code_buffer, sizeof(g_ddr_manufactures_code_buffer), &ack_msg->cmd_data0,
		sizeof(struct devdrv_ipc_imu) - offsetof(struct devdrv_ipc_imu, cmd_data0));
	if (ret != 0) {
		memory_drv_err("memcpy_s failed. ret = %d\n", ret);
		return ret;
	}

	get_ddr_manu_info_from_buffer(buff, size);

	return 0;
}

STATIC uint8_t get_ipc_cmd_type(uint32_t sub_cmd)
{
	uint8_t cmd_type;
	if (sub_cmd == DMS_SUBCMD_MEMORY_MANUFACTURES) {
		cmd_type = SUB_CMD_QUERY_DDR_MANUFACTURES_INFO;
	} else {
		cmd_type = SUB_CMD_QUERY_DDR_BASE_INFO;
	}

	return cmd_type;
}

STATIC void memory_free(char **buff)
{
	if (*buff != NULL) {
		kfree(*buff);
		*buff = NULL;
	}
}

STATIC int32_t memory_query_from_lp(uint32_t dev_id, struct devdrv_ipc_imu *ipc_msg, struct devdrv_ipc_imu *ack_msg,
	char **buff)
{
	int32_t ret = 0;

	ret = memory_dev_get_msg_from_lp(dev_id, ipc_msg, ack_msg, sizeof(uint32_t));
	if (ret != 0) {
		memory_drv_err("drv_memory_get_msg_from_lp failed, ret = %d.\n", ret);
		memory_free(buff);
		return ret;
	}

	ret = memory_dev_check_lp_msg_validity(ack_msg);
	if (ret != 0) {
		memory_drv_err("The message obtained from the lp is invalid. please check lp_err_code.\n");
		memory_free(buff);
		return ret;
	}

	return ret;
}

STATIC int32_t ipc_info_save_to_buff(uint32_t sub_cmd, char **buff, uint32_t out_len, struct devdrv_ipc_imu *ack_msg)
{
	int32_t ret = 0;

	if (sub_cmd == DMS_SUBCMD_MEMORY_MANUFACTURES) {
		ret = get_ddr_manu_info_result((void *)(*buff), out_len, ack_msg);
		if (ret != 0) {
			memory_drv_err("get ddr manu info error. (ret=%d)", ret);
			memory_free(buff);
			return ret;
		}
		g_ddr_manu_info_query_flag = true;
	} else {
		ret = get_ddr_base_info_result(sub_cmd, (void *)(*buff), ack_msg);
		if (ret != 0) {
			memory_drv_err("get ddr base info error. (ret=%d)", ret);
			memory_free(buff);
			return ret;
		}
		g_ddr_base_info_query_flag = true;
	}

	return ret;
}

STATIC int32_t copy_data_to_usr(char **in, char **buff, uint32_t out_len)
{
	int32_t ret = 0;
#ifndef DRV_MEMORY_GTEST
	ret = copy_to_user(*in, *buff, out_len);
	if (ret != 0)
		memory_drv_err("Failed to invoke copy_from_user. (size=%u)\n", out_len);
#endif
	memory_free(buff);

	return ret;
}

STATIC int32_t query_memory_info_from_buff(uint32_t sub_cmd, char **in, char **buff, uint32_t out_len)
{
	int32_t ret = 0;
#ifndef DRV_MEMORY_GTEST
	if (sub_cmd == DMS_SUBCMD_MEMORY_MANUFACTURES) {
		get_ddr_manu_info_from_buffer((void *)(*buff), out_len);
		return copy_data_to_usr(in, buff, out_len);
	} else {
		ret = get_ddr_base_info_from_buffer(sub_cmd, (void *)(*buff));
		if (ret != 0) {
			memory_drv_err("get_ddr_base_info_from_buffer error. (ret=%d)", ret);
			memory_free(buff);
			return ret;
		}
		return copy_data_to_usr(in, buff, out_len);
	}
#endif
	return ret;
}

STATIC bool is_query_from_buff(uint32_t sub_cmd)
{
	return (g_ddr_manu_info_query_flag && (sub_cmd == DMS_SUBCMD_MEMORY_MANUFACTURES)) ||
		(g_ddr_base_info_query_flag && (sub_cmd != DMS_SUBCMD_MEMORY_MANUFACTURES));
}

STATIC int32_t query_info_preprocess(uint32_t sub_cmd, uint32_t **out_len, char **buff)
{
	int32_t ret = 0;

	ret = parse_info_out_size_by_cmd(sub_cmd, *out_len);
	if (ret != 0) {
		memory_drv_err("Parse cmd error. (ret=%d)", ret);
		return ret;
	}
#ifndef DRV_MEMORY_GTEST
	*buff = kzalloc(*(*out_len), GFP_KERNEL | __GFP_ACCOUNT);
	if (*buff == NULL) {
		memory_drv_err("Buffer kzalloc failed.\n");
		return -ENOMEM;
	}
#endif
	return ret;
}

int32_t memory_get_ddr_device_info(uint32_t dev_id, uint32_t sub_cmd, char *in, uint32_t in_len, uint32_t *out_len)
{
	int32_t ret;
	struct devdrv_ipc_imu ipc_msg = {0};
	struct devdrv_ipc_imu ack_msg = {0};
	char *buff = NULL;

	ret = query_info_preprocess(sub_cmd, &out_len, &buff);
	if (ret != 0) {
		memory_drv_err("query_info_preprocess error, ret=%d\n", ret);
		return ret;
	}

	// 由于不支持频繁发送IPC，加入缓存机制：第一次查询时发送IPC查询，并将结果保存到缓存，后续查询直接读取缓存。
	if (is_query_from_buff(sub_cmd))
		return query_memory_info_from_buff(sub_cmd, &in, &buff, *out_len);

	ipc_msg.source_id = 0;
	ipc_msg.target_id = OBJ_IMU;
	ipc_msg.cmd_type0 = CMD_INQUIRY;
	ipc_msg.cmd_type1 = get_ipc_cmd_type(sub_cmd);

	ret = memory_query_from_lp(dev_id, &ipc_msg, &ack_msg, &buff);
	if (ret != 0) {
		memory_drv_err("Query info from lp error. (dev_id=%u; ret=%d)", dev_id, ret);
		return ret;
	}

	ret = ipc_info_save_to_buff(sub_cmd, &buff, *out_len, &ack_msg);
	if (ret != 0) {
		memory_drv_err("Save ddr info to buff error. (dev_id=%u; ret=%d)", dev_id, ret);
		return ret;
	}

	return copy_data_to_usr(&in, &buff, *out_len);
}
#endif
