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
*/
#ifndef LPM_DEVMNG_FLASH_H
#define LPM_DEVMNG_FLASH_H

#include <linux/mutex.h>
#include <linux/workqueue.h>
#include "lpm_devmng_common.h"

enum lpm_flash_item_type {
	LPM_FLASH_ITEM_PROFILE,
	LPM_FLASH_ITEM_MAX
};

enum lpm_flash_op_way {
	LPM_FLASH_OP_WAY_DIRECT, // read or write flash direct
	LPM_FLASH_OP_WAY_THREAD, // read or write flash by create thread
	LPM_FLASH_OP_WAY_MAX
};

struct lpm_flash_op_info {
	uint32_t dev_id;
	enum lpm_flash_op_way op_way;
	enum lpm_flash_item_type item_type;
	uint8_t *data;
	uint32_t data_len;
};

typedef int32_t (*fn_lpm_flash_recover)(
	uint32_t dev_id, bool flash_valid, const uint8_t *data, uint32_t data_len);

struct lpm_flash_recover_module {
	enum lpm_flash_item_type item_type;
	fn_lpm_flash_recover fn_recover;
};

#ifdef LPM_FLASH

#define LPM_DEVMNG_FLASH_MAX_LEN 4096 // 4K

enum lpm_flash_op_type {
	LPM_FLASH_OP_TYPE_READ,
	LPM_FLASH_OP_TYPE_WRITE,
	LPM_FLASH_OP_TYPE_MAX
};

struct lpm_flash_profile_data {
	uint32_t valid;
	uint32_t profile_id;
} __attribute__((packed));

// total len should not large than LPM_DEVMNG_FLASH_MAX_LEN
struct lpm_flash_detail {
	struct lpm_flash_profile_data profile;
} __attribute__((packed));

struct lpm_flash_item_cfg {
	uint32_t offset;
	uint32_t len;
};

struct lpm_flash_item_info {
	struct lpm_flash_item_cfg item_cfg[(uint32_t)LPM_FLASH_ITEM_MAX];
};

struct lpm_flash_thread_param {
	enum lpm_flash_op_type op_type;
	struct lpm_flash_op_info op_info;
};

struct lpm_flash_thread_data {
	// kthread status
	atomic_t thread_status;
	// 0: read failed, 1: read success
	bool op_result;
	// whether flash has been configured, focus only on read operations
	bool flash_valid;
	struct lpm_flash_thread_param thread_param;
};

struct lpm_flash_dev_data {
	struct lpm_flash_thread_data thread_data;
	// all flash info, len=LPM_DEVMNG_FLASH_MAX_LEN
	uint8_t *flash_mem;
	struct mutex flash_mutex;
};

struct lpm_flash_recover_task {
	uint32_t module_num;
	struct lpm_flash_recover_module module_cfg[(uint32_t)LPM_FLASH_ITEM_MAX];
	// used to recover profile config from flash
	struct work_struct init_work;
};

struct lpm_flash_priv {
	// false: configuration completed not completed
	// true : configuration recovery completed
	bool recover_status;

	struct lpm_flash_item_info item_info;
	struct lpm_flash_dev_data dev_data[LPM_DEVMNG_DEV_MAX_NUM];
	struct lpm_flash_recover_task recover_task;
};

int32_t lpm_flash_probe(uint64_t *param, uint32_t param_num);
int32_t lpm_flash_remove(uint64_t *param, uint32_t param_num);

// return value:
// 0: read success
// -ENOENT: has not config yet
// other: read failed
// use mutex to protect critical resources, please pay attention when using
int32_t lpm_flash_read(struct lpm_flash_op_info *op_info);

// return value:
// 0: write success
// other: write failed
// use mutex to protect critical resources, please pay attention when using
int32_t lpm_flash_write(struct lpm_flash_op_info *op_info);

void lpm_flash_suspend_prepare(void);
void lpm_flash_post_suspend(void);

// attention: should call before flash probe
int32_t lpm_flash_register_recover_module(const struct lpm_flash_recover_module *module_cfg);

#else
static inline int32_t lpm_flash_probe(uint64_t *param, uint32_t param_num)
{
	// not support
	(void)param;
	(void)param_num;
	return 0;
}
static inline int32_t lpm_flash_remove(uint64_t *param, uint32_t param_num)
{
	// not support
	(void)param;
	(void)param_num;
	return 0;
}

static inline void lpm_flash_suspend_prepare(void)
{
}

static inline void lpm_flash_post_suspend(void)
{
}

#endif

#endif