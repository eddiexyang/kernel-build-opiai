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

#include <linux/types.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/securec.h>
#include <linux/errno.h>
#include <linux/version.h>
#include <linux/uaccess.h>

#include "dev_upgrade_crl.h"
#include "dev_upgrade_def.h"
#include "dev_upgrade_ufs.h"
#include "dev_upgrade_adapt.h"
#include "dev_upgrade_public.h"
#include "dev_upgrade_mdccore.h"
#include "pkicms_api.h"
#include "pkicms.h"
#include "drv_pkicms.h"

static int g_new_img_id = IMAGE_ID_MAX;
STATIC loff_t g_ufs_read_offset = 0;
STATIC img_crl_info_t *g_img_crl_info = NULL;
static unsigned char *g_local_crl_info = NULL;

STATIC int dev_upgrade_read_ufs_img_file(unsigned char *buf, unsigned int size, unsigned int read_offset, int flag);
static struct local_image_verify g_local_imgs[IMG_ID_MAX_NUM];

typedef struct __component_img_id_info {
    char *name;
    int img_id;
} component_img_id_info_t;

static component_img_id_info_t g_img_id_info[IMG_ID_MAX_NUM] = {
    {"/itrustee.img", ITEE_IMG_ID},
    {"/dt.img", DTB_IMG_ID},
    {"/Image", ZIMAGE_ID},
    {"/filesystem-le.cpio.gz", FS_IMG_ID},
    {"/sd-pek-dt.img", SD_PEK_DTB_IMG_ID},
    {"/sd.img", SD_IMG_ID},
    {"/dp.img", DP_IMG_ID},
    {"/sysDrv_rootfs.ext4", ROOTFS_IMG_ID},
    {"/app.img", APP_IMG_ID},
    {"/dt-dp-pek.img", DTB_DP_PEK_IMG_ID},
    {"/dt-sd-pek.img", DTB_SD_PEK_IMG_ID},
#ifdef  CFG_SOC_PLATFORM_MDC_V11
    {"/dt-pek.img", DTB_SD_PEK_IMG_ID},
    {"/pek.img", SD_PEK_IMG_ID},
#else
    {"/pek.img", PEK_IMG_ID},
#endif
    {"/dp-pek.img", DP_PEK_IMG_ID},
    {"/sd-pek.img", SD_PEK_IMG_ID},
    {"/dp-core.img", DP_CORE_IMG_ID},
};

struct local_image_verify *dev_upgrade_get_local_imgs(void)
{
    return g_local_imgs;
}

/*
 * Prototype    : dev_upgrade_ufs_open
 * Description  : stub for ufs open
 * Params
 *   [IN] file_name: open file name
 * Return Value : always return 1
 */
uintptr_t dev_upgrade_crl_storage_open(const char *file_name)
{
    dev_upgrade_info("dev_upgrade_crl_storage_open.\n");
    return 1;
}

/*
 * Prototype    : dev_upgrade_ufs_read
 * Description  : read data from ufs
 * Params
 *   [IN] fp: not use
 *   [IN] buf: buffer for read ufs
 *   [IN] size: buffer size
 *   [OUT] read_size: actual read size
 * Return Value : size of read, fail for negative or zero.
 */
int dev_upgrade_crl_storage_read(uintptr_t fp, char *buf, unsigned int size, int *read_size)
{
    int ret;
    int read_status;
    unsigned int read_len;

    if (buf == NULL) {
        dev_upgrade_err("buf is null.\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    if (read_size == NULL) {
        dev_upgrade_err("read_size is null.\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    if (g_img_crl_info == NULL) {
        dev_upgrade_err("image crl info is null, can't read ufs data.\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    if (((unsigned int)g_ufs_read_offset + size) > g_img_crl_info->ufs_img_size) {
        dev_upgrade_err("read ufs offset if out of range, size=0x%llx, offset=0x%x.\n", g_ufs_read_offset, size);
        return READ_ERROR;
    }

    if (((unsigned int)g_ufs_read_offset + size) < g_img_crl_info->ufs_img_size) {
        read_len = size;
        read_status = READ_CONTINUE;
    } else {
        read_len = g_img_crl_info->ufs_img_size - (unsigned int)g_ufs_read_offset;
        read_status = READ_END;
    }

    ret = memcpy_s(buf, size, g_img_crl_info->ufs_img_data + g_ufs_read_offset, size);
    if (ret != 0) {
        dev_upgrade_err("memcpy_s fail, ret=%d.\n", ret);
        return READ_ERROR;
    }

    *read_size = read_len;
    g_ufs_read_offset += read_len;

    return read_status;
}

/*
 * Prototype    : dev_upgrade_ufs_lseek
 * Description  : set for read offset
 * Params
 *   [IN] fp: not use
 *   [IN] offset: set read offset.
 *   [IN] whence: not use
 * Return Value : offset for read ufs
 */
loff_t dev_upgrade_crl_storage_lseek(uintptr_t fp, loff_t offset, int whence)
{
    if (whence == SEEK_SET) {
        g_ufs_read_offset = offset;
    } else {
        dev_upgrade_err("not support for other whence(%d).\n", whence);
        return -1;
    }

    dev_upgrade_info("dev_upgrade_crl_storage_lseek:0x%llx.\n", g_ufs_read_offset);
    return offset;
}

/*
 * Prototype    : dev_upgrade_ufs_get_cur_offset
 * Description  : get current read offset
 * Params
 *   [IN] fp: not use
 * Return Value : offset for read ufs
 */
loff_t dev_upgrade_crl_get_cur_offset(uintptr_t fp)
{
    dev_upgrade_info("dev_upgrade_crl_get_cur_offset:0x%llx.\n", g_ufs_read_offset);
    return g_ufs_read_offset;
}

/*
 * Prototype    : dev_upgrade_crl_storage_release
 * Description  : release resource
 * Params
 *   [IN] fp: not use
 * Return Value : 0 for success, others for fail
 */
void dev_upgrade_crl_storage_release(uintptr_t fp)
{
    g_ufs_read_offset = 0;
    dev_upgrade_info("dev_upgrade_crl_storage_release.\n");
}

/*
 * Prototype    : dev_upgrade_cmp_crl_file
 * Description  : compare crl file's timestamp
 * Params
 *   [IN] crl_info: crl information for compare
 *   [OUT] stat: compare result
 * Return Value : 0 for success, others for fail
 */
STATIC int dev_upgrade_cmp_crl_file(crl_info_t *crl_info, int *stat)
{
    return pkicms_compare_crl((const char *)crl_info->new_crl_data, crl_info->new_crl_size,
        (const char *)crl_info->old_crl_data, crl_info->old_crl_size, stat);
}

/*
 * Prototype    : dev_upgrade_get_cms_file_info
 * Description  : to judge if crl file exist in ufs
 * Params
 *   [IN] img_buf: buffer for image file
 *   [IN] in_size: size of image buffer
 *   [OUT] crl_buf: buffer for output crl file
 *   [OUT] out_size: size of crl buffer
 * Return Value : 0 for success, others for fail
 */
STATIC int dev_upgrade_get_cms_file_info(crl_info_t *crl_info, hw_type_cms_struct *cms_file_info)
{
    esbc_header_t *esbcHead = NULL;
    unsigned int align;
    unsigned long long cms_tag;
    unsigned long long len;
    pkicms_sign_alg sign_alg;
    u32 cms_size;
    cms_tag = *(unsigned long long*)(crl_info->img_data + CMS_TAG_OFFSET);
    if (cms_tag != CMS_TAG) {
        dev_upgrade_err("this image file not has cms crl\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    /* The CMS size in the PSS signature is different from that in the PKCS1.
     * The CMS size needs to be determined based on the signature type. */
    esbcHead = (esbc_header_t *)(uintptr_t)crl_info->img_data;
    sign_alg = pkicms_get_alg_type_by_sign_data(esbcHead->sign_alg);
    cms_size = pkicms_get_cms_file_size(sign_alg);
    dev_upgrade_info("sign_alg=%x, sign_alg=%u, cms_size=%u\n", esbcHead->sign_alg, sign_alg, cms_size);
    align = CMS_DATA_RESERVE_GAP - (esbcHead->code_len % CMS_DATA_ALIGN);  // for cms 16 byte align

    len = SECURE_HDR_SIZE + esbcHead->code_len + align + cms_size + CMS_INI_FILE_SIZE_2K + CMS_TAG_AND_LEN_SIZE;
    if (len >= crl_info->img_data_size) {
        dev_upgrade_err("crl locat offset(%lld) over img_data_size(%d).\n", len, crl_info->img_data_size);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_LENGTH);
    }

    cms_file_info->cms_header = (unsigned char *)(uintptr_t)(crl_info->img_data + SECURE_HDR_SIZE +
                                esbcHead->code_len + align);
    cms_file_info->cms = cms_file_info->cms_header + CMS_TAG_AND_LEN_SIZE;
    cms_file_info->cms_size = *(unsigned int *)(uintptr_t)(cms_file_info->cms_header + CMS_NAME_DOMAIN_LEN);

    cms_file_info->src_header = cms_file_info->cms_header + cms_size;
    cms_file_info->src = cms_file_info->src_header + CMS_TAG_AND_LEN_SIZE;
    cms_file_info->src_size = *(unsigned int *)(uintptr_t)(cms_file_info->src_header + CMS_NAME_DOMAIN_LEN);

    cms_file_info->crl_header = cms_file_info->src_header + CMS_INI_FILE_SIZE_2K;
    cms_file_info->crl = cms_file_info->crl_header + CMS_TAG_AND_LEN_SIZE;
    cms_file_info->crl_size = *(unsigned int *)(uintptr_t)(cms_file_info->crl_header + CMS_NAME_DOMAIN_LEN);

    len += cms_file_info->crl_size;
    if (len > crl_info->img_data_size) {
        dev_upgrade_err("total len(%lld) over img_data_size(%d).\n", len, crl_info->img_data_size);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_LENGTH);
    }

    if ((cms_file_info->cms_size > (cms_size - CMS_TAG_AND_LEN_SIZE)) || (cms_file_info->cms_size == 0) ||
        (cms_file_info->src_size > (CMS_INI_FILE_SIZE_2K - CMS_TAG_AND_LEN_SIZE)) || (cms_file_info->src_size == 0) ||
        (cms_file_info->crl_size > (CMS_CRL_FILE_SIZE_MAX - CMS_TAG_AND_LEN_SIZE)) || (cms_file_info->crl_size == 0)) {
        dev_upgrade_err("cms[0x%x] or ini[0x%x] or crl[0x%x] size not valid.\n",
            cms_file_info->cms_size, cms_file_info->src_size, cms_file_info->crl_size);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_LENGTH);
    }

    return 0;
}

/*
 * Prototype    : dev_upgrade_check_crl_file_validity
 * Description  : compare crl file's timestamp
 * Params
 *   [IN] buf: crl buffer
 *   [IN] size: buffer size
 *   [OUT] valid: validity result
 * Return Value : 0 for success, others for fail
 */
STATIC int dev_upgrade_check_crl_file_validity(unsigned char *buf, unsigned int size, int *valid)
{
    int ret;
    crl_info_t crl_info_tmp = {0};
    int stat = 0;

    crl_info_tmp.new_crl_data = buf;
    crl_info_tmp.new_crl_size = size;
    crl_info_tmp.old_crl_data = crl_info_tmp.new_crl_data;
    crl_info_tmp.old_crl_size = crl_info_tmp.new_crl_size;

    ret = dev_upgrade_cmp_crl_file(&crl_info_tmp, &stat);
    if (ret != 0) {
        dev_upgrade_err("dev_upgrade_cmp_crl_file failed, ret=%d.\n", ret);
        return ret;
    }

    dev_upgrade_info("compare crl file result: 0x%x.\n", stat);
    if (stat == CRL_CMP_SAME) {
        *valid = CRL_FILE_VALID;
        return 0;
    }

    *valid = CRL_FILE_NOT_VALID;
    return 0;
}

/*
 * Prototype    : dev_upgrade_verify_image_by_crl
 * Description  : use crl file to check if image is valid or not
 * Params
 *   [IN] img_type: image type, file or ufs
 *   [IN] file_name: image file path
 *   [IN] crl_data: buffer for crl file
 *   [IN] crl_size: size of crl file
 * Return Value : 0 for success, others for fail
 */
STATIC int dev_upgrade_verify_image_by_crl(int img_type, const char *file_name,
                                           unsigned char *crl_data, unsigned int crl_size)
{
    int ret;
    image_fop_t image_fops = {0};
    image_fop_t *fops = NULL;
    crl_info_st crl_info;

    if (img_type == IMAGE_TYPE_UFS) {
        image_fops.open = dev_upgrade_crl_storage_open;
        image_fops.read = dev_upgrade_crl_storage_read;
        image_fops.lseek = dev_upgrade_crl_storage_lseek;
        image_fops.release = dev_upgrade_crl_storage_release;
        image_fops.cur_offset = dev_upgrade_crl_get_cur_offset;
        fops = &image_fops;
    }

    crl_info.crl = crl_data;
    crl_info.crl_size = crl_size;
    ret = pkicms_verify_cms(ITEE_IMG_ID, file_name, &crl_info, fops);
    if (ret != 0) {
        dev_upgrade_err("pkicms_verify_cms fail, ret=%d.\n", ret);
        return ret;
    }

    return ret;
}

/*
 * Prototype    : dev_upgrade_get_img_exist_status
 * Description  : to judge if image exist in ufs
 * Params
 *   [IN] img_buf: buffer for image
 *   [IN] size: size of buffer
 * Return Value : 0 for success, others for fail
 */
STATIC int dev_upgrade_get_img_exist_status(const unsigned char *img_buf, unsigned int size, int *status)
{
    unsigned int tmp = *(unsigned int *)img_buf;

    if (tmp == IMG_FILE_HEAD) {
        *status = FILE_EXIST;
        return 0;
    }

    *status = FILE_NOT_EXIST;
    return 0;
}

/*
 * Prototype    : dev_upgrade_get_crl_exist_status
 * Description  : to judge if crl file exist in ufs
 * Params
 *   [IN] status: 0 exist, others not exist
 * Return Value : 0 for success, others for fail
 */
STATIC int dev_upgrade_get_crl_exist_status(img_crl_info_t *info)
{
    int ret;
    unsigned int crl_size;
    int valid = CRL_FILE_NOT_VALID;

    if (strcmp((char *)info->ufs_crl_head, CRL_FILE_TAG) == 0) {
        crl_size = *((unsigned int*)(info->ufs_crl_head + CRL_SIZE_OFFSET));
        if (crl_size > CRL_FILE_SIZE_MAX) {
            info->ufs_crl_status = CRL_FILE_NOT_VALID;
            dev_upgrade_info("crl file is not exist in ufs.\n");
            return 0;
        }

        ret = dev_upgrade_check_crl_file_validity(info->ufs_crl_head + CRL_HEAD_SIZE, crl_size, &valid);
        if (ret < 0) {
            dev_upgrade_err("dev_upgrade_check_crl_file_validity failed, ret=%d.\n", ret);
            return ret;
        }

        if (valid == CRL_FILE_VALID) {
            info->ufs_crl_data = info->ufs_crl_head + CRL_HEAD_SIZE;
            info->ufs_crl_size = crl_size;
        }
    }

    info->ufs_crl_status = valid;
    return 0;
}

/*
 * Prototype    : dev_upgrade_check_img_crl_status
 * Description  : to check if image has crl file or not
 * Params
 *   [IN] img_buf: buffer for image file
 *   [IN] in_size: size of image buffer
 *   [OUT] status: file exist or not
 * Return Value : 0 for success, others for fail
 */
STATIC int dev_upgrade_check_img_crl_status(const unsigned char *img_buf, unsigned int in_size, int *status)
{
    unsigned long long cms_tag;

    if (in_size < CMS_TAG_OFFSET) {
        dev_upgrade_warn("image size(%u) is too small.\n", in_size);
        *status = FILE_NOT_EXIST;
        return 0;
    }

    cms_tag = *(unsigned long long*)(img_buf + CMS_TAG_OFFSET);
    if (cms_tag != CMS_TAG) {
        dev_upgrade_warn("this image file not has cms crl\n");
        *status = FILE_NOT_EXIST;
        return 0;
    }

    *status = FILE_EXIST;
    return 0;
}

/*
 * Prototype    : dev_upgrade_get_img_crl_file
 * Description  : to judge if crl file exist in ufs
 * Params
 *   [IN OUT] crl_info: crl information for compare
 * Return Value : 0 for success, others for fail
 */
STATIC int dev_upgrade_get_img_crl_file(crl_info_t *crl_info)
{
    int ret;
    hw_type_cms_struct cms_file_info = {0};

    ret = dev_upgrade_get_cms_file_info(crl_info, &cms_file_info);
    if (ret != 0) {
        dev_upgrade_err("dev_upgrade_get_cms_file_info fail, ret=%d.\n", ret);
        return ret;
    }

    crl_info->new_crl_data = cms_file_info.crl;
    crl_info->new_crl_size = cms_file_info.crl_size;
    return ret;
}

/*
 * Prototype    : dev_upgrade_write_ufs_crl_file
 * Description  : write crl file in ufs
 * Params
 *   [IN] buf: crl file buffer
 *   [IN] size: size of crl file buffer
 * Return Value : 0 for success, others for fail
 */
STATIC int dev_upgrade_write_ufs_crl_file(unsigned char *buf, unsigned int size)
{
    int ret;
    char *write_crl_data = NULL;

    if (size > (CRL_FILE_SIZE_MAX - CRL_HEAD_SIZE)) {
        dev_upgrade_err("write size[%u] is out of range.\n", size);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    write_crl_data = (char *)vzalloc(size + CRL_HEAD_SIZE);
    if (write_crl_data == NULL) {
        dev_upgrade_err("write crl data vzalloc fail.\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    ret = memcpy_s(write_crl_data, size + CRL_HEAD_SIZE, CRL_FILE_TAG, strlen(CRL_FILE_TAG) + 1);
    if (ret != 0) {
        dev_upgrade_err("crl tag memcpy_s fail, ret=%d.\n", ret);
        goto free_alloc_exit;
    }

    *(unsigned int *)(write_crl_data + CRL_SIZE_OFFSET) = size;
    ret = memcpy_s(write_crl_data + CRL_HEAD_SIZE, size, buf, size);
    if (ret != 0) {
        dev_upgrade_err("write rl data memcpy_s fail, ret=%d.\n", ret);
        goto free_alloc_exit;
    }

    ret = dev_upgrade_disk_write(QUERY_FROM_BOOTSTRAP, CRL_FILE_LUN, CRL_FILE_OFFSET,
        write_crl_data, size + CRL_HEAD_SIZE);
    if (ret != 0) {
        dev_upgrade_err("write_block_to_ufs_storage fail, ret=%d.\n", ret);
        goto free_alloc_exit;
    }

free_alloc_exit:
    DEV_UPGRADE_VFREE(write_crl_data);
    return ret;
}

/*
 * Prototype    : dev_upgrade_read_ufs_crl_file
 * Description  : read crl file in ufs
 * Params
 *   [IN] buf: crl file buffer
 *   [IN] size: size of crl file buffer
 * Return Value : 0 for success, others for fail
 */
STATIC int dev_upgrade_read_ufs_crl_file(unsigned char *buf, unsigned int size)
{
    if (size > CRL_FILE_SIZE_MAX) {
        dev_upgrade_err("read size[%u] is out of range.\n", size);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    return dev_upgrade_disk_read(QUERY_FROM_BOOTSTRAP, CRL_FILE_LUN, CRL_FILE_OFFSET, (char *)buf, size);
}

/*
 * Prototype    : dev_upgrade_read_ufs_img_file
 * Description  : read itrustee.bin img file in ufs
 * Params
 *   [IN] buf: img file buffer
 *   [IN] size: size of image file buffer
 *   [IN] flag: update image or update crl file flag
 * Return Value : 0 for success, others for fail
 */
STATIC int dev_upgrade_read_ufs_img_file(unsigned char *buf, unsigned int size, unsigned int read_offset, int flag)
{
    unsigned int i;
    unsigned int count;
    unsigned char lun_wr;
    unsigned long long start_offset;
    struct mdc_file_storage file_storage_info = {0};
    upgrade_ufs_component_info *local_component = NULL;

    local_component = dev_upgrade_ufs_get_local_component(DSMI_COMPONENT_TYPE_RAWDATA);
    if (local_component == NULL) {
        dev_upgrade_err("find component fail, component: %u\n", DSMI_COMPONENT_TYPE_RAWDATA);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_INDEX);
    }

    count = local_component->sub_pkt_m.sub_file_count;
    for (i = 0; i < count; i++) {
        if (strcmp(local_component->sub_pkt_m.sub_file_info[i]->name, ITRUSTEE_IMG_FILE) == 0) {
            file_storage_info = local_component->sub_pkt_m.sub_file_info[i]->storage;
            break;
        }
    }

    if (i == count) {
        dev_upgrade_err("ufs image not found.\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_INDEX);
    }

    if (flag == CRL_PROCESS_TYPE_UPDATE_IMG) {
        if (local_component->cur_flag == UPGRADE_MASTER_AREA) {
            lun_wr = file_storage_info.lun;
            start_offset = file_storage_info.start;
        } else {
            lun_wr = file_storage_info.back_lun;
            start_offset = file_storage_info.back;
        }
    } else {
        if (local_component->cur_flag == UPGRADE_MASTER_AREA) {
            lun_wr = file_storage_info.lun;
            start_offset = file_storage_info.start;
        } else {
            lun_wr = file_storage_info.back_lun;
            start_offset = file_storage_info.back;
        }
    }

    return  dev_upgrade_disk_read(QUERY_FROM_BOOTSTRAP, lun_wr, start_offset + read_offset,
        (char *)buf, size);
}

/*
 * Prototype    : dev_upgrade_read_update_img_data
 * Description  : read update image data
 * Params
 *   [IN] update_img_path: update image path for get image data
 *   [IN] read_size: read data size, 0 means total file size
 *   [IN] read_flag: read data by input size or read by file size
 *   [OUT] info: informatioin for update image and it's crl
 * Return Value : 0 for success, others for fail
 */
STATIC int dev_upgrade_read_update_img_data(const char *update_img_path, int read_flag,
                                            int in_size, img_crl_info_t *info)
{
    int ret;
    struct file *fp = NULL;
    int file_size;
    int read_size = 0;

    fp = open_mdc_package(update_img_path);
    if (IS_ERR_OR_NULL(fp)) {
        dev_upgrade_err("error occured while open file %s.\n", update_img_path);
        ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
        goto RESOURCE_OP_FAIL;
    }

    /* if flag is total, read according to file size */
    file_size = fp->f_inode->i_size;

    /* if flag is by size, read according to input size */
    if (read_flag == READ_IMG_FILE_FLAG_BY_SIZE) {
        if ((in_size != 0) && (in_size < file_size)) {
            file_size = in_size;
        }
    }

    dev_upgrade_debug("open file size: %d.\n", file_size);
    if (file_size > TEE_IMG_SIZE_MAX) {
        dev_upgrade_err("file size(%d) out of range.\n", file_size);
        ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
        goto RESOURCE_OP_FAIL;
    }

    info->update_img_size = file_size;
    info->update_img_data = (unsigned char *)vzalloc(file_size);
    if (info->update_img_data == NULL) {
        dev_upgrade_err("vzalloc fail.\n");
        ret = dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
        goto RESOURCE_OP_FAIL;
    }

    ret = read_mdc_package_content(fp, (char *)info->update_img_data, file_size, &read_size);
    if ((ret != READ_CONTINUE) || (read_size != file_size)) {
        dev_upgrade_err("read_mdc_package_content fail, ret=%d, read_size:%d.\n", ret, read_size);
        goto RESOURCE_OP_FAIL;
    }

    close_mdc_package(fp);
    fp = NULL;
    return OK;

RESOURCE_OP_FAIL:
    if (info->update_img_data != NULL) {
        DEV_UPGRADE_VFREE(info->update_img_data);
    }
    close_mdc_package(fp);
    fp = NULL;
    return ret;
}

/*
 * Prototype    : dev_upgrade_get_update_img_info
 * Description  : initialize image and crl related resource
 * Params
 *   [IN] update_img_path: update image path for get image data
 *   [OUT] info: informatioin for update image and it's crl
 * Return Value : 0 for success, others for fail
 */
STATIC int dev_upgrade_get_update_img_info(const char *update_img_path, img_crl_info_t *info)
{
    int ret;
    int crl_status = FILE_NOT_EXIST;
    crl_info_t crl_info = {0};

    info->update_img_status = IMG_STATUS_NOT_FOUND;
    ret = dev_upgrade_read_update_img_data(update_img_path, READ_IMG_FILE_FLAG_TOTAL, 0, info);
    if (ret != 0) {
        dev_upgrade_err("dev_upgrade_read_update_img_data fail, ret=%d.\n", ret);
        return ret;
    }

    /* check if crl file exist in image */
    ret = dev_upgrade_check_img_crl_status((const unsigned char *)info->update_img_data,
                                           info->update_img_size, &crl_status);
    if (ret != 0) {
        dev_upgrade_err("dev_upgrade_check_img_crl_status fail, ret=%d.\n", ret);
        return ret;
    }

    /* if image crl exist, get crl file */
    if (crl_status == FILE_EXIST) {
        crl_info.img_data = info->update_img_data;
        crl_info.img_data_size = info->update_img_size;
        ret = dev_upgrade_get_img_crl_file(&crl_info);
        if (ret != 0) {
            dev_upgrade_err("dev_upgrade_get_img_crl_file fail, ret=%d.\n", ret);
            return ret;
        }

        info->update_img_crl_data = crl_info.new_crl_data;
        info->update_img_crl_size = crl_info.new_crl_size;
        info->update_img_status = IMG_STATUS_HAVE_CRL;
    } else {
        info->update_img_status = IMG_STATUS_NOT_HAVE_CRL;
    }

    return ret;
}

/*
 * Prototype    : dev_upgrade_get_ufs_img_info
 * Description  : initialize image and crl related resource
 * Params
 *   [OUT] info: informatioin for update image and it's crl
 *   [IN] flag: update image or update crl file flag
 * Return Value : 0 for success, others for fail
 */
STATIC int dev_upgrade_get_ufs_img_info(img_crl_info_t *info, int flag)
{
    int ret;
    int img_status = FILE_NOT_EXIST;
    int crl_status = FILE_NOT_EXIST;
    crl_info_t crl_info = {0};

    info->ufs_img_status = IMG_STATUS_NOT_FOUND;
    info->ufs_img_data = (unsigned char *)vzalloc(TEE_IMG_SIZE_MAX);
    if (info->ufs_img_data == NULL) {
        dev_upgrade_err("vzalloc ufs image data failed.\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    info->ufs_img_size = TEE_IMG_SIZE_MAX;
    ret = dev_upgrade_read_ufs_img_file(info->ufs_img_data, info->ufs_img_size, 0, flag);
    if (ret != 0) {
        dev_upgrade_err("dev_upgrade_read_ufs_img_file fail, ret=%d.\n", ret);
        DEV_UPGRADE_VFREE(info->ufs_img_data);
        return ret;
    }

    /* check if image is exist or not */
    ret = dev_upgrade_get_img_exist_status((const unsigned char *)info->ufs_img_data, info->ufs_img_size, &img_status);
    if (ret != 0) {
        dev_upgrade_err("dev_upgrade_get_img_exist_status fail, ret=%d.\n", ret);
        DEV_UPGRADE_VFREE(info->ufs_img_data);
        return ret;
    }

    if (img_status == FILE_EXIST) {
        /* check if crl file exist in image */
        ret = dev_upgrade_check_img_crl_status((const unsigned char *)info->ufs_img_data,
            info->ufs_img_size, &crl_status);
        if (ret != 0) {
            dev_upgrade_err("dev_upgrade_check_img_crl_status fail, ret=%d.\n", ret);
            DEV_UPGRADE_VFREE(info->ufs_img_data);
            return ret;
        }

        if (crl_status == FILE_EXIST) {
            crl_info.img_data = info->ufs_img_data;
            crl_info.img_data_size = info->ufs_img_size;
            ret = dev_upgrade_get_img_crl_file(&crl_info);
            if (ret != 0) {
                dev_upgrade_warn("ufs img crl struct invalid, ret=%d.\n", ret);
                DEV_UPGRADE_VFREE(info->ufs_img_data);
                info->ufs_img_status = IMG_STATUS_NOT_HAVE_CRL;
                info->ufs_img_size = 0;
                return OK;
            }

            info->ufs_img_crl_data = crl_info.new_crl_data;
            info->ufs_img_crl_size = crl_info.new_crl_size;
            info->ufs_img_status = IMG_STATUS_HAVE_CRL;
        } else {
            info->ufs_img_status = IMG_STATUS_NOT_HAVE_CRL;
        }
    }

    return OK;
}

/*
 * Prototype    : dev_upgrade_get_ufs_crl_info
 * Description  : initialize image and crl related resource
 * Params
 *   [OUT] info: informatioin for update image and it's crl
 * Return Value : 0 for success, others for fail
 */
STATIC int dev_upgrade_get_ufs_crl_info(img_crl_info_t *info)
{
    int ret;

    info->ufs_crl_size = CRL_FILE_SIZE_MAX;
    info->ufs_crl_head = (unsigned char *)vzalloc(info->ufs_crl_size);
    if (info->ufs_crl_head == NULL) {
        dev_upgrade_err("vzalloc ufs crl data fail.\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    /* get crl file from ufs */
    ret = dev_upgrade_read_ufs_crl_file(info->ufs_crl_head, info->ufs_crl_size);
    if (ret != 0) {
        dev_upgrade_err("dev_upgrade_read_ufs_crl_file fail, ret=%d.\n", ret);
        DEV_UPGRADE_VFREE(info->ufs_crl_head);
        return ret;
    }

    ret = dev_upgrade_get_crl_exist_status(info);
    if (ret != 0) {
        dev_upgrade_err("dev_upgrade_get_crl_exist_status fail, ret=%d.\n", ret);
        DEV_UPGRADE_VFREE(info->ufs_crl_head);
        return ret;
    }

    return ret;
}

/*
 * Prototype    : dev_upgrade_get_update_crl_info
 * Description  : get update crl information.
 * Params
 *   [OUT] info: informatioin for update image and it's crl
 * Return Value : 0 for success, others for fail
 */
STATIC int dev_upgrade_get_update_crl_info(img_crl_info_t *info)
{
    int ret;
    int valid = CRL_FILE_NOT_VALID;

    /* check crl file's validity */
    ret = dev_upgrade_check_crl_file_validity(info->update_crl_data, info->update_crl_size, &valid);
    if (ret != 0) {
        dev_upgrade_err("dev_upgrade_check_crl_file_validity failed, ret=%d.\n", ret);
        return ret;
    }

    info->update_crl_status = valid;
    return ret;
}

STATIC void dev_upgrade_print_image_crl_info(img_crl_info_t *info)
{
    dev_upgrade_info("/=============== print image crl information.================\n");
    dev_upgrade_info("update_img_size = 0x%x.\n", info->update_img_size);
    dev_upgrade_info("update_img_crl_size = 0x%x.\n", info->update_img_crl_size);
    dev_upgrade_info("update_img_status = 0x%x.\n", info->update_img_status);
    dev_upgrade_info("ufs_img_size = 0x%x.\n", info->ufs_img_size);
    dev_upgrade_info("ufs_img_crl_size = 0x%x.\n", info->ufs_img_crl_size);
    dev_upgrade_info("ufs_img_status = 0x%x.\n", info->ufs_img_status);
    dev_upgrade_info("ufs_crl_size = 0x%x.\n", info->ufs_crl_size);
    dev_upgrade_info("ufs_crl_status = 0x%x.\n", info->ufs_crl_status);
    dev_upgrade_info("update_crl_size = 0x%x.\n", info->update_crl_size);
    dev_upgrade_info("update_crl_status = 0x%x.\n", info->update_crl_status);
    dev_upgrade_info("result = 0x%x.\n", info->result);
    dev_upgrade_info("/=============== print image crl information.================\n");
}

/*
 * Prototype    : dev_upgrade_init_img_crl_resource
 * Description  : initialize image and crl related resource
 * Params
 *   [IN] dev_id: device id
 *   [IN] buffer: buffer for crl file
 *   [IN] size: size of buffer
 * Return Value : 0 for success, others for fail
 */
STATIC int dev_upgrade_init_img_crl_resource(const char *update_img_path, int flag, img_crl_info_t *info)
{
    int ret;

    /* get update image information */
    if (flag == CRL_PROCESS_TYPE_UPDATE_IMG) {
        ret = dev_upgrade_get_update_img_info(update_img_path, info);
        if (ret != 0) {
            dev_upgrade_err("dev_upgrade_get_update_img_info fail, ret=%d.\n", ret);
            return ret;
        }
    } else {
        /* get update crl information */
        ret = dev_upgrade_get_update_crl_info(info);
        if (ret != 0) {
            dev_upgrade_err("dev_upgrade_get_update_crl_info fail, ret=%d.\n", ret);
            return ret;
        }
    }

    /* get ufs image information */
    ret = dev_upgrade_get_ufs_img_info(info, flag);
    if (ret != 0) {
        dev_upgrade_err("dev_upgrade_get_ufs_img_info fail, ret=%d.\n", ret);
        return ret;
    }

    /* get ufs crl information */
    ret = dev_upgrade_get_ufs_crl_info(info);
    if (ret != 0) {
        dev_upgrade_err("dev_upgrade_get_ufs_crl_info fail, ret=%d.\n", ret);
        return ret;
    }

    dev_upgrade_print_image_crl_info(info);
    g_img_crl_info = info;

    return ret;
}

STATIC void dev_upgrade_local_crl_info_set(unsigned char *crl_info)
{
    g_local_crl_info = crl_info;
}

STATIC void dev_upgrade_local_crl_info_reset(void)
{
    if (g_local_crl_info != NULL) {
        vfree(g_local_crl_info);
        g_local_crl_info = NULL;
    }
}

/*
 * Prototype    : dev_upgrade_release_img_crl_resource
 * Description  : release image and crl resource
 * Params
 *   [IN] info: image and crl information for release resource
 * Return Value : 0 for success, others for fail
 */
STATIC void dev_upgrade_release_img_crl_resource(img_crl_info_t *info)
{
    if (info->update_img_data != NULL) {
        DEV_UPGRADE_VFREE(info->update_img_data);
    }

    if (info->ufs_img_data != NULL) {
        DEV_UPGRADE_VFREE(info->ufs_img_data);
    }

    if (info->ufs_crl_head != NULL) {
        DEV_UPGRADE_VFREE(info->ufs_crl_head);
    }

    dev_upgrade_local_crl_info_reset();

    g_img_crl_info = NULL;
}

/*
 * Prototype    : dev_upgrade_verify_img_crl_process
 * Description  : upgdate crl file process
 * Params
 *   [IN] dev_id: device id
 *   [IN] buffer: buffer for crl file
 *   [IN] size: size of buffer
 * Return Value : 0 for success, others for fail
 */
STATIC int dev_upgrade_img_crl_process(const char *update_img_path, unsigned char *update_crl_data,
    unsigned int update_crl_size, int flag, dev_upgrade_crl_handler process_handler)
{
    int ret;
    int sec_flag = PKICMS_SEC_CHECK_DISABLE;
    img_crl_info_t img_crl_info = {0};

    if (process_handler == NULL) {
        dev_upgrade_err("input handler is null.\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    ret = pkicms_get_sec_check_enable_flag(&sec_flag);
    if (ret != 0) {
        dev_upgrade_err("pkicms get sec enable flag failed, ret = %d.\n", ret);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    if (sec_flag == PKICMS_SEC_CHECK_DISABLE) {
        return 0;
    }

    img_crl_info.update_crl_data = update_crl_data;
    img_crl_info.update_crl_size = update_crl_size;

    /* initialize image and crl resource */
    ret = dev_upgrade_init_img_crl_resource(update_img_path, flag, &img_crl_info);
    if (ret != 0) {
        dev_upgrade_err("dev_upgrade_init_img_crl_resource fail, ret=%d.\n", ret);
        goto EXIT_OUT;
    }

    /* process handler */
    ret = process_handler(update_img_path, &img_crl_info);
    if (ret != 0) {
        dev_upgrade_err("process handler fail, ret=%d.\n", ret);
        goto EXIT_OUT;
    }

EXIT_OUT:
    /* release img and crl resource */
    dev_upgrade_release_img_crl_resource(&img_crl_info);

    return ret;
}

/*
 * Prototype    : dev_upgrade_update_crl_handler
 * Description  : handler for img crl verify process
 * Params
 *   [IN] info: image and crl information
 * Return Value : 0 for success, others for fail
 */
STATIC int dev_upgrade_update_crl_handler(const char *file_path_in, img_crl_info_t *info)
{
    int ret;
    int stat = CRL_CMP_MIX;
    crl_info_t crl_info_tmp = {0};

    if (info->update_crl_status == CRL_FILE_NOT_VALID) {
        dev_upgrade_err("update crl file is not valid, stop upgrade.\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

   /* if old crl exist, compare crl's timestamp */
    if (info->ufs_crl_status == CRL_FILE_VALID) {
        crl_info_tmp.new_crl_data = info->update_crl_data;
        crl_info_tmp.new_crl_size = info->update_crl_size;
        crl_info_tmp.old_crl_data = info->ufs_crl_data;
        crl_info_tmp.old_crl_size = info->ufs_crl_size;
        ret = dev_upgrade_cmp_crl_file(&crl_info_tmp, &stat);
        if (ret != 0) {
            dev_upgrade_err("dev_upgrade_cmp_crl_file fail, ret=%d.\n", ret);
            return ret;
        }

        dev_upgrade_info("compare result is %d.\n", stat);
        if ((stat != CRL_CMP_NEW) && (stat != CRL_CMP_SAME)) {
            dev_upgrade_err("crl file is not the newer version, stop upgrade.\n");
            return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
        } else if (stat == CRL_CMP_SAME) {
            dev_upgrade_info("crl file is the same version, no need to upgrade.\n");
            return 0;
        }
    }

    if (info->ufs_img_status == IMG_STATUS_HAVE_CRL) {
        /* use update crl to verify ufs image */
        ret = dev_upgrade_verify_image_by_crl(IMAGE_TYPE_UFS, file_path_in, info->update_crl_data,
                                              info->update_crl_size);
        if (ret != 0) {
            dev_upgrade_err("dev_upgrade_verify_image_by_crl failed, ret=%d, please update image.\n", ret);
            return ret;
        }
    }

    ret = dev_upgrade_write_ufs_crl_file(info->update_crl_data, info->update_crl_size);
    if (ret != 0) {
        dev_upgrade_err("dev_upgrade_write_ufs_crl_file fail, ret=%d.\n", ret);
        return ret;
    }

    return ret;
}

STATIC int dev_upgrade_read_local_crl_file(unsigned char *local_crl_buf, unsigned int *local_crl_size,
    pkicms_sign_alg type)
{
    struct file *fp = NULL;
    char *local_crl_path = NULL;
    ssize_t read_len;
    loff_t pos;

    if (type == PKICMS_RSA_PKCS1) {
        local_crl_path = DEV_UPGRADE_LOCAL_PKCS_CRL_PATH;
    } else {
        local_crl_path = DEV_UPGRADE_LOCAL_PSS_CRL_PATH;
    }

    fp = filp_open(local_crl_path, O_RDONLY, 0);
    if (IS_ERR_OR_NULL(fp)) {
        dev_upgrade_err("unable to open file: %s, type = %d errno = %ld.\n", local_crl_path, type, PTR_ERR(fp));
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_OPEN);
    }

    pos = fp->f_pos;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
    read_len = kernel_read(fp, local_crl_buf, DEV_UPGRADE_CRL_BUF_SIZE, &pos);
#else
    mm_segment_t old_fs;
    old_fs = get_fs();
    set_fs(KERNEL_DS);
    read_len = vfs_read(fp, local_crl_buf, DEV_UPGRADE_CRL_BUF_SIZE, &pos);
    set_fs(old_fs);
#endif

    if (read_len <= 0) {
        dev_upgrade_err("read %s failed, read_len(%lu).\n", local_crl_path, read_len);
        filp_close(fp, NULL);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_FS_READ);
    }

    *local_crl_size = read_len;
    dev_upgrade_info("load local file(%s) success, type(%d).\n", local_crl_path, type);

    filp_close(fp, NULL);
    return 0;
}

STATIC int dev_upgrade_load_new_local_crl(img_crl_info_t *info, int *upd_crl_flag)
{
    int ret;
    pkicms_sign_alg ufs_img_sign_type, upgrade_img_sign_type;
    esbc_header_t *pimage_head;
    unsigned char *local_crl_buf = NULL;
    unsigned int local_crl_size = 0;

    pimage_head = (esbc_header_t *)(uintptr_t)info->ufs_img_data;
    ufs_img_sign_type = pkicms_get_alg_type_by_sign_data(pimage_head->sign_alg);
    pimage_head = (esbc_header_t *)(uintptr_t)info->update_img_data;
    upgrade_img_sign_type = pkicms_get_alg_type_by_sign_data(pimage_head->sign_alg);

    /* The signature algorithm is upgraded, the latest local CRL certificate
       is used for signature verification, and the CRL is updated. */
    if (ufs_img_sign_type != upgrade_img_sign_type) {
        local_crl_buf = (unsigned char *)vzalloc(DEV_UPGRADE_CRL_BUF_SIZE);
        if (local_crl_buf == NULL) {
            dev_upgrade_err("local_crl_buf vzalloc failed, len(%d).\n", DEV_UPGRADE_CRL_BUF_SIZE);
            return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
        }

        ret = dev_upgrade_read_local_crl_file(local_crl_buf, &local_crl_size, upgrade_img_sign_type);
        if (ret != 0) {
            dev_upgrade_err("read_local_crl_file failed, ret(%d), upgrade_img_sign_type(%d).\n",
                            ret, upgrade_img_sign_type);
            vfree(local_crl_buf);
            local_crl_buf = NULL;
            return ret;
        }
        dev_upgrade_local_crl_info_set(local_crl_buf);
        info->ufs_crl_data = local_crl_buf;
        info->ufs_crl_size = local_crl_size;
        *upd_crl_flag = 1;
    }

    return 0;
}

/*
 * Prototype    : dev_upgrade_cmp_and_verify_crl_handler
 * Description  : compare and verify crl handler
 * Params
 *   [IN] info: image and crl information
 * Return Value : 0 for success, others for fail
 */
STATIC int dev_upgrade_cmp_and_verify_crl_handler(const char *file_path_in, img_crl_info_t *info)
{
    int ret;
    crl_info_t crl_info_tmp = {0};
    int stat = CRL_CMP_MIX;
    int upd_crl_flag = 0;

    ret = dev_upgrade_load_new_local_crl(info, &upd_crl_flag);
    if (ret) {
        dev_upgrade_err("dev_upgrade_load_new_local_crl failed. ret=%d\n", ret);
        return ret;
    }

    crl_info_tmp.new_crl_data = info->ufs_crl_data;
    crl_info_tmp.new_crl_size = info->ufs_crl_size;
    crl_info_tmp.old_crl_data = info->update_img_crl_data;
    crl_info_tmp.old_crl_size = info->update_img_crl_size;

    dev_upgrade_debug("new crl size=%u, old crl size=%u.\n", crl_info_tmp.new_crl_size, crl_info_tmp.old_crl_size);

    ret = dev_upgrade_cmp_crl_file(&crl_info_tmp, &stat);
    if (ret != 0) {
        dev_upgrade_err("dev_upgrade_cmp_crl_file fail, ret=%d.\n", ret);
        return ret;
    }

    if (stat == CRL_CMP_OLD) {
        crl_info_tmp.new_crl_data = crl_info_tmp.old_crl_data;
        crl_info_tmp.new_crl_size = crl_info_tmp.old_crl_size;
    } else if ((stat != CRL_CMP_SAME) && (stat != CRL_CMP_NEW)) {
        dev_upgrade_err("crl file is not valid, stop upgrading.\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    /* use update crl to verify ufs image */
    ret = dev_upgrade_verify_image_by_crl(IMAGE_TYPE_FILE, file_path_in, crl_info_tmp.new_crl_data,
        crl_info_tmp.new_crl_size);
    if (ret != 0) {
        dev_upgrade_err("dev_upgrade_verify_image_by_crl failed, ret=%d.\n", ret);
        return ret;
    }

    /* if ufs crl is older, update ufs crl file */
    if ((stat != CRL_CMP_OLD) || (upd_crl_flag == 1)) {
        /* write crl file to ufs crl zone */
        ret = dev_upgrade_write_ufs_crl_file(crl_info_tmp.new_crl_data, crl_info_tmp.new_crl_size);
        if (ret != 0) {
            dev_upgrade_err("dev_upgrade_write_ufs_crl_file fail, ret=%d.\n", ret);
            return ret;
        }
    }

    return ret;
}

/*
 * upgrade the crl file in ufs
 *
 * If the upgrade image package is in PSS format and the UFS CRL is invalid,
 * update the UFS CRL file to the CRL file in the upgrading image package.
 */
STATIC int dev_upgrade_upgrade_ufs_crl_file(img_crl_info_t *info)
{
    int ret;

    if (info->ufs_img_status == IMG_STATUS_HAVE_CRL) {
        /* 1. if ufs crl is not invalid and ufs image have crl, write ufs image crl to ufs crl zone. */
        ret = dev_upgrade_write_ufs_crl_file(info->ufs_img_crl_data, info->ufs_img_crl_size);
        if (ret != 0) {
            dev_upgrade_err("dev_upgrade_write_ufs_crl_file fail, ret=%d.\n", ret);
            return ret;
        }

        info->ufs_crl_data = info->ufs_img_crl_data;
        info->ufs_crl_size = info->ufs_img_crl_size;
    }

    return 0;
}

/*
 * Prototype    : dev_upgrade_compare_crl
 * Description  : compare upgrade img crl and ufs crl
 *                if upgrade img crl and ufs crl compare stat is not
 *                CRL_CMP_SAME、CRL_CMP_NEW or CRL_CMP_OLD
 *                info->ufs_crl_data = info->ufs_img_crl_data
 * Params
 *   [IN] info: image and crl information
 * Return Value : 0 for success, others for fail
 */
STATIC int dev_upgrade_compare_crl(img_crl_info_t *info)
{
    crl_info_t crl_info_tmp = {0};
    int stat = CRL_CMP_MIX;
    int ret;

    crl_info_tmp.new_crl_data = info->ufs_crl_data;
    crl_info_tmp.new_crl_size = info->ufs_crl_size;
    crl_info_tmp.old_crl_data = info->update_img_crl_data;
    crl_info_tmp.old_crl_size = info->update_img_crl_size;

    ret = dev_upgrade_cmp_crl_file(&crl_info_tmp, &stat);
    if (ret != 0) {
        dev_upgrade_err("ufs_crl_data and update_img_crl_data compare fail, ret=%d.\n", ret);
        return ret;
    }

    if ((stat != CRL_CMP_SAME) && (stat != CRL_CMP_NEW) && (stat != CRL_CMP_OLD)) {
        dev_upgrade_debug("stat maybe not valid!\n");
        info->ufs_crl_data = info->ufs_img_crl_data;
        info->ufs_crl_size = info->ufs_img_crl_size;
    }

    return 0;
}

/*
 * Prototype    : dev_upgrade_verify_img_handler
 * Description  : handler for img crl verify process
 * Params
 *   [IN] info: image and crl information
 * Return Value : 0 for success, others for fail
 */
STATIC int dev_upgrade_verify_img_handler(const char *file_path_in, img_crl_info_t *info)
{
    int ret;
    int valid = CRL_FILE_NOT_VALID;

    /*
     * 1. if ufs image and ufs crl is invalid;
     * 2. if ufs image is exist but crl not exist, ufs crl not exist
     *    if neither of image have crl, no need to verify crl
     *    if update image have crl, use crl to verify image and write crl file to ufs.
     */
    if ((info->ufs_crl_status == CRL_FILE_NOT_VALID) &&
        ((info->ufs_img_status == IMG_STATUS_NOT_FOUND) || (info->ufs_img_status == IMG_STATUS_NOT_HAVE_CRL))) {
        if (info->update_img_status == IMG_STATUS_NOT_HAVE_CRL) {
            dev_upgrade_info("ufs image and update image neither have crl, no need to verify crl.\n");
            return 0;
        }

        if (info->update_img_status == IMG_STATUS_HAVE_CRL) {
            ret = dev_upgrade_check_crl_file_validity(info->update_img_crl_data, info->update_img_crl_size, &valid);
            if (ret != 0) {
                dev_upgrade_err("dev_upgrade_check_crl_file_validity failed, ret=%d.\n", ret);
                return ret;
            }

            if (valid != CRL_FILE_VALID) {
                dev_upgrade_err("update image has illegal crl file, stop upgrade.\n");
                return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
            }

            /*
             * because image verify is done before crl check, so no need to verify again
             * write crl file to ufs crl zone directly
             */
            ret = dev_upgrade_write_ufs_crl_file(info->update_img_crl_data, info->update_img_crl_size);
            if (ret != 0) {
                dev_upgrade_err("dev_upgrade_write_ufs_crl_file fail, ret=%d.\n", ret);
                return ret;
            }
        }
    } else {
        if (info->ufs_crl_status == CRL_FILE_NOT_VALID) {
            ret = dev_upgrade_upgrade_ufs_crl_file(info);
            if (ret != 0) {
                dev_upgrade_err("dev_upgrade_upgrade_ufs_crl_file fail, ret=%d.\n", ret);
                return ret;
            }
        }

        ret = dev_upgrade_compare_crl(info);
        if (ret != 0) {
            dev_upgrade_err("dev_upgrade_compare_crl fail, ret=%d.\n", ret);
            return ret;
        }

        /* if ufs have crl and image not have crl, stop upgrade */
        if (info->update_img_status == IMG_STATUS_NOT_HAVE_CRL) {
            dev_upgrade_err("ufs image have crl,but update image not have crl, stop upgrade.\n");
            return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
        }

        /* compare update image crl and ufs crl, then use the newer crl to verify update image */
        ret = dev_upgrade_cmp_and_verify_crl_handler(file_path_in, info);
        if (ret != 0) {
            dev_upgrade_err("dev_upgrade_cmp_and_verify_crl_handler fail, ret=%d.\n", ret);
            return ret;
        }
    }

    return 0;
}

/*
 * Prototype    : dev_upgrade_verify_img_crl_process
 * Description  : verify image crl file process
 * Params
 *   [IN] update_img_path: update image path
 * Return Value : 0 for success, others for fail
 */
int dev_upgrade_verify_img_crl_process(const char *update_img_path)
{
    return dev_upgrade_img_crl_process(update_img_path, NULL, 0, CRL_PROCESS_TYPE_UPDATE_IMG,
        dev_upgrade_verify_img_handler);
}

/*
 * Prototype    : dev_upgrade_update_crl_process
 * Description  : upgdate crl file process
 * Params
 *   [IN] buffer: buffer for crl file
 *   [IN] size: size of buffer
 * Return Value : 0 for success, others for fail
 */
int dev_upgrade_update_crl_process(unsigned char *buf, unsigned int size)
{
    return dev_upgrade_img_crl_process(NULL, buf, size, CRL_PROCESS_TYPE_UPDATE_CRL,
        dev_upgrade_update_crl_handler);
}

void dev_upgrade_img_id_init(void)
{
    unsigned int i;

    for (i = 0; i < IMG_ID_MAX_NUM; i++) {
        if ((g_img_id_info[i].name != NULL) && (g_img_id_info[i].img_id < IMAGE_ID_MAX)) {
            continue;
        }
        g_img_id_info[i].name = NULL;
        g_img_id_info[i].img_id = IMG_ID_MAX_NUM;
    }

    return;
}

void dev_upgrade_img_id_uninit(void)
{
    unsigned int i;

    for (i = 0; i < IMG_ID_MAX_NUM; i++) {
        if ((g_img_id_info[i].name != NULL) && (g_img_id_info[i].img_id < IMAGE_ID_MAX)) {
            continue;
        }
        DEV_UPGRADE_VFREE(g_img_id_info[i].name);
        g_img_id_info[i].img_id = IMG_ID_MAX_NUM;
    }

    return;
}

int dev_upgrade_get_new_img_id(const char *path_name, int *img_id, int index)
{
    int ret, len;
    int sep = '/';
    char *name_start = NULL;
    char *dst_name = NULL;

    if (index >= IMG_ID_MAX_NUM) {
        dev_upgrade_warn("index over limit(index=%d).\n", index);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    if (path_name == NULL) {
        dev_upgrade_err("path_name is NULL.\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_NULL_POINT);
    }

    name_start = (char *)strrchr(path_name, sep);
    if (name_start == NULL) {
        dev_upgrade_err("path_name can't find '/'.\n");
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }

    len = strlen(name_start) + 1;
    if (len > PATH_MAX) {
        dev_upgrade_err("len(%d) over limit(%d).\n", len, PATH_MAX);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_LENGTH);
    }

    dst_name = (char *)vzalloc(len);
    if (dst_name == NULL) {
        dev_upgrade_err("base_path vzalloc(%d) fail.\n", len);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_MALLOC);
    }

    ret = memcpy_s(dst_name, len, name_start, strlen(name_start));
    if (ret < 0) {
        dev_upgrade_err("memcpy_s fail, len(%d).\n", len);
        DEV_UPGRADE_VFREE(dst_name);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_MEM_INIT);
    }

    g_img_id_info[index].img_id = g_new_img_id;
    if (g_img_id_info[index].img_id >= IMG_ID_MAX_NUM) {
        dev_upgrade_warn("img id over limit(image_id=%d).\n", g_img_id_info[index].img_id);
        g_img_id_info[index].img_id = IMG_ID_MAX_NUM;
        DEV_UPGRADE_VFREE(dst_name);
        return dev_errno_make(DEV_MID_UPGRADE, ERRNO_INVALID_PARAMS);
    }
    g_new_img_id++;
    g_img_id_info[index].name = dst_name;
    *img_id = g_img_id_info[index].img_id;

    return 0;
}

/*
 * Prototype    : dev_upgrade_get_img_id
 * Description  : get image id for verify
 * Params
 *   [IN] path_name: image path name
 *   [IN] img_id: image id
 * Return Value : None
 */
void dev_upgrade_get_img_id(const char *path_name, int *img_id)
{
    int ret;
    int i;
    char *ret_str = NULL;
    int id = IMG_ID_MAX_NUM;

    for (i = 0; i < IMG_ID_MAX_NUM; i++) {
        if (g_img_id_info[i].name == NULL) {
            break;
        }
        ret_str = (char *)strstr(path_name, g_img_id_info[i].name);
        if (ret_str != NULL) {
            dev_upgrade_info("name: %s, img_id: %d.\n", g_img_id_info[i].name, g_img_id_info[i].img_id);
            *img_id = g_img_id_info[i].img_id;
            return;
        }
    }

    ret = dev_upgrade_get_new_img_id(path_name, &id, i);
    if (ret != 0) {
        dev_upgrade_err("dev_upgrade_find_file_name fail, ret=%d.\n", ret);
        *img_id = IMG_ID_MAX_NUM;
        return;
    }

    dev_upgrade_info("name: %s, img_id: %d.\n", g_img_id_info[i].name, g_img_id_info[i].img_id);
    *img_id = id;

    return;
}

STATIC uintptr_t dev_upgrade_open_img(const char *filename)
{
    int img_id;
    struct local_image_verify *local_imgs = NULL;

    dev_upgrade_get_img_id(filename, &img_id);
    if (img_id >= IMG_ID_MAX_NUM) {
        dev_upgrade_err("error img_id: %d\n", img_id);
        return (uintptr_t)NULL;
    }
    local_imgs = dev_upgrade_get_local_imgs();
    local_imgs[img_id].img_loff = 0;
    return (uintptr_t)(&local_imgs[img_id]);
}

int dev_upgrade_read_img(uintptr_t local_img, char *buff, u32 len, int *read_len)
{
    int ret;
    int len_tmp;
    int read_flag;
    struct local_image_verify *img_info = (struct local_image_verify *)local_img;

    if (img_info->img_len - img_info->img_loff >= len) {
        len_tmp = len;
        read_flag = READ_CONTINUE;
    } else {
        len_tmp = img_info->img_len - img_info->img_loff;
        read_flag = READ_END;
    }
    ret = memcpy_s(buff, len_tmp, img_info->img_buff + img_info->img_loff, len_tmp);
    if (ret < 0) {
        dev_upgrade_err("copy img fail, ret = %d\n", ret);
        return READ_ERROR;
    }
    img_info->img_loff += len_tmp;
    *read_len = len_tmp;
    return read_flag;
}

STATIC loff_t dev_upgrade_lseek(uintptr_t local_img, loff_t offset, int whence)
{
    struct local_image_verify *img_info = (struct local_image_verify *)local_img;
    if ((offset > img_info->img_len) || ((offset < 0) && (img_info->img_len + offset < 0))) {
        dev_upgrade_err("offset = %lld, is larger than file size: %lld\n", offset, img_info->img_len);
        return -ERRNO_FS_SEEK;
    }
    if (whence == SEEK_CUR) {
        dev_upgrade_err("not support SEEK_CUR\n");
        return -ERRNO_NONSUPPORT_ITEM;
    } else if (whence == SEEK_SET) {
        img_info->img_loff = offset;
    } else if (whence == SEEK_END) {
        img_info->img_loff = img_info->img_len + offset;
    } else {
        dev_upgrade_err("not support whence(%d)\n", whence);
        return -ERRNO_FS_SEEK;
    }
    return img_info->img_loff;
}

STATIC void dev_upgrade_free_img(uintptr_t local_img)
{
    struct local_image_verify *img_info = (struct local_image_verify *)local_img;
    img_info->img_loff = 0;
}

STATIC loff_t dev_upgrade_current_offset(uintptr_t local_img)
{
    struct local_image_verify *img_info = (struct local_image_verify *)local_img;
    return img_info->img_loff;
}

STATIC image_fop_t image_verify_ops = {
    .open = dev_upgrade_open_img,
    .read = dev_upgrade_read_img,
    .lseek = dev_upgrade_lseek,
    .release = dev_upgrade_free_img,
    .cur_offset = dev_upgrade_current_offset,
};

image_fop_t *get_image_verify_ops(void)
{
    return &image_verify_ops;
}
/*
 * Prototype    : dev_upgrade_update_crl_process
 * Description  : upgdate crl file process
 * Params
 *   [IN] component_type: component_type
 *   [IN] path_name: image path name
 * Return Value : 0 for success, others for fail
 */
int dev_upgrade_verify_cms_img_process(int component_type, const char *path_name)
{
    int ret;
    int img_id = IMG_ID_MAX_NUM;
    crl_info_st crl_info = {NULL, 0};

    if ((component_type > (int)DSMI_COMPONENT_TYPE_CUSTOMIZED)
        && (component_type != (int)DSMI_COMPONENT_TYPE_RECOVERY)) {
        dev_upgrade_info("not signed component(%d), no need to verify.\n", component_type);
        return 0;
    }

    dev_upgrade_get_img_id(path_name, &img_id);

    ret = pkicms_verify_cms(img_id, path_name, &crl_info, &image_verify_ops);
    if (ret != 0) {
        dev_upgrade_err("pkicms_verify_cms fail, img_id=%d, ret=%d.\n", img_id, ret);
    } else {
        dev_upgrade_info("pkicms_verify_cms succ, img_id = %d, path_name = %s\n", img_id, path_name);
    }

    return ret;
}
