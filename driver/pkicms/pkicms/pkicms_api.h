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

#ifndef PKICMS_API_H_
#define PKICMS_API_H_

#include "pkicms_common.h"
#include <linux/types.h>

#define EBSC_FILE_CMS_MAX_LEN                   (32*1024)       /* more than 18K */

#define  CMS_VERIFY_SUCCESS                     0x0
#define  CMS_VERIFY_FAILED                      0xFFFFFFFFU
#define  CMS_SHA256_BLOCK_SIZE                  (16 * 1024 * 1024)
#define  CMS_SHA256_BLOCK_TIMEGAP               20
#define  CMS_SHA256_BLOCK_MAX_TIMEGAP           (20 * CMS_SHA256_BLOCK_TIMEGAP)
#define  CMS_LEN_MAX_SIZE                       1024
#define  CMS_SHA256_LEN_IN_BYTE                 32
#define  CMS_SHIFT_32BITS                       32

#define  CMS_IMAGE_ID_DEF                        0xEF
#define  PADDR_TO_VADDR_SUCCESS                  0x0

#define  CMS_HASH_NOT_END_PART                   0xF
#define  CMS_HASH_END_PART                       0xFF

#define UPGRADE_PKG_SHA256_MAX 128

#define PKICMS_ERR_BASK    0xFFFF01

/*
* cms signature file struct
*/
typedef struct cms_signature {
    u32 cms_size;
    u8 *cms;
    u32 ini_size;
    u8 *ini;
    u32 crl_size;
    u8 *crl;
} cms_signature_t;

/*
* cms file struct
*/
typedef struct cms_file_struct {
    u32 image_type;
    esbc_header_t *header;
    cms_signature_t signature;
    u8 *cms_buff; /* Used only inside this module */
} cms_file_t;

typedef struct verify_info {
    const char *file_name;
    u32 ini_type;
    int efuse_flag;
    u32 image_id;
} verify_info_st;

typedef struct second_sign {
    char *file_name;
    int image_id;
} second_sign_t;

typedef struct crl_info {
    u8 *crl;
    u32 crl_size;
} crl_info_st;

enum DM_VERIFY_FLAG {
    CHECK_INI_HASH_ENABLE = 0,
    CHECK_INI_HASH_DISABLE = 1
};
/**
 * read cms from file or storage
 */
int free_cms_file(cms_file_t *cms_info);
int alloc_cms_file(cms_file_t *cms_info);
int get_image_cms_file_info(verify_info_st *verify_info,
                            const char *filename,
                            cms_file_t *cms_info,
                            image_fop_t *file_op);

/**
* pkicms_verify_cms: verify file cms signature
* @image_id: see enum @IMAGE_ID
* @filename: file name and path
* @crl_info->crl: upgrade crl list file, if NULL, no need to update
* @crl_info->crl_size: crl_info->crlcrl data size, is crl_info->crlcrl is NULL, this value invald
* @op: if op is NULL, mean to use local file operations.
*
* verify file cms signature. and check file degiest use RSA256
*/
u32 pkicms_verify_cms(u32 image_id, const char *filename, crl_info_st* crl_info, image_fop_t *op);

/**
* pkicms_compare_crl: compare crl file's timestamp
* @crl_data1: crl data1
* @data1_size: crl data1 size
* @crl_data2: crl data2
* @data2_size: crl data2 size
*
* compare crl timestamp
*/
int pkicms_compare_crl(const char *crl_data1, u32 data1_size, const char *crl_data2, u32 data2_size, int *stat);

/**
* pkicms_get_root_hash: get root hash in boot_ini when using dm-verify
* @image_id: see enum @IMAGE_ID
* @filename: file name and path
* @roothash: buff to save roothash value
* @roothashlen: input buff length, when proc succ, the value should be change to actural value length
*
*/
int pkicms_get_root_hash(u32 image_id, const char *filename, char *roothash, int* roothashlen);
#endif
