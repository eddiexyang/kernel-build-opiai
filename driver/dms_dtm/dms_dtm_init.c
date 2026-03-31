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
#include "securec.h"

#include "dms_define.h"
#include "dms_common.h"
#include "dms_probe.h"

#ifndef AOS_LLVM_BUILD
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 20, 0)
#include <linux/namei.h>
#endif
#else
#include <linux/namei.h>
#endif

#define HOST_LOCAL 1000

struct dms_system_ctrl_block *g_dms_system_ccb = NULL;

struct dms_system_ctrl_block* dms_get_sys_ctrl_cb(void)
{
    return g_dms_system_ccb;
}
EXPORT_SYMBOL(dms_get_sys_ctrl_cb);

void dms_free_sys_ctrl_cb(void)
{
    if (g_dms_system_ccb != NULL) {
        kfree(g_dms_system_ccb);
        g_dms_system_ccb = NULL;
    }
}

int dms_check_device_id(int dev_id)
{
    if ((dev_id < 0) || (dev_id >= DEVICE_NUM_MAX)) {
        return -1;
    }
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

struct dms_dev_ctrl_block* dms_get_dev_cb(int dev_id)
{
    struct dms_system_ctrl_block* cb = dms_get_sys_ctrl_cb();
    int ret;
    if (cb == NULL) {
        dms_err("g_dms_system_ccb is NULL. \n");
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
    struct dms_dev_ctrl_block *dev_cb = NULL;

    dev_cb = dms_get_dev_cb(dev_id);
    if (dev_cb == NULL) {
        dms_err("Get dev_ctrl block failed. (dev_id=%d)\n", dev_id);
        return false ;
    }

    if (dev_cb->state != DMS_IN_USED) {
        dms_err("Device state not in used. (dev_id=%d; state=%u)\n", dev_id, dev_cb->state);
        return false ;
    }

    return true;
}
EXPORT_SYMBOL(dms_is_devid_valid);

struct dms_dev_ctrl_block* get_dev_ctrl_block(u32 dev_id)
{
    if (dev_id == HOST_LOCAL) {
        return &g_dms_system_ccb->base_cb;
    }
    if (dev_id >= DEVICE_NUM_MAX) {
        return NULL;
    }
    return &g_dms_system_ccb->dev_cb_table[dev_id];
}

STATIC int dms_init_dev_cb(void)
{
    int i;
    struct dms_dev_ctrl_block* cb;

    g_dms_system_ccb = kzalloc(sizeof(struct dms_system_ctrl_block), GFP_KERNEL | __GFP_ACCOUNT);
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

STATIC void dms_uninit_dev_cb(void)
{
    int i;
    struct dms_dev_ctrl_block* cb;

    cb = &g_dms_system_ccb->base_cb;
    mutex_lock(&cb->node_lock);
    /* destroy nodelist */
    mutex_unlock(&cb->node_lock);
    mutex_destroy(&cb->node_lock);
    INIT_LIST_HEAD(&cb->dev_node_list);

    for (i = 0; i < DEVICE_NUM_MAX; i++) {
        cb = &g_dms_system_ccb->dev_cb_table[i];
        mutex_lock(&cb->node_lock);
        /* destroy nodelist */
        /* exit dev sensor */
        mutex_unlock(&cb->node_lock);
        mutex_destroy(&cb->node_lock);
        INIT_LIST_HEAD(&cb->dev_node_list);
    }

    dms_free_sys_ctrl_cb();
}

STATIC int __init dms_dtm_init(void)
{
    int ret;

    dms_info("dms_dtm_init start.\n");

    ret = dms_init_dev_cb();
    if (ret != 0) {
        dms_err("dms init dev cb failed.(ret=%d)\n", ret);
        return ret;
    }

    dms_info("Dms driver init success.\n");
    return ret;
}

STATIC void __exit dms_dtm_exit(void)
{
    dms_info("dms_dtm_exit start.\n");
    dms_uninit_dev_cb();
    dms_info("Dms dtm driver exit success.\n");
}

module_init(dms_dtm_init);
module_exit(dms_dtm_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("DAVINCI driver");
