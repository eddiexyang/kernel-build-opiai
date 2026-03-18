/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: gfbg rotate operation
 */

#include "gfbg_rotate.h"
#include "gfbg_rotate_tde.h"

static int_callback g_tde_rotate_call_back;
static vgs_callback g_vgs_rotate_call_back;

gfbg_rotate_ops g_gfbg_rotate_ops = {0};

td_void gfbg_rotation_register(td_void)
{
    g_gfbg_rotate_ops.gfbg_drv_rotate = gfbg_drv_rotate_tde_process;
    g_gfbg_rotate_ops.is_support = TD_TRUE;
    return;
}

td_bool gfbg_get_rotation_support(td_void)
{
    return g_gfbg_rotate_ops.is_support;
}

gfbg_rotate gfbg_get_rotation(td_void)
{
    return g_gfbg_rotate_ops.gfbg_drv_rotate;
}

td_s32 gfbg_drv_set_tde_rotate_callback(int_callback tde_rot_callback)
{
    g_tde_rotate_call_back = tde_rot_callback;
    return TD_SUCCESS;
}

td_s32 gfbg_drv_set_vgs_rotate_callback(vgs_callback vgs_rot_callback)
{
    g_vgs_rotate_call_back = vgs_rot_callback;
    return TD_SUCCESS;
}

int_callback gfbg_drv_get_tde_rotate_callback(td_void)
{
    return g_tde_rotate_call_back;
}

vgs_callback gfbg_drv_get_vgs_rotate_callback(td_void)
{
    return g_vgs_rotate_call_back;
}
