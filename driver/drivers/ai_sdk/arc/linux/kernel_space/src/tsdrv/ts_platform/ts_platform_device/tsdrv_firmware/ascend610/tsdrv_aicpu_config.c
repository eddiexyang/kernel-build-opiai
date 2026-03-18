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
#include "aicpu_dt.h"
// ///////////////////nmanager////////////////////
#include "devdrv_interface.h"
#include "tsdrv_aicpu_config.h"

#define POWER_REG_PA_BASE 0x1000E8000ULL

#define SRAM_PA_BASE 0x29500000ULL
#define SRAM_PA_SIZE 0x800ULL

#define GICD_REG_PA_SIZE 0x100000ULL
#define GICR_REG_PA_SIZE 0x40000ULL

#define DT_GIC_MULTICHIP_OFF 0
#define DEV_NUM 5
#define TOTAL_SIZE (5 * 1024)
#define AIC_TS_STARTUP_BASE_ADDR 0xba080000
#define VEC_TS_STARTUP_BASE_ADDR 0xb9080000
#define WAIT_TIMES 30
#define SLEEP_TIME_MS 100

#define SOC_PERI_BASE                 0x0
#define SOC_PERI_GIC0_REG             (SOC_PERI_BASE + 0x109000000)
#define SOC_PERI_IPC_REG_REG          (SOC_PERI_BASE + 0x102090000)

#define TS_MEMTYPE_MAX 2
STATIC u32 ts_vaild_nid_type[][TS_MEMTYPE_MAX] = {
    {DBL_MEMTYPE_DDR, DBL_SUB_MEMTYPE_TS}
};

STATIC struct ipc_cpu_id ipc_data = {
    .ipc_cpu_id_ts = 0,
    .ipc_cpu_id_acpu = -1,
    .ipc_cpu_id_aicpu0 = 2,  // aicpu0 id is 2
};

// //////////////// arch_timer //////////////////
STATIC struct aicpu_platform_device_info arch_timer_device_info = {
    .name = "arm,armv8-timer",
    .res = {
        {
            .start = 30, // start num is 30
            .end = 30,   // end num is 30
            .flags = IORESOURCE_IRQ,
        },
        {
            .start = 27,  // start num is 27
            .end = 27,    // end num is 27
            .flags = IORESOURCE_IRQ,
        },
    },
    .num_res = 2,  // num_res is 2
    .data = {
        0,
    },
    .data_size = 0,
};

// ///////////////// arch_pmu //////////////////
STATIC struct aicpu_platform_device_info arch_pmu_device_info = {
    .name = "arm,armv8-pmu",
    .res = {
        {
            .start = 23,  // start num is 23
            .end = 23,    // end num is 23
            .flags = IORESOURCE_IRQ,
        },
    },
    .num_res = 1,
    .data = {
        0,
    },
    .data_size = 0,
};

// ///////////////// gic_common //////////////////
STATIC struct aicpu_platform_device_info gic_device_info = {
    .name = "arm,gic-v3",
    .res = {
        {
            .start = SOC_PERI_GIC0_REG,
            .end = SOC_PERI_GIC0_REG + GICD_REG_PA_SIZE - 1,
            .flags = IORESOURCE_MEM,
        },
    },
    .num_res = 1,
    .data = {
        0,
    },
    .data_size = 0,
};

// ///////////////// ipc //////////////////
STATIC struct aicpu_platform_device_info ipc_device_info = {
    .name = "arm,ipc",
    .res = {
        {
            .start = SOC_PERI_IPC_REG_REG,
            .end = SOC_PERI_IPC_REG_REG + 0xFFF,
            .flags = IORESOURCE_MEM,
        },
        {
            .start = 201,   // start num is 201
            .end = 201,     // end num is 201
            .flags = IORESOURCE_IRQ,
        },
        {
            .start = 213,  // start num is 213
            .end = 213,    // end num is 213
            .flags = IORESOURCE_IRQ,
        },
    },
    .num_res = 3,  // num_res is 3
    .data = {
        0,
    },
    .data_size = 0,
};

// ///////////////// power_manager //////////////////
STATIC struct aicpu_platform_device_info pwr_device_info = {
    .name = "core-subctrl",
    .res = {
        {
            .start = POWER_REG_PA_BASE,
            .end = POWER_REG_PA_BASE + 0xFFF,
            .flags = IORESOURCE_MEM,
        },
    },
    .num_res = 1,
    .data = {
        0,
    },
    .data_size = 0,
};

// ///////////gic_commo////////////////////////
STATIC struct aicpu_ts_ctrl_intr gic_data = {
    .intr_type = AICPU_TS_CTRL_INTR_SPI,
};

int devdrv_construct_aicpu_dt(u64 vaddr, u32 dev_id, u32 tsid)
{
    int ret;
    struct dt_machine_desc *mach_desc = NULL;
    struct aicpu_system_config *config = NULL;
    struct dt_platform_device_info_desc *platform_device_info_desc = NULL;

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
    /* Construct dt_machine_desc */
    mach_desc = (struct dt_machine_desc *)((uintptr_t)(vaddr + MACHINE_DESC_OFF));
    mach_desc->magic = SYSTEM_CONFIG_FLAG;
    mach_desc->dev_num = DEV_NUM;
    /* Hisi: chip_id, chip_type, chip_version */
    mach_desc->chip_id = 0;
    mach_desc->chip_type = 0;
    mach_desc->chip_version = 0;

    ret = strcpy_s(mach_desc->name, DT_NAME_MAX_SIZE, "hisilicon,hi1910");
    if (ret != 0) {
        devdrv_drv_err("strcpy_s error, ret = %d\n", ret);
        return -EINVAL;
    }

    ret = memcpy_s(&ipc_device_info.data, DT_DEV_DATA_MAX_NUM, &ipc_data, sizeof(struct ipc_cpu_id));
    if (ret != 0) {
        devdrv_drv_err("memcpy_s error, ret = %d\n", ret);
        return -EINVAL;
    }

    ipc_device_info.data_size = sizeof(struct ipc_cpu_id);

    ret = memcpy_s(&gic_device_info.data, DT_DEV_DATA_MAX_NUM, &gic_data, sizeof(struct aicpu_ts_ctrl_intr));
    if (ret != 0) {
        devdrv_drv_err("memcpy_s error, ret = %d\n", ret);
        return -EINVAL;
    }
    gic_device_info.data_size = sizeof(struct aicpu_ts_ctrl_intr);

    /* Construct dt_platform_device_info_desc */
    platform_device_info_desc = (struct dt_platform_device_info_desc *)((uintptr_t)(vaddr + DEV_GRP_DESC_OFF));
    ret = memcpy_s(&platform_device_info_desc->plat_dev_info_grp[0], sizeof(struct aicpu_platform_device_info),
                   &arch_timer_device_info, sizeof(struct aicpu_platform_device_info));
    if (ret != 0) {
        devdrv_drv_err("memcpy_s error, ret = %d\n", ret);
        return -EINVAL;
    }
    /* memcpy_s arch_pmu_device_info to plat_dev_info_grp[1] */
    ret = memcpy_s(&platform_device_info_desc->plat_dev_info_grp[1], sizeof(struct aicpu_platform_device_info),
                   &arch_pmu_device_info, sizeof(struct aicpu_platform_device_info));
    if (ret != 0) {
        devdrv_drv_err("memcpy_s error, ret = %d\n", ret);
        return -EINVAL;
    }
    /* memcpy_s gic_device_info to plat_dev_info_grp[2] */
    ret = memcpy_s(&platform_device_info_desc->plat_dev_info_grp[2], sizeof(struct aicpu_platform_device_info),
                   &gic_device_info, sizeof(struct aicpu_platform_device_info));
    if (ret != 0) {
        devdrv_drv_err("memcpy_s error, ret = %d\n", ret);
        return -EINVAL;
    }
    /* memcpy_s ipc_device_info to plat_dev_info_grp[3] */
    ret = memcpy_s(&platform_device_info_desc->plat_dev_info_grp[3], sizeof(struct aicpu_platform_device_info),
                   &ipc_device_info, sizeof(struct aicpu_platform_device_info));
    if (ret != 0) {
        devdrv_drv_err("memcpy_s error, ret = %d\n", ret);
        return -EINVAL;
    }
    /* memcpy_s pwr_device_info to plat_dev_info_grp[4] */
    ret = memcpy_s(&platform_device_info_desc->plat_dev_info_grp[4], sizeof(struct aicpu_platform_device_info),
                   &pwr_device_info, sizeof(struct aicpu_platform_device_info));
    if (ret != 0) {
        devdrv_drv_err("memcpy_s error, ret = %d\n", ret);
        return -EINVAL;
    }

    return 0;
}
#if (!defined(CFG_FEATURE_RC_MODE) && !defined(AOS_LLVM_BUILD) && !defined(CFG_NOT_SURPPORT_PCIE_HOST_DEVICE_COMM))
STATIC int devdrv_config_ts_dma_chan_sqcq_desc(u32 dev_id, u32 chan_num,
    struct ts_dma_chan_sqcq_desc *sq_desc, struct ts_dma_chan_sqcq_desc *cq_desc)
{
    u32 i;
    int ret;
    for (i = 0; i < chan_num; i++) {
        ret = devdrv_dma_alloc_sq_desc_for_ts(dev_id, &sq_desc[i].dma_addr, &sq_desc[i].phy_addr, &sq_desc[i].len);
        if (ret != 0) {
            devdrv_drv_err("dev %u id %u alloc sq desc failed ret %d.\n", dev_id, i, ret);
            return ret;
        }

        ret = devdrv_dma_alloc_cq_desc_for_ts(dev_id, &cq_desc[i].dma_addr, &cq_desc[i].phy_addr, &cq_desc[i].len);
        if (ret != 0) {
            devdrv_drv_err("dev %u id %u alloc sq desc failed ret %d.\n", dev_id, i, ret);
            return ret;
        }
    }
    return 0;
}
#endif

void devdrv_config_ts_dma_chan_info(struct devdrv_info *dev_info, u64 vaddr)
{
#if ((defined CFG_FEATURE_RC_MODE) || (defined CFG_NOT_SURPPORT_PCIE_HOST_DEVICE_COMM))
    return;
#else
    struct agentdrv_ts_dma_chan_info dma_chan_info = {0};
    struct aicpu_system_config *config = NULL;
    int ret;
    int i;

    config = (struct aicpu_system_config *)(uintptr_t)(vaddr);
    if (config == NULL) {
        return;
    }
#ifdef AOS_LLVM_BUILD
    config->chan_id_base =0;
    config->chan_num = 0;
    config->nvme_pf_num = 0;
    config->product_num = 0;
#else
    for (i = 0; i < WAIT_TIMES; i++) {
        if (agentdrv_get_ts_dma_chan_info(dev_info->dev_id, &dma_chan_info) != 0) {
            msleep(SLEEP_TIME_MS);
            continue;
        } else {
            break;
        }
    }
    if ((i == WAIT_TIMES) || (dma_chan_info.chan_num > (u32)TS_DMA_CHAN_MAX_NUM)) {
        devdrv_drv_err("Get ts dma channel info fail.\n");
        return;
    }
    config->chan_id_base =dma_chan_info.chan_id_base;
    config->chan_num = dma_chan_info.chan_num;
    config->nvme_pf_num = dma_chan_info.pf_num;
    config->product_num = dma_chan_info.func_total;

    ret = devdrv_config_ts_dma_chan_sqcq_desc(dev_info->dev_id, config->chan_num, config->sq_desc, config->cq_desc);
    if (ret != 0) {
        devdrv_drv_err("ts dma chan sqcq desc config failed.\n");
        return;
    }
#endif
#endif
}

#ifndef TSDRV_UT
int devdrv_config_ts_valid_nid_type(u32 nid_type[][TS_MEMTYPE_MAX], u32 *nid_type_num)
{
    u32 num = sizeof(ts_vaild_nid_type) / sizeof(ts_vaild_nid_type[0]);
    int i, j;

    if (num > TS_MEM_VALID_RANGE_MAX_NUM) {
        devdrv_drv_err("ts_vaild_nid_type num %u is invalid, max nid_num: %u.\n", num, TS_MEM_VALID_RANGE_MAX_NUM);
        return -EINVAL;
    }

    *nid_type_num = num;
    for (i = 0; i < num; i++) {
        for (j = 0; j < TS_MEMTYPE_MAX; j++) {
            nid_type[i][j] = ts_vaild_nid_type[i][j];
        }
    }
    return 0;
}
#endif

#ifdef TSDRV_UT
void devdrv_destroy_ts_irq(struct devdrv_info *dev_info)
{
}
#endif
