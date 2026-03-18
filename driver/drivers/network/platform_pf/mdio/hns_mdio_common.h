/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 * Description: mdio
 * Author: huawei
 * Create: 2023-03-20
 */

#ifndef __HNS_MDIO_COMMON_H__
#define __HNS_MDIO_COMMON_H__

#include <linux/errno.h>
#include <linux/etherdevice.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mfd/syscon.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/netdevice.h>
#include <linux/of_address.h>
#include <linux/of.h>
#include <linux/of_mdio.h>
#include <linux/of_platform.h>
#include <linux/phy.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/cdev.h>
#include <linux/version.h>

#define MDIO_TIMEOUT 100

#define drv_printk(level, module, fmt, ...) \
    printk(level " [%s] [%s %d] " fmt, module, __func__, __LINE__, ##__VA_ARGS__)

#define drv_err(module, fmt...) drv_printk(KERN_ERR, module, fmt)
#define drv_warn(module, fmt...) drv_printk(KERN_WARNING, module, fmt)
#define drv_info(module, fmt...) drv_printk(KERN_INFO, module, fmt)
#define drv_debug(module, fmt...) drv_printk(KERN_DEBUG, module, fmt)

#define MDIO_LOG_PREFIX "mdio"
#define mdio_err(fmt...) drv_err(MDIO_LOG_PREFIX, fmt)
#define mdio_warn(fmt...) drv_warn(MDIO_LOG_PREFIX, fmt)
#define mdio_info(fmt...) drv_info(MDIO_LOG_PREFIX, fmt)
#define mdio_debug(fmt...) drv_debug(MDIO_LOG_PREFIX, fmt)

/* mdio reg */
#define MDIO_COMMAND_REG 0x0
#define MDIO_ADDR_REG 0x4
#define MDIO_WDATA_REG 0x8
#define MDIO_RDATA_REG 0xc
#define MDIO_STA_REG 0x10

/* cfg phy bit map */
#define MDIO_CMD_DEVAD_M 0x1f
#define MDIO_CMD_DEVAD_S 0
#define MDIO_CMD_PRTAD_M 0x1f
#define MDIO_CMD_PRTAD_S 5
#define MDIO_CMD_OP_S 10
#define MDIO_CMD_ST_S 12
#define MDIO_CMD_START_B 14

#define MDIO_ADDR_DATA_M 0xffff
#define MDIO_ADDR_DATA_S 0

#define MDIO_WDATA_DATA_M 0xffff
#define MDIO_WDATA_DATA_S 0

#define MDIO_RDATA_DATA_M 0xffff
#define MDIO_RDATA_DATA_S 0

#define MDIO_STATE_STA_B 0

#define MDIO_CHECK_SET_ST 1
#define MDIO_CHECK_CLR_ST 0

enum {
    MEM_MDIO_IOBASE,
    MEM_GPIO2_IOBASE,
    MEM_SYSCTRL_IOBASE,
    MEM_IOBASE_MAX
};

enum mdio_st_clause {
    MDIO_ST_CLAUSE_45 = 0,
    MDIO_ST_CLAUSE_22
};

enum mdio_c22_op_seq {
    MDIO_C22_WRITE = 1,
    MDIO_C22_READ = 2
};

enum mdio_c45_op_seq {
    MDIO_C45_WRITE_ADDR = 0,
    MDIO_C45_WRITE_DATA,
    MDIO_C45_READ_INCREMENT,
    MDIO_C45_READ
};

struct peri_sc_mdio_reg {
    u16 mdio_clk_en;
    u16 mdio_clk_dis;
    u16 mdio_reset_req;
    u16 mdio_reset_dreq;
    u16 mdio_ctrl;
    u16 mdio_clk_st;
    u16 mdio_reset_st;
};

struct hns_mdio_device {
    void *vbase; /* mdio reg base address */
    struct regmap *subctrl_vbase;
    const struct peri_sc_mdio_reg *sc_reg;

    struct platform_device *pdev;
    struct mii_bus *bus;

    struct cdev mdio_cdev;
    struct class *mdio_cdev_cls;
    dev_t mdio_devno;
};

void mdio_write_reg(void *base, u32 reg, u32 value);
void mdio_set_reg_field(void *base, u32 reg, u32 mask, u32 shift, u32 val);
u32 mdio_get_reg_field(void *base, u32 reg, u32 mask, u32 shift);

#define MDIO_WRITE_REG(a, reg, value) mdio_write_reg((a)->vbase, (reg), (value))

#define mdio_set_field(origin, mask, shift, val) do { \
        (origin) &= (~((mask) << (shift)));        \
        (origin) |= (((val) & (mask)) << (shift)); \
    } while (0)

#define mdio_get_field(origin, mask, shift) (((origin) >> (shift)) & (mask))
#define MDIO_SET_REG_FIELD(dev, reg, mask, shift, val) mdio_set_reg_field((dev)->vbase, (reg), (mask), (shift), (val))
#define MDIO_GET_REG_FIELD(dev, reg, mask, shift) mdio_get_reg_field((dev)->vbase, (reg), (mask), (shift))
#define MDIO_GET_REG_BIT(dev, reg, bit) mdio_get_reg_field((dev)->vbase, (reg), 0x1ull, (bit))


int hns_mdio_write(struct mii_bus *bus, int phy_id, int regnum, u16 data);
int hns_mdio_read(struct mii_bus *bus, int phy_id, int regnum);

#endif // __HNS_MDIO_COMMON_H__