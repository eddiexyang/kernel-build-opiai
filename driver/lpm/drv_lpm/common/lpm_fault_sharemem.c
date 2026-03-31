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
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include "ascend_kernel_hal.h"
#ifndef DRV_LPM_FAULT
#include "lpm_devmng_sharemem.h"
#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
#include "lpm_devmng_debugfs.h"
#include <linux/slab.h>
#endif
#else
#include "lpm_fault_sharemem.h"
#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
#include "lpm_fault_debugfs.h"
#include <linux/slab.h>
#endif
#endif

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
STATIC int32_t lpm_sharemem_debugfs_init(void);
#endif

STATIC struct lpm_sharemem_priv *lpm_sharemem_priv_info(void)
{
	static struct lpm_sharemem_priv sharemem_priv_info = {0};
	return &sharemem_priv_info;
}

STATIC void lpm_sharemem_iounmap_addr(uint32_t dev_num)
{
	uint32_t dev_id;
	struct lpm_sharemem_priv *priv_data = lpm_sharemem_priv_info();

	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		if (priv_data->dev_data[dev_id].viraddr != NULL) {
			(void)iounmap(priv_data->dev_data[dev_id].viraddr);
			priv_data->dev_data[dev_id].viraddr = NULL;
			priv_data->dev_data[dev_id].viraddr_size = 0;
		}
	}
}

STATIC int32_t lpm_sharemem_calc_base_phy_addr(uint32_t dev_id, unsigned long *base_phy_addr)
{
	int32_t ret;
	uint32_t chip_id = 0;
	uint32_t die_id = 0;
	devdrv_hardware_info_t hardware_info = {0};

	lpm_common_get_chip_die_id(dev_id, &chip_id, &die_id);

#if defined(CFG_SOC_PLATFORM_CLOUD_V2)
	// For die0:
	// Components using reserved memory may access 0-2GB space through the winding offset address
	// (the offset is 0x10_0000_0000),
	// and the address range of this access method is not within the protection range of the
	// Mirror configuration
	// Mirror protects 0-2GB (0x0 – 0x8000_0000) low address space
	// Therefore, accessing die0 needs to use low address space
	if (die_id == 0) {
		*base_phy_addr = (unsigned long)LPM_SHAREMEM_DIE0_BASE_ADDR;
		return 0;
	}
#endif

	ret = hal_kernel_get_hardware_info(dev_id, &hardware_info);
	if (ret != 0) {
		lpm_log_err("hal kernel get hardware failed, dev_id=%u, chip_id=%u, die_id=%u\n",
			dev_id, chip_id, die_id);
		return ret;
	}

	*base_phy_addr = LPM_SHAREMEM_BASE_ADDR + hardware_info.phy_addr_offset;
	lpm_log_info("calc sharemem addr success, dev_id=%u, chip_id=%u, die_id=%u, connect_type=%u, offset=0x%llx\n",
		dev_id, chip_id, die_id, hardware_info.base_hw_info.connect_type, hardware_info.phy_addr_offset);
	return 0;
}

STATIC int32_t lpm_sharemem_ioremap_addr(uint32_t dev_num)
{
	uint32_t dev_id;
	int32_t ret;
	unsigned long base_phy_addr;
	struct lpm_sharemem_priv *priv_data = lpm_sharemem_priv_info();

	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		ret = lpm_sharemem_calc_base_phy_addr(dev_id, &base_phy_addr);
		if (ret != 0) {
			lpm_log_err("remap memory space failed, dev_id=%u\n", dev_id);
			goto remap_fail;
		}
		priv_data->dev_data[dev_id].viraddr = ioremap(base_phy_addr, LPM_SHAREMEM_ADDR_LEN);
		if (priv_data->dev_data[dev_id].viraddr == NULL) {
			lpm_log_err("remap memory space failed, dev_id=%u\n", dev_id);
			goto remap_fail;
		}
		priv_data->dev_data[dev_id].viraddr_size = (unsigned long)LPM_SHAREMEM_ADDR_LEN;

		spin_lock_init(&priv_data->dev_data[dev_id].mem_lock);
	}
	return 0;

remap_fail:
	lpm_sharemem_iounmap_addr(dev_id);
	return -1;
}

STATIC void lpm_sharemem_reg_read_to_user(
	uint32_t dev_id, unsigned long offset, uint8_t __user *out, unsigned long out_len)
{
	unsigned long cpy_len;
	struct lpm_sharemem_priv *priv_data = lpm_sharemem_priv_info();
	uintptr_t viraddr = (uintptr_t)priv_data->dev_data[dev_id].viraddr + (uintptr_t)offset;

	cpy_len = copy_to_user((void __user *)out, (void __iomem *)viraddr, out_len);
	if (cpy_len != 0) {
		lpm_log_err("sharemem copy to user failed, cpy_len=%lu\n", cpy_len);
	}
}

STATIC void lpm_sharemem_reg_read(
	uint32_t dev_id, unsigned long offset, uint8_t *out, unsigned long out_len)
{
	struct lpm_sharemem_priv *priv_data = lpm_sharemem_priv_info();
	uintptr_t viraddr = (uintptr_t)priv_data->dev_data[dev_id].viraddr + (uintptr_t)offset;

	// pay attention:
	// Internal function, parameter legality is guaranteed by the caller
	// to reduce unnecessary param checking
	(void)memset_s(out, out_len, 0, out_len);
	memcpy_fromio((void *)out, (void __iomem *)viraddr, out_len);
}

STATIC void lpm_sharemem_reg_write(
	uint32_t dev_id, unsigned long offset, const uint8_t *out, unsigned long out_len)
{
	unsigned long irq_flags;
	struct lpm_sharemem_priv *priv_data = lpm_sharemem_priv_info();
	uintptr_t viraddr = (uintptr_t)priv_data->dev_data[dev_id].viraddr + (uintptr_t)offset;

	// pay attention:
	// Internal function, parameter legality is guaranteed by the caller
	// to reduce unnecessary param checking
	spin_lock_irqsave(&priv_data->dev_data[dev_id].mem_lock, irq_flags);
	memcpy_toio((void __iomem *)viraddr, (const void *)out, out_len);
	spin_unlock_irqrestore(&priv_data->dev_data[dev_id].mem_lock, irq_flags);
}

STATIC void lpm_sharemem_reg_clear(
	uint32_t dev_id, unsigned long offset, unsigned long out_len)
{
	unsigned long irq_flags;
	struct lpm_sharemem_priv *priv_data = lpm_sharemem_priv_info();
	uintptr_t viraddr = (uintptr_t)priv_data->dev_data[dev_id].viraddr + (uintptr_t)offset;

	// pay attention:
	// Internal function, parameter legality is guaranteed by the caller
	// to reduce unnecessary param checking
	spin_lock_irqsave(&priv_data->dev_data[dev_id].mem_lock, irq_flags);
	memset_io((void __iomem *)viraddr, 0, out_len);
	spin_unlock_irqrestore(&priv_data->dev_data[dev_id].mem_lock, irq_flags);
}

STATIC int32_t lpm_sharemem_check_reg_offset(
	uint32_t dev_id, unsigned long offset, unsigned long out_len)
{
	struct lpm_sharemem_priv *priv_data = lpm_sharemem_priv_info();

	if (out_len == 0) {
		lpm_log_err("check share memory reg offset failed, out_len is error."
			"(offset=%lu, out_len=%lu)\n", offset, out_len);
		return -EINVAL;
	}

	if ((offset + out_len) > priv_data->dev_data[dev_id].viraddr_size) {
		lpm_log_err("check share memory reg offset failed, illegal operating range."
			"(offset=%lu, out_len=%lu)\n", offset, out_len);
		return -EINVAL;
	}
	return 0;
}

int32_t lpm_sharemem_get_value_to_user(
	uint32_t dev_id, unsigned long offset, uint8_t __user *out, unsigned long out_len)
{
	int32_t ret;

	ret = lpm_sharemem_check_reg_offset(dev_id, offset, out_len);
	if (ret != 0) {
		lpm_log_err("fail to get value from sharememory. (dev_id=%u, offset=%lu, out_len=%lu, ret=%d)\n",
			dev_id, offset, out_len, ret);
		return ret;
	}
	lpm_sharemem_reg_read_to_user(dev_id, offset, out, out_len);

	return 0;
}

// inner function, the parameter legality of dev_id and out is guaranteed by the caller
// out_len indicates the array length of out
int32_t lpm_sharemem_get_value(
	uint32_t dev_id, unsigned long offset, uint8_t *out, unsigned long out_len)
{
	int32_t ret;

	ret = lpm_sharemem_check_reg_offset(dev_id, offset, out_len);
	if (ret != 0) {
		lpm_log_err("fail to get value from sharememory. (dev_id=%u, offset=%lu, out_len=%lu, ret=%d)\n",
			dev_id, offset, out_len, ret);
		return ret;
	}
	lpm_sharemem_reg_read(dev_id, offset, out, out_len);

	return 0;
}

// inner function, the parameter legality of dev_id and out is guaranteed by the caller
// out_len indicates the array length of out
int32_t lpm_sharemem_set_value(
	uint32_t dev_id, unsigned long offset, const uint8_t *out, unsigned long out_len)
{
	int32_t ret;

	ret = lpm_sharemem_check_reg_offset(dev_id, offset, out_len);
	if (ret != 0) {
		lpm_log_err("fail to set value to sharememory. (dev_id=%u, offset=%lu, out_len=%lu, ret=%d)\n",
			dev_id, offset, out_len, ret);
		return ret;
	}
	lpm_sharemem_reg_write(dev_id, offset, out, out_len);

	return 0;
}

// inner function, the parameter legality of dev_id is guaranteed by the caller
int32_t lpm_sharemem_clear_value(
	uint32_t dev_id, unsigned long offset, unsigned long out_len)
{
	int32_t ret;

	ret = lpm_sharemem_check_reg_offset(dev_id, offset, out_len);
	if (ret != 0) {
		lpm_log_err("fail to clear value of sharememory. (dev_id=%u, offset=%lu, out_len=%lu, ret=%d)\n",
			dev_id, offset, out_len, ret);
		return ret;
	}
	lpm_sharemem_reg_clear(dev_id, offset, out_len);

	return 0;
}

int32_t lpm_sharemem_probe(uint64_t *param, uint32_t param_num)
{
	int32_t ret;
	uint32_t dev_num = lpm_common_get_dev_num();

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
	ret = lpm_sharemem_debugfs_init();
	if (ret != 0) {
		lpm_log_err("lpm sharemem init debugfs failed, ret=%d\n", ret);
		return ret;
	}
#endif

	ret = lpm_sharemem_ioremap_addr(dev_num);
	if (ret != 0) {
		lpm_log_err("lpm sharemem ioremap failed, ret=%d\n", ret);
		return ret;
	}

	(void)param;
	(void)param_num;
	lpm_log_info("lpm sharemem probe success, dev_num=%u\n", dev_num);
	return 0;
}

int32_t lpm_sharemem_remove(uint64_t *param, uint32_t param_num)
{
	uint32_t dev_num = lpm_common_get_dev_num();

	lpm_sharemem_iounmap_addr(dev_num);

	(void)param;
	(void)param_num;
	lpm_log_info("lpm sharemem remove success, dev_num=%u\n", dev_num);
	return 0;
}

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
// false: illegal, true: legal
STATIC bool lpm_sharemem_check_query_param(uint32_t *user_cfg, uint32_t param_num)
{
	// inner function, no need check param_num
	(void)param_num;

	// user_cfg[1] means dev_id
	if (!lpm_common_check_dev_id(user_cfg[1])) {
		lpm_log_err("debugfs set sharemem query param failed, dev_id=%u is out of range\n", user_cfg[1]);
		return false;
	}

	// user_cfg[0]:0-read sharemem, 1-write sharemem
	// user_cfg[2] means offset, check before read or write sharemem, no need check here
	if (user_cfg[0] == 0x0) {
		// user_cfg[0x3] means read len
		if ((user_cfg[0x3] == 0) || (user_cfg[0x3] > 0x400)) {
			lpm_log_err("debugfs set sharemem query param failed, "
				"read_len=%u should large than 0, and not exceed 1024 bytes\n",
				user_cfg[0x3]);
			return false;
		}
	} else if (user_cfg[0] == 0x1) {
		// user_cfg[0x3] means write len
		if ((user_cfg[0x3] == 0) || (user_cfg[0x3] > 0x8)) {
			lpm_log_err("debugfs set sharemem query param failed, "
				"read_len=%u should large than 0, and not exceed 8 bytes\n",
				user_cfg[0x3]);
			return false;
		}
	} else {
		lpm_log_err("debugfs set sharemem query param failed, op_type=%u is out of range\n", user_cfg[0]);
		return false;
	}
	return true;
}

STATIC void lpm_sharemem_show_dev_info(uint32_t dev_id, struct seq_file *seq)
{
	uint32_t chip_id = 0;
	uint32_t die_id = 0;

	lpm_common_get_chip_die_id(dev_id, &chip_id, &die_id);
	seq_printf(seq, "dev_id=%u, chip_id=%u, die_id=%u\n", dev_id, chip_id, die_id);
}

STATIC void lpm_sharemem_write_query_param(uint32_t *user_cfg, uint32_t param_num)
{
	int32_t ret;
	uint8_t write_value[0x8] = {0};
	struct lpm_sharemem_priv *priv_data = lpm_sharemem_priv_info();

	if (!lpm_sharemem_check_query_param(user_cfg, param_num)) {
		lpm_log_err("debugfs write sharemem param check failed\n");
		return;
	}

	// user_cfg[0]:0-read sharemem, 1-write sharemem
	if (user_cfg[0] == 0x0) {
		priv_data->debugfs_param.debug_dev_id      = user_cfg[0x1]; // dev_id
		priv_data->debugfs_param.debug_read_offset = user_cfg[0x2]; // read_offset
		priv_data->debugfs_param.debug_read_len    = user_cfg[0x3]; // read_len
		lpm_log_info("debugfs set share mem query parameters success\n");
		return;
	}

	// trans user config to write value
	*(uint32_t *)&write_value[0x0] = user_cfg[0x4];
	*(uint32_t *)&write_value[0x4] = user_cfg[0x5];

	// user_cfg[0x1] means dev_id
	// user_cfg[0x2] means read offset
	// user_cfg[0x3] means write len
	ret = lpm_sharemem_set_value(user_cfg[0x1], (long)user_cfg[0x2], write_value, (long)user_cfg[0x3]);
	if (ret != 0) {
		lpm_log_err("debugfs write share mem failed, ret=%d\n", ret);
		return;
	}

	lpm_log_info("debugfs write share mem success, "
		"value=0x%02x-0x%02x-0x%02x-0x%02x-0x%02x-0x%02x-0x%02x-0x%02x, len=%u\n",
		write_value[0x0], write_value[0x1], write_value[0x2], write_value[0x3],
		write_value[0x4], write_value[0x5], write_value[0x6], write_value[0x7],
		user_cfg[0x3]);
}

STATIC void lpm_sharemem_show_info(struct seq_file *seq)
{
	uint8_t *out_buf = NULL;
	uint32_t i;
	int32_t ret;
	struct lpm_sharemem_priv *priv_data = lpm_sharemem_priv_info();

	if (!lpm_common_check_dev_id(priv_data->debugfs_param.debug_dev_id)) {
		seq_printf(seq, "dev_id=%u is out of range\n", priv_data->debugfs_param.debug_dev_id);
		return;
	}

	// show chip id and die id
	lpm_sharemem_show_dev_info(priv_data->debugfs_param.debug_dev_id, seq);

	seq_printf(seq, "\n query parameters for read sharemem: dev_id:%u, offset:0x%x, read_len:%u\n",
		priv_data->debugfs_param.debug_dev_id, priv_data->debugfs_param.debug_read_offset,
		priv_data->debugfs_param.debug_read_len);
	// read len must large than 0, and not large than 1024
	// it will check in set read param
	if (priv_data->debugfs_param.debug_read_len == 0) {
		seq_printf(seq, "    read_len is zero\n");
		return;
	}
	if (priv_data->debugfs_param.debug_read_len > 0x400) {
		seq_printf(seq, "    read_len is large than 0x400, debug_read_len=%u\n",
			priv_data->debugfs_param.debug_read_len);
		return;
	}

	out_buf = (uint8_t *)kmalloc(priv_data->debugfs_param.debug_read_len, GFP_KERNEL);
	if (out_buf == NULL) {
		seq_printf(seq, "    read from share mem alloc mem failed\n");
		return;
	}

	(void)memset_s(out_buf, priv_data->debugfs_param.debug_read_len, 0, priv_data->debugfs_param.debug_read_len);

	// read offset and read len will check when get value
	ret = lpm_sharemem_get_value(priv_data->debugfs_param.debug_dev_id,
		(long)priv_data->debugfs_param.debug_read_offset, out_buf, (long)priv_data->debugfs_param.debug_read_len);
	if (ret != 0) {
		kfree(out_buf);
		seq_printf(seq, "    read from share mem failed, ret=%d\n", ret);
		return;
	}

	// show sharemem info
	// each line show 16 byte
	seq_printf(seq, " value read from share mem:");
	for (i = 0; i < (long)priv_data->debugfs_param.debug_read_len; i++) {
		if ((i % 16) == 0) {
			seq_printf(seq, "\n0x%04x: ", i);
		}
		seq_printf(seq, "0x%02x ", out_buf[i]);
	}

	seq_printf(seq, "\n");
	kfree(out_buf);
}

STATIC ssize_t lpm_sharemem_debugfs_write(
	struct file *file, const char __user *buf, size_t len, loff_t *pos)
{
	int32_t ret;
	uint32_t user_cfg[LPM_DEBUGFS_MAX_USER_INPUT_NUM] = {0};

	(void)file;
	(void)pos;

	// op_type(0:get, 1:set), dev_id, offset, len, value0, value1
	// op_type = 0:
	// read value from sharemem, focus only on dev_id, op_type, offset, len
	// after setting the query parameters, call cat to get the query results
	ret = lpm_debugfs_get_input(buf, len, user_cfg, 0x6);
	if (ret != 0) {
		lpm_log_err("debugfs write sharemem failed, ret=%d\n", ret);
		return -1;
	}

	lpm_sharemem_write_query_param(user_cfg, 0x6);
	return (ssize_t)len;
}

STATIC int32_t lpm_sharemem_debugfs_show(struct seq_file *seq, void *v)
{
	(void)v;
	lpm_sharemem_show_info(seq);
	return 0;
}

STATIC int32_t lpm_sharemem_debugfs_open(struct inode *inode, struct file *file)
{
	return single_open(file, lpm_sharemem_debugfs_show, inode->i_private);
}

STATIC int32_t lpm_sharemem_debugfs_init(void)
{
	int32_t ret;
	struct lpm_debugfs_init_cfg init_cfg = {
		.name = "sharemem",
		.fn_open = lpm_sharemem_debugfs_open,
		.fn_write = lpm_sharemem_debugfs_write
	};

	ret = lpm_debugfs_register_module(&init_cfg);
	if (ret != 0) {
		lpm_log_err("lpm sharemem register debugfs failed, ret=%d\n", ret);
		return ret;
	}
	return 0;
}
#endif
