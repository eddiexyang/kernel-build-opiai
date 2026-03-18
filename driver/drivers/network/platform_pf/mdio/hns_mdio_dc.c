/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 * Description: mdio
 * Author: huawei
 * Create: 2023-03-20
 */

#include "hns_mdio_dc.h"

int hns_mac_mdio_init(struct hclge_plf_dev *hdev)
{
#define PHY_INEXISTENT  255
#define TEMP_ID_LEN     2
#define PHY_ID_OFFSET   16
    struct platform_device *pdev = hdev->pdev;
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    struct hns_mdio_device *mdio_dev = NULL;
    struct phy_device *phydev;
    struct mii_bus *mdio_bus;
    u32 tmp_id[TEMP_ID_LEN] = {0};
    u32 phy_id;
    int ret;

    if (hdev->hw.mac.phy_addr == PHY_INEXISTENT) {
        dev_info(&hdev->pdev->dev, "No phy device is connected to mdio bus\n");
        return 0;
    }

    mdio_dev = (struct hns_mdio_device *)devm_kzalloc(&pdev->dev, sizeof(struct hns_mdio_device), GFP_KERNEL);
    if (mdio_dev == NULL) {
        return -ENOMEM;
    }

    mdio_bus = devm_mdiobus_alloc(&pdev->dev);
    if (mdio_bus == NULL) {
        mdio_err("Failed to alloc MDIO bus\n");
        return -ENOMEM;
    }

    mdio_dev->vbase = hdev->hw.mac.mdio_base;
    mdio_bus->name = "hisilicon,hi1910B-mdio";
    mdio_bus->read = hns_mdio_read;
    mdio_bus->write = hns_mdio_write;
    (void)snprintf_s(mdio_bus->id, MII_BUS_ID_SIZE, MII_BUS_ID_SIZE - 1, "%s-%s", "mii", dev_name(&hdev->pdev->dev));

    mdio_bus->parent = &hdev->pdev->dev;
    mdio_bus->priv = mdio_dev;

    mdio_bus->phy_mask = ~(1 << mac->phy_addr);
    ret = mdiobus_register(mdio_bus);
    if (ret) {
        dev_err(mdio_bus->parent, "Failed to register MDIO bus ret = %d\n", ret);
        return ret;
    }

    phydev = mdiobus_get_phy(mdio_bus, mac->phy_addr);
    if (!phydev) {
        dev_err(mdio_bus->parent, "Failed to get phy device\n");
        mdiobus_unregister(mdio_bus);
        return -EIO;
    }

    mac->phydev = phydev;
    mac->mdio_bus = mdio_bus;

    ret = phy_read(phydev, PHY_ID0_REG);
    if (ret < 0) {
        dev_err(mdio_bus->parent, "Failed to read phy id 0 register, ret = %d\n", ret);
        mdiobus_unregister(mdio_bus);
        return ret;
    }
    tmp_id[0] = ret;

    ret = phy_read(phydev, PHY_ID1_REG);
    if (ret < 0) {
        dev_err(mdio_bus->parent, "Failed to read phy id 1 register, ret = %d\n", ret);
        mdiobus_unregister(mdio_bus);
        return ret;
    }
    tmp_id[1] = ret;

    phy_id = ((tmp_id[0] << PHY_ID_OFFSET) | tmp_id[1]);

    if (phydev->phy_id != phy_id) {
        phydev->phy_id = phy_id;
    }

    pr_info("PHY_ID : 0x%x 0x%x\n", phy_id, phydev->phy_id);

    return 0;
}
