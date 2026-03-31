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

#ifndef ESCHED_DEVICE_MSG_H
#define ESCHED_DEVICE_MSG_H

#include "esched_msg_def.h"

#define ESCHED_DRV_HOST_IRQ_MAX (0x1 << 11)  /* 11: bit0 ~ bit10 denote the intr_id */

struct sched_msg_ops {
    int valid;
    int (*add_host_pid)(u32 dev_id, struct esched_ctrl_msg_cfg_host_pid *host_pid_msg);
    int (*del_host_pid)(u32 dev_id, struct esched_ctrl_msg_cfg_host_pid *host_pid_msg);
    int (*add_host_pool)(u32 dev_id, struct esched_ctrl_msg_cfg_pool *pool_msg);
    int (*get_host_cpu_mbid)(u32 dev_id, struct esched_ctrl_msg_get_cpu_mbid *mbid_msg);
    int (*add_host_mb)(u32 dev_id, struct esched_ctrl_msg_cfg_mb *mb_msg);
    int (*conf_host_intr)(u32 dev_id, struct esched_ctrl_msg_intr *intr_msg);
};

void esched_setup_sia_msg_ops(u32 dev_id);
void esched_setup_mia_msg_ops(u32 dev_id);
void esched_drv_register_msg_ops(u32 dev_id, struct sched_msg_ops *ops);

#endif
