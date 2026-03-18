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
#include <linux/property.h>

#include "tsdrv_adapt_firmware_load.h"
#include "tsdrv_soc_pm.h"
#include "tsdrv_soc_common.h"
#include "tsdrv_firmware_load.h"
#include "devdrv_manager_common.h"
#include "devdrv_common.h"
#include "devdrv_pm.h"
#include "aicpu.h"
#include "drv_ipc.h"
#include "tsdrv_aicpu_config.h"
#include "tsdrv_common.h"
#include "user_cfg_interface.h"
#include "devdrv_platform.h"
#include "tsdrv_osal_bbox.h"
#include "tsdrv_log.h"

#define INT_START_ID_NUM 2

int aicpu_get_dts_config(struct devdrv_info *dev_info, struct aicpu_dts_config *conf)
{
    u32 int_start_id[MAX_CHIP_NUM][INT_START_ID_NUM] = {{ 65533, 0x7F47 }, { 65498, 0xbF47 },
                                                        { 65448, 0xdF47 }, { 65398, 0xfF47 }};
    u32 freq = 0;

    if (conf->flag != 0) {
        /* have been get dts config once */
        devdrv_drv_info("get dts config already. \n");
        return 0;
    }

    /* get freq */
    READ_SYSTEM_COUNTER_FREQ(freq);

    conf->fw_cpu_id_base = dev_info->ai_cpu_core_id;
    conf->fw_cpu_num = dev_info->ai_cpu_core_num;

    devdrv_drv_info("version=%u, control cpu num = %u, aicpu core num = %u\n", dev_info->firmware_hardware_version,
                    conf->fw_cpu_id_base, conf->fw_cpu_num);

    conf->system_cnt_freq = freq;

    conf->ts_int_start_id = int_start_id[dev_info->dev_id][0];
    conf->ctrl_cpu_int_start_id = int_start_id[dev_info->dev_id][1];
    conf->ipc_mbx_int_start_id = (u32)hisi_mbx_int_start_id(dev_info->dev_id);
    /* vaild setting */
    conf->flag = 1;

    devdrv_drv_info("aicpu firmware config set ok \n");

    return 0;
}

#ifdef CFG_SOC_PLATFORM_CLOUD
STATIC void soc_dereset_core_tscpu(struct device *dev, u64 reg_base, u32 firmware_hardware_version)
{
    void __iomem *sysctrl = NULL;
    void __iomem *rst_reg = NULL;

    if (dev == NULL) {
        devdrv_drv_err("dev_manager_info is invalid.\n");
        return;
    }
    sysctrl = devm_ioremap(dev, reg_base, 0x100);
    if (sysctrl == NULL) {
        devdrv_drv_err("devm_ioremap failed.\n");
        return;
    }
    /* cluster cold reset */
    rst_reg = sysctrl + TSCPU_SOFT_RESET_ENABLE_REG_OFFSET;
    set_tscpu_reg(rst_reg, 0xFFFFF);

    /* Close clock */
    rst_reg = sysctrl + TSCPU_CLOCK_DISABLE_REG_OFFSET;
    set_tscpu_reg(rst_reg, 0x1FF);

    /* Reset evacuation */
    rst_reg = sysctrl + LOW_POWER_DEBUG;
#ifdef CFG_SOC_PLATFORM_CLOUD_V2
    set_tscpu_reg(rst_reg, 0xFFF1F);
#else
    set_tscpu_reg(rst_reg, 0xFFFFF);
#endif
    /* Enable clock */
    rst_reg = sysctrl + TSCPU_CLOCK_GATE_ENABLE_REG_OFFSET;
    set_tscpu_reg(rst_reg, 0x1FF);

    wmb();
    devm_iounmap(dev, sysctrl);
}
#else
STATIC void soc_dereset_core_tscpu(struct device *dev, u64 reg_base, u32 firmware_hardware_version)
{
    void __iomem *sysctrl = NULL;
    void __iomem *rst_reg = NULL;

    if (dev == NULL) {
        devdrv_drv_err("device is invalid.\n");
        return;
    }
    sysctrl = devm_ioremap(dev, reg_base, 0x100);
    if (sysctrl == NULL) {
        devdrv_drv_err("devm ioremap failed. reg base=%pK\n", (void *)reg_base);
        return;
    }
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
#endif

#define STARS_SHIM_ADDR_BASE       0x680030000
#define STARS_SHIM_ADDR_SIZE       0x10000
#define STARS_SHIM_WD_OFFSET       (0xCULL)
#define STARS_SHIM_SHIFT_BIT_NUM   17

#define SC_ATS_ARADDR_SRC00_STARS  0x100
#define SC_ATS_ARADDR_LEN00_STARS  0x104
#define SC_ATS_ARADDR_DES00_STARS  0x108
#define SC_ATS_ARADDR_SRC01_STARS  0x10c
#define SC_ATS_ARADDR_LEN01_STARS  0x110
#define SC_ATS_ARADDR_DES01_STARS  0x114

#define SC_ATS_AWADDR_SRC00_STARS  0x200
#define SC_ATS_AWADDR_LEN00_STARS  0x204
#define SC_ATS_AWADDR_DES00_STARS  0x208
#define SC_ATS_AWADDR_SRC01_STARS  0x20c
#define SC_ATS_AWADDR_LEN01_STARS  0x210
#define SC_ATS_AWADDR_DES01_STARS  0x214

#define WINDOWN_15 15
#define WINDOWN_14 14

#define W15_DIE0_START_ADDR     0x80000000ULL       /* 高2g滑到chip0,die0 */
#define W15_DIE0_END_ADDR       0x8000000000ULL     /* tscpu最多访问40bits */
#define W14_DIE0_TSFW_START_ADDR    0x1019E00000ULL /* die0加载ts时需要滑到0x19e00000 */
#define W14_DIE0_TSFW_END_ADDR      0x101E900000ULL

#define W15_DIE1_START_ADDR     0x80000000ULL   /* 高2g滑到chip0,die0 */
#define W15_DIE1_END_ADDR       0x7FFFFFFFFULL
/* VA_PCIE_SUBSYS的128M跳过,由tsfw配置:die1需要访问die0的pcie寄存器,               双die从die0出pcie */
#define W14_DIE1_START_ADDR     0x900000000ULL
#define W14_DIE1_END_ADDR       0x8000000000ULL

#ifndef TSDRV_UT
static void tsdrv_set_read_addr_sliding(void *stars_shim_base, u64 start, u64 end, u64 offset, u8 window_idx)
{
    void *addr;
    u32 val;

    val = (u32)(offset >> STARS_SHIM_SHIFT_BIT_NUM);
    addr = (void *)((unsigned long)(uintptr_t)stars_shim_base + SC_ATS_ARADDR_SRC00_STARS +
            STARS_SHIM_WD_OFFSET * window_idx);
    writel_relaxed((u32)(start >> STARS_SHIM_SHIFT_BIT_NUM), addr);

    addr = (void *)((unsigned long)(uintptr_t)stars_shim_base + SC_ATS_ARADDR_LEN00_STARS +
            STARS_SHIM_WD_OFFSET * window_idx);
    writel_relaxed((u32)(end >> STARS_SHIM_SHIFT_BIT_NUM), addr);

    addr = (void *)((unsigned long)(uintptr_t)stars_shim_base + SC_ATS_ARADDR_DES00_STARS +
            STARS_SHIM_WD_OFFSET * window_idx);
    writel_relaxed(val, addr);
}

static void tsdrv_set_write_addr_sliding(void *stars_shim_base, u64 start, u64 end, u64 offset, u8 window_idx)
{
    void *addr;
    u32 val;

    val = (u32)(offset >> STARS_SHIM_SHIFT_BIT_NUM);
    addr = (void *)((unsigned long)(uintptr_t)stars_shim_base + SC_ATS_AWADDR_SRC00_STARS +
            STARS_SHIM_WD_OFFSET * window_idx);
    writel_relaxed((u32)(start >> STARS_SHIM_SHIFT_BIT_NUM), addr);

    addr = (void *)((unsigned long)(uintptr_t)stars_shim_base + SC_ATS_AWADDR_LEN00_STARS +
            STARS_SHIM_WD_OFFSET * window_idx);
    writel_relaxed((u32)(end >> STARS_SHIM_SHIFT_BIT_NUM), addr);

    addr = (void *)((unsigned long)(uintptr_t)stars_shim_base + SC_ATS_AWADDR_DES00_STARS +
            STARS_SHIM_WD_OFFSET * window_idx);
    writel_relaxed(val, addr);
}
#endif

void tscpu_set_addr_sliding(struct device *dev, struct devdrv_info *dev_info)
{
    void *reg = NULL;
    u64 stars_shim_base;
    u32 chipid;
    u32 dieid;
    u64 offset;

    tsdrv_get_chip_version(dev_info, &chipid, &dieid);
#ifndef TSDRV_UT
    stars_shim_base = dev_info->pdata->ts_pdata[0].ts_sysctl_paddr - 0x10000;
    reg = ioremap(stars_shim_base, 0x400);
    if (reg == NULL) {
        devdrv_drv_err("devm_ioremap failed.\n");
        return;
    }

    devdrv_drv_info("devid(%u), chipid(%u), dieid(%u)\n", dev_info->dev_id, chipid, dieid);
    offset = tsdrv_get_addr_chip_die_offset(dev_info->dev_id, chipid, dieid);

    if (dieid == 1) {
        tsdrv_set_read_addr_sliding(reg, W15_DIE1_START_ADDR, W15_DIE1_END_ADDR, offset, WINDOWN_15);
        tsdrv_set_write_addr_sliding(reg, W15_DIE1_START_ADDR, W15_DIE1_END_ADDR, offset, WINDOWN_15);
        tsdrv_set_read_addr_sliding(reg, W14_DIE1_START_ADDR, W14_DIE1_END_ADDR, offset, WINDOWN_14);
        tsdrv_set_write_addr_sliding(reg, W14_DIE1_START_ADDR, W14_DIE1_END_ADDR, offset, WINDOWN_14);
    }

    if (dieid == 0) {
        /* tsfw addr 0 - 2G mirror sliding */
        /* tsfw加载时，die0访问0x1019e滑窗到0x19e，die1也是0x1019e，使用w15滑窗 */
        offset = 0xFFF000000000 & 0xFFFFFFFFFFFF;
        tsdrv_set_read_addr_sliding(reg, W14_DIE0_TSFW_START_ADDR, W14_DIE0_TSFW_END_ADDR, offset, WINDOWN_14);
        tsdrv_set_write_addr_sliding(reg, W14_DIE0_TSFW_START_ADDR, W14_DIE0_TSFW_END_ADDR, offset, WINDOWN_14);

        /* tsfw addr > 2G normal sliding */
        offset = tsdrv_get_addr_chip_die_offset(dev_info->dev_id, chipid, dieid);
        tsdrv_set_read_addr_sliding(reg, W15_DIE0_START_ADDR, W15_DIE0_END_ADDR, offset, WINDOWN_15);
        tsdrv_set_write_addr_sliding(reg, W15_DIE0_START_ADDR, W15_DIE0_END_ADDR, offset, WINDOWN_15);
    }

    iounmap(reg);
#endif
}

#ifndef CFG_SOC_PLATFORM_CLOUD_V2
STATIC void devdrv_get_bbox_config_info(u32 dev_id, struct aicpu_system_config *config)
{
    bbox_tsconfig *bbox_config = NULL;

    bbox_config = bbox_get_tsconfig(dev_id);
    if (bbox_config == NULL) {
        devdrv_drv_info("Not set ts bbox config. (devid=%u)\n", dev_id);
        config->ts_blackbox_base = 0;
        config->ts_blackbox_size = 0;
        config->ts_start_log_base = 0;
        config->ts_start_log_size = 0;
        config->enable_bbox = 0;
    } else {
        config->ts_blackbox_base = bbox_config->ts_paddr[TS_MNTN_BUFFER].addr;
        config->ts_blackbox_size = (u64)bbox_config->ts_paddr[TS_MNTN_BUFFER].len;
        config->ts_start_log_base = bbox_config->ts_paddr[TS_MNTN_START_LOG_BUFFER].addr;
        config->ts_start_log_size = (u64)bbox_config->ts_paddr[TS_MNTN_START_LOG_BUFFER].len;
        config->enable_bbox = bbox_config->enable_bbox;
        devdrv_drv_info("Set ts bbox config. (devid=%u; enable_bbox=%d)\n",
            dev_id, config->enable_bbox);
    }
}
#endif

STATIC void __iomem *tsdrv_aicpu_system_addr_remap(u32 devid, u32 tsid, u32 chipid, u32 dieid)
{
    void __iomem *vaddr = NULL;

#ifdef CFG_SOC_PLATFORM_CLOUD_V2
    u64 config_addr = ((dieid == 0) ? (SYSTEM_CONFIG_BASE - DIE0_HBM_ADDR_OFFSET) : (SYSTEM_CONFIG_BASE));
    if (dieid == 0) {
        vaddr = ioremap(config_addr, SYSTEM_CONFIG_SIZE);
    } else {
        #ifndef TSDRV_UT
        vaddr = ioremap(config_addr + tsdrv_get_addr_chip_die_offset(devid, chipid, dieid), SYSTEM_CONFIG_SIZE);
        #endif
    }
#else
    vaddr = ioremap(SYSTEM_CONFIG_BASE + (CHIP_BASE_PA_DDR * devid), SYSTEM_CONFIG_SIZE);
#endif

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
    return;
}

STATIC void tsdrv_config_cpu_info(struct devdrv_info *dev_info, struct aicpu_system_config *config)
{
    u32 num;
    u32 *status = NULL;
    u32 i;

#ifdef CFG_FEATURE_PG
    config->vpc_bitmap = dev_info->pg_info.comPgInfo.vpcPara.bitMap;
    config->jpegd_bitmap = dev_info->pg_info.comPgInfo.jpegdPara.bitMap;
    config->mata_bitmap = dev_info->pg_info.comPgInfo.mataPara.bitMap;
    config->mata_num = dev_info->pg_info.comPgInfo.mataPara.minNum;
#endif

    config->aicore_bitmap = dev_info->aicore_bitmap;
    config->aicore_freq = dev_info->aicore_freq;

    config->connect_protocol = devdrv_get_connect_protocol(dev_info->dev_id);
    devdrv_drv_info("(connect_protocol=%d)\n", config->connect_protocol);

    config->vector_core_bitmap = 0;
    config->vector_core_freq = 0;
    config->aicpu_id_base = dev_info->dts_cfg.fw_cpu_id_base;
    config->aicpu_num = dev_info->dts_cfg.fw_cpu_num;

    num = dev_info->dts_cfg.fw_cpu_id_base + dev_info->dts_cfg.fw_cpu_num;
    status = (u32 *)(uintptr_t)((uintptr_t)config + (SYSTEM_FIRMWARE_STATUS - SYSTEM_CONFIG_BASE));
    for (i = dev_info->dts_cfg.fw_cpu_id_base; i < num; i++) {
        status[i] = 0;
    }

    return;
}

STATIC int aicpu_set_system_config(struct devdrv_info *dev_info, u32 tsid)
{
    struct aicpu_system_config *config = NULL;
    void __iomem *vaddr = NULL;
    int ret;
    u32 i;

    vaddr = tsdrv_aicpu_system_addr_remap(dev_info->dev_id, tsid, dev_info->chip_id, dev_info->die_id);
    if (vaddr == NULL) {
        return -EINVAL;
    }

    config = (struct aicpu_system_config *)((uintptr_t)vaddr);
    tsdrv_config_boot_info(dev_info, config);
    tsdrv_config_cpu_info(dev_info, config);

#ifndef CFG_SOC_PLATFORM_CLOUD_V2
    config->ts_int_start_id = dev_info->dts_cfg.ts_int_start_id;
#endif
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
                          (long)(unsigned)dev_info->dev_id * CHIP_BASEADDR_PA_OFFSET;
    config->sq_pa_size = DEVDRV_RESERVE_MEM_SIZE;
#ifdef CFG_SOC_PLATFORM_CLOUD_V2
    ret = tsdrv_get_ffts_mcu_irq_id(dev_info->dev_id, &config->ffts_mcu_int_start_id);
    if (ret != 0) {
        devdrv_drv_err("Failed to get irq id. (devid=%u)\n", dev_info->dev_id);
        iounmap(config);
        return ret;
    }
#endif
#ifndef CFG_SOC_PLATFORM_CLOUD_V2
    devdrv_get_bbox_config_info(dev_info->dev_id, config);
#endif
    for (i = 0; i < CONFIG_CPU_MAX_NUM; i++) {
        config->alg_memory_base[i] = virt_to_phys(dev_info->fw_info.alg_mem[i]);
    }

    devdrv_config_ts_dma_chan_info(dev_info, (u64)(uintptr_t)vaddr);
    devdrv_config_ts_valid_mem_range(dev_info, (u64)(uintptr_t)vaddr);
    ret = devdrv_construct_aicpu_dt((u64)(uintptr_t)vaddr, dev_info->dev_id, tsid);
    if (ret != 0) {
        devdrv_drv_err("Construct aicpu dt failed. (devid=%u)\n", dev_info->dev_id);
        iounmap(config);
        return ret;
    }

    iounmap(config);
    aicpu_set_current_time(dev_info);

    devdrv_drv_info("the config of the boot para area is complete. (devid=%u)\n", dev_info->dev_id);
    return 0;
}

int aicpu_dereset(u32 tsid, struct devdrv_info *dev_info)
{
    struct devdrv_platform_data *pdata = NULL;
    u64 reg_base;
#ifdef CFG_SOC_PLATFORM_CLOUD_V2
    u32 chipid, dieid;
    u64 boot_addr;
#endif

    if (dev_info->dev_id >= MAX_CHIP_NUM) {
        devdrv_drv_err("invalid dev_id %u.\n", dev_info->dev_id);
        return -EINVAL;
    }
    pdata = (struct devdrv_platform_data *)dev_info->pdata;
    /* the boot address and system configuration only need to be configured once */

    /* config boot address */
    devdrv_drv_debug("config boot address\n");
#ifdef CFG_SOC_PLATFORM_CLOUD_V2
    tsdrv_get_chip_version(dev_info, &chipid, &dieid);
    boot_addr = (dieid == 0) ? TS_STATIC_ADDR_BASE + DIE_BASEADDR_HIGH_OFFSET_WITHIN_2G :
        TS_STATIC_ADDR_BASE + DIE_BASEADDR_HIGH_OFFSET_WITHIN_2G +
            tsdrv_get_addr_chip_die_offset(dev_info->dev_id, chipid, dieid);
    tsdrv_soc_startup(dev_info->dev_id, boot_addr,
        pdata->platform_info.devdrv_addr_base[DEVDRV_DTS_TS_SUBSYSCTL_INDEX], dev_info->env_type);
#else
    tsdrv_soc_startup(dev_info->dev_id, dev_info->fw_info.ts_boot_addr,
        pdata->platform_info.devdrv_addr_base[DEVDRV_DTS_TS_SUBSYSCTL_INDEX], dev_info->env_type);
#endif

    /* config system configuration */
    devdrv_drv_debug("config system configuration\n");
    if (aicpu_set_system_config(dev_info, tsid)) {
        devdrv_drv_err("config failed. dev_id %u.\n", dev_info->dev_id);
        return -EINVAL;
    }

    reg_base = pdata->platform_info.devdrv_addr_base[DEVDRV_DTS_TS_SUBSYSCTL_INDEX];
    devdrv_drv_info("Dereset TSCPU. (devid=%u)\n", dev_info->dev_id);
#ifdef CFG_SOC_PLATFORM_CLOUD_V2
    tscpu_set_addr_sliding(dev_info->dev, dev_info);
#endif
    soc_dereset_core_tscpu(dev_info->dev, reg_base, dev_info->firmware_hardware_version);
    isb();

    return 0;
}

void *devdrv_tscpu_alloc_memory(struct devdrv_info *dev_info, u32 tsid,
    struct device *dev, dma_addr_t *dma_addr)
{
    phys_addr_t phys_addr;
#ifdef CFG_SOC_PLATFORM_CLOUD_V2
    u32 dieid = 0;
    u32 chipid;
    DRV_CHECK_PTR(dma_addr, return NULL, "dma_addr is NULL\n");
    tsdrv_get_chip_version(dev_info, &chipid, &dieid);
    phys_addr = (dieid == 0) ? TS_STATIC_ADDR_BASE : TS_STATIC_ADDR_BASE + DIE_BASEADDR_HIGH_OFFSET_WITHIN_2G +
        tsdrv_get_addr_chip_die_offset(dev_info->dev_id, chipid, dieid);
#else
    u32 devid = dev_info->dev_id;

    DRV_CHECK_EXP_ACT(devid >= MAX_CHIP_NUM, return NULL, "invalid devid(%u)\n", devid);
    DRV_CHECK_PTR(dma_addr, return NULL, "dma_addr is NULL\n");
    phys_addr = TS_STATIC_ADDR_BASE + CHIP_BASEADDR_PA_OFFSET * devid;
#endif

    *dma_addr = phys_addr;
    return ioremap(phys_addr, DEVDRV_TS_MEMORY_SIZE);
}

int devdrv_fw_load_init(struct devdrv_info *dev_info, u32 tsid)
{
    struct devdrv_platform_data *pdata = NULL;
    struct semaphore *core_info_sema = NULL;
    u32 __iomem *sc_testreg = NULL;
    u32 times = 0;
    u32 reg_val;
    int ret;
    int i;

    DRV_CHECK_PTR(dev_info, return -EINVAL, "dev_info is NULL\n");
    DRV_CHECK_PTR(dev_info->pdata, return -EINVAL, "dev_info->pada is NULL\n");
    DRV_CHECK_EXP_ACT(dev_info->dev_id >= MAX_CHIP_NUM, return -EINVAL, "invalid devid(%u)\n", dev_info->dev_id);
    DRV_CHECK_EXP_ACT(tsid >= DEVDRV_MAX_TS_NUM, return -EINVAL,
        "invalid tsid(%u), devid(%u)\n", tsid, dev_info->dev_id);

    dev_info->inuse.ai_core_num = dev_info->ai_core_num;
    dev_info->inuse.ai_core_error_bitmap = 0;
    dev_info->inuse.ai_cpu_num = dev_info->ai_cpu_core_num;
    dev_info->inuse.ai_cpu_error_bitmap = 0;

    pdata = dev_info->pdata;
    sc_testreg = (u32 *)((unsigned long)(uintptr_t)pdata->ts_pdata[0].ts_sysctl_vaddr + DEVDRV_SC_TESTREG_OFFSET);
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
    while (1) {
        reg_val = readl_relaxed(sc_testreg);
        if ((reg_val >= DEVDRV_SC_TESTREG_TS_READY) && (reg_val != 0xFFFFFFFFUL)) {
            tsdrv_set_ts_status(dev_info->dev_id, tsid, TS_WORK);
            devdrv_drv_info("TS firmware load success. (devid=%u; reg_value=0x%x; times=%u)\n",
                dev_info->dev_id, reg_val, times);
#ifndef CFG_SOC_PLATFORM_CLOUD_V2
            devdrv_get_active_core(dev_info);
#endif
            pdata->ts_pdata[tsid].ts_start_fail = 0;

#ifdef CFG_SOC_PLATFORM_CLOUD_V2
            if (devdrv_load_ffts_fw(dev_info, tsid)) {
                devdrv_drv_err("load ffts mcu binary failed\n");
            }
#endif
            break;
        }

#ifdef CFG_SOC_PLATFORM_ESL_FPGA
        msleep(3000);
#else
        msleep(10);
#endif

        times++;
        /* the max number of times to load TS is 1000 */
        if (times > 1000) {
            tsdrv_set_ts_status(dev_info->dev_id, tsid, TS_DOWN);
            devdrv_drv_err("TS firmware load timeout. (devid=%u; reg_val=0x%x).\n", dev_info->dev_id, reg_val);
#ifndef TSDRV_UT
            tsdrv_bbox_system_err(dev_info->dev_id, MNTN_TS0_START_ERROR_CODE, 0);
#endif
            pdata->ts_pdata[tsid].ts_start_fail = 1;
            tsdrv_tsfw_boot_reg_print(sc_testreg, DEVDRV_SC_TESTREG_MAX_NUM);
            ret = -TS_STARTUP_FAILED;
            break;
        }
        devdrv_drv_debug("Retry load TS firmware. (devid=%u; times=%u; status=%u)\n",
            dev_info->dev_id, times, reg_val);
    }
    devdrv_mn_mutex_unlock(dev_info->dev_id);
    return ret;
}

int devdrv_config_get_pss_cfg_stub(u32 dev_id, int *sign)
{
    return devdrv_config_get_pss_cfg(dev_id, sign);
}
