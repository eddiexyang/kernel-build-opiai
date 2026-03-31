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
 * Description: i2c_misc_610 header file
 * Author: huawei
 * Create: 2023-10-12
 */
#ifndef __I2C_MISC_610_H_
#define __I2C_MISC_610_H_

enum {
    MEDIA_I2C0,  /* bus0 */
    AO_I2C1,     /* bus1 */
    MEDIA_I2C2,  /* bus2 */
    ISP_I2C3,    /* bus3 */
    MEDIA_I2C8,  /* bus4 */
    PERI_I2C9,   /* bus5 */
    MEDIA_I2C10, /* bus6 */
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

#define MEDIA_I2C0_RESET_REQ_REG            (0xe88)  /* Soft reset request control register */
#define MEDIA_I2C0_RESET_DREQ_REG           (0xe8c)  /* Soft reset derequest control register */
#define MEDIA_I2C0_RESET_ST_REG             (0x5e88)  /* Soft reset status register */
#define MEDIA_I2C0_ICG_EN_REG               (0x3b0)  /* Clock enable control register */
#define MEDIA_I2C0_ICG_DIS_REG              (0x3b4)  /* Clock disable control register */
#define MEDIA_I2C0_ICG_ST_REG               (0x53b0)  /* Clock enable status register */

#define AO_I2C1_RESET_REQ_REG               (0xe88)  /* Soft reset request control register */
#define AO_I2C1_RESET_DREQ_REG              (0xe8c)  /* Soft reset derequest control register */
#define AO_I2C1_RESET_ST_REG                (0x5e88)  /* Soft reset status register */
#define AO_I2C1_ICG_EN_REG                  (0x3b0)  /* Clock enable control register */
#define AO_I2C1_ICG_DIS_REG                 (0x3b4)  /* Clock disable control register */
#define AO_I2C1_ICG_ST_REG                  (0x53b0)  /* Clock enable status register */

#define MEDIA_I2C2_RESET_REQ_REG            (0xe90)  /* Soft reset request control register */
#define MEDIA_I2C2_RESET_DREQ_REG           (0xe94)  /* Soft reset derequest control register */
#define MEDIA_I2C2_RESET_ST_REG             (0x5e90)  /* Soft reset status register */
#define MEDIA_I2C2_ICG_EN_REG               (0x3b8)  /* Clock enable control register */
#define MEDIA_I2C2_ICG_DIS_REG              (0x3bc)  /* Clock disable control register */
#define MEDIA_I2C2_ICG_ST_REG               (0x53b8)  /* Clock enable status register */

#define ISP_I2C3_RESET_REQ_REG              (0x398)  /* Soft reset request control register */
#define ISP_I2C3_RESET_DREQ_REG             (0x39c)  /* Soft reset derequest control register */
#define ISP_I2C3_RESET_ST_REG               (0x5398)  /* Soft reset status register */
#define ISP_I2C3_ICG_EN_REG                 (0x390)  /* Clock enable control register */
#define ISP_I2C3_ICG_DIS_REG                (0x394)  /* Clock disable control register */
#define ISP_I2C3_ICG_ST_REG                 (0x5390)  /* Clock enable status register */

#define MEDIA_I2C8_RESET_REQ_REG            (0xe98)  /* Soft reset request control register */
#define MEDIA_I2C8_RESET_DREQ_REG           (0xe9c)  /* Soft reset derequest control register */
#define MEDIA_I2C8_RESET_ST_REG             (0x5e98)  /* Soft reset status register */
#define MEDIA_I2C8_ICG_EN_REG               (0x3c0)  /* Clock enable control register */
#define MEDIA_I2C8_ICG_DIS_REG              (0x3c4)  /* Clock disable control register */
#define MEDIA_I2C8_ICG_ST_REG               (0x53c0)  /* Clock enable status register */

#define PERI_I2C9_RESET_REQ_REG             (0xa80)  /* Soft reset request control register */
#define PERI_I2C9_RESET_DREQ_REG            (0xa84)  /* Soft reset derequest control register */
#define PERI_I2C9_RESET_ST_REG              (0x5a80)  /* Soft reset status register */
#define PERI_I2C9_ICG_EN_REG                (0x348)  /* Clock enable control register */
#define PERI_I2C9_ICG_DIS_REG               (0x34c)  /* Clock disable control register */
#define PERI_I2C9_ICG_ST_REG                (0x5348)  /* Clock enable status register */

#define MEDIA_I2C10_RESET_REQ_REG           (0xea8)  /* Soft reset request control register */
#define MEDIA_I2C10_RESET_DREQ_REG          (0xeac)  /* Soft reset derequest control register */
#define MEDIA_I2C10_RESET_ST_REG            (0x5ea8)  /* Soft reset status register */
#define MEDIA_I2C10_ICG_EN_REG              (0x3d0)  /* Clock enable control register */
#define MEDIA_I2C10_ICG_DIS_REG             (0x3d4)  /* Clock disable control register */
#define MEDIA_I2C10_ICG_ST_REG              (0x53d0)  /* Clock enable status register */

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

#define MEDIA_I2C0_CFG_REG            (0x2600)
#define AO_I2C1_CFG_REG               (0x2600)
#define MEDIA_I2C2_CFG_REG            (0x2608)
#define ISP_I2C3_CFG_REG              (0x20d4)
#define MEDIA_I2C8_CFG_REG            (0x2610)
#define PERI_I2C9_CFG_REG             (0x3850)
#define MEDIA_I2C10_CFG_REG           (0x2620)

#define I2C_CLK_MUX_SEL   BIT(0)
#define I2C_CLK_OE_CFG    BIT(1)
#define I2C_SCL_CFG       BIT(2)
#define I2C_DAT_MUX_SEL   BIT(3)
#define I2C_DAT_OE_CFG    BIT(4)

struct i2c_num_subctrl_cell {
    int i2c_num;
    int subctrl_id;
};

extern const u64 g_subctrl_pbase[SUB_CTRL_MAX];
extern const struct i2c_crg_reset_cell g_subctrl_resets[I2C_MAX_NUM];
extern const struct i2c_crg_clkgate g_subctrl_clkgates[I2C_MAX_NUM];
extern struct i2c_subctrl_recover_cell g_i2c_recover_cfg[I2C_MAX_NUM];
extern struct i2c_num_subctrl_cell g_i2c_num_subctrl[I2C_MAX_NUM];
int bus_id_to_subctrl_id(u32 bus_id);
#endif