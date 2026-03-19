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

#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/bitops.h>
#include <linux/suspend.h>
#include <linux/notifier.h>
#include <linux/version.h>
#include <linux/list.h>
#include <linux/ioctl.h>
#include <linux/module.h>
#include <linux/atomic.h>
#include <linux/poll.h>
#include <linux/sort.h>
#include <linux/vmalloc.h>

#include "devdrv_user_common.h"
#include "securec.h"
#include "dms_define.h"
#include "dms_init.h"
#include "urd_feature.h"
#include "dms_common.h"
#include "dms_kv.h"
#include "dms_probe.h"
#include "dms_template.h"
#include "dms_basic_info.h"
#include "dms_timer.h"
#if (defined CFG_FEATURE_HOTRESET) || (defined AOS_LLVM_BUILD)
#include "dms_power.h"
#endif
#include "dms_event_adapt.h"
#include "dms_product.h"
#ifndef AOS_LLVM_BUILD
#include "devmng_dms_adapt.h"
#else
#include "dms_pid_map.h"
#endif
#include "davinci_api.h"
#include "dms_sysfs.h"
#ifdef CFG_FEATURE_TIMESYNC
#include "dms_time.h"
#endif
#ifdef CFG_FEATURE_LPM
#include "dms_lpm.h"
#endif
#ifdef CFG_FEATURE_TS
#include "dms_ts_info.h"
#endif
#ifdef CFG_FEATURE_MEM
#include "dms_mem_info.h"
#endif
#ifndef AOS_LLVM_BUILD
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 20, 0)
#include <linux/namei.h>
#endif
#else
#include <linux/namei.h>
#endif

#ifdef CFG_FEATURE_PARTIAL_GOOD
#include "dms_pg_info.h"
#endif

#ifdef CFG_SOC_PLATFORM_MDC_V51
#include "dms_chip_info.h"
#include "dms_bbox.h"
#endif

#if !defined(CFG_HOST_ENV) && defined(CFG_FEATURE_HCCS)
#include "dms_hccs_feature.h"
#endif

#ifdef CFG_FEATURE_HOST_AICPU
#include "dms_host_aicpu_info.h"
#endif

#ifdef CFG_FEATURE_OSC_FREQ
#include "dms_osc_freq.h"
#endif

struct dms_system_ctrl_block *g_dms_system_ccb;

#define MAX_EVENT_CONFIG_SIZE  (2*1024*1024)

struct dms_system_ctrl_block *dms_get_sys_ctrl_cb(void)
{
	return g_dms_system_ccb;
}
EXPORT_SYMBOL(dms_get_sys_ctrl_cb);

static void dms_free_sys_ctrl_cb(void)
{
	kfree(g_dms_system_ccb);
	g_dms_system_ccb = NULL;
}

int dms_check_device_id(int dev_id)
{
	if (dev_id < 0 || dev_id >= DEVICE_NUM_MAX)
		return -1;

	return 0;
}
EXPORT_SYMBOL(dms_check_device_id);

DMS_MODE_T dms_get_rc_ep_mode(void)
{
#ifdef CFG_FEATURE_EP_MODE
	return DMS_EP_MODE;
#else
	return DMS_RC_MODE;
#endif
}
EXPORT_SYMBOL(dms_get_rc_ep_mode);

struct dms_dev_ctrl_block *dms_get_dev_cb(int dev_id)
{
	struct dms_system_ctrl_block *cb = dms_get_sys_ctrl_cb();
	int ret;

	if (cb == NULL) {
		dms_err("g_dms_system_ccb is NULL.\n");
		return NULL;
	}

	ret = dms_check_device_id(dev_id);
	if (ret != 0) {
		dms_err("dev_id out of range. (ret=%d; dev_id=%d)\n", ret, dev_id);
		return NULL;
	}

	return &cb->dev_cb_table[dev_id];
}
EXPORT_SYMBOL(dms_get_dev_cb);

bool dms_is_devid_valid(int dev_id)
{
	struct dms_dev_ctrl_block *dev_cb = dms_get_dev_cb(dev_id);

	if (dev_cb == NULL) {
		dms_err("Get dev_ctrl block failed. (dev_id=%d)\n", dev_id);
		return false;
	}

	if (dev_cb->state != DMS_IN_USED) {
		dms_err("Device state not in used. (dev_id=%d; state=%u)\n",
			dev_id, dev_cb->state);
		return false;
	}

	return true;
}
EXPORT_SYMBOL(dms_is_devid_valid);

/*
 * Some legacy TS/RMS modules still expect a PASID lookup hook from the vendor
 * memory-management stack. On 6.18 we do not have that integration yet, so
 * use the caller TGID as a stable software fallback identifier.
 */
int svm_get_pasid(pid_t vpid, int dev_id)
{
	(void)dev_id;
	return (int)vpid;
}
EXPORT_SYMBOL(svm_get_pasid);

struct dms_dev_ctrl_block *get_dev_ctrl_block(u32 dev_id)
{
	if (dev_id == 1000)
		return &g_dms_system_ccb->base_cb;

	if (dev_id >= DEVICE_NUM_MAX)
		return NULL;

	return &g_dms_system_ccb->dev_cb_table[dev_id];
}

static int dms_init_dev_cb(void)
{
	int i;
	struct dms_dev_ctrl_block *cb;

	g_dms_system_ccb = kzalloc(sizeof(*g_dms_system_ccb),
				   GFP_KERNEL | __GFP_ACCOUNT);
	if (g_dms_system_ccb == NULL) {
		dms_err("Memory alloc for g_dms_system_ccb failed.\n");
		return -ENOMEM;
	}

	cb = &g_dms_system_ccb->base_cb;
	mutex_init(&cb->node_lock);
	INIT_LIST_HEAD(&cb->dev_node_list);

	for (i = 0; i < DEVICE_NUM_MAX; i++) {
		cb = &g_dms_system_ccb->dev_cb_table[i];
		mutex_init(&cb->node_lock);
		INIT_LIST_HEAD(&cb->dev_node_list);
	}

	return 0;
}

static void dms_uninit_dev_cb(void)
{
	int i;
	struct dms_dev_ctrl_block *cb;

	if (g_dms_system_ccb == NULL)
		return;

	cb = &g_dms_system_ccb->base_cb;
	mutex_destroy(&cb->node_lock);
	INIT_LIST_HEAD(&cb->dev_node_list);

	for (i = 0; i < DEVICE_NUM_MAX; i++) {
		cb = &g_dms_system_ccb->dev_cb_table[i];
		mutex_destroy(&cb->node_lock);
		INIT_LIST_HEAD(&cb->dev_node_list);
	}

	dms_free_sys_ctrl_cb();
}

static int read_file_to_buf(size_t file_size, char *config_buf)
{
    size_t read_size;
    int ret = -EIO;
    struct file *src_filp = NULL;
    loff_t offset = 0;

    src_filp = filp_open(EVENT_INFO_CONFIG_PATH, O_RDONLY, S_IRUSR);
    if (IS_ERR(src_filp)) {
        dms_err("unable to open (file: %s, errno = %ld)\n", EVENT_INFO_CONFIG_PATH, PTR_ERR(src_filp));
        goto _end;
    }

#ifndef AOS_LLVM_BUILD
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
    read_size = kernel_read(src_filp, config_buf, file_size, &offset);
#else
    read_size = kernel_read(src_filp, offset, config_buf, file_size);
#endif
#else
    read_size = kernel_read(src_filp, config_buf, file_size, &offset);
#endif
    if (read_size != file_size) {
        dms_err("read file not success. (read bytes=%lu, file_size=%lu)\n", read_size, file_size);
        goto _end;
    }
    ret = 0;

_end:
    if (!IS_ERR(src_filp)) {
        filp_close(src_filp, NULL);
    }

    return ret;
}

static int dms_event_config_verify(u32 event_code, u32 severity, int config_cnt)
{
    int i;

    if (severity > DMS_EVENT_CRITICAL) {
        dms_err("severity invalid. (event_code=0x%x; severity=%u)\n", event_code, severity);
        return -EINVAL;
    }

    for (i = 0; i < config_cnt; i++) {
        if ((event_code == g_event_configs.event_configs[i].event_code) &&
            (severity != g_event_configs.event_configs[i].severity)) {
            dms_err("event_code repeated. (event_code=0x%x; severity=%u)\n", event_code, severity);
            return -EINVAL;
        }
    }

    return 0;
}

static int dms_parse_event_config(const char *line_buf, int size, struct dms_event_config *conf, int *cnt)
{
    int ret;
    u32 event_code, severity;

    if (line_buf == NULL || size == 0) {
        dms_err("line_buf is NULL or size is 0\n");
        return -EINVAL;
    }

    ret = sscanf_s(line_buf, "0x%x %u", &event_code, &severity);
    if (ret != 2) { /* 2: sscanf_s element number */
        dms_err("sscanf_s fail. (ret=%d; linecnt=%d)\n", ret, *cnt);
        return -ENOMEM;
    }

    ret = dms_event_config_verify(event_code, severity, *cnt);
    if (ret != 0) {
        dms_err("dms event_config verify failed. (ret=%d)\n", ret);
        return ret;
    }

    conf->event_code = event_code;
    conf->severity = severity;

    (*cnt)++;
    if (*cnt >= EVENT_INFO_ARRAY_MAX) {
        dms_err("The number of input event config is greater than the max array size. (max_array_size=%d)\n",
            EVENT_INFO_ARRAY_MAX);
        return -EFBIG;
    }

    return 0;
}

#define TMP_BUF_MAX 128
static int get_eventinfo_from_buf(char *config_buf, size_t lBufLens)
{
    int ret;
    int i;
    char *curr = config_buf;
    int cnt = 0;
    int tpcnt = 0;
    char *line_buf = NULL;

    line_buf = kzalloc(TMP_BUF_MAX, GFP_KERNEL | __GFP_ACCOUNT);
    if (line_buf == NULL) {
        dms_err("kzalloc line_buf failed, (size = %d)\n", TMP_BUF_MAX);
        return -ENOMEM;
    }

    for (i = 0; i < (int)lBufLens; i++) {
        if (*curr == '#') { /* handle '#' line */
            while (*(++curr) != '\n') {
                i++;
            }
            continue;
        }

        if ((*curr != '\n') && (*curr != '\r') && (tpcnt < TMP_BUF_MAX)) {
            line_buf[tpcnt++] = *curr++;
            continue;
        }

        if (tpcnt >= TMP_BUF_MAX) {
            dms_err("line length exceed buf size.\n");
            goto free_tmp_buf;
        }

        if (tpcnt == 0) { /* handle empty line */
            curr++;
            continue;
        }

        line_buf[tpcnt] = '\0';
        ret = dms_parse_event_config(line_buf, tpcnt, &g_event_configs.event_configs[cnt], &cnt);
        if (ret != 0) {
            dms_err("dms_parse_event_config fail, (ret = %d, linecnt = %d)\n", ret, cnt);
            goto free_tmp_buf;
        }

        ret = memset_s(line_buf, TMP_BUF_MAX, 0, TMP_BUF_MAX);
        if (ret != 0) {
            dms_err("memset_s fail, (ret = %d)\n", ret);
            goto free_tmp_buf;
        }
        tpcnt = 0;
        curr++;
    }

    g_event_configs.config_cnt = cnt;
    kfree(line_buf);
    return 0;

free_tmp_buf:
    kfree(line_buf);
    return -1;
}

static int cmp(const void *a, const void *b)
{
    return (*(struct dms_event_config*)a).event_code -
        (*(struct dms_event_config*)b).event_code;
}

static int get_file_size(size_t *buf_size)
{
    int ret;
    struct kstat src_stat;

#ifndef AOS_LLVM_BUILD
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 20, 0)
    struct path kernel_path;

    ret = kern_path(EVENT_INFO_CONFIG_PATH, LOOKUP_FOLLOW, &kernel_path);
    if (ret < 0) {
        return ret;
    }
    ret = vfs_getattr(&kernel_path, &src_stat, STATX_BASIC_STATS, AT_NO_AUTOMOUNT);
#else
    mm_segment_t old_fs;

    old_fs = get_fs();
    set_fs(KERNEL_DS);
    ret = vfs_stat(EVENT_INFO_CONFIG_PATH, &src_stat);
    set_fs(old_fs);
#endif
#else
    struct path kernel_path;

    ret = kern_path(EVENT_INFO_CONFIG_PATH, LOOKUP_FOLLOW, &kernel_path);
    if (ret < 0) {
        dms_err("[aos-core] kern_path failed, ret = %d. \n",  ret);
        return ret;
    }

    ret = vfs_getattr(&kernel_path, &src_stat, STATX_SIZE, AT_STATX_SYNC_AS_STAT);
#endif
    if (ret != 0) {
        dms_err("vfs_getattr failed. (file: %s, src_stat.size = %lld, ret = %d)\n",
            EVENT_INFO_CONFIG_PATH, src_stat.size, ret);
        if (ret == -ENOENT) {
            return -ENOENT;
        }
        return ret;
    }
    *buf_size = (size_t)src_stat.size;
    return 0;
}

STATIC int get_eventinfo_from_config(void)
{
    int ret;
    size_t file_size = 0;
    char *config_buf = NULL;
    (void)memset_s(&g_event_configs, sizeof(g_event_configs), 0, sizeof(g_event_configs));

    ret = get_file_size(&file_size);
    if (ret == -ENOENT) {
        dms_err("event config file is not exit.\n");
        return 0;
    }

    if (ret != 0) {
        dms_err("get_file_size fail! (ret = %d)\n", ret);
        return ret;
    }

    if (file_size >= MAX_EVENT_CONFIG_SIZE) {
        dms_err("The file size(%lu) exceeds the upper limit.\n", file_size);
        return -EFBIG;
    }

    config_buf = kzalloc(file_size + 1, GFP_KERNEL | __GFP_ACCOUNT);
    if (config_buf == NULL) {
        dms_err("kzalloc config_buf failed. (size = %lu)\n", file_size);
        return -ENOMEM;
    }
    ret = read_file_to_buf(file_size, config_buf);
    if (ret != 0) {
        dms_err("read file to buf failed, (ret = %d)\n", ret);
        goto undo_acBuf_alloc;
    }
    config_buf[file_size] = '\n';

    ret = get_eventinfo_from_buf(config_buf, file_size + 1);
    if (ret != 0) {
        dms_err("get_event_info from buf failed! (ret =  %d)\n", ret);
        goto undo_acBuf_alloc;
    }

    kfree(config_buf);
    config_buf = NULL;
    sort(g_event_configs.event_configs, g_event_configs.config_cnt,
        sizeof(g_event_configs.event_configs[0]), cmp, NULL);

    return 0;

undo_acBuf_alloc:
    kfree(config_buf);
    config_buf = NULL;
    return ret;
}


STATIC int dms_release_prepare(struct file *file_op, unsigned long mode)
{
    if (mode != NOTIFY_MODE_RELEASE_PREPARE) {
        dms_err("Invalid mode. (mode=%lu).\n", mode);
        return -EINVAL;
    }

    dms_event_release_proc();
    return 0;
}

const struct notifier_operations dms_notifier_ops = {
    .owner = THIS_MODULE,
    .notifier_call = dms_release_prepare,
};

static struct sub_module_ops g_sub_table[] = {
#ifndef AOS_LLVM_BUILD
    {dms_timer_init, dms_timer_uninit},
    {devdrv_manager_init, devdrv_manager_exit},
#endif

#if (defined CFG_FEATURE_HOTRESET) || (defined AOS_LLVM_BUILD)
    {dms_power_init, dms_power_exit},
#endif
#ifdef CFG_FEATURE_LPM
    {dms_lpm_init, dms_lpm_uninit},
#endif
#ifdef CFG_FEATURE_TS
    {dms_ts_init, dms_ts_uninit},
#endif
    {dms_product_init, dms_product_exit},
#ifdef CFG_FEATURE_MEM
    {dms_mem_init, dms_mem_uninit},
#endif
#ifdef CFG_FEATURE_PARTIAL_GOOD
    {init_pg_info_thread, uninit_pg_info_thread},
#endif
#ifdef CFG_FEATURE_HOST_AICPU
    {dms_host_aicpu_init, dms_host_aicpu_exit},
#endif
#ifndef AOS_LLVM_BUILD
#ifdef CFG_SOC_PLATFORM_MDC_V51
    {dms_bist_init, dms_bist_uninit},
    {dms_bbox_init, dms_bbox_uninit},
#endif
#if (defined CFG_FEATURE_OSC_FREQ) \
    && ((defined CFG_HOST_ENV) || (defined CFG_FEATURE_RC_MODE) \
    || (defined CFG_SOC_PLATFORM_MDC_V51) || (defined CFG_SOC_PLATFORM_MDC_V11))
    {osc_freq_init, osc_freq_exit}
#endif
#endif
};

STATIC int dms_init_submodule(void)
{
    int index, ret;
    int table_size = sizeof(g_sub_table) / sizeof(struct sub_module_ops);

    for (index = 0; index < table_size; index++) {
        ret = g_sub_table[index].init();
        if  (ret != 0) {
            goto out;
        }
    }
    return 0;
 out:
    for (; index > 0; index--) {
        g_sub_table[index - 1].uninit();
    }
    return ret;
}

STATIC void dms_exit_submodule(void)
{
    int index;
    int table_size = sizeof(g_sub_table) / sizeof(struct sub_module_ops);

    for (index = table_size; index > 0; index--) {
        g_sub_table[index - 1].uninit();
    }
}

STATIC int __init dms_init(void)
{
    int ret;

    dms_debug("dms_init start.\n");

    ret = drv_ascend_register_notify(DAVINCI_INTF_MODULE_DMS, &dms_notifier_ops);
    if (ret != 0) {
        dms_err("Register notify fail. (ret=%d)\n", ret);
        goto register_notify_fail;
    }

#ifndef CFG_FEATURE_UNSUPPORT_FAULT_MANAGE
    ret = get_eventinfo_from_config();
    if (ret != 0) {
        dms_err("get_eventinfo_from_config failed. (ret=%d)\n", ret);
        goto register_notify_fail;
    }
#endif

	ret = dms_init_dev_cb();
	if (ret != 0) {
		dms_err("init dev cb failed. (ret=%d)\n", ret);
		goto register_notify_fail;
	}

    ret = dms_init_submodule();
    if (ret) {
        dms_err("init sub module failed. (ret=%d)\n", ret);
        goto init_dev_cb_fail;
    }
    dms_event_adapt_init();
    /* Initialize sensor global resources */
#if !defined(CFG_HOST_ENV) && defined(CFG_FEATURE_HCCS)
    dms_hccs_init();
#endif
    CALL_INIT_MODULE(DMS_MODULE_BASIC_INFO);

#ifndef AOS_LLVM_BUILD
    dms_sysfs_init();
#else
    dms_procfs_init();
    dms_pid_map_init();
#endif
    dms_info("Dms driver init success.\n");
    return 0;

init_dev_cb_fail:
	dms_uninit_dev_cb();
register_notify_fail:
    return ret;
}

STATIC void __exit dms_exit(void)
{
#ifndef AOS_LLVM_BUILD
    dms_sysfs_uninit();
#else
    dms_procfs_uninit();
    dms_pid_map_exit();
#endif

    dms_info("dms_exit start.\n");
#if !defined(CFG_HOST_ENV) && defined(CFG_FEATURE_HCCS)
    dms_hccs_exit();
#endif
    CALL_EXIT_MODULE(DMS_MODULE_BASIC_INFO);
    dms_event_adapt_exit();

    dms_exit_submodule();
	dms_uninit_dev_cb();

    (void)drv_ascend_unregister_notify(DAVINCI_INTF_MODULE_DMS);

    dms_info("Dms driver exit success.\n");
    return;
}

module_init(dms_init);
module_exit(dms_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("DAVINCI driver");
