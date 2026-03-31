/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "asp_misc.h"
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <securec.h>
#include "soc_adss_sub_subctrl_interface.h"
#include "soc_asp_cfg_interface.h"
#include "soc_asp_wrapctrl_interface.h"
#include "soc_disp_interface.h"
#include "mkp_aio.h"
#include "audio_fault.h"
#define COMP_ASP_MISC_NAME              "hisilicon,aspmisc"

#define AUDIO_SMMU_BYPASS_BIT_START     12
#define AUDIO_SMMU_BYPASS_BIT_END       14
#define SOC_DISP_DAW_EN_ASP             3
#define DALAY_TIME                      10
#define MAX_TIME_OUT                    100

enum asp_platform_type {
    ASP_PLATFORM_FPGA,
    ASP_PLATFORM_ASIC
};

enum asp_reg_type {
    ASP_WRAP_REG = 0,
    ASP_ADSS_SUBCTRL_REG,
    ASP_CFG_REG,
    ASP_DISP_REG,
    ASP_REG_BUT
};

struct asp_misc_priv {
    void __iomem* asp_reg_base[(td_s32)ASP_REG_BUT];
    enum asp_platform_type platform_type;
};

static struct asp_misc_priv asp_misc_data;

static int asp_reg_map(struct device_node *node, struct asp_misc_priv *priv)
{
    int index;
    int ret = 0;
    void *base = NULL;

    for (index = (td_s32)ASP_WRAP_REG; index < (td_s32)ASP_REG_BUT; ++index) {
        base = of_iomap(node, index);
        if (!base) {
            aio_err_trace("of_iomap reg failed");
            ret = -ENOMEM;
            goto out;
        }
        priv->asp_reg_base[index] = base;
    }

    return ret;
out:
    for (index = index - 1; index >= (td_s32)ASP_WRAP_REG; --index) {
        iounmap(priv->asp_reg_base[index]);
        priv->asp_reg_base[index] = NULL;
    }

    return ret;
}

static void asp_reg_unmap(struct asp_misc_priv *priv)
{
    int index;

    for (index = (td_s32)ASP_WRAP_REG; index < (td_s32)ASP_REG_BUT; ++index) {
        iounmap(priv->asp_reg_base[index]);
        priv->asp_reg_base[index] = NULL;
    }
}

static int asp_subsys_disable_clk(struct asp_misc_priv *priv)
{
    void *adss_subctrl_base = priv->asp_reg_base[ASP_ADSS_SUBCTRL_REG];

    writel(0x3FF, SOC_ADSS_SUB_SUBCTRL_SC_AUDIO_ICG_DIS_ADDR(adss_subctrl_base)); // value :0x3FF offset:0x404

    return 0;
}

static int asp_subsys_enable_clk(struct asp_misc_priv *priv)
{
    void *adss_subctrl_base = priv->asp_reg_base[ASP_ADSS_SUBCTRL_REG];

    writel(0x3FF, SOC_ADSS_SUB_SUBCTRL_SC_AUDIO_ICG_EN_ADDR(adss_subctrl_base)); // value :0x3FF offset:0x400
    return 0;
}

static int asp_subsys_reset(struct asp_misc_priv *priv)
{
    void *adss_subctrl_base = priv->asp_reg_base[ASP_ADSS_SUBCTRL_REG];

    writel(0x3B, SOC_ADSS_SUB_SUBCTRL_SC_AUDIO_RESET_REQ(adss_subctrl_base)); // value: 0x3B offset:0xA00
    return 0;
}

static int asp_crg_dereset(struct asp_misc_priv *priv)
{
    void *adss_subctrl_base = priv->asp_reg_base[ASP_ADSS_SUBCTRL_REG];

    writel(0x4, SOC_ADSS_SUB_SUBCTRL_SC_AUDIO_RESET_DREQ(adss_subctrl_base)); // value: 0x4 offset:0xA04
    return 0;
}

static int asp_subsys_dereset(struct asp_misc_priv *priv)
{
    void *adss_subctrl_base = priv->asp_reg_base[ASP_ADSS_SUBCTRL_REG];

    writel(0x3F, SOC_ADSS_SUB_SUBCTRL_SC_AUDIO_RESET_DREQ(adss_subctrl_base)); // value: 0x3F offset:0xA04
    return 0;
}

static void asp_reset(void __iomem *asp_cfg_base)
{
    /* reset all asp modules */
    writel(0xFFFFFF, SOC_ASP_CFG_R_RST_CTRLEN_ADDR(asp_cfg_base)); // value: 0xFFFFFF offset:0x0
}

static void asp_dereset(void __iomem *asp_cfg_base)
{
    /* dereset all asp modules */
    writel(0xFFFFFFFF, SOC_ASP_CFG_R_RST_CTRLDIS_ADDR(asp_cfg_base)); // value: 0xFFFFFFFF offset:0x4
}

static void asp_enable_clk(void __iomem *asp_cfg_base)
{
    /* enable all asp modules clock */
    writel(0xFFFFFFFF, SOC_ASP_CFG_R_GATE_EN_ADDR(asp_cfg_base));
    /* enable asp clock divider */
    writel(0xFFFFFFFF, SOC_ASP_CFG_R_GATE_CLKDIV_EN_ADDR(asp_cfg_base)); // value: 0xFFFFFFFF offset:0x1c
    writel(0xFFFFFFFF, SOC_ASP_CFG_R_GATE_R_AXI2TDM_CFG_ADDR(asp_cfg_base)); // value: 0xFFFFFFFF offset:0x2e4
}

static int asp_config(struct asp_misc_priv *priv)
{
    void *asp_cfg_base = priv->asp_reg_base[ASP_CFG_REG];

    /* reset asp */
    asp_reset(asp_cfg_base);

    mdelay(DALAY_TIME);

    /* dereset asp */
    asp_dereset(asp_cfg_base);

    /* enable asp clock and clock divider */
    asp_enable_clk(asp_cfg_base);

    return 0;
}

static void asp_enable_dispatch(struct asp_misc_priv *priv)
{
    uint32_t val;
    void *adss_dispatch_base = priv->asp_reg_base[ASP_DISP_REG];

    val = readl(SOC_DISP_DAW_EN_ADDR(adss_dispatch_base));
    val |= (1 << SOC_DISP_DAW_EN_ASP);
    writel(val, SOC_DISP_DAW_EN_ADDR(adss_dispatch_base));
}

static int asp_disable_dispatch(struct asp_misc_priv *priv)
{
    uint32_t val;
    void *adss_dispatch_base = priv->asp_reg_base[ASP_DISP_REG];

    val = readl(SOC_DISP_DAW_EN_ADDR(adss_dispatch_base));
    val &= ~(1 << SOC_DISP_DAW_EN_ASP);
    writel(val, SOC_DISP_DAW_EN_ADDR(adss_dispatch_base));

    return 0;
}

static int asp_enable_stop_bus(struct asp_misc_priv *priv)
{
    uint32_t val;
    uint32_t time_out = 0;
    void *asp_wrap_base = priv->asp_reg_base[ASP_WRAP_REG];

    /* start to stop bus */
    val = readl(SOC_ASP_WRAPCTRL_SC_AUDIO_BUS_CTRL_ADDR(asp_wrap_base));
    val |= (1 << SOC_ASP_WRAPCTRL_SC_AUDIO_BUS_CTRL_audio_bus_stop_START);
    writel(val, SOC_ASP_WRAPCTRL_SC_AUDIO_BUS_CTRL_ADDR(asp_wrap_base));

    /* wait for bus stop to finish */
    val = readl(SOC_ASP_WRAPCTRL_SC_AUDIO_DWAXI_ST_ADDR(asp_wrap_base));
    while (time_out < MAX_TIME_OUT && !(val & (1 << SOC_ASP_WRAPCTRL_SC_AUDIO_DWAXI_ST_audio_bus_stop_ok_START))) {
        time_out++;
        mdelay(1);
        val = readl(SOC_ASP_WRAPCTRL_SC_AUDIO_DWAXI_ST_ADDR(asp_wrap_base));
    }

    /* time out */
    if (!(val & (1 << SOC_ASP_WRAPCTRL_SC_AUDIO_DWAXI_ST_audio_bus_stop_ok_START))) {
        aio_err_trace("wait for bus stop to finish time out");
        return -EBUSY;
    }

    return 0;
}

static int asp_disable_stop_bus(struct asp_misc_priv *priv)
{
    uint32_t val;
    void *asp_wrap_base = priv->asp_reg_base[ASP_WRAP_REG];

    val = readl(SOC_ASP_WRAPCTRL_SC_AUDIO_BUS_CTRL_ADDR(asp_wrap_base));
    val &= ~(1 << SOC_ASP_WRAPCTRL_SC_AUDIO_BUS_CTRL_audio_bus_stop_START);
    writel(val, SOC_ASP_WRAPCTRL_SC_AUDIO_BUS_CTRL_ADDR(asp_wrap_base));

    return 0;
}

static void asp_power_on(void)
{
    // STEP 1
    (void)asp_disable_dispatch(&asp_misc_data);

    // STEP 2
    (void)asp_subsys_disable_clk(&asp_misc_data);

    // STEP 3
    (void)asp_crg_dereset(&asp_misc_data);

    // STEP 4
    (void)asp_subsys_enable_clk(&asp_misc_data);

    // STEP 5
    (void)asp_subsys_dereset(&asp_misc_data);

    // STEP 6
    asp_enable_dispatch(&asp_misc_data);

    // STEP 7
    (void)asp_config(&asp_misc_data);
}

static void asp_power_off(void)
{
    // step 1.1,1.2 使能总线排空
    (void)asp_enable_stop_bus(&asp_misc_data);

    // step 1.3
    (void)asp_subsys_reset(&asp_misc_data);

    // step 1.4
    (void)asp_disable_stop_bus(&asp_misc_data);

    // step 2
    (void)asp_disable_dispatch(&asp_misc_data);

    // step 3
    (void)asp_subsys_disable_clk(&asp_misc_data);
}


void asp_do_resume(void)
{
    asp_power_on();
}

void asp_do_suspend(void)
{
    asp_power_off();
}

int asp_misc_init(struct platform_device *pdev)
{
    int ret;
    struct device_node *node = pdev->dev.of_node;
    if (!node) {
        aio_err_trace("dts[%s] node not found", COMP_ASP_MISC_NAME);
        ret = -ENODEV;
        return ret;
    }

    ret = memset_s((void*)&asp_misc_data, sizeof(struct asp_misc_priv), 0, sizeof(struct asp_misc_priv));
    if (ret) {
        aio_err_trace("memset asp_misc_data failed");
        ret = -ENODEV;
        return ret;
    }

    ret = asp_reg_map(node, &asp_misc_data);
    if (ret) {
        aio_err_trace("asp_reg_map failed");
        ret = -ENODEV;
        return ret;
    }

    asp_power_on();
    audio_fault_init();
    aio_info_trace("asp_config ok\n");

    return ret;
}

int asp_misc_exit()
{
    asp_power_off();
    audio_fault_exit();
    asp_reg_unmap(&asp_misc_data);
    aio_info_trace("asp_config exit ok\n");
    return 0;
}

MODULE_DESCRIPTION("ASP MISC Driver");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
