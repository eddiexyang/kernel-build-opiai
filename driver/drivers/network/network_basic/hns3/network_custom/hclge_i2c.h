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

#ifndef __HCLGE_I2C_H
#define __HCLGE_I2C_H

#ifndef DEFINE_HNS_LLT
#define STATIC static
#else
#define STATIC
#endif

#define HCLGE_I2C_CHAN_S    0
#define HCLGE_I2C_CHAN_M    (0x1 << HCLGE_I2C_CHAN_S)

#define HCLGE_I2C_DEV_ADDR_S    0
#define HCLGE_I2C_DEV_ADDR_M    (0xFFFF << HCLGE_I2C_DEV_ADDR_S)

#define HCLGE_I2C_REG_ADDR_S    0
#define HCLGE_I2C_REG_ADDR_M    (0xFFFF << HCLGE_I2C_REG_ADDR_S)

#define HCLGE_I2C_REG_LEN_S     0
#define HCLGE_I2C_REG_LEN_M     (0x1 << HCLGE_I2C_REG_LEN_S)

#define HCLGE_I2C_MSG_LEN_S     0
#define HCLGE_I2C_MSG_LEN_M     (0x1 << HCLGE_I2C_MSG_LEN_S)

#define HCLGE_I2C_MSG_VAL_S     0
#define HCLGE_I2C_MSG_VAL_M     (0xFF << HCLGE_I2C_MSG_VAL_S)

#define HCLGE_XSFP_OP_INDEX_S    0
#define HCLGE_XSFP_OP_INDEX_M    (0xFFFF << HCLGE_XSFP_OP_INDEX_S)

#define HCLGE_I2CDEV_QSFP_PRESENT   0
#define TEMP_MSB_SHIFT      8
#define TEMP_SIGN_BIT       15
#define TEMP_DATA_BIT       0x7FFF
#define TEMP_FULL_SCALE     128
#define TEMP_DA_SCALE       32768
#define TEMP_SCAL_UINT      255

#define QSFP_CHAN               0
#define QSFP_DEV_ADDR           0x50
#define QSFP_HIGH_POWER_EN_ADDR    0x5d

#define QSFP_TEMP_MSB_REG       0x16
#define QSFP_TEMP_LSB_REG       0x17
#define QSFP_IDENTIFIER_REG       0x0
#define QSFP_REG_LEN            1
#define QSFP_MSG_LEN            1

#define RETIMER_LANE_NUM    8
#define RETIMER_OUT_BUF_LEN  24
#define RETIMER_TEST_FW_VER_LEN 3
#define RETIMER_L_FW_VER_LEN  16  // new version 12
#define RETIMER_H_FW_VER_LEN  20

#define HCLGE_VENDOR_NAME_LEN             16
#define HCLGE_VENDOR_OUI_LEN              3
#define HCLGE_VENDOR_PN_LEN               16
#define HCLGE_WAVE_LEN                    2
#define HCLGE_VENDOR_SN_LEN               16
#define HCLGE_DATA_CODE_LEN               8
#define HCLGE_OPTICAL_POWER_LEN           8
#define HCLGE_MAX_UDATA_LEN   16
#define HCLGE_OPTICAL_MODULE_MEDIA_LEN    4

#define HCLGE_TEMP_INVALID      0x7EFF
#define HCLGE_TEMP_FAULT        0x7FFF

enum hclge_get_xsfp_op_index {
    HCLGE_OPC_GET_XSFP_VEN_NAME = 0,
    HCLGE_OPC_GET_XSFP_VEN_OUI,
    HCLGE_OPC_GET_XSFP_VEN_PN,
    HCLGE_OPC_GET_XSFP_VEN_SN,
    HCLGE_OPC_GET_XSFP_WAVELENGTH,
    HCLGE_OPC_GET_XSFP_DATE_CODE,
    HCLGE_OPC_GET_XSFP_TEMPERATURE,
    HCLGE_OPC_GET_XSFP_IDENTIFIER,
    HCLGE_OPC_GET_XSFP_HIGH_POWER_ENABLE
};

enum hclge_get_xsfp_additional_op_index {
    HCLGE_OPC_GET_XSFP_VOLTAGE = 0,
    HCLGE_OPC_GET_XSFP_TX_POWER,
    HCLGE_OPC_GET_XSFP_RX_POWER,
    HCLGE_OPC_GET_XSFP_VCC_HIGH_THRESHOLD,
    HCLGE_OPC_GET_XSFP_VCC_LOW_THRESHOLD,
    HCLGE_OPC_GET_XSFP_VCC_HIGH_ALARM_THRESHOLD,
    HCLGE_OPC_GET_XSFP_VCC_LOW_ALARM_THRESHOLD,
    HCLGE_OPC_GET_XSFP_TEMP_HIGH_THRESHOLD,
    HCLGE_OPC_GET_XSFP_TEMP_LOW_THRESHOLD,
    HCLGE_OPC_GET_XSFP_TEMP_HIGH_ALARM_THRESHOLD,
    HCLGE_OPC_GET_XSFP_TEMP_LOW_ALARM_THRESHOLD,
    HCLGE_OPC_GET_XSFP_TX_POWER_HIGH_THRESHOLD,
    HCLGE_OPC_GET_XSFP_TX_POWER_LOW_THRESHOLD,
    HCLGE_OPC_GET_XSFP_TX_POWER_HIGH_ALARM_THRESHOLD,
    HCLGE_OPC_GET_XSFP_TX_POWER_LOW_ALARM_THRESHOLD,
    HCLGE_OPC_GET_XSFP_RX_POWER_HIGH_THRESHOLD,
    HCLGE_OPC_GET_XSFP_RX_POWER_LOW_THRESHOLD,
    HCLGE_OPC_GET_XSFP_RX_POWER_HIGH_ALARM_THRESHOLD,
    HCLGE_OPC_GET_XSFP_RX_POWER_LOW_ALARM_THRESHOLD,
    HCLGE_OPC_GET_XSFP_TX_BIAS_HIGH_THRESHOLD,
    HCLGE_OPC_GET_XSFP_TX_BIAS_LOW_THRESHOLD,
    HCLGE_OPC_GET_XSFP_TX_BIAS_HIGH_ALARM_THRESHOLD,
    HCLGE_OPC_GET_XSFP_TX_BIAS_LOW_ALARM_THRESHOLD,
    HCLGE_OPC_GET_XSFP_TX_BIAS,
    HCLGE_OPC_GET_XSFP_TX_LOS,
    HCLGE_OPC_GET_XSFP_RX_LOS,
    HCLGE_OPC_GET_XSFP_MEDIA_TYPE
};

struct additional_cmd_list {
    int cmd_type;
    char *outbuf;
    int info_len;
};

#define HCLGE_QSFP_VENDOR_NAME_LEN  16
#define HCLGE_QSFP_VENDOR_NAME_LEN  16
struct hclge_i2c_cfg_cmd {
    unsigned int chan;
    unsigned int dev_addr;
    unsigned int reg_addr;
    unsigned int reg_len;
    unsigned int msg_len;
    unsigned int msg_val;
};

struct hclge_retimer_prim_fw_version_info {
    unsigned char major_ver;
    unsigned char minor_ver;
    unsigned char implement_ver;
    unsigned char revision_ver;
};

struct hclge_retimer_l1_fw_version_info {
    unsigned char major_ver;
    unsigned char minor_ver;
    unsigned char implement_ver;
    unsigned char revision_ver;
};

struct hclge_retimer_l2_fw_version_info {
    unsigned char major_ver;
    unsigned char minor_ver;
    unsigned char implement_ver;
    unsigned char revision_ver;
};

struct hclge_retimer_hilk_calbrt_fw_version_info {
    unsigned char major_ver;
    unsigned char minor_ver;
    unsigned char implement_ver;
    unsigned char revision_ver;
};

struct hclge_retimer_hilk_adapt_fw_version_info {
    unsigned char major_ver;
    unsigned char minor_ver;
    unsigned char implement_ver;
    unsigned char revision_ver;
};

typedef struct hclge_smbs_retimer_fw_ver {
    struct hclge_retimer_prim_fw_version_info prim_fw_version_info;
    struct hclge_retimer_l2_fw_version_info l2_fw_version_info;
    struct hclge_retimer_hilk_calbrt_fw_version_info hilk_calbrt_fw_version_info;
    struct hclge_retimer_hilk_adapt_fw_version_info hilk_adapt_fw_version_info;
} retimer_fw_ver;

struct in_out_buf {
    int has_in_buf;
    union {
        unsigned int reg_addr;
        unsigned int reg_val;
        char buff[RETIMER_OUT_BUF_LEN];
    };
};

struct hclge_i2c_params {
    unsigned int chan;
    unsigned int dev_addr;
    unsigned int reg_addr;
    unsigned int reg_len;
};

enum i2c_enable_status {
    I2C_ENABLE,
    I2C_DISABLE
};

enum i2c_channel_type {
    RETIMER_I2C_CHANNEL,
    XSFP_I2C_CHANNEL
};

struct i2c_control_struct {
    enum i2c_enable_status control_status;
    enum i2c_channel_type i2c_channel;
};

struct xsfp_additional_cmd {
    int xsfp_voltage;
    int xsfp_vcc_high_threshold;
    int xsfp_vcc_low_threshold;
    int xsfp_vcc_high_alarm_threshold;
    int xsfp_vcc_low_alarm_threshold;
    int xsfp_temp_high_threshold;
    int xsfp_temp_low_threshold;
    int xsfp_temp_high_alarm_threshold;
    int xsfp_temp_low_alarm_threshold;
    int xsfp_tx_power_high_threshold;
    int xsfp_tx_power_low_threshold;
    int xsfp_tx_power_high_alarm_threshold;
    int xsfp_tx_power_low_alarm_threshold;
    int xsfp_rx_power_high_threshold;
    int xsfp_rx_power_low_threshold;
    int xsfp_rx_power_high_alarm_threshold;
    int xsfp_rx_power_low_alarm_threshold;
    int xsfp_tx_bias_high_threshold;
    int xsfp_tx_bias_low_threshold;
    int xsfp_tx_bias_high_alarm_threshold;
    int xsfp_tx_bias_low_alarm_threshold;
    char xsfp_tx_power[HCLGE_OPTICAL_POWER_LEN];
    char xsfp_rx_power[HCLGE_OPTICAL_POWER_LEN];
    char xsfp_tx_bias[HCLGE_OPTICAL_POWER_LEN];
    char xsfp_moudle_media[HCLGE_OPTICAL_MODULE_MEDIA_LEN];
    int xsfp_tx_los;
    int xsfp_rx_los;
};

extern int hns_xsfp_get_temperature(unsigned int dev_id, unsigned int port_id, int *i2cdev_temp);
extern int hns_xsfp_get_high_power_en(unsigned int dev_id, unsigned int port_id, unsigned int *enable);
extern int hns_xsfp_get_optical_identifier(unsigned int dev_id, unsigned int port_id, int *identifier);
extern int hclge_retimer_get_fw_version(unsigned int dev_id, unsigned int port_id, char *fw_version);
extern int hns_xsfp_get_vendor_name(unsigned int dev_id, unsigned int port_id,
    unsigned char *vendor_name, unsigned int name_len);
extern int hns_xsfp_get_vendor_pn(unsigned int dev_id, unsigned int port_id,
    unsigned char *vendor_pn, unsigned int pn_len);
extern int hns_xsfp_get_vendor_oui(unsigned int dev_id, unsigned int port_id,
    unsigned char *vendor_oui, unsigned int oui_len);
extern int hns_xsfp_get_vendor_sn(unsigned int dev_id, unsigned int port_id,
    unsigned char *vendor_sn, unsigned int sn_len);
extern int hns_xsfp_get_wavelength(unsigned int dev_id, unsigned int port_id,
    unsigned char *wavelength, unsigned int wavelength_len);
extern int hns_xsfp_get_date_code(unsigned int dev_id, unsigned int port_id,
    unsigned char *date_code, unsigned int date_code_len);
extern int hclge_i2c_control(unsigned int dev_id, enum i2c_enable_status enable_status,
    enum i2c_channel_type i2c_channel_num);
extern int hclge_get_qsfp_present(unsigned int dev_id, unsigned int *qsfp_present);
extern int hclge_retimer_get_temp(unsigned int dev_id, unsigned int port_id, short int *retimer_temp);
extern int hns_xsfp_get_additional_info(unsigned int dev_id, unsigned int port_id,
    struct xsfp_additional_cmd *additional_cmd);
extern int hclge_set_autoadapt(unsigned int dev_id, unsigned int auto_adapt_flag);
#endif
