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

#ifndef SOC_MISC_FEATURE_H
#define SOC_MISC_FEATURE_H

#include "drv_type.h"

#define DMS_SOC_CMD_NAME "DMS_SOC_MISC"

int dms_trans_and_check_id(u32 logical_dev_id, u32 *physical_dev_id, u32 *vfid);
int soc_misc_get_board_id(void *feature, char *in, u32 in_len,
    char *out, u32 out_len);
int soc_misc_get_pcb_id(void *feature, char *in, u32 in_len,
    char *out, u32 out_len);
int soc_misc_get_bom_id(void *feature, char *in, u32 in_len,
    char *out, u32 out_len);
int soc_misc_get_slot_id(void *feature, char *in, u32 in_len,
    char *out, u32 out_len);
int soc_misc_get_cpu_info(void *feature, char *in, u32 in_len,
    char *out, u32 out_len);
int soc_misc_feature_get_die_id(void *feature, char *in, u32 in_len,
    char *out, u32 out_len);
#ifdef CFG_FEATURE_CHIP_EXPAND_VERSION
int soc_misc_feature_get_chip_version(void *feature, char *in, u32 in_len,
    char *out, u32 out_len);
#endif
void soc_misc_feature_init(void);
void soc_misc_feature_uninit(void);
#endif
