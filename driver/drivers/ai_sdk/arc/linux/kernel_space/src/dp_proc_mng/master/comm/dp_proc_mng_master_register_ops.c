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
#include <linux/fs.h>
#include <linux/slab.h>

#include "devdrv_interface.h"
#include "dp_proc_mng_cmd.h"
#include "dp_proc_mng_register_ops.h"
#include "davinci_api.h"
#include "dp_proc_mng_channel.h"
#include "dp_proc_mng_proc_info.h"

STATIC void dp_process_sign_release(pid_t pid)
{
    struct dp_proc_mng_info *d_info = NULL;
    struct dp_proc_mng_sign *d_sign = NULL;
    struct list_head *pos = NULL;
    struct list_head *n = NULL;

    d_info = dp_proc_get_manager_info();
    if (d_info == NULL) {
        dp_proc_mng_drv_err("d_info is NULL.\n");
        return;
    }
    mutex_lock(&d_info->dp_proc_mng_sign_list_lock);
    if (list_empty_careful(&d_info->hostpid_list_header) == 0) {
        list_for_each_safe(pos, n, &d_info->hostpid_list_header)
        {
            d_sign = list_entry(pos, struct dp_proc_mng_sign, list);
            if (d_sign->hostpid == pid) {
                list_del(&d_sign->list);
                d_info->dp_proc_mng_sign_count[d_sign->docker_id]--;
                dp_proc_mng_drv_info("end dp_proc_mng_get_process_sign, count = %u, docker_id = %u\n",
                    d_info->dp_proc_mng_sign_count[d_sign->docker_id], d_sign->docker_id);

                kfree(d_sign);
                d_sign = NULL;
                break;
            }
        }
    }
    mutex_unlock(&d_info->dp_proc_mng_sign_list_lock);

    return;
}

STATIC int dp_proc_mng_release_prepare(struct file *file_op, unsigned long mode)
{
    dp_process_sign_release(current->tgid);
    return 0;
}

static const struct notifier_operations dp_notifier_ops = {
    .notifier_call = dp_proc_mng_release_prepare,
};

int dp_proc_mng_register_ops_init(void)
{
    int ret;

    ret = drv_ascend_register_notify(DAVINCI_DP_PROC_MNG_SUB_MODULE_NAME, &dp_notifier_ops);
    if (ret != 0) {
        dp_proc_mng_drv_err("Register notify fail,(ret=%d).\n", ret);
        return -ENODEV;
    }

    return 0;
}

void dp_proc_mng_unregister_ops_init(void)
{
}
