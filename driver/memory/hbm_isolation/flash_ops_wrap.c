/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
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
* Create: 2019-10-15
*/
#include "memory_log.h"
#include <linux/errno.h>

#ifdef CFG_NOR_FLASH_OPS_VER1

#include "hisfc300_def.h"

int hbm_flash_read(const unsigned char *part_name, unsigned int offset, size_t len, unsigned char *buf)
{
	int ret;
	size_t ret_len;

	ret = hisi_sfc_ctl_flash_read(part_name, offset, len, &ret_len, buf);
	if (ret != 0) {
		memory_drv_err("read flash fail, part_name: %s, offset: %u, len: %lu, ret: %d!\n",
					   part_name, offset, len, ret);
		return ret;
	}

	if (len != ret_len) {
		memory_drv_err("read data err, len(%lu) != ret_len(%lu)!\n", len, ret_len);
		return -EIO;
	}

	return 0;
}

int hbm_flash_write(const unsigned char *part_name, unsigned int offset, size_t len, unsigned char *buf)
{
	int ret;
	size_t ret_len;

	memory_drv_event("ecc_config write data to flash.\n");
	ret = hisi_sfc_ctl_flash_erase(part_name, offset, len);
	if (ret != 0) {
		memory_drv_err("erase flash fail, ret: %d!\n", ret);
		return ret;
	}

	ret = hisi_sfc_ctl_flash_write((unsigned char *)part_name, offset, len, &ret_len, buf);
	if (ret != 0) {
		memory_drv_err("write flash fail, part_name: %s, offset: %u, len: %lu, ret: %d!\n",
			part_name, offset, len, ret);
		return ret;
	}
	if (len != ret_len) {
		memory_drv_err("write data err, len(%lu) != ret_len(%lu)!\n", len, ret_len);
		return -EIO;
	}

	return 0;
}

int hbm_flash_erase(const unsigned char *part_name, unsigned int offset, size_t len)
{
	return hisi_sfc_ctl_flash_erase(part_name, offset, len);
}

#endif

#ifdef CFG_NOR_FLASH_OPS_VER2

#include "hsm_norflash.h"
#include <linux/kthread.h>
#include <linux/delay.h>

#define SEC_FLASH_DELAY_MAX_COUNT	100U
#define SEC_FLASH_DELAY_TIME_MS		100U
#define SEC_FLASH_OPER_SUCCESS		1U

#define SEC_FLASH_OPER_READ			0U
#define SEC_FLASH_OPER_WRITE		1U
#define SEC_FLASH_OPER_ERASE		2U

struct flash_op_para {
	unsigned int oper_type;
	unsigned int dev_id;
	unsigned int offset;
	unsigned int buf_len;
	unsigned char *buf;
	atomic_t flash_fin_flag;
};

#ifndef DRV_MEM_GTEST
STATIC int sec_ecc_exec_task(struct flash_op_para *para)
{
	if (para->oper_type == SEC_FLASH_OPER_WRITE) {
		return sec_flash_write(para->dev_id, para->offset, (const unsigned char *)para->buf, para->buf_len);
	} else if (para->oper_type == SEC_FLASH_OPER_ERASE) {
		return sec_flash_erase(para->dev_id, para->offset, para->buf_len);
	} else {
		return sec_flash_read(para->dev_id, para->offset, para->buf, para->buf_len);
	}
}

STATIC int sec_ecc_config_task(void *arg)
{
	int ret = 0;
	int loop_cnt = 0;

	struct flash_op_para *p_para = (struct flash_op_para *)arg;

	/* start to read flash */
	while (loop_cnt <= SEC_FLASH_DELAY_MAX_COUNT) {
		ret = sec_ecc_exec_task(p_para);
		if (ret == 0) {
			break;
		} else {
			/* retry */
			loop_cnt++;
			msleep(SEC_FLASH_DELAY_TIME_MS);
			memory_drv_err("sec_ecc_config_task ret=%d, loop_cnt=%d\n", ret, loop_cnt);
		}
	}

	atomic_set(&p_para->flash_fin_flag, SEC_FLASH_OPER_SUCCESS);

	return ret;
}

STATIC int sec_ecc_config_handle(unsigned int oper_type, unsigned int offset, size_t len, unsigned char *buf)
{
	struct flash_op_para para;
	struct task_struct *task = NULL;

	para.oper_type = oper_type;
	para.dev_id = 0; // always device 0
	para.offset = offset;
	para.buf_len = len;
	para.buf = buf;
	atomic_set(&para.flash_fin_flag, 0);

	/* must use kthread to call TEE function */
	task = kthread_run(sec_ecc_config_task, (void *)&para, "Hbm Flash Ops");
	if (IS_ERR_OR_NULL(task)) {
		memory_drv_err("hbm task(%u) failed. (errno=%ld).\n", oper_type, PTR_ERR(task));
		return -EINVAL;
	}

	/* wait max 10s and flag = 1 to make sure ecc write finished */
	while (atomic_read(&para.flash_fin_flag) == 0) {
		msleep(SEC_FLASH_DELAY_TIME_MS);
	}

	return 0;
}
#endif

int hbm_flash_read(const unsigned char *part_name, unsigned int offset, size_t len, unsigned char *buf)
{
	(void)part_name;
	return sec_ecc_config_handle(SEC_FLASH_OPER_READ, offset, len, buf);
}

int hbm_flash_write(const unsigned char *part_name, unsigned int offset, size_t len, unsigned char *buf)
{
	(void)part_name;
	return sec_ecc_config_handle(SEC_FLASH_OPER_WRITE, offset, len, buf);
}

int hbm_flash_erase(const unsigned char *part_name, unsigned int offset, size_t len)
{
	(void)part_name;
	return sec_ecc_config_handle(SEC_FLASH_OPER_ERASE, offset, len, NULL);
}

#endif
