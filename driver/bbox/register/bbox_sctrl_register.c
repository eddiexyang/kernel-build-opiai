/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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
 * Create: 2022-08-13
 */

#include "bbox_sctrl_register.h"
#include <linux/io.h>
#include <linux/securec.h>
#ifndef AOS_LLVM_BUILD
#include <linux/securectype.h>
#endif
#include "bbox_register.h"
#include "bbox_platform.h"
#include "common/bbox_print.h"
#include "common/bbox_common.h"
#include "config/bbox_config.h"

STATIC u64 g_reset_reg = 0;

/*
 * @brief       : get the pmu addr, which record reset reason
 * @return      : u64   reg addr
 */
STATIC u64 bbox_get_reset_reg(void)
{
    return g_reset_reg;
}

/*
 * @brief       : get pmu reset reg's base add
 * @param [in]  : const struct bbox_config_data *config  config
 * @return      : != 0 failure; ==0 success
 */
STATIC s32 bbox_get_reset_reg_init(const struct bbox_config_data *config)
{
    u64 reset_reg_dev_addr;
    BB_CHECK_PTR(config, return BBOX_FAILURE, "invalid imput param, config is NULL.\n");
    reset_reg_dev_addr = bbox_register_get_device_addr(DEFAULT_DEVICE_ID, config->pmu_reset_reg.addr);
    g_reset_reg = (u64)(uintptr_t)ioremap(reset_reg_dev_addr, config->pmu_reset_reg.length);
    if (g_reset_reg == 0) {
        BB_PRINT_ERR("get pmu reset reg error\n");
        return BBOX_FAILURE;
    }

    return BBOX_SUCCESS;
}

/*
 * @brief       : record exceptions to reg
 * @param [in]  : u8 e_type         exceptions type
 * @return      : NA
 */
void bbox_record_exce_type(u8 e_type)
{
    u32 value;
    u64 pmu_reset_reg;

    pmu_reset_reg = bbox_get_reset_reg();
    if (pmu_reset_reg != 0) {
        value = readl((char *)(uintptr_t)pmu_reset_reg);
        value &= (RESET_REG_MASK);
        value |= e_type;
        BB_PRINT_INFO("record exce type: %u.\n", value);
        writel(value, (char *)(uintptr_t)pmu_reset_reg);
    }

    return;
}

/*
 * @brief       : record the start dot in reg
 * @param [in]  : u8 stage  the start dot
 * @return      : NA
 */
void bbox_record_kernel_stage(u8 stage)
{
    u32 value;
    u64 pmu_reset_reg;

    pmu_reset_reg = bbox_get_reset_reg();
    if (pmu_reset_reg != 0) {
        value = readl((char *)(uintptr_t)(pmu_reset_reg + RESET_REG_SIZE));
        value &= (RESET_REG_MASK);
        value |= stage;
        BB_PRINT_INFO("record kernel stage is %u.\n", value);
        writel(value, (char *)(uintptr_t)(pmu_reset_reg + RESET_REG_SIZE));
    }
    return;
}

enum reg_type_list bbox_register_type(const char *name, u32 nlen)
{
    UNUSED(name);
    UNUSED(nlen);
    return REG_TYPE_SCTRL;
}

void *bbox_register_ioremap(const char *name, u32 nlen, u64 addr, u32 size)
{
    UNUSED(name);
    UNUSED(nlen);
    return ioremap(addr, size);
}

void bbox_register_iounmap(const char *name, u32 nlen, void *map_addr)
{
    UNUSED(name);
    UNUSED(nlen);
    if (!(IS_ERR_OR_NULL(map_addr))) {
        iounmap(map_addr);
    }
}

void bbox_register_dump(enum reg_type_list reg_type, u8 *dst, u32 dstsz, const u8 *src, u32 srcsz)
{
    UNUSED(reg_type);
    if ((dst != NULL) && (dstsz != 0) && (src != NULL) && (srcsz != 0)) {
        s32 ret = memcpy_s(dst, dstsz, src, srcsz);
        BB_CHECK_MEMCPY(ret, return);
    }
}

/*
 * @brief       : register module init
 * @return      : NA
 */
s32 bbox_register_init(void)
{
    const struct bbox_config_data *config = bbox_get_config();
    return bbox_get_reset_reg_init(config);
}

/*
 * @brief       : register module exit
 * @return      : NA
 */
void bbox_register_exit(void)
{
    if (g_reset_reg != 0) {
        iounmap((void *)(uintptr_t)g_reset_reg);
        g_reset_reg = 0;
    }
}

