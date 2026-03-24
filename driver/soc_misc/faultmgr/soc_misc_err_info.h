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

#ifndef SOC_MISC_ERR_INFO_H
#define SOC_MISC_ERR_INFO_H

#include <linux/types.h>
#include <linux/cper.h>

#include "fpdc.h"

#define SOC_ERR_SECTOR_SAFETY (0xFFU)
#define SOC_ERR_SECTOR_RAS    (0xFEU)

enum {
    SOC_MISC_RAS_ERR_UC = 0,
    SOC_MISC_RAS_ERR_CE = 1
};

enum soc_misc_cache_level {
    SOC_MISC_IFU = 0,
    SOC_MISC_LSU = 1,
    SOC_MISC_MMU = 2,
    SOC_MISC_L2C = 3,
    SOC_MISC_L3D = 4,
    SOC_MISC_L3T = 5
};

struct cper_arm_cache_err {
    u64 valid_bit : 16;
    u64 transaction_type : 2;
    u64 operation : 4;
    u64 cache_level : 3;      /* 0-IFU
                               * 1-LSU
                               * 2-MMU
                               * 3-L2C
                               * 4-L3D
                               * 5-L3T
                               */
    u64 proc_ctx_corrupt : 1; /* 0-not corrupted
                               * 1-corrupted
                               */
    u64 corrected : 1;        /* 1: Corrected
                               * 0: Uncorrected
                               */
    u64 precise_program_counter : 1;
    u64 restartable_program_exec : 1;
    u64 reserved : 35;
};

/**
 * check notify data signed by FPDC
 *
 * @return: 0: OK, -EINVAL: invalid
 */
int soc_misck_check_notify_data(const struct notify_data *pdata);

/**
 * get ERR.status from cper_sec_proc_arm or hisi_common_error_info
 */
int soc_misc_get_err_status(const struct notify_data *pdata, u32 *err_status);

/**
 * get cpu cluster id from MPIDR
 */
int soc_misc_get_cpu_cluster_id(struct cper_sec_proc_arm *pdata_ras, u8 *cpu_cluster);

/**
 * compare section type guid
 */
int soc_misc_sec_type_guid_cmp(const guid_t *u1, const guid_t *u2);

int soc_misc_arm_get_vendor_info(struct cper_sec_proc_arm *arm_err,
    struct vendor_specific_error_info **vendor_info);
#endif
