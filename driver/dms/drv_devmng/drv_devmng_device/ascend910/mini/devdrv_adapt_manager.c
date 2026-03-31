/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
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

#ifndef DEVMNG_UT
#ifdef CONFIG_GENERIC_BUG
#undef CONFIG_GENERIC_BUG
#endif
#ifdef CONFIG_BUG
#undef CONFIG_BUG
#endif

#include <linux/types.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/io.h>
#include <linux/dma-mapping.h>
#include <linux/uio_driver.h>

#include "devdrv_user_common.h"
#include "tsdrv_soc_pm.h"
#include "devdrv_common.h"
#include "devdrv_platform.h"
#include "devdrv_firmware_load.h"
#include "devdrv_pm.h"
#include "devdrv_manager.h"
#include "devdrv_platform_resource.h"
#include "devdrv_platform_register.h"
#include "devdrv_driver_pm.h"
#include "devdrv_adapt_manager.h"
#include "devdrv_dfm.h"
#include "devdrv_parse_pdata.h"

#define MINI_RC_1_BOARDID     1000
#define MINI_RC_2_BOARDID     2000
#define MINI_RC_EMMC_BOARDID  3004
#define MINI_RC_FLASH_BOARDID 4004

int devdrv_fw_load_init(struct devdrv_info *dev_info, u32 tsid)
{
    struct devdrv_platform_data *pdata = NULL;
    struct semaphore *core_info_sema = NULL;
    u32 __iomem *sc_testreg = NULL;
    u32 times = 0;
    u32 reg_val;
    int i;

    DRV_CHECK_PTR(dev_info, return -EINVAL, "dev_info is NULL\n");
    DRV_CHECK_PTR(dev_info->pdata, return -EINVAL, "dev_info->pdata is NULL\n");
    DRV_CHECK_EXP_ACT(dev_info->dev_id >= DEVDRV_MAX_DAVINCI_NUM, return -EINVAL, "Invalid devid(%u)\n",
                      dev_info->dev_id);
    DRV_CHECK_EXP_ACT(tsid >= DEVDRV_MAX_TS_NUM, return -EINVAL, "Invalid tsid(%u), devid(%u)\n", tsid,
                      dev_info->dev_id);

    dev_info->inuse.ai_core_num = dev_info->ai_core_num;
    dev_info->inuse.ai_core_error_bitmap = 0;
    dev_info->inuse.ai_cpu_num = dev_info->ai_cpu_core_num;
    dev_info->inuse.ai_cpu_error_bitmap = 0;

    pdata = dev_info->pdata;
    tsdrv_enable_disp_nfe(pdata->platform_info.disp_base);
    tsdrv_enable_ts_disp(pdata->platform_info.disp_base);

    sc_testreg = (u32 *)((uintptr_t)((unsigned long)(uintptr_t)pdata->ts_pdata[tsid].ts_sysctl_vaddr +
                                      DEVDRV_SC_TESTREG_OFFSET));
    writel_relaxed(DEVDRV_SC_TESTREG_INIT, &sc_testreg[DEVDRV_SC_TESTREG_TS]);

    for (i = DEVDRV_SC_TESTREG_FIRST_AI_CPU; i <= DEVDRV_SC_TESTREG_LAST_AI_CPU; i++) {
        writel_relaxed(DEVDRV_SC_TESTREG_INIT, &sc_testreg[i]);
    }
    wmb();

    devdrv_mn_mutex_lock(0);
    core_info_sema = devdrv_get_core_info_sema(dev_info->dev_id);
    if (core_info_sema != NULL) {
        sema_init(core_info_sema, 0);
    }
    if (devdrv_load_cpu_fw(dev_info, FIRMWARE_TYPE_TS)) {
        devdrv_drv_err("load ts binary failed\n");
    }
    /* poll for TS load done */
    while (1) {
        reg_val = readl_relaxed(sc_testreg);
        if ((reg_val >= DEVDRV_SC_TESTREG_TS_READY) && (reg_val != 0xFFFFFFFFUL)) {
            tsdrv_set_ts_status(dev_info->dev_id, tsid, TS_WORK);
            devdrv_drv_info("TS load succ, times: %d.\n", times);
            devdrv_get_active_core(dev_info);
            pdata->ts_pdata[tsid].ts_load_fail = 0;
            break;
        }
        msleep(10);
        times++;
        if (times > 1000) {
            tsdrv_set_ts_status(dev_info->dev_id, tsid, TS_DOWN);
            devdrv_drv_err("load TS timeout, reg_val=%pK.\n", (void *)(uintptr_t)reg_val);
            dfm_system_error_report(dev_info->dev_id, MNTN_TS_START_ERROR_CODE, 0);
            pdata->ts_pdata[tsid].ts_start_fail = 1;
            pdata->ts_pdata[tsid].ts_load_fail = 1;
            break;
        }
    }
    devdrv_mn_mutex_unlock(0);
    return 0;
}
EXPORT_SYMBOL(devdrv_fw_load_init);

u32 devdrv_get_freq(void)
{
    struct devdrv_manager_info *manager_info = devdrv_get_manager_info();
    struct devdrv_platform_data *pdata = NULL;
    u8 __iomem *sysctl = NULL;
    u8 __iomem *reg = NULL;
    u64 sysctrl_paddr;
    u32 mode = 4;
    u32 value;
    u32 freq;

    if (manager_info == NULL || manager_info->dev_info[CHIP0_ID] == NULL) {
        devdrv_drv_err("dev_manager_info is invalid.\n");
        return 0;
    }
    pdata = (struct devdrv_platform_data *)manager_info->dev_info[CHIP0_ID]->pdata;
    sysctrl_paddr = pdata->platform_info.devdrv_addr_base[DEVDRV_DTS_SYSCTL_INDEX];
    sysctl = devm_ioremap(manager_info->dev, sysctrl_paddr, DEVDRV_SYSCTL_REG_SIZE);
    if (sysctl != NULL) {
        reg = sysctl + DEVDRV_SYSCTL_PROFILE_RATE;
        value = readl_relaxed(reg);
        mode = value & 0xFF;

        devdrv_drv_debug("profile mode: %d.\n", mode);

        devm_iounmap(manager_info->dev, sysctl);
        sysctl = NULL;
    }

    switch (mode) {
        case 0:
            freq = 600;
            break;
        case 1:
        case 2:
            freq = 1200;
            break;
        case 3:
        case 4:
            freq = 1600;
            break;
        default:
            freq = 1600;
    };

    return freq;
}

unsigned long devdrv_manager_get_wait_time(struct devdrv_info *dev_info)
{
    int boardid = devdrv_get_boardid();
    unsigned long timeout;

    devdrv_drv_info("env_type = %u\n", dev_info->env_type);

    /*
    * try to inform host ai subsystem is ready,
    * if env type is EMU or boardid equals 1000, 2000, 3004 , 4004, set timeout to 50
    */
    if ((dev_info->env_type == DEVDRV_PLAT_TYPE_EMU) || (boardid == MINI_RC_1_BOARDID) ||
        (boardid == MINI_RC_2_BOARDID) || (boardid == MINI_RC_EMMC_BOARDID) ||
        (boardid == MINI_RC_FLASH_BOARDID)) {
        timeout = msecs_to_jiffies(DEVDRV_WAIT_TIME_SHORT_TIME);
    } else if (dev_info->env_type == DEVDRV_PLAT_TYPE_FPGA) {
        timeout = msecs_to_jiffies(DEVDRV_WAIT_TIME_FPGA_DC);
    } else if (dev_info->env_type == DEVDRV_PLAT_TYPE_ESL) {
        timeout = msecs_to_jiffies(DEVDRV_WAIT_TIME_SHORT_TIME);
    } else {
        devdrv_drv_info("wait time = %u\n", DEVDRV_WAIT_TIME_NORMAL);
        timeout = msecs_to_jiffies(DEVDRV_WAIT_TIME_NORMAL);
    }

#ifdef CFG_SOC_PLATFORM_HELPER
    timeout = msecs_to_jiffies(DEVDRV_WAIT_TIME_SHORT_TIME);
#endif

    devdrv_drv_info("boardid = %d, timeout=%lu\n", boardid, timeout);

    return timeout;
}

int devdrv_manager_nfe_irq_register(struct devdrv_info *dev_info)
{
    struct devdrv_platform_data *pdata = dev_info->pdata;
    int ret;

    tasklet_init(&dev_info->nfe_task, devdrv_ts_exception_task, ((unsigned long)(uintptr_t)dev_info));
    ret = request_irq(pdata->ts_pdata[0].disp_nfe_irq, devdrv_nfe_handler, IRQF_TRIGGER_RISING | IRQF_SHARED,
                      "nfe-exception", dev_info);
    if (ret)
        devdrv_drv_err("request irq failed\n");

    return ret;
}

void devdrv_free_tsfw_mem(struct device *dev, int size, void *addr, dma_addr_t dma_addr)
{
    DRV_CHECK_PTR(dev, return, "dev is NULL\n");
    DRV_CHECK_PTR(addr, return, "addr is NULL\n");
    DRV_CHECK_EXP_ACT(dma_addr == 0, return, "dma_addr is NULL\n");
    dma_free_coherent(dev, DEVDRV_TS_MEMORY_SIZE, addr, dma_addr);
}

int devdrv_get_l2_buffer(u32 devid, u64 *base, u64 *len)
{
    struct devdrv_info *dev_info = NULL;
    struct device_node *l2buff = NULL;
    struct resource r;
    int err;

    if ((base == NULL) || (len == NULL) || (devid >= DEVDRV_MAX_DAVINCI_NUM)) {
        devdrv_drv_err("invalid input handler.\n");
        return -EINVAL;
    }

    dev_info = dev_manager_info->dev_info[devid];
    if (dev_info == NULL) {
        devdrv_drv_err("device does not exist, devid = %u.\n", devid);
        return -EINVAL;
    }

    l2buff = of_parse_phandle(dev_info->dev->of_node, "l2buffer-region", 0);
    if (l2buff == NULL) {
        devdrv_drv_err("of_parse_phandle failed, devid = %u.\n", devid);
        return -EFAULT;
    }

    err = of_address_to_resource(l2buff, 0, &r);
    if (err) {
        devdrv_drv_err("of_address_to_resource failed, devid = %u, err = %d.\n", devid, err);
        return err;
    }

    *base = r.start;
    *len = resource_size(&r);

    devdrv_drv_debug("L2 buffer  size: 0x%llx.\n", *len);

    return 0;
}
EXPORT_SYMBOL(devdrv_get_l2_buffer);
#else
int devdrv_fw_load_init(struct devdrv_info *dev_info, unsigned int tsid)
{
    return 0;
}
#endif

