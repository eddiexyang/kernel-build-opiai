
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2023. All rights reserved.
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

#include "can_drv_feature.h"
#include <linux/securec.h>
#include <linux/slab.h>
#include "dms_define.h"
#include "dms_template.h"
#include "dms_cmd_def.h"
#include "dms_acc_ctrl.h"
#include "can_drv_api.h"
#include "can_drv_log.h"

#ifdef RUN_IN_AOS
#include <linux/uaccess.h>
#endif
#ifdef STATIC_SKIP
    #define STATIC
#else
    #define STATIC static
#endif

#define CAN_DSMI_MAIN_CMD_LEN   20
static char g_can_dsmi_main_cmd[CAN_DSMI_MAIN_CMD_LEN] = {0};
// DMS_SUPPORT_ALL
#define CAN_DSMI_SUB_CMD_CAN_INDEX_OFFSET 24
#define CAN_DSMI_SUB_CMD_MASK 0xFF
BEGIN_DMS_MODULE_DECLARATION(DMS_CAN_CMD_NAME)
BEGIN_FEATURE_COMMAND()
ADD_FEATURE_COMMAND(DMS_CAN_CMD_NAME, DMS_MAIN_CMD_CAN, DMS_SUBCMD_GET_CAN_STATUS, NULL, NULL,
                    DMS_SUPPORT_ALL, mttcan_get_can_status)
ADD_FEATURE_COMMAND(DMS_CAN_CMD_NAME, DMS_GET_SET_DEVICE_INFO_CMD, ZERO_CMD, g_can_dsmi_main_cmd,
                    "dmp_daemon", DMS_SUPPORT_ALL, mttcan_dmsi_set_can_config)
ADD_FEATURE_COMMAND(DMS_CAN_CMD_NAME, DMS_GET_GET_DEVICE_INFO_CMD, ZERO_CMD, g_can_dsmi_main_cmd,
                    NULL, DMS_SUPPORT_ALL, mttcan_dmsi_get_can_config)
END_FEATURE_COMMAND()
END_MODULE_DECLARATION()

enum {
    CAN_FEATURE_UNREG = 0,
    CAN_FEATURE_REGED = 1
};

static int can_feature_reg_flag = CAN_FEATURE_UNREG;
int can_drv_feature_init(void)
{
    int32_t ret;
    if (can_feature_reg_flag != CAN_FEATURE_UNREG) {
        mttcan_info("feature register already done, no need reg again.(reg_falg=%d)\n", can_feature_reg_flag);
        return 0;
    }
    ret = sprintf_s(g_can_dsmi_main_cmd, sizeof(g_can_dsmi_main_cmd),
        "main_cmd=0x%x", DMS_MAIN_CMD_CAN);
    if (ret <= 0) {
        mttcan_err("sprintf_s can main_cmd failed.(ret=%d, main_cmd=0x%x)\n", ret, DMS_MAIN_CMD_CAN);
        return ret;
    }
    CALL_INIT_MODULE(DMS_CAN_CMD_NAME);

    mttcan_info("can devmng dmsi init success, %s\n", g_can_dsmi_main_cmd);
    can_feature_reg_flag = CAN_FEATURE_REGED;
    return 0;
}

void can_drv_feature_uninit(void)
{
    if (can_feature_reg_flag != CAN_FEATURE_REGED) {
        mttcan_info("can feature unregister already done, no need unreg again.(reg_falg = %d)\n", can_feature_reg_flag);
        return;
    }

    CALL_EXIT_MODULE(DMS_CAN_CMD_NAME);
    can_feature_reg_flag = CAN_FEATURE_UNREG;
}

// parm check
STATIC int mttcan_feature_para_check(char *in, u32 in_len, u32 correct_in_len,
    char *out, u32 out_len, u32 correct_out_len)
{
    if ((in == NULL) || (in_len != correct_in_len)) {
        mttcan_err("Input char is NULL or in_len is wrong. (in_len=%u; correct_in_len=%u)\n",
            in_len, correct_in_len);
        return -EINVAL;
    }

    if ((out == NULL) || (out_len != correct_out_len)) {
        mttcan_err("Output char is NULL or out_len is wrong. (out_len=%u; correct_out_len=%u)\n",
            out_len, correct_out_len);
        return -EINVAL;
    }

    return 0;
}

STATIC int mttcan_dsmi_check_buff_size(size_t buff_size)
{
    if ((buff_size > SID_FILTER_MAX_SIZE) || (buff_size > XID_FILTER_MAX_SIZE)) {
        return -EINVAL;
    }
    return 0;
}

#define CAN_DSMI_NAME_LEN_MAX 6
int mttcan_get_can_status(void *feature, char *in, unsigned int in_len, char *out, unsigned int out_len)
{
    int ret;
    u32 namelen = in_len;
    unsigned char name[CAN_DSMI_NAME_LEN_MAX] = {0};
    struct can_status_stru can_status = {0};

    // input parm check, don't need to check in_len, because it will be check in get_can_status
    ret = mttcan_feature_para_check(in, in_len, in_len, out, out_len, sizeof(struct can_status_stru));
    if (ret != 0) {
        mttcan_err("Para check failed.\n");
        return ret;
    }

    // in shoule be char array like "can0"/"can10" ... without end char \0, so in len max len should be 5
    ret = memcpy_s(name, CAN_DSMI_NAME_LEN_MAX - 1, in, in_len);
    if (ret != 0) {
        mttcan_err("memcpy_s fail(ret = %d, in_len = %d)\n", ret, in_len);
        return ret;
    }
    ret = get_can_status(name, namelen, &can_status);
    if (ret != 0) {
        mttcan_err("Get can status failed. (ret=%d; can_name=%s)\n", ret, name);
        return ret;
    }

    *(struct can_status_stru *)out = can_status;
    return 0;
}

int mttcan_dmsi_set_can_config(void *feature, char *in, unsigned int in_len, char *out, unsigned int out_len)
{
    int32_t ret;
    unsigned int can_id;
    CAN_SUB_CMD sub_cmd;
    char *tmp_buff = NULL;
    struct dms_set_device_info_in *in_cfg = NULL;

    if ((in == NULL) || (in_len != sizeof(struct dms_set_device_info_in))) {
        mttcan_err("Input char is NULL or in_len is wrong. (in_len=%u)\n", in_len);
        return -EINVAL;
    }

    in_cfg = (struct dms_set_device_info_in *)in;
    can_id = (unsigned int)(in_cfg->sub_cmd >> CAN_DSMI_SUB_CMD_CAN_INDEX_OFFSET);
    sub_cmd = in_cfg->sub_cmd & CAN_DSMI_SUB_CMD_MASK;

    ret = mttcan_dsmi_check_buff_size(in_cfg->buff_size);
    if (ret != 0) {
        mttcan_err("Input buffer size is invalid. (buff_size=%u)\n", in_cfg->buff_size);
        return ret;
    }

    tmp_buff = (char *)kmalloc(in_cfg->buff_size, GFP_KERNEL);
    if (tmp_buff == NULL) {
        mttcan_err("kmalloc tmp_buff failed.\n");
        return -ENOMEM;
    }
    if (copy_from_user(tmp_buff, in_cfg->buff, in_cfg->buff_size)) {
        mttcan_err("copy_from_user failed\n");
        ret = -EFAULT;
        goto free_buff;
    }
    ret = mttcan_set_can_config(can_id, sub_cmd, (void *)tmp_buff, in_cfg->buff_size);
    if (ret != 0) {
        mttcan_err("can set fail.(ret = %d).\n", ret);
    }

free_buff:
    kfree(tmp_buff);
    (void)feature;
    (void)out;
    (void)out_len;

    return ret;
}

int mttcan_dmsi_get_can_config(void *feature, char *in, unsigned int in_len, char *out, unsigned int out_len)
{
    int32_t ret;
    unsigned int can_id;
    CAN_SUB_CMD sub_cmd;
    char *tmp_buff = NULL;
    struct dms_get_device_info_in *in_cfg = NULL;

    ret = mttcan_feature_para_check(in, in_len, sizeof(struct dms_get_device_info_in),
                                    out, out_len, sizeof(struct dms_get_device_info_out));
    if (ret != 0) {
        mttcan_err("Para check failed.\n");
        return ret;
    }

    in_cfg = (struct dms_get_device_info_in *)in;
    can_id = (unsigned int)(in_cfg->sub_cmd >> CAN_DSMI_SUB_CMD_CAN_INDEX_OFFSET);
    sub_cmd = in_cfg->sub_cmd & CAN_DSMI_SUB_CMD_MASK;

    ret = mttcan_dsmi_check_buff_size(in_cfg->buff_size);
    if (ret != 0) {
        mttcan_err("Input buffer size is invalid. (buff_size=%u)\n", in_cfg->buff_size);
        return ret;
    }

    tmp_buff = (char *)kmalloc(in_cfg->buff_size, GFP_KERNEL);
    if (tmp_buff == NULL) {
        mttcan_err("kmalloc tmp_buff failed.\n");
        return -ENOMEM;
    }
    if (copy_from_user(tmp_buff, in_cfg->buff, in_cfg->buff_size)) {
        mttcan_err("copy_from_user failed\n");
        ret = -EFAULT;
        goto free_buff;
    }
    ret = mttcan_get_can_config(can_id, sub_cmd, (void *)tmp_buff,
                                in_cfg->buff_size, (unsigned int *)out);
    if (ret != 0) {
        mttcan_err("can get fail.(ret = %d).\n", ret);
        ret = -EFAULT;
        goto free_buff;
    }

    if (copy_to_user(in_cfg->buff, tmp_buff, in_cfg->buff_size)) {
        mttcan_err("copy to user fail");
        ret = -EFAULT;
    }
free_buff:
    kfree(tmp_buff);

    (void)feature;
    return ret;
}