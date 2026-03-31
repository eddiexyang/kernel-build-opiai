/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2023. All rights reserved.
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

#ifndef EVENT_SCHED_UT

#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/vmalloc.h>
#include <asm/io.h>

#include "tsdrv_interface.h"
#include "soc_res.h"
#include "devdrv_interface.h"
#include "hwts_task_info.h"

#include "securec.h"

#include "ascend_hal_define.h"
#include "event_sched_inner.h"

#include "esched_drv_adapt.h"
#include "esched_drv.h"
#include "topic_sched.h"
#include "topic_sched_common.h"
#include "topic_sched_chip_def.h"
#include "user_cfg_interface.h"

#ifdef CFG_SOC_PLATFORM_MDC_V11
#include <linux/aos/cpu_domain_info.h>
#endif

STATIC void esched_drv_intr_clr(struct topic_data_chan *topic_chan);
void esched_cpu_port_reset(struct topic_data_chan *topic_chan);

#define TOPIC_SCHED_HOST_POOL_CCPU_MASK   0x1
#define TOPIC_SCHED_HOST_POOL_CCPU_NUM    1

#ifdef CONFIG_PREEMPT_RT
#define TOPIC_SCHED_IRQ_FLAG IRQF_NO_THREAD
#else
#define TOPIC_SCHED_IRQ_FLAG 0
#endif

#ifdef CFG_SOC_PLATFORM_MDC_V11
#define ESCHED_DRV_CPU_DEFAULT_PRI 1
#endif
u64 esched_drv_get_host_ccpu_mask(u32 pool_id)
{
    return TOPIC_SCHED_HOST_CCPU_MASK & (TOPIC_SCHED_HOST_POOL_CCPU_MASK << (pool_id * TOPIC_SCHED_HOST_POOL_CCPU_NUM));
}

u32 esched_get_cpuid_in_node(u32 cpuid)
{
    u32 chip_id = (u32)cpu_to_node(cpuid);
    u32 cpu_nums_pre = sched_get_previous_cpu_num(chip_id);

    return (cpuid - cpu_nums_pre);
}

u32 esched_get_cpuid_in_os(u32 chip_id, u32 cpuid_in_node)
{
    return  (sched_get_previous_cpu_num(chip_id) + cpuid_in_node);
}

STATIC irqreturn_t esched_drv_task_interrupt(int irq, void *data)
{
    struct topic_data_chan *topic_chan = (struct topic_data_chan *)data;

    esched_drv_intr_clr(topic_chan);
    tasklet_schedule(&topic_chan->sched_task);

    return IRQ_HANDLED;
}

u32 esched_get_devid_from_hw_vfid(u32 chip_id, u32 vfid, u32 sub_dev_num)
{
    /* in pf case, sub_dev_num and vfid both should be 0 */
    if ((sub_dev_num == 0) && (vfid == 0)) {
        return chip_id;
    }

    /* otherwise, in vf case, devid = chip_id * 16 + vfid + 32 */
    return (chip_id * MAX_VF_NUM_PER_DEVICE) + vfid + DEVICE_VF_DEVID_START;
}

u32 esched_get_hw_vfid_from_devid(u32 dev_id)
{
    return (dev_id >= DEVICE_VF_DEVID_START) ? ((dev_id - DEVICE_VF_DEVID_START) % MAX_VF_NUM_PER_DEVICE) : 0U;
}

u32 esched_get_chipid_from_devid(u32 dev_id)
{
    return (dev_id >= DEVICE_VF_DEVID_START) ? ((dev_id - DEVICE_VF_DEVID_START) / MAX_VF_NUM_PER_DEVICE) : dev_id;
}

bool esched_is_phy_dev(u32 dev_id)
{
    return (dev_id >= DEVICE_VF_DEVID_START) ? false : true;
}

int esched_drv_config_pid(struct sched_proc_ctx *proc_ctx, u32 identity, devdrv_host_pids_info_t *pids_info)
{
    struct sched_hard_res *res = NULL;
    int pid_type, ret;
    unsigned int i;

    res = esched_get_hard_res(esched_get_chipid_from_devid(proc_ctx->node->node_id));

    mutex_lock(&res->mutex);
    for (i = 0; i < pids_info->vaild_num; i++) {
        if (pids_info->cp_type[i] == (unsigned int)DEVDRV_PROCESS_USER) {
            continue;
        }

        pid_type = ((pids_info->cp_type[i] == (unsigned int)DEVDRV_PROCESS_CP1) ||
            (pids_info->cp_type[i] == (unsigned int)DEVDRV_PROCESS_QS)) ? DEVICE_STD_PROC : DEVICE_USER_PROC;
        ret = topic_sched_config_pid(res->io_base, identity, (u32)proc_ctx->host_pid, pid_type, proc_ctx->pid);
    }
    mutex_unlock(&res->mutex);
    return ret;
}

void esched_drv_del_pid(struct sched_proc_ctx *proc_ctx, u32 identity)
{
    struct sched_hard_res *res = esched_get_hard_res(esched_get_chipid_from_devid(proc_ctx->node->node_id));

    mutex_lock(&res->mutex);
    topic_sched_del_host_pid(res->io_base, identity, proc_ctx->host_pid, DEVICE_SIDE_SET_PID, proc_ctx->pid);
    mutex_unlock(&res->mutex);
}

int esched_drv_init_topic_table(u32 chip_id, u32 identity)
{
    int ret, i;
    u32 vfid = identity & 0xFFFFU; /* low 16 bit is vfid */
    struct sched_hard_res *res = esched_get_hard_res(chip_id);

    vfid = (vfid >= 1U) ? (vfid - 1U) : vfid;
    mutex_lock(&res->mutex);
    for (i = 0; i < PID_TYPE_MAX; i++) {
        ret = topic_sched_config_pid(res->io_base, identity, vfid, i, (int)vfid);
        if (ret != 0) {
            sched_err("Failed to stub topic table. (chip_id=%u; vfid=%u; identity=%u; ret=%d)\n",
                chip_id, vfid, identity, ret);
            topic_sched_del_host_pid(res->io_base, identity, vfid, DEVICE_SIDE_SET_PID, vfid);
            mutex_unlock(&res->mutex);
            return ret;
        }
    }
    mutex_unlock(&res->mutex);

    return DRV_ERROR_NONE;
}

void esched_drv_uninit_topic_table(u32 chip_id, u32 identity)
{
    u32 vfid = identity & 0xFFFFU; /* low 16 bit is vfid */
    struct sched_hard_res *res = esched_get_hard_res(chip_id);

    vfid = (vfid >= 1U) ? (vfid - 1U) : vfid;
    mutex_lock(&res->mutex);
    topic_sched_del_host_pid(res->io_base, identity, vfid, DEVICE_SIDE_SET_PID, vfid);
    mutex_unlock(&res->mutex);
}

STATIC void esched_drv_intr_clr(struct topic_data_chan *topic_chan)
{
    topic_sched_mb_intr_clr(topic_chan->hard_res->io_base, topic_chan->mb_id);
}

STATIC bool esched_drv_must_report_normal(u32 topic_id)
{
    /* For the CDQM/SPLIT event, the topic requires that no exception be replied. */
    if ((topic_id == (u32)EVENT_CDQ_MSG) || (topic_id == (u32)EVENT_SPLIT_KERNEL)) {
        return true;
    }
    return false;
}

void esched_drv_status_report(struct topic_data_chan *topic_chan, u32 status)
{
#ifdef CFG_SOC_PLATFORM_ESL
    status = 1; /* ESL not surport abnormal report sched, The ESL will suspend. */
#endif

    if (esched_drv_must_report_normal(topic_chan->wait_mb->topic_id)) {
        status = TOPIC_FINISH_STATUS_NORMAL;
    }

    if (((topic_chan->mb_type == ACPU_DEVICE) || (topic_chan->mb_type == DCPU_DEVICE)) &&
        (topic_chan->cpu_port != NULL)) {
        struct topic_sched_cpu_port *cpu_port = topic_chan->cpu_port;

        /* status report should be exclusive with esched_cpu_port_submit_task */
        spin_lock_bh(&cpu_port->lock);
        esched_cpu_port_reset(topic_chan);
        topic_chan->wait_mb_status = TOPIC_SCHED_MB_STATUS_IDLE;
        topic_sched_cpu_status_report(topic_chan->hard_res->io_base, topic_chan->mb_id, status);
        spin_unlock_bh(&cpu_port->lock);
    } else {
        topic_sched_cpu_status_report(topic_chan->hard_res->io_base, topic_chan->mb_id, status);
    }
}

void esched_drv_errcode_report(struct topic_data_chan *topic_chan, u32 error_code)
{
    if (esched_drv_must_report_normal(topic_chan->wait_mb->topic_id)) {
        error_code = 0;
    }

    topic_sched_cpu_errcode_report(topic_chan->hard_res->io_base, topic_chan->mb_id, error_code);
}

void esched_drv_get_status_report(struct topic_data_chan *topic_chan, u32 status)
{
    topic_sched_cpu_get_status_report(topic_chan->hard_res->io_base, topic_chan->mb_id, status);
}

bool esched_drv_is_mb_valid(struct topic_data_chan *topic_chan)
{
    return topic_sched_cpu_is_mb_valid(topic_chan->hard_res->io_base, topic_chan->mb_id);
}

void esched_drv_mb_intr_enable(struct topic_data_chan *topic_chan)
{
    sched_debug("Show details. (mb_id=%u; cpu_type=%u; irq=%d)\n",
        topic_chan->mb_id, topic_chan->mb_type, topic_chan->irq);

    topic_sched_cpu_intr_enable(topic_chan->hard_res->io_base, topic_chan->mb_id);
}

bool esched_drv_is_get_mb_valid(struct topic_data_chan *topic_chan)
{
    return topic_sched_aicpu_is_get_mb_valid(topic_chan->hard_res->io_base, topic_chan->mb_id);
}

STATIC bool esched_drv_is_wait_mb_busy(struct topic_data_chan *topic_chan)
{
    return (topic_chan->wait_mb_status == TOPIC_SCHED_MB_STATUS_BUSY);
}

int esched_cpu_port_submit_task(struct topic_data_chan *topic_chan, void *sqe, u32 timeout)
{
    struct topic_sched_cpu_port *cpu_port = topic_chan->cpu_port;
    u32 head;
    int ret;

    spin_lock_bh(&cpu_port->lock);

    if (!esched_drv_is_wait_mb_busy(topic_chan)) {
        spin_unlock_bh(&cpu_port->lock);
#ifndef EMU_ST
        sched_err("Wait mb is idle. (chip_id=%u; chan_id=%u)\n", topic_chan->hard_res->dev_id, topic_chan->mb_id);
#endif
        return DRV_ERROR_INVALID_VALUE;
    }

    /* sq is full */
    topic_sched_get_cpu_port_head(topic_chan->hard_res->io_base, cpu_port->port_id, &head);
    if ((cpu_port->tail + 1) == cpu_port->depth) {
        spin_unlock_bh(&cpu_port->lock);
        sched_err("Cpu port sq full. (port_id=%u; head=%u; tail=%u depth=%u)\n",
            cpu_port->port_id, head, (u32)cpu_port->tail, (u32)cpu_port->depth);
        return DRV_ERROR_QUEUE_FULL;
    }

    ret = memcpy_s((void *)(uintptr_t)(cpu_port->sq_base + (cpu_port->tail * TOPIC_SCHED_SQE_SIZE)),
        TOPIC_SCHED_SQE_SIZE, sqe, TOPIC_SCHED_SQE_SIZE);
    if (ret != 0) {
        spin_unlock_bh(&cpu_port->lock);
#ifndef EMU_ST
        sched_err("Failed to copy sqe to cpu port. (chip_id=%u; chan_id=%u; ret=%d)\n",
            topic_chan->hard_res->dev_id, topic_chan->mb_id, ret);
#endif
        return DRV_ERROR_MEMORY_OPT_FAIL;
    }

    cpu_port->tail += 1;

    wmb();

    topic_sched_set_cpu_port_tail(topic_chan->hard_res->io_base, cpu_port->port_id, cpu_port->tail);

    /* Re-enable the cpu port for each round of splitting operator. */
    if (cpu_port->status == SCHED_INVALID) {
        topic_sched_enable_cpu_port(topic_chan->hard_res->io_base, cpu_port->port_id);
        cpu_port->status = SCHED_VALID;
    }

    spin_unlock_bh(&cpu_port->lock);

    sched_debug("End of calling esched_cpu_port_submit_task. (port_id=%u; head=%u; tail=%u)\n",
        cpu_port->port_id, head, (u32)cpu_port->tail);

    return 0;
}

void esched_cpu_port_reset(struct topic_data_chan *topic_chan)
{
    struct topic_sched_cpu_port *cpu_port = topic_chan->cpu_port;

    if (cpu_port->status == SCHED_INVALID) {
        return;
    }

    /* disable the cpu port */
    topic_sched_disable_cpu_port(topic_chan->hard_res->io_base, cpu_port->port_id);
    cpu_port->status = SCHED_INVALID;

    topic_sched_clr_cpu_port_task(topic_chan->hard_res->io_base, cpu_port->port_id, topic_chan->mb_id, cpu_port->tail);
    cpu_port->tail = 0;
}

void esched_drv_reset_pool(struct sched_hard_res *res)
{
    u32 i;

    for (i = 0; i < TOPIC_SCHED_POOL_NUM; i++) {
        topic_sched_reset_pool_dev_cpu(res->io_base, i);
    }

    for (i = 0; i < TOPIC_SCHED_HOST_POOL_NUM; i++) {
        topic_sched_reset_pool_host_cpu(res->io_base, i);
    }
}

u32 esched_drv_get_node_aicpu_chan_mask(u32 start_id, u32 aicpu_chan_num)
{
    u32 mask = 0;
    u32 i;

    for (i = start_id; i < start_id + aicpu_chan_num; i++) {
        mask |= (0x1U << i);
    }

    return mask;
}

static u32 esched_drv_get_aicpu_mb_mask(u32 start_id, u32 aicpu_chan_num)
{
    return esched_drv_get_node_aicpu_chan_mask(start_id, aicpu_chan_num);
}

static u32 esched_drv_get_aicpu_slot_mask(u32 start_id, u32 aicpu_chan_num)
{
    return esched_drv_get_node_aicpu_chan_mask(start_id, aicpu_chan_num);
}

static u32 esched_drv_get_non_aicpu_mb_mask(void)
{
    u32 mask = 0;

    mask = (0x1U << TOPIC_SCHED_TSCPU_POOL_CFG_OFFSET);
    mask |= (0x1U << TOPIC_SCHED_DVPPCPU_POOL_CFG_OFFSET);
    mask |= (0x1U << TOPIC_SCHED_CCPU_POOL_CFG_OFFSET);

    return mask;
}

void esched_drv_init_aicpu_pool(struct sched_numa_node *node, u32 start_id, u32 aicpu_chan_num)
{
    struct sched_hard_res *res = esched_get_hard_res(node->node_id);
    u32 mb_mask = esched_drv_get_aicpu_mb_mask(start_id, aicpu_chan_num);
    u32 slot_mask = esched_drv_get_aicpu_slot_mask(start_id, aicpu_chan_num);

    topic_sched_pool_add_mb(res->io_base, TOPIC_SCHED_ACPU_POOL_ID, mb_mask);
    topic_sched_pool_add_aicpu_slot(res->io_base, TOPIC_SCHED_ACPU_POOL_ID, slot_mask);
    /* The slot resources of the rtsq alloced for runtime are assigned to pool 0. */
    topic_sched_pool_add_ccpu_slot(res->io_base, TOPIC_SCHED_ACPU_POOL_ID, TOPIC_SCHED_ACPU_POOL_CCPU_SLOT);

    sched_info("Init aicpu pool. (dev_id=%u; pool_id=%d; mb_mask=0x%x; acpu slot_mask=0x%x; ccpu slot_mask=0x%x)\n",
        res->dev_id, TOPIC_SCHED_ACPU_POOL_ID, mb_mask, slot_mask, TOPIC_SCHED_ACPU_POOL_CCPU_SLOT);
}

STATIC void esched_drv_uninit_aicpu_pool(struct sched_hard_res *res)
{
    topic_sched_reset_pool_dev_cpu(res->io_base, TOPIC_SCHED_ACPU_POOL_ID);
    sched_info("Reset aicpu pool. (dev_id=%u; pool_id=%d)\n", res->dev_id, TOPIC_SCHED_ACPU_POOL_ID);
}

void esched_drv_init_non_aicpu_pool(struct sched_hard_res *res)
{
    u32 mb_mask = esched_drv_get_non_aicpu_mb_mask();
    u32 acpu_slot_mask = TOPIC_SCHED_NON_ACPU_POOL_ACPU_SLOT;
    u32 ccpu_slot_mask = TOPIC_SCHED_NON_ACPU_POOL_CCPU_SLOT;

    topic_sched_pool_add_mb(res->io_base, TOPIC_SCHED_OTHER_CPU_POOL_ID, mb_mask);
    topic_sched_pool_add_aicpu_slot(res->io_base, TOPIC_SCHED_CCPU_SLOT_POOL_ID, acpu_slot_mask);
    topic_sched_pool_add_ccpu_slot(res->io_base, TOPIC_SCHED_CCPU_SLOT_POOL_ID, ccpu_slot_mask);

    sched_info("Init ccpu pool. (dev_id=%u; pool_id=%d mb_mask=0x%x; pool_id=%d acpu_slot_mask=0x%x; "
        "pool_id=%d ccpu_slot_mask=0x%x)\n",
        res->dev_id, TOPIC_SCHED_OTHER_CPU_POOL_ID, mb_mask, TOPIC_SCHED_CCPU_SLOT_POOL_ID, acpu_slot_mask,
        TOPIC_SCHED_CCPU_SLOT_POOL_ID, ccpu_slot_mask);
}

STATIC void esched_drv_uninit_non_aicpu_pool(struct sched_hard_res *res)
{
    u32 mb_mask = esched_drv_get_non_aicpu_mb_mask();
    u32 acpu_slot_mask = TOPIC_SCHED_NON_ACPU_POOL_ACPU_SLOT;
    u32 ccpu_slot_mask = TOPIC_SCHED_NON_ACPU_POOL_CCPU_SLOT;

    topic_sched_pool_del_ccpu_slot(res->io_base, TOPIC_SCHED_CCPU_SLOT_POOL_ID, ccpu_slot_mask);
    topic_sched_pool_del_ccpu_slot(res->io_base, TOPIC_SCHED_CCPU_SLOT_POOL_ID, acpu_slot_mask);
    topic_sched_pool_del_mb(res->io_base, TOPIC_SCHED_OTHER_CPU_POOL_ID, mb_mask);

    sched_info("Uninit ccpu pool. (dev_id=%u; pool_id=%d mb_mask=0x%x; pool_id=%d acpu_slot_mask=0x%x; "
        "pool_id=%d ccpu_slot_mask=0x%x)\n",
        res->dev_id, TOPIC_SCHED_OTHER_CPU_POOL_ID, mb_mask, TOPIC_SCHED_CCPU_SLOT_POOL_ID, acpu_slot_mask,
        TOPIC_SCHED_CCPU_SLOT_POOL_ID, ccpu_slot_mask);
}

STATIC int esched_drv_init_cpu_port(u32 chip_id, u32 chan_id)
{
    struct topic_data_chan *topic_chan = esched_drv_get_topic_chan(chip_id, chan_id);
    u32 offset = (TOPIC_SCHED_PER_CPU_RSV_MEM_SIZE * chan_id) + TOPIC_SCHED_MB_SIZE + TOPIC_SCHED_MB_SIZE;

    topic_chan->cpu_port = (struct topic_sched_cpu_port *)vzalloc(sizeof(struct topic_sched_cpu_port));
    if (topic_chan->cpu_port == NULL) {
#ifndef EMU_ST
        sched_err("Valloc Mem failed. (chip_id=%u; size=0x%lx; chan_id=%u)\n",
            chip_id, sizeof(struct topic_sched_cpu_port), chan_id);
        return DRV_ERROR_OUT_OF_MEMORY;
#endif
    }

    /* enable topic sched sq submit task chan */
    topic_chan->cpu_port->port_id = chan_id;
    topic_chan->cpu_port->status = SCHED_INVALID;
    topic_chan->cpu_port->sq_base = topic_chan->hard_res->rsv_mem_va + offset;
    topic_chan->cpu_port->tail = 0;
    topic_chan->cpu_port->depth = (TOPIC_SCHED_PER_CPU_RSV_MEM_SIZE - (TOPIC_SCHED_MB_SIZE + TOPIC_SCHED_MB_SIZE)) /
        TOPIC_SCHED_SQE_SIZE;
    spin_lock_init(&topic_chan->cpu_port->lock);

    /* According to register description:
       The reserved queue depth = the value configured to the STARS register + 1
     */
    topic_sched_init_cpu_port(topic_chan->hard_res->io_base, topic_chan->cpu_port->port_id,
        topic_chan->hard_res->rsv_mem_pa +(u64)offset, (u32)(topic_chan->cpu_port->depth - 1U));

    sched_debug("Init cpu port success. (chip_id=%u; cpu_port_id=%u)\n", chip_id, chan_id);

    return 0;
}

STATIC void esched_drv_uninit_cpu_port(u32 chip_id, u32 chan_id)
{
    struct topic_data_chan *topic_chan = esched_drv_get_topic_chan(chip_id, chan_id);
    topic_sched_uninit_cpu_port(topic_chan->hard_res->io_base, topic_chan->cpu_port->port_id);
    vfree(topic_chan->cpu_port);
    topic_chan->cpu_port = NULL;
}

STATIC int esched_drv_init_all_cpu_port(u32 chip_id, u32 start_id, u32 chan_num)
{
    u32 i, j;
    int ret;

    for (i = start_id; i < start_id + chan_num; i++) {
        ret = esched_drv_init_cpu_port(chip_id, i);
        if (ret != 0) {
#ifndef EMU_ST
            sched_err("Failed to init cpu port. (chip_id=%u; port_id=%u)\n", chip_id, i);
            for (j = start_id; j < i; j++) {
                esched_drv_uninit_cpu_port(chip_id, j);
            }
            return ret;
#endif
        }
    }

    return 0;
}

STATIC void esched_drv_uninit_all_cpu_port(u32 chip_id, u32 start_id, u32 chan_num)
{
    u32 i;

    for (i = start_id; i < start_id + chan_num; i++) {
        esched_drv_uninit_cpu_port(chip_id, i);
    }
}

int esched_drv_init_ccpu_chan(u32 chip_id)
{
    u32 ccpu_chan_id = NON_SCHED_DEFAULT_CPUID;
    u32 cpu_id = esched_get_cpuid_in_os(chip_id, ccpu_chan_id);
    struct sched_numa_node *node = sched_get_numa_node(chip_id);
    struct topic_data_chan *topic_chan = NULL;
    u32 offset = TOPIC_SCHED_PER_CPU_RSV_MEM_SIZE * ccpu_chan_id;
    int ret;

    topic_chan = esched_drv_get_topic_chan(chip_id, ccpu_chan_id);
    if (topic_chan == NULL) {
        sched_err("Get topic_chan failed. (chip_id=%u, ccpu_chan_id=%u)\n", chip_id, ccpu_chan_id);
        return DRV_ERROR_PARA_ERROR;
    }

    topic_chan->report_flag = SCHED_DRV_REPORT_NONE;
    topic_chan->hard_res = esched_get_hard_res(chip_id);
    topic_chan->serial_no = 0;
    topic_chan->mb_id = ccpu_chan_id;
    topic_chan->irq = topic_chan->hard_res->irq_base + ccpu_chan_id;
    topic_chan->mb_type = CCPU_DEVICE;
    /* Each node has its own cpu_ctx for ccpu, but the index is same in os.
     * Note the creation index of the cpu_ctx. */
    topic_chan->cpu_ctx = (void *)sched_get_cpu_ctx(node, ccpu_chan_id);
    topic_chan->cpu_ctx->topic_chan = topic_chan;

    topic_chan->wait_mb = (struct topic_sched_mailbox *)(topic_chan->hard_res->rsv_mem_va + offset);
    topic_sched_init_cpu_mailbox(topic_chan->hard_res->io_base, topic_chan->mb_id,
        topic_chan->hard_res->rsv_mem_pa + offset);

    tasklet_init(&topic_chan->sched_task, esched_ccpu_sched_task, (uintptr_t)topic_chan);

    ret = request_irq((u32)topic_chan->irq, esched_drv_task_interrupt, TOPIC_SCHED_IRQ_FLAG,
        "topic_sched_ccpu", (void *)topic_chan);
    if (ret != 0) {
        tasklet_kill(&topic_chan->sched_task);
        sched_err("Failed to invoke the request_irq. (chip_id=%u; irq=%d; ret=%d)\n", chip_id, topic_chan->irq, ret);
        return ret;
    }

    (void)irq_set_affinity_hint((u32)topic_chan->irq, get_cpu_mask(cpu_id));

    topic_chan->valid = 1;

    sched_debug("Init ccpu chan success. (chip_id=%u; ccpu_chan_id=%u; cpu_id=%u)\n", chip_id, ccpu_chan_id, cpu_id);

    return 0;
}

STATIC int esched_drv_init_aicpu_chan(u32 chip_id, u32 aicpu_chan_id)
{
    u32 cpu_id = esched_get_cpuid_in_os(chip_id, aicpu_chan_id);
    struct topic_data_chan *topic_chan = NULL;
    u32 offset = TOPIC_SCHED_PER_CPU_RSV_MEM_SIZE * aicpu_chan_id;
    int ret;

    topic_chan = esched_drv_get_topic_chan(chip_id, aicpu_chan_id);
    if (topic_chan == NULL) {
        sched_err("Get topic_chan failed. (chip_id=%u; chan_id=%u)\n", chip_id, aicpu_chan_id);
        return DRV_ERROR_PARA_ERROR;
    }

    topic_chan->report_flag = SCHED_DRV_REPORT_NONE;
    topic_chan->hard_res = esched_get_hard_res(chip_id);
    topic_chan->serial_no = 0;
    topic_chan->mb_id = aicpu_chan_id;
    topic_chan->wait_mb_status = TOPIC_SCHED_MB_STATUS_IDLE;
    topic_chan->irq = topic_chan->hard_res->irq_base + aicpu_chan_id;
    topic_chan->mb_type = ACPU_DEVICE;

    topic_chan->wait_mb = (struct topic_sched_mailbox *)(topic_chan->hard_res->rsv_mem_va + offset);
    topic_sched_init_cpu_mailbox(topic_chan->hard_res->io_base, topic_chan->mb_id,
        topic_chan->hard_res->rsv_mem_pa + offset);

    topic_chan->get_mb = (struct topic_sched_mailbox *)(topic_chan->hard_res->rsv_mem_va + offset +
        TOPIC_SCHED_MB_SIZE);
    topic_sched_init_aicpu_get_mailbox(topic_chan->hard_res->io_base, topic_chan->mb_id,
        topic_chan->hard_res->rsv_mem_pa + offset + TOPIC_SCHED_MB_SIZE);

    tasklet_init(&topic_chan->sched_task, esched_aicpu_sched_task, (uintptr_t)topic_chan);

    ret = request_irq((u32)topic_chan->irq, esched_drv_task_interrupt, TOPIC_SCHED_IRQ_FLAG,
        "topic_sched_aicpu", (void *)topic_chan);
    if (ret != 0) {
        tasklet_kill(&topic_chan->sched_task);
        sched_err("Failed to invoke the request_irq. (chip_id=%u; irq=%d; ret=%d)\n", chip_id, topic_chan->irq, ret);
        return ret;
    }

    (void)irq_set_affinity_hint((u32)topic_chan->irq, get_cpu_mask(cpu_id));

    topic_chan->valid = 1;

    sched_debug("Init aicpu chan success. (chip_id=%u; aicpu_chan_id=%u; cpu_id:%u)\n", chip_id, aicpu_chan_id, cpu_id);

    return 0;
}

STATIC void esched_drv_uninit_aicpu_chan(u32 chip_id, u32 aicpu_chan_id)
{
    struct topic_data_chan *topic_chan = esched_drv_get_topic_chan(chip_id, aicpu_chan_id);

    if (topic_chan->valid == 1) {
        if (topic_chan->cpu_ctx != NULL) {
            topic_chan->cpu_ctx->topic_chan = NULL;
        }
        (void)irq_set_affinity_hint((u32)topic_chan->irq, NULL);
        (void)free_irq((u32)topic_chan->irq, topic_chan);
        tasklet_kill(&topic_chan->sched_task);
        topic_chan->valid = 0;
    }

    sched_debug("Uninit aicpu chan success. (chip_id=%u; aicpu_chan_id=%u)\n", chip_id, aicpu_chan_id);
}

STATIC int esched_drv_init_all_aicpu_chan(u32 chip_id, u32 start_id, u32 aicpu_chan_num)
{
    u32 i, j;
    int ret;

    for (i = start_id; i < start_id + aicpu_chan_num; i++) {
        ret = esched_drv_init_aicpu_chan(chip_id, i);
        if (ret != 0) {
#ifndef EMU_ST
            for (j = start_id; j < i; j++) {
                esched_drv_uninit_aicpu_chan(chip_id, j);
            }
            return ret;
#endif
        }
    }

    return 0;
}

STATIC void esched_drv_uninit_all_aicpu_mb(u32 chip_id, u32 start_id, u32 aicpu_chan_num)
{
    u32 i;

    for (i = start_id; i < start_id + aicpu_chan_num; i++) {
        esched_drv_uninit_aicpu_chan(chip_id, i);
    }
}

int esched_drv_init_comm_pid_mapping(u32 node_id)
{
#if defined(CFG_FEATURE_NO_BIND_SCHED) || !defined(CFG_FEATURE_SIA_MAP_TOPIC_TABLE)
    return esched_drv_init_topic_table(node_id, 0);
#else
    return DRV_ERROR_NONE;
#endif
}

STATIC void esched_drv_uninit_comm_pid_mapping(u32 node_id)
{
#if defined(CFG_FEATURE_NO_BIND_SCHED) || !defined(CFG_FEATURE_SIA_MAP_TOPIC_TABLE)
    esched_drv_uninit_topic_table(node_id, 0);
#endif
}

STATIC void esched_drv_uninit_ccpu_chan(u32 chip_id)
{
    struct topic_data_chan *topic_chan = esched_drv_get_topic_chan(chip_id, NON_SCHED_DEFAULT_CPUID);

    if (topic_chan->valid == SCHED_VALID) {
        topic_chan->cpu_ctx->topic_chan = NULL;
        (void)irq_set_affinity_hint((u32)topic_chan->irq, NULL);
        (void)free_irq((u32)topic_chan->irq, topic_chan);
        tasklet_kill(&topic_chan->sched_task);
        topic_chan->valid = 0;
    }
}

int esched_drv_node_topic_cpu_ctx_init(struct sched_numa_node *node, u32 sched_cpu_num)
{
    struct topic_data_chan *topic_chan = NULL;
    u32 i;

    if (sched_cpu_num != node->hard_res.aicpu_chan_num) {
        sched_err("Sched_cpu_num not equal to aicpu_chan_num. (sched_cpu_num=%u, aicpu_chan_num=%u)\n",
            sched_cpu_num, node->hard_res.aicpu_chan_num);
        return DRV_ERROR_PARA_ERROR;
    }

    for (i = 0; i < sched_cpu_num; i++) {
        topic_chan = esched_drv_get_topic_chan(node->node_id, node->hard_res.aicpu_chan_start_id + i);
        topic_chan->cpu_ctx = sched_get_cpu_ctx(node, node->sched_cpuid[i]);
        topic_chan->cpu_ctx->topic_chan = topic_chan;
    }

    return 0;
}

int esched_drv_conf_sched_cpu(struct sched_numa_node *node, u32 sched_cpu_num)
{
    int ret;

    ret = esched_drv_node_topic_cpu_ctx_init(node, sched_cpu_num);
    if (ret != 0) {
        sched_err("Failed to init cpu ctx for topic chan. (node_id=%u)\n", node->node_id);
        return ret;
    }

    sched_debug("Config sched cpu success. (node_id=%u; sched_cpu_num=%u)\n", node->node_id, sched_cpu_num);

    return 0;
}

#ifdef CFG_SOC_PLATFORM_MDC_V11
static int dev_get_cpudomain_number(dev_cpu_nums_cfg_t *cpu_nums_cfg)
{
    int ret;
    struct cpu_domain_info domain_info;

    ret = get_cpudomain_info(&domain_info);
    if (ret != 0) {
        sched_err("get_cpudomain_info failed. (ret = %d)\n", ret);
        return DRV_ERROR_SCHED_INNER_ERR;
    }

    /* ctrlcpu_num the minimum value is 1. */
    if (domain_info.ctrlcpu_num == 0U) {
        sched_err("Ctrlcpu_num should be greater than 0. (ctrlcpuNum = %u)\n", domain_info.ctrlcpu_num);
        return DRV_ERROR_SCHED_INNER_ERR;
    }

    cpu_nums_cfg->aicpu_num = domain_info.aicpu_num;
    cpu_nums_cfg->ccpu_num = domain_info.ctrlcpu_num;
    cpu_nums_cfg->dcpu_num = domain_info.datacpu_num;
    cpu_nums_cfg->tscpu_num = 0;
    cpu_nums_cfg->aicpu_os_sched = ESCHED_DRV_CPU_DEFAULT_PRI;
    cpu_nums_cfg->ccpu_os_sched = ESCHED_DRV_CPU_DEFAULT_PRI;
    cpu_nums_cfg->dcpu_os_sched = ESCHED_DRV_CPU_DEFAULT_PRI;
    cpu_nums_cfg->tscpu_os_sched = 0;

    return 0;
}
#endif

int esched_get_sched_cpu_mask(struct sched_numa_node *node, struct sched_sched_cpu_mask *cpu_mask)
{
    struct agentdrv_cpu_info cpuInfo;
    u32 sched_cpu_start, sched_cpu_num;
    int ret;
    u32 i;

    sched_cpu_start = 0;
    for (i = 0; i <= node->node_id; i++) {
#ifndef CFG_FEATURE_CPU_NUMS_FIXED
        ret = agentdrv_get_cpudomain_info(i, 0, 0, &cpuInfo);
#else
#ifdef CFG_SOC_PLATFORM_MDC_V11
        ret = dev_get_cpudomain_number((dev_cpu_nums_cfg_t *)&cpuInfo);
#else
        ret = dev_user_cfg_get_cpu_number(i, (dev_cpu_nums_cfg_t *)&cpuInfo);
#endif
#endif
        if (ret != DRV_ERROR_NONE) {
            sched_err("Failed to get cpu info. (dev_id=%u; ret=%d)\n", i, ret);
            return DRV_ERROR_SCHED_INNER_ERR;
        }
#ifdef CFG_SOC_PLATFORM_MDC_V11
        if (cpuInfo.ccpu_num == 0) {
            sched_err("Ccpu num is 0. (ccpu_num=%u; dcpu_num=%u, aicpu_num=%u)\n",
                cpuInfo.ccpu_num, cpuInfo.dcpu_num, cpuInfo.aicpu_num);
            return DRV_ERROR_SCHED_INNER_ERR;
        }
#endif
        if (cpuInfo.ccpu_os_sched != 0) {
            sched_cpu_start += cpuInfo.ccpu_num;
        }
        if (cpuInfo.dcpu_os_sched != 0) {
            sched_cpu_start += cpuInfo.dcpu_num;
        }

        if (i < node->node_id) {
            sched_cpu_start += cpuInfo.aicpu_num;
        } else {
            sched_cpu_num = cpuInfo.aicpu_num;
        }
    }

    if ((sched_cpu_start + sched_cpu_num) > SCHED_MASK_BIT_NUM) {
        sched_err("Invalid para. (sched_cpu_start=%u; sched_cpu_num=%u)\n", sched_cpu_start, sched_cpu_num);
        return DRV_ERROR_INVALID_VALUE;
    }

    for (i = 0; i < sched_cpu_num; i++) {
        cpu_mask->mask[0] |= (0x1ULL << (i + sched_cpu_start));
    }

    return 0;
}

static int esched_set_sched_cpu(struct sched_numa_node *node)
{
    struct sched_sched_cpu_mask cpu_mask = {0};
    int ret;

    ret = esched_get_sched_cpu_mask(node, &cpu_mask);
    if (ret != 0) {
        sched_err("Failed to get aicpu mask. (dev_id=%u)\n", node->node_id);
        return ret;
    }

    ret = sched_set_sched_cpu(node->node_id, &cpu_mask);
    if (ret != DRV_ERROR_NONE) {
        sched_err("Failed to set sched cpu. (dev_id=%u)\n", node->node_id);
        return ret;
    }

    return DRV_ERROR_NONE;
}

int esched_init_node_aicpu_chan(u32 devid, struct sched_numa_node *node, u32 *start_id, u32 *aicpu_chan_num)
{
    struct agentdrv_cpu_info cpuInfo = { 0 };
    u32 aicpu_chan_start = 0;
    int ret;

#ifndef CFG_FEATURE_CPU_NUMS_FIXED
    ret = agentdrv_get_cpudomain_info(devid, 0, 0, &cpuInfo);
#else
#ifdef CFG_SOC_PLATFORM_MDC_V11
    ret = dev_get_cpudomain_number((dev_cpu_nums_cfg_t *)&cpuInfo);
#else
    ret = dev_user_cfg_get_cpu_number(devid, (dev_cpu_nums_cfg_t *)&cpuInfo);
#endif
#endif
    if (ret != DRV_ERROR_NONE) {
        sched_err("Failed to get cpu info. (dev_id=%u; ret=%d)\n", devid, ret);
        return DRV_ERROR_SCHED_INNER_ERR;
}

    aicpu_chan_start += (cpuInfo.ccpu_os_sched == 0) ? 0U : cpuInfo.ccpu_num;
    aicpu_chan_start += (cpuInfo.dcpu_os_sched == 0) ? 0U : cpuInfo.dcpu_num;

#ifdef CFG_SOC_PLATFORM_MDC_V11
    if (cpuInfo.ccpu_num == 0) {
        sched_err("Ccpu num is 0. (ccpu_num=%u; dcpu_num=%d; aicpu_num=%u)\n",
            cpuInfo.ccpu_num, cpuInfo.dcpu_num, cpuInfo.aicpu_num);
        return DRV_ERROR_SCHED_INNER_ERR;
    }
#endif

    if (cpuInfo.aicpu_num != 0) {
        *aicpu_chan_num = cpuInfo.aicpu_num;
        aicpu_chan_start = aicpu_chan_start;
    } else {
#ifdef CFG_SOC_PLATFORM_MDC_V11
        *aicpu_chan_num = 1;
#else
        *aicpu_chan_num = cpuInfo.ccpu_num + cpuInfo.dcpu_num - 1;
#endif
        aicpu_chan_start = aicpu_chan_start - cpuInfo.ccpu_num - cpuInfo.dcpu_num + 1;
    }

    node->hard_res.topic_sched_chan_num = *aicpu_chan_num;
    node->hard_res.topic_sched_chan_start_id = aicpu_chan_start;

    if (uda_get_chip_type(devid) == HISI_CLOUD_V2) {
#ifndef EMU_ST
        node->hard_res.topic_sched_chan_num += cpuInfo.dcpu_num;
        node->hard_res.topic_sched_chan_start_id -= cpuInfo.dcpu_num;
#endif
    }

    *start_id = aicpu_chan_start;

    sched_info("Init node aicpu chan num success. (devid=%u; node_id=%u; topic_sched_chan_num=%u; "
        "topic_sched_chan_start_id=%u; aicpu_chan_num=%u; aicpu_chan_start=%u)\n",
        devid, node->node_id, node->hard_res.topic_sched_chan_num,
        node->hard_res.topic_sched_chan_start_id, *aicpu_chan_num, aicpu_chan_start);

    return 0;
}

STATIC void esched_drv_reset_phy_sched_cpu(u32 devid)
{
    esched_drv_uninit_aicpu_pool(esched_get_hard_res(devid));
    esched_drv_uninit_sched_task_submit_chan(devid);
}

STATIC void esched_drv_reset_phy_host_pool(u32 devid)
{
    struct sched_hard_res *res = esched_get_hard_res(devid);
    topic_sched_reset_pool_host_cpu(res->io_base, TOPIC_SCHED_HOST_POOL_ID);

    sched_info("Reset host pool complete. (devid=%u; pool_id=%d)\n", devid, TOPIC_SCHED_HOST_POOL_ID);
}

STATIC void esched_drv_restore_phy_host_pool(u32 devid)
{
    struct sched_hard_res *res = esched_get_hard_res(devid);
    u64 host_ccpu_mask = esched_drv_get_host_ccpu_mask(TOPIC_SCHED_HOST_POOL_ID);

    topic_sched_add_host_pool(res->io_base, CCPU_HOST, TOPIC_SCHED_HOST_POOL_ID, host_ccpu_mask);
    sched_info("Add ccpu host pool. (devid=%u; pool_id=%d; host_ccpu_mask=0x%llx)\n",
        devid, TOPIC_SCHED_HOST_POOL_ID, host_ccpu_mask);

    topic_sched_add_host_pool(res->io_base, ACPU_HOST, TOPIC_SCHED_HOST_POOL_ID, TOPIC_SCHED_HOST_ACPU_MASK);
    sched_info("Add aicpu host pool. (devid=%u; pool_id=%d; host_acpu_mask=0x%llx)\n",
        devid, TOPIC_SCHED_HOST_POOL_ID, (u64)TOPIC_SCHED_HOST_ACPU_MASK);

    sched_debug("Restore host pool complete. (devid=%u; pool_id=%d)\n", devid, TOPIC_SCHED_HOST_POOL_ID);
}

void esched_drv_reset_phy_dev(u32 devid)
{
#ifndef EMU_ST
    struct sched_numa_node *node = esched_dev_get(devid);
    if (node == NULL) {
        sched_err("Node is null, (chip_id=%u).\n", devid);
        return;
    }

    if (node->sched_set_cpu_flag != SCHED_VALID) {
        (void)esched_set_sched_cpu(node);
    }

    (void)cancel_delayed_work_sync(&node->guard_work);
    esched_drv_uninit_comm_pid_mapping(devid);
    esched_drv_reset_phy_host_pool(devid);
    esched_drv_reset_phy_sched_cpu(devid);
    esched_drv_uninit_non_sched_task_submit_chan(devid);
    esched_dev_put(node);
#endif
}

void esched_drv_restore_phy_dev(u32 devid)
{
#ifndef EMU_ST
    struct sched_numa_node *node = esched_dev_get(devid);
    if (node == NULL) {
        sched_err("Node is null, (chip_id=%u).\n", devid);
        return;
    }

    (void)esched_drv_init_non_sched_task_submit_chan(devid, TOPIC_SCHED_ACPU_POOL_ID);
    (void)esched_drv_init_sched_task_submit_chan(devid, TOPIC_SCHED_ACPU_POOL_ID, TOPIC_SCHED_RESV_RTSQ_NUM,
        node->hard_res.aicpu_chan_num);
    esched_drv_init_aicpu_pool(node, node->hard_res.aicpu_chan_start_id, node->hard_res.aicpu_chan_num);
    esched_drv_restore_phy_host_pool(devid);
    (void)esched_drv_init_comm_pid_mapping(devid);
    (void)schedule_delayed_work_on(0, &node->guard_work, msecs_to_jiffies(SCHED_GUARD_WORK_PERIOD));
    esched_dev_put(node);
#endif
}

STATIC int esched_drv_device_ccpu_chan_init(u32 chip_id)
{
    int ret;
    struct topic_data_chan *topic_chan = NULL;
    struct sched_hard_res *res = esched_get_hard_res(chip_id);

    /* Init non-sched channel on aicpu pool. */
    ret = esched_drv_init_non_sched_task_submit_chan(chip_id, TOPIC_SCHED_ACPU_POOL_ID);
    if (ret != 0) {
        return ret;
    }

    /* Init non-sched channel on non-aicpu pool. */
    esched_drv_init_non_aicpu_pool(res);

    topic_chan = esched_drv_create_one_topic_chan(chip_id, NON_SCHED_DEFAULT_CPUID);
    if (topic_chan == NULL) {
        esched_drv_uninit_non_aicpu_pool(res);
        esched_drv_uninit_non_sched_task_submit_chan(chip_id);
        return DRV_ERROR_INNER_ERR;
    }

    ret = esched_drv_init_ccpu_chan(chip_id);
    if (ret != 0) {
        esched_drv_destroy_one_topic_chan(chip_id, NON_SCHED_DEFAULT_CPUID);
        esched_drv_uninit_non_aicpu_pool(res);
        esched_drv_uninit_non_sched_task_submit_chan(chip_id);
        return ret;
    }

    sched_debug("Init ccpu chan complete. (chip_id=%u)\n", chip_id);

    return 0;
}

STATIC void esched_drv_device_ccpu_chan_uninit(u32 chip_id)
{
    struct sched_hard_res *res = esched_get_hard_res(chip_id);

    esched_drv_uninit_ccpu_chan(chip_id);
    esched_drv_destroy_one_topic_chan(chip_id, NON_SCHED_DEFAULT_CPUID);
    esched_drv_uninit_non_aicpu_pool(res);
    esched_drv_uninit_non_sched_task_submit_chan(chip_id);
}

STATIC int esched_drv_device_aicpu_chan_init(u32 chip_id)
{
    int ret;
    u32 aicpu_chan_num = 0;
    u32 start_id;
    struct sched_hard_res *res = esched_get_hard_res(chip_id);
    struct sched_numa_node *node = sched_get_numa_node(chip_id);

    ret = esched_init_node_aicpu_chan(chip_id, node, &start_id, &aicpu_chan_num);
    if (ret != 0) {
        return ret;
    }

    ret = esched_drv_init_sched_task_submit_chan(node->node_id, TOPIC_SCHED_ACPU_POOL_ID, TOPIC_SCHED_RESV_RTSQ_NUM,
        aicpu_chan_num);
    if (ret != 0) {
        return ret;
    }

    esched_drv_init_aicpu_pool(node, start_id, aicpu_chan_num);

    ret = esched_drv_create_topic_chans(chip_id, res->topic_sched_chan_start_id, res->topic_sched_chan_num);
    if (ret != 0) {
#ifndef EMU_ST
        esched_drv_uninit_aicpu_pool(res);
        esched_drv_uninit_sched_task_submit_chan(chip_id);
        return ret;
#endif
    }

    ret = esched_drv_init_all_aicpu_chan(chip_id, res->topic_sched_chan_start_id, res->topic_sched_chan_num);
    if (ret != 0) {
#ifndef EMU_ST
        esched_drv_destroy_topic_chans(chip_id, res->topic_sched_chan_start_id, res->topic_sched_chan_num);
        esched_drv_uninit_aicpu_pool(res);
        esched_drv_uninit_sched_task_submit_chan(chip_id);
        return ret;
#endif
}

    ret = esched_drv_init_all_cpu_port(chip_id, res->topic_sched_chan_start_id, res->topic_sched_chan_num);
    if (ret != 0) {
#ifndef EMU_ST
        esched_drv_uninit_all_aicpu_mb(chip_id, res->topic_sched_chan_start_id, res->topic_sched_chan_num);
        esched_drv_destroy_topic_chans(chip_id, res->topic_sched_chan_start_id, res->topic_sched_chan_num);
        esched_drv_uninit_aicpu_pool(res);
        esched_drv_uninit_sched_task_submit_chan(chip_id);
        return ret;
#endif
    }

    /* Assign the value after topic channel be created. */
    node->hard_res.aicpu_chan_num = aicpu_chan_num;
    node->hard_res.aicpu_chan_start_id = start_id;

    sched_debug("Init aicpu chan complete. (chip_id=%u)\n", chip_id);

    return 0;
}

STATIC void esched_drv_device_aicpu_chan_uninit(u32 chip_id)
{
    struct sched_hard_res *res = esched_get_hard_res(chip_id);

    esched_drv_uninit_all_cpu_port(chip_id, res->topic_sched_chan_start_id, res->topic_sched_chan_num);
    esched_drv_uninit_all_aicpu_mb(chip_id, res->topic_sched_chan_start_id, res->topic_sched_chan_num);
    esched_drv_destroy_topic_chans(chip_id, res->topic_sched_chan_start_id, res->topic_sched_chan_num);
    esched_drv_uninit_aicpu_pool(res);
    esched_drv_uninit_sched_task_submit_chan(chip_id);
    sched_debug("Uninit aicpu chan complete. (chip_id=%u)\n", chip_id);
}

int esched_hw_dev_init(u32 chip_id)
{
    struct sched_hard_res *res = esched_get_hard_res(chip_id);
    struct res_inst_info inst;
    struct soc_reg_base_info topic_reg_base_info;
    struct soc_reg_base_info int_reg_base_info;
    size_t size;
    int ret;

    esched_init_topic_types();
    mutex_init(&res->mutex);

#ifdef CFG_FEATURE_CDQM
    ret = tsdrv_cdqm_set_topic_id(chip_id, EVENT_CDQ_MSG);
    if (ret != 0) {
        sched_err("Failed to invoke the tsdrv_cdqm_set_topic_id. (chip_id=%u; ret=%d)\n", chip_id, ret);
        return ret;
    }
#endif
    res->dev_id = chip_id;

    soc_resmng_inst_pack(&inst, chip_id, TS_SUBSYS, 0);
    ret = soc_resmng_get_irq_by_index(&inst, TS_STARS_TOPIC_IRQ, 0, &res->irq_base);
    if (ret != 0) {
        sched_err("Failed to get stars topic irq base. (chip_id=%u)\n", chip_id);
        return ret;
    }

    ret = soc_resmng_get_reg_base(&inst, "TS_STARS_TOPIC_REG", &topic_reg_base_info);
    if (ret != 0) {
        sched_err("Failed to get stars topic reg base. (chip_id=%u)\n", chip_id);
        return ret;
    }

    res->io_base = ioremap(topic_reg_base_info.io_base, topic_reg_base_info.io_base_size);
    if (res->io_base == NULL) {
        sched_err("Failed to invoke the ioremap. (chip_id=%u; size=%x)\n", chip_id, (u32)TOPIC_SCHED_NS_REG_SIZE);
        return -ENOMEM;
    }

    ret = soc_resmng_get_reg_base(&inst, "TS_STARS_INT_REG", &int_reg_base_info);
    if (ret != 0) {
        sched_err("Failed to get stars int reg base. (chip_id=%u; ret=%d)\n", chip_id, ret);
        goto iounmap_io_base;
    }

    res->int_io_base = ioremap(int_reg_base_info.io_base, int_reg_base_info.io_base_size);
    if (res->int_io_base == NULL) {
        sched_err("Failed to invoke the ioremap. (chip_id=%u; size=%x)\n", chip_id, (u32)TOPIC_INT_NS_REG_SIZE);
        ret = -ENOMEM;
        goto iounmap_io_base;
    }

    ret = topic_sched_get_rsv_mem(chip_id, &res->rsv_mem_pa, &size);
    if (ret != 0) {
        sched_err("Get reserved memory failed. (chip_id=%u; ret=%d)\n", chip_id, ret);
        goto iounmap_int_io_base;
    }

    res->rsv_mem_va = ioremap(res->rsv_mem_pa, size);
    if (res->rsv_mem_va == NULL) {
        sched_err("Failed to invoke the ioremap. (chip_id=%u; size=%lu)\n", chip_id, size);
        ret = -ENOMEM;
        goto iounmap_int_io_base;
    }

    topic_sched_set_user_kernel_type(res->io_base, TOPIC_SCHED_CUSTOM_KERNEL_TYPE);
    esched_drv_reset_pool(res);

    ret = esched_drv_device_ccpu_chan_init(chip_id);
    if (ret != 0) {
        sched_err("Failed to init device ccpu topic. (chip_id=%u; ret=%d)\n", chip_id, ret);
        goto iounmap_rsv_mem_va;
    }

    ret = esched_drv_device_aicpu_chan_init(chip_id);
    if (ret != 0) {
        sched_err("Failed to init device aicpu topic. (chip_id=%u; ret=%d)\n", chip_id, ret);
        goto uninit_device_ccpu_topic;
    }

    ret = esched_drv_init_comm_pid_mapping(chip_id);
    if (ret != 0) {
        sched_err("Failed to init comm topic pid table. (chip_id=%u; ret=%d)\n", chip_id, ret);
        goto uninit_device_aicpu_topic;
    }

    res->init_flag = SCHED_VALID;
    return 0;

uninit_device_aicpu_topic:
    esched_drv_device_aicpu_chan_uninit(chip_id);

uninit_device_ccpu_topic:
    esched_drv_device_ccpu_chan_uninit(chip_id);

iounmap_rsv_mem_va:
    iounmap(res->rsv_mem_va);

iounmap_int_io_base:
    iounmap(res->int_io_base);

iounmap_io_base:
    iounmap(res->io_base);
    return ret;
}

void esched_hw_dev_uninit(u32 chip_id)
{
    struct sched_hard_res *res = esched_get_hard_res(chip_id);

    esched_drv_uninit_comm_pid_mapping(chip_id);
    esched_drv_device_aicpu_chan_uninit(chip_id);
    esched_drv_device_ccpu_chan_uninit(chip_id);
    iounmap(res->rsv_mem_va);
    iounmap(res->int_io_base);
    iounmap(res->io_base);
}

#else
int tmp_esched_drv_cpu_inti_enable(void)
{
    return 0;
}
#endif

