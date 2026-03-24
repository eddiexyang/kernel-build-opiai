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
 * Author:huawei
 * Create: 2022-12-05
 */
#ifndef __USB_MISC_H__
#define __USB_MISC_H__

#include <linux/device.h>
#include "usb_hisi.h"

int usb_get_clk_rst_info(struct hisi_usb *hiusb, struct device *dev);
int hisi_subctrl_usb_reset_assert(struct hisi_usb *hiusb);
int hisi_subctrl_usb_reset_deassert(struct hisi_usb *hiusb);
int hisi_subctrl_usb_phy_reset_assert(struct hisi_usb *hiusb);
int hisi_subctrl_usb_phy_reset_deassert(struct hisi_usb *hiusb);
int hisi_subctrl_usb_get_reset_status(struct hisi_usb *hiusb);
int hisi_subctrl_usb_clkgate_enable(struct hisi_usb *hiusb);
int hisi_subctrl_usb_clkgate_disable(struct hisi_usb *hiusb);
int hisi_subctrl_usb_set_cacheline(struct hisi_usb *hiusb);
int hisi_subctrl_usb_set_oca_cfg(struct hisi_usb *hiusb);

#endif
