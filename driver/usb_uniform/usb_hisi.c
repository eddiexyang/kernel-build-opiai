/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
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
 * Author:huawei
 * Create: 2021-03-30
 */
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/kallsyms.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/version.h>
#include <linux/reset-controller.h>
#include <linux/reset.h>
#include <linux/clk.h>
#include "kdrv_usb_api.h"
#include "usb_hisi.h"
#include "usb_misc.h"

#define MAX_PORT 4
#define PORT_REG_SIZE 0x10
#define RES_NUM 2
#define MAX_QOS_VALUE 0xF
#define FS_MS_PBTYE_VALUE 0x53d
#define MAX_CFG_NUM 16
#define RESP_DOORBELL_TIME 5


#define USB_REG_HCCAPA 0
#define USB_REG_RTSOFF 0x18

#define USB_REG_USBSTS_OPER_OFF  0x04
#define USB_REG_CRCR0_OPER_OFF  0x18
#define USB_REG_CONFIG_OPER_OFF 0x38
#define USB_REG_PORTSC_OPER_OFF 0x400
#define USB_REG_PORTPMSC_OPER_OFF 0x404
#define USB_REG_PORTLI_OPER_OFF 0x408

#define USB_REG_IMAN_RUNTIME_OFF 0x20

#define OPER_REG_OFF GENMASK(7, 0)
#define USBSTS_HC_ERROR BIT(12)
#define USBSTS_HC_ERROR_SHIFT 12
#define USBSTS_SS_ERROR BIT(10)
#define USBSTS_SS_ERROR_SHIFT 10
#define USBSTS_HOST_SYS_ERR BIT(2)
#define USBSTS_HOST_SYS_ERR_SHIFT 2
#define PORTSC_PLC BIT(22)
#define PORTSC_PLC_SHIFT 22
#define PORTSC_CSC BIT(17)
#define PORTSC_CSC_SHIFT 17
#define PORTSC_SPEED GENMASK(13, 10)
#define PORTSC_SPEED_SHIFT 10
#define PORTSC_PP BIT(9)
#define PORTSC_PP_SHIFT 9
#define PORTSC_PLS GENMASK(8, 5)
#define PORTSC_PLS_SHIFT 5
#define PORTSC_OCA BIT(3)
#define PORTSC_OCA_SHIFT 3
#define PORTSC_PED BIT(1)
#define PORTSC_PED_SHIFT 1
#define PORTSC_CONNECT BIT(0)

#define IMAN_INT_ENABLE BIT(1)
#define IMAN_INT_ENABLE_SHIFT 1
#define IMAN_INT_PENDING BIT(0)

#define CRCR0_CRR BIT(3)
#define CRCR0_CRR_SHIFT 3
#define CRCR0_RCS BIT(0)

#define CONFIG_CIE BIT(9)
#define CONFIG_CIE_SHIFT 9
#define CONFIG_U3E BIT(8)
#define CONFIG_U3E_SHIFT 8
#define CONFIG_SLOT_ENA GENMASK(7, 0)

#define U2_PORTPMSC_TEST_MODE GENMASK(31, 28)
#define U2_PORTPMSC_TEST_MODE_SHIFT 28
#define U2_PORTPMSC_HLE BIT(16)
#define U2_PORTPMSC_HLE_SHIFT 16
#define U2_PORTPMSC_WAKE_ENA BIT(3)
#define U2_PORTPMSC_WAKE_ENA_SHIFT 3
#define U2_PORTPMSC_L1_STS GENMASK(2, 0)
#define U2_PORTPMSC_L1_STS_SHIFT 0

#define U3_PORTPMSC_U2_TIMEOUT GENMASK(15, 8)
#define U3_PORTPMSC_U2_TIMEOUT_SHIFT 8
#define U3_PORTPMSC_U1_TIMEOUT GENMASK(7, 0)
#define U3_PORTPMSC_U1_TIMEOUT_SHIFT 0

#define U3_PORTLI_TLC GENMASK(23, 20)
#define U3_PORTLI_TLC_SHIFT 20
#define U3_PORTLI_RLC GENMASK(19, 16)
#define U3_PORTLI_RLC_SHIFT 16
#define U3_PORTLI_LINK_ERR_CNT GENMASK(15, 0)
#define U3_PORTLI_LINK_ERR_CNT_SHIFT 0

struct host_find_data {
    uint32_t host_no;
    struct hisi_usb *hiusb;
};

int usb_reset(struct hisi_usb *hiusb)
{
    int ret;

    ret = hisi_subctrl_usb_clkgate_disable(hiusb);
    if (ret != 0) {
        dev_err(hiusb->dev, "hisi_subctrl_usb_clkgate_disable failed, ret=%d\n", ret);
        return ret;
    }

    (void)hisi_subctrl_usb_phy_reset_assert(hiusb);
    ret = hisi_subctrl_usb_reset_assert(hiusb);
    if (ret != 0) {
        dev_err(hiusb->dev, "soft reset failed, ret=%d\n", ret);
        return ret;
    }

    ret = hisi_subctrl_usb_clkgate_enable(hiusb);
    if (ret != 0) {
        dev_err(hiusb->dev, "hisi_subctrl_usb_clkgate_enable failed, ret=%d\n", ret);
        return ret;
    }

    udelay(100);

    ret = hisi_subctrl_usb_clkgate_disable(hiusb);
    if (ret != 0) {
        dev_err(hiusb->dev, "hisi_subctrl_usb_clkgate_disable failed, ret=%d\n", ret);
        return ret;
    }

    ret = hisi_subctrl_usb_reset_deassert(hiusb);
    if (ret != 0) {
        dev_err(hiusb->dev, "soft dereset failed, ret=%d\n", ret);
        return ret;
    }
    (void)hisi_subctrl_usb_phy_reset_deassert(hiusb);

    ret = hisi_subctrl_usb_get_reset_status(hiusb);
    if (ret != 0) {
        dev_err(hiusb->dev, "dev is not deassert status, ret=%d\n", ret);
        return ret;
    }

    ret = hisi_subctrl_usb_clkgate_enable(hiusb);
    if (ret != 0) {
        dev_err(hiusb->dev, "hisi_subctrl_usb_clkgate_enable failed, ret=%d\n", ret);
        return ret;
    }

    return 0;
}

static int  usb_parse_dts(struct platform_device *pdev, struct hisi_usb *hiusb)
{
    u64 para64[RES_NUM] = {0};
    struct device *dev = &pdev->dev;
    struct fwnode_handle *phy;
    int ret;

    ret = device_property_read_u32(dev, "host_no", &hiusb->host_no);
    if (ret != 0) {
        dev_err(dev, "can't get host_no, ret=%d\n", ret);
        return ret;
    }
    if (device_property_read_u64_array(dev, "reg_base", para64, RES_NUM) != 0) {
        dev_err(dev, "get reg_base fail\r\n");
        return -EINVAL;
    }
    hiusb->host.vaddr = devm_ioremap(dev, para64[0], (u32)para64[1]);
    hiusb->host.io_size = (u32)para64[1];
    if (hiusb->host.vaddr == NULL) {
        return -ENOMEM;
    }
#ifdef CONFIG_USB_PCS
    if (device_property_read_u64_array(dev, "pcs_base", para64, RES_NUM) == 0) {
        hiusb->host.pcs_vaddr = devm_ioremap(dev, para64[0], (u32)para64[1]);
        hiusb->host.pcs_size = (u32)para64[1];
        if (hiusb->host.pcs_vaddr == NULL) {
            return -ENOMEM;
        }
    } else {
        dev_err(dev, "get pcs_base fail\r\n");
        return -EINVAL;
    }
#endif
    if (device_property_read_u32(dev, "u2_disable", &hiusb->host.u2_disable) != 0) {
        hiusb->host.u2_disable = 0;
    }

    phy = device_get_named_child_node(dev, "phy");
    if (phy != NULL) {
        if (fwnode_property_read_u64_array(phy, "phy_base", para64, RES_NUM) != 0) {
            dev_err(dev, "get phy base fail\r\n");
            return -EINVAL;
        }
        hiusb->host.phy_vaddr = devm_ioremap(dev, para64[0], (u32)para64[1]);
        hiusb->host.phy_size = (u32)para64[1];
        if (hiusb->host.phy_vaddr == NULL) {
            return -ENOMEM;
        }
    }
    return 0;
}

static void usb_disable_u2(struct hisi_usb *hiusb)
{
    if (hiusb->host.u2_disable != 0) {
        usbdrv_disable_u2(hiusb->host.vaddr);
    }
}

static void usb_axi_init(struct hisi_usb *hiusb)
{
    usbdrv_axi_init(hiusb->host.vaddr);
}
#ifdef CONFIG_USB_PCS
static void usb_pcs_init(struct hisi_usb *hiusb)
{
    usbdrv_pcs_init(hiusb->host.pcs_vaddr);
}
#endif
static void usb_set_fs_ms_pbyte_num(struct hisi_usb *hiusb)
{
    usbdrv_set_fs_ms_pbyte_num(hiusb->host.vaddr, FS_MS_PBTYE_VALUE);
}

static void usb_set_trpu_block_mode(struct hisi_usb *hiusb)
{
    usbdrv_set_trpu_block_mode(hiusb->host.vaddr, 0);
}

static void usb_set_trpu_resp_doorbell_time(struct hisi_usb *hiusb)
{
    usbdrv_set_trpu_resp_doorbell_time(hiusb->host.vaddr, RESP_DOORBELL_TIME);
}

void usb_hiusbcv100_init(struct hisi_usb *hiusb)
{
    usb_disable_u2(hiusb);
    usb_axi_init(hiusb);
#ifdef CONFIG_USB_PCS
    usb_pcs_init(hiusb);
#endif
    usb_set_trpu_block_mode(hiusb);
    usb_set_fs_ms_pbyte_num(hiusb);
    usb_set_trpu_resp_doorbell_time(hiusb);
}

static void usb_phy_cfg_adjust(struct hisi_usb *hiusb)
{
    struct device *dev = hiusb->dev;
    struct fwnode_handle *phy;
    int addr_cnt, mask_cnt, value_cnt, i;
    u32 addr[MAX_CFG_NUM] = {0};
    u32 mask[MAX_CFG_NUM] = {0};
    u32 value[MAX_CFG_NUM] = {0};
    u32 reg_val;

    phy = device_get_named_child_node(dev, "phy");
    if (phy == NULL) {
        return;
    }
    addr_cnt = fwnode_property_count_u32(phy, "cfg_off");
    mask_cnt = fwnode_property_count_u32(phy, "cfg_mask");
    value_cnt = fwnode_property_count_u32(phy, "cfg_value");
    if (addr_cnt != mask_cnt || addr_cnt != value_cnt) {
        dev_err(dev, "phy cfg num is not equal\n");
        return;
    }
    if (addr_cnt > MAX_CFG_NUM || addr_cnt <= 0) {
        dev_err(dev, "phy cfg num is out of range\n");
        return;
    }
    (void)fwnode_property_read_u32_array(phy, "cfg_off", addr, addr_cnt);
    (void)fwnode_property_read_u32_array(phy, "cfg_mask", mask, mask_cnt);
    (void)fwnode_property_read_u32_array(phy, "cfg_value", value, value_cnt);
    for (i = 0; i < addr_cnt; i++) {
        if (addr[i] >= hiusb->host.phy_size || addr[i] % 4 != 0) {
            dev_err(dev, "phy addr is illegal\n");
            return;
        }
        reg_val = readl(hiusb->host.phy_vaddr + addr[i]) & (~mask[i]);
        reg_val |= value[i] & mask[i];
        writel(reg_val, hiusb->host.phy_vaddr + addr[i]);
    }
}
static int usb_hisi_probe(struct platform_device *pdev)
{
    struct hisi_usb *hiusb;
    struct device *dev = &pdev->dev;
    int ret;

    hiusb = devm_kzalloc(&pdev->dev, sizeof(*hiusb), GFP_KERNEL);
    if (hiusb == NULL) {
        dev_err(dev, "devm_kzalloc hiusb failed\n");
        return -ENOMEM;
    }
    hiusb->dev = &pdev->dev;
    ret = usb_parse_dts(pdev, hiusb);
    if (ret != 0) {
        dev_err(dev, "usb_parse_dts failed, ret=%d\n", ret);
        return ret;
    }
    platform_set_drvdata(pdev, hiusb);
    ret = usb_get_clk_rst_info(hiusb, dev);
    if (ret != 0) {
        dev_err(dev, "get clk and rst failed, ret=%d\n", ret);
        goto exit;
    }
#ifdef CONFIG_UDRV
    ret = hisi_subctrl_usb_init(hiusb);
    if (ret != 0) {
        dev_err(dev, "usb subctrl init failed, ret=%d\n", ret);
        goto exit;
    }
#endif
    ret = usb_reset(hiusb);
    if (ret != 0) {
        goto exit;
    }
    hiusb->host.oper_base = readl(hiusb->host.vaddr + USB_REG_HCCAPA) & OPER_REG_OFF;
    hiusb->host.runtime_base = readl(hiusb->host.vaddr + USB_REG_RTSOFF);

    (void)hisi_subctrl_usb_set_cacheline(hiusb);
    (void)hisi_subctrl_usb_set_oca_cfg(hiusb);
    usb_hiusbcv100_init(hiusb);
    usb_phy_cfg_adjust(hiusb);
    return 0;

exit:
    platform_set_drvdata(pdev, NULL);
    return ret;
}

static void usb_hisi_remove(struct platform_device *pdev)
{
    struct hisi_usb *hiusb;
    hiusb = platform_get_drvdata(pdev);

    devm_iounmap(&pdev->dev, hiusb->host.vaddr);
    platform_set_drvdata(pdev, NULL);
    
}

static int usb_suspend(struct device *dev)
{
    struct hisi_usb *hiusb;
    int ret;
    hiusb = dev_get_drvdata(dev);
    ret = hisi_subctrl_usb_set_usb_5v_off(hiusb);
    if (ret) {
        dev_err(dev, "usb suspend : gpio set failed, ret=%d\n", ret);
        return ret;
    }
    return 0;
}

static int usb_resume(struct device *dev)
{
    struct hisi_usb *hiusb;
    int ret;
    hiusb = dev_get_drvdata(dev);

    ret = hisi_subctrl_usb_set_usb_5v_on(hiusb);
    if (ret) {
        dev_err(dev, "usb resume : gpio set failed, ret=%d\n", ret);
        return ret;
    }

    ret = usb_reset(hiusb);
    if (ret != 0) {
        dev_err(dev, "usb resume : reset failed, ret=%d\n", ret);
        return ret;
    }
    (void)hisi_subctrl_usb_set_cacheline(hiusb);
    (void)hisi_subctrl_usb_set_oca_cfg(hiusb);
    usb_hiusbcv100_init(hiusb);
    usb_phy_cfg_adjust(hiusb);
    return 0;
}

static const struct dev_pm_ops hisi_usb_pm_ops = {
    SET_LATE_SYSTEM_SLEEP_PM_OPS(usb_suspend, usb_resume)
};

static const struct of_device_id usb_hisi_match[] = {
    { .compatible = "hisilicon,udrv_usb" },
    {}
};
MODULE_DEVICE_TABLE(of, usb_hisi_match);

static struct platform_driver usb_hisi_driver = {
    .probe = usb_hisi_probe,
    .remove = usb_hisi_remove,
    .driver = {
        .name = "usb_hisi",
        .pm = &hisi_usb_pm_ops,
        .of_match_table = usb_hisi_match,
    },
};

module_platform_driver(usb_hisi_driver);

static int match_hisi_usb(struct device *dev, void *data)
{
    struct host_find_data *find_data = (struct host_find_data *)data;
    struct hisi_usb *hiusb = dev_get_drvdata(dev);
    if (hiusb->host_no == find_data->host_no) {
        find_data->hiusb = hiusb;
        return 1;
    }
    return 0;
}
 
struct hisi_usb *get_hisi_usb(u32 host)
{
    struct host_find_data find_data = {0};
    int ret;
    find_data.host_no = host;
 
    ret = driver_for_each_device(&usb_hisi_driver.driver, NULL, &find_data, match_hisi_usb);
    if (!ret) {
        return NULL;
    }
    return find_data.hiusb;
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("usb_hisi driver");
MODULE_ALIAS("platform:usb_hisi");

