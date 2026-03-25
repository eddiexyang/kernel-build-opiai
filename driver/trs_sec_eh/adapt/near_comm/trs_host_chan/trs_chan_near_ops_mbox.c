/*
* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
* Create: 2023-1-10
*/
#include "devdrv_interface.h"

#include "trs_pm_adapt.h"
#include "trs_chan_mbox.h"
#include "trs_chan_mem.h"
#include "trs_host_comm.h"
#include "soc_adapt.h"
#include "trs_chan_near_ops_rsv_mem.h"

void trs_chan_update_ssid(struct trs_id_inst *inst, struct trs_chan_info *chan_info)
{
    if ((chan_info->types.type == CHAN_TYPE_HW) && (chan_info->types.sub_type == CHAN_SUB_TYPE_HW_DVPP)) {
        int ssid;
        int ret = trs_host_get_ssid(inst, &ssid);
        if (ret != 0) {
            trs_warn("Get ssid fail. (devid=%u; ret=%d)\n", inst->devid, ret);
        }
        chan_info->ssid = (u16)ssid;
    }
}

static int trs_chan_update_rsv_mem_addr(struct trs_id_inst *inst, struct trs_chan_info *chan_info)
{
    int ret;

    if (trs_soc_get_sq_mem_side(inst) == TRS_CHAN_DEV_MEM) {
        ret = trs_chan_near_sqcq_rsv_mem_h2d(inst, chan_info->sq_info.sq_phy_addr, &chan_info->sq_info.sq_phy_addr);
        if (ret != 0) {
            trs_err("Rsv mem h2d failed. (devid=%u; tsid=%u; ret=%d)\n", inst->devid, inst->tsid, ret);
            return ret;
        }
    }

    if (trs_soc_get_cq_mem_side(inst) == TRS_CHAN_DEV_MEM) {
        ret = trs_chan_near_sqcq_rsv_mem_h2d(inst, chan_info->cq_info.cq_phy_addr, &chan_info->cq_info.cq_phy_addr);
        if (ret != 0) {
            trs_err("Rsv mem h2d failed. (devid=%u; tsid=%u; ret=%d)\n", inst->devid, inst->tsid, ret);
            return ret;
        }
    }

    return 0;
}

static void trs_chan_update_phy_addr_check(u32 devid)
{
    if (devdrv_get_connect_protocol(devid) == CONNECT_PROTOCOL_HCCS) {
        u32 host_flag = 0;
        int ret;

        ret = devdrv_get_host_phy_mach_flag(devid, &host_flag);
        if (ret != 0) {
            trs_warn("Get phy mach flag failed. (devid=%u; ret=%d)\n", devid, ret);
            return;
        }

        /* not support passthrough virtual machine */
        if (host_flag != DEVDRV_HOST_PHY_MACH_FLAG) {
            trs_warn("Sqcq mem not support hccs. (devid=%u)\n", devid);
        }
    }
}

int trs_chan_near_ops_mbox_send(struct trs_id_inst *inst, struct trs_chan_info *chan_info)
{
    struct trs_chan_adapt_info adapt;
    int ret;

    adapt.app_type = TRS_MBOX_SEND_FROM_HOST;
    adapt.sq_side = (trs_soc_get_sq_mem_side(inst) == TRS_CHAN_DEV_MEM) ? 0 : 1;
    adapt.cq_side = (trs_soc_get_cq_mem_side(inst) == TRS_CHAN_DEV_MEM) ? 0 : 1;
    adapt.fid = 0;

    trs_chan_update_ssid(inst, chan_info);

    ret = trs_chan_update_rsv_mem_addr(inst, chan_info);
    if (ret != 0) {
        return ret;
    }

    trs_chan_update_phy_addr_check(inst->devid);

    return trs_chan_mbox_send(inst, chan_info, &adapt);
}
