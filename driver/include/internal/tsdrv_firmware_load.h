/**
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

#ifndef TSDRV_FIRMWARE_LOAD_H
#define TSDRV_FIRMWARE_LOAD_H

#include <linux/types.h>
#include <linux/device.h>

#include "devdrv_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __aarch64__
#define READ_SYSTEM_COUNTER_FREQ(freq) asm volatile("mrs %0, CNTFRQ_EL0" : "=r"(freq) :)
#else
#define READ_SYSTEM_COUNTER_FREQ(freq)
#endif

#if defined(CFG_SOC_PLATFORM_CLOUD)
#ifndef CFG_SOC_PLATFORM_CLOUD_V2
#define TS_STATIC_ADDR_BASE 0x62000000
#else
#define TS_STATIC_ADDR_BASE 0x19E00000
#define DIE0_HBM_ADDR_OFFSET 0x1000000000
#endif
#elif defined(CFG_SOC_PLATFORM_MINIV3)
#ifdef CFG_MEMORY_OPTIMIZE
#define TSAICFW_LOAD_DDR_ADDR 0x7580000
#else
#define TSAICFW_LOAD_DDR_ADDR 0xA000000
#endif
#define TSAIVFW_LOAD_DDR_ADDR 0x1B400000
#elif defined(CFG_SOC_PLATFORM_MINIV2)
#define TSAICFW_LOAD_DDR_ADDR 0xFB00000
#define TSAIVFW_LOAD_DDR_ADDR 0x1B400000
#endif

#if defined(CFG_SOC_PLATFORM_MINIV2)
#define MNTN_TS0_START_ERROR_CODE 0xA8060FFFU
#define MNTN_TS1_START_ERROR_CODE 0xA8061FFFU
#else
#define MNTN_TS0_START_ERROR_CODE 0xA807FFFFU
#endif

/* ********** HARDWARE VERSION ************** */
#define FIRMWARE_HW_ASIC 0x3UL
#define FIRMWARE_HW_ESL 0x2UL
#define FIRMWARE_HW_EMU 0x1UL
#define FIRMWARE_HW_FPGA 0x0UL

#define MNTN_TS_START_SUCC_CODE 0xA4060001U

void tsdrv_get_chip_version(struct devdrv_info *dev_info, u32 *chip_id, u32 *die_id);
u64 tsdrv_get_addr_chip_die_offset(u32 devid, u32 chipid, u32 dieid);
int tsdrv_firmware_load(struct devdrv_info *dev_info);

void tsdrv_firmware_load_init(void);
void tsdrv_firmware_load_exit(void);

/**
 * devdrv_load_cpu_fw  :load firmware of aicpu or task scheduler
 * @param  fw_path firmware path in filesystem
 * @param  type    0:ai cpu;1:ts cpu
 * @return         0:succ others:fail
 */
int devdrv_load_cpu_fw(struct devdrv_info *dev_info, u32 type);
int devdrv_load_riscv_fw(u32 tsid, const char *firmware_path);
int devdrv_load_ffts_fw(struct devdrv_info *dev_info, u32 tsid);
void tsdrv_tsfw_boot_reg_print(u32 __iomem *testreg, u32 reg_num);
void aicpu_set_current_time(struct devdrv_info *dev_info);

void tsdrv_flush_cache(struct devdrv_info *dev_info, u64 base, u32 len);
void devdrv_get_active_core(struct devdrv_info *dev_info);
void devdrv_config_ts_valid_mem_range(struct devdrv_info *dev_info, u64 vaddr);
u32 devdrv_get_hardware_info(struct devdrv_info *dev_info);

struct semaphore *devdrv_get_core_info_sema(u32 dev_id);

void devdrv_destroy_ts_irq(struct devdrv_info *dev_info);

#ifdef TSDRV_UT
typedef void (* tsfw_kickstart_handler)(u32 devid, u32 tsid);
typedef void (* tsfw_stop_handler)(u32 devid, u32 tsid);
void tsdrv_tsfw_kickstart(u32 devid, u32 tsid);
void tsdrv_tsfw_stop(u32 devid, u32 tsid);

#endif /* TSDRV_UT */

#ifdef CFG_SOC_PLATFORM_MINI
extern int devdrv_flash_get_aicpu_config(const char *name, unsigned int *aicpu_num);
#endif

#ifdef __cplusplus
};
#endif

#endif
