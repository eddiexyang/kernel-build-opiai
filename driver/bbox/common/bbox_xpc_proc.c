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
#include <linux/xpc_kernel_inter.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include "bbox_print.h"
#include "bbox_xpc_common.h"
#include "rdr/rdr_core.h"
#include "rdr/rdr_field_core.h"
#include "bbox_config.h"
#include "bootparam/bbox_boot_param.h"
#include "bbox_xpc_proc.h"

STATIC s32 g_xpc_chnl_id = -1;

STATIC s32 bbox_xpc_proc_thread_body(void *arg)
{
    s32 ret;
    struct chl_poll_ret pollret[1];
    BB_PRINT_INFO("bbox_xpc thread running.\n");
    UNUSED(arg);

    if (bbox_xpcshm_open_channel(&g_xpc_chnl_id) == BBOX_FAILURE) {
        return BBOX_FAILURE;
    }

    while (!kthread_should_stop()) {
        ret = bbox_xpc_read_msg(g_xpc_chnl_id, pollret, bbox_xpc_xpcshm_read);
        if (ret < 0 && ret != XPC_ERR_POLL_TIMEOUT && ret != XPC_ERR_POLL_CANCELED_BY_OTHERS) {
            BB_PRINT_ERR("bbox xpc poll failed, errno is %d.\n", ret);
        }
    }
    return BBOX_SUCCESS;
}

void bbox_xpc_xpcshm_read(char *data, u32 len)
{
    struct bbox_report_info *info = (struct bbox_report_info *)data;

    if (len != sizeof(struct bbox_report_info)) {
        BB_PRINT_ERR("len is invalid, len=%u, size=%lu.\n", len, sizeof(struct bbox_report_info));
        return;
    }

    (void)rdr_exception_report(info->devid, info->excepid, &info->time, info->arg);
}

STATIC struct task_struct *g_bbox_xpc_thread = NULL;

s32 bbox_xpc_init(void)
{
    if (bbox_get_aosmode_flag() == AOS_CORE_DISABLE) {
        BB_PRINT_INFO("xpc doesn't need init.\n");
        return BBOX_SUCCESS;
    }

    g_bbox_xpc_thread = bbox_kthread_proc(bbox_xpc_proc_thread_body, NULL, "bbox_xpc");
    if (g_bbox_xpc_thread == NULL) {
        BB_PRINT_ERR("bbox create thread bbox_xpc_thread failed.\n");
        return BBOX_FAILURE;
    }
    BB_PRINT_INFO("bbox_xpc init sucess.\n");
    return BBOX_SUCCESS;
}

void bbox_xpc_exit(void)
{
    bbox_xpchsm_close_channel(g_xpc_chnl_id);
    if (g_bbox_xpc_thread != NULL) {
        (void)kthread_stop(g_bbox_xpc_thread);
    }
}
#else
void bbox_xpc_exit(void)
{
    return;
}
#endif
