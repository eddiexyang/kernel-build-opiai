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

#define SLEEP_MS 10
#define LOOP_MS  1000

int soc_misc_init_cpu_info_from_pcie(struct soc_misc_info_st *soc_info)
{
#ifndef CFG_FEATURE_CPU_NUMS_FIXED
    u32 loop_time = LOOP_MS;
    struct agentdrv_cpu_info cpu_info = { 0 };

    if (soc_info == NULL) {
        soc_misc_drv_err("Soc_info is NULL.\n");
        return -EINVAL;
    }

    while (loop_time > 0) {
        /* The cpu information is the tag information,so wate here for ever if get fail */
        if (!agentdrv_get_cpu_info(soc_info->dev_id, &cpu_info)) {
            break;
        }
        msleep(SLEEP_MS);
        loop_time--;
    }
    if (loop_time == 0) {
        soc_misc_drv_err("Get cpu info timeout.\n");
        return -ENODEV;
    }

    soc_info->cpu_info.aicpu_num = cpu_info.aicpu_num;
    soc_info->cpu_info.aicpu_os_sched = cpu_info.aicpu_os_sched;
    soc_info->cpu_info.ccpu_num = cpu_info.ccpu_num;
    soc_info->cpu_info.ccpu_os_sched = cpu_info.ccpu_os_sched;
    soc_info->cpu_info.dcpu_num = cpu_info.dcpu_num;
    soc_info->cpu_info.dcpu_os_sched = cpu_info.dcpu_os_sched;
    soc_info->cpu_info.tscpu_num = cpu_info.tscpu_num;
    soc_info->cpu_info.tscpu_os_sched = cpu_info.tscpu_os_sched;

    return 0;
#else
    unsigned int dev_id;
    dev_cpu_nums_cfg_t *cpu_cfg = NULL;

    if (soc_info == NULL) {
        soc_misc_drv_err("Soc_info is NULL.\n");
        return -EINVAL;
    }

    dev_id = soc_info->dev_id;
    cpu_cfg = (dev_cpu_nums_cfg_t *)&soc_info->cpu_info;
    return dev_user_cfg_get_cpu_number(dev_id, cpu_cfg);
#endif
}
