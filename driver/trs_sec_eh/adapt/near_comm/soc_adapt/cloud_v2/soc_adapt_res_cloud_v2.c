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
* Create: 2022-10-08
*/
#include "devdrv_interface.h"

#include "trs_chan_mem.h"
#include "trs_chip_def_comm.h"
#include "soc_adapt_res_cloud_v2.h"

/* Trs notify */
#define TRS_CLOUD_V2_NOTIFY_SIZE             4
#define TRS_CLOUD_V2_NOTIFY_SLICE_SIZE (64 * 1024)
#define TRS_CLOUD_V2_NOTIFY_NUM_PER_SLICE 512   /* total silce num is 16 */
#define TRS_CLOUD_V2_NOTIFY_AVG_SIZE_PER_NOTIFY 128 /* total slice num * per slice size / total notify num */

/* Doorbell */
#define TRS_SOC_CLOUD_V2_DB_STRIDE   (4 * 1024)
#define TRS_CLOUD_V2_STARS_SCHED_STRIDE (4 * 1024)

#define TRS_DB_CLOUD_V2_ONLINE_MBOX_START   0u
#define TRS_DB_CLOUD_V2_ONLINE_MBOX_END     1u

#define TRS_DB_CLOUD_V2_TRIGGER_SQ_START    1u
#define TRS_DB_CLOUD_V2_TRIGGER_SQ_END      2u

#define TRS_DB_CLOUD_V2_MAINT_SQ_START      992u
#define TRS_DB_CLOUD_V2_MAINT_SQ_END        996u

#define TRS_DB_CLOUD_V2_MAINT_CQ_START      996u
#define TRS_DB_CLOUD_V2_MAINT_CQ_END        1006u

u32 trs_soc_get_cloud_v2_notify_offset(u32 id)
{
    return (id % TRS_CLOUD_V2_NOTIFY_NUM_PER_SLICE) * TRS_CLOUD_V2_NOTIFY_SIZE +
        (id / TRS_CLOUD_V2_NOTIFY_NUM_PER_SLICE) * TRS_CLOUD_V2_NOTIFY_SLICE_SIZE;
}

size_t trs_soc_get_cloud_v2_notify_size(void)
{
    return (size_t)TRS_CLOUD_V2_NOTIFY_AVG_SIZE_PER_NOTIFY;
}

size_t trs_soc_get_cloud_v2_db_stride(void)
{
    return (size_t)TRS_SOC_CLOUD_V2_DB_STRIDE;
}

size_t trs_soc_get_cloud_v2_stars_sched_stride(void)
{
    return (u32)TRS_CLOUD_V2_STARS_SCHED_STRIDE;
}

int trs_soc_get_cloud_v2_db_cfg(int db_type, u32 *start, u32 *end)
{
    switch (db_type) {
        case TRS_DB_ONLINE_MBOX:
            *start = TRS_DB_CLOUD_V2_ONLINE_MBOX_START;
            *end = TRS_DB_CLOUD_V2_ONLINE_MBOX_END;
            break;
        case TRS_DB_TRIGGER_SQ:
            *start = TRS_DB_CLOUD_V2_TRIGGER_SQ_START;
            *end = TRS_DB_CLOUD_V2_TRIGGER_SQ_END;
            break;
        case TRS_DB_MAINT_SQ:
            *start = TRS_DB_CLOUD_V2_MAINT_SQ_START;
            *end = TRS_DB_CLOUD_V2_MAINT_SQ_END;
            break;
        case TRS_DB_MAINT_CQ:
            *start = TRS_DB_CLOUD_V2_MAINT_CQ_START;
            *end = TRS_DB_CLOUD_V2_MAINT_CQ_END;
            break;
        default:
            trs_err("Unkonwn db_type. (db_type=%d)\n", db_type);
            return -ENODEV;
    }

    return 0;
}

u32 trs_soc_get_cloud_v2_sq_mem_side(u32 devid)
{
    if (devdrv_get_connect_protocol(devid) == CONNECT_PROTOCOL_HCCS) {
        return TRS_CHAN_HOST_PHY_MEM;
    }
    return TRS_CHAN_HOST_MEM;
}

u32 trs_soc_get_cloud_v2_cq_mem_side(u32 devid)
{
    if (devdrv_get_connect_protocol(devid) == CONNECT_PROTOCOL_HCCS) {
        return TRS_CHAN_HOST_PHY_MEM;
    }
    return TRS_CHAN_HOST_MEM;
}

