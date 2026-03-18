/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
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
#include <linux/bitmap.h>
#include "devdrv_interface.h"
#include "devdrv_devinit.h"
#include "tsdrv_capacity.h"
#include "tsdrv_device.h"
#include "devdrv_cb.h"

#include "hvtsdrv_tsagent.h"
#include "tsdrv_id.h"
#ifdef AOS_LLVM_BUILD
#include "devdrv_manager_comm.h"

STATIC int get_aicore_num_by_dms(u32 devid, u32 fid, u32 *aicore_num)
{
    struct devdrv_info *dev_info = NULL;

    if ((aicore_num == NULL) || (fid != 0)) {
        TSDRV_PRINT_ERR("invalid parameter, dev_id = %u, fid = %u.\n", devid, fid);
        return -EINVAL;
    }

    dev_info = dms_get_devinfo(devid);
    if (dev_info == NULL) {
        TSDRV_PRINT_ERR("device manager is not initialized, dev_id = %u\n", devid);
        return -ENODEV;
    }

    *aicore_num = dev_info->ai_core_num;
    return 0;
}
#endif

STATIC int tsdrv_set_capacity_ratio(u32 devid, u32 fid, enum TSDRV_CAPACITY capacity)
{
    struct tsdrv_dev_resource *dev_res = tsdrv_get_dev_resource(devid, fid);
    u32 aicore_num = 0;
    u32 cap_num = 0;
    int ret;
    int ratio = 0;

    if (capacity == FULL_CAPACITY) {
        return CAPACITY_RATIO_1;
    }
#ifndef TSDRV_UT
    if (capacity != INVALID_CAPACITY) {
#ifndef AOS_LLVM_BUILD
        ret = hvdevmng_get_aicore_num(devid, TSDRV_PM_FID, &aicore_num);
#else
        ret = get_aicore_num_by_dms(devid, TSDRV_PM_FID, &aicore_num);
#endif
        if ((ret != 0) || (aicore_num == 0)) {
            TSDRV_PRINT_ERR("(devid=%u; fid=%u; ret=%d; aicore_num=%u)\n",
                devid, fid, ret, aicore_num);
            return ret;
        }
    }
    cap_num = 1 << (u32)capacity;
    ratio = aicore_num / cap_num;
    TSDRV_PRINT_INFO("(devid=%u; fid=%u; capacity=%u; cap_num=%u; aicore_num=%u; ratio=%d)\n",
        devid, fid, (u32)capacity, cap_num, aicore_num, ratio);
    dev_res->aicore_num = aicore_num;
    dev_res->cap_num = cap_num;
#endif
    return ratio;
}

int tsdrv_set_ts_capacity(struct tsdrv_ts_resource *ts_res, enum TSDRV_CAPACITY capacity)
{
    struct tsdrv_device *tsdrv_dev = tsdrv_res_to_dev(ts_res);
    struct tsdrv_dev_resource *dev_res = container_of(ts_res, struct tsdrv_dev_resource,
        ts_resource[ts_res->tsid]);
    struct tsdrv_id_capacity id_cap = {0};
    u32 aicore_num = 32; /* inited */
    u32 tsid, id_type;
    int ratio;

    /* default value */
    dev_res->aicore_num = aicore_num;
    dev_res->cap_num = aicore_num;

    for (tsid = 0; tsid < tsdrv_get_dev_tsnum(tsdrv_dev->devid); tsid++) {
        if (capacity == INVALID_CAPACITY) {
            for (id_type = TSDRV_STREAM_ID; id_type < TSDRV_MAX_ID; id_type++) {
                ts_res[tsid].id_res[id_type].id_capacity = TSDRV_INVALIC_CAPACITY;
            }
            continue;
        }
        ratio = tsdrv_set_capacity_ratio(tsdrv_dev->devid, dev_res->fid, capacity);
        if (ratio <= 0) {
            TSDRV_PRINT_ERR("Failed to set capacity ratio. (devid=%u; fid=%u; capacity=%d; ratio=%d)\n",
                tsdrv_dev->devid, dev_res->fid, (int)capacity, ratio);
            return -EINVAL;
        }

        ts_res[tsid].id_res[TSDRV_SQ_ID].id_capacity = DEVDRV_MAX_SQ_NUM / ratio - RESERVED_NUM;
        ts_res[tsid].id_res[TSDRV_CQ_ID].id_capacity =
            (DEVDRV_MAX_CQ_NUM - CALLBACK_MAX_CQ_NUM) / ratio - RESERVED_NUM;
        ts_res[tsid].id_res[TSDRV_STREAM_ID].id_capacity =
            tsdrv_get_stream_id_max_num(tsdrv_dev->devid, tsid) / ratio;
        ts_res[tsid].id_res[TSDRV_EVENT_HW_ID].id_capacity =
            (DEVDRV_MAX_HW_EVENT_ID - DEVDRV_MAX_IPC_EVENT_ID) / ratio;
        ts_res[tsid].id_res[TSDRV_EVENT_SW_ID].id_capacity =
            (DEVDRV_MAX_SW_EVENT_ID - DEVDRV_MAX_HW_EVENT_ID - 1) / ratio;
        ts_res[tsid].id_res[TSDRV_MODEL_ID].id_capacity =
            tsdrv_get_model_id_max_num(tsdrv_dev->devid, tsid) / ratio;
#if defined(CFG_SOC_PLATFORM_MINI) && !defined(CFG_SOC_PLATFORM_MINIV2) && !defined(CFG_SOC_PLATFORM_MINIV3)
        ts_res[tsid].id_res[TSDRV_NOTIFY_ID].id_capacity = 0;
#else
        ts_res[tsid].id_res[TSDRV_NOTIFY_ID].id_capacity = DEVDRV_MAX_NOTIFY_ID / ratio;
#endif
#ifdef CFG_FEATURE_CMO
        ts_res[tsid].id_res[TSDRV_CMO_ID].id_capacity = (DEVDRV_MAX_CMO_ID - 1) / ratio;
#else
        ts_res[tsid].id_res[TSDRV_CMO_ID].id_capacity = 0;
#endif

#ifdef CFG_SOC_PLATFORM_CLOUD_V2
        if (tsdrv_get_pfvf_type_by_devid(tsdrv_dev->devid) == DEVDRV_SRIOV_TYPE_VF) {
            ts_res[tsid].id_res[TSDRV_EVENT_HW_ID].id_capacity = 0;
            ts_res[tsid].id_res[TSDRV_EVENT_SW_ID].id_capacity = DEVDRV_MAX_SW_EVENT_ID / ratio;
        }
#endif
        TSDRV_PRINT_INFO("sq(%u), cq(%u), stream(%u), event hw(%u), event sw(%u), model(%u), notify(%u),"
            "cmo(%u).\n",
            ts_res[tsid].id_res[TSDRV_SQ_ID].id_capacity,
            ts_res[tsid].id_res[TSDRV_CQ_ID].id_capacity,
            ts_res[tsid].id_res[TSDRV_STREAM_ID].id_capacity,
            ts_res[tsid].id_res[TSDRV_EVENT_HW_ID].id_capacity,
            ts_res[tsid].id_res[TSDRV_EVENT_SW_ID].id_capacity,
            ts_res[tsid].id_res[TSDRV_MODEL_ID].id_capacity,
            ts_res[tsid].id_res[TSDRV_NOTIFY_ID].id_capacity,
            ts_res[tsid].id_res[TSDRV_CMO_ID].id_capacity);

        id_cap.stream_capacity = ts_res[tsid].id_res[TSDRV_STREAM_ID].id_capacity;
        id_cap.event_capacity = ts_res[tsid].id_res[TSDRV_EVENT_SW_ID].id_capacity +
            ts_res[tsid].id_res[TSDRV_EVENT_HW_ID].id_capacity;
        id_cap.notify_capacity = ts_res[tsid].id_res[TSDRV_NOTIFY_ID].id_capacity;
        id_cap.model_capacity = ts_res[tsid].id_res[TSDRV_MODEL_ID].id_capacity;
#ifndef AOS_LLVM_BUILD
        hvdevmng_set_dev_ts_resource(tsdrv_dev->devid, dev_res->fid, tsid, (void *)&id_cap);
#endif
    }
    return 0;
}

int tsdrv_get_ts_capacity(struct tsdrv_ts_resource *ts_res, struct tsdrv_id_capacity *capacity)
{
    capacity->stream_capacity = ts_res->id_res[TSDRV_STREAM_ID].id_capacity;
    capacity->event_capacity  = ts_res->id_res[TSDRV_EVENT_SW_ID].id_capacity +
        ts_res->id_res[TSDRV_EVENT_HW_ID].id_capacity;
    capacity->model_capacity  = ts_res->id_res[TSDRV_MODEL_ID].id_capacity;
    capacity->notify_capacity = ts_res->id_res[TSDRV_NOTIFY_ID].id_capacity;
    capacity->sq_capacity     = ts_res->id_res[TSDRV_SQ_ID].id_capacity;
    capacity->cq_capacity     = ts_res->id_res[TSDRV_CQ_ID].id_capacity;
    capacity->cmo_capacity    = ts_res->id_res[TSDRV_CMO_ID].id_capacity;

    TSDRV_PRINT_DEBUG("[devid-%u tsid-%u fid-%u] capacity:stream(%u),event(%u),model(%u),notify(%u),sq(%u),cq(%u),"
        "cmo(%u)\n",
        tsdrv_get_devid_by_res(ts_res), ts_res->tsid, tsdrv_get_fid_by_res(ts_res),
        capacity->stream_capacity,
        capacity->event_capacity,
        capacity->model_capacity,
        capacity->notify_capacity,
        capacity->sq_capacity,
        capacity->cq_capacity,
        capacity->cmo_capacity);
    return 0;
}

