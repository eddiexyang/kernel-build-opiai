/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
#ifndef LPM_DEBUGFS_BASE_H
#define LPM_DEBUGFS_BASE_H

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
#include <linux/types.h>
#include <linux/printk.h>
#include <linux/refcount.h>
#include <linux/seq_file.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include "lpm_log_base.h"

#define MODULE_LPM_DBG "[DRV_LPM_DBG]"
#ifndef LPM_VPRINTK
#define lpm_dbg_err(...)   (void)printk(KERN_ERR MODULE_LPM_DBG __VA_ARGS__)
#define lpm_dbg_info(...)  (void)printk(KERN_INFO MODULE_LPM_DBG __VA_ARGS__)
#else
#define lpm_dbg_err(...)   (void)lpm_log_vprintk_emit(LOGLEVEL_ERR,  MODULE_LPM_DBG __VA_ARGS__)
#define lpm_dbg_info(...)  (void)lpm_log_vprintk_emit(LOGLEVEL_INFO, MODULE_LPM_DBG __VA_ARGS__)
#endif

// the maximum input length when the user modifies the configuration
#define LPM_DEBUGFS_MAX_USER_INPUT_LEN    100
#define LPM_DEBUGFS_MAX_USER_INPUT_NUM    10

typedef int32_t (*fn_debugfs_open)(struct inode *inode, struct file *file);
typedef ssize_t (*fn_debugfs_write)(
	struct file *file, const char __user *buf, size_t len, loff_t *pos);

struct lpm_debugfs_init_cfg {
	const char *name;
	fn_debugfs_open fn_open;
	fn_debugfs_write fn_write;
};

struct lpm_degbufs_module_cfg {
	struct list_head list_node;
	const char *name;
	struct file_operations fops;
};

struct lpm_debugfs_file_list {
	struct list_head file_head;
	struct mutex file_lock;
};

struct lpm_debugfs_root {
#ifndef LPM_FAULT_RUN_IN_AOS
	struct dentry *debugfs_dir;
#else
	struct proc_dir_entry *debugfs_dir;
#endif
	refcount_t ref_count;
	struct lpm_debugfs_file_list file_list;
};

int32_t lpm_debugfs_get_input(const char __user *buf, size_t len, uint32_t *user_cfg, uint32_t expect_num);
int32_t lpm_debugfs_create(const char *dir_name, struct lpm_debugfs_root *debugfs_root);
int32_t lpm_debugfs_remove(struct lpm_debugfs_root *debugfs_root);
int32_t lpm_debugfs_create_file(struct lpm_debugfs_root *debugfs_root, struct lpm_debugfs_init_cfg *init_cfg);

#endif

#endif