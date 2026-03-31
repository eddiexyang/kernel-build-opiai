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
* Create: 2022-10-12
*/

#include <linux/slab.h>
#include <linux/vmalloc.h>

#include "virtmng_interface.h"
#include "devdrv_interface.h"

#include "trs_pub_def.h"
#include "trs_mia_cfg.h"

#define TRS_ID_BITNUM 16

static DEFINE_RWLOCK(mia_cfg_lock);
static struct trs_mia_cfg *g_mia_cfg[TRS_TS_INST_MAX_NUM];

static int trs_mia_get_valid_bitnum(u32 sia_bitnum, u32 bitmap)
{
    u32 bit;
    int mia_bitnum = 0;

    for (bit = 0; bit < sia_bitnum; bit++) {
        if (trs_bitmap_bit_is_vaild(bitmap, bit)) {
            mia_bitnum++;
        }
    }

    return mia_bitnum;
}

int trs_mia_cfg_create(struct trs_id_inst *inst, struct vmngd_client_instance *instance)
{
    u32 ts_inst_id = trs_id_inst_to_ts_inst(inst);
    struct trs_mia_cfg *mia_cfg = NULL;
    u32 pf_id, vf_id;
    int ret;

    mia_cfg = (struct trs_mia_cfg *)kzalloc(sizeof(struct trs_mia_cfg), GFP_KERNEL);
    if (mia_cfg == NULL) {
        return -ENOMEM;
    }

    ret = vmngd_get_pfvf_id_by_devid(inst->devid, &pf_id, &vf_id);
    if (ret != 0) {
        trs_err("Get pf vf id failed. (devid=%u)\n", inst->devid);
        kfree(mia_cfg);
        return ret;
    }

    mia_cfg->inst.devid = inst->devid;
    mia_cfg->inst.tsid = inst->tsid;
    mia_cfg->vfid = vf_id - 1;
    mia_cfg->pm_inst.devid = pf_id;
    mia_cfg->pm_inst.tsid = inst->tsid;
    mia_cfg->sia_bitnum = TRS_ID_BITNUM;
    mia_cfg->rtsq_bitmap = instance->vdev_ctrl.vf_cfg.accelerator.rtsq_slice_bitmap;
    mia_cfg->event_bitmap = instance->vdev_ctrl.vf_cfg.accelerator.event_slice_bitmap;
    mia_cfg->notify_bitmap = instance->vdev_ctrl.vf_cfg.accelerator.notify_slice_bitmap;
    mia_cfg->mia_bitnum = trs_mia_get_valid_bitnum(mia_cfg->sia_bitnum, mia_cfg->rtsq_bitmap);

    mutex_init(&mia_cfg->mutex);
    kref_init(&mia_cfg->ref);

    write_lock_bh(&mia_cfg_lock);
    if (g_mia_cfg[ts_inst_id] != NULL) {
        write_unlock_bh(&mia_cfg_lock);
        kfree(mia_cfg);
        trs_err("Repeat create. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        return -EINVAL;
    }

    g_mia_cfg[ts_inst_id] = mia_cfg;
    write_unlock_bh(&mia_cfg_lock);

    trs_info("Mia create. (devid=%u; tsid=%u; mia_bitnum=%u; rtsqmap=%u; eventmap=%u; notifymap=%u)\n",
        inst->devid, inst->tsid, mia_cfg->mia_bitnum,
        mia_cfg->rtsq_bitmap, mia_cfg->event_bitmap, mia_cfg->notify_bitmap);
    return 0;
}

static void trs_mia_cfg_release(struct kref *kref)
{
    struct trs_mia_cfg *mia_cfg = container_of(kref, struct trs_mia_cfg, ref);

    if (mia_cfg->cqe_done != NULL) {
        vfree(mia_cfg->cqe_done);
    }

    kfree(mia_cfg);
}

void trs_mia_cfg_destroy(struct trs_id_inst *inst)
{
    u32 ts_inst_id = trs_id_inst_to_ts_inst(inst);
    struct trs_mia_cfg *mia_cfg = NULL;

    write_lock_bh(&mia_cfg_lock);
    mia_cfg = g_mia_cfg[ts_inst_id];
    g_mia_cfg[ts_inst_id] = NULL;
    write_unlock_bh(&mia_cfg_lock);

    if (mia_cfg != NULL) {
        kref_put(&mia_cfg->ref, trs_mia_cfg_release);
    }
}

struct trs_mia_cfg *trs_mia_cfg_get(struct trs_id_inst *inst)
{
    u32 ts_inst_id = trs_id_inst_to_ts_inst(inst);
    struct trs_mia_cfg *mia_cfg = NULL;

    read_lock_bh(&mia_cfg_lock);
    mia_cfg = g_mia_cfg[ts_inst_id];
    if (mia_cfg != NULL) {
        kref_get(&mia_cfg->ref);
    }
    read_unlock_bh(&mia_cfg_lock);

    return mia_cfg;
}

void trs_mia_cfg_put(struct trs_mia_cfg *mia_cfg)
{
    kref_put(&mia_cfg->ref, trs_mia_cfg_release);
}

