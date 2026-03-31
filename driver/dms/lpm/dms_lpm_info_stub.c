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
#include "dms_lpm_common.h"
#include "dms_ipc_interface.h"

#define LPM_INFO_VALUE 0x5A

int lpm_get_temperature_stub(u32 dev_id, u32 core_id, u32 *temperature)
{
    int ret;

    ret = dms_lpm_id_check(dev_id, core_id);
    if (ret != 0) {
        dms_err("Parameter error. (ret=%d)\n", ret);
        return ret;
    }

    *temperature = LPM_INFO_VALUE;
    return 0;
}

int lpm_get_power_stub(u32 dev_id, u32 core_id, u32 *power)
{
    int ret;

    ret = dms_lpm_id_check(dev_id, LPM_SOC_ID);
    if (ret != 0) {
        dms_err("Parameter error. (ret=%d)\n", ret);
        return ret;
    }

    *power = LPM_INFO_VALUE;
    return 0;
}

int lpm_get_voltage_stub(u32 dev_id, u32 core_id, u32 *voltage)
{
    int ret;

    ret = dms_lpm_id_check(dev_id, core_id);
    if (ret != 0) {
        dms_err("Parameter error. (ret=%d)\n", ret);
        return ret;
    }

    *voltage = LPM_INFO_VALUE;
    return 0;
}

int lpm_get_frequency_stub(u32 dev_id, u32 core_id, u32 *frequency)
{
    int ret;

    ret = dms_lpm_id_check(dev_id, core_id);
    if (ret != 0) {
        dms_err("Parameter error. (ret=%d)\n", ret);
        return ret;
    }

    *frequency = LPM_INFO_VALUE;
    return 0;
}

int lpm_get_tsensor_stub(u32 dev_id, u32 core_id, u32 *value)
{
    int ret;

    ret = dms_lpm_id_check(dev_id, LPM_SOC_ID);
    if (ret != 0) {
        dms_err("Parameter error. (ret=%d)\n", ret);
        return ret;
    }
    if (core_id >= DMS_INVALID_TSENSOR_ID) {
        dms_err("Invalid core_id. (core_id=%u)\n", core_id);
        return DRV_ERROR_NOT_SUPPORT;
    }

    *value = LPM_INFO_VALUE;
    return 0;
}

int lpm_get_max_frequency_stub(u32 dev_id, u32 core_id, u32 *frequency)
{
    int ret;

    ret = dms_lpm_id_check(dev_id, core_id);
    if (ret != 0) {
        dms_err("Parameter error. (ret=%d)\n", ret);
        return ret;
    }

    *frequency = LPM_INFO_VALUE;
    return 0;
}