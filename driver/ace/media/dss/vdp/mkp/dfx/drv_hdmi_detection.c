/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: hdmi debug.
 * Author: Hisilicon multimedia software group
 * Create: 2012/06/26
 */
#include "drv_hdmi_detection.h"
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

td_s32 hdmi_set_detection(drv_hdmi_detection *hdmi_detection)
{
    void *hdmi_reg_base = NULL;
    td_u64 base = HDMI_TX_REG_BASE_ADDRESS;
    hdmi_reg_base = ioremap(base, 0x2804);
    if (hdmi_reg_base == NULL) {
        hdmi_dfx_err("hdmi_reg_base remap failed!\n");
        return TD_FAILURE;
    }

    void *ctrl = hdmi_reg_base + HDMI_GEN_SUB_PKT1_H_OFFSET;
    void *hreg = hdmi_reg_base + HDMI_FDET_HORT_RES_OFFSET;
    void *vreg = hdmi_reg_base + HDMI_FDET_VERT_RES_OFFSET;
    hdmi_gen_sub_pkt1_h gen_sub_pkt1_h;
    gen_sub_pkt1_h.bits.gen_sub_pkt1_pb4 = 0xF8;
    gen_sub_pkt1_h.bits.gen_sub_pkt1_pb5 = 0x0;
    gen_sub_pkt1_h.bits.gen_sub_pkt1_pb6 = 0x0;
    writel(gen_sub_pkt1_h.u32, ctrl);

    gen_sub_pkt1_h.bits.gen_sub_pkt1_pb4 = 0xF0;
    writel(gen_sub_pkt1_h.u32, ctrl);

    hdmi_fdet_hort_res fdet_hort_res;
    hdmi_fdet_vert_res hort_vert_res;
    mdelay(100U);
    fdet_hort_res.u32 = readl(hreg);
    hort_vert_res.u32 = readl(vreg);
    td_u32 h_active = fdet_hort_res.bits.hsync_active_cnt;
    td_u32 v_active = hort_vert_res.bits.vsync_active_cnt;
    hdmi_dfx_info("format: [%u %u]\n", h_active, v_active);

    hdmi_detection->timing_height = v_active;
    hdmi_detection->timing_width = h_active;
    iounmap(hdmi_reg_base);

    return TD_SUCCESS;
}