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

#ifndef CMSCBB_H
#define CMSCBB_H
#include "pkicms_common.h"
#include "cmscbb_adapter.h"
#include "soc_verify.h"

#define DIGEST_SIZE_SHA256              32
#define DIGEST_SIZE_SHA384              48
#define DIGEST_SIZE_SHA512              64
#define CVB_UINT_MAX_NUM                0xFFFFFFFF
#define RSA_KEY_LEN_2048                0x100
#define RSA_KEY_LEN_4096                0x200
#define CMS_ATTR_NUM_2ATTRS             2
#define RSA_PSS_TAIL_V                  0xBC
#define RSA_PSS_TAIL_LEN                1

#define ITEE_CMS_MEM_1K_SIZE            1024
#define ITEE_CMS_MEM_SIZE               (1023 *1024)
#define ITEE_CMS_BITMAP_SIZE            (1024)
#define ITEE_MAX_INDEX_1K               (1023)
#define ITEE_CMS_TYPE_MAGIC             0x4a4a5b5b
#define ITEE_CMS_SADDR_MAGIC            0x3a3a2d2d

#define OEM_MEM_USED_MAGIC              0x5a5a5a5a
#define OEM_MEM_MANAGE_MAGIC            0xa5a5a5a5

#define RSA_PSS_2048_BYTES                   256
#define RSA_PSS_4096_BYTES                   512
#define DBMASK_DATA_LEN                      518  // max_slatlen(478 byte) + hashlen(32 byte) + padding1(8 byte)
#define SHA512_BLEN                          64
#define RSA_PSS_CNT_LEN                      4
#define RSA_PSS_HEAD_MASK                    0x7F

#define W2B_SIZE                                        4
#define W2B_OFF0                                        0
#define W2B_OFF1                                        1
#define W2B_OFF2                                        2
#define W2B_OFF3                                        3
#define BYTE_MASK                                       0xFF
#define SHIFT0                                          0
#define SHIFT8                                          8
#define SHIFT16                                         16
#define SHIFT24                                         24
#define PSS_PADDING1                                    0x1
#define RSA_PSS_P1_LEN                                  8

typedef struct crypto_md_st {
    shash_sha256_t op;
    CVB_UINT32 hash_algo;
    const CVB_BYTE *pbData;
    CVB_UINT32 len;
} TA_CRYPTO_MD;

typedef struct crypto_vrf_st {
    TA_CRYPTO_MD *md_ctx;
    uint32_t key_size;
    void *nkey;
    void *ekey;
} TA_CRYPTO_VRF;

typedef struct {
    CVB_UINT32    Magic;
    CVB_UINT32    Length; // memory len, 0.5K as unit
    CVB_UINT32    Used;
    CVB_UINT32    StartAddr;
    CVB_UINT32    Type; // stands for basic unit is 1k or 0.5k
    CVB_UINT32    Resv[3];
} OEM_MEM_HEADER;

#endif
