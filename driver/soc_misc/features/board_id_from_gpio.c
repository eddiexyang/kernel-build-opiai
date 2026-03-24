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
#include "soc_spec.h"
#include "dms_define.h"

int soc_misc_init_board_id_from_gpio(struct soc_misc_info_st *soc_info)
{
    struct devdrv_info *dev_info = NULL;
    struct dms_dev_ctrl_block *dev_cb = NULL;
    struct gpio_desc *board_arry[BOARD_GPIO_SIZE] = {0};
    unsigned int board_id = 0;
    unsigned int value;
    unsigned int i;
    int ret;

    dev_cb = dms_get_dev_cb(soc_info->dev_id);
    if (dev_cb == NULL) {
        dms_err("Get dev_ctrl block failed. (dev_id=%u)\n", soc_info->dev_id);
        return -ENODEV ;
    }

    if (dev_cb->dev_info == NULL) {
        dms_err("Device ctrl dev_info is null. (dev_id=%u)\n", soc_info->dev_id);
        return -ENODEV ;
    }

    dev_info = (struct devdrv_info *)dev_cb->dev_info;
    for (i = 0; i < BOARD_GPIO_SIZE; i++) {
        board_arry[i] = gpiod_get_index(dev_info->dev, GPIO_BOARD_ID_NAME, i, GPIOD_IN);
        ret = gpiod_direction_input(board_arry[i]);
        if (ret != 0) {
            soc_misc_drv_err("gpio set direction failed, (board_arry=%u; ret=%d)\n", i, ret);
            return ret;
        }

        value = gpiod_get_value(board_arry[i]);
        board_id |= (value << i);
        gpiod_put(board_arry[i]);
    }

    soc_info->board_info.board_id = board_id;
    return 0;
}
