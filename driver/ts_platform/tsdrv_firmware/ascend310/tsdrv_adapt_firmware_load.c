/*
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
#include <linux/list.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/rtc.h>
#include <linux/jiffies.h>
#include <linux/pm.h>
#include <linux/version.h>

#include "devdrv_manager_common.h"
#include "tsdrv_soc_common.h"
#include "devdrv_manager.h"
#include "tsdrv_firmware_load.h"
#include "tsdrv_adapt_firmware_load.h"
#include "tsdrv_aicpu_config.h"
#include "tsdrv_soc_pm.h"
#include "aicpu.h"
#include "config.h"

#include "tsdrv_osal_bbox.h"

#ifdef CONFIG_ACPI
int aicpu_get_dts_config(struct devdrv_info *dev_info, struct aicpu_dts_config *conf)
{
    u64 freq = 0;
    if (conf->flag != 0) {
        return 0; /* already valid */
    }
    struct devdrv_manager_info *manager_info = devdrv_get_manager_info();
    READ_SYSTEM_COUNTER_FREQ(freq);
    devdrv_drv_info("SYSTEM COUNTER FREQ:%llu\n", freq);
    /* for different hardware */
    if (dev_info->firmware_hardware_version == FIRMWARE_HW_FPGA) {
        conf->fw_cpu_id_base = FW_CPU_ID_BASW_OF_FPGA;
        conf->fw_cpu_num = FW_CPU_NUM_OF_FPGA;
    } else {
        conf->fw_cpu_id_base = FW_CPU_ID_BASW;
        conf->fw_cpu_num = FW_CPU_NUM;
    }
    conf->fw_cpu_id_base = manager_info->dev_info[0]->ai_cpu_core_id;
    conf->fw_cpu_num = manager_info->dev_info[0]->ai_cpu_core_num;
    devdrv_drv_info("AICPU BASE ID %u NUM %u\n", conf->fw_cpu_id_base, conf->fw_cpu_num);
    conf->system_cnt_freq = freq;

    /* vaild setting */
    conf->flag = 1;

    devdrv_drv_info("aicpu frimware config set ok \n");

    return 0;
}
#else
int aicpu_get_dts_config(struct devdrv_info *dev_info, struct aicpu_dts_config *conf)
{
    struct devdrv_manager_info *manager_info = devdrv_get_manager_info();
    u64 freq = 0;
    struct device_node *dev_np = NULL;
    dev_np = dev_info->dev->of_node;

    if (conf->flag != 0) {
        return 0;
    }
    READ_SYSTEM_COUNTER_FREQ(freq);
    devdrv_drv_info("SYSTEM COUNTER FREQ:%llu\n", freq);
    /* for different hardware */
    if (dev_info->firmware_hardware_version == FIRMWARE_HW_FPGA) {
        conf->fw_cpu_id_base = FW_CPU_ID_BASW_OF_FPGA;
        conf->fw_cpu_num = FW_CPU_NUM_OF_FPGA;
    } else {
        conf->fw_cpu_id_base = FW_CPU_ID_BASW;
        conf->fw_cpu_num = FW_CPU_NUM;
    }
    conf->fw_cpu_id_base = manager_info->dev_info[0]->ai_cpu_core_id;
    conf->fw_cpu_num = manager_info->dev_info[0]->ai_cpu_core_num;
    devdrv_drv_info("AICPU BASE ID %u NUM %u\n", conf->fw_cpu_id_base, conf->fw_cpu_num);
    conf->system_cnt_freq = freq;
    if (of_property_read_u32(dev_np, "aicpu,ts2ai_interrupt", &conf->ts_int_start_id)) {
        return -1;
    }

    if (of_property_read_u32(dev_np, "aicpu,host_interrpt", &conf->ctrl_cpu_int_start_id)) {
        return -1;
    }

    if (of_property_read_u32(dev_np, "aicpu,ipc_cpu_interrpt", &conf->ipc_cpu_int_start_id)) {
        return -1;
    }

    if (of_property_read_u32(dev_np, "aicpu,ipc_mbx_interrpt", &conf->ipc_mbx_int_start_id)) {
        return -1;
    }

    /* vaild setting */
    conf->flag = 1;

    devdrv_drv_info("aicpu frimware config set ok \n");

    return 0;
}
#endif

STATIC void soc_dereset_core_tscpu(struct device *dev, u64 reg_base, u32 firmware_hardware_version)
{
    void __iomem *sysctrl = NULL;
    void __iomem *rst_reg = NULL;

    if (dev == NULL) {
        devdrv_drv_err("dev_manager_info is invalid.\n");
        return;
    }
    sysctrl = devm_ioremap(dev, reg_base, 0x100);
    rst_reg = sysctrl + TSCPU_CLOCK_GATE_ENABLE_REG_OFFSET;
    set_tscpu_reg(rst_reg, 0x3F);
    rst_reg = sysctrl + TSCPU_SOFT_RESET_ENABLE_REG_OFFSET;
    set_tscpu_reg(rst_reg, 0x7FF);

    rst_reg = sysctrl + LOW_POWER_DEBUG;

    if (firmware_hardware_version == FIRMWARE_HW_ESL)
        set_tscpu_reg(rst_reg, 0x1FF);
    else
        set_tscpu_reg(rst_reg, 0x7FF);
    wmb();
    devm_iounmap(dev, sysctrl);
}

STATIC void __iomem *tsdrv_aicpu_system_addr_remap(u32 devid, u32 tsid, u32 chipid, u32 dieid)
{
    void __iomem *vaddr = NULL;

    vaddr = ioremap_cache(SYSTEM_CONFIG_BASE + (CHIP_BASE_PA_DDR * devid), SYSTEM_CONFIG_SIZE);
    if (vaddr == NULL) {
        devdrv_drv_err("Ioremap aicpu system config failed. (devid=%u; tsid=%u)\n",
            devid, tsid);
        return NULL;
    }

    return vaddr;
}

STATIC void tsdrv_config_boot_info(struct devdrv_info *dev_info, struct aicpu_system_config *config)
{
    config->flag = SYSTEM_CONFIG_FLAG;
    config->ts_blackbox_base = dev_info->fw_info.ts_blackbox_base;
    config->ts_blackbox_size = dev_info->fw_info.ts_blackbox_size;
    config->ts_start_log_base = dev_info->fw_info.ts_start_log_base;
    config->ts_start_log_size = dev_info->fw_info.ts_start_log_size;
    config->enable_bbox = dev_info->fw_info.enable_bbox;
    config->ts_boot_addr = (u64)dev_info->fw_info.ts_boot_addr;
    config->ts_host_irq_base = 0;
    config->ts_host_irq_mode = 0;
    return;
}

STATIC void tsdrv_flash_get_aicpu_config(struct devdrv_info *dev_info, struct aicpu_system_config *config)
{
    int ret;
    u32 aicpu_occupy_bitmap = 0;
    unsigned long bitmap_tmp = 0;

    ret = devdrv_flash_get_aicpu_config(AICPU_CONFIG_NAME, &aicpu_occupy_bitmap);
    if (ret == -ENODEV) {
        devdrv_drv_err("get the aicpu config fail, aicpu config:%u\n", aicpu_occupy_bitmap);
        aicpu_occupy_bitmap = 0;
    } else if (ret == -EINVAL) {
        devdrv_drv_info("not set the aicpu config in flash\n");
        aicpu_occupy_bitmap = 0;
    }

    if (aicpu_occupy_bitmap != 0) {
        config->aicpu_id_base = __ffs(aicpu_occupy_bitmap);
        config->aicpu_num = __fls(aicpu_occupy_bitmap) - __ffs(aicpu_occupy_bitmap) + 1;
        dev_info->ai_cpu_core_id = config->aicpu_id_base;
        dev_info->ai_cpu_core_num = config->aicpu_num;
        dev_info->aicpu_occupy_bitmap = aicpu_occupy_bitmap;
        dev_info->dts_cfg.fw_cpu_id_base = config->aicpu_id_base;
        dev_info->dts_cfg.fw_cpu_num = config->aicpu_num;
        dev_info->ctrl_cpu_core_num = num_possible_cpus() - config->aicpu_num;
        bitmap_set(&bitmap_tmp, 0, dev_info->ctrl_cpu_core_num);
        dev_info->ctrl_cpu_occupy_bitmap = (u32)bitmap_tmp;
    } else {
#ifndef TSDRV_UT
        config->aicpu_id_base = dev_info->dts_cfg.fw_cpu_id_base;
        config->aicpu_num = dev_info->dts_cfg.fw_cpu_num;
#endif
    }
}

STATIC int aicpu_set_system_config(struct devdrv_info *dev_info, u32 tsid)
{
    struct aicpu_system_config *config = NULL;
    u32 *status = NULL;
    void __iomem *vaddr = NULL;
    u32 num;
    u32 i;

    vaddr = tsdrv_aicpu_system_addr_remap(dev_info->dev_id, tsid, dev_info->chip_id, dev_info->die_id);
    if (vaddr == NULL) {
        return -EINVAL;
    }

    config = (struct aicpu_system_config *)((uintptr_t)vaddr);
    tsdrv_config_boot_info(dev_info, config);
    tsdrv_flash_get_aicpu_config(dev_info, config);

    config->ts_int_start_id = dev_info->dts_cfg.ts_int_start_id;
    config->ctrl_cpu_int_start_id = dev_info->dts_cfg.ctrl_cpu_int_start_id;
    config->ipc_cpu_int_start_id = dev_info->dts_cfg.ipc_cpu_int_start_id;
    config->ipc_mbx_int_start_id = dev_info->dts_cfg.ipc_mbx_int_start_id;
    config->firmware_bin_size = (u32)FIRMWARE_SIZE;
    config->system_cnt_freq = dev_info->dts_cfg.system_cnt_freq;
    config->print_init_flag = 1;
    config->alg_memory_size = ALG_MEM_SIZE;
    config->aicpu_alloc_size = dev_info->fw_info.aicpu_fw_mem_size;
    config->ts_alloc_size = DEVDRV_TS_MEMORY_SIZE;
    config->sq_pa_base = DEVDRV_RESERVE_MEM_BASE + (long)(unsigned)tsid * DEVDRV_RESERVE_MEM_SIZE +
                          (long)(unsigned)dev_info->dev_id * CHIP_BASE_PA_DDR;
    config->sq_pa_size = DEVDRV_RESERVE_MEM_SIZE;
    for (i = 0; i < CONFIG_CPU_MAX_NUM; i++) {
        config->alg_memory_base[i] = virt_to_phys(dev_info->fw_info.alg_mem[i]);
    }

    num = dev_info->dts_cfg.fw_cpu_id_base + dev_info->dts_cfg.fw_cpu_num;
    status = (u32 *)(uintptr_t)(vaddr + (SYSTEM_FIRMWARE_STATUS - SYSTEM_CONFIG_BASE));
    for (i = dev_info->dts_cfg.fw_cpu_id_base; i < num; i++) {
        status[i] = 0;
    }

    devdrv_config_ts_dma_chan_info(dev_info, (u64)(uintptr_t)vaddr);

    (void)devdrv_construct_aicpu_dt((u64)(uintptr_t)vaddr, dev_info->dev_id, tsid);
    devdrv_config_ts_valid_mem_range(dev_info, (u64)(uintptr_t)vaddr);
    tsdrv_flush_cache(dev_info, (u64)((uintptr_t)config), (u32)SYSTEM_CONFIG_SIZE);

    iounmap(config);
    aicpu_set_current_time(dev_info);
    return 0;
}

int aicpu_dereset(u32 tsid, struct devdrv_info *dev_info)
{
    struct devdrv_platform_data *pdata = NULL;
    static u32 flag = 0;
    u64 reg_base;
    u64 ts_subsysctl_base;

    pdata = (struct devdrv_platform_data *)dev_info->pdata;
    if (flag == 0) {
        ts_subsysctl_base = pdata->platform_info.devdrv_addr_base[DEVDRV_DTS_TS_SUBSYSCTL_INDEX];
        tsdrv_soc_startup(ts_subsysctl_base, dev_info->fw_info.ts_boot_addr);
        if (aicpu_set_system_config(dev_info, tsid)) {
            devdrv_drv_err("Set aicpu system config failed. (devid=%u)", dev_info->dev_id);
            return -EINVAL;
        }
    }

    flag = 1;
    reg_base = pdata->platform_info.devdrv_addr_base[DEVDRV_DTS_TS_SUBSYSCTL_INDEX];
    soc_dereset_core_tscpu(dev_info->dev, reg_base, dev_info->firmware_hardware_version);
    isb();
    return 0;
}

int devdrv_fw_load_init(struct devdrv_info *dev_info, u32 tsid)
{
    struct devdrv_platform_data *pdata = NULL;
    struct semaphore *core_info_sema = NULL;
    u32 __iomem *sc_testreg = NULL;
    struct timespec64 os_time;
    u32 reg_val;
    u32 times;
    int i;
    int ret;

    dev_info->inuse.ai_core_num = dev_info->ai_core_num;
    dev_info->inuse.ai_core_error_bitmap = 0;
    dev_info->inuse.ai_cpu_num = dev_info->ai_cpu_core_num;
    dev_info->inuse.ai_cpu_error_bitmap = 0;

    pdata = (struct devdrv_platform_data *)dev_info->pdata;
    tsdrv_enable_disp_nfe(pdata->platform_info.disp_base);
    tsdrv_enable_ts_disp(pdata->platform_info.disp_base);

    sc_testreg = (u32 *)((uintptr_t)((unsigned long)(uintptr_t)pdata->ts_pdata[tsid].ts_sysctl_vaddr +
        DEVDRV_SC_TESTREG_OFFSET));
    writel_relaxed(DEVDRV_SC_TESTREG_INIT, &sc_testreg[DEVDRV_SC_TESTREG_TS]);

    for (i = DEVDRV_SC_TESTREG_FIRST_AI_CPU; i <= DEVDRV_SC_TESTREG_LAST_AI_CPU; i++) {
        writel_relaxed(DEVDRV_SC_TESTREG_INIT, &sc_testreg[i]);
    }

    wmb();

    devdrv_mn_mutex_lock(dev_info->dev_id);
    core_info_sema = devdrv_get_core_info_sema(dev_info->dev_id);
    if (core_info_sema == NULL) {
        devdrv_mn_mutex_unlock(dev_info->dev_id);
        devdrv_drv_err("Get sema failed. (devid=%u)\n", dev_info->dev_id);
        return -EINVAL;
    }
    sema_init(core_info_sema, 0);

    ret = devdrv_load_cpu_fw(dev_info, tsid);
    if (ret != 0) {
        devdrv_mn_mutex_unlock(dev_info->dev_id);
        devdrv_drv_err("Load TS firmware failed. (devid=%u; tsid=%u)\n",
            dev_info->dev_id, tsid);
        return ret;
    }

#ifdef TSDRV_UT
    tsdrv_tsfw_kickstart(dev_info->dev_id, tsid);
#endif

    /* poll for TS load done */
    times = 0;
    while (1) {
        reg_val = readl_relaxed(sc_testreg);
        if ((reg_val >= DEVDRV_SC_TESTREG_TS_READY) && (reg_val != 0xFFFFFFFFUL)) {
            tsdrv_set_ts_status(dev_info->dev_id, 0, TS_WORK);
            devdrv_drv_info("TS firmware load success. (devid=%u; times=%u)\n", dev_info->dev_id, times);
            devdrv_drv_info("Set ts work. (devid=%u; tsid=%u)\n", dev_info->dev_id, tsid);
            devdrv_get_active_core(dev_info);
            pdata->ts_pdata[tsid].ts_load_fail = 0;
            break;
        }
#ifndef TSDRV_UT
        msleep(10);
        times++;
        if (times > 1000) {
            tsdrv_set_ts_status(dev_info->dev_id, 0, TS_DOWN);
            devdrv_drv_err("TS firmware load timeout. (devid=%u; reg_val=0x%x).\n", dev_info->dev_id, reg_val);
            devdrv_drv_info("Set ts down. (devid=%u; tsid=%u)\n", dev_info->dev_id, tsid);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
            ktime_get_coarse_real_ts64(&os_time);
#else
            os_time = current_kernel_time64();
#endif
            tsdrv_bbox_system_err_time(dev_info->dev_id, MNTN_TS0_START_ERROR_CODE, &os_time, 0);
            pdata->ts_pdata[tsid].ts_start_fail = 1;
            tsdrv_tsfw_boot_reg_print(sc_testreg, DEVDRV_SC_TESTREG_MAX_NUM);
            ret = -TS_STARTUP_FAILED;
            break;
        }
#endif /* TSDRV_UT */
    }
    devdrv_mn_mutex_unlock(dev_info->dev_id);
    return ret;
}

void *devdrv_tscpu_alloc_memory(struct devdrv_info *dev_info, u32 tsid, struct device *dev, dma_addr_t *dma_addr)
{
    if ((dev_info->dev_id >= DEVDRV_MAX_DAVINCI_NUM) || (tsid >= DEVDRV_MAX_TS_NUM)) {
        devdrv_drv_err("invalid param, devid=%u tsid=%u\n", dev_info->dev_id, tsid);
        return NULL;
    }

    if (dma_addr == NULL) {
        devdrv_drv_err("dma addr is NULL, devid=%u tsid=%u\n", dev_info->dev_id, tsid);
        return NULL;
    }

    return dma_alloc_coherent(dev, DEVDRV_TS_MEMORY_SIZE, dma_addr, GFP_KERNEL | __GFP_ACCOUNT);
}

int devdrv_config_get_pss_cfg_stub(u32 dev_id, int *sign)
{
#define PKCS_SIGN_TYPE_ON   0
    *sign = PKCS_SIGN_TYPE_ON;
    return 0;
}
