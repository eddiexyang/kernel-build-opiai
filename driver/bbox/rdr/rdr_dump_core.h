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

#ifndef RDR_DUMP_CORE_H
#define RDR_DUMP_CORE_H

#include "rdr_core.h"
#include "device/bbox_types.h"
#include "device/bbox_pub.h"
#include "common/bbox_common.h"
#include "communication/bbox_message.h"

enum BBOX_EVENT_BBIT {
    EVENT_NONE          = 0,
    EVENT_OOM_TRIGGER   = 1 << 0,
    EVENT_CHANNEL_ERROR = 1 << 1,
    EVENT_MAX           = 0xFFFF
};

// log subsystem functions
// 1. save log to ddr, wait send
void rdr_save_history_log_for_undef_exception(const struct rdr_syserr_param_s *param);
void rdr_save_history_log_for_runtime(u32 devid, const struct bbox_exception_info *info, const struct bbox_time *tm);
void rdr_save_history_log_for_bootcheck(const struct rdr_exception_msg_info *info);
void rdr_save_history_log_for_oom(u32 devid, u8 coreid, u8 etype, u32 excepid, const struct bbox_time *tm);
void rdr_save_history_log_for_comm(u32 devid, const struct bbox_exception_info *info, const struct bbox_time *tm);
void rdr_save_history_log_for_panic(u32 devid, const struct bbox_exception_info *info, const struct bbox_time *tm);

// 2. send module log on ddr
s32 rdr_save_dump_for_rmodule(u32 devid, u32 excepid, u8 coreid, const struct bbox_time *tm);
s32 rdr_save_dump_for_reset(u32 devid, u32 excepid, u8 coreid, const struct bbox_time *tm);
void rdr_save_dump(u32 devid, u8 coreid, u32 excepid, u8 etype, const struct bbox_time *tm);

#endif

