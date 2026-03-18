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
#include <linux/types.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>
#include <linux/wait.h>
#include <linux/cpumask.h>
#ifndef LOG_UT
#include "drv_cpu_type.h"
#endif
#include "log_drv_dev.h"
#include "log_xpc_mng.h"
#include "log_drv_agent.h"

#ifndef LOG_UT
struct task_struct *g_log_agent_thread = NULL;
static const struct file_operations log_fops = {
    .owner          = THIS_MODULE,
};

int log_dev_reg(void)
{
    int ret;
    ret = register_driver(LOG_DEVICE_NAME, &log_fops, LOG_NONE_ROOT_ACCESS, NULL);
    return ret;
}

void log_dev_unreg(void)
{
    unregister_driver(LOG_DEVICE_NAME);
}
#define LOG_AGENT_WAIT_TS_MAX_CNT 1000
#define LOG_AGENT_WAIT_TS_INTERVAL 10 // 10ms
void log_agent_wait_ts(void)
{
    bool work_flag = false;
    unsigned int cnt = 0;

    while ((work_flag == false) && (cnt < LOG_AGENT_WAIT_TS_MAX_CNT)) {
        work_flag = tsdrv_is_ts_work(0, 0);
        if (work_flag == true) {
            break;
        }

        cnt++;
        msleep(LOG_AGENT_WAIT_TS_INTERVAL);
    }
    if (cnt == LOG_AGENT_WAIT_TS_MAX_CNT) {
        slog_drv_err("TS is not working. (times=%u)\n", cnt);
    } else {
        slog_drv_info("TS work success.\n");
    }
}

void log_work_thread_bind_core(struct task_struct *task)
{
    cpumask_t ctrlcpu_cpumask = {};
    drv_get_ctrlcpu_mask(&ctrlcpu_cpumask);
    if (cpumask_empty(&ctrlcpu_cpumask)) {
        slog_drv_warn("cpumask is empty\n");
        return;
    }

    if (cpumask_available(&ctrlcpu_cpumask)) {
        set_cpus_allowed_ptr(task, &ctrlcpu_cpumask);
    }
}

int log_agent_start_work_thread(void)
{
    g_log_agent_thread = kthread_create(log_agent_work_thread, NULL, "logdrv_agent_task");
    if (IS_ERR(g_log_agent_thread)) {
        slog_drv_err("log agent thread start failed.\n");
        return LOG_RET_ERROR;
    }
    log_work_thread_bind_core(g_log_agent_thread);
    (void)wake_up_process(g_log_agent_thread);
    return 0;
}

void log_agent_stop_work_thread(void)
{
    if (IS_ERR_OR_NULL(g_log_agent_thread)) {
        return;
    }
    kthread_stop(g_log_agent_thread);
}


int __init log_agent_init(void)
{
    int ret;
    ret = log_dev_reg();
    if (ret != 0) {
        slog_drv_err("module register failed, %d\n", ret);
        return ret;
    }

    if (!log_is_ts_valid()) {
        return 0;
    }

    log_agent_wait_ts();
    ret = log_xpc_open_all_channel();
    if (ret != 0) {
        slog_drv_err("log agent open channel err. ret=%d\n", ret);
        return ret;
    }
    ret = log_agent_start_work_thread();
    if (ret != 0) {
        return ret;
    }
    slog_drv_info("Module init success.\n");
    return 0;
}
module_init(log_agent_init);

void __exit log_agent_exit(void)
{
    log_agent_stop_work_thread();
    log_xpc_close_all_channel();
    log_dev_unreg();
    pr_info("Module exit.\n");
}
module_exit(log_agent_exit);

MODULE_LICENSE("GPL v2");
#else
void log_agent_stop_work_thread(void)
{
    return;
}
#endif
