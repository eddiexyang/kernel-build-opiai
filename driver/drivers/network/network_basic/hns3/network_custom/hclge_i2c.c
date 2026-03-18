/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
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
* Description:
* Author: huawei
* Create: 2019-10-15
*/

#include <linux/kernel.h>
#include "network_custom.h"
#include "hclge_cmd.h"
#include "hclge_main.h"
#include "hclge_i2c.h"

extern int memcpy_s(void *dest, size_t destMax, const void *src, size_t count);

STATIC int hclge_read_xsfp_info(u32 devid, u32 port_id, u32 opcode, u8 *info_buff, u32 info_len)
{
    struct hnae3_ae_dev *ae_dev = NULL;
    struct hclge_dev *hdev = NULL;
    struct pci_dev *pdev = NULL;
    struct hclge_desc desc;
    int ret;

    pdev = hclge_get_pci_dev(devid);
    if (pdev == NULL || info_buff == NULL) {
        pr_err("hclge: hclge gets pci dev failed or msg_val is %pK\n", info_buff);
        return -ENOMEM;
    }

    ae_dev = pci_get_drvdata(pdev);
    if (ae_dev == NULL) {
        pr_err("hclge: %s: pci gets drv data failed\n", __func__);
        return -ENOMEM;
    }

    hdev = ae_dev->priv;
    if (hdev == NULL) {
        pr_err("hclge: %s: i2c read:ae_dev gets dpriv failed\n", __func__);
        return -ENOMEM;
    }

    if (test_bit(HCLGE_STATE_CMD_DISABLE, &hdev->state)) {
        return 0;
    }

    hclge_cmd_setup_basic_desc(&desc, HCLGE_OPC_GET_XSFP_BASE_INFO, true);
    hnae3_set_field(desc.data[0], HCLGE_XSFP_OP_INDEX_M, HCLGE_XSFP_OP_INDEX_S, opcode);
#ifndef DEFINE_HNS_LLT
    ret = hclge_cmd_send(&hdev->hw, &desc, 1);
    if (ret) {
        dev_err(&hdev->pdev->dev, "[hclge_i2c_read] send i2c cmd fail, ret %d.\n", ret);
        return ret;
    }
#endif
    ret = memcpy_s(info_buff, info_len, desc.data, info_len);
    if (ret) {
        dev_err(&hdev->pdev->dev, "%s: copy i2c cmd data fail, ret %d.\n", __func__, ret);
        return ret;
    }

    return 0;
}

STATIC int hclge_read_xsfp_additional_info(u32 devid, u32 port_id, u32 opcode, u8 *info_buff, u32 info_len)
{
    struct hnae3_ae_dev *ae_dev = NULL;
    struct hclge_dev *hdev = NULL;
    struct pci_dev *pdev = NULL;
    struct hclge_desc desc;
    int ret;

    pdev = hclge_get_pci_dev(devid);
    if (pdev == NULL || info_buff == NULL) {
        pr_err("hclge: hclge gets pci dev failed or info_buff is %pK\n", info_buff);
        return -ENODEV;
    }

    ae_dev = pci_get_drvdata(pdev);
    if (ae_dev == NULL) {
        pr_err("hclge: %s: pci gets drv data failed\n", __func__);
        return -ENODEV;
    }

    hdev = ae_dev->priv;
    if (hdev == NULL) {
        pr_err("hclge: %s: i2c read:ae_dev gets hdev failed\n", __func__);
        return -ENODEV;
    }

    if (test_bit(HCLGE_STATE_CMD_DISABLE, &hdev->state)) {
        return 0;
    }

    hclge_cmd_setup_basic_desc(&desc, HCLGE_OPC_GET_XSFP_ADDITIONAL_INFO, true);
    hnae3_set_field(desc.data[0], HCLGE_XSFP_OP_INDEX_M, HCLGE_XSFP_OP_INDEX_S, opcode);

    ret = hclge_cmd_send(&hdev->hw, &desc, 1);
    if (ret == -EOPNOTSUPP) {
        dev_warn(&hdev->pdev->dev, "[hclge_i2c_read] xsfp additional info not support.\n");
        return 0;
    }else if (ret) {
        dev_err(&hdev->pdev->dev, "[hclge_i2c_read] send i2c cmd fail, ret %d.\n", ret);
        return ret;
    }

    ret = memcpy_s(info_buff, info_len, desc.data, info_len);
    if (ret) {
        dev_err(&hdev->pdev->dev, "%s: copy i2c cmd data fail, ret %d.\n", __func__, ret);
        return ret;
    }

    return 0;
}

int hns_xsfp_get_temperature(u32 dev_id, u32 port_id, int *i2cdev_temp)
{
    unsigned int temperature;
    int ret;

    if (dev_id > MAX_DEV_ID || i2cdev_temp == NULL || port_id > MAX_PORT_ID) {
        pr_err("hclge: [xsfp_get_temperature] param is invalid, dev_id %u, port_id %u i2cdev_temp %pK\n", dev_id,
            port_id, i2cdev_temp);
        return -EINVAL;
    }

    ret = hclge_read_xsfp_info(dev_id, port_id, HCLGE_OPC_GET_XSFP_TEMPERATURE, (u8 *)&temperature, sizeof(int));
    if (ret) {
        pr_err("hclge: hns_xsfp_get_temperature err! ret %d\n", ret);
        return ret;
    }

    if ((temperature & 0xFFFF) == 0 || (temperature & 0xFFFF) == HCLGE_TEMP_INVALID) {
        *i2cdev_temp = HCLGE_TEMP_INVALID;
    } else if ((temperature & 0xFFFF) == HCLGE_TEMP_FAULT) {
        *i2cdev_temp = HCLGE_TEMP_FAULT;
    } else {
        *i2cdev_temp = (temperature & 0xFFFF) / TEMP_SCAL_UINT;
    }

    return 0;
}
EXPORT_SYMBOL(hns_xsfp_get_temperature);

int hns_xsfp_get_high_power_en(u32 dev_id, u32 port_id, unsigned int *enable)
{
    int ret;

    if (dev_id > MAX_DEV_ID || enable == NULL || port_id > MAX_PORT_ID) {
        pr_err("hclge: [xsfp_get_high_power_en] param is invalid, dev_id %u, port_id %u enable %pK\n", dev_id,
            port_id, enable);
        return -EINVAL;
    }

    ret = hclge_read_xsfp_info(dev_id, port_id, HCLGE_OPC_GET_XSFP_HIGH_POWER_ENABLE, (u8 *)enable, sizeof(int));
    if (ret) {
        pr_err("hclge: hns_xsfp_get_high_power_en err! ret %d\n", ret);
        return ret;
    }

    return 0;
}
EXPORT_SYMBOL(hns_xsfp_get_high_power_en);

int hns_xsfp_get_optical_identifier(u32 dev_id, u32 port_id, int *identifier)
{
    int ret;

    if (dev_id > MAX_DEV_ID || identifier == NULL || port_id > MAX_PORT_ID) {
        pr_err("hclge: [xsfp_get_optical_identifier] param invalid, dev_id %u, port_id %u identifier %pK\n", dev_id,
            port_id, identifier);
        return -EINVAL;
    }

    ret = hclge_read_xsfp_info(dev_id, port_id, HCLGE_OPC_GET_XSFP_IDENTIFIER, (u8 *)identifier, sizeof(int));
    if (ret) {
        pr_err("hclge: hns_xsfp_get_optical_identifier err! ret %d\n", ret);
        return ret;
    }

    return 0;
}
EXPORT_SYMBOL(hns_xsfp_get_optical_identifier);

int hclge_retimer_get_fw_version(u32 dev_id, u32 port_id, char *fw_version)
{
    return 0;
}
EXPORT_SYMBOL(hclge_retimer_get_fw_version);

int hns_xsfp_get_vendor_name(u32 dev_id, u32 port_id, u8 *vendor_name, u32 name_len)
{
    int ret;

    if (dev_id > MAX_DEV_ID || vendor_name == NULL || port_id > MAX_PORT_ID || name_len != HCLGE_VENDOR_NAME_LEN) {
        pr_err("hclge: [hns_xsfp_get_vendor_name] param invalid, devid %u, port_id %u, vendor_name %pK, len %u\n",
            dev_id, port_id, vendor_name, name_len);
        return -EINVAL;
    }

    ret = hclge_read_xsfp_info(dev_id, port_id, HCLGE_OPC_GET_XSFP_VEN_NAME, vendor_name, name_len);
    if (ret) {
        pr_err("hclge: hns_xsfp_get_vendor_name err! ret %d\n", ret);
        return ret;
    }

    return 0;
}
EXPORT_SYMBOL(hns_xsfp_get_vendor_name);

int hns_xsfp_get_vendor_pn(u32 dev_id, u32 port_id, u8 *vendor_pn, u32 pn_len)
{
    int ret;

    if (dev_id > MAX_DEV_ID || vendor_pn == NULL || port_id > MAX_PORT_ID || pn_len != HCLGE_VENDOR_PN_LEN) {
        pr_err("hclge: [hns_xsfp_get_vendor_pn] param invalid, devid %u, port_id %u, vendor_pn %pK, len %u\n",
            dev_id, port_id, vendor_pn, pn_len);
        return -EINVAL;
    }

    ret = hclge_read_xsfp_info(dev_id, port_id, HCLGE_OPC_GET_XSFP_VEN_PN, vendor_pn, pn_len);
    if (ret) {
        pr_err("hclge: hns_xsfp_get_vendor_pn err! ret %d\n", ret);
        return ret;
    }

    return 0;
}
EXPORT_SYMBOL(hns_xsfp_get_vendor_pn);

int hns_xsfp_get_vendor_oui(u32 dev_id, u32 port_id, u8 *vendor_oui, u32 oui_len)
{
    int ret;

    if (dev_id > MAX_DEV_ID || vendor_oui == NULL || port_id > MAX_PORT_ID || oui_len != HCLGE_VENDOR_OUI_LEN) {
        pr_err("hclge: [hns_xsfp_get_vendor_oui] param invalid, devid %u, port_id %u, vendor_oui %pK, len %u\n",
            dev_id, port_id, vendor_oui, oui_len);
        return -EINVAL;
    }

    ret = hclge_read_xsfp_info(dev_id, port_id, HCLGE_OPC_GET_XSFP_VEN_OUI, vendor_oui, oui_len);
    if (ret) {
        pr_err("hclge: hns_xsfp_get_vendor_oui err! ret %d\n", ret);
        return ret;
    }

    return 0;
}
EXPORT_SYMBOL(hns_xsfp_get_vendor_oui);

int hns_xsfp_get_vendor_sn(u32 dev_id, u32 port_id, u8 *vendor_sn, u32 sn_len)
{
    int ret;

    if (dev_id > MAX_DEV_ID || vendor_sn == NULL || port_id > MAX_PORT_ID || sn_len != HCLGE_VENDOR_SN_LEN) {
        pr_err("hclge: [hns_xsfp_get_vendor_sn] param invalid, devid %u, port_id %u, vendor_sn %pK, len %u\n",
            dev_id, port_id, vendor_sn, sn_len);
        return -EINVAL;
    }

    ret = hclge_read_xsfp_info(dev_id, port_id, HCLGE_OPC_GET_XSFP_VEN_SN, vendor_sn, sn_len);
    if (ret) {
        pr_err("hclge: hns_xsfp_get_vendor_sn err! ret %d\n", ret);
        return ret;
    }

    return 0;
}
EXPORT_SYMBOL(hns_xsfp_get_vendor_sn);

int hns_xsfp_get_wavelength(u32 dev_id, u32 port_id, u8 *wavelength, u32 wavelength_len)
{
    int ret;

    if (dev_id > MAX_DEV_ID || wavelength == NULL || port_id > MAX_PORT_ID || wavelength_len != HCLGE_WAVE_LEN) {
        pr_err("hclge: [hns_xsfp_get_wavelength] param invalid, devid %u, port_id %u, wavelength %pK, len %u\n",
            dev_id, port_id, wavelength, wavelength_len);
        return -EINVAL;
    }

    ret = hclge_read_xsfp_info(dev_id, port_id, HCLGE_OPC_GET_XSFP_WAVELENGTH, wavelength, wavelength_len);
    if (ret) {
        pr_err("hclge: hns_xsfp_get_wavelength err! ret %d\n", ret);
        return ret;
    }

    return 0;
}
EXPORT_SYMBOL(hns_xsfp_get_wavelength);

int hns_xsfp_get_date_code(u32 dev_id, u32 port_id, u8 *date_code, u32 date_code_len)
{
    int ret;

    if (dev_id > MAX_DEV_ID || date_code == NULL || port_id > MAX_PORT_ID || date_code_len != HCLGE_DATA_CODE_LEN) {
        pr_err("hclge: [hns_xsfp_get_date_code] param invalid, devid %u, port_id %u, date_code %pK, len %u\n",
            dev_id, port_id, date_code, date_code_len);
        return -EINVAL;
    }

    ret = hclge_read_xsfp_info(dev_id, port_id, HCLGE_OPC_GET_XSFP_DATE_CODE, date_code, date_code_len);
    if (ret) {
        pr_err("hclge: hns_xsfp_get_date_code err! ret %d\n", ret);
        return ret;
    }

    return 0;
}
EXPORT_SYMBOL(hns_xsfp_get_date_code);

int hns_xsfp_get_additional_info(u32 dev_id, u32 port_id, struct xsfp_additional_cmd *additional_cmd)
{
    int ret;
    int op_index = 0;
    int index_len;

    if (dev_id > MAX_DEV_ID || additional_cmd == NULL || port_id > MAX_PORT_ID) {
        pr_err("hclge: [hns_xsfp_get_additional_info] param is invalid, dev_id %u, port_id %u additional_cmd %pK\n",
            dev_id, port_id, additional_cmd);
        return -EINVAL;
    }

    struct additional_cmd_list cmd_list[] = {
        {HCLGE_OPC_GET_XSFP_VOLTAGE, (char *)&additional_cmd->xsfp_voltage, sizeof(int)},
        {HCLGE_OPC_GET_XSFP_TX_POWER, additional_cmd->xsfp_tx_power, HCLGE_OPTICAL_POWER_LEN},
        {HCLGE_OPC_GET_XSFP_RX_POWER, additional_cmd->xsfp_rx_power, HCLGE_OPTICAL_POWER_LEN},
        {HCLGE_OPC_GET_XSFP_VCC_HIGH_THRESHOLD, (char *)&additional_cmd->xsfp_vcc_high_threshold, sizeof(int)},
        {HCLGE_OPC_GET_XSFP_VCC_LOW_THRESHOLD, (char *)&additional_cmd->xsfp_vcc_low_threshold, sizeof(int)},
        {HCLGE_OPC_GET_XSFP_VCC_HIGH_ALARM_THRESHOLD, (char *)&additional_cmd->xsfp_vcc_high_alarm_threshold, sizeof(int)},
        {HCLGE_OPC_GET_XSFP_VCC_LOW_ALARM_THRESHOLD, (char *)&additional_cmd->xsfp_vcc_low_alarm_threshold, sizeof(int)},
        {HCLGE_OPC_GET_XSFP_TEMP_HIGH_THRESHOLD, (char *)&additional_cmd->xsfp_temp_high_threshold, sizeof(int)},
        {HCLGE_OPC_GET_XSFP_TEMP_LOW_THRESHOLD, (char *)&additional_cmd->xsfp_temp_low_threshold, sizeof(int)},
        {HCLGE_OPC_GET_XSFP_TEMP_HIGH_ALARM_THRESHOLD, (char *)&additional_cmd->xsfp_temp_high_alarm_threshold, sizeof(int)},
        {HCLGE_OPC_GET_XSFP_TEMP_LOW_ALARM_THRESHOLD, (char *)&additional_cmd->xsfp_temp_low_alarm_threshold, sizeof(int)},
        {HCLGE_OPC_GET_XSFP_TX_POWER_HIGH_THRESHOLD,
            (char *)&additional_cmd->xsfp_tx_power_high_threshold, sizeof(int)},
        {HCLGE_OPC_GET_XSFP_TX_POWER_LOW_THRESHOLD, (char *)&additional_cmd->xsfp_tx_power_low_threshold, sizeof(int)},
        {HCLGE_OPC_GET_XSFP_TX_POWER_HIGH_ALARM_THRESHOLD,
            (char *)&additional_cmd->xsfp_tx_power_high_alarm_threshold, sizeof(int)},
        {HCLGE_OPC_GET_XSFP_TX_POWER_LOW_ALARM_THRESHOLD, (char *)&additional_cmd->xsfp_tx_power_low_alarm_threshold, sizeof(int)},
        {HCLGE_OPC_GET_XSFP_RX_POWER_HIGH_THRESHOLD,
            (char *)&additional_cmd->xsfp_rx_power_high_threshold, sizeof(int)},
        {HCLGE_OPC_GET_XSFP_RX_POWER_LOW_THRESHOLD, (char *)&additional_cmd->xsfp_rx_power_low_threshold, sizeof(int)},
        {HCLGE_OPC_GET_XSFP_RX_POWER_HIGH_ALARM_THRESHOLD,
            (char *)&additional_cmd->xsfp_rx_power_high_alarm_threshold, sizeof(int)},
        {HCLGE_OPC_GET_XSFP_RX_POWER_LOW_ALARM_THRESHOLD, (char *)&additional_cmd->xsfp_rx_power_low_alarm_threshold, sizeof(int)},
        {HCLGE_OPC_GET_XSFP_TX_BIAS_HIGH_THRESHOLD,
            (char *)&additional_cmd->xsfp_tx_bias_high_threshold, sizeof(int)},
        {HCLGE_OPC_GET_XSFP_TX_BIAS_LOW_THRESHOLD, (char *)&additional_cmd->xsfp_tx_bias_low_threshold, sizeof(int)},
        {HCLGE_OPC_GET_XSFP_TX_BIAS_HIGH_ALARM_THRESHOLD,
            (char *)&additional_cmd->xsfp_tx_bias_high_alarm_threshold, sizeof(int)},
        {HCLGE_OPC_GET_XSFP_TX_BIAS_LOW_ALARM_THRESHOLD, (char *)&additional_cmd->xsfp_tx_bias_low_alarm_threshold, sizeof(int)},
        {HCLGE_OPC_GET_XSFP_TX_BIAS, additional_cmd->xsfp_tx_bias, HCLGE_OPTICAL_POWER_LEN},
        {HCLGE_OPC_GET_XSFP_TX_LOS, (char *)&additional_cmd->xsfp_tx_los, sizeof(int)},
        {HCLGE_OPC_GET_XSFP_RX_LOS, (char *)&additional_cmd->xsfp_rx_los, sizeof(int)},
        {HCLGE_OPC_GET_XSFP_MEDIA_TYPE, additional_cmd->xsfp_moudle_media, HCLGE_OPTICAL_MODULE_MEDIA_LEN},
    };

    index_len = sizeof(cmd_list) / sizeof(cmd_list[0]);
    for (op_index = 0; op_index < index_len; op_index++) {
        ret = hclge_read_xsfp_additional_info(dev_id, port_id, cmd_list[op_index].cmd_type,
            (u8 *)cmd_list[op_index].outbuf, cmd_list[op_index].info_len);
        if (ret) {
            pr_err("hclge: hns_xsfp_get_additional_info err! device id %d, cmd= %d, ret %d\n",
                dev_id, cmd_list[op_index].cmd_type, ret);
            return ret;
        }
    }
    return 0;
}
EXPORT_SYMBOL(hns_xsfp_get_additional_info);

int hclge_get_qsfp_present(u32 dev_id, u32 *qsfp_present)
{
    struct hnae3_ae_dev *ae_dev = NULL;
    struct hclge_dev *hdev = NULL;
    struct pci_dev *pdev = NULL;
    struct hclge_desc desc;
    int ret;

    if ((dev_id > MAX_DEV_ID) || (qsfp_present == NULL)) {
        pr_err("hclge: hclge qsfp present param invalid, dev_id:%u\n", dev_id);
        return -EINVAL;
    }

    pdev = hclge_get_pci_dev(dev_id);
    if (pdev == NULL) {
        pr_err("hclge: hclge gets pci dev failed\n");
        return -ENODEV;
    }

    ae_dev = pci_get_drvdata(pdev);
    if (ae_dev == NULL) {
        pr_err("hclge: %s: pci gets drv data failed\n", __func__);
        return -ENXIO;
    }

    hdev = ae_dev->priv;
    if (hdev == NULL) {
        pr_err("hclge: %s: qsfp present:ae_dev gets dpriv failed\n", __func__);
        return -ENXIO;
    }

    if (test_bit(HCLGE_STATE_CMD_DISABLE, &hdev->state)) {
        return 0;
    }

    hclge_cmd_setup_basic_desc(&desc, NETWORK_OPC_QUERY_OPTICAL_PRESENT, true);
    ret = hclge_cmd_send(&hdev->hw, &desc, 1);
    if (ret) {
        dev_err(&hdev->pdev->dev, "failed to get qsfp present, ret = %d\n", ret);
        return ret;
    }

    *qsfp_present = le32_to_cpu(desc.data[0]);
    return 0;
}
EXPORT_SYMBOL(hclge_get_qsfp_present);

int hclge_set_autoadapt(u32 dev_id, u32 auto_adapt_flag)
{
    struct hnae3_ae_dev *ae_dev = NULL;
    struct hclge_dev *hdev = NULL;
    struct pci_dev *pdev = NULL;
    struct hclge_desc desc = {0};
    int ret;

    if (dev_id > MAX_DEV_ID) {
        pr_err("hclge: hclge set autoadapt param invalid, dev_id:%u\n", dev_id);
        return -EINVAL;
    }

    if ((auto_adapt_flag != 0) && (auto_adapt_flag != 1)) {
        pr_err("hclge: hclge set autoadapt param invalid, auto_adapt_flag:%u\n", auto_adapt_flag);
        return -EINVAL;
    }

    pdev = hclge_get_pci_dev(dev_id);
    if (pdev == NULL) {
        pr_err("hclge: hclge gets pci dev failed\n");
        return -ENODEV;
    }

    ae_dev = pci_get_drvdata(pdev);
    if (ae_dev == NULL) {
        pr_err("hclge: %s: pci gets drv data failed\n", __func__);
        return -ENODEV;
    }

    hdev = ae_dev->priv;
    if (hdev == NULL) {
        pr_err("hclge: %s: set autoadapt:ae_dev gets dpriv failed\n", __func__);
        return -ENODEV;
    }

    hclge_cmd_setup_basic_desc(&desc, NETWORK_OPC_SET_OPTICAL_AUTO_ADAPT, false);
    desc.data[0] = cpu_to_le32(auto_adapt_flag);
    ret = hclge_cmd_send(&hdev->hw, &desc, 1);
    if (ret) {
        dev_err(&hdev->pdev->dev, "failed to set auto_adapt, ret = %d\n", ret);
        return ret;
    }

    return 0;
}
EXPORT_SYMBOL(hclge_set_autoadapt);