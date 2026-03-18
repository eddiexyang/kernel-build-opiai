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
#include <linux/kallsyms.h>
#ifdef AOS_LLVM_BUILD
#include <linux/firmware.h>
#endif

#include "tsdrv_adapt_firmware_load.h"
#include "tsdrv_soc_common.h"
#include "tsdrv_soc_pm.h"
#include "tsdrv_firmware_load.h"
#include "tsdrv_firmware_reader.h"
#include "tsdrv_aicpu_config.h"
#include "tsdrv_common.h"
#include "tsdrv_log.h"
#include "devdrv_manager_common.h"
#include "devdrv_platform.h"
#include "devdrv_manager.h"
#include "aicpu.h"
#include "tsdrv_pdata.h"
#include "tsdrv_parse.h"

#include "tsdrv_osal_bbox.h"

#define RISCV_BIN_PATH_LENGTH 32

#define MINIV2_RETRY_TIMEOUT 200
#define MDC_RETRY_INTERVAL (10U)   /* 10 ms */
#define MDC_RETRY_TIMES_PER_SECOND (100U)

#define MAX_RISC_FW_NUM 2

#ifdef CONFIG_ACPI
int aicpu_get_dts_config(struct devdrv_info *dev_info, struct aicpu_dts_config *conf)
{
    u64 freq = 0;
    struct devdrv_manager_info *manager_info = NULL;

    if (conf->flag != 0) {
        return 0; /* already valid */
    }

    manager_info = devdrv_get_manager_info();
    if (manager_info == NULL) {
        devdrv_drv_err("manager_info is NULL.\n");
        return -1;
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
    devdrv_drv_debug("AICPU BASE ID %u NUM %u\n", conf->fw_cpu_id_base, conf->fw_cpu_num);
    conf->system_cnt_freq = freq;

#ifdef CFG_SOC_PLATFORM_MINIV3
    conf->ipc_mbx_int_start_id = hisi_mbx_int_start_id(dev_info->dev_id);
    devdrv_drv_info("Get ipc_mbx_int_start_id (%u).\n", conf->ipc_mbx_int_start_id);
#endif

    /* vaild setting */
    conf->flag = 1;

    return 0;
}
#else
int aicpu_get_dts_config(struct devdrv_info *dev_info, struct aicpu_dts_config *conf)
{
#ifndef AOS_LLVM_BUILD
    struct devdrv_manager_info *manager_info = devdrv_get_manager_info();
#endif
    u64 freq = 0;
    struct device_node *dev_np = NULL;
    dev_np = dev_info->dev->of_node;

    if (conf->flag != 0) {
        return 0;
    }
    READ_SYSTEM_COUNTER_FREQ(freq);
    devdrv_drv_info("SYSTEM COUNTER FREQ:%llu\n", freq);
#ifndef AOS_LLVM_BUILD
    conf->fw_cpu_id_base = manager_info->dev_info[dev_info->dev_id]->ai_cpu_core_id;
    conf->fw_cpu_num = manager_info->dev_info[dev_info->dev_id]->ai_cpu_core_num;
#else
    conf->fw_cpu_id_base = dev_info->ai_cpu_core_id;
    conf->fw_cpu_num = dev_info->ai_cpu_core_num;
#endif
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

#ifdef CFG_SOC_PLATFORM_MINIV3
    conf->ipc_mbx_int_start_id = hisi_mbx_int_start_id(dev_info->dev_id);
    devdrv_drv_info("Get ipc_mbx_int_start_id (%u).\n", conf->ipc_mbx_int_start_id);
#endif

    /* vaild setting */
    conf->flag = 1;

    devdrv_drv_info("aicpu frimware config set ok \n");

    return 0;
}
#endif

STATIC bool tsdrv_file_exist(const char *file_path)
{
#ifndef AOS_LLVM_BUILD
    struct file *fp = NULL;

    fp = filp_open(file_path, O_RDONLY, 0);
    if (IS_ERR_OR_NULL((void const *)fp)) {
        return false;
    }

    (void)filp_close(fp, NULL);
    fp = NULL;
#else
    const struct firmware *fw = NULL;
    int ret = request_firmware(&fw, file_path, NULL);
    if (ret || (fw == NULL)) {
        return false;
    }
    release_firmware(fw);
#endif
    return true;
}

#define DEVDRV_TS_BINARY_PATH_ASCEND610 "/var/tsfw_610.bin"
#define DEVDRV_TS_BINARY_PATH_BS9SX1A "/var/tsfw_615.bin"

STATIC const char *tsdrv_get_spec_name(enum tsdrv_hwts_spec ts_spec, char *spec_name, u32 size)
{
    if (ts_spec == HWTS_SPEC_BASE) {
        (void)strcpy_s(spec_name, size, "base");
    } else if (ts_spec == HWTS_SPEC_PREMIUM) {
        (void)strcpy_s(spec_name, size, "premium");
    } else if (ts_spec == HWTS_SPEC_ULTIMATE) {
        (void)strcpy_s(spec_name, size, "ultimate");
    } else {
        devdrv_drv_err("invalid ts specification.\n");
        return NULL;
    }
    return spec_name;
}

const char *tsdrv_get_tsfw_path(enum tsdrv_hwts_spec ts_spec, char *tsfw_path, u32 size)
{
    char spec_name[16];
    int ret = memset_s(tsfw_path, size, 0, size);
    if (ret != EOK) {
#ifndef TSDRV_UT
        devdrv_drv_warn("ignore memset tsfw path. (ret=%d).\n", ret);
#endif
    }

    if (tsdrv_get_spec_name(ts_spec, spec_name, sizeof(spec_name)) == NULL) {
        devdrv_drv_err("get hwts specification name failed\n");
        return NULL;
    }
#ifndef CFG_SOC_MDC_V51_LITE
    if (tsdrv_get_chip_type() == CHIP_TYPE_MDC_BS9SX1A) {
        (void)sprintf_s(tsfw_path, size, "/var/tsfw_%s_615.bin", spec_name);
    } else {
        (void)sprintf_s(tsfw_path, size, "/var/tsfw_%s_610.bin", spec_name);
    }
#else
    (void)sprintf_s(tsfw_path, size, "/var/tsfw_%s_610_lite.bin", spec_name);
#endif
    if (tsdrv_file_exist(tsfw_path)) {
        devdrv_drv_debug("fw file %s exist\n", tsfw_path);
        return tsfw_path;
    }

    /* If the file does not exist, the previous firmware is used by default */
    devdrv_drv_warn("file %s not exist. try to load no specification fw\n", tsfw_path);
    if (tsdrv_get_chip_type() == CHIP_TYPE_MDC_BS9SX1A) {
        (void)strcpy_s(tsfw_path, size, DEVDRV_TS_BINARY_PATH_BS9SX1A);
    } else {
        (void)strcpy_s(tsfw_path, size, DEVDRV_TS_BINARY_PATH_ASCEND610);
    }

    if (!tsdrv_file_exist(tsfw_path)) {
        devdrv_drv_info("file %s not exist. use default firmware %s\n", tsfw_path, DEVDRV_TS_BINARY_PATH);
        (void)strcpy_s(tsfw_path, size, DEVDRV_TS_BINARY_PATH);
    }

    return tsfw_path;
}

/* for STL */
#ifdef CFG_SOC_PLATFORM_MDC_V51
STATIC int devdrv_load_firmware_file(void __iomem *virt_addr, u64 virt_addr_size, const char *firmware_file)
{
#ifndef TSDRV_UT
    int ret = -EINVAL;
#ifndef AOS_LLVM_BUILD
    struct file *fp = NULL;
    size_t fsize;
    loff_t pos;
    fsize = get_file_size(firmware_file);
    if (fsize > virt_addr_size) {
        devdrv_drv_err("file size is too large. need little than %llu Byte, file=%s\n", virt_addr_size, firmware_file);
        return -ENOMEM;
    }

    fp = filp_open(firmware_file, O_RDONLY, 0);
    if (IS_ERR(fp)) {
        devdrv_drv_err("can't open stl file.\n");
        return ret;
    }

    pos = 0;

    if (!copy_firmware(fp, fsize, pos, virt_addr)) {
        devdrv_drv_err("ERROR: can't copy stl firmware file\n");
        (void)filp_close(fp, NULL);
        return -EIO;
    }
    devdrv_drv_info("file size=%lu.\n", fsize);

    isb();
    (void)filp_close(fp, NULL);
#else
    ret = tsdrv_get_firmware(firmware_file, virt_addr, virt_addr_size);
    if (ret) {
        return ret;
    }
#endif
#endif /* TSDRV_UT */
    return 0;
}

STATIC int devdrv_load_dcache_firmware(const struct devdrv_info *dev_info, struct aicpu_system_config *config)
{
#ifndef TSDRV_UT
#define   DUMP_ROW_SIZE  16
#define   DUMP_LENGTH    64
    void __iomem *dcache_vaddr = NULL;
    int ret;
    u32  max_size = 2 * TS_MDC_DCACHE_SIZE;
    dcache_vaddr = ioremap(TS_MDC_DCACHE_BASE, max_size);
    if (dcache_vaddr == NULL) {
        devdrv_drv_err("ioremap stl dcache failed. dev_id = %u\n", dev_info->dev_id);
        ret = -ENOMEM;
        goto _err;
    }

    if (((u64)(uintptr_t)dcache_vaddr & 0x1FFFFFF) != 0) {
        config->dcache_va = ((u64)(uintptr_t)dcache_vaddr & 0xFFFFFFFFFE000000) + TS_MDC_DCACHE_SIZE;
        config->dcache_pa = TS_MDC_DCACHE_BASE + (config->dcache_va - (u64)(uintptr_t)dcache_vaddr);
    } else {
        config->dcache_va = (u64)(uintptr_t)dcache_vaddr;
        config->dcache_pa = TS_MDC_DCACHE_BASE;
    }

    ret = devdrv_load_firmware_file((void *)(uintptr_t)config->dcache_va, TS_MDC_DCACHE_SIZE, "/var/lock_operator.bin");
    if (ret != 0) {
        devdrv_drv_err("load dcache firmware data error. dev_id = %u\n", dev_info->dev_id);
        goto _err;
    }

    devdrv_drv_info("Load dcache firmware. (va=%pK; pa=%pK)\n",
        (void *)(uintptr_t)config->dcache_va, (void *)(uintptr_t)config->dcache_pa);
#ifndef AOS_LLVM_BUILD
    print_hex_dump(KERN_DEBUG, "DCACHE:", 0, DUMP_ROW_SIZE, 1, (void *)(uintptr_t)config->dcache_va,
        DUMP_LENGTH, false);
#endif
    return 0;
_err:
    if (dcache_vaddr != NULL) {
        iounmap(dcache_vaddr);
    }
    config->dcache_va = 0;
    config->dcache_pa = 0;
    return ret;
#endif
}

STATIC int devdrv_load_stl_firmware(uintptr_t stl_pa, u64 stl_size, const char *firmware_file)
{
#ifndef TSDRV_UT
    int ret;
    void __iomem *stl_vaddr = NULL;

    stl_vaddr = ioremap(stl_pa, stl_size);
    if (stl_vaddr == NULL) {
        devdrv_drv_err("ioremap stl failed. \n");
        return -ENOMEM;
    }

    ret = devdrv_load_firmware_file((void *)(uintptr_t)stl_vaddr, stl_size, firmware_file);
    if (ret != 0) {
        devdrv_drv_err("load stl firmware file fail.\n");
        iounmap(stl_vaddr);
        stl_vaddr = NULL;
        return ret;
    }

    iounmap(stl_vaddr);
    stl_vaddr = NULL;
    return 0;
#endif
}

STATIC int devdrv_load_aic_aiv_stl(const struct devdrv_info *dev_info, struct aicpu_system_config *config, u32 tsid)
{
#ifndef TSDRV_UT
    int ret;

    config->stl_va = 0;
    config->stl_ssid = 0;
    config->stl_test_period = 0;
    if (dev_info->pdata->ts_pdata[tsid].stl_enable_flag == 0) {
        config->stl_va = SYSTEM_CONFIG_FLAG;
        devdrv_drv_info("The current configuration does not support the stl operator function. (tsid=%u).\n", tsid);
        return 0;
    } else {
        config->stl_test_period = get_ts_stl_test_period_from_device_node(dev_info->dev_id, tsid);
    }

#ifdef AOS_LLVM_BUILD
    config->aos_flag = AOS_CORE_FLAG;
#else
    config->aos_flag = AOS_KERNEL_FLAG;
#endif

    if (tsid == 0) {
        ret = devdrv_load_stl_firmware(TS_MDC_AIC_STL_BASE, TS_MDC_AIC_STL_SIZE, "/var/aic_stl.bin");
    } else {
        ret = devdrv_load_stl_firmware(TS_MDC_AIV_STL_BASE, TS_MDC_AIV_STL_SIZE, "/var/aiv_stl.bin");
    }
    if (ret != 0) {
        devdrv_drv_err("load stl firmware file fail. (tsid=%u)\n", tsid);
        return ret;
    }
    devdrv_drv_info("Load stl firmware success, (tsid=%u).\n", tsid);

    return 0;
#endif
}

STATIC int devdrv_load_stl(const struct devdrv_info *dev_info, struct aicpu_system_config *config, u32 tsid)
{
#ifndef TSDRV_UT
    int ret = devdrv_load_dcache_firmware(dev_info, config);
    if (ret != 0) {
        devdrv_drv_err("load dcache firmware failed. ret=%d.\n", ret);
        return ret;
    }

    ret = devdrv_load_aic_aiv_stl(dev_info, config, tsid);
    if (ret != 0) {
        devdrv_drv_err("load stl firmware failed. (ret=%d, tsid=%u).\n", ret, tsid);
        return ret;
    }
#endif
    return 0;
}
#endif

STATIC void __iomem *tsdrv_aicpu_system_addr_remap(u32 devid, u32 tsid, u32 chipid, u32 dieid)
{
    void __iomem *vaddr = NULL;
#ifdef CFG_SOC_PLATFORM_MDC_V51
    vaddr = ioremap(SYSTEM_CONFIG_BASE + ((phys_addr_t)tsid * (phys_addr_t)SYSTEM_CONFIG_SIZE) +
                                          (CHIP_BASEADDR_PA_OFFSET * devid),
                                          SYSTEM_CONFIG_SIZE);
#else
    vaddr = ioremap_cache(SYSTEM_CONFIG_BASE + ((phys_addr_t)tsid * (phys_addr_t)SYSTEM_CONFIG_SIZE) +
                                          (CHIP_BASEADDR_PA_OFFSET * devid),
                                          SYSTEM_CONFIG_SIZE);
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
    config->aicore_bitmap = dev_info->aicore_bitmap;
    config->aicore_freq = dev_info->aicore_freq;

#if ((!defined CFG_SOC_PLATFORM_MDC_V51) && (!defined CFG_SOC_PLATFORM_HELPER) && (!defined CFG_SOC_PLATFORM_MINIV3))
    config->connect_protocol = devdrv_get_connect_protocol(dev_info->dev_id);
    devdrv_drv_info("(connect_protocol=%u)\n", (u32)config->connect_protocol);
#endif

    config->vector_core_bitmap = dev_info->vector_core_bitmap;
    config->vector_core_freq = dev_info->vector_core_freq;
    config->board_id = (int)dev_info->pdata->platform_info.board_id;
    devdrv_drv_info("(devid=%u; board_id=%u)\n", dev_info->dev_id, (u32)config->board_id);

    config->aicpu_id_base = dev_info->dts_cfg.fw_cpu_id_base;
    config->aicpu_num = dev_info->dts_cfg.fw_cpu_num;
    return;
}

#ifdef CFG_SOC_PLATFORM_MINIV3
STATIC int get_tscpu_irq(u32 dev_id, u32 *start_irq, u32 *irq_num)
{
    return devdrv_get_tscpu_irq_info(dev_id, start_irq, irq_num);
}

STATIC void set_ts_int_start_id(u32 dev_id, struct aicpu_system_config *config)
{
    int ret;
    u32 irq_num = 0;
    u32 start_irq = 0;

    ret = get_tscpu_irq(dev_id, &start_irq, &irq_num);
    if (ret != 0) {
        devdrv_drv_err("dev_id %u get irq failed\n", dev_id);
        return;
    }

    devdrv_drv_info("dev_id %u start_irq %u irq_num %u\n", dev_id, start_irq, irq_num);

    if (irq_num > 0) {
        config->ts_int_start_id = start_irq;
    }
}

#ifndef TSDRV_UT
STATIC irqreturn_t devdrv_ts_ipc_done_interrupt(int irq, void *data)
{
    return IRQ_HANDLED;
}
#endif

#define TS_CPU_ID       4
#define TS_IPC_MBX_NUM  4
STATIC void config_ts_ipcint_route(struct devdrv_info *dev_info)
{
    u32 int_index;
    int ret;
    u32 i;

    if (dev_info->dts_cfg.ipc_mbx_int_start_id == 0) {
        devdrv_drv_err("dev[%u] mbx int start id is invalid\n", dev_info->dev_id);
        return;
    }

#ifndef TSDRV_UT
    int_index = dev_info->dts_cfg.ipc_mbx_int_start_id;
    devdrv_drv_info("ipc mbx int start id (%u).\n", int_index);
    for (i = int_index; i < int_index + TS_IPC_MBX_NUM; i++) {
        ret = request_irq(i, devdrv_ts_ipc_done_interrupt, 0, "ts_ipc_done_interrupt", NULL);
        devdrv_drv_info("request_irq. irq(%u)\n", i);
        if (ret != 0) {
            devdrv_drv_err("request devdrv_ts_ipc_interrupt error!\n");
            return;
        }
        (void)devdrv_set_irq_affinity(i, get_cpu_mask(TS_CPU_ID));
    }
#endif

    return;
}
#endif

STATIC void tsdrv_config_addr_info(struct devdrv_info *dev_info, struct aicpu_system_config *config, u32 tsid)
{
#ifndef AOS_LLVM_BUILD
    u32 i;
#endif
#ifdef CFG_SOC_PLATFORM_MINIV3
    set_ts_int_start_id(dev_info->dev_id, config);
#else
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
#ifndef AOS_LLVM_BUILD
    for (i = 0; i < CONFIG_CPU_MAX_NUM; i++) {
        config->alg_memory_base[i] = virt_to_phys(dev_info->fw_info.alg_mem[i]);
    }
#endif
}

STATIC void devdrv_get_bbox_config_info(struct aicpu_system_config *config)
{
    config->ts_blackbox_base = 0;
    config->ts_blackbox_size = 0;
    config->ts_start_log_base = 0;
    config->ts_start_log_size = 0;
    config->enable_bbox = 0;
}

STATIC int aicpu_set_system_config(struct devdrv_info *dev_info, u32 tsid)
{
    struct aicpu_system_config *config = NULL;
    void __iomem * vaddr = NULL;
#ifdef CFG_SOC_PLATFORM_MINIV3
    u32 hwirq = 0;
#endif
    int ret;

    vaddr = tsdrv_aicpu_system_addr_remap(dev_info->dev_id, tsid, dev_info->chip_id, dev_info->die_id);
    if (vaddr == NULL) {
        return -EINVAL;
    }

    config = (struct aicpu_system_config *)((uintptr_t)vaddr);
    tsdrv_config_boot_info(dev_info, config);
    tsdrv_config_cpu_info(dev_info, config);

#ifdef CFG_SOC_PLATFORM_MDC_V51
    /* load MDC STL */
    if (tsdrv_get_chip_type() == CHIP_TYPE_MDC_BS9SX1A) {
        ret = devdrv_load_stl(dev_info, config, tsid);
        if (ret != 0) {
#ifndef TSDRV_UT
            devdrv_drv_err("Load stl failed. (devid=%u)\n", dev_info->dev_id);
            goto exit;
#endif
        }
    }
#endif

    tsdrv_config_addr_info(dev_info, config, tsid);
    devdrv_get_bbox_config_info(config);

#ifdef CFG_SOC_PLATFORM_MINIV3
    config_ts_ipcint_route(dev_info);
    ret = tsdrv_get_hwirq_from_irq(config->ipc_mbx_int_start_id, &hwirq);
    if (ret != 0) {
        devdrv_drv_err("failed to get hwirq from irq.\n, irq=%u", config->ipc_mbx_int_start_id);
    }
    devdrv_drv_info("hwirq = %u.\n", hwirq);
    config->ipc_mbx_int_start_id = hwirq;
#endif

    devdrv_config_ts_dma_chan_info(dev_info, (u64)(uintptr_t)vaddr);
    devdrv_config_ts_valid_mem_range(dev_info, (u64)(uintptr_t)vaddr);
    ret = devdrv_construct_aicpu_dt((u64)(uintptr_t)vaddr, dev_info->dev_id, tsid);
    if (ret != 0) {
        devdrv_drv_err("Construct aicpu dt failed. (devid=%u)\n", dev_info->dev_id);
        goto exit;
    }

    tsdrv_flush_cache(dev_info, (u64)((uintptr_t)config), (u32)SYSTEM_CONFIG_SIZE);
    iounmap(config);
    aicpu_set_current_time(dev_info);
    return 0;
exit:
    iounmap(config);
    return ret;
}

#ifdef CFG_SOC_PLATFORM_MINIV3
STATIC void soc_dereset_core_tscpu(struct device *dev, u64 reg_base, u32 firmware_hardware_version)
{
#ifndef TSDRV_UT
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
    rst_reg = sysctrl + TSCPU_SOFT_RESET_ENABLE_REG_OFFSET; /* 0xc80 */
    set_tscpu_reg(rst_reg, 0x0FFD10 & 0xFFFFF);             /* [19:0] */

    rst_reg = sysctrl + SOC_TS_SYSCTRL_PERIPHCTRL38_OFFSET; /* 0x134 */
    set_tscpu_reg(rst_reg, 0x111);                          /* [31:0] */

    rst_reg = sysctrl + SOC_TS_SYSCTRL_PERIPHCTRL58_OFFSET; /* 0x184 */
    set_tscpu_reg(rst_reg, 0x80080008);                     /* [31:0] */

    rst_reg = sysctrl + SOC_TS_SYSCTRL_PERIPHCTRL55_OFFSET; /* 0x178 */
    set_tscpu_reg(rst_reg, 0x80080308);                     /* [31:0] */
    wmb();

    /* Close clock */
    rst_reg = sysctrl + TSCPU_CLOCK_DISABLE_REG_OFFSET;     /* 0xC0 */
    set_tscpu_reg(rst_reg, 0x1F1);                          /* [31:0] */

    rst_reg = sysctrl + TSCPU_SOFT_RESET_ENABLE_REG_OFFSET; /* 0xC8 */
    set_tscpu_reg(rst_reg, 0x7FFF);                         /* [31:0] */

    rst_reg = sysctrl + SOC_TS_SYSCTRL_PERIPHCTRL38_OFFSET; /* 0x134 */
    set_tscpu_reg(rst_reg, 0x7F0);                          /* [31:0] */

    /* Reset evacuation */
    rst_reg = sysctrl + LOW_POWER_DEBUG;                    /* 0xCC */
    set_tscpu_reg(rst_reg, 0x7FFF);                         /* [31:0] */

    rst_reg = sysctrl + SOC_TS_SYSCTRL_PERIPHCTRL39_OFFSET; /* 0x138 */
    set_tscpu_reg(rst_reg, 0x7F0);                          /* [31:0] */

    /* Enable clock */
    rst_reg = sysctrl + TSCPU_CLOCK_GATE_ENABLE_REG_OFFSET; /* 0xBC */
    set_tscpu_reg(rst_reg, 0x1F1);                          /* [31:0] */

    wmb();
    devm_iounmap(dev, sysctrl);
#endif
}
#endif

int aicpu_dereset(u32 tsid, struct devdrv_info *dev_info)
{
    int ret;
    u64 boot_addr;

    ret = aicpu_set_system_config(dev_info, tsid);
    if (ret != 0) {
        devdrv_drv_err("config set failed.\n");
        return ret;
    }

    boot_addr = (u64)dev_info->fw_info.ts_boot_addr;

#ifdef CFG_SOC_PLATFORM_MINIV3
    tsdrv_soc_startup(dev_info->dev_id, boot_addr, dev_info->pdata->ts_pdata[0].ts_sysctl_paddr, dev_info->env_type);
    soc_dereset_core_tscpu(dev_info->dev, dev_info->pdata->ts_pdata[0].ts_sysctl_paddr,
        dev_info->firmware_hardware_version);
    isb();
#else
    tsdrv_soc_startup(tsid, dev_info->dev_id, boot_addr);
#endif
#ifdef CFG_SOC_MDC_V51_LITE
    TSDRV_PRINT_INFO("Tsdrv finished starting SOC. Wait for starting TS. (tsid=%u)\n", tsid);
#else
    TSDRV_PRINT_DEBUG("Tsdrv finished starting SOC. Wait for starting TS. (tsid=%u)\n", tsid);
#endif
    return ret;
}

void *devdrv_tscpu_alloc_memory(struct devdrv_info *dev_info, u32 tsid,
    struct device *dev, dma_addr_t *dma_addr)
{
    u64 phys_addr;
    u32 devid = dev_info->dev_id;

    DRV_CHECK_EXP_ACT(devid >= MAX_CHIP_NUM, return NULL, "Invalid devid(%u)\n", devid);
    DRV_CHECK_EXP_ACT(tsid >= DEVDRV_MAX_TS_NUM, return NULL, "Invalid tsid(%u), devid(%u)\n", tsid, devid);
    DRV_CHECK_PTR(dma_addr, return NULL, "dma_addr is NULL\n");

    if (tsid == 0) {
        phys_addr = TSAICFW_LOAD_DDR_ADDR + (devid * CHIP_BASEADDR_PA_OFFSET);
    } else {
        phys_addr = TSAIVFW_LOAD_DDR_ADDR + (devid * CHIP_BASEADDR_PA_OFFSET);
    }

    *dma_addr = phys_addr;
    return devm_ioremap(dev, phys_addr, DEVDRV_TS_MEMORY_SIZE);
}

static int dfm_report_ts_load_result(struct devdrv_info *dev_info, u32 tsid, u32 code)
{
    int ret;

    if (dev_info->fw_verify != 1) {
        (void)tsdrv_bbox_write(dev_info->dev_id, "ts firmware verify failed");
        ret = -TS_FW_VERIFY_FAILED;
    } else {
        (void)tsdrv_bbox_write(dev_info->dev_id, "wait for ts going ready time out.");
        ret = -TS_STARTUP_FAILED;
    }

    devdrv_drv_err("ts start error. send error report to black box, ret=%d", ret);
    tsdrv_bbox_system_err(dev_info->dev_id, code, 0);
    return ret;
}

STATIC int devdrv_wait_ts_load_done(struct devdrv_info *dev_info, u32 tsid,
                            u32 __iomem *sc_testreg, struct devdrv_platform_data *pdata)
{
    u32 code = MNTN_TS0_START_ERROR_CODE;
    u32 reg_val;
    u32 times;
    u32 retry;
    int ret = 0;
    /* poll for TS load done */
    times = 0;
    retry = MINIV2_RETRY_TIMEOUT;
#ifdef TSDRV_UT
    tsdrv_tsfw_kickstart(dev_info->dev_id, tsid);
#endif

    TSDRV_PRINT_DEBUG("begin to waiting TS startup...\n");
    while (1) {
#ifndef AOS_LLVM_BUILD
        reg_val = readl_relaxed(sc_testreg);
#else
        reg_val = readl(sc_testreg);
#endif
#ifdef CFG_SOC_MDC_V51_LITE
        TSDRV_PRINT_INFO("waiting for ts startup. has retry %u(ms), reg_val=%u.\n",
            times * MDC_RETRY_INTERVAL, reg_val);
#endif
        if ((reg_val >= DEVDRV_SC_TESTREG_TS_READY) && (reg_val != 0xFFFFFFFFUL)) {
            tsdrv_set_ts_status(dev_info->dev_id, tsid, TS_WORK);
            TSDRV_PRINT_INFO("TS firmware load success. (devid=%u; tsid=%u; reg_value=0x%x; times=%u)\n",
                dev_info->dev_id, tsid, reg_val, times);
            devdrv_get_active_core(dev_info);
            code = MNTN_TS_START_SUCC_CODE;
            ret = 0;
            break;
        }
#ifndef TSDRV_UT
        msleep(MDC_RETRY_INTERVAL);
        times++;
        /* the maximum time to load TS is 200 */
        if (times > retry) {
            tsdrv_set_ts_status(dev_info->dev_id, tsid, TS_DOWN);
            TSDRV_PRINT_ERR("TS firmware load timeout. (devid=%u; tsid=%u; reg_val=0x%x).\n",
                dev_info->dev_id, tsid, reg_val);
#if defined(CFG_SOC_PLATFORM_MINIV2)
            code = ((tsid == 0) ? MNTN_TS0_START_ERROR_CODE : MNTN_TS1_START_ERROR_CODE);
#else
            code = MNTN_TS0_START_ERROR_CODE;
#endif
            pdata->ts_pdata[tsid].ts_start_fail = 1;
            tsdrv_tsfw_boot_reg_print(sc_testreg, DEVDRV_SC_TESTREG_MAX_NUM);
            ret = dfm_report_ts_load_result(dev_info, tsid, code);
            break;
        }
        if ((times + 1) % MDC_RETRY_TIMES_PER_SECOND == 0) {
            TSDRV_PRINT_DEBUG("waiting for ts startup. has retry %u(ms).\n", times * MDC_RETRY_INTERVAL);
        }
#endif /* TSDRV_UT */
    }
    return ret;
}

int devdrv_fw_load_init(struct devdrv_info *dev_info, u32 tsid)
{
#ifndef CFG_SOC_PLATFORM_MINIV3
    const char risc_v_bin_path[MAX_RISC_FW_NUM][RISCV_BIN_PATH_LENGTH] = {
        "/var/riscv-firmware.elf",
        "/var/riscv-firmware_vec.elf"
    };
    const char risc_v_bin_path_bs9sx1a[MAX_RISC_FW_NUM][RISCV_BIN_PATH_LENGTH] = {
        "/var/riscv-firmware_615.elf",
        "/var/riscv-firmware_615_vec.elf"
    };
#endif
    struct devdrv_platform_data *pdata = NULL;
    struct semaphore *core_info_sema = NULL;
    u32 __iomem *sc_testreg = NULL;
    int ret;

    if (tsid >= MAX_RISC_FW_NUM) {
        TSDRV_PRINT_ERR("Invalid tsid, (tsid=%u)\n", tsid);
        return -ENODEV;
    }

    pdata = dev_info->pdata;
    dev_info->inuse.ai_core_num = dev_info->ai_core_num;
    dev_info->inuse.ai_core_error_bitmap = 0;
    dev_info->inuse.ai_cpu_num = dev_info->ai_cpu_core_num;
    dev_info->inuse.ai_cpu_error_bitmap = 0;
#ifndef AOS_LLVM_BUILD
    devdrv_mn_mutex_lock(dev_info->dev_id);
#endif
    core_info_sema = devdrv_get_core_info_sema(dev_info->dev_id);
    if (core_info_sema == NULL) {
#ifndef AOS_LLVM_BUILD
        devdrv_mn_mutex_unlock(dev_info->dev_id);
#endif
        devdrv_drv_err("Get sema failed. (devid=%u)\n", dev_info->dev_id);
        return -EINVAL;
    }
    sema_init(core_info_sema, 0);

    sc_testreg = (u32 *)(uintptr_t)((uintptr_t)pdata->ts_pdata[tsid].ts_sysctl_vaddr + DEVDRV_SC_TESTREG_OFFSET);
#ifndef AOS_LLVM_BUILD
    writel_relaxed(DEVDRV_SC_TESTREG_INIT, &sc_testreg[DEVDRV_SC_TESTREG_TS]);
#else
    writel(DEVDRV_SC_TESTREG_INIT, &sc_testreg[DEVDRV_SC_TESTREG_TS]);
#endif
    wmb();

#ifndef CFG_SOC_PLATFORM_MINIV3
    if (tsdrv_get_chip_type() == CHIP_TYPE_MDC_BS9SX1A) {
        ret = devdrv_load_riscv_fw(tsid, risc_v_bin_path_bs9sx1a[tsid]);
    } else {
        ret = devdrv_load_riscv_fw(tsid, risc_v_bin_path[tsid]);
    }
    if (ret != 0) {
        TSDRV_PRINT_WARN("load riscv binary warn.\n");
    }
    TSDRV_PRINT_DEBUG("Loading riscv binary was complete. (tsid=%u)\n", tsid);
#endif

    ret = devdrv_load_cpu_fw(dev_info, tsid);
    if (ret != 0) {
#ifndef AOS_LLVM_BUILD
        devdrv_mn_mutex_unlock(dev_info->dev_id);
#endif
        TSDRV_PRINT_ERR("Load TS firmware failed. (devid=%u; tsid=%u)\n",
            dev_info->dev_id, tsid);
        return ret;
    }
    ret = devdrv_wait_ts_load_done(dev_info, tsid, sc_testreg, pdata);
#ifndef AOS_LLVM_BUILD
    devdrv_mn_mutex_unlock(dev_info->dev_id);
#endif
    return ret;
}
