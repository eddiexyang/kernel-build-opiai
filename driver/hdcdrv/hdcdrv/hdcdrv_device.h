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

#ifndef _HDCDRV_DEVICE_H_
#define _HDCDRV_DEVICE_H_
typedef int (*check_hostpid)(int hostpid, u32 devid, u32 vfid);
typedef int (*query_fid)(int cp_pid, u32 *chip_id, u32 *fid, u32 *hostpid, int *process_type);
typedef int (*query_localpid)(u32 hostpid, u32 chip_id, int process_type, u32 vfid, int *pid);
typedef int (*query_master_pid_by_host_slave)(int slave_pid, u32 *master_pid);
#endif
