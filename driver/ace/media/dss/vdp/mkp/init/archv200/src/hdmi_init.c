/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2020-2022. All rights reserved.
 * Description:hdmi init.
 * Author: Hisilicon multimedia software group
 * Create: 2020/03/27
 */
#include "ot_hdmi_mod_init.h"
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_platform.h>
#include "ascend_kernel_hal.h"
#include "ot_common.h"
#include "drv_hdmi_intf.h"

#define HDMI_PORT_MAX 2
#define HDMI_DEV_NAME_LENGTH 16

void hdmi_bootdot_init_block(uint32_t block_id, uint32_t magic, uint32_t execption_id, uint32_t expect_status)
{
    (void)osal_bootdot_init_blk(block_id, magic, execption_id, expect_status);
}

void hdmi_bootdot_set_block_status(uint32_t block_id, uint32_t magic, uint32_t current_status)
{
    (void)osal_bootdot_set_blk(block_id, magic, current_status);
}

static int hdmi_probe(struct platform_device *pdev)
{
    td_s32 i, ret;
    td_char *hdmi_reg = NULL;
    td_char *hdmi_phy = NULL;
    struct resource *mem = NULL;
    td_char hdmi_dev_name[HDMI_PORT_MAX][HDMI_DEV_NAME_LENGTH] = { "hdmi0", "hdmi1" };
    td_char hdmi_phy_name[HDMI_PORT_MAX][HDMI_DEV_NAME_LENGTH] = { "phy0", "phy1" };

    hdmi_bootdot_init_block(HDMI_BOOTDOT_BLOCK_ID, HDMI_BOOTDOT_MAGIC_NUM, HDMI_INIT_FAIL, INIT_END);
    hdmi_bootdot_set_block_status(HDMI_BOOTDOT_BLOCK_ID, HDMI_BOOTDOT_MAGIC_NUM, INIT_START);

    for (i = 0; i < HDMI_PORT_MAX; i++) {
        mem = osal_platform_get_resource_byname(pdev, IORESOURCE_MEM, hdmi_dev_name[i]);
        hdmi_reg = devm_ioremap_resource(&pdev->dev, mem);
        if (IS_ERR(hdmi_reg)) {
            return (td_s32)PTR_ERR(hdmi_reg);
        }

        ret = hdmi_set_reg(i, hdmi_reg);
        if (ret != TD_SUCCESS) {
            return ret;
        }

        mem = osal_platform_get_resource_byname(pdev, IORESOURCE_MEM, hdmi_phy_name[i]);
        hdmi_phy = devm_ioremap_resource(&pdev->dev, mem);
        if (IS_ERR(hdmi_phy)) {
            return (td_s32)PTR_ERR(hdmi_phy);
        }

        ret = hdmi_set_phy(i, hdmi_phy);
        if (ret != TD_SUCCESS) {
            return ret;
        }
    }

    ret = hdmi_drv_mod_init();
    hdmi_check_pid_spin_lock_init();

    hdmi_bootdot_set_block_status(HDMI_BOOTDOT_BLOCK_ID, HDMI_BOOTDOT_MAGIC_NUM, INIT_END);

    drv_hdmi_phy_product_power_set_param();

    return ret;
}

static void hdmi_remove(struct platform_device *pdev)
{
    ot_unused(pdev);
    hdmi_drv_mod_exit();
    hdmi_set_reg(0, NULL);
    hdmi_set_reg(1, NULL);

    hdmi_check_pid_spin_lock_destory();
}

static const struct of_device_id g_hdmi_match[] = {
    { .compatible = "vendor,hdmi" },
    {}
};

MODULE_DEVICE_TABLE(of, g_hdmi_match);

static struct platform_driver g_hdmi_driver = {
    .probe  = hdmi_probe,
    .remove = hdmi_remove,
    .driver = {
        .name = "hdmi",
        .of_match_table = g_hdmi_match,
    },
};

osal_module_platform_driver(g_hdmi_driver);

MODULE_LICENSE("GPL");
