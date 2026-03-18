/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
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
 * Description: DMS qos related interface
 * Author: huawei
 * Create: 2021-12-15
 */

#ifndef __DMS_LPM_INTERFACE_H
#define __DMS_LPM_INTERFACE_H

#include "ascend_hal_error.h"

#define DMS_LPM_SOC_ID 8
#define DMS_VF_ID_PF 0
#define DMS_IPC_TIMEOUT 110

#define DMS_LPM_GET_TEMPERATURE 0
#define DMS_LPM_GET_POWER 1
#define DMS_LPM_GET_VOLTAGE 2
#define DMS_LPM_GET_FREQUENCY 3
#define DMS_LPM_GET_TSENSOR 4
#define DMS_LPM_GET_MAX_FREQUENCY 5
#define DMS_SUBCMD_GET_LP_STATUS 7
#define DMS_LPM_SUB_CMD_MAX 8

#define DMS_SUBCMD_LP_SUSPEND       0

struct dms_lpm_info_in {
    unsigned int dev_id;
    unsigned int vfid;
    unsigned int core_id;
    unsigned int sub_cmd;
};

#define POWER_INFO_RESERVE_LEN 8
struct dms_power_state_info {
    unsigned int type;
    unsigned int mode;
    unsigned int value;
    unsigned int reserve[POWER_INFO_RESERVE_LEN];
};

struct dms_lp_state_in {
    unsigned int dev_id;
    struct dms_power_state_info power_info;
};

drvError_t DmsGetLpmInfo(struct dms_lpm_info_in *in, void *result, unsigned int result_size);
drvError_t DmsSetLpState(unsigned int dev_id, unsigned int sub_cmd, struct dms_power_state_info *power_info);
int DmsGetLowPowerInfo(unsigned int dev_id, unsigned int vfid, unsigned int sub_cmd, void *buf,
    unsigned int *size);
int DmsSetLowPowerInfo(unsigned int dev_id, unsigned int sub_cmd, void *buf, unsigned int size);
int DmsGetTempFromLp(unsigned int dev_id, unsigned int vfid, unsigned int sub_cmd, void *buf, unsigned int *size);
int DmsLpmPassThroughMcu(unsigned char rw_flag, const unsigned char *buf, unsigned char buf_len,
    unsigned char *resp_buff, unsigned char *recv_len);
int DmsGetInfoFromLp(unsigned int dev_id, unsigned int vfid, unsigned int sub_cmd, void *out_buf,
    unsigned int *buf_size);
int DmsGetTemperature(unsigned int dev_id, unsigned int vfid, unsigned int sub_cmd, void *out_buf,
    unsigned int *buf_size);
int DmsSetTemperature(unsigned int dev_id, unsigned int sub_cmd, void *out_buf, unsigned int buf_size);
#endif
