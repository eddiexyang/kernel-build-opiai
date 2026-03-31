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
#include <asm/uaccess.h>
#include <linux/uaccess.h>

#include "dms_lpm.h"
#include "dms_lpm_common.h"
#include "dms_lpm_info.h"
#include "dms_ipc_interface.h"

int lpm_get_frequency_info(u32 dev_id, u32 core_id, u32 *frequency)
{
    int ret;

#ifdef CFG_LPM_INFO_FROM_SHAREMEM
    if (core_id != LPM_HBM_ID) {
        ret = lpm_get_frequency_from_sharemem(dev_id, core_id, frequency);
        if (ret != 0) {
            dms_err("Get frequency from share memory failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
            return ret;
        }
        return 0;
    }
#else
    if (core_id == LPM_CLUSTER_ID) {
        ret = lpm_get_ccpu_freq(dev_id, frequency);
        if (ret != 0) {
            dms_err("Get ccpu frequency failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
        }
        return ret;
    }

    if (core_id == LPM_AICORE1_ID) {
        ret = lpm_get_aicore_rated_frequency(dev_id, frequency);
        if (ret != 0) {
            dms_err("Get aicore1 frequency failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
        }
        return ret;
    }
#endif
    ret = lpm_get_frequency_from_ipc(dev_id, core_id, frequency);
    if (ret != 0) {
        dms_err("Lpm ger frequency failed from ipc. (dev_id=%u; core_id=%u; ret=%d)\n",
            dev_id, core_id, ret);
        return ret;
    }

    return 0;
}

#ifdef CFG_GET_TSENSOR_INFO
int lpm_get_tsensor_info(u32 dev_id, u32 core_id, u32 *value)
{
    int ret;
    u32 dev_num;

    ret = devdrv_get_devnum(&dev_num);
    if (ret != 0) {
        dms_err("Get device num failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }
    if (dev_id >= dev_num) {
        dms_err("Device id not valied. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    if (core_id <= DMS_AICORE_BASE_FREQ_ID) {
        ret = lpm_get_temp_from_sharemem(core_id, value);
    } else if (core_id == DMS_NPU_DDR_FREQ_ID) {
        ret = lpm_get_ddr_freq_from_ipc(dev_id, value);
    } else if (core_id == DMS_THERMAL_THRESHOLD_ID) {
        ret = lpm_get_thermal_threshold_from_ipc(dev_id, value);
    } else if (core_id == DMS_SOC_TEMP_ID) {
        ret = lpm_get_soc_temp_from_ipc(dev_id, value);
    } else if (core_id == DMS_NTC_TEMP_ID) {
        ret = lpm_get_temp_ntc(value);
    } else {
        dms_err("Invaild core id. (dev_id=%u; core_id=%d)\n", dev_id, core_id);
        ret = -EOPNOTSUPP;
    }

    if (ret != 0) {
        dms_err("Lpm get tsensor info failed. (dev_id=%u; core_id=%d)\n", dev_id, core_id);
        return ret;
    }

    return 0;
}
#endif
