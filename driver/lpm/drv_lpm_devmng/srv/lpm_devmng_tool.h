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
#ifndef LPM_DEVMNG_TOOL_H
#define LPM_DEVMNG_TOOL_H

#include <linux/types.h>
#include <linux/cdev.h>
#include "lpm_devmng_sharemem_drv.h"

#if defined(LPM_LPTEST) && defined(LPM_BUILD_DEBUG)

#ifndef CLASS_UT
#define CLASS_UT class
#endif

#define LPTEST_CMD_NAME_LEN         22  // same as ipc max playload

#define LPTEST_SR_TIMEOUT_MIN       20000000 // 20s
#define LPTEST_SR_TIMEOUT_MAX       21000000 // 21s

// lpm supsend inject type
enum {
	LPM_TEST_FAIL,      // return -1
	LPM_TEST_PANIC,     // kernel panic
	LPM_TEST_TIMEOUT    // wait 20~21s and continue
};

// lpm supsend inject stage
enum {
	LPM_TEST_SUSPEND_PREPARE,
	LPM_TEST_SUSPEND,
	LPM_TEST_SUSPEND_LATE,
	LPM_TEST_SUSPEND_NO_IRQ,
	LPM_TEST_RESUME_NO_IRQ,
	LPM_TEST_RESUME_EARLY,
	LPM_TEST_RESUME,
	LPM_TEST_POST_SUSPEND,
	LPM_TEST_SR_MAX
};

struct lpm_lptest_msg {
	uint32_t in_buff_len;
	uint32_t out_buff_len;
	char *in_buff;
	char *out_buff;
};

#pragma pack(1)
struct lpm_lptest_ipc_data {
	uint8_t cmd_name[LPTEST_CMD_NAME_LEN];
};

struct lpm_lptest_ipc_ack {
	uint32_t result;
};

enum lpm_lptest_quick_cmd_type {
	LPTEST_QUICK_CMD_INVALID = 0,
	LPTEST_QUICK_CMD_POWER_SAMPLING,
	LPTEST_QUICK_CMD_SET_AIC_MAX_FREQ,
};

#define LPM_QUICK_CMD_MAGIC 0x5f // ascii as "_\0\0\0"
struct lpm_lptest_quick_cmd {
	uint32_t magic;
	uint32_t cmd_type;
	uint32_t res_ignore : 1;
	uint32_t reserved : 31;
	uint32_t data_len;
	uint8_t data[0];
};

enum lpm_tool_cmd_type {
	LPM_CMD_TYPE_LPTEST,
	LPM_CMD_TYPE_SUSPEND,
	LPM_CMD_TYPE_QUICK,
	LPM_CMD_TYPE_MAX
};

#define LPM_TOOL_CMD_RAW_SIZE 1024
struct lpm_tool_cmd {
	enum lpm_tool_cmd_type cmd_type;
	uint32_t total_len;
	union {
		uint8_t raw_data[LPM_TOOL_CMD_RAW_SIZE];
		struct lpm_lptest_cmd lptest_cmd;
		struct lpm_lptest_quick_cmd quick_cmd;
	} cmd_data;
};

typedef int32_t (*lpm_tool_cmd_handler)(uint32_t dev_id, const struct lpm_tool_cmd *tool_cmd);

#pragma pack()

struct lpm_lptest_dev_cfg {
	struct cdev cdev;
	struct device *dev;
	struct CLASS_UT *myclass;
	dev_t dev_id;
};

int32_t lpm_lptest_init(uint64_t *param, uint32_t param_num);
int32_t lpm_lptest_exit(uint64_t *param, uint32_t param_num);
int32_t lpm_dsmi_set_lptest(uint32_t dev_id, const char *in, uint32_t in_len);
int32_t lpm_dsmi_get_lptest(uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len);

#else

static inline int32_t lpm_lptest_init(uint64_t *param, uint32_t param_num)
{
	(void)param;
	(void)param_num;
	return 0;
}

static inline int32_t lpm_lptest_exit(uint64_t *param, uint32_t param_num)
{
	(void)param;
	(void)param_num;
	return 0;
}

static inline int32_t lpm_dsmi_set_lptest(uint32_t dev_id, const char *in, uint32_t in_len)
{
	(void)dev_id;
	(void)in;
	(void)in_len;
	return 0;
}

static inline int32_t lpm_dsmi_get_lptest(uint32_t dev_id, char *in, uint32_t in_len, uint32_t *out_len)
{
	(void)dev_id;
	(void)in;
	(void)in_len;
	(void)out_len;
	return 0;
}

#endif
#endif
