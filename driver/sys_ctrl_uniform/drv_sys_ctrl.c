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

#include <linux/err.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <linux/printk.h>

#define NOT_DEFINE_DTSNODE
#define DRV_NAME "sys-ctrl-drv"

struct rst_infos {
    phys_addr_t reg_phy_addr;   // 32bits register's phy addr
    void __iomem *reg_ioremap_addr; // vir addr for register's phy addr, must initilied as zero
    u32	mask_bits;          // 32bits register, set bits means these bits should be set value
    u32	mask_bits_value;    // set bits values only use for mask bits
    u32	delay_ns;           // set register value then delay ns, some registers will take effect after set it
    u32	sec_area;           // register is whether in security area, if it is, TEE must export the register's access
                            // privilege, and we use tee interface to access the register,
                            // 0 is not in security area(normal area), we could access it directly
};

struct rst_infos rst_infos_arr[] = {
    {0xa0120388, 0, 0b1111, 0b1111, 0, 0},      /* USB clock enable */
    {0xa012043c, 0, 0b111111111111, 0b111111111111, 0, 0},  /* USB controller unset */
    {0xa0120444, 0, 0b111, 0b111, 0, 0},        /* USB phy unset */
    {0xa0123604, 0, 0b10000, 0b10000, 0, 0},    /* not support integrity USB0 cachline read */
    {0xa0123614, 0, 0b10000, 0b10000, 0, 0},    /* not support integrity USB0 cachline write */
    {0xa0123624, 0, 0b10000, 0b10000, 0, 0},    /* not support integrity USB1 cachline read */
    {0xa0123634, 0, 0b10000, 0b10000, 0, 0},    /* not support integrity USB1 cachline write */
    {0xa0123644, 0, 0b10000, 0b10000, 0, 0},    /* not support integrity USB2 cachline read */
    {0xa0123654, 0, 0b10000, 0b10000, 0, 0},    /* not support integrity USB2 cachline write */
    {0xa0123664, 0, 0b10000, 0b10000, 0, 0},    /* not support integrity USB3 cachline read */
    {0xa0123674, 0, 0b10000, 0b10000, 0, 0},    /* not support integrity USB3 cachline write */
    {0xa0123690, 0, 0b100000000, 0b100000000, 0, 0},    /* USBC0~3 port_oca from OVRCUR */

    {0xa0120390, 0, 0b11111111111, 0b11111111111, 0, 0},    /* SATA clock enable */
    {0xa012044c, 0, 0b11111111111, 0b11111111111, 0, 0},    /* SATA controller unset */
    {0xa6000144, 0, 0x0800000, 0, 0, 0}, /* SATA close pm_clr_bug_en */
    {0xa60001c4, 0, 0x0800000, 0, 0, 0}, /* SATA close pm_clr_bug_en */
    {0xa6000244, 0, 0x0800000, 0, 0, 0}, /* SATA close pm_clr_bug_en */
    {0xa60002c4, 0, 0x0800000, 0, 0, 0}, /* SATA close pm_clr_bug_en */

    {0xc0120120, 0, 0b1, 0b0, 0, 0}     /* SC_SPI5_CTRL controller syn unset */
};

static int sys_ctrl_enable_rst(void)
{
    u32 reg_val;
    u32 set_val;
    u32 idx;
    u32 size;
    pr_info("in sys_ctrl_enable_rst\n");

    size = sizeof(rst_infos_arr) / sizeof(struct rst_infos);
    for (idx = 0; idx < size; ++idx) {
        if (rst_infos_arr[idx].sec_area == 0) {
            pr_info("the reg is in normal area\n");
            reg_val = ioread32(rst_infos_arr[idx].reg_ioremap_addr);
            pr_info("mask_bits: 0x%04x, ~mask_bits:0x%04x", rst_infos_arr[idx].mask_bits,
                ~rst_infos_arr[idx].mask_bits);
            set_val = reg_val&(~rst_infos_arr[idx].mask_bits);
            set_val |= rst_infos_arr[idx].mask_bits_value;
            iowrite32(set_val, rst_infos_arr[idx].reg_ioremap_addr);
            ndelay(rst_infos_arr[idx].delay_ns);
        } else if (rst_infos_arr[idx].sec_area == 1) {
            pr_info("the reg is in security area\n");
        } else {
            pr_warn("Not support\n");
            return -1;
        }
    }

    return 0;
}

static int sys_ctrl_probe(struct platform_device *pdev)
{
    u32 size;
    u32 idx;
    u32 i;
    int ret;
    size = sizeof(rst_infos_arr) / sizeof(struct rst_infos);

    for (idx = 0; idx < size; ++idx) {
        rst_infos_arr[idx].reg_ioremap_addr = ioremap(rst_infos_arr[idx].reg_phy_addr, sizeof(u32));
        if (rst_infos_arr[idx].reg_ioremap_addr == NULL) {
            goto EXIT;
        }
    }

    ret = sys_ctrl_enable_rst();
    if (ret != 0) {
        pr_warn("Some settings is not support, please check\n");
    }
    return 0;

EXIT:
    for (i = 0; i < idx; i++) {
        if (rst_infos_arr[i].reg_ioremap_addr != NULL) {
            iounmap(rst_infos_arr[i].reg_ioremap_addr);
            rst_infos_arr[i].reg_ioremap_addr = NULL;
        }
    }
    pr_err("Probe sys ctrl failed.\n");
    return -1;
}

static void sys_ctrl_remove(struct platform_device *pdev)
{
    u32 idx;
    u32 size;
    size = sizeof(rst_infos_arr) / sizeof(struct rst_infos);
    for (idx = 0; idx < size; ++idx) {
        if (rst_infos_arr[idx].reg_ioremap_addr != NULL) {
            iounmap(rst_infos_arr[idx].reg_ioremap_addr);
            rst_infos_arr[idx].reg_ioremap_addr = NULL;
        }
    }
    /* return removed */
}

static int sys_ctrl_suspend_noirq(struct device *dev)
{
    pr_info("in sys_ctrl_suspend_noirq\n");
    return 0;
}

static int sys_ctrl_resume_noirq(struct device *dev)
{
    pr_info("in sys_ctrl_suspend_noirq\n");
    return sys_ctrl_enable_rst();
}

static const struct dev_pm_ops sys_ctrl_dev_pm_ops = {
    SET_NOIRQ_SYSTEM_SLEEP_PM_OPS(sys_ctrl_suspend_noirq, sys_ctrl_resume_noirq)
};

#ifdef NOT_DEFINE_DTSNODE
static const struct of_device_id sys_ctrl_compatible_ids[] = {
    {.compatible = "sys-ctrl-1910B", .data = NULL},
    {},
};
MODULE_DEVICE_TABLE(of, sys_ctrl_compatible_ids);
#endif

static struct platform_driver	sys_ctrl_driver = {
    .probe = sys_ctrl_probe,
    .remove	= sys_ctrl_remove,
    .driver = {
        .name = DRV_NAME,
        .of_match_table = sys_ctrl_compatible_ids,
        .pm = &sys_ctrl_dev_pm_ops,
    },
};
#ifdef NOT_DEFINE_DTSNODE
static struct platform_device *sys_ctrl_pdev;
#endif

static int __init sys_ctrl_init(void)
{
    int ret = 0;
#ifdef NOT_DEFINE_DTSNODE
    /* Also could use dts node to register device auto */
    sys_ctrl_pdev = platform_device_alloc(DRV_NAME, -1);
    if (!sys_ctrl_pdev) {
        pr_err("Failed to allocate sys_ctrl device\n");
        return -1;
    }
    ret = platform_device_add(sys_ctrl_pdev);
    if (ret != 0) {
        pr_err("Failed to register sys_ctrl device\n");
        platform_device_put(sys_ctrl_pdev);
        return ret;
    }
#endif

    ret = platform_driver_register(&sys_ctrl_driver);
    if (ret != 0) {
        pr_err("Failed to register sys_ctrl driver\n");
#ifdef NOT_DEFINE_DTSNODE
        platform_device_unregister(sys_ctrl_pdev);
#endif
        return ret;
    }
    pr_info("in sys_ctrl_init\n");
    return 0;
}

static void __exit sys_ctrl_exit(void)
{
    platform_driver_unregister(&sys_ctrl_driver);
#ifdef NOT_DEFINE_DTSNODE
    platform_device_unregister(sys_ctrl_pdev);
#endif
    pr_info("in sys_ctrl_exit\n");
}

module_init(sys_ctrl_init);
module_exit(sys_ctrl_exit);

/* Module information */
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("sys ctrl driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:sys_ctrl_driver");
