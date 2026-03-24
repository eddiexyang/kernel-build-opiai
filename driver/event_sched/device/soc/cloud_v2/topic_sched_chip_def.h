/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2023. All rights reserved.
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

#ifndef TOPIC_SCHED_CHIP_DEF_H
#define TOPIC_SCHED_CHIP_DEF_H

#include "devdrv_interface.h"
#include "ascend_hal_error.h"
#include "esched.h"

#define TOPIC_SCHED_HOST_RSV_MEM_OFFSET_ADDR 0x40000
#define TOPIC_SCHED_HOST_RSV_MEM_VF_OFFSET   0x10000


/* pool id width is 2bit£¬pool 0~5 are used for AI CPU mailbox£¬pool 7 is used for special CPU mailbox
   vfg(pool) id 0~5 are corresponding to pool id 0~5 */
#define TOPIC_SCHED_POOL_NUM 8
#define TOPIC_SCHED_OTHER_CPU_POOL_ID 7
#define TOPIC_SCHED_ACPU_POOL_ID 0
#define TOPIC_SCHED_CCPU_SLOT_POOL_ID 7
#define TOPIC_SCHED_ACPU_POOL_CCPU_SLOT     0xFFFF
#define TOPIC_SCHED_NON_ACPU_POOL_ACPU_SLOT 0x0
#define TOPIC_SCHED_NON_ACPU_POOL_CCPU_SLOT 0xFFFF0000
#define TOPIC_SCHED_VFG_NUM 8

#define TOPIC_SCHED_VFG_CCPU_SLOT_NUM   2
#define TOPIC_SCHED_VFG_CCPU_SLOT_MASK  0x3

#define TOPIC_SCHED_HOST_ACPU_MASK               0xFFFFFFFFFFFFFFFFULL
#define TOPIC_SCHED_HOST_CCPU_MASK               0xFFFF
#define TOPIC_SCHED_HOST_CCPU_WAIT_MB_ID_START   64

#define TOPIC_SCHED_RTSQ_NUM_PER_BIT  4
#define TOPIC_SCHED_RESV_RTSQ_NUM     (16 * TOPIC_SCHED_RTSQ_NUM_PER_BIT)   /* 16 rtsq pools */

static inline int topic_sched_get_rsv_mem(u32 chip_id, u64 *addr, size_t *size)
{
    int ret;

    ret = agentdrv_get_addr_info(chip_id, DEVDRV_ADDR_STARS_TOPIC_SCHED_RES_MEM_BASE, 0, addr, size);
    if (ret != 0) {
        sched_err("Failed to get reserved memory. (chip_id=%u; type=%d; ret=%d)\n",
            chip_id, (int)DEVDRV_ADDR_STARS_TOPIC_SCHED_RES_MEM_BASE, ret);
        return DRV_ERROR_BAD_ADDRESS;
    }

    sched_info("Get reserved memory successfully. (size=%u)\n", (u32)*size);
    return 0;
}

#endif
