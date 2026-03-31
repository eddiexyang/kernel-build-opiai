/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2023. All rights reserved.
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
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/uaccess.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/time.h>
#include <linux/timex.h>
#include <linux/vmalloc.h>
#ifdef AOS_LLVM_BUILD
#include <linux/timecounter.h>
#endif

#include "tsdrv_firmware_load.h"
#include "tsdrv_adapt_firmware_load.h"
#include "tsdrv_firmware_reader.h"
#include "tsdrv_aicpu_config.h"
#include "devdrv_cache_flush.h"
#include "devdrv_platform.h"
#include "tsdrv_file_check.h"
#include "tsdrv_common.h"
#include "tsdrv_log.h"
#include "devdrv_manager.h"
#include "devdrv_manager_common.h"
#include "devdrv_pm.h"
#include "devdrv_ipc.h"
#include "aicpu_dt.h"
#include "drv_ipc.h"
#include "tsdrv_ipc.h"
#ifndef AOS_LLVM_BUILD
#include "config.h"
#endif
#include "tsdrv_file_load.h"

#include "tsdrv_osal_bbox.h"
#include "tsdrv_log.h"
#include "tsmng_interface.h"

#ifdef CFG_SOC_PLATFORM_MDC_V51
#ifndef AOS_LLVM_BUILD
#include "drv_notify.h"
#endif
# include "devmng_dts.h"
#endif

STATIC struct semaphore devdrv_core_info_sema[MAX_CHIP_NUM];

#define STARS_SYSCTL_SC_SYSSTAT5_OFFSET 0x1C
#define DIEID_REG_OFFSET 8

#ifdef CFG_SOC_PLATFORM_MDC_V51
#define TSFW_PATH_MAX 256
#endif

void tsdrv_get_chip_version(struct devdrv_info *dev_info, u32 *chip_id, u32 *die_id)
{
#ifdef CFG_SOC_PLATFORM_CLOUD_V2
    void *info_reg = NULL;
    u32 reg_value;

    info_reg = (u32 *)((unsigned long)(uintptr_t)dev_info->pdata->ts_pdata[0].ts_sysctl_vaddr
        + STARS_SYSCTL_SC_SYSSTAT5_OFFSET);
    reg_value = readl_relaxed(info_reg);

    *chip_id = (reg_value >> 4) & 0xF; /* chip offset 4 */
    *die_id = (reg_value >> DIEID_REG_OFFSET) & 0x3;
#else
    *chip_id = dev_info->dev_id;
    *die_id = 0;
#endif
    devdrv_drv_info("devid(%u), chipid(%u), dieid(%u)\n", dev_info->dev_id, *chip_id, *die_id);
}

u64 tsdrv_get_addr_chip_die_offset(u32 devid, u32 chipid, u32 dieid)
{
    u64 offset;
#ifndef TSDRV_UT

    offset = chipid * CHIP_BASEADDR_PA_OFFSET + dieid * DIE_BASEADDR_PA_OFFSET;
#ifdef CFG_SOC_PLATFORM_CLOUD_V2
    if (devdrv_get_connect_protocol(devid) == CONNECT_PROTOCOL_HCCS) {
        offset = chipid * HCCS_CHIP_BASEADDR_PA_OFFSET + dieid * DIE_BASEADDR_PA_OFFSET + HCCS_BASEADDR_PA_OFFSET;
    }
#endif

#endif
    return offset;
}

void tsdrv_flush_cache(struct devdrv_info *dev_info, u64 base, u32 len)
{
    if ((dev_info->drv_ops == NULL) || (dev_info->drv_ops->flush_cache == NULL)) {
        return;
    }

    dev_info->drv_ops->flush_cache(base, len);
}

void aicpu_set_current_time(struct devdrv_info *dev_info)
{
    struct aicpu_system_config *config = NULL;
    struct timespec64 wall64;

#ifdef CFG_SOC_PLATFORM_CLOUD_V2
    u64 config_addr = (dev_info->die_id == 0) ? (SYSTEM_CONFIG_BASE - DIE0_HBM_ADDR_OFFSET) : (SYSTEM_CONFIG_BASE);
    if (dev_info->die_id == 0) {
        config = (struct aicpu_system_config *)ioremap(config_addr, SYSTEM_CONFIG_SIZE);
    } else {
        config = (struct aicpu_system_config *)ioremap(config_addr +
            tsdrv_get_addr_chip_die_offset(dev_info->dev_id, dev_info->chip_id, dev_info->die_id), SYSTEM_CONFIG_SIZE);
    }
#else
    config = (struct aicpu_system_config *)ioremap(SYSTEM_CONFIG_BASE, SYSTEM_CONFIG_SIZE);
#endif
    if (config == NULL) {
        devdrv_drv_err("ioremap_cache failed.\n");
        return;
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
    ktime_get_coarse_real_ts64(&wall64);
#else
    wall64 = current_kernel_time64();
#endif
    config->boot_time.sec = (u64)wall64.tv_sec;
    config->boot_time.nsec = (u64)wall64.tv_nsec;

    iounmap(config);
}

#ifndef TSDRV_UT
static int tsdrv_get_nids_by_type(u32 devid, u32 memtype, u32 sub_memtype, int nids[], int num)
{
    int nid_cnt = dbl_nid_get_nid_num(devid, memtype, sub_memtype);
    if ((nid_cnt > 0) && (nid_cnt <= num)) {
        (void)dbl_nid_get_nid(devid, memtype, sub_memtype, nids, nid_cnt);
    }
    return nid_cnt;
}
#endif

void devdrv_config_ts_valid_mem_range(struct devdrv_info *dev_info, u64 vaddr)
{
#ifndef TSDRV_UT
    struct aicpu_system_config *config = (struct aicpu_system_config *)(uintptr_t)(vaddr);
    u32 valid_nid_type[TS_MEM_VALID_RANGE_MAX_NUM][TS_MEMTYPE_MAX];
    u32 nid_num = 0, nid_type_num, i, j;
    int ret;

    config->valid_range_num = 0;
    if (dev_info->ts_mem_restrict_valid != TS_MEM_RESTRICT_VALID) {
        return;
    }

    ret = devdrv_config_ts_valid_nid_type(valid_nid_type, &nid_type_num);
    if (ret != 0) {
        devdrv_drv_err("The valid_nid_type is invalid. (ret=%d; nid_type_num=%u).\n", ret, nid_type_num);
        return;
    }

    for (i = 0; i < nid_type_num; i++) {
        int nids[DBL_NUMA_ID_MAX_NUM];
        int num = tsdrv_get_nids_by_type(dev_info->dev_id, valid_nid_type[i][0], valid_nid_type[i][1], nids,
            DBL_NUMA_ID_MAX_NUM);
        if ((num <= 0) || (num > DBL_NUMA_ID_MAX_NUM)) {
            devdrv_drv_err("Failed to get nids. (devid=%u)\n", dev_info->dev_id);
            return;
        }
        for (j = 0; j < (u32)num; j++) {
            config->addr_ranges[nid_num].start = __pfn_to_phys(node_start_pfn(nids[j]));
            config->addr_ranges[nid_num].end = __pfn_to_phys(node_end_pfn(nids[j]));

            devdrv_drv_info("Node info. (devid=%u; mtype=%u; sub_mtype=%u; nid=%d; start=0x%pK; end=0x%pK)\n",
                dev_info->dev_id, valid_nid_type[i][0], valid_nid_type[i][1], nids[j],
                (void *)config->addr_ranges[nid_num].start, (void *)config->addr_ranges[nid_num].end);
            nid_num++;
            if (nid_num >= TS_MEM_VALID_RANGE_MAX_NUM) {
                devdrv_drv_err("Too much nids. (nid_num=%u; max_nid_num=%u)\n", nid_num, TS_MEM_VALID_RANGE_MAX_NUM);
                return;
            }
        }
    }
    config->valid_range_num = nid_num;
#endif
}

/**
 * devdrv_get_hardware_info :get current hardware information
 * @return:  0 fpga 1 emu 2 ESL 3 ASIC
 */
u32 devdrv_get_hardware_info(struct devdrv_info *dev_info)
{
#ifndef AOS_LLVM_BUILD
    struct devdrv_manager_info *manager_info = devdrv_get_manager_info();
#endif
    struct devdrv_platform_data *pdata = NULL;
    void __iomem *version_reg = NULL;
    u64 hw_version_reg_base;
    u32 reg_val;
#ifndef AOS_LLVM_BUILD
    if (manager_info == NULL) {
        devdrv_drv_err("dev_manager_info is invalid.\n");
        return FIRMWARE_HW_ASIC;
    }
#endif
    pdata = (struct devdrv_platform_data *)dev_info->pdata;
    hw_version_reg_base = pdata->platform_info.devdrv_addr_base[DEVDRV_DTS_SYSCTL_INDEX] +
        DEVDRV_SYSCTL_VERSION_REG_OFFSET;
#ifndef AOS_LLVM_BUILD
    version_reg = devm_ioremap(manager_info->dev, hw_version_reg_base, 0x10);
#else
    version_reg = devm_ioremap(dev_info->dev, hw_version_reg_base, 0x10);
#endif
    if (version_reg == NULL) {
        devdrv_drv_err("devm_ioremap failed.\n");
        return FIRMWARE_HW_ASIC;
    }

#ifndef AOS_LLVM_BUILD
    reg_val = readl_relaxed(version_reg);
#else
    reg_val = readl(version_reg);
#endif
#ifndef AOS_LLVM_BUILD
    devm_iounmap(manager_info->dev, version_reg);
#else
    devm_iounmap(dev_info->dev, version_reg);
#endif
    version_reg = NULL;

    if (reg_val == 0)
        return FIRMWARE_HW_ASIC;
    reg_val = reg_val >> 16; /* reg_val shifts rignt 16 bits */
    reg_val &= (u32)DEVDRV_SYSCTL_VERSION_PLAT_MASK;
    return reg_val;
}

int devdrv_load_riscv_fw(u32 tsid, const char *firmware_path)
{
#define AIC_RISCV_RESERVED_ADDR 0x1B300000
#define AIV_RISCV_RESERVED_ADDR 0x26C00000
#define AIV_RISCV_RESERVED_SIZE 0x100000
#ifndef AOS_LLVM_BUILD
    struct file *fp = NULL;
    size_t fsize;
    loff_t pos;
#endif
    int ret = -1;
    void *riscv_vir_addr = NULL;
    u32 riscv_reserved_addr;
    if (tsid == 0) {
        riscv_reserved_addr = AIC_RISCV_RESERVED_ADDR;
    } else {
        riscv_reserved_addr = AIV_RISCV_RESERVED_ADDR;
    }

    riscv_vir_addr = (void __iomem *)ioremap(riscv_reserved_addr, AIV_RISCV_RESERVED_SIZE);
    if ((void *)riscv_vir_addr == NULL) {
#ifndef TSDRV_UT
        devdrv_drv_err("ioremap failed. \n");
        return -EINVAL;
#endif
    }
#ifndef AOS_LLVM_BUILD
    fsize = get_file_size(firmware_path);
    if (fsize > AIV_RISCV_RESERVED_SIZE) {
#ifndef TSDRV_UT
        devdrv_drv_err("File size is invalid. (fsize=%lu)\n", fsize);
        iounmap(riscv_vir_addr);
        riscv_vir_addr = NULL;
        return -EINVAL;
#endif
    }

    fp = filp_open(firmware_path, O_RDONLY, 0);
    if (IS_ERR(fp)) {
        TSDRV_PRINT_WARN("can't open risc_v file\n");
        iounmap(riscv_vir_addr);
        riscv_vir_addr = NULL;
        return ret;
    }
    pos = 0;
    if (!copy_firmware(fp, fsize, pos, riscv_vir_addr)) {
        devdrv_drv_err("ERROR: can't copy firmware file\n");
        iounmap(riscv_vir_addr);
        riscv_vir_addr = NULL;
        (void)filp_close(fp, NULL);
        return ret;
    }
    (void)filp_close(fp, NULL);
#else
    ret = tsdrv_get_firmware(firmware_path, riscv_vir_addr, AIV_RISCV_RESERVED_SIZE);
    if (ret) {
        iounmap(riscv_vir_addr);
        riscv_vir_addr = NULL;
        return ret;
    }
#endif
    isb();

    iounmap(riscv_vir_addr);
    riscv_vir_addr = NULL;
    return 0;
}

#ifdef CFG_SOC_PLATFORM_CLOUD_V2
int devdrv_ffts_dereset(struct devdrv_info *dev_info, u32 tsid, u64 ffts_reserved_addr)
{
#ifndef TSDRV_UT
    struct devdrv_platform_data *pdata = NULL;
    u32 *sc_testreg = NULL;
    u32 *reg_addr = NULL;
    int sleeptime = 10;
    u32 printtime = 100;
    u32 times = 0;
    int ret = -1;
    u32 reg_val;
    u32 reg_offset;
    u32 reset_core;
    u32 *dfx_reg_addr = NULL;
    u32 dfx_reg_value;

    pdata = (struct devdrv_platform_data *)dev_info->pdata;

    sc_testreg = (u32 *)((unsigned long)(uintptr_t)pdata->ts_pdata[tsid].ts_sysctl_vaddr + FFTS_SC_TESTREG10);
    writel(FFTS_SC_TESTREG_INIT, &sc_testreg[0]);

    dfx_reg_addr = (u32 *)((unsigned long)(uintptr_t)pdata->ts_pdata[tsid].ts_sysctl_vaddr + FFTS_DFX_TESTREG6);

    reg_offset = FFTS_PERIPHCTRL70;
    reset_core = 0x40;

    reg_addr = (u32 *)((unsigned long)(uintptr_t)pdata->ts_pdata[tsid].ts_sysctl_vaddr + reg_offset);
    writel((u32)ffts_reserved_addr, reg_addr);
    reg_addr = (u32 *)((unsigned long)(uintptr_t)pdata->ts_pdata[tsid].ts_sysctl_vaddr
               + reg_offset + sizeof(u32));
    writel((u32)((ffts_reserved_addr >> FFTS_ADDR_HIGH_BIT) | 0x200), reg_addr);

    reg_addr = (u32 *)((unsigned long)(uintptr_t)pdata->ts_pdata[tsid].ts_sysctl_vaddr + FFTS_PERIPHCTRL12);
    writel(reset_core, reg_addr);

    while (1) {
        reg_val = readl(sc_testreg);
        if (reg_val == FFTS_SC_TESTREG_RDY) {
            devdrv_drv_info("devid(%u) load ffts succ, reg_val %u times: %u.\n", dev_info->dev_id, reg_val, times);
            ret = 0;
            break;
        }
        msleep(sleeptime);
        times++;
        /* the max number of times to load ffts is 1000 */
        if (times > 1000) {
            dfx_reg_value = readl(dfx_reg_addr);
            devdrv_drv_err("device(%u) tsid(%u) load ffts timeout. reg_val=%u, dfx_val=%u\n",
                dev_info->dev_id, tsid, reg_val, dfx_reg_value);
            ret = -1;
            break;
        }
        if (times % printtime == 0) {
            devdrv_drv_info("devid %u load ffts retry time %u status %u\n", dev_info->dev_id, times, reg_val);
        }
    }
    return ret;
#endif
}

int devdrv_load_ffts_fw(struct devdrv_info *dev_info, u32 tsid)
{
    const char *ffts_bin_path = "/var/ffts_plus_fw.bin";
    void *ffts_src_addr = NULL;
    void *ffts_boot_vir_addr = NULL;
    u64 ffts_reserved_addr, ffts_start_addr;
    u64 offset;
    size_t fsize;
    u32 chipid, dieid;
    int ret;

    tsdrv_get_chip_version(dev_info, &chipid, &dieid);

    ret = tsdrv_ffts_read(ffts_bin_path, &ffts_src_addr, &fsize);
    if (ret != 0) {
        devdrv_drv_err("Failed to tsdrv_ffts_read, devid(%u), chipid(%u), dieid(%u).\n",
            dev_info->dev_id, chipid, dieid);
        return ret;
    }

#ifndef TSDRV_UT
    ret = tsdrv_ffts_check(dev_info->dev_id, ffts_src_addr, fsize);
    if (ret != 0) {
        devdrv_drv_err("ffts fw check filed. (dev_id=%u)\n", dev_info->dev_id);
        goto free_ffts_src_addr;
    }

    offset = tsdrv_get_addr_chip_die_offset(dev_info->dev_id, chipid, dieid);
    ffts_reserved_addr = (dieid == 0) ? DIE0_FFTS_RESERVED_ADDR : DIE1_FFTS_RESERVED_ADDR + offset;
    ffts_boot_vir_addr = (void __iomem *)ioremap(ffts_reserved_addr, FFTS_RESERVED_SIZE);
    if ((void *)ffts_boot_vir_addr == NULL) {
        devdrv_drv_err("ffts fw vaddr: ioremap failed. \n");
        ret = -EINVAL;
        goto free_ffts_src_addr;
    }

    ret = tsdrv_ffts_write(dev_info, ffts_src_addr, fsize, ffts_boot_vir_addr, FFTS_RESERVED_SIZE);
    if (ret != 0) {
        devdrv_drv_err("tsdrv_ffts_write failed. \n");
        goto unmap_boot_addr;
    }

    devdrv_drv_info("dev_id(%u) tsid(%u) fw len(%lu).\n", dev_info->dev_id, tsid, fsize);
    ffts_start_addr = (dieid == 0) ? DIE1_FFTS_RESERVED_ADDR : DIE1_FFTS_RESERVED_ADDR + offset;
    ret = devdrv_ffts_dereset(dev_info, tsid, ffts_start_addr);

unmap_boot_addr:
    iounmap(ffts_boot_vir_addr);
    ffts_boot_vir_addr = NULL;
free_ffts_src_addr:
    vfree(ffts_src_addr);
    ffts_src_addr = NULL;
    return ret;
#endif
}
#endif
void tsdrv_tsfw_boot_reg_print(u32 __iomem *testreg, u32 reg_num)
{
    u32 reg_val, i;

    for (i = 0; i < reg_num; i++) {
#ifndef AOS_LLVM_BUILD
        reg_val = readl_relaxed(&testreg[i]);
#else
        reg_val = readl(&testreg[i]);
#endif
        devdrv_drv_info("TS_SYSCTRL reg_ral[%u] = 0x%x.\n", i, reg_val);
    }
}

#ifdef TSDRV_UT
static tsfw_kickstart_handler g_tsfw_kickstart;
static tsfw_stop_handler g_tsfw_stop;

void ut_device_tsdrv_set_tsfw_kickstart(tsfw_kickstart_handler handler)
{
    g_tsfw_kickstart = handler;
}

void ut_device_tsdrv_tsfw_set_stop(tsfw_stop_handler handler)
{
    g_tsfw_stop = handler;
}

void tsdrv_tsfw_kickstart(u32 devid, u32 tsid)
{
    if (g_tsfw_kickstart != NULL) {
        g_tsfw_kickstart(devid, tsid);
    }
}

void tsdrv_tsfw_stop(u32 devid, u32 tsid)
{
    if (g_tsfw_stop != NULL) {
        g_tsfw_stop(devid, tsid);
    }
}
#endif /* TSDRV_UT */

/* *
devdrv_load_cpu_fw  :load firmware of aicpu or task scheduler
@param  fw_path firmware path in filesystem
@param  type    0:ai cpu;1:ts cpu
@return         0:succ others:fail
*/
int devdrv_load_cpu_fw(struct devdrv_info *dev_info, u32 tsid)
{
#ifdef CFG_SOC_PLATFORM_MDC_V51
    char tsfw_path[TSFW_PATH_MAX];
#endif
    int ret;

    if (dev_info == NULL) {
        devdrv_drv_err("ERROR :dev_info is NULL\n");
        return -EINVAL;
    }
    devdrv_drv_info("Start load TS firmware. (devid=%u)\n", dev_info->dev_id);

#ifdef CFG_SOC_PLATFORM_MDC_V51
    if (tsdrv_get_tsfw_path(dev_info->pdata->ts_spec, tsfw_path, sizeof(tsfw_path))  == NULL) {
        devdrv_drv_err("get ts firmware file name failed.\n");
        return -EINVAL;
    }
    devdrv_drv_info("load ts firmware %s\n", tsfw_path);
    ret = devdrv_tsfw_bin_check(dev_info, tsfw_path);
#else
    ret = devdrv_tsfw_bin_check(dev_info, DEVDRV_TS_BINARY_PATH);
#endif
    if (ret != 0) {
        devdrv_drv_err("Check firmware failed. (devid=%u; tsid=%u)\n", dev_info->dev_id, tsid);
        return ret;
    }

    /* get version esl/fgpa/asic/emu? */
    dev_info->firmware_hardware_version = devdrv_get_hardware_info(dev_info);

    /* get configuration dts/acpi */
    ret = aicpu_get_dts_config(dev_info, &dev_info->dts_cfg);
    if (ret != 0) {
        devdrv_drv_warn("Get aicpu dts config failed. (ret=%d)", ret);
    }

    ret = tsdrv_firmware_write(dev_info);
    if (ret != 0) {
        devdrv_drv_err("Write firmware failed. (devid=%u; tsid=%u)\n", dev_info->dev_id, tsid);
        return ret;
    }

    ret = aicpu_dereset(tsid, dev_info);
    if (ret != 0) {
        devdrv_drv_err("dereset failed, ret = %d, dev_id = %u.\n", ret, dev_info->dev_id);
        return ret;
    }

    return 0;
}
EXPORT_SYMBOL(devdrv_load_cpu_fw);

struct semaphore *devdrv_get_core_info_sema(u32 dev_id)
{
    if (dev_id >= MAX_CHIP_NUM) {
        devdrv_drv_err("***chip id invalid.%u ***\n", dev_id);
        return NULL;
    }
    return &devdrv_core_info_sema[dev_id];
}

void tsdrv_ts_inform_core_info(struct notifier_block *nb, unsigned long len, void *data)
{
    struct devdrv_ts_ai_ready_info *ready_info = NULL;
    struct ipcdrv_message *ipc_msg = NULL;
    struct devdrv_info *dev_info = NULL;
    struct tsdrv_ipc_chan *ipc_chan = NULL;

    ipc_chan = notifier_block_to_ipc_chan(nb);
#ifndef AOS_LLVM_BUILD
    dev_info = devdrv_manager_get_devdrv_info(ipc_chan->devid);
#else
    dev_info = dms_get_devinfo(ipc_chan->devid);
#endif
    if (dev_info == NULL) {
        devdrv_drv_err("Device does not initialized. (devid=%u)\n", ipc_chan->devid);
        return;
    }

    ipc_msg = (struct ipcdrv_message *)data;
    if (tsmng_chk_ipc_crc16(ipc_msg) != 0) {
        devdrv_drv_err("crc mismatched.\n");
        return;
    }
    ready_info = (struct devdrv_ts_ai_ready_info *)ipc_msg->ipcdrv_payload;

    TSDRV_PRINT_INFO("ai cpu num: %d,"
        "ai cpu broken bitmap: 0x%x,"
        "ai core num: %d,"
        "ai core broken bitmap: 0x%x, "
        "ai subsys broken bitmap: 0x%x.\n",
        ready_info->ai_cpu_ready_num, ready_info->ai_cpu_broken_map, ready_info->ai_core_ready_num,
        ready_info->ai_core_broken_map, ready_info->ai_subsys_ip_map);

    dev_info->inuse.ai_core_num = ready_info->ai_core_ready_num;
    dev_info->inuse.ai_core_error_bitmap = ready_info->ai_core_broken_map;
    dev_info->inuse.ai_cpu_num = ready_info->ai_cpu_ready_num;
    dev_info->inuse.ai_cpu_error_bitmap = ready_info->ai_cpu_broken_map;

    dev_info->ai_subsys_ip_broken_map = ready_info->ai_subsys_ip_map;
#ifndef AOS_LLVM_BUILD
    if (__sw_hweight32(dev_info->inuse.ai_cpu_error_bitmap) != 0) {
#else
    if (hweight32(dev_info->inuse.ai_cpu_error_bitmap) != 0) {
#endif
        devdrv_drv_warn("[dev_id = %u]:ai cpu exist error core, ai_cpu_num = %u, err_bitmap = 0x%x.\n",
            dev_info->dev_id, dev_info->ai_cpu_core_num, dev_info->inuse.ai_cpu_error_bitmap);
    }
#ifndef AOS_LLVM_BUILD
    if (__sw_hweight32(dev_info->inuse.ai_core_error_bitmap) != 0) {
#else
    if (hweight32(dev_info->inuse.ai_core_error_bitmap) != 0) {
#endif
        devdrv_drv_warn("[dev_id = %u]:ai core exist error core, ai_core_num = %u, err_bimap = 0x%x.\n",
            dev_info->dev_id, dev_info->ai_core_num, dev_info->inuse.ai_core_error_bitmap);
    }

    up(&devdrv_core_info_sema[dev_info->dev_id]);
}

void devdrv_get_active_core(struct devdrv_info *dev_info)
{
    u32 jiffy;
    int ret;

    if ((dev_info == NULL) || (dev_info->dev_id >= MAX_CHIP_NUM)) {
#ifndef TSDRV_UT
        devdrv_drv_err("invalid param, (dev_info == NULL)=%d, dev_id=%d.\n", (dev_info == NULL),
            (dev_info == NULL) ? (-1) : (dev_info->dev_id));
        return;
#endif
    }

    if ((dev_info->env_type == DEVDRV_PLAT_TYPE_EMU) || (dev_info->env_type == DEVDRV_PLAT_TYPE_ESL))
        jiffy = msecs_to_jiffies(100); /* 100ms */
    else
        jiffy = msecs_to_jiffies(5000); /* 5000ms */

    ret = down_timeout(&devdrv_core_info_sema[dev_info->dev_id], jiffy);
    if (ret != 0) {
        devdrv_drv_warn("down timeout, fail to get TS core info ipc in 5s.\n");
        return;
    }
}

STATIC int tsdrv_firmware_load_by_ts(struct devdrv_info *dev_info, u32 tsid)
{
    struct devdrv_platform_data *pdata = NULL;
    u32 devid;

    DRV_CHECK_PTR(dev_info, return -ENOMEM, "dev_info is NULL\n");
    DRV_CHECK_EXP_ACT(dev_info->dev_id >= TSDRV_MAX_DAVINCI_NUM, return -EINVAL, "Invalid devid(%u)\n",
        dev_info->dev_id);
    DRV_CHECK_EXP_ACT(tsid >= DEVDRV_MAX_TS_NUM, return -EINVAL, "Inlalid tsid(%u), devid(%u)\n", tsid,
        dev_info->dev_id);

    tsdrv_set_ts_status(dev_info->dev_id, tsid, TS_INITING);

    devid = dev_info->dev_id;
    pdata = dev_info->pdata;
    pdata->ts_pdata[tsid].ts_load_fail = 0;
    pdata->ts_pdata[tsid].ts_load_addr = devdrv_tscpu_alloc_memory(dev_info, tsid, dev_info->dev,
        &pdata->ts_pdata[tsid].ts_dma_handle);
    if (pdata->ts_pdata[tsid].ts_load_addr == NULL) {
#ifndef TSDRV_UT
        devdrv_drv_err("Failed to alloc memory for TS.\n");
        return -ENOMEM;
#endif
    }
    dev_info->fw_info.ts_boot_addr = (u64)pdata->ts_pdata[tsid].ts_dma_handle;
    dev_info->fw_info.ts_boot_addr_virt = (u64)((uintptr_t)pdata->ts_pdata[tsid].ts_load_addr);

    tsdrv_set_ts_status(dev_info->dev_id, tsid, TS_BOOTING);

    return devdrv_fw_load_init(dev_info, tsid);
}

int tsdrv_firmware_load(struct devdrv_info *dev_info)
{
    int ret = 0;
    u32 tsid;

    if (dev_info == NULL) {
        devdrv_drv_err("dev info invalid.\n");
        return -EINVAL;
    }

    for (tsid = 0; tsid < dev_info->pdata->ts_num; tsid++) {
        ret = tsdrv_firmware_load_by_ts(dev_info, tsid);
        if (ret == -TS_FW_VERIFY_FAILED) {
            (void)tsdrv_bbox_write(dev_info->dev_id,
                "Ts firmware load faied. detail: ts firmware verify failed.");
        } else if (ret == -TS_STARTUP_FAILED) {
            (void)tsdrv_bbox_write(dev_info->dev_id,
                "Ts firmware load faied. detail: wait for ts going ready time out.");
        } else if (ret != 0) {
            devdrv_drv_err("Ts firmware load failed. (devid=%u; tsid=%u; ret=%d)\n",
                dev_info->dev_id, tsid, ret);
        }
    }

    return ret;
}

void tsdrv_firmware_load_init(void)
{
    int ret;

    ret = tsdrv_ipc_handler_register(IPCDRV_TS_INFORM_CORE_INFO, tsdrv_ts_inform_core_info);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Register ts inform core info handler failed.\n");
    }
}

void tsdrv_firmware_load_exit(void)
{
    tsdrv_ipc_handler_unregister(IPCDRV_TS_INFORM_CORE_INFO);
}

