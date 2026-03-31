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
#include <linux/errno.h>

#include <linux/device.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <asm/uaccess.h>

#ifdef CFG_FEATURE_DIE_ID_PMU
#include <hisi_pmic.h>
#endif

#include "soc_misc_common.h"
#include "soc_misc_init.h"
#include "soc_misc_adapt.h"
#include "soc_misc_chip_info.h"
#include "soc_misc_info.h"

#include "davinci_api.h"
#include "dms_define.h"
#include "devdrv_common.h"
#include "devdrv_manager_comm.h"
#include "ascend_kernel_hal.h"
#include "soc_spec.h"

#define DIE_INFO_MAP_SIZE 4096UL

int soc_misc_get_soc_dieid(dms_soc_die_id_t *soc_dieid)
{
    int ret;
    int count;
    unsigned int value;

#ifdef CFG_FEATURE_DIE_ID_PMU
    if (soc_dieid == NULL) {
        soc_misc_drv_err("Input die id is null.\n");
        return -EINVAL;
    }

    for (count = 0; count < PMU_DIEID_GET_VALUE; count++) {
        ret = soc_misc_reg_op(REG_OP_TYPE_RD, SYS_CTRL_BASE_ADDR,
            (unsigned long)(SYS_CTRL_SC_DIE_ID0_OFFSET + (((long int)count) * SYS_CTRL_SC_DIE_ID_REG_OFFSET)),
            SOC_DIEID_MAP_SIZE, &value);
        if (ret < 0) {
            soc_misc_drv_err("Read register for soc die id failed. (count=%d)\n", count);
            return ret;
        }
        soc_dieid->data[count] = value;
    }
#else
    unsigned long dieid_addr = DIE_ID_ADDR;
    devdrv_hardware_info_t hardware_info = {0};

    if (soc_dieid == NULL) {
        soc_misc_drv_err("Input die id is null.\n");
        return -EINVAL;
    }

#ifdef CFG_FEATURE_DIE_ID_N_V
    if (soc_dieid->pmu_dieid_type == DEVDRV_SOC_N_DIE_ID) {
        dieid_addr = N_DIE_ID_ADDR;
    }
#endif

    ret = hal_kernel_get_hardware_info(soc_dieid->dev_id, &hardware_info);
    if (ret != 0) {
        soc_misc_drv_err("Failed to invoke hal_kernel_get_hardware_info. (devid=%u)\n", soc_dieid->dev_id);
        return ret;
    }

    for (count = 0; count < DIE_ID_NUM; count++) {
        ret = soc_misc_reg_op(REG_OP_TYPE_RD, dieid_addr + hardware_info.phy_addr_offset,
            count * sizeof(unsigned int), DIE_INFO_MAP_SIZE, &value);
        if (ret < 0) {
            soc_misc_drv_err("Read register for soc die id failed. (count=%d; dev_id=%u)\n", count, soc_dieid->dev_id);
            return ret;
        }
        soc_dieid->data[count] = value;
    }
#endif

    return ret;
}

#ifdef CFG_FEATURE_DIE_ID_PMU
int soc_misc_get_pmu_dieid(dms_soc_die_id_t *pmu_dieid)
{
#ifdef CONFIG_HISI_SPMI_PMIC_VOLTAGE
    int ret;

    if (pmu_dieid == NULL) {
        soc_misc_drv_err("Pmu die id is null.\n");
        return -EINVAL;
    }

    switch (pmu_dieid->pmu_dieid_type) {
        case PMU_MAIN_DIEID:
            ret = get_main_pmu_die_id((unsigned char *)&pmu_dieid->data, pmu_dieid->size);
            break;
        case PMU_SECOND_DIEID:
            ret = get_second_pmu_die_id(pmu_dieid->pmu_dev_id, (unsigned char *)&pmu_dieid->data, pmu_dieid->size);
            break;
        default:
            soc_misc_drv_err("Undefine PMU die id type. (pmu_dieid_type=%u)\n", pmu_dieid->pmu_dieid_type);
            return -EINVAL;
    }

    if (ret != 0) {
        soc_misc_drv_err("Get PMU die id failed. (ret=%d)\n", ret);
        return ret;
    }

    return ret;
#else
    return -ENOTSUPP;
#endif
}
#endif

int soc_misc_get_die_id(dms_soc_die_id_t *die_id)
{
    int ret;

    if (die_id == NULL) {
        soc_misc_drv_err("Input die id information is null.\n");
        return -EINVAL;
    }

    if ((soc_misc_check_dev_id(die_id->dev_id) != 0) || (die_id->size > DMS_PMU_DIEID_DATA_SIZE)) {
        soc_misc_drv_err("Input parameter invalid. (dev_id=%u; data_size=%u)\n", die_id->dev_id, die_id->size);
        return -EINVAL;
    }

    switch (die_id->soc_type) {
        case DFT_SOC_DIEID:
            ret = soc_misc_get_soc_dieid(die_id);
            break;
#ifdef CFG_FEATURE_DIE_ID_N_V
        case DFT_SOC_N_DIEID:
            die_id->pmu_dieid_type = DEVDRV_SOC_N_DIE_ID;
            ret = soc_misc_get_soc_dieid(die_id);
            break;
#endif
#ifdef CFG_FEATURE_DIE_ID_PMU
        case PMU_FIRST_DIEID:
            die_id->pmu_dieid_type = PMU_MAIN_DIEID;
            die_id->pmu_dev_id = FIRST_PMU_DEVICE_ID;
            die_id->size = PMU_DIEID_MAX_LEN;
            ret = soc_misc_get_pmu_dieid(die_id);
            break;
        case PMU_SECOND_PRE_DIEID:
            die_id->pmu_dieid_type = PMU_SECOND_DIEID;
            die_id->pmu_dev_id = SECOND_PMU_PRE_DEVICE_ID;
            die_id->size = PMU_DIEID_MAX_LEN;
            ret = soc_misc_get_pmu_dieid(die_id);
            break;
        case PMU_SECOND_BACK_DIEID:
            die_id->pmu_dieid_type = PMU_SECOND_DIEID;
            die_id->pmu_dev_id = SECOND_PMU_BACK_DEVICE_ID;
            die_id->size = PMU_DIEID_MAX_LEN;
            ret = soc_misc_get_pmu_dieid(die_id);
            break;
#endif
        default:
            soc_misc_drv_err("Input soc type is invalid. (soc_type=%u)\n", die_id->soc_type);
            return -EINVAL;
    }

    if (ret != 0) {
        soc_misc_drv_err("Get die id fail. (dev_id=%u; soc_type=%u; ret=%d\n)", die_id->dev_id, die_id->soc_type, ret);
        return ret;
    }

    return 0;
}

#ifdef CFG_FEATURE_CHIP_EXPAND_VERSION
int soc_misc_get_chip_version(u32 dev_id, u8 *chip_version)
{
    int ret;
    u32 value;

    ret = soc_misc_reg_op(REG_OP_TYPE_RD, CHIP_VERSION_ADDR + dev_id * SOC_CHIP_OFFSET, 0,
        (unsigned long)sizeof(u32), &value);
    if (ret < 0) {
        soc_misc_drv_err("Read register for chip version failed. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    *chip_version = (value & CHIP_EXPAND_VERSION_MASK) ? 1 : 0;

    return 0;
}
#endif

static int soc_misc_info_check_sriov_info(struct vmng_sriov_info *sriov_info)
{
    /* milan max dev num is 2 */
    if ((sriov_info->sriov_status >= VMNGH_PF_STATUS_MAX) || (sriov_info->dev_id > 2)) {
        soc_misc_drv_err("Input parameter invalid. (dev_id=%u, sriov_status=%u)\n",
            sriov_info->dev_id, sriov_info->sriov_status);
        return -EINVAL;
    }

    return 0;
}

int soc_misc_set_cpu_info(void *data)
{
    struct soc_misc_info_st *soc_info = NULL;
    struct vmng_sriov_info *sriov_info = NULL;
    unsigned int ccpu_num = 1;
    unsigned int dcpu_num;
    unsigned int aicpu_num;
    int ret;

    if (data == NULL) {
        soc_misc_drv_err("Input parameter is null.\n");
        return -EINVAL;
    }

    sriov_info = (struct vmng_sriov_info *)data;
    ret = soc_misc_info_check_sriov_info(sriov_info);
    if (ret != 0) {
        soc_misc_drv_err("check para failed.\n");
        return ret;
    }

    if (sriov_info->sriov_status == VMNGH_PF_SRIOV_ENABLE) {
        /* aicpu_num set to 7 when SRIOV enable */
        aicpu_num = 7;
    } else {
        /* aicpu_num set to 6 when SRIOV disable */
        aicpu_num = 6;
    }

    dcpu_num = 8 - (ccpu_num + aicpu_num); /* total cpu num is 8 */
    if (dcpu_num > 1) {
        soc_misc_drv_err("Invalid dcpu_num. (ccpu_num = %u, aicpu_num=%u, dcpu_num=%u)\n",
                         ccpu_num, aicpu_num, dcpu_num);
        return -EINVAL;
    }

    soc_info = soc_misc_get_soc_info(sriov_info->dev_id);
    if (soc_info == NULL) {
        soc_misc_drv_err("get soc info failed. (dev_id=%u)\n", sriov_info->dev_id);
        return -EINVAL;
    }

    soc_info->cpu_info.aicpu_num = aicpu_num;
    soc_info->cpu_info.dcpu_num = dcpu_num;

    return 0;
}
