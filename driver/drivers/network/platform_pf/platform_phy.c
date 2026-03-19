/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: platform_phy
 * Author: huawei
 * Create: 2021-12-28
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/phy.h>
#include <linux/platform_device.h>

#include "kcompat.h"
#include "hclge_plf_main.h"
#include "platform_mac.h"

#ifndef DEFINE_HNS_LLT
#define STATIC static
#else
#define STATIC
#endif

struct hns_phy_driver {
    u32 phy_id;
    u32 phy_id_mask;
    const char *name;

    int (*config_init)(struct phy_device *phydev);
};

#define MOTORCOMM_PHY_ID_MASK           0x001fffff
#define MOTORCOMM_PHY_ID_8521           0x11a

#define REALTEK_PHY_ID_MASK             0x001fffff
#define REALTEK_PHY_ID_8211F            0x001cc916
#define REALTEK_PHY_ID_9010             0x001ccb30 // only 1000M/100M

#define MARVELL_PHY_ID_MASK 0xfffffff0
#define MARVELL_PHY_ID_88E1510 0x01410dd0
#define MARVELL_PHY_ID_88E1545 0x01410ea0

#define MII_MARVELL_GEN_PAGE 18
#define MII_MARVELL_PHY_PAGE_ADDR 22
#define MII_MARVELL_COPPER_PAGE 0x00
#define MII_MARVELL_MSCR_PAGE 0x02
#define MII_MARVELL_LED_PAGE 0x03

#define MII_M1011_PHY_STATUS 0x11
#define MII_M1011_PHY_STATUS_1000 0x8000
#define MII_M1011_PHY_STATUS_100 0x4000
#define MII_M1011_PHY_STATUS_SPD_MASK 0xc000
#define MII_M1011_PHY_STATUS_FULLDUPLEX 0x2000
#define MII_M1011_PHY_STATUS_RESOLVED 0x0800
#define MII_M1011_PHY_STATUS_LINK 0x0400

#define MII_PHY_LED_CTRL 16
#define MII_88E1510_PHY_LED_DEF 0x1177

#define MII_88E1318S_PHY_MSCR1_REG 16
#define MII_88E1318S_PHY_MSCR1_PAD_ODD (1 << 6)

#define MII_88E1510_GEN_CTRL_REG_1 0x14
#define MII_88E1510_GEN_CTRL_REG_1_MODE_MASK 0x7
#define MII_88E1510_GEN_CTRL_REG_1_MODE_SGMII 0x1 /* SGMII to copper */
#define MII_88E1510_GEN_CTRL_REG_1_MODE_RGMII 0x0 /* RGMII to copper */
#define MII_88E1510_GEN_CTRL_REG_1_RESET 0x8000   /* Soft reset */

#define MII_88E1510_COPPER_CTRL_REG 0
#define MII_88E1510_COPPER_CTRL_REG_VAL 0x1340 /* 1G + restart auto nego */

#define RTL821x_PAGE_SELECT             0x1f

#define RTL8211F_PAGE_A42               0xa42
#define RTL8211F_PAGE_D04               0xd04
#define RTL8211F_PAGE_D08               0xd08
#define RTL8211F_PAGE_DC8               0xdc8

#define RTL8211F_FIFO_REG               0x10
#define RTL8211F_TX_DELAY_REG           0x11
#define RTL8211F_SGMII_NWAY_REG         0x12
#define RTL8211F_SGMII_NWAY_BYPASS      0x14

#define RTL8211F_NWAY_DISABLE           (1 << 8)
#define RTL8211F_NWAY_BYPASS            (1 << 1)

#define RTL8211F_TX_DELAY               (1 << 8)
#define RTL8211F_FIFO_RESET             (1 << 6)

#define MII_BMCR            0x00    /* Basic mode control register */
#define MII_BMSR            0x01    /* Basic mode status register  */
#define BMCR_FULLDPLX       0x0100  /* Full duplex                 */
#define BMCR_ISOLATE        0x0400  /* Isolate data paths from MII */
#define BMCR_RESET          0x8000  /* Reset to default state      */
#define BMSR_LSTATUS        0x0004  /* Link status                 */

int hns_rtl8211f_config_init(struct phy_device *phydev);
int hns_marvel_config_init(struct phy_device *phydev);
int hns_yt8521_config_init(struct phy_device *phydev);
int hns_rtl9010_config_init(struct phy_device *phydev);
static struct hns_phy_driver g_phy_driver[] = {
    {
        .phy_id                 = MARVELL_PHY_ID_88E1510,
        .phy_id_mask            = MARVELL_PHY_ID_MASK,
        .name                   = "Marvell 88E1510",
        .config_init            = &hns_marvel_config_init,
    },
    {
        .phy_id                 = MARVELL_PHY_ID_88E1545,
        .phy_id_mask            = MARVELL_PHY_ID_MASK,
        .name                   = "Marvell 88E1545",
        .config_init            = &hns_marvel_config_init,
    },
#ifdef CONFIG_PLATFORM_ASIC
    {
        .phy_id                 = REALTEK_PHY_ID_8211F,
        .phy_id_mask            = REALTEK_PHY_ID_MASK,
        .name                   = "RTL8211F",
        .config_init            = &hns_rtl8211f_config_init,
    },
    {
        .phy_id                 = REALTEK_PHY_ID_9010,
        .phy_id_mask            = REALTEK_PHY_ID_MASK,
        .name                   = "RTL9010",
        .config_init            = &hns_rtl9010_config_init,
    },
    {
        .phy_id                 = MOTORCOMM_PHY_ID_8521,
        .phy_id_mask            = MOTORCOMM_PHY_ID_MASK,
        .name                   = "YT8521",
        .config_init            = &hns_yt8521_config_init,
    }
#endif
};

STATIC int hns_phy_modify(struct phy_device *phydev, u32 regnum, u16 mask, u16 set)
{
    int value, ret;

    ret = phy_read(phydev, regnum);
    if (ret < 0) {
        pr_err("failed to read phy reg:0x%x, ret:%d\n", regnum, ret);
        return ret;
    }

    value = (ret & ~mask) | set;
    if (value == ret)
        return 0;

    ret = phy_write(phydev, regnum, value);
    if (ret != 0) {
        pr_err("failed to write phy reg:0x%x, ret:%d\n", regnum, ret);
    }

    return ret;
}

static int hns_phy_soft_reset_wait_done(struct phy_device *phydev)
{
#define PHY_RESET_TIMES     12
#define PHY_RESET_DELAY     50

    u32 retry_time = PHY_RESET_TIMES;
    int val;

    do {
        mdelay(PHY_RESET_DELAY);

        val = phy_read(phydev, MII_BMCR);
        if (val < 0) {
            return val;
        }
#ifdef DEFINE_HNS_LLT
        val = 0;
#endif
        if (!(val & BMCR_RESET)) {
            return 0;
        }
    } while (--retry_time);

    return -1;
}

STATIC int hns_phy_soft_reset(struct phy_device *phydev)
{
    int ret;

    ret = hns_phy_modify(phydev, MII_BMCR, BMCR_ISOLATE, BMCR_RESET);
    if (ret != 0) {
        return ret;
    }

    return hns_phy_soft_reset_wait_done(phydev);
}

#ifdef CONFIG_PLATFORM_ASIC
int hns_rtl9010_config_init(struct phy_device *phydev)
{
    return 0;
}

STATIC int hns_yt8521_ext_reg_cfg(struct phy_device *phydev, u32 reg, u16 set)
{
#define MOTORCOMM_EXT_OFFSET_REG 0x1e
#define MOTORCOMM_EXT_DATA_REG 0x1f
    int ret;

    ret = hns_phy_modify(phydev, MOTORCOMM_EXT_OFFSET_REG, 0xffff, (u16)reg);
    if (ret != 0) {
        return ret;
    }

    return hns_phy_modify(phydev, MOTORCOMM_EXT_DATA_REG, 0xffff, set);
}

STATIC int hns_yt8521_led_cfg(struct phy_device *phydev)
{
#define MOTORCOMM_LED0_CFG_EX_REG 0xa00c
#define MOTORCOMM_LED2_CFG_EX_REG 0xa00e
    int ret;

    /* led0 used to identify port link */
    ret = hns_yt8521_ext_reg_cfg(phydev, MOTORCOMM_LED0_CFG_EX_REG, 0x70); // 0x70 : bit4/5/6 -> 10M/100M/1000M
    if (ret != 0) {
        return ret;
    }

    /* led2 used to identify traffic passing through. */
    return hns_yt8521_ext_reg_cfg(phydev, MOTORCOMM_LED2_CFG_EX_REG, 0x2600); // 0x2600 : bit9/10/13 -> rx/tx/blink
}

STATIC int hns_yt8521_delay_and_driver_cfg(struct phy_device *phydev)
{
#define MOTORCOMM_RGMII_CONFIG1_REG 0xa003
#define MOTORCOMM_DRIVE_STRENGTH_REG 0xa010
    int ret;

    /* Hardware test signal configuration : rx delay */
    ret = hns_yt8521_ext_reg_cfg(phydev, MOTORCOMM_RGMII_CONFIG1_REG, 0x14f1);
    if (ret != 0) {
        return ret;
    }

    /* Hardware test signal configuration : driver */
    return hns_yt8521_ext_reg_cfg(phydev, MOTORCOMM_DRIVE_STRENGTH_REG, 0x3ff);
}

int hns_yt8521_config_init(struct phy_device *phydev)
{
    int ret;

    ret = hns_yt8521_delay_and_driver_cfg(phydev);
    if (ret != 0) {
        pr_err("failed to config delay & driver, ret:%d\n", ret);
        return ret;
    }

    return hns_yt8521_led_cfg(phydev);
}

static int hns_rtl8211f_rgmii_txdly_cfg(struct phy_device *phydev)
{
    int ret;

    ret = hns_phy_modify(phydev, RTL821x_PAGE_SELECT, 0xffff, RTL8211F_PAGE_D08);
    if (ret != 0) {
        return ret;
    }

    ret = hns_phy_modify(phydev, RTL8211F_TX_DELAY_REG, RTL8211F_TX_DELAY, 0);
    if (ret != 0) {
        return ret;
    }

    ret = hns_phy_modify(phydev, RTL821x_PAGE_SELECT, 0xffff, RTL8211F_PAGE_A42);
    if (ret != 0) {
        return ret;
    }

    return 0;
}

static int hns_rtl8211f_rgmii_fifo_reset_cfg(struct phy_device *phydev)
{
#define PHY_RGMII_FIFO_RESET_DELAY 120
#define PHY_RGMII_FIFO_DERESET_DELAY 10
    int ret;

    mdelay(PHY_RGMII_FIFO_RESET_DELAY);

    ret = hns_phy_modify(phydev, RTL821x_PAGE_SELECT, 0xffff, RTL8211F_PAGE_D08);
    if (ret != 0) {
        return ret;
    }

    ret = hns_phy_modify(phydev, RTL8211F_FIFO_REG, RTL8211F_FIFO_RESET, 0);
    if (ret != 0) {
        return ret;
    }

    mdelay(PHY_RGMII_FIFO_DERESET_DELAY);

    ret = hns_phy_modify(phydev, RTL8211F_FIFO_REG, RTL8211F_FIFO_RESET, 0x40);
    if (ret != 0) {
        return ret;
    }

    ret = hns_phy_modify(phydev, RTL821x_PAGE_SELECT, 0xffff, RTL8211F_PAGE_A42);
    if (ret != 0) {
        return ret;
    }

    return 0;
}

static int hns_rtl8211f_led_cfg(struct phy_device *phydev)
{
#define RTL8211F_LED_SEL        0x6c0b /* link: LED0  ACT: LED2 */
#define RTL8211F_LED_EEE_EN     0xc
#define RTL8211F_LCR_REG        0x10
#define RTL8211F_EEELCR_REG     0x11

    int ret;

    ret = hns_phy_modify(phydev, RTL821x_PAGE_SELECT, 0xffff, RTL8211F_PAGE_D04);
    if (ret != 0) {
        return ret;
    }

    ret = hns_phy_modify(phydev, RTL8211F_LCR_REG, 0xffff, RTL8211F_LED_SEL);
    if (ret != 0) {
        return ret;
    }

    ret = hns_phy_modify(phydev, RTL8211F_EEELCR_REG, 0xf, RTL8211F_LED_EEE_EN);
    if (ret != 0) {
        return ret;
    }

    ret = hns_phy_modify(phydev, RTL821x_PAGE_SELECT, 0xffff, RTL8211F_PAGE_A42);
    if (ret != 0) {
        return ret;
    }

    return 0;
}

int hns_rtl8211f_config_init(struct phy_device *phydev)
{
    int ret;

    ret = hns_phy_soft_reset(phydev);
    if (ret != 0) {
        pr_err("failed to reset rtl8211f phy, ret:%d\n", ret);
        return ret;
    }

    ret = hns_rtl8211f_rgmii_txdly_cfg(phydev);
    if (ret != 0) {
        pr_err("failed to cfg rtl8211f rgmii txdly, ret:%d\n", ret);
        return ret;
    }

    ret = hns_rtl8211f_rgmii_fifo_reset_cfg(phydev);
    if (ret != 0) {
        pr_err("failed to cfg rtl8211f rgmii fifo reset, ret:%d\n", ret);
        return ret;
    }

    ret = hns_rtl8211f_led_cfg(phydev);
    if (ret != 0) {
        pr_err("failed to cfg rtl8211f led, ret:%d\n", ret);
        return ret;
    }

    return 0;
}
#endif

int hns_marvel_config_init(struct phy_device *phydev)
{
    int phy_mode;
    int ret;

    ret = hns_phy_soft_reset(phydev);
    if (ret != 0) {
        pr_err("failed to reset marvel phy, ret:%d\n", ret);
        return ret;
    }

    /* set to page 18 */
    ret = hns_phy_modify(phydev, MII_MARVELL_PHY_PAGE_ADDR, 0xffff, MII_MARVELL_GEN_PAGE);
    if (ret != 0) {
        return ret;
    }

    phy_mode = MII_88E1510_GEN_CTRL_REG_1_MODE_SGMII;
    ret = hns_phy_modify(phydev, MII_88E1510_GEN_CTRL_REG_1, MII_88E1510_GEN_CTRL_REG_1_MODE_MASK, phy_mode);
    if (ret != 0) {
        return ret;
    }

    /* PHY reset is necessary after changing MODE[2:0] */
    ret = hns_phy_modify(
        phydev, MII_88E1510_GEN_CTRL_REG_1, MII_88E1510_GEN_CTRL_REG_1_RESET, MII_88E1510_GEN_CTRL_REG_1_RESET);
    if (ret != 0) {
        return ret;
    }

    /* set to page 0 */
    ret = hns_phy_modify(phydev, MII_MARVELL_PHY_PAGE_ADDR, 0xffff, 0);
    if (ret != 0) {
        return ret;
    }

    /* set copper ctrl reg */
    return hns_phy_modify(phydev, MII_88E1510_COPPER_CTRL_REG, 0xffff, MII_88E1510_COPPER_CTRL_REG_VAL);
}

int phy_hw_init(struct hclge_plf_dev *hdev)
{
    struct phy_device *phydev = hdev->hw.mac.phydev;
#ifndef DEFINE_HNS_LLT
#define PHY_INEXISTENT	255
    size_t size = sizeof(g_phy_driver) / sizeof(g_phy_driver[0]);
    struct hns_phy_driver *phydrv;
    size_t i;

    if (hdev->hw.mac.phy_addr == PHY_INEXISTENT) {
        return 0;
    }

    for (i = 0; i < size; i++) {
        phydrv = &g_phy_driver[i];
        if ((phydrv->phy_id & phydrv->phy_id_mask) == (phydev->phy_id & phydrv->phy_id_mask)) {
            return phydrv->config_init(phydev);
        }
    }

    return -1;
#else
    return hns_marvel_config_init(phydev);
#endif
}

#ifdef CONFIG_PLATFORM_ASIC
static void hclge_plf_mac_adjust_link(struct net_device *netdev)
{
    struct hnae3_handle *h = *((void **)netdev_priv(netdev));
    struct hclge_plf_vport *vport = hclge_plf_get_vport(h);
    struct hclge_plf_dev *hdev = vport->back;
    int duplex, speed;
    int ret;

    /* When phy link down, do nothing */
    if (netdev->phydev->link == 0) {
        return;
    }

    speed = netdev->phydev->speed;
    duplex = netdev->phydev->duplex;

    ret = hclge_plf_cfg_mac_speed_dup(hdev, speed, duplex);
    if (ret != 0)
        netdev_err(netdev, "failed to adjust link.\n");

    ret = hclge_plf_cfg_flowctrl(hdev);
    if (ret != 0)
        netdev_err(netdev, "failed to configure flow control.\n");
}

int hclge_plf_mac_connect_phy(struct hnae3_handle *handle)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;
    struct net_device *netdev = hdev->vport[0].nic.netdev;
    struct phy_device *phydev = hdev->hw.mac.phydev;
#ifdef HAS_LINK_MODE_OPS
    __ETHTOOL_DECLARE_LINK_MODE_MASK(mask) = { 0, };
#endif
    int ret;

    if (!phydev)
        return 0;

#ifdef MARVELL_PHY_LED0_LINK_LED1_ACTIVE
    phydev->dev_flags |= MARVELL_PHY_LED0_LINK_LED1_ACTIVE;
#endif

#ifdef HAS_LINK_MODE_OPS
    linkmode_clear_bit(ETHTOOL_LINK_MODE_FIBRE_BIT, phydev->supported);

    ret = phy_connect_direct(netdev, phydev, hclge_plf_mac_adjust_link, PHY_INTERFACE_MODE_SGMII);
    if (ret != 0) {
        netdev_err(netdev, "phy_connect_direct err.\n");
        return ret;
    }

    linkmode_copy(mask, hdev->hw.mac.supported);
    linkmode_and(phydev->supported, phydev->supported, mask);
    linkmode_copy(phydev->advertising, phydev->supported);

    /* supported flag is Pause and Asym Pause, but default advertising
        * should be rx on, tx on, so need clear Asym Pause in advertising
        * flag
        */
    linkmode_clear_bit(ETHTOOL_LINK_MODE_Asym_Pause_BIT, phydev->advertising);
#else
    phydev->supported &= ~SUPPORTED_FIBRE;

    ret = phy_connect_direct(netdev, phydev, hclge_plf_mac_adjust_link, PHY_INTERFACE_MODE_SGMII);
    if (ret != 0) {
        netdev_err(netdev, "phy_connect_direct err.\n");
        return ret;
    }

    phydev->supported &= *hdev->hw.mac.supported;
    phydev->advertising = phydev->supported;

    /* supported flag is Pause and Asym Pause, but default advertising
        * should be rx on, tx on, so need clear Asym Pause in advertising
        * flag
        */
    phydev->advertising &= ~ADVERTISED_Asym_Pause;
#endif

    phy_attached_info(phydev);

    return 0;
}

void hclge_plf_mac_disconnect_phy(struct hnae3_handle *handle)
{
    struct hclge_plf_vport *vport = hclge_plf_get_vport(handle);
    struct hclge_plf_dev *hdev = vport->back;
    struct phy_device *phydev = hdev->hw.mac.phydev;

    if (!phydev) {
        return;
    }

    phy_disconnect(phydev);
}

void hclge_plf_mac_start_phy(struct hclge_plf_dev *hdev)
{
    struct phy_device *phydev = hdev->hw.mac.phydev;

    if (!phydev) {
        return;
    }

    phy_loopback(phydev, false, 0);

    phy_start(phydev);
}

void hclge_plf_mac_stop_phy(struct hclge_plf_dev *hdev)
{
    struct net_device *netdev = hdev->vport[0].nic.netdev;
    struct phy_device *phydev = netdev->phydev;

    if (!phydev) {
        return;
    }

    phy_stop(phydev);
}

int hclge_plf_enable_phy_loopback(struct hclge_plf_dev *hdev, struct phy_device *phydev)
{
    int ret;

    if (!phydev->suspended) {
        ret = phy_suspend(phydev);
        if (ret != 0) {
            pr_err("failed to suspend phy, ret:%d\n", ret);
            return ret;
        }
    }

    hdev->hw.mac.duplex_last = phydev->duplex;

    ret = phy_set_bits(phydev, MII_BMCR, BMCR_FULLDPLX);
    if (ret != 0) {
        pr_err("failed to set phy bits(0x%x), ret:%d\n", (u32)MII_BMCR, ret);
        return ret;
    }

    ret = phy_resume(phydev);
    if (ret != 0) {
        pr_err("failed to resume phy, ret:%d\n", ret);
        return ret;
    }

    return genphy_loopback(phydev, true, 0);
}

int hclge_plf_disable_phy_loopback(struct hclge_plf_dev *hdev, struct phy_device *phydev)
{
    int ret;

    ret = genphy_loopback(phydev, false, 0);
    if (ret != 0) {
        pr_err("failed to set phy loopback, ret:%d\n", ret);
        return ret;
    }

    if (hdev->hw.mac.duplex_last == DUPLEX_HALF) {
        ret = phy_clear_bits(phydev, MII_BMCR, BMCR_FULLDPLX);
        if (ret != 0) {
            pr_err("failed to clear phy bits(0x%x), ret:%d\n", (u32)MII_BMCR, ret);
            return ret;
        }
    }

    return phy_suspend(phydev);
}

int hclge_plf_get_phy_link(struct phy_device *phydev)
{
    int status;

    /* Do a fake read */
    status = phy_read(phydev, MII_BMSR);
    if (status < 0) {
        return status;
    }

    /* Read link and autonegotiation status */
    status = phy_read(phydev, MII_BMSR);
    if (status < 0) {
        return status;
    }

    if ((status & BMSR_LSTATUS) == 0) {
        return 0;
    }

    return 1;
}
#endif
