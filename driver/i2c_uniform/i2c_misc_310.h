/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
 * Description: i2c_misc_310 header file
 * Author: huawei
 * Create: 2023-10-12
 */
#ifndef __I2C_MISC_310_H_
#define __I2C_MISC_310_H_

 enum {
    ISP_I2C0,
    ISP_I2C1,
    ISP_I2C2,
    ISP_I2C3,
    AO_I2C4,
    AO_I2C5,
    AO_I2C6,
    PERI_I2C7,
    PERI_I2C8,
    AO_I2C9,
    AO_HISS_I2C10,
    PERI_I2C11,
    PERI_I2C12,
    MEDIA_I2C_DSI,
    I2C_MAX_NUM,
};

enum {
    PERI_SUB_CTRL,
    AO_HISS_SUB_CTRL,
    ISP_SUB_CTRL,
    AO_SUB_CTRL,
    MEDIA_SUB_CTRL,
    SUB_CTRL_MAX,
};

#define PERI_I2C_RESET_REQ_REG               (0xa08)  /* Soft reset request control register */
#define PERI_I2C_RESET_DREQ_REG              (0xa0c)  /* Soft reset derequest control register */
#define PERI_I2C_RESET_ST_REG                (0x5a08)  /* Soft reset status register */

#define MEDIA_I2C_RESET_REQ_REG               (0xc08)  /* Soft reset request control register */
#define MEDIA_I2C_RESET_DREQ_REG              (0xc0c)  /* Soft reset derequest control register */
#define MEDIA_I2C_RESET_ST_REG                (0x5c08)  /* Soft reset status register */

#define AO_HISS_I2C_RESET_REQ_REG               (0xa60)  /* Soft reset request control register */
#define AO_HISS_I2C_RESET_DREQ_REG              (0xa64)  /* Soft reset derequest control register */
#define AO_HISS_I2C_RESET_ST_REG                (0x5a60)  /* Soft reset status register */

#define ISP_I2C_RESET_REQ_REG               (0xa10)  /* Soft reset request control register */
#define ISP_I2C_RESET_DREQ_REG              (0xa14)  /* Soft reset derequest control register */
#define ISP_I2C_RESET_ST_REG                (0x5a10)  /* Soft reset status register */

#define AO_I2C4_RESET_REQ_REG               (0xa00)  /* Soft reset request control register */
#define AO_I2C4_RESET_DREQ_REG              (0xa04)  /* Soft reset derequest control register */
#define AO_I2C4_RESET_ST_REG                (0x5a00)  /* Soft reset status register */

#define AO_I2C5_RESET_REQ_REG               (0xa08)  /* Soft reset request control register */
#define AO_I2C5_RESET_DREQ_REG              (0xa0c)  /* Soft reset derequest control register */
#define AO_I2C5_RESET_ST_REG                (0x5a08)  /* Soft reset status register */

#define AO_I2C6_RESET_REQ_REG               (0xa10)  /* Soft reset request control register */
#define AO_I2C6_RESET_DREQ_REG              (0xa14)  /* Soft reset derequest control register */
#define AO_I2C6_RESET_ST_REG                (0x5a10)  /* Soft reset status register */

#define AO_I2C9_RESET_REQ_REG               (0xa18)  /* Soft reset request control register */
#define AO_I2C9_RESET_DREQ_REG              (0xa1c)  /* Soft reset derequest control register */
#define AO_I2C9_RESET_ST_REG                (0x5a18)  /* Soft reset status register */

#define PERI_I2C_ICG_EN_REG               (0x310)  /* Clock enable control register */
#define PERI_I2C_ICG_DIS_REG              (0x314)  /* Clock disable control register */
#define PERI_I2C_ICG_ST_REG               (0x5310)  /* Clock enable status register */

#define MEDIA_I2C_ICG_EN_REG               (0x3b8)  /* Clock enable control register */
#define MEDIA_I2C_ICG_DIS_REG              (0x3bc)  /* Clock disable control register */
#define MEDIA_I2C_ICG_ST_REG               (0x53b8)  /* Clock enable status register */

#define AO_HISS_I2C_ICG_EN_REG               (0x3B0)
#define AO_HISS_I2C_ICG_DIS_REG              (0x3B4)
#define AO_HISS_I2C_ICG_ST_REG               (0x53B0)

#define ISP_I2C_ICG_EN_REG               (0x318)
#define ISP_I2C_ICG_DIS_REG              (0x31c)
#define ISP_I2C_ICG_ST_REG               (0x5318)

#define AO_I2C4_ICG_EN_REG               (0x308)
#define AO_I2C4_ICG_DIS_REG              (0x30c)
#define AO_I2C4_ICG_ST_REG               (0x5308)

#define AO_I2C5_ICG_EN_REG               (0x310)
#define AO_I2C5_ICG_DIS_REG              (0x314)
#define AO_I2C5_ICG_ST_REG               (0x5310)

#define AO_I2C6_ICG_EN_REG               (0x318)
#define AO_I2C6_ICG_DIS_REG              (0x31c)
#define AO_I2C6_ICG_ST_REG               (0x5318)

#define AO_I2C9_ICG_EN_REG               (0x320)
#define AO_I2C9_ICG_DIS_REG              (0x324)
#define AO_I2C9_ICG_ST_REG               (0x5320)

struct i2c_crg_reset_cell {
    u32 reset_req_offset;
    u32 reset_dreq_offset;
    u32 reset_st_offset;
    u32 reset_mask;
};

struct i2c_crg_clkgate {
    u32 id;
    const char *name;
    u32 en_offset;
    u32 dis_offset;
    u32 st_offset;
    u32 mask;
};

struct i2c_subctrl_recover_cell {
    u32 cfg_offset;
    u32 clk_mux_sel_mask;
    u32 clk_oe_cfg_mask;
    u32 dat_mux_sel_mask;
    u32 scl_cfg_mask;
    u32 dat_oe_cfg_mask;
};

#define ISP_I2C0_CFG_REG               (0x100)
#define ISP_I2C1_CFG_REG               (0x104)
#define ISP_I2C2_CFG_REG               (0x108)
#define ISP_I2C3_CFG_REG               (0x10C)
#define AO_I2C4_CFG_REG               (0x104)
#define AO_I2C5_CFG_REG               (0x108)
#define AO_I2C6_CFG_REG               (0x10c)
#define PERI_I2C7_CFG_REG               (0x104)
#define PERI_I2C8_CFG_REG               (0x108)
#define AO_I2C9_CFG_REG               (0x110)
#define AO_HISS_I2C10_CFG_REG               (0x110)
#define PERI_I2C11_CFG_REG               (0x10c)
#define PERI_I2C12_CFG_REG               (0x110)
#define MEDIA_I2C_DSI_CFG_REG               (0x104)
#define I2C_CLK_MUX_SEL   BIT(0)
#define I2C_CLK_OE_CFG   BIT(1)
#define I2C_DAT_MUX_SEL   BIT(2)
#define I2C_SCL_CFG   BIT(3)
#define I2C_DAT_OE_CFG   BIT(5)

extern const u64 g_subctrl_pbase[SUB_CTRL_MAX];
extern const struct i2c_crg_reset_cell g_subctrl_resets[I2C_MAX_NUM];
extern const struct i2c_crg_clkgate g_subctrl_clkgates[I2C_MAX_NUM];
extern struct i2c_subctrl_recover_cell g_i2c_recover_cfg[I2C_MAX_NUM];
int bus_id_to_subctrl_id(u32 bus_id);

#endif