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

int soc_misc_init_board_id_from_dts(struct soc_misc_info_st *soc_info)
{
    struct device_node *np = NULL;
    unsigned int boardid[DTS_BOARD_ID_ARRAY_SIZE];
    int ret;

    np = of_find_compatible_node(NULL, NULL, DEV_NODE_COMPATIBLE_STR);
    if (np == NULL) {
        soc_misc_drv_info("Unable to find hisilicon, sysctrl node.\n");
        return -EINVAL;
    }

    ret = of_property_read_u32_array(np, BOARD_ID_PROPNAME, boardid, DTS_BOARD_ID_ARRAY_SIZE);
    if (ret != 0) {
        soc_misc_drv_info("Unable to read board id. (ret=%d)\n", ret);
        return -EINVAL;
    }

    soc_info->board_info.board_id = boardid[0] * THOUSANDS_PLACE + boardid[1] * HUNDREDS_PLACE +
            boardid[2] * TENS_PLACE + boardid[3]; /* index 2 and 3 for calculate */

    return 0;
}
