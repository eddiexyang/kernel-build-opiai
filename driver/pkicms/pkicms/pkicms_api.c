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

#ifdef LINUX_KERNEL_BUILD
#include "pkicms_api.h"
#include "cmscbb.h"
#include "pkicms.h"
#include "soc_verify.h"
#include "drv_pkicms.h"
#include <crypto/hash.h>
#include <linux/securec.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/vmalloc.h>
#include <linux/string.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <limits.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include "securec.h"
#include "pkicms.h"
#include "pkicms_api.h"
#include "ascend_inpackage_hal.h"
#endif
#include "cmscbb_types.h"

#define SECURE_HEAD_SIZE 0x2000
#define SHA256_BYTE_LEN 128

#define CMS_DATA_ALIGN 16
#define CMS_DATA_RESERVE_GAP 32
#define CMS_NAME_DOMAIN_LEN 12
#define CMS_TAG_AND_LEN_SIZE 16

#define FS_VERSION_OFFSET   (-4)
#define FS_VERSION_SIZE     4
#define HEAD_ADDR_OFFSET    (-8)
#define HEAD_ADDR_OFFSET_EX (-12)
#define HEAD_ADDR_SIZE      4
#define HEAD_ADDR_SIZE_EX   8
#define NORMAL_PACKET_TYPE  0
#define LARGE_PACKET_TYPE   0x4c474550   // LGEP magic

#define PACKAGE_FORMAT_HWHEAD_FIRST 1
#define PACKAGE_FORMAT_IMAGE_FIRST 2

#define HW_SIGNATURE_HEAD_SIZE (4 * 1024)
#define CUSTOMER_SIGNATURE_HEAD_SIZE (4 * 1024)
#define SECOND_HEAD_SIZE 256
#define ABL_PATCH_MAX_LEN (20 * 1024 * 1024)

#define ESBC_HEADER_SIZE sizeof(esbc_header_t)

#define PKICMS_RSA_PSS_MASK (0x08010000U)
#define PKICMS_RSA_PKCS_MASK (0x00000000U)

#define HEX_TO_STRING_COFF (2)
#define INT_TAG_SUFFIX_LEN (1)
STATIC int ufs_img_param_check(u32 ini_type)
{
    if (ini_type >= TYPE_INI_MAX) {
        pkicms_err("cms verify ini_type params wrong! ini_type = %u\n", ini_type);
        return -EINVAL;
    }

    return 0;
}

u32 pkicms_cms_verify(cms_signature_t *signature, u32 ini_type)
{
    CMSCBB_ERROR_CODE check_state;

    if (signature == NULL) {
        pkicms_err("input signature is null\n");
        return CMS_VERIFY_FAILED;
    }

    check_state = pkicms_verify_cms_data(signature->cms,
        signature->cms_size,
        signature->ini,
        signature->ini_size,
        signature->crl,
        signature->crl_size,
        ini_type);
    if (check_state != CVB_SUCCESS) {
        pkicms_err("cms info verify failed, check state = 0x%x\n", check_state);
        return CMS_VERIFY_FAILED;
    }

    return CMS_VERIFY_SUCCESS;
}
EXPORT_SYMBOL(pkicms_cms_verify);

int free_cms_file(cms_file_t *cms_info)
{
    if (cms_info == NULL) {
        pkicms_err("cms_info is NULL pointer.\n");
        return -EINVAL;
    }

    if (cms_info->header != NULL) {
        (void)kfree(cms_info->header);
        cms_info->header = NULL;
    }

    if (cms_info->cms_buff != NULL) {
        (void)vfree(cms_info->cms_buff);
        cms_info->cms_buff = NULL;
    }
    return 0;
}
EXPORT_SYMBOL(free_cms_file);

int alloc_cms_file(cms_file_t *cms_info)
{
    int ret;

    if (cms_info == NULL) {
        pkicms_err("cms_info is NULL pointer.\n");
        return -EINVAL;
    }

    cms_info->header = (esbc_header_t *)kmalloc(ESBC_HEADER_SIZE, GFP_KERNEL);
    if (cms_info->header == NULL) {
        pkicms_err("alloc memory for cms header failed.\n");
        goto free_header;
    }

    ret = memset_s(cms_info->header, ESBC_HEADER_SIZE, 0, ESBC_HEADER_SIZE);
    if (ret != EOK) {
        pkicms_err("memset cms header failed.\n");
        goto free_header;
    }

    cms_info->cms_buff = (u8 *)vmalloc(EBSC_FILE_CMS_MAX_LEN);
    if (cms_info->cms_buff == NULL) {
        pkicms_err("alloc memory failed for cms buffer failed.\n");
        goto free_header;
    }

    ret = memset_s(cms_info->cms_buff, EBSC_FILE_CMS_MAX_LEN, 0, EBSC_FILE_CMS_MAX_LEN);
    if (ret != EOK) {
        pkicms_err("memset cms buffer failed.\n");
        goto free_header;
    }
    return 0;

free_header:
    free_cms_file(cms_info);
    return -ENOMEM;
}
EXPORT_SYMBOL(alloc_cms_file);

STATIC int get_image_format(u32 ini_type, int *format)
{
    int ret;

    ret = ufs_img_param_check(ini_type);
    if (ret != 0) {
        pkicms_err("ini_type is illegal, ret = %d\n", ret);
        return ret;
    }

    if (ini_type == TYPE_ROOTFS || ini_type == TYPE_APP) {
        *format = PACKAGE_FORMAT_IMAGE_FIRST;
    } else {
        *format = PACKAGE_FORMAT_HWHEAD_FIRST;
    }
    return 0;
}

u32 pkicms_get_cms_file_size(pkicms_sign_alg sign_type)
{
    unsigned int cms_file_size;
    if (sign_type == PKICMS_RSA_PKCS1) {
        cms_file_size = CMS_FILE_SIZE_8K;
    } else {
        cms_file_size = CMS_FILE_SIZE_16K;
    }
    return cms_file_size;
}
EXPORT_SYMBOL(pkicms_get_cms_file_size);

static void parse_cms_signature(u8 *read_buff, cms_file_t *cms_info)
{
    pkicms_sign_alg sign_type = pkicms_get_rsa_sign_alg_type();

    /* read cms */
    cms_info->signature.cms_size = *(u32 *)(uintptr_t)(read_buff + CMS_NAME_DOMAIN_LEN);
    cms_info->signature.cms = read_buff + CMS_TAG_AND_LEN_SIZE;
    read_buff += pkicms_get_cms_file_size(sign_type);

    /* read code hash code which tag is ini */
    cms_info->signature.ini_size = *(u32 *)(uintptr_t)(read_buff + CMS_NAME_DOMAIN_LEN);
    cms_info->signature.ini = read_buff + CMS_TAG_AND_LEN_SIZE;
    read_buff += CMS_INI_FILE_SIZE_2K;

    /* read crl */
    cms_info->signature.crl_size = *(u32 *)(uintptr_t)(read_buff + CMS_NAME_DOMAIN_LEN);
    cms_info->signature.crl = read_buff + CMS_TAG_AND_LEN_SIZE;
}

STATIC int get_fs_version_flag(uintptr_t pfile, image_fop_t *file_op, u32 *version_flag)
{
    int ret;
    int read_len = 0;
    char buf[FS_VERSION_SIZE] = {0};

    if (file_op->lseek(pfile, FS_VERSION_OFFSET, SEEK_END) < 0) {
        pkicms_err("lseek failed. get version flag offset failed.\n");
        return -EFAULT;
    }

    ret = file_op->read(pfile, buf, FS_VERSION_SIZE, &read_len);
    if ((ret == READ_ERROR) || (read_len < (int)FS_VERSION_SIZE)) {
        pkicms_err("read file failed. read_pos=%lld.\n", file_op->cur_offset(pfile));
        return -EIO;
    }

    *version_flag = *(u32 *)(uintptr_t)buf;
    return 0;
}

STATIC int get_hwhead_offset(uintptr_t pfile, image_fop_t *file_op, loff_t *offset)
{
    int ret;
    int read_len = 0;
    u32 version_flag = 0;
    u32 hwhead_addr_size;
    loff_t hwhead_addr_offset;
    char buf[HEAD_ADDR_SIZE_EX] = {0};

    ret = get_fs_version_flag(pfile, file_op, &version_flag);
    if (ret != 0) {
        pkicms_err("get filesystem version flag failed.\n");
        return ret;
    }

    if (version_flag == NORMAL_PACKET_TYPE) {
        hwhead_addr_offset = HEAD_ADDR_OFFSET;
        hwhead_addr_size = HEAD_ADDR_SIZE;
    } else if (version_flag == LARGE_PACKET_TYPE) {
        hwhead_addr_offset = HEAD_ADDR_OFFSET_EX;
        hwhead_addr_size = HEAD_ADDR_SIZE_EX;
    } else {
        pkicms_err("version flag is invalid.(version_flag=%u)\n", version_flag);
        return -EFAULT;
    }

    if (file_op->lseek(pfile, hwhead_addr_offset, SEEK_END) < 0) {
        pkicms_err("lseek failed. get hw_sighead offset failed.\n");
        return -EFAULT;
    }

    ret = file_op->read(pfile, buf, hwhead_addr_size, &read_len);
    if ((ret == READ_ERROR) || (read_len < (int)hwhead_addr_size)) {
        pkicms_err("read file failed. read_pos=%lld.\n", file_op->cur_offset(pfile));
        return -EIO;
    }

    *offset = *(loff_t *)(uintptr_t)buf;
    return 0;
}

STATIC int get_cms_file_info(uintptr_t pfile, const image_fop_t *file_op, cms_file_t *cms_info, loff_t cms_offset)
{
    int read_flag;
    loff_t seek_ret;
    u8 *read_buff = NULL;
    int read_len = 0;
    pkicms_sign_alg sign_alg = pkicms_get_rsa_sign_alg_type();
    u32 max_cms_size = pkicms_get_cms_file_size(sign_alg);

    seek_ret = file_op->lseek(pfile, cms_offset, SEEK_SET);
    if (seek_ret < 0) {
        pkicms_err("seek file failed. maybe file format error, ret = %lld.\n", seek_ret);
        return -EINVAL;
    }

    read_buff = cms_info->cms_buff;
    read_flag = file_op->read(pfile, (char *)read_buff, EBSC_FILE_CMS_MAX_LEN, &read_len);
    if ((read_flag == READ_ERROR) || (read_len < 0)) {
        pkicms_err("read file failed. read pos=%lld.\n", file_op->cur_offset(pfile));
        return -EINVAL;
    }

    /* Check whether the CMS signature read contains cms, ini, and crl. */
    if (read_len <= ((int)max_cms_size + CMS_INI_FILE_SIZE_2K + CMS_TAG_AND_LEN_SIZE)) {
        pkicms_err("read file failed. read len error. read_len = %d, max_cms_size = %d\n",
            read_len,
            (int)max_cms_size + CMS_INI_FILE_SIZE_2K + CMS_TAG_AND_LEN_SIZE);
        return -EINVAL;
    }
    parse_cms_signature(read_buff, cms_info);

    return 0;
}

STATIC int get_var_info(uintptr_t pfile, image_fop_t *file_op, img_ver_header *img_ver, loff_t cms_domain_offset)
{
    int ret;
    int read_flag;
    loff_t seek_ret;
    u8 read_buff[sizeof(img_ver_header) + 1] = {0};
    int read_len = 0;

    seek_ret = file_op->lseek(pfile, cms_domain_offset, SEEK_SET);
    if (seek_ret < 0) {
        pkicms_err("seek file failed. maybe file format error, ret = %lld.\n", seek_ret);
        return -EINVAL;
    }

    read_flag = file_op->read(pfile, (char *)read_buff, sizeof(img_ver_header), &read_len);
    if ((read_flag == READ_ERROR) || (read_len < 0)) {
        pkicms_err("read file failed. read_pos=%lld.\n", file_op->cur_offset(pfile));
        return -EINVAL;
    }

    /* Check whether the VAR read contains MAGIC, TAG, NVCNT and VAR. */
    if (read_len < sizeof(img_ver_header)) {
        pkicms_err("read file failed. read len(%d) error.\n", read_len);
        return -EINVAL;
    }

    ret = memcpy_s(img_ver, sizeof(img_ver_header), read_buff, sizeof(img_ver_header));
    if (ret != EOK) {
        pkicms_err("memcpy_s img_code failed, ret is %d", ret);
        return ret;
    }

    return 0;
}

STATIC int get_image_var_info(
    verify_info_st *verify_info, const char *filename, img_ver_header *img_ver, image_fop_t *file_op)
{
    uintptr_t pfile;
    int ret;
    int format = 0;
    loff_t var_offset;
    loff_t hwhead_offset = 0;

    if (file_op == NULL) {
        pkicms_err("image file operation obj is NULL.\n");
        return -EINVAL;
    }

    pfile = file_op->open(filename);
    if (pfile == 0) {
        pkicms_err("error occured while open file.\n");
        return -EIO;
    }

    ret = get_image_format(verify_info->ini_type, &format);
    if (ret != 0) {
        pkicms_err("get image format failed, ini_type = %d\n", verify_info->ini_type);
        goto close_file;
    }

    if (format == PACKAGE_FORMAT_IMAGE_FIRST) {
        ret = get_hwhead_offset(pfile, file_op, &hwhead_offset);
        if (ret != 0) {
            pkicms_err("get hwhead offset failed.\n");
            goto close_file;
        }

        if (file_op->lseek(pfile, hwhead_offset, SEEK_SET) < 0) {
            ret = -EIO;
            pkicms_err("seek hwhead failed. can not point to hw signature head.\n");
            goto close_file;
        }
    }

    if (format == PACKAGE_FORMAT_IMAGE_FIRST) {
        if (hwhead_offset <= SECOND_HEAD_SIZE) {
            ret = -EIO;
            goto close_file;
        }
        var_offset = hwhead_offset - SECOND_HEAD_SIZE;
    } else {
        var_offset = SECURE_HEAD_SIZE;
    }

    ret = get_var_info(pfile, file_op, img_ver, var_offset);

close_file:
    file_op->release(pfile);
    pfile = 0;
    return ret;
}

static int get_cms_offset(uintptr_t pfile, image_fop_t *file_op, esbc_header_t *header, int format, loff_t *offset)
{
    int ret, read_flag;
    int read_len = 0;
    loff_t hwhead_offset = 0;

    if (format == PACKAGE_FORMAT_IMAGE_FIRST) {
        ret = get_hwhead_offset(pfile, file_op, &hwhead_offset);
        if (ret != 0) {
            pkicms_err("get hwhead offset failed.\n");
            return -EFAULT;
        }

        if (file_op->lseek(pfile, hwhead_offset, SEEK_SET) < 0) {
            pkicms_err("seek hwhead failed. can not point to hw signature head.\n");
            return -EIO;
        }
    }

    read_flag = file_op->read(pfile, (char *)header, ESBC_HEADER_SIZE, &read_len);
    if ((read_flag == READ_ERROR) || (read_len < (int)ESBC_HEADER_SIZE)) {
        pkicms_err("read file failed. read_pos=%lld.\n", file_op->cur_offset(pfile));
        return -EIO;
    }

    if (format == PACKAGE_FORMAT_IMAGE_FIRST) {
        *offset = hwhead_offset + HW_SIGNATURE_HEAD_SIZE + CUSTOMER_SIGNATURE_HEAD_SIZE;
    } else {
        *offset = SECURE_HEAD_SIZE + header->code_len + CMS_DATA_RESERVE_GAP - (header->code_len % CMS_DATA_ALIGN);
    }
    return 0;
}

int get_image_cms_file_info(verify_info_st *verify_info, const char *filename, cms_file_t *cms_info,
    image_fop_t *file_op)
{
    uintptr_t pfile;
    int ret;
    int format = 0;
    loff_t cms_domain_offset;

    if (file_op == NULL) {
        pkicms_err("image file operation obj is NULL.\n");
        return -EINVAL;
    }

    if (verify_info == NULL || cms_info == NULL) {
        pkicms_err("verify_info or cms_info is NULL pointer.\n");
        return -EINVAL;
    }

    pfile = file_op->open(filename);
    if (pfile == 0) {
        pkicms_err("error occured while open file.\n");
        return -EIO;
    }

    ret = get_image_format(verify_info->ini_type, &format);
    if (ret != 0) {
        pkicms_err("get image format failed, ini_type = %d is illegal\n", verify_info->ini_type);
        goto close_file;
    }

    ret = get_cms_offset(pfile, file_op, cms_info->header, format, &cms_domain_offset);
    if (ret != 0) {
        pkicms_err("get_cms_offset failed.\n");
        goto close_file;
    }

    if (verify_info->efuse_flag == PKICMS_SEC_CHECK_ENABLE) {
        ret = get_cms_file_info(pfile, file_op, cms_info, cms_domain_offset);
    }

close_file:
    file_op->release(pfile);
    pfile = 0;
    return ret;
}
EXPORT_SYMBOL(get_image_cms_file_info);

int pkicms_get_enable_by_image_id(u32 image_id, int *flag)
{
    if (image_id == ABL_PATCH_IMG_ID) {
        *flag = PKICMS_SEC_CHECK_ENABLE;
        return 0;
    }
    return pkicms_get_sec_check_enable_flag(flag);
}

static int pkicms_verify_cms_init(const char *filename, cms_file_t *cms_info, verify_info_st *verify_info, u32 image_id)
{
    int ret;

    if (image_id >= TYPE_INI_MAX) {
        pkicms_err("image_id is invalid, image_id = %d, TYPE_INI_MAX = %d\n", image_id, TYPE_INI_MAX);
        return -EINVAL;
    }

    verify_info->file_name = filename;
    verify_info->efuse_flag = PKICMS_SEC_CHECK_ENABLE;
    verify_info->image_id = image_id;
    verify_info->ini_type = TYPE_INI_MAX;

    ret = alloc_cms_file(cms_info);
    if (ret != 0) {
        pkicms_err("init cms file failed. ret = %d\n", ret);
        return -EIO;
    }

    verify_info->ini_type = verify_info->image_id;

    ret = pkicms_get_enable_by_image_id(image_id, &verify_info->efuse_flag);
    if (ret != 0) {
        pkicms_err("get check_enable flag failed, ret = %d\n", ret);
        return -EIO;
    }

    return 0;
}

STATIC int pkicms_get_cms_info(verify_info_st *verify_info, cms_file_t *cms_info,
    image_fop_t *op, u8 *crl, u32 crl_size)
{
    int ret;

    ret = get_image_cms_file_info(verify_info, verify_info->file_name, cms_info, op);
    if (ret != 0) {
        pkicms_err("get cms info failed. ret = %d\n", ret);
        return -EIO;
    }

    // upgrade crl
    if (crl != NULL) {
        if (crl_size > CMS_CRL_FILE_SIZE_16K) {
            pkicms_err("invalid filename, crl_size more than 16K\n");
            return -EINVAL;
        }
        cms_info->signature.crl = crl;
        cms_info->signature.crl_size = crl_size;
    }
    return 0;
}

STATIC u32 pkicms_check_hash(verify_info_st *verify_info, cms_file_t *cms_info,
    u8 *hashcode, u32 hash_len, image_fop_t *op)
{
    int ret;
    unsigned long code_len = cms_info->header->code_len;

    if (cms_info->header->code_len == 0) {
        code_len = cms_info->header->code_len_extend;
    }

    ret = pkicms_calc_digest(verify_info->file_name, cms_info->header->code_offset, code_len,
        hashcode, hash_len, op);
    if (ret != 0) {
        pkicms_err("calc file digest failed\n");
        return CMS_VERIFY_FAILED;
    }

    if (verify_info->efuse_flag == PKICMS_SEC_CHECK_DISABLE) {
        return ((memcmp(hashcode, cms_info->header->src_hash, SHA256_LEN) == 0) ? 0 : CMS_VERIFY_FAILED);
    }
    return pkicms_check_ini_hash(hashcode, SHA256_LEN, verify_info->ini_type);
}

STATIC u32 pkicms_verify_img_ver_head(verify_info_st *verify_info, image_fop_t *img_op)
{
    int ret;
    CMSCBB_ERROR_CODE check_state;
    img_ver_header img_ver = {0};
    u32 hw_l2_nv_cnt = 0;

    ret = get_image_var_info(verify_info, verify_info->file_name, &img_ver, img_op);
    if (ret != 0) {
        pkicms_err("get file var info failed, ret = %d\n", ret);
        return CMS_VERIFY_FAILED;
    }

    if (verify_info->image_id != ABL_PATCH_IMG_ID) {
        ret = pkicms_nvcnt_read(0, &hw_l2_nv_cnt);
        if (ret != 0) {
            pkicms_err("pkicms_nvcnt_read fail, ret = %d\n", ret);
            return CMS_VERIFY_FAILED;
        }
    }

    check_state =
        pkicms_ver_check(VERIFY_TYPE_CMS, verify_info->image_id, (const img_ver_header *)&img_ver, hw_l2_nv_cnt);
    if (check_state != CMS_VERIFY_SUCCESS) {
        pkicms_err("pkicms_ver_check failed! img_nvcnt(%d):efuse_nvcnt(0x%x).\n", img_ver.nv_cnt, hw_l2_nv_cnt);
        return CMS_VERIFY_FAILED;
    }

    pkicms_info("pkicms_ver_check succeed. img_nvcnt(%d):efuse_nvcnt(0x%x).\n", img_ver.nv_cnt, hw_l2_nv_cnt);

    return CMS_VERIFY_SUCCESS;
}

/**
 * brief: Determine whether hash verification is required.
 * note: read dm verity flag from cmdline, if find "dmverityFlag=true" return false,  else return true.
 * return: true - check hash,  false-dont check hash.
 **/
STATIC bool is_check_hash(image_fop_t *op)
{
/* Only in the MDC and user mode, need to parse cmdline */
#if defined(CFG_SOC_PLATFORM_MDC_V51) && !defined(LINUX_KERNEL_BUILD)
#define MAX_BUFF_LEN (4096)
#define MAX_READ_LEN (MAX_BUFF_LEN - 1)
    uintptr_t pfile;
    char *buf = NULL;
    char *pos = NULL;
    int len = 0;
    int ret;

    if (op == NULL) {
        pkicms_err("op is NULL.\n");
        return true;
    }

    pfile = op->open("/proc/cmdline");
    if (pfile == 0) {
        pkicms_err("error occured while open file.\n");
        return true;
    }

    buf = (char *)malloc(MAX_BUFF_LEN);
    if (buf == NULL) {
        pkicms_err("malloc fail.\n");
        op->release(pfile);
        return true;
    }

    ret = op->read(pfile, buf, MAX_READ_LEN, &len);
    if (ret == READ_ERROR) {
        pkicms_err("read fail.\n");
        free(buf);
        op->release(pfile);
        return true;
    }

    pos = strstr(buf, "dmverityFlag=true");
    if (pos == NULL) {
        pkicms_info("dm verity disable, need to check img hash.\n");
        printf("dm verity disable, need to check img hash.\n");
        free(buf);
        op->release(pfile);
        return true;
    } else {
        pkicms_info("dm verity enable, no need to check img hash.\n");
        printf("dm verity enable, no need to check img hash.\n");
        free(buf);
        op->release(pfile);
        return false;
    }
#else
    return true;
#endif
}
STATIC int pkicms_read_file_buf(const char *path, u32 offset, u8 *buf, u32 len, image_fop_t *img_op)
{
    uintptr_t pfile;
    int read_flag;
    loff_t seek_ret;
    int read_len = 0;
    int ret;

    if (path == NULL || buf == NULL || img_op == NULL) {
        pkicms_err("null param. (path=%d; buf=%d; img_op=%d)\n", path != NULL, buf != NULL, img_op != NULL);
        return -EINVAL;
    }

    pfile = img_op->open(path);
    if (pfile == 0) {
        pkicms_err("error occured while open file.\n");
        return -EIO;
    }
    seek_ret = img_op->lseek(pfile, offset, SEEK_SET);
    if (seek_ret < 0) {
        pkicms_err("seek file failed. maybe file format error, ret = %lld.\n", seek_ret);
        ret = -EINVAL;
        goto close_file;
    }

    read_flag = img_op->read(pfile, (char *)buf, len, &read_len);
    if ((read_flag == READ_ERROR) || (read_len != len)) {
        pkicms_err("read file failed. (read_flag=%d;pos=%lld;read_len=%d).\n",
            read_flag, img_op->cur_offset(pfile), read_len);
        ret = -EINVAL;
        goto close_file;
    }
    ret = 0;
close_file:
    img_op->release(pfile);
    return ret;
}

STATIC u32 pkicms_strip_file(verify_info_st *verify_info, cms_file_t *cms_info, image_fop_t *img_op)
{
    u8 *file_buf = NULL;
    u32 offset;
    u32 len;

    if (verify_info == NULL || cms_info == NULL || img_op == NULL || cms_info->header == NULL) {
        pkicms_err("null param. (verify_info=%d; cms_info=%d; img_op=%d)\n",
            verify_info != NULL, cms_info != NULL, img_op != NULL);
        return CMS_VERIFY_FAILED;
    }
    if (verify_info->image_id != ABL_PATCH_IMG_ID) {
        return CMS_VERIFY_SUCCESS;
    }
    offset = cms_info->header->code_offset;
    len = cms_info->header->code_len;
    if (len <= SECOND_HEAD_SIZE || offset > ABL_PATCH_MAX_LEN || len > ABL_PATCH_MAX_LEN) {
        pkicms_err("len offset invalid. (len=%u; offset=%u)\n", len, offset);
        return CMS_VERIFY_FAILED;
    }
    offset += SECOND_HEAD_SIZE;
    len -= SECOND_HEAD_SIZE;
    file_buf = (u8 *)vmalloc(len);
    if (file_buf == NULL) {
        pkicms_err("malloc fail. (len=%u)\n", len);
        return CMS_VERIFY_FAILED;
    }
    if (pkicms_read_file_buf(verify_info->file_name, offset, file_buf, len, img_op) != 0) {
        vfree(file_buf);
        return CMS_VERIFY_FAILED;
    }
    if (pkicms_write_file_buf(verify_info->file_name, file_buf, len) != 0) {
        vfree(file_buf);
        return CMS_VERIFY_FAILED;
    }
    vfree(file_buf);
    return CMS_VERIFY_SUCCESS;
}

STATIC u32 pkicms_verify_cms_inner(u32 image_id, const char *filename, crl_info_st *crl_info, image_fop_t *op)
{
    int ret;
    CMSCBB_ERROR_CODE check_state = CMS_VERIFY_FAILED;
    cms_file_t cms_info = {0};
    image_fop_t local_op = {0};
    verify_info_st verify_info = {0};
    u8 hashcode[SHA256_BYTE_LEN] = {0};
    image_fop_t *img_op = NULL;

    if (op == NULL) {
        (void)set_local_file_operations(&local_op);
        img_op = &local_op;
    } else {
        img_op = op;
    }

    ret = pkicms_verify_cms_init(filename, &cms_info, &verify_info, image_id);
    if (ret != 0) {
        pkicms_err("initialization failed, ret = %d\n", ret);
        goto end;
    }

    ret = pkicms_get_cms_info(&verify_info, &cms_info, img_op, crl_info->crl, crl_info->crl_size);
    if (ret != 0) {
        pkicms_err("get file cms information failed\n");
        goto end;
    }

    /* pss alg need salt length when verify signature. set sign alg from image header */
    if (pkicms_get_rsa_sign_alg_type() == PKICMS_RSA_PSS) {
        pkicms_set_rsa_sign_alg(cms_info.header->sign_alg);
    }

    if (verify_info.efuse_flag == PKICMS_SEC_CHECK_ENABLE) {
        check_state = pkicms_cms_verify(&cms_info.signature, verify_info.ini_type);
        if (check_state != CMS_VERIFY_SUCCESS) {
            pkicms_err("cms_src_crl data verify failed! state = 0x%x\n", check_state);
            goto end;
        }

        check_state = pkicms_verify_img_ver_head(&verify_info, img_op);
        if (check_state != CMS_VERIFY_SUCCESS) {
            pkicms_err("pkicms_verify_nvcnt failed\n");
            goto end;
        }
    } else {
        check_state = CMS_VERIFY_SUCCESS;
        pkicms_debug("unsecurity booting\n");
    }

    if (is_check_hash(img_op)) {
        check_state = pkicms_check_hash(&verify_info, &cms_info, hashcode, SHA256_BYTE_LEN, img_op);
        if (check_state != CVB_SUCCESS) {
            pkicms_err("ini_hash_check failed! check state = 0x%x\n", check_state);
            goto end;
        }
    }
    check_state = pkicms_strip_file(&verify_info, &cms_info, img_op);

end:
    pkicms_reset_ini_hash(verify_info.ini_type);
    (void)free_cms_file(&cms_info);
    return check_state;
}

#ifndef LINUX_KERNEL_BUILD
STATIC void add_pid_to_sec_drv(void)
{
#ifdef CFG_FEATURE_UADK
    int ret;
    struct uadk_certified_info pid_info = {0};

    pid_info.pid = getpid();
    ret = uadk_config_certified_info(UADK_CFG_ADD_PID_CMD, &pid_info);
    if (ret != 0) {
        printf("[pkicms_tool]fail to add uadk pid.\n");
    }
#endif  // end of CFG_FEATURE_UADK
}
#endif  // end of LINUX_KERNEL_BUILD

#ifndef LINUX_KERNEL_BUILD
STATIC void del_pid_from_sec_drv(void)
{
#ifdef CFG_FEATURE_UADK
    int ret;
    struct uadk_certified_info pid_info = {0};

    pid_info.pid = getpid();
    ret = uadk_config_certified_info(UADK_CFG_DEL_PID_CMD, &pid_info);
    if (ret != 0) {
        printf("[pkicms_tool]fail to del uadk pid.\n");
    }
#endif  // end of CFG_FEATURE_UADK
}
#endif  // end of LINUX_KERNEL_BUILD

u32 pkicms_verify_cms(u32 image_id, const char *filename, crl_info_st *crl_info, image_fop_t *op)
{
    u32 ret;
    int efuse_flag;

    ret = pkicms_get_enable_by_image_id(image_id, &efuse_flag);
    if (ret != 0) {
        pkicms_err("get check_enable flag failed, ret = %d\n", ret);
        return (u32)-EIO;
    }

#ifndef LINUX_KERNEL_BUILD
    add_pid_to_sec_drv();
#endif
#ifndef CFG_SOC_PLATFORM_MDC_V11
    if (efuse_flag == PKICMS_SEC_CHECK_ENABLE && image_id != ABL_PATCH_IMG_ID) {
        pkicms_set_rsa_sign_alg(PKICMS_RSA_PKCS_MASK);
        ret = pkicms_verify_cms_inner(image_id, filename, crl_info, op);
        if (ret == CMS_VERIFY_SUCCESS) {
            pkicms_info("pkicms verify success\n");
#ifndef LINUX_KERNEL_BUILD
            del_pid_from_sec_drv();
#endif
            return ret;
        }
    }

    pkicms_debug("pkicms verify with pkcs alg no pass(%u). try to verify with pss alg\n", ret);
#endif
    pkicms_set_rsa_sign_alg(PKICMS_RSA_PSS_MASK);
    ret = pkicms_verify_cms_inner(image_id, filename, crl_info, op);
#ifndef LINUX_KERNEL_BUILD
    del_pid_from_sec_drv();
#endif
    return ret;
}
EXPORT_SYMBOL(pkicms_verify_cms);

#define PKICMS_SIGN_ALG_TAG "\52\206\110\206\367\15\1\1"
#define PKICMS_SIGN_ALG_TAG_SIZE (8U)
#define PKICMS_SIGN_ALG_RSA '\1'
#define PKICMS_SIGN_ALG_RSASSAPSS '\12'
#define PKICMS_SIGN_ALG_UNKOWN '\377'
#define PKICMS_CLR_SIGN_ALG_POSITION (4U)
#define PKICMS_CLR_SIGN_ALG_DATA_MAX_LEN (28U)

STATIC unsigned int pkicms_get_sign_type_from_crl(const char *crl_data, u32 data_size)
{
    unsigned char crl_sign_data[PKICMS_CLR_SIGN_ALG_DATA_MAX_LEN + 1];
    const char *sign_data_ptr;

    if (data_size < PKICMS_CLR_SIGN_ALG_POSITION + PKICMS_CLR_SIGN_ALG_DATA_MAX_LEN) {
        pkicms_err("copy crl sign data failed. crl data is too short.\n");
        return (unsigned int)PKICMS_SIGN_ALG_UNKOWN;
    }

    if (memcpy_s(crl_sign_data,
        sizeof(crl_sign_data),
        crl_data + PKICMS_CLR_SIGN_ALG_POSITION,
        PKICMS_CLR_SIGN_ALG_DATA_MAX_LEN) != 0) {
        pkicms_err("copy crl sign data failed.\n");
        return (unsigned int)PKICMS_SIGN_ALG_UNKOWN;
    }
    crl_sign_data[PKICMS_CLR_SIGN_ALG_DATA_MAX_LEN] = '\0';

    sign_data_ptr = strstr((const char *)crl_sign_data, PKICMS_SIGN_ALG_TAG);
    if (sign_data_ptr == NULL) {
        pkicms_err("get crl sign data failed.\n");
        return (unsigned int)PKICMS_SIGN_ALG_UNKOWN;
    }

    pkicms_debug("crl sign type: %x\n", sign_data_ptr[PKICMS_SIGN_ALG_TAG_SIZE]);
    return *(sign_data_ptr + PKICMS_SIGN_ALG_TAG_SIZE);
}

int pkicms_compare_crl(const char *crl_data1, u32 data1_size, const char *crl_data2, u32 data2_size, int *stat)
{
    CMSCBB_ERROR_CODE ret;
    crl_cmp_info_t cmp_info = {0};

    if (crl_data1 == NULL || crl_data2 == NULL || stat == NULL) {
        pkicms_err("The input parameter of the function contains a null pointer.\n");
        return -EINVAL;
    }

    cmp_info.crl_data1 = (unsigned char *)crl_data1;
    cmp_info.crl_size1 = data1_size;
    cmp_info.crl_data2 = (unsigned char *)crl_data2;
    cmp_info.crl_size2 = data2_size;

    if (pkicms_get_sign_type_from_crl(crl_data1, data1_size) == PKICMS_SIGN_ALG_RSASSAPSS) {
        pkicms_debug("local crl is pss!!!\n");
        pkicms_set_rsa_sign_alg(PKICMS_RSA_PSS_MASK);
    } else {
        pkicms_debug("local crl is pkcs!!!\n");
        pkicms_set_rsa_sign_alg(PKICMS_RSA_PKCS_MASK);
    }

    ret = pkicms_hw_crl_compare(&cmp_info, stat);
    if (ret != 0) {
        pkicms_err("pkicms_hw_crl_compare fail, ret=%d.\n", ret);
    }

    return ret;
}
EXPORT_SYMBOL(pkicms_compare_crl);

#if (defined CFG_SOC_PLATFORM_MDC_V51)
#ifdef LINUX_KERNEL_BUILD
int get_image_cms_file_info_with_offset(const char *filename, cms_file_t *cms_info, image_fop_t *file_op, u32 offset)
{
    uintptr_t pfile;
    int ret = 0;
    int read_flag;
    u8 *read_buff = NULL;
    int read_len = 0;

    pkicms_info("offset=%d.\n", offset);

    pfile = file_op->open(filename);
    if (pfile == 0) {
        pkicms_err("error occured while open file.\n");
        return -EIO;
    }

    read_buff = (u8 *)cms_info->header;

    if (file_op->lseek(pfile, offset, SEEK_SET) < 0) {
        ret = -EIO;
        pkicms_err("seek file failed. can not point to hw signature head.\n");
        goto close_file;
    }
    read_flag = file_op->read(pfile, (char *)read_buff, ESBC_HEADER_SIZE, &read_len);
    if ((read_flag == READ_ERROR) || (read_len < (int)ESBC_HEADER_SIZE)) {
        ret = -EIO;
        pkicms_err("read_flag=%d, read_len=%d, read_pos=%lld.\n", read_flag, read_len, file_op->cur_offset(pfile));
        goto close_file;
    }

close_file:
    file_op->release(pfile);
    pfile = 0;
    return ret;
}

int pkicms_get_first_sign_addr(
    second_sign_t *image_input, char *read_buff, struct file *pfile, int *code_offset, unsigned int *first_len)
{
    int ret;
    cms_file_t cms_info = {0};
    image_fop_t local_op = {0}, *img_op = NULL;
    verify_info_st verify_info = {0};
    img_ver_header *image_position = NULL;

    (void)set_local_file_operations(&local_op);
    img_op = &local_op;

    ret = pkicms_verify_cms_init(image_input->file_name, &cms_info, &verify_info, (u32)image_input->image_id);
    if (ret != 0) {
        pkicms_err("get first sign initialization failed, ret = %d\n", ret);
        goto end;
    }
    // get first sign head
    ret = get_image_cms_file_info_with_offset(
        image_input->file_name, &cms_info, img_op, (SECURE_HEAD_SIZE + SECOND_HEAD_SIZE));
    if (ret != 0) {
        pkicms_err("get first sign head failed\n");
        goto end;  // not second sign, continue to write ufs
    }

    if (cms_info.header->head_magic != BOOT_MAGIC_CODE) {  // is it 2 times sign?
        pkicms_info("not second sign, head_magic:0x%x.\n", cms_info.header->head_magic);
        goto end;
    }
    image_position = (img_ver_header *)(read_buff + GENERAL_IMAGE_OFFSET);

    ret = pkicms_check_img_tag(VERIFY_TYPE_CMS, (u32)image_input->image_id, image_position);
    if (ret != 0) {
        pkicms_info("pkicms img_tag is not exist , ret=%d.\n", ret);
        goto end;
    }

    *code_offset = SECURE_HEAD_SIZE + SECOND_HEAD_SIZE;
    *first_len = cms_info.header->img_len;

end:
    (void)free_cms_file(&cms_info);
    return 0;
}
EXPORT_SYMBOL(pkicms_get_first_sign_addr);

#else

static int pkicms_get_cms_offset(uintptr_t pfile, image_fop_t *file_op, loff_t *offset, u32 ini_type)
{
    int ret;
    int format;
    esbc_header_t *header = NULL;

    if (file_op == NULL || offset == NULL) {
        pkicms_err("file_op or offset is NULL pointer.\n");
        return -EINVAL;
    }

    header = (esbc_header_t *)vzalloc(ESBC_HEADER_SIZE);
    if (header == NULL) {
        pkicms_err("alloc memory failed\n");
        return -EFAULT;
    }

    ret = get_image_format(ini_type, &format);
    if (ret != 0) {
        pkicms_err("get image format failed, ini_type = %d is illegal\n", ini_type);
        goto free_buff;
    }

    ret = get_cms_offset(pfile, file_op, header, format, offset);
    if (ret != 0) {
        pkicms_err("get cms offset failed, format = %d\n", format);
        goto free_buff;
    }

free_buff:
    if (header != NULL) {
        (void)vfree(header);
    }
    return ret;
}

const char g_boot_hash_tag[] = "dm-roothash";
STATIC int get_root_hash(u8 *ini_buff, char *roothash, int *roothashlen)
{
    char *p = NULL;
    char *root_hash = NULL;
    int ret;
    p = strstr((const char *)ini_buff, g_boot_hash_tag);
    if (p == NULL) {
        pkicms_err("dont match boot hash tag.\n");
        return -EFAULT;
    }

    root_hash = p + strlen(g_boot_hash_tag) + INT_TAG_SUFFIX_LEN;  // 跳过“;”
    ret = memcpy_s(roothash, *roothashlen, root_hash, SHA256_LEN * HEX_TO_STRING_COFF);
    if (ret != 0) {
        pkicms_err("memcpy_s hash error, ret = %d!\n", ret);
        return ret;
    }
    *roothashlen = SHA256_LEN * HEX_TO_STRING_COFF;
    return 0;
}

STATIC int try_get_root_hash(uintptr_t pfile, image_fop_t *file_op, loff_t cms_offset, char *roothash, int *roothashlen)
{
    int ret;
    int i;
    int read_flag, read_len;
    loff_t seek_ret, ini_offset, sign_offset;
    u8 *ini_buff = NULL;
    pkicms_sign_alg alg_type[] = {PKICMS_RSA_PKCS1, PKICMS_RSA_PSS};
    int alg_size = sizeof(alg_type) / sizeof(pkicms_sign_alg);

    ini_buff = (u8 *)vzalloc(CMS_INI_FILE_SIZE_2K * sizeof(u8));
    if (ini_buff == NULL) {
        pkicms_err("alloc memory failed\n");
        return -EFAULT;
    }

    for (i = 0; i < alg_size; ++i) {
        ini_offset = cms_offset + pkicms_get_cms_file_size(alg_type[i]);
        sign_offset = ini_offset + CMS_TAG_AND_LEN_SIZE;
        pkicms_warn("try get hash, sign_offset = %lld, try alg = %d.\n", sign_offset, alg_type[i]);
        seek_ret = file_op->lseek(pfile, sign_offset, SEEK_SET);
        if (seek_ret < 0) {
            pkicms_warn("seek file. ret = %d, sign_offset = %lld, try alg = %d.\n", seek_ret, sign_offset, alg_type[i]);
            continue;
        }

        read_flag = file_op->read(pfile, (char *)ini_buff, CMS_INI_FILE_SIZE_2K, &read_len);
        if ((read_flag == READ_ERROR) || (read_len < CMS_INI_FILE_SIZE_2K)) {
            pkicms_warn("read file. read pos=%lld, try alg = %d.\n", file_op->cur_offset(pfile), alg_type[i]);
            continue;
        }

        // ini_buff结尾都是0，当做字符串用
        ret = get_root_hash(ini_buff, roothash, roothashlen);
        if (ret == 0) {
            goto end;
        }
    }

    pkicms_err("try all sign alg fail!\n");
    ret = -EFAULT;
end:
    if (ini_buff != NULL) {
        vfree(ini_buff);
    }
    return ret;
}

int pkicms_get_root_hash(u32 image_id, const char *filename, char *roothash, int *roothashlen)
{
    int ret;
    image_fop_t file_op = {0};
    uintptr_t pfile;
    loff_t cms_domain_offset;

    if (*roothashlen < SHA256_LEN * HEX_TO_STRING_COFF) {
        pkicms_err("input roothashlen is small, len is %d.\n", *roothashlen);
        return -EFAULT;
    }
    set_local_file_operations(&file_op);
    pfile = file_op.open(filename);
    if (pfile == 0) {
        pkicms_err("error occured while open file.\n");
        return -EIO;
    }

    ret = pkicms_get_cms_offset(pfile, &file_op, &cms_domain_offset, image_id);
    if (ret != 0) {
        pkicms_err("get bootup ini offset fail. ret = %d\n", ret);
        goto close_file;
    }

    // try get bootini by different sign algo: PKICMS_RSA_PKCS1,PKICMS_RSA_PSS
    ret = try_get_root_hash(pfile, &file_op, cms_domain_offset, roothash, roothashlen);
    if (ret != 0) {
        pkicms_err("try to get root hash fail, ret = %d\n", ret);
        goto close_file;
    }
close_file:
    file_op.release(pfile);
    pfile = 0;
    return ret;
}
EXPORT_SYMBOL(pkicms_get_root_hash);
#endif
#endif
