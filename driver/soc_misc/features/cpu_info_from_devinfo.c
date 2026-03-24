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

#include <linux/delay.h>

#include "soc_misc_init.h"
#include "soc_misc_info.h"
#include "devdrv_common.h"
#include "davinci_api.h"
#include "devdrv_interface.h"
#include "dms_define.h"
#include "user_cfg_interface.h"

int soc_misc_init_cpu_info_from_devinfo(struct soc_misc_info_st *soc_info)
{
    struct devdrv_info *dev_info = NULL;
    struct dms_dev_ctrl_block *dev_cb = NULL;

    if (soc_info == NULL) {
        soc_misc_drv_err("Soc_info is NULL.\n");
        return -EINVAL;
    }

    dev_cb = dms_get_dev_cb(soc_info->dev_id);
    if (dev_cb == NULL) {
        soc_misc_drv_err("Get dev_ctrl block failed. (dev_id=%u)\n", soc_info->dev_id);
        return -ENODEV ;
    }

    dev_info = (struct devdrv_info *)dev_cb->dev_info;
    if (dev_info == NULL) {
        soc_misc_drv_err("Invalid dev_info.\n");
        return -ENODEV ;
    }

    soc_info->cpu_info.tscpu_num = dev_info->pdata->ts_pdata[0].ts_cpu_core_num;
    soc_info->cpu_info.tscpu_os_sched = 1;
    soc_info->cpu_info.aicpu_num = dev_info->ai_cpu_core_num;
    soc_info->cpu_info.aicpu_os_sched = 1;
    soc_info->cpu_info.ccpu_num = dev_info->ctrl_cpu_core_num;
    soc_info->cpu_info.ccpu_os_sched = 1;

    return 0;
}
