/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: kbox record panic scene log
 * Author: chenjialong
 * Create: 2019-03-13
 */

#ifndef __KBOX_RLOCK_H
#define __KBOX_RLOCK_H

#include <linux/notifier.h>

#define MAX_CPUS 512
#define INT_BUF_SIZE (MAX_CPUS * 1024)
#define RLOCK_REASON_UNKWN "unknown rlock reason!"

int kbox_rlock_init(void);
void kbox_rlock_fini(void);

extern void kbox_store_cur_safe_buf(int fd);

#endif
