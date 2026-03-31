/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2012-2022. All rights reserved.
 * Description: hdmi debug.
 * Author: Hisilicon multimedia software group
 * Create: 2012/06/26
 */
#include "drv_hdmi_hotplug.h"
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

td_s32 hdmi_set_hotplug(drv_hdmi_hotplug_status *hotplug_status)
{
    td_u64 base = HDMI_TX_REG_BASE_ADDRESS;
    void *hdmi_hotplug_state = NULL;
    hdmi_hotplug_state = ioremap(base + HDMI_HOTPLUG_STATE_OFFSET, 4U);
    if (hdmi_hotplug_state == NULL) {
        hdmi_dfx_err("hdmi_hotplug_state remap failed!\n");
        return TD_FAILURE;
    }

    hdmi_tx_aon_state hotplug_state;
    hotplug_state.u32 = readl(hdmi_hotplug_state);
    td_u32 result = hotplug_state.bits.hotplug_state;
    hdmi_dfx_info("hdmi hotplug_state==%u\n", result);
    hotplug_status->hotplug_on = result;
    iounmap(hdmi_hotplug_state);

    return TD_SUCCESS;
}