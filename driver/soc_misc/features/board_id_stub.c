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

#define SOC_MISC_STUB_ID 0x5A
int soc_misc_init_board_id_stub(struct soc_misc_info_st *soc_info)
{
    soc_info->board_info.board_id = SOC_MISC_STUB_ID;
    return 0;
}
