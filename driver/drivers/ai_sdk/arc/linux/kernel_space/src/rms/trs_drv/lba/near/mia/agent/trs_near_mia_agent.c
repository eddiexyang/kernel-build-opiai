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
#include <linux/init.h>
#include <linux/module.h>

#include "devdrv_interface.h"
#include "trs_pub_def.h"
#include "trs_chip_def_comm.h"
#include "trs_device_agent.h"
#include "trs_mia_adapt.h"
#include "trs_mia_agent.h"
#include "trs_pm_agent.h"
#include "trs_mailbox_def.h"
#include "trs_mia_device_id.h"
#include "trs_cache_id_recycle.h"

static int trs_mia_agent_get_id(u32 devid, u32 tsid, struct trs_msg_id_sync *data)
{
    struct trs_id_inst inst;
    u32 req_num = data->head.req_num;
    u32 alloc_num = 0;
    int ret;

    if ((req_num == 0) || (req_num > TRS_MSG_ID_SYNC_MAX_NUM)) {
        trs_err("Invalid req num. (req_num=%u)\n", req_num);
        return -EINVAL;
    }

    trs_id_inst_pack(&inst, devid, tsid);
    do {
        u32 alloc_id;
        ret = trs_mia_adapt_alloc_id(&inst, data->head.type, &alloc_id);
        if (ret != 0) {
            break;
        }

        data->id[alloc_num] = alloc_id;
        trs_id_recycle_set(&inst, data->head.type, data->id[alloc_num]);
        alloc_num++;
    } while ((--req_num) != 0);

    if (alloc_num == 0) {
        trs_info("Id exhausted. (devid=%u; tsid=%u; type=%s; used_num=%d; ret=%d)\n",
            devid, tsid, trs_id_type_to_name(data->head.type),
            trs_get_id_recycle_num(&inst, data->head.type), ret);
        return -ENOSPC;
    }

    data->head.ret_num = alloc_num;
    return 0;
}

static int trs_mia_agent_put_id(u32 devid, u32 tsid, struct trs_msg_id_sync *data)
{
    struct trs_id_inst inst;
    u32 req_num = data->head.req_num;
    u32 free_num = 0;

    if ((req_num == 0) || (req_num > TRS_MSG_ID_SYNC_MAX_NUM)) {
        trs_err("Invalid req num. (devid=%u; req_num=%u)\n", devid, req_num);
        return -EINVAL;
    }

    trs_id_inst_pack(&inst, devid, tsid);
    do {
        int ret = trs_id_recycle_clear(&inst, data->head.type, data->id[free_num]);
        if (ret != 0) {
            trs_err("Invalid id. (devid=%u; type=%u; id=%u)\n", devid, data->head.type, data->id[free_num]);
            break;
        }

        trs_mia_adapt_free_id(&inst, data->head.type, data->id[free_num]);
        free_num++;
    } while ((--req_num) != 0);

    data->head.ret_num = free_num;
    return 0;
}

static int trs_mia_agent_get_cap(u32 devid, u32 tsid, struct trs_msg_id_cap *id_cap)
{
    u32 id_start, id_end, total_num, split;
    struct trs_id_inst inst;
    int ret;

    trs_id_inst_pack(&inst, devid, tsid);
    ret = trs_mia_adapt_get_id_range(&inst, id_cap->type, &id_start, &id_end);
    ret |= trs_mia_adapt_get_id_total_num(&inst, id_cap->type, &total_num);
    ret |= trs_mia_adapt_get_id_split(&inst, id_cap->type, &split);
    if (ret != 0) {
        return ret;
    }
    id_cap->id_start = id_start;
    id_cap->id_end = id_end;
    id_cap->total_num = total_num;
    id_cap->split = split;

    return 0;
}

static int trs_mia_agent_get_phy_addr(u32 devid, u32 tsid, struct trs_msg_get_phy_addr *info)
{
    return trs_pm_agent_get_phy_addr(devid, tsid, info);
}

static int trs_mia_agent_get_cq_group(u32 devid, u32 tsid, struct trs_msg_cq_group *info)
{
    struct trs_id_inst inst;
    int ret;

    trs_id_inst_pack(&inst, devid, tsid);
    ret = trs_mia_adapt_get_cq_group(&inst, info->group, MAX_GROUP_NUM, &info->group_num);
    if (ret != 0) {
        trs_err("Get cq group failed. (devid=%u; tsid=%u)\n", devid, tsid);
    }
    return ret;
}

static int trs_mia_agent_send_ssid(u32 devid, u32 tsid, int ssid, u32 hpid)
{
    struct trs_ssid_msg mbox_data;
    struct trs_id_inst inst;
    u32 phy_devid, vfid;
    int ret;

    ret = devdrv_get_pfvf_id_by_devid(devid, &phy_devid, &vfid);
    if (ret != 0) {
        trs_err("Get pfvf id fail. (devid=%u; ret=%d)\n", devid, ret);
        return ret;
    }

    trs_mbox_init_header(&mbox_data.header, TRS_MBOX_NOTICE_SSID);
    mbox_data.hostpid = hpid;
    mbox_data.ssid = ssid;
    mbox_data.vfid = vfid;

    trs_id_inst_pack(&inst, phy_devid, tsid);
    ret = trs_mbox_send(&inst, 0, &mbox_data, sizeof(struct trs_ssid_msg), 3000); // timeout: 3000 ms
    if (ret != 0 || mbox_data.header.result != 0) {
        trs_err("Send mbox fail. (devid=%u; tsid=%u; result=%u; ret=%d)\n",
            phy_devid, tsid, mbox_data.header.result, ret);
        ret = -EFAULT;
    }
    return ret;
}

static int trs_mia_agent_get_proc_num(u32 devid, u32 tsid, struct trs_msg_proc_num *data)
{
    struct trs_id_inst inst;

    trs_id_inst_pack(&inst, devid, tsid);
    return trs_mia_core_ops_get_support_proc_num(&inst, &data->proc_num);
}

static int trs_mia_agent_instance(u32 devid, u32 tsid)
{
    struct trs_id_inst inst;

    trs_id_inst_pack(&inst, devid, tsid);
    return trs_id_recycle_create(&inst);
}

static void trs_mia_agent_uninstance(u32 devid, u32 tsid)
{
    struct trs_id_inst inst;
    int type;

    trs_id_inst_pack(&inst, devid, tsid);
    for (type = 0; type < TRS_ID_TYPE_MAX; type++) {
        trs_id_recycle_clear_all(&inst, type);
    }
    trs_id_recycle_destroy(&inst);
}

static struct trs_device_agent_ops mia_agent_ops = {
    .trs_agent_get_id = trs_mia_agent_get_id,
    .trs_agent_put_id = trs_mia_agent_put_id,
    .trs_agent_get_cap = trs_mia_agent_get_cap,
    .trs_agent_get_phy_addr = trs_mia_agent_get_phy_addr,
    .trs_agent_get_cq_group = trs_mia_agent_get_cq_group,
    .trs_agent_get_proc_num = trs_mia_agent_get_proc_num,
    .trs_agent_send_ssid_to_ts = trs_mia_agent_send_ssid,
    .trs_agent_instance = trs_mia_agent_instance,
    .trs_agent_uninstance = trs_mia_agent_uninstance,
};

int __init trs_mia_agent_init(void)
{
    int i;

    for (i = TRS_DEVICE_VIR_DEV_BASE; i < TRS_DEVICE_VIR_DEV_BASE + TRS_DEVICE_VIR_DEV_NUM; i++) {
        trs_agent_register_ops(i, &mia_agent_ops);
    }

    return 0;
}

void __exit trs_mia_agent_exit(void)
{
    int i;

    for (i = TRS_DEVICE_VIR_DEV_BASE; i < TRS_DEVICE_VIR_DEV_BASE + TRS_DEVICE_VIR_DEV_NUM; i++) {
        trs_agent_unregister_ops(i);
    }
}

module_init(trs_mia_agent_init);
module_exit(trs_mia_agent_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("trs near mia agent driver");

