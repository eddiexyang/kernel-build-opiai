/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
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
* Create: 2019-10-15
*/

#ifndef MEMORY_LOG_H
#define MEMORY_LOG_H

#include <linux/sched.h>
#include <linux/printk.h>

#define MODULE_MEMORY "drv_memory"
#define MEMORY_VPRINTK_FACILITY ((int32_t)91) // facility = kernel15// error conditions
#define LOGLEVEL_EMERG      0
#define LOGLEVEL_ALERT      1
#define LOGLEVEL_CRIT       2
#define LOGLEVEL_ERR        3
#define LOGLEVEL_WARNING    4
#define LOGLEVEL_NOTICE     5
#define LOGLEVEL_INFO       6
#define LOGLEVEL_DEBUG      7

#if defined (STATIC_SKIP) || defined (DRV_MEM_GTEST)
#define STATIC
#define memory_drv_err(fmt, ...)
#define memory_drv_warn(fmt, ...)
#define memory_drv_info(fmt, ...)
#define memory_drv_debug(fmt, ...)
#define memory_drv_event(fmt, ...)
#else
#define STATIC static

int32_t drv_memory_vprintk_emit(int32_t level, const char *fmt, ...);
#ifdef DDR_VPRINTK
#define memory_drv_err(fmt, ...) \
	(void)drv_memory_vprintk_emit(LOGLEVEL_ERR, "[ascend] [%s] [%s %d] <%s:%d> " fmt, MODULE_MEMORY,\
		__func__, __LINE__, current->comm, current->tgid, ##__VA_ARGS__)
#define memory_drv_warn(fmt, ...) \
	(void)drv_memory_vprintk_emit(LOGLEVEL_WARNING, "[ascend] [%s] [%s %d] <%s:%d> " fmt, MODULE_MEMORY,\
	__func__, __LINE__, current->comm, current->tgid, ##__VA_ARGS__)
#define memory_drv_info(fmt, ...) \
	(void)drv_memory_vprintk_emit(LOGLEVEL_INFO, "[ascend] [%s] [%s %d] <%s:%d> " fmt, MODULE_MEMORY,\
	__func__, __LINE__, current->comm, current->tgid, ##__VA_ARGS__)
#define memory_drv_event(fmt, ...) \
	(void)drv_memory_vprintk_emit(LOGLEVEL_NOTICE, "[ascend] [%s] [%s %d] <%s:%d> " fmt, MODULE_MEMORY,\
	__func__, __LINE__, current->comm, current->tgid, ##__VA_ARGS__)
#define memory_drv_debug(fmt, ...) \
	(void)drv_memory_vprintk_emit(LOGLEVEL_DEBUG, "[ascend] [%s] [%s %d] <%s:%d> " fmt, MODULE_MEMORY,\
	__func__, __LINE__, current->comm, current->tgid, ##__VA_ARGS__)
#else
#define memory_drv_err(fmt, ...) \
	(void)printk(KERN_ERR "[ascend] [%s] [%s %d] <%s:%d> " fmt, MODULE_MEMORY, __func__, __LINE__, current->comm, \
		current->tgid, ##__VA_ARGS__)
#define memory_drv_warn(fmt, ...) \
	(void)printk(KERN_WARNING "[ascend] [%s] [%s %d] <%s:%d> " fmt, MODULE_MEMORY, __func__, __LINE__, current->comm, \
		current->tgid, ##__VA_ARGS__)
#define memory_drv_info(fmt, ...) \
	(void)printk(KERN_INFO "[ascend] [%s] [%s %d] <%s:%d> " fmt, MODULE_MEMORY, __func__, __LINE__, current->comm, \
		current->tgid, ##__VA_ARGS__)
#define memory_drv_event(fmt, ...) \
	(void)printk(KERN_NOTICE "[ascend] [%s] [%s %d] <%s:%d> " fmt, MODULE_MEMORY, __func__, __LINE__, current->comm, \
		current->tgid, ##__VA_ARGS__)
#define memory_drv_debug(fmt, ...) \
	pr_debug("[ascend] [%s] [%s %d] <%s:%d> " fmt, MODULE_MEMORY, __func__, __LINE__, current->comm, current->tgid, \
		##__VA_ARGS__)
#endif

#endif


#endif
