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
 * Description: usb subctrl
 * Author:huawei
 * Create: 2022-12-01
 */

#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/kallsyms.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include "usb_hisi.h"

enum {
    USB_0,
    USB_1,
    USB_2,
    USB_3,
    USB_MAX_NUM
};

#define IO_USB_RESET_REQ_REG               (0x438)
#define IO_USB_RESET_DREQ_REG              (0x43c)
#define IO_USB_RESET_ST_REG                (0x5438)

#define IO_PHY_RESET_REQ_REG               (0x440)
#define IO_PHY_RESET_DREQ_REG              (0x444)
#define IO_PHY_RESET_ST_REG                (0x5440)

#define IO_USB_ICG_EN_REG               (0x388)
#define IO_USB_ICG_DIS_REG              (0x38c)
#define IO_USB_ICG_ST_REG               (0x5388)

#define GPIO4_BASE                      (0xa0160000)
#define IOMUX_BASE                      (0xa0140000)
#define GPIO_DIR_OFFSET                 (0x4)


static u64 g_subctrl_base = 0xa0120000;

struct usb_crg_reset_cell {
    unsigned int reset_req_offset;
    unsigned int reset_dreq_offset;
    unsigned int reset_st_offset;
    unsigned int reset_mask;
};

struct usb_crg_clkgate {
    unsigned int en_offset;
    unsigned int dis_offset;
    unsigned int st_offset;
    unsigned int mask;
};

static struct usb_crg_reset_cell g_usb_subctrl_resets[USB_MAX_NUM] = {
    { IO_USB_RESET_REQ_REG, IO_USB_RESET_DREQ_REG, IO_USB_RESET_ST_REG, 0x111},
    { IO_USB_RESET_REQ_REG, IO_USB_RESET_DREQ_REG, IO_USB_RESET_ST_REG, 0x222},
    { IO_USB_RESET_REQ_REG, IO_USB_RESET_DREQ_REG, IO_USB_RESET_ST_REG, 0x444},
    { IO_USB_RESET_REQ_REG, IO_USB_RESET_DREQ_REG, IO_USB_RESET_ST_REG, 0x888},
};
static struct usb_crg_reset_cell g_phy_subctrl_resets[USB_MAX_NUM] = {
    { IO_PHY_RESET_REQ_REG, IO_PHY_RESET_DREQ_REG, IO_PHY_RESET_ST_REG, 0x0},
    { IO_PHY_RESET_REQ_REG, IO_PHY_RESET_DREQ_REG, IO_PHY_RESET_ST_REG, 0x1},
    { IO_PHY_RESET_REQ_REG, IO_PHY_RESET_DREQ_REG, IO_PHY_RESET_ST_REG, 0x2},
    { IO_PHY_RESET_REQ_REG, IO_PHY_RESET_DREQ_REG, IO_PHY_RESET_ST_REG, 0x4},
};
static struct usb_crg_clkgate g_usb_subctrl_clkgates[USB_MAX_NUM] = {
    { IO_USB_ICG_EN_REG, IO_USB_ICG_DIS_REG, IO_USB_ICG_ST_REG, 0x1},
    { IO_USB_ICG_EN_REG, IO_USB_ICG_DIS_REG, IO_USB_ICG_ST_REG, 0x2},
    { IO_USB_ICG_EN_REG, IO_USB_ICG_DIS_REG, IO_USB_ICG_ST_REG, 0x4},
    { IO_USB_ICG_EN_REG, IO_USB_ICG_DIS_REG, IO_USB_ICG_ST_REG, 0x8},
};

int hisi_subctrl_usb_init(struct hisi_usb *hiusb)
{
    hiusb->host.subctrl_vbase = devm_ioremap(hiusb->dev, g_subctrl_base, 0x10000);
    if (hiusb->host.subctrl_vbase == NULL) {
        pr_err("ioremap failed\n");
        return -ENOMEM;
    }
    return 0;
}

int hisi_subctrl_usb_reset_assert(struct hisi_usb *hiusb)
{
    u32 reg_offset;
    u32 reg_val;
    u32 mask;

    reg_offset = g_usb_subctrl_resets[hiusb->host_no].reset_req_offset;
    mask = g_usb_subctrl_resets[hiusb->host_no].reset_mask;

    reg_val = readl_relaxed(hiusb->host.subctrl_vbase + reg_offset);
    rmb();
    writel_relaxed((reg_val | mask), (hiusb->host.subctrl_vbase + reg_offset));
    rmb();
    return 0;
}

int hisi_subctrl_usb_phy_reset_assert(struct hisi_usb *hiusb)
{
    u32 reg_offset;
    u32 reg_val;
    u32 mask;

    reg_offset = g_phy_subctrl_resets[hiusb->host_no].reset_req_offset;
    mask = g_phy_subctrl_resets[hiusb->host_no].reset_mask;

    reg_val = readl_relaxed(hiusb->host.subctrl_vbase + reg_offset);
    rmb();
    writel_relaxed((reg_val | mask), (hiusb->host.subctrl_vbase + reg_offset));
    rmb();
    return 0;
}

int hisi_subctrl_usb_reset_deassert(struct hisi_usb *hiusb)
{
    u32 reg_offset;
    u32 reg_val;
    u32 mask;

    reg_offset = g_usb_subctrl_resets[hiusb->host_no].reset_dreq_offset;
    mask = g_usb_subctrl_resets[hiusb->host_no].reset_mask;
    reg_val = readl_relaxed(hiusb->host.subctrl_vbase + reg_offset);
    rmb();
    writel_relaxed((reg_val | mask), (hiusb->host.subctrl_vbase + reg_offset));
    rmb();
    return 0;
}

int hisi_subctrl_usb_phy_reset_deassert(struct hisi_usb *hiusb)
{
    u32 reg_offset;
    u32 reg_val;
    u32 mask;

    reg_offset = g_phy_subctrl_resets[hiusb->host_no].reset_dreq_offset;
    mask = g_phy_subctrl_resets[hiusb->host_no].reset_mask;
    reg_val = readl_relaxed(hiusb->host.subctrl_vbase + reg_offset);
    rmb();
    writel_relaxed((reg_val | mask), (hiusb->host.subctrl_vbase + reg_offset));
    rmb();
    return 0;
}

int hisi_subctrl_usb_get_reset_status(struct hisi_usb *hiusb)
{
    u32 reg_offset;
    u32 reg_val;
    u32 mask;

    reg_offset = g_usb_subctrl_resets[hiusb->host_no].reset_st_offset;
    mask = g_usb_subctrl_resets[hiusb->host_no].reset_mask;
    reg_val = readl_relaxed(hiusb->host.subctrl_vbase + reg_offset);
    rmb();

    return ((mask & reg_val) ? 1 : 0);
}

int hisi_subctrl_usb_clkgate_enable(struct hisi_usb *hiusb)
{
    u32 reg_offset;
    u32 reg_val;
    u32 mask;

    reg_offset = g_usb_subctrl_clkgates[hiusb->host_no].en_offset;
    mask = g_usb_subctrl_clkgates[hiusb->host_no].mask;

    reg_val = readl_relaxed(hiusb->host.subctrl_vbase + reg_offset);
    rmb();
    reg_val = reg_val | mask;
    writel_relaxed(reg_val, (hiusb->host.subctrl_vbase + reg_offset));
    rmb();

    return 0;
}

int hisi_subctrl_usb_clkgate_disable(struct hisi_usb *hiusb)
{
    u32 reg_offset;
    u32 reg_val;
    u32 mask;

    reg_offset = g_usb_subctrl_clkgates[hiusb->host_no].dis_offset;
    mask = g_usb_subctrl_clkgates[hiusb->host_no].mask;

    reg_val = readl_relaxed(hiusb->host.subctrl_vbase + reg_offset);
    rmb();
    reg_val = reg_val | mask;
    writel_relaxed(reg_val, (hiusb->host.subctrl_vbase + reg_offset));
    rmb();
    return 0;
}

struct rst_infos {
    phys_addr_t reg_phy_addr;       // 32bits register's phy addr
    u32 mask_bits;                  // 32bits register, set bits means these bits should be set value
    u32 mask_bits_value;            // set bits values only use for mask bits
    u32 delay_ns;                   // set register value then delay ns, some registers will take effect after set it
};

#define CACHE_NUM_PER_USB 2
struct rst_infos cache_line_cfg_arr[] = {
    {0x3604, 0b10000, 0b10000, 0},               /* not support integrity USB0 cachline read */
    {0x3614, 0b10000, 0b10000, 0},               /* not support integrity USB0 cachline write */
    {0x3624, 0b10000, 0b10000, 0},               /* not support integrity USB1 cachline read */
    {0x3634, 0b10000, 0b10000, 0},               /* not support integrity USB1 cachline write */
    {0x3644, 0b10000, 0b10000, 0},               /* not support integrity USB2 cachline read */
    {0x3654, 0b10000, 0b10000, 0},               /* not support integrity USB2 cachline write */
    {0x3664, 0b10000, 0b10000, 0},               /* not support integrity USB3 cachline read */
    {0x3674, 0b10000, 0b10000, 0},               /* not support integrity USB3 cachline write */
};

struct rst_infos oca_cfg_arr = {0x3690, 0b100000000, 0b100000000, 0};       /* USBC0~3 port_oca from OVRCUR */

int hisi_subctrl_usb_set_cacheline(struct hisi_usb *hiusb)
{
    u32 reg_val = 0;
    u32 set_val = 0;
    u32 idx = 0;

    for (idx = hiusb->host_no * CACHE_NUM_PER_USB;
        idx < hiusb->host_no * CACHE_NUM_PER_USB + CACHE_NUM_PER_USB; ++idx) {
        reg_val = ioread32(hiusb->host.subctrl_vbase + cache_line_cfg_arr[idx].reg_phy_addr);
        set_val = reg_val & (~cache_line_cfg_arr[idx].mask_bits);
        set_val |= cache_line_cfg_arr[idx].mask_bits_value;
        iowrite32(set_val, hiusb->host.subctrl_vbase + cache_line_cfg_arr[idx].reg_phy_addr);
        ndelay(cache_line_cfg_arr[idx].delay_ns);
    }
    return 0;
}

int hisi_subctrl_usb_set_oca_cfg(struct hisi_usb *hiusb)
{
    u32 reg_val = 0;
    u32 set_val = 0;

    reg_val = ioread32(hiusb->host.subctrl_vbase + oca_cfg_arr.reg_phy_addr);
    set_val = reg_val & (~oca_cfg_arr.mask_bits);
    set_val |= oca_cfg_arr.mask_bits_value;
    iowrite32(set_val, hiusb->host.subctrl_vbase + oca_cfg_arr.reg_phy_addr);
    ndelay(oca_cfg_arr.delay_ns);
    return 0;
}

int usb_get_clk_rst_info(struct hisi_usb *hiusb, struct device *dev)
{
    return 0;
}

struct rst_infos usb_iomux_gpio_5v[] = {
    {0xc0, 0x3, 0x2, 0},
    {0x80, 0x3, 0x3, 0},
    {0x8c, 0x3, 0x3, 0},
    {0x98, 0x3, 0x3, 0},
};

int usb_gpio_5v_bit[] = {0, 8, 11, 14};

int hisi_subctrl_usb_set_usb_5v_off(struct hisi_usb *hiusb)
{
    u32 reg_val = 0;
    void __iomem *g_usb_gpio4_vbase = 0;

    if (hiusb->host_no == 0)
        return 0;

    g_usb_gpio4_vbase = ioremap(GPIO4_BASE, 0x1000);
    if (g_usb_gpio4_vbase == NULL) {
        return -ENOMEM;
    }
    reg_val = ioread32(g_usb_gpio4_vbase + GPIO_DIR_OFFSET);
    reg_val |= BIT(usb_gpio_5v_bit[hiusb->host_no]);
    iowrite32(reg_val, g_usb_gpio4_vbase + GPIO_DIR_OFFSET);

    reg_val = ioread32(g_usb_gpio4_vbase);
    reg_val &= ~BIT(usb_gpio_5v_bit[hiusb->host_no]);
    iowrite32(reg_val, g_usb_gpio4_vbase);
    iounmap(g_usb_gpio4_vbase);
    return 0;
}

int hisi_subctrl_usb_set_usb_5v_on(struct hisi_usb *hiusb)
{
    u32 reg_val = 0;
    u32 set_val = 0;
    void __iomem *g_usb_iomux_vbase = 0;
    void __iomem *g_usb_gpio4_vbase = 0;

    g_usb_iomux_vbase = ioremap(IOMUX_BASE, 0x1000);
    if (g_usb_iomux_vbase == NULL) {
        return -ENOMEM;
    }
    reg_val = ioread32(g_usb_iomux_vbase + usb_iomux_gpio_5v[hiusb->host_no].reg_phy_addr);
    set_val = reg_val & (~usb_iomux_gpio_5v[hiusb->host_no].mask_bits);
    set_val |= usb_iomux_gpio_5v[hiusb->host_no].mask_bits_value;
    iowrite32(set_val, g_usb_iomux_vbase + usb_iomux_gpio_5v[hiusb->host_no].reg_phy_addr);
    ndelay(usb_iomux_gpio_5v[hiusb->host_no].delay_ns);
    iounmap(g_usb_iomux_vbase);

    if (hiusb->host_no == 0)
        return 0;

    g_usb_gpio4_vbase = ioremap(GPIO4_BASE, 0x1000);
    if (g_usb_gpio4_vbase == NULL) {
        return -ENOMEM;
    }
    reg_val = ioread32(g_usb_gpio4_vbase + GPIO_DIR_OFFSET);
    reg_val |= BIT(usb_gpio_5v_bit[hiusb->host_no]);
    iowrite32(reg_val, g_usb_gpio4_vbase + GPIO_DIR_OFFSET);

    reg_val = ioread32(g_usb_gpio4_vbase);
    reg_val |= BIT(usb_gpio_5v_bit[hiusb->host_no]);
    iowrite32(reg_val, g_usb_gpio4_vbase);
    iounmap(g_usb_gpio4_vbase);
    return 0;
}