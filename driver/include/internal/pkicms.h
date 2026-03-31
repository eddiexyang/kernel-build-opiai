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

#ifndef PKICMS_H
#define PKICMS_H

#ifdef LINUX_KERNEL_BUILD
#  include "cmscbb_adapter.h"
#else
#  include "cmscbb_types.h"
#endif

#define IS_BETWEEN_VALUE(value, min, max)   (((value) >= (min)) && ((value) <= (max)))
#define CAL_CHAR_VALUE(value, min, inc)     ((value) - (min) + ((char)inc))

#define CMS_CRL_FILE_SIZE_16K               (16 * 1024)
#define CMS_FILE_SIZE_8K                    (8 * 1024)
#define CMS_FILE_SIZE_16K                   (16 * 1024)
#define CMS_INI_FILE_SIZE_2K                (2 * 1024)
#define TAG_MAX_SIZE                        32
#define SHA256_LEN                          32
#define IMAGE_NUMS                          4

#define SCPS_UPDATE_DESTROY_DEV_VALID 4
#define SCPS_UPDATE_VALID_DEV_DESTROY 5

enum INI_TYPE {
    TYPE_TEE = 0,
    TYPE_DTS,
    TYPE_KERNEL,
    TYPE_INITRD,
    TYPE_SD_PEK_DTB,
    TYPE_SD,
    TYPE_PEK,
    TYPE_DP,
    TYPE_ROOTFS,
    TYPE_APP,
    TYPE_DTB_DP_PEK,
    TYPE_DTB_SD_PEK,
    TYPE_DP_PEK,
    TYPE_SD_PEK,
    TYPE_DP_CORE,
    TYPE_ABL_PATCH,
    TYPE_INI_MAX = 100
};

typedef struct cms_crl_flags {
    unsigned int FLAG;  /* Enable the flag */
    unsigned int CRL_LEN;
    unsigned int SO_LEN1;
    unsigned int SO_LEN2;
    unsigned int RAT_LEN;
    unsigned int CMS_LEN;
    unsigned int reserved;
} CMS_CRL_flags;

typedef struct hash_value {
    unsigned char tag[TAG_MAX_SIZE];
    unsigned char hash[SHA256_LEN];
    unsigned int inited;
} INI_TABLE;

/* crl info for crl timestamp compare */
typedef struct __crl_cmp_info {
    unsigned int crl_size1;
    unsigned char *crl_data1;
    unsigned int crl_size2;
    unsigned char *crl_data2;
} crl_cmp_info_t;

CMSCBB_ERROR_CODE pkicms_check_ini_hash(const unsigned char *hash, unsigned int hash_size, unsigned int type);
CMSCBB_ERROR_CODE pkicms_verify_cms_data(unsigned char *pb_cms, size_t cms_len,
                                         unsigned char *pb_src, size_t src_len,
                                         unsigned char *pb_crl, size_t crl_len,
                                         unsigned int ini_type);
void pkicms_reset_ini_hash(unsigned int ini_type);
CMSCBB_ERROR_CODE pkicms_hw_crl_compare(crl_cmp_info_t *crl_info, int *stat);


typedef enum pkicms_esbc_sign_alg {
    PKICMS_RSA_PKCS1 = 0,
    PKICMS_RSA_PSS = 1
} pkicms_sign_alg;

/**
* get rsa signature alg type
*
* This interface is used to obtain the signature type based on
* uwESBCSignAlg in the signature file header. Signature algorithm
* types include pkcs1 and PSS.
*/
enum pkicms_esbc_sign_alg pkicms_get_alg_type_by_sign_data(unsigned int esbc_sign_alg);

/*
 * get the signature algorithm type based on the current ESBC signature data.
 *
 * @return:
 *    RSA_PKCS1 or RSA_PSS
 */
enum pkicms_esbc_sign_alg pkicms_get_rsa_sign_alg_type(void);

unsigned int pkicms_get_rsa_pss_salt_len(void);
void pkicms_set_rsa_sign_alg(unsigned int sign_alg);
unsigned int pkicms_get_cms_file_size(pkicms_sign_alg sign_type);

#endif
