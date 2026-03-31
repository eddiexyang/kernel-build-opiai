/*
* Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
* Create: 2022-11-11
*/

#ifndef HCCS_FEATURE_H
#define HCCS_FEATURE_H

#include <linux/time.h>
#include "dms_template.h"

/* PCS status */
#define ST_CH_PCS_LANE_MODE_CHANGE_OFFSET 0x860
#define ST_PCS_MODE_WORKING_X4 0x2
#define ST_PCS_USE_WORKING_X4 0xF
#define PCS_STATUS_OFFSET 0
#define PCS_INDEX_OFFSET 8
#define PCS_MODE_WORKING_OFFSET 16
#define PCS_USE_WORKING_OFFSET 24

#define PCS_NUM 8
#define PCS0_BASE_ADDR 0x000703B90000
#define PCS1_BASE_ADDR 0x000703BA0000
#define PCS2_BASE_ADDR 0x000703BB0000
#define PCS3_BASE_ADDR 0x000703BC0000
#define PCS4_BASE_ADDR 0x000703BD0000
#define PCS5_BASE_ADDR 0x000703BE0000
#define PCS6_BASE_ADDR 0x000703BF0000
#define PCS7_BASE_ADDR 0x000703C00000

#define DMS_HCCS_INFO_RESERVED_BYTES 8
typedef struct hccs_info_struct {
    unsigned int pcs_status;
    unsigned char reserved[DMS_HCCS_INFO_RESERVED_BYTES];
} hccs_info_t;

int dms_hccs_init(void);
int dms_hccs_exit(void);
int dms_get_hccs_status_by_dev_id(unsigned int dev_id, hccs_info_t *hccs_status);

#ifdef CFG_FEATURE_GET_PCS_BITMAP_BY_BOARD_TYPE
int dms_get_hpcs_bitmap_by_board_type(unsigned int dev_id, unsigned long long *bitmap);
#define DMS_GET_HPCS_BITMAP dms_get_hpcs_bitmap_by_board_type
#else
int dms_get_hpcs_bitmap_default(unsigned int dev_id, unsigned long long *bitmap);
#define DMS_GET_HPCS_BITMAP dms_get_hpcs_bitmap_default
#endif

#define DMS_MODULE_HCCS  "hccs"
INIT_MODULE_FUNC(DMS_MODULE_HCCS);
EXIT_MODULE_FUNC(DMS_MODULE_HCCS);

#endif