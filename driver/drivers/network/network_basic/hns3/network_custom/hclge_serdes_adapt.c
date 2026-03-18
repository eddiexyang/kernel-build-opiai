/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 * Create: 2023-2-7
 */
#include "hclge_main.h"
#include "hclge_serdes_reg_macro.h"
#include "hclge_serdes_common.h"
#include "hclge_serdes_adapt.h"

#ifndef DEFINE_HNS_LLT
#define STATIC static
#else
#define STATIC
#endif

STATIC u32 clear_online_adapt_status_h60(struct hclge_dev *hdev, CHIP_INFO_S *chip_info)
{
    u8 ds_num = chip_info->ds_num;
    u8 ds_mask = chip_info->ds_mask;
    u32 reg_val = 0;
    u32 ds_index;
    u32 ret = 0;

    for (ds_index = 0; ds_index < ds_num; ds_index++) {
        if (GET_BIT(ds_mask, ds_index) == 0) {
            continue;
        }
        chip_info->csds_id = (u8)ds_index;
        h60_read_reg_field(hdev, H60_CONT_ADAPT_STATUS_APIf, DSAPI_REG_OFFSET(chip_info), &reg_val);
        if (reg_val != 0) {
            h60_write_reg_field(hdev, H60_CONT_ADAPT_EN_APIf, DSAPI_REG_OFFSET(chip_info), 0x2);
        }
    }

    for (ds_index = 0; ds_index < ds_num; ds_index++) {
        if (GET_BIT(ds_mask, ds_index) == 0) {
            continue;
        }
        chip_info->csds_id = (u8)ds_index;
        /* 1000 times x 1000 us = 1s */
        ret |= h60_wait_reg_value(hdev, chip_info, H60_CONT_ADAPT_STATUS_APIf,
            DSAPI_REG_OFFSET(chip_info), 0x0, 1000, 1000);
        if (ret) {
            return SERDES_ERROR;
        }
    }

    return SERDES_OK;
}

STATIC u32 clear_init_adapt_status_h60(struct hclge_dev *hdev, CHIP_INFO_S *chip_info)
{
    u8 ds_num = chip_info->ds_num;
    u8 ds_mask = chip_info->ds_mask;
    u32 reg_val = 0;
    u32 ds_index;
    u32 ret = 0;

    for (ds_index = 0; ds_index < ds_num; ds_index++) {
        if (GET_BIT(ds_mask, ds_index) == 0) {
            continue;
        }
        chip_info->csds_id = (u8)ds_index;
        h60_read_reg_field(hdev, H60_INIT_ADAPT_STATUS_APIf, DSAPI_REG_OFFSET(chip_info), &reg_val);
        if (reg_val != 0) {
            h60_write_reg_field(hdev, H60_INIT_ADAPT_EN_APIf, DSAPI_REG_OFFSET(chip_info), 0x2);
        }
    }

    for (ds_index = 0; ds_index < ds_num; ds_index++) {
        if (GET_BIT(ds_mask, ds_index) == 0) {
            continue;
        }
        chip_info->csds_id = (u8)ds_index;
        ret |= h60_wait_reg_value(hdev, chip_info, H60_INIT_ADAPT_STATUS_APIf, DSAPI_REG_OFFSET(chip_info),
            0x0, 1000, 1000);
        if (ret) {
            return SERDES_ERROR;
        }
    }

    return SERDES_OK;
}

u32 hisds_ds_freeze_adaptation_h60(struct hclge_dev *hdev, CHIP_INFO_S *chip_info)
{
    u8 ds_num = chip_info->ds_num;
    u32 ret = SERDES_OK;
    u32 ds_state = 0;
    u32 reg_val = 0;
    u8 ds_index;

    // clear init adapt and online adapt status
    for (ds_index = 0; ds_index < ds_num; ds_index++) {
        if (GET_BIT(chip_info->ds_mask, ds_index) == 0) {
            continue;
        }

        chip_info->csds_id = ds_index;
        h60_write_reg_field(hdev, H60_CS_READTEMPBYPASS_APIf, CS_REG_OFFSET(chip_info), 1);
        h60_read_reg_field(hdev, H60_DS_STATE_APIf, DSAPI_REG_OFFSET(chip_info), &ds_state);
        if (ds_state == H60_DS_STATE_INIT_ADAPT) {
            h60_write_reg_field(hdev, H60_INIT_ADAPT_EN_APIf, DSAPI_REG_OFFSET(chip_info), 0x2);
            h60_read_reg_field(hdev, H60_CONT_ADAPT_EN_APIf, DSAPI_REG_OFFSET(chip_info), &reg_val);
            if (reg_val == 1) {
                h60_write_reg_field(hdev, H60_CONT_ADAPT_EN_APIf, DSAPI_REG_OFFSET(chip_info), 0x2);
            }
        }

        if (ds_state == H60_DS_STATE_CONT_ADAPT || ds_state == H60_DS_STATE_ABIST) {
            h60_write_reg_field(hdev, H60_CONT_ADAPT_EN_APIf, DSAPI_REG_OFFSET(chip_info), 0x2);
        }
    }

    // set ds state to idle
    for (ds_index = 0; ds_index < ds_num; ds_index++) {
        if (GET_BIT(chip_info->ds_mask, ds_index) == 0) {
            continue;
        }

        chip_info->csds_id = ds_index;
        /* wait 1000 times x 1000 us = 1s */
        ret |= h60_wait_reg_value(hdev, chip_info, H60_DS_STATE_APIf, DSAPI_REG_OFFSET(chip_info), 0xF, 1000, 1000);
    }

    return ret;
}

u32 hisds_ds_unfreeze_adaptation_h60(struct hclge_dev *hdev, CHIP_INFO_S *chip_info)
{
    u8 ds_num = chip_info->ds_num;
    u32 ret = SERDES_OK;
    u32 reg_val = 0;
    u8 ds_index;

    for (ds_index = 0; ds_index < ds_num; ds_index++) {
        if (GET_BIT(chip_info->ds_mask, ds_index) == 0) {
            continue;
        }

        chip_info->csds_id = ds_index;
        h60_read_reg_field(hdev, H60_INIT_ADAPT_EN_APIf, DSAPI_REG_OFFSET(chip_info), &reg_val);
        if (reg_val == 0x2) {
            h60_write_reg_field(hdev, H60_INIT_ADAPT_EN_APIf, DSAPI_REG_OFFSET(chip_info), 0x1);
        }

        h60_read_reg_field(hdev, H60_CONT_ADAPT_EN_APIf, DSAPI_REG_OFFSET(chip_info), &reg_val);
        if (reg_val == 0x2) {
            h60_write_reg_field(hdev, H60_CONT_ADAPT_EN_APIf, DSAPI_REG_OFFSET(chip_info), 0x1);
        }

        h60_write_reg_field(hdev, H60_CS_READTEMPBYPASS_APIf, CS_REG_OFFSET(chip_info), 0x0);
    }

    return ret;
}

STATIC void hisds_adapt_bypass_cfg(struct hclge_dev *hdev, CHIP_INFO_S *chip_info, u32 rx_date_rate)
{
    if (rx_date_rate >= 5000 && rx_date_rate <= 7500) { /* 5000khz ~  7500khz*/
        h60_write_reg_field(hdev, H60_CONT_ABGCBYPASS_APIf, DSAPI_REG_OFFSET(chip_info), 0x1);
        h60_write_reg_field(hdev, H60_CONT_SAROFSTBYPASS_APIf, DSAPI_REG_OFFSET(chip_info), 0x1);
        h60_write_reg_field(hdev, H60_CONT_TNHGAINBYPASS_APIf, DSAPI_REG_OFFSET(chip_info), 0x1);
        h60_write_reg_field(hdev, H60_CONT_CDRPREBYPASS_APIf, DSAPI_REG_OFFSET(chip_info), 0x1);
    } else if (rx_date_rate > 7500 && rx_date_rate <= 30000) { /* 7500khz ~  30000khz*/
        h60_write_reg_field(hdev, H60_CONT_ABGCBYPASS_APIf, DSAPI_REG_OFFSET(chip_info), 0x0);
        h60_write_reg_field(hdev, H60_CONT_SAROFSTBYPASS_APIf, DSAPI_REG_OFFSET(chip_info), 0x1);
        h60_write_reg_field(hdev, H60_CONT_TNHGAINBYPASS_APIf, DSAPI_REG_OFFSET(chip_info), 0x1);
        h60_write_reg_field(hdev, H60_CONT_CDRPREBYPASS_APIf, DSAPI_REG_OFFSET(chip_info), 0x0);
    } else if (rx_date_rate > 30000 && rx_date_rate <= 58125) { /* 30000khz ~  58125khz*/
        h60_write_reg_field(hdev, H60_CONT_ABGCBYPASS_APIf, DSAPI_REG_OFFSET(chip_info), 0x0);
        h60_write_reg_field(hdev, H60_CONT_SAROFSTBYPASS_APIf, DSAPI_REG_OFFSET(chip_info), 0x0);
        h60_write_reg_field(hdev, H60_CONT_TNHGAINBYPASS_APIf, DSAPI_REG_OFFSET(chip_info), 0x1);
        h60_write_reg_field(hdev, H60_CONT_CDRPREBYPASS_APIf, DSAPI_REG_OFFSET(chip_info), 0x0);
    }
}

STATIC u32 get_A_X_board_rx_insertion_loss(CHIP_INFO_S *chip_info)
{
    u32 rx_insertion_loss;
    u32 ham_npu_id = chip_info->chip_id % A_X_BOARD_INSERT_LOSS_CYCLE_NUM;

    switch(ham_npu_id) {
        case HAM1_NPU0:
            rx_insertion_loss = HAM1_NPU0_ETH_M0_RX_INSERTION_LOSS;
            break;
        case HAM1_NPU1:
            rx_insertion_loss = HAM1_NPU1_ETH_M0_RX_INSERTION_LOSS;
            break;
        case HAM2_NPU0:
            rx_insertion_loss = HAM2_NPU0_ETH_M0_RX_INSERTION_LOSS;
            break;
        case HAM2_NPU1:
            rx_insertion_loss = HAM2_NPU1_ETH_M0_RX_INSERTION_LOSS;
            break;
        default:
            rx_insertion_loss = HAM1_NPU0_ETH_M0_RX_INSERTION_LOSS;
            break;
    }

    return rx_insertion_loss;
}

void set_tx_fir(struct hclge_dev *hdev, CHIP_INFO_S *chip_info, int txpre, int txpost)
{
    // rate mode full, set programmable delay for FIR taps to 0
    h60_write_reg_field(hdev, H60_TX_DRV_TAPDEL_SELf, DS_REG_OFFSET(chip_info), 0x000);
    /* enable FIR taps */
    h60_write_reg_field(hdev, H60_TX_DRV_TAP_PWRDNBf, DS_REG_OFFSET(chip_info), 0xFFF);
    /* enable hardware tap mapper */
    h60_write_reg_field(hdev, H60_TXDRV_TAPMAP_ENf, DS_REG_OFFSET(chip_info), 0x1);
    /* pre cfg: 1 = negative, 0 = positive */
    h60_write_reg_field(hdev, H60_TXFIRPRE1_SIGNf, DS_REG_OFFSET(chip_info), (u32)((txpre < 0) ? 0x1 : 0x0));
    h60_write_reg_field(hdev, H60_TXFIRPRE1f, DS_REG_OFFSET(chip_info), abs(txpre));
    /* post cfg: 1 = negative, 0 = positive */
    h60_write_reg_field(hdev, H60_TXFIRPOST_SIGNf, DS_REG_OFFSET(chip_info), (u32)((txpost < 0) ? 0x1 : 0x0));
    h60_write_reg_field(hdev, H60_TXFIRPOSTf, DS_REG_OFFSET(chip_info), abs(txpost));
}

STATIC void hisds_config_rx_insertion_loss(struct hclge_dev *hdev, CHIP_INFO_S *chip_info)
{
    u32 rx_insertion_loss;

    switch (hdev->board_type) {
        case EVB_BOARD:
            rx_insertion_loss = EVB_RX_INSERTION_LOSS;
            break;
        case A_X_BOARD:
            rx_insertion_loss = get_A_X_board_rx_insertion_loss(chip_info);
            set_tx_fir(hdev, chip_info, A_X_TX_FFE_PRE_VAL, A_X_TX_FFE_POST_VAL);
            break;
        case PoD_BOARD:
            rx_insertion_loss = PoD_RX_INSERTION_LOSS;
            set_tx_fir(hdev, chip_info, PoD_TX_FFE_PRE_VAL, PoD_TX_FFE_POST_VAL);
            break;
        case A_K_BOARD:
            rx_insertion_loss = PoD_RX_INSERTION_LOSS;
            set_tx_fir(hdev, chip_info, A_K_TX_FFE_PRE_VAL, A_K_TX_FFE_POST_VAL);
            break;
        case PoD_BUSINESS_BOARD:
            rx_insertion_loss = PoD_RX_INSERTION_LOSS;
            set_tx_fir(hdev, chip_info, PoD_BUSINESS_TX_FFE_PRE_VAL, PoD_BUSINESS_TX_FFE_POST_VAL);
            break;
        default:
            rx_insertion_loss = EVB_RX_INSERTION_LOSS;
            break;
    }
    h60_write_reg_field(hdev, H60_DS_RX_INSERTION_LOSS_APIf, DSAPI_REG_OFFSET(chip_info), rx_insertion_loss);
}

STATIC void hisds_config_polarity(struct hclge_dev *hdev, CHIP_INFO_S *chip_info)
{
    u32 tx_invert_val = 0; /* tx polarity value: 0=non-invert, 1=inverted */
    u32 rx_invert_val = 0; /* rx polarity value: 0=non-invert, 1=inverted */

    if (hdev->board_type == A_X_BOARD) {
        if (hdev->device_id == DEVICE_1) {
            tx_invert_val = 0;
        } else {
            tx_invert_val = 1;
        }

        if (hdev->device_id == DEVICE_0 || hdev->device_id == DEVICE_1 ||
            hdev->device_id == DEVICE_3 || hdev->device_id == DEVICE_13) {
            rx_invert_val = 0;
        } else {
            rx_invert_val = 1;
        }
    } else if (hdev->board_type == PoD_BUSINESS_BOARD) {
        tx_invert_val = 0;
        rx_invert_val = 1;
    }

    h60_write_reg_field(hdev, H60_TXPOLARITYf, DS_REG_OFFSET(chip_info), tx_invert_val);
    h60_write_reg_field(hdev, H60_RXPOLARITYf, DS_REG_OFFSET(chip_info), rx_invert_val);
}

/*
 * if adapt_type is SERDES_ADAPT_STEP_INIT, this function executes ds adaptation to complete
 * if adapt_type is SERDES_ADAPT_STEP_CONT, this function only executes ds online adaptation
 */
STATIC u32 adapt_process_h60(struct hclge_dev *hdev, CHIP_INFO_S *chip_info, SERDES_ADAPT_STEP_E adapt_type)
{
    struct pci_dev *pdev = hdev->pdev;
    u8 ds_mask = chip_info->ds_mask;
    u8 ds_num = chip_info->ds_num;
    u32 rx_insertion_loss = 0;
    u32 insertion_loss_ajust;
    u32 rx_date_rate = 0;
    u32 ds_index;
    u32 ret = 0;

    ret = clear_online_adapt_status_h60(hdev, chip_info);
    if (ret) {
        dev_err(&pdev->dev, "clear_online_adapt_status_h60 failed! ret is [%d]\n", ret);
        return ret;
    }
    ret = clear_init_adapt_status_h60(hdev, chip_info);
    if (ret) {
        dev_err(&pdev->dev, "clear_init_adapt_status_h60 failed! ret is [%d]\n", ret);
        return ret;
    }

    for (ds_index = 0; ds_index < ds_num; ds_index++) {
        if (GET_BIT(ds_mask, ds_index) == 0) {
            continue;
        }
        chip_info->csds_id = (u8)ds_index;

        hisds_config_polarity(hdev, chip_info);
        /* set tos threshold parameters */
        /* Set Lms Los configure */
        h60_write_reg_field(hdev, H60_LOS_LMS_THRESHf, DS_REG_OFFSET(chip_info), 0x8);
        /* Set CDR Los configure 1000 */
        h60_write_reg_field(hdev, H60_CDR_LI_INTEGRATOR_OUTf, DS_REG_OFFSET(chip_info), 1000);
        /* Set RND Los configure 200 */
        h60_write_reg_field(hdev, H60_RND_MAX_THRESHf, DSAPI_REG_OFFSET(chip_info), 200);

        /* user configure base on rx insertion loss */
        hisds_config_rx_insertion_loss(hdev, chip_info);

        h60_write_reg_field(hdev, H60_DS_INCREASE_CDR_LOCK_APIf, DSAPI_REG_OFFSET(chip_info), 0x0);
        h60_write_reg_field(hdev, H60_DSP_ADC_UPGRADE_TH_APIf, DSAPI_REG_OFFSET(chip_info), 0x3);
        h60_write_reg_field(hdev, H60_DSP_FFE_UPGRADE_TH_APIf, DSAPI_REG_OFFSET(chip_info), 0x3);
        h60_write_reg_field(hdev, H60_DSP_PD_UPGRADE_TH_APIf, DSAPI_REG_OFFSET(chip_info), 0x3);

        hisds_get_data_rate_h60(hdev, chip_info, SERDES_RX, &rx_date_rate);
        hisds_adapt_bypass_cfg(hdev, chip_info, rx_date_rate);
        /* CONT_ABGCBYPASS_API and CONT_CDRPREBYPASS_API need be ajusted basing on insertion loss */
        h60_read_reg_field(hdev, H60_DS_RX_INSERTION_LOSS_APIf, DSAPI_REG_OFFSET(chip_info), &rx_insertion_loss);
        /* insertion loss < 10 speed 0~10313khz or insertion loss < 5 speed 0~15000khz */
        insertion_loss_ajust = (rx_insertion_loss <= 10 && rx_date_rate <= 10313) ||
            (rx_insertion_loss <= 5 && rx_date_rate <= 15000);
        if (insertion_loss_ajust) {
            h60_write_reg_field(hdev, H60_CONT_ABGCBYPASS_APIf, DSAPI_REG_OFFSET(chip_info), 0x1);
            h60_write_reg_field(hdev, H60_CONT_CDRPREBYPASS_APIf, DSAPI_REG_OFFSET(chip_info), 0x1);
        }
        /* Clear online adaptation bypass */
        h60_write_reg_field(hdev, H60_CONT_CTLEOFSTBYPASS_APIf, DSAPI_REG_OFFSET(chip_info), 0x0);
        h60_write_reg_field(hdev, H60_CONT_BIASRCVITRIM_APIf, DSAPI_REG_OFFSET(chip_info), 0x0);
        h60_write_reg_field(hdev, H60_CONT_CTLECMTRKBYPASS_APIf, DSAPI_REG_OFFSET(chip_info), 0x1);
        h60_write_reg_field(hdev, H60_CONT_LINKRECOVERYBYPASS_APIf, DSAPI_REG_OFFSET(chip_info), 0x0);
        h60_write_reg_field(hdev, H60_CONT_MLBYPASS_APIf, DSAPI_REG_OFFSET(chip_info), 0x1);
        /* API for V101 only (V100 not used) */
        h60_write_reg_field(hdev, H60_CONT_NINETAPCDRBYPASS_APIf, DSAPI_REG_OFFSET(chip_info), 0x1);
        /* Set DS online adaptation API */
        h60_write_reg_field(hdev, H60_MIN_PRE2POST_APIf, DSAPI_REG_OFFSET(chip_info), 0x3);
        h60_write_reg_field(hdev, H60_DS_CALIB_EN_APIf, DSAPI_REG_OFFSET(chip_info), 0x0);
        h60_write_reg_field(hdev, H60_CONT_ADAPT_EN_APIf, DSAPI_REG_OFFSET(chip_info), 0x0);
        if (adapt_type == SERDES_ADAPT_STEP_INIT) {
            h60_write_reg_field(hdev, H60_INIT_ADAPT_EN_APIf, DSAPI_REG_OFFSET(chip_info), 0x1);
        }
        h60_write_reg_field(hdev, H60_CONT_ADAPT_EN_APIf, DSAPI_REG_OFFSET(chip_info), 0x1);
    }

    return SERDES_OK;
}

/* status_mask : bit x set 1 means lane x adapt has done, 0 for not done */
STATIC u32 h60_adapt_done_check(struct hclge_dev *hdev, CHIP_INFO_S *chip_info, u32 ds_mask,
    SERDES_ADAPT_STEP_E step, u32 *status_mask)
{
    struct pci_dev *pdev = hdev->pdev;
    u8 ds_num = chip_info->ds_num;
    u32 ds_index;
    /*
     * adapt_completed:
     * 00: Process not performed
     * 01: Process completed successfully
     * 11: Error in processing
     * 10: start to process
     */
    u32 adapt_completed = 0;
    u32 ds_idle = 0;

    if(status_mask == NULL) {
        dev_err(&pdev->dev, "h60_adapt_done_check failed! status_mask is NULL!\n");
        return SERDES_NULL;
    }

    *status_mask = 0;
    for (ds_index = 0; ds_index < ds_num; ds_index++) {
        if (GET_BIT(ds_mask, ds_index) == 0) {
            continue;
        }
        chip_info->csds_id = (u8)ds_index;
        switch (step) {
            case SERDES_ADAPT_STEP_INIT:
                h60_read_reg_field(hdev, H60_INIT_ADAPT_STATUS_APIf, DSAPI_REG_OFFSET(chip_info), &adapt_completed);
                h60_read_reg_field(hdev, H60_DS_STATE_APIf, DSAPI_REG_OFFSET(chip_info), &ds_idle);
                adapt_completed = ((adapt_completed == 1) && (ds_idle == 0xf)); /* 1: Process completed successfully */
                break;
            case SERDES_ADAPT_FAST_SEARCH:
                h60_read_reg_field(hdev, H60_INIT_ADAPT_STATUS_APIf, DSAPI_REG_OFFSET(chip_info), &adapt_completed);
                break;
            case SERDES_ADAPT_STEP_CONT:
                h60_read_reg_field(hdev, H60_CONT_ADAPT_STATUS_APIf, DSAPI_REG_OFFSET(chip_info), &adapt_completed);
                break;
            case SERDES_ADAPT_FAST_REFINE:
                h60_read_reg_field(hdev, H60_LT_TX_PROPOSAL_IND_APIf, DSAPI_REG_OFFSET(chip_info), &adapt_completed);
                break;
            default:
                return SERDES_ERROR;
        }
        if (adapt_completed == 1) { /* 1: Process completed successfully */
            SET_BIT(*status_mask, ds_index);
        }
    }
    return 0;
}

STATIC void hclge_pam4_bitorder_config(struct hclge_dev *hdev, CHIP_INFO_S *chip_info)
{
    u8 ds_num = chip_info->ds_num;
    u8 ds_mask = chip_info->ds_mask;
    u32 pam4_en = 0; /* 1-pam4 0-NRZ */
    u8 ds_index;

    for (ds_index = 0; ds_index < ds_num; ds_index++) {
        if (GET_BIT(ds_mask, ds_index) == 0) {
            continue;
        }
        chip_info->csds_id = ds_index;
        h60_read_reg_field(hdev, H60_TX_PAM4_MODEf, DS_REG_OFFSET(chip_info), &pam4_en);
        if (pam4_en == H60_PAM4_MODE_EN) {
            h60_write_reg_field(hdev, H60_TX_GRAY_BYPASSf, DS_REG_OFFSET(chip_info), 0);
            h60_write_reg_field(hdev, H60_TXBITORDERf, DS_REG_OFFSET(chip_info), 1);
        }

        h60_read_reg_field(hdev, H60_MODCONFIGf, DS_REG_OFFSET(chip_info), &pam4_en);
        if (pam4_en == H60_PAM4_MODE_EN) {
            h60_write_reg_field(hdev, H60_RXBITORDERf, DS_REG_OFFSET(chip_info), 1);
            h60_write_reg_field(hdev, H60_RXGRAYDECf, DS_REG_OFFSET(chip_info), 1);
            h60_write_reg_field(hdev, H60_RX_PRBS_GRAY_BYPASSf, DS_REG_OFFSET(chip_info), 0);
            h60_write_reg_field(hdev, H60_RX_PRBS_MOD4_BYPASSf, DS_REG_OFFSET(chip_info), 1);
        }
    }
}

STATIC u32 h60_adapt_normal(struct hclge_dev *hdev, CHIP_INFO_S *chip_info)
{
    struct pci_dev *pdev = hdev->pdev;
    u32 check_count = 0;
    u32 status = 0;
    u32 adapt_done;
    u32 ret;

    hclge_pam4_bitorder_config(hdev, chip_info);

    ret = adapt_process_h60(hdev, chip_info, SERDES_ADAPT_STEP_INIT);
    if (ret) {
        dev_err(&pdev->dev, "adapt_process_h60 failed! ret is [%d]\n", ret);
        return ret;
    }

    do {
        usleep_range(1000, 1000);
        check_count++;
        ret = h60_adapt_done_check(hdev, chip_info, H60_ADAPT_INIT_MASK, SERDES_ADAPT_STEP_CONT, &status);
        adapt_done = (status == H60_ADAPT_INIT_MASK);
    } while ((!adapt_done) && (check_count < H60_ADAPT_TIMEOUT)); /* wait 5000ms */

    /* timeout */
    if (check_count >= H60_ADAPT_TIMEOUT) {
        if (status != 0) {
            dev_err(&pdev->dev, "serdes adapt timeout, adapt status:0x%x\n", status);
        }
        return SERDES_TIMEOUT;
    }
    dev_info(&pdev->dev, "h60_adapt_normal success! macro_id is 0x%x\n", chip_info->macro_id);

    return ret;
}

STATIC u32 h60_check_lms_cdr_los(struct hclge_dev *hdev, CHIP_INFO_S *chip_info,
    u32 *status_mask, SERDES_LOS_E los_type)
{
    struct pci_dev *pdev = hdev->pdev;
    u32 ds_mask = chip_info->ds_mask;
    u32 h60_los_status = 0;
    u32 ds_index;

    if(status_mask == NULL) {
        dev_err(&pdev->dev, "h60_check_lms_cdr_los failed! status_mask is NULL\n");
        return SERDES_NULL;
    }

    *status_mask = 0;
    for (ds_index = 0; ds_index < chip_info->ds_num; ds_index++) {
        if (GET_BIT(ds_mask, ds_index) == 0) {
            continue;
        }

        chip_info->csds_id = (u8)ds_index;
        if (los_type == LMS_LOS) {
            h60_read_reg_field(hdev, H60_LMS_LOS_DET_STATUS_APIf, DSAPI_REG_OFFSET(chip_info),
                &h60_los_status);
        }

        if (los_type == CDR_LOS) {
            h60_read_reg_field(hdev, H60_CDR_LOS_DET_STATUS_APIf, DSAPI_REG_OFFSET(chip_info),
                &h60_los_status);
        }

        if (h60_los_status == 1) {
            SET_BIT(*status_mask, ds_index);
        }
    }
    return 0;
}

int hclge_do_h60_adapt(struct hclge_dev *hdev)
{
    struct pci_dev *pdev = hdev->pdev;
    CHIP_INFO_S chip_info = {0};
    u32 ret = 0;

    if (macro0_in_use(hdev)) {
        hclge_init_chip_info(hdev, &chip_info, H60_MACRO_OFFSET_0);
        ret |= h60_adapt_normal(hdev, &chip_info);
    }

    if (macro1_in_use(hdev)) {
        hclge_init_chip_info(hdev, &chip_info, H60_MACRO_OFFSET_1);
        ret |= h60_adapt_normal(hdev, &chip_info);
    }

    return ret;
}

int hclge_h60_check_los(struct hclge_dev *hdev, u32 *los_status, SERDES_LOS_E los_type)
{
    struct pci_dev *pdev = hdev->pdev;
    CHIP_INFO_S chip_info = {0};
    u32 status_m0 = 0;
    u32 status_m1 = 0;
    int ret = 0;

    if (los_status == NULL) {
        dev_err(&pdev->dev, "los_status is NULL\n");
        return -EINVAL;
    }

    if (macro0_in_use(hdev)) {
        hclge_init_chip_info(hdev, &chip_info, H60_MACRO_OFFSET_0);
        ret |= h60_check_lms_cdr_los(hdev, &chip_info, &status_m0, los_type);
    }

    if (macro1_in_use(hdev)) {
        hclge_init_chip_info(hdev, &chip_info, H60_MACRO_OFFSET_1);
        ret |= h60_check_lms_cdr_los(hdev, &chip_info, &status_m1, los_type);
    }

    if (ret) {
        dev_err(&pdev->dev, "hclge_h60_check_los failed! ret is 0x%x\n", ret);
        return ret;
    }

    *los_status = status_m0 | (status_m1 << H60_DEFAULT_SERDES_LANE_NUM);

    return ret;
}
