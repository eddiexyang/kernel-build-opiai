/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
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
 * Author:huawei
 * Create: 2021-03-30
 */
#ifndef __USB_HISI_H__
#define __USB_HISI_H__

#include <linux/device.h>

struct usb_host_data {
    void __iomem *vaddr;
    u32 io_size;
    u32 oper_base;
    u32 runtime_base;
    void __iomem *pcs_vaddr;
    u32 pcs_size;
    u32 u2_disable;
    void __iomem *phy_vaddr;
    u32 phy_size;
    void __iomem *subctrl_vbase;
};

struct hisi_usb {
    struct device *dev;
    struct reset_control *rst;
    struct clk *clk;
    u32 host_no;
    struct usb_host_data host;
};

struct hisi_usb *get_hisi_usb(u32 host);
void usbdrv_disable_u2(void __iomem *base);
void usbdrv_axi_init(void __iomem *base);
void usbdrv_pcs_init(void __iomem *base);
void usbdrv_set_axi_qos(void __iomem *base, u32 qos);
u32 usbdrv_get_axi_qos(void __iomem *base);
void usbdrv_set_fs_ms_pbyte_num(void __iomem *base, u32 num);
void usbdrv_set_trpu_block_mode(void __iomem *base, u32 mode);
void usbdrv_set_trpu_resp_doorbell_time(void __iomem *base, u32 time);
#ifdef CONFIG_UDRV
int hisi_subctrl_usb_init(struct hisi_usb *hiusb);
int hisi_subctrl_usb_set_usb_5v_on(struct hisi_usb *hiusb);
int hisi_subctrl_usb_set_usb_5v_off(struct hisi_usb *hiusb);
#endif
#endif
