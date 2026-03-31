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
#ifndef LPM_DEVMNG_IDLE_ASCEND310B_H
#define LPM_DEVMNG_IDLE_ASCEND310B_H

#include <linux/types.h>
#include "lpm_devmng_common.h"

#ifdef LPM_IDLE

// Wait for 1ms each time, up to 5000 times
#define LPM_IDLE_WAIT_RESUME_CNT 5000
#define LPM_IDLE_WAIT_RESUME_TIME LPM_DELAY_1000_US

#define LPM_IDLE_SYS_NAME_LEN 10

// send the config of 3 subsystems in one ipc messages at most
#define LPM_IDLE_MAX_SYSTEM_EACH_IPC 3U

enum lpm_idle_reg_type {
	LPM_IDLE_STATUS_REG,
	LPM_IDLE_CNT_REG
};

enum lpm_notify_type {
	LPM_IDLE_SUSPEND,
	LPM_IDLE_RESUME
};

enum lpm_idle_config_type {
	LPM_IDLE_CONFIG_EN,
	LPM_IDLE_CONFIG_FORCE_IDLE,
	LPM_IDLE_CONFIG_FORCE_BUSY,
	LPM_IDLE_CONFIG_CYCLE_TH,
	LPM_IDLE_CONFIG_MAX
};

enum lpm_idle_sys_id {
	LPM_IDLE_ISP,
	LPM_IDLE_MEDIA,
	LPM_IDLE_AO,
	LPM_IDLE_MN,
	LPM_IDLE_STARS,
	LPM_IDLE_MATA,
	LPM_IDLE_CPU,
	LPM_IDLE_PERI,
	LPM_IDLE_AIC,
	LPM_IDLE_IO,
	LPM_IDLE_DVPP,
	LPM_IDLE_RING,
	LPM_IDLE_SUBSYSTEMS_MAX
};

struct lpm_idle_config {
	bool valid; // whether dts is configured
	bool idle_en;
	uint16_t force_busy;
	uint16_t force_idle;
	uint16_t idle_cycle_th;
};

struct lpm_idle_subsys_config {
	char sys_name[LPM_IDLE_SYS_NAME_LEN];
	uint8_t support; // 0: not support idle config, 1: support
	uint8_t bit_in_ring;
};

struct lpm_idle_drv_priv {
	uint32_t sys_num;
	struct lpm_idle_subsys_config *subsys_config;
	struct lpm_idle_config user_config[LPM_IDLE_SUBSYSTEMS_MAX];
};

struct lpm_idle_ipc_send_data {
	uint8_t start_id;
	uint8_t sys_num;
	uint16_t idle_en_mask;
	uint16_t force_idle[LPM_IDLE_MAX_SYSTEM_EACH_IPC];
	uint16_t force_busy[LPM_IDLE_MAX_SYSTEM_EACH_IPC];
	uint16_t cycle_th[LPM_IDLE_MAX_SYSTEM_EACH_IPC];
};


struct lpm_idle_ipc_notify_data {
	uint8_t flag; // 0: suspend, 1: resume
};

// ack
struct lpm_idle_ipc_ack {
	uint8_t result; // 0: success, 1: failed
};

#endif
#endif