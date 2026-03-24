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
#include "soc_res.h"
#include "devdrv_manager_comm.h"
#include "id_pool.h"
#include "trs_chip_def_comm.h"
#include "trs_device_agent.h"
#include "trs_pub_def.h"
#include "trs_device_comm.h"
#include "trs_device_id.h"
#include "trs_pm_adapt.h"
#include "trs_cache_id_recycle.h"

static int trs_pm_agent_get_id(u32 devid, u32 tsid, struct trs_msg_id_sync *data)
{
    struct id_pool_inst pool_inst;
    struct trs_id_inst inst;
    u32 req_num = data->head.req_num;
    u32 alloc_num = 0;
    int ret;

    if ((req_num == 0) || (req_num > TRS_MSG_ID_SYNC_MAX_NUM)) {
        trs_err("Invalid req num. (req_num=%u)\n", req_num);
        return -EINVAL;
    }

    id_pool_inst_pack(&pool_inst, devid, tsid);
    trs_id_inst_pack(&inst, devid, tsid);
    do {
        u32 alloc_id;
        ret = id_pool_alloc(&pool_inst, data->head.type, &alloc_id);
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

static int trs_pm_agent_put_id(u32 devid, u32 tsid, struct trs_msg_id_sync *data)
{
    struct id_pool_inst pool_inst;
    struct trs_id_inst inst;
    u32 req_num = data->head.req_num;
    u32 free_num = 0;

    if ((req_num == 0) || (req_num > TRS_MSG_ID_SYNC_MAX_NUM)) {
        trs_err("Invalid req num. (devid=%u; req_num=%u)\n", devid, req_num);
        return -EINVAL;
    }

    id_pool_inst_pack(&pool_inst, devid, tsid);
    trs_id_inst_pack(&inst, devid, tsid);
    do {
        int ret = trs_id_recycle_clear(&inst, data->head.type, data->id[free_num]);
        if (ret != 0) {
            trs_err("Invalid id. (devid=%u; type=%u; id=%u)\n", devid, data->head.type, data->id[free_num]);
            break;
        }

        id_pool_free(&pool_inst, data->head.type, data->id[free_num]);
        free_num++;
    } while ((--req_num) != 0);

    trs_debug("Put id info. (devid=%u; type=%u; num=%u)\n", devid, data->head.type, free_num);

    data->head.ret_num = free_num;
    return 0;
}

static int trs_pm_agent_get_cap(u32 devid, u32 tsid, struct trs_msg_id_cap *id_cap)
{
    u32 id_start, id_end, total_num, split;
    struct trs_id_inst inst;
    int ret;

    trs_id_inst_pack(&inst, devid, tsid);
    ret = trs_device_get_id_range(&inst, id_cap->type, &id_start, &id_end);
    ret |= trs_device_get_id_total_num(&inst, id_cap->type, &total_num);
    ret |= trs_device_get_id_split(&inst, id_cap->type, &split);
    if (ret != 0) {
        return ret;
    }
    id_cap->id_start = id_start;
    id_cap->id_end = id_end;
    id_cap->total_num = total_num;
    id_cap->split = split;

    trs_debug("Get id cap. (devid=%u; tsid=%u; type=%d)\n", devid, tsid, id_cap->type);

    return 0;
}

int trs_pm_agent_get_phy_addr(u32 devid, u32 tsid, struct trs_msg_get_phy_addr *info)
{
    struct soc_rsv_mem_info rsv_mem;
    struct res_inst_info res_inst;
    int ret;

    info->name[SYNC_MAX_NAME_LEN - 1] = '\0';
    soc_resmng_inst_pack(&res_inst, devid, TS_SUBSYS, tsid);
    ret = soc_resmng_get_rsv_mem(&res_inst, info->name, &rsv_mem);
    if (ret != 0) {
        trs_info("Rsv mem is not set. (devid=%u; tsid=%u; type=%s)\n", devid, tsid, info->name);
        return ret;
    }

    info->addr = (u64)rsv_mem.rsv_mem;
    info->size = (u32)rsv_mem.rsv_mem_size;

    return 0;
}
EXPORT_SYMBOL(trs_pm_agent_get_phy_addr);

static int trs_pm_agent_get_cq_group(u32 devid, u32 tsid, struct trs_msg_cq_group *info)
{
    struct trs_id_inst inst;

    trs_id_inst_pack(&inst, devid, tsid);
    return trs_get_host_irq_group(&inst, info->group, MAX_GROUP_NUM, &info->group_num);
}

static int trs_pm_agent_get_proc_num(u32 devid, u32 tsid, struct trs_msg_proc_num *data)
{
    struct trs_id_inst inst;

    trs_id_inst_pack(&inst, devid, tsid);
    return trs_core_ops_get_support_proc_num(&inst, &data->proc_num);
}

static int trs_pm_agent_get_res_avail_num(u32 devid, u32 tsid, struct trs_msg_res_num *data)
{
    struct id_pool_inst pool_inst;

    id_pool_inst_pack(&pool_inst, devid, tsid);
    return id_pool_get_avail_num(&pool_inst, data->type, &data->avail_num);
}

static struct trs_device_agent_ops pm_agent_ops = {
    .trs_agent_get_id = trs_pm_agent_get_id,
    .trs_agent_put_id = trs_pm_agent_put_id,
    .trs_agent_get_cap = trs_pm_agent_get_cap,
    .trs_agent_get_phy_addr = trs_pm_agent_get_phy_addr,
    .trs_agent_get_cq_group = trs_pm_agent_get_cq_group,
    .trs_agent_get_proc_num = trs_pm_agent_get_proc_num,
    .trs_agent_get_res_avail_num = trs_pm_agent_get_res_avail_num,
    .trs_agent_send_ssid_to_ts = NULL /* Pm do not need to send ssid */
};

void trs_pm_agent_init(void)
{
    int i;

    for (i = 0; i < TRS_DEVICE_MAX_PHY_DEV; i++) {
        trs_agent_register_ops(i, &pm_agent_ops);
    }

    trs_set_feature_mode();
}
