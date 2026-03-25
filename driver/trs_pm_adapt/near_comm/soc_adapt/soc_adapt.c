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
* Create: 2022-7-15
*/
#include "devdrv_interface.h"
#include "trs_chip_def_comm.h"
#include "trs_chan_mem.h"
#include "soc_adapt_res_mini_v3.h"
#include "soc_adapt_res_cloud_v2.h"
#include "soc_adapt_res_mini_v2.h"

#include "soc_adapt.h"

static const int trs_soc_chip_type[HISI_CHIP_UNKNOWN] = {
    [HISI_MINI_V1] = TRS_CHIP_TYPE_MINI_V1,
    [HISI_CLOUD_V1] = TRS_CHIP_TYPE_CLOUD_V1,
    [HISI_MINI_V2] = TRS_CHIP_TYPE_MINI_V2,
    [HISI_CLOUD_V2] = TRS_CHIP_TYPE_CLOUD_V2,
    [HISI_MINI_V3] = TRS_CHIP_TYPE_MINI_V3,
};

int trs_soc_get_chip_type(u32 phy_devid)
{
    u32 chip_type = devdrv_get_dev_chip_type(phy_devid);
    if (chip_type >= HISI_CHIP_UNKNOWN) {
        trs_err("Get chip type fail. (devid=%u; chip_type=%u)\n", phy_devid, chip_type);
        return TRS_CHIP_TYPE_UNKNOWN;
    }
    return trs_soc_chip_type[chip_type];
}
EXPORT_SYMBOL(trs_soc_get_chip_type);

int trs_soc_get_hw_type(u32 phy_devid)
{
    return trs_get_hw_type_by_chip_type(trs_soc_get_chip_type(phy_devid));
}

static size_t(*const soc_get_db_stride[TRS_CHIP_TYPE_MAX])(void) = {
    [TRS_CHIP_TYPE_MINI_V3] = trs_soc_get_mini_v3_db_stride,
    [TRS_CHIP_TYPE_CLOUD_V2] = trs_soc_get_cloud_v2_db_stride,
    [TRS_CHIP_TYPE_MINI_V2] = trs_soc_get_mini_v2_db_stride,
};
int trs_soc_get_db_stride(struct trs_id_inst *inst, size_t *size)
{
    int chip_type = trs_soc_get_chip_type(inst->devid);
    if (chip_type == TRS_CHIP_TYPE_UNKNOWN) {
        return -ENODEV;
    }

    if (soc_get_db_stride[chip_type] != NULL) {
        *size = soc_get_db_stride[chip_type]();
        return 0;
    }

    return -EINVAL;
}

static int(*const soc_get_db_cfg[TRS_CHIP_TYPE_MAX])(int db_type, u32 *start, u32 *end) = {
    [TRS_CHIP_TYPE_MINI_V3] = trs_soc_get_mini_v3_db_cfg,
    [TRS_CHIP_TYPE_CLOUD_V2] = trs_soc_get_cloud_v2_db_cfg,
    [TRS_CHIP_TYPE_MINI_V2] = trs_soc_get_mini_v2_db_cfg,
};
int trs_soc_get_db_cfg(struct trs_id_inst *inst, int db_type, u32 *start, u32 *end)
{
    int chip_type = trs_soc_get_chip_type(inst->devid);
    int ret = -EINVAL;

    if (chip_type == TRS_CHIP_TYPE_UNKNOWN) {
        return -ENODEV;
    }

    if (soc_get_db_cfg[chip_type] != NULL) {
        ret = soc_get_db_cfg[chip_type](db_type, start, end);
    }

    return ret;
}

static size_t(*const soc_get_notify_size[TRS_CHIP_TYPE_MAX])(void) = {
    [TRS_CHIP_TYPE_MINI_V3] = trs_soc_get_mini_v3_notify_size,
    [TRS_CHIP_TYPE_CLOUD_V2] = trs_soc_get_cloud_v2_notify_size,
    [TRS_CHIP_TYPE_MINI_V2] = trs_soc_get_mini_v2_notify_size,
};
int trs_soc_get_notify_size(struct trs_id_inst *inst, size_t *notify_size)
{
    int chip_type = trs_soc_get_chip_type(inst->devid);
    if (chip_type == TRS_CHIP_TYPE_UNKNOWN) {
        return -ENODEV;
    }

    if (soc_get_notify_size[chip_type] != NULL) {
        *notify_size = soc_get_notify_size[chip_type]();
        return 0;
    }

    return -EINVAL;
}

static u32(*const soc_get_notify_offset[TRS_CHIP_TYPE_MAX])(u32 notify_id) = {
    [TRS_CHIP_TYPE_MINI_V3] = trs_soc_get_mini_v3_notify_offset,
    [TRS_CHIP_TYPE_CLOUD_V2] = trs_soc_get_cloud_v2_notify_offset,
    [TRS_CHIP_TYPE_MINI_V2] = trs_soc_get_mini_v2_notify_offset,
};

int trs_soc_get_notify_offset(struct trs_id_inst *inst, u32 notify_id, u32 *offset)
{
    int chip_type = trs_soc_get_chip_type(inst->devid);
    if (chip_type == TRS_CHIP_TYPE_UNKNOWN) {
        return -ENODEV;
    }

    if (soc_get_notify_offset[chip_type] != NULL) {
        *offset = soc_get_notify_offset[chip_type](notify_id);
        return 0;
    }

    return -EINVAL;
}

static u32(*const soc_get_sq_mem_side[TRS_CHIP_TYPE_MAX])(u32 devid) = {
    [TRS_CHIP_TYPE_MINI_V3] = trs_soc_get_mini_v3_sq_mem_side,
    [TRS_CHIP_TYPE_CLOUD_V2] = trs_soc_get_cloud_v2_sq_mem_side,
    [TRS_CHIP_TYPE_MINI_V2] = trs_soc_get_mini_v2_sq_mem_side,
};
u32 trs_soc_get_sq_mem_side(struct trs_id_inst *inst)
{
    int chip_type = trs_soc_get_chip_type(inst->devid);
    u32 sq_mem_side = TRS_CHAN_DEV_MEM;

    if (chip_type == TRS_CHIP_TYPE_UNKNOWN) {
        return sq_mem_side;
    }

    if (soc_get_sq_mem_side[chip_type] != NULL) {
        sq_mem_side = soc_get_sq_mem_side[chip_type](inst->devid);
    }

    return sq_mem_side;
}

static u32(*const soc_get_cq_mem_side[TRS_CHIP_TYPE_MAX])(u32 devid) = {
    [TRS_CHIP_TYPE_MINI_V3] = trs_soc_get_mini_v3_cq_mem_side,
    [TRS_CHIP_TYPE_CLOUD_V2] = trs_soc_get_cloud_v2_cq_mem_side,
    [TRS_CHIP_TYPE_MINI_V2] = trs_soc_get_mini_v2_cq_mem_side,
};
u32 trs_soc_get_cq_mem_side(struct trs_id_inst *inst)
{
    int chip_type = trs_soc_get_chip_type(inst->devid);
    u32 cq_mem_side = TRS_CHAN_DEV_MEM;

    if (chip_type == TRS_CHIP_TYPE_UNKNOWN) {
        return cq_mem_side;
    }

    if (soc_get_cq_mem_side[chip_type] != NULL) {
        cq_mem_side = soc_get_cq_mem_side[chip_type](inst->devid);
    }

    return cq_mem_side;
}

static size_t(*const soc_get_stars_sched_stride[TRS_CHIP_TYPE_MAX])(void) = {
    [TRS_CHIP_TYPE_MINI_V3] = trs_soc_get_mini_v3_stars_sched_stride,
    [TRS_CHIP_TYPE_CLOUD_V2] = trs_soc_get_cloud_v2_stars_sched_stride,
};
int trs_soc_get_stars_sched_stride(struct trs_id_inst *inst, size_t *stride)
{
    int chip_type = trs_soc_get_chip_type(inst->devid);
    if (chip_type == TRS_CHIP_TYPE_UNKNOWN) {
        return -ENODEV;
    }

    if (soc_get_stars_sched_stride[chip_type] != NULL) {
        *stride = soc_get_stars_sched_stride[chip_type]();
        return 0;
    }

    return -EINVAL;
}

static int(*const soc_get_hwcq_rsv_mem_type[TRS_CHIP_TYPE_MAX])(void) = {
    [TRS_CHIP_TYPE_MINI_V3] = trs_soc_get_mini_v3_hwcq_rsv_mem_type,
    [TRS_CHIP_TYPE_CLOUD_V2] = trs_soc_get_cloud_v2_hwcq_rsv_mem_type,
    [TRS_CHIP_TYPE_MINI_V2] = trs_soc_get_mini_v2_hwcq_rsv_mem_type,
};
int trs_soc_get_hwcq_rsv_mem_type(struct trs_id_inst *inst, int *rsv_mem_type)
{
    int chip_type = trs_soc_get_chip_type(inst->devid);
    if (chip_type == TRS_CHIP_TYPE_UNKNOWN) {
        return -ENODEV;
    }

    if (soc_get_hwcq_rsv_mem_type[chip_type] != NULL) {
        *rsv_mem_type = soc_get_hwcq_rsv_mem_type[chip_type]();
        return 0;
    }

    return -EINVAL;
}

