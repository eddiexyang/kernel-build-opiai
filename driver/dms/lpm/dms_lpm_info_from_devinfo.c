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
#include "devdrv_manager.h"

int lpm_get_temperature_from_devinfo(u32 dev_id, u32 core_id, u32 *temperature)
{
    struct dms_share_mem *share_mem_base = NULL;
    struct devdrv_platform_data *pdata = NULL;
    struct devdrv_info *dev_info = NULL;
    int ret;

    ret = dms_lpm_id_check(dev_id, core_id);
    if (ret != 0) {
        dms_err("Parameter error. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    dev_info = devdrv_get_default_devdrv_info();
    if (dev_info == NULL) {
        dms_err("Default dev_info is null. (dev_id=%u)\n", dev_id);
        return -EFAULT;
    }

    pdata = (struct devdrv_platform_data *)dev_info->pdata;
    share_mem_base = (struct dms_share_mem *)pdata->ts_pdata[0].tsensor_shm_vaddr;

    *temperature = share_mem_base->soc_max_temp;

    return 0;
}

extern u32 devdrv_get_freq(void);

int lpm_get_frequency_from_devinfo(u32 dev_id, u32 core_id, u32 *frequency)
{
    struct dms_share_mem *share_mem_base = NULL;
    struct devdrv_platform_data *pdata = NULL;
    struct devdrv_info *dev_info = NULL;
    u8 high;
    u8 low;
    int ret;

    ret = dms_lpm_id_check(dev_id, core_id);
    if (ret != 0) {
        dms_err("Parameter error. (dev_id=%u; ret=%d)\n", dev_id, ret);
        return ret;
    }

    dev_info = devdrv_get_default_devdrv_info();
    if (dev_info == NULL) {
        dms_err("Default dev_info is null. (dev_id=%u)\n", dev_id);
        return -EFAULT;
    }

    pdata = (struct devdrv_platform_data *)dev_info->pdata;
    share_mem_base = (struct dms_share_mem *)pdata->ts_pdata[0].tsensor_shm_vaddr;

    if (core_id == LPM_CLUSTER_ID) {
        *frequency = devdrv_get_freq();
    } else if (core_id == LPM_DDR_ID) {
        high = share_mem_base->ddr_freq_h;
        low = share_mem_base->ddr_freq_l;
        *frequency = (high << OFFSET_EIGHT) | low;
    } else if ((core_id == LPM_AICORE0_ID) || (core_id == LPM_AICORE1_ID)) {
        high = share_mem_base->aicore_base_freq_h;
        low = share_mem_base->aicore_base_freq_l;
        *frequency = (high << OFFSET_EIGHT) | low;
    } else {
        dms_err("Invalid core id. (dev_id=%u; core_id=%u)\n", dev_id, core_id);
        return -EOPNOTSUPP;
    }

    return 0;
}

int lpm_get_temp_from_sharemem(u32 core_id, u32 *temp)
{
    struct dms_share_mem *share_mem_base = NULL;
    struct devdrv_platform_data *pdata = NULL;
    struct devdrv_info *dev_info = NULL;

    dev_info = devdrv_get_default_devdrv_info();
    if (dev_info == NULL) {
        dms_err("Default dev_info is null.\n");
        return -EFAULT;
    }

    pdata = (struct devdrv_platform_data *)dev_info->pdata;
    share_mem_base = (struct dms_share_mem *)pdata->ts_pdata[0].tsensor_shm_vaddr;

    if (core_id == DMS_CLUSTER_TEMP_ID) {
        *(char *)temp =share_mem_base->cluster_temp;
    } else if (core_id == DMS_PERI_TEMP_ID) {
        *(char *)temp =share_mem_base->peri_temp;
    } else if (core_id == DMS_AICORE0_TEMP_ID) {
        *(char *)temp =share_mem_base->aicore0_temp;
    } else if (core_id == DMS_AICORE1_TEMP_ID) {
        *(char *)temp =share_mem_base->aicore1_temp;
    } else if (core_id == DMS_AICORE_LIMIT_ID) {
        *(u8 *)temp =share_mem_base->aicore_limit;
    } else if (core_id == DMS_AICORE_TOTAL_PER_ID) {
        *(u8 *)temp =share_mem_base->aicore_total_period;
    } else if (core_id == DMS_AICORE_ELIM_PER_ID) {
        *(u8 *)temp =share_mem_base->aicore_elim_period;
    } else if (core_id == DMS_AICORE_BASE_FREQ_ID) {
        *(u16 *)temp = (((u16)share_mem_base->aicore_base_freq_h << OFFSET_EIGHT)
            | share_mem_base->aicore_base_freq_l);
    } else {
        dms_err("Invaild core id. (core_id=%u)\n", core_id);
        return -EOPNOTSUPP;
    }

    return 0;
}

int lpm_get_aicore_rated_frequency(u32 dev_id, u32 *freq)
{
    struct devdrv_info *dev_info = NULL;
    struct dms_dev_ctrl_block *dev_cb = NULL;

    dev_cb = dms_get_dev_cb(dev_id);
    if (dev_cb == NULL) {
        dms_err("Get dev_ctrl block failed. (dev_id=%u)\n", dev_id);
        return -EFAULT;
    }

    dev_info = (struct devdrv_info *)dev_cb->dev_info;
    if (dev_info == NULL) {
        dms_err("Invalid dev_info. (dev_id=%u)\n", dev_id);
        return -EFAULT;
    }

    *freq = dev_info->aicore_freq;

    return 0;
}
