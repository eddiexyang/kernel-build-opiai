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
 * Create: 20122-08-13
 */

#ifndef __DP_PROC_MNG_CMD_H__
#define __DP_PROC_MNG_CMD_H__

#include "ascend_hal_define.h"
#include "drv_type.h"

#ifdef EMU_ST
#define STATIC
#define INLINE
#else
#define STATIC static
#define INLINE inline
#endif

#define DAVINCI_DP_PROC_MNG_SUB_MODULE_NAME "DP_PROC_MNG"
#define DAVINCI_DP_PROC_MNG_AGENT_SUB_MODULE_NAME "DP_PROC_MNG_AGENT"

#define DP_PROC_MNG_MAX_DEVICE_NUM 64
#define DP_PROC_MNG_DEVICE_SIDE_AGENT_NUM DP_PROC_MNG_MAX_DEVICE_NUM
#define DP_PROC_MNG_HOST_SIDE_AGENT_NUM 1
#define DP_PROC_MNG_MAX_AGENT_NUM (DP_PROC_MNG_DEVICE_SIDE_AGENT_NUM + DP_PROC_MNG_HOST_SIDE_AGENT_NUM)
#define DP_PROC_MNG_MAX_VF_NUM 32

#define DP_PROC_MNG_INVALID_DEVICE_PHYID 0xff


#define DP_PROC_MNG_HOST_AGENT_ID DP_PROC_MNG_DEVICE_SIDE_AGENT_NUM
#define DP_PROC_MNG_MAX_AGENT_DEVICE_NUM 4

#define PROCESS_SIGN_LENGTH  49
#define PROCESS_RESV_LENGTH  4

/*
 * user mode: devid means logic_id
 * kernel mode: devid means phyid
 */
struct dp_proc_mng_devid {
    uint32_t logical_devid;
    uint32_t devid;
    uint32_t vfid;
};

struct dp_proc_mng_query_process_status_para {
    processType_t process_type;
    processStatus_t pid_status;
};

struct dp_proc_mng_bind_cgroup_para {
    BIND_CGROUP_TYPE bind_type;
};

struct dp_proc_mng_get_process_sign {
    pid_t tgid;
    char sign[PROCESS_SIGN_LENGTH];
    char resv[PROCESS_RESV_LENGTH];
};

struct dp_proc_mng_bind_host_pid {
    pid_t host_pid;
    unsigned int vfid;
    unsigned int chip_id;
    int mode;
    int cp_type;
    unsigned int len;
    char sign[PROCESS_SIGN_LENGTH];
};

struct dp_proc_mng_query_pid {
    pid_t pid;
    pid_t host_pid;
    unsigned int cp_type;
};

struct dp_proc_mng_ioctl_arg {
    struct dp_proc_mng_devid head; // 用户态无需感知
    union {
        struct dp_proc_mng_query_process_status_para query_process_status_para;
        struct dp_proc_mng_get_process_sign get_process_sign_para;
        struct dp_proc_mng_bind_host_pid bind_host_pid_para;
        struct dp_proc_mng_query_pid query_pid_para;
        struct dp_proc_mng_bind_cgroup_para bind_cgroup_para;
    } data;
};

struct dp_proc_mng_process_id {
    int32_t hostpid;
    uint32_t devid;
    uint32_t vm_id; // 虚拟机
    uint32_t vfid; // 算力分组容器
};

#define DP_PROC_MNG_PROCESS_STATUS_QUERY   _IOW(DP_PROC_MNG_MAGIC, 1, struct dp_proc_mng_ioctl_arg)
#define DP_PROC_MNG_GET_PROCESS_SIGN       _IOW(DP_PROC_MNG_MAGIC, 2, struct dp_proc_mng_ioctl_arg)
#define DP_PROC_MNG_BIND_PID_ID            _IOW(DP_PROC_MNG_MAGIC, 3, struct dp_proc_mng_ioctl_arg)
#define DP_PROC_MNG_UNBIND_PID_ID          _IOW(DP_PROC_MNG_MAGIC, 4, struct dp_proc_mng_ioctl_arg)
#define DP_PROC_MNG_QUERY_PID_ID           _IOW(DP_PROC_MNG_MAGIC, 5, struct dp_proc_mng_ioctl_arg)
#define DP_PROC_MNG_BIND_CGROUP            _IOW(DP_PROC_MNG_MAGIC, 6, struct dp_proc_mng_ioctl_arg)
#define DP_PROC_MNG_CMD_MAX_CMD            7

#define DP_PROC_MNG_MAGIC                  'G'

#endif
