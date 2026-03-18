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
#ifndef __DP_PROC_MNG_PID_MAP_H__
#define __DP_PROC_MNG_PID_MAP_H__

#ifdef CFG_SOC_PLATFORM_MDC_V51
#define VFID_NUM_MAX 1
#else
#define VFID_NUM_MAX 32  /* virtual function id max num */
#endif

enum tagAicpufwPlat {
    AICPUFW_ONLINE_PLAT = 0,
    AICPUFW_OFFLINE_PLAT,
    AICPUFW_MAX_PLAT,
};

#define DP_PROC_MNG_PID_INVALID (-1)
#define DP_PROC_MNG_PID_START_ONCE (-2)

int dp_proc_mng_check_process_sign(pid_t hostpid, const char *sign, u32 len);

int dp_proc_mng_bind_host_pid(struct dp_proc_mng_bind_host_pid *para_info);

#endif
