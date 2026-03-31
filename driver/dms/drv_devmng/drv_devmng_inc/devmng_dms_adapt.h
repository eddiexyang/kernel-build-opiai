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

#ifndef __DEVMNG_DMS_ADAPT_H
#define __DEVMNG_DMS_ADAPT_H

int devdrv_manager_init(void);
void devdrv_manager_exit(void);

int dms_event_subscribe_from_host(u32 devid, void *msg, u32 in_len, u32 *ack_len);
int dms_event_clean_from_host(u32 devid, void *msg, u32 in_len, u32 *ack_len);
int dms_event_mask_from_host(u32 devid, void *msg, u32 in_len, u32 *ack_len);
int dms_event_get_exception_from_device(void *msg, u32 *ack_len);
int dms_get_event_code(u32 devid, u32 fid, u32 *health_code,
    struct devdrv_error_code_para *code_para);
int dms_get_health_code(u32 phyid, u32 fid, u32 *health_code);
int dms_event_id_to_error_string(u32 devid, u32 event_id,
    char *event_str, u32 name_len);

#ifdef CFG_SOC_PLATFORM_MDC_V51
typedef enum {
    SUSPEND_FLAG,
    RESUME_FLAG,
    MAX_BIST_FLAG
} BIST_POWER_FLAG;

typedef struct dms_bist_mode {
    uint32_t seconds;
    BIST_POWER_FLAG bist_flag;
    uint32_t start_vector_id;
} BIST_MODE_STRU;

void dms_clear_bist_mode(void);
BIST_MODE_STRU *dms_get_bist_mode_addr(void);
int dms_get_bist_mode(struct dms_bist_mode *bist_mode);
void dms_clear_bist_rslt_magic(void);
#endif
#endif
