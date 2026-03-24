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

#define PERI_SUBSYS_DISP_REG_BASE 0x80120000ULL
#define PERI_SUBSYS_DISP_REG_SIZE (64 * 1024)
#define PERI_SUBSYS_SCHE1_REG_BASE 0x80110000ULL
#define PERI_SUBSYS_SCHE1_REG_SIZE (64 * 1024)
#ifdef CFG_SOC_PLATFORM_MDC_V11
#define DEVDRV_POLL_TS_SUSPEND_TIME 5
#else
#define DEVDRV_POLL_TS_SUSPEND_TIME 50
#endif

void tsdrv_sleep(u64 sleep_time_ms)
{
#ifdef CFG_SOC_PLATFORM_MDC_V11
    usleep_range(sleep_time_ms * 1000, sleep_time_ms * 1000 + 100); // ms * 1000, interval 100us
#else
    msleep(sleep_time_ms);
#endif
    return;
}

/* config TSCPU boot address */
void tsdrv_soc_startup(u32 devid, u64 boot_addr, u64 ts_subsysctl, u32 env_type)
{
    u64 addr;

    addr = (u64)(uintptr_t)ioremap(ts_subsysctl + DEVDRV_TS_SYSCTRL_PERIPHCTRL66, 0x20);
    if ((void *)(uintptr_t)addr == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("ioremap ts_sysctrl failed.\n");
        return;
#endif
    }

    writel((u32)boot_addr, (void *)(uintptr_t)addr); /* [1:0] reserved */
    writel(((u32)(boot_addr >> 32) | 0x200) & 0x2FF, (void *)(uintptr_t)(addr + sizeof(u32)));

    iounmap((void *)(uintptr_t)addr);
    addr = (uintptr_t)NULL;
}
EXPORT_SYMBOL(tsdrv_soc_startup);

void tsdrv_sche_port0_clamp_en(void)
{
    u64 vaddr;

    vaddr = (u64)(uintptr_t)ioremap(PERI_SUBSYS_SCHE1_REG_BASE, PERI_SUBSYS_SCHE1_REG_SIZE);
    if ((void *)(uintptr_t)vaddr == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("ioremap ts_sysctrl failed.\n");
        return;
#endif
    }

    set_tscpu_reg((void *)(uintptr_t)(vaddr + 0x20), 0x0);
    iounmap((void *)(uintptr_t)vaddr);
}
EXPORT_SYMBOL(tsdrv_sche_port0_clamp_en);

static void tsdrv_disable_ts_disp(void)
{
    u64 vaddr;

    vaddr = (u64)(uintptr_t)ioremap(PERI_SUBSYS_DISP_REG_BASE, PERI_SUBSYS_DISP_REG_SIZE);
#ifndef TSDRV_UT
    if ((void *)(uintptr_t)vaddr == NULL) {
        TSDRV_PRINT_ERR("ioremap ts_sysctrl failed.\n");
        return;
    }
    set_tscpu_reg((void *)(uintptr_t)(vaddr + 0xD0), 0xB & 0xFFF);  /* [11:0] */
#endif
    iounmap((void *)(uintptr_t)vaddr);
    TSDRV_PRINT_INFO("disable ts disp daw.\n");
}

static void tsdrv_enable_ts_disp(void)
{
    u64 vaddr;

    vaddr = (u64)(uintptr_t)ioremap(PERI_SUBSYS_DISP_REG_BASE, PERI_SUBSYS_DISP_REG_SIZE);
    if ((void *)(uintptr_t)vaddr == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("ioremap ts_sysctrl failed.\n");
        return;
#endif
    }

    set_tscpu_reg((void *)(uintptr_t)(vaddr + 0xD0), 0xF & 0xFFF);
    iounmap((void *)(uintptr_t)vaddr);
    TSDRV_PRINT_INFO("enable ts disp daw.\n");
}

int tsdrv_soc_reset_tscpu(void __iomem *sysctrl, uint32_t tsid)
{
    void __iomem *rst_reg = NULL;
    int times;
    u32 reg_val;

    if (tsid >= DEVDRV_MAX_TS_NUM) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("devm_ioremap invalid tsid=%u.\n", tsid);
        return -1;
#endif
    }

    if (sysctrl == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("devm_ioremap failed.\n");
        return -ENOMEM;
#endif
    }

    /* comfirm ts is idle, 0xBE0 is tscpu boot reg */
    rst_reg = sysctrl + 0xBE0;
    times = 0;
    while (1) {
        reg_val = readl(rst_reg);
        if (reg_val == 0x1) {
            TSDRV_PRINT_INFO("Ts suspend ready. (current status=%x; times=%d)\n", reg_val, times);
            break;
        }
        tsdrv_sleep(100);
        times++;

        TSDRV_PRINT_INFO("Waiting ts suspend. (current_status=%x; times=%d)\n", reg_val, times);
        if (times >= DEVDRV_POLL_TS_SUSPEND_TIME) {
            TSDRV_PRINT_ERR("ts is not idles status.\n");
            return -1;
        }
    }

    /* close A55 clk */
    rst_reg = sysctrl + 0xC0;
    set_tscpu_reg(rst_reg, 0x1F1);

    /* A55 reset */
    rst_reg = sysctrl + 0xCC;
    set_tscpu_reg(rst_reg, 0x1FD10);
    rst_reg = sysctrl + 0x134;
    set_tscpu_reg(rst_reg, 0x110);

    /* enable ts address protect, [31:1] reserved */
    rst_reg = sysctrl + 0x11C;
    set_tscpu_reg(rst_reg, 0x0);

    /* close IP clk */
    rst_reg = sysctrl + 0x1CC;
    set_tscpu_reg(rst_reg, 0x588);

    /* IP soft reset */
    rst_reg = sysctrl + 0x1D4;
    set_tscpu_reg(rst_reg, 0x458);
    rst_reg = sysctrl + 0x134;
    set_tscpu_reg(rst_reg, 0x1);

    /* disable disp DAW */
    tsdrv_disable_ts_disp();

    wmb();
    TSDRV_PRINT_INFO(" suspend ts done .\n");
    return 0;
}
EXPORT_SYMBOL(tsdrv_soc_reset_tscpu);

int tsdrv_soc_dereset_tscpu(void __iomem *sysctrl)
{
    void __iomem *rst_reg = NULL;

    if (sysctrl == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("devm_ioremap failed.\n");
        return -ENOMEM;
#endif
    }

    tsdrv_enable_ts_disp();

    /* cluster cold reset */
    rst_reg = sysctrl + TSCPU_SOFT_RESET_ENABLE_REG_OFFSET;
    set_tscpu_reg(rst_reg, 0x0FFD10);

    rst_reg = sysctrl + SOC_TS_SYSCTRL_PERIPHCTRL38_OFFSET;
    set_tscpu_reg(rst_reg, 0x111);

    rst_reg = sysctrl + SOC_TS_SYSCTRL_PERIPHCTRL58_OFFSET;
    set_tscpu_reg(rst_reg, 0x80080008);

    rst_reg = sysctrl + SOC_TS_SYSCTRL_PERIPHCTRL55_OFFSET;
    set_tscpu_reg(rst_reg, 0x80080108);

    wmb();

    /* Close clock */
    rst_reg = sysctrl + TSCPU_CLOCK_DISABLE_REG_OFFSET;
    set_tscpu_reg(rst_reg, 0x1F1);

    rst_reg = sysctrl + TSCPU_SOFT_RESET_ENABLE_REG_OFFSET;
    set_tscpu_reg(rst_reg, 0x7FFF);

    rst_reg = sysctrl + SOC_TS_SYSCTRL_PERIPHCTRL38_OFFSET;
    set_tscpu_reg(rst_reg, 0x7F0);

    /* Reset evacuation */
    rst_reg = sysctrl + LOW_POWER_DEBUG;
    set_tscpu_reg(rst_reg, 0x7FFF);

    rst_reg = sysctrl + SOC_TS_SYSCTRL_PERIPHCTRL39_OFFSET;
    set_tscpu_reg(rst_reg, 0x7F0);

    /* Enable clock */
    rst_reg = sysctrl + TSCPU_CLOCK_GATE_ENABLE_REG_OFFSET;
    set_tscpu_reg(rst_reg, 0x1F1);

    wmb();

    return 0;
}
EXPORT_SYMBOL(tsdrv_soc_dereset_tscpu);

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

    while (((count--) != 0)) {
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
