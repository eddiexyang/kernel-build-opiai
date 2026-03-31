/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: kbox event type and struct info define
 * Author:
 * Create: 2019-03-07
 */

#ifndef _KBOX_KBOX_EVENT_H_
#define _KBOX_KBOX_EVENT_H_

#include <linux/rtc.h>

#define RESET_NOT_RECORD 0
#define RESET_RECORDED 1
#define RESET_RECORDED_SAVED 2
#define MAX_RESET 32
#define MAX_LOG_CONTENT 32

#define UNKNOWN_STR "unknown"
#define PANIC_STR "panic"
#define DIE_STR "die"
#define OOM_STR "oom"
#define REBOOT_STR "reboot"
#define EMERGE_STR "emerge"
#define RLOCK_STR "rlock"
#define PREINT_STR "preint"
#define FS_EXCEPTION_STR "fs_exception"
#define RESET_INFO_RESERVE_LEN 4

enum log_reset_type {
	TYPE_PROTECT = 0,
	PANIC_TYPE = 1,
	DIE_TYPE = 2,
	OOM_TYPE = 3,
	REBOOT_TYPE = 4,
	EMERGE_TYPE = 5,
	RLOCK_TYPE = 6,
	PREINT_TYPE = 7,
	FS_EXCEPTION_TYPE = 8,
	TYPE_BUTTON
};

struct reset_info {
	unsigned long valid;
	unsigned long ts_nsec;
	struct rtc_time time;
	unsigned int reason;
	unsigned long reserve[RESET_INFO_RESERVE_LEN];
};

enum kbox_die_reason {
	KBOX_DIE_GRACE = 8,
	KBOX_DIE_PANIC,
	KBOX_DIE_CPULOCK
};

#endif
