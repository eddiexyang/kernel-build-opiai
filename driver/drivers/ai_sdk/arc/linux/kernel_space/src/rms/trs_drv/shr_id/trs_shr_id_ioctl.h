/*
* Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
* Create: 2022-7-29
*/

#ifndef TRS_SHR_ID_IOCTL_H
#define TRS_SHR_ID_IOCTL_H
#include <asm/ioctl.h>

#include "drv_type.h"

#define DAVINCI_INTF_MODULE_TRS_SHR_ID "TRS_SHR_ID"

#define SHR_ID_EVENT_MASK 0x8000
#define SHR_ID_NSM_NAME_SIZE    65
#define SHR_ID_PID_MAX_NUM  16
struct shr_id_ioctl_info {
    u32 opened_devid; /* out:logic devid */
    u32 devid;
    u32 tsid;
    u32 shr_id;
    u32 id_type;

    u64 dev_addr;
    u64 host_addr;
    char name[SHR_ID_NSM_NAME_SIZE];
    pid_t pid[SHR_ID_PID_MAX_NUM];
    u32 flag; /* in:remote flag, out:event flag */
};

#define SHR_ID_NOTIFY_MAGIC 'N'
#define SHR_ID_CREATE _IOWR(SHR_ID_NOTIFY_MAGIC, 1, struct shr_id_ioctl_info)
#define SHR_ID_OPEN _IOWR(SHR_ID_NOTIFY_MAGIC, 2, struct shr_id_ioctl_info)
#define SHR_ID_CLOSE _IOWR(SHR_ID_NOTIFY_MAGIC, 3, struct shr_id_ioctl_info)
#define SHR_ID_DESTROY _IOW(SHR_ID_NOTIFY_MAGIC, 4, struct shr_id_ioctl_info)
#define SHR_ID_SET_PID _IOW(SHR_ID_NOTIFY_MAGIC, 5, struct shr_id_ioctl_info)
#define SHR_ID_MAX_CMD   6

#endif

