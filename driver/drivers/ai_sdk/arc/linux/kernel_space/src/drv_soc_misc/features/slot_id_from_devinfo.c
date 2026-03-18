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

#include <linux/device.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <asm/uaccess.h>

#include "soc_misc_board_info.h"
#include "soc_misc_init.h"
#include "devdrv_common.h"
#include "davinci_api.h"
#include "dms_define.h"

#ifdef CFG_FEATURE_SLOT_ID_FROM_MODULE_ID
#include "ascend_platform.h"
#endif

int soc_misc_init_slot_id_from_devinfo(struct soc_misc_info_st *soc_info)
{
    struct devdrv_info *dev_info = NULL;
    struct dms_dev_ctrl_block *dev_cb = NULL;

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

#ifdef CFG_FEATURE_SLOT_ID_FROM_MODULE_ID
    if (dev_info->mainboard_id == MAINBOARD_ID_DUAL_SERVER_SECOND) {
        soc_info->board_info.slot_id = dev_info->chip_id + DUAL_SERVER_EACH_CHIP_COUNT;
    } else {
        soc_info->board_info.slot_id = dev_info->chip_id;
    }
#else
    soc_info->board_info.slot_id = dev_info->slot_id;
#endif

    soc_misc_drv_debug("Get slot id success. (dev_id=%u; slot_id=%u)\n",
                       soc_info->dev_id, soc_info->board_info.slot_id);
    return 0;
}
