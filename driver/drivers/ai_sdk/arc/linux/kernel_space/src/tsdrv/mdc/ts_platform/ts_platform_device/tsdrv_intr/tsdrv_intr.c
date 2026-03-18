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

#include <linux/types.h>
#include <linux/module.h>
#include <linux/delay.h>
#ifndef AOS_LLVM_BUILD
#include <linux/irqchip/arm-gic-v3.h>
#endif
#include "tsdrv_intr.h"
#include "tsdrv_intr_comm.h"
#include "tsdrv_pdata.h"
#include "tsdrv_log.h"

STATIC struct tsdrv_intr_info g_intr_info[DEVDRV_MAX_DAVINCI_NUM];

STATIC struct tsdrv_intr_info *tsdrv_get_intr_info(u32 devid)
{
    return &g_intr_info[devid];
}

STATIC int tsdrv_intr_info_init(u32 devid)
{
    int ret;
    size_t gicv3_size = 0;
    phys_addr_t gicv3_paddr = 0;
    struct tsdrv_intr_info *intr_info = NULL;

    intr_info = tsdrv_get_intr_info(devid);
    ret = tsdrv_get_gicv3_addr(devid, &gicv3_paddr, &gicv3_size);
    if ((ret != 0) || (gicv3_paddr == 0) || (gicv3_size == 0)) {
        TSDRV_PRINT_ERR("Get gicv3 addr failed. (ret=%d, size=%lu)\n",
            ret, gicv3_size);
        return -EFAULT;
    }

    intr_info->gicv3_vbase = ioremap(gicv3_paddr, gicv3_size);
    if (intr_info->gicv3_vbase == NULL) {
        TSDRV_PRINT_ERR("Ioremap gicv3 paddr failed. (size=%lu)\n", gicv3_size);
        return -ENODEV;
    }

    intr_info->gicv3_size = gicv3_size;
    return 0;
}

STATIC void tsdrv_intr_info_destroy(u32 devid)
{
    struct tsdrv_intr_info *intr_info = NULL;

    intr_info = tsdrv_get_intr_info(devid);
    if (intr_info->gicv3_vbase != NULL) {
        iounmap(intr_info->gicv3_vbase);
        intr_info->gicv3_vbase = NULL;
        intr_info->gicv3_size = 0;
    }
}
#ifndef AOS_LLVM_BUILD
STATIC void tsdrv_irq_enable(struct tsdrv_intr_info *intr_info,
    struct tsdrv_intr_config *intr_config)
{
#ifndef TSDRV_UT
    u32 uirq; /* irq must be nonnegative. */
    u32 value;
    void __iomem *gicv3_base = NULL;
    void __iomem *enable_reg = NULL;

    uirq = intr_config->irq;
    gicv3_base = intr_info->gicv3_vbase;
    enable_reg = gicv3_base +
        (GICD_ISENABLER + (((u64)uirq >> GICD_ENABLE_REG_OFFSET1) << GICD_ENABLE_REG_OFFSET2));

    value = readl_relaxed(enable_reg);
    value |= (0x1UL << (uirq % IRQ_ENABLE_DIVISOR));
    writel_relaxed(value, enable_reg);
    return;
#endif
}

STATIC void tsdrv_irq_disable(struct tsdrv_intr_info *intr_info,
    struct tsdrv_intr_config *intr_config)
{
    u32 uirq; /* irq must be nonnegative. */
    u32 value;
    void __iomem *gicv3_base = NULL;
    void __iomem *disable_reg = NULL;

    uirq = intr_config->irq;
    gicv3_base = intr_info->gicv3_vbase;
    disable_reg = gicv3_base +
        (GICD_ICENABLER + (((u64)uirq >> GICD_DISABLE_REG_OFFSET1) << GICD_DISABLE_REG_OFFSET2));

    value = readl_relaxed(disable_reg);
    value |= (0x1UL << (uirq % IRQ_DISABLE_DIVISOR));
    writel_relaxed(value, disable_reg);
    return;
}

STATIC void tsdrv_gic_wait_for_rwp(struct tsdrv_intr_info *intr_info)
{
    u32 count = TSDRV_WAIT_GICD_RWP_TIMEOUT; /* timeout is 1s. */
    void __iomem *gicv3_base = NULL;
    void __iomem *ctrl_reg = NULL;

    gicv3_base = intr_info->gicv3_vbase;
    ctrl_reg = gicv3_base + GICD_CTLR;

    while (readl_relaxed(ctrl_reg) & GICD_CTLR_RWP) {
        count--;
        if (count == 0) {
            TSDRV_PRINT_ERR("RWP timeout, gone fishing.\n");
            return;
        }
        cpu_relax();
        udelay(1);
    };
}

STATIC int tsdrv_gicd_irq_config(struct tsdrv_intr_info *intr_info,
    struct tsdrv_intr_config *intr_config)
{
    u32 sclid;
    u32 uirq; /* irq must be nonnegative */
    u64 affinity;
    u32 cluster;
    u32 value;
    void __iomem *irouter_reg = NULL;
    void __iomem *icfgr_reg = NULL;
    void __iomem *gicv3_base = NULL;

    gicv3_base = intr_info->gicv3_vbase;
    uirq = intr_config->irq;
    cluster = tsdrv_get_cpu_cluster(intr_config->devid, intr_config->cpu_type);
    if (cluster == TSDRV_INVALID_CPU_CLUSTER) {
        TSDRV_PRINT_ERR("Get cpu cluster failed. (irq=%u; cpu_type=%u)\n",
            uirq, intr_config->cpu_type);
        return -EINVAL;
    }

    sclid = tsdrv_get_cpu_sclid(intr_config->devid);
    affinity = tsdrv_get_irq_affinity(intr_config->chipid, intr_config->dieid, sclid, cluster, intr_config->cpu_index);

    irouter_reg = gicv3_base + (GICD_IROUTER + ((u64)uirq << GICD_IROUTER_REG_OFFSET));
    writeq_relaxed(affinity, irouter_reg);

    TSDRV_PRINT_DEBUG("irq=%u; cluster=%u; sclid=%u; affinity=%llx; cpu=%u; reg=%llx; value=%llx\n",
        uirq, cluster, sclid, affinity, intr_config->cpu_index,
        GICD_IROUTER + ((u64)uirq << GICD_IROUTER_REG_OFFSET), readq_relaxed(irouter_reg));

    if (intr_config->irq_trigger_type != TSDRV_IRQ_NONE_TRIGGER) {
        icfgr_reg = gicv3_base +
            (GICD_ICFGR + (((u64)uirq >> GICD_ICFGR_REG_OFFSET1) << GICD_ICFGR_REG_OFFSET2));
        value = readl_relaxed(icfgr_reg);
        value |= ((u32)intr_config->irq_trigger_type << ((uirq % IRQ_DIVISOR) << IRQ_DIVISOR_OFFSET));
        writel_relaxed(value, icfgr_reg);
        TSDRV_PRINT_DEBUG("icfgr_reg=%llx; write_value=%x; icfgr_value=%x\n",
            (GICD_ICFGR + (((u64)uirq >> GICD_ICFGR_REG_OFFSET1) << GICD_ICFGR_REG_OFFSET2)),
            value, readl_relaxed(icfgr_reg));
    }

    return 0;
}

STATIC int tsdrv_set_single_intr(struct tsdrv_intr_info *intr_info,
    struct tsdrv_intr_config *intr_config)
{
    int ret;

    ret = tsdrv_gicd_irq_config(intr_info, intr_config);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Config gicd failed. (devid=%u; irq=%u)\n",
            intr_config->devid, intr_config->irq);
        return ret;
#endif
    }

    if (intr_config->enable == TSDRV_IRQ_ENABLE) {
        tsdrv_irq_enable(intr_info, intr_config);
    } else {
        tsdrv_irq_disable(intr_info, intr_config);
    }

    tsdrv_gic_wait_for_rwp(intr_info);
    return 0;
}
#else
STATIC int tsdrv_set_single_intr(struct tsdrv_intr_info *intr_info,
    struct tsdrv_intr_config *intr_config)
{
    if (intr_config->enable == TSDRV_IRQ_ENABLE) {
        enable_irq(intr_config->irq);
    } else {
        disable_irq(intr_config->irq);
    }
    return 0;
}
#endif

STATIC int tsdrv_set_intr_config(u32 devid, u32 chipid, u32 dieid)
{
    int ret;
    u32 irq_idx;
    struct tsdrv_intr_config *intr_config = NULL;
    struct tsdrv_intr_info *intr_info = NULL;

    intr_info = tsdrv_get_intr_info(devid);
    intr_config = tsdrv_get_intr_config(devid, chipid, dieid);

    for (irq_idx = 0; irq_idx < TSDRV_MAX_INTR_ROUTE_NUM; irq_idx++) {
        if (intr_config[irq_idx].irq_name == NULL) {
            break;
        }
        intr_config[irq_idx].devid = devid;
        intr_config[irq_idx].chipid = chipid;
        intr_config[irq_idx].dieid = dieid;

        ret = tsdrv_set_single_intr(intr_info, &intr_config[irq_idx]);
        if (ret != 0) {
#ifndef TSDRV_UT
            TSDRV_PRINT_ERR("Set intr config failed. (devid=%u; irq_idx=%u; irq=%u)\n",
                devid, irq_idx, intr_config[irq_idx].irq);
            return ret;
#endif
        }
    }
    return 0;
}

int tsdrv_intr_init(struct devdrv_info *dev_info)
{
    u32 devid = dev_info->dev_id;
    u32 chipid = dev_info->chip_id;
    u32 dieid = dev_info->die_id;
    int ret;

    if (devid >= DEVDRV_MAX_DAVINCI_NUM) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Dev id is invalid. (devid=%u)\n", devid);
        return -ENODEV;
#endif
    }
    /*
     * remap gicv3 and init cpu clusters
     * must be called first
     */
    ret = tsdrv_intr_info_init(devid);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Init intr info failed. (devid=%u)\n", devid);
        return ret;
#endif
    }

    ret = tsdrv_set_intr_config(devid, chipid, dieid);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Set intr config failed. (devid=%u)\n", devid);
        tsdrv_intr_info_destroy(devid);
        return ret;
#endif
    }

    /* unmap gicv3 */
    tsdrv_intr_info_destroy(devid);
    return 0;
}
