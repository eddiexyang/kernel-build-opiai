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

#define TOPIC_SCHED_HOST_RSV_MEM_OFFSET_ADDR 0x50000

/* pool id width is 1bit£¬pool 0 is used for AI CPU mailbox£¬pool 2 is used for special CPU mailbox£¬
    pool 1 is reserved for determinative scheduling      vfg(pool) id is always 0 */
#define TOPIC_SCHED_RSV_MEM_SIZE (32 * 1024)

#ifdef CFG_SOC_PLATFORM_MDC_V11
#define TOPIC_SCHED_RSV_MEM_BASE_ADDR 0xB900000ULL
#else
#define TOPIC_SCHED_RSV_MEM_BASE_ADDR 0x22000000ULL
#endif

#define TOPIC_SCHED_POOL_NUM 3
#define TOPIC_SCHED_OTHER_CPU_POOL_ID 2
#define TOPIC_SCHED_ACPU_POOL_ID 0
#define TOPIC_SCHED_CCPU_SLOT_POOL_ID 1
#define TOPIC_SCHED_ACPU_POOL_CCPU_SLOT     0xF
/* The topic type CCPU_HOST needs acpu slot on mini_v3, config remaining acpu slot to non acpu pool. */
#define TOPIC_SCHED_NON_ACPU_POOL_ACPU_SLOT 0xF0
#define TOPIC_SCHED_NON_ACPU_POOL_CCPU_SLOT 0xF0
#define TOPIC_SCHED_VFG_NUM 1

#define TOPIC_SCHED_VFG_CCPU_SLOT_NUM   2
#define TOPIC_SCHED_VFG_CCPU_SLOT_MASK  0x3

#define TOPIC_SCHED_HOST_ACPU_MASK               0x0
#define TOPIC_SCHED_HOST_CCPU_MASK               0x3
#define TOPIC_SCHED_HOST_CCPU_WAIT_MB_ID_START   0

#ifdef CFG_SOC_PLATFORM_MDC_V11
#define TOPIC_SCHED_RTSQ_NUM_PER_BIT  1
#else
#define TOPIC_SCHED_RTSQ_NUM_PER_BIT  3
#endif

#define TOPIC_SCHED_RESV_RTSQ_NUM     (16 * TOPIC_SCHED_RTSQ_NUM_PER_BIT)   /* 16 rtsq pools */

static inline int topic_sched_get_rsv_mem(u32 chip_id, u64 *addr, size_t *size)
{
    *addr = TOPIC_SCHED_RSV_MEM_BASE_ADDR + (chip_id * TOPIC_SCHED_RSV_MEM_SIZE);
    *size = TOPIC_SCHED_RSV_MEM_SIZE;
    return 0;
}

#endif
