/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: adp manage
 * Author: Hisilicon multimedia software group
 * Create: 2018/10/30
 */
#include "tde_adp.h"
#include "ot_drv_tde.h"

struct media_mem_file *g_tde_mem_file = TD_NULL;

struct media_mem_file *tde_get_mem_file(void)
{
    return g_tde_mem_file;
}

td_void tde_set_mem_file(struct media_mem_file *mem_file)
{
    g_tde_mem_file = mem_file;
}

td_void tde_hal_get_capability(td_u32 *capability)
{
    if (capability != TD_NULL) {
        *capability = TDE_CAPABILITY;
    }

    return;
}