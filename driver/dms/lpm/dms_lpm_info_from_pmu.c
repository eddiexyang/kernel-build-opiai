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
#include <linux/hisi-spmi.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>

#include "hisi_pmic.h"
#include "dms_lpm.h"
#include "dms_lpm_common.h"
#include "dms_lpm_info.h"
#include "dms_ipc_interface.h"

#define PMU_BUCK6 6
#define PMU_BUCK0 0
#define SECOND_PMU_ID_A 0x0B
#define SECOND_PMU_ID_B 0x02

int lpm_get_voltage_from_pmu(u32 dev_id, u32 core_id, u32 *voltage)
{
#ifdef CONFIG_HISI_SPMI_PMIC_VOLTAGE
    int ret;

    ret = dms_lpm_id_check(dev_id, core_id);
    if (ret != 0) {
        dms_err("Parameter error. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    switch (core_id) {
        case LPM_CLUSTER_ID:
            ret = get_main_pmu_buck_volt(PMU_BUCK6, voltage);
            break;
        case LPM_PERI_ID:
            ret = get_main_pmu_buck_volt(PMU_BUCK0, voltage);
            break;
        case LPM_AICORE0_ID:
            ret = get_second_pmu_buck_volt(SECOND_PMU_ID_A, PMU_BUCK0, voltage);
            break;
        case LPM_AICORE1_ID:
            ret = get_second_pmu_buck_volt(SECOND_PMU_ID_B, PMU_BUCK0, voltage);
            break;
        default:
            dms_err("Not support core id. (dev_id=%u; core_id=%d)\n", dev_id, core_id);
            return -EOPNOTSUPP;
    }

    if (ret != 0) {
        dms_err("Get voltage from pmu failed. (dev_id=%u; core_id=%d; ret=%d)\n", dev_id, core_id, ret);
        return ret;
    }
    dms_debug("Get voltage from pmu success.\n");

    return 0;
#else
    dms_info("CONFIG_HISI_SPMI_PMIC_VOLTAGE is not defined "
        "in linux kernel, can not get voltage.\n");

    return 0;
#endif
}

int lpm_get_temp_ntc(u32 *temp)
{
    int ret = 0;
    int i;

    for (i = 0; i < THERMISTOR_NUM; i++) {
        ret = ntc_read_temp(i + CHANNAL_OFFSET, (int *)&temp[i]);
        if (ret != 0) {
            dms_err("ntc_read_temp error. (i=%d; ret=%d)\n", i, ret);
            return -ENODATA;
        }
    }

    return 0;
}
