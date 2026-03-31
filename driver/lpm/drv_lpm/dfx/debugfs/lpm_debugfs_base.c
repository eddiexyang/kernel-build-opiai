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

#if defined(CONFIG_DEBUG_FS) && defined(LPM_BUILD_DEBUG)
#ifndef LPM_FAULT_RUN_IN_AOS
#include <linux/debugfs.h>
#else
#include <linux/proc_fs.h>
#endif
#include <linux/kernel.h>
#include <linux/limits.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/securec.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include "lpm_debugfs_base.h"

#ifdef LPM_FAULT_RUN_IN_AOS
static bool is_end_ch(int32_t ch)
{
	return (ch == '\n');
}

static bool is_digit(int32_t ch)
{
	return (ch >= '0') && (ch <= '9');
}

static bool is_xdigit(int32_t ch)
{
	if (is_digit(ch)) {
		return true;
	}

	if ((ch >= 'a') && (ch <= 'f')) {
		return true;
	}

	return (ch >= 'A') && (ch <= 'F');
}

/* Works only for digits and letters, but small and fast */
#define to_lower(x) ((x) | 0x20)

static int32_t simple_guess_base(const char *cp, uint32_t *base)
{
	char tmp_ch;

	if (cp[0] == '0') {
		if (is_end_ch(cp[1])) {
			*base = 10;
			return 0;
		}

		tmp_ch = to_lower(cp[1]);
		if (tmp_ch != 'x') {
			*base = 8;
			return 0;
		}
		if (is_end_ch(cp[2])) {
			lpm_dbg_err("point to end char for hexadecimal\n");
			return -1;
		}

		if (is_xdigit(cp[2])) {
			*base = 16;
			return 0;
		}

		lpm_dbg_err("point to illegal char for hexadecimal, cp[2]=%c.\n", cp[2]);
		return -1;
	} else {
		*base = 10;
		return 0;
	}
}

/**
 * simple_strtoull - convert a string to an unsigned long long
 * @cp: The start of the string
 * @endp: A pointer to the end of the parsed string will be placed here
 * @base: The number base to use
 * aos core not support this api, need to implement it refer to the Linux
 */
static unsigned long long simple_strtoull(const char *cp, char **endp, uint32_t base)
{
	unsigned long long result = 0;
	uint32_t value;
	char tmp_ch;
	uint32_t actual_base = base;
	int32_t ret;

	if (is_end_ch(*cp)) {
		if (endp != NULL) {
			*endp = (char *)cp;
		}
		return 0;
	}

	if (base == 0) {
		ret = simple_guess_base(cp, &actual_base);
		if (ret != 0) {
			if (endp != NULL) {
				*endp = (char *)cp;
			}
			return 0;
		}
	}

	if (actual_base == 16) {
		cp += 2;
	}

	while (is_xdigit(*cp)) {
		tmp_ch = to_lower(*cp);
		value = is_digit(*cp) ? (*cp - '0') : ((tmp_ch - 'a') + 10);
		if (value >= actual_base) {
			break;
		}
		result = (result * actual_base) + value;
		cp++;
	}
	if (endp != NULL) {
		*endp = (char *)cp;
	}
	return result;
}

#endif

static int32_t lpm_debugfs_copy_input(
	const char __user *buf, size_t len, char *input, uint32_t input_len)
{
	if ((len == 0) || (len > input_len)) {
		lpm_dbg_err("get debugfs input failed, len too long or zero, len=%lu\n", len);
		return -1;
	}

	if (copy_from_user(input, buf, len) != 0) {
		lpm_dbg_err("get debugfs input copy from user failed\n");
		return -1;
	}

	// make sure input have null-termination
	input[input_len - (uint32_t)0x1] = '\0';
	return 0;
}

int32_t lpm_debugfs_get_input(
	const char __user *buf, size_t len, uint32_t *user_cfg, uint32_t expect_num)
{
	int32_t ret;
	char input[LPM_DEBUGFS_MAX_USER_INPUT_LEN] = {0};
	unsigned long long tmp_result;
	char *endPtr = NULL;
	char *startPtr = &input[0];
	uint32_t cnt = 0;

	ret = lpm_debugfs_copy_input(buf, len, input, LPM_DEBUGFS_MAX_USER_INPUT_LEN);
	if (ret != 0) {
		lpm_dbg_err("get debugfs input failed\n");
		return -1;
	}

	// internal function, self-guarantee expect_num not more than 10
	while (startPtr <= &input[len - (size_t)1]) {
		tmp_result = simple_strtoull(startPtr, &endPtr, 0);
		if (tmp_result > UINT_MAX) {
			lpm_dbg_err("cnt=%u, input value=0x%llx is too big\n", cnt, tmp_result);
			return -1;
		}
		if (startPtr == endPtr) {
			lpm_dbg_err("cnt=%u, can't get valid input, tmp_result=0x%llx, *startPtr=%c\n",
				cnt, tmp_result, *startPtr);
			return -1;
		}
		if ((*endPtr != ' ') && (*endPtr != '\n')) {
			lpm_dbg_err("cnt=%u, endPtr is not bland or new line, tmp_result=0x%llx, *endPtr=%c\n",
				cnt, tmp_result, *endPtr);
			return -1;
		}

		user_cfg[cnt] = (uint32_t)tmp_result;
		cnt++;
		while ((*endPtr == ' ') && (endPtr <= &input[len - (size_t)1])) {
			endPtr++;
		}

		if (cnt >= expect_num) {
			if (*endPtr != '\n') {
				lpm_dbg_err("too long, input=%s.\n", input);
				return -1;
			} else {
				break;
			}
		}

		startPtr = endPtr;
	}

	if (cnt != expect_num) {
		lpm_dbg_err("input value is too less, cnt=%u, input=%s.\n", cnt, input);
		return -1;
	}

	return 0;
}

static int32_t lpm_debugfs_check_create_param(
	const char *dir_name, struct lpm_debugfs_root *debugfs_root)
{
	if (dir_name == NULL) {
		lpm_dbg_err("lpm failed to create debugfs, dir_name is null.\n");
		return -EINVAL;
	}
	if (debugfs_root == NULL) {
		lpm_dbg_err("lpm failed to create %s debugfs, debugfs_root is null.\n", dir_name);
		return -EINVAL;
	}
	if (debugfs_root->debugfs_dir != NULL) {
		lpm_dbg_err("lpm failed to create %s debugfs, debugfs_dir is not null.\n", dir_name);
		return -EINVAL;
	}

	return 0;
}

static int32_t lpm_debugfs_dir_create(struct lpm_debugfs_root *debugfs_root, const char *dir_name)
{
	refcount_set(&debugfs_root->ref_count, 0);

#ifndef LPM_FAULT_RUN_IN_AOS
	debugfs_root->debugfs_dir = debugfs_create_dir(dir_name, NULL);
#else
	debugfs_root->debugfs_dir = proc_mkdir(dir_name, NULL);
#endif
	if ((debugfs_root->debugfs_dir == NULL) || IS_ERR(debugfs_root->debugfs_dir)) {
		lpm_dbg_err("lpm failed to create %s debugfs directory\n", dir_name);
		return -1;
	}

	refcount_set(&debugfs_root->ref_count, 1);
	lpm_dbg_info("lpm debugfs create %s success\n", dir_name);
	return 0;
}

static void lpm_debugfs_dir_remove(struct lpm_debugfs_root *debugfs_root)
{
	if (refcount_read(&debugfs_root->ref_count) == 1) {
		if (debugfs_root->debugfs_dir != NULL) {
#ifndef LPM_FAULT_RUN_IN_AOS
			debugfs_remove_recursive(debugfs_root->debugfs_dir);
#else
			proc_remove(debugfs_root->debugfs_dir);
#endif
			debugfs_root->debugfs_dir = NULL;
		}
		refcount_set(&debugfs_root->ref_count, 0);
	}

	lpm_dbg_info("lpm debugfs directory remove success\n");
}

static void lpm_debugfs_fill_cfg_with_read(
	struct lpm_degbufs_module_cfg *module_cfg, struct lpm_debugfs_init_cfg *init_cfg)
{
	module_cfg->name         = init_cfg->name;
	module_cfg->fops.owner   = THIS_MODULE,
	module_cfg->fops.open    = init_cfg->fn_open,
	module_cfg->fops.write   = init_cfg->fn_write,
	module_cfg->fops.read    = seq_read;
	module_cfg->fops.llseek  = seq_lseek;
	module_cfg->fops.release = single_release;
}

static void lpm_debugfs_fill_cfg_without_read(
	struct lpm_degbufs_module_cfg *module_cfg, struct lpm_debugfs_init_cfg *init_cfg)
{
	module_cfg->name         = init_cfg->name;
	module_cfg->fops.owner   = THIS_MODULE,
	module_cfg->fops.open    = NULL,
	module_cfg->fops.write   = init_cfg->fn_write,
	module_cfg->fops.read    = NULL;
	module_cfg->fops.llseek  = NULL;
	module_cfg->fops.release = NULL;
}

static int32_t lpm_debugfs_do_create_file(
	struct lpm_debugfs_root *debugfs_root, struct lpm_degbufs_module_cfg *module_cfg)
{
	// create debugfs file
#ifndef LPM_FAULT_RUN_IN_AOS
	const struct dentry *lpm_debug_dentry = NULL;
	lpm_debug_dentry = debugfs_create_file(module_cfg->name, S_IRUSR | S_IWUSR,
		debugfs_root->debugfs_dir, NULL, &module_cfg->fops);
#else
	const struct proc_dir_entry *lpm_debug_dentry = NULL;
	lpm_debug_dentry = proc_create_data(module_cfg->name, S_IRUSR | S_IWUSR,
		debugfs_root->debugfs_dir, &module_cfg->fops, NULL);
#endif
	if ((lpm_debug_dentry == NULL) || IS_ERR(lpm_debug_dentry)) {
		lpm_dbg_err("lpm failed to create %s debugfs file.\n", module_cfg->name);
		return -1;
	}

	return 0;
}

static void lpm_debugfs_init_file_list(struct lpm_debugfs_file_list *file_list)
{
	// init file list node
	mutex_init(&file_list->file_lock);
	INIT_LIST_HEAD(&file_list->file_head);
}

static void lpm_debugfs_free_file_list(struct lpm_debugfs_file_list *file_list)
{
	struct list_head *curr = NULL;
	struct list_head *next = NULL;
	struct lpm_degbufs_module_cfg *node = NULL;

	mutex_lock(&file_list->file_lock);
	list_for_each_safe(curr, next, &file_list->file_head) {
		node = (struct lpm_degbufs_module_cfg *)list_entry(curr, struct lpm_degbufs_module_cfg, list_node);
		list_del(curr);
		kfree(node);
	}
	mutex_unlock(&file_list->file_lock);

	mutex_destroy(&file_list->file_lock);
}

static struct lpm_degbufs_module_cfg *lpm_debugfs_node_find(
	const char *name, struct lpm_debugfs_file_list *file_list)
{
	struct list_head *curr = NULL;
	struct list_head *next = NULL;
	struct lpm_degbufs_module_cfg *node = NULL;

	// attention:
	// the caller should use mutex lock to ensure data safety
	list_for_each_safe(curr, next, &file_list->file_head) {
		node = (struct lpm_degbufs_module_cfg *)list_entry(curr, struct lpm_degbufs_module_cfg, list_node);
		if (strcmp(node->name, name) == 0) {
			return node;
		}
	}
	return NULL;
}

static bool lpm_debugfs_check_file_init_param(
	struct lpm_debugfs_root *debugfs_root, struct lpm_debugfs_init_cfg *init_cfg)
{
	if (debugfs_root == NULL) {
		lpm_dbg_err("debugfs register module failed, debugfs_root is null.\n");
		return false;
	}
	if (debugfs_root->debugfs_dir == NULL) {
		lpm_dbg_err("debugfs register module failed, debugfs_dir is null.\n");
		return false;
	}
	// check if the module name is null
	if (init_cfg == NULL) {
		lpm_dbg_err("debugfs register module failed, init_cfg is null\n");
		return false;
	}

	// check if the module name is null
	if (init_cfg->name == NULL) {
		lpm_dbg_err("debugfs register module failed, module name is null\n");
		return false;
	}

	// check if the module open function hook is null
	// check if the module write function hook is null
	// if both are null, it means no need register
	if ((init_cfg->fn_open == NULL) && (init_cfg->fn_write == NULL)) {
		lpm_dbg_err("debugfs register module=%s failed, fn_open and fn_write null\n", init_cfg->name);
		return false;
	}
	return true;
}

int32_t lpm_debugfs_create_file(
	struct lpm_debugfs_root *debugfs_root, struct lpm_debugfs_init_cfg *init_cfg)
{
	int32_t ret;
	struct lpm_degbufs_module_cfg *module_cfg = NULL;

	if (!lpm_debugfs_check_file_init_param(debugfs_root, init_cfg)) {
		return -EINVAL;
	}

	mutex_lock(&debugfs_root->file_list.file_lock);
	// check is register repeat
	module_cfg = lpm_debugfs_node_find(init_cfg->name, &debugfs_root->file_list);
	if (module_cfg != NULL) {
		mutex_unlock(&debugfs_root->file_list.file_lock);
		lpm_dbg_err("debugfs register module failed, file_name=%s is repeat.\n", init_cfg->name);
		return -EEXIST;
	}

	module_cfg = (struct lpm_degbufs_module_cfg *)kzalloc(sizeof(*module_cfg), GFP_KERNEL);
	if (module_cfg == NULL) {
		mutex_unlock(&debugfs_root->file_list.file_lock);
		lpm_dbg_err("debugfs register file failed, alloc memory fail.\n");
		return -ENOSPC;
	}

	// save module name to check for duplicate registrations of modules
	if (init_cfg->fn_open != NULL) {
		lpm_debugfs_fill_cfg_with_read(module_cfg, init_cfg);
	} else {
		lpm_debugfs_fill_cfg_without_read(module_cfg, init_cfg);
	}

	ret = lpm_debugfs_do_create_file(debugfs_root, module_cfg);
	if (ret != 0) {
		kfree(module_cfg);
		mutex_unlock(&debugfs_root->file_list.file_lock);
		lpm_dbg_err("debugfs register file failed, alloc memory fail.\n");
		return ret;
	}

	list_add(&module_cfg->list_node, &debugfs_root->file_list.file_head);
	mutex_unlock(&debugfs_root->file_list.file_lock);
	lpm_dbg_info("lpm create %s debugfs file success\n", init_cfg->name);
	return 0;
}

int32_t lpm_debugfs_remove(struct lpm_debugfs_root *debugfs_root)
{
	if (debugfs_root == NULL) {
		lpm_dbg_err("lpm failed to remove debugfs directory, debugfs_root is null\n");
		return -EINVAL;
	}

	lpm_debugfs_dir_remove(debugfs_root);
	lpm_debugfs_free_file_list(&debugfs_root->file_list);
	lpm_dbg_info("lpm debugfs directory remove success\n");
	return 0;
}

int32_t lpm_debugfs_create(const char *dir_name, struct lpm_debugfs_root *debugfs_root)
{
	int32_t ret;

	ret = lpm_debugfs_check_create_param(dir_name, debugfs_root);
	if (ret != 0) {
		return ret;
	}
	ret = lpm_debugfs_dir_create(debugfs_root, dir_name);
	if (ret != 0) {
		return ret;
	}
	lpm_debugfs_init_file_list(&debugfs_root->file_list);
	return 0;
}
#endif