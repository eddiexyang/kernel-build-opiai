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
#include "trs_chan_mem.h"
#include "trs_chip_def_comm.h"
#include "soc_adapt_res_mini_v2.h"

/* Notify */
#define TRS_MINI_V2_NOTIFY_SIZE             8

/* Doorbell */
#define TRS_SOC_MINI_V2_DB_STRIDE   (4 * 1024)

#define TRS_DB_MINI_V2_ONLINE_MBOX_START    1006u
#define TRS_DB_MINI_V2_ONLINE_MBOX_END      1007u

#define TRS_DB_MINI_V2_ONLINE_TASK_SQ_START    0u
#define TRS_DB_MINI_V2_ONLINE_TASK_SQ_END      512u

#define TRS_DB_MINI_V2_ONLINE_TASK_CQ_START    512u
#define TRS_DB_MINI_V2_ONLINE_TASK_CQ_END      864u

u32 trs_soc_get_mini_v2_notify_offset(u32 notify_id)
{
    return notify_id * TRS_MINI_V2_NOTIFY_SIZE;
}

size_t trs_soc_get_mini_v2_notify_size(void)
{
    return (size_t)TRS_MINI_V2_NOTIFY_SIZE;
}

size_t trs_soc_get_mini_v2_db_stride(void)
{
    return (size_t)TRS_SOC_MINI_V2_DB_STRIDE;
}

int trs_soc_get_mini_v2_db_cfg(int db_type, u32 *start, u32 *end)
{
    switch (db_type) {
        case TRS_DB_ONLINE_MBOX:
            *start = TRS_DB_MINI_V2_ONLINE_MBOX_START;
            *end = TRS_DB_MINI_V2_ONLINE_MBOX_END;
            break;
        case TRS_DB_TASK_SQ:
            *start = TRS_DB_MINI_V2_ONLINE_TASK_SQ_START;
            *end = TRS_DB_MINI_V2_ONLINE_TASK_SQ_END;
            break;
        case TRS_DB_TASK_CQ:
            *start = TRS_DB_MINI_V2_ONLINE_TASK_CQ_START;
            *end = TRS_DB_MINI_V2_ONLINE_TASK_CQ_END;
            break;
        default:
            trs_err("Unkonwn db_type. (db_type=%d)\n", db_type);
            return -ENODEV;
    }

    return 0;
}

