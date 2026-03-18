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
#include <linux/msi.h>
#include <linux/gpio.h>
#include <linux/acpi.h>
#include <linux/of_irq.h>

#include "devdrv_platform.h"
#include "tsdrv_log.h"
#include "tsdrv_pdata.h"

#define TSDRV_MAX_TS_NUM 1
#ifdef CFG_SOC_PLATFORM_CLOUD_V2
#define NUMA_NUM_PER_DEVICE  0x2
#define STARS_FFTS_ID 2

#define STARS_CORE_NUM 4
#else
#define NUMA_NUM_PER_DEVICE  0x4 /* divide new ts ddr node, this macro used to identify old and new BIOS */
#endif

#define SLOT_GPIO_SIZE  4
#define BOARD_GPIO_SIZE 14

#define AK_CLOUD_BOARDID          0x0020
#define AK_CLOUD_BOARDID_AG_280T  0x0270
#define AK_CLOUD_BOARDID_MASK     0xFFF0

#define CLOUD_MODULE_BOARID_SHIFT 9
#define CLOUD_MODULE_BOARDID_MASK 0x1

enum acpi_addr_index {
    GIC_BASE_INDEX = 0,
    TS_SUBSYSCTL_INDEX = 1,
    TS_DOORBELL_INDEX = 2,
    TS_SRAM_INDEX = 3,    /* ts addr end. */
    DISPATCH_INDEX = 4,
    SYSCTL_INDEX = 5,
    STARS_INDEX = 7,      /* adapt for ascend910B dts */
    ADDR_MAX_INDEX
};

static const char *acpi_addr_name[ADDR_MAX_INDEX] = {
    "GICV3_base",
    "TS_SUBSYSCTL_base",
    "TS_DOORBELL_base",
    "TS_SRAM_base",
    "Dispatch_base",
    "SYSCTL_base",
    "NULL",
    "STARS_base",
};

static const char *acpi_size_name[ADDR_MAX_INDEX] = {
    "GICV3_lenth",
    "TS_SUBSYSCTL_lenth",
    "TS_DOORBELL_lenth",
    "TS_SRAM_lenth",
    "Dispatch_lenth",
    "SYSCTL_lenth",
    "NULL",
    "STARS_lenth",
};

#ifdef CFG_SOC_PLATFORM_CLOUD_V2
enum dts_ts_irq_index {
    TS_MAILBOX_ACK_IRQ_INDEX = 1,
    TS_FUNC_CQ_IRQ_INDEX = 2,
    TS_MAILBOX_DATA_ACK_IRQ_INDEX = 3,
    TS_DISP_NFE_IRQ_INDEX = 4,
    TS_CQ_UPDATE_IRQ_INDEX = 6,
    TS_IRQ_MAX_INDEX
};

#define TS_CQ_UPDATE_IRQ_NUM 1
#define TS_MAILBOX_ACK_IRQ_NUM 1
#define TS_FUNC_CQ_IRQ_NUM 1
#define TS_MAILBOX_DATA_ACK_IRQ_NUM 1
#define TS_DISP_NFE_IRQ_NUM 1

#else
enum dts_ts_irq_index {
    TS_CQ_UPDATE_IRQ_INDEX = 0,
    TS_FUNC_CQ_IRQ_INDEX = 31,
    TS_MAILBOX_ACK_IRQ_INDEX = 32,
    TS_MAILBOX_DATA_ACK_IRQ_INDEX = 33,
    TS_DISP_NFE_IRQ_INDEX = 34,
    TS_IRQ_MAX_INDEX
};

#define TS_CQ_UPDATE_IRQ_NUM 31
#define TS_MAILBOX_ACK_IRQ_NUM 1
#define TS_FUNC_CQ_IRQ_NUM 1
#define TS_MAILBOX_DATA_ACK_IRQ_NUM 1
#define TS_DISP_NFE_IRQ_NUM 1

#endif

static int get_addr_from_acpi(u32 devid, phys_addr_t *paddr, size_t *size, int addr_index)
{
    int ret;
    u64 read_size;
    struct platform_device *pdev = NULL;

    if ((addr_index < 0) || (addr_index >= (int)ADDR_MAX_INDEX)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR(" Addr_index is invalid. (index=%d).\n", addr_index);
        return -EINVAL;
#endif
    }

    pdev = tsdrv_get_platform_device(devid);
    if (pdev == NULL) {
        TSDRV_PRINT_ERR("Get platform device failed. (devid=%u)\n", devid);
        return -ENODEV;
    }

    ret = device_property_read_u64(&pdev->dev, acpi_addr_name[addr_index], paddr);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Read device property failed. (ret=%d; index=%d)\n", ret, addr_index);
        return ret;
    }

    ret = device_property_read_u64(&pdev->dev, acpi_size_name[addr_index], &read_size);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Read device property failed. (ret=%d; index=%d)\n", ret,  addr_index);
        return ret;
    }

    *size = (size_t)read_size;
    TSDRV_PRINT_DEBUG("Get addr info. (devid=%u; name=%s; addr=%pK; size=%llx)\n",
        devid, acpi_addr_name[addr_index], (void *)*paddr, read_size);
    return 0;
}

static int get_addr_from_dts(u32 devid, phys_addr_t *paddr, size_t *size, int addr_index)
{
    struct resource *res = NULL;
    struct platform_device *pdev = NULL;

    pdev = tsdrv_get_platform_device(devid);
    if (pdev == NULL) {
        TSDRV_PRINT_ERR("Get platform device failed. (devid=%u)\n", devid);
        return -ENODEV;
    }

    res = platform_get_resource(pdev, IORESOURCE_MEM, (u32)addr_index);
    if (res == NULL) {
        TSDRV_PRINT_ERR("platform_get_resource failed.\n");
        return -EINVAL;
    }

    *paddr = res->start;
    *size = (size_t)(res->end + 1 - res->start);
    TSDRV_PRINT_DEBUG("Get addr info. (devid=%u; name=%s; addr=0x%pK; size=%llx)\n",
        devid, acpi_addr_name[addr_index], (void *)*paddr, (u64)*size);
    return 0;
}

static int get_addr_by_index(u32 devid, phys_addr_t *paddr, size_t *size, int addr_index)
{
    if (!acpi_disabled) {
        return get_addr_from_acpi(devid, paddr, size, addr_index);
    } else {
        return get_addr_from_dts(devid, paddr, size, addr_index);
    }
}

int tsdrv_get_gicv3_addr(u32 devid, phys_addr_t *paddr, size_t *size)
{
    return get_addr_by_index(devid, paddr, size, (int)GIC_BASE_INDEX);
}

int tsdrv_get_dispatch_addr(u32 devid, phys_addr_t *paddr, size_t *size)
{
    return get_addr_by_index(devid, paddr, size, (int)DISPATCH_INDEX);
}

int tsdrv_get_sysctrl_addr(u32 devid, phys_addr_t *paddr, size_t *size)
{
    return get_addr_by_index(devid, paddr, size, (int)SYSCTL_INDEX);
}

int tsdrv_get_ts_stars_addr(u32 devid, u32 tsid, phys_addr_t *paddr, size_t *size)
{
#ifdef CFG_SOC_PLATFORM_CLOUD_V2
    return get_addr_by_index(devid, paddr, size, (int)STARS_INDEX);
#else
    *paddr = 0;
    *size = 0;
    return 0;
#endif
}

int tsdrv_get_ts_stars_rtsq_addr(u32 devid, u32 tsid, phys_addr_t *paddr, size_t *size)
{
#ifdef CFG_SOC_PLATFORM_CLOUD_V2
    int ret;

    ret = get_addr_by_index(devid, paddr, size, (int)STARS_INDEX);
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

int tsdrv_get_ts_sysctrl_addr(u32 devid, u32 tsid, phys_addr_t *paddr, size_t *size)
{
    return get_addr_by_index(devid, paddr, size, (int)TS_SUBSYSCTL_INDEX);
}

int tsdrv_get_ts_doorbell_addr(u32 devid, u32 tsid, phys_addr_t *paddr, size_t *size)
{
#ifdef CFG_SOC_PLATFORM_CLOUD_V2
    int ret;

    ret = get_addr_by_index(devid, paddr, size, (int)TS_DOORBELL_INDEX);
    if (ret != 0) {
        return ret;
    }

    /* CLOUD_V2 doorbell size fix. */
    *size = DEVDRV_TS_DOORBELL_SIZE;
    return 0;
#else
    return get_addr_from_acpi(devid, paddr, size, (int)TS_DOORBELL_INDEX);
#endif
}

int tsdrv_get_ts_sram_addr(u32 devid, u32 tsid, phys_addr_t *paddr, size_t *size)
{
    return get_addr_by_index(devid, paddr, size, (int)TS_SRAM_INDEX);
}

int tsdrv_get_tsensor_shm_addr(u32 devid, u32 tsid, phys_addr_t *paddr, size_t *size)
{
    *paddr = 0;
    *size = 0;
    return 0;
}

#ifdef CFG_SOC_PLATFORM_CLOUD_V2
#define ASIC_MIN_PG_TAISHAN_CORE_NUM 2
#define ASIC_1DIE_GIC_INIT_NUM 8
#define FPGA_2DIE_TAISHAN_CORE_NUM 4

#define FPGA_2DIE_GIC_INIT_NUM_DIE0 10
#define FPGA_2DIE_GIC_INIT_NUM_DIE1 16
int tsdrv_get_fftscpu_id(u32 devid)
{
    u32 cpu_num;

#ifndef TSDRV_UT
    cpu_num = num_possible_cpus();  // 2die, cpu num is 16; 2p(0p cpu num is 8)

    TSDRV_PRINT_INFO("devid(%u), cpu num(%u)\n", devid, cpu_num);
    if (cpu_num == ASIC_MIN_PG_TAISHAN_CORE_NUM) {
        cpu_num = ASIC_1DIE_GIC_INIT_NUM;
        TSDRV_PRINT_INFO("devid(%u), cpu num(%u)\n", devid, cpu_num);
    }

    if (cpu_num == FPGA_2DIE_TAISHAN_CORE_NUM) {
        if (devid == 0) {
            cpu_num = FPGA_2DIE_GIC_INIT_NUM_DIE0;
        } else {
            cpu_num = FPGA_2DIE_GIC_INIT_NUM_DIE1;
        }
        TSDRV_PRINT_INFO("devid(%u), cpu num(%u)\n", devid, cpu_num);
    }
#endif
    return (int)(cpu_num + STARS_FFTS_ID + STARS_CORE_NUM * devid);
}
#else
int tsdrv_get_fftscpu_id(u32 devid)
{
    return -EINVAL;
}
#endif

u32 tsdrv_get_ts_num(u32 devid)
{
    return TSDRV_MAX_TS_NUM;
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
        return ret;
    }

    if (*chip_id >= MAX_CHIP_NUM) {
        TSDRV_PRINT_ERR("Chip id is invalid. (chip_id=%u)\n", *chip_id);
        return -EINVAL;
    }

    return 0;
}

static void write_lpi_msg(struct msi_desc *desc, struct msi_msg *msg)
{
}

static void devdrv_free_lpi_irqs(void *data)
{
    struct device *dev = data;
    platform_msi_domain_free_irqs(dev);
}

int tsdrv_alloc_irqs(u32 devid)
{
    int ret;
    struct platform_device *pdev = NULL;

    if (acpi_disabled) {
        return 0;
    }

    pdev = tsdrv_get_platform_device(devid);
    if (pdev == NULL) {
        TSDRV_PRINT_ERR("Get platform device failed. (devid=%u)\n", devid);
        return -ENODEV;
    }

    ret = platform_msi_domain_alloc_irqs(&pdev->dev, DEVDRV_LPI_INT_NUM, write_lpi_msg);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Alloc irqs failed. (ret=%d)\n", ret);
#endif
        return ret;
    }

    (void)devm_add_action(&pdev->dev, devdrv_free_lpi_irqs, &pdev->dev);
    return 0;
}

static void tsdrv_parse_ts_irq(struct platform_device *pdev, u32 *irq_map, u32 irq_num)
{
    struct msi_desc *desc = NULL;

    for_each_msi_entry(desc, &pdev->dev) {
        if (desc->platform.msi_index >= irq_num) {
            break;
        }
        irq_map[desc->platform.msi_index] = desc->irq;
        TSDRV_PRINT_DEBUG("Get msi irq. (index=%d; irq=%u)\n",
            desc->platform.msi_index, desc->irq);
    }
}

#ifndef TSDRV_UT
static void tsdrv_parse_ts_irq_from_dts(struct platform_device *pdev, u32 *irq_map, u32 irq_num)
{
    struct device_node *node = pdev->dev.of_node;
    struct device_node *son = NULL;
    u32 tsid;
    u32 i;

    for (tsid = 0; ((son = of_get_next_child(node, son)) != NULL) && (tsid < DEVDRV_MAX_TS_NUM); tsid++) {
        for (i = 0; i < irq_num; i++) {
            irq_map[i] = irq_of_parse_and_map(son, i);
            if (irq_map[i] == 0) {
                TSDRV_PRINT_ERR("irq parse fail, tsid=%u, index=%u.\n", tsid, i);
                break;
            }
            TSDRV_PRINT_FPGA(" tsid = %u, irq_map[%u] = %u.\n", tsid, i, irq_map[i]);
        }
    }
}
#endif

static int get_msi_irqs(u32 devid, u32 *irq, u32 base, u32 irq_num)
{
    u32 i;
    u32 irq_map[DEVDRV_LPI_INT_NUM] = { 0 };
    struct platform_device *pdev = NULL;

    pdev = tsdrv_get_platform_device(devid);
    if (pdev == NULL) {
        TSDRV_PRINT_ERR("Get platform device failed. (devid=%u)\n", devid);
        return -ENODEV;
    }

    if (!acpi_disabled) {
        tsdrv_parse_ts_irq(pdev, irq_map, DEVDRV_LPI_INT_NUM);
    } else {
#ifndef TSDRV_UT
        tsdrv_parse_ts_irq_from_dts(pdev, irq_map, DEVDRV_LPI_INT_NUM);
#endif
    }

    for (i = 0; i < irq_num; i++) {
        if ((base + i) < DEVDRV_LPI_INT_NUM) {
            irq[i] = irq_map[base + i];
            TSDRV_PRINT_DEBUG("Get irq. (index=%u; irq=%u)\n", i, irq_map[base + i]);
        }
    }
    return 0;
}

u32 tsdrv_get_cq_update_irq_num(void)
{
    return TS_CQ_UPDATE_IRQ_NUM;
}

int tsdrv_get_cq_update_irq(u32 devid, u32 tsid, u32 *irq)
{
    return get_msi_irqs(devid, irq, TS_CQ_UPDATE_IRQ_INDEX, TS_CQ_UPDATE_IRQ_NUM);
}

int tsdrv_get_cq_update_hwirq(u32 *irq, u32 *hwirq)
{
    int i;

    /* LPI irqs return offset. */
    for (i = 0; i < TS_CQ_UPDATE_IRQ_NUM; i++) {
        hwirq[i] = (u32)i;
    }

    return 0;
}

int tsdrv_get_mailbox_ack_irq(u32 devid, u32 tsid, u32 *irq)
{
    return get_msi_irqs(devid, irq,
        TS_MAILBOX_ACK_IRQ_INDEX, TS_MAILBOX_ACK_IRQ_NUM);
}

int tsdrv_get_mailbox_ack_hwirq(u32 irq, u32 *hwirq)
{
    /* LPI irqs return offset. */
    *hwirq = TS_MAILBOX_ACK_IRQ_INDEX;
    return 0;
}

u32 tsdrv_get_mailbox_data_ack_irq_num(void)
{
    return TS_MAILBOX_DATA_ACK_IRQ_NUM;
}

int tsdrv_get_mailbox_data_ack_irq(u32 devid, u32 tsid, u32 *irq)
{
    return get_msi_irqs(devid, irq,
        TS_MAILBOX_DATA_ACK_IRQ_INDEX, TS_MAILBOX_DATA_ACK_IRQ_NUM);
}

int tsdrv_get_mailbox_data_ack_hwirq(u32 irq, u32 *hwirq)
{
    /* LPI irqs return offset. */
    *hwirq = TS_MAILBOX_DATA_ACK_IRQ_INDEX;
    return 0;
}

int tsdrv_get_func_cq_irq(u32 devid, u32 tsid, u32 *irq)
{
    return get_msi_irqs(devid, irq,
        TS_FUNC_CQ_IRQ_INDEX, TS_FUNC_CQ_IRQ_NUM);
}

int tsdrv_get_func_cq_hwirq(u32 irq, u32 *hwirq)
{
    /* LPI irqs return offset. */
    *hwirq = TS_FUNC_CQ_IRQ_INDEX;
    return 0;
}

u32 tsdrv_get_disp_nfe_irq_num(void)
{
    return TS_DISP_NFE_IRQ_NUM;
}

int tsdrv_get_disp_nfe_irq(u32 devid, u32 tsid, u32 *irq)
{
    return get_msi_irqs(devid, irq,
        TS_DISP_NFE_IRQ_INDEX, TS_DISP_NFE_IRQ_NUM);
}

int tsdrv_get_ts_irq_base(u32 devid, u32 tsid, u32 *irq)
{
    return get_msi_irqs(devid, irq, 0, 1);
}

int tsdrv_get_numa_num_per_dev(void)
{
    return NUMA_NUM_PER_DEVICE;
}

#ifndef CFG_SOC_PLATFORM_CLOUD_V2
static int get_board_id(struct platform_device *pdev, u32 *board_id)
{
    int i;
    int ret;
    u32 value;
    u32 id = 0;
    struct gpio_desc *board_arry[BOARD_GPIO_SIZE] = {0};

    for (i = 0; i < BOARD_GPIO_SIZE; i++) {
        board_arry[i] = gpiod_get_index(&pdev->dev, "board", (u32)i, GPIOD_IN);
        ret = gpiod_direction_input(board_arry[i]);
        if (ret != 0) {
            TSDRV_PRINT_WARN("Set gpio direction failed. (index=%d; ret=%d)\n",
                i, ret);
            return ret;
        }
        value = (u32)gpiod_get_value(board_arry[i]);
        id |= (value << i);
        gpiod_put(board_arry[i]);
    }

    *board_id = id;
    return 0;
}

static int get_slot_id(struct platform_device *pdev, u32 *slot_id)
{
    int i;
    int ret;
    u32 value;
    u32 id = 0;
    struct gpio_desc *slot_arry[SLOT_GPIO_SIZE] = {0};

    for (i = 0; i < SLOT_GPIO_SIZE; i++) {
        slot_arry[i] = gpiod_get_index(&pdev->dev, "slot", (u32)i, GPIOD_IN);
        ret = gpiod_direction_input(slot_arry[i]);
        if (ret != 0) {
            TSDRV_PRINT_WARN("Set gpio direction failed. (index=%d; ret=%d)\n", i, ret);
            return ret;
        }
        value = (u32)gpiod_get_value(slot_arry[i]);
        id |= (value << i);
        gpiod_put(slot_arry[i]);
    }

    *slot_id = id;
    return 0;
}

#endif

int tsdrv_get_board_slot_id(u32 devid, u32 *board_id, u32 *slot_id)
{
#ifndef CFG_SOC_PLATFORM_CLOUD_V2

    bool is_cloud_arm_server = false;
    u32 slotid = 0;
    u32 boardid = 0;
    int ret;
    struct platform_device *pdev = NULL;

    pdev = tsdrv_get_platform_device(devid);
    if (pdev == NULL) {
        TSDRV_PRINT_ERR("Get platform device failed. (devid=%u)\n", devid);
        return -ENODEV;
    }

    ret = get_board_id(pdev, &boardid);
    if (ret != 0) {
        return 0;
    }

    *board_id = boardid;
    /* check whether board type is AI SERVER type */
    if ((boardid & 0x3000) == 0x0) {
        boardid = boardid & AK_CLOUD_BOARDID_MASK;
        /* arm server board id rule: GPIO[75:73]=001 */
        is_cloud_arm_server = (((boardid >> CLOUD_MODULE_BOARID_SHIFT) == CLOUD_MODULE_BOARDID_MASK) ||
                               (boardid == AK_CLOUD_BOARDID));

        ret = get_slot_id(pdev, &slotid);
        if (ret != 0) {
            return 0;
        }

        /* board type is ARM AI SERVER type */
        if (is_cloud_arm_server) {
            /* A+K slot id low 2 bits reverse */
            *slot_id = (slotid & 0x4) + ((slotid ^ 0x3) & 0x3);
        } else { // board type is X86 AI SERVER type
            *slot_id = (slotid & 0x7);  // get bit0  bit1  bit2
        }
    } else {
        TSDRV_PRINT_INFO("it's not AI server ! not support slot id \n");
        *slot_id = 0xff;
    }
#else
    *board_id = 0;
    *slot_id = 0xff;
#endif
    return 0;
}

int tsdrv_get_partial_good(u32 devid, u32 *enable)
{
    *enable = 0;
    return 0;
}
