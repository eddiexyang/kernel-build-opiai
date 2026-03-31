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

#ifndef DEVDRV_MANAGER_CHECK_H
#define DEVDRV_MANAGER_CHECK_H

#include <linux/uaccess.h>

#include "devdrv_common.h"

#define DEVDRV_TS_BIN_MAX_SEGMENT_NUM 16
#define DEVDRV_TS_BIN_CHEKC_LEN 32

struct devdrv_check_sum {
    u8 check[DEVDRV_TS_BIN_CHEKC_LEN];
};

struct devdrv_ts_bin_segment {
    u32 offset;
    u32 len;
    struct devdrv_check_sum segment_check;
};

struct devdrv_ts_bin_info {
    u32 ts_check_file;
    u32 fw_data_len;
    struct devdrv_check_sum fw_data_check;
    u32 segment_num;
    struct devdrv_ts_bin_segment segment[DEVDRV_TS_BIN_MAX_SEGMENT_NUM];
};

int devdrv_judge_bin_validity_early(struct file *fp, loff_t *fsize, loff_t *pos, u32 *ts_check_file);
int devdrv_judge_bin_validity(u32 type, u32 ts_check_file, void *ts_bin, u32 check_fw_data);
int devdrv_is_ts_check_file(void);

#endif