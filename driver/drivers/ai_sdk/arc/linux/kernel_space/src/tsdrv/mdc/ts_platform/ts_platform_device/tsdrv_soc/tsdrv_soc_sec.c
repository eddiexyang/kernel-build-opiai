/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
 * Create: 2022-5-6
 */
#include <linux/kthread.h>
#include <linux/delay.h>

#include "hiss/hsm_status.h"
#include "tsdrv_common.h"
#include "devdrv_user_common.h"
#include "tsdrv_log.h"
#include "tsdrv_cpumask.h"
#ifdef AOS_LLVM_BUILD
#include "dms_pg_info.h"
#endif
#define TSDRV_TEE_TASK_WAIT_TIME_MS 10
enum tsdrv_tee_task_flag {
    TEE_THREAD_STOP,
    TEE_THREAD_RUN
};

struct tsdrv_tee_task_thread {
    u32 dev_id;
    atomic_t flag;
    pg_cmd_data query_cmd;
    u64 query_cmd_result;
};

struct tsdrv_tee_task_info {
    struct tsdrv_tee_task_thread thread_data;
    struct task_struct *task;
};

STATIC struct tsdrv_tee_task_info tee_task_info[DEVDRV_MAX_DAVINCI_NUM] = {0};
#ifdef AOS_LLVM_BUILD
int sec_read_pg_info(uint32_t dev_id, pg_cmd_data cmd, uint64_t *out_arg)
{
    return dms_get_pg_info(dev_id, cmd, (u64 *)out_arg);
}
#endif

#if defined(CFG_SOC_PLATFORM_MDC_LITE_ESL) || defined(CFG_SOC_MDC_V51_LITE)
int sec_read_pg_info(uint32_t dev_id, pg_cmd_data cmd, uint64_t *out_arg)
{
    TSDRV_PRINT_INFO("LITE ESL do not support sec. (module:%u).\n", (u32)cmd.module);
    return 0;
}
#endif

STATIC int tsdrv_sec_read_pg_info_thread(void *arg)
{
    struct tsdrv_tee_task_thread *thread_data = (struct tsdrv_tee_task_thread *)arg;
    int ret = sec_read_pg_info(thread_data->dev_id, thread_data->query_cmd, (uint64_t *)&thread_data->query_cmd_result);
    if (ret != 0) {
        TSDRV_PRINT_ERR("dev(%u) sec read core num fail, ret: %d\n", thread_data->dev_id, ret);
    }

    atomic_set(&thread_data->flag, TEE_THREAD_STOP);
    while (!kthread_should_stop())
        msleep(TSDRV_TEE_TASK_WAIT_TIME_MS);

    return ret;
}

int tsdrv_sec_read_pg_info(u32 dev_id, pg_cmd_data cmd, u64 *cmd_ret_val)
{
    struct tsdrv_tee_task_info *info = &tee_task_info[dev_id];
    int ret;

    info->thread_data.dev_id = dev_id;
    info->thread_data.query_cmd = cmd;
    atomic_set(&info->thread_data.flag, TEE_THREAD_RUN);
    info->task = kthread_create(tsdrv_sec_read_pg_info_thread, (void *)&info->thread_data,
        "devdrv_query_kthread_%u", dev_id);
    if (IS_ERR(info->task)) {
        TSDRV_PRINT_ERR("Failed to created the thread for sec_read_pg_info. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    tsdrv_bind_cpu(info->task);
    (void)wake_up_process(info->task);

    while (atomic_read(&info->thread_data.flag) == (int)TEE_THREAD_RUN)
        msleep(TSDRV_TEE_TASK_WAIT_TIME_MS);
    ret = kthread_stop(info->task);
    info->task = NULL;
    *cmd_ret_val = info->thread_data.query_cmd_result;

    return ret;
}

