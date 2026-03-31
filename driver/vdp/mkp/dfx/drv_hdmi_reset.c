/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: hdmi debug.
 * Author: Hisilicon multimedia software group
 * Create: 2012/06/26
 */
#include "drv_hdmi_reset.h"
#include <linux/file.h>
#include <linux/major.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/hwspinlock.h>
#include <linux/pm_wakeup.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <securec.h>
#include "drv_hdmi_common.h"
#include "drv_hdmi_intf.h"
#include "drv_hdmi_event.h"
#include "drv_hdmi_edid.h"
#include "hdmi_osal.h"
#include "drv_hdmi_compatibility.h"
#include "drv_hdmi_debug.h"
#include "drv_hdmi_detection.h"

td_s32 hdmi_reset_clock(const drv_hdmi_reset_clk *reset_clk)
{
    td_u32 reset_mode = reset_clk->reset_mode;
    if (reset_mode > 1) {
        hdmi_dfx_err("reset_clk->reset_mode is out of range ,mode =%u\n", reset_mode);
        return TD_FAILURE;
    }
    hdmi_dfx_info("reset_clk->reset_mode==%u\n", reset_mode);

    td_u64 base = HDMI_TX_REG_BASE_ADDRESS;
    void *tx_pwd_rst_ctrl = NULL;
    tx_pwd_rst_ctrl = ioremap(base + TX_PWD_RST_CTRL_OFFSET, 4U);
    if (tx_pwd_rst_ctrl == NULL) {
        hdmi_dfx_err("tx_pwd_rst_ctrl remap failed!\n");
        return TD_FAILURE;
    }

    if (reset_mode == 1) {
        writel(0x401, tx_pwd_rst_ctrl); // reset
    } else if (reset_mode == 0) {
        writel(0x400, tx_pwd_rst_ctrl); // enable
    }
    iounmap(tx_pwd_rst_ctrl);

    return TD_SUCCESS;
}