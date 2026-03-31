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

#ifndef DEVDRV_MANAGER_PID_MAP_H
#define DEVDRV_MANAGER_PID_MAP_H

#include <linux/types.h>
#include <linux/list.h>
#include <linux/hashtable.h>

#include "devdrv_manager_comm.h"
#include "devdrv_user_common.h"
#include "devdrv_manager_common.h"

#ifdef CFG_SOC_PLATFORM_MDC_V51
#define VFID_NUM_MAX 1
#else
#define VFID_NUM_MAX 32  /* virtual function id max num */
#endif

#define DEVMNG_PID_INVALID (-1)
#define DEVMNG_PID_START_ONCE (-2)

enum tagAicpufwPlat {
    AICPUFW_ONLINE_PLAT = 0,
    AICPUFW_OFFLINE_PLAT,
    AICPUFW_MAX_PLAT,
};

/* side: host 1 device 0 */
#define DEVICE_SIDE 0
#define HOST_SIDE 1
#define DEVMNG_USER_PROC_MAX 256

#define HAL_BIND_ALL_DEVICE 0xffff

struct devdrv_process_user_info {
    u32 valid;
    u32 devid; /* local udevid */
    u32 vfid;
    pid_t pid;
};

#ifdef CFG_HOST_ENV
#define PID_MAP_DEVNUM 64
#else
#define PID_MAP_DEVNUM DEVDRV_MAX_NODE_NUM
#endif

struct devdrv_process_sign {
    pid_t hostpid;
    u32 host_process_status;
    struct devdrv_process_user_info user_proc_host[DEVMNG_USER_PROC_MAX];
    struct devdrv_process_user_info user_proc_device[DEVMNG_USER_PROC_MAX];
    pid_t devpid[PID_MAP_DEVNUM][VFID_NUM_MAX][DEVDRV_PROCESS_CPTYPE_MAX];
    char sign[PROCESS_SIGN_LENGTH];
    u32 cp_count; /* count cp1 & dev_only devpid num for releasing sign */
    u32 sync_proc_cnt;
    u32 docker_id;
    u32 in_use_count;
    struct list_head list;
#ifndef DEVMNG_UT
    struct hlist_node link; /* hash find link */
#endif
};

int devdrv_bind_hostpid(struct devdrv_ioctl_para_bind_host_pid para_info);
int devdrv_unbind_hostpid(struct devdrv_ioctl_para_bind_host_pid para_info);
void devdrv_manager_process_sign_release(pid_t devpid);
void devdrv_manager_free_hashtable(void);
int devdrv_fop_query_host_pid(struct file *filep, unsigned int cmd, unsigned long arg);
int devdrv_notice_process_exit(u32 host_pid);
#ifdef CFG_HOST_ENV
int devdrv_pid_map_sync_proc(void *msg, u32 *ack_len);
#else
int devdrv_pid_map_sync_proc(u32 devid, void *msg, u32 in_len, u32 *ack_len);
#endif


#endif
