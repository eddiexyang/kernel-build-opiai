/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
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
#include <linux/io.h>
#include <linux/delay.h>

#include "devdrv_platform_register.h"
#include "tsdrv_kernel_common.h"
#include "tsdrv_log.h"
#include "tsdrv_soc_common.h"
#include "tsdrv_soc_pm.h"
#include "tsdrv_soc_smc.h"

void tsdrv_soc_startup(u64 ts_subsysctrl_base, u64 boot_addr)
{
    void *__iomem addr = NULL;

    addr = ioremap(ts_subsysctrl_base + TSCPU_BOOT_ADDR0_REG_OFFSET, 0x10);
    if (addr == NULL) {
        TSDRV_PRINT_ERR("devm_ioremap failed.\n");
        return;
    }
    writel(lower_32_bits(boot_addr), addr);
    writel(upper_32_bits(boot_addr) | 0x200, addr + sizeof(u32));
    iounmap(addr);
}

void tsdrv_enable_disp_nfe(void __iomem *disp_base)
{
    u32 __iomem *mask = NULL;

    /* unmask disp intmask register */
    mask = (u32 __iomem *)((unsigned long)(uintptr_t)disp_base + DEVDRV_DISP_INTMASK0);
    writel_relaxed(0, mask);
}
EXPORT_SYMBOL(tsdrv_enable_disp_nfe);

void tsdrv_enable_ts_disp(void __iomem *disp_base)
{
    u32 __iomem *daw_en = NULL;
    u32 reg_val;

    /* enable ts's daw_en bit */
    daw_en = (u32 __iomem *)((uintptr_t)((unsigned long)((uintptr_t)disp_base) + DEVDRV_DISP_DAW_EN));
    reg_val = readl_relaxed(daw_en);
    reg_val = reg_val | (0x01UL << DEVDRV_DISP_TS_DAW_INDEX);
    writel_relaxed(reg_val, daw_en);
}
EXPORT_SYMBOL(tsdrv_enable_ts_disp);

int tsdrv_soc_dereset_tscpu(void __iomem *ts_sysctrl)
{
    void __iomem *rst_reg = NULL;

    if (ts_sysctrl == NULL) {
        TSDRV_PRINT_ERR("devm_ioremap failed.\n");
        return -ENOMEM;
    }

    rst_reg = ts_sysctrl + 0xBC;
    set_tscpu_reg(rst_reg, 0x3F);
    rst_reg = ts_sysctrl + 0xCC;
    set_tscpu_reg(rst_reg, 0x7FF);

    wmb();
    return 0;
}
EXPORT_SYMBOL(tsdrv_soc_dereset_tscpu);

int tsdrv_soc_reset_tscpu(void __iomem *sysctrl, u32 tsid)
{
    void __iomem *rst_reg = NULL;
    u32 reg_val;
    int times;
    int ret = 0;

    if (tsid >= DEVDRV_MAX_TS_NUM) {
        TSDRV_PRINT_ERR("devm_ioremap invalid tsid=%u.\n", tsid);
        return -1;
    }

    if (sysctrl == NULL) {
        TSDRV_PRINT_ERR("devm_ioremap failed.\n");
        return -ENOMEM;
    }

    /* wait for idle */
    rst_reg = sysctrl + 0x18;
    times = 0;
    while (1) {
        reg_val = readl(rst_reg);
        if (reg_val & 0x01) {
            TSDRV_PRINT_DEBUG("poll TS wfi state succ.\n");
            break;
        }
        msleep(1);
        times++;
        if (times >= 500) {
            TSDRV_PRINT_ERR("poll TS wfi state timeout.\n");
            ret = -1;
            goto out;
        }
    }

    TSDRV_PRINT_DEBUG("begin to reset TS.\n");

    /* reset TS */
    rst_reg = sysctrl + 0xC8;
    set_tscpu_reg(rst_reg, 0x600);
    set_tscpu_reg(rst_reg, 0x1FF);
    /* close TS clock */
    rst_reg = sysctrl + 0xC0;
    set_tscpu_reg(rst_reg, 0x3F);
    wmb();

    TSDRV_PRINT_DEBUG("reset TS end.\n");
    ret = 0;

out:
    return ret;
}
EXPORT_SYMBOL(tsdrv_soc_reset_tscpu);

void tsdrv_soc_reset_prot_disable(void __iomem *ts_sysctrl)
{
    u32 __iomem *reset_reg = NULL;
    u32 reg_val;

    reset_reg = ts_sysctrl + TSCPU_PERIPHCTRL32_OFFSET;

    reg_val = readl_relaxed(reset_reg);
    reg_val &= (~0x1UL);
    writel_relaxed(reg_val, reset_reg);
}
EXPORT_SYMBOL(tsdrv_soc_reset_prot_disable);

int tsdrv_config_gicr(u64 addr)
{
    u32 value;
    int ret;
    int count = 500;

    ret = atf_read_reg_el3(addr, &value);
    TSDRV_PRINT_DEBUG("read value: 0x%x, func result: %d.\n", value, ret);
    (void)ret;
    value |= 0x02;
    ret = atf_write_reg_el3(addr, value);
    TSDRV_PRINT_DEBUG("call atf_write_reg_el3  value: 0x%x, ret: %d.\n", value, ret);

    while (count--) {
        (void)ret;
        ret = atf_read_reg_el3(addr, &value);
        (void)ret;
        if ((value & 0x04UL) != 0)
            break;
    }

    TSDRV_PRINT_DEBUG("call atf_read_reg_el3  value: 0x%x, func result: %d.\n", value, ret);

    if ((value & 0x04UL) != 0)
        ret = 0;
    else
        ret = 1;

    TSDRV_PRINT_DEBUG("ret: %d.\n", ret);

    return ret;
}
EXPORT_SYMBOL(tsdrv_config_gicr);

