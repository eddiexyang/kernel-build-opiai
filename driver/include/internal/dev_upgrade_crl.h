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

#ifndef _DEV_UPGRADE_CRL_H_
#define _DEV_UPGRADE_CRL_H_

#include "dev_upgrade_public.h"
#include "pkicms_api.h"

#define ITRUSTEE_IMG_FILE "itrustee.img"

#define DEV_UPGRADE_CRL_BUF_SIZE (16 * 1024)     // 16KB
#define DEV_UPGRADE_LOCAL_PKCS_CRL_PATH "/usr/local/Ascend/driver/crl/pkcs_crls.crl"
#define DEV_UPGRADE_LOCAL_PSS_CRL_PATH "/usr/local/Ascend/driver/crl/pss_crls.crl"

#define CRL_FILE_LUN 0
#define CRL_FILE_OFFSET 0x900000
#define CRL_FILE_TAG "crl"
#define CRL_SIZE_OFFSET 12
#define FILE_EXIST 0
#define FILE_NOT_EXIST 1
#define TEE_IMG_SIZE_MAX 0x1000000

#define IMG_FILE_HEAD 0x55aa55aa
#define CRL_FILE_VALID 0
#define CRL_FILE_NOT_VALID 1

#define CRL_CMP_SAME 0
#define CRL_CMP_NEW 1
#define CRL_CMP_OLD 2
#define CRL_CMP_MIX 3

#define CMS_TAG (0xABCD1234AA55AA55)
#define CMS_TAG_OFFSET offsetof(esbc_header_t, cms_tag)

#define IMG_STATUS_NOT_EXIST 0
#define IMG_STATUS_NOT_VERIFY 1
#define IMG_STATUS_VERIFY_FAIL 2
#define IMG_STATUS_VERIFY_SUCCESS 3

/* flag for crl process type */
#define CRL_PROCESS_TYPE_UPDATE_IMG 0
#define CRL_PROCESS_TYPE_UPDATE_CRL 1

/* status for image */
#define IMG_STATUS_NOT_FOUND 0
#define IMG_STATUS_NOT_HAVE_CRL 1
#define IMG_STATUS_HAVE_CRL 2

/* update image and crl result */
#define CRL_VERIFY_RESULT_NOT_UPGRADE 0
#define CRL_VERIFY_RESULT_UPGRADE 1
#define CRL_UPDATE_RESULT_NOT_UPGRADE 2
#define CRL_UPDATE_RESULT_UPGRADE 3

#define HASH_SIZE (32)
#define USER_DEFINE_SIZE 32
#define RSA_ROOTKEY_WIDTH_BYTES (512)

#define CMS_NAME_DOMAIN_LEN (12)
#define CMS_TAG_AND_LEN_SIZE (16)
#define CMS_DATA_ALIGN (16)
#define CMS_CRL_FILE_SIZE_16K (16 * 1024)
#define CMS_INI_FILE_SIZE_2K (2 * 1024)
#define CMS_CRL_FILE_SIZE_MAX 0x200000U

#define CMS_DATA_RESERVE_GAP (32)
#define SECURE_HDR_SIZE (0x2000)

#define IMAGE_TYPE_FILE 0
#define IMAGE_TYPE_UFS 1

#define PKICMS_SEC_CHECK_ENABLE 1
#define PKICMS_SEC_CHECK_DISABLE 0

#define IMG_ID_MAX_NUM 100

/* read image flag */
#define READ_IMG_FILE_FLAG_TOTAL 0 /* read file data according to file size */
#define READ_IMG_FILE_FLAG_BY_SIZE 1 /* read file data according to input size */

typedef struct _hw_type_cms_struct {
    unsigned long base;
    unsigned long size;
    unsigned int image_type;
    unsigned char *cms_header;
    unsigned char *cms;
    unsigned int cms_size;
    unsigned char *src_header;
    unsigned char *src;
    unsigned int src_size;
    unsigned char *crl_header;
    unsigned char *crl;
    unsigned int crl_size;
} hw_type_cms_struct;

typedef struct __img_crl_info {
    /* update image info */
    unsigned int update_img_size;
    unsigned char *update_img_data;
    unsigned int update_img_crl_size;
    unsigned char *update_img_crl_data;
    unsigned int update_img_status;
    /* ufs image info */
    unsigned int ufs_img_size;
    unsigned char *ufs_img_data;
    unsigned int ufs_img_crl_size;
    unsigned char *ufs_img_crl_data;
    unsigned int ufs_img_status;
    /* ufs crl info */
    unsigned int ufs_crl_size;
    unsigned char *ufs_crl_head;
    unsigned char *ufs_crl_data;
    unsigned int ufs_crl_status;
    /* update crl info */
    unsigned int update_crl_size;
    unsigned char *update_crl_data;
    unsigned int update_crl_status;
    unsigned int result;
} img_crl_info_t;

/* image header's data struct */
typedef struct tagBootImageStr {
    unsigned int uwLPreamble; /* 0x55aa55aa */
    unsigned int uwLHeadLen;  /* don't check during secure booting */
    unsigned int uwLUserLen;
    unsigned char ucLUserDefineData[USER_DEFINE_SIZE]; /* don't check during secure booting */
    unsigned char ucLHash[HASH_SIZE];
    unsigned int uwSubKeyCertOffset;
    unsigned int uwL2SignAlg; /* [15:0]Hash algorithm;0x0-SHA256, others: reserved
                                                [31:22](10bit)signature parasms.RSA_PKCS1-0,RSA_PSS standsfor salt
                             length(byte) [21:16](6bit)signature algorithm.0x0:RSA_PKCS1; 0x1: RSA_PSS */
    unsigned int uwRootPubKLen;
    unsigned char ucRootPubKE[RSA_ROOTKEY_WIDTH_BYTES]; /* E valuse & fixed to 65537 */
    unsigned char ucRootPubK[RSA_ROOTKEY_WIDTH_BYTES];  /* N value, length is RootPubKLen */
    unsigned int uwLCodeOffset;                       /* offset between image header */
    unsigned int uwLCodeLen;
    unsigned int uwLSign1Offset;
    unsigned int uwLSign2Offset;
    unsigned int uwLHeadMagic; /* 0x33cc33cc */
} SE_IMAGE_HEAD;

typedef struct __crl_info {
    unsigned int old_crl_size;
    unsigned char *old_crl_data;
    unsigned int new_crl_size;
    unsigned char *new_crl_data;
    unsigned int img_data_size;
    unsigned char *img_data;
} crl_info_t;

typedef int (*dev_upgrade_crl_handler)(const char *file_path, img_crl_info_t *info);

int dev_upgrade_update_crl_process(unsigned char *buf, unsigned int size);
int dev_upgrade_verify_img_crl_process(const char *update_img_path);
int dev_upgrade_verify_cms_img_process(int component_type, const char *path_name);

void dev_upgrade_get_img_id(const char *path_name, int *img_id);
int dev_upgrade_read_img(uintptr_t local_img, char *buff, u32 len, int *read_len);
struct local_image_verify *dev_upgrade_get_local_imgs(void);
image_fop_t *get_image_verify_ops(void);
void dev_upgrade_img_id_uninit(void);
void dev_upgrade_img_id_init(void);

#endif
