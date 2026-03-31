/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
 * Create: 2023-4-1
 */
#include "hi_osal.h"

#ifdef AOS_LLVM_BUILD
#include <linux/export.h>
#endif

typedef struct {
    hi_s32 (*pfn_bootdot_init_blk)(hi_u32 block_id, hi_u32 magic, hi_u32 execption_id, hi_u32 expect_status);
    hi_s32 (*pfn_bootdot_set_blk)(hi_u32 block_id, hi_u32 magic, hi_u32 current_status);
} bootdot_exports_funcs;

static bootdot_exports_funcs g_bootdot_funcs;

hi_s32 osal_bootdot_sym_init()
{
    g_bootdot_funcs.pfn_bootdot_init_blk = osal_symbol_get("bootdot_init_blk");
    if (g_bootdot_funcs.pfn_bootdot_init_blk == HI_NULL) {
        return HI_FAILURE;
    }

    g_bootdot_funcs.pfn_bootdot_set_blk = osal_symbol_get("bootdot_set_blk");
    if (g_bootdot_funcs.pfn_bootdot_set_blk == HI_NULL) {
        osal_symbol_put("bootdot_init_blk");
        g_bootdot_funcs.pfn_bootdot_init_blk = HI_NULL;
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

hi_void osal_bootdot_sym_deinit()
{
    if (g_bootdot_funcs.pfn_bootdot_init_blk != HI_NULL) {
        osal_symbol_put("bootdot_init_blk");
        g_bootdot_funcs.pfn_bootdot_init_blk = HI_NULL;
    }

    if (g_bootdot_funcs.pfn_bootdot_set_blk != HI_NULL) {
        osal_symbol_put("bootdot_set_blk");
        g_bootdot_funcs.pfn_bootdot_set_blk = HI_NULL;
    }
}

hi_s32 osal_bootdot_init_blk(hi_u32 block_id, hi_u32 magic, hi_u32 execption_id, hi_u32 expect_status)
{
    if (g_bootdot_funcs.pfn_bootdot_init_blk != HI_NULL) {
        return g_bootdot_funcs.pfn_bootdot_init_blk(block_id, magic, execption_id, expect_status);
    }

    return 0;
}
EXPORT_SYMBOL(osal_bootdot_init_blk);

hi_s32 osal_bootdot_set_blk(hi_u32 block_id, hi_u32 magic, hi_u32 current_status)
{
    if (g_bootdot_funcs.pfn_bootdot_set_blk != HI_NULL) {
        return g_bootdot_funcs.pfn_bootdot_set_blk(block_id, magic, current_status);
    }

    return 0;
}
EXPORT_SYMBOL(osal_bootdot_set_blk);
