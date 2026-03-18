/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 * Create: 2023-6-5
 */


#include "hclge_serdes_reg_macro.h"
#include "hclge_serdes_common.h"
#include "hclge_serdes_adapt.h"
#include "hclge_serdes_loopback.h"
#include "hns3_enet.h"

STATIC void hisds_set_loopback_disable_h60(struct hclge_dev *hdev, CHIP_INFO_S *chip_info)
{
    /* Select TX data from RX */
    h60_write_reg_field(hdev, H60_TXRXPARLPBKENf, DS_REG_OFFSET(chip_info), SERDES_DISABLE);
}

STATIC u32 hisds_set_lp_proc_par_tx2rx_h60(struct hclge_dev *hdev, const CHIP_INFO_S *chip_info)
{
    return h60_init_reg_write_ensure(hdev, chip_info, H60_TXRXPARLPBKENf, DS_REG_OFFSET(chip_info),
        SERDES_ENABLE);
}

STATIC u32 hisds_set_loopback_process_h60(struct hclge_dev *hdev, CHIP_INFO_S *chip_info,
    SERDES_LOOPBACK_TYPE_E loopback_type)
{
    struct pci_dev *pdev = hdev->pdev;
    u32 ret = 0;

    /* set loopback */
    switch (loopback_type) {
        case SERDES_LOOPBACK_ALL_DISABLE:
            hisds_set_loopback_disable_h60(hdev, chip_info);
            break;

        case SERDES_LOOPBACK_PARALLEL_TX_TO_RX:
            /* Select TX data to RX */
            ret = hisds_set_lp_proc_par_tx2rx_h60(hdev, chip_info);
            break;

        default:
            dev_err(&pdev->dev, "loop type %u unsupport\n", loopback_type);
            ret = SERDES_NOT_SUPPORT;
            break;
    }

    return ret;
}

STATIC u32 hisds_ds_freeze_adaptation(struct hclge_dev *hdev, CHIP_INFO_S *chip_info)
{
    struct pci_dev *pdev = hdev->pdev;
    u32 ret = SERDES_OK;
    u32 ds_state = 0;
    u32 reg_val = 0;

    // clear init adapt and online adapt status
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

    // set ds state to idle
    ret = h60_wait_reg_value(hdev, chip_info, H60_DS_STATE_APIf, DSAPI_REG_OFFSET(chip_info), 0xF, 1000, 1000);

    return ret;
}

STATIC void hisds_ds_unfreeze_adaptation(struct hclge_dev *hdev, CHIP_INFO_S *chip_info)
{
    u32 reg_val = 0;

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

STATIC void hisds_get_loopback_h60(struct hclge_dev *hdev, CHIP_INFO_S *chip_info,
    SERDES_LOOPBACK_TYPE_E *loopback_type)
{
    u32 tx_lpbk_selp = 0;
    u32 tx_lpbk_selm = 0;
    u32 tx_data_sel = 0;
    u32 tx2rx_par = 0;

    /* read loop reg */
    h60_read_reg_field(hdev, H60_TXRXPARLPBKENf, DS_REG_OFFSET(chip_info), &tx2rx_par);
    h60_read_reg_field(hdev, H60_TXDATASELf, DS_REG_OFFSET(chip_info), &tx_data_sel);
    h60_read_reg_field(hdev, H60_TX_LPBK_SELPf, DS_REG_OFFSET(chip_info), &tx_lpbk_selp);
    h60_read_reg_field(hdev, H60_TX_LPBK_SELMf, DS_REG_OFFSET(chip_info), &tx_lpbk_selm);

    if (tx2rx_par) {
        *loopback_type = SERDES_LOOPBACK_PARALLEL_TX_TO_RX;
    } else if (tx_lpbk_selp || tx_lpbk_selm) {
        *loopback_type = SERDES_LOOPBACK_SERIAL_PREDRV_TX_TO_RX;
    } else if (tx_data_sel == 1) {
        *loopback_type = SERDES_LOOPBACK_PARALLEL_RX_TO_TX;
    } else {
        *loopback_type = SERDES_LOOPBACK_ALL_DISABLE;
    }
}

STATIC u32 hisds_set_loopback_h60(struct hclge_dev *hdev, CHIP_INFO_S *chip_info,
    SERDES_LOOPBACK_TYPE_E loopback_type)
{
    SERDES_LOOPBACK_TYPE_E loopback_type_now = SERDES_LOOPBACK_END;
    struct pci_dev *pdev = hdev->pdev;
    u32 ret = 0;

    hisds_get_loopback_h60(hdev, chip_info, &loopback_type_now);
    if (loopback_type_now == loopback_type) {
        return SERDES_OK;
    }

    /* stop adaptation before set loopback */
    if (loopback_type != SERDES_LOOPBACK_ALL_DISABLE) {
        ret = hisds_ds_freeze_adaptation(hdev, chip_info);
        if (ret) {
            dev_err(&pdev->dev, "ds freeze adaptation failed, ret:0x%x\n", ret);
            return ret;
        }
        h60_write_reg_field(hdev, H60_LOS_LMS_THRESHf, DS_REG_OFFSET(chip_info), 0x0);
    }

    if ((loopback_type_now != SERDES_LOOPBACK_ALL_DISABLE) && (loopback_type != SERDES_LOOPBACK_ALL_DISABLE)) {
        hisds_set_loopback_disable_h60(hdev, chip_info);
    }

    ret = hisds_set_loopback_process_h60(hdev, chip_info, loopback_type);
    if (ret) {
        dev_err(&pdev->dev, "set loopback process failed, ret:0x%x\n", ret);
        return ret;
    }

    if (loopback_type == SERDES_LOOPBACK_ALL_DISABLE) {
        h60_write_reg_field(hdev, H60_LOS_LMS_THRESHf, DS_REG_OFFSET(chip_info), 0x8); /* 8 is default value*/
        /* restart adaptation after set loopback */
        hisds_ds_unfreeze_adaptation(hdev, chip_info);
    }

    return SERDES_OK;
}

int network_custom_set_loopback(struct hnae3_handle *handle, bool en, u32 loop_mode)
{
    SERDES_LOOPBACK_TYPE_E loopback_type = SERDES_LOOPBACK_END;
    struct hnae3_ae_dev *ae_dev = hns3_get_ae_dev(handle);
    struct hclge_dev *hdev = ae_dev->priv;
    struct pci_dev *pdev = hdev->pdev;
    CHIP_INFO_S chip_info = { 0 };
    u32 ds_index;
    u32 ret = 0;
    u8 ds_num;

    if (macro0_in_use(hdev)) {
        hclge_init_chip_info(hdev, &chip_info, H60_MACRO_OFFSET_0);
    } else if (macro1_in_use(hdev)) {
        hclge_init_chip_info(hdev, &chip_info, H60_MACRO_OFFSET_1);
    } else {
        dev_err(&pdev->dev, "set serdes loop macro failed\n");
        return SERDES_ERROR_PARA;
    }

    if (loop_mode == HNAE3_LOOP_PARALLEL_SERDES) {
        loopback_type = SERDES_LOOPBACK_PARALLEL_TX_TO_RX;
    } else {
        dev_err(&pdev->dev, "set serdes loop type unspport, loop_mode:0x%x\n", loop_mode);
        return SERDES_NOT_SUPPORT;
    }

    if (!en) {
        loopback_type = SERDES_LOOPBACK_ALL_DISABLE;
    }

    ds_num = chip_info.ds_num;
    for (ds_index = 0; ds_index < ds_num; ds_index++) {
        if (GET_BIT(chip_info.ds_mask, ds_index) == 0) {
            continue;
        }

        chip_info.csds_id = ds_index;
        ret = hisds_set_loopback_h60(hdev, &chip_info, loopback_type);
        if (ret) {
            dev_err(&pdev->dev, "set serdes loopback failed, ret:0x%x\n", ret);
            return SERDES_ERROR;
        }
    }

    return ret;
}