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

#include "dms_lpm.h"
#include "dms_define.h"
#include "dms_template.h"
#include "dms_cmd_def.h"
#include "urd_acc_ctrl.h"
#include "dms_lpm_info.h"
#include "dms_lpm_common.h"
#include "dms_ipc_interface.h"
#include "devdrv_info.h"

#ifdef CFG_FEATURE_DEVICE_DEVID_CONVERT
#include "devdrv_manager_container.h"
#include "devdrv_manager.h"
#endif

#ifdef CFG_FEATURE_PASS_THROUGH_MCU_BY_IMU
#include "dms_lpm_pass_through_mcu.h"
#endif

BEGIN_DMS_MODULE_DECLARATION(DMS_LPM_CMD_NAME)
BEGIN_FEATURE_COMMAND()
#ifndef CFG_SOC_RECONSITUTION_TEMPORARY
ADD_FEATURE_COMMAND(DMS_LPM_CMD_NAME, DMS_MAIN_CMD_LPM, DMS_SUBCMD_GET_TEMPERATURE, NULL, NULL,
                    DMS_SUPPORT_ALL, dms_lpm_get_temperature)
#endif
ADD_FEATURE_COMMAND(DMS_LPM_CMD_NAME, DMS_MAIN_CMD_LPM, DMS_SUBCMD_GET_POWER, NULL, NULL,
                    DMS_SUPPORT_ALL, dms_lpm_get_power)
ADD_FEATURE_COMMAND(DMS_LPM_CMD_NAME, DMS_MAIN_CMD_LPM, DMS_SUBCMD_GET_VOLTAGE, NULL, NULL,
                    DMS_SUPPORT_ALL, dms_lpm_get_voltage)
#ifndef CFG_SOC_RECONSITUTION_TEMPORARY
ADD_FEATURE_COMMAND(DMS_LPM_CMD_NAME, DMS_MAIN_CMD_LPM, DMS_SUBCMD_GET_FREQUENCY, NULL, NULL,
                    DMS_SUPPORT_ALL, dms_lpm_get_frequency)
#elif defined CFG_SOC_GET_HBM_FREQ_TEMPORARY
ADD_FEATURE_COMMAND(DMS_LPM_CMD_NAME, DMS_MAIN_CMD_LPM, DMS_SUBCMD_GET_FREQUENCY, "frequency_of_hbm", NULL,
                    DMS_SUPPORT_ALL, dms_lpm_get_frequency)
#endif
ADD_FEATURE_COMMAND(DMS_LPM_CMD_NAME, DMS_MAIN_CMD_LPM, DMS_SUBCMD_GET_TSENSOR, NULL, NULL,
                    DMS_SUPPORT_ALL, dms_lpm_get_tsensor)
ADD_FEATURE_COMMAND(DMS_LPM_CMD_NAME, DMS_MAIN_CMD_LPM, DMS_SUBCMD_GET_MAX_FREQUENCY, NULL, NULL,
                    DMS_SUPPORT_ALL, dms_lpm_get_max_frequency)
#ifdef CFG_FEATURE_PASS_THROUGH_MCU_BY_IMU
ADD_FEATURE_COMMAND(DMS_LPM_CMD_NAME, DMS_MAIN_CMD_LPM, DMS_SUBCMD_PASS_THROUGTH_MCU, NULL, "dmp_daemon",
                    DMS_SUPPORT_ALL, dms_lpm_pass_through_mcu)
#endif

#ifdef CFG_REATURE_GET_INFO_FORM_IPC
ADD_FEATURE_COMMAND(DMS_LPM_CMD_NAME, DMS_MAIN_CMD_LPM, DMS_SUBCMD_GET_LP_STATUS, NULL, NULL,
                    DMS_SUPPORT_ALL, dms_lpm_get_lp_status)
ADD_FEATURE_COMMAND(DMS_LPM_CMD_NAME, DMS_GET_GET_DEVICE_INFO_CMD, ZERO_CMD, FILTER_DEVICE_INFO, "dmp_daemon",
                    DMS_SUPPORT_ALL, dms_lpm_get_device_info)
ADD_FEATURE_COMMAND(DMS_LPM_CMD_NAME, DMS_GET_GET_DEVICE_INFO_CMD, ZERO_CMD, FILTER_TEMP_INFO, "dmp_daemon",
                    DMS_SUPPORT_ALL, dms_lpm_get_temp_info)
ADD_FEATURE_COMMAND(DMS_LPM_CMD_NAME, DMS_GET_SET_DEVICE_INFO_CMD, ZERO_CMD, FILTER_TEMP_INFO, "dmp_daemon",
                    DMS_SUPPORT_ALL, dms_lpm_set_temp_info)
#endif
END_FEATURE_COMMAND()
END_MODULE_DECLARATION()

int dms_lpm_init(void)
{
    CALL_INIT_MODULE(DMS_LPM_CMD_NAME);
    (void)lpm_scan_init();

    return 0;
}

void dms_lpm_uninit(void)
{
    CALL_EXIT_MODULE(DMS_LPM_CMD_NAME);
    (void)lpm_scan_uninit();
}

STATIC int dms_lpm_para_check(char *in, u32 in_len, u32 correct_in_len,
    char *out, u32 out_len, u32 correct_out_len)
{
    if ((in == NULL) || (in_len != correct_in_len)) {
        dms_err("Input char is NULL or in_len is wrong. (in_len=%u; correct_in_len=%u)\n",
            in_len, correct_in_len);
        return -EINVAL;
    }

    if ((out == NULL) || (out_len != correct_out_len)) {
        dms_err("Output char is NULL or out_len is wrong. (out_len=%u; correct_out_len=%u)\n",
            out_len, correct_out_len);
        return -EINVAL;
    }

    return 0;
}

STATIC int dms_lpm_get_lp_info(u32 dev_id, u32 cmd_type, u32 core_id, u32 *value)
{
    int ret;
    struct lpm_scan_list *item = NULL;

    if (dev_id >= LPM_DEVICE_NUM_MAX) {
        dms_err("Invalid device id. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    item = find_operate_info_str(cmd_type, core_id);
    if (item == NULL) {
        dms_err("Get handle struct failed. (dev_id=%u; core_id=%u)\n", dev_id, core_id);
        return -EOPNOTSUPP;
    }

    if (item->scan_flag == 0) {
        ret = item->get_value_handle(dev_id, core_id, value);
        if (ret != 0) {
            dms_err("Dms get lp info failed. (dev_id=%u; core_id=%u; ret=%d)\n", dev_id, core_id, ret);
            return ret;
        }
    } else {
        *value = item->pf_value[dev_id];
    }

    return 0;
}

#ifndef DRV_SOFT_UT
STATIC int dms_trans_and_check_id(u32 logical_dev_id, u32 *pf_id, u32 *vfid)
{
#ifdef CFG_FEATURE_DEVICE_DEVID_CONVERT
    if (devdrv_manager_container_logical_id_to_physical_id(logical_dev_id, pf_id, vfid) != 0) {
        dms_err("Failed to transfer logical ID to physical ID. (dev_id=%u)\n", logical_dev_id);
        return -EINVAL;
    }

    /* It is to get information from PF. */
    if (VDAVINCI_IS_VDEV(*pf_id)) {
        *pf_id = VDAVINCI_GET_PFID(*pf_id);
    }
#else
    *pf_id = logical_dev_id;
    *vfid = 0;
#endif
    return 0;
}
#endif

int dms_lpm_get_temperature(void *feature, char *in, u32 in_len, char *out, u32 out_len)
{
    struct dms_lpm_info_in *lpm_info = NULL;
    int ret;
    u32 temperature;
    u32 pf_id = DEVDRV_MAX_DAVINCI_NUM + 1, vfid = 0;

    ret = dms_lpm_para_check(in, in_len, sizeof(struct dms_lpm_info_in), out, out_len, sizeof(int));
    if (ret != 0) {
        dms_err("Para check failed.\n");
        return ret;
    }

    lpm_info = (struct dms_lpm_info_in *)in;
    ret = dms_trans_and_check_id(lpm_info->dev_id, &pf_id, &vfid);
    if (ret != 0) {
        dms_err("Failed to transfer logical ID to physical ID. (dev_id=%u; ret=%d)\n", lpm_info->dev_id, ret);
        return ret;
    }

    ret = dms_lpm_get_lp_info(pf_id, DMS_SUBCMD_GET_TEMPERATURE, lpm_info->core_id, &temperature);
    if (ret != 0) {
        dms_err("Dms get temperature failed. (dev_id=%u; core_id=%u; ret=%d)\n",
            lpm_info->dev_id, lpm_info->core_id, ret);
        return ret;
    }

    *(int *)out = (int)temperature;

    return 0;
}

int dms_lpm_get_power(void *feature, char *in, u32 in_len, char *out, u32 out_len)
{
    struct dms_lpm_info_in *lpm_info = NULL;
    int ret;
    u32 power;
    u32 pf_id = DEVDRV_MAX_DAVINCI_NUM + 1, vfid = 0;

    ret = dms_lpm_para_check(in, in_len, sizeof(struct dms_lpm_info_in), out, out_len, sizeof(u32));
    if (ret != 0) {
        dms_err("Para check failed.\n");
        return ret;
    }

    lpm_info = (struct dms_lpm_info_in *)in;
    ret = dms_trans_and_check_id(lpm_info->dev_id, &pf_id, &vfid);
    if (ret != 0) {
        dms_err("Failed to transfer logical ID to physical ID. (dev_id=%u; ret=%d)\n", lpm_info->dev_id, ret);
        return ret;
    }

    ret = dms_lpm_get_lp_info(pf_id, DMS_SUBCMD_GET_POWER, lpm_info->core_id, &power);
    if (ret != 0) {
        dms_err("Dms get power failed. (dev_id=%u; core_id=%u; ret=%d)\n",
            lpm_info->dev_id, lpm_info->core_id, ret);
        return ret;
    }

    *(u32 *)out = power;

    return 0;
}

int dms_lpm_get_voltage(void *feature, char *in, u32 in_len, char *out, u32 out_len)
{
    struct dms_lpm_info_in *lpm_info = NULL;
    int ret;
    u32 voltage;
    u32 pf_id = DEVDRV_MAX_DAVINCI_NUM + 1, vfid = 0;

    ret = dms_lpm_para_check(in, in_len, sizeof(struct dms_lpm_info_in), out, out_len, sizeof(u32));
    if (ret != 0) {
        dms_err("Para check failed.\n");
        return ret;
    }

    lpm_info = (struct dms_lpm_info_in *)in;
    ret = dms_trans_and_check_id(lpm_info->dev_id, &pf_id, &vfid);
    if (ret != 0) {
        dms_err("Failed to transfer logical ID to physical ID. (dev_id=%u; ret=%d)\n", lpm_info->dev_id, ret);
        return ret;
    }

    ret = dms_lpm_get_lp_info(pf_id, DMS_SUBCMD_GET_VOLTAGE, lpm_info->core_id, &voltage);
    if (ret != 0) {
        dms_err("Dms get voltage failed. (dev_id=%u; core_id=%u; ret=%d)\n",
            lpm_info->dev_id, lpm_info->core_id, ret);
        return ret;
    }

    *(u32 *)out = voltage;

    return 0;
}

int dms_lpm_get_frequency(void *feature, char *in, u32 in_len, char *out, u32 out_len)
{
    struct dms_lpm_info_in *lpm_info = NULL;
    int ret;
    u32 frequency;
    u32 pf_id = DEVDRV_MAX_DAVINCI_NUM + 1, vfid = 0;

    ret = dms_lpm_para_check(in, in_len, sizeof(struct dms_lpm_info_in), out, out_len, sizeof(u32));
    if (ret != 0) {
        dms_err("Para check failed.\n");
        return ret;
    }

    lpm_info = (struct dms_lpm_info_in *)in;
    ret = dms_trans_and_check_id(lpm_info->dev_id, &pf_id, &vfid);
    if (ret != 0) {
        dms_err("Failed to transfer logical ID to physical ID. (dev_id=%u; ret=%d)\n", lpm_info->dev_id, ret);
        return ret;
    }

    ret = dms_lpm_get_lp_info(pf_id, DMS_SUBCMD_GET_FREQUENCY, lpm_info->core_id, &frequency);
    if (ret != 0) {
        dms_err("Dms get frequency failed. (dev_id=%u; core_id=%u; ret=%d)\n",
            lpm_info->dev_id, lpm_info->core_id, ret);
        return ret;
    }

    *(u32 *)out = frequency;

    return 0;
}

int dms_lpm_get_max_frequency(void *feature, char *in, u32 in_len, char *out, u32 out_len)
{
    struct dms_lpm_info_in *lpm_info = NULL;
    int ret;
    u32 max_freq;
    u32 pf_id = DEVDRV_MAX_DAVINCI_NUM + 1, vfid = 0;

    ret = dms_lpm_para_check(in, in_len, sizeof(struct dms_lpm_info_in), out, out_len, sizeof(u32));
    if (ret != 0) {
        dms_err("Para check failed.\n");
        return ret;
    }

    lpm_info = (struct dms_lpm_info_in *)in;
    ret = dms_trans_and_check_id(lpm_info->dev_id, &pf_id, &vfid);
    if (ret != 0) {
        dms_err("Failed to transfer logical ID to physical ID. (dev_id=%u; ret=%d)\n", lpm_info->dev_id, ret);
        return ret;
    }

    ret = dms_lpm_get_lp_info(pf_id, DMS_SUBCMD_GET_MAX_FREQUENCY, lpm_info->core_id, &max_freq);
    if (ret != 0) {
        dms_err("Dms get max frequency failed. (dev_id=%u; core_id=%u; ret=%d)\n",
            lpm_info->dev_id, lpm_info->core_id, ret);
        return ret;
    }

    *(u32 *)out = max_freq;

    return 0;
}

int dms_lpm_get_lp_status(void *feature, char *in, u32 in_len, char *out, u32 out_len)
{
    int ret;
    struct dms_lpm_info_in *lpm_info = NULL;
    struct devdrv_lp_status lp_status = {0};
    u32 pf_id = DEVDRV_MAX_DAVINCI_NUM + 1, vfid = 0;

    ret = dms_lpm_para_check(in, in_len, sizeof(struct dms_lpm_info_in), out, out_len, sizeof(struct devdrv_lp_status));
    if (ret != 0) {
        dms_err("Para check failed.\n");
        return ret;
    }

    lpm_info = (struct dms_lpm_info_in *)in;
    ret = dms_trans_and_check_id(lpm_info->dev_id, &pf_id, &vfid);
    if (ret != 0) {
        dms_err("Failed to transfer logical ID to physical ID. (dev_id=%u; ret=%d)\n", lpm_info->dev_id, ret);
        return ret;
    }

    ret = dms_lpm_get_lp_info(pf_id, DMS_SUBCMD_GET_LP_STATUS, lpm_info->core_id, (u32 *)&lp_status);
    if (ret != 0) {
        dms_err("Dms get lp status failed. (dev_id=%u; ret=%d)\n",
            lpm_info->dev_id, ret);
        return ret;
    }

    *(struct devdrv_lp_status *)out = lp_status;

    return 0;
}

STATIC int dms_lpm_get_vol_cur_from_lp(u32 sub_cmd, struct ioctl_arg *user_arg, u32 in_len, u32 *out_len)
{
    int ret;
    u32 cmd_type;
    u32 pf_id = DEVDRV_MAX_DAVINCI_NUM + 1, vfid = 0;
    struct devdrv_dev_info vol_cur = {0};

    ret = dms_trans_and_check_id(user_arg->dev_id, &pf_id, &vfid);
    if (ret != 0) {
        dms_err("Failed to transfer logical ID to physical ID. (dev_id=%u; ret=%d)\n", user_arg->dev_id, ret);
        return ret;
    }

    cmd_type = sub_cmd + DMS_SUBCMD_GET_AIC_VOL_CUR;  // DMS_SUBCMD_GET_AIC_VOL_CUR
    vol_cur.info_type = sub_cmd + LPM_AIC_VOL_CUR;    // LPM_AIC_VOL_CUR

    ret = dms_lpm_get_lp_info(pf_id, cmd_type, user_arg->type, (u32 *)&vol_cur);
    if (ret != 0) {
        dms_err("Dms get voltage current failed. (dev_id=%u; ret=%d)\n",
            user_arg->dev_id, ret);
        return ret;
    }

    user_arg->data1 = vol_cur.data1;
    user_arg->data2 = vol_cur.data2;
    user_arg->data3 = vol_cur.data3;

    *out_len = (u32)(sizeof(u32) + sizeof(u32) + sizeof(int));

    return 0;
}

STATIC int dms_lpm_get_acg_from_lp(u32 sub_cmd, struct ioctl_arg *user_arg, u32 in_len, u32 *out_len)
{
    int ret;
    u32 cmd_type;
    u32 pf_id = DEVDRV_MAX_DAVINCI_NUM + 1, vfid = 0;
    u32 acg_data;

    ret = dms_trans_and_check_id(user_arg->dev_id, &pf_id, &vfid);
    if (ret != 0) {
        dms_err("Failed to transfer logical ID to physical ID. (dev_id=%u; ret=%d)\n", user_arg->dev_id, ret);
        return ret;
    }

    cmd_type = sub_cmd + DMS_SUBCMD_GET_AIC_VOL_CUR;

    ret = dms_lpm_get_lp_info(pf_id, cmd_type, user_arg->type, &acg_data);
    if (ret != 0) {
        dms_err("Dms get acg data failed. (dev_id=%u; ret=%d)\n",
            user_arg->dev_id, ret);
        return ret;
    }

    user_arg->data1 = acg_data;
    if (user_arg->data1 == 0xFFFFFFFFU) { // LPM errcoed, lp not support get acg on ASCEND610
        dms_err("The ipc communication is normal, lp not support get acg on this chip.\n");
        return -EOPNOTSUPP;
    }

    *out_len = (u32)sizeof(u32);

    return 0;
}

int dms_lpm_get_device_info(void *feature, char *in, u32 in_len, char *out, u32 out_len)
{
    int ret;
    u32 result_len = 0;
    struct ioctl_arg user_arg = {0};
    struct dms_get_device_info_in *in_cfg = NULL;
    struct dms_get_device_info_out *cfg_out = NULL;

    ret = dms_lpm_para_check(in, in_len, sizeof(struct dms_get_device_info_in),
                             out, out_len, sizeof(struct dms_get_device_info_out));
    if (ret != 0) {
        dms_err("Para check failed.\n");
        return ret;
    }

    in_cfg = (struct dms_get_device_info_in *)in;
    cfg_out = (struct dms_get_device_info_out *)out;
    if (in_cfg->buff_size != sizeof(struct ioctl_arg)) {
        dms_err("Buff size is error, Para check failed.\n");
        return -EINVAL;
    }

    ret = copy_from_user_safe(&user_arg, (void *)((uintptr_t)in_cfg->buff), sizeof(struct ioctl_arg));
    if (ret != 0) {
        dms_err("copy_from_user_safe failed. ret=%d\n", ret);
        return ret;
    }

    switch (in_cfg->sub_cmd) {
        case DSMI_LP_SUB_CMD_AICORE_VOLTAGE_CURRENT:
        case DSMI_LP_SUB_CMD_HYBIRD_VOLTAGE_CURRENT:
        case DSMI_LP_SUB_CMD_TAISHAN_VOLTAGE_CURRENT:
        case DSMI_LP_SUB_CMD_DDR_VOLTAGE_CURRENT:
            ret = dms_lpm_get_vol_cur_from_lp(in_cfg->sub_cmd, &user_arg, in_cfg->buff_size, &result_len);
            break;

        case DSMI_LP_SUB_CMD_ACG:
            ret = dms_lpm_get_acg_from_lp(in_cfg->sub_cmd, &user_arg, in_cfg->buff_size, &result_len);
            break;

        default:
            dms_err("dsmi sub_cmd is illegal! sub_cmd=%u\n", in_cfg->sub_cmd);
            return -EINVAL;
    }

    if (ret != 0) {
        dms_err("get info from LPM failed, sub_cmd=%u, ret=%d.\n", in_cfg->sub_cmd, ret);
        return ret;
    }

    ret = copy_to_user_safe((void *)(uintptr_t)in_cfg->buff, &user_arg, sizeof(struct ioctl_arg));
    if (ret != 0) {
        dms_err("copy to user failed, ret=%d\n", ret);
        return ret;
    }

    cfg_out->out_size = result_len;

    return 0;
}

u32 g_cmd_type[] = { DMS_SUBCMD_GET_TEMP_DDR, DMS_SUBCMD_GET_DDR_THOLD,
                     DMS_SUBCMD_GET_SOC_THOLD, DMS_SUBCMD_GET_SOC_MIN_THOLD };
u32 g_info_type[] = { LPM_TEMP_DDR, LPM_DDR_THOLD, LPM_SOC_THOLD, LPM_SOC_MIN_THOLD };

#define DEVDRV_DDR_TEMP_IPC_OFFSET 8
STATIC int dms_lpm_get_ddr_temperature(void *in_buf, u32 in_len, u32 *out_len)
{
    int ret;
    u32 pf_id = DEVDRV_MAX_DAVINCI_NUM + 1, vfid = 0;
    struct dms_ddr_temp_arg user_arg = {0};
    struct dms_ipc_lpm_msg ack_msg = {0};

    if (in_len != sizeof(struct dms_ddr_temp_arg) || in_buf == NULL) {
        dms_err("In buff is NULL or buff size error, Para check failed.\n");
        return -EINVAL;
    }
    ret = copy_from_user_safe(&user_arg, (void *)((uintptr_t)in_buf), sizeof(struct dms_ddr_temp_arg));
    if (ret != 0) {
        dms_err("copy_from_user_safe failed. ret=%d\n", ret);
        return ret;
    }
    ret = dms_trans_and_check_id(user_arg.dev_id, &pf_id, &vfid);
    if (ret != 0) {
        dms_err("Failed to transfer logical ID to physical ID. (dev_id=%u; ret=%d)\n", user_arg.dev_id, ret);
        return ret;
    }
    ret = dms_lpm_get_lp_info(pf_id, DMS_SUBCMD_GET_TEMP_DDR, user_arg.core_id, (u32 *)&ack_msg.cmd_data0);
    if (ret != 0) {
        dms_err("Dms get ddr temperature failed. (dev_id=%u; ret=%d)\n",
            user_arg.dev_id, ret);
        return ret;
    }

    if (user_arg.commoninfo_len < sizeof(ack_msg) - DEVDRV_DDR_TEMP_IPC_OFFSET) {
        dms_err("struct length Not compatible, input lenght = %d, need length %d\n",
            user_arg.commoninfo_len, (u32)sizeof(ack_msg) - DEVDRV_DDR_TEMP_IPC_OFFSET);
        return -EINVAL;
    }
    ret = copy_to_user_safe(user_arg.commoninfo, &ack_msg.cmd_data0, sizeof(ack_msg) - DEVDRV_DDR_TEMP_IPC_OFFSET);
    if (ret) {
        dms_err("copy commoninfo failed, ret = %d\n", ret);
        return ret;
    }
    user_arg.commoninfo_len = sizeof(ack_msg) - DEVDRV_DDR_TEMP_IPC_OFFSET;
    ret = copy_to_user_safe((void *)(uintptr_t)in_buf, &user_arg, sizeof(struct dms_ddr_temp_arg));
    if (ret != 0) {
        dms_err("copy to user failed, ret=%d\n", ret);
        return ret;
    }

    *out_len = user_arg.commoninfo_len;

    return 0;
}

STATIC int dms_lpm_get_temperature_threshold(u32 sub_cmd, void *in_buf, u32 in_len, u32 *out_len)
{
    int ret;
    u32 pf_id = DEVDRV_MAX_DAVINCI_NUM + 1, vfid = 0;
    struct ioctl_arg user_arg = {0};
    struct devdrv_dev_info temp_thold = {0};

    if (in_len != sizeof(struct ioctl_arg) || in_buf == NULL) {
        dms_err("In buff is NULL or buff size error, Para check failed.\n");
        return -EINVAL;
    }
    ret = copy_from_user_safe(&user_arg, (void *)((uintptr_t)in_buf), sizeof(struct ioctl_arg));
    if (ret != 0) {
        dms_err("copy_from_user_safe failed. ret=%d\n", ret);
        return ret;
    }
    ret = dms_trans_and_check_id(user_arg.dev_id, &pf_id, &vfid);
    if (ret != 0) {
        dms_err("Failed to transfer logical ID to physical ID. (dev_id=%u; ret=%d)\n", user_arg.dev_id, ret);
        return ret;
    }

    temp_thold.info_type = g_info_type[sub_cmd];
    ret = dms_lpm_get_lp_info(pf_id, g_cmd_type[sub_cmd], user_arg.type, (u32 *)&temp_thold);
    if (ret != 0) {
        dms_err("Dms get temperature threshold failed. (dev_id=%u; ret=%d)\n",
            user_arg.dev_id, ret);
        return ret;
    }

    if (sub_cmd == DSMI_TEMP_SUB_CMD_DDR_THOLD || sub_cmd == DSMI_TEMP_SUB_CMD_SOC_THOLD) {
        user_arg.data1 = temp_thold.data1;
    } else {
        user_arg.data1 = temp_thold.data3;
    }

    user_arg.data2 = sizeof(user_arg.data1);

    ret = copy_to_user_safe((void *)(uintptr_t)in_buf, &user_arg, sizeof(struct ioctl_arg));
    if (ret != 0) {
        dms_err("copy to user failed, ret=%d\n", ret);
        return ret;
    }

    *out_len = (u32)(sizeof(user_arg.data1) + sizeof(user_arg.data2));

    return 0;
}

int dms_lpm_get_temp_info(void *feature, char *in, u32 in_len, char *out, u32 out_len)
{
    int ret;
    u32 result_len = 0;
    struct dms_get_device_info_in *in_cfg = NULL;
    struct dms_get_device_info_out *cfg_out = NULL;
    ret = dms_lpm_para_check(in, in_len, sizeof(struct dms_get_device_info_in),
                             out, out_len, sizeof(struct dms_get_device_info_out));
    if (ret != 0) {
        dms_err("Para check failed.\n");
        return ret;
    }

    in_cfg = (struct dms_get_device_info_in *)in;
    cfg_out = (struct dms_get_device_info_out *)out;

    switch (in_cfg->sub_cmd) {
        case DSMI_SUB_CMD_TEMP_DDR:
            ret = dms_lpm_get_ddr_temperature(in_cfg->buff, in_cfg->buff_size, &result_len);
            break;
        case DSMI_TEMP_SUB_CMD_DDR_THOLD:
        case DSMI_TEMP_SUB_CMD_SOC_THOLD:
        case DSMI_TEMP_SUB_CMD_SOC_MIN_THOLD:
            ret = dms_lpm_get_temperature_threshold(in_cfg->sub_cmd, in_cfg->buff, in_cfg->buff_size, &result_len);
            break;
        default:
            dms_err("dsmi sub_cmd is illegal! sub_cmd=%u\n", in_cfg->sub_cmd);
            return -EINVAL;
    }
    if (ret != 0) {
        dms_err("get info from LPM failed, sub_cmd=%u, ret=%d.\n", in_cfg->sub_cmd, ret);
        return ret;
    }

    cfg_out->out_size = result_len;

    return 0;
}
#ifdef CFG_REATURE_GET_INFO_FORM_IPC
#define SET_TEMP_MAX_THRESHOLD 1U
#define SET_TEMP_MIN_THRESHOLD 2U
STATIC int dms_temp_state_set_data(u32 sub_cmd, struct ioctl_arg *arg)
{
    int ret;
    u32 pf_id = DEVDRV_MAX_DAVINCI_NUM + 1, vfid = 0;
    struct devdrv_dev_info temp_state = {0};

    ret = dms_trans_and_check_id(arg->dev_id, &pf_id, &vfid);
    if (ret != 0) {
        dms_err("Failed to transfer logical ID to physical ID. (dev_id=%u; ret=%d)\n", arg->dev_id, ret);
        return ret;
    }
    switch (sub_cmd) {
        case DSMI_TEMP_SUB_CMD_DDR_THOLD:
            temp_state.data1 = arg->data1;
            temp_state.info_type = LPM_SET_DDR_TEMP_THOLD;
            break;
        case DSMI_TEMP_SUB_CMD_SOC_THOLD:
            temp_state.data1 = SET_TEMP_MAX_THRESHOLD;
            temp_state.data3 = arg->data1;
            temp_state.info_type = LPM_SET_SOC_TEMP_THOLD;
            break;
        case DSMI_TEMP_SUB_CMD_SOC_MIN_THOLD:
            temp_state.data1 = SET_TEMP_MIN_THRESHOLD;
            temp_state.data3 = arg->data1;
            temp_state.info_type = LPM_SET_SOC_TEMP_THOLD;
            break;
        default:
            dms_err("dsmi sub_cmd is not support! sub_cmd=%u\n", sub_cmd);
            return -EINVAL;
    }

    ret = dms_lpm_get_lp_info(pf_id, DMS_SUBCMD_SET_TEMP_THOLD, arg->type, (u32 *)&temp_state);
    if (ret != 0) {
        dms_err("Dms set temperature threshold failed. (dev_id=%u; ret=%d)\n",
            arg->dev_id, ret);
        return ret;
    }

    return 0;
}

STATIC int dms_lpm_set_temp_state(u32 sub_cmd, const void *in_buf, u32 in_len)
{
    int ret;
    struct ioctl_arg user_arg = {0};

    if (in_len != sizeof(struct ioctl_arg) || in_buf == NULL) {
        dms_err("In buff is NULL or buff size error,(in size = %u, current size = %u).\n",
            in_len, (u32)sizeof(struct ioctl_arg));
        return -EINVAL;
    }
    ret = copy_from_user_safe(&user_arg, (void *)((uintptr_t)in_buf), in_len);
    if (ret != 0) {
        dms_err("copy_from_user_safe failed. ret=%d\n", ret);
        return ret;
    }

    return dms_temp_state_set_data(sub_cmd, &user_arg);
}

int dms_lpm_set_temp_info(void *feature, char *in, u32 in_len, char *out, u32 out_len)
{
    int ret;
    struct dms_set_device_info_in *in_cfg = NULL;

    if ((in == NULL) || (in_len != sizeof(struct dms_set_device_info_in))) {
        dms_err("Input char is NULL or in_len is wrong. (in_len=%u)\n", in_len);
        return -EINVAL;
    }

    /* check process whitelist */
    ret = devdrv_manager_check_call_process();
    if (ret != 0) {
        devdrv_drv_err("devdrv_manager_check_call_process ret = %d\n", ret);
        return ret;
    }
    in_cfg = (struct dms_set_device_info_in *)in;
    ret = dms_lpm_set_temp_state(in_cfg->sub_cmd, in_cfg->buff, in_cfg->buff_size);
    if (ret != 0) {
        dms_err("set temp state from LPM failed, sub_cmd=%u, ret=%d.\n", in_cfg->sub_cmd, ret);
        return ret;
    }

    return 0;
}
#endif

u32 dms_lpm_coreid_to_size(u32 core_id)
{
    u32 size;

    if (core_id == DMS_THERMAL_THRESHOLD_ID) {
        size = LPM_THERMAL_THRESHOLD_NUM * sizeof(char);
    } else if (core_id == DMS_NTC_TEMP_ID) {
        size = LPM_NTC_NUM * sizeof(int);
    } else if (core_id == DMS_AICORE_BASE_FREQ_ID || core_id == DMS_NPU_DDR_FREQ_ID) {
        size = sizeof(u16);
    } else if (core_id == DMS_FP_TEMP_ID || core_id == DMS_N_DIE_TEMP_ID || core_id == DMS_HBM_TEMP_ID) {
        size = sizeof(int);
    } else {
        size = sizeof(u8);
    }

    return size;
}

int dms_lpm_get_tsensor(void *feature, char *in, u32 in_len, char *out, u32 out_len)
{
    struct dms_lpm_info_in *lpm_info = NULL;
    int ret;
    u32 tsensor[LPM_NTC_NUM] = {0};
    u32 pf_id = DEVDRV_MAX_DAVINCI_NUM + 1, vfid = 0;

    if ((in == NULL) || (in_len != sizeof(struct dms_lpm_info_in))) {
        dms_err("Input char is NULL or in_len is wrong. (in_len=%u)\n", in_len);
        return -EINVAL;
    }

    lpm_info = (struct dms_lpm_info_in *)in;
    if ((out == NULL) || (out_len != dms_lpm_coreid_to_size(lpm_info->core_id))) {
        dms_err("Output char is NULL or out_len is wrong. (out_len=%u)\n", out_len);
        return -EINVAL;
    }

    ret = dms_trans_and_check_id(lpm_info->dev_id, &pf_id, &vfid);
    if (ret != 0) {
        dms_err("Failed to transfer logical ID to physical ID. (dev_id=%u; ret=%d)\n", lpm_info->dev_id, ret);
        return ret;
    }

    ret = dms_lpm_get_lp_info(pf_id, DMS_SUBCMD_GET_TSENSOR, lpm_info->core_id, tsensor);
    if (ret != 0) {
        dms_err("Dms get tsensor info failed. (dev_id=%u; core_id=%u; ret=%d)\n",
            lpm_info->dev_id, lpm_info->core_id, ret);
        return ret;
    }

    ret = memcpy_s(out, out_len, tsensor, dms_lpm_coreid_to_size(lpm_info->core_id));
    if (ret != 0) {
        dms_err("Memcpy failed. (dev_id=%u)\n", lpm_info->dev_id);
        return -EINVAL;
    }

    return 0;
}
