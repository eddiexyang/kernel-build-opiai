/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
 * Create: 2022-09-28
 */
#include <linux/securec.h>
#include "chip_config.h"
#include "board_config.h"
#include "ascend_hal_error.h"
#include "board_serdes_info.h"

static struct dbl_board_serdes_info s_board_serdes_info;
void bdcfg_serdes_default(void)
{
    int i = 0;
    memset_s((char *)&s_board_serdes_info, sizeof(struct dbl_board_serdes_info), 0,
        sizeof(struct dbl_board_serdes_info));
    s_board_serdes_info.lan_count = MAX_CHIP_SERDES_LAN_NUM;
    for (i = 0; i < s_board_serdes_info.lan_count; i++) {
        s_board_serdes_info.lan_info[i].lan_index = i;
        s_board_serdes_info.lan_info[i].ssc_enable = 0;
        s_board_serdes_info.lan_info[i].polarity_tx = 0;
        s_board_serdes_info.lan_info[i].polarity_rx = 0;
        s_board_serdes_info.lan_info[i].lan_order = 0;
        s_board_serdes_info.lan_info[i].bandwidth = DBL_INVALID_CONFIG_VALUE;
        s_board_serdes_info.lan_info[i].align_mode = DBL_INVALID_CONFIG_VALUE;
        s_board_serdes_info.lan_info[i].frequency = SERDES_FREQ_NULL;
        s_board_serdes_info.lan_info[i].type = SERDES_TYPE_USB;
    }
}
int bdcfg_serdes_init(void)
{
    bdcfg_serdes_default();
    // Read the serdes count from chip config dbl_get_serdes_lan_count
    // Read the serdes configuration information of elastic configuration from the safe world memory through TEE
    return 0;
}
int bdcfg_serdes_exit(void)
{
    return 0;
}

struct dbl_board_serdes_info *dbl_get_board_serdes_info(void)
{
    return &s_board_serdes_info;
}
