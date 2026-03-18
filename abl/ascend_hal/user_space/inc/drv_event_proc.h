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
 * Create: 2023-10-07
 */

#ifndef DRV_EVENT_PROC_H
#define DRV_EVENT_PROC_H

#include "ascend_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

struct drv_event_proc {
    drvError_t (*proc_func)(unsigned int devId, const void *msg, struct event_proc_result *rsp);
    unsigned proc_size;
    char *proc_name;
};

void drvRegistertEventProc(DRV_SUBEVENT_ID id, struct drv_event_proc *event_proc);

#ifdef __cplusplus
}
#endif

#endif
