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
 * Create: 2022-05-21
 */
#include <linux/spinlock.h>
#include <linux/list.h>
#include "virtmng_interface.h"
#include "tsdrv_kernel_common.h"
#include "tsdrv_device.h"
#include "devdrv_devinit.h"
#include "devdrv_shm.h"
#include "devdrv_id.h"
#include "dvtsdrv_id.h"
#include "hvtsdrv_id.h"
#include "tsdrv_id.h"
#include "tsdrv_capacity.h"
#ifdef CFG_FEATURE_CDQM
#include "tsdrv_cdqm_module.h"
#endif

bool tsdrv_check_id_is_belong_to_vf(u32 bitmap, u32 sum, u32 index)
{
    if ((bitmap & (0x1U << (index / (sum / TSDRV_MAX_VM_FID_NUM)))) != 0) {
        return true;
    }

    return false;
}

void dvtsdrv_free_reserved_ids(u32 devid, u32 fid, u32 tsid)
{
    struct tsdrv_ts_resource *ts_res = tsdrv_get_ts_resoruce(devid, fid, tsid);
    u32 sqid, cqid, id;
    u32 sqcq_reserved_num = ts_res->sqcq_reserved_num;

    for (id = 0; id < sqcq_reserved_num; id++) {
        sqid = ts_res->reserved_sqcq[id].sq_info.index;
        cqid = ts_res->reserved_sqcq[id].cq_info.index;
        tsdrv_kernel_free_sq(devid, fid, tsid, sqid);
        tsdrv_kernel_free_cq(devid, fid, tsid, cqid);
        ts_res->sqcq_reserved_num--;
        TSDRV_PRINT_DEBUG("Free researved sqcq. (devid=%u; sqid=%u; cqid=%u; sqcq_reserved_num=%u)\n",
            devid, sqid, cqid, ts_res->sqcq_reserved_num);
    }
}

void dvtsdrv_res_id_unint(u32 devid, u32 fid, u32 tsid, enum tsdrv_id_type id_type)
{
    struct tsdrv_ts_resource *ts_res = tsdrv_get_ts_resoruce(devid, fid, tsid);
    struct tsdrv_id_info *id_info = NULL;
    u32 num, id, virt_id, id_capacity;

    TSDRV_PRINT_DEBUG("Uninit res_id. (devid=%u; fid=%u; tsid=%u; id_type=%d; id_available_num=%u)\n",
        devid, fid, tsid, id_type, ts_res->id_res[id_type].id_available_num);

    spin_lock(&ts_res->id_res[id_type].spinlock);
    num = ts_res->id_res[id_type].id_available_num;
    for (id = 0; id < num; id++) {
        id_info = list_first_entry(&ts_res->id_res[id_type].id_available_list,
            struct tsdrv_id_info, list);
        list_del(&id_info->list);
        ts_res->id_res[id_type].id_available_num--;
    }
    /* because some ids have synchronized to host, not in device available list,
       so traverse all virtual ids to atomic_dec ref */
    id_capacity = ts_res->id_res[id_type].id_capacity;
    if (id_type == TSDRV_SQ_ID || id_type == TSDRV_CQ_ID) {
        id_capacity += RESERVED_NUM;
    }
    for (virt_id = 0; virt_id < id_capacity; virt_id++) {
        if (id_type == TSDRV_EVENT_HW_ID) {
            continue;
        }
        id = tsdrv_vrit_to_physic_id(&ts_res->id_res[id_type], virt_id);
        id_info = devdrv_find_one_id(&ts_res->id_res[id_type], id);
        atomic_dec(&id_info->ref);
    }
    spin_unlock(&ts_res->id_res[id_type].spinlock);

    ts_res->id_res[id_type].id_addr = NULL;
    if (ts_res->id_res[id_type].pm_id != NULL) {
        kfree(ts_res->id_res[id_type].pm_id);
        ts_res->id_res[id_type].pm_id = NULL;
    }
}

static void dvtsdrv_res_id_restore(u32 devid, u32 fid, u32 tsid, enum tsdrv_id_type id_type)
{
    struct tsdrv_ts_resource *ts_res = tsdrv_get_ts_resoruce(devid, fid, tsid);
    struct tsdrv_id_info *id_info = NULL;
    u32 id;

    id = (id_type == TSDRV_SQ_ID || id_type == TSDRV_CQ_ID) ? TSDRV_SQCQ_RESEARVED_MAX_NUM : 0;
    spin_lock(&ts_res->id_res[id_type].spinlock);
    for (; id < tsdrv_get_max_id_num(id_type); id++) {
        id_info = devdrv_find_one_id(&ts_res->id_res[id_type], id);
        list_add_tail(&id_info->list, &ts_res->id_res[id_type].id_available_list);
        ts_res->id_res[id_type].id_available_num++;
    }
    spin_unlock(&ts_res->id_res[id_type].spinlock);
}

void dvtsdrv_res_ids_restore(u32 devid, u32 fid, u32 tsid)
{
    enum tsdrv_id_type id_type;

    for (id_type = TSDRV_STREAM_ID; id_type < TSDRV_MAX_ID; id_type++) {
        if (id_type == TSDRV_EVENT_HW_ID || id_type == TSDRV_IPC_EVENT_ID || id_type == TSDRV_CMO_ID) {
            continue;
        }
        dvtsdrv_res_id_restore(devid, fid, tsid, id_type);
    }
}

static int dvtsdrv_res_id_config(u32 devid, u32 fid, u32 tsid, u32 bitmap, enum tsdrv_id_type id_type)
{
    struct tsdrv_ts_resource *ts_res = tsdrv_get_ts_resoruce(devid, fid, tsid);
    struct tsdrv_ts_resource *pm_ts_res = NULL;
    struct tsdrv_id_info *id_info = NULL;
    u32 phy_devid, vfid, id;
    u32 virt_id = 0;
    int ret;

    ret = devdrv_get_pfvf_id_by_devid(devid, &phy_devid, &vfid);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to get pf and vf id. (devid=%u)\n", devid);
        return ret;
    }
    pm_ts_res = tsdrv_get_ts_resoruce(phy_devid, TSDRV_PM_FID, tsid);

    TSDRV_PRINT_DEBUG("(devid=%u; phy_devid=%u; fid=%u; tsid=%u; id_type=%d; id_num=%u)\n",
        devid, phy_devid, fid, tsid, id_type, ts_res->id_res[id_type].id_capacity);
    for (id = 0; id < tsdrv_get_max_id_num(id_type); id++) {
        if (tsdrv_check_id_is_belong_to_vf(bitmap, tsdrv_get_max_id_num(id_type), id)) {
            spin_lock(&pm_ts_res->id_res[id_type].spinlock);
            id_info = devdrv_find_one_id(&pm_ts_res->id_res[id_type], id);
            if (atomic_read(&id_info->ref) != 0) {
#ifndef TSDRV_UT
                spin_unlock(&pm_ts_res->id_res[id_type].spinlock);
                TSDRV_PRINT_ERR("This id range has been used. (devid=%u; fid=%u; id_type=%d; id=%u; bitmap=0x%x)\n",
                    devid, fid, id_type, id, bitmap);
#endif
                return -EINVAL;
            }
            id_info->virt_id = virt_id++;
            id_info->id = id;   /* id use phy_id currently */
            atomic_inc(&id_info->ref);
            spin_unlock(&pm_ts_res->id_res[id_type].spinlock);
            spin_lock(&ts_res->id_res[id_type].spinlock);
            list_add_tail(&id_info->list, &ts_res->id_res[id_type].id_available_list);
            ts_res->id_res[id_type].pm_id[id_info->virt_id] = id_info->phy_id;
            ts_res->id_res[id_type].id_available_num++;
            spin_unlock(&ts_res->id_res[id_type].spinlock);
        }
    }
    TSDRV_PRINT_INFO("Config id to VF. (devid=%u; fid=%u; tsid=%u; id_type=%d, virt_id=%u; phy_id=%u; "
        "bitmap=0x%x; id_available_num=%u)\n", devid, fid, tsid, id_type, id_info->virt_id,
        id_info->phy_id, bitmap, ts_res->id_res[id_type].id_available_num);
    ts_res->id_res[id_type].id_addr = pm_ts_res->id_res[id_type].id_addr;
    return 0;
}

int dvtsdrv_res_id_init(u32 devid, u32 fid, u32 tsid, vf_ac_info_t *ts_ac_info, enum tsdrv_id_type id_type)
{
    struct tsdrv_ts_resource *ts_res = tsdrv_get_ts_resoruce(devid, fid, tsid);
    u32 bitmap, id_cnt;
    int ret;

    if (id_type == TSDRV_EVENT_HW_ID || id_type == TSDRV_IPC_EVENT_ID || id_type == TSDRV_CMO_ID) {
        return 0;
    }

    if (ts_res->id_res[id_type].id_available_num != 0) {
        TSDRV_PRINT_ERR("This VF has been init. (devid=%u; fid=%u; id_type=%d, id_available_num=%u)\n",
            devid, fid, id_type, ts_res->id_res[id_type].id_available_num);
        return -EAGAIN;
    }

    id_cnt = ts_res->id_res[id_type].id_capacity;
    bitmap = ts_ac_info->rtsq_slice_bitmap;
    if (id_type == TSDRV_EVENT_SW_ID) {
        bitmap = ts_ac_info->event_slice_bitmap;
    }
    if (id_type == TSDRV_NOTIFY_ID) {
        bitmap = ts_ac_info->notify_slice_bitmap;
    }
    if (id_type == TSDRV_SQ_ID || id_type == TSDRV_CQ_ID) {
        id_cnt += RESERVED_NUM;
    }
    TSDRV_PRINT_DEBUG("(devid=%u; fid=%u; id_type=%d; id_cnt=%u; bitmap=0x%x)\n", devid, fid, id_type, id_cnt, bitmap);
    ts_res->id_res[id_type].pm_id = kzalloc(id_cnt * sizeof(u32), GFP_KERNEL);
    if (ts_res->id_res[id_type].pm_id == NULL) {
        TSDRV_PRINT_ERR("Failed to kzalloc ID mapping table. (devid=%u; id_type=%d)\n", devid, id_type);
        return -ENOMEM;
    }

    ret = dvtsdrv_res_id_config(devid, fid, tsid, bitmap, id_type);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to confid id. (devid=%u; fid=%u; id_type=%d, bitmap=0x%x)\n",
            devid, fid, id_type, bitmap);
        kfree(ts_res->id_res[id_type].pm_id);
        return ret;
    }
    return 0;
}

int dvtsdrv_res_ids_init(u32 devid, u32 fid, u32 tsid, vf_ac_info_t *ts_ac_info)
{
    enum tsdrv_id_type id_type, i;
    int ret;

    for (id_type = TSDRV_STREAM_ID; id_type < TSDRV_MAX_ID; id_type++) {
        ret = dvtsdrv_res_id_init(devid, fid, tsid, ts_ac_info, id_type);
        if (ret != 0) {
            TSDRV_PRINT_ERR("Failed to init id recourse. (devid=%u; fid=%u; id_type=%d)\n",
                devid, fid, id_type);
            goto err_res_id_init;
        }
    }

    return 0;

err_res_id_init:
    for (i = TSDRV_STREAM_ID; i < id_type; i++) {
        dvtsdrv_res_id_unint(devid, fid, tsid, i);
    }
    return ret;
}

void dvtsdrv_res_ids_uninit(u32 devid, u32 fid, u32 tsid)
{
    enum tsdrv_id_type id_type;

    for (id_type = TSDRV_STREAM_ID; id_type < TSDRV_MAX_ID; id_type++) {
        dvtsdrv_res_id_unint(devid, fid, tsid, id_type);
    }
}

static int dvtsdrv_shm_init(u32 devid, u32 fid, u32 tsid)
{
    struct tsdrv_ts_resource *ts_res = tsdrv_get_ts_resoruce(devid, fid, tsid);
    struct tsdrv_ts_resource *pm_ts_res = NULL;
    u32 phy_devid, vfid;
    int ret;

    ret = devdrv_get_pfvf_id_by_devid(devid, &phy_devid, &vfid);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to get pf and vf id. (devid=%u)\n", devid);
        return ret;
    }
    pm_ts_res = tsdrv_get_ts_resoruce(phy_devid, TSDRV_PM_FID, tsid);

    ts_res->mem_info[DEVDRV_SQ_MEM].bar_addr = pm_ts_res->mem_info[DEVDRV_SQ_MEM].bar_addr;
    ts_res->mem_info[DEVDRV_SQ_MEM].phy_addr = pm_ts_res->mem_info[DEVDRV_SQ_MEM].phy_addr;
    ts_res->mem_info[DEVDRV_SQ_MEM].virt_addr = pm_ts_res->mem_info[DEVDRV_SQ_MEM].virt_addr;
    ts_res->mem_info[DEVDRV_SQ_MEM].size = DEVDRV_RESERVE_MEM_SIZE;

    ts_res->mem_info[DEVDRV_INFO_MEM].phy_addr = pm_ts_res->mem_info[DEVDRV_INFO_MEM].phy_addr;
    ts_res->mem_info[DEVDRV_INFO_MEM].virt_addr = pm_ts_res->mem_info[DEVDRV_INFO_MEM].virt_addr;
    ts_res->mem_info[DEVDRV_INFO_MEM].size = DEVDRV_VM_INFO_MEM_SIZE;

    ts_res->mem_info[DEVDRV_DOORBELL_MEM].phy_addr = pm_ts_res->mem_info[DEVDRV_DOORBELL_MEM].phy_addr;
    ts_res->mem_info[DEVDRV_DOORBELL_MEM].size = pm_ts_res->mem_info[DEVDRV_DOORBELL_MEM].size;

    return 0;
}

static void dvtsdrv_shm_destroy(u32 devid, u32 fid, u32 tsid)
{
    struct tsdrv_ts_resource *ts_res = tsdrv_get_ts_resoruce(devid, fid, tsid);

    ts_res->mem_info[DEVDRV_SQ_MEM].bar_addr = 0;
    ts_res->mem_info[DEVDRV_SQ_MEM].phy_addr = 0;
    ts_res->mem_info[DEVDRV_SQ_MEM].virt_addr = 0;
    ts_res->mem_info[DEVDRV_SQ_MEM].size = 0;

    ts_res->mem_info[DEVDRV_INFO_MEM].phy_addr = 0;
    ts_res->mem_info[DEVDRV_INFO_MEM].virt_addr = 0;
    ts_res->mem_info[DEVDRV_INFO_MEM].size = 0;

    ts_res->mem_info[DEVDRV_DOORBELL_MEM].phy_addr = 0;
    ts_res->mem_info[DEVDRV_DOORBELL_MEM].size = 0;
}

int dvtsdrv_ids_init(u32 devid, u32 fid, u32 tsid, vf_ac_info_t *ts_ac_info)
{
    int ret;

    ret = dvtsdrv_shm_init(devid, fid, tsid);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to init shm. (devid=%u; fid=%u; ret=%d)\n", devid, fid, ret);
        return ret;
    }

    ret = dvtsdrv_res_ids_init(devid, fid, tsid, ts_ac_info);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to init sqcq. (devid=%u; fid=%u; ret=%d)\n", devid, fid, ret);
        return ret;
    }

    return 0;
}

void dvtsdrv_ids_destroy(u32 devid, u32 fid, u32 tsid)
{
    dvtsdrv_res_ids_uninit(devid, fid, tsid);
    dvtsdrv_shm_destroy(devid, fid, tsid);
}

#ifdef CFG_FEATURE_CDQM
static int dvtsdrv_cdqm_drv_init(u32 devid, u32 tsnum)
{
    return 0;
}

static void dvtsdrv_cdqm_drv_uninit(u32 devid, u32 tsnum)
{
}
#endif

int dvtsdrv_basic_module_init(u32 devid, u32 fid, u32 tsnum, vf_ac_info_t *ts_ac_info)
{
    u32 tsid, i;
    int ret;

    ret = tsdrv_mbox_init(devid, tsnum);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to init mailbox. (devid=%u; ret=%d)\n", devid, ret);
        return ret;
    }

    ret = tsdrv_cq_irq_init(devid, tsnum);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to init cq interrupts. (devid=%u; ret=%d)\n", devid, ret);
        goto err_cb_irq_init;
    }

    for (tsid = 0; tsid < tsnum; tsid++) {
        ret = dvtsdrv_ids_init(devid, fid, tsid, ts_ac_info);
        if (ret != 0) {
            TSDRV_PRINT_ERR("Failed to init ids. (devid=%u; fid=%u; ret=%d)\n", devid, fid, ret);
            goto err_ids_init;
        }
    }

#ifdef CFG_FEATURE_CDQM
    ret = dvtsdrv_cdqm_drv_init(devid, tsnum);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to init cdqm. (devid=%u; ret=%d)\n", devid, ret);
        goto err_cdqm_drv_init;
    }
#endif

    return 0;

#ifdef CFG_FEATURE_CDQM
err_cdqm_drv_init:
#endif
err_ids_init:
    for (i = 0; i < tsid; i++) {
        dvtsdrv_ids_destroy(devid, fid, i);
    }
    tsdrv_cq_irq_exit(devid, tsnum);
err_cb_irq_init:
    tsdrv_mbox_exit(devid, tsnum);
    return ret;
}

void dvtsdrv_basic_module_uninit(u32 devid, u32 fid, u32 tsnum)
{
    u32 tsid;
#ifdef CFG_FEATURE_CDQM
    dvtsdrv_cdqm_drv_uninit(devid, tsnum);
#endif
    for (tsid = 0; tsid < tsnum; tsid++) {
        dvtsdrv_ids_destroy(devid, fid, tsid);
    }
    tsdrv_cq_irq_exit(devid, tsnum);
    tsdrv_mbox_exit(devid, tsnum);
}

