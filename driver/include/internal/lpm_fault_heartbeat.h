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
#ifndef LPM_FAULT_HEARTBEAT_H
#define LPM_FAULT_HEARTBEAT_H

#include <linux/types.h>
#include <linux/spinlock.h>
#include <linux/timer.h>
#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
#include <linux/seq_file.h>
#endif
#include "lpm_fault_common.h"

#ifdef LPM_FAULT_HEARTBEAT

#if defined(LPM_SHAREMEM_V1)
// actual length: sizeof(lpm_heartbeat_sm_data)
#define LPM_SHAREMEM_HEARTBEAT_OFFSET  0x21000ULL
#elif defined(LPM_SHAREMEM_V2)
// allocated len: 0x400, actual length: sizeof(lpm_heartbeat_sm_data)
#define LPM_SHAREMEM_HEARTBEAT_OFFSET  0x41C00ULL
#endif

#define LPM_SHAREMEM_HEARTBEAT_MAGIC   0x4C504842 // 'LPHB'
struct lpm_heartbeat_sm_data {
	uint32_t magic_num;
	uint32_t sm_counter;
} __attribute__((packed));

// if the heartbeats ipc does not receive a response for 3 consecutive,
// it is considered that the heartbeat is lost
#define LPM_HEARTBEAT_BROKEN_CNT    3

#define LPM_HEARTBEAT_QUEUE_MAX     10
#define LPM_HEARTBEAT_SEQ_INVALID   0xFFFF

#define LPM_HEARTBEAT_IPC_SEND_FAIL 0x2
#define LPM_HEARTBEAT_IPC_SEND_SUCC 0x1
#define LPM_HEARTBEAT_IPC_NOT_SEND  0x0

#define LPM_HEARTBEAT_MAX_COUNTER     255

enum lpm_heartbeat_status {
	LPM_HEART_NORMAL    = 0x0,
	LPM_HEART_IPC_BLOCK = 0x1,
	LPM_HEART_BROKEN    = 0x2
};

// old version has time_usec_high info,
// but old version ipc not care about the actual length
// and microsecond high 32bit must be 0,
// so delete time_usec_high has no effect
struct lpm_heartbeat_ipc_data {
	uint32_t count;
	uint32_t time_sec_low;
	uint32_t time_sec_high;
	uint32_t time_usec_low;
};

struct lpm_heartbeat_seq {
	uint16_t ipc_counter;
	uint32_t sm_counter;
};

struct lpm_heartbeat_cfg {
	uint32_t dev_id;
	// whether to enable heartbeat detection for dev
	// enabled by default
	bool hb_en;

	// whether to stop heartbeat detection,
	// the default is 0, which means not to stop,
	// set to 0 to not detect when detect heartbeat broken
	// set to 1 when suspend, and return to 0 when resume
	volatile uint8_t stop;
	// the default is 1, which means normal heartbeat detection,
	// set to 0 to not detect when detect heartbeat broken
	volatile uint8_t work;

	// record whether the latest heartbeat ipc is sent successfully
	// 0: has not send ipc, 1: ipc send success, 2: ipc send fail
	volatile uint8_t send_ipc_status;

	// whether there is a heartbeat ipc block, 1: block, 0: normal
	volatile uint8_t ipc_block;

	// whether there is a heartbeat broken, 1: broken, 0: normal
	volatile uint8_t broken;

	// heartbeat ipc message sequence number, incrementing from 0
	// max value is LPM_HEARTBEAT_MAX_COUNTER
	// if the variable type is modified,
	// LPM_HEARTBEAT_MAX_COUNTER needs to be updated synchronously
	volatile uint8_t inc_counter;

	// latest available index in the queue, legal range: 0~9
	// index for queue
	uint8_t queue_ptr;

	// latest available index in the ipc_data, legal range: 0~9
	// index for ipc_data
	uint8_t ipc_data_ptr;

	// record the sequence number of the ipc
	// that driver sent to lpmcu and did not receive a response from lpmcu
	struct lpm_heartbeat_seq queue[LPM_HEARTBEAT_QUEUE_MAX];

	// Last 10 ipc sending timestamps
	struct lpm_heartbeat_ipc_data ipc_data[LPM_HEARTBEAT_QUEUE_MAX];

	// heartbeat detection timer
	struct timer_list hb_timer;
	// time of last detection
	unsigned long last_timestamp;

	spinlock_t hb_spinlock;
};

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
#define LPM_HB_INJECT_ALL_DEV 0xfe

enum lpm_heartbeat_debug_type {
	LPM_HB_UNHANDLE_IPC = 0, // not handle the heartbeat ipc ack
	LPM_HB_UNSEND_IPC   = 1, // not send the heartbeat ipc
	LPM_HB_UNUPDATE_SM  = 2, // not update heartbeat counter read from sharemem
	LPM_HB_FAULT_TYPE_MAX
};

struct lpm_heartbeat_debug_info {
	// specify the dev id which inject fault
	uint32_t dev_id;
	// true: inject fault, false: cancel inject fault
	bool inject_flag;
};

struct lpm_heartbeat_debug_cfg {
	struct lpm_heartbeat_debug_info fault_inject[(uint32_t)LPM_HB_FAULT_TYPE_MAX];
};
#endif

struct lpm_heartbeat_common_cfg {
	// whether to check heartbeat broken when resume
	bool is_resume_check;
	// whether to check heartbeat recovery after broken
	bool is_recovery_check;
	// whether to check heartbeat seq read from sharemem
	bool is_sharemem_check;
};

struct lpm_fault_heartbeat_priv {
	uint32_t dev_num;
	struct lpm_heartbeat_common_cfg common_cfg;
	struct lpm_heartbeat_cfg hb_cfg[LPM_DMS_NODE_MAX_NUM];
#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
	struct lpm_heartbeat_debug_cfg hb_debug_cfg;
#endif
};

int32_t lpm_fault_heartbeat_init(uint32_t dev_num);
int32_t lpm_fault_heartbeat_exit(uint32_t dev_num);

void lpm_hb_task_suspend_prepare(void);
void lpm_hb_task_post_suspend(void);

#else
static inline int32_t lpm_fault_heartbeat_init(uint32_t dev_num)
{
	(void)dev_num;
	return 0;
}
static inline int32_t lpm_fault_heartbeat_exit(uint32_t dev_num)
{
	(void)dev_num;
	return 0;
}

static inline void lpm_hb_task_suspend_prepare(void)
{
}

static inline void lpm_hb_task_post_suspend(void)
{
}

#endif

#endif