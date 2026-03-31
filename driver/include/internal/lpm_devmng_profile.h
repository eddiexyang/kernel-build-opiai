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
#ifndef LPM_DEVMNG_PROFILE_H
#define LPM_DEVMNG_PROFILE_H

#include <linux/types.h>
#include <linux/mutex.h>
#include "lpm_devmng_common.h"
#include "lpm_devmng_ipc.h"
#include "lpm_devmng_flash.h"

// dsmi:config cur profile tops
struct lpm_profile_dsmi_cfg_in {
	uint32_t profile_id; // profile tops index
	uint32_t save_flash; // 0: not save to flash, 1: save to flash
	uint8_t rsv[32];     // invalid data: 0xFF
};

#ifdef LPM_PROFILE

#define LPM_PROFILE_IPC_LIMIT ICM_SUB_CMD_PROFILE_LIMIT

// ipc
struct lpm_profile_ipc_send {
	uint8_t profile_index;
};

struct lpm_profile_ipc_ack {
	uint8_t result; // 0: success, other: failed
};

// query cur profile tops
// and number of currently supported computing power levels
struct lpm_profile_tops_summary {
	uint32_t cur_tops;
	uint32_t tops_num;
};

struct lpm_profile_dev_data {
	bool is_recovery;
	uint32_t cur_tops;
	uint32_t last_flash_tops;
	struct mutex profile_mutex;
};

struct lpm_profile_priv {
	struct lpm_profile_dev_data dev_data[LPM_DEVMNG_DEV_MAX_NUM];
};

int32_t lpm_profile_get_tops(uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len);
int32_t lpm_profile_set_tops(uint32_t dev_id, const char *in, uint32_t in_len);
int32_t lpm_profile_get_tops_detail(uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len);
int32_t lpm_profile_probe(uint64_t *param, uint32_t param_num);
int32_t lpm_profile_remove(uint64_t *param, uint32_t param_num);

#else
static inline int32_t lpm_profile_get_tops(
	uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len)
{
	(void)dev_id;
	(void)in;
	(void)in_len;
	(void)out_len;
	// Operation not supported
	return -EOPNOTSUPP;
}
static inline int32_t lpm_profile_set_tops(uint32_t dev_id, const char *in, uint32_t in_len)
{
	(void)dev_id;
	(void)in;
	(void)in_len;
	// Operation not supported
	return -EOPNOTSUPP;
}
static inline int32_t lpm_profile_get_tops_detail(
	uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len)
{
	(void)dev_id;
	(void)in;
	(void)in_len;
	(void)out_len;
	// Operation not supported
	return -EOPNOTSUPP;
}
static inline int32_t lpm_profile_probe(uint64_t *param, uint32_t param_num)
{
	(void)param;
	(void)param_num;
	return 0;
}
static inline int32_t lpm_profile_remove(uint64_t *param, uint32_t param_num)
{
	(void)param;
	(void)param_num;
	return 0;
}


#endif
#endif