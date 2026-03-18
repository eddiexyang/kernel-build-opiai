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
 * Create: 2022-12-12
 */

#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/vmalloc.h>
#include <asm/io.h>
#include "esched.h"
#include "esched_device_msg.h"

#ifdef CFG_FEATURE_HARDWARE_SCHED
#include "topic_sched_chip_def.h"
#include "topic_sched_common.h"
#include "esched_drv_adapt.h"
#endif

#ifdef CFG_FEATURE_HARDWARE_SCHED
#define SCHED_HOST_PF_VFID 0

int esched_drv_sia_add_host_pid(u32 dev_id, struct esched_ctrl_msg_cfg_host_pid *host_pid_msg)
{
    int pid_type = (host_pid_msg->pid_type == (int)DEVDRV_PROCESS_CP1) ? HOST_STD_PROC : HOST_USER_PROC;
    int ret;
    struct sched_hard_res *res = NULL;

    res = esched_get_hard_res(dev_id);
    if (res == NULL) {
        sched_err("Failed to get hard res. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    mutex_lock(&res->mutex);
    ret = topic_sched_config_pid(res->io_base, 0, (u32)host_pid_msg->host_ctrl_pid, pid_type, host_pid_msg->pid);
    mutex_unlock(&res->mutex);

    return ret;
}

int esched_drv_sia_del_host_pid(u32 dev_id, struct esched_ctrl_msg_cfg_host_pid *host_pid_msg)
{
    struct sched_hard_res *res = NULL;

    res = esched_get_hard_res(dev_id);
    if (res == NULL) {
        sched_err("Failed to get hard res. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    mutex_lock(&res->mutex);
    topic_sched_del_host_pid(res->io_base, 0, (u32)host_pid_msg->host_ctrl_pid,
        HOST_SIDE_SET_PID, (u32)host_pid_msg->pid);
    mutex_unlock(&res->mutex);

    return 0;
}

int esched_drv_sia_add_host_pool(u32 dev_id, struct esched_ctrl_msg_cfg_pool *pool_msg)
{
    struct sched_hard_res *res = NULL;
    u64 mask;

    res = esched_get_hard_res(dev_id);
    if (res == NULL) {
        sched_err("Failed to get hard res. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    mask = (pool_msg->cpu_type == CCPU_HOST) ?
        esched_drv_get_host_ccpu_mask(TOPIC_SCHED_HOST_POOL_ID) : TOPIC_SCHED_HOST_ACPU_MASK;
    topic_sched_add_host_pool(res->io_base, pool_msg->cpu_type, TOPIC_SCHED_HOST_POOL_ID, mask);

    sched_info("Add host pool. (dev_id=%u; pool_id=%u; cpu_type=%u; mask=0x%llx)\n",
        dev_id, TOPIC_SCHED_HOST_POOL_ID, pool_msg->cpu_type, mask);

    return 0;
}

int esched_drv_sia_get_host_cpu_mbid(u32 dev_id, struct esched_ctrl_msg_get_cpu_mbid *msg)
{
    if (msg->cpu_type == CCPU_HOST) {
        msg->mb_id = TOPIC_SCHED_HOST_POOL_ID;
        msg->wait_mb_id = TOPIC_SCHED_HOST_CCPU_WAIT_MB_ID_START + TOPIC_SCHED_HOST_POOL_ID;
        sched_debug("Get host ccpu mbid. (dev_id=%u; mb_id=%u; wait_mb_id=%u)\n", dev_id, msg->mb_id, msg->wait_mb_id);
        return 0;
    } else {
        sched_err("Not support cpu type. (dev_id=%u; cpu_type=%u)\n", dev_id, msg->cpu_type);
        return DRV_ERROR_NOT_SUPPORT;
    }
}

int esched_drv_sia_add_host_mb(u32 dev_id, struct esched_ctrl_msg_cfg_mb *mb_msg)
{
    struct sched_hard_res *res = NULL;
    u64 mb_addr;

    res = esched_get_hard_res(dev_id);
    if (res == NULL) {
        sched_err("Failed to get hard res. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    mb_addr = res->rsv_mem_pa + TOPIC_SCHED_HOST_RSV_MEM_OFFSET_ADDR;
    topic_sched_init_host_cpu_mailbox(res->io_base, mb_addr, SCHED_HOST_PF_VFID);

    sched_debug("Add host mb success. (dev_id=%u; vf_id=%d)\n", res->dev_id, SCHED_HOST_PF_VFID);

    return 0;
}

int esched_drv_sia_conf_host_intr(u32 dev_id, struct esched_ctrl_msg_intr *intr_msg)
{
    struct sched_hard_res *res = NULL;

    res = esched_get_hard_res(dev_id);
    if (res == NULL) {
        sched_err("Failed to get hard res. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    if (intr_msg->irq >= ESCHED_DRV_HOST_IRQ_MAX) {
        sched_err("The member irq in intr_msg is out of range. (dev_id=%u; irq=%u)\n", dev_id, intr_msg->irq);
        return -EINVAL;
    }

    topic_sched_conf_host_intr(res->int_io_base, intr_msg->irq, SCHED_HOST_PF_VFID);

    sched_info("Config host intr success. (dev_id=%u; vf_id=%u; irq=%u)\n",
        res->dev_id, SCHED_HOST_PF_VFID, intr_msg->irq);

    return 0;
}
#endif

void esched_setup_sia_msg_ops(u32 dev_id)
{
    struct sched_msg_ops ops;

    if (dev_id >= SCHED_MAX_CHIP_NUM) {
        return;
    }

#ifdef CFG_FEATURE_HARDWARE_SCHED
    ops.add_host_pid = esched_drv_sia_add_host_pid;
    ops.del_host_pid = esched_drv_sia_del_host_pid;
    ops.add_host_pool = esched_drv_sia_add_host_pool;
    ops.get_host_cpu_mbid = esched_drv_sia_get_host_cpu_mbid;
    ops.add_host_mb = esched_drv_sia_add_host_mb;
    ops.conf_host_intr = esched_drv_sia_conf_host_intr;
#else
    ops.add_host_pid = NULL;
    ops.del_host_pid = NULL;
    ops.add_host_pool = NULL;
    ops.get_host_cpu_mbid = NULL;
    ops.add_host_mb = NULL;
    ops.conf_host_intr = NULL;
#endif

    esched_drv_register_msg_ops(dev_id, &ops);
}

