/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: kbox drv printk info
 * Author: wangshouping
 * Create: 2019-02-14
 */

#ifndef _KBOX_INCLUDE_DRV_PRINT_H_
#define _KBOX_INCLUDE_DRV_PRINT_H_

#include <linux/printk.h>
#include <linux/types.h>

#include <asm/atomic.h>

#include "drv_oper.h"

#define DRV_DEBUG 0

#if DRV_DEBUG
#define drv_debug(fmt, ...) do {                                   \
        atomic_set(&g_drv_printk_flag, 1);                              \
        printk(KERN_DEBUG "[kbox drv debug] " fmt "\n", ##__VA_ARGS__);       \
        atomic_set(&g_drv_printk_flag, 0);                              \
} while (0)
#else
#define drv_debug(fmt, ...) do { } while (0)
#endif

#define drv_info(fmt, ...) do {                                    \
        atomic_set(&g_drv_printk_flag, 1);                              \
        printk(KERN_INFO "[kbox drv] " fmt "\n", ##__VA_ARGS__);        \
        atomic_set(&g_drv_printk_flag, 0);                              \
} while (0)

#define drv_err(fmt, ...) do {                                     \
        atomic_set(&g_drv_printk_flag, 1);                              \
        printk(KERN_ERR "[kbox drv] " fmt "\n", ##__VA_ARGS__);         \
        atomic_set(&g_drv_printk_flag, 0);                              \
} while (0)

#endif /* _KBOX_INCLUDE_DRV_PRINT_H_ */
