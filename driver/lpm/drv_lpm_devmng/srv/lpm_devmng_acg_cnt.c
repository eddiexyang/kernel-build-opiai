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
#include <linux/types.h>
#include "lpm_devmng_common.h"
#include "lpm_devmng_ipc.h"
#include "lpm_devmng_acg_cnt.h"

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
#include "lpm_devmng_debugfs.h"

STATIC int32_t lpm_acg_cnt_debugfs_init(void);
#endif

STATIC int32_t lpm_get_acg_cnt_from_ipc(uint32_t dev_id, uint32_t *acg_cnt)
{
	int32_t ret;
	struct lpm_acg_ipc_ack ack_data = {0};
	struct lpm_devmng_ipc_msg ipc_msg;

	ipc_msg.send_type = LPM_IPC_QUERY_LP_ACG;
	ipc_msg.in        = NULL;
	ipc_msg.in_len    = 0;
	ipc_msg.out       = (void *)&ack_data;
	ipc_msg.out_pos   = 0;
	ipc_msg.out_len   = (uint32_t)sizeof(ack_data);

	ret = lpm_ipc_send_sync_msg(dev_id, &ipc_msg);
	if (ret != 0) {
		lpm_log_err("Get acg_cnt send ipc failed, ret=%d, dev_id=%u\n", ret, dev_id);
		return ret;
	}

	*acg_cnt = ack_data.acg_cnt;
	return 0;
}

int32_t lpm_get_acg_cnt(uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len)
{
	unsigned long cpy_len;
	uint32_t acg_cnt = 0;
	int32_t ret = lpm_check_dsmi_get_param(dev_id, in, in_len, (uint32_t)sizeof(acg_cnt), out_len);
	if (ret != 0) {
		lpm_log_err("Get acg_cnt info, check param failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
		return ret;
	}

	ret = lpm_get_acg_cnt_from_ipc(dev_id, &acg_cnt);
	if (ret != 0) {
		lpm_log_err("Get acg_cnt from ipc failed. (dev_id=%u;ret=%d)\n", dev_id, ret);
		return ret;
	}

	cpy_len = copy_to_user(in, (void *)&acg_cnt, sizeof(acg_cnt));
	if (cpy_len != 0) {
		lpm_log_err("Get acg_cnt info, copy to user failed. (dev_id=%u; cpy_len=%lu)\n", dev_id, cpy_len);
		return -EIO;
	}

	*out_len = (uint32_t)sizeof(acg_cnt);
	return 0;
}

int32_t lpm_acg_cnt_probe(uint64_t *param, uint32_t param_num)
{
#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
	int32_t ret = lpm_acg_cnt_debugfs_init();
	if (ret != 0) {
		lpm_log_err("lpm acg_cnt init debugfs failed, ret=%d\n", ret);
		return ret;
	}
#endif
	return 0;
}

int32_t lpm_acg_cnt_remove(uint64_t *param, uint32_t param_num)
{
	return 0;
}

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)

STATIC int32_t lpm_acg_cnt_debugfs_show(struct seq_file *seq, void *v)
{
	int32_t ret;
	uint32_t dev_id;
	uint32_t acg_cnt = 0;
	uint32_t dev_num = lpm_common_get_dev_num();

	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		ret = lpm_get_acg_cnt_from_ipc(dev_id, &acg_cnt);
		if (ret != 0) {
			seq_printf(seq, "get acg_cnt failed, dev_id=%u, ret=%d\n", dev_id, ret);
		} else {
			seq_printf(seq, "get acg_cnt succ, dev_id=%u, acg_cnt=%u\n", dev_id, acg_cnt);
		}
	}

	(void)v;
	return 0;
}

STATIC int32_t lpm_acg_cnt_debugfs_open(struct inode *inode, struct file *file)
{
	return single_open(file, lpm_acg_cnt_debugfs_show, inode->i_private);
}

STATIC int32_t lpm_acg_cnt_debugfs_init(void)
{
	int32_t ret;
	struct lpm_debugfs_init_cfg init_cfg = {
		.name = "acg_cnt",
		.fn_open = lpm_acg_cnt_debugfs_open,
		.fn_write = NULL
	};

	ret = lpm_debugfs_register_module(&init_cfg);
	if (ret != 0) {
		lpm_log_err("lpm acg_cnt register debugfs failed, ret=%d\n", ret);
		return ret;
	}

	return 0;
}

#endif