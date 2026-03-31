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

#include <linux/securec.h>
#include <linux/uaccess.h>
#include "lpm_devmng_ipc.h"
#include "lpm_devmng_sharemem_drv.h"
#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
#include "lpm_devmng_debugfs.h"
#endif
#include "lpm_devmng_vrd_upgrade.h"

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
STATIC int32_t lpm_vrd_upgrade_debugfs_init(void);
#endif

// ipc msg transfer with lpmcu
struct lpm_vrd_upgrade_ipc_send {
	uint8_t slave_addr;
	uint8_t vrd_type;
	uint16_t version;
	uint32_t flag;
	uint32_t firmware_size;
	uint32_t firmware_addr;   // addr access from lpmcu
};

STATIC int32_t lpm_vrd_upgrade_ipc_send_request(
	uint32_t dev_id, const struct lpm_vrd_upgrade_ipc_send *vrd_upgrade_data)
{
	struct lpm_devmng_ipc_msg ipc_msg;
	int32_t ret;

	ipc_msg.send_type = LPM_IPC_NOTIFY_VRD_UPGRADE;
	ipc_msg.in        = (void *)vrd_upgrade_data;
	ipc_msg.in_len    = (uint32_t)sizeof(*vrd_upgrade_data);
	ipc_msg.out       = NULL;
	ipc_msg.out_pos   = 0;
	ipc_msg.out_len   = 0;

	// request lpm vrd upgrade, vrd upgrade will processed in next system power off, not immediately
	ret = lpm_ipc_send_async_msg(dev_id, &ipc_msg);
	if (ret != 0) {
		lpm_log_err("lpm vrd upgrade send ipc failed, ret=%d\n", ret);
		return ret;
	}

	return 0;
}

STATIC int32_t lpm_vrd_upgrade_request_check_valid(const struct lpm_vrd_upgrade_info *vrd_upgrade_info)
{
	// mps2975 and xdpe122 only
	if ((vrd_upgrade_info->vrd_type != VRD_MP2975) &&
		(vrd_upgrade_info->vrd_type != VRD_XDPE122)) {
		lpm_log_err("vrd type %u invalid\n", vrd_upgrade_info->vrd_type);
		return -EINVAL;
	}

	if ((vrd_upgrade_info->flag & UPGRADE_VALID) != 0) {
		if (vrd_upgrade_info->firmware_buf == NULL) {
			lpm_log_err("vrd firmware buffer null\n");
			return -EINVAL;
		}

		if ((vrd_upgrade_info->firmware_size == 0) ||
			(vrd_upgrade_info->firmware_size > VRD_FIRMWARE_SIZE_MAX)) {
			lpm_log_err("vrd firmware size %u invalid\n", vrd_upgrade_info->firmware_size);
			return -EINVAL;
		}
	}

	return 0;
}

int32_t lpm_vrd_upgrade_request(uint32_t dev_id, const struct lpm_vrd_upgrade_info *vrd_upgrade_info)
{
	int32_t ret;
	struct lpm_vrd_upgrade_ipc_send upgrade_ipc = {0};

	if ((!lpm_common_check_dev_id(dev_id)) || (vrd_upgrade_info == NULL)) {
		lpm_log_err("vrd upgrade failed, dev_id=%u out of range or upgrade info null\n", dev_id);
		return -EINVAL;
	}

	ret = lpm_vrd_upgrade_request_check_valid(vrd_upgrade_info);
	if (ret != 0) {
		lpm_log_err("vrd request check failed, ret=%d\n", ret);
		return ret;
	}

	if (vrd_upgrade_info->flag & UPGRADE_VALID) {
		ret = lpm_save_vrd_upgrade_to_sharemem(0, vrd_upgrade_info->firmware_buf, vrd_upgrade_info->firmware_size);
		if (ret < 0) {
			lpm_log_err("copy firmware to sharemem failed, ret=%d\n", ret);
			return ret;
		}
		upgrade_ipc.firmware_size = vrd_upgrade_info->firmware_size;
		upgrade_ipc.firmware_addr = (uint32_t)lpm_get_vrd_upgrade_sharemem_addr();
	}

	upgrade_ipc.flag       = vrd_upgrade_info->flag;
	upgrade_ipc.slave_addr = vrd_upgrade_info->slave_addr;
	upgrade_ipc.vrd_type   = vrd_upgrade_info->vrd_type;
	upgrade_ipc.version    = vrd_upgrade_info->version;
	return lpm_vrd_upgrade_ipc_send_request(dev_id, &upgrade_ipc);
}
EXPORT_SYMBOL(lpm_vrd_upgrade_request);

int32_t lpm_vrd_upgrade_get_status_info(char *in, uint32_t in_len, char *out, uint32_t out_len)
{
	int32_t ret;
	struct lpm_device_vrd_status_info vrd_status = {0};
	struct lpm_vrd_info_cfg_in *lpm_info = NULL;
	struct lpm_devmng_dsmi_in_param in_param;

	in_param.in              = in;
	in_param.in_len          = in_len;
	in_param.correct_in_len  = (uint32_t)sizeof(struct lpm_vrd_info_cfg_in);
	in_param.out             = out;
	in_param.out_len         = out_len;
	in_param.correct_out_len = (uint32_t)sizeof(struct lpm_device_vrd_status_info);

	if (!lpm_common_check_dsmi_in_param(&in_param)) {
		lpm_log_err("lpm vrd info query param check failed\n");
		return -EINVAL;
	}

	lpm_info = (struct lpm_vrd_info_cfg_in *)in;

	if (!lpm_common_check_dev_id(lpm_info->dev_id)) {
		lpm_log_err("lpm check get vrd info param failed, dev_id=%u is out of range\n", lpm_info->dev_id);
		return -ENODEV;
	}

	ret = lpm_get_vrd_info_from_sharemem(lpm_info->dev_id, &vrd_status);
	if (ret != 0) {
		lpm_log_err("lpm get vrd info failed, dev_id=%u, ret=%d\n", lpm_info->dev_id, ret);
		return ret;
	}

	*(struct lpm_device_vrd_status_info *)out = vrd_status;
	return 0;
}

int32_t lpm_vrd_upgrade_probe(uint64_t *param, uint32_t param_num)
{
#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
	int32_t ret;

	ret = lpm_vrd_upgrade_debugfs_init();
	if (ret != 0) {
		lpm_log_err("lpm vrd_upgrade init debugfs failed, ret=%d\n", ret);
		return ret;
	}
#endif

	(void)param;
	(void)param_num;
	lpm_log_info("lpm vrd_upgrade probe success\n");
	return 0;
}

int32_t lpm_vrd_upgrade_remove(uint64_t *param, uint32_t param_num)
{
	(void)param;
	(void)param_num;
	lpm_log_info("lpm vrd_upgrade remove success\n");
	return 0;
}

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)

STATIC int32_t lpm_vrd_upgrade_debugfs_read_firmware(uint32_t *firmware_buf, uint32_t size)
{
	uint32_t cnt;
	struct file *file = NULL;

	file = filp_open(VRD_FIRMWARE_FILE_PATH, O_RDONLY, 0);
	if (IS_ERR(file)) {
		lpm_log_err("Opening file is abnormal. file name %s\n", VRD_FIRMWARE_FILE_PATH);
		return -EINVAL;
	}

	for (cnt = 0; cnt < size; cnt++) {
		uint32_t value = 0;
		if (kernel_read(file, &value, sizeof(value), &file->f_pos) != 0) {
			firmware_buf[cnt] = value;
		} else {
			break;
		}
	}

	filp_close(file, NULL);
	return (int32_t)cnt;
}

STATIC ssize_t lpm_vrd_upgrade_debugfs_write(struct file *file, const char __user *buf, size_t len, loff_t *pos)
{
	int32_t ret;
	int32_t cnt;
	uint32_t args[0x4];   // salve_addr/vrd_type/version/flag
	uint32_t firmware_size;
	struct lpm_vrd_upgrade_info vrd_upgrade_info = {0};
	static uint32_t firmware_buf[VRD_FIRMWARE_SIZE_MAX / sizeof(uint32_t)];

	(void)file;
	(void)pos;

	ret = lpm_debugfs_get_input(buf, len, args, ARRAY_SIZE(args));
	if (ret != 0) {
		lpm_log_err("getdebugfs input failed, ret=%d\n", ret);
		return ret;
	}

	cnt = lpm_vrd_upgrade_debugfs_read_firmware(firmware_buf, ARRAY_SIZE(firmware_buf));
	if (cnt <= 0) {
		lpm_log_err("read firmware fail, file name %s\n", VRD_FIRMWARE_FILE_PATH);
		return -1;
	}

	lpm_log_info("vrd firmware data cnt: %d\n", cnt);
	firmware_size = cnt * sizeof(uint32_t);
	ret = lpm_save_vrd_upgrade_to_sharemem(0, (const uint8_t *)firmware_buf, firmware_size);
	if (ret < 0) {
		lpm_log_err("copy firmware to sharemem failed, ret=%d\n", ret);
		return ret;
	}

	vrd_upgrade_info.slave_addr    = args[0x0];
	vrd_upgrade_info.vrd_type      = args[0x1];
	vrd_upgrade_info.version       = args[0x2];
	vrd_upgrade_info.flag          = args[0x3];
	vrd_upgrade_info.firmware_size = firmware_size;
	vrd_upgrade_info.firmware_buf  = (uint8_t *)&firmware_buf[0];
	ret = lpm_vrd_upgrade_request(0, &vrd_upgrade_info);
	if (ret < 0) {
		lpm_log_err("vrd upgrade request failed, ret=%d\n", ret);
		return ret;
	}

	lpm_log_info("vrd upgrade request succ, please trig power off to process upgrade\n");
	return (ssize_t)len;
}

STATIC void lpm_debugfs_show_vrd_info(
	struct seq_file *seq, const struct lpm_device_vrd_status_info *vrd_status)
{
	uint32_t i;
	const struct lpm_vrd_status_info *vrd_info = NULL;
	for (i = 0; i < vrd_status->num; i++) {
		vrd_info = &vrd_status->vrd_info[i];
		seq_printf(seq, "     [%02u]valid=%u, vrd_type=%u, slave=0x%x, version=%u, upgrade_remain_cnt=%u\n",
			i, vrd_info->valid, vrd_info->vrd_type, vrd_info->slave_addr,
			vrd_info->version, vrd_info->upgrade_remain_cnt);
	}
}

STATIC int32_t lpm_vrd_upgrade_debugfs_show(struct seq_file *seq, void *v)
{
	int32_t ret;
	uint32_t dev_id;
	uint32_t dev_num = lpm_common_get_dev_num();
	uint32_t in_len  = (uint32_t)sizeof(struct lpm_vrd_info_cfg_in);
	uint32_t out_len = (uint32_t)sizeof(struct lpm_device_vrd_status_info);
	struct lpm_vrd_info_cfg_in cfg_in = {0};
	struct lpm_device_vrd_status_info vrd_status = {0};

	seq_printf(seq, "Get vrd info from sharemem:\n");

	for (dev_id = 0; dev_id < dev_num; dev_id++) {
		cfg_in.dev_id = dev_id;
		seq_printf(seq, " [dev_id=%u]\n", dev_id);

		ret = lpm_vrd_upgrade_get_status_info((char *)&cfg_in, in_len, (char *)&vrd_status, out_len);
		if (ret != 0) {
			seq_printf(seq, "   Read vrd info from sharemem failed, ret=%d\n", ret);
		} else {
			lpm_debugfs_show_vrd_info(seq, &vrd_status);
		}
	}

	(void)v;
	return 0;
}

STATIC int32_t lpm_vrd_upgrade_debugfs_open(struct inode *inode, struct file *file)
{
	return single_open(file, lpm_vrd_upgrade_debugfs_show, inode->i_private);
}

STATIC int32_t lpm_vrd_upgrade_debugfs_init(void)
{
	int32_t ret;
	struct lpm_debugfs_init_cfg init_cfg = {
		.name = "vrd_upgrade",
		.fn_open = lpm_vrd_upgrade_debugfs_open,
		.fn_write = lpm_vrd_upgrade_debugfs_write
	};

	ret = lpm_debugfs_register_module(&init_cfg);
	if (ret != 0) {
		lpm_log_err("lpm vrd_upgrade register debugfs failed, ret=%d\n", ret);
		return ret;
	}
	return 0;
}

#endif
