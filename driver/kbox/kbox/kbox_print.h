/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: kbox printk info
 * Author: wangshouping
 * Create: 2019-02-14
 */

#ifndef _KBOX_INCLUDE_KBOX_PRINT_H_
#define _KBOX_INCLUDE_KBOX_PRINT_H_

#include <linux/printk.h>
#include <linux/types.h>

#include <asm/atomic.h>

extern atomic_t g_kbox_printk_flag;

#define kbox_info(fmt, ...) do {                                    \
        printk(KERN_INFO "[kbox] " fmt "\n", ##__VA_ARGS__);        \
} while (0)

#define kbox_err(fmt, ...) do {                                     \
        printk(KERN_ERR "[kbox] " fmt "\n", ##__VA_ARGS__);         \
} while (0)

#define kbox_log(fmt, ...) do {                                       \
		atomic_set(&g_kbox_printk_flag, 1);                              \
		printk(KERN_NOTICE "[kbox notice] " fmt "\n", ##__VA_ARGS__);    \
		atomic_set(&g_kbox_printk_flag, 0);                              \
} while (0)

#define kbox_err_ratelimit(fmt, ...) do {                                     \
	printk_ratelimited(KERN_ERR "[kbox] " fmt "\n", ##__VA_ARGS__);         \
} while (0)

#endif /* _KBOX_INCLUDE_KBOX_PRINT_H_ */
