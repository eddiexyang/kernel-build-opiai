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

#ifndef BBOX_PROXY_COMMON_H
#define BBOX_PROXY_COMMON_H

#include <linux/semaphore.h>
#include "device/bbox_pub.h"
#include "bbox_inner.h"


struct bbox_proxy_dump_info {
    u8 etype;
    u32 devid;
    u32 main_excepid;
    u32 sub_excepid;
    excep_time tm;
    bbox_dump_done_ops done;
};

struct bbox_proxy_module_mgr {
    struct bbox_proxy_info *info;
    struct task_struct *thread;
    char thread_name[THREAD_NAME_LEN];
    struct bbox_proxy_dump_info current_info;
    struct semaphore sem;
    u8 coreid;
    u8 initialized;
    u64 ddr_addr;
    u32 ddr_len;
};

struct bbox_proxy_mgr {
    s32 count;
    struct bbox_proxy_module_mgr mgr[BBOX_COREID_MAX];
};

s32 bbox_product_proxy_init(struct bbox_proxy_mgr *proxy);
void bbox_product_proxy_exit(void);
void bbox_proxy_module_dump(const struct bbox_dump_done_ops_info *info, bbox_dump_done_ops fndone);

s32 bbox_proxy_init(void);
void bbox_proxy_exit(void);

#endif