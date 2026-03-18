/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2023-2023. All rights reserved.
 * Description: mipi entry function
 * Author: Hisilicon multimedia software group
 * Create: 2023-03-08
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/version.h>
#include <linux/of_platform.h>
#include <linux/pinctrl/consumer.h>
#include <linux/init.h>
#include <linux/of.h>
#include <hi_mipi_rx.h>
#include <hi_osal.h>
#if defined(ML_V2) || defined(CAMERAV2_UT_TEST)
#include <linux/bootdot.h>
#else
#include "ascend_kernel_hal.h"
#endif
#include "hi_debug.h"
#include "hi_mipi_rx_mod_init.h"
#include "mipi_rx_hal.h"
#include "slvs_hal.h"
#include "mipi_rx_type.h"
#include "camera_black_box.h"

#ifdef CAMERAV2_UT_TEST
#include "mipi_rx_utest_stub.h"
#endif

static hi_bool  g_mipi_rx_succ_loaded = HI_FALSE;

typedef enum {
    MIPI_INIT_START = 0,
    MIPI_ISPSUBCTRL_RESOURCE_INIT,
    MIPI_MIPI_RX_RESOURCE_INIT,
    MIPI_MIPI_RX_INTERRUPT_INIT,
    MIPI_SLVS_EC_RESOURCE_INIT,
    MIPI_SLVS_EC_INTERRUPT_INIT,
    MIPI_MOD_INIT,
    MIPI_INIT_END
} hi_mipi_start_status;

/**
 * @brief: Config the function multiplexing of pins such as sensor_clk sensor_rst
 * @param[in]: dev: mipirx device
 */
static int hi_sensor_ctl_iomux_init(struct device *dev)
{
    struct pinctrl *pinctrl = NULL;
    struct pinctrl_state *pins_default = NULL;
    int ret;
    mipi_rx_info_trace("Config sensor control interface pin iomux\n");

    // get pinctrl handle
    pinctrl = devm_pinctrl_get(dev);
    if (IS_ERR(pinctrl)) {
        mipi_rx_err_trace("current envaroment not suport pinctrl.\n");
        return -ENOENT;
    }

    // get default pin function
    pins_default = pinctrl_lookup_state(pinctrl, PINCTRL_STATE_DEFAULT);
    if (IS_ERR(pins_default)) {
        mipi_rx_err_trace("pinctrl_lookup_state failed\n");
        devm_pinctrl_put(pinctrl);
        return -ENOENT;
    }

    // select default pin function
    ret = pinctrl_select_state(pinctrl, pins_default);
    if (ret < 0) {
        mipi_rx_err_trace("pinctrl_select_state failed\n");
        devm_pinctrl_put(pinctrl);
        return ret;
    }

    return 0;
}

static int hi_mipi_rx_probe(struct platform_device *pdev)
{
    mipi_rx_info_trace("Kernel hi_mipi_rx_probe enter\n");
    int ret;
    hi_u32 out_value = 0U;
    struct device_node* media_device_node;

    if (pdev == NULL) {
        mipi_rx_err_trace("NULL pointer!\n");
        goto fail0;
    }

    // 读取DTS初始化开关
    media_device_node = of_find_node_by_path("/media");
    if (media_device_node == NULL) {
        mipi_rx_err_trace("find media_device_node failed\n");
        goto fail0;
    }

    if (of_property_read_u32(media_device_node, "isp_enable", &out_value) != 0) {
        mipi_rx_notice_trace("read isp_enable failed\n");
    }

    if (out_value != 1U) {
        mipi_rx_notice_trace("mipi_rx does not support enable\n");
        return 0;
    }

    // 黑匣子打点
    (hi_void)bootdot_init_blk(MEIDA_BLOCK_BOX_BLOCK_ID, 0U, (hi_u32)CAMERA_MODID_MIPI_START_FAIL, MIPI_INIT_END);
    (hi_void)bootdot_set_blk(MEIDA_BLOCK_BOX_BLOCK_ID, 0U, MIPI_INIT_START);

    // ISP SubCtrl Memory Map
    ret = isp_subctrl_reg_remap();
    if (ret < 0) {
        mipi_rx_err_trace("isp_subctrl_reg_remap fail!\n");
        goto fail0;
    }
    // 黑匣子打点
    (hi_void)bootdot_set_blk(MEIDA_BLOCK_BOX_BLOCK_ID, 0, MIPI_ISPSUBCTRL_RESOURCE_INIT);

    // MIPIRx Memory Map
    struct resource *mem = NULL;
    mem = osal_platform_get_resource_byname(pdev, IORESOURCE_MEM, "MIPI_RX");
    void *regs = NULL;
    regs = devm_ioremap_resource(&pdev->dev, mem); // if mem is null, will return err
    if (IS_ERR(regs)) {
        dev_err(&pdev->dev, "mipi_rx remap mem error.\n");
        goto fail1;
    }
    mipi_rx_set_regs(regs);
    // 黑匣子打点
    (hi_void)bootdot_set_blk(MEIDA_BLOCK_BOX_BLOCK_ID, 0, MIPI_MIPI_RX_RESOURCE_INIT);

    // MIPIRx Interrupt Config
    int irq_num;
    irq_num = osal_platform_get_irq_byname(pdev, "MIPI_RX");
    mipi_rx_info_trace("Kernel MIPI Rx driver probe. mipi_rx irq_num: %d\n", irq_num);
    if (irq_num <= 0) {
        dev_err(&pdev->dev, "can not find mipi_rx IRQ\n");
        goto fail2;
    }
    mipi_rx_set_irq_num((unsigned int)irq_num);
    // 黑匣子打点
    (hi_void)bootdot_set_blk(MEIDA_BLOCK_BOX_BLOCK_ID, 0, MIPI_MIPI_RX_INTERRUPT_INIT);

    // SLVS_EC Memory Map
    mem = osal_platform_get_resource_byname(pdev, IORESOURCE_MEM, "SLVS_EC");
    regs = devm_ioremap_resource(&pdev->dev, mem); // if mem is null, will return err
    if (IS_ERR(regs)) {
        dev_err(&pdev->dev, "slvs_ec remap mem error.\n");
        goto fail3;
    }
    slvs_ec_set_regs(regs);
    // 黑匣子打点
    (hi_void)bootdot_set_blk(MEIDA_BLOCK_BOX_BLOCK_ID, 0, MIPI_SLVS_EC_RESOURCE_INIT);

    // SLVS_EC Interrupt Config
    int irq_ec_num = osal_platform_get_irq_byname(pdev, "SLVS_EC");
    mipi_rx_info_trace("Kernel MIPI Rx driver probe. slvs_ec irq_num: %d\n", irq_ec_num);
    if (irq_ec_num <= 0) {
        dev_err(&pdev->dev, "can not find slvs_ec IRQ\n");
        goto fail4;
    }
    slvs_set_irq_num((unsigned int)irq_ec_num);
    // 黑匣子打点
    (hi_void)bootdot_set_blk(MEIDA_BLOCK_BOX_BLOCK_ID, 0, MIPI_SLVS_EC_INTERRUPT_INIT);

    // Module Init
    ret = mipi_rx_mod_init();
    if (ret != 0) {
        dev_err(&pdev->dev, "Kernel MIPI Rx driver probe. mipi_rx_mod_init failed\n");
        goto fail5;
    }
    // 黑匣子打点
    (hi_void)bootdot_set_blk(MEIDA_BLOCK_BOX_BLOCK_ID, 0, MIPI_MOD_INIT);

    // Pinmux Config
    ret = hi_sensor_ctl_iomux_init(&pdev->dev);
    if (ret != 0) {
        mipi_rx_err_trace("Kernel MIPI Rx driver probe. config sensor control interface pin mux failed\n");
        goto fail6;
    }
    // 黑匣子打点
    (hi_void)bootdot_set_blk(MEIDA_BLOCK_BOX_BLOCK_ID, 0, MIPI_INIT_END);

    g_mipi_rx_succ_loaded = HI_TRUE;
    return 0;

fail6:
    mipi_rx_mod_exit();
fail5:
    slvs_set_irq_num(SLVS_EC0_IRQ);
fail4:
    slvs_ec_set_regs(NULL);
fail3:
    mipi_rx_set_irq_num(MIPI_RX_IRQ);
fail2:
    mipi_rx_set_regs(NULL);
fail1:
    isp_subctrl_reg_unremap();
fail0:
    return HI_FAILURE;
}

static int hi_mipi_rx_remove(struct platform_device *pdev)
{
    if (pdev == NULL) {
        mipi_rx_err_trace("NULL pointer!\n");
        return HI_FAILURE;
    }

    if (g_mipi_rx_succ_loaded != HI_TRUE) {
        mipi_rx_warn_trace("unload mipi_rx.ko as mipi_rx dev failed load!\n");
        return HI_FAILURE;
    }
    mipi_rx_mod_exit();
    isp_subctrl_reg_unremap();
    mipi_rx_set_regs(NULL);
    slvs_ec_set_regs(NULL);
    g_mipi_rx_succ_loaded = HI_FALSE;
    return 0;
}

static const struct of_device_id g_ot_mipi_rx_match[] = {
    { .compatible = "hisilicon,hisi-media-mipi_rx" },
    {},
};
MODULE_DEVICE_TABLE(of, g_ot_mipi_rx_match);

static struct platform_driver g_ot_mipi_rx_driver = {
    .probe  = hi_mipi_rx_probe,
    .remove = hi_mipi_rx_remove,
    .driver = {
        .name           = "mipi_rx",
        .of_match_table = g_ot_mipi_rx_match,
    },
};

osal_module_platform_driver(g_ot_mipi_rx_driver);

MODULE_AUTHOR("Digital Media Team.");
MODULE_DESCRIPTION("Mipi_Rx Driver");
MODULE_VERSION("OT_VERSION = mipi_rx @OT_MPP");
MODULE_LICENSE("GPL");
