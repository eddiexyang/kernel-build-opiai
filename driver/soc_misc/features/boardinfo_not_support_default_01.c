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

#include "soc_misc_board_info.h"
#include "soc_misc_init.h"
#include "devdrv_interface.h"

int soc_misc_init_pcb_id_not_support_default_01(struct soc_misc_info_st *soc_info)
{
    soc_info->board_info.pcb_id = 0x1;
    return 0;
}

int soc_misc_init_bom_id_not_support_default_01(struct soc_misc_info_st *soc_info)
{
    soc_info->board_info.bom_id = 0x1;
    return 0;
}
