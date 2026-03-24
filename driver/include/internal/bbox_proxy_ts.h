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
 * Create: 2022-08-13
 */

#ifndef BBOX_PROXY_TS_H
#define BBOX_PROXY_TS_H

#include <linux/semaphore.h>
#include "device/bbox_pub.h"
#include "device/bbox_types.h"
#include "bbox_inner.h"

#define TS_START_FAIL_EXCEPTION         0xA807FFFF
#define TS_HEART_BEAT_LOST_EXCEPTION    0xA607FFFF

#define TS_BOOT_ADDR_PHY_ADDR           0x9fe010
#define TS_BOOT_SIZE_PHY_ADDR           0x9fe170

#define TS_DUMP_WAIT_TIMEOUT            10000 // ms
#define TS_WAIT_CYCLE                   100   // ms
#define TS_WAIT_FOR_DUMP_CYCLE          1000  // ms
#define TS_STARTLOG_MEM_SIZE            (50 * 1024) // 50k

// ts virtual add
typedef struct _ts_vaddr_ {
    void *start_except;     // ts startup exception add
    void *run_except;       // ts runtime exception add
} ts_vaddr_t;

typedef struct ts_except {
    u32 devid;
    u32 excepid;
    bbox_dump_done_ops pf_dumpdone;
    excep_time time;
    struct semaphore sem;
    u8 etype;
} ts_except_t;

typedef struct ts_physical_addr_t {
    u64 addr;
    u32 len;
} ts_physical_addr;

enum MNTN_BUFFER_TYPE {
    TS_MNTN_BUFFER = 0x0,       // ts buffer, static reserved, size: 512k
    TS_MNTN_START_LOG_BUFFER    // ts start log buffer, dynamic mallocced, size: 50k
};

#define MNTN_BUFFER_TYPE_MAX 0x2

typedef struct bbox_config_t {
    u8 enable_bbox;             // (0x1 enable for liteos vmcore);
    ts_physical_addr  ts_paddr[MNTN_BUFFER_TYPE_MAX];
} bbox_tsconfig;

bbox_tsconfig *bbox_get_tsconfig(u32 devid);
s32 bbox_ts_init(void);
void bbox_ts_exit(void);

#endif
