/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 * Create: 2023-2-7
 */
#define _GNU_SOURCE
#include <asm/io.h>
#include <linux/ioport.h>
#include "hclge_serdes_reg_macro.h"
#include "hclge_serdes_common.h"

STATIC H60_FREQ_CFG hilink_h60_freq_table[] = {
    /* {1:ref_clk_khz   2:ref_clk_sel      3:hs_clk_mhz         4:m_div           5:fpll_mode      6:ka_div */
    /*  7:kb_div        8:vco_freq_khz     9:na_div             10:nb_div_p       11:nb_div_s      12:sdmdiv4divider */
    /*  13:fcw_hex      14:coreclkdiv_sel  15:coreclkdiv_double 16:coreclk2digdiv 17:jcom          18:j40b          */
    /*  19:j32b         20:pam4mode        21:rate_mode_div     22:rx_bus_sel     23:tx_bus_sel */
    /*   {1       2   3       4   5   6   7   8       9   10  11  12  13  14  15  16  17  18 19}, 20   21 22  23 */
    {25, {156250, 1,  12890,  1,  0,  0,  0,  12890,  2,  11, 5,  0,  0,  9,  1,  0,  0,  0, 0},  0,   0,  0, 0},
    {26, {156250, 1,  13281,  1,  0,  0,  0,  13281,  1,  19, 3,  0,  0,  4,  1,  1,  0,  0, 0},  0,   0,  0, 0},
    {51, {156250, 1,  12890,  1,  0,  0,  0,  12890,  2,  11, 5,  0,  0,  9,  1,  0,  0,  0, 0},  1,   0,  0, 0},
    {53, {156250, 1,  13281,  1,  0,  0,  0,  13281,  1,  19, 3,  0,  0,  4,  1,  1,  0,  0, 0},  1,   0,  0, 0},
};

H60_FREQ_CFG *hisds_get_freq_data_h60(u32 data_rate)
{
    size_t hilink_h60_freq_num = sizeof(hilink_h60_freq_table) / sizeof(hilink_h60_freq_table[0]);
    size_t i;

    for (i = 0; i < hilink_h60_freq_num; i++) {
        if (data_rate == hilink_h60_freq_table[i].data_rate) {
            return &(hilink_h60_freq_table[i]);
        }
    }

    return NULL;
}

STATIC void h60_read_reg(struct hclge_dev *hdev, u32 reg, u32 offset, u32 *val)
{
    *val = readw(hdev->hw.hilink_base + reg + offset);
}

STATIC void h60_write_reg(struct hclge_dev *hdev, u32 reg, u32 offset, u32 val)
{
    writew(val, hdev->hw.hilink_base + reg + offset);
}

STATIC u32 h60_get_reg_field_pos(u32 mask)
{
    u32 pos = 0;

    while ((mask & 0x1) == 0) {
        mask >>= 1;
        pos++;
    }

    return pos;
}

void h60_read_reg_field(struct hclge_dev *hdev, u32 reg, u32 mask, u32 offset, u32 *val)
{
    u32 pos = h60_get_reg_field_pos(mask);
    u32 reg_val = 0;

    h60_read_reg(hdev, reg, offset, &reg_val);
    *val = (reg_val & mask) >> pos;
}

void h60_write_reg_field(struct hclge_dev *hdev, u32 reg, u32 mask, u32 offset, u32 val)
{
    u32 pos = h60_get_reg_field_pos(mask);
    u32 reg_val = 0;

    h60_read_reg(hdev, reg, offset, &reg_val);
    reg_val &= ~mask;
    reg_val |= (val << pos) & mask;
    h60_write_reg(hdev, reg, offset, reg_val);
    h60_read_reg(hdev, reg, offset, &reg_val);
}

/*
 * Wait until the field_id of the reg changes to a expect_value
 *      within max_wait_times * delay_per_loop_us times.
 * Otherwise, a failure message is returned.
 */
u32 h60_wait_reg_value(struct hclge_dev *hdev, const CHIP_INFO_S *chip_info, u32 reg, u32 mask, u32 offset,
    u32 expect_value, u32 max_wait_times, u32 delay_per_loop_us)
{
    struct pci_dev *pdev = hdev->pdev;
    u32 read_value = 0;
    u32 i;

    for (i = 0; i < max_wait_times; i++) {
        h60_read_reg_field(hdev, reg, mask, offset, &read_value);
        if (read_value == expect_value) {
            return SERDES_OK;
        }
        usleep_range(delay_per_loop_us, delay_per_loop_us);
    }

    dev_err(&pdev->dev, "chip:%u macro_id:%u cs/ds:%u wait reg:0x%x mask:0x%x to 0x%x fail, got value 0x%x!\n",
            chip_info->chip_id, chip_info->macro_id, chip_info->csds_id, reg, mask, expect_value, read_value);

    return SERDES_ERROR;
}

u32 h60_init_reg_write_ensure(struct hclge_dev *hdev, const CHIP_INFO_S *chip_info, u32 reg, u32 mask, u32 offset,
                              u32 expect_value)
{
    h60_write_reg_field(hdev, reg, mask, offset, expect_value);
    /* wait 20 times x 5 us = 100 us */
    return h60_wait_reg_value(hdev, chip_info, reg, mask, offset, expect_value, 20, 5);
}

void hisds_get_hsclk_h60(struct hclge_dev *hdev, const CHIP_INFO_S *chip_info, u32 pll_id, u32 *hsclk)
{
    if (pll_id == HILINK_SERDES_PLL_ID_0) {
        h60_read_reg_field(hdev, H60_CS_SPARE0f, CS_REG_OFFSET(chip_info), hsclk);
        return;
    }
    h60_read_reg_field(hdev, H60_CS_SPARE1f, CS_REG_OFFSET(chip_info), hsclk);
}

void hisds_get_data_rate_h60(struct hclge_dev *hdev, const CHIP_INFO_S *chip_info, SERDES_MODULE_E module,
    u32 *data_rate_mhz)
{
    u32 pll_id = HILINK_SERDES_PLL_ID_0;
    u32 pam4_en = H60_NRZ_MODE_EN;
    u32 rate_mode = 0; /* 1-half 0-full */
    u32 hsclk_mhz = 0;

    if (module == SERDES_TX) {
        h60_read_reg_field(hdev, H60_DSCLK_BUS_SELTXf, DS_REG_OFFSET(chip_info), &pll_id);
        h60_read_reg_field(hdev, H60_TXRATEMODEf, DS_REG_OFFSET(chip_info), &rate_mode);
        h60_read_reg_field(hdev, H60_TX_PAM4_MODEf, DS_REG_OFFSET(chip_info), &pam4_en);
    } else {
        h60_read_reg_field(hdev, H60_DSCLK_BUS_SELRXf, DS_REG_OFFSET(chip_info), &pll_id);
        h60_read_reg_field(hdev, H60_RXRATEMODEf, DS_REG_OFFSET(chip_info), &rate_mode);
        h60_read_reg_field(hdev, H60_MODCONFIGf, DS_REG_OFFSET(chip_info), &pam4_en);
    }
    hisds_get_hsclk_h60(hdev, chip_info, pll_id, &hsclk_mhz);
    *data_rate_mhz = hsclk_mhz * 2 * (pam4_en + 1) / (rate_mode + 1); /* speed = hsclk x 2 x pam4 / rate mode */
}

bool macro0_in_use(struct hclge_dev *hdev)
{
    return ((hdev->hw.mac.mac_id == HIMAC_MAC_0) && (hdev->mac_mode == MAC_MODE_400G)) ||
           ((hdev->hw.mac.mac_id == HIMAC_MAC_0) &&
            (hdev->mac_mode == MAC_MODE_100G || hdev->mac_mode == MAC_MODE_200G));
}

bool macro1_in_use(struct hclge_dev *hdev)
{
    return ((hdev->hw.mac.mac_id == HIMAC_MAC_0) && (hdev->mac_mode == MAC_MODE_400G)) ||
           ((hdev->hw.mac.mac_id == HIMAC_MAC_2) &&
            (hdev->mac_mode == MAC_MODE_100G || hdev->mac_mode == MAC_MODE_200G));
}

void hclge_init_chip_info(struct hclge_dev *hdev, CHIP_INFO_S *chip_info, u8 macro_id)
{
    chip_info->chip_id = (u8)hdev->chip_id;
    chip_info->die_id = (u8)hdev->die_id;
    chip_info->macro_id = macro_id;
    chip_info->csds_id = H60_DS_0;
    chip_info->ds_num = H60_DEFAULT_SERDES_LANE_NUM;
    chip_info->ds_mask = H60_DS_INIT_MASK;
    chip_info->connect_type = ((struct custom_dev *)hdev->custom_priv)->connect_type;
}
