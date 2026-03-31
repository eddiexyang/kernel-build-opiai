/*
 * Copyright (C) Hisilicon Technologies Co., Ltd. 2016-2020. All rights reserved.
 * Description: mipi tx file.
 * Author: Hisilicon multimedia software group
 * Create: 2016/06/28
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/version.h>
#include <linux/of_platform.h>
#include "ot_mipi_tx_mod_init.h"
#include "ascend_kernel_hal.h"
#include "ot_osal.h"
#include "mipi_tx.h"

static int g_smooth = 0;

int mipi_tx_get_smooth(void)
{
    return g_smooth;
}

void mipi_bootdot_init_block(uint32_t block_id, uint32_t magic, uint32_t execption_id, uint32_t expect_status)
{
    int32_t ret;

    ret = bootdot_init_blk(block_id, magic, execption_id, expect_status);
    if (ret != 0) {
        mipi_tx_err("mipi init block fail\n");
    }
}

void mipi_bootdot_set_block_status(uint32_t block_id, uint32_t magic, uint32_t current_status)
{
    int32_t ret;

    ret = bootdot_set_blk(block_id, magic, current_status);
    if (ret != 0) {
        mipi_tx_err("mipi set block status fail\n");
    }
}

static int mipi_tx_probe(struct platform_device *pdev)
{
    mipi_bootdot_init_block(MIPI_BOOTDOT_BLOCK_ID, MIPI_BOOTDOT_MAGIC_NUM, MIPI_INIT_FAIL, INIT_END);
    mipi_bootdot_set_block_status(MIPI_BOOTDOT_BLOCK_ID, MIPI_BOOTDOT_MAGIC_NUM, INIT_START);

    struct resource *mem = NULL;
    void *regs = NULL;

    mem = osal_platform_get_resource_byname(pdev, IORESOURCE_MEM, "mipi_tx");
    regs = devm_ioremap_resource(&pdev->dev, mem);
    if (IS_ERR(regs)) {
        dev_err(&pdev->dev, "mipi_tx remap mem error.\n");
        return (int)(PTR_ERR(regs));
    }

    mipi_tx_set_regs(regs);

    int ret = mipi_tx_module_init(g_smooth);
    mipi_check_pid_spin_lock_init();

    mipi_bootdot_set_block_status(MIPI_BOOTDOT_BLOCK_ID, MIPI_BOOTDOT_MAGIC_NUM, INIT_END);

    return ret;
}

static int mipi_tx_remove(struct platform_device *pdev)
{
    (void)pdev;

    mipi_tx_module_exit();
    mipi_tx_set_regs(NULL);
    mipi_check_pid_spin_lock_destory();

    return 0;
}

static const struct of_device_id mipi_tx_match[] = {
    { .compatible = "vendor,mipi_tx" },
    {}
};
MODULE_DEVICE_TABLE(of, mipi_tx_match);

static struct platform_driver mipi_tx_driver = {
    .probe  = mipi_tx_probe,
    .remove = mipi_tx_remove,
    .driver = {
        .name   = "mipi_tx",
        .of_match_table = mipi_tx_match,
    }
};

module_param(g_smooth, int, S_IRUGO);
module_platform_driver(mipi_tx_driver);

MODULE_DESCRIPTION("mipi_tx driver");
MODULE_VERSION("mipi_tx");
MODULE_LICENSE("GPL");
