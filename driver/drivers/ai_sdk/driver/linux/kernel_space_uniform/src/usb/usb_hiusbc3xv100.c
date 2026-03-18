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
 * Description: usb dfx api
 * Author:huawei
 * Create: 2022-04-20
 */
#include <linux/io.h>
#include "kdrv_usb_api.h"
#include "usb_hisi.h"

#define USB_REG_DRD_MODE 0x10004
#define USB_REG_DFX_EN 0x20700
#define USB_REG_REC_GOOD_PKT_NUM 0x2071c
#define USB_REG_REC_BAD_PKT_NUM 0x20720
#define USB_REG_SEND_PKT_NUM 0x20728

#define USB_REG_TRPU_HOST_DB_ITVL 0x3010
#define USB_REG_TRPU_HOST_CTRL_MODE 0x3018
#define USB_REG_FS_MS_PBYTE_NUM 0x4014
#define USB_REG_U3_CONTINUE_MODE 0x10600
#define USB_REG_PHY_SEL_MODE 0x10628
#define USB_REG_AXI_AXLEN 0x17000
#define USB_REG_AXI_QOS_MAP 0x17004
#define USB_REG_AXI_AXCACHE 0x17008

#define GSTS_CURMOD BIT(0)
#define GSTS_CURMOD_SHIFT 0

#define DFX_EN BIT(0)

union usb_axi_qos_map_cfg {
    struct {
        unsigned int csr_lmi_axi_qos_0 : 4; /* [3:0] */
        unsigned int csr_lmi_axi_qos_1 : 4; /* [7:4] */
        unsigned int rsv : 24; /* [31:8] */
    } bits;
    unsigned int u32;
};

union usb_trpu_host_ctrl_mode {
    struct {
        unsigned int trpu_host_block_mode : 1; /* [0] */
        unsigned int trpu_host_u2lpkt_mode : 1; /* [1] */
        unsigned int trpu_host_u3lpkt_mode : 1; /* [2] */
        unsigned int trpu_host_u2perf_mode : 1; /* [3] */
        unsigned int trpu_host_u3perf_mode : 1; /* [4] */
        unsigned int trpu_host_csnum_mode : 1; /* [5] */
        unsigned int rsv : 26; /* [31:6] */
    } bits;
    unsigned int u32;
};

union usb_fs_ms_pbyte_num {
    struct {
        unsigned int fs_ms_pbyte_num : 11; /* [10:0] */
        unsigned int rsv : 21; /* [31:11] */
    } bits;
    unsigned int u32;
};

union usb_resp_doorbell_time {
    struct {
        unsigned int resp_doorbell_time : 4; /* [3:0] */
        unsigned int rsv : 28; /* [31:4] */
    } bits;
    unsigned int u32;
};

union usb_u3_continue_mode {
    struct {
        unsigned int u3_continue_mode : 1; /* [0] */
        unsigned int usbc_u2_en : 1; /* [1] */
        unsigned int usbc_u3_en : 1; /* [2] */
        unsigned int u1_u2_timeout_clr_en : 1; /* [3] */
        unsigned int u3_link_fail_dfx_mode : 1; /* [4] */
        unsigned int rsv : 27; /* [31:5] */
    } bits;
    unsigned int u32;
};

union usb_phy_sel_mode {
    struct {
        unsigned int phy_sel_mode : 2; /* [1:0] */
        unsigned int rsv : 30; /* [31:2] */
    } bits;
    unsigned int u32;
};

enum usb_axi_axlen {
    USB_AXI_AXLEN_8 = 1,
    USB_AXI_AXLEN_16,
};

enum usb_axi_axcache {
    AXI_CACHE_DEV_NOBUF,
    AXI_CACHE_DEV_BUF,
    AXI_CACHE_NORM_NOCACHE_NOBUF,
    AXI_CACHE_NORM_NOCACHE_BUF,
    AXI_CACHE_WRITEBACK_RD_WR_ALLOC = 0xF,
};

enum usb_phy_mode {
    U2_PHY_ONLY,
    U3_PHY_ONLY,
    U2_U3_PHY_BOTH,
};

static void usb_set_axi_axlen(void __iomem *base)
{
    writel(USB_AXI_AXLEN_8, base + USB_REG_AXI_AXLEN);
}

static void usb_set_axi_axcache(void __iomem *base)
{
    writel(AXI_CACHE_WRITEBACK_RD_WR_ALLOC, base + USB_REG_AXI_AXCACHE);
}

static void usb_set_u2_en(void __iomem *base, u32 enable)
{
    union usb_u3_continue_mode value;
    value.u32 = readl(base + USB_REG_U3_CONTINUE_MODE);
    value.bits.usbc_u2_en = enable;
    writel(value.u32, base + USB_REG_U3_CONTINUE_MODE);
}

static void usb_set_phy_sel_mode(void __iomem *base, u32 mode)
{
    union usb_phy_sel_mode value;
    value.u32 = readl(base + USB_REG_PHY_SEL_MODE);
    value.bits.phy_sel_mode = mode;
    writel(value.u32, base + USB_REG_PHY_SEL_MODE);
}

void usbdrv_disable_u2(void __iomem *base)
{
    usb_set_u2_en(base, 0);
    usb_set_phy_sel_mode(base, U3_PHY_ONLY);
}

void usbdrv_axi_init(void __iomem *base)
{
    usb_set_axi_axlen(base);
    usb_set_axi_axcache(base);
}

void usbdrv_set_axi_qos(void __iomem *base, u32 qos)
{
    union usb_axi_qos_map_cfg value;
    value.u32 = readl(base + USB_REG_AXI_QOS_MAP);
    value.bits.csr_lmi_axi_qos_0 = qos;
    writel(value.u32, base + USB_REG_AXI_QOS_MAP);
}

u32 usbdrv_get_axi_qos(void __iomem *base)
{
    union usb_axi_qos_map_cfg value;
    value.u32 = readl(base + USB_REG_AXI_QOS_MAP);
    return value.bits.csr_lmi_axi_qos_0;
}

void usbdrv_set_fs_ms_pbyte_num(void __iomem *base, u32 num)
{
    union usb_fs_ms_pbyte_num value;
    value.u32 = readl(base + USB_REG_FS_MS_PBYTE_NUM);
    value.bits.fs_ms_pbyte_num = num;
    writel(value.u32, base + USB_REG_FS_MS_PBYTE_NUM);
}

void usbdrv_set_trpu_block_mode(void __iomem *base, u32 mode)
{
    union usb_trpu_host_ctrl_mode value;
    value.u32 = readl(base + USB_REG_TRPU_HOST_CTRL_MODE);
    value.bits.trpu_host_block_mode = mode;
    writel(value.u32, base + USB_REG_TRPU_HOST_CTRL_MODE);
}

void usbdrv_set_trpu_resp_doorbell_time(void __iomem *base, u32 time)
{
    union usb_resp_doorbell_time value;
    value.u32 = readl(base + USB_REG_TRPU_HOST_DB_ITVL);
    value.bits.resp_doorbell_time = time;
    writel(value.u32, base + USB_REG_TRPU_HOST_DB_ITVL);
}