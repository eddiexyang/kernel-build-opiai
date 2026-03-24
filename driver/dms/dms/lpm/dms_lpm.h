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

#ifndef __LPM_LPM_H__
#define __LPM_LPM_H__

#include <linux/types.h>
#include <linux/time.h>

#include "ascend_hal_error.h"
#include "dms_define.h"
#include "drv_type.h"

#define DMS_LPM_CMD_NAME "DMS_LPM_CMD_NAME"

#ifdef CFG_SOC_PLATFORM_CLOUD
    #define LPM_DEVICE_NUM_MAX  4
#else

#ifdef CFG_SOC_PLATFORM_MINIV2
    #define LPM_DEVICE_NUM_MAX 2
#else
    #define LPM_DEVICE_NUM_MAX 1
#endif

#endif

#define LPM_SCAN_FAILURE_INTERVAL_TIME_MAX        60000

typedef int (*lpm_get_info_handle)(u32 dev_id, u32 core_id, u32 *value);

struct lpm_retry_cycle {
    unsigned int interval_cnt;
    unsigned int remain_cnt;
    struct timeval prev_fail_time;
};

struct lpm_scan_list {
    lpm_get_info_handle get_value_handle;
    u32 type_id;
    u32 core_id;
    u32 pf_value[LPM_DEVICE_NUM_MAX];
    struct lpm_retry_cycle period_ctrl[LPM_DEVICE_NUM_MAX];
    u32 scan_flag;
};

struct dms_lpm_info_in {
    unsigned int dev_id;
    unsigned int vfid;
    unsigned int core_id;
    unsigned int sub_cmd;
};

struct dms_ddr_temp_arg {
    unsigned int dev_id;
    unsigned int vfid;
    unsigned int core_id;
    void *commoninfo;
    unsigned int commoninfo_len;
};

int dms_lpm_init(void);
void dms_lpm_uninit(void);

int dms_lpm_get_temperature(void *feature, char *in, u32 in_len, char *out, u32 out_len);
int dms_lpm_get_power(void *feature, char *in, u32 in_len, char *out, u32 out_len);
int dms_lpm_get_voltage(void *feature, char *in, u32 in_len, char *out, u32 out_len);
int dms_lpm_get_frequency(void *feature, char *in, u32 in_len, char *out, u32 out_len);
int dms_lpm_get_ai_utilization(void *feature, char *in, u32 in_len, char *out, u32 out_len);
int dms_lpm_get_tsensor(void *feature, char *in, u32 in_len, char *out, u32 out_len);
int dms_lpm_get_max_frequency(void *feature, char *in, u32 in_len, char *out, u32 out_len);
int dms_lpm_get_lp_status(void *feature, char *in, u32 in_len, char *out, u32 out_len);
int dms_lpm_get_device_info(void *feature, char *in, u32 in_len, char *out, u32 out_len);
int dms_lpm_get_temp_info(void *feature, char *in, u32 in_len, char *out, u32 out_len);
#ifdef CFG_REATURE_GET_INFO_FORM_IPC
int dms_lpm_set_temp_info(void *feature, char *in, u32 in_len, char *out, u32 out_len);
#endif
struct lpm_scan_list *find_operate_info_str(u32 type_id, u32 core_id);
int lpm_scan_init(void);
int lpm_scan_uninit(void);
#endif
