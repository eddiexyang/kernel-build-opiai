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

int soc_misc_init_slot_id_from_gpio(struct soc_misc_info_st *soc_info)
{
    struct dms_dev_ctrl_block *dev_cb = NULL;
    struct devdrv_info *dev_info = NULL;
    bool is_cloud_arm_server = false;
    struct gpio_desc *slot_arry[SLOT_GPIO_SIZE] = {0};
    unsigned int board_id = 0;
    unsigned int slot_id = 0;
    unsigned int value = 0;
    unsigned int i;
    int ret;

    dev_cb = dms_get_dev_cb(soc_info->dev_id);
    if (dev_cb == NULL) {
        dms_err("Get dev_ctrl block failed. (dev_id=%u)\n", soc_info->dev_id);
        return -ENODEV ;
    }

    board_id = soc_info->board_info.board_id;
    dev_info = (struct devdrv_info *)dev_cb->dev_info;
    if (dev_info == NULL) {
        soc_misc_drv_err("Invalid dev_info.\n");
        return -ENODEV ;
    }

    if ((board_id & 0x3000) == 0x0) {
        board_id = board_id & AK_CLOUD_BOARDID_MASK;
        /* arm server board id rule: GPIO[75:73]=001 */
        is_cloud_arm_server = (((board_id >> CLOUD_MODULE_BOARID_SHIFT) == CLOUD_MODULE_BOARDID_MASK) ||
                               (board_id == AK_CLOUD_BOARDID));

        for (i = 0; i < SLOT_GPIO_SIZE; i++) {
            slot_arry[i] = gpiod_get_index(dev_info->dev, GPIO_SLOT_ID_NAME, i, GPIOD_IN);
            ret = gpiod_direction_input(slot_arry[i]);
            if (ret != 0) {
                soc_misc_drv_err("Gpio set direction failed. (slot_arry_index=%u; ret=%d)\n", i, ret);
                return -EINVAL;
            }
            value = gpiod_get_value(slot_arry[i]);
            slot_id |= (value << i);
            gpiod_put(slot_arry[i]);
        }

        /* board type is ARM AI SERVER type */
        if (is_cloud_arm_server) {
            /* A+K slot id low 2 bits reverse */
            soc_info->board_info.slot_id = (slot_id & 0x4) + ((slot_id ^ 0x3) & 0x3);
        } else { /* board type is X86 AI SERVER type */
            soc_info->board_info.slot_id = (slot_id & 0x7);  /* get bit0  bit1  bit2 */
        }
    } else {
        soc_misc_drv_info("it's not AI server, not support slot id.\n");
        soc_info->board_info.slot_id = 0xff;
    }

    return 0;
}
