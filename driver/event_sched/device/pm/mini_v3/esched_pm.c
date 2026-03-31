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
 *
 * Description:
 * Author: huawei
 * Create: 2023-01-03
 */

#if !defined(EVENT_SCHED_UT) && !defined(EMU_ST)

#include "esched.h"
#include "esched_drv_adapt.h"
#include "topic_sched.h"
#include "topic_sched_common.h"
#include "topic_sched_chip_def.h"
#include "esched_drv_mia.h"
#include "tsdrv_interface.h"

STATIC int esched_resume_hw_dev(struct sched_numa_node *node)
{
    struct sched_hard_res *hard_res = &node->hard_res;
    struct topic_data_chan *topic_chan = esched_drv_get_topic_chan(node->node_id, NON_SCHED_DEFAULT_CPUID);
    u32 i;
    u32 offset;
    u32 aicpu_chan_id;
    u32 cpu_id;
    int ret;

#ifdef CFG_FEATURE_CDQM
    ret = tsdrv_cdqm_set_topic_id(node->node_id, EVENT_CDQ_MSG);
    if (ret != 0) {
        sched_err("Failed to invoke the tsdrv_cdqm_set_topic_id. (chip_id=%u; ret=%d)\n", node->node_id, ret);
        return ret;
    }
#endif
    topic_sched_set_user_kernel_type(hard_res->io_base, TOPIC_SCHED_CUSTOM_KERNEL_TYPE);
    esched_drv_reset_pool(hard_res);
    esched_drv_init_non_aicpu_pool(hard_res);

    cpu_id = esched_get_cpuid_in_os(node->node_id, NON_SCHED_DEFAULT_CPUID);
    topic_sched_init_cpu_mailbox(hard_res->io_base, topic_chan->mb_id, hard_res->rsv_mem_pa);
    (void)irq_set_affinity_hint((u32)topic_chan->irq, get_cpu_mask(cpu_id));

    esched_drv_init_aicpu_pool(node, hard_res->aicpu_chan_start_id, hard_res->aicpu_chan_num);
    for (i = 0; i < node->hard_res.aicpu_chan_num; i++) {
        aicpu_chan_id = node->hard_res.aicpu_chan_start_id + i;
        cpu_id = esched_get_cpuid_in_os(node->node_id, aicpu_chan_id);
        topic_chan = esched_drv_get_topic_chan(node->node_id, aicpu_chan_id);
        offset = TOPIC_SCHED_PER_CPU_RSV_MEM_SIZE * aicpu_chan_id;
        topic_sched_init_cpu_mailbox(hard_res->io_base, topic_chan->mb_id, hard_res->rsv_mem_pa + offset);
        topic_sched_init_aicpu_get_mailbox(hard_res->io_base, topic_chan->mb_id,
            hard_res->rsv_mem_pa + offset + TOPIC_SCHED_MB_SIZE);
        (void)irq_set_affinity_hint((u32)topic_chan->irq, get_cpu_mask(cpu_id));
    }

    ret = esched_drv_init_comm_pid_mapping(node->node_id);
    if (ret != 0) {
        sched_err("Failed to init comm topic pid table. (chip_id=%u; ret=%d)\n", node->node_id, ret);
        return ret;
    }

    return DRV_ERROR_NONE;
}

STATIC void esched_resume_sched_cpu(struct sched_numa_node *node)
{
    struct sched_hard_res *hard_res = &node->hard_res;
    struct topic_data_chan *topic_chan = NULL;
    u32 cpuid_in_node;
    u32 offset;
    u32 i;

    for (i = 0; i < node->sched_cpu_num; i++) {
        topic_chan = sched_get_cpu_ctx(node, node->sched_cpuid[i])->topic_chan;
        cpuid_in_node = esched_get_cpuid_in_node(node->sched_cpuid[i]);
        offset = ((u32)TOPIC_SCHED_PER_CPU_RSV_MEM_SIZE * cpuid_in_node) +
                  (u32)TOPIC_SCHED_MB_SIZE + (u32)TOPIC_SCHED_MB_SIZE;
        topic_sched_init_cpu_port(hard_res->io_base, topic_chan->cpu_port->port_id,
            hard_res->rsv_mem_pa +(u64)offset, (u32)((u32)topic_chan->cpu_port->depth - 1U));

        (void)irq_set_affinity_hint((u32)topic_chan->irq, get_cpu_mask(node->sched_cpuid[i]));
    }
}

STATIC int esched_cancel_guard_work(u32 chip_id)
{
    struct sched_numa_node *node;
    node = sched_get_numa_node(chip_id);
    if (node == NULL) {
        sched_warn("node is null, (chip_id=%u).\n", chip_id);
        return DRV_ERROR_UNINIT;
    }

    (void)cancel_delayed_work_sync(&node->guard_work);
    return DRV_ERROR_NONE;
}

int esched_pm_shutdown(u32 chip_id)
{
    int ret;
    sched_info("Esched entry shutdown, (chip_id=%u).\n", chip_id);
    ret = esched_cancel_guard_work(chip_id);
    sched_info("Esched shutdown, (chip_id=%u, ret=%d).\n", chip_id, ret);
    return ret;
}
EXPORT_SYMBOL(esched_pm_shutdown);

int esched_pm_suspend(u32 chip_id)
{
    int ret;
    sched_info("Esched entry suspend, (chip_id=%u).\n", chip_id);
    ret = esched_cancel_guard_work(chip_id);
    sched_info("Esched suspend, (chip_id=%u, ret=%d).\n", chip_id, ret);
    return ret;
}
EXPORT_SYMBOL(esched_pm_suspend);

int esched_pm_resume(u32 chip_id)
{
    struct sched_numa_node *node;
    int ret;

    sched_info("Esched entry resume, (chip_id=%u).\n", chip_id);
    node = sched_get_numa_node(chip_id);
    if (node == NULL) {
        sched_warn("node is null, (chip_id=%u).\n", chip_id);
        return DRV_ERROR_UNINIT;
    }

    ret = esched_resume_hw_dev(node);
    if (ret != 0) {
        sched_err("Failed to resume hw device. (chip_id=%u; ret=%d)\n", chip_id, ret);
        return ret;
    }

    if (node->sched_set_cpu_flag == SCHED_VALID) {
        esched_resume_sched_cpu(node);
    }

    (void)schedule_delayed_work_on(0, &node->guard_work, msecs_to_jiffies(SCHED_GUARD_WORK_PERIOD));

    sched_info("Esched resume success, (chip_id=%u).\n", chip_id);
    return DRV_ERROR_NONE;
}
EXPORT_SYMBOL(esched_pm_resume);

#else
int esched_pm_stub()
{
    return 0;
}
#endif

