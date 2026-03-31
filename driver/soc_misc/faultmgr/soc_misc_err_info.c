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

#include "soc_misc_err_info.h"
#include "soc_misc_init.h"
#include "receiver/fpdc_ras_receiver.h"
#include "drv_ras_common.h"

#include <linux/securec.h>

#define SOC_ARM_RAS_VENDOR_BUFF_LEN (64U)

/* RAS error status */
#define CPER_VENDOR_SPEC_ERR_STATUS_OFFSET  (0)
#define CPER_VENDOR_SPEC_ERR_STATUS_SIZE    (4)

/* valid flag */
#define CPER_VENDOR_SPEC_VALID_FLAG_OFFSET  (12)
#define CPER_VENDOR_SPEC_VALID_FLAG_SIZE     (1)

/* sub module id */
#define CPER_VENDOR_SPEC_SUB_MODULE_OFFSET  (15)
#define CPER_VENDOR_SPEC_SUB_MODULE_SIZE     (1)

#define HISI_COMMON_ERR_STATUS_OFFSET       (4)
#define HISI_COMMON_MIN_REGISTER_ARRAY_SIZE (HISI_COMMON_ERR_STATUS_OFFSET + 1)

int soc_misc_sec_type_guid_cmp(const guid_t *u1, const guid_t *u2)
{
    if (u1 == NULL || u2 == NULL) {
        return 0;
    }

    return memcmp(u1, u2, sizeof(guid_t)) == 0;
}

int soc_misck_check_notify_data(const struct notify_data *pdata)
{
    guid_t hisi_common_sec_type = CPER_SEC_HISI_COMMON;

    if ((pdata == NULL) || (pdata->origin_data == NULL)) {
        soc_misc_drv_err("notify data is invalid\n");
        return -EINVAL;
    }

    if ((soc_misc_sec_type_guid_cmp(pdata->section_type, &hisi_common_sec_type) &&
        pdata->data_len < sizeof(hisi_common_error_info)) || (pdata->data_len < sizeof(struct cper_sec_proc_arm))) {
        soc_misc_drv_err("ras data is not too short. (len=%u)\n", pdata->data_len);
        return -EINVAL;
    }

    return 0;
}

int soc_misc_get_err_status(const struct notify_data *pdata, u32 *err_status)
{
    int ret;
    struct cper_sec_proc_arm *pdata_ras = NULL;
    struct vendor_specific_error_info *vendor_info = NULL;
    hisi_common_error_info *hisi_error = NULL;
    guid_t hisi_common_sec_type = CPER_SEC_HISI_COMMON;

    if (soc_misc_sec_type_guid_cmp(pdata->section_type, &hisi_common_sec_type)) {
        hisi_error = (hisi_common_error_info *)pdata->origin_data;
        if (hisi_error->register_array_size < HISI_COMMON_MIN_REGISTER_ARRAY_SIZE) {
            soc_misc_drv_err("hisi common register array don't contain err status info.\n");
            return -EINVAL;
        }
        *err_status = *((u32 *)hisi_error->register_array + HISI_COMMON_ERR_STATUS_OFFSET);
    } else {
        pdata_ras = (struct cper_sec_proc_arm *)pdata->origin_data;
        ret = soc_misc_arm_get_vendor_info(pdata_ras, &vendor_info);
        if (ret) {
            soc_misc_drv_err("read vendor specific err info failed. (ret=%d)\n", ret);
            return ret;
        }

        *err_status = vendor_info->err_status;
    }

    return 0;
}

/** get cpu cluster id
 *
 * vendor specific error info form as below:
 *   bit[31:0]:ARER Error  status
 *   bit[95:32]:ARER Error address
 *   bit[103:96]:valid flag, 1 mean valid. current definition (
                    bit0: socket id
                    bit1: die id
                    bit2: sub module
                    bit3~bit7 reserved)
 *   bit[111:104]:socket id(0~3:socket0~socket3)
 *   bit[119:112]:die id
 *   bit[127:120]:sub module;(CPU core: 0~15; L3D:0~3; L3T:0~3)
 */
int soc_misc_get_cpu_cluster_id(struct cper_sec_proc_arm *pdata_ras, u8 *cpu_cluster)
{
    struct vendor_specific_error_info *vendor_info = NULL;
    int ret;

    if (cpu_cluster == NULL) {
        soc_misc_drv_err("cpu cluster is null\n");
        return -EINVAL;
    }

    ret = soc_misc_arm_get_vendor_info(pdata_ras, &vendor_info);
    if (ret) {
        soc_misc_drv_err("read vendor specific err info failed. (ret=%d)\n", ret);
        return ret;
    }

    *cpu_cluster = (u8)(vendor_info->oem_sub_module);
    return 0;
}

int soc_misc_arm_get_vendor_info(struct cper_sec_proc_arm *arm_err, struct vendor_specific_error_info **vendor_info)
{
    int i, len, size;
    struct cper_arm_err_info *err_info = NULL;
    struct cper_arm_ctx_info *ctx_info = NULL;

    len = arm_err->section_length - (sizeof(*arm_err) + arm_err->err_info_num * (sizeof(*err_info)));
    if (len < 0) {
        soc_misc_drv_err("Section length is too small. (section_length=%u)\n", arm_err->section_length);
        return -EINVAL;
    }

    err_info = (struct cper_arm_err_info *)(arm_err + 1);
    ctx_info = (struct cper_arm_ctx_info *)(err_info + arm_err->err_info_num);

    for (i = 0; i < arm_err->context_info_num; i++) {
        if ((u32)len < sizeof(*ctx_info)) {
            soc_misc_drv_err("len is less than context head. (len=%d; size=%lu)\n", len, sizeof(*ctx_info));
            return -EINVAL;
        }
        size = sizeof(*ctx_info) + ctx_info->size;
        if (len < size) {
            soc_misc_drv_err("len is less than context info. (len=%d; size=%d)\n", len, size);
            return -EINVAL;
        }

        len -= size;
        ctx_info = (struct cper_arm_ctx_info *)((long)ctx_info + size);
    }

    if ((size_t)len < sizeof(struct vendor_specific_error_info)) {
        soc_misc_drv_warn("len is less than vendor size. (len=%d; vendor info=%lu)\n",
            len, sizeof(struct vendor_specific_error_info));
        return 0;
    }

    *vendor_info = (struct vendor_specific_error_info *)ctx_info;
    return 0;
}
