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
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/time.h>
#include <linux/timex.h>
#include <linux/rtc.h>
#include <linux/jiffies.h>
#include <linux/pm.h>
#include <linux/errno.h>
#include <linux/securec.h>

#include "devdrv_common.h"
#include "devdrv_manager_comm.h"
#include "aicpu.h"
#include "aicpu_dt.h"
#include "tsdrv_aicpu_config.h"
// ///////////////////nmanager////////////////////
#include "devdrv_interface.h"

#define POWER_REG_PA_BASE 0x1000E8000ULL

#define SRAM_PA_BASE 0x29500000ULL
#define SRAM_PA_SIZE 0x800ULL

#define GICD_REG_PA_SIZE 0x100000ULL
#define GICR_REG_PA_SIZE 0x40000ULL

#define DT_GIC_MULTICHIP_OFF 0
#define DEV_NUM 5
#define TOTAL_SIZE (5 * 1024)
#ifdef CFG_SOC_MDC_V51_LITE
#define AIC_TS_STARTUP_BASE_ADDR 0xb9080000
#else
#define AIC_TS_STARTUP_BASE_ADDR 0xba080000
#endif
#define VEC_TS_STARTUP_BASE_ADDR 0xb9080000
#define WAIT_TIMES 30
#define SLEEP_TIME_MS 100

#define SOC_PERI_BASE                 0x0
#define SOC_PERI_GIC0_REG             (SOC_PERI_BASE + 0x109000000)
#define SOC_PERI_IPC_REG_REG          (SOC_PERI_BASE + 0x102090000)

STATIC u32 ts_vaild_nid_type[] = { DEVDRV_TS_NODE_DDR_MEM };

int devdrv_construct_aicpu_dt(u64 vaddr, u32 dev_id, u32 tsid)
{
    struct aicpu_system_config *config = NULL;

    /* Construct aicpu_system_config */
    config = (struct aicpu_system_config *)((uintptr_t)vaddr);
    if (config == NULL)
        return -1;
    config->gic_multichip_off = DT_GIC_MULTICHIP_OFF;

    config->sram_pa_base = SRAM_PA_BASE + (tsid * SRAM_PA_SIZE) + (dev_id * CHIP_BASEADDR_PA_OFFSET);
    config->sram_pa_size = SRAM_PA_SIZE;
    config->gicd_pa_base = SOC_PERI_GIC0_REG;
    config->gicd_pa_size = GICD_REG_PA_SIZE;
    config->gicr_pa_size = GICR_REG_PA_SIZE;
    config->tzpc_pa_base = DT_INVALID_PA_BASE;
    config->total_size = TOTAL_SIZE;
    if (tsid == 0) {
        config->ts_aicpu_status_base = AIC_TS_STARTUP_BASE_ADDR + (dev_id * CHIP_BASEADDR_PA_OFFSET);
    } else {
        config->ts_aicpu_status_base = VEC_TS_STARTUP_BASE_ADDR + (dev_id * CHIP_BASEADDR_PA_OFFSET);
    }

    return 0;
}

void devdrv_config_ts_dma_chan_info(struct devdrv_info *dev_info, u64 vaddr)
{
    struct aicpu_system_config *config = NULL;

    config = (struct aicpu_system_config *)(uintptr_t)(vaddr);
    if (config == NULL) {
        return;
    }

    config->chan_id_base =0;
    config->chan_num = 0;
    config->nvme_pf_num = 0;
    config->product_num = 0;
}

u32 *devdrv_config_ts_valid_nid_type(u32 *nid_num)
{
#ifndef TSDRV_UT
    u32 num = sizeof(ts_vaild_nid_type) / sizeof(u32);

    if (num > TS_MEM_VALID_RANGE_MAX_NUM) {
#ifndef TSDRV_UT
        devdrv_drv_err("ts_vaild_nid_type num %u is invalid, max nid_num: %u.\n", num, TS_MEM_VALID_RANGE_MAX_NUM);
        return NULL;
#endif
    }

    *nid_num = num;
    return ts_vaild_nid_type;
#endif
}

#ifdef TSDRV_UT
void devdrv_destroy_ts_irq(struct devdrv_info *dev_info)
{
}
#endif
