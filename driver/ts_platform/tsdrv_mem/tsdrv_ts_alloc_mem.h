/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
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

#ifndef TSDRV_TS_ALLOC_MEM_H
#define TSDRV_TS_ALLOC_MEM_H

#include <linux/types.h>
#include "drv_ipc.h"

#define TS_MEM_ALLOC_REQ_CMD            0
#define TS_MEM_ALLOC_RESP_CMD           1
#define TS_MEM_OP_FLAG_CONTINUOUS_ADDR  1U
#define TS_MEM_OP_FLAG_WITHIN_4G        2U
#define TS_MEM_ALLOC_RESP_MSG_LEN       20
#define TS_MEM_MAX_SIZE                 0x40000000    // 1024 * 1024 * 1024
#define TS_MEM_MAX_CONTINUOUS_SIZE      0x3FF000
#define TS_MEM_ISOLATION_SIZE           0x800
#define TS_MEM_ISOLATION_MAGIC          0x5

/* the size of this struct must not excess 32B */
struct tsdrv_mem_alloc_req_msg {
    struct ipcdrv_msg_header header;
    u32 sub_cmd_type : 8;
    u32 sn : 8;
    u32 flag : 16;
    u32 size;
};

/* the size of this struct must not excess 32B */
struct tsdrv_mem_alloc_resp_msg {
    struct ipcdrv_msg_header header;
    u32 sub_cmd_type : 8;
    u32 sn : 8;
    u32 flag : 16;
    u64 vaddr;
    u64 paddr;
};

#ifdef CFG_FEATURE_TS_ALLOC_MEM
void tsdrv_ts_mem_mgmt_init(void);
void tsdrv_ts_mem_mgmt_exit(void);
u32 tsdrv_get_isolation_mem_num(u32 devid, u32 tsid);
u64 tsdrv_get_allocated_mem_size(u32 devid, u32 tsid);
#else
static void tsdrv_ts_mem_mgmt_init(void)
{
}

static void tsdrv_ts_mem_mgmt_exit(void)
{
}
#endif

#endif
