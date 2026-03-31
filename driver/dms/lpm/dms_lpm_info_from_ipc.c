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

#include <linux/fs.h>
#include <linux/of_gpio.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/reboot.h>
#include <linux/kallsyms.h>
#include <linux/errno.h>
#include <linux/jiffies.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>

#include "dms_lpm.h"
#include "dms_lpm_common.h"
#include "dms_lpm_info.h"
#include "devdrv_manager.h"
#include "dms_ipc_interface.h"
#include "devdrv_ipc.h"
#include "tsmng_interface.h"

int lpm_get_temperature_from_ipc(u32 dev_id, u32 core_id, u32 *temperature)
{
    int ret;
    u64 value;
    u32 len = sizeof(u64);

    ret = dms_lpm_id_check(dev_id, core_id);
    if (ret != 0) {
        dms_err("Parameter error. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    ret = lpm_get_info_from_ipc(dev_id, LPM_TEMPERATURE, core_id, (u8 *)&value, &len);
    if (ret != 0) {
        dms_err("Failed to obtain the power through the ipc."
            "(dev_id=%u; core_id=%u; ret=%d)\n", dev_id, core_id, ret);
        return ret;
    }

    if (core_id == DMS_VRD_TEMP) {
        *temperature = (u32)(value >> DMS_MOVE_OFFSET);
    } else {
        *temperature = (u32)value;
    }

    return 0;
}

int lpm_get_power_from_ipc(u32 dev_id, u32 core_id, u32 *power)
{
    int ret;
    u32 len = sizeof(u32);

    ret = dms_lpm_id_check(dev_id, core_id);
    if (ret != 0) {
        dms_err("Parameter error. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    ret = lpm_get_info_from_ipc(dev_id, LPM_POWER, LPM_SOC_ID, (u8 *)power, &len);
    if (ret != 0) {
        dms_err("Failed to obtain the power through the ipc."
            "(dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    *power /= DMS_PRECISION_CONVER;
    if (*power > UINT16_MAX) {
        dms_err("Power value is too large. (dev_id=%u; pwoer=%u)\n", dev_id, *power);
        return -EINVAL;
    }

    return 0;
}

int lpm_get_voltage_from_ipc(u32 dev_id, u32 core_id, u32 *voltage)
{
    int ret;
    u32 len = sizeof(u32);

    ret = dms_lpm_id_check(dev_id, core_id);
    if (ret != 0) {
        dms_err("Parameter error. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    ret = lpm_get_info_from_ipc(dev_id, LPM_VOLTAGE, LPM_SOC_ID, (u8 *)voltage, &len);
    if (ret != 0) {
        dms_err("Failed to obtain the voltage through the ipc."
            "(dev_id=%u; core_id=%u; ret=%d)\n", dev_id, core_id, ret);
        return ret;
    }

    return 0;
}

int lpm_get_frequency_from_ipc(u32 dev_id, u32 core_id, u32 *frequency)
{
    int ret;
    u32 len = sizeof(u32);

    ret = dms_lpm_id_check(dev_id, core_id);
    if (ret != 0) {
        dms_err("Parameter error. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    ret = lpm_get_info_from_ipc(dev_id, LPM_FREQUENCY, core_id, (u8 *)frequency, &len);
    if (ret != 0) {
        dms_err("Failed to obtain the frequency from the IMU through the ipc."
            "(dev_id=%u; core_id=%u; ret=%d)\n", dev_id, core_id, ret);
        return ret;
    }

    return 0;
}

int lpm_get_tsensor_from_ipc(u32 dev_id, u32 core_id, u32 *value)
{
    int ret;
    int type;
    int temp;

    if (core_id == DMS_SOC_TEMP_ID) {
        type = LPM_SOC_ID;
#ifdef CFG_DIE_HBM_TEMP
    } else if (core_id == DMS_N_DIE_TEMP_ID) {
        type = LPM_N_DIE_ID;
    } else if (core_id == DMS_HBM_TEMP_ID) {
        type = LPM_HBM_ID;
#endif
    } else {
        dms_err("Invaild core id. (dev_id=%u; core_id=%u)\n", dev_id, core_id);
        return DRV_ERROR_NOT_SUPPORT;
    }

    ret = lpm_get_temperature_from_ipc(dev_id, type, &temp);
    if (ret != 0) {
        dms_err("Get soc sensor info failed. (dev_id=%u; core_id=%u; ret=%d)\n", dev_id, core_id, ret);
        return ret;
    }

    *value = (u32)temp;
    return 0;
}

int lpm_get_ddr_freq_from_ipc(u32 dev_id, u32 *freq)
{
    int ret;
    u32 len = sizeof(u16);

    ret = lpm_get_info_from_ipc(dev_id, LPM_FREQUENCY, LPM_DDR_ID, (u8 *)freq, &len);
    if (ret) {
        dms_err("Lpm get info from ipc failed. (dev_id=%u; core_id=%d; ret=%d)\n", dev_id, LPM_DDR_ID, ret);
        return ret;
    }

    return 0;
}

int lpm_get_thermal_threshold_from_ipc(u32 dev_id, u32 *temp)
{
    int ret;
    u32 len = sizeof(u16);

    ret = lpm_get_info_from_ipc(dev_id, LPM_TEMPERATURE, LPM_THERMAL_THRESHOLD_ID, (u8 *)temp, &len);
    if (ret) {
        dms_err("Lpm get info from ipc failed. (dev_id=%u; core_id=%d; ret=%d)\n",
            dev_id, LPM_THERMAL_THRESHOLD_ID, ret);
        return ret;
    }

    return 0;
}

int lpm_get_soc_temp_from_ipc(u32 dev_id, u32 *temp)
{
    int ret;
    u32 len = sizeof(u8);

    ret = lpm_get_info_from_ipc(dev_id, LPM_TEMPERATURE, LPM_SOC_ID, (u8 *)temp, &len);
    if (ret) {
        dms_err("Lpm get info from ipc failed. (dev_id=%u; core_id=%d; ret=%d)\n", dev_id, LPM_SOC_ID, ret);
        return ret;
    }

    return 0;
}

int lpm_get_lp_status_from_ipc(u32 dev_id, u32 core_id, u32 *lp_status)
{
    int ret;
    u32 len = sizeof(struct devdrv_lp_status);

    ret = dms_lpm_id_check(dev_id, core_id);
    if (ret != 0) {
        dms_err("Parameter error. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    ret = lpm_get_info_from_ipc(dev_id, LPM_STATUS, core_id, (u8 *)lp_status, &len);
    if (ret != 0) {
        dms_err("Failed to obtain the power through the ipc."
            "(dev_id=%u; core_id=%u; ret=%d)\n", dev_id, core_id, ret);
        return ret;
    }

    return 0;
}

int lpm_get_vol_cur_from_ipc(u32 dev_id, u32 core_id, u32 *vol_cur)
{
    int ret;
    u32 len = sizeof(struct devdrv_dev_info);
    struct devdrv_dev_info *usr_vol_cur = NULL;

    ret = dms_lpm_id_check(dev_id, core_id);
    if (ret != 0) {
        dms_err("Parameter error. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }
    usr_vol_cur = (struct devdrv_dev_info *)vol_cur;
    ret = lpm_get_info_from_ipc(dev_id, usr_vol_cur->info_type, core_id, (u8 *)vol_cur, &len);
    if (ret != 0) {
        dms_err("Failed to obtain the power through the ipc."
            "(dev_id=%u; core_id=%u; ret=%d)\n", dev_id, core_id, ret);
        return ret;
    }

    return 0;
}

int lpm_get_acg_from_ipc(u32 dev_id, u32 core_id, u32 *acg_data)
{
    int ret;
    u32 len = sizeof(u32);

    ret = dms_lpm_id_check(dev_id, core_id);
    if (ret != 0) {
        dms_err("Parameter error. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    ret = lpm_get_info_from_ipc(dev_id, LPM_ACG, core_id, (u8 *)acg_data, &len);
    if (ret != 0) {
        dms_err("Failed to obtain the power through the ipc."
            "(dev_id=%u; core_id=%u; ret=%d)\n", dev_id, core_id, ret);
        return ret;
    }

    return 0;
}

int lpm_get_ddr_temp_from_ipc(u32 dev_id, u32 core_id, u32 *ddr_temp)
{
    int ret;
    u32 len = DDR_TEMP_TEMPERATURE_SIZE;

    ret = dms_lpm_id_check(dev_id, core_id);
    if (ret != 0) {
        dms_err("Parameter error. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    ret = lpm_get_info_from_ipc(dev_id, LPM_TEMP_DDR, core_id, (u8 *)ddr_temp, &len);
    if (ret != 0) {
        dms_err("Failed to obtain the power through the ipc."
            "(dev_id=%u; core_id=%u; ret=%d)\n", dev_id, core_id, ret);
        return ret;
    }

    return 0;
}

int lpm_get_temp_thold_from_ipc(u32 dev_id, u32 core_id, u32 *temp_thold)
{
    int ret;
    u32 len = sizeof(struct devdrv_dev_info);
    struct devdrv_dev_info *temp_thold_arg = NULL;

    ret = dms_lpm_id_check(dev_id, core_id);
    if (ret != 0) {
        dms_err("Parameter error. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }
    temp_thold_arg = (struct devdrv_dev_info *)temp_thold;
    ret = lpm_get_info_from_ipc(dev_id, temp_thold_arg->info_type, core_id, (u8 *)temp_thold, &len);
    if (ret != 0) {
        dms_err("Failed to obtain the power through the ipc."
            "(dev_id=%u; core_id=%u; ret=%d)\n", dev_id, core_id, ret);
        return ret;
    }

    return 0;
}

#ifdef CFG_REATURE_GET_INFO_FORM_IPC
int lpm_set_temp_thold_from_ipc(u32 dev_id, u32 core_id, u32 *temp_state)
{
    int ret;
    u32 len = sizeof(struct devdrv_dev_info);
    struct devdrv_dev_info *temp_state_arg = NULL;

    ret = dms_lpm_id_check(dev_id, core_id);
    if (ret != 0) {
        dms_err("Parameter error. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }
    temp_state_arg = (struct devdrv_dev_info *)temp_state;
    ret = lpm_set_info_to_ipc(dev_id, temp_state_arg->info_type, core_id, (u8 *)temp_state, &len);
    if (ret != 0) {
        dms_err("Failed to obtain the power through the ipc."
            "(dev_id=%u; core_id=%u; ret=%d)\n", dev_id, core_id, ret);
        return ret;
    }

    return 0;
}
#endif