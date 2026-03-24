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

#ifndef DMS_CHIP_INFO_H
#define DMS_CHIP_INFO_H

#include "drv_type.h"

#ifdef CFG_SOC_PLATFORM_MDC_V51

#define POWERON_BIST_MAGIC           0x42494F53
#define POWERON_BIST_STATE_MASK      0x10
#define POWERON_BIST_TIMEOUT_MASK    0x08
#define POWERON_BIST_FAIL_FLAG_MASK  0x04
#define POWERON_BIST_FININSH_MASK    0x02
#define POWERON_BIST_DONE_MASK       0x01

#define MBIST_VECTOR_INVALID_ID   0xff

#define MBIST_HW_ERR_CODE         0x01U
#define LBIST_HW_ERR_CODE         0x02U
#define POWERON_BIST_HW_ERR       0x01U
#define POWERON_BIST_SW_ERR       0x02U

typedef struct UNI_MSG_HDR_ {
    u32 magic;    // "BIOS" -- 0x42494F53
    u32 version;  // ver 100
    u32 len;      // len of PWRON_BIST_RLST
    u32 check_sum; // checksum of PWRON_BIST_RLST
} UNI_MSG_HDR;

typedef struct PWRON_BIST_RLST_ {
    u32 bist_result;        // dms填写，bios填0
    u32 error_code;         // 0:succ, 1:hardware err, 2:software err
    u32 vector_tested_cnt;  // 已测试向量数（等于向量总数）
    u32 vector_sum;         // 向量总数
    u32 vector_failed_id;   // BIOS不提供，默认0
    u32 vector_failed_cnt;  // 总共失败的向量数
    u32 resv[6];            // 保留，凑齐64字节
} PWRON_BIST_RLST;

// 64 byte
typedef struct BIOS_BIST_INFO_ {
    UNI_MSG_HDR    info_hdr;
    PWRON_BIST_RLST bist_info;
} BIOS_BIST_INFO;

struct dms_lp_bist_rslt_info {
    u32 error_code;
    u8 total_num;
    u8 next_vector_id;
    u8 next_seg_id;
    u8 tested_vector_num;
    u8 failed_vector_num;
    u8 first_failed_id;
    u8 reserved[6];  // aligned
} __attribute__((packed));

struct dms_sil_bist_rslt_info {
    u32 error_code;
    u32 dot_code;
    u8 total_num;
    u8 image_num;
    u8 next_image_id;
    u8 next_vector_id;
    u8 tested_vector_num;
    u8 failed_vector_num;
    u8 first_failed_id;
    u8 reserved[5];  // aligned
} __attribute__((packed));

struct dms_bist_unified_info {
    u32 failed_vector_id;
    u32 tested_vector_cnt;
};

typedef struct bist_result_stru {
    u32 magic;  // “BIST”
    BIOS_BIST_INFO  pwr_on_bist;
    struct dms_sil_bist_rslt_info sil_mbist_rslt;
    struct dms_lp_bist_rslt_info  lp_mbist_rslt;
    struct dms_lp_bist_rslt_info  lbist_rslt;
    struct dms_bist_unified_info  unified_info;
} BIST_RESULT_STRU;

typedef enum {
    POWER_ON_BIST,
    LOGIC_BIST,
    MEM_BIST,
    MAX_BIST_TYPE,
} BIST_TYPE;

#define BIST_INFO_RESERVED_LEN 10
typedef struct dms_bist_info {
    u32 bist_result;
    u32 error_code;
    u32 vector_tested_cnt;
    u32 vector_sum;
    u32 vector_failed_id;
    u32 vector_failed_cnt;
    u32 reserved[BIST_INFO_RESERVED_LEN];
} DMS_BIST_INFO;

typedef struct dms_bist_result_stru {
    u32 bist_result;
    u32 failed_vector_id;
    u32 tested_vector_cnt;
    DMS_BIST_INFO bist_info[MAX_BIST_TYPE];
} DMS_BIST_RESULT_STRU;

#define MAX_BIST_VEC_CNT 128
typedef struct bist_vector_info {
    u32 valid;
    u32 vector_cnt;
    u32 lp_bist_vec_offset;
    u32 vector_time[MAX_BIST_VEC_CNT];
} BIST_VECTOR_INFO;

#endif

int dms_feature_get_chip_info(void *feature, char *in, u32 in_len, char *out, u32 out_len);
int dms_trans_and_check_id(u32 logical_dev_id, u32 *physical_dev_id, u32 *vfid);
#ifdef CFG_FEATURE_REBOOT_REASON
int dms_get_reboot_reason(void *feature, char *in, u32 in_len, char *out, u32 out_len);
#endif
#ifdef CFG_SOC_PLATFORM_MDC_V51
int dms_get_bist_info(void *feature, char *in, u32 in_len, char *out, u32 out_len);
int dms_set_bist_mode(void *feature, char *in, u32 in_len, char *out, u32 out_len);
BIST_VECTOR_INFO *dms_get_bist_vector_info_addr(void);

int dms_bist_init(void);
void dms_bist_uninit(void);
#endif

#endif
