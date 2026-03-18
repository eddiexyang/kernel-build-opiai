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

#ifndef RDR_CORE_H
#define RDR_CORE_H

#include <linux/list.h>
#include "device/bbox_types.h"
#include "device/bbox_pub.h"
#include "common/bbox_common.h"

struct rdr_syserr_param_s {
    struct list_head syserr_list;
    struct bbox_time tm;
    u32 excepid;
    u32 devid;
    u32 arg;
};


#define RDR_SYSERR_SET_PARAM(_p, _excepid, _devid, _arg, _sec, _nsec) do { \
    (_p).excepid = (_excepid);                                             \
    (_p).devid = (_devid);                                                 \
    (_p).arg = (_arg);                                                     \
    (_p).tm.tv_sec = (_sec);                                             \
    (_p).tm.tv_nsec = (_nsec);                                           \
} while (0)


bool rdr_get_os_status(void);
void rdr_syserr_process_for_os(const struct bbox_exception_info *einfo, const struct bbox_time *tm);
bool rdr_syserr_list_empty(void);
bool rdr_init_done(void);
s32 rdr_exception_report(u32 devid, u32 excepid, const struct excep_time *tm, u32 arg);

s32 bbox_rdr_init(void);
void bbox_rdr_exit(void);

#endif

