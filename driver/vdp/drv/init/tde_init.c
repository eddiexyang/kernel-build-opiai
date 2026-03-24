/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2018-2020. All rights reserved.
 * Description: tde init
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/platform_device.h>
#include <linux/of_platform.h>
#include "ot_tde_mod_init.h"
#include "ot_type.h"
#include "ot_common.h"
#include "ot_osal.h"
#include "securec.h"
#include "tde_adp.h"
#include "tde_hal.h"
#include "tde_define.h"
#include "ascend_kernel_hal.h"

td_u32 g_tde_tmp_buf = 1658880U; /* 1658880 buffer size */
bool g_is_resize_filter = true;
td_u32 g_max_node_num = 201U;      /* 201 nums */
td_u32 g_rgb_truncation_mode = 1U; /* 1 for rgb truncation mode */

#ifdef TDE_MPARAM_RESERVE
module_param(g_tde_tmp_buf, uint, (td_u32)S_IRUGO);
module_param(g_is_resize_filter, bool, (td_u32)S_IRUGO);
module_param(g_max_node_num, uint, (td_u32)S_IRUGO);
module_param(g_rgb_truncation_mode, uint, (td_u32)S_IRUGO);
#endif

static void tde_bootdot_init_block(uint32_t block_id, uint32_t magic, uint32_t execption_id, uint32_t expect_status)
{
    int32_t ret;

    ret = bootdot_init_blk(block_id, magic, execption_id, expect_status);
    if (ret != 0) {
        tde_error("tde init block fail\n");
    }
}

static void tde_bootdot_set_block_status(uint32_t block_id, uint32_t magic, uint32_t current_status)
{
    int32_t ret;

    ret = bootdot_set_blk(block_id, magic, current_status);
    if (ret != 0) {
        tde_error("tde set block status fail\n");
    }
}

static int ot_tde_probe(struct platform_device *pdev)
{
    tde_bootdot_init_block(TDE_BOOTDOT_BLOCK_ID, TDE_BOOTDOT_MAGIC_NUM, TDE_INIT_FAIL, INIT_END);
    tde_bootdot_set_block_status(TDE_BOOTDOT_BLOCK_ID, TDE_BOOTDOT_MAGIC_NUM, INIT_START);

    struct resource *mem = TD_NULL;
    td_s32 tde_irq;
    td_u32 *base_vir_addr = TD_NULL;

    mem = osal_platform_get_resource_byname(pdev, IORESOURCE_MEM, "tde");

    /* base_vir_addr will be automatically released by the system. */
    base_vir_addr = devm_ioremap_resource(&pdev->dev, mem);
    if (IS_ERR(base_vir_addr)) {
        tde_error("base_vir_addr is invalid!");
        return (td_s32)PTR_ERR(base_vir_addr);
    }
    tde_hal_set_base_vir_addr(base_vir_addr);

    tde_irq = osal_platform_get_irq_byname(pdev, "tde_osr_isr");
    if (tde_irq <= 0) {
        tde_error("cannot find tde IRQ\n");
        return TD_FAILURE;
    }
    set_tde_irq((td_u32)tde_irq);
    if (g_max_node_num > 0) {
        tde_set_max_node_num(g_max_node_num);
    }
    tde_set_resize_filter((td_bool)g_is_resize_filter);
    if ((g_rgb_truncation_mode == 0) || (g_rgb_truncation_mode == 1)) { /* 0 1 for rgb truncation mode */
        tde_init_set_rgb_truncation_mode(g_rgb_truncation_mode);
    }
    if (g_tde_tmp_buf > 0) {
        tde_set_tde_tmp_buffer(g_tde_tmp_buf);
    }
    if (tde_drv_mod_init() != TD_SUCCESS) {
        tde_error("load drv_tde.ko failed!\n");
        return TD_FAILURE;
    }

    tde_info("load drv_tde.ko success!\n");
    tde_bootdot_set_block_status(TDE_BOOTDOT_BLOCK_ID, TDE_BOOTDOT_MAGIC_NUM, INIT_END);
    return TD_SUCCESS;
}

static int ot_tde_remove(struct platform_device *pdev)
{
    ot_unused(pdev);
    tde_drv_mod_exit();
    tde_info("unload drv_tde.ko success!\n");
    return TD_SUCCESS;
}

static int ot_tde_suspend(struct platform_device *pdev, pm_message_t state)
{
    tde_bootdot_init_block(TDE_BOOTDOT_BLOCK_ID, TDE_BOOTDOT_MAGIC_NUM, TDE_SUSPEND_FAIL, SUSPEND_END);
    tde_bootdot_set_block_status(TDE_BOOTDOT_BLOCK_ID, TDE_BOOTDOT_MAGIC_NUM, SUSPEND_START);
    if (pdev == TD_NULL) {
        tde_error("dev is null!!\n");
        return TD_FAILURE;
    }
    tde_info("Start TDE Suspend.\n");
    if (tde_hal_ctl_is_idle_safely()) {
        if (tde_wrap_soft_reset() != TD_TRUE) {
            return TD_FAILURE;
        }
        if (tde_hal_media_subctl_reset() != TD_TRUE) {
            return TD_FAILURE;
        }
        if (tde_hal_media_subctl_disable() != TD_TRUE) {
            return TD_FAILURE;
        }
    } else {
        tde_error("Work busy, TDE Suspended Failed!\n");
        return TD_FAILURE;
    }
    pdev->dev.power.power_state = state;
    tde_info("TDE Suspended!\n");
    tde_bootdot_set_block_status(TDE_BOOTDOT_BLOCK_ID, TDE_BOOTDOT_MAGIC_NUM, SUSPEND_END);
    return TD_SUCCESS;
}

static int ot_tde_resume(struct platform_device *pdev)
{
    tde_bootdot_init_block(TDE_BOOTDOT_BLOCK_ID, TDE_BOOTDOT_MAGIC_NUM, TDE_RESUME_FAIL, RESUME_END);
    tde_bootdot_set_block_status(TDE_BOOTDOT_BLOCK_ID, TDE_BOOTDOT_MAGIC_NUM, RESUME_START);
    if (pdev == TD_NULL) {
        tde_error("dev is null!!\n");
        return TD_FAILURE;
    }

    tde_info("Start TDE Resume.\n");

    if (tde_hal_media_subctl_enable() != TD_TRUE) {
        return TD_FAILURE;
    }

    if (tde_hal_media_subctl_cancel_reset() != TD_TRUE) {
        return TD_FAILURE;
    }

    if (tde_hal_wrap_enable_clock() != TD_TRUE) {
        return TD_FAILURE;
    }

    if (tde_wrap_soft_reset() != TD_TRUE) {
        return TD_FAILURE;
    }

    if (tde_wrap_soft_dereset() != TD_TRUE) {
        return TD_FAILURE;
    }

    pdev->dev.power.power_state = PMSG_ON;
    tde_info("TDE Resumed!\n");

    tde_bootdot_set_block_status(TDE_BOOTDOT_BLOCK_ID, TDE_BOOTDOT_MAGIC_NUM, RESUME_END);

    return TD_SUCCESS;
}

static const struct of_device_id g_ot_tde_match[] = {
    { .compatible = "vendor,tde" },
    {}
};
MODULE_DEVICE_TABLE(of, g_ot_tde_match);

static struct platform_driver g_ot_tde_driver = {
    .probe = ot_tde_probe,
    .remove = ot_tde_remove,
    .resume = ot_tde_resume,
    .suspend = ot_tde_suspend,
    .driver = {
        .name = "ot_tde",
        .of_match_table = g_ot_tde_match
    }
};

osal_module_platform_driver(g_ot_tde_driver);

MODULE_LICENSE("GPL");
