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
* Create: 2022-11-7
*/

#include <linux/slab.h>
#include <linux/vmalloc.h>

#include "devdrv_interface.h"
#include "soc_res.h"
#include "dbl/uda.h"

#include "trs_pub_def.h"
#include "trs_msg.h"
#include "trs_host_id.h"
#include "trs_sec_eh_cfg.h"

static DEFINE_RWLOCK(sec_eh_cfg_lock);

struct trs_sec_eh_ts_inst *sec_eh_ts_inst[TRS_TS_INST_MAX_NUM];

static u32 trs_sec_eh_get_id_bitmap(struct trs_sec_eh_ts_inst *sec_eh_cfg, u32 type)
{
    if (type == TRS_EVENT_ID) {
        return sec_eh_cfg->event_bitmap;
    } else if (type == TRS_NOTIFY_ID) {
        return sec_eh_cfg->notify_bitmap;
    } else {
        return sec_eh_cfg->rtsq_bitmap;
    }
}

static int trs_sec_eh_cfg_get_sqcq_unit_per_bit(struct trs_sec_eh_ts_inst *sec_eh_cfg, u32 *unit_per_bit)
{
    struct trs_msg_id_cap hw_sqcq_id_cap, rsv_sqcq_id_cap;
    int ret;

    ret = trs_host_get_id_cap(&sec_eh_cfg->pm_inst, TRS_HW_CQ_ID, &hw_sqcq_id_cap);
    ret |= trs_host_get_id_cap(&sec_eh_cfg->pm_inst, TRS_RSV_HW_CQ_ID, &rsv_sqcq_id_cap);
    if (ret != 0) {
        return ret;
    }

    *unit_per_bit = (hw_sqcq_id_cap.total_num + rsv_sqcq_id_cap.total_num) / 16; /* id is 16 servings */
    return ret;
}

static int trs_sec_eh_cfg_init(struct trs_sec_eh_ts_inst *sec_eh_cfg)
{
    struct trs_sec_eh_id_info *id_info = NULL;
    struct trs_msg_id_cap id_cap;
    u32 sqcq_num_per_bit;
    int ret, type, j;

    ret = trs_sec_eh_cfg_get_sqcq_unit_per_bit(sec_eh_cfg, &sqcq_num_per_bit);
    if (ret != 0) {
        return ret;
    }

    for (type = 0; type < TRS_CORE_MAX_ID_TYPE; type++) {
        ret = trs_host_get_id_cap(&sec_eh_cfg->pm_inst, type, &id_cap);
        if (ret != 0) {
            trs_warn("Id not support. (devid=%u; tsid=%u; type=%d)\n",
                sec_eh_cfg->pm_inst.devid, sec_eh_cfg->pm_inst.tsid, type);
            continue;
        }

        id_info = &sec_eh_cfg->id_info[type];
        id_info->id_proc_map = (int *)vzalloc(sizeof(int) * id_cap.id_end);
        if (id_info->id_proc_map == NULL) {
            goto free_mem;
        }

        id_info->start = id_cap.id_start;
        id_info->end = id_cap.id_end;
        id_info->type = type;
        id_info->bitnum = 16; /* id is 16 servings */
        id_info->bitmap = trs_sec_eh_get_id_bitmap(sec_eh_cfg, type);
        if ((type == TRS_HW_SQ) || (type == TRS_HW_CQ)) {
            id_info->num_per_bit = sqcq_num_per_bit;
        } else {
            id_info->num_per_bit = (id_cap.id_end - id_cap.id_start) / id_info->bitnum;
        }

        trs_info("Id cap. (devid=%u; tsid=%u; type=%d; maxid=%u; bitmap=%u; num_per_bit=%u; start=%u; end=%u)\n",
            sec_eh_cfg->pm_inst.devid, sec_eh_cfg->pm_inst.tsid, type, id_cap.id_end, id_info->bitmap,
            id_info->num_per_bit, id_cap.id_start, id_cap.id_end);

        if (type == TRS_HW_SQ) {
            sec_eh_cfg->sq_ctx = (struct trs_sec_eh_sq_ctx *)vzalloc(sizeof(struct trs_sec_eh_sq_ctx) * id_cap.id_end);
            if (sec_eh_cfg->sq_ctx == NULL) {
                goto free_mem;
            }
        }
    }

    mutex_init(&sec_eh_cfg->mutex);
    kref_init(&sec_eh_cfg->ref);

    return 0;
free_mem:
    for (j = type; j >= 0; j--) {
        id_info = &sec_eh_cfg->id_info[j];
        if (id_info->id_proc_map != NULL) {
            vfree(id_info->id_proc_map);
        }
        if (j == TRS_HW_SQ) {
            if (sec_eh_cfg->sq_ctx != NULL) {
                vfree(sec_eh_cfg->sq_ctx);
            }
        }
    }
    return -ENOMEM;
}

static int trs_sec_eh_init_mia_res(struct trs_id_inst *inst, struct trs_sec_eh_ts_inst *sec_eh_cfg)
{
    struct res_inst_info res_inst;
    u64 bitmap;
    u32 unit_per_bit;
    int ret;

    soc_resmng_inst_pack(&res_inst, inst->devid, TS_SUBSYS, inst->tsid);

    ret = soc_resmng_get_mia_res(&res_inst, MIA_STARS_RTSQ, &bitmap, &unit_per_bit);
    if (ret != 0) {
        trs_err("Get rtsq bitmap failed. (devid=%u; ret=%d)\n", inst->devid, ret);
        return ret;
    }
    sec_eh_cfg->rtsq_bitmap = (u32)bitmap;

    ret = soc_resmng_get_mia_res(&res_inst, MIA_STARS_EVENT, &bitmap, &unit_per_bit);
    if (ret != 0) {
        trs_err("Get event bitmap failed. (devid=%u; ret=%d)\n", inst->devid, ret);
        return ret;
    }
    sec_eh_cfg->event_bitmap = (u32)bitmap;

    ret = soc_resmng_get_mia_res(&res_inst, MIA_STARS_NOTIFY, &bitmap, &unit_per_bit);
    if (ret != 0) {
        trs_err("Get notify bitmap failed. (devid=%u; ret=%d)\n", inst->devid, ret);
        return ret;
    }
    sec_eh_cfg->notify_bitmap = (u32)bitmap;

    trs_info("Res. (devid=%u; rtsq_bitmap=%x; event_bitmap=%x; notify_bitmap=%x)\n",
        inst->devid, sec_eh_cfg->rtsq_bitmap, sec_eh_cfg->event_bitmap, sec_eh_cfg->notify_bitmap);

    return 0;
}

int trs_sec_eh_ts_inst_create(struct trs_id_inst *inst)
{
    u32 ts_inst_id = trs_id_inst_to_ts_inst(inst);
    struct trs_sec_eh_ts_inst *sec_eh_cfg = NULL;
    struct uda_mia_dev_para mia_para;
    int ret;

    sec_eh_cfg = (struct trs_sec_eh_ts_inst *)kzalloc(sizeof(struct trs_sec_eh_ts_inst), GFP_KERNEL);
    if (sec_eh_cfg == NULL) {
        return -ENOMEM;
    }

    sec_eh_cfg->inst = *inst;

    ret = uda_udevid_to_mia_devid(inst->devid, &mia_para);
    if (ret != 0) {
        kfree(sec_eh_cfg);
        trs_err("Get pf vf id failed. (devid=%u)\n", inst->devid);
        return ret;
    }
    sec_eh_cfg->pm_inst.devid = mia_para.phy_devid;
    sec_eh_cfg->pm_inst.tsid = inst->tsid;

    ret = trs_sec_eh_init_mia_res(inst, sec_eh_cfg);
    if (ret != 0) {
        kfree(sec_eh_cfg);
        return ret;
    }

    if (sec_eh_ts_inst[ts_inst_id] != NULL) {
        kfree(sec_eh_cfg);
        trs_err("Repeat create. (devid=%u; tsid=%u)\n", inst->devid, inst->tsid);
        return -EEXIST;
    }

    ret = trs_sec_eh_cfg_init(sec_eh_cfg);
    if (ret != 0) {
        kfree(sec_eh_cfg);
        trs_err("Cfg init failed. (devid=%u; tsid=%u; ret=%d)\n", inst->devid, inst->tsid, ret);
        return ret;
    }
    sec_eh_ts_inst[ts_inst_id] = sec_eh_cfg;

    return 0;
}

static void trs_sec_eh_ts_inst_release(struct kref *kref)
{
    struct trs_sec_eh_ts_inst *sec_eh_cfg = container_of(kref, struct trs_sec_eh_ts_inst, ref);
    int type;

    if (sec_eh_cfg->sq_ctx != NULL) {
        vfree(sec_eh_cfg->sq_ctx);
    }

    for (type = 0; type < TRS_CORE_MAX_ID_TYPE; type++) {
        if (sec_eh_cfg->id_info[type].id_proc_map != NULL) {
            vfree(sec_eh_cfg->id_info[type].id_proc_map);
        }
    }

    kfree(sec_eh_cfg);
}

void trs_sec_eh_ts_inst_destroy(struct trs_id_inst *inst)
{
    u32 ts_inst_id = trs_id_inst_to_ts_inst(inst);
    struct trs_sec_eh_ts_inst *sec_eh_cfg = NULL;

    if (ts_inst_id >= TRS_TS_INST_MAX_NUM) {
        return;
    }

    write_lock_bh(&sec_eh_cfg_lock);
    sec_eh_cfg = sec_eh_ts_inst[ts_inst_id];
    sec_eh_ts_inst[ts_inst_id] = NULL;
    write_unlock_bh(&sec_eh_cfg_lock);

    if (sec_eh_cfg != NULL) {
        kref_put(&sec_eh_cfg->ref, trs_sec_eh_ts_inst_release);
    }
}

struct trs_sec_eh_ts_inst *trs_sec_eh_ts_inst_get(struct trs_id_inst *inst)
{
    u32 ts_inst_id = trs_id_inst_to_ts_inst(inst);
    struct trs_sec_eh_ts_inst *sec_eh_cfg = NULL;

    if (ts_inst_id >= TRS_TS_INST_MAX_NUM) {
        return NULL;
    }

    read_lock_bh(&sec_eh_cfg_lock);
    sec_eh_cfg = sec_eh_ts_inst[ts_inst_id];
    if (sec_eh_cfg != NULL) {
        kref_get(&sec_eh_cfg->ref);
    }
    read_unlock_bh(&sec_eh_cfg_lock);

    return sec_eh_cfg;
}

void trs_sec_eh_ts_inst_put(struct trs_sec_eh_ts_inst *sec_eh_cfg)
{
    kref_put(&sec_eh_cfg->ref, trs_sec_eh_ts_inst_release);
}

