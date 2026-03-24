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
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/msi.h>
#include <linux/gpio/consumer.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/property.h>
#include <linux/nodemask.h>
#include <linux/acpi.h>

#include "tsdrv_log.h"
#include "devdrv_platform.h"
#include "devdrv_pm.h"
#include "devdrv_manager.h"
#include "tee/tz_api.h"
#include "devdrv_parse_pdata.h"
#include "tsdrv_pdata.h"
#include "securec.h"
#include "tsdrv_parse.h"
#include "tsdrv_soc_sec.h"
bbox_tsconfig g_config;
struct devdrv_cpu_info g_cpu_info[MAX_CHIP_NUM] = {0};

EXPORT_SYMBOL_GPL(g_cpu_info);

#define DEVDRV_GICD_REG_BASE_ADDR 0xAB000000
#define DEVDRV_GICD_REG_SIZE      0x10000

#define GET_CPU_ID_TIME 100

#ifdef CFG_SOC_PLATFORM_CLOUD_V2
#define ESL_AICORE_BITMAP 0xFFFFFF
#define ESL_AICORE_NUM 24
#define FPGA_AICORE_BITMAP 0xA
#define FPGA_AICORE_NUM 2
#define EMU_AICORE_BITMAP 0xA
#define EMU_AICORE_NUM 2
#endif

STATIC int devdrv_manager_get_aicore_num_level_from_tee(u32 dev_id, u8 *aicore_num_level)
{
#ifndef CFG_SOC_PLATFORM_CLOUD_V2
    pg_cmd_data cmd;
    u64 aiCoreNumLevel = 0;
    int ret;

    ret = verify_init_session(dev_id);
    if (ret != 0) {
        TSDRV_PRINT_ERR("dev(%u) verify init session fail, ret: %d\n", dev_id, ret);
        return ret;
    }

    cmd.module = PG_MODULE_TYPE_AIC;
    cmd.data = PG_DATA_TYPE_NUM_LEVEL;
    ret = tsdrv_sec_read_pg_info(dev_id, cmd, &aiCoreNumLevel);
    if (ret != 0) {
#ifndef TSDRV_UT
        verify_finalize_session(dev_id);
        TSDRV_PRINT_ERR("dev(%u) sec read pg aicore num level fail, ret: %d\n", dev_id, ret);
        return ret;
#endif
    }

    *aicore_num_level = (u8)aiCoreNumLevel;
    verify_finalize_session(dev_id);
#else
    *aicore_num_level = 0;
#endif
    return 0;
}
STATIC int devdrv_manager_get_aicore_freq_level_from_tee(u32 dev_id, u8 *aicore_freq_level)
{
#ifndef CFG_SOC_PLATFORM_CLOUD_V2
    pg_cmd_data cmd;
    u64 aiCoreFreqLevel = 0;
    int ret;

    ret = verify_init_session(dev_id);
    if (ret != 0) {
        TSDRV_PRINT_ERR("dev(%u) verify init session fail, ret: %d\n", dev_id, ret);
        return ret;
    }

    cmd.module = PG_MODULE_TYPE_AIC;
    cmd.data = PG_DATA_TYPE_FREQ_LEVEL;
    ret = tsdrv_sec_read_pg_info(dev_id, cmd, &aiCoreFreqLevel);
    if (ret != 0) {
        verify_finalize_session(dev_id);
        TSDRV_PRINT_ERR("dev(%u) sec read pg aicore freq level fail, ret: %d\n", dev_id, ret);
        return ret;
    }

    *aicore_freq_level = (u8)aiCoreFreqLevel;
    verify_finalize_session(dev_id);
#else
    *aicore_freq_level = 0;
#endif
    return 0;
}
STATIC int devdrv_manager_get_aicore_bitmap_from_tee(u32 dev_id, u64 *aicore_bitmap)
{
#ifndef CFG_SOC_PLATFORM_CLOUD_V2
    pg_cmd_data cmd;
    u64 bitmap = 0;
    int ret;

    ret = verify_init_session(dev_id);
    if (ret != 0) {
        TSDRV_PRINT_ERR("dev(%u) verify init session fail, ret: %d\n", dev_id, ret);
        return ret;
    }

    cmd.module = PG_MODULE_TYPE_AIC;
    cmd.data = PG_DATA_TYPE_CORE_MAP;
    ret = tsdrv_sec_read_pg_info(dev_id, cmd, &bitmap);
    if (ret != 0) {
#ifndef TSDRV_UT
        verify_finalize_session(dev_id);
        TSDRV_PRINT_ERR("dev(%u) sec read pg aicore num fail, ret: %d\n", dev_id, ret);
        return ret;
#endif
    }
    *aicore_bitmap = bitmap;

    verify_finalize_session(dev_id);
#else
    *aicore_bitmap = ESL_AICORE_BITMAP;
#endif
    return 0;
}

STATIC void devdrv_manager_get_aicore_num(u64 bitmap, u32 *aicore_num)
{
    u32 num = 0;
    u32 i;

    for (i = 0; i < FULL_GOOD_CORE_NUM; i++) {
        if (bitmap & (0x01ULL << i))
            num++;
    }
    *aicore_num = num;
}

STATIC int devdrv_manager_get_aicore_freq_from_tee(u32 dev_id, u32 *aicore_freq)
{
#ifndef CFG_SOC_PLATFORM_CLOUD_V2
    pg_cmd_data cmd;
    u64 freq = 0;
    int ret;

    ret = verify_init_session(dev_id);
    if (ret != 0) {
        TSDRV_PRINT_ERR("dev(%u) verify init session fail, ret: %d\n", dev_id, ret);
        return ret;
    }

    cmd.module = PG_MODULE_TYPE_AIC;
    cmd.data = PG_DATA_TYPE_FREQ;
    ret = tsdrv_sec_read_pg_info(dev_id, cmd, &freq);
    if (ret != 0) {
#ifndef TSDRV_UT
        verify_finalize_session(dev_id);
        TSDRV_PRINT_ERR("dev(%u) sec read pg aicore freq fail, ret: %d\n", dev_id, ret);
        return ret;
#endif
    }
    *aicore_freq = (u32)freq;

    verify_finalize_session(dev_id);
#else
    *aicore_freq = 1;
#endif
    return 0;
}

/* The data obtained from efuse is only to avoid version compatibility issues
    and will be deleted in the near future so the return value will not be judged */
STATIC int devdrv_manager_get_aicore_bitmap_from_efuse(u32 dev_id, u64 *bitmap)
{
    void __iomem *efuse_vir_addr = NULL;
    u32 efuse;

    efuse_vir_addr = (void __iomem *)ioremap_wc(EFUSE_BASE_ADDR + dev_id * DEVDRV_CHIP_ADDR_OFFSET, SIZE_OF_64K);
    if (efuse_vir_addr == NULL) {
        TSDRV_PRINT_ERR("efuse_vir_addr memory req fail.\n");
        return -EINVAL;
    }
    efuse = readl_relaxed(efuse_vir_addr + EFUSE_AICORE_NUM_OFFSET);
    iounmap(efuse_vir_addr);
    efuse_vir_addr = NULL;

    if (efuse == 0) {
        TSDRV_PRINT_WARN("efuse may not loaded.\n");
        efuse = 0xFFFFFFFF;
    }

    *bitmap = (u64)efuse;
    return 0;
}

STATIC int devdrv_manager_get_aicore_freq_from_efuse(u32 dev_id, u32 *aicoreFreq)
{
    void __iomem *efuse_vir_addr = NULL;
    u32 efuse;

    efuse_vir_addr = (void __iomem *)ioremap_wc(EFUSE_BASE_ADDR + dev_id * DEVDRV_CHIP_ADDR_OFFSET, SIZE_OF_64K);
    if (efuse_vir_addr == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("efuse_vir_addr memory req fail.\n");
        return -EINVAL;
#endif
    }
    efuse = readl_relaxed(efuse_vir_addr + EFUSE_AICORE_FREQ_OFFSET);
    iounmap(efuse_vir_addr);
    efuse_vir_addr = NULL;

    efuse = (efuse >> PARTIAL_GOOD_EFUSE_OFFSET_NINE) & PARTIAL_GOOD_EFUSE_AND_SEVEN;
    TSDRV_PRINT_DEBUG("get from efuse and efuse=%u.\n", efuse);

    switch (efuse) {
        case PARTIAL_GOOD_FREQ_EFUSE_VAL_ZERO:
            *aicoreFreq = PARTIAL_GOOD_FREQ_LITE;
            break;
        case PARTIAL_GOOD_FREQ_EFUSE_VAL_ONE:
            *aicoreFreq = PARTIAL_GOOD_FREQ_MEDIUM;
            break;
        case PARTIAL_GOOD_FREQ_EFUSE_VAL_THREE:
            *aicoreFreq = PARTIAL_GOOD_FREQ_LITE;
            break;
        case PARTIAL_GOOD_FREQ_EFUSE_VAL_FIVE:
            *aicoreFreq = PARTIAL_GOOD_FREQ_PRO;
            break;
        default:
            TSDRV_PRINT_ERR("get from efuse err(%u).\n", efuse);
            *aicoreFreq = 0;
            return -EINVAL;
    }
    return 0;
}

STATIC void devdrv_get_aicore_info(struct devdrv_platform_data *pdata)
{
    int ret;

    ret = devdrv_manager_get_aicore_bitmap_from_tee(pdata->dev_id, &(pdata->ai_core_bitmap));
    if (ret != 0) {
        TSDRV_PRINT_WARN("dev(%u) get aicore bitmap from tee failed.\n", pdata->dev_id);
        ret = devdrv_manager_get_aicore_bitmap_from_efuse(pdata->dev_id, &(pdata->ai_core_bitmap));
        if (ret != 0) {
            TSDRV_PRINT_WARN("dev(%u) get aicore bitmap from efuse failed.\n", pdata->dev_id);
        }
    }

    devdrv_manager_get_aicore_num(pdata->ai_core_bitmap, &(pdata->ai_core_num));

    ret = devdrv_manager_get_aicore_freq_from_tee(pdata->dev_id, &(pdata->ai_core_freq));
    if (ret != 0) {
        TSDRV_PRINT_WARN("dev(%u) get aicore freq from tee failed.\n", pdata->dev_id);
        ret = devdrv_manager_get_aicore_freq_from_efuse(pdata->dev_id, &(pdata->ai_core_freq));
        if (ret != 0) {
            TSDRV_PRINT_WARN("dev(%u) get aicore freq from efuse failed.\n", pdata->dev_id);
        }
    }

    ret = devdrv_manager_get_aicore_num_level_from_tee(pdata->dev_id, &(pdata->ai_core_num_level));
    if (ret != 0) {
        pdata->ai_core_num_level = 0;
        TSDRV_PRINT_WARN("dev(%u) get aicore num level from tee failed.\n", pdata->dev_id);
    }

    ret = devdrv_manager_get_aicore_freq_level_from_tee(pdata->dev_id, &(pdata->ai_core_freq_level));
    if (ret != 0) {
        pdata->ai_core_freq_level = 0;
        TSDRV_PRINT_WARN("dev(%u) get aicore freq level from tee failed.\n", pdata->dev_id);
    }

    TSDRV_PRINT_INFO("dev(%u) aicore num(%u) aicore num level(%u) aicore freq(%u) "
        "aicore freq level(%u) bitmap(0x%llx).\n",
        pdata->dev_id, pdata->ai_core_num, (u32)pdata->ai_core_num_level,
        pdata->ai_core_freq, (u32)pdata->ai_core_freq_level, pdata->ai_core_bitmap);
    return;
}

int tsdrv_get_plat_info(struct devdrv_platform_data *pdata)
{
#define CUR_REG_RIGHT_SHIFT 16
#define CUR_ONE_AICORE      1
#define CUR_TWO_AICORES     2

    u32 __iomem *version_base = NULL;
    u32 reg_val;
    u32 type;
    u32 ver;
    u32 num;

    version_base = (u32 __iomem *)pdata->platform_info.sysctl_base;
    if (version_base == NULL) {
        TSDRV_PRINT_ERR("sysctl base is NULL.\n");
        return -ENOMEM;
    }

    reg_val = readl_relaxed((void __iomem *)((uintptr_t)version_base + DEVDRV_SYSCTL_VERSION_REG_OFFSET));
    if (reg_val == 0) {
        pdata->env_type = DEVDRV_PLAT_TYPE_ASIC;
        devdrv_get_aicore_info(pdata);
        return 0;
    }
#ifndef TSDRV_UT
    type = (reg_val & DEVDRV_PLAT_MASK) >> CUR_REG_RIGHT_SHIFT; // shift right 16 bits
    ver = reg_val & DEVDRV_PLAT_VERSION_MASK;
    num = reg_val & DEVDRV_PLAT_AI_CORE_NUM_MASK;

    if (type == DEVDRV_PLAT_TYPE_EMU) {
        pdata->env_type = DEVDRV_PLAT_TYPE_EMU;
    } else if (type == DEVDRV_PLAT_TYPE_ESL) {
        pdata->env_type = DEVDRV_PLAT_TYPE_ESL;
    } else if (type == DEVDRV_PLAT_TYPE_FPGA) {
        pdata->env_type = DEVDRV_PLAT_TYPE_FPGA;
    } else {
        TSDRV_PRINT_ERR("invalid plat type, type = %u.\n", type);
        return -EINVAL;
    }

#ifdef CFG_SOC_PLATFORM_CLOUD_V2
    pdata->ai_core_num = num;
    if (type == DEVDRV_PLAT_TYPE_EMU) {
        pdata->ai_core_num = EMU_AICORE_NUM;
    } else if (type == DEVDRV_PLAT_TYPE_ESL) {
        pdata->ai_core_num = ESL_AICORE_NUM;
    } else {
        pdata->ai_core_num = FPGA_AICORE_NUM;
    }
#else
    if (num == DEVDRV_PLAT_AI_CORE_NUM_1)
        pdata->ai_core_num = CUR_ONE_AICORE;
    else if (num == DEVDRV_PLAT_AI_CORE_NUM_2)
        pdata->ai_core_num = CUR_TWO_AICORES; // ai_core_num is 2
    else if (num == DEVDRV_PLAT_AI_CORE_NUM_3)
        pdata->ai_core_num = CUR_TWO_AICORES; // ai_core_num is 2
    else {
        TSDRV_PRINT_ERR("invalid aicore num, num = %u.\n", num);
        return -EINVAL;
    }
#endif
#endif /* TSDRV_UT */
    return 0;
}

STATIC void devdrv_set_devdrv_cpu_info(u32 dev_id, struct agentdrv_cpu_info cpu_info)
{
    g_cpu_info[dev_id].cpu_info = cpu_info;
    g_cpu_info[dev_id].set_flag = CPU_INFO_CONFIGED;
}

u32 devdrv_get_cpu_number(u32 dev_id, u32 cpu_type)
{
    if (g_cpu_info[dev_id].set_flag != CPU_INFO_CONFIGED) {
        TSDRV_PRINT_ERR("CPU info of dev[%u] has not been configured yet.\n", dev_id);
        return 0;
    }

    switch (cpu_type) {
        case CPU_TYPE_OF_CCPU:
            return g_cpu_info[dev_id].cpu_info.ccpu_num;
        case CPU_TYPE_OF_DCPU:
            return g_cpu_info[dev_id].cpu_info.dcpu_num;
        case CPU_TYPE_OF_TS:
            return g_cpu_info[dev_id].cpu_info.tscpu_num;
        case CPU_TYPE_OF_AICPU:
            return g_cpu_info[dev_id].cpu_info.aicpu_num;
        default:
            return 0;
    }
}

int tsdrv_get_aicpu_occupy_bitmap(u32 dev_id, u32 plat_type, u32 *bitmap)
{
    struct agentdrv_cpu_info cpu_info = { 0 };
    u32 i;

    *bitmap = 0;

    while (1) {
        /* The cpu information is the tag information,so wate here for ever if get fail */
        if (!agentdrv_get_cpu_info(dev_id, &cpu_info)) {
            break;
        }
        msleep(10);
    }

    for (i = 0; i < cpu_info.aicpu_num; i++)
        *bitmap |= (1U << (i + cpu_info.ccpu_num + cpu_info.dcpu_num));
    devdrv_set_devdrv_cpu_info(dev_id, cpu_info);

    return 0;
}

int devdrv_get_acpi_addr_info(struct platform_device *pdev, struct devdrv_platform_data *pdata)
{
    const char *devdrv_addr_base_name[DEVDRV_DTS_MAX_RESOURCE_NODE] = {
        "GICV3_base",
        "TS_SUBSYSCTL_base",
        "TS_DOORBELL_base",
        "TS_SRAM_base",
        "Dispatch_base",
        "SYSCTL_base",
        "MN_base",
#ifdef CFG_SOC_PLATFORM_CLOUD_V2
        "STARS_base",
        "RAS0_base",
        "RAS2_base",
#else
        NULL,
        NULL,
        NULL,
#endif
        NULL
    };
    u64 addr = 0;
    int ret;
    u32 i;

    for (i = 0; i < DEVDRV_DTS_MAX_RESOURCE_NODE; i++) {
        if (devdrv_addr_base_name[i] != NULL) {
            if ((ret = device_property_read_u64(&pdev->dev, devdrv_addr_base_name[i], &addr)) != 0) {
                TSDRV_PRINT_ERR("device_property_read_u64 %s "
                                "failed.ret=%d, 0x%pK\n",
                                devdrv_addr_base_name[i], ret, (void *)(uintptr_t)addr);
                return -EINVAL;
            }
            pdata->platform_info.devdrv_addr_base[i] = addr;
            TSDRV_PRINT_FPGA("i(%u), name(%s), addr(0x%pK).\n", i, devdrv_addr_base_name[i], (void *)(uintptr_t)addr);
        }
    }

    return 0;
}

int devdrv_get_dts_addr_info(struct platform_device *pdev, struct devdrv_platform_data *pdata)
{
#ifndef TSDRV_UT
    struct resource *res = NULL;
    u32 i;

    for (i = 0; i < DEVDRV_DTS_MAX_RESOURCE_NODE; i++) {
        res = platform_get_resource(pdev, IORESOURCE_MEM, i);
        if (res == NULL) {
            TSDRV_PRINT_INFO("platform_get_resource i = %u.\n", i);
            return 0;
        }
        pdata->platform_info.devdrv_addr_base[i] = res->start;
        TSDRV_PRINT_FPGA("i(%u), addr(0x%pK).\n", i, (void *)(uintptr_t)res->start);
    }
#endif
    return 0;
}

int devdrv_get_base_addr_info(struct platform_device *pdev, struct devdrv_platform_data *pdata)
{
    if (!acpi_disabled) {
        return devdrv_get_acpi_addr_info(pdev, pdata);
    } else {
        return devdrv_get_dts_addr_info(pdev, pdata);
    }
}

void tsdrv_dfx_cq_irq_bind_core(struct devdrv_platform_data *pdata)
{
}

#ifndef TSDRV_UT
int devdrv_get_aicpu_freq_from_tee(u32 dev_id, u32 *aicpu_freq)
{
#ifndef CFG_SOC_PLATFORM_CLOUD_V2
    pg_cmd_data cmd;
    u64 freq = 0;
    int ret;

    ret = verify_init_session(dev_id);
    if (ret != 0) {
        TSDRV_PRINT_ERR("dev(%u) verify init session fail, ret: %d\n", dev_id, ret);
        return ret;
    }

    cmd.module = PG_MODULE_TYPE_CPU;
    cmd.data = PG_DATA_TYPE_FREQ;
    ret = tsdrv_sec_read_pg_info(dev_id, cmd, &freq);
    if (ret != 0) {
        verify_finalize_session(dev_id);
        TSDRV_PRINT_ERR("dev(%u) sec read pg aicpu freq fail, ret: %d\n", dev_id, ret);
        return ret;
    }
    *aicpu_freq = (u32)freq;

    verify_finalize_session(dev_id);
#else
    *aicpu_freq = 0;
#endif
    return 0;
}
#endif

