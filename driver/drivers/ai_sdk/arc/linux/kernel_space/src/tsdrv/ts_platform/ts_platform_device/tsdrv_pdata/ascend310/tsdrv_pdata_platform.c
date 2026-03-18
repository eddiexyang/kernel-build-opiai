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

#include "devdrv_platform.h"
#include "tsdrv_log.h"
#include "tsdrv_pdata.h"

#define TSDRV_MAX_TS_NUM 1
#define NUMA_NUM_PER_DEVICE 0x2

enum dts_addr_index {
    GIC_BASE_INDEX = 0,
    TS_SUBSYSCTL_INDEX = 1,
    TS_DOORBELL_INDEX = 2,
    TS_SRAM_INDEX = 3,
    DISPATCH_INDEX = 4,
    SYSCTL_INDEX = 5,
    TSENSOR_SHRAEMEN_INDEX = 11
};

enum dts_ts_irq_index {
    TS_CQ_UPDATE_IRQ_INDEX = 0,
    TS_FUNC_CQ_IRQ_INDEX = 31,
    TS_MAILBOX_ACK_IRQ_INDEX = 32,
    TS_MAILBOX_DATA_ACK_IRQ_INDEX = 33,
    TS_DISP_NFE_IRQ_INDEX = 34
};

#define TS_CQ_UPDATE_IRQ_NUM 31
#define TS_MAILBOX_ACK_IRQ_NUM 1
#define TS_FUNC_CQ_IRQ_NUM 1
#define TS_MAILBOX_DATA_ACK_IRQ_NUM 1
#define TS_DISP_NFE_IRQ_NUM 1

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

int tsdrv_get_tsensor_shm_addr(u32 devid, u32 tsid, phys_addr_t *paddr, size_t *size)
{
    return tsdrv_get_addr_by_index(devid, TSENSOR_SHRAEMEN_INDEX, paddr, size);
}

int tsdrv_get_ts_sysctrl_addr(u32 devid, u32 tsid, phys_addr_t *paddr, size_t *size)
{
    return tsdrv_get_addr_by_index(devid, TS_SUBSYSCTL_INDEX, paddr, size);
}

int tsdrv_get_ts_doorbell_addr(u32 devid, u32 tsid, phys_addr_t *paddr, size_t *size)
{
    return tsdrv_get_addr_by_index(devid, TS_DOORBELL_INDEX, paddr, size);
}

int tsdrv_get_ts_sram_addr(u32 devid, u32 tsid, phys_addr_t *paddr, size_t *size)
{
    return tsdrv_get_addr_by_index(devid, TS_SRAM_INDEX, paddr, size);
}

int tsdrv_get_ts_stars_addr(u32 devid, u32 tsid, phys_addr_t *paddr, size_t *size)
{
    *paddr = 0;
    *size = 0;
    return 0;
}

int tsdrv_get_ts_stars_rtsq_addr(u32 devid, u32 tsid, phys_addr_t *paddr, size_t *size)
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
    return TSDRV_MAX_TS_NUM;
}

int tsdrv_get_chip_id(struct platform_device *pdev, u32 *chip_id)
{
    *chip_id = CHIP0_ID;
    return 0;
}

int tsdrv_alloc_irqs(u32 devid)
{
    return 0;
}

static int get_irq_from_platform_device(u32 devid, u32 *irq, u32 base, u32 irq_num)
{
    u32 i;
    struct platform_device *pdev = NULL;

    pdev = tsdrv_get_platform_device(devid);
    if (pdev == NULL) {
        TSDRV_PRINT_ERR("Get platform device failed. (devid=%u)\n", devid);
        return -ENODEV;
    }

    for (i = 0; i < irq_num; i++) {
        irq[i] = platform_get_irq(pdev, (base + i));
        TSDRV_PRINT_DEBUG("Get irq. (index=%u; irq=%u)\n", base + i, irq[i]);
    }

    return 0;
}

u32 tsdrv_get_cq_update_irq_num(void)
{
    return TS_CQ_UPDATE_IRQ_NUM;
}

int tsdrv_get_cq_update_irq(u32 devid, u32 tsid, u32 *irq)
{
    return get_irq_from_platform_device(devid, irq,
        TS_CQ_UPDATE_IRQ_INDEX, TS_CQ_UPDATE_IRQ_NUM);
}

int tsdrv_get_cq_update_hwirq(u32 *irq, u32 *hwirq)
{
    int i;

    for (i = 0; i < TS_CQ_UPDATE_IRQ_NUM; i++) {
        if (tsdrv_get_hwirq_from_irq(irq[i], &hwirq[i]) != 0) {
            return -EINVAL;
        }
    }

    return 0;
}

int tsdrv_get_func_cq_irq(u32 devid, u32 tsid, u32 *irq)
{
    return get_irq_from_platform_device(devid, irq,
        TS_FUNC_CQ_IRQ_INDEX, TS_FUNC_CQ_IRQ_NUM);
}

int tsdrv_get_func_cq_hwirq(u32 irq, u32 *hwirq)
{
    return tsdrv_get_hwirq_from_irq(irq, hwirq);
}

int tsdrv_get_mailbox_ack_irq(u32 devid, u32 tsid, u32 *irq)
{
    return get_irq_from_platform_device(devid, irq,
        TS_MAILBOX_ACK_IRQ_INDEX, TS_MAILBOX_ACK_IRQ_NUM);
}

int tsdrv_get_mailbox_ack_hwirq(u32 irq, u32 *hwirq)
{
    return tsdrv_get_hwirq_from_irq(irq, hwirq);
}

u32 tsdrv_get_mailbox_data_ack_irq_num(void)
{
    return TS_MAILBOX_DATA_ACK_IRQ_NUM;
}

int tsdrv_get_mailbox_data_ack_irq(u32 devid, u32 tsid, u32 *irq)
{
    return get_irq_from_platform_device(devid, irq,
        TS_MAILBOX_DATA_ACK_IRQ_INDEX, TS_MAILBOX_DATA_ACK_IRQ_NUM);
}

int tsdrv_get_mailbox_data_ack_hwirq(u32 irq, u32 *hwirq)
{
    return tsdrv_get_hwirq_from_irq(irq, hwirq);
}

u32 tsdrv_get_disp_nfe_irq_num(void)
{
    return TS_DISP_NFE_IRQ_NUM;
}

int tsdrv_get_disp_nfe_irq(u32 devid, u32 tsid, u32 *irq)
{
    return get_irq_from_platform_device(devid, irq,
        TS_DISP_NFE_IRQ_INDEX, TS_DISP_NFE_IRQ_NUM);
}

int tsdrv_get_ts_irq_base(u32 devid, u32 tsid, u32 *irq)
{
    return get_irq_from_platform_device(devid, irq, 0, 1);
}

int tsdrv_get_board_slot_id(u32 devid, u32 *board_id, u32 *slot_id)
{
#define BOARDID_LENGTH 4
    struct device_node *node = NULL;
    u32 boardid[BOARDID_LENGTH] = {0};
    u32 shift[BOARDID_LENGTH] = {1000, 100, 10, 1};
    int ret;
    int i;

    node = of_find_compatible_node(NULL, NULL, "hisilicon,mini");
    if (node == NULL) {
        TSDRV_PRINT_ERR("Failed to find root node.\n");
        return -EINVAL;
    }

    ret = of_property_read_u32_array(node, "hisi,boardid", boardid, BOARDID_LENGTH);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Get board id failed. (ret=%d)\n", ret);
        return ret;
    }

    *board_id = 0;
    for (i = 0; i < BOARDID_LENGTH; i++) {
        *board_id += boardid[i] * shift[i];
    }
    *slot_id = 0;
    return 0;
}

int tsdrv_get_numa_num_per_dev(void)
{
    return NUMA_NUM_PER_DEVICE;
}

int tsdrv_get_partial_good(u32 devid, u32 *enable)
{
    *enable = 0;
    return 0;
}
