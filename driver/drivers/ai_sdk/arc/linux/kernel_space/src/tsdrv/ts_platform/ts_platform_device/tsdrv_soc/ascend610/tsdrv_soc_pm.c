/* *
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
#include <linux/io.h>
#include <linux/delay.h>
#ifdef AOS_LLVM_BUILD
#include <linux/export.h>
#endif

#include "tsdrv_soc_pm.h"
#include "tsdrv_soc_common.h"
#include "tsdrv_soc_smc.h"
#include "devdrv_user_common.h"
#include "tsdrv_kernel_common.h"
#include "tsdrv_log.h"
#include "devdrv_platform_register.h"

#define AIC_TS_STARTUP_BASE_ADDR 0xba080000
#define VEC_TS_STARTUP_BASE_ADDR 0xb9080000

#define AIC_TS_STARTUP_BASE_ADDR_LEN 0x300
#define VEC_TS_STARTUP_BASE_ADDR_LEN 0x300

#define TS_SYSCTRL_PERIPHCTRL8 0xbc
#define TS_SYSCTRL_PERIPHCTRL9 0xc0
#define TS_SYSCTRL_PERIPHCTRL12 0xcc
#define TS_SYSCTRL_PERIPHCTRL32 0x11c
#define TS_SYSCTRL_PERIPHCTRL35 0x128
#define TS_SYSCTRL_PERIPHCTRL36 0x12c
#define TS_SYSCTRL_PERIPHCTRL37 0x130
#define TS_SYSCTRL_PERIPHCTRL39 0x138
#define TS_SYSCTRL_PERIPHCTRL40 0x13c
#define TS_SYSCTRL_PERIPHCTRL52 0x16c
#define TS_SYSCTRL_PERIPHCTRL69 0x1b0
#define TS_SYSCTRL_PERIPHCTRL148 0x2ec

#define TS0CPU_BASE_ADDR 0xba060000
#define TS0HWTSCPU_BASE_ADDR 0xba070000
#define TS1CPU_BASE_ADDR 0xb9060000
#define TS1HWTSCPU_BASE_ADDR 0xb9070000

#define TSXCPU_BASE_ADDR_LEN 0x4000
#define SC_ATS_ADDR_SRC0 0x3100
#define SC_ATS_ADDR_LEN0 0x3104
#define SC_ATS_ADDR_DES0 0x3108

#define SC_ATS_ADDR_SRC1 0x310C
#define SC_ATS_ADDR_LEN1 0x3110
#define SC_ATS_ADDR_DES1 0x3114

#define TSFW_BOOT_ADDR 0xFB00000
#define BOOT_ADDR_OFFSET 12

void tsdrv_soc_startup(u32 tsid, u32 devid, u64 boot_addr)
{
    u32 reg_val;
    void __iomem *addr = NULL;
    void __iomem *tscpu_addr = NULL;

    u64 reg_base_addr;
    u32 reg_size;
    u64 ts_ats_addr;

    if (tsid == 0) {
        reg_base_addr = AIC_TS_STARTUP_BASE_ADDR + (devid * CHIP_BASEADDR_PA_OFFSET);
        reg_size = AIC_TS_STARTUP_BASE_ADDR_LEN;
        ts_ats_addr = TS0CPU_BASE_ADDR + (devid * CHIP_BASEADDR_PA_OFFSET);
    } else {
        reg_base_addr = VEC_TS_STARTUP_BASE_ADDR + (devid * CHIP_BASEADDR_PA_OFFSET);
        reg_size = VEC_TS_STARTUP_BASE_ADDR_LEN;
        ts_ats_addr = TS1CPU_BASE_ADDR + (devid * CHIP_BASEADDR_PA_OFFSET);
    }
    TSDRV_PRINT_DEBUG("begin to startup tsdrv firmware.(tsid=%u, startup_base=0x%pK ts_ats_addr=0x%pK)\n",
        tsid, (void *)(uintptr_t)reg_base_addr, (void *)(uintptr_t)ts_ats_addr);
    /* ********** config vec ts startup ********** */
    addr = (void __iomem *)ioremap(reg_base_addr, reg_size);
    if ((void *)addr == NULL) {
        TSDRV_PRINT_ERR("ioremap failed. \n");
        return;
    }

    /* step-1 ts addr protect disable */
    reg_val = 0x1;
    writel(reg_val, (void *)(addr + TS_SYSCTRL_PERIPHCTRL32));

    /* step0-0 clk enable */
    reg_val = 0x1;
    writel(reg_val, (void *)(addr + TS_SYSCTRL_PERIPHCTRL8));

    /* step0-1 IP clk enable */
    reg_val = 0x7FFF;
    writel(reg_val, (void *)(addr + TS_SYSCTRL_PERIPHCTRL35));

    /* step0-2 R52 clk disable */
    reg_val = 0x1;
    writel(reg_val, (void *)(addr + TS_SYSCTRL_PERIPHCTRL9));

    /* step0-3 IP clk disable */
    reg_val = 0x7FFF;
    writel(reg_val, (void *)(addr + TS_SYSCTRL_PERIPHCTRL36));

    /* step1-0 boot addr */
    reg_val = (TSFW_BOOT_ADDR & 0xffffffe0) | (0x3);
    writel(reg_val, (void *)(addr + TS_SYSCTRL_PERIPHCTRL52));

    /* step3 R52 untoprst */
    reg_val = 0x1;
    writel(reg_val, (void *)(addr + TS_SYSCTRL_PERIPHCTRL12));

    /* step4-0 R52 unrst */
    reg_val |= 0xE;
    writel(reg_val, (void *)(addr + TS_SYSCTRL_PERIPHCTRL12));

    /* step4-1 R52 clk eable */
    reg_val = 0x1;
    writel(reg_val, (void *)(addr + TS_SYSCTRL_PERIPHCTRL8));

    /* step5-0 IP unrst */
#ifndef AOS_LLVM_BUILD
    reg_val = readl_relaxed((void *)(addr + TS_SYSCTRL_PERIPHCTRL40));
#else
    reg_val = readl((void *)(addr + TS_SYSCTRL_PERIPHCTRL40));
#endif
    reg_val |= 0x800;
    writel(reg_val, (void *)(addr + TS_SYSCTRL_PERIPHCTRL39));

    /* step5-1 IP clk eable */
#ifndef AOS_LLVM_BUILD
    reg_val = readl_relaxed((void *)(addr + TS_SYSCTRL_PERIPHCTRL37));
#else
    reg_val = readl((void *)(addr + TS_SYSCTRL_PERIPHCTRL37));
#endif
    reg_val |= 0x800;
    writel(reg_val, (void *)(addr + TS_SYSCTRL_PERIPHCTRL35));

    /* step6 boot */
    tscpu_addr = (void __iomem *)ioremap(ts_ats_addr, TSXCPU_BASE_ADDR_LEN);
    if (tscpu_addr == NULL) {
        TSDRV_PRINT_ERR("ioremap failed. \n");
        iounmap(addr);
        addr = NULL;
        return;
    }

    /*
     * while reserved add is different from 0xFB00000, need to config ats,
     * only dev0 tsid 0 is 0xFB00000
     */
    if (tsid == 1) { /* tsv for mdc, ats0 can't used under un-security mode, use ats1 instead */
        writel(0xFB00000, (void *)(tscpu_addr + SC_ATS_ADDR_SRC1));
        writel(0x1B400000, (void *)(tscpu_addr + SC_ATS_ADDR_LEN1));
        reg_val = (boot_addr - TSFW_BOOT_ADDR) >> BOOT_ADDR_OFFSET;
        writel(reg_val, (void *)(tscpu_addr + SC_ATS_ADDR_DES1));
    }

    if (devid == 1) {
        writel(0xFB00000, (void *)(tscpu_addr + SC_ATS_ADDR_SRC0));
        writel(0x1B400000, (void *)(tscpu_addr + SC_ATS_ADDR_LEN0));
        reg_val = (boot_addr - TSFW_BOOT_ADDR) >> BOOT_ADDR_OFFSET;
        writel(reg_val, (void *)(tscpu_addr + SC_ATS_ADDR_DES0));
    }

    /* step7 R52 release force enable */
#ifndef AOS_LLVM_BUILD
    reg_val = readl_relaxed((void *)(addr + TS_SYSCTRL_PERIPHCTRL148));
#else
    reg_val = readl((void *)(addr + TS_SYSCTRL_PERIPHCTRL148));
#endif
    reg_val &= 0xFFFFFFF8;
    writel(reg_val, (void *)(addr + TS_SYSCTRL_PERIPHCTRL148));

    /* step8 R52 unhalt */
#ifndef AOS_LLVM_BUILD
    reg_val = readl_relaxed((void *)(addr + TS_SYSCTRL_PERIPHCTRL52));
#else
    reg_val = readl((void *)(addr + TS_SYSCTRL_PERIPHCTRL52));
#endif
    reg_val &= 0xFFFFFFFE;
    writel(reg_val, (void *)(addr + TS_SYSCTRL_PERIPHCTRL52));

    iounmap(tscpu_addr);
    tscpu_addr = NULL;
    iounmap(addr);
    addr = NULL;
}
EXPORT_SYMBOL(tsdrv_soc_startup);

int tsdrv_soc_reset_tscpu(void __iomem *sysctrl, uint32_t tsid)
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

    /* step 6 reset tscpu */
    rst_reg = sysctrl + 0x2C;
    times = 0;
    while (1) {
        reg_val = readl(rst_reg);
        if ((reg_val & 0x10) != 0) {
            TSDRV_PRINT_INFO("poll TS pmc state succ.\n");
            break;
        }
        TSDRV_PRINT_INFO("waiting pmc ready. current status: %x\n", reg_val);

        msleep(100);
        times++;
        if (times >= DEVDRV_POLL_TS_TIME) {
            TSDRV_PRINT_ERR("poll TS pmc state timeout.\n");
            break;
        }
    }

    /* step 7 reset tscpu */
    rst_reg = sysctrl + 0x4C;
    times = 0;
    while (1) {
        reg_val = readl(rst_reg);
        if ((reg_val & 0x08) != 0) {
            TSDRV_PRINT_INFO("poll TS wfi state succ.\n");
            break;
        }
        TSDRV_PRINT_INFO("waiting wfi ready. current status: %x\n", reg_val);

        msleep(100);
        times++;
        if (times >= DEVDRV_POLL_TS_TIME) {
            TSDRV_PRINT_ERR("poll TS wfi state timeout. Forcibly disable the clock.\n");
            break;
        }
    }

    TSDRV_PRINT_INFO("begin to reset TS.\n");

    /* step 8 reset tscpu */
    rst_reg = sysctrl + 0xC8;
    set_tscpu_reg(rst_reg, 0xf);

    /* step 9 reset hwts */
    rst_reg = sysctrl + 0x134;
    set_tscpu_reg(rst_reg, 0x7fff);

    /* step 10 reset other */
    rst_reg = sysctrl + 0x1AC;
    set_tscpu_reg(rst_reg, 0x6);

    /* step 11 close TS clock */
    rst_reg = sysctrl + 0xC0;
    set_tscpu_reg(rst_reg, 0x1);

    /* step 12 close other clock */
    rst_reg = sysctrl + 0x12C;
    set_tscpu_reg(rst_reg, 0x7fff);

    TSDRV_PRINT_INFO("reset TS end.\n");
    return ret;
}
EXPORT_SYMBOL(tsdrv_soc_reset_tscpu);
