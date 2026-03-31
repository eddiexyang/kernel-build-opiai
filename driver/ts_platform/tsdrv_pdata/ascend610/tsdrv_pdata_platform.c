/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
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

#include <linux/kernel.h>
#include <linux/property.h>
#ifndef AOS_LLVM_BUILD
#include <linux/irq.h>
#endif
#include <linux/of_irq.h>
#include <linux/acpi.h>

#include "devdrv_platform.h"
#include "tsdrv_log.h"
#include "tsdrv_pdata.h"

#define NUMA_NUM_PER_DEVICE  0x3

enum dts_addr_index {
    GIC_BASE_INDEX = 0,
    DISPATCH_INDEX = 1,
    SYSCTL_INDEX = 2,
};

enum dts_ts_addr_index {
    TS_SUBSYSCTL_INDEX = 0,
    TS_DOORBELL_INDEX = 1,
    TS_SRAM_INDEX = 2,
    TS_STARS_INDEX = 3,
    TS_ADDR_MAX_INDEX
};

static const char *dts_ts_addr_name[TS_ADDR_MAX_INDEX] = {
    "ts_sysctl_paddr",
    "ts_doorbell_paddr",
    "ts_sram_paddr",
    "stars_reg",
};

static const char *dts_ts_size_name[TS_ADDR_MAX_INDEX] = {
    "ts_sysctl_size",
    "ts_doorbell_size",
    "ts_sram_size",
    "stars_size",
};

#ifdef CFG_SOC_PLATFORM_MINIV3
enum dts_ts_irq_index {
    TS_TRIGGER_IRQ0_INDEX = 0,
    TS_MAILBOX_ACK_IRQ_INDEX = 1,
    TS_FUNC_CQ_IRQ_INDEX = 2,
    TS_TRIGGER_IRQ1_INDEX = 3,
    TS_TRIGGER_IRQ2_INDEX = 4,
    TS_TRIGGER_IRQ3_INDEX = 5,
    TS_CQ_UPDATE_IRQ_INDEX = 6,
    TS_TRIGGER_IRQ4_INDEX = 7,
    TS_IRQ_MAX_INDEX = 20
};

enum ts_hwirq {
    TS_TRIGGER_HWIRQ0 = 14,
    TS_MAILBOX_ACK_HWIRQ = 15,
    TS_FUNC_CQ_HWIRQ = 16,
    TS_TRIGGER_HWIRQ1 = 17,
    TS_TRIGGER_HWIRQ2 = 18,
    TS_TRIGGER_HWIRQ3 = 19,
    TS_CQ_UPDATE_HWIRQ = 20,
    TS_TRIGGER_HWIRQ4 = 21,
};
#else
enum dts_ts_irq_index {
    TS_CQ_UPDATE_IRQ_INDEX = 0,
    TS_MAILBOX_ACK_IRQ_INDEX = 1,
    TS_FUNC_CQ_IRQ_INDEX = 2,
#ifdef CFG_FEATURE_PROF_HWTS_THROUGH
    TS_PROF_AICORE_IRQ_INDEX = 3,
    TS_PROF_HWTS_LOG_IRQ_INDEX = 4,
#endif
    TS_IRQ_MAX_INDEX
};
#endif

#define TS_CQ_UPDATE_IRQ_NUM 1
#define TS_MAILBOX_ACK_IRQ_NUM 1
#define TS_FUNC_CQ_IRQ_NUM 1
#define TS_PROF_AICORE_IRQ_NUM 1
#define TS_PROF_HWTS_LOG_IRQ_NUM 1

int tsdrv_get_gicv3_addr(u32 devid, phys_addr_t *paddr, size_t *size)
{
    return tsdrv_get_addr_by_index(devid, GIC_BASE_INDEX, paddr, size);
}

int tsdrv_get_dispatch_addr(u32 devid, phys_addr_t *paddr, size_t *size)
{
    return tsdrv_get_addr_by_index(devid, DISPATCH_INDEX, paddr, size);
}

int tsdrv_get_sysctrl_addr(u32 devid, phys_addr_t *paddr, size_t *size)
{
    return tsdrv_get_addr_by_index(devid, SYSCTL_INDEX, paddr, size);
}

static int get_ts_addr_from_device_node(u32 devid, u32 tsid,
    phys_addr_t *paddr, size_t *size, u32 addr_index)
{
    u32 of_tsid;
    u32 read_size;
    struct device_node *node = NULL;
    struct device_node *son = NULL;
    struct platform_device *pdev = NULL;

    pdev = tsdrv_get_platform_device(devid);
    if (pdev == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Get platform device failed. (devid=%u)\n", devid);
#endif
        return -ENODEV;
    }

    node = pdev->dev.of_node;
    for_each_child_of_node(node, son) {
        if (of_property_read_u32(son, "tsid", &of_tsid) != 0) {
            TSDRV_PRINT_ERR("Get ts id failed. (devid=%u)\n", devid);
            continue;
        }

        if (of_tsid == tsid) {
            if (of_property_read_u64(son, dts_ts_addr_name[addr_index], paddr) != 0) {
                TSDRV_PRINT_ERR("Get ts addr failed. (devid=%u; tsid=%u; index=%u)\n",
                    devid, tsid, addr_index);
                return -EINVAL;
            }

            if (of_property_read_u32(son, dts_ts_size_name[addr_index], &read_size) != 0) {
                TSDRV_PRINT_ERR("Get ts addr size failed. (devid=%u; tsid=%u; index=%u)\n",
                    devid, tsid, addr_index);
                return -EINVAL;
            }

            TSDRV_PRINT_DEBUG("Get addr info. (devid=%u; tsid=%u; name=%s; addr=%pK; size=%x)\n",
                devid, tsid, dts_ts_addr_name[addr_index], (void *)(*paddr), read_size);
            *size = read_size;
            return 0;
        }
    }

    return -EINVAL;
}

static int get_ts_addr_from_acpi(u32 devid, u32 tsid,
    phys_addr_t *paddr, size_t *size, u32 addr_index)
{
#ifndef TSDRV_UT
    struct platform_device *pdev = NULL;
    u64 read_size;
    int ret;

    if (addr_index >= TS_ADDR_MAX_INDEX) {
        TSDRV_PRINT_ERR(" index is invalid. (idx=%u).\n", addr_index);
        return -EINVAL;
    }

    pdev = tsdrv_get_platform_device(devid);
    if (pdev == NULL) {
        TSDRV_PRINT_ERR("Get platform device failed. (devid=%u)\n", devid);
        return -ENODEV;
    }

    ret = device_property_read_u64(&pdev->dev, dts_ts_addr_name[addr_index], paddr);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Read device property failed. (ret=%d; index=%u)\n", ret, addr_index);
        return ret;
    }

    ret = device_property_read_u64(&pdev->dev, dts_ts_size_name[addr_index], &read_size);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Read device property failed. (ret=%d; index=%u)\n", ret,  addr_index);
        return ret;
    }

    *size = (size_t)read_size;
    TSDRV_PRINT_FPGA("Get addr info. (devid=%u; name=%s; addr=%pK; size=%llx)\n",
        devid, dts_ts_addr_name[addr_index], (void *)*paddr, read_size);
#endif
    return 0;
}
int tsdrv_get_ts_sysctrl_addr(u32 devid, u32 tsid, phys_addr_t *paddr, size_t *size)
{
    if (!acpi_disabled) {
        return get_ts_addr_from_acpi(devid, tsid, paddr, size, TS_SUBSYSCTL_INDEX);
    } else {
        return get_ts_addr_from_device_node(devid, tsid, paddr, size, TS_SUBSYSCTL_INDEX);
    }
}

int tsdrv_get_ts_doorbell_addr(u32 devid, u32 tsid, phys_addr_t *paddr, size_t *size)
{
    if (!acpi_disabled) {
        return get_ts_addr_from_acpi(devid, tsid, paddr, size, TS_DOORBELL_INDEX);
    } else {
        return get_ts_addr_from_device_node(devid, tsid, paddr, size, TS_DOORBELL_INDEX);
    }
}

int tsdrv_get_ts_sram_addr(u32 devid, u32 tsid, phys_addr_t *paddr, size_t *size)
{
    if (!acpi_disabled) {
        return get_ts_addr_from_acpi(devid, tsid, paddr, size, TS_SRAM_INDEX);
    } else {
        return get_ts_addr_from_device_node(devid, tsid, paddr, size, TS_SRAM_INDEX);
    }
}

int tsdrv_get_ts_stars_addr(u32 devid, u32 tsid, phys_addr_t *paddr, size_t *size)
{
#ifdef CFG_SOC_PLATFORM_MINIV3
    return get_ts_addr_from_device_node(devid, tsid, paddr, size, TS_STARS_INDEX);
#else
    return 0;
#endif
}

int tsdrv_get_ts_stars_rtsq_addr(u32 devid, u32 tsid, phys_addr_t *paddr, size_t *size)
{
#ifdef CFG_SOC_PLATFORM_MINIV3
    int ret;

    ret = get_ts_addr_from_device_node(devid, tsid, paddr, size, TS_STARS_INDEX);
    if (ret != 0) {
        return ret;
    }

    *paddr += TSDRV_STARS_RTSQ_OFFSET;
    *size = TSDRV_STARS_RTSQ_SIZE;
    return 0;
#else
    *paddr = 0;
    *size = 0;
    return 0;
#endif
}

int tsdrv_get_tsensor_shm_addr(u32 devid, u32 tsid, phys_addr_t *paddr, size_t *size)
{
    *paddr = 0;
    *size = 0;
    return 0;
}

int tsdrv_get_fftscpu_id(u32 devid)
{
    return -EINVAL;
}

u32 tsdrv_get_ts_num(u32 devid)
{
    u32 ts_num;
    u32 tsid;
    struct device_node *node = NULL;
    struct device_node *son = NULL;
    struct platform_device *pdev = NULL;

    pdev = tsdrv_get_platform_device(devid);
    if (pdev == NULL) {
        TSDRV_PRINT_ERR("Get platform device failed. (devid=%u)\n", devid);
        return 0;
    }

    ts_num = 0;
    if (acpi_disabled) {
        node = pdev->dev.of_node;
        for_each_child_of_node(node, son) {
            if (of_property_read_u32(son, "tsid", &tsid) == 0) {
                ts_num++;
            }
        }
    } else {
        if (device_property_read_u32(&pdev->dev, "tsid", &tsid) == 0) {
            ts_num++;
        } else {
            TSDRV_PRINT_WARN("read tsid failed .\n");
            ts_num = 1;
        }
    }

    TSDRV_PRINT_INFO("Get ts num. (ts_num=%u)\n", ts_num);
    return ts_num;
}

int tsdrv_get_chip_id(struct platform_device *pdev, u32 *chip_id)
{
    int ret;

    if (!acpi_disabled) {
        ret = device_property_read_u32(&pdev->dev, "chip_id", chip_id);
    } else {
        ret = of_property_read_u32(pdev->dev.of_node, "chip_id", chip_id);
    }
    if (ret != 0) {
        TSDRV_PRINT_ERR("Get chip id failed. (ret=%d)\n", ret);
        return -EINVAL;
    }

    if (*chip_id >= MAX_CHIP_NUM) {
        TSDRV_PRINT_ERR("Chip id is invalid. (chip_id=%u)\n", *chip_id);
        return -EINVAL;
    }

    return 0;
}

int tsdrv_get_specification(struct platform_device *pdev, enum tsdrv_hwts_spec *spec)
{
    int ret;
    const char *ts_spec = NULL;

    ret = of_property_read_string(pdev->dev.of_node, "ts_spec", &ts_spec);
    /* ts_spec is not configured., The default setting is basic specifications. */
    if (ret != 0) {
        if (ret == -EINVAL || ret == -ENODATA) {
            TSDRV_PRINT_WARN("ts_spec is not configured. set basic specifications as default. (ret=%d)\n", ret);
            *spec = HWTS_SPEC_BASE;
            return 0;
        } else {
            TSDRV_PRINT_ERR("Get ts_psec failed. (ret=%d)\n", ret);
            return -EFAULT;
        }
    }

    if (ts_spec == NULL) {
        TSDRV_PRINT_ERR("ts specification is null.\n");
        return -EFAULT;
    }

    TSDRV_PRINT_INFO("HWTS specification is %s\n", ts_spec);
    if (strcmp(ts_spec, "base") == 0) {
        *spec = HWTS_SPEC_BASE;
    } else if (strcmp(ts_spec, "premium") == 0) {
        *spec = HWTS_SPEC_PREMIUM;
    } else if (strcmp(ts_spec, "ultimate") == 0) {
        *spec = HWTS_SPEC_ULTIMATE;
    } else {
        TSDRV_PRINT_ERR("ts specification configuration is invalid.\n");
        return -EFAULT;
    }
    return 0;
}

int tsdrv_alloc_irqs(u32 devid)
{
    return 0;
}

static int tsdrv_parse_ts_irq(struct device_node *node, u32 *irq_map, int irq_num)
{
    int i;

    for (i = 0; i < irq_num; i++) {
        irq_map[i] = irq_of_parse_and_map(node, i);
        if (irq_map[i] == 0) {
            TSDRV_PRINT_ERR("Parse and map irq failed. (index=%d)\n", i);
            return -ENODEV;
        }
        TSDRV_PRINT_DEBUG("Get irq. (index=%d; irq=%u)\n", i, irq_map[i]);
    }

    return 0;
}

static struct device_node *get_ts_device_node(u32 devid, u32 tsid)
{
    u32 of_tsid;
    struct device_node *node = NULL;
    struct device_node *son = NULL;
    struct platform_device *pdev = NULL;

    pdev = tsdrv_get_platform_device(devid);
    if (pdev == NULL) {
        TSDRV_PRINT_ERR("Get platform device failed. (devid=%u)\n", devid);
        return NULL;
    }

    node = pdev->dev.of_node;
    for_each_child_of_node(node, son) {
        if (of_property_read_u32(son, "tsid", &of_tsid) != 0) {
            TSDRV_PRINT_ERR("Get ts id failed. (devid=%u)\n", devid);
            continue;
        }

        if (tsid == of_tsid) {
            return son;
        }
    }

    return NULL;
}

static int get_irq_from_acpi(u32 devid, u32 tsid, u32 *irq,
    u32 base, u32 irq_num)
{
#ifndef TSDRV_UT
    struct platform_device *pdev = NULL;
    u32 i = 0;

    if (base >= TS_IRQ_MAX_INDEX ||
        irq_num >= TS_IRQ_MAX_INDEX ||
        (base + irq_num) > TS_IRQ_MAX_INDEX) {
        TSDRV_PRINT_ERR("Irq base or num is invalid. (base=%u; num=%u)\n",
            base, irq_num);
        return -EINVAL;
    }

    pdev = tsdrv_get_platform_device(devid);
    if (pdev == NULL) {
        TSDRV_PRINT_ERR("Get platform device failed. (devid=%u)\n", devid);
        return -ENODEV;
    }

    for (i = 0; i < irq_num; i++) {
        irq[i] = (u32)platform_get_irq(pdev, (base + i));
        TSDRV_PRINT_FPGA("Get irq. (index=%u; irq=%u)\n", base + i, irq[i]);
    }
#endif
    return 0;
}
static int get_irq_from_ts_node(u32 devid, u32 tsid, u32 *irq,
    u32 base, u32 irq_num)
{
    int ret;
    u32 i;
    u32 irq_map[TS_IRQ_MAX_INDEX] = { 0 };
    struct device_node *ts_node = NULL;

    if (base >= TS_IRQ_MAX_INDEX ||
        irq_num >= TS_IRQ_MAX_INDEX ||
        (base + irq_num) > TS_IRQ_MAX_INDEX) {
        TSDRV_PRINT_ERR("Irq base or num is invalid. (base=%u; num=%u)\n",
            base, irq_num);
        return -EINVAL;
    }

    ts_node = get_ts_device_node(devid, tsid);
    if (ts_node == NULL) {
        return -ENODEV;
    }

    ret = tsdrv_parse_ts_irq(ts_node, irq_map, TS_IRQ_MAX_INDEX);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Prase irq failed. (devid=%u; tsid=%u)\n", devid, tsid);
        return -EFAULT;
    }

    for (i = 0; i < irq_num ; i++) {
        irq[i] = irq_map[base + i];
        TSDRV_PRINT_DEBUG("Get irq. (index=%u; irq=%u). \n", i, irq_map[base + i]);
    }

    return 0;
}

#ifdef CFG_FEATURE_PROF_HWTS_THROUGH
int tsdrv_get_prof_aicore_irq(u32 devid, u32 tsid, u32 *irq)
{
    if (!acpi_disabled) {
        return -EOPNOTSUPP;
    } else {
        return get_irq_from_ts_node(devid, tsid, irq, TS_PROF_AICORE_IRQ_INDEX, TS_PROF_AICORE_IRQ_NUM);
    }
}
EXPORT_SYMBOL(tsdrv_get_prof_aicore_irq);

int tsdrv_get_prof_hwts_log_irq(u32 devid, u32 tsid, u32 *irq)
{
    if (!acpi_disabled) {
        return -EOPNOTSUPP;
    } else {
        return get_irq_from_ts_node(devid, tsid, irq, TS_PROF_HWTS_LOG_IRQ_INDEX, TS_PROF_HWTS_LOG_IRQ_NUM);
    }
}
EXPORT_SYMBOL(tsdrv_get_prof_hwts_log_irq);
#endif

u32 tsdrv_get_cq_update_irq_num(void)
{
    return TS_CQ_UPDATE_IRQ_NUM;
}

int tsdrv_get_cq_update_irq(u32 devid, u32 tsid, u32 *irq)
{
    if (!acpi_disabled) {
        return get_irq_from_acpi(devid, tsid, irq, TS_CQ_UPDATE_IRQ_INDEX, TS_CQ_UPDATE_IRQ_NUM);
    } else {
        return get_irq_from_ts_node(devid, tsid, irq, TS_CQ_UPDATE_IRQ_INDEX, TS_CQ_UPDATE_IRQ_NUM);
    }
}

int tsdrv_get_cq_update_hwirq(u32 *irq, u32 *hwirq)
{
    int i;

    /* LPI irqs return offset. SPI irqs return hardware irqs. */
    for (i = 0; i < TS_CQ_UPDATE_IRQ_NUM; i++) {
#ifdef CFG_SOC_PLATFORM_MINIV3
        hwirq[i] = TS_CQ_UPDATE_HWIRQ;
#else
        if (tsdrv_get_hwirq_from_irq(irq[i], &hwirq[i]) != 0) {
            return -EINVAL;
        }
#endif
    }

    return 0;
}

int tsdrv_get_mailbox_ack_irq(u32 devid, u32 tsid, u32 *irq)
{
    if (!acpi_disabled) {
        return get_irq_from_acpi(devid, tsid, irq, TS_MAILBOX_ACK_IRQ_INDEX, TS_MAILBOX_ACK_IRQ_NUM);
    } else {
        return get_irq_from_ts_node(devid, tsid, irq, TS_MAILBOX_ACK_IRQ_INDEX, TS_MAILBOX_ACK_IRQ_NUM);
    }
}

int tsdrv_get_mailbox_ack_hwirq(u32 irq, u32 *hwirq)
{
    /* LPI irqs return offset. SPI irqs return hardware irqs. */
#ifdef CFG_SOC_PLATFORM_MINIV3
    *hwirq = TS_MAILBOX_ACK_HWIRQ;
    return 0;
#else
    return tsdrv_get_hwirq_from_irq(irq, hwirq);
#endif
}

#ifdef CFG_SOC_PLATFORM_MINIV3
int tsdrv_get_trigger_irq_info(u32 devid, struct devdrv_ts_pdata *ts_pdata)
{
    int *irq_request = ts_pdata->irq_sq_trigger_request;
    int *irq = ts_pdata->irq_sq_trigger;
    u32 tsid = ts_pdata->tsid;
    int ret;

    ret = get_irq_from_ts_node(devid, tsid, &irq_request[0], TS_TRIGGER_IRQ0_INDEX, 1);  /* pf(vf 0) */
    ret |= get_irq_from_ts_node(devid, tsid, &irq_request[1], TS_TRIGGER_IRQ1_INDEX, 1); /* vf 1 */
    ret |= get_irq_from_ts_node(devid, tsid, &irq_request[2], TS_TRIGGER_IRQ2_INDEX, 1); /* vf 2 */
    ret |= get_irq_from_ts_node(devid, tsid, &irq_request[3], TS_TRIGGER_IRQ3_INDEX, 1); /* vf 3 */
    ret |= get_irq_from_ts_node(devid, tsid, &irq_request[4], TS_TRIGGER_IRQ4_INDEX, 1); /* vf 4 */
    if (ret != 0) {
        return ret;
    }

    irq[0] = TS_TRIGGER_HWIRQ0; /* pf(vf 0) */
    irq[1] = TS_TRIGGER_HWIRQ1; /* vf 1 */
    irq[2] = TS_TRIGGER_HWIRQ2; /* vf 2 */
    irq[3] = TS_TRIGGER_HWIRQ3; /* vf 3 */
    irq[4] = TS_TRIGGER_HWIRQ4; /* vf 4 */
    TSDRV_PRINT_INFO("Get trigger irq ok. (devid=%u; tsid=%u)\n", devid, tsid);
    return 0;
}
#endif

u32 tsdrv_get_mailbox_data_ack_irq_num(void)
{
    return 0;
}

int tsdrv_get_mailbox_data_ack_irq(u32 devid, u32 tsid, u32 *irq)
{
    return -EOPNOTSUPP;
}

int tsdrv_get_mailbox_data_ack_hwirq(u32 irq, u32 *hwirq)
{
    return -EOPNOTSUPP;
}

int tsdrv_get_func_cq_irq(u32 devid, u32 tsid, u32 *irq)
{
    if (!acpi_disabled) {
        return get_irq_from_acpi(devid, tsid, irq, TS_FUNC_CQ_IRQ_INDEX, TS_FUNC_CQ_IRQ_NUM);
    } else {
        return get_irq_from_ts_node(devid, tsid, irq, TS_FUNC_CQ_IRQ_INDEX, TS_FUNC_CQ_IRQ_NUM);
    }
}

int tsdrv_get_func_cq_hwirq(u32 irq, u32 *hwirq)
{
    /* LPI irqs return offset. SPI irqs return hardware irqs. */
#ifdef CFG_SOC_PLATFORM_MINIV3
    *hwirq = TS_FUNC_CQ_HWIRQ;
    return 0;
#else
    return tsdrv_get_hwirq_from_irq(irq, hwirq);
#endif
}

u32 tsdrv_get_disp_nfe_irq_num(void)
{
    return 0;
}

int tsdrv_get_disp_nfe_irq(u32 devid, u32 tsid, u32 *irq)
{
    return -EOPNOTSUPP;
}

int tsdrv_get_ts_irq_base(u32 devid, u32 tsid, u32 *irq)
{
    if (!acpi_disabled) {
        return get_irq_from_acpi(devid, tsid, irq, 0, 1);
    } else {
        return get_irq_from_ts_node(devid, tsid, irq, 0, 1);
    }
}

int tsdrv_get_numa_num_per_dev(void)
{
    return NUMA_NUM_PER_DEVICE;
}

int tsdrv_get_board_slot_id(u32 devid, u32 *board_id, u32 *slot_id)
{
    *board_id = 0;
    *slot_id = 0;
    return 0;
}

int tsdrv_get_partial_good(u32 devid, u32 *enable)
{
    struct platform_device *pdev = NULL;
    struct device_node *node = NULL;
    int ret;

    if (!acpi_disabled) {
        pdev = tsdrv_get_platform_device(devid);
        if (pdev == NULL) {
            TSDRV_PRINT_ERR("Get platform device failed. (devid=%u)\n", devid);
            return -ENODEV;
        }

        ret = device_property_read_u32(&pdev->dev, "aicpu_partial_good_enable", enable);
    } else {
        node = of_find_compatible_node(NULL, NULL, "hisilicon,ascend610");
        if (node == NULL) {
            TSDRV_PRINT_ERR("failed to find hisilicon,sysctrl node\n");
            return -EINVAL;
        }

        ret = of_property_read_u32_array(node, "aicpu_partial_good_enable", enable, 1);
    }

    if (ret != 0) {
        TSDRV_PRINT_WARN("Get aicpu partial good enable failed. (ret=%d)\n", ret);
        return -EINVAL;
    }

    TSDRV_PRINT_INFO("Get aicpu partial good enable. (enable=%u). \n", *enable);
    return 0;
}

