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
#include "tsdrv_hwinfo.h"
#include "tsdrv_device.h"
#include "devdrv_common.h"
#include "dvtsdrv_sriov_hwinfo.h"

static int tsdrv_set_vdev_stars_sqcq_intr_hwinfo(u32 devid, u32 tsnum)
{
    struct tsdrv_stars_sqcq_intr_hwinfo *pf_stars_sqcq_intr_hwinfo = NULL;
    u32 phy_devid, vfid, tsid;
    int ret;

    ret = devdrv_get_pfvf_id_by_devid(devid, &phy_devid, &vfid);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Failed to get pf and vf id. (devid=%u)\n", devid);
        return ret;
#endif
    }
    for (tsid = 0; tsid < tsnum; tsid++) {
        pf_stars_sqcq_intr_hwinfo = tsdrv_get_stars_sqcq_intr_hwinfo(phy_devid, tsid);
        ret = tsdrv_set_stars_sqcq_intr_hwinfo(devid, tsid,
            pf_stars_sqcq_intr_hwinfo->pa, pf_stars_sqcq_intr_hwinfo->size);
        if (ret != 0) {
            return ret;
        }
    }

    return 0;
}

static int tsdrv_set_vdev_tsfw_hwinfo(u32 devid, u32 tsnum)
{
    struct tsdrv_tsfw_hwinfo *pf_tsfw_hwinfo = NULL;
    u32 phy_devid, vfid, tsid;
    int ret;

    ret = devdrv_get_pfvf_id_by_devid(devid, &phy_devid, &vfid);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Failed to get pf and vf id. (devid=%u)\n", devid);
        return ret;
#endif
    }
    for (tsid = 0; tsid < tsnum; tsid++) {
        pf_tsfw_hwinfo = tsdrv_get_tsfw_hwinfo(phy_devid, tsid);
        ret = tsdrv_set_tsfw_hwinfo(devid, tsid, pf_tsfw_hwinfo);
        if (ret != 0) {
            return -ENODEV;
        }
    }

    return 0;
}

static int tsdrv_set_vdev_mbox_hwinfo(u32 devid, u32 tsnum)
{
    struct tsdrv_mbox_hwinfo *pf_mbox_hwinfo = NULL;
    u32 phy_devid, vfid, tsid;
    int ret;

    ret = devdrv_get_pfvf_id_by_devid(devid, &phy_devid, &vfid);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Failed to get pf and vf id. (devid=%u)\n", devid);
        return ret;
#endif
    }
    for (tsid = 0; tsid < tsnum; tsid++) {
        pf_mbox_hwinfo = tsdrv_get_ts_mbox_hwinfo(phy_devid, tsid, TSDRV_TS_MBOX);
        ret = tsdrv_set_ts_mbox_hwinfo(devid, tsid, pf_mbox_hwinfo);
        if (ret != 0) {
            return ret;
        }
    }

    return 0;
}

static int tsdrv_set_vdev_db_info(u32 devid, u32 tsnum)
{
    struct tsdrv_db_hwinfo *pf_db_hwinfo = NULL;
    struct tsdrv_db_info db_info;
    u32 phy_devid, vfid, tsid;
    int ret;

    ret = devdrv_get_pfvf_id_by_devid(devid, &phy_devid, &vfid);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Failed to get pf and vf id. (devid=%u)\n", devid);
        return ret;
#endif
    }
    for (tsid = 0; tsid < tsnum; tsid++) {
        pf_db_hwinfo = tsdrv_get_db_hwinfo(phy_devid, tsid);
        db_info.db_paddr = pf_db_hwinfo->ts_db_paddr;
        db_info.db_size = pf_db_hwinfo->ts_db_size;
        db_info.dbs = tsdrv_get_db_base_addr(phy_devid, tsid);
        db_info.num = TSDRV_DB_NUM;
        db_info.db_stride = DEVDRV_TS_DOORBELL_STRIDE;
        TSDRV_PRINT_DEBUG("TS doorbell. (devid=%u; tsid=%u; dbs=0x%pK; num=%u; db_stride=0x%lx)\n",
            devid, tsid, db_info.dbs, db_info.num, db_info.db_stride);
        ret = tsdrv_set_ts_db_info(devid, tsid, &db_info);
        if (ret != 0) {
            return -ENODEV;
        }
        db_info.num = TS_DB_NUM_EACH_VF;
        tsdrv_set_mbox_db_info(devid, tsid, &db_info);
        db_info.db_paddr = pf_db_hwinfo->db_paddr;
        db_info.db_size = pf_db_hwinfo->db_size;
        db_info.dbs = pf_db_hwinfo->db_hwinfo_t[TSDRV_SQ_DB].dbs;
        TSDRV_PRINT_DEBUG("Stars doorbell. (devid=%u; tsid=%u; dbs=0x%pK; num=%u; db_stride=0x%lx)\n",
            devid, tsid, db_info.dbs, db_info.num, db_info.db_stride);
        ret = tsdrv_set_stars_db_info(devid, tsid, &db_info);
        if (ret != 0) {
            return -ENODEV;
        }
    }

    return 0;
}

static int tsdrv_set_vdev_sq_hwinfo(u32 devid, u32 tsnum)
{
    struct tsdrv_sq_hwinfo *pf_sq_hwinfo = NULL;
    u32 phy_devid, vfid, tsid;
    int ret;

    ret = devdrv_get_pfvf_id_by_devid(devid, &phy_devid, &vfid);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Failed to get pf and vf id. (devid=%u)\n", devid);
        return ret;
#endif
    }
    for (tsid = 0; tsid < tsnum; tsid++) {
        pf_sq_hwinfo = tsdrv_get_ts_sq_hwinfo(phy_devid, tsid);
        ret = tsdrv_set_ts_sq_hwinfo(devid, tsid, pf_sq_hwinfo);
        if (ret != 0) {
            return ret;
        }
    }

    return 0;
}

static int tsdrv_set_vdev_cq_hwinfo(u32 devid, u32 tsnum)
{
    struct tsdrv_cq_hwinfo *pf_cq_hwinfo = NULL;
    u32 phy_devid, vfid, tsid;
    int ret;

    ret = devdrv_get_pfvf_id_by_devid(devid, &phy_devid, &vfid);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Failed to get pf and vf id. (devid=%u)\n", devid);
        return ret;
#endif
    }
    for (tsid = 0; tsid < tsnum; tsid++) {
        pf_cq_hwinfo = tsdrv_get_cq_hwinfo(phy_devid, tsid);
        ret = tsdrv_set_ts_cq_hwinfo(devid, tsid, pf_cq_hwinfo);
        if (ret != 0) {
            return ret;
        }
    }

    return 0;
}

int tsdrv_set_vdev_hwinfo(u32 devid, u32 tsnum)
{
    int ret;

    ret = tsdrv_set_vdev_stars_sqcq_intr_hwinfo(devid, tsnum);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Failed to set stars sqcq intr_hwinfo. (devid=%u)\n", devid);
        return ret;
#endif
    }
    ret = tsdrv_set_vdev_tsfw_hwinfo(devid, tsnum);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Failed to set tsfw hwinfo. (devid=%u)\n", devid);
        return ret;
#endif
    }
    ret = tsdrv_set_vdev_mbox_hwinfo(devid, tsnum);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Failed to set mailbox hwinfo. (devid=%u)\n", devid);
        return ret;
#endif
    }
    ret = tsdrv_set_vdev_db_info(devid, tsnum);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Failed to set doorbell hwinfo. (devid=%u)\n", devid);
        return ret;
#endif
    }
    ret = tsdrv_set_vdev_sq_hwinfo(devid, tsnum);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Failed to set sq hwinfo. (devid=%u)\n", devid);
        return ret;
#endif
    }
    ret = tsdrv_set_vdev_cq_hwinfo(devid, tsnum);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Failed to set cq hwinfo. (devid=%u)\n", devid);
        return ret;
#endif
    }
    TSDRV_PRINT_INFO("set vdev hwinfo success. (devid=%u)\n", devid);

    return 0;
}
#endif

