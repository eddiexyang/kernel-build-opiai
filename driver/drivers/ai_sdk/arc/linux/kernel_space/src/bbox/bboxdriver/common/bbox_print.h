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
 *
 * Description:
 * Author: huawei
 * Create: 2022-08-13
 */

#ifndef BBOX_PRINT_H
#define BBOX_PRINT_H

#include <linux/kern_levels.h>
#include <linux/kernel.h>
#ifdef AOS_LLVM_BUILD
#include <linux/printk.h>
#endif


#ifdef BB_RDR_DEBUG
#define BB_PRINT_PN(args...)        (void)printk(KERN_ERR "[bbox] " args)
#define BB_PRINT_ERR(args...)       (void)printk(KERN_ERR "[bbox] " args)
#define BB_PRINT_WARN(args...)       (void)printk(KERN_WARNING "[bbox] " args)
#define BB_PRINT_INFO(args...)      (void)printk(KERN_INFO "[bbox] " args)
#define BB_PRINT_DBG(args...)       (void)printk(KERN_INFO "[bbox] " args)
#define BB_PRINT_START(args...) \
    (void)printk(KERN_INFO "[bbox] >>>>>enter blackbox %s: %.4d.\n", __func__, __LINE__)
#define BB_PRINT_END(args...) \
    (void)printk(KERN_INFO "[bbox] <<<<<exit  blackbox %s: %.4d.\n", __func__, __LINE__)
#else
#if (defined CFG_FEATURE_MDC_DRV_LOG) && (!defined (BBOX_UT))
#include "drv_smp_os_adapt.h"
#include "drv_log.h"

#define MODULE_BBOX "bbox"

#define BB_PRINT_PN(fmt, ...) \
    drv_err(MODULE_BBOX, "<%s:%d,%d,%u> " fmt, current->comm, current->tgid, current->pid, \
        drv_get_cur_processor_id(), ##__VA_ARGS__)
#define BB_PRINT_ERR(fmt, ...) \
    drv_err(MODULE_BBOX, "<%s:%d,%d,%u> " fmt, current->comm, current->tgid, current->pid, \
        drv_get_cur_processor_id(), ##__VA_ARGS__)
#define BB_PRINT_WARN(fmt, ...) \
    drv_warn(MODULE_BBOX, "<%s:%d,%d,%u> " fmt, current->comm, current->tgid, current->pid, \
        drv_get_cur_processor_id(), ##__VA_ARGS__)
#define BB_PRINT_INFO(fmt, ...) \
    drv_info(MODULE_BBOX, "<%s:%d,%d,%u> " fmt, current->comm, current->tgid, current->pid, \
        drv_get_cur_processor_id(), ##__VA_ARGS__)
#else
#define BB_PRINT_PN(args...)        (void)printk(KERN_ERR "[bbox] " args)
#define BB_PRINT_ERR(args...)       (void)printk(KERN_ERR "[bbox] " args)
#define BB_PRINT_WARN(args...)       (void)printk(KERN_WARNING "[bbox] " args)
#define BB_PRINT_INFO(args...)      (void)printk(KERN_INFO "[bbox] " args)
#endif
#define BB_PRINT_DBG(args...)
#define BB_PRINT_START(args...)
#define BB_PRINT_END(args...)
#endif

#define LOG_INTERVAL 10UL
#define UPDATE_TMSP 1
#define KEEP_TMSP   0

#define BB_CHECK_RET(expr, fmt, ...) do { \
    if (expr) {                           \
        BB_PRINT_ERR(fmt, ##__VA_ARGS__); \
    }                                     \
} while (0)

#define BB_CHECK_EXP_ACT(expr, action, fmt, ...) do { \
    if (expr) {                                       \
        BB_PRINT_ERR(fmt, ##__VA_ARGS__);             \
        action;                                       \
    }                                                 \
} while (0)

#define BB_CHECK_EXP_CTRL(log_level, expr, action, fmt, ...) do { \
    if (expr) {                                                   \
        log_level(fmt, ##__VA_ARGS__);                            \
        action;                                                   \
    }                                                             \
} while (0)

#define BB_CHECK_PTR(p, action, fmt, ...) do { \
    if ((p) == NULL) {                         \
        BB_PRINT_PN(fmt, ##__VA_ARGS__);       \
        action;                                \
    }                                          \
} while (0)

// log print every interval seconds after recorded timestamp
#define BB_LOG_CTRL_INTV(log_level, last_jiffies, interval, update_tmsp, fmt, ...) do {  \
    if (time_after(jiffies, ((last_jiffies) + ((interval) * (u32)HZ)))) {                \
        log_level(fmt, ##__VA_ARGS__);                                                   \
        if (update_tmsp == UPDATE_TMSP) {                                                \
            (last_jiffies) = jiffies;                                                    \
        }                                                                                \
    }                                                                                    \
} while (0)

#define BB_LOG_CTRL_INTV_UPDATE_TMSP(last_jiffies) ((last_jiffies) = jiffies)

#define BB_CHECK_MEMSET(ret, action) do {                                               \
    if ((ret) != EOK) {                                                                 \
        BB_PRINT_ERR("[%s:%4d] memset_s failed with %d.\n", __func__, __LINE__, ret);   \
        action;                                                                         \
    }                                                                                   \
} while (0)

#define BB_CHECK_MEMCPY(ret, action) do {                                               \
    if ((ret) != EOK) {                                                                 \
        BB_PRINT_ERR("[%s:%4d] memcpy_s failed with %d.\n", __func__, __LINE__, ret);   \
        action;                                                                         \
    }                                                                                   \
} while (0)

#define BB_CHECK_STRNCPY(ret, action) do {                                              \
    if ((ret) != EOK) {                                                                 \
        BB_PRINT_ERR("[%s:%4d] strncpy_s failed with %d.\n", __func__, __LINE__, ret);  \
        action;                                                                         \
    }                                                                                   \
} while (0)

#define BB_CHECK_STRCPY(ret, action) do {                                               \
    if ((ret) != EOK) {                                                                 \
        BB_PRINT_ERR("[%s:%4d] strcpy_s failed with %d.\n", __func__, __LINE__, ret);   \
        action;                                                                         \
    }                                                                                   \
} while (0)

#define BB_CHECK_SPRINTF(ret, action) do {                                              \
    if ((ret) == -1) {                                                                  \
        BB_PRINT_ERR("[%s:%4d] sprintf_s failed with %d.\n", __func__, __LINE__, ret);  \
        action;                                                                         \
    }                                                                                   \
} while (0)

#define BB_CHECK_SECUREC(check_func, ret) do {                                          \
    check_func(ret, break);                                                             \
} while (0)

#endif /* End #define BBOX_PRINT_H__ */
