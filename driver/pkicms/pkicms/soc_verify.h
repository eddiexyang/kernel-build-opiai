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

#ifndef SOC_VERIFY_H
#define SOC_VERIFY_H
#include "cmscbb.h"
#include "pkicms_common.h"

#ifdef CFG_SOC_PLATFORM_CLOUD
#define DEVICE_NUM_MAX 4
#elif (defined CFG_SOC_PLATFORM_MINIV2)
#define DEVICE_NUM_MAX 2
#define DEVICE_NUM_MAX_1P 1
#define DEVICE_NUM_MAX_2P 2
#elif (defined CFG_SOC_PLATFORM_CLOUD_V2)
#define DEVICE_NUM_MAX 2
#else
#define DEVICE_NUM_MAX 1
#endif

#define SC_PAD_INFO_PAGE_SIZE 0x1000
#define SC_PAD_INFO_BASE 0x8000E000
#define SC_PAD_INFO_OFFSET 0x8C
#define BOOT_CFG_MASK 0x20

#define IMG_HEAD_HASH_OFFSET 0x4BC

#define SIGN_RSA_ALG_SHIFT 16
#define SIGN_RSA_ALG_MASK 0x3F /* 16-21 bit indicate sign alg */
#define SIGN_RSA_PKCS_MODE 0x0
#define SIGN_RSA_PSS_MODE 0x1

#define RSA_SUBKEY_2048_BYTES 256
#define RSA_SUBKEY_4096_BYTES 512

#define RSA_SUBKEY_WIDTH_BYTES  256
#define RSA_E_BYTES             3

#define BOOTROM_SIGNATURE_OFFSET 0xC000
#define HEAD_OFFSET_NUM          2

#define BOOT_PRE_PARAM  0x55aa55aa
#define BOOT_MAGIC_CODE 0x33cc33cc

#define EFUSE_NS_MAGIC           0x1a4a5252

#ifdef CFG_FEATURE_SIGN_VERSION_1
#define SUBKEY_CERT_OFFSET       0x600
#else
#define SUBKEY_CERT_OFFSET       0x500
#endif
#define CODE_SIGN_OFFSET         0xE00
#define GENERAL_IMAGE_OFFSET     0x2000

#define MAX_IMAGE_SIZE           0x20000000 /* max image size 512M */
#define SUBKEY_ID_NUM_MASK       0x1F

#define  SOC_VERIFY_SUCCESS                     0x0
#define  SOC_VERIFY_FAILED                      0xFFFFFFFF

#define PASS_SLEN_SHIFT                     0x16
#define PASS_SLEN_MASK                      0x3FF

#define CHIP_TYPE_BS9SX1A 2

enum efuse_check_result {
    CHECK_FAIL,
    CHECK_SUCCESS,
    CHECK_BLANK
};

typedef struct {
    u32 hw_hash_check;
    u32 hw_category;
    u32 hw_sub_key_id;
    u32 hw_l2_nv_cnt;
} efuse_info_t;

typedef union {
    struct {
        u8 subkey_n[RSA_SUBKEY_2048_BYTES]; /* subkey module value */
        u8 subkey_e[RSA_SUBKEY_2048_BYTES]; /* subkye E value */
        u8 subkey_sign[ESBC_ROOTKEY_LEN]; /* signture result */
        u8 reserved[RSA_SUBKEY_4096_BYTES]; /* Reserved */
    } subkey_2048;
    struct {
        u8 subkey_n[RSA_SUBKEY_4096_BYTES]; /* subkey module value */
        u8 subkey_e[RSA_SUBKEY_4096_BYTES]; /* subkye E value */
        u8 subkey_sign[ESBC_ROOTKEY_LEN]; /* signture result */
    } subkey_4096;
} SUBKEY;

typedef struct {
    u32 sub_key_version;
    u32 sub_key_sign_alg;
    u32 sub_key_category;
    u32 sub_key_id;
    u32 sub_key_len;
    u32 sub_key_sign_len;
    SUBKEY sub_key;
} sub_key_cert_t;

s32 soc_verify_init(void);
s32 pkicms_get_chip_info(void);

#endif
