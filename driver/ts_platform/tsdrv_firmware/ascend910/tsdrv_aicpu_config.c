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
#include <linux/interrupt.h>
#include <linux/kallsyms.h>
#include <linux/mmzone.h>
#include <linux/securec.h>

#include "tsdrv_platform_interface.h"
#include "tsdrv_firmware_load.h"
#include "devdrv_interface.h"
#include "devdrv_common.h"
#include "devdrv_manager_comm.h"
#include "tsdrv_aicpu_config.h"
#include "aicpu_dt.h"
#include "devdrv_platform.h"

#define POWER_REG_PA_BASE 0x81888000ULL

#define SRAM_PA_BASE 0xAF201000ULL
#define SRAM_PA_SIZE (0x40000ULL - 0x1000ULL)

#define GICD_REG_PA_BASE 0xAB000000ULL
#define GICD_REG_PA_SIZE 0x100000ULL
#define GICR_REG_PA_SIZE 0x40000ULL

#define GITS_REG_PA_BASE 0xAD000000ULL
#define GITS_REG_PA_SIZE 0x100000ULL

#define TS_SYSCTRL_BASE 0xAF000000ULL

#define DT_GIC_MULTICHIP_OFF 0
#define DEV_NUM 4
#define TOTAL_SIZE (5 * 1024)

#define GET_CHAN_INFO_MAX_NUM   30
#define TS_SPECIAL_MBX_START_ID 16
#define TS_SPECIAL_MBX_NUM 8
#define TS_COMMON_MBX_START_ID 41
#ifdef CFG_SOC_PLATFORM_CLOUD_V2
#define TS_COMMON_MBX_NUM 8
#else
#define TS_COMMON_MBX_NUM 4
#endif
#define TS_CORE0_INDEX 16
#define TS_CORE1_INDEX 17
#define TS_CORE2_INDEX 18
#define TS_CORE3_INDEX 19

#ifdef CFG_SOC_PLATFORM_CLOUD_V2_ESL
#define TS_START_CPUID 8
#else
#define TS_START_CPUID 2
#endif

STATIC u32 ts_vaild_nid_type[][TS_MEMTYPE_MAX] = {
    {DBL_MEMTYPE_HBM, DBL_SUB_MEMTYPE_TS},
    {DBL_MEMTYPE_DDR, DBL_SUB_MEMTYPE_TS}
};

// //////////////// arch_timer //////////////////
struct aicpu_platform_device_info arch_timer_device_info = {
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
struct aicpu_platform_device_info arch_pmu_device_info = {
    .name = "arm,armv8-pmu",
    .res = {
        {
            .start = 23, // start num is 23
            .end = 23,   // end num is 23
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
struct aicpu_platform_device_info gic_device_info = {
    .name = "arm,gic-v3",
    .res = {
        {
            .start = GITS_REG_PA_BASE,
            .end = GITS_REG_PA_BASE + GITS_REG_PA_SIZE,
            .flags = IORESOURCE_MEM,
        },
    },
    .num_res = 1,
    .data = {
        0,
    },
    .data_size = 0,
};

// ///////////////// power_manager //////////////////
struct aicpu_platform_device_info pwr_device_info = {
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

// ///////////gic_common////////////////////////
struct aicpu_ts_ctrl_intr gic_data = {
    .intr_type = AICPU_TS_CTRL_INTR_LPI,
};

int devdrv_construct_aicpu_dt(u64 vaddr, u32 dev_id, u32 tsid)
{
    struct dt_platform_device_info_desc *platform_device_info_desc = NULL;
    struct aicpu_system_config *config = NULL;
    struct dt_machine_desc *mach_desc = NULL;
    int ret;

    /* Construct aicpu_system_config */
    config = (struct aicpu_system_config *)(uintptr_t)(vaddr);
    if (config == NULL) {
        devdrv_drv_err("config is null!\n");
        return -EINVAL;
    }
    config->gic_multichip_off = DT_GIC_MULTICHIP_OFF;

    config->sram_pa_base = SRAM_PA_BASE + CHIP_BASE_PA_DDR * dev_id;
    config->sram_pa_size = SRAM_PA_SIZE;
    config->gicd_pa_base = GICD_REG_PA_BASE + CHIP_BASE_PA_DDR * dev_id;
    config->gicd_pa_size = GICD_REG_PA_SIZE;
    config->gicr_pa_size = GICR_REG_PA_SIZE;
    config->tzpc_pa_base = DT_INVALID_PA_BASE;
    config->total_size = TOTAL_SIZE;
    config->ts_aicpu_status_base = TS_SYSCTRL_BASE + CHIP_BASE_PA_DDR * dev_id;
    /* Construct dt_machine_desc */
    mach_desc = (struct dt_machine_desc *)(uintptr_t)(vaddr + MACHINE_DESC_OFF);
    mach_desc->magic = SYSTEM_CONFIG_FLAG;
    mach_desc->dev_num = DEV_NUM;
    /* Hisi: chip_id, chip_type, chip_version */
    mach_desc->chip_id = 0;
    mach_desc->chip_type = 0;
    mach_desc->chip_version = 0;

    ret = strcpy_s(mach_desc->name, sizeof(mach_desc->name), "hisilicon,hi1980");
    if (ret != 0) {
        devdrv_drv_err("copy mach_desc->name fail, ret=%d!\n", ret);
        return -EINVAL;
    }

    ret = memcpy_s(gic_device_info.data, DT_DEV_DATA_MAX_NUM, &gic_data,
                   sizeof(struct aicpu_ts_ctrl_intr));
    if (ret != 0) {
        devdrv_drv_err("copy gic_device_info.data fail, ret=%d!\n", ret);
        return -EINVAL;
    }
    gic_device_info.data_size = sizeof(struct aicpu_ts_ctrl_intr);
    gic_device_info.res[0].start += CHIP_BASE_PA_DDR * dev_id;
    gic_device_info.res[0].end += CHIP_BASE_PA_DDR * dev_id;
    pwr_device_info.res[0].start += CHIP_BASE_PA_DDR * dev_id;
    pwr_device_info.res[0].end += CHIP_BASE_PA_DDR * dev_id;

    /* Construct dt_platform_device_info_desc */
    platform_device_info_desc = (struct dt_platform_device_info_desc *)(uintptr_t)(vaddr + DEV_GRP_DESC_OFF);
    ret = memcpy_s(&platform_device_info_desc->plat_dev_info_grp[0], sizeof(struct aicpu_platform_device_info),
                   &arch_timer_device_info, sizeof(struct aicpu_platform_device_info));
    if (ret != 0) {
        devdrv_drv_err("copy plat_dev_info_grp[0] fail, ret=%d!\n", ret);
        return -EINVAL;
    }
    /* memcpy_s  arch_pmu_device_info to plat_dev_info_grp[1] */
    ret = memcpy_s(&platform_device_info_desc->plat_dev_info_grp[1], sizeof(struct aicpu_platform_device_info),
                   &arch_pmu_device_info, sizeof(struct aicpu_platform_device_info));
    if (ret != 0) {
        devdrv_drv_err("copy plat_dev_info_grp[1] fail, ret=%d!\n", ret);
        return -EINVAL;
    }
    /* memcpy_s  gic_device_info to plat_dev_info_grp[2] */
    ret = memcpy_s(&platform_device_info_desc->plat_dev_info_grp[2], sizeof(struct aicpu_platform_device_info),
                   &gic_device_info, sizeof(struct aicpu_platform_device_info));
    if (ret != 0) {
        devdrv_drv_err("copy plat_dev_info_grp[2] fail, ret=%d!\n", ret);
        return -EINVAL;
    }
    /* memcpy_s  pwr_device_info to plat_dev_info_grp[3] */
    ret = memcpy_s(&platform_device_info_desc->plat_dev_info_grp[3], sizeof(struct aicpu_platform_device_info),
                   &pwr_device_info, sizeof(struct aicpu_platform_device_info));
    if (ret != 0) {
        devdrv_drv_err("copy plat_dev_info_grp[3] fail, ret=%d!\n", ret);
        return -EINVAL;
    }

    return 0;
}

STATIC irqreturn_t devdrv_ts_dma_done_interrupt(int irq, void *data)
{
    return IRQ_HANDLED;
}

static struct irq_data *get_top_parent_irq_data(int irq)
{
    struct irq_data *irq_data = NULL;
    struct irq_data *parent = NULL;
    struct irq_desc *desc = NULL;

    if ((desc = irq_to_desc((u32)irq)) == NULL) {
        devdrv_drv_err("irq invalid.\n");
        return NULL;
    }
    irq_data = irq_desc_get_irq_data(desc);
    parent = irq_data->parent_data;

    while (parent != NULL) {
        irq_data = parent;
        parent = irq_data->parent_data;
    }
    return irq_data;
}

#define STARS_TSCPU_ID 0
#define STARS_TSCPU_NUM 4
STATIC void config_ts_ipcint_route(struct devdrv_info *dev_info)
{
    struct devdrv_platform_data *pdata = dev_info->pdata;
    u32 ts_core_id;
    int int_index;
    int num_cpus;
    int ret;
    int i;

    num_cpus = num_possible_cpus();
#ifdef CFG_SOC_PLATFORM_CLOUD_V2
    ts_core_id = num_cpus + dev_info->dev_id * STARS_TSCPU_NUM + STARS_TSCPU_ID;
    devdrv_drv_info("(devid=%u; num_cpus=(%u, %u); ts_core_id=%u)\n", dev_info->dev_id, num_cpus,
        num_possible_cpus(), ts_core_id);
#else
    if (dev_info->env_type == FIRMWARE_HW_FPGA) {
        ts_core_id = (u32)num_cpus + (DEVDRV_MAX_TS_CORE_NUM * dev_info->dev_id) +
                pdata->ts_pdata[0].ts_cpu_core_num - 1U;
    } else {
        ts_core_id = (u32)num_cpus + (DEVDRV_MAX_TS_CORE_NUM * dev_info->dev_id) +
                pdata->ts_pdata[0].ts_cpu_core_num - 1U;
    }
#endif

    if (dev_info->dts_cfg.ipc_mbx_int_start_id == 0) {
        devdrv_drv_err("dev[%u] mbx int start id is invalid\n", dev_info->dev_id);
        return;
    }
    devdrv_drv_info("print cpu info below\n");
    devdrv_drv_info("evn_type = %u, "
                    "ai_cpu_core_id = %u, "
                    "ai_cpu_core_num = %u, "
                    "dev_id = %u, "
                    "ts_cpu_core_num = %u, "
                    "ts_core_id = %u, "
                    "num_cpus = %d\n",
                    (u32)dev_info->env_type, dev_info->ai_cpu_core_id, dev_info->ai_cpu_core_num, dev_info->dev_id,
                    pdata->ts_pdata[0].ts_cpu_core_num, ts_core_id, num_cpus);

#ifdef CFG_SOC_PLATFORM_CLOUD_V2
#ifndef TSDRV_UT
    int_index = dev_info->dts_cfg.ipc_mbx_int_start_id;
    for (i = int_index; i < (int_index + TS_COMMON_MBX_NUM); i++) {
        devdrv_drv_info("--- ipc i(%u) close ---\n", i);
        ret = request_irq(i, devdrv_ts_dma_done_interrupt, 0, "ts_ipc_done_interrupt", NULL);
        if (ret != 0) {
            devdrv_drv_err("request devdrv_ts_ipc_interrupt error!\n");
            return;
        }

        (void)devdrv_set_irq_affinity(i, get_cpu_mask(ts_core_id));
    }
#endif
#else
    int_index = dev_info->dts_cfg.ipc_mbx_int_start_id + TS_SPECIAL_MBX_START_ID;
    for (i = int_index; i < (int_index + TS_SPECIAL_MBX_NUM); i++) {
        ret = request_irq((u32)i, devdrv_ts_dma_done_interrupt, 0, "ts_ipc_done_interrupt", NULL);
        if (ret != 0) {
            devdrv_drv_err("request devdrv_ts_ipc_interrupt error!\n");
            return;
        }

        (void)devdrv_set_irq_affinity((u32)i, get_cpu_mask(ts_core_id));
    }

    int_index = dev_info->dts_cfg.ipc_mbx_int_start_id + TS_COMMON_MBX_START_ID;
    for (i = int_index; i < (int_index + TS_COMMON_MBX_NUM); i++) {
        ret = request_irq((u32)i, devdrv_ts_dma_done_interrupt, 0, "ts_ipc_done_interrupt", NULL);
        if (ret != 0) {
            devdrv_drv_err("request devdrv_ts_ipc_interrupt error!\n");
            return;
        }
        (void)devdrv_set_irq_affinity((u32)i, get_cpu_mask(ts_core_id));
    }
#endif
    return;
}

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

STATIC irqreturn_t tscpu_irq_handle(int irq, void *data)
{
    return IRQ_HANDLED;
}

#define STARS_CPU_NUM 4
#define STARS_TSCPU_ID 0

#define TMP_ASIC_MIN_PG_TAISHAN_CORE_NUM 2
#define TMP_ASIC_1DIE_GIC_INIT_NUM 8
#define TMP_FPGA_2DIE_TAISHAN_CORE_NUM 4

#define TMP_FPGA_2DIE_GIC_INIT_NUM_DIE0 10
#define TMP_FPGA_2DIE_GIC_INIT_NUM_DIE1 16

STATIC void devdrv_config_set_ts_irq_affinity(struct devdrv_info *dev_info, struct aicpu_system_config *config)
{
#ifndef TSDRV_UT
    u32 i;
    u32 irq_num = 0;
    u32 start_irq = 0;
    int ret, tscpu_id;
    struct irq_data *irq_data = NULL;
    u32 dev_id = dev_info->dev_id;
    u32 cpu_num;

    cpu_num = num_possible_cpus();  // 2die, cpu num is 16; 2p(0p cpu num is 8)
    devdrv_drv_info("dev_id(%u), taishan cpu_num(%u)\n", dev_id, cpu_num);

    if (cpu_num == TMP_ASIC_MIN_PG_TAISHAN_CORE_NUM) {
        cpu_num = TMP_ASIC_1DIE_GIC_INIT_NUM;
        devdrv_drv_info("devid(%u), cpu num(%u)\n", dev_id, cpu_num);
    }

    if (cpu_num == TMP_FPGA_2DIE_TAISHAN_CORE_NUM) {
        if (dev_id == 0) {
            cpu_num = TMP_FPGA_2DIE_GIC_INIT_NUM_DIE0;
        } else {
            cpu_num = TMP_FPGA_2DIE_GIC_INIT_NUM_DIE1;
        }

        devdrv_drv_info("devid(%u), cpu num(%u)\n", dev_id, cpu_num);
    }

    ret = devdrv_get_tscpu_irq_info(dev_id, &start_irq, &irq_num);
    if (ret != 0) {
        devdrv_drv_err("dev_id %u get irq failed\n", dev_id);
        return;
    }

    devdrv_drv_info("dev_id %u start_irq %u irq_num %u\n", dev_id, start_irq, irq_num);

    if (irq_num == 0) {
        return;
    }

    tscpu_id = cpu_num + STARS_CPU_NUM * dev_id + STARS_TSCPU_ID;

    devdrv_drv_info("dev_id(%u), tscpuid(%d)\n", dev_id, tscpu_id);
    for (i = 0; i < irq_num; i++) {
        ret = request_irq(start_irq + i, tscpu_irq_handle, 0, "tscpu_proc", NULL);
        if (ret != 0) {
            devdrv_drv_err("dev_id %u request_irq %u failed ret %d\n", dev_id, start_irq + i, ret);
            return;
        }

        (void)devdrv_set_irq_affinity(start_irq + i, get_cpu_mask((u32)tscpu_id));
    }

    irq_data = get_top_parent_irq_data(start_irq);
    config->ts_int_start_id = (u32)irq_data->hwirq;
    devdrv_drv_info("dev_id(%u), tscpu_id(%d), start irq(%u), start hwirq(%u), irq(%u)\n",
        dev_id, tscpu_id, start_irq, config->ts_int_start_id, irq_data->irq);
#endif
}

static int devdrv_check_ts_dma_chan_info(struct devdrv_info *dev_info, u64 vaddr)
{
    if (dev_info == NULL) {
        devdrv_drv_err("The dev_info is NULL.\n");
        return -EINVAL;
    }

    if (vaddr == 0) {
        devdrv_drv_err("The vaddr is NULL.\n");
        return -EINVAL;
    }
    return 0;
}

void devdrv_config_ts_dma_chan_info(struct devdrv_info *dev_info, u64 vaddr)
{
    struct agentdrv_ts_dma_chan_info dma_chan_info = {0};
    struct aicpu_system_config *config = NULL;
    struct irq_data *irq_data = NULL;
    int ts_core_id;
    int ret;
    u32 i;
    u32 j;
    u32 chip_type;

    ret = devdrv_check_ts_dma_chan_info(dev_info, vaddr);
    if (ret != 0) {
#ifndef TSDRV_UT
        devdrv_drv_err("Failed to check ts dma channel info.\n");
        return;
#endif
    }

    config = (struct aicpu_system_config *)(uintptr_t)(vaddr);
    chip_type = devdrv_get_dev_chip_type(dev_info->dev_id);
    if (chip_type == HISI_CLOUD_V2) {
        devdrv_config_set_ts_irq_affinity(dev_info, config);
        config_ts_ipcint_route(dev_info);

        irq_data = get_top_parent_irq_data(dev_info->dts_cfg.ipc_mbx_int_start_id);
        if (irq_data == NULL) {
#ifndef TSDRV_UT
            devdrv_drv_err("Get ts dma hwirq fail.\n");
            return;
#endif
        }
        config->ipc_mbx_int_start_id = (u32)irq_data->hwirq;
        devdrv_drv_info("dev_id(%u), ipc start irq(%u), ipc hwirq(%lu), cfg ipc irq(%u)\n",
            dev_info->dev_id, dev_info->dts_cfg.ipc_mbx_int_start_id, irq_data->hwirq, config->ipc_mbx_int_start_id);
        return;
    }

    for (i = 0; i < (u32)GET_CHAN_INFO_MAX_NUM; i++) {
        if (agentdrv_get_ts_dma_chan_info(dev_info->dev_id, &dma_chan_info)) {
            msleep(100);
            continue;
        } else {
            break;
        }
    }
    if ((i == (u32)GET_CHAN_INFO_MAX_NUM) || (dma_chan_info.chan_num > (u32)TS_DMA_CHAN_MAX_NUM)) {
        devdrv_drv_err("Get ts dma channel info fail.\n");
        return;
    }

    config->chan_id_base = dma_chan_info.chan_id_base;
    config->chan_num = dma_chan_info.chan_num;
    config->nvme_pf_num = dma_chan_info.pf_num;
    config->product_num = dma_chan_info.func_total;

    ret = devdrv_config_ts_dma_chan_sqcq_desc(dev_info->dev_id, config->chan_num, config->sq_desc, config->cq_desc);
    if (ret != 0) {
        devdrv_drv_err("ts dma chan sqcq desc config failed.\n");
        return;
    }

    irq_data = get_top_parent_irq_data(dma_chan_info.chan_done_irq_base);
    if (irq_data == NULL) {
        devdrv_drv_err("Get ts dma hwirq fail.\n");
        return;
    }
    config->chan_done_irq_base = irq_data->hwirq;

    /* root to ts 0 core */
    ts_core_id = num_possible_cpus() + DEVDRV_MAX_TS_CORE_NUM * dev_info->dev_id;
    devdrv_drv_info("Bind irq to ts cpu.(tscpu=%d; irq_base=%u)\n", ts_core_id, config->chan_done_irq_base);
    for (i = dma_chan_info.chan_done_irq_base; i < (dma_chan_info.chan_done_irq_base + config->chan_num); i++) {
        ret = request_irq(i, devdrv_ts_dma_done_interrupt, 0, "ts_dma_done_interrupt", NULL);
        if (ret != 0) {
            devdrv_drv_err("request devdrv_ts_dma_done_interrupt error!\n");
            goto unmap_irq;
        }
        (void)devdrv_set_irq_affinity(i, get_cpu_mask((u32)ts_core_id));
    }

    irq_data = get_top_parent_irq_data(dev_info->dts_cfg.ipc_mbx_int_start_id);
    if (irq_data == NULL) {
        devdrv_drv_err("Get ts ipc hwirq fail.\n");
        return;
    }
    config_ts_ipcint_route(dev_info);
    config->ipc_mbx_int_start_id = irq_data->hwirq;
    dev_info->ts_irq_init = 1;
    return;

unmap_irq:
    j = i;
    for (i = dma_chan_info.chan_done_irq_base; i < j; i++) {
        (void)free_irq(i, NULL);
    }
}

#ifdef TSDRV_UT
#define GET_TS_DMA_CHAN_TIME 100
void devdrv_destroy_ts_irq(struct devdrv_info *dev_info)
{
    u32 i;
    u32 int_index;
    struct irq_data *irq_data = NULL;
    struct agentdrv_ts_dma_chan_info dma_chan_info = {0};

    if (dev_info->ts_irq_init == 0) {
        return;
    }

    for (i = 0; i < (u32)GET_CHAN_INFO_MAX_NUM; i++) {
        if (agentdrv_get_ts_dma_chan_info(dev_info->dev_id, &dma_chan_info)) {
            msleep(GET_TS_DMA_CHAN_TIME);
            continue;
        } else {
            break;
        }
    }

    if ((i == (u32)GET_CHAN_INFO_MAX_NUM) || (dma_chan_info.chan_num > (u32)TS_DMA_CHAN_MAX_NUM)) {
        devdrv_drv_err("Get ts dma channel info fail.\n");
        return;
    }

    irq_data = get_top_parent_irq_data(dma_chan_info.chan_done_irq_base);
    if (irq_data == NULL) {
        devdrv_drv_err("Get ts dma hwirq fail.\n");
        return;
    }

    for (i = dma_chan_info.chan_done_irq_base;
        i < (dma_chan_info.chan_done_irq_base + dma_chan_info.chan_num); i++) {
        (void)devdrv_set_irq_affinity(i, NULL);
        (void)free_irq(i, NULL);
    }

    int_index = dev_info->dts_cfg.ipc_mbx_int_start_id + TS_SPECIAL_MBX_START_ID;
    for (i = int_index; i < (int_index + TS_SPECIAL_MBX_NUM); i++) {
        (void)devdrv_set_irq_affinity(i, NULL);
        (void)free_irq(i, NULL);
    }

    int_index = dev_info->dts_cfg.ipc_mbx_int_start_id + TS_COMMON_MBX_START_ID;
    for (i = int_index; i < (int_index + TS_COMMON_MBX_NUM); i++) {
        (void)devdrv_set_irq_affinity(i, NULL);
        (void)free_irq(i, NULL);
    }
}
#endif

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
