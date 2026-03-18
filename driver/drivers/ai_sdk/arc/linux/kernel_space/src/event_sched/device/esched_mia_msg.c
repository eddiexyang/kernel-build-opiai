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
#if !defined(EVENT_SCHED_UT) && !defined(EMU_ST)

#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/vmalloc.h>
#include <asm/io.h>

#include "esched.h"
#include "esched_drv_mia.h"
#include "esched_device_msg.h"

#ifdef CFG_FEATURE_HARDWARE_SCHED
#include "topic_sched.h"
#include "topic_sched_chip_def.h"
#include "topic_sched_common.h"
#include "esched_drv_adapt.h"
#endif

#ifdef CFG_FEATURE_HARDWARE_SCHED
int esched_drv_mia_add_host_pid(u32 dev_id, struct esched_ctrl_msg_cfg_host_pid *host_pid_msg)
{
    int pid_type = (host_pid_msg->pid_type == (int)DEVDRV_PROCESS_CP1) ? HOST_STD_PROC : HOST_USER_PROC;
    int ret;
    struct sched_hard_res *res = NULL;

    res = esched_get_hard_res(esched_get_chipid_from_devid(dev_id));
    if (res == NULL) {
        sched_err("Failed to get hard res. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    mutex_lock(&res->mutex);
    ret = topic_sched_config_pid(res->io_base, 0, (u32)host_pid_msg->host_ctrl_pid, pid_type, host_pid_msg->pid);
    mutex_unlock(&res->mutex);

    return ret;
}

int esched_drv_mia_del_host_pid(u32 dev_id, struct esched_ctrl_msg_cfg_host_pid *host_pid_msg)
{
    struct sched_hard_res *res = NULL;

    res = esched_get_hard_res(esched_get_chipid_from_devid(dev_id));
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

int esched_drv_mia_add_host_pool(u32 dev_id, struct esched_ctrl_msg_cfg_pool *pool_msg)
{
    struct sched_hard_res *res = NULL;
    struct esched_mia_dev *mia_dev = NULL;
    u64 mask;

    res = esched_get_hard_res(dev_id);
    if (res == NULL) {
        sched_err("Failed to get hard res. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    mia_dev = esched_drv_get_mia_dev(dev_id);
    if (mia_dev == NULL) {
        sched_err("Invalid mia dev. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    if (mia_dev->pool_id >= TOPIC_SCHED_HOST_POOL_NUM) {
        sched_err("The pool_id is out of range. (pool_id=%u)\n", mia_dev->pool_id);
        return -EINVAL;
    }

    mask = (pool_msg->cpu_type == CCPU_HOST) ?
        esched_drv_get_host_ccpu_mask(mia_dev->pool_id) : mia_dev->host_sched_cpu_mask;
    topic_sched_add_host_pool(res->io_base, pool_msg->cpu_type, mia_dev->pool_id, mask);

    sched_info("Add host pool. (chip_id=%u; dev_id=%u; pool_id=%u; cpu_type=%u; mask=%llx)\n",
        mia_dev->chip_id, mia_dev->dev_id, mia_dev->pool_id, pool_msg->cpu_type, mask);

    return 0;
}

int esched_drv_mia_get_host_cpu_mbid(u32 dev_id, struct esched_ctrl_msg_get_cpu_mbid *msg)
{
    struct esched_mia_dev *mia_dev = NULL;

    mia_dev = esched_drv_get_mia_dev(dev_id);
    if (mia_dev == NULL) {
        sched_err("Invalid mia dev. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    if (msg->cpu_type == CCPU_HOST) {
        msg->mb_id = mia_dev->pool_id;
        msg->wait_mb_id = TOPIC_SCHED_HOST_CCPU_WAIT_MB_ID_START + mia_dev->pool_id;
        sched_debug("Get host ccpu mbid. (dev_id=%u; mb_id=%u; wait_mb_id=%u)\n",
            mia_dev->dev_id, msg->mb_id, msg->wait_mb_id);
        return 0;
    } else {
        sched_err("Not support cpu type. (dev_id=%u; cpu_type=%u)\n", mia_dev->dev_id, msg->cpu_type);
        return DRV_ERROR_NOT_SUPPORT;
    }
}

int esched_drv_mia_add_host_mb(u32 dev_id, struct esched_ctrl_msg_cfg_mb *mb_msg)
{
    u64 mb_addr;
    u32 vf_id;
    struct sched_hard_res *res = NULL;
    struct esched_mia_dev *mia_dev = NULL;

    res = esched_get_hard_res(dev_id);
    if (res == NULL) {
        sched_err("Failed to get hard res. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    mia_dev = esched_drv_get_mia_dev(dev_id);
    if (mia_dev == NULL) {
        sched_err("Invalid mia dev. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    if (mia_dev->vfid >= STARS_TOPIC_MAX_VF_NUM) {
        sched_err("The vf_id is out of range. (dev_id=%u; vf_id=%u; max=%d)\n",
            mia_dev->dev_id, mia_dev->vfid, STARS_TOPIC_MAX_VF_NUM);
        return -EINVAL;
    }

    vf_id = (mia_dev->vfid == 0) ? mia_dev->vfid : (mia_dev->vfid - 1);
    mb_addr = res->rsv_mem_pa + TOPIC_SCHED_HOST_RSV_MEM_OFFSET_ADDR + TOPIC_SCHED_HOST_RSV_MEM_VF_OFFSET * vf_id;

    topic_sched_init_host_cpu_mailbox(res->io_base, mb_addr, vf_id);

    sched_debug("Add host mb success. (dev_id=%u; vf_id=%u)\n", mia_dev->dev_id, vf_id);
    return 0;
}

/*
   vf_num: range from 1 to 16
   vf_id: range from 0 to 15
*/
int esched_drv_mia_conf_host_intr(u32 dev_id, struct esched_ctrl_msg_intr *intr_msg)
{
    int int_value, vf_num, vf_id;
    struct sched_hard_res *res = NULL;
    struct esched_mia_dev *mia_dev = NULL;

    res = esched_get_hard_res(dev_id);
    if (res == NULL) {
        sched_err("Failed to get hard res. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    mia_dev = esched_drv_get_mia_dev(dev_id);
    if (mia_dev == NULL) {
        sched_err("Invalid mia dev. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    if (intr_msg->irq >= ESCHED_DRV_HOST_IRQ_MAX) {
        sched_err("The member irq in intr_msg is out of range. (dev_id=%u; irq=%u)\n", mia_dev->dev_id, intr_msg->irq);
        return -EINVAL;
    }

    vf_num = mia_dev->vfid;
    vf_id = (mia_dev->vfid == 0) ? mia_dev->vfid : (mia_dev->vfid - 1);
    int_value = intr_msg->irq | (vf_num << STARS_INT_HTIC_TOPIC_HCPU_VF_NUM_OFFSET);

    topic_sched_conf_host_intr(res->int_io_base, int_value, vf_id);

    sched_info("Config host intr success. (dev_id=%u; vf_id=%u; irq=%u)\n", mia_dev->dev_id, vf_id, intr_msg->irq);

    return 0;
}
#endif

void esched_setup_mia_msg_ops(u32 dev_id)
{
    struct sched_msg_ops ops;

#ifdef CFG_FEATURE_HARDWARE_SCHED
    ops.add_host_pid = esched_drv_mia_add_host_pid;
    ops.del_host_pid = esched_drv_mia_del_host_pid;
    ops.add_host_pool = esched_drv_mia_add_host_pool;
    ops.get_host_cpu_mbid = esched_drv_mia_get_host_cpu_mbid;
    ops.add_host_mb = esched_drv_mia_add_host_mb;
    ops.conf_host_intr = esched_drv_mia_conf_host_intr;
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
#else
void esched_setup_mia_msg_ops(unsigned int dev_id)
{
    return;
}
#endif
