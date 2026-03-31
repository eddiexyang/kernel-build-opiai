/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
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
#include "apb_comm_drv.h"
#include "resource_comm_drv.h"

STATIC u32 g_res_func_total_pre_dev = 1; /* pf + vf totol number */
STATIC u32 g_res_func_pf_num = 1;        /* pf number */

int agentdrv_get_connect_type(void)
{
    return agentdrv_get_connect_type_by_hw_info();
}

u32 agentdrv_res_get_func_total(void)
{
    return g_res_func_total_pre_dev;
}

void agentdrv_res_set_func_total(u32 totl_num)
{
    g_res_func_total_pre_dev = totl_num;
}

void agentdrv_res_init_func_total_num(void)
{
    u32 func_totl_num;

    func_totl_num = agentdrv_soc_get_func_total();
    agentdrv_res_set_func_total(func_totl_num);
}

u32 agentdrv_res_get_func_pf(void)
{
    return g_res_func_pf_num;
}

STATIC void agentdrv_res_set_func_pf(u32 pf_num)
{
    g_res_func_pf_num = pf_num;
}

void agentdrv_res_init_func_pf_num(void)
{
    u32 pf_num = agentdrv_soc_get_pf_func_total();
    agentdrv_res_set_func_pf(pf_num);
}
