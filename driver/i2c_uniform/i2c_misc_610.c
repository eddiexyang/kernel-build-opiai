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
 * Description: i2c misc 610 source file
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
#include "i2c_misc_610.h"

const u64 g_subctrl_pbase[SUB_CTRL_MAX] = {
    [PERI_SUB_CTRL] = 0xa40c0000,
    [AO_HISS_SUB_CTRL] = 0x110100000,
    [ISP_SUB_CTRL] = 0xa80c0000,
    [AO_SUB_CTRL] = 0xc0120000,
    [MEDIA_SUB_CTRL] = 0xfc0c0000,
};

const struct i2c_crg_reset_cell g_subctrl_resets[I2C_MAX_NUM] = {
    { MEDIA_I2C0_RESET_REQ_REG, MEDIA_I2C0_RESET_DREQ_REG, MEDIA_I2C0_RESET_ST_REG, 0x1},
    { AO_I2C1_RESET_REQ_REG, AO_I2C1_RESET_DREQ_REG, AO_I2C1_RESET_ST_REG, 0x1},
    { MEDIA_I2C2_RESET_REQ_REG, MEDIA_I2C2_RESET_DREQ_REG, MEDIA_I2C2_RESET_ST_REG, 0x1},
    { ISP_I2C3_RESET_REQ_REG, ISP_I2C3_RESET_DREQ_REG, ISP_I2C3_RESET_ST_REG, 0x2},
    { MEDIA_I2C8_RESET_REQ_REG, MEDIA_I2C8_RESET_DREQ_REG, MEDIA_I2C8_RESET_ST_REG, 0x1},
    { PERI_I2C9_RESET_REQ_REG, PERI_I2C9_RESET_DREQ_REG, PERI_I2C9_RESET_ST_REG, 0x1},
    { MEDIA_I2C10_RESET_REQ_REG, MEDIA_I2C10_RESET_DREQ_REG, MEDIA_I2C10_RESET_ST_REG, 0x1},
};

const struct i2c_crg_clkgate g_subctrl_clkgates[I2C_MAX_NUM] = {
    { MEDIA_I2C0, "icg_en_i2c0", MEDIA_I2C0_ICG_EN_REG, MEDIA_I2C0_ICG_DIS_REG, MEDIA_I2C0_ICG_ST_REG, 0x1},
    { AO_I2C1, "icg_en_i2c1", AO_I2C1_ICG_EN_REG, AO_I2C1_ICG_DIS_REG, AO_I2C1_ICG_ST_REG, 0x1},
    { MEDIA_I2C2, "icg_en_i2c2", MEDIA_I2C2_ICG_EN_REG, MEDIA_I2C2_ICG_DIS_REG, MEDIA_I2C2_ICG_ST_REG, 0x1},
    { ISP_I2C3, "icg_en_i2c3", ISP_I2C3_ICG_EN_REG, ISP_I2C3_ICG_DIS_REG, ISP_I2C3_ICG_ST_REG, 0x2},
    { MEDIA_I2C8, "icg_en_i2c8", MEDIA_I2C8_ICG_EN_REG, MEDIA_I2C8_ICG_DIS_REG, MEDIA_I2C8_ICG_ST_REG, 0x1},
    { PERI_I2C9, "icg_en_i2c9", PERI_I2C9_ICG_EN_REG, PERI_I2C9_ICG_DIS_REG, PERI_I2C9_ICG_ST_REG, 0x1},
    { MEDIA_I2C10, "icg_en_i2c10", MEDIA_I2C10_ICG_EN_REG, MEDIA_I2C10_ICG_DIS_REG, MEDIA_I2C10_ICG_ST_REG, 0x1},
};

struct i2c_subctrl_recover_cell g_i2c_recover_cfg[I2C_MAX_NUM] = {
    { MEDIA_I2C0_CFG_REG, I2C_CLK_MUX_SEL, I2C_CLK_OE_CFG, I2C_DAT_MUX_SEL, I2C_SCL_CFG, I2C_DAT_OE_CFG},
    { AO_I2C1_CFG_REG, I2C_CLK_MUX_SEL, I2C_CLK_OE_CFG, I2C_DAT_MUX_SEL, I2C_SCL_CFG, I2C_DAT_OE_CFG},
    { MEDIA_I2C2_CFG_REG, I2C_CLK_MUX_SEL, I2C_CLK_OE_CFG, I2C_DAT_MUX_SEL, I2C_SCL_CFG, I2C_DAT_OE_CFG},
    { ISP_I2C3_CFG_REG, I2C_CLK_MUX_SEL, I2C_CLK_OE_CFG, I2C_DAT_MUX_SEL, I2C_SCL_CFG, I2C_DAT_OE_CFG},
    { MEDIA_I2C8_CFG_REG, I2C_CLK_MUX_SEL, I2C_CLK_OE_CFG, I2C_DAT_MUX_SEL, I2C_SCL_CFG, I2C_DAT_OE_CFG},
    { PERI_I2C9_CFG_REG, I2C_CLK_MUX_SEL, I2C_CLK_OE_CFG, I2C_DAT_MUX_SEL, I2C_SCL_CFG, I2C_DAT_OE_CFG},
    { MEDIA_I2C10_CFG_REG, I2C_CLK_MUX_SEL, I2C_CLK_OE_CFG, I2C_DAT_MUX_SEL, I2C_SCL_CFG, I2C_DAT_OE_CFG},
};

struct i2c_num_subctrl_cell g_i2c_num_subctrl[I2C_MAX_NUM] = {
    { MEDIA_I2C0,  MEDIA_SUB_CTRL },
    { AO_I2C1,     AO_SUB_CTRL    },
    { MEDIA_I2C2,  MEDIA_SUB_CTRL },
    { ISP_I2C3,    ISP_SUB_CTRL   },
    { MEDIA_I2C8,  MEDIA_SUB_CTRL },
    { PERI_I2C9,   PERI_SUB_CTRL  },
    { MEDIA_I2C10, MEDIA_SUB_CTRL },
};

int bus_id_to_subctrl_id(u32 bus_id)
{
    if (bus_id >= I2C_MAX_NUM) {
        return -1;
    }

    return g_i2c_num_subctrl[bus_id].subctrl_id;
}