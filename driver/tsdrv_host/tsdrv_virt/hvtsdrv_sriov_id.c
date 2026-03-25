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
#ifdef CFG_FEATURE_VFIO
#include <linux/spinlock.h>
#include <linux/vmalloc.h>
#include <linux/list.h>
#include <linux/workqueue.h>

#include "devdrv_common.h"
#include "devdrv_cb.h"
#include "tsdrv_device.h"
#include "tsdrv_log.h"
#include "tsdrv_interface.h"
#include "tsdrv_vsq.h"
#include "hvtsdrv_cqsq.h"
#include "hvtsdrv_sriov_id.h"
#include "tsdrv_id.h"

static int hvtsdrv_vf_shm_init(u32 devid, u32 fid, u32 tsid)
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

static void hvtsdrv_vf_shm_destroy(u32 devid, u32 fid, u32 tsid)
{
    struct tsdrv_ts_resource *ts_res = tsdrv_get_ts_resoruce(devid, fid, tsid);

    ts_res->mem_info[DEVDRV_SQ_MEM].bar_addr = 0;
    ts_res->mem_info[DEVDRV_SQ_MEM].phy_addr = 0;
    ts_res->mem_info[DEVDRV_SQ_MEM].size = 0;

    ts_res->mem_info[DEVDRV_INFO_MEM].phy_addr = 0;
    ts_res->mem_info[DEVDRV_INFO_MEM].size = 0;

    ts_res->mem_info[DEVDRV_DOORBELL_MEM].phy_addr = 0;
    ts_res->mem_info[DEVDRV_DOORBELL_MEM].size = 0;
}

static void hvtsdrv_res_ids_uninit(u32 devid, u32 tsid)
{
    struct tsdrv_ts_resource *ts_resource = tsdrv_get_ts_resoruce(devid, TSDRV_PM_FID, tsid);
    enum tsdrv_id_type id_type;

    for (id_type = TSDRV_STREAM_ID; id_type < TSDRV_CMO_ID; id_type++) {
        if (id_type == TSDRV_EVENT_HW_ID || id_type == TSDRV_IPC_EVENT_ID) {
            continue;
        }
        if (ts_resource->id_res[id_type].pm_id != NULL) {
            kfree(ts_resource->id_res[id_type].pm_id);
            ts_resource->id_res[id_type].pm_id = NULL;
        }
    }
}

static int hvtsdrv_res_ids_init(u32 devid, u32 tsid)
{
    struct tsdrv_ts_resource *ts_resource = tsdrv_get_ts_resoruce(devid, TSDRV_PM_FID, tsid);
    struct tsdrv_ts_resource *pm_ts_res = NULL;
    enum tsdrv_id_type id_type;
    u32 phy_devid, vfid;
    int ret;

    ret = devdrv_get_pfvf_id_by_devid(devid, &phy_devid, &vfid);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to get pf and vf id. (devid=%u)\n", devid);
        return ret;
    }

    pm_ts_res = tsdrv_get_ts_resoruce(phy_devid, TSDRV_PM_FID, tsid);
    for (id_type = TSDRV_STREAM_ID; id_type < TSDRV_CMO_ID; id_type++) {
        if (id_type == TSDRV_EVENT_HW_ID || id_type == TSDRV_IPC_EVENT_ID) {
            continue;
        }
        ts_resource->id_res[id_type].pm_id = kzalloc(tsdrv_get_max_id_num(id_type) * sizeof(u32), GFP_KERNEL);
        if (ts_resource->id_res[id_type].pm_id == NULL) {
#ifndef TSDRV_UT
            TSDRV_PRINT_ERR("Failed to kzalloc id mapping table. (devid=%u; id_type=%d; max_num=%u)\n",
                devid, id_type, tsdrv_get_max_id_num(id_type));
            goto err_kzalloc;
#endif
        }
        ts_resource->id_res[id_type].id_addr = pm_ts_res->id_res[id_type].id_addr;
    }

    return 0;

#ifndef TSDRV_UT
err_kzalloc:
    hvtsdrv_res_ids_uninit(devid, tsid);
    return ret;
#endif
}

int hvtsdrv_ids_init(u32 devid, u32 tsid)
{
    int ret;

    ret = hvtsdrv_vf_shm_init(devid, TSDRV_PM_FID, tsid);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Failed to init shm. (devid=%u; tsid=%u; ret=%d)\n", devid, tsid, ret);
        return ret;
#endif
    }
#ifdef CFG_FEATURE_SUPPORT_VSQ_MNG
    ret = tsdrv_vsq_info_mem_create(devid, TSDRV_PM_FID, tsid);
    if (ret != 0) {
        TSDRV_PRINT_ERR("init vsq info failed with ret(%d)\n", ret);
#ifndef TSDRV_UT
        goto err_create_vsq_info;
#endif
    }
#endif

    ret = hvtsdrv_res_ids_init(devid, tsid);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Failed to init res_ids. (devid=%u; tsid=%u; ret=%d)\n", devid, tsid, ret);
        goto err_res_ids_init;
#endif
    }

    return 0;

#ifndef TSDRV_UT
err_res_ids_init:
#ifdef CFG_FEATURE_SUPPORT_VSQ_MNG
    tsdrv_vsq_info_mem_destroy(devid, TSDRV_PM_FID, tsid);
err_create_vsq_info:
#endif
    hvtsdrv_vf_shm_destroy(devid, TSDRV_PM_FID, tsid);
    return ret;
#endif
}

void hvtsdrv_ids_uninit(u32 devid, u32 tsid)
{
    hvtsdrv_res_ids_uninit(devid, tsid);
#ifdef CFG_FEATURE_SUPPORT_VSQ_MNG
    tsdrv_vsq_info_mem_destroy(devid, TSDRV_PM_FID, tsid);
#endif
    hvtsdrv_vf_shm_destroy(devid, TSDRV_PM_FID, tsid);
}
#endif

