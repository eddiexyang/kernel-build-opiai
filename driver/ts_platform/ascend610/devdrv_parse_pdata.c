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
#include <linux/of_irq.h>
#include <linux/delay.h>
#include "devdrv_parse_pdata.h"
#include <linux/of_gpio.h>
#include <linux/acpi.h>
#ifdef CFG_SOC_PLATFORM_MDC_V11
#include <linux/aos/cpu_domain_info.h>
#endif
#ifdef AOS_LLVM_BUILD
#include <linux/compiler_types.h>
#include <linux/gpio.h>
#include <linux/property.h>

#include "davinci_intf_info.h"
#endif
#include "ascend_platform.h"
#include "tsdrv_common.h"
#include "tsdrv_log.h"
#include "devdrv_platform.h"
#include "devdrv_pm.h"
#include "devdrv_manager.h"
#include "devdrv_manager.h"
#include "hiss/hsm_info.h"
#include "devdrv_parse_pdata.h"
#include "user_cfg_interface.h"
#include "tsdrv_pdata.h"
#include "tsdrv_parse.h"
#include "tsdrv_soc_sec.h"
#include "drv_comm.h"

#define AI_CORE_TYPE 0
#define VECTOR_CORE_TYPE 1
#define FULL_GOOD_CORE_NUM 64

/* sofy pin 338 */
#define DEVDRV_MDC_GPIO_SLOT_ID 338
#define DEVDRV_GPIO_NAME "gpio-read"
#define DEVDRV_GICV3_REG_SIZE (0x10000)

struct devdrv_cpu_info g_cpu_info[MAX_CHIP_NUM] = {0};
EXPORT_SYMBOL(g_cpu_info);

STATIC int devdrv_manager_get_core_bitmap_from_tee(u32 dev_id, u64 *aicore_bitmap,
    unsigned char core_logic_type)
{
#ifdef CFG_FEATURE_PG_V2
    *aicore_bitmap = 0;
#else
    pg_cmd_data cmd;
    u64 bitmap = 0;
    int ret;

    if (core_logic_type == AI_CORE_TYPE) {
        cmd.module = PG_MODULE_TYPE_AIC;
    } else if (core_logic_type == VECTOR_CORE_TYPE) {
        cmd.module = PG_MODULE_TYPE_AIV;
    }

    cmd.data = PG_DATA_TYPE_CORE_MAP;
    ret = tsdrv_sec_read_pg_info(dev_id, cmd, &bitmap);
    if (ret != 0) {
        TSDRV_PRINT_ERR("dev(%u) sec read core num fail, ret: %d\n", dev_id, ret);
        return ret;
    }
    *aicore_bitmap = bitmap;
#endif
    return 0;
}

STATIC void devdrv_manager_get_core_num(u64 bitmap, u32 *core_num)
{
    u32 num = 0;
    u32 i;

    for (i = 0; i < FULL_GOOD_CORE_NUM; i++) {
        if (bitmap & (0x01ULL << i))
            num++;
    }
    *core_num = num;
}

STATIC int devdrv_manager_get_core_freq_from_tee(u32 dev_id, u32 *core_freq,
    unsigned char core_logic_type)
{
#ifdef CFG_FEATURE_PG_V2
    *core_freq = 0;
#else
    pg_cmd_data cmd;
    u64 freq = 0;
    int ret;

    if (core_logic_type == AI_CORE_TYPE) {
        cmd.module = PG_MODULE_TYPE_AIC;
    } else if (core_logic_type == VECTOR_CORE_TYPE) {
        cmd.module = PG_MODULE_TYPE_AIV;
    }

    cmd.data = PG_DATA_TYPE_FREQ;
    ret = tsdrv_sec_read_pg_info(dev_id, cmd, &freq);
    if (ret != 0) {
        TSDRV_PRINT_ERR("dev(%u) sec read core freq fail, ret: %d\n", dev_id, ret);
        return ret;
    }
    *core_freq = (u32)freq;
#endif
    return 0;
}

STATIC void devdrv_get_aicore_info(struct devdrv_platform_data *pdata)
{
    int ret;

    ret = devdrv_manager_get_core_bitmap_from_tee(pdata->dev_id, &(pdata->ai_core_bitmap), AI_CORE_TYPE);
    if (ret != 0) {
        TSDRV_PRINT_WARN("dev(%u) get aicore bitmap from tee failed.\n", pdata->dev_id);
    }

    devdrv_manager_get_core_num(pdata->ai_core_bitmap, &(pdata->ai_core_num));

    ret = devdrv_manager_get_core_freq_from_tee(pdata->dev_id, &(pdata->ai_core_freq), AI_CORE_TYPE);
    if (ret != 0) {
        TSDRV_PRINT_WARN("dev(%u) get aicore freq from tee failed.\n", pdata->dev_id);
    }
    TSDRV_PRINT_WARN("dev(%u) aicore num(%u) aicore freq(%u) bitmap(0x%llx).\n",
        pdata->dev_id, pdata->ai_core_num, pdata->ai_core_freq, pdata->ai_core_bitmap);
    return;
}


STATIC void devdrv_get_vector_core_info(struct devdrv_platform_data *pdata)
{
    int ret;

    ret = devdrv_manager_get_core_bitmap_from_tee(pdata->dev_id, &(pdata->vector_core_bitmap), VECTOR_CORE_TYPE);
    if (ret != 0) {
        TSDRV_PRINT_WARN("dev(%u) get vectorcore bitmap from tee failed.\n", pdata->dev_id);
    }

    devdrv_manager_get_core_num(pdata->vector_core_bitmap, &(pdata->vector_core_num));

    ret = devdrv_manager_get_core_freq_from_tee(pdata->dev_id, &(pdata->vector_core_freq), VECTOR_CORE_TYPE);
    if (ret != 0) {
        TSDRV_PRINT_WARN("dev(%u) get vectorcore freq from tee failed.\n", pdata->dev_id);
    }
    TSDRV_PRINT_WARN("dev(%u) vectorcore num(%u) vectorcore freq(%u) bitmap(0x%llx).\n",
        pdata->dev_id, pdata->vector_core_num, pdata->vector_core_freq, pdata->vector_core_bitmap);
    return;
}

#ifdef CFG_SOC_PLATFORM_MDC_V11
STATIC int devdrv_get_gpio_value(int gpio_num, u32 *gpio_value)
{
    int ret;

    if (!gpio_is_valid(gpio_num)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("invalid gpio, gpio=%d\n", gpio_num);
        return -EINVAL;
#endif
    }

    ret = gpio_request(gpio_num, DEVDRV_GPIO_NAME);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("gpio [%d] request failed\n", gpio_num);
        return ret;
#endif
    }

    *gpio_value = gpio_get_value(gpio_num);

    (void)gpio_free(gpio_num);

    return 0;
}

STATIC int devdrv_get_slot_id_by_gpio(u32 *slot_id)
{
    int ret;
    ret = devdrv_get_gpio_value(SLOTID_GPIO_NUM, slot_id);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("get slot_id failed, (ret=%d)\n", ret);
        return ret;
#endif
    }
    return 0;
}

STATIC int devdrv_check_is_evb_board(u32 board_id)
{
    return (((board_id >= MIN_BOARD_ID) && (board_id <= MAX_BOARD_ID)) ? 1 : 0);
}

#endif

STATIC bool tsdrv_env_type_valid(u32 type)
{
#ifndef TSDRV_UT

    if ((type == DEVDRV_PLAT_TYPE_EMU) ||
        (type == DEVDRV_PLAT_TYPE_ESL) ||
        (type == DEVDRV_PLAT_TYPE_FPGA)) {
        return true;
    } else {
        return false;
    }
#endif
}

#define TSDRV_1_AI_CORE    (1U)
#define TSDRV_2_AI_CORE    (2U)

STATIC int tsdrv_chk_and_set_ai_core_num(u32 num, u32 *ai_core_num)
{
#ifndef TSDRV_UT
    if (num == DEVDRV_PLAT_AI_CORE_NUM_1) {
        *ai_core_num = TSDRV_2_AI_CORE;
    } else if (num == DEVDRV_PLAT_AI_CORE_NUM_2) {
        *ai_core_num = TSDRV_2_AI_CORE;
    } else if (num == DEVDRV_PLAT_AI_CORE_NUM_3) {
        *ai_core_num = TSDRV_1_AI_CORE;
    } else {
        TSDRV_PRINT_ERR("invalid ai core num.\n");
        if (tsdrv_get_chip_type() == CHIP_TYPE_MDC_BS9SX1A) {
            TSDRV_PRINT_ERR("V2 assign ai core num to 1.\n");
            *ai_core_num = TSDRV_1_AI_CORE;
        } else {
            return -1;
        }
    }
#endif
    return 0;
}

int tsdrv_get_plat_info(struct devdrv_platform_data *pdata)
{
#define CUR_REG_RIGHT_SHIFT 16

    u32 __iomem *version_base = NULL;
    u32 board_id;
    u32 slot_id = 0;
    u32 reg_val = 0;
    u32 type = 0;
    u32 ver;
    u32 num = 0;
    int ret;

    version_base = (u32 __iomem *)pdata->platform_info.sysctl_base;
    if (version_base == NULL) {
        TSDRV_PRINT_ERR("sysctl base is NULL.\n");
        return -ENOMEM;
    }
#ifndef AOS_LLVM_BUILD
    board_id = readl_relaxed((void __iomem *)(uintptr_t)((u64)(uintptr_t)version_base + DRV_BOARD_ID_REG_OFFSET));
#else
    board_id = readl((void __iomem *)(uintptr_t)((u64)(uintptr_t)version_base + DRV_BOARD_ID_REG_OFFSET));
#endif
    board_id &= BOARD_ID_REG_VALUE_MASK; /* The lower sixteen bits of this register indicate the board ID. */
    pdata->platform_info.board_id = board_id;

#ifdef CFG_SOC_PLATFORM_MDC_V11
    if (!devdrv_check_is_evb_board(board_id)) {
        ret = devdrv_get_slot_id_by_gpio(&slot_id);
        if (ret != 0) {
#ifndef TSDRV_UT
            TSDRV_PRINT_ERR("read slot id failed ret = %d, slot_id = %u\n", ret, slot_id);
            slot_id = 0xff;
#endif
        }
    } else {
        TSDRV_PRINT_INFO("board is evb type, board_id = %u\n", board_id);
        slot_id = 0; /* if board type is evb, can not get slot id information, set default 0 */
    }
#else
#ifndef AOS_LLVM_BUILD
    slot_id = readl_relaxed((void __iomem *)(uintptr_t)((u64)(uintptr_t)version_base +
                SOC_SYSCTRL_CFG_SC_PAD_INFO_REG - SOC_SYSCTRL_CFG_BASE));
#else
    slot_id = readl((void __iomem *)(uintptr_t)((u64)(uintptr_t)version_base +
                SOC_SYSCTRL_CFG_SC_PAD_INFO_REG - SOC_SYSCTRL_CFG_BASE));
#endif
    slot_id &= DEVDRV_SLOT_ID_BIT;
#endif

    pdata->platform_info.slot_id = slot_id;
    TSDRV_PRINT_INFO("board id = 0x%x, slot id = 0x%x\n", pdata->platform_info.board_id, pdata->platform_info.slot_id);

#ifndef AOS_LLVM_BUILD
    reg_val = readl_relaxed((void __iomem *)(uintptr_t)((u64)(uintptr_t)version_base +
        DEVDRV_SYSCTL_VERSION_REG_OFFSET));
#else
    reg_val = readl((void __iomem *)(uintptr_t)((u64)(uintptr_t)version_base +
        DEVDRV_SYSCTL_VERSION_REG_OFFSET));
#endif
    if (reg_val == 0) {
        pdata->env_type = DEVDRV_PLAT_TYPE_ASIC;
        /* 获取失败，记录日志，不影响初始化流程 */
        devdrv_get_aicore_info(pdata);
        devdrv_get_vector_core_info(pdata);
        return 0;
    }
    type = (reg_val & DEVDRV_PLAT_MASK) >> CUR_REG_RIGHT_SHIFT; /* shift right 16 bits */
    ver = reg_val & DEVDRV_PLAT_VERSION_MASK;
    num = reg_val & DEVDRV_PLAT_AI_CORE_NUM_MASK;

    if (!tsdrv_env_type_valid(type)) {
        TSDRV_PRINT_ERR("invalid plat type. type(%u)\n", type);
        return -1;
    } else {
        pdata->env_type = type;
    }

    ret = tsdrv_chk_and_set_ai_core_num(num, &pdata->ai_core_num);
    if (ret != 0) {
        TSDRV_PRINT_ERR("invalid ai core num. num(%u) ver(%u)\n", num, ver);
        return ret;
    }

    TSDRV_PRINT_DEBUG("dev(%u) env type=%u aicore num(%u) aicore freq(%u) bitmap(0x%llx).\n",
        pdata->dev_id, type, pdata->ai_core_num, pdata->ai_core_freq, pdata->ai_core_bitmap);
    return 0;
}

int devdrv_get_acpi_addr_info(struct platform_device *pdev, struct devdrv_platform_data *pdata)
{
    const char *devdrv_addr_base_name[] = {
        "GICV3_base",
        "Dispatch_base",
        "SYSCTL_base",
        "MN_base",
        "LPM3_STATUS_base",
        "FW_CORE_RESET_base",
        "FW_CL_SUBCTRL_base",
        "TS_FW_CORE_RESET_base",
    };
    u64 addr = 0;
    int ret;
    u32 i;

    for (i = 0; i < DEVDRV_DTS_MAX_RESOURCE_NODE; i++) {
        if (devdrv_addr_base_name[i] != NULL) {
            if ((ret = device_property_read_u64(&pdev->dev, devdrv_addr_base_name[i], &addr)) != 0) {
                TSDRV_PRINT_ERR("device_property_read_u64 %s failed. ret=%d, 0x%pK\n",
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
    struct resource *res = NULL;
    u32 i;

    for (i = 0; i < DEVDRV_DTS_MAX_RESOURCE_NODE; i++) {
        res = platform_get_resource(pdev, IORESOURCE_MEM, i);
        if (res == NULL) {
#ifndef TSDRV_UT
            TSDRV_PRINT_ERR("platform_get_resource failed i = %u.\n", i);
            return -EINVAL;
#endif
        }
        pdata->platform_info.devdrv_addr_base[i] = res->start;
    }

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

STATIC void devdrv_set_devdrv_cpu_info(u32 dev_id, struct agentdrv_cpu_info cpu_info)
{
    g_cpu_info[dev_id].cpu_info = cpu_info;
    g_cpu_info[dev_id].set_flag = CPU_INFO_CONFIGED;
}

u32 devdrv_get_cpu_number(u32 dev_id, u32 cpu_type)
{
    if (g_cpu_info[dev_id].set_flag != CPU_INFO_CONFIGED) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("CPU info of dev[%u] has not been configured yet.\n", dev_id);
        return 0;
#endif
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
#ifdef CFG_SOC_PLATFORM_MDC_V11
int tsdrv_get_aicpu_occupy_bitmap(u32 dev_id, u32 plat_type, u32 *bitmap)
{
    struct agentdrv_cpu_info cpu_info = { 0 };
    struct cpu_domain_info domain_info;
    u64 total_core_num;
    u32 i;
    int err;

    *bitmap = 0;

    err = get_cpudomain_info(&domain_info);
    if (err != 0) {
        TSDRV_PRINT_ERR("get_cpudomain_info failed,err = %d.\n", err);
        return err;
    }
    cpu_info.ccpu_num = domain_info.ctrlcpu_num;
    cpu_info.dcpu_num = domain_info.datacpu_num;
    cpu_info.aicpu_num = domain_info.aicpu_num;
    cpu_info.tscpu_num = DEVDRV_TSCPU_CORE_NUM;

    if (tsdrv_get_chip_type() != CHIP_TYPE_MDC_BS9SX1A && tsdrv_get_chip_type() != HISI_MINI_V3) {
        if ((cpu_info.aicpu_num < 1) && (cpu_info.ccpu_num < 1)) {
            TSDRV_PRINT_ERR("dev_id(%u) ctrl cpu num[%u] aicpu num[%u] out of range.\n",
                dev_id, cpu_info.ccpu_num, cpu_info.aicpu_num);
            return -EINVAL;
        }
    }

    TSDRV_PRINT_INFO("Get cpu info. (ccpu_num=%u; aicpu_num=%u; tscpu_num=%u)\n",
        cpu_info.ccpu_num, cpu_info.aicpu_num, cpu_info.tscpu_num);

    total_core_num = (u64)cpu_info.aicpu_num + cpu_info.ccpu_num + cpu_info.dcpu_num;
    for (i = 0; i < cpu_info.aicpu_num; i++) {
        *bitmap |= (1U << (i + cpu_info.ccpu_num + cpu_info.dcpu_num));
    }

    devdrv_set_devdrv_cpu_info(dev_id, cpu_info);
    return 0;
}
#else
int tsdrv_get_aicpu_occupy_bitmap(u32 dev_id, u32 plat_type, u32 *bitmap)
{
    struct agentdrv_cpu_info cpu_info = { 0 };
    u64 total_core_num;
    s32 retry_time = CPU_INFO_RETRY_TIME;
#ifdef CFG_FEATURE_AICPU_NUM_CFG_UNLIMIT
    u32 mini_aicpu_num = 0;
#else
    u32 mini_aicpu_num = 1;
#endif
    u32 i;

    *bitmap = 0;

#ifndef CFG_FEATURE_CPU_NUMS_FIXED
    while (retry_time > 0) {
        /* The cpu information is the tag information,so wate here for ever if get fail */
#ifndef AOS_LLVM_BUILD
        if (!agentdrv_get_cpu_info(dev_id, &cpu_info)) {
#else
        if (!davinci_intf_get_cpu_info(dev_id, &cpu_info)) {
#endif
            break;
        }
        msleep(10); /* 10 delay for get cpu info */
        retry_time--;
    }
#else
    if (dev_user_cfg_get_cpu_number(dev_id, (dev_cpu_nums_cfg_t *)&cpu_info) != 0) {
        retry_time = 0;
    }
#endif

    if (retry_time == 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_WARN("dev_id(%u) get cpu info from pcie interface failed, set default info.\n", dev_id);
        cpu_info.ccpu_num = DEFAULT_CCPU_CORE_NUM;
        cpu_info.dcpu_num = DEFAULT_DCPU_CORE_NUM;
        cpu_info.aicpu_num = DEFAULT_AICPU_CORE_NUM;
#endif
    }

    cpu_info.tscpu_num = DEVDRV_TSCPU_CORE_NUM;

    if (tsdrv_get_chip_type() != CHIP_TYPE_MDC_BS9SX1A && tsdrv_get_chip_type() != HISI_MINI_V3) {
        if ((cpu_info.aicpu_num < mini_aicpu_num) || (cpu_info.ccpu_num < 1)) {
            TSDRV_PRINT_ERR("dev_id(%u) ctrl cpu num[%u] aicpu num[%u] out of range.\n",
                dev_id, cpu_info.ccpu_num, cpu_info.aicpu_num);
            return -EINVAL;
        }
    }

    TSDRV_PRINT_INFO("Get cpu info. (ccpu_num=%u; aicpu_num=%u; tscpu_num=%u)\n",
        cpu_info.ccpu_num, cpu_info.aicpu_num, cpu_info.tscpu_num);

    total_core_num = (u64)cpu_info.aicpu_num + cpu_info.ccpu_num + cpu_info.dcpu_num;
    for (i = 0; i < cpu_info.aicpu_num; i++) {
        *bitmap |= (1U << (i + cpu_info.ccpu_num + cpu_info.dcpu_num));
    }

    devdrv_set_devdrv_cpu_info(dev_id, cpu_info);
    return 0;
}
#endif

void tsdrv_dfx_cq_irq_bind_core(struct devdrv_platform_data *pdata)
{
}
