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

#ifndef TSDRV_GET_SSID_H
#define TSDRV_GET_SSID_H

#include <linux/types.h>
#include "devdrv_common.h"

#define TSDRV_INVALID_SSID      (-1)

s32 tsdrv_sync_ssid_proc(u32 devid, struct tsdrv_msg_info *msg, u32 *ack_len);
int tsdrv_msg_sync_ssid(u32 dev_id, u32 tsid, u32 hpid, u32 vfid);
#ifndef AOS_LLVM_BUILD
static inline int svm_get_pasid(pid_t vpid, int dev_id __maybe_unused)
{
	return (int)vpid;
}
#endif
#endif
