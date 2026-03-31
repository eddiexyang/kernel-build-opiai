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
#include "lpm_devmng_sharemem_v1.h"
#include "lpm_devmng_sharemem_v2.h"
#include "lpm_devmng_sharemem_drv.h"

STATIC int32_t lpm_read_adj_cause_from_sharemem(
	uint32_t dev_id, struct lpm_aic_freq_adj_info *adj_info, bool *is_retry)
{
	int32_t ret;
	uint32_t expect_crc;

	*is_retry = false;

	ret = lpm_read_aic_freq_adj_from_sharemem(dev_id, adj_info);
	if (ret != 0) {
		lpm_log_err("read aic freq adj cause from sharemem failed, ret=%d\n", ret);
		return ret;
	}

	// check magic num
	if (adj_info->magic != LPM_SHAREMEM_AIC_FREQ_ADJ_MAGIC) {
		lpm_log_err("aic freq adj info magic error, "
			"magic=0x%x, size=0x%x, crc=0x%x, reason=0x%llx, sys_time=0x%llx, utc_time=0x%llx\n",
			adj_info->magic, adj_info->size, adj_info->crc,
			adj_info->adj_cause.reason, adj_info->adj_cause.sys_time, adj_info->adj_cause.utc_time);
		return -EINVAL;
	}

	if (adj_info->size > LPM_AIC_FREQ_MAX_INFO_LEN) {
		lpm_log_err("aic freq adj info size error, "
			"magic=0x%x, size=0x%x, crc=0x%x, reason=0x%llx, sys_time=0x%llx, utc_time=0x%llx\n",
			adj_info->magic, adj_info->size, adj_info->crc,
			adj_info->adj_cause.reason, adj_info->adj_cause.sys_time, adj_info->adj_cause.utc_time);
		return -EINVAL;
	}

	if (adj_info->size <= (uint32_t)sizeof(adj_info->crc)) {
		lpm_log_err("aic freq adj info size less than size of crc, "
			"magic=0x%x, size=0x%x, sizeof crc=0x%x\n",
			adj_info->magic, adj_info->size, (uint32_t)sizeof(adj_info->crc));
		return -EINVAL;
	}

	// check crc
	expect_crc = lpm_devmng_crc16(
		(uint8_t *)adj_info, (uint16_t)adj_info->size - (uint16_t)sizeof(adj_info->crc));
	if (expect_crc != adj_info->crc) {
		// the crc error may be because lp is updating the data,
		// you can wait for a while and try to read again
		*is_retry = true;
		lpm_log_warn("aic freq adj info crc=0x%x error, expect_crc=0x%x, "
			"magic=0x%x, size=0x%x, reason=0x%llx, sys_time=0x%llx, utc_time=0x%llx\n",
			adj_info->crc, expect_crc, adj_info->magic, adj_info->size,
			adj_info->adj_cause.reason, adj_info->adj_cause.sys_time, adj_info->adj_cause.utc_time);
		return 0;
	}

	return 0;
}

int32_t lpm_get_adj_cause_from_sharemem(
	uint32_t dev_id, uint32_t retry_times, uint32_t wait_us, struct lpm_aic_freq_adj_cause *adj_cause)
{
	int32_t ret;
	bool is_retry = false;
	uint32_t i;
	struct lpm_aic_freq_adj_info adj_info = {0};

	for (i = 0; i < retry_times; i++) {
		ret = lpm_read_adj_cause_from_sharemem(dev_id, &adj_info, &is_retry);
		if (ret != 0) {
			lpm_log_err("get aic freq adj info failed, i=%u, ret=%d, dev_id=%u\n", i, ret, dev_id);
			return ret;
		}
		if (is_retry) {
			usleep_range(wait_us, wait_us);
			continue;
		}

		ret = memcpy_s(adj_cause, sizeof(*adj_cause), &adj_info.adj_cause, sizeof(adj_info.adj_cause));
		if (ret != 0) {
			lpm_log_err("copy adj cause failed, i=%u, ret=%d, "
				"dev_id=%u, reason=0x%llx, sys_time=0x%llx, utc_time=0x%llx\n",
				i, ret, dev_id, adj_cause->reason, adj_cause->sys_time, adj_cause->utc_time);
			return ret;
		}

		return 0;
	}

	// if the verification fails for multiple reads, an exception is returned
	lpm_log_err("read aic frequency reduce cause from sharemem failed after retry %u times."
		" dev_id=%u, wait_us=%u\n", retry_times, dev_id, wait_us);
	return -EREMOTEIO;
}
