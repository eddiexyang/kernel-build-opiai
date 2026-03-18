/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 * Description: mdio
 * Author: huawei
 * Create: 2023-03-20
 */

#include "hns_mdio_common.h"

#ifndef DEFINE_HNS_LLT
#define STATIC static
#else
#define STATIC
#endif

void mdio_write_reg(void *base, u32 reg, u32 value)
{
    u8 __iomem *reg_addr;

    reg_addr = (u8 __iomem *)base;

    writel_relaxed(value, reg_addr + reg);
}

STATIC u32 mdio_read_reg(void *base, u32 reg)
{
    u8 __iomem *reg_addr;

    reg_addr = (u8 __iomem *)base;

    return readl_relaxed(reg_addr + reg);
}

void mdio_set_reg_field(void *base, u32 reg, u32 mask, u32 shift, u32 val)
{
    u32 origin = mdio_read_reg(base, reg);

    mdio_set_field(origin, mask, shift, val);
    mdio_write_reg(base, reg, origin);
}

u32 mdio_get_reg_field(void *base, u32 reg, u32 mask, u32 shift)
{
    u32 origin;

    origin = mdio_read_reg(base, reg);
    return mdio_get_field(origin, mask, shift);
}

STATIC int hns_mdio_wait_ready(struct mii_bus *bus, bool check_sta)
{
    struct hns_mdio_device *mdio_dev = bus->priv;
    u32 cmd_reg_value = 0;
    u16 reg_val;
    int i;

    /* waitting for MDIO_COMMAND_REG 's mdio_start==0 */
    /* after that can do read or write */
    for (i = 0; i < MDIO_TIMEOUT; i++) {
        cmd_reg_value = MDIO_GET_REG_BIT(mdio_dev, MDIO_COMMAND_REG, MDIO_CMD_START_B);
        if (!cmd_reg_value) {
            return 0;
        }
        udelay(1);

        /* check MDIO_STA_REG's mdio_sta==1 */
        if (check_sta) {
            reg_val = MDIO_GET_REG_BIT(mdio_dev, MDIO_STA_REG, MDIO_STATE_STA_B);
            if (reg_val) {
                return -EBUSY;
            }
        }
    }

    return -ETIMEDOUT;
}

STATIC void hns_mdio_cmd_write(struct hns_mdio_device *mdio_dev, u8 is_c45, u8 op, u8 phy_id, u16 cmd)
{
    u8 st = is_c45 ? MDIO_ST_CLAUSE_45 : MDIO_ST_CLAUSE_22;
    u32 cmd_reg_value;

    cmd_reg_value = st << MDIO_CMD_ST_S;
    cmd_reg_value |= op << MDIO_CMD_OP_S;
    cmd_reg_value |= (phy_id & MDIO_CMD_PRTAD_M) << MDIO_CMD_PRTAD_S;
    cmd_reg_value |= (cmd & MDIO_CMD_DEVAD_M) << MDIO_CMD_DEVAD_S;
    cmd_reg_value |= 1 << MDIO_CMD_START_B;

    MDIO_WRITE_REG(mdio_dev, MDIO_COMMAND_REG, cmd_reg_value);
}

/* *
 * hns_mdio_write - access phy register
 * @bus: mdio bus
 * @phy_id: phy id
 * @regnum: register num
 * @value: register value
 *
 * Return 0 on success, negative on failure
 */
int hns_mdio_write(struct mii_bus *bus, int phy_id, int regnum, u16 data)
{
    struct hns_mdio_device *mdio_dev = (struct hns_mdio_device *)bus->priv;
    u8 is_c45 = (u8)!!((u32)regnum & MII_ADDR_C45);
    u8 devad = (u8)(((u32)regnum >> 16) & 0x1f);
    u16 reg = (u16)((u32)regnum & 0xffff);
    u16 cmd_reg_cfg;
    int ret;
    u8 op;

    /* wait for ready */
    ret = hns_mdio_wait_ready(bus, false);
    if (ret) {
        mdio_err("MDIO bus is busy, ret = %d\n", ret);
        return ret;
    }

    if (!is_c45) {
        cmd_reg_cfg = reg;
        op = MDIO_C22_WRITE;
    } else {
        /* config the cmd-reg to write addr */
        MDIO_SET_REG_FIELD(mdio_dev, MDIO_ADDR_REG, MDIO_ADDR_DATA_M, MDIO_ADDR_DATA_S, reg);

        hns_mdio_cmd_write(mdio_dev, is_c45, MDIO_C45_WRITE_ADDR, phy_id, devad);

        /* check for read or write opt is finished */
        ret = hns_mdio_wait_ready(bus, false);
        if (ret) {
            mdio_err("MDIO bus is busy, ret = %d\n", ret);
            return ret;
        }

        /* config the data needed writing */
        cmd_reg_cfg = devad;
        op = MDIO_C45_WRITE_ADDR;
    }

    MDIO_SET_REG_FIELD(mdio_dev, MDIO_WDATA_REG, MDIO_WDATA_DATA_M, MDIO_WDATA_DATA_S, data);

    hns_mdio_cmd_write(mdio_dev, is_c45, op, phy_id, cmd_reg_cfg);

    return 0;
}

/* *
 * hns_mdio_read - access phy register
 * @bus: mdio bus
 * @phy_id: phy id
 * @regnum: register num
 * @value: register value
 *
 * Return phy register value
 */
int hns_mdio_read(struct mii_bus *bus, int phy_id, int regnum)
{
    struct hns_mdio_device *mdio_dev = (struct hns_mdio_device *)bus->priv;
    u8 is_c45 = (u8)!!((u32)regnum & MII_ADDR_C45);
    u8 devad = (u8)(((u32)regnum >> 16) & 0x1f);
    u16 reg = (u16)((u32)regnum & 0xffff);
    u16 reg_val;
    int ret;

    /* Step 1: wait for ready */
    ret = hns_mdio_wait_ready(bus, false);
    if (ret) {
        mdio_err("MDIO bus is busy, ret = %d\n", ret);
        return ret;
    }

    if (!is_c45) {
        hns_mdio_cmd_write(mdio_dev, is_c45, MDIO_C22_READ, phy_id, reg);
    } else {
        MDIO_SET_REG_FIELD(mdio_dev, MDIO_ADDR_REG, MDIO_ADDR_DATA_M, MDIO_ADDR_DATA_S, reg);

        /* Step 2: config the cmd-reg to write addr */
        hns_mdio_cmd_write(mdio_dev, is_c45, MDIO_C45_WRITE_ADDR, phy_id, devad);

        /* Step 3: check for read or write opt is finished */
        ret = hns_mdio_wait_ready(bus, false);
        if (ret) {
            mdio_err("MDIO bus is busy, ret = %d\n", ret);
            return ret;
        }

        hns_mdio_cmd_write(mdio_dev, is_c45, MDIO_C45_WRITE_ADDR, phy_id, devad);
    }

    /* Step 5: waitting for MDIO_COMMAND_REG 's mdio_start==0 again */
    /* check for read or write opt is finished */
    ret = hns_mdio_wait_ready(bus, false);
    if (ret) {
        mdio_err("MDIO bus is busy, ret = %d\n", ret);
        return ret;
    }

#if (!defined(CONFIG_PLATFORM_MDC) && !defined(DEFINE_HNS_LLT))
    reg_val = MDIO_GET_REG_BIT(mdio_dev, MDIO_STA_REG, MDIO_STATE_STA_B);
    if (reg_val) {
        mdio_err("ERROR! MDIO Read failed!\n");
        return -EBUSY;
    }
#endif

    /* Step 6: get out data */
    reg_val = (int)MDIO_GET_REG_FIELD(mdio_dev, MDIO_RDATA_REG, MDIO_RDATA_DATA_M, MDIO_RDATA_DATA_S);
    return reg_val;
}
