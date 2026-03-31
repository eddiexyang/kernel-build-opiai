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
 * Create: 2022-10-15
 */

#if !defined(EVENT_SCHED_UT) && !defined(EMU_ST)

#include <linux/mutex.h>
#include "securec.h"
#include "devdrv_interface.h"
#include "virtmng_interface.h"
#include "esched.h"
#include "esched_drv_adapt.h"
#include "esched_drv_mia.h"
#include "user_cfg_interface.h"
#include "tsdrv_interface.h"

#ifdef CFG_FEATURE_HARDWARE_SCHED
#include "topic_sched_common.h"
#include "topic_sched_chip_def.h"
#endif

#ifdef CFG_FEATURE_REMOTE_SUBMIT
#include "esched_device_msg.h"
#endif

static DEFINE_MUTEX(sched_mia_dev_mutex);

struct esched_mia_dev mia_devs[SCHED_MAX_CHIP_NUM];

struct sched_thread_ctx *esched_drv_get_cpu_next_thread(u32 chip_id, u32 vfid, struct sched_cpu_ctx *cpu_ctx);
u32 esched_drv_get_node_aicpu_chan_mask(u32 start_id, u32 aicpu_chan_num);
int esched_get_sched_cpu_mask(struct sched_numa_node *node, struct sched_sched_cpu_mask *cpu_mask);
int esched_init_node_aicpu_chan(u32 devid, struct sched_numa_node *node, u32 *start_id, u32 *aicpu_chan_num);
int esched_drv_node_topic_cpu_ctx_init(struct sched_numa_node *node, u32 sched_cpu_num);

struct esched_mia_dev *esched_drv_get_mia_dev(u32 dev_id)
{
    struct esched_mia_dev *mia_dev = &mia_devs[dev_id];
    return (mia_dev->valid == 1) ? mia_dev : NULL;
}

/* check hit_mask is in the mask range */
static bool esched_is_mask_hit(u32 mask, u32 hit_mask)
{
    return ((hit_mask & (~mask)) == 0);
}

int esched_get_pfvf_id_by_devid(u32 dev_id, u32 *chip_id, u32 *vfid)
{
    return vmngd_get_pfvf_id_by_devid(dev_id, chip_id, vfid);
}

static inline u32 esched_drv_get_vfg_ccpu_slot_mask(u32 pool_id)
{
    return TOPIC_SCHED_ACPU_POOL_CCPU_SLOT &
        (TOPIC_SCHED_VFG_CCPU_SLOT_MASK << (pool_id * TOPIC_SCHED_VFG_CCPU_SLOT_NUM));
}

static void esched_drv_init_vf_pool(struct esched_mia_dev *mia_dev)
{
    struct sched_hard_res *res = esched_get_hard_res(mia_dev->chip_id);
    u32 mb_mask = mia_dev->sched_cpu_mask;
    u32 slot_mask = mia_dev->slot_mask;
    u32 pool_id = mia_dev->pool_id;
    u32 ccpu_slot_mask = esched_drv_get_vfg_ccpu_slot_mask(pool_id);

    topic_sched_pool_add_mb(res->io_base, pool_id, mb_mask);
    topic_sched_pool_add_aicpu_slot(res->io_base, pool_id, slot_mask);
    topic_sched_pool_add_ccpu_slot(res->io_base, pool_id, ccpu_slot_mask);

    sched_info("Show details. (pool_id=%d; mb_mask=%x; acpu_slot_mask=0x%08x; ccpu_slot_mask=0x%08x)\n",
        pool_id, mb_mask, slot_mask, ccpu_slot_mask);
}

static void esched_drv_uninit_vf_pool(struct esched_mia_dev *mia_dev)
{
    struct sched_hard_res *res = esched_get_hard_res(mia_dev->chip_id);
    u32 pool_id = mia_dev->pool_id;
    u32 ccpu_slot_mask = esched_drv_get_vfg_ccpu_slot_mask(pool_id);
    u32 mb_deleted_mask = mia_dev->sched_cpu_mask;
    u64 host_mb_deleted_mask = mia_dev->host_sched_cpu_mask;
    u32 slot_deleted_mask = mia_dev->slot_mask;
    int index;

    for (index = 0; index < SCHED_MAX_CHIP_NUM; ++index) {
        struct esched_mia_dev *dev = &mia_devs[index];

        if ((dev->valid == 0) || (dev->chip_id != mia_dev->chip_id) || (dev->pool_id != mia_dev->pool_id) ||
            (dev->dev_id == mia_dev->dev_id)) {
            continue;
        }

        ccpu_slot_mask = 0;
        if ((dev->sched_cpu_mask & mia_dev->sched_cpu_mask) != 0) {
            mb_deleted_mask &= (~dev->sched_cpu_mask);
        }

        if ((dev->host_sched_cpu_mask & mia_dev->host_sched_cpu_mask) != 0) {
            host_mb_deleted_mask &= (~dev->host_sched_cpu_mask);
        }

        if ((dev->slot_mask & mia_dev->slot_mask) != 0) {
            slot_deleted_mask &= (~dev->slot_mask);
        }
    }

    topic_sched_pool_del_mb(res->io_base, pool_id, mb_deleted_mask);
    topic_sched_del_host_pool(res->io_base, ACPU_HOST, pool_id, host_mb_deleted_mask);
    topic_sched_pool_del_aicpu_slot(res->io_base, pool_id, slot_deleted_mask);
    topic_sched_pool_del_ccpu_slot(res->io_base, pool_id, ccpu_slot_mask);

    sched_info("Show details. (poolid=%d; mb_del_mask=%x; host_mb_del_mask=%llx; acpu_slot=%x; ccpu_slot=%x)\n",
        pool_id, mb_deleted_mask, host_mb_deleted_mask, slot_deleted_mask, ccpu_slot_mask);
}

static void esched_mia_dev_init_hard_res(struct esched_mia_dev *mia_dev)
{
    struct sched_hard_res *hard_res = esched_get_hard_res(mia_dev->chip_id);
    struct sched_hard_res *mia_hard_res = esched_get_hard_res(mia_dev->dev_id);

    mutex_init(&mia_hard_res->mutex);
    mia_hard_res->dev_id = mia_dev->dev_id;
    mia_hard_res->io_base = hard_res->io_base;
    mia_hard_res->int_io_base = hard_res->int_io_base;
    mia_hard_res->rsv_mem_pa = hard_res->rsv_mem_pa;
    mia_hard_res->rsv_mem_va = hard_res->rsv_mem_va;
    mia_hard_res->init_flag = SCHED_VALID;
}

static int esched_mia_dev_init_topic_chan(struct esched_mia_dev *mia_dev, u32 chan_id)
{
    struct topic_data_chan *topic_chan = esched_drv_get_topic_chan(mia_dev->chip_id, chan_id);
    struct topic_data_chan *mia_topic_chan = esched_drv_create_one_topic_chan(mia_dev->dev_id, chan_id);
    if (mia_topic_chan == NULL) {
        return DRV_ERROR_OUT_OF_MEMORY;
    }

    *mia_topic_chan = *topic_chan; /* Inherit resources from physical devices */

    mia_topic_chan->hard_res = esched_get_hard_res(mia_dev->dev_id);
    if (chan_id == NON_SCHED_DEFAULT_CPUID) {
        /* Only get ccpu channel cpu_ctx, the index is alway 0(the macro defined) in os. */
        mia_topic_chan->cpu_ctx = sched_get_cpu_ctx(sched_get_numa_node(mia_dev->dev_id), chan_id);
        mia_topic_chan->cpu_ctx->topic_chan = mia_topic_chan;
    }
    return 0;
}

static void esched_mia_dev_uninit_topic_chan(struct esched_mia_dev *mia_dev, u32 chan_id)
{
    esched_drv_destroy_one_topic_chan(mia_dev->dev_id, chan_id);
}

static int esched_mia_dev_ccpu_chan_init(struct esched_mia_dev *mia_dev)
{
    int ret;

    ret = esched_drv_init_non_sched_task_submit_chan(mia_dev->dev_id, mia_dev->pool_id);
    if (ret != 0) {
        sched_err("Fail to init task submit chan. (dev_id=%u; pool_id=%u)\n", mia_dev->dev_id, mia_dev->pool_id);
        return ret;
    }

    ret = esched_mia_dev_init_topic_chan(mia_dev, NON_SCHED_DEFAULT_CPUID);
    if (ret != 0) {
        sched_err("Fail to init topic chan. (dev_id=%u; chan_id=%d)\n", mia_dev->dev_id, NON_SCHED_DEFAULT_CPUID);
        esched_drv_uninit_non_sched_task_submit_chan(mia_dev->dev_id);
    }

    sched_debug("Init ccpu chan complete. (dev_id=%u)\n", mia_dev->dev_id);
    return ret;
}

static void esched_mia_dev_ccpu_chan_uninit(struct esched_mia_dev *mia_dev)
{
    esched_mia_dev_uninit_topic_chan(mia_dev, NON_SCHED_DEFAULT_CPUID);
    esched_drv_uninit_non_sched_task_submit_chan(mia_dev->dev_id);
}

static int esched_mia_dev_aicpu_chan_init(struct esched_mia_dev *mia_dev)
{
    struct sched_numa_node *node = sched_get_numa_node(mia_dev->dev_id);
    u32 aicpu_chan_num = 0;
    u32 start_id;
    u32 mia_aicpu_chan_num = __sw_hweight32(mia_dev->sched_cpu_mask);
    u32 i, j;
    int ret;

    ret = esched_init_node_aicpu_chan(mia_dev->chip_id, node, &start_id, &aicpu_chan_num);
    if (ret != 0) {
        return ret;
    }

    ret = esched_drv_init_sched_task_submit_chan(mia_dev->dev_id, mia_dev->pool_id, mia_dev->rtsq_num,
        mia_aicpu_chan_num);
    if (ret != 0) {
        sched_err("Fail to init task submit chan. (dev_id=%u; pool_id=%u; mia_aicpu_chan_num=%u)\n",
            mia_dev->dev_id, mia_dev->pool_id, mia_aicpu_chan_num);
        return ret;
    }

    for (i = 0; i < aicpu_chan_num; i++) {
        ret = esched_mia_dev_init_topic_chan(mia_dev, start_id + i);
        if (ret != 0) {
            sched_err("Failed to init topic chan. (dev_id=%u; aicpu_chan_id=%u)\n",
                mia_dev->dev_id, start_id + i);
            for (j = 0; j < i; j++) {
                esched_mia_dev_uninit_topic_chan(mia_dev, start_id + j);
            }
            return ret;
        }
    }

    node->hard_res.aicpu_chan_num = aicpu_chan_num;
    node->hard_res.aicpu_chan_start_id = start_id;

    sched_debug("Init aicpu chan complete. (dev_id=%u)\n", mia_dev->dev_id);

    return 0;
}

static void esched_mia_dev_aicpu_chan_uninit(struct esched_mia_dev *mia_dev)
{
    struct sched_numa_node *node = sched_get_numa_node(mia_dev->dev_id);
    u32 i;

    for (i = 0; i < node->hard_res.aicpu_chan_num; i++) {
        esched_mia_dev_uninit_topic_chan(mia_dev, node->hard_res.aicpu_chan_start_id + i);
    }
    esched_drv_uninit_sched_task_submit_chan(mia_dev->dev_id);
}

static u32 esched_drv_mia_get_identity(u32 devid)
{
    struct esched_mia_dev *mia_dev = &mia_devs[devid];
    return ((mia_dev->pool_id << 16) | mia_dev->vfid); /* hign 16 bit is poolid */
}

static int esched_mia_dev_init(struct esched_mia_dev *mia_dev)
{
    struct sched_sched_cpu_mask cpu_mask = { 0 };
    int ret;

#ifndef CFG_FEATURE_MIA_MAP_TOPIC_TABLE
    ret = esched_drv_init_topic_table(mia_dev->chip_id, esched_drv_mia_get_identity(mia_dev->dev_id));
    if (ret != 0) {
        return ret;
    }
#endif

    esched_mia_dev_init_hard_res(mia_dev);
    ret = esched_mia_dev_ccpu_chan_init(mia_dev);
    if (ret != 0) {
        goto uninit_pid_table;
    }

    ret = esched_mia_dev_aicpu_chan_init(mia_dev);
    if (ret != 0) {
        goto uninit_ccpu_chan;
    }

    ret = esched_get_sched_cpu_mask(sched_get_numa_node(mia_dev->chip_id), &cpu_mask);
    if (ret != 0) {
        sched_err("Get sched cpu mask failed. (chip_id=%u)\n", mia_dev->chip_id);
        goto uninit_aicpu_chan;
    }

    ret = sched_set_sched_cpu(mia_dev->dev_id, &cpu_mask);
    if (ret != 0) {
        sched_err("Set sched cpu failed. (dev_id=%u)\n", mia_dev->dev_id);
        goto uninit_aicpu_chan;
    }

    esched_drv_init_vf_pool(mia_dev);
    return 0;

uninit_aicpu_chan:
    esched_mia_dev_aicpu_chan_uninit(mia_dev);

uninit_ccpu_chan:
    esched_mia_dev_ccpu_chan_uninit(mia_dev);

uninit_pid_table:
#ifndef CFG_FEATURE_MIA_MAP_TOPIC_TABLE
    esched_drv_uninit_topic_table(mia_dev->chip_id, esched_drv_mia_get_identity(mia_dev->dev_id));
#endif
    return ret;
}

static void esched_mia_dev_uninit(struct esched_mia_dev *mia_dev)
{
    esched_drv_uninit_vf_pool(mia_dev);
    esched_mia_dev_aicpu_chan_uninit(mia_dev);
    esched_mia_dev_ccpu_chan_uninit(mia_dev);

#ifndef CFG_FEATURE_MIA_MAP_TOPIC_TABLE
    esched_drv_uninit_topic_table(mia_dev->chip_id, esched_drv_mia_get_identity(mia_dev->dev_id));
#endif
}

#ifndef CFG_FEATURE_HARD_SOFT_SCHED
static struct sched_thread_ctx *esched_drv_mia_get_next_thread(struct sched_cpu_ctx *cpu_ctx)
{
    struct esched_mia_dev *mia_dev = &mia_devs[cpu_ctx->node->node_id];
    return esched_drv_get_cpu_next_thread(mia_dev->chip_id, mia_dev->vfid - 1, cpu_ctx);
}
#endif

static int esched_drv_mia_conf_sched_cpu(struct sched_numa_node *node, u32 sched_cpu_num)
{
    return esched_drv_node_topic_cpu_ctx_init(node, sched_cpu_num);
}

static int esched_drv_mia_map_host_pid(struct sched_proc_ctx *proc_ctx)
{
    return esched_drv_map_host_dev_pid(proc_ctx, esched_drv_mia_get_identity(proc_ctx->node->node_id));
}

static void esched_drv_mia_unmap_host_pid(struct sched_proc_ctx *proc_ctx)
{
    esched_drv_unmap_host_dev_pid(proc_ctx, esched_drv_mia_get_identity(proc_ctx->node->node_id));
}

bool sched_is_cpu_belongs_to_vf(struct sched_numa_node *node, u32 vfid, struct sched_cpu_ctx *cpu_ctx)
{
    struct esched_mia_dev *mia_dev = &mia_devs[node->node_id];

    if (esched_is_phy_dev(node->node_id) == true) {
        return true;
    }

    if ((0x1ULL << cpu_ctx->cpuid) & mia_dev->sched_cpu_mask) {
        return true;
    }

    return false;
}

static int sched_create_mia_dev(struct esched_mia_dev *mia_dev)
{
    struct sched_dev_ops ops;
    int ret;

    /* replace some of phy dev ops */
    esched_setup_dev_hw_ops(&ops);
#ifndef CFG_FEATURE_HARD_SOFT_SCHED
    ops.sched_cpu_get_next_thread = esched_drv_mia_get_next_thread;
#endif
    ops.conf_sched_cpu = esched_drv_mia_conf_sched_cpu;
    ops.map_host_pid = esched_drv_mia_map_host_pid;
    ops.unmap_host_pid = esched_drv_mia_unmap_host_pid;

    ret = esched_create_dev(mia_dev->dev_id, &ops);
    if (ret != 0) {
        sched_err("Create mia dev failed. (devid=%u)\n", mia_dev->dev_id);
        return ret;
    }

    ret = esched_mia_dev_init(mia_dev);
    if (ret != 0) {
        esched_destroy_dev(mia_dev->dev_id);
        sched_err("Init mia dev failed. (dev_id=%u)\n", mia_dev->dev_id);
        return ret;
    }

    return 0;
}

static void sched_destroy_mia_dev(struct esched_mia_dev *mia_dev)
{
    esched_mia_dev_uninit(mia_dev);
    esched_destroy_dev(mia_dev->dev_id);
}

/* check hit_mask is in the mask range */
static bool esched_is_mia_dev_sched_cpu_conflict(struct esched_mia_dev *mia_dev)
{
    int i;

    for (i = 0; i < SCHED_MAX_CHIP_NUM; i++) {
        struct esched_mia_dev *dev = &mia_devs[i];
        if ((dev->valid == 1) && (dev->dev_id != mia_dev->dev_id) && (dev->chip_id == mia_dev->chip_id) &&
            (dev->pool_id != mia_dev->pool_id) && ((dev->sched_cpu_mask & mia_dev->sched_cpu_mask) != 0) &&
            ((dev->host_sched_cpu_mask & mia_dev->host_sched_cpu_mask) != 0)) {
            sched_err("Sched cpu conflicts. (dev_id=%u; sched_cpu_mask=%x; dev_id=%u; sched_cpu_mask=%x; pool_id=%u)\n",
                mia_dev->dev_id, mia_dev->sched_cpu_mask, dev->dev_id, dev->sched_cpu_mask, mia_dev->pool_id);
            return true;
        }
    }

    return false;
}

static int _sched_create_vf_dev(struct esched_mia_dev *mia_dev)
{
    struct sched_numa_node *node = NULL;
    struct sched_hard_res *hard_res = NULL;
    u32 chip_sched_cpu_mask;
    int ret;

    node = sched_get_numa_node(mia_dev->dev_id);
    if (node != NULL) {
        sched_err("Repeat created. (dev_id=%u; dtype=%u)\n", mia_dev->dev_id, mia_dev->dtype);
        return DRV_ERROR_REPEATED_INIT;
    }

    node = esched_dev_get(mia_dev->chip_id); /* put phy dev on destroy vf dev */
    if (node == NULL) {
        sched_err("Phy dev not be created. (dev_id=%u; chip_id=%u)\n", mia_dev->dev_id, mia_dev->chip_id);
        return DRV_ERROR_NO_DEVICE;
    }

    chip_sched_cpu_mask = esched_drv_get_node_aicpu_chan_mask(node->hard_res.topic_sched_chan_start_id,
        node->hard_res.topic_sched_chan_num);
    if (!esched_is_mask_hit(chip_sched_cpu_mask, mia_dev->sched_cpu_mask)) {
        sched_err("Sched cpu mask not hit. (dev_id=%u; chip_id=%u; sched_cpu_mask=%x; chip_sched_cpu_mask=%x)\n",
            mia_dev->dev_id, mia_dev->chip_id, mia_dev->sched_cpu_mask, chip_sched_cpu_mask);
        esched_dev_put(node);
        return DRV_ERROR_INVALID_VALUE;
    }

    if (esched_is_mia_dev_sched_cpu_conflict(mia_dev)) {
        esched_dev_put(node);
        return DRV_ERROR_INVALID_VALUE;
    }

    ret = sched_create_mia_dev(mia_dev);
    if (ret != 0) {
        esched_dev_put(node);
        return ret;
    }

    hard_res = esched_get_hard_res(mia_dev->chip_id);
    hard_res->sub_dev_num++;
    mia_dev->valid = 1;

    sched_info("Mia dev create success. (dev_id=%u; pool_id=%u; sched_cpu_mask=%x; slot_mask=%x; sub_dev_num=%u)\n",
        mia_dev->dev_id, mia_dev->pool_id, mia_dev->sched_cpu_mask, mia_dev->slot_mask, hard_res->sub_dev_num);

    return 0;
}

static int sched_create_vf_dev(struct vmngd_client_instance *instance)
{
    struct esched_mia_dev *mia_dev = NULL;
    struct vmng_vdev_ctrl *vdev_ctrl = NULL;
    int ret;

    if (instance == NULL) {
        sched_err("The variable instance is NULL.\n");
        return DRV_ERROR_INNER_ERR;
    }

    vdev_ctrl = &instance->vdev_ctrl;
    if ((vdev_ctrl->dev_id >= SCHED_MAX_CHIP_NUM) || (vdev_ctrl->vf_cfg.id.vfg_id >= TOPIC_SCHED_VFG_NUM)) {
        sched_err("Invalid para. (dev_id=%u; vfg_id=%u)\n", vdev_ctrl->dev_id, vdev_ctrl->vf_cfg.id.vfg_id);
        return DRV_ERROR_INVALID_VALUE;
    }

    mia_dev = &mia_devs[vdev_ctrl->dev_id];

    ret = esched_get_pfvf_id_by_devid(vdev_ctrl->dev_id, &mia_dev->chip_id, &mia_dev->vfid);
    if (ret != 0) {
        sched_err("Fail to get pfvf id. (dev_id=%u)\n", vdev_ctrl->dev_id);
        return ret;
    }

    mutex_lock(&sched_mia_dev_mutex);

#ifdef CFG_FEATURE_REMOTE_SUBMIT
    esched_setup_mia_msg_ops(vdev_ctrl->dev_id);
#endif

    mia_dev->dev_id = vdev_ctrl->dev_id;
    mia_dev->dtype = vdev_ctrl->dtype;
    mia_dev->pool_id = vdev_ctrl->vf_cfg.id.vfg_id;
    mia_dev->host_sched_cpu_mask = vdev_ctrl->vf_cfg.cpu.host_aicpu_bitmap;
    mia_dev->rtsq_num = __sw_hweight32(vdev_ctrl->vf_cfg.accelerator.rtsq_slice_bitmap) * TOPIC_SCHED_RTSQ_NUM_PER_BIT;
    mia_dev->slot_mask = vdev_ctrl->vf_cfg.cpu.topic_aicpu_slot_bitmap;
    if (vdev_ctrl->vf_cfg.cpu.device_aicpu_bitmap == 0) {
        u32 aicpu_chan_mask;
        struct sched_numa_node *node = sched_get_numa_node(mia_dev->chip_id);
        aicpu_chan_mask = esched_drv_get_node_aicpu_chan_mask(node->hard_res.aicpu_chan_start_id,
            node->hard_res.aicpu_chan_num);
        mia_dev->sched_cpu_mask = aicpu_chan_mask;
    } else {
        mia_dev->sched_cpu_mask = vdev_ctrl->vf_cfg.cpu.device_aicpu_bitmap;
    }

    ret = _sched_create_vf_dev(mia_dev);

    mutex_unlock(&sched_mia_dev_mutex);

    return ret;
}

static int _sched_destroy_vf_dev(struct esched_mia_dev *mia_dev)
{
    struct sched_numa_node *node = NULL;
    struct sched_hard_res *hard_res = NULL;

    node = sched_get_numa_node(mia_dev->dev_id);
    if (node == NULL) {
        sched_err("Vf dev not be created. (dev_id=%u; dtype=%u)\n", mia_dev->dev_id, mia_dev->dtype);
        return DRV_ERROR_REPEATED_INIT;
    }

    sched_destroy_mia_dev(mia_dev);

    hard_res = esched_get_hard_res(mia_dev->chip_id);
    hard_res->sub_dev_num--;

    esched_dev_put(sched_get_numa_node(mia_dev->chip_id)); /* put phy dev which is get in create vf dev */
    mia_dev->valid = 0;

    sched_info("Mia dev destroy success. (dev_id=%u; pool_id=%u; sched_cpu_mask=%x; sub_dev_num=%u)\n",
        mia_dev->dev_id, mia_dev->pool_id, mia_dev->sched_cpu_mask, hard_res->sub_dev_num);

    return 0;
}

static int sched_destroy_vf_dev(struct vmngd_client_instance *instance)
{
    struct esched_mia_dev *mia_dev = NULL;
    struct vmng_vdev_ctrl *vdev_ctrl = NULL;
    int ret;

    if (instance == NULL) {
        sched_err("The variable instance is NULL.\n");
        return DRV_ERROR_INNER_ERR;
    }

    vdev_ctrl = &instance->vdev_ctrl;
    if ((vdev_ctrl->dev_id >= SCHED_MAX_CHIP_NUM) || (vdev_ctrl->vf_cfg.id.vfg_id >= TOPIC_SCHED_VFG_NUM)) {
        sched_err("Invalid para. (dev_id=%u; vfg_id=%u)\n", vdev_ctrl->dev_id, vdev_ctrl->vf_cfg.id.vfg_id);
        return DRV_ERROR_INVALID_VALUE;
    }

    mutex_lock(&sched_mia_dev_mutex);
    mia_dev = &mia_devs[vdev_ctrl->dev_id];
    ret = _sched_destroy_vf_dev(mia_dev);
    mutex_unlock(&sched_mia_dev_mutex);

    return ret;
}

static struct vmngd_client esched_vmngd_client = {
    .type = VMNGD_CLIENT_TYPE_ESCHED,
    .init_instance = sched_create_vf_dev,
    .uninit_instance = sched_destroy_vf_dev,
};

void sched_vf_init(void)
{
    int ret;

    (void)memset_s((void *)mia_devs, sizeof(mia_devs), 0, sizeof(mia_devs));

    ret = vmngd_register_client(&esched_vmngd_client);
    if (ret != 0) {
        sched_err("Failed to invoke the vmngd_register_client. (ret=%d)\n", ret);
        return;
    }
}

void sched_vf_uninit(void)
{
    int ret;

    ret = vmngd_unregister_client(&esched_vmngd_client);
    if (ret != 0) {
        sched_err("Failed to invoke the vmngd_unregister_client. (ret=%d)\n", ret);
        return;
    }
}

#else
int tmp_esched_get_pfvf_id_by_devid()
{
    return 0;
}
#endif

