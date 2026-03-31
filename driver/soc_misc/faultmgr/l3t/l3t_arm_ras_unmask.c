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

#include <linux/io.h>
#include <linux/sizes.h>
#include <linux/slab.h>
#include <linux/workqueue.h>

#include "soc_misc_init.h"
#include "drv_ras_common.h"
#include "soc_misc_spec.h"
#include "soc_misc_dms_def.h"
#include "l3t_arm_ras_unmask.h"

#define RAS_L3T_UNMASK_REMAP_SIZE (0x4)
#define L3T_NUM_MAX               (4)

#ifdef CFG_FEATURE_ENTIRETY_UNMASK
struct ras_l3t_unmask_hw_info {
    unsigned long long reg_addr;
    unsigned long long umask_offset;
    unsigned int unmask_val;
};

STATIC struct ras_l3t_unmask_hw_info g_l3t_unmask_hw_info[RAS_L3T_NUM_MAX] = {
    { RAS_L3T0_BASE_ADDR, RAS_L3T0_UNMASK_OFFSET, RAS_L3T0_UNMASK_VAL},
    { RAS_L3T1_BASE_ADDR, RAS_L3T1_UNMASK_OFFSET, RAS_L3T1_UNMASK_VAL},
};
#endif

struct unmask_work {
    u32 dev_id;
    struct delayed_work unmask_irq_work;
    unsigned char  work_data[SOC_UNMASK_IRQ_WORK_DATA];
};

STATIC struct unmask_work g_l3t_unmask_work[SOC_MISC_DEVICE_NUM_MAX];

STATIC void write_l3t_reg32(unsigned int dev_id, unsigned long long reg_phy_addr, unsigned int value)
{
#ifdef CFG_FEATURE_ENTIRETY_UNMASK
    (void)ras_int_unmask_reg_write(reg_phy_addr, value, value);
#else
    void __iomem *reg_virt_addr = NULL;
    unsigned int unmask_val = value;

    reg_virt_addr = ioremap(reg_phy_addr, RAS_L3T_UNMASK_REMAP_SIZE);
    if (reg_virt_addr == NULL) {
        soc_misc_drv_err("Failed to ioremap l3t unmask irq register. (devid=%u)\n", dev_id);
        return;
    }

    unmask_val = value;
    writel(unmask_val, (volatile void *)reg_virt_addr);
    iounmap(reg_virt_addr);
    reg_virt_addr = NULL;
#endif
    soc_misc_drv_event("Unmask l3t local ras irq success. (devid=%u)\n", dev_id);
}

#ifdef CFG_FEATURE_ENTIRETY_UNMASK
STATIC void l3t_fault_unmask_multi_type(unsigned int die_id,
    unsigned long long reg_addr, unsigned int reg_val)
{
    unsigned int i;

    for (i = 0; i < RAS_INT_TYPE_NUM_MAX; i++) {
        if ((RAS_INT_TYPE_ENABLE_MASK & (0x01 << i)) != 0) {
            write_l3t_reg32(die_id, reg_addr + (i * RAS_INT_TYPE_OFFSET), reg_val);
        }
    }
}
#endif

void l3t_fault_unmask_task(struct work_struct *work)
{
    struct unmask_work *s_dev = container_of(work, struct unmask_work, unmask_irq_work.work);
    unsigned long long reg_phy_addr;
    unsigned int i;

#ifndef CFG_FEATURE_ENTIRETY_UNMASK
    unsigned long long l3t_base_addr_arg[L3T_NUM_MAX] = {RAS_L3T0_BASE_ADDR, RAS_L3T1_BASE_ADDR,
        RAS_L3T2_BASE_ADDR, RAS_L3T3_BASE_ADDR};

    for (i = 0; i < RAS_L3T_NUM; i++) {
        if (s_dev->work_data[i] == 1) {
            reg_phy_addr = DIE_DEVICE_OFFSET * s_dev->dev_id + l3t_base_addr_arg[i] + RAS_L3T_UNMASK_OFFSET;
            write_l3t_reg32(s_dev->dev_id, reg_phy_addr, RAS_L3T_UNMASK_VAL);
            s_dev->work_data[i] = 0;
        }
    }
#else
    unsigned int die_id = 0;
    unsigned int chip_id = 0;
    int ret;

    ret = devdrv_get_chip_die_id(s_dev->dev_id, &chip_id, &die_id);
    if (ret != 0) {
        soc_misc_drv_err("Failed to get chip die id. (devid=%u)\n", s_dev->dev_id);
        return;
    }

    for (i = 0; i < RAS_L3T_NUM; i++) {
        if (s_dev->work_data[i] == 1) {
            reg_phy_addr = DIE_DEVICE_OFFSET * die_id + CHIP_DEVICE_OFFSET * chip_id +
                g_l3t_unmask_hw_info[i].reg_addr + g_l3t_unmask_hw_info[i].umask_offset;
            l3t_fault_unmask_multi_type(s_dev->dev_id, reg_phy_addr, g_l3t_unmask_hw_info[i].unmask_val);
            s_dev->work_data[i] = 0;
        }
    }
#endif
}

STATIC bool l3t_check_unmask(unsigned char l3t_id, unsigned device_id)
{
#ifdef CFG_FEATURE_ENTIRETY_UNMASK
    unsigned long long reg_phy_addr;
    void __iomem *reg_virt_addr = NULL;
    unsigned int unmask_val = g_l3t_unmask_hw_info[l3t_id].unmask_val;
    unsigned int die_id = 0;
    unsigned int chip_id = 0;
    unsigned int i;
    bool is_mask = false;
    int ret;

    ret = devdrv_get_chip_die_id(device_id, &chip_id, &die_id);
    if (ret != 0) {
        soc_misc_drv_err("Failed to get chip die id. (devid=%u)\n", device_id);
        return false;
    }

    reg_phy_addr = DIE_DEVICE_OFFSET * die_id + CHIP_DEVICE_OFFSET * chip_id +
        g_l3t_unmask_hw_info[l3t_id].reg_addr + g_l3t_unmask_hw_info[l3t_id].umask_offset;

    for (i = 0; i < RAS_INT_TYPE_NUM_MAX; i++) {
        if ((RAS_INT_TYPE_ENABLE_MASK & (0x01 << i)) != 0) {
            reg_virt_addr = ioremap(reg_phy_addr + (i * RAS_INT_TYPE_OFFSET), RAS_L3T_UNMASK_REMAP_SIZE);
            if (reg_virt_addr == NULL) {
                soc_misc_drv_err("Failed to ioremap l3t unmask irq register. (devid=%u)\n", die_id);
                return false;
            }

            unmask_val = readl(reg_virt_addr);
            unmask_val = unmask_val & g_l3t_unmask_hw_info[l3t_id].unmask_val;

            iounmap(reg_virt_addr);
            reg_virt_addr = NULL;
            is_mask = (unmask_val != g_l3t_unmask_hw_info[l3t_id].unmask_val) ? true : false;
            if (is_mask) {
                break;
            }
        }
    }

    return is_mask;
#else
    return true;
#endif
}

void l3t_relieve_suppresion(unsigned char device_id, unsigned char oem_sub_module)
{
    if (oem_sub_module < RAS_L3T_NUM) {
        if (l3t_check_unmask(oem_sub_module, device_id)) {
            g_l3t_unmask_work[device_id].work_data[oem_sub_module] = 1;
            /* 300000:5 minutes */
            (void)schedule_delayed_work(&g_l3t_unmask_work[device_id].unmask_irq_work, msecs_to_jiffies(300000));
        }
    }
}

void l3t_arm_ras_unmask_init(u32 dev_id)
{
    g_l3t_unmask_work[dev_id].dev_id = dev_id;
    INIT_DELAYED_WORK(&g_l3t_unmask_work[dev_id].unmask_irq_work, l3t_fault_unmask_task);
}

void l3t_arm_ras_unmask_uninit(u32 dev_id)
{
    if (g_l3t_unmask_work[dev_id].unmask_irq_work.work.func != NULL) {
        (void)cancel_delayed_work_sync(&g_l3t_unmask_work[dev_id].unmask_irq_work);
    }
}
