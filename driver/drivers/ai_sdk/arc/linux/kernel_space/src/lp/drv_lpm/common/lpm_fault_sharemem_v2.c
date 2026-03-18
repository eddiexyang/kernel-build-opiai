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
#include "lpm_fault_sharemem.h"
#include "lpm_fault_sharemem_v2.h"

// for devmng read
STATIC int32_t lpm_sharemem_devmng_get_head_info(
	uint32_t dev_id, uint32_t type, struct lpm_sharemem_header *head)
{
	uint32_t expect_crc;
	unsigned long head_len;
	unsigned long entry_len;
	unsigned long base_offset = (unsigned long)LPM_SHAREMEM_DEVMNG_ADDR_OFFSET;

	// read magic_num, version and entry_num
	head_len = (unsigned long)sizeof(head->magic_num) +
		(unsigned long)sizeof(head->version) + (unsigned long)sizeof(head->entry_num);
	(void)lpm_sharemem_get_value(dev_id, base_offset, (uint8_t *)head, head_len);

	if (head->magic_num != LPM_SHAREMEM_MAGIC_NUM) {
		lpm_log_err("get head information failed, magic_num=%#x error, version=%u, entry_num=%u\n",
			head->magic_num, head->version, head->entry_num);
		return -EINVAL;
	}

	if (head->entry_num > LPM_SHAREMEM_DEVMNG_MAX_TYPE_NUM) {
		lpm_log_err("get head information failed, entry_num=%u error, version=%u\n",
			head->entry_num, head->version);
		return -EINVAL;
	}

	entry_len = (unsigned long)sizeof(struct lpm_sharemem_info_entry) * (unsigned long)head->entry_num;
	(void)lpm_sharemem_get_value(dev_id, base_offset + head_len, (uint8_t *)&head->entry[0], entry_len);

	head_len += entry_len;
	(void)lpm_sharemem_get_value(
		dev_id, base_offset + head_len, (uint8_t *)&head->crc, (unsigned long)sizeof(head->crc));

	// check crc
	expect_crc = lpm_common_crc16((uint8_t *)head, (uint16_t)head_len);
	if (expect_crc != head->crc) {
		lpm_log_err("get type=%u head information failed, expect_crc=%u not equal to actual crc=%u, version=%u\n",
			type, expect_crc, head->crc, head->version);
		return -EINVAL;
	}

	if (type >= head->entry_num) {
		lpm_log_err("get head information failed, type invalid, type=%u, entry_num=%u, version=%u\n",
			type, head->entry_num, head->version);
		return -EINVAL;
	}

	return 0;
}

STATIC bool lpm_sharemem_check_devmng_entry_info(
	uint32_t dev_id, uint32_t type, struct lpm_sharemem_header *head)
{
	if (type != head->entry[type].type) {
		lpm_log_err("get head information type not match. type=%u; entry.type=%u; version=%u\n",
			type, head->entry[type].type, head->version);
		return false;
	}

	if (head->entry[type].offset > LPM_SHAREMEM_DEVMNG_SIZE) {
		lpm_log_err("get head information offset error. type=%u; entry.offset=%u; version=%u\n",
			type, head->entry[type].offset, head->version);
		return false;
	}

	if ((head->entry[type].len == 0) || (head->entry[type].len > LPM_SHAREMEM_DEVMNG_SIZE)) {
		lpm_log_err("get head information len error. type=%u; entry.len=%u; version=%u\n",
			type, head->entry[type].len, head->version);
		return false;
	}

	if ((head->entry[type].offset + head->entry[type].len) > (uint32_t)LPM_SHAREMEM_DEVMNG_SIZE) {
		lpm_log_err("get head information range error. type=%u; entry.offset=%u; entry.len=%u; version=%u\n",
			type, head->entry[type].offset, head->entry[type].len, head->version);
		return false;
	}

	return true;
}

STATIC int32_t lpm_sharemem_devmng_get_entry_info(uint32_t dev_id, struct lpm_sharemem_info_entry *info)
{
	int32_t ret;
	struct lpm_sharemem_header head = {0};

	// the devmng sharemem header information is determined
	// during initialization and will not be chaned during operation
	ret = lpm_sharemem_devmng_get_head_info(dev_id, info->type, &head);
	if (ret != 0) {
		lpm_log_err("get head information failed, ret=%d\n", ret);
		return ret;
	}

	if (!lpm_sharemem_check_devmng_entry_info(dev_id, info->type, &head)) {
		lpm_log_err("check head information failed. info.type=%u\n", info->type);
		return -EINVAL;
	}

	info->offset = head.entry[info->type].offset + (uint32_t)LPM_SHAREMEM_DEVMNG_ADDR_OFFSET;
	info->len = head.entry[info->type].len;

	return 0;
}

// read devmng info from sharemem
// out_len: the number of bytes to read
STATIC int32_t lpm_sharemem_devmng_get_entry_offset(uint32_t dev_id, enum lpm_sharemem_info_type info_type,
	uint32_t offset, uint32_t out_len, uint32_t *entry_offset)
{
	int32_t ret;
	struct lpm_sharemem_info_entry entry_info = {0};

	if (info_type >= LPM_SHAREMEM_TYPE_MAX) {
		lpm_log_err("info_type=%u error, dev_id=%u, offset=%u, out_len=%u\n",
			(uint32_t)info_type, dev_id, offset, out_len);
		return -EINVAL;
	}

	entry_info.type = (uint32_t)info_type;
	ret = lpm_sharemem_devmng_get_entry_info(dev_id, &entry_info);
	if (ret != 0) {
		lpm_log_err("dev_id=%u get %u entry information failed, offset=%u, out_len=%u, ret=%d\n",
			dev_id, (uint32_t)info_type, offset, out_len, ret);
		return ret;
	}

	if ((offset + out_len) > entry_info.len) {
		lpm_log_err("offset=%u and out_len=%u is large than entry len=%u, dev_id=%u, info_type=%u, entry offset=%u\n",
			offset, out_len, entry_info.len, dev_id, (uint32_t)info_type, entry_info.offset);
		return -EINVAL;
	}

	*entry_offset = entry_info.offset;

	return 0;
}

// read devmng info from sharemem
// out_len indicates the array length of out
// the buf len of out = out_len
// inner function, the parameter legality of dev_id and out is guaranteed by the caller
STATIC int32_t lpm_sharemem_devmng_get_value(uint32_t dev_id,
	enum lpm_sharemem_info_type info_type, uint32_t offset, uint8_t *out, uint32_t out_len)
{
	int32_t ret;
	uint32_t entry_offset = 0;

	ret = lpm_sharemem_devmng_get_entry_offset(dev_id, info_type, offset, out_len, &entry_offset);
	if (ret != 0) {
		lpm_log_err("dev_id=%u get %u entry information failed, ret=%d\n",
			dev_id, (uint32_t)info_type, ret);
		return ret;
	}

	(void)lpm_sharemem_get_value(dev_id, (unsigned long)entry_offset + (unsigned long)offset,
		out, (unsigned long)out_len);

	return 0;
}

int32_t lpm_get_suspend_status_from_sharemem(uint32_t dev_id, bool *in_suspend)
{
	int32_t ret;
	struct lpm_sys_status_info sys_status = {0};

	*in_suspend = true;

	// read sys status from sharemem
	ret = lpm_sharemem_devmng_get_value(dev_id, LPM_SHAREMEM_TYPE_SYS_STATUS, 0,
		(uint8_t *)&sys_status, (uint32_t)sizeof(struct lpm_sys_status_info));
	if (ret != 0) {
		lpm_log_err("read sys status from sharemem failed, dev_id=%u, ret=%d\n", dev_id, ret);
		return ret;
	}

	// lp has completed the resume
	if (sys_status.suspend_status == (uint32_t)LPM_SUSPEND_STATUS_RESUME) {
		*in_suspend = false;
	}
	return 0;
}