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
#include <linux/slab.h>
#include "memory_log.h"
#include <linux/errno.h>
#include <linux/debugfs.h>
#include <linux/fs.h>
#include "hbm_isolation_common.h"
#include <hbm_user_type.h>

#define MEM_BUFF_MAX_LEN	64U

static char memory_buf[MEM_BUFF_MAX_LEN] = "test for memory isolation\n";
struct dentry *memory_dir = NULL;
struct dentry *memory_ctrl_file = NULL;

int hbm_get_isolated_page_statistics(void *feature, char *in, uint32_t in_len, char *out, uint32_t out_len);
int hbm_get_ecc_record_info(void *feature, char *in, uint32_t in_len, char *out, uint32_t out_len);
int hbm_clear_isolated_info(void *feature, char *in, uint32_t in_len, char *out, uint32_t out_len);

struct cmd_map {
	char cmd0;
	char cmd1;
	const char *func_name;
	int (*func_ptr)(void);
};

// 清空flash
STATIC int flash_clear_isolated(void)
{
	uint32_t in = 0;
	return hbm_clear_isolated_info(NULL, (char *)&in, sizeof(uint32_t), NULL, 0);
}

// 获取隔离统计信息
STATIC int get_isolated_page_statistics(void)
{
	int ret;
	uint32_t in = memory_buf[0x2U] - '0'; // devid
	struct ecc_isolated_statistics_record out = {0};

	ret = hbm_get_isolated_page_statistics(NULL, (char *)&in, sizeof(uint32_t), (char *)&out, sizeof(out));
	if (ret) {
		memory_drv_err("get_isolated_page_statistics err: %d\n", ret);
		return ret;
	}

	// 输出统计信息
	memory_drv_info("GET_ISOLATED_PAGE_STAT dev_id:%u\n", in);
	memory_drv_info("hbm_single_bit_count: %d\n", out.hbm_single_bit_count);
	memory_drv_info("hbm_mul_bit_count: %d\n", out.hbm_mul_bit_count);
	memory_drv_info("hbm_single_isolated_page_count: %d\n", out.hbm_single_isolated_page_count);
	memory_drv_info("hbm_multi_isolated_page_count: %d\n", out.hbm_multi_isolated_page_count);

	return 0;
}

#ifndef DRV_MEM_GTEST
// 获取隔离统计信息
STATIC int get_ecc_record_info(void)
{
	int ret, count;
	struct ecc_config_udata_s udata = {0};
	struct ecc_config_udata_s out = {0};

	udata.dev_id = memory_buf[0x2U] - '0'; // devid
	udata.op_type = memory_buf[0x3U] - '0'; // func
	udata.data_index = memory_buf[0x4U] - '0'; // index

	ret = hbm_get_ecc_record_info(NULL, (char *)&udata, sizeof(udata), (char *)&out, sizeof(out));
	if (ret) {
		memory_drv_err("get_ecc_record_info err: %d\n", ret);
		return ret;
	}

	// 解析返回值
	memory_drv_info("GET_ECC_RECORD_INFO dev_id:%u\n", udata.dev_id);

	if (udata.op_type == MULTI_ECC_TIMES_READ) {
		memory_drv_info("MULTI_ECC_TIMES_READ info:\n");
		memory_drv_info("multi_record_count : %u\n", out.multi_ecc_time_data.multi_record_count);
		for (count = 0; count < out.multi_ecc_time_data.multi_record_count; count++) {
			memory_drv_info("multi_record_count[%u] : %x\n", count, out.multi_ecc_time_data.multi_ecc_times[count]);
		}
	} else if (udata.op_type == SINGLE_ECC_INFO_READ) {
		memory_drv_info("SINGLE_ECC_INFO_READ info: NO\n");
	} else if (udata.op_type == MULTI_ECC_INFO_READ) {
		memory_drv_info("MULTI_ECC_INFO_READ info:\n");
		memory_drv_info("physical_addr : %llx\n", out.multi_ecc_data.physical_addr);
		memory_drv_info("rank : %u\n", out.multi_ecc_data.rank);
		memory_drv_info("module_id : %u\n", out.multi_ecc_data.module_id);
		memory_drv_info("type : %u\n", out.multi_ecc_data.type);
		memory_drv_info("module : %u\n", out.multi_ecc_data.module);
		memory_drv_info("bank : %u\n", out.multi_ecc_data.bank);
		memory_drv_info("row : %u\n", out.multi_ecc_data.row);
		memory_drv_info("column : %u\n", out.multi_ecc_data.column);
		memory_drv_info("timer_stamp : %u\n", out.multi_ecc_data.timer_stamp);
	} else if (udata.op_type == ECC_ADDRESS_COUNT_READ) {
		memory_drv_info("ECC_ADDRESS_COUNT_READ info:\n");
		memory_drv_info("single_ecc_addr_cnt : %u\n", out.ecc_address_count.single_ecc_addr_cnt);
		memory_drv_info("multi_ecc_addr_cnt : %u\n", out.ecc_address_count.multi_ecc_addr_cnt);
	} else {
		memory_drv_err("get_ecc_record_info op_type err: %u\n", udata.op_type);
	}

	return 0;
}

struct cmd_map g_cmd_list[] = {
	{ 'a', 'a', "flash_clear_isolated", flash_clear_isolated},
	{ 'a', 'b', "get_isolated_page_statistics", get_isolated_page_statistics},
	{ 'a', 'c', "get_ecc_record_info", get_ecc_record_info},
};

int memory_drv_cmd_parse(const char *buf, size_t count)
{
	int step;

	if (count < 0x2U) {
		return -EINVAL;
	}

	for (step = 0; step < (sizeof(g_cmd_list) / sizeof(g_cmd_list[0])); step++) {
		if ((buf[0] == g_cmd_list[step].cmd0) && (buf[1] == g_cmd_list[step].cmd1)) {
			memory_drv_info("memory_drv_cmd_parse exec cmd: %s\n", g_cmd_list[step].func_name);
			return g_cmd_list[step].func_ptr();
		}
	}

	memory_drv_info("memory_drv_cmd_parse no cmd found\n");

	return 0;
}

static int memory_open(struct inode *inode, struct file *filp)
{
	memory_drv_info("memory ctrl open\n");
	filp->private_data = inode->i_private;

	return 0;
}

ssize_t memory_read(struct file *filp, char __user *buf, size_t count, loff_t *offp)
{
	int retval = 0;
	if ((*offp + count) > MEM_BUFF_MAX_LEN)
		count = MEM_BUFF_MAX_LEN - *offp;

	memory_drv_info("read request: count:%lu, offset:%llu\n", count, *offp);
	if (copy_to_user(buf, memory_buf + *offp, count)) {
		memory_drv_info("copy to user failed, count:%lu\n", count);
		retval = -EFAULT;
		goto out;
	}
	*offp += count;
	retval = count;

out:
	return retval;
}

ssize_t memory_write(struct file *filp, const char __user *buff, size_t count, loff_t *offp)
{
	int retval;
	memory_drv_info("write request: count:%lu, offset:%llu\n", count, *offp);
	if (*offp > MEM_BUFF_MAX_LEN)
		return 0;

	if (*offp + count > MEM_BUFF_MAX_LEN)
		count = MEM_BUFF_MAX_LEN - *offp;

	if (copy_from_user(memory_buf + *offp, buff, count)) {
		memory_drv_info("copy from user failed, count:%lu\n", count);
		retval = -EFAULT;
		goto out;
	}
	*offp += count;
	retval = count;

	// 做命令解析
	if (memory_drv_cmd_parse(memory_buf, count)) {
		memory_drv_info("parse or exec cmd failed, count:%lu\n", count);
		retval = -EFAULT;
		goto out;
	}

out:
	return retval;
}

struct file_operations memory_fops = {
	.owner = THIS_MODULE,
	.read = memory_read,
	.write = memory_write,
	.open = memory_open,
};

int memory_debug_fs_init(void)
{
	memory_drv_info("memory_debug_fs_init\n");
	memory_dir = debugfs_create_dir("memory", NULL);
	if (memory_dir == NULL) {
		memory_drv_info("memory_debug_fs_init fail 0\n");
		goto fail0;
	}

	memory_ctrl_file = debugfs_create_file("ctrl", 0644, memory_dir, NULL, &memory_fops);
	if (memory_ctrl_file == NULL) {
		memory_drv_info("memory_debug_fs_init fail 1\n");
		goto fail1;
	}

	return 0;

fail1:
	debugfs_remove(memory_dir);

fail0:
	return -EINVAL;
}

int memory_debug_fs_destroy(void)
{
	memory_drv_info("memory debug fs exit\n");
	debugfs_remove_recursive(memory_dir);

	return 0;
}
#endif
