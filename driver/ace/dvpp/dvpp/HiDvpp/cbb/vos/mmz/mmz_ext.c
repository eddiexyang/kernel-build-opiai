/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2023. All rights reserved.
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
 * Create: 2020-4-20
 */
#include "hi_osal.h"
#include "osal_mmz.h"
#include "mmz_ext.h"

#define MMZ_ALIGN_BYTE (64)
#define MMZ_GFP (0)
#define MMZ_ALLOC_TYPE (0)

// 内存起始位置和内存大小待定
static ot_mmz_create_attr g_mmz_create_attr[MAX_MMZ_NUM] = {
    {.gfp = MMZ_GFP, .phys_start = 0x48000000, .nbytes = 16 * 1024 * 1024, .block_align = MMZ_ALIGN_BYTE,
        .alloc_type = MMZ_ALLOC_TYPE},
    {.gfp = MMZ_GFP, .phys_start = 0x49000000, .nbytes = 16 * 1024 * 1024, .block_align = MMZ_ALIGN_BYTE,
        .alloc_type = MMZ_ALLOC_TYPE},
    {.gfp = MMZ_GFP, .phys_start = 0x4A000000, .nbytes = 16 * 1024 * 1024, .block_align = MMZ_ALIGN_BYTE,
        .alloc_type = MMZ_ALLOC_TYPE},
    {.gfp = MMZ_GFP, .phys_start = 0x4B000000, .nbytes = 16 * 1024 * 1024, .block_align = MMZ_ALIGN_BYTE,
        .alloc_type = MMZ_ALLOC_TYPE},
    {.gfp = MMZ_GFP, .phys_start = 0x4C000000, .nbytes = 16 * 1024 * 1024, .block_align = MMZ_ALIGN_BYTE,
        .alloc_type = MMZ_ALLOC_TYPE},
    {.gfp = MMZ_GFP, .phys_start = 0x4D000000, .nbytes = 16 * 1024 * 1024, .block_align = MMZ_ALIGN_BYTE,
        .alloc_type = MMZ_ALLOC_TYPE},
};
static hi_char *g_mmz_name[MAX_MMZ_NUM] = {"mmz_dvpp", "mmz_isp", "mmz_dss_trans", "mmz_untrusted",
    "mmz_trusted", "mmz_noperm"};


// osal_vmalloc/kmalloc使用mmz_trusted
// media_mem_malloc 需要业务代码传递mmz_name
// 如果是用户+加速器使用，则使用mmz_untrusted
// 如果是内核和加速器都使用，mmz_trusted
// 如果只有加速器使用, mmz_noperm
// hi_mpi_dvpp_malloc，使用mmz_untrusted
// mmz_dvpp待确认是否要使用

// 初始化6个mmz
hi_s32 mmz_init(hi_void)
{
    hi_u32 i = 0;
    ot_mmz_t *mmz_zone = NULL;
    hi_s32 ret;

    for (i = 0; i < MAX_MMZ_NUM; ++i) {
        mmz_zone = ot_mmz_create_v2(g_mmz_name[i], &g_mmz_create_attr[i], i);
        if (mmz_zone == NULL) {
            mmz_exit_check();
            return -1;
        }

        if (ot_mmz_register(mmz_zone)) {
            mmz_trace(MMZ_WARNING_LEVEL, "Add MMZ failed: " OT_MMZ_FMT_S "\n", ot_mmz_fmt_arg(mmz_zone));
            mmz_exit_check();
            return -1;
        }
    }

    return 0;
}

hi_s32 mmz_exit(hi_void)
{
    mmz_exit_check();
}

hi_u64 mmz_malloc(const hi_char *mmz_name, const hi_char *buf_name, hi_ulong size)
{
    hi_u32 align = MMZ_ALIGN_BYTE;
    // VOS下size需要申请mmb控制块信息，另外需要对齐，保证返回给用户的仍然是对齐后的地址
    hi_u32 mmb_cb_size = mmz_align2(sizeof(ot_mmb_t), align);
    size += mmb_cb_size;

    ot_mmb_t *mmb = ot_mmb_alloc(buf_name, size, align, MMZ_GFP, mmz_name);
    if (mmb == HI_NULL) {
        return 0;
    }

    return mmb->phys_addr + mmb_cb_size;
}

hi_void mmz_free(hi_u64 phy_addr, hi_void *vir_addr)
{
    // 先找到物理地址对应的mmb，然后进行free
    hi_u32 align = MMZ_ALIGN_BYTE;
    hi_u32 mmb_cb_size = mmz_align2(sizeof(ot_mmb_t), align);
    hi_u32 tmp_addr = phy_addr - mmb_cb_size;
    ot_mmb_t *mmb = ot_mmb_getby_phys(tmp_addr);
    if (mmb == HI_NULL) {
        return;
    }
    ot_mmb_free(mmb);
}

hi_s32 mmz_is_valid(const hi_char *mmz_name, hi_u64 phy_addr)
{
    // 先拿到对应的mmz
    ot_mmz_t *zone = ot_mmz_find(MMZ_GFP, mmz_name);
    if (zone == NULL) {
        return HI_FALSE;
    }

    hi_u32 align = MMZ_ALIGN_BYTE;
    hi_u32 mmb_cb_size = mmz_align2(sizeof(ot_mmb_t), align);
    if ((phy_addr >= (zone->phys_start + mmb_cb_size)) && (phy_addr <= (zone->phys_start + zone->nbytes))) {
        return HI_TRUE;
    }

    return HI_FALSE;
}