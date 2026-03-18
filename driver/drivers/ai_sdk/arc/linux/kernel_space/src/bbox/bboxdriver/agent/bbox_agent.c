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

#ifndef BBOX_UT
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/xpc_kernel_inter.h>
#include <linux/spinlock.h>
#include <linux/list.h>
#include <linux/securec.h>
#include <linux/delay.h>

#include "../common/bbox_print.h"
#include "../config/bbox_config.h"
#include "device/bbox_types.h"
#include "device/bbox_pub.h"
#include "bbox_agent.h"
#include "bbox_agent_os.h"
#include "../bbox_platform.h"

STATIC int32_t g_agent_chnl_id = -1;
STATIC struct bbox_agent_msg_list g_agent_msg_list;

s32 bbox_agent_msg_proc(const struct bbox_report_info *info)
{
    struct bbox_agent_msg_node *node = NULL;
    struct bbox_report_info *msg = NULL;
    unsigned long flags;

    node = (struct bbox_agent_msg_node *)bbox_vmalloc(sizeof(struct bbox_agent_msg_node));
    if (node == NULL) {
        BB_PRINT_ERR("bbox vmalloc node failed.\n");
        return BBOX_FAILURE;
    }
    node->msg = (struct bbox_report_info *)bbox_vmalloc(sizeof(struct bbox_report_info));
    if (node->msg == NULL) {
        BB_PRINT_ERR("bbox vmalloc msg failed.\n");
        bbox_vfree(node);
        return BBOX_FAILURE;
    }
    msg = node->msg;
    msg->arg = info->arg;
    msg->devid = info->devid;
    msg->excepid = info->excepid;
    msg->time = info->time;

    spin_lock_irqsave(&g_agent_msg_list.lock, flags);
    list_add_tail(&node->list, &g_agent_msg_list.list);
    spin_unlock_irqrestore(&g_agent_msg_list.lock, flags);

    return BBOX_SUCCESS;
}

STATIC s32 bbox_agent_send_one_msg(struct bbox_agent_msg_node *node)
{
    return xpcshm_write(g_agent_chnl_id, (const char *)node->msg, sizeof(struct bbox_report_info));
}

STATIC void bbox_free_node(struct bbox_agent_msg_node *node)
{
    bbox_vfree(node->msg);
    bbox_vfree(node);
}

STATIC void bbox_agent_send_msg()
{
    s32 ret;
    struct bbox_agent_msg_node *node = NULL;
    struct list_head *pos = NULL;
    struct list_head *next = NULL;
    struct list_head *head = NULL;
    unsigned long flags;

    spin_lock_irqsave(&g_agent_msg_list.lock, flags);

    // if empty, return
    if (list_empty(&g_agent_msg_list.list) != 0) {
        spin_unlock_irqrestore(&g_agent_msg_list.lock, flags);
        return;
    }

    // get first node
    head = &(g_agent_msg_list.list);
    list_for_each_safe(pos, next, head) {
        node = (struct bbox_agent_msg_node *)list_entry(pos, struct bbox_agent_msg_node, list);
        list_del(pos);
        break;
    }
    spin_unlock_irqrestore(&(g_agent_msg_list.lock), flags);

    if (node == NULL) {
        BB_PRINT_ERR("bbox agent msg node is null.");
        return;
    }

    // process node
    ret = bbox_agent_send_one_msg(node);
    if (ret != 0) {
        // if send failed, resend
        spin_lock_irqsave(&(g_agent_msg_list.lock), flags);
        list_add(&node->list, &g_agent_msg_list.list);
        spin_unlock_irqrestore(&(g_agent_msg_list.lock), flags);
        BB_PRINT_INFO("bbox agent xpc write errno(%d)", ret);
    } else {
        bbox_free_node(node);
    }
}

STATIC s32 bbox_agent_xpc_proc_thread_body(void *arg)
{
    s32 ret;
    struct chl_poll_ret pollret[1];

    BB_PRINT_INFO("bbox_agent_xpc thread running.\n");
    UNUSED(arg);

    if (bbox_xpcshm_open_channel(&g_agent_chnl_id) != BBOX_SUCCESS) {
        return BBOX_FAILURE;
    }

    while (!kthread_should_stop()) {
        bbox_agent_send_msg();
        usleep_range(BBOX_WAITTIME_MIN, BBOX_WAITTIME_MAX);
    }
    return BBOX_SUCCESS;
}

STATIC struct task_struct *g_bbox_agent_xpc_thread = NULL;

STATIC s32 bbox_agent_xpc_init()
{
    INIT_LIST_HEAD(&g_agent_msg_list.list);
    spin_lock_init(&g_agent_msg_list.lock);

    g_bbox_agent_xpc_thread = bbox_kthread_proc(bbox_agent_xpc_proc_thread_body, NULL, "bbox_agent_xpc");
    if (g_bbox_agent_xpc_thread == NULL) {
        BB_PRINT_ERR("Bbox agent create thread bbox_agent_xpc thread failed.\n");
        return BBOX_FAILURE;
    }
    return BBOX_SUCCESS;
}

STATIC void bbox_agent_xpc_exit()
{
    bbox_xpchsm_close_channel(g_agent_chnl_id);
    if (g_bbox_agent_xpc_thread != NULL) {
        (void)kthread_stop(g_bbox_agent_xpc_thread);
    }
}

STATIC s32 __init bbox_agent_init(void)
{
    s32 ret;

    ret = bbox_agent_xpc_init();
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("xpc init failed.\n");
        return BBOX_FAILURE;
    }

    ret = bbox_config_init();
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("get config failed.\n");
        bbox_agent_xpc_exit();
        return BBOX_FAILURE;
    }

    ret = bbox_register_init();
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("register init failed.\n");
        bbox_agent_xpc_exit();
        return BBOX_FAILURE;
    }

    ret = bbox_agent_os_init();
    if (ret != BBOX_SUCCESS) {
        BB_PRINT_ERR("os init failed.\n");
        bbox_register_exit();
        bbox_agent_xpc_exit();
        return BBOX_FAILURE;
    }

    BB_PRINT_INFO("bbox agent init sucess.\n");
    return BBOX_SUCCESS;
}

STATIC void __exit bbox_agent_exit(void)
{
    bbox_agent_os_exit();
    bbox_register_exit();
    bbox_agent_xpc_exit();
}

module_init(bbox_agent_init);
module_exit(bbox_agent_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("mntn_bbox_agent");
#else
int bbox_agent_test(void)
{
    return 0;
}
#endif

