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
 * Description: i2c misc 310 source file
 * Author: huawei
 * Create: 2023-10-12
 */

#include <linux/module.h>
#include <linux/mm.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/securec.h>
#include <asm/delay.h>
#include <linux/version.h>
#include <linux/device.h>
#include "i2c_misc_310.h"

const u64 g_subctrl_pbase[SUB_CTRL_MAX] = {
    [PERI_SUB_CTRL] = 0x80130000,
    [AO_HISS_SUB_CTRL] = 0x110100000,
    [ISP_SUB_CTRL] = 0x300130000,
    [AO_SUB_CTRL] = 0xc0120000,
    [MEDIA_SUB_CTRL] = 0x400130000,
};

const struct i2c_crg_reset_cell g_subctrl_resets[I2C_MAX_NUM] = {
    { ISP_I2C_RESET_REQ_REG, ISP_I2C_RESET_DREQ_REG, ISP_I2C_RESET_ST_REG, 0x1},
    { ISP_I2C_RESET_REQ_REG, ISP_I2C_RESET_DREQ_REG, ISP_I2C_RESET_ST_REG, 0x2},
    { ISP_I2C_RESET_REQ_REG, ISP_I2C_RESET_DREQ_REG, ISP_I2C_RESET_ST_REG, 0x4},
    { ISP_I2C_RESET_REQ_REG, ISP_I2C_RESET_DREQ_REG, ISP_I2C_RESET_ST_REG, 0x8},
    { AO_I2C4_RESET_REQ_REG, AO_I2C4_RESET_DREQ_REG, AO_I2C4_RESET_ST_REG, 0x1},
    { AO_I2C5_RESET_REQ_REG, AO_I2C5_RESET_DREQ_REG, AO_I2C5_RESET_ST_REG, 0x1},
    { AO_I2C6_RESET_REQ_REG, AO_I2C6_RESET_DREQ_REG, AO_I2C6_RESET_ST_REG, 0x1},
    { PERI_I2C_RESET_REQ_REG, PERI_I2C_RESET_DREQ_REG, PERI_I2C_RESET_ST_REG, 0x1},
    { PERI_I2C_RESET_REQ_REG, PERI_I2C_RESET_DREQ_REG, PERI_I2C_RESET_ST_REG, 0x2},
    { AO_I2C9_RESET_REQ_REG, AO_I2C9_RESET_DREQ_REG, AO_I2C9_RESET_ST_REG, 0x1},
    { AO_HISS_I2C_RESET_REQ_REG, AO_HISS_I2C_RESET_DREQ_REG, AO_HISS_I2C_RESET_ST_REG, 0x1},
    { PERI_I2C_RESET_REQ_REG, PERI_I2C_RESET_DREQ_REG, PERI_I2C_RESET_ST_REG, 0x4},
    { PERI_I2C_RESET_REQ_REG, PERI_I2C_RESET_DREQ_REG, PERI_I2C_RESET_ST_REG, 0x8},
    { MEDIA_I2C_RESET_REQ_REG, MEDIA_I2C_RESET_DREQ_REG, MEDIA_I2C_RESET_ST_REG, 0x1},
};
const struct i2c_crg_clkgate g_subctrl_clkgates[I2C_MAX_NUM] = {
    { ISP_I2C0, "icg_en_i2c0", ISP_I2C_ICG_EN_REG, ISP_I2C_ICG_DIS_REG, ISP_I2C_ICG_ST_REG, 0x1},
    { ISP_I2C1, "icg_en_i2c1", ISP_I2C_ICG_EN_REG, ISP_I2C_ICG_DIS_REG, ISP_I2C_ICG_ST_REG, 0x2},
    { ISP_I2C2, "icg_en_i2c2", ISP_I2C_ICG_EN_REG, ISP_I2C_ICG_DIS_REG, ISP_I2C_ICG_ST_REG, 0x4},
    { ISP_I2C3, "icg_en_i2c3", ISP_I2C_ICG_EN_REG, ISP_I2C_ICG_DIS_REG, ISP_I2C_ICG_ST_REG, 0x8},
    { AO_I2C4, "icg_en_i2c4", AO_I2C4_ICG_EN_REG, AO_I2C4_ICG_DIS_REG, AO_I2C4_ICG_ST_REG, 0x1},
    { AO_I2C5, "icg_en_i2c5", AO_I2C5_ICG_EN_REG, AO_I2C5_ICG_DIS_REG, AO_I2C5_ICG_ST_REG, 0x1},
    { AO_I2C6, "icg_en_i2c6", AO_I2C6_ICG_EN_REG, AO_I2C6_ICG_DIS_REG, AO_I2C6_ICG_ST_REG, 0x1},
    { PERI_I2C7, "icg_en_i2c7", PERI_I2C_ICG_EN_REG, PERI_I2C_ICG_DIS_REG, PERI_I2C_ICG_ST_REG, 0x1},
    { PERI_I2C8, "icg_en_i2c8", PERI_I2C_ICG_EN_REG, PERI_I2C_ICG_DIS_REG, PERI_I2C_ICG_ST_REG, 0x2},
    { AO_I2C9, "icg_en_i2c9", AO_I2C9_ICG_EN_REG, AO_I2C9_ICG_DIS_REG, AO_I2C9_ICG_ST_REG, 0x1},
    { AO_HISS_I2C10, "icg_en_i2c10", AO_HISS_I2C_ICG_EN_REG, AO_HISS_I2C_ICG_DIS_REG, AO_HISS_I2C_ICG_ST_REG, 0x1},
    { PERI_I2C11, "icg_en_i2c11", PERI_I2C_ICG_EN_REG, PERI_I2C_ICG_DIS_REG, PERI_I2C_ICG_ST_REG, 0x4},
    { PERI_I2C12, "icg_en_i2c12", PERI_I2C_ICG_EN_REG, PERI_I2C_ICG_DIS_REG, PERI_I2C_ICG_ST_REG, 0x8},
    { MEDIA_I2C_DSI, "icg_en_i2c_dsi", MEDIA_I2C_ICG_EN_REG, MEDIA_I2C_ICG_DIS_REG, MEDIA_I2C_ICG_ST_REG, 0x1},
};

struct i2c_subctrl_recover_cell g_i2c_recover_cfg[I2C_MAX_NUM] = {
    { ISP_I2C0_CFG_REG, I2C_CLK_MUX_SEL, I2C_CLK_OE_CFG, I2C_DAT_MUX_SEL, I2C_SCL_CFG, I2C_DAT_OE_CFG},
    { ISP_I2C1_CFG_REG, I2C_CLK_MUX_SEL, I2C_CLK_OE_CFG, I2C_DAT_MUX_SEL, I2C_SCL_CFG, I2C_DAT_OE_CFG},
    { ISP_I2C2_CFG_REG, I2C_CLK_MUX_SEL, I2C_CLK_OE_CFG, I2C_DAT_MUX_SEL, I2C_SCL_CFG, I2C_DAT_OE_CFG},
    { ISP_I2C3_CFG_REG, I2C_CLK_MUX_SEL, I2C_CLK_OE_CFG, I2C_DAT_MUX_SEL, I2C_SCL_CFG, I2C_DAT_OE_CFG},
    { AO_I2C4_CFG_REG, I2C_CLK_MUX_SEL, I2C_CLK_OE_CFG, I2C_DAT_MUX_SEL, I2C_SCL_CFG, I2C_DAT_OE_CFG},
    { AO_I2C5_CFG_REG, I2C_CLK_MUX_SEL, I2C_CLK_OE_CFG, I2C_DAT_MUX_SEL, I2C_SCL_CFG, I2C_DAT_OE_CFG},
    { AO_I2C6_CFG_REG, I2C_CLK_MUX_SEL, I2C_CLK_OE_CFG, I2C_DAT_MUX_SEL, I2C_SCL_CFG, I2C_DAT_OE_CFG},
    { PERI_I2C7_CFG_REG, I2C_CLK_MUX_SEL, I2C_CLK_OE_CFG, I2C_DAT_MUX_SEL, I2C_SCL_CFG, I2C_DAT_OE_CFG},
    { PERI_I2C8_CFG_REG, I2C_CLK_MUX_SEL, I2C_CLK_OE_CFG, I2C_DAT_MUX_SEL, I2C_SCL_CFG, I2C_DAT_OE_CFG},
    { AO_I2C9_CFG_REG, I2C_CLK_MUX_SEL, I2C_CLK_OE_CFG, I2C_DAT_MUX_SEL, I2C_SCL_CFG, I2C_DAT_OE_CFG},
    { AO_HISS_I2C10_CFG_REG, I2C_CLK_MUX_SEL, I2C_CLK_OE_CFG, I2C_DAT_MUX_SEL, I2C_SCL_CFG, I2C_DAT_OE_CFG},
    { PERI_I2C11_CFG_REG, I2C_CLK_MUX_SEL, I2C_CLK_OE_CFG, I2C_DAT_MUX_SEL, I2C_SCL_CFG, I2C_DAT_OE_CFG},
    { PERI_I2C12_CFG_REG, I2C_CLK_MUX_SEL, I2C_CLK_OE_CFG, I2C_DAT_MUX_SEL, I2C_SCL_CFG, I2C_DAT_OE_CFG},
    { MEDIA_I2C_DSI_CFG_REG, I2C_CLK_MUX_SEL, I2C_CLK_OE_CFG, I2C_DAT_MUX_SEL, I2C_SCL_CFG, I2C_DAT_OE_CFG},
};

int bus_id_to_subctrl_id(u32 bus_id)
{
    int index;
    switch (bus_id) {
        case PERI_I2C7:
        case PERI_I2C8:
        case PERI_I2C11:
        case PERI_I2C12:
            index = PERI_SUB_CTRL;
            break;
        case ISP_I2C0:
        case ISP_I2C1:
        case ISP_I2C2:
        case ISP_I2C3:
            index = ISP_SUB_CTRL;
            break;
        case AO_HISS_I2C10:
            index = AO_HISS_SUB_CTRL;
            break;
        case MEDIA_I2C_DSI:
            index = MEDIA_SUB_CTRL;
            break;
        case AO_I2C4:
        case AO_I2C5:
        case AO_I2C6:
        case AO_I2C9:
            index = AO_SUB_CTRL;
            break;
        default:
            index = -1;
    }
    return index;
}