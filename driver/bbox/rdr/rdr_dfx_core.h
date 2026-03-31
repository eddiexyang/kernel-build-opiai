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

#ifndef RDR_DFX_CORE_H
#define RDR_DFX_CORE_H

#include <linux/list.h>
#include <linux/types.h>
#include "bbox_platform.h"
#include "device/bbox_pub.h"

#define DEVICE_ERROR_CODE_NUM_MAX 128
#define DEVICE_NORMAL       0
#define DEVICE_EXCEPTION    1

struct rdr_excepid_param_s {
    struct list_head excepid_list;
    u32 excepid;
    u32 count;
    u8 status[DEVICE_MAX_NUM];
    u8 desc[BBOX_EXCEPTIONDESC_MAXLEN];
};

s32 rdr_del_device_errorcode(u32 devid, u32 excepid);
s32 rdr_add_device_errorcode(u32 devid, u32 excepid, const struct bbox_exception_info *info);

s32 rdr_dfx_add_exception(u32 devid, u32 excepid, const struct excep_time *tm, u32 arg);
void rdr_dfx_del_exception(u32 excepid);

s32 rdr_dfx_init(void);
void rdr_dfx_exit(void);

#endif
