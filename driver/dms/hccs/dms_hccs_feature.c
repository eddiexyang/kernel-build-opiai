/*
* Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
* Create: 2022-11-11
*/
#include <linux/securec.h>
#include <asm/io.h>
#include <linux/uaccess.h>

#include "devdrv_user_common.h"
#include "dms_define.h"
#include "dms_template.h"
#include "dms_cmd_def.h"
#include "urd_acc_ctrl.h"
#include "dms_basic_info.h"
#include "devdrv_manager_hccs.h"
#include "devdrv_common.h"
#include "ascend_kernel_hal.h"
#include "dms_hccs_feature.h"

typedef struct hpcs_status_reg {
    unsigned int st_pcs_mode_change_done : 1;
    unsigned int reserved_1 : 3;
    unsigned int st_pcs_mode_working : 2;
    unsigned int reserved_2 : 2;
    unsigned int st_pcs_use_working : 8;
    unsigned int reserved_3 : 16;
} hccs_pcs_status_reg_t;

#ifdef CFG_FEATURE_GET_PCS_BITMAP_BY_BOARD_TYPE
/*
 *  BOARD ID
 *  bit7: 0 1die; 1 2die
 *  bit4-6: 000 EVB
 *          001 PCIE(1die), module(2die)
 *          others: module
 */
#define BOARD_ID_TYPE_MASK 0xF0
#define BOARD_ID_TYPE_OFFSET 4
#define BOARD_TYPE_EVB_SINGLE_DIE 0
#define BOARD_TYPE_EVB_DOUBLE_DIE 8
#define BOARD_TYPE_PCIE_SINGLE_DIE 1
#define PCS_BITMAP_PCIE_EVB 0xFC
#define PCS_BITMAP_MODULE 0xFE
int dms_get_hpcs_bitmap_by_board_type(unsigned int dev_id, unsigned long long *bitmap)
{
    struct devdrv_info *dev_info = NULL;
    struct dms_dev_ctrl_block *dev_cb = NULL;
    unsigned int board_id;
    unsigned int board_type;

    dev_cb = dms_get_dev_cb(dev_id);
    if (dev_cb == NULL) {
        dms_err("Get device ctrl block failed. (dev_id=%u)\n", dev_id);
        return -ENODEV ;
    }

    if (dev_cb->dev_info == NULL) {
        dms_err("Device ctrl dev_info is null. (dev_id=%u)\n", dev_id);
        return -ENODEV ;
    }

    dev_info = (struct devdrv_info *)dev_cb->dev_info;
    board_id = dev_info->board_id;
    board_type = (board_id & BOARD_ID_TYPE_MASK) >> BOARD_ID_TYPE_OFFSET;
    if ((board_type == BOARD_TYPE_EVB_SINGLE_DIE) ||
        (board_type == BOARD_TYPE_EVB_DOUBLE_DIE) ||
        (board_type == BOARD_TYPE_PCIE_SINGLE_DIE)) {
        *bitmap = PCS_BITMAP_PCIE_EVB;
    } else {
        *bitmap = PCS_BITMAP_MODULE;
    }

    return 0;
}

#else
int dms_get_hpcs_bitmap_default(unsigned int dev_id, unsigned long long *bitmap)
{
    *bitmap = 0;
    return 0;
}

#endif

int dms_get_hccs_status_by_dev_id(unsigned int dev_id, hccs_info_t *hccs_status)
{
    int i, ret;
    unsigned long long pcs_bitmap = 0;
    unsigned int pcs_status_reg;
    void __iomem *hccs_base_addr = NULL;
    unsigned long pcs_phy_addr[PCS_NUM] = {PCS0_BASE_ADDR, PCS1_BASE_ADDR, PCS2_BASE_ADDR, PCS3_BASE_ADDR,
        PCS4_BASE_ADDR, PCS5_BASE_ADDR, PCS6_BASE_ADDR, PCS7_BASE_ADDR};
    devdrv_hardware_info_t hardware_info = {0};

    ret = DMS_GET_HPCS_BITMAP(dev_id, &pcs_bitmap);
    if (ret != 0) {
        dms_err("Get hpcs bitmap failed. (device_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    ret = hal_kernel_get_hardware_info(dev_id, &hardware_info);
    if (ret != 0) {
        dms_err("Failed to invoke hal_kernel_get_hardware_info. (devid=%u)\n", dev_id);
        return ret;
    }

    for (i = 0; i < PCS_NUM; i++) {
        if (!(pcs_bitmap & (1 << i))) {
            continue;
        }

        hccs_base_addr = ioremap(pcs_phy_addr[i] + hardware_info.phy_addr_offset, PAGE_SIZE);
        if (hccs_base_addr == NULL) {
            dms_err("Remap addr for pcs status failed. (dev_id=%u)\n", dev_id);
            return -ENOMEM;
        }

        pcs_status_reg = HCCS_REG_RD(hccs_base_addr, ST_CH_PCS_LANE_MODE_CHANGE_OFFSET);
        iounmap(hccs_base_addr);
        hccs_base_addr = NULL;

        if ((((hccs_pcs_status_reg_t *)&pcs_status_reg)->st_pcs_mode_working == ST_PCS_MODE_WORKING_X4) &&
            (((hccs_pcs_status_reg_t *)&pcs_status_reg)->st_pcs_use_working == ST_PCS_USE_WORKING_X4)) {
            hccs_status->pcs_status = 0;
        } else {
            hccs_status->pcs_status = (1 << PCS_STATUS_OFFSET) | (i << PCS_INDEX_OFFSET) |
                (((hccs_pcs_status_reg_t *)&pcs_status_reg)->st_pcs_mode_working << PCS_MODE_WORKING_OFFSET) |
                (((hccs_pcs_status_reg_t *)&pcs_status_reg)->st_pcs_use_working << PCS_USE_WORKING_OFFSET);
            break;
        }
    }
    return 0;
}

STATIC int dms_get_hccs_status(struct dms_get_device_info_in *in, unsigned int *out_size)
{
    int ret;
    hccs_info_t hccs_status = {0};

    ret = dms_get_hccs_status_by_dev_id(in->dev_id, &hccs_status);
    if (ret != 0) {
        return ret;
    }

    ret = copy_to_user((void *)(uintptr_t)in->buff, &hccs_status, sizeof(hccs_info_t));
    if (ret != 0) {
        dms_err("copy_to_user failed. (dev_id=%u; ret=%d)\n", in->dev_id, ret);
        return ret;
    }

    dms_debug("Get HPCS status success. (dev_id=%u; status=0x%x)\n", in->dev_id, hccs_status.pcs_status);
    *out_size = sizeof(hccs_info_t);
    return 0;
}

int dms_feature_get_hccs_info(void *feature, char *in, u32 in_len, char *out, u32 out_len)
{
    int ret;
    struct dms_get_device_info_in *input = NULL;
    struct dms_get_device_info_out *output = {0};
    unsigned int out_size = 0;

    if ((in == NULL) || (in_len != sizeof(struct dms_get_device_info_in))) {
        dms_err("Input argument is null, or in_len is wrong. (in_len=%u)\n", in_len);
        return -EINVAL;
    }

    input = (struct dms_get_device_info_in *)in;
    if ((input->buff == NULL) || (input->buff_size < sizeof(hccs_info_t))) {
        dms_err("Input buffer is null or buffer size is not valild. (buff_is_null=%d; buff_size=%u)\n",
            (input->buff != NULL), input->buff_size);
        return -EINVAL;
    }

    output = (struct dms_get_device_info_out *)out;
    if ((out == NULL) || (out_len != sizeof(struct dms_get_device_info_out))) {
        dms_err("Output argument is null, or out_len is wrong. (out_len=%u)\n", out_len);
        return -EINVAL;
    }

    switch (input->sub_cmd) {
        case DMS_HCCS_SUB_CMD_STATUS:
            ret = dms_get_hccs_status(input, &out_size);
            if (ret != 0) {
                dms_err("Dms get hccs info failed. (dev_id=%u; ret=%d)\n", input->dev_id, ret);
                return ret;
            }
            break;
        default:
            dms_err("Input sub_cmd is invalid. (dev_id=%u; sub_cmd=%d)\n", input->dev_id, input->sub_cmd);
            return -EINVAL;
    }

    output->out_size = out_size;
    return 0;
}

int dms_hccs_init(void)
{
    dms_info("dms hccs init.\n");
    CALL_INIT_MODULE(DMS_MODULE_HCCS);
    return 0;
}

int dms_hccs_exit(void)
{
    dms_info("dms hccs exit.\n");
    CALL_EXIT_MODULE(DMS_MODULE_HCCS);
    return 0;
}

BEGIN_DMS_MODULE_DECLARATION(DMS_MODULE_HCCS)
BEGIN_FEATURE_COMMAND()
ADD_FEATURE_COMMAND(DMS_MODULE_HCCS,
    DMS_GET_GET_DEVICE_INFO_CMD,
    ZERO_CMD,
    DMS_FILTER_HCCS,
    "dmp_daemon",
    DMS_SUPPORT_ALL,
    dms_feature_get_hccs_info)
END_FEATURE_COMMAND()
END_MODULE_DECLARATION()
