/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Author      : wangshouping
 * Create      : 2019-02-14
 * Description : process the kernel reboot timeout
 */

#ifndef __KBOX_REBOOT_TIMER_H
#define __KBOX_REBOOT_TIMER_H
#include <linux/types.h>
extern atomic_t g_reboot_timer_exist;
void reboot_timer_fini(void);
void reboot_timer_init(void);
#define DSTAT_TASK 1
#define ALL_TASK 2
#endif
