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
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include "lpm_devmng_sharemem.h"
#include "lpm_devmng_sharemem_drv.h"
#include "lpm_devmng_sharemem_v2.h"

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
	expect_crc = lpm_devmng_crc16((uint8_t *)head, (uint16_t)head_len);
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

int32_t lpm_get_temperature_from_sharemem(
	uint32_t dev_id, enum lpm_devmng_temperature_type temp_type, uint32_t *temperature)
{
	int32_t ret;
	uint16_t tmp_value = 0;
	// each temperature value occupies 16bit
	uint32_t offset = (uint32_t)temp_type * (uint32_t)sizeof(uint16_t);

	ret = lpm_sharemem_devmng_get_value(dev_id, LPM_SHAREMEM_TYPE_TEMP, offset,
		(uint8_t *)&tmp_value, (uint32_t)sizeof(tmp_value));
	if (ret != 0) {
		lpm_log_err("read temperature from sharemem failed, dev_id=%u, temp_type=%u, ret=%d\n",
			dev_id, (uint32_t)temp_type, ret);
		return ret;
	}

	*temperature = tmp_value;

	return 0;
}

int32_t lpm_get_frequency_from_sharemem(
	uint32_t dev_id, enum lpm_devmng_frequency_type freq_type, uint32_t *frequency)
{
	int32_t ret;
	// each frequency value occupies 32 bit
	uint32_t offset = (uint32_t)freq_type * (uint32_t)sizeof(uint32_t);
	uint32_t tmp_value = 0;

	ret = lpm_sharemem_devmng_get_value(dev_id, LPM_SHAREMEM_TYPE_FREQ, offset,
		(uint8_t *)&tmp_value, (uint32_t)sizeof(tmp_value));
	if (ret != 0) {
		lpm_log_err("read frequency from sharemem failed, dev_id=%u, freq_type=%u, ret=%d\n",
			dev_id, (uint32_t)freq_type, ret);
		return ret;
	}

	*frequency = tmp_value;

	return 0;
}

int32_t lpm_get_power_from_sharemem(
	uint32_t dev_id, enum lpm_devmng_power_type power_type, uint32_t *power_value)
{
	int32_t ret;
	// each frequency value occupies 32 bit
	uint32_t offset = (uint32_t)power_type * (uint32_t)sizeof(uint32_t);

	ret = lpm_sharemem_devmng_get_value(dev_id, LPM_SHAREMEM_TYPE_POWER, offset,
		(uint8_t *)power_value, (uint32_t)sizeof(uint32_t));
	if (ret != 0) {
		lpm_log_err("read power from sharemem failed. dev_id=%u, power_type=%u, ret=%d\n",
			dev_id, (uint32_t)power_type, ret);
		return ret;
	}

	return 0;
}

int32_t lpm_get_system_status_from_sharemem(uint32_t dev_id, struct lpm_sys_status_info *sys_status)
{
	int32_t ret;

	// read sys status from sharemem
	ret = lpm_sharemem_devmng_get_value(dev_id, LPM_SHAREMEM_TYPE_SYS_STATUS, 0,
		(uint8_t *)sys_status, (uint32_t)sizeof(struct lpm_sys_status_info));
	if (ret != 0) {
		lpm_log_err("read sys status from sharemem failed, dev_id=%u, ret=%d\n", dev_id, ret);
		return ret;
	}

	return 0;
}

// return value:
// true: wait success
// false: wait failed after (wait_cnt*wait_us) us or read failed
int32_t lpm_wait_lp_suspend_status_done(uint32_t dev_id, uint32_t wait_cnt, uint32_t wait_us,
	enum lpm_devmng_suspend_status target_status)
{
	uint32_t try_cnt;
	int32_t ret;
	struct lpm_sys_status_info sys_status = {0};

	for (try_cnt = 0; try_cnt < wait_cnt; try_cnt++) {
		ret = lpm_get_system_status_from_sharemem(dev_id, &sys_status);
		if (ret != 0) {
			lpm_log_err("try read sys status from sharemem failed, "
				"dev_id=%u, try_cnt=%u, target_status=%u, ret=%d\n",
				dev_id, try_cnt, (uint32_t)target_status, ret);
			return ret;
		}

		if (sys_status.suspend_status == (uint32_t)target_status) {
			return 0;
		}

		usleep_range(wait_us, wait_us);
	}

	lpm_log_err("wait lp target_status=%u failed, dev_id=%u, wait_cnt=%u, wait_us=%u\n",
		(uint32_t)target_status, wait_cnt, dev_id, wait_us);
	return -ETIMEDOUT;
}

bool lpm_profile_check_tops(uint32_t config_tops)
{
	return (config_tops < LPM_PROFILE_TOPS_MAX);
}

STATIC int32_t lpm_profile_tops_check(struct lpm_profile_tops_list *tops_list)
{
	uint32_t i;

	if (tops_list->tops_num > LPM_PROFILE_TOPS_MAX) {
		lpm_log_err("check profile tops read from sharemem failed. tops_num=%u\n",
			tops_list->tops_num);
		return -EINVAL;
	}

	for (i = 0; i < tops_list->tops_num; i++) {
		if (!lpm_profile_check_tops(tops_list->tops_table[i].profile_id)) {
			lpm_log_err("check profile tops read from sharemem failed. tops_table[%u].profile_id=%u\n",
				i, tops_list->tops_table[i].profile_id);
			return -EINVAL;
		}
	}
	return 0;
}

STATIC bool lpm_profile_check_tops_info(struct lpm_profile_tops_info *tops_info)
{
	int32_t ret;
	uint16_t expect_crc;

	// check magic num
	if (tops_info->magic != LPM_SHAREMEM_PROFILE_MAGIC) {
		lpm_log_err("profile tops info magic error, magic=0x%x, crc=0x%x, tops_num=0x%u\n",
			tops_info->magic, tops_info->crc, tops_info->tops_list.tops_num);
		return false;
	}

	// check crc
	// During the startup process,
	// lp has already written the profile information into the shared memory,
	// and it will not be changed later,
	// so the crc verification fails and will not be retried
	expect_crc = lpm_devmng_crc16(
		(uint8_t *)tops_info, (uint16_t)sizeof(struct lpm_profile_tops_info) - (uint16_t)sizeof(tops_info->crc));
	if (expect_crc != tops_info->crc) {
		lpm_log_warn("profile tops info crc=0x%x error, expect_crc=0x%x, magic=0x%x, tops_num=0x%u\n",
			tops_info->crc, expect_crc, tops_info->magic, tops_info->tops_list.tops_num);
		return false;
	}

	ret = lpm_profile_tops_check(&tops_info->tops_list);
	if (ret != 0) {
		lpm_log_err("profile tops read from sharemem invalid, ret=%d\n", ret);
		return false;
	}

	if (!lpm_profile_check_tops(tops_info->init_profile)) {
		lpm_log_err("profile init tops read from sharemem invalid, init_profile=%u\n",
			tops_info->init_profile);
		return false;
	}

	return true;
}

int32_t lpm_get_profile_from_sharemem(uint32_t dev_id, struct lpm_profile_tops_info *tops_info)
{
	int32_t ret;

	ret = lpm_sharemem_devmng_get_value(dev_id, LPM_SHAREMEM_TYPE_PROFILE, 0,
		(uint8_t *)tops_info, (uint32_t)sizeof(struct lpm_profile_tops_info));
	if (ret != 0) {
		lpm_log_err("read profile tops from sharemem failed, dev_id=%u, ret=%d\n", dev_id, ret);
		return ret;
	}

	if (!lpm_profile_check_tops_info(tops_info)) {
		lpm_log_err("check profile tops read from sharemem failed\n");
		return -EINVAL;
	}

	return 0;
}

int32_t lpm_get_pmu_volt_current_from_sharemem(
	uint32_t dev_id, enum lpm_devmng_volt_current_type volt_current_type,
	struct lpm_devmng_volt_current_info *volt_current)
{
	int32_t ret;
	uint32_t offset = (uint32_t)volt_current_type * (uint32_t)sizeof(struct lpm_devmng_volt_current_info);

	ret = lpm_sharemem_devmng_get_value(dev_id, LPM_SHAREMEM_TYPE_PMU_VOLT_CURRENT, offset,
		(uint8_t *)volt_current, (uint32_t)sizeof(struct lpm_devmng_volt_current_info));
	if (ret != 0) {
		lpm_log_err("read pmu volt current from sharemem failed. dev_id=%u, ret=%d\n", dev_id, ret);
		return ret;
	}

	return 0;
}

int32_t lpm_get_vrd_info_from_sharemem(uint32_t dev_id, struct lpm_device_vrd_status_info *vrd_status)
{
	int32_t ret;

	ret = lpm_sharemem_devmng_get_value(dev_id, LPM_SHAREMEM_TYPE_VRD_STATUS, 0,
		(uint8_t *)vrd_status, (uint32_t)sizeof(struct lpm_device_vrd_status_info));
	if (ret != 0) {
		lpm_log_err("lpm get vrd info from sharemem failed, dev_id=%u, ret=%d\n", dev_id, ret);
		return ret;
	}

	return 0;
}

int32_t lpm_get_temp_threshold_from_sharemem(
	uint32_t dev_id, uint32_t retry_times, uint32_t wait_us, struct lpm_temp_threshold_data *temp_data)
{
	int32_t ret;
	uint32_t i;
	uint32_t expect_crc;

	for (i = 0; i < retry_times; i++) {
		ret = lpm_sharemem_devmng_get_value(dev_id, LPM_SHAREMEM_TYPE_TEMP_THRESHOLD, 0,
			(uint8_t *)temp_data, (uint32_t)sizeof(struct lpm_temp_threshold_data));
		if (ret != 0) {
			lpm_log_err("read temperature threshold from sharemem failed. dev_id=%u, ret=%d\n", dev_id, ret);
			return ret;
		}

		// check crc
		expect_crc = lpm_devmng_crc16((uint8_t *)temp_data,
			(uint16_t)offsetof(struct lpm_temp_threshold_data, crc));
		if (expect_crc != temp_data->crc) {
			// the crc may be error when lp is updating the data,
			// you can wait for a while and try to read again
			lpm_log_warn("check temperature threshold crc from sharemem. read crc=0x%x, expect_crc=0x%x\n",
				temp_data->crc, expect_crc);

			usleep_range(wait_us, wait_us);
		} else {
			return 0; // read data from sharemem and check crc success
		}
	}

	lpm_log_err("read temperature threshold from sharemem failed after retry %u times. dev_id=%u, wait_us=%u\n",
		retry_times, dev_id, wait_us);
	return -EREMOTEIO;
}

STATIC int32_t lpm_read_syscnt_from_sharemem(
	uint32_t dev_id, struct lpm_aic_freq_syscnt_info *syscnt_info, bool *is_retry)
{
	int32_t ret;
	uint32_t expect_crc;
	uint16_t last_index;
	struct lpm_aic_freq_syscnt_crc crc_data = {0};

	*is_retry = false;

	ret = lpm_sharemem_get_value(dev_id, LPM_SHAREMEM_FREQENCY_OFFSET,
		(uint8_t *)syscnt_info, sizeof(struct lpm_aic_freq_syscnt_info));
	if (ret != 0) {
		lpm_log_err("read aic freq syscnt from sharemem failed, dev_id=%u, ret=%d\n", dev_id, ret);
		return ret;
	}

	// check magic num
	if (syscnt_info->magic != LPM_SHAREMEM_AIC_FREQ_SYSCNT_MAGIC) {
		lpm_log_err("read aic freq syscnt magic error, magic=0x%x, freq_index=0x%x, crc=0x%x\n",
			syscnt_info->magic, syscnt_info->freq_index, syscnt_info->crc);
		return -EINVAL;
	}

	// check crc, calculate the crc of the latest frequency and freq_index
	if (syscnt_info->freq_index != LPM_AIC_FREQ_SYSCNT_INVALID_INDEX) {
		if (syscnt_info->freq_index >= LPM_AIC_FREQ_SYSCNT_MAC_NUM) {
			lpm_log_err("read aic freq syscnt freq_index error, magic=0x%x, freq_index=0x%x, crc=0x%x\n",
				syscnt_info->magic, syscnt_info->freq_index, syscnt_info->crc);
			return -EINVAL;
		}
		last_index = (syscnt_info->freq_index +
			LPM_AIC_FREQ_SYSCNT_MAC_NUM - (uint16_t)0x1) % LPM_AIC_FREQ_SYSCNT_MAC_NUM;
	} else {
		// has not data ever
		last_index = 0;
	}

	crc_data.syscnt_freq.freq   = syscnt_info->syscnt[last_index].freq;
	crc_data.syscnt_freq.syscnt = syscnt_info->syscnt[last_index].syscnt;
	crc_data.freq_index         = syscnt_info->freq_index;
	expect_crc = lpm_devmng_crc16((uint8_t *)&crc_data, (uint16_t)sizeof(crc_data));
	if (expect_crc != syscnt_info->crc) {
		// the crc error may be because lp is updating the data,
		// you can wait for a while and try to read again
		*is_retry = true;
		lpm_log_warn("read aic freq syscnt crc error, magic=0x%x, freq_index=0x%x, crc=0x%x, expect_crc=0x%x\n",
			syscnt_info->magic, syscnt_info->freq_index, syscnt_info->crc, expect_crc);
		return 0;
	}

	return 0;
}

int32_t lpm_read_aic_freq_syscnt_from_sharemem(uint32_t dev_id, uint32_t retry_times,
	uint32_t wait_us, struct lpm_aic_freq_syscnt_info *syscnt_info)
{
	int32_t ret;
	bool is_retry = false;
	uint32_t i;

	for (i = 0; i < retry_times; i++) {
		ret = lpm_read_syscnt_from_sharemem(dev_id, syscnt_info, &is_retry);
		if (ret != 0) {
			lpm_log_err("read aic frequency syscnt from sharemem failed, i=%u, ret=%d, dev_id=%u\n",
				i, ret, dev_id);
			return ret;
		}

		if (!is_retry) {
			// crc check success
			return 0;
		}

		// crc check failed, retry
		usleep_range(wait_us, wait_us);
	}

	// if the verification fails for multiple reads, an exception is returned
	lpm_log_err("read aic frequency syscnt from sharemem failed after retry %u times."
		" dev_id=%u, wait_us=%u\n", retry_times, dev_id, wait_us);
	return -EREMOTEIO;
}

#ifdef LPM_POWER_SAMPLING
STATIC int32_t lpm_read_power_sampling_check(uint32_t dev_id)
{
	unsigned long magic_ofst = LPM_SHAREMEM_POWER_SAMPLING_OFFSET +
		offsetof(struct lpm_power_sampling_mem_info, magic);
	uint32_t magic;
	int32_t ret = lpm_sharemem_get_value(dev_id, magic_ofst, (uint8_t *)&magic, sizeof(uint32_t));
	if (ret != 0) {
		lpm_log_err("read power sampling magic failed, dev_id=%u, ret=%d\n", dev_id, ret);
		return ret;
	}
	if (magic != LPM_SHAREMEM_POWER_SAMPLING_MAGIC) {
		lpm_log_err("read power sampling magic=%u invalid ,dev_id=%u\n", magic, dev_id);
		return -EINVAL;
	}
	return 0;
}

STATIC int32_t lpm_read_power_sampling_index(uint32_t dev_id, uint16_t *read_index, uint16_t *write_index)
{
	unsigned long read_index_ofst = LPM_SHAREMEM_POWER_SAMPLING_OFFSET +
		offsetof(struct lpm_power_sampling_mem_info, read_index);
	unsigned long write_index_ofst = LPM_SHAREMEM_POWER_SAMPLING_OFFSET +
		offsetof(struct lpm_power_sampling_mem_info, write_index);

	int32_t ret = lpm_sharemem_get_value(dev_id, read_index_ofst, (uint8_t *)read_index, sizeof(uint16_t));
	if (ret != 0) {
		lpm_log_err("read power sampling read_index failed, dev_id=%u, ret=%d\n", dev_id, ret);
		return ret;
	}

	ret = lpm_sharemem_get_value(dev_id, write_index_ofst, (uint8_t *)write_index, sizeof(uint16_t));
	if (ret != 0) {
		lpm_log_err("read power sampling write_index failed, dev_id=%u, ret=%d\n", dev_id, ret);
		return ret;
	}

	if ((*read_index >= POWER_SAMPLING_DATA_SIZE) || (*write_index >= POWER_SAMPLING_DATA_SIZE)) {
		lpm_log_err("read power sampling read_index=%u write_index=%u invalid ,dev_id=%u\n",
			*read_index, *write_index, dev_id);
		return -EINVAL;
	}
	return 0;
}

STATIC int32_t lpm_read_power_sampling_read_lt_write(uint32_t dev_id,
	struct lpm_power_sampling_usr_info *user_info, uint16_t read_index, uint16_t write_index)
{
	unsigned long power_data_ofst = LPM_SHAREMEM_POWER_SAMPLING_OFFSET +
		offsetof(struct lpm_power_sampling_mem_info, power_data);
	uint32_t copy_count = write_index - read_index;
	int32_t ret = lpm_sharemem_get_value_to_user(dev_id,
		power_data_ofst + (read_index * sizeof(struct lpm_power_sampling_data)),
		(uint8_t __user *)&user_info->power_data[0],
		copy_count * sizeof(struct lpm_power_sampling_data));
	if (ret != 0) {
		lpm_log_err("read power sampling power_data0 failed,"
			"dev_id=%u, ret=%d, read_index=%u, write_index=%u\n",
			dev_id, ret, read_index, write_index);
		return ret;
	}
	user_info->count = copy_count;
	return 0;
}

STATIC int32_t lpm_read_power_sampling_read_gt_write(uint32_t dev_id,
	struct lpm_power_sampling_usr_info *user_info, uint16_t read_index, uint16_t write_index)
{
	int32_t ret;
	unsigned long power_data_ofst = LPM_SHAREMEM_POWER_SAMPLING_OFFSET +
		offsetof(struct lpm_power_sampling_mem_info, power_data);
	uint32_t copy_count = POWER_SAMPLING_DATA_SIZE - read_index - 1;
	if (copy_count != 0) {
		ret = lpm_sharemem_get_value_to_user(dev_id,
			power_data_ofst + (read_index * sizeof(struct lpm_power_sampling_data)),
			(uint8_t __user *)&user_info->power_data[0],
			copy_count * sizeof(struct lpm_power_sampling_data));
		if (ret != 0) {
			lpm_log_err("read power sampling power_data1 failed,"
				"dev_id=%u, ret=%d, read_index=%u, write_index=%u\n",
				dev_id, ret, read_index, write_index);
			return ret;
		}
	}
	user_info->count = copy_count;

	copy_count = write_index + 1;
	ret = lpm_sharemem_get_value_to_user(dev_id,
		power_data_ofst,
		(uint8_t __user *)&user_info->power_data[user_info->count],
		copy_count * sizeof(struct lpm_power_sampling_data));
	if (ret != 0) {
		lpm_log_err("read power sampling power_data3 failed,"
			"dev_id=%u, ret=%d, read_index=%u, write_index=%u\n",
			dev_id, ret, read_index, write_index);
		return ret;
	}
	user_info->count += copy_count;
	return 0;
}

STATIC int32_t lpm_power_sampling_update_read_index(uint32_t dev_id, uint16_t next_index)
{
	unsigned long read_index_ofst = LPM_SHAREMEM_POWER_SAMPLING_OFFSET +
		offsetof(struct lpm_power_sampling_mem_info, read_index);

	return lpm_sharemem_set_value(dev_id, read_index_ofst, (const uint8_t *)&next_index, sizeof(uint16_t));
}

int32_t lpm_read_power_sampling_from_sharemem(uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len)
{
	int32_t ret;
	uint16_t read_index, write_index;

	struct lpm_power_sampling_usr_info user_info = {
		.count = 0,
		.power_data = (struct lpm_power_sampling_data *)in,
	};

	if (in_len < sizeof(struct lpm_power_sampling_data) * POWER_SAMPLING_DATA_SIZE) {
		lpm_log_err("read power sampling in_len=%u invalid\n", in_len);
		return -1;
	}

	ret = lpm_read_power_sampling_check(dev_id);
	if (ret != 0) {
		lpm_log_err("read power sampling check failed, ret=%d\n", ret);
		return ret;
	}

	ret = lpm_read_power_sampling_index(dev_id, &read_index, &write_index);
	if (ret != 0) {
		lpm_log_err("read power sampling index failed, ret=%d\n", ret);
		return ret;
	}

	if (read_index == write_index) {
	    return 0;
	} else if (read_index < write_index) {
		ret = lpm_read_power_sampling_read_lt_write(dev_id, &user_info, read_index, write_index);
		if (ret != 0) {
			lpm_log_err("read power sampling lt failed, ret=%d\n", ret);
			return ret;
		}
	} else {
		ret = lpm_read_power_sampling_read_gt_write(dev_id, &user_info, read_index, write_index);
		if (ret != 0) {
			lpm_log_err("read power sampling gt failed, ret=%d\n", ret);
			return ret;
		}
	}

	ret = lpm_power_sampling_update_read_index(dev_id, write_index);
	if (ret != 0) {
		lpm_log_err("read power sampling set read index failed, dev_id=%u, ret=%d\n", dev_id, ret);
		return ret;
	}
	*out_len += user_info.count * sizeof(struct lpm_power_sampling_data);

	return 0;
}
#endif // LPM_POWER_SAMPLING

int32_t lpm_read_lptest_result_from_sharemem(
	uint32_t dev_id, uint32_t *log_length, uint8_t *log_buffer, uint32_t buf_len)
{
	int32_t ret;

	// empty result log string
	*log_length = 0;
	(void)memset_s(log_buffer, buf_len, 0, buf_len);

	// get result log length, string end '\0' included
	ret = lpm_sharemem_get_value(dev_id, LPM_SHAREMEM_LPTEST_OFFSET + offsetof(struct lpm_lptest_info, length),
		(uint8_t *)log_length, sizeof(uint32_t));
	if (ret != 0) {
		lpm_log_err("get log length from sharemem failed %d\n", ret);
		return ret;
	}

	if ((*log_length > buf_len) || (*log_length == 0)) {
		lpm_log_err("result log length %u invalid\n", *log_length);
		return -EINVAL;
	}

	// copy sharemem result log to global log buffer
	ret = lpm_sharemem_get_value(dev_id, LPM_SHAREMEM_LPTEST_OFFSET + offsetof(struct lpm_lptest_info, result_log),
		log_buffer, *log_length);
	if (ret != 0) {
		lpm_log_err("get lptest_sharemem failed %d\n", ret);
		return ret;
	}

	// update result log length
	log_buffer[*log_length - 1U] = 0; // avoid non-ended string
	return 0;
}

int32_t lpm_save_lptest_cmd_to_sharemem(uint32_t dev_id, const struct lpm_lptest_cmd *lptest_cmd)
{
	int32_t ret;

	// clean ddr share memory
	ret = lpm_sharemem_clear_value(dev_id, LPM_SHAREMEM_LPTEST_OFFSET, sizeof(struct lpm_lptest_info));
	if (ret != 0) {
		lpm_log_err("clear sharemem failed %d\n", ret);
		return ret;
	}

	// copy command and args to ddr share memory
	ret = lpm_sharemem_set_value(dev_id, LPM_SHAREMEM_LPTEST_OFFSET,
		(const uint8_t *)lptest_cmd, sizeof(struct lpm_lptest_cmd));
	if (ret != 0) {
		lpm_log_err("set lp cmd to sharemem failed %d\n", ret);
		return ret;
	}

	return 0;
}

int32_t lpm_save_vrd_upgrade_to_sharemem(
	uint32_t dev_id, const uint8_t *firmware_buf, unsigned long firmware_size)
{
	return lpm_sharemem_set_value(0, LPM_SHAREMEM_VRD_FW_OFFSET, firmware_buf, firmware_size);
}

uint64_t lpm_get_vrd_upgrade_sharemem_addr(void)
{
	return (LPM_SHAREMEM_BASE_ADDR + LPM_SHAREMEM_VRD_FW_OFFSET + LPM_ACCESS_DDR_OFFSET);
}

int32_t lpm_read_aic_freq_adj_from_sharemem(uint32_t dev_id, struct lpm_aic_freq_adj_info *adj_info)
{
	return lpm_sharemem_devmng_get_value(dev_id, LPM_SHAREMEM_TYPE_AIC_FREQ, 0,
		(uint8_t *)adj_info, (uint32_t)sizeof(struct lpm_aic_freq_adj_info));
}
