/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: kbox common
 * Author:
 * Create: 2019-02-14
 */
#ifndef _KBOX_COMMON_H_
#define _KBOX_COMMON_H_
#include <linux/spinlock.h>
#include <linux/atomic.h>
#define KBOX_DATE_LENGTH 100
#define KBOX_VMALLOC_OP 1
#ifndef unused
#define unused(x)   do { (x) = (x); (void)(x); } while (0)
#endif
#define TIME_STR "kbox_get_time_failed"
#define TIME_STR_LEN (strlen(TIME_STR) + 1)
#define KBOX_STAT_INIT 0
#define KBOX_STAT_WORK 1
#define KBOX_STAT_EXIT 2
extern char *g_die_notify_func;
extern int g_kbox_log_fd;
extern spinlock_t g_kbox_event_lock;
extern atomic_t g_kbox_status;
#endif
