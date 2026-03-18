/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 * Create: 2023-2-7
 */
#include "hclge_serdes_common.h"
#include "hclge_serdes_adapt.h"
#include "hclge_serdes_rate_switch.h"

#ifndef DEFINE_HNS_LLT
#define STATIC static
#else
#define STATIC
#endif

STATIC u8 hclge_sds_rate_mode_convert(u8 eth_speed)
{
    STATIC h60_sds_speed_table speed_table[] = {
        {ETH_SDS_RATE_53P125G, H60_DATA_RATE_53P125G},
        {ETH_SDS_RATE_51P5625G, H60_DATA_RATE_51P5625G},
        {ETH_SDS_RATE_26P5625G, H60_DATA_RATE_26P5625G},
        {ETH_SDS_RATE_25P78125G, H60_DATA_RATE_25P78125G},
    };
    size_t size = sizeof(speed_table) / sizeof(h60_sds_speed_table);
    size_t i;

    for (i = 0; i < size; i++) {
        if (speed_table[i].eth_speed == eth_speed) {
            return speed_table[i].h60_speed;
        }
    }

    return H60_DATA_RATE_UNKNOWN;
}

STATIC SWITCH_STRATEGY_E hisds_get_switch_strategy_h60(struct hclge_dev *hdev, CHIP_INFO_S *chip_info,
    u8 h60_speed, u8 *pll_sel)
{
    u8 pll_id = HILINK_SERDES_PLL_ID_0;
    H60_FREQ_CFG *freq = NULL;
    u32 pll_hsclk;

    freq = hisds_get_freq_data_h60(h60_speed);
    if (freq == NULL) {
        dev_err(&hdev->pdev->dev, "Get freq data fail, freq is null, h60_speed is %u\n", h60_speed);
        return UNSUPPORT_SWITCH;
    }

    hisds_get_hsclk_h60(hdev, chip_info, pll_id, &pll_hsclk);
    if (freq->cs_cfg.hs_clk_mhz == pll_hsclk) {
        *pll_sel = pll_id;
        return SELECT_PLL_ONLY;
    }

    return CHANGE_PLL_FREQ;
}

STATIC void hisds_update_macro_info_h60(const CHIP_INFO_S *chip_info, HILINK_MACRO_INIT_S *macro_info,
    u8 h60_speed, u8 pll_sel)
{
    STATIC HILINK_TX_PARA_S default_tx_cfg = {0, 0, -10, 53, 0, 0, 0xF, 0};
    u8 ds_num = chip_info->ds_num;
    u8 ds_index;

    for (ds_index = 0; ds_index < ds_num; ds_index++) {
        if (GET_BIT(chip_info->ds_mask, ds_index) == 0) {
            continue;
        }

        macro_info->ds_info[ds_index].tx_cs_sel = pll_sel;
        macro_info->ds_info[ds_index].tx_cfg = default_tx_cfg;
    }

    macro_info->cs_info[pll_sel].serdes_rate = h60_speed;
}

STATIC void hisds_restore_high_speed_txpa_lock(struct hclge_dev *hdev, CHIP_INFO_S *chip_info)
{
    u8 ds_num = chip_info->ds_num;
    u32 txalign_mode = 0;
    u8 ds_index;

    for (ds_index = 0; ds_index < ds_num; ds_index++) {
        if (GET_BIT(chip_info->ds_mask, ds_index) == 0) {
            continue;
        }

        chip_info->csds_id = ds_index;
        /* Restore high speed TXPA locking if master mode */
        h60_read_reg_field(hdev, H60_TXALIGNMODEf, DS_REG_OFFSET(chip_info), &txalign_mode);
        if (txalign_mode == 0) {
            h60_write_reg_field(hdev, H60_TXPA_ACCDEPTHf, DS_REG_OFFSET(chip_info), 0x0);
        }
    }

    return;
}

STATIC void hisds_set_pll_bleed_ctrl_h60(struct hclge_dev *hdev, CHIP_INFO_S *chip_info, u32 pll_id, u32 en_ctrl)
{
    if (pll_id == HILINK_SERDES_PLL_ID_0) {
        h60_write_reg_field(hdev, H60_PLL0VREGPLLBLEEDBf, CS_REG_OFFSET(chip_info), en_ctrl);
        h60_write_reg_field(hdev, H60_PLL0VREGDLFBLEEDBf, CS_REG_OFFSET(chip_info), en_ctrl);
        h60_write_reg_field(hdev, H60_PLL0VREGVCODIVBLEEDBf, CS_REG_OFFSET(chip_info), en_ctrl);
        return;
    }

    h60_write_reg_field(hdev, H60_PLL1VREGPLLBLEEDBf, CS_REG_OFFSET(chip_info), en_ctrl);
    h60_write_reg_field(hdev, H60_PLL1VREGDLFBLEEDBf, CS_REG_OFFSET(chip_info), en_ctrl);
    h60_write_reg_field(hdev, H60_PLL1VREGVCODIVBLEEDBf, CS_REG_OFFSET(chip_info), en_ctrl);
    return;
}

STATIC u32 hisds_cs_pll_power_up_h60(struct hclge_dev *hdev, CHIP_INFO_S *chip_info, u32 pll_id)
{
    SERDES_REG_FIELD_S pll_pwd_tri_reg[HILINK_SERDES_MAX_CS_NUM] = {
        {H60_PLL0PWRDBTRIGGERf},
        {H60_PLL1PWRDBTRIGGERf},
    };
    u32 ret = SERDES_OK;
    u32 pll_pwr = 0;
    u32 reg_val = 0;

    /* read 3 times to ensure the robustness of CS register reading */
    h60_read_reg_field(hdev, pll_pwd_tri_reg[pll_id].reg_id, pll_pwd_tri_reg[pll_id].field_id,
                       CS_REG_OFFSET(chip_info), &pll_pwr);
    h60_read_reg_field(hdev, pll_pwd_tri_reg[pll_id].reg_id, pll_pwd_tri_reg[pll_id].field_id,
                       CS_REG_OFFSET(chip_info), &reg_val);
    pll_pwr &= reg_val;
    h60_read_reg_field(hdev, pll_pwd_tri_reg[pll_id].reg_id, pll_pwd_tri_reg[pll_id].field_id,
                       CS_REG_OFFSET(chip_info), &reg_val);
    pll_pwr &= reg_val;
    if (pll_pwr == SERDES_POWER_UP) {
        return SERDES_OK;
    }

    hisds_set_pll_bleed_ctrl_h60(hdev, chip_info, pll_id, 0x0);
    h60_write_reg_field(hdev, pll_pwd_tri_reg[pll_id].reg_id, pll_pwd_tri_reg[pll_id].field_id,
                        CS_REG_OFFSET(chip_info), SERDES_POWER_UP);
    /* wait for cs power sequence done 50 times * 10 us */
    ret |= h60_wait_reg_value(hdev, chip_info, H60_CSPWRSEQDONEf, CS_REG_OFFSET(chip_info), 1, 50, 10);
    hisds_set_pll_bleed_ctrl_h60(hdev, chip_info, pll_id, 0x1);
    return ret;
}

STATIC SERDES_CFG_REG_S g_pre_cfg_reg[G_PRE_CFG_REG_SIZE] = {
    /* PLL0_REG                             PLL1_REG                                value */
    {{{H60_PLL0REFCLKSELf},                {H60_PLL1REFCLKSELf}},                   0x0},
    {{{H60_PLL0VCODIVNAf},                 {H60_PLL1VCODIVNAf}},                    0x0},
    {{{H60_PLL0VCODIVKAf},                 {H60_PLL1VCODIVKAf}},                    0x0},
    {{{H60_PLL0FBDIVPf},                   {H60_PLL1FBDIVPf}},                      0x0},
    {{{H60_PLL0FBDIVSf},                   {H60_PLL1FBDIVSf}},                      0x0},
    {{{H60_PLL0MCLKDIVf},                  {H60_PLL1MCLKDIVf}},                     0x0},
    {{{H60_PLL0MCLK2DIGDIV1PWRDNBf},       {H60_PLL1MCLK2DIGDIV1PWRDNBf}},          0x0},
    {{{H60_PLL0HSCLKDISTSPAREf},           {H60_PLL1HSCLKDISTSPAREf}},              0x0},
    {{{H60_PLL0REFCLKDIVf},                {H60_PLL1REFCLKDIVf}},                   0x0},
    {{{H60_PLL0MCLKDIV40Bf},               {H60_PLL1MCLKDIV40Bf}},                  0x0},
    {{{H60_PLL0MCLKDIV32Bf},               {H60_PLL1MCLKDIV32Bf}},                  0x0},
    {{{H60_PLL0_FRACN_PI_MUXCTRLf},        {H60_PLL1_FRACN_PI_MUXCTRLf}},           0x0},
    {{{H60_PLL0CORECLKDIVSELf},            {H60_PLL1CORECLKDIVSELf}},               0x0},
    {{{H60_CORECLK2DIGDIVf},               {H60_CORECLK2DIGDIVf}},                  0x0},
    {{{H60_PLL0HSCLKDIVf},                 {H60_PLL1HSCLKDIVf}},                    0x0},
    {{{H60_PLL0_FRACN_PI_DIV1_PWRDNBf},    {H60_PLL1_FRACN_PI_DIV1_PWRDNBf}},       0x0},
    {{{H60_PLL0_FRACN_PI_DIVCTRLf},        {H60_PLL1_FRACN_PI_DIVCTRLf}},           0x0},
    {{{H60_PLL0DLFCINTSELf},               {H60_PLL1DLFCINTSELf}},                  0x1f},
    {{{H60_PLL0CPINTCRNTSELf},             {H60_PLL1CPINTCRNTSELf}},                0x0},
    {{{H60_PLL0CPPROPCRNTSELf},            {H60_PLL1CPPROPCRNTSELf}},               0x8},
};

STATIC SERDES_CFG_REG_S g_cfg_fpll_reg[G_CFG_FPLL_REG_SIZE] = {
    {{{H60_PLL0_FRACN_PI_FCW_UPDATE_ENf},  {H60_PLL1_FRACN_PI_FCW_UPDATE_ENf}},     0x0},
    {{{H60_PLL0_FRACN_PI_PWRDNBf},         {H60_PLL1_FRACN_PI_PWRDNBf}},            0x1},
    {{{H60_PLL0_FRACN_PI_QDIV_PWRDNBf},    {H60_PLL1_FRACN_PI_QDIV_PWRDNBf}},       0x1},
    {{{H60_PLL0_FRACN_PI_DIVMAIN_PWRDNBf}, {H60_PLL1_FRACN_PI_DIVMAIN_PWRDNBf}},    0x1},
    {{{H60_PLL0_FRACN_PI_SDM_RST_Nf},      {H60_PLL1_FRACN_PI_SDM_RST_Nf}},         0x0},
    {{{H60_PLL0_FRACN_PI_SDM_RST_Nf},      {H60_PLL1_FRACN_PI_SDM_RST_Nf}},         0x1},
    {{{H60_PLL0_FRACN_PI_SDM_ENf},         {H60_PLL1_FRACN_PI_SDM_ENf}},            0x1},
    {{{H60_PLL0_FRACN_PI_FCWLf},           {H60_PLL1_FRACN_PI_FCWLf}},              0x0},
    {{{H60_PLL0_FRACN_PI_FCWHf},           {H60_PLL1_FRACN_PI_FCWHf}},              0x0},
    {{{H60_PLL0_FRACN_PI_FCW_VALIDf},      {H60_PLL1_FRACN_PI_FCW_VALIDf}},         0x1},
    {{{H60_PLL0_FRACN_PI_FCW_VALIDf},      {H60_PLL1_FRACN_PI_FCW_VALIDf}},         0x0}
};

STATIC SERDES_CFG_REG_S g_post_cfg_reg[G_POST_CFG_REG_SIZE] = {
    {{{H60_CS_FNPLL0_EN_APIf},             {H60_CS_FNPLL1_EN_APIf}},                0x0},
    {{{H60_PLL0_FRACN_PI_FCW_UPDATE_ENf},  {H60_PLL1_FRACN_PI_FCW_UPDATE_ENf}},     0x0},
    {{{H60_PLL0CORECLKDIVDOUBLEf},         {H60_PLL1CORECLKDIVDOUBLEf}},            0x1},
    {{{H60_REFCLK0_FREQM100_APIf},         {H60_REFCLK1_FREQM100_APIf}},            0x0}
};

STATIC void hisds_fpll_cfg_freq_plan_h60(struct hclge_dev *hdev, CHIP_INFO_S *chip_info, u32 pll_id, H60_FREQ_CFG *freq)
{
    SERDES_REG_FIELD_S *reg;
    u32 cfg_index;

    /* fcw_hex low 16 bit are placed in the fpll array 7 */
    g_cfg_fpll_reg[IDX_7].value = freq->cs_cfg.fcw_hex & 0xFFFF; /* low 16 bit */
    /* fcw_hex high 16 bit are placed in the fpll array 8 */
    g_cfg_fpll_reg[IDX_8].value = (freq->cs_cfg.fcw_hex >> 16) & 0x1FFF; /* high 16 bit */
    for (cfg_index = 0; cfg_index < G_CFG_FPLL_REG_SIZE; cfg_index++) {
        reg = &(g_cfg_fpll_reg[cfg_index].pll_reg[pll_id]);
        h60_write_reg_field(hdev, reg->reg_id, reg->field_id, CS_REG_OFFSET(chip_info),
                            g_cfg_fpll_reg[cfg_index].value);
    }
    return;
}

STATIC u32 hisds_cs_cfg_freq_plan_h60(struct hclge_dev *hdev, CHIP_INFO_S *chip_info, HILINK_MACRO_INIT_S *macro_info,
    u32 pll_id)
{
    H60_FREQ_CFG *freq = NULL;
    SERDES_REG_FIELD_S *reg;
    u32 fpll_mode;
    u32 cfg_index;

    freq = hisds_get_freq_data_h60(macro_info->cs_info[pll_id].serdes_rate);
    if (freq == NULL) {
        dev_err(&hdev->pdev->dev, "freq is null\n");
        return SERDES_NULL;
    }

    /* Assign values to arrays in sequence. */
    g_pre_cfg_reg[IDX_0].value  = freq->cs_cfg.ref_clk_sel;
    g_pre_cfg_reg[IDX_1].value  = freq->cs_cfg.na_div;
    g_pre_cfg_reg[IDX_2].value  = freq->cs_cfg.ka_div;
    g_pre_cfg_reg[IDX_3].value  = freq->cs_cfg.nb_div_p;
    g_pre_cfg_reg[IDX_4].value  = freq->cs_cfg.nb_div_s;
    g_pre_cfg_reg[IDX_5].value  = freq->cs_cfg.jcom;
    g_pre_cfg_reg[IDX_8].value  = freq->cs_cfg.m_div;
    g_pre_cfg_reg[IDX_9].value  = freq->cs_cfg.j40b;
    g_pre_cfg_reg[IDX_10].value = freq->cs_cfg.j32b;
    g_pre_cfg_reg[IDX_11].value = freq->cs_cfg.fpll_mode;
    g_pre_cfg_reg[IDX_12].value = freq->cs_cfg.coreclkdiv_sel;
    g_pre_cfg_reg[IDX_13].value = freq->cs_cfg.coreclk2digdiv;
    g_pre_cfg_reg[IDX_14].value = freq->cs_cfg.kb_div;
    g_pre_cfg_reg[IDX_15].value = freq->cs_cfg.sdmdiv4divider >> 3; /* >> 3: high 1 bit */
    g_pre_cfg_reg[IDX_16].value = freq->cs_cfg.sdmdiv4divider & 0x7; /* &0x7: low 3 bit */
    for (cfg_index = 0; cfg_index < G_PRE_CFG_REG_SIZE; cfg_index++) {
        reg = &(g_pre_cfg_reg[cfg_index].pll_reg[pll_id]);
        h60_write_reg_field(hdev, reg->reg_id, reg->field_id, CS_REG_OFFSET(chip_info), g_pre_cfg_reg[cfg_index].value);
    }

    if (freq->pam4mode == H60_NRZ_MODE_EN) {
        reg = &(g_pre_cfg_reg[IDX_17].pll_reg[pll_id]);
        h60_write_reg_field(hdev, reg->reg_id, reg->field_id, CS_REG_OFFSET(chip_info), 0x14);
        reg = &(g_pre_cfg_reg[IDX_18].pll_reg[pll_id]);
        h60_write_reg_field(hdev, reg->reg_id, reg->field_id, CS_REG_OFFSET(chip_info), 0x2);
        reg = &(g_pre_cfg_reg[IDX_19].pll_reg[pll_id]);
        h60_write_reg_field(hdev, reg->reg_id, reg->field_id, CS_REG_OFFSET(chip_info), 0xF);
    }

    fpll_mode = freq->cs_cfg.fpll_mode;
    if (fpll_mode != 0) {
        hisds_fpll_cfg_freq_plan_h60(hdev, chip_info, pll_id, freq);
    }

    g_post_cfg_reg[IDX_0].value = freq->cs_cfg.fpll_mode;
    g_post_cfg_reg[IDX_2].value = freq->cs_cfg.coreclkdiv_double;
    g_post_cfg_reg[IDX_3].value = freq->cs_cfg.ref_clk_khz / 10; /* khz->10khz divided by 10 */
    for (cfg_index = 0; cfg_index < G_POST_CFG_REG_SIZE; cfg_index++) {
        reg = &(g_post_cfg_reg[cfg_index].pll_reg[pll_id]);
        h60_write_reg_field(hdev, reg->reg_id, reg->field_id, CS_REG_OFFSET(chip_info),
                            g_post_cfg_reg[cfg_index].value);
    }

    return SERDES_OK;
}

STATIC u32 hisds_cs_calib_mode_cfg_h60(struct hclge_dev *hdev, CHIP_INFO_S *chip_info, u8 pll_id)
{
    H60_CS_CALIB_MODE cal_mode;
    u32 ret = SERDES_OK;

    if (pll_id == HILINK_SERDES_PLL_ID_0) {
        cal_mode = H60_CS_PLL0_VCO_CAL;
    } else if (pll_id == HILINK_SERDES_PLL_ID_1) {
        cal_mode = H60_CS_PLL1_VCO_CAL;
    } else {
        return SERDES_NOT_SUPPORT;
    }

    ret |= h60_init_reg_write_ensure(hdev, chip_info, H60_CS_CAL_MODE_APIf, CS_REG_OFFSET(chip_info), (u32)cal_mode);
    ret |= h60_init_reg_write_ensure(hdev, chip_info, H60_CS_POWER_MODE_APIf, CS_REG_OFFSET(chip_info), 0);
    return ret;
}

STATIC void hisds_cs_calib_clk_cfg_h60(struct hclge_dev *hdev, CHIP_INFO_S *chip_info, u8 pll_id)
{
    STATIC SERDES_REG_FIELD_S cfg_reg[HILINK_SERDES_MAX_CS_NUM] = {
        {H60_PLL0CORECLKDIVRSTNf},
        {H60_PLL1CORECLKDIVRSTNf},
    };
    u32 core_clk_sel = 0;
    u32 reg_val;

    /* core_clk controlled by register configure, read 3 times for reading robustness */
    h60_read_reg_field(hdev, H60_CS_CORECLK_SELEXT_FROMPINf, CS_REG_OFFSET(chip_info), &core_clk_sel);
    h60_read_reg_field(hdev, H60_CS_CORECLK_SELEXT_FROMPINf, CS_REG_OFFSET(chip_info), &reg_val);
    core_clk_sel &= reg_val;
    h60_read_reg_field(hdev, H60_CS_CORECLK_SELEXT_FROMPINf, CS_REG_OFFSET(chip_info), &reg_val);
    core_clk_sel &= reg_val;
    if (core_clk_sel == 0) {
        /* Use Refclk for coreclk source */
        h60_write_reg_field(hdev, H60_CORECLKSELf, CS_REG_OFFSET(chip_info), 0);
    }

    /* Reset PLL coreclk2dig output */
    h60_write_reg_field(hdev, cfg_reg[pll_id].reg_id, cfg_reg[pll_id].field_id, CS_REG_OFFSET(chip_info), 0);
}

STATIC u32 hisds_macro_cs_cfg_before_calib_h60(struct hclge_dev *hdev, CHIP_INFO_S *chip_info,
    HILINK_MACRO_INIT_S *macro_info, u8 pll_id)
{
    u32 ret = SERDES_OK;

    hisds_cs_calib_clk_cfg_h60(hdev, chip_info, pll_id);

    /* pll power up */
    ret = hisds_cs_pll_power_up_h60(hdev, chip_info, pll_id);
    if (ret != SERDES_OK) {
        dev_err(&hdev->pdev->dev, "power up pll %d fail, ret=%u\n", pll_id, ret);
        return ret;
    }

    /* PLL frequency divider configuration */
    ret = hisds_cs_cfg_freq_plan_h60(hdev, chip_info, macro_info, pll_id);
    if (ret != SERDES_OK) {
        dev_err(&hdev->pdev->dev, "cfg freq plan fail, serdes_rate=%d ret=%u\n",
                macro_info->cs_info[pll_id].serdes_rate, ret);
        return ret;
    }

    ret = hisds_cs_calib_mode_cfg_h60(hdev, chip_info, pll_id);
    if (ret != SERDES_OK) {
        dev_err(&hdev->pdev->dev, "mode cfg pll %d fail, ret=%u\n", pll_id, ret);
        return ret;
    }

    /* set CORECLK_OPT_BYPASS_API to 0 for higher coreclk frequency */
    h60_write_reg_field(hdev, H60_CORECLK_OPT_BYPASS_APIf, CS_REG_OFFSET(chip_info), 0);
    return ret;
}

STATIC u32 hisds_cs_calib_status_clear_h60(struct hclge_dev *hdev, CHIP_INFO_S *chip_info)
{
    u32 ret = SERDES_OK;
    u32 calib_status = 0;

    h60_read_reg_field(hdev, H60_CS_CALIB_STATUS_APIf, CS_REG_OFFSET(chip_info), &calib_status);

    if (calib_status != 0) {
        h60_write_reg_field(hdev, H60_CS_CALIB_EN_APIf, CS_REG_OFFSET(chip_info), 0x2);
        /* wait 10000 times * 1000 us = 10s */
        ret = h60_wait_reg_value(hdev, chip_info, H60_CS_CALIB_STATUS_APIf, CS_REG_OFFSET(chip_info), 0x0, 10000, 1000);
    }

    return ret;
}

STATIC u32 hisds_do_cs_calib(struct hclge_dev *hdev, CHIP_INFO_S *chip_info)
{
    u32 ret = SERDES_OK;
    u32 mcurst_val = 0;

    ret = hisds_cs_calib_status_clear_h60(hdev, chip_info);
    if (ret != SERDES_OK) {
        dev_err(&hdev->pdev->dev, "clear cs calib status fail, ret=%u\n", ret);
        return ret;
    }

    h60_write_reg_field(hdev, H60_CS_CALIB_EN_APIf, CS_REG_OFFSET(chip_info), 0x1);
    /* wait 10000 times * 1000 us = 10s */
    ret = h60_wait_reg_value(hdev, chip_info, H60_CS_CALIB_STATUS_APIf, CS_REG_OFFSET(chip_info), 0x1, 10000, 1000);

    if (ret != SERDES_OK) {
        h60_read_reg_field(hdev, H60_MCURSTBf, CS_REG_OFFSET(chip_info), &mcurst_val);
        dev_err(&hdev->pdev->dev, "cs calib fail, mcurst=0x%x, ret=%u\n", mcurst_val, ret);
    }
    return ret;
}

STATIC u32 hisds_macro_cs_fw_calib_h60(struct hclge_dev *hdev, CHIP_INFO_S *chip_info)
{
    u32 pll0_out_of_lock = 0; /* 0 lcoked, 1 out of lock */
    u32 pll1_out_of_lock = 0;
    u32 ret = SERDES_OK;
    u32 cal_mode = 0;

    ret = hisds_do_cs_calib(hdev, chip_info);

    h60_read_reg_field(hdev, H60_CS_CAL_MODE_APIf, CS_REG_OFFSET(chip_info), &cal_mode);
    // delay 10000us, then check pll lock status
    usleep_range(10000, 10000);
    h60_read_reg_field(hdev, H60_PLL0OUTOFLOCKf, CS_REG_OFFSET(chip_info), &pll0_out_of_lock);
    h60_read_reg_field(hdev, H60_PLL1OUTOFLOCKf, CS_REG_OFFSET(chip_info), &pll1_out_of_lock);

    if (ret != SERDES_OK) {
        dev_err(&hdev->pdev->dev, "do cs calib first fail, ret=%u\n", ret);
        goto cs_fw_clib_err_process;
    }

    if (cal_mode == H60_CS_PLL0_VCO_CAL && pll0_out_of_lock == 1) {
        ret = hisds_do_cs_calib(hdev, chip_info);
    } else if (cal_mode == H60_CS_PLL1_VCO_CAL && pll1_out_of_lock == 1) {
        ret = hisds_do_cs_calib(hdev, chip_info);
    }
    if (ret != SERDES_OK) {
        dev_err(&hdev->pdev->dev, "do cs calib second fail, ret=%u\n", ret);
        goto cs_fw_clib_err_process;
    }

    return ret;

cs_fw_clib_err_process:
    dev_err(&hdev->pdev->dev, "cs calib failed! chip:%d macro:%d cal_mode:0x%x, pll0:0x%x, pll1:0x%x\n",
            chip_info->chip_id, chip_info->macro_id, cal_mode, pll0_out_of_lock, pll1_out_of_lock);
    return ret;
}

STATIC u32 hisds_cs_switch_rate_h60(struct hclge_dev *hdev, CHIP_INFO_S *chip_info, HILINK_MACRO_INIT_S *macro_info,
    u8 pll_id)
{
    u32 ret;

    ret = hisds_ds_freeze_adaptation_h60(hdev, chip_info);
    if (ret != SERDES_OK) {
        dev_err(&hdev->pdev->dev, "cs freeze adaption failed\n");
        return ret;
    }

    ret = hisds_macro_cs_cfg_before_calib_h60(hdev, chip_info, macro_info, pll_id);
    if (ret != SERDES_OK) {
        dev_err(&hdev->pdev->dev, "cs cfg before calib fail! pll_id=%d ret=%u\n", pll_id, ret);
        hisds_ds_unfreeze_adaptation_h60(hdev, chip_info);
        return ret;
    }

    ret = hisds_macro_cs_fw_calib_h60(hdev, chip_info);
    hisds_ds_unfreeze_adaptation_h60(hdev, chip_info);
    return ret;
}

STATIC u32 hisds_ds_cfg_before_data_rate_switch_h60(struct hclge_dev *hdev, CHIP_INFO_S *chip_info)
{
    u32 ret = SERDES_OK;

    ret = hisds_ds_freeze_adaptation_h60(hdev, chip_info);
    hisds_restore_high_speed_txpa_lock(hdev, chip_info);
    return ret;
}

STATIC void hisds_tx_gray_cfg_h60(struct hclge_dev *hdev, CHIP_INFO_S *chip_info, u8 pam4_en)
{
    u8 tx_gray = (pam4_en == H60_PAM4_MODE_EN ? 0 : 1); /* 0: gray encoder is used, 1: bypassed (default) */

    h60_write_reg_field(hdev, H60_TX_GRAY_BYPASSf, DS_REG_OFFSET(chip_info), tx_gray);
    h60_write_reg_field(hdev, H60_TX_MOD4ENC_BYPASSf, DS_REG_OFFSET(chip_info), 1);
    h60_write_reg_field(hdev, H60_TXSYMBORDERf, DS_REG_OFFSET(chip_info), 0);
}

STATIC void hisds_rx_gray_cfg_h60(struct hclge_dev *hdev, CHIP_INFO_S *chip_info, u8 pam4_en)
{
    u8 rx_gray = (pam4_en == H60_PAM4_MODE_EN ? 0 : 1); /* 0: enable, 1: disable */

    h60_write_reg_field(hdev, H60_RXGRAYDECf, DS_REG_OFFSET(chip_info), rx_gray);
    h60_write_reg_field(hdev, H60_RX_PRBS_MOD4_BYPASSf, DS_REG_OFFSET(chip_info), 1);
    h60_write_reg_field(hdev, H60_RX_PRBS_GRAY_BYPASSf, DS_REG_OFFSET(chip_info), rx_gray);
}

STATIC u32 hisds_ds_data_rate_switch_h60(struct hclge_dev *hdev, CHIP_INFO_S *chip_info,
    HILINK_MACRO_INIT_S *macro_info)
{
    u8 ds_num = chip_info->ds_num;
    H60_FREQ_CFG *freq = NULL;
    u8 rate_mode;
    u8 ds_index;
    u8 pam4_en;
    u8 pll_id;
    u8 speed;

    for (ds_index = 0; ds_index < ds_num; ds_index++) {
        if (GET_BIT(chip_info->ds_mask, ds_index) == 0) {
            continue;
        }

        chip_info->csds_id = ds_index;
        // set spine clock seletion for TX/RX clocking
        pll_id = macro_info->ds_info[ds_index].tx_cs_sel;
        h60_write_reg_field(hdev, H60_DSCLK_BUS_SELTXf, DS_REG_OFFSET(chip_info), pll_id);
        h60_write_reg_field(hdev, H60_DSCLK_BUS_SELRXf, DS_REG_OFFSET(chip_info), pll_id);

        speed = macro_info->cs_info[pll_id].serdes_rate;
        freq = hisds_get_freq_data_h60(speed);
        if (freq == NULL) {
            dev_err(&hdev->pdev->dev, "freq is null\n");
            return SERDES_NULL;
        }

        rate_mode = freq->rate_mode_div;
        pam4_en = freq->pam4mode;
        // set TX/RX rate mode, data mode, gray cfg
        h60_write_reg_field(hdev, H60_TXRATEMODEf, DS_REG_OFFSET(chip_info), rate_mode);
        h60_write_reg_field(hdev, H60_RXRATEMODEf, DS_REG_OFFSET(chip_info), rate_mode);
        h60_write_reg_field(hdev, H60_TX_PAM4_MODEf, DS_REG_OFFSET(chip_info), pam4_en);
        h60_write_reg_field(hdev, H60_MODCONFIGf, DS_REG_OFFSET(chip_info), pam4_en);
        hisds_tx_gray_cfg_h60(hdev, chip_info, pam4_en);
        hisds_rx_gray_cfg_h60(hdev, chip_info, pam4_en);
    }

    return SERDES_OK;
}

STATIC u32 hisds_ds_rx_trkclk_delay_set_h60(struct hclge_dev *hdev, CHIP_INFO_S *chip_info)
{
    u32 ret = SERDES_OK;

    h60_write_reg_field(hdev, H60_RXCAL_GOSC_ENf, DS_REG_OFFSET(chip_info), 1);
    h60_write_reg_field(hdev, H60_RXCAL_GOSC_MODEf, DS_REG_OFFSET(chip_info), 0);
    h60_write_reg_field(hdev, H60_RXCAL_NUM_CLK_CYCLESf, DS_REG_OFFSET(chip_info), 2048); /* 2048 clock cycles */
    h60_write_reg_field(hdev, H60_RX_SAR_DAC_WRENf, DS_REG_OFFSET(chip_info), 1);
    h60_write_reg_field(hdev, H60_RX_SAR_SIZE_REG_SELf, DS_REG_OFFSET(chip_info), 0);
    h60_write_reg_field(hdev, H60_RX_SAR_DAC_REG_SELf, DS_REG_OFFSET(chip_info), 1);

    /* sar track delay */
    h60_write_reg_field(hdev, H60_RX_SAR_BK0_TRKCLK_DELAYf, DS_REG_OFFSET(chip_info), 0xF);
    h60_write_reg_field(hdev, H60_RX_SAR_BK90_TRKCLK_DELAYf, DS_REG_OFFSET(chip_info), 0xF);
    h60_write_reg_field(hdev, H60_RX_SAR_BK180_TRKCLK_DELAYf, DS_REG_OFFSET(chip_info), 0xF);
    h60_write_reg_field(hdev, H60_RX_SAR_BK270_TRKCLK_DELAYf, DS_REG_OFFSET(chip_info), 0xF);

    ret |= h60_init_reg_write_ensure(hdev, chip_info, H60_RX_SAR_DAC_WRENf, DS_REG_OFFSET(chip_info), 0);
    ret |= h60_init_reg_write_ensure(hdev, chip_info, H60_RX_SAR_DAC_COUNT_RSTBf, DS_REG_OFFSET(chip_info), 1);
    ret |= h60_init_reg_write_ensure(hdev, chip_info, H60_RX_SAR_SIZE_REG_SELf, DS_REG_OFFSET(chip_info), 0);
    ret |= h60_init_reg_write_ensure(hdev, chip_info, H60_RX_SAR_DAC_REG_SELf, DS_REG_OFFSET(chip_info), 0);
    return ret;
}

STATIC void hisds_set_tx_fir_h60(struct hclge_dev *hdev, CHIP_INFO_S *chip_info,const HILINK_TX_PARA_S *ffe_cfg)
{
    u32 txdrv_tap_pwd = 0;

    set_tx_fir(hdev, chip_info, ffe_cfg->fir_pre1, ffe_cfg->fir_post1);

    /* hisds power save fir */
    h60_read_reg_field(hdev, H60_TX_DRV_TAP_PWRDNBf, DS_REG_OFFSET(chip_info), &txdrv_tap_pwd);
    /* Save power if pre is zero */
    if (ffe_cfg->fir_pre1 == 0) {
        txdrv_tap_pwd = (txdrv_tap_pwd & 0xEEEU);
    }
    /* Save power if post is zero */
    if (ffe_cfg->fir_post1 == 0) {
        txdrv_tap_pwd = (txdrv_tap_pwd & 0x777U);
    }
    h60_write_reg_field(hdev, H60_TX_DRV_TAP_PWRDNBf, DS_REG_OFFSET(chip_info), txdrv_tap_pwd);
    return;
}

STATIC u32 hisds_ds_align_txpa_slave_sw_align_h60(struct hclge_dev *hdev, CHIP_INFO_S *chip_info)
{
    u32 curr_out_of_band = 0;
    u32 curr_slave_flag = 0;
    u32 prev_out_of_band;
    u32 prev_slave_flag;
    u32 tx_bit_slip = 0;
    u8 time_out = 1000;
    u8 i;

    /* Get the current state of tx_bitslip control signal */
    h60_read_reg_field(hdev, H60_TX_BITSLIPf, DS_REG_OFFSET(chip_info), &tx_bit_slip);

    /* Toggle tx_bitslip until the out-of-band signal transitions from 3 to 0 */
    h60_read_reg_field(hdev, H60_TXPA_OUTOFBANDf, DS_REG_OFFSET(chip_info), &curr_out_of_band);
    for (i = 0; i < time_out; i++) {
        prev_out_of_band = curr_out_of_band;
        tx_bit_slip = 1 - tx_bit_slip;
        h60_write_reg_field(hdev, H60_TX_BITSLIPf, DS_REG_OFFSET(chip_info), tx_bit_slip);
        h60_read_reg_field(hdev, H60_TXPA_OUTOFBANDf, DS_REG_OFFSET(chip_info), &curr_out_of_band);
        /* out-of-band signal transitions from 3 to 0 */
        if ((prev_out_of_band == 3) && (curr_out_of_band == 0)) {
            break;
        }
    }
    if (i >= time_out) {
        dev_err(&hdev->pdev->dev, "chip%d macro%d ds%d waite out-of-band signal transitions from 3 to 0 timeout\n",
                chip_info->chip_id, chip_info->macro_id, chip_info->csds_id);
        return SERDES_TIMEOUT;
    }

    /* Toggle tx_bitslip until the txpa_slave_flag transitions from 0 to 1 */
    h60_read_reg_field(hdev, H60_TXPA_SLAVE_FLAGf, DS_REG_OFFSET(chip_info), &curr_slave_flag);
    for (i = 0; i < time_out; i++) {
        prev_slave_flag = curr_slave_flag;
        tx_bit_slip = 1 - tx_bit_slip;
        h60_write_reg_field(hdev, H60_TX_BITSLIPf, DS_REG_OFFSET(chip_info), tx_bit_slip);
        h60_read_reg_field(hdev, H60_TXPA_SLAVE_FLAGf, DS_REG_OFFSET(chip_info), &curr_slave_flag);
        /* txpa_slave_flag transitions from 0 to 1 */
        if ((prev_slave_flag == 0) && (curr_slave_flag == 1)) {
            break;
        }
    }
    if (i >= time_out) {
        dev_err(&hdev->pdev->dev, "chip%d macro%d ds%d waite txpa_slave_flag transitions from 0 to 1 timeout\n",
                chip_info->chip_id, chip_info->macro_id, chip_info->csds_id);
        return SERDES_TIMEOUT;
    }

    return SERDES_OK;
}

STATIC u32 hisds_macro_ds_align_txpa_slave_h60(struct hclge_dev *hdev, CHIP_INFO_S *chip_info)
{
    u8 ds_num = chip_info->ds_num;
    u32 txalign_mode = 0;
    u32 ret = SERDES_OK;
    u8 ds_index;

    for (ds_index = 0; ds_index < ds_num; ds_index++) {
        chip_info->csds_id = ds_index;

        h60_read_reg_field(hdev, H60_TXALIGNMODEf, DS_REG_OFFSET(chip_info), &txalign_mode);
        /* TX PA in mission Slave mode */
        if (txalign_mode == 1) {
            ret = hisds_ds_align_txpa_slave_sw_align_h60(hdev, chip_info);
        } else if (txalign_mode == 0) {
            h60_write_reg_field(hdev, H60_TXPA_ACCDEPTHf, DS_REG_OFFSET(chip_info), 0x7);
        } else {
            dev_err(&hdev->pdev->dev, "txalign_mode %u is not supported\n", txalign_mode);
            return SERDES_NOT_SUPPORT;
        }
    }

    return ret;
}

STATIC u32 hisds_ds_cfg_after_data_rate_switch_h60(struct hclge_dev *hdev, CHIP_INFO_S *chip_info,
    HILINK_MACRO_INIT_S *macro_info)
{
    u8 ds_num = chip_info->ds_num;
    u32 ret = SERDES_OK;
    u8 ds_index;

    for (ds_index = 0; ds_index < ds_num; ds_index++) {
        if (GET_BIT(chip_info->ds_mask, ds_index) == 0) {
            continue;
        }

        chip_info->csds_id = ds_index;
        ret |= hisds_ds_rx_trkclk_delay_set_h60(hdev, chip_info);
        hisds_set_tx_fir_h60(hdev, chip_info, &macro_info->ds_info[ds_index].tx_cfg);
    }

    /* set master slave mode */
    ret |= hisds_macro_ds_align_txpa_slave_h60(hdev, chip_info);
    return ret;
}

STATIC u32 hisds_ds_switch_rate_h60(struct hclge_dev *hdev, CHIP_INFO_S *chip_info, HILINK_MACRO_INIT_S *macro_info)
{
    u32 ret = SERDES_OK;

    ret = hisds_ds_cfg_before_data_rate_switch_h60(hdev, chip_info);
    if (ret != SERDES_OK) {
        dev_err(&hdev->pdev->dev, "ds cfg before data rate switch failed, ret=%u\n", ret);
        return ret;
    }

    ret = hisds_ds_data_rate_switch_h60(hdev, chip_info, macro_info);
    ret |= hisds_ds_cfg_after_data_rate_switch_h60(hdev, chip_info, macro_info);

    return ret;
}

STATIC u32 hisds_data_rate_switch(struct hclge_dev *hdev, CHIP_INFO_S *chip_info, HILINK_MACRO_INIT_S *macro_info,
    u8 h60_speed)
{
    u8 pll_sel = HILINK_SERDES_PLL_ID_0;
    SWITCH_STRATEGY_E strategy;
    u32 ret = SERDES_OK;

    strategy = hisds_get_switch_strategy_h60(hdev, chip_info, h60_speed, &pll_sel);
    if (strategy == UNSUPPORT_SWITCH) {
        dev_err(&hdev->pdev->dev, "strategy is not support to switch\n");
        return SERDES_NOT_SUPPORT;
    }

    hisds_update_macro_info_h60(chip_info, macro_info, h60_speed, pll_sel);
    if (strategy == CHANGE_PLL_FREQ) {
        ret = hisds_cs_switch_rate_h60(hdev, chip_info, macro_info, pll_sel);
        if (ret != SERDES_OK) {
            dev_err(&hdev->pdev->dev, "hisds cs switch rate fail, strategy:%u, ret=%u\n", strategy, ret);
            return ret;
        }
    }

    return hisds_ds_switch_rate_h60(hdev, chip_info, macro_info);
}

STATIC u32 chip_serdes_datarate_switch(struct hclge_dev *hdev, CHIP_INFO_S *chip_info, u8 h60_speed)
{
    HILINK_MACRO_INIT_S macro_info = {0};
    u32 data_rate_mhz = 0;

    hisds_get_data_rate_h60(hdev, chip_info, SERDES_RX, &data_rate_mhz);
    // serdes rate matched, no need to do datarate switch
    if (h60_speed == (u8)(data_rate_mhz / H60_DATA_RATE_GB_SCALAR)) {
        return SERDES_OK;
    }

    return hisds_data_rate_switch(hdev, chip_info, &macro_info, h60_speed);
}

int hclge_do_h60_datarate_switch(struct hclge_dev *hdev, u8 eth_speed)
{
    u8 h60_speed = H60_DATA_RATE_UNKNOWN;
    CHIP_INFO_S chip_info = {0};
    int ret = SERDES_OK;

    h60_speed = hclge_sds_rate_mode_convert(eth_speed);
    if (h60_speed == H60_DATA_RATE_UNKNOWN) {
        dev_err(&hdev->pdev->dev, "hclge sds rate mode convert eth_speed 0x%x unknown\n", eth_speed);
        return SERDES_ERROR_PARA;
    }

    if (macro0_in_use(hdev)) {
        hclge_init_chip_info(hdev, &chip_info, H60_MACRO_OFFSET_0);
        ret |= chip_serdes_datarate_switch(hdev, &chip_info, h60_speed);
    }

    if (macro1_in_use(hdev)) {
        hclge_init_chip_info(hdev, &chip_info, H60_MACRO_OFFSET_1);
        ret |= chip_serdes_datarate_switch(hdev, &chip_info, h60_speed);
    }

    return ret;
}
