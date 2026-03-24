/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: platform_mac
 * Author: huawei
 * Create: 2021-12-28
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>

#include "kcompat.h"
#include "hclge_plf_main.h"
#include "platform_mac.h"

#ifndef DEFINE_HNS_LLT
#define STATIC static
#else
#define STATIC
#endif

void mag_enable_igu_egu_txrx(struct hclge_plf_dev *hdev);

STATIC int mac_cfg_calendar(struct hclge_plf_dev *hdev, enum XXVGE_MAC_SPEED speed)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    u_xxvge_pma_calendar_table pma_calendar_table;
    u_xxvge_tx_calendar_table tx_calendar_table;
    u_xxvge_tx_calen_depth tx_calen_depth;
    u32 pma_calendar_table_addr;
    u32 tx_calendar_table_addr;
    u32 tx_calen_depth_addr;

    pma_calendar_table_addr = XXVGE_PMA_CALENDAR_TABLE_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD;
    tx_calendar_table_addr = XXVGE_TX_CALENDAR_TABLE_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD;
    tx_calen_depth_addr = XXVGE_TX_CALEN_DEPTH_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD;
    pma_calendar_table.value = hclge_read_dev(&hdev->hw, pma_calendar_table_addr);
    tx_calendar_table.value = hclge_read_dev(&hdev->hw, tx_calendar_table_addr);
    tx_calen_depth.value = hclge_read_dev(&hdev->hw, tx_calen_depth_addr);
    switch (speed) {
        case XXVGE_MAC_SPEED_10M:
        case XXVGE_MAC_SPEED_100M:
        case XXVGE_MAC_SPEED_1G:
        case XXVGE_MAC_SPEED_2_5G:
            /* depth 4 */
            tx_calen_depth.bits.depth = 4;
            hclge_write_mag_dev(&hdev->hw, tx_calen_depth_addr, tx_calen_depth.value);
            /* 0123 schedule */
            if (mac->mac_mode == XXVGE_MAC_MODE_RGMII) {
                tx_calendar_table.value = 0x0123; // rgmii sche 0123
            } else {
                tx_calendar_table.value = 0x1010; // other sche 1010
            }
            hclge_write_mag_dev(&hdev->hw, tx_calendar_table_addr, tx_calendar_table.value);
            pma_calendar_table.bits.content0 = 0;
            pma_calendar_table.bits.content1 = 0;
            pma_calendar_table.bits.content2 = 0;
            pma_calendar_table.bits.content3 = 0;
            pma_calendar_table.bits.content4 = 0;
            pma_calendar_table.bits.content5 = 0;
            pma_calendar_table.bits.content6 = 0;
            pma_calendar_table.bits.content7 = 0;
            hclge_write_mag_dev(&hdev->hw, pma_calendar_table_addr, pma_calendar_table.value);
            break;
        default:
            dev_err(&hdev->pdev->dev, "speed(%d) not support\n", speed);
            return -1;
    }

    return 0;
}

#if (defined CONFIG_PLATFORM_EMU) || (defined CONFIG_PLATFORM_ASIC)
STATIC int mac_cfg_sgmii_freq(struct hclge_plf_dev *hdev)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    u32 work_freq_h = PLATFORM_MAC_WORK_FREQ_H;
    u32 work_freq_l = PLATFORM_MAC_WORK_FREQ_L;
    u32 base_freq_h, base_freq_l;

    switch (mac->speed) {
        case XXVGE_MAC_SPEED_10M:
            base_freq_h = PLATFORM_MAC_10M_SGMII_BASE_FREQ_H;
            base_freq_l = PLATFORM_MAC_10M_SGMII_BASE_FREQ_L;
            break;
        case XXVGE_MAC_SPEED_100M:
            base_freq_h = PLATFORM_MAC_100M_SGMII_BASE_FREQ_H;
            base_freq_l = PLATFORM_MAC_100M_SGMII_BASE_FREQ_L;
            break;
        case XXVGE_MAC_SPEED_1G:
            base_freq_h = PLATFORM_MAC_1000M_SGMII_BASE_FREQ_H;
            base_freq_l = PLATFORM_MAC_1000M_SGMII_BASE_FREQ_L;
            break;
        case XXVGE_MAC_SPEED_2_5G:
            base_freq_h = PLATFORM_MAC_2500M_SGMII_BASE_FREQ_H;
            base_freq_l = PLATFORM_MAC_2500M_SGMII_BASE_FREQ_L;
            break;
        default:
            dev_err(&hdev->pdev->dev, "speed(%d) not supported\n", mac->speed);
            return -1;
    }

    /* wrok frequency */
    hclge_write_mag_dev(&hdev->hw, (XXVGE_WORK_FREQ_H_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD), work_freq_h);
    hclge_write_mag_dev(&hdev->hw, (XXVGE_WORK_FREQ_L_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD), work_freq_l);

    /* tx mac frequency */
    hclge_write_mag_dev(&hdev->hw, (XXVGE_TXMAC_BASE_FREQ_H_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD), base_freq_h);
    hclge_write_mag_dev(&hdev->hw, (XXVGE_TXMAC_BASE_FREQ_L_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD), base_freq_l);

    return 0;
}

STATIC int mac_cfg_rgmii_freq(struct hclge_plf_dev *hdev)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    u32 work_freq_h = PLATFORM_MAC_WORK_FREQ_H;
    u32 work_freq_l = PLATFORM_MAC_WORK_FREQ_L;
    u32 base_freq_h, base_freq_l;

    switch (mac->speed) {
        case XXVGE_MAC_SPEED_10M:
            base_freq_h = PLATFORM_MAC_10M_RGMII_BASE_FREQ_H;
            base_freq_l = PLATFORM_MAC_10M_RGMII_BASE_FREQ_L;
            break;
        case XXVGE_MAC_SPEED_100M:
            base_freq_h = PLATFORM_MAC_100M_RGMII_BASE_FREQ_H;
            base_freq_l = PLATFORM_MAC_100M_RGMII_BASE_FREQ_L;
            break;
        case XXVGE_MAC_SPEED_1G:
            base_freq_h = PLATFORM_MAC_1000M_RGMII_BASE_FREQ_H;
            base_freq_l = PLATFORM_MAC_1000M_RGMII_BASE_FREQ_L;
            break;
        default:
            dev_err(&hdev->pdev->dev, "speed(%d) not supported\n", mac->speed);
            return -1;
    }

    /* wrok frequency */
    hclge_write_mag_dev(&hdev->hw, (XXVGE_WORK_FREQ_H_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD), work_freq_h);
    hclge_write_mag_dev(&hdev->hw, (XXVGE_WORK_FREQ_L_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD), work_freq_l);

    /* tx mac frequency */
    hclge_write_mag_dev(&hdev->hw, (XXVGE_TXMAC_BASE_FREQ_H_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD), base_freq_h);
    hclge_write_mag_dev(&hdev->hw, (XXVGE_TXMAC_BASE_FREQ_L_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD), base_freq_l);

    return 0;
}

STATIC int mac_cfg_freq(struct hclge_plf_dev *hdev)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;

    if (mac->mac_mode == XXVGE_MAC_MODE_RGMII) {
        return mac_cfg_rgmii_freq(hdev);
    } else if (mac->mac_mode < XXVGE_MAC_MODE_MAX) { // SGMII & SFP
        return mac_cfg_sgmii_freq(hdev);
    } else {
        return -1;
    }
}
#else
/*
 * work freq
 * 10/100/1000M : 400M
 * txmac_base freq
 * 10M : 1.25 * 2.5 * 8 = 25M
 * 100M : 12.5 / 4 * 8 = 25M
 * 1000M : 125M
 */
STATIC int mac_cfg_rgmii_freq(struct hclge_plf_dev *hdev)
{
/* 400M 0x5D21DBA000 */
#define FREQ_H_400M    0x5d
#define FREQ_L_400M    0x21dba000
/* 25M 0x5D21DBA00 */
#define FREQ_H_25M    0x5
#define FREQ_L_25M    0xD21DBA00
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    u32 work_freq_h = FREQ_H_400M;
    u32 work_freq_l = FREQ_L_400M;
    u32 base_freq_h, base_freq_l;

    switch (mac->speed) {
        case XXVGE_MAC_SPEED_10M:
            base_freq_h = FREQ_H_25M;
            base_freq_l = FREQ_L_25M;
            break;
        case XXVGE_MAC_SPEED_100M:
            base_freq_h = FREQ_H_25M;
            base_freq_l = FREQ_L_25M;
            break;
        case XXVGE_MAC_SPEED_1G:
            base_freq_h = PLATFORM_MAC_1000M_RGMII_BASE_FREQ_H;
            base_freq_l = PLATFORM_MAC_1000M_RGMII_BASE_FREQ_L;
            break;
        default:
            dev_err(&hdev->pdev->dev, "speed(%d) not supported\n", mac->speed);
            return -1;
    }

    /* work frequency */
    hclge_write_mag_dev(&hdev->hw, (XXVGE_WORK_FREQ_H_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD), work_freq_h);
    hclge_write_mag_dev(&hdev->hw, (XXVGE_WORK_FREQ_L_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD), work_freq_l);

    /* tx mac frequency */
    hclge_write_mag_dev(&hdev->hw, (XXVGE_TXMAC_BASE_FREQ_H_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD), base_freq_h);
    hclge_write_mag_dev(&hdev->hw, (XXVGE_TXMAC_BASE_FREQ_L_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD), base_freq_l);

    return 0;
}

STATIC int mac_cfg_freq(struct hclge_plf_dev *hdev)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;

    if (mac->mac_mode == XXVGE_MAC_MODE_SGMII) {
        /* work frequency, FPGA 12.5M */
        hclge_write_mag_dev(&hdev->hw, (XXVGE_WORK_FREQ_H_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD), 0x2);
        hclge_write_mag_dev(&hdev->hw, (XXVGE_WORK_FREQ_L_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD), 0xE90EDD00);

        /* for CONFIG_FPGA_COMAG, 0x1 74876E80 (real_freq/2 = 6.25M) */
        hclge_write_mag_dev(&hdev->hw, (XXVGE_TXMAC_BASE_FREQ_H_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD), 0x1);
        hclge_write_mag_dev(&hdev->hw, (XXVGE_TXMAC_BASE_FREQ_L_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD), 0x74876E80);

        return 0;
    }

    return mac_cfg_rgmii_freq(hdev);
}
#endif

STATIC void mac_cfg_txmac_rf_tx_en(struct hclge_plf_dev *hdev)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    u_xxvge_txmac_lf_rf_control lf_rf_control;
    u32 lf_rf_control_addr;

    lf_rf_control_addr = XXVGE_TXMAC_LF_RF_CONTROL_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD;
    lf_rf_control.value = hclge_read_dev(&hdev->hw, lf_rf_control_addr);
    if (mac->mac_mode == XXVGE_MAC_MODE_SGMII) {
        lf_rf_control.bits.rf_tx_en = 0;
    } else {
        lf_rf_control.bits.rf_tx_en = 1;
    }
    hclge_write_mag_dev(&hdev->hw, lf_rf_control_addr, lf_rf_control.value);
}

STATIC void mac_cfg_txmac_bit_delay(struct hclge_plf_dev *hdev, u32 bit_delay)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    u_xxvge_txmac_lane_1ui_dly lane_1ui_dly;
    u32 lane_1ui_dly_addr;

    lane_1ui_dly_addr = XXVGE_TXMAC_LANE_1UI_DLY_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD;
    lane_1ui_dly.value = hclge_read_dev(&hdev->hw, lane_1ui_dly_addr);
    lane_1ui_dly.bits.bit_delay = bit_delay;
    hclge_write_mag_dev(&hdev->hw, lane_1ui_dly_addr, lane_1ui_dly.value);
}

STATIC void mac_cfg_txmac_replicate(struct hclge_plf_dev *hdev, u32 sel, u32 factor)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    u_xxvge_txmac_replicate replicate;
    u32 replicate_addr = XXVGE_TXMAC_REPLICATE_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD;

    replicate.value = hclge_read_dev(&hdev->hw, replicate_addr);
    replicate.bits.replicate_sel = sel;
    replicate.bits.factor = factor;
    hclge_write_mag_dev(&hdev->hw, replicate_addr, replicate.value);
}

STATIC void mac_cfg_rxmac_bit_delay(struct hclge_plf_dev *hdev, u32 bit_delay)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    u_xxvge_rxmac_lane_1ui_dly lane_1ui_dly;
    u32 lane_1ui_dly_addr;

    lane_1ui_dly_addr = XXVGE_RXMAC_LANE_1UI_DLY_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD;
    lane_1ui_dly.value = hclge_read_dev(&hdev->hw, lane_1ui_dly_addr);
    lane_1ui_dly.bits.bit_delay = bit_delay;
    hclge_write_mag_dev(&hdev->hw, lane_1ui_dly_addr, lane_1ui_dly.value);
}

STATIC void mac_cfg_rxmac_replicate(struct hclge_plf_dev *hdev, u32 sel, u32 factor)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    u_xxvge_rxmac_replicate replicate;
    u32 replicate_addr;

    replicate_addr = XXVGE_RXMAC_REPLICATE_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD;
    replicate.value = hclge_read_dev(&hdev->hw, replicate_addr);
    replicate.bits.replicate_sel = sel;
    replicate.bits.factor = factor;
    hclge_write_mag_dev(&hdev->hw, replicate_addr, replicate.value);
}

STATIC void mac_disable_1588(struct hclge_plf_dev *hdev)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    u_xxvge_rxmac_control rxmac_control;
    u32 rxmac_control_addr;

    rxmac_control_addr = XXVGE_RXMAC_CONTROL_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD;
    rxmac_control.value = hclge_read_dev(&hdev->hw, rxmac_control_addr);
    rxmac_control.bits.rx_1588_en = 0;
    hclge_write_mag_dev(&hdev->hw, rxmac_control_addr, rxmac_control.value);
}

STATIC void mac_enable_ipg_check(struct hclge_plf_dev *hdev)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    u_xxvge_rxmac_control rxmac_control;
    u32 rxmac_control_addr;

    rxmac_control_addr = XXVGE_RXMAC_CONTROL_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD;
    rxmac_control.value = hclge_read_dev(&hdev->hw, rxmac_control_addr);
    rxmac_control.bits.ipg_chk_en = 1;
    hclge_write_mag_dev(&hdev->hw, rxmac_control_addr, rxmac_control.value);
}

STATIC void mac_cfg_rxmac_rx_ipg(struct hclge_plf_dev *hdev)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    u_xxvge_rxmac_control1 rxmac_control1;
    u32 rxmac_control1_addr;
    u8 rx_ipg = 5; /* rx ipg default 5 */

    if (mac->mac_mode == XXVGE_MAC_MODE_RGMII &&
        (mac->speed == XXVGE_MAC_SPEED_10M || mac->speed == XXVGE_MAC_SPEED_100M)) {
        rx_ipg = 10; /* rgmii 10M/100M use 10 ipg */
    }

    rxmac_control1_addr = XXVGE_RXMAC_CONTROL1_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD;
    rxmac_control1.value = hclge_read_dev(&hdev->hw, rxmac_control1_addr);
    rxmac_control1.bits.rx_ipg = rx_ipg;
    hclge_write_mag_dev(&hdev->hw, rxmac_control1_addr, rxmac_control1.value);
}

STATIC void mac_cfg_common_link_sync(struct hclge_plf_dev *hdev, u32 link_sync)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    u_xxvge_link_control link_control;
    u32 link_control_addr;

    link_control_addr = XXVGE_LINK_CONTROL_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD;
    link_control.value = hclge_read_dev(&hdev->hw, link_control_addr);
    link_control.bits.link_sync = link_sync;
    hclge_write_mag_dev(&hdev->hw, link_control_addr, link_control.value);
}

STATIC void mac_cfg_common_an(struct hclge_plf_dev *hdev, u32 phy_link_en, u32 speed_sel,
                              u32 pause_sel, u32 duplex_sel)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    u_xxvge_an_cfg_sel an_cfg_sel;
    u32 an_cfg_sel_addr;

    an_cfg_sel_addr = XXVGE_AN_CFG_SEL_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD;
    an_cfg_sel.value = hclge_read_dev(&hdev->hw, an_cfg_sel_addr);
    an_cfg_sel.bits.phy_link_en = phy_link_en;
    an_cfg_sel.bits.speed_sel = speed_sel;
    an_cfg_sel.bits.pause_sel = pause_sel;
    an_cfg_sel.bits.duplex_sel = duplex_sel;
    hclge_write_mag_dev(&hdev->hw, an_cfg_sel_addr, an_cfg_sel.value);
}

STATIC void mac_cfg_pcs_an_np_en(struct hclge_plf_dev *hdev, u32 np_en)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    u_xxvge_pcs_an_ctrl pcs_an_ctrl;
    u32 pcs_an_ctrl_addr;

    pcs_an_ctrl_addr = XXVGE_PCS_AN_CTRL_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD;
    pcs_an_ctrl.value = hclge_read_dev(&hdev->hw, pcs_an_ctrl_addr);
    pcs_an_ctrl.bits.np_en = np_en;
    hclge_write_mag_dev(&hdev->hw, pcs_an_ctrl_addr, pcs_an_ctrl.value);
}

STATIC void mac_cfg_pcs_an_autoneg_en(struct hclge_plf_dev *hdev, u32 autoneg_en)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    u_xxvge_pcs_an_ctrl pcs_an_ctrl;
    u32 pcs_an_ctrl_addr;

    pcs_an_ctrl_addr = XXVGE_PCS_AN_CTRL_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD;
    pcs_an_ctrl.value = hclge_read_dev(&hdev->hw, pcs_an_ctrl_addr);
    pcs_an_ctrl.bits.autoneg_en = autoneg_en;
    hclge_write_mag_dev(&hdev->hw, pcs_an_ctrl_addr, pcs_an_ctrl.value);
}

STATIC void mac_cfg_pcs_an_link_time(struct hclge_plf_dev *hdev, u32 link_time)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    u_xxvge_pcs_an_link_time pcs_an_link_time;
    u32 pcs_an_link_time_addr;

    pcs_an_link_time_addr = XXVGE_PCS_AN_LINK_TIME_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD;
    pcs_an_link_time.value = hclge_read_dev(&hdev->hw, pcs_an_link_time_addr);
    pcs_an_link_time.bits.link_time = link_time;
    hclge_write_mag_dev(&hdev->hw, pcs_an_link_time_addr, pcs_an_link_time.value);
}

STATIC void mac_cfg_an_base_page(struct hclge_plf_dev *hdev, u32 port_speed, u8 duplex_mode, u8 link_status)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    u_xxvge_pcs_an_base_page pcs_an_base_page;
    u32 pcs_an_base_page_addr;

    pcs_an_base_page_addr = XXVGE_PCS_AN_BASE_PAGE + mac->mac_id * XXVGE_MAC_REG_PERIOD;
    pcs_an_base_page.value = hclge_read_dev(&hdev->hw, pcs_an_base_page_addr);
    pcs_an_base_page.bits.port_speed = port_speed;
    pcs_an_base_page.bits.duplex_mode = duplex_mode;
    pcs_an_base_page.bits.link_status = link_status;
    hclge_write_mag_dev(&hdev->hw, pcs_an_base_page_addr, pcs_an_base_page.value);
}

void mac_sgmii_cfg_autoneg(struct hclge_plf_dev *hdev, bool enable)
{
    mac_cfg_common_link_sync(hdev, 0);
    mac_cfg_common_an(hdev, 1, 0, 0, 1);
    mac_cfg_pcs_an_np_en(hdev, 0);
#if defined (CONFIG_PLATFORM_FPGA)
    mac_cfg_pcs_an_link_time(hdev, 0x4e20); /* 1.6MS, 12.5M */
    mac_cfg_an_base_page(hdev, 2, 1, 1); /* port_speed is 2 */
#elif defined (CONFIG_PLATFORM_ASIC)
    mac_cfg_pcs_an_link_time(hdev, 0x61a80); /* 1.6MS, 250M */
#endif
    mac_cfg_pcs_an_autoneg_en(hdev, enable);

    dev_info(&hdev->pdev->dev, " %s mac(%d) cfg autoneg OK.\n", enable ? "Enable" : "Disable", hdev->hw.mac.mac_id);
}

STATIC void mac_cfg_rxpma_enable(struct hclge_plf_dev *hdev, u32 enable)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    u_xxvge_rxpma_enable rxpma_enable;
    u32 rxpma_enable_addr;

    rxpma_enable_addr = XXVGE_RXPMA_ENABLE_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD;
    rxpma_enable.value = hclge_read_dev(&hdev->hw, rxpma_enable_addr);
    rxpma_enable.bits.enable = enable;
    hclge_write_mag_dev(&hdev->hw, rxpma_enable_addr, rxpma_enable.value);
}

void mac_cfg_common_link_up_int(struct hclge_plf_dev *hdev, u32 enable)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    u_xxvge_int_enable int_enable;
    u32 int_enable_addr;

    int_enable_addr = XXVGE_INT_ENABLE_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD;
    int_enable.value = hclge_read_dev(&hdev->hw, int_enable_addr);
    int_enable.bits.link_up = enable;
    hclge_write_mag_dev(&hdev->hw, int_enable_addr, int_enable.value);
}

void mac_cfg_common_link_down_int(struct hclge_plf_dev *hdev, u32 enable)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    u_xxvge_int_enable int_enable;
    u32 int_enable_addr;

    int_enable_addr = XXVGE_INT_ENABLE_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD;
    int_enable.value = hclge_read_dev(&hdev->hw, int_enable_addr);
    int_enable.bits.link_down = enable;
    hclge_write_mag_dev(&hdev->hw, int_enable_addr, int_enable.value);
}

STATIC int mac_cfg_port(struct hclge_plf_dev *hdev)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    u_xxvge_port_speed port_speed;
    u_xxvge_port_mode port_mode;
    u32 port_speed_addr;
    u32 port_mode_addr;

    if (mac->mac_mode >= XXVGE_MAC_MODE_MAX) {
        dev_err(&hdev->pdev->dev, "not support mac mode(%d)\n", mac->mac_mode);
        return -1;
    }

    port_mode_addr = XXVGE_PORT_MODE_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD;
    port_speed_addr = XXVGE_PORT_SPEED_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD;
    port_mode.value = hclge_read_dev(&hdev->hw, port_mode_addr);
    port_speed.value = hclge_read_dev(&hdev->hw, port_speed_addr);

    if (mac->mac_mode == XXVGE_MAC_MODE_SGMII) {
        port_mode.bits.tx_mode = 0x3;
        port_mode.bits.rx_mode = 0x3;
        hclge_write_mag_dev(&hdev->hw, port_mode_addr, port_mode.value);
    } else if (mac->mac_mode == XXVGE_MAC_MODE_RGMII) {
        port_mode.bits.tx_mode = 0x6;
        port_mode.bits.rx_mode = 0x6;
        hclge_write_mag_dev(&hdev->hw, port_mode_addr, port_mode.value);
    } else if (mac->mac_mode == XXVGE_MAC_MODE_SFP_1000BASE) {
        port_mode.bits.tx_mode = 0x1;
        port_mode.bits.rx_mode = 0x1;
        hclge_write_mag_dev(&hdev->hw, port_mode_addr, port_mode.value);
    } else if (mac->mac_mode == XXVGE_MAC_MODE_SFP_2500BASE) {
        port_mode.bits.tx_mode = 0x2; // 2.5GBASE-X mode : 2
        port_mode.bits.rx_mode = 0x2; // 2.5GBASE-X mode : 2
        hclge_write_mag_dev(&hdev->hw, port_mode_addr, port_mode.value);
    } else {
        /* nothing */
    }

    switch (mac->speed) {
        case XXVGE_MAC_SPEED_10M:
            port_speed.bits.tx_speed = 0;
            port_speed.bits.rx_speed = 0;
            hclge_write_mag_dev(&hdev->hw, port_speed_addr, port_speed.value);
            break;
        case XXVGE_MAC_SPEED_100M:
            port_speed.bits.tx_speed = 1;
            port_speed.bits.rx_speed = 1;
            hclge_write_mag_dev(&hdev->hw, port_speed_addr, port_speed.value);
            break;
        case XXVGE_MAC_SPEED_1G:
            port_speed.bits.tx_speed = 2; // 1G mode 2
            port_speed.bits.rx_speed = 2; // 1G mode 2
            hclge_write_mag_dev(&hdev->hw, port_speed_addr, port_speed.value);
            break;
        case XXVGE_MAC_SPEED_2_5G:
            port_speed.bits.tx_speed = 3; // 2.5G mode 3
            port_speed.bits.rx_speed = 3; // 2.5G mode 3
            hclge_write_mag_dev(&hdev->hw, port_speed_addr, port_speed.value);
            break;
        default:
            dev_err(&hdev->pdev->dev, "speed(%d) not supported\n", mac->speed);
            return -1;
    }

    return 0;
}

STATIC void mac_cfg_width(struct hclge_plf_dev *hdev, enum XXVGE_PMA_WIDTH width)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    u_xxvge_txpma_control txpma_control;
    u_xxvge_rxpma_control rxpma_control;
    u32 txpma_control_addr;
    u32 rxpma_control_addr;

    txpma_control_addr = XXVGE_TXPMA_CONTROL_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD;
    rxpma_control_addr = XXVGE_RXPMA_CONTROL_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD;
    txpma_control.value = hclge_read_dev(&hdev->hw, txpma_control_addr);
    rxpma_control.value = hclge_read_dev(&hdev->hw, rxpma_control_addr);
    txpma_control.bits.para_data_width = width;
    rxpma_control.bits.para_data_width = width;
    hclge_write_mag_dev(&hdev->hw, txpma_control_addr, txpma_control.value);
    hclge_write_mag_dev(&hdev->hw, rxpma_control_addr, rxpma_control.value);
}

STATIC void mac_cfg_reverse(struct hclge_plf_dev *hdev, u32 reverse)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    u_xxvge_txpma_control txpma_control;
    u_xxvge_rxpma_control rxpma_control;
    u32 txpma_control_addr;
    u32 rxpma_control_addr;

    txpma_control_addr = XXVGE_TXPMA_CONTROL_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD;
    rxpma_control_addr = XXVGE_RXPMA_CONTROL_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD;
    txpma_control.value = hclge_read_dev(&hdev->hw, txpma_control_addr);
    rxpma_control.value = hclge_read_dev(&hdev->hw, rxpma_control_addr);
    txpma_control.bits.tx_bit_reverse = reverse;
    rxpma_control.bits.rx_bit_reverse = reverse;
    hclge_write_mag_dev(&hdev->hw, txpma_control_addr, txpma_control.value);
    hclge_write_mag_dev(&hdev->hw, rxpma_control_addr, rxpma_control.value);
}

void mac_pause_en_cfg(struct hclge_plf_dev *hdev, u32 tx, u32 rx)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    u_xxvge_mac_pause_pfc_ctrl pause_ctrl;
    u32 pause_ctrl_addr = XXVGE_MAC_PAUSE_PFC_CTRL_REG(mac->mac_id);

    pause_ctrl.value = hclge_read_dev(&hdev->hw, pause_ctrl_addr);
    pause_ctrl.bits.tx_pause_en = ((tx != 0) ? 1 : 0);
    pause_ctrl.bits.rx_pause_en = ((rx != 0) ? 1 : 0);
    hclge_write_mag_dev(&hdev->hw, pause_ctrl_addr, pause_ctrl.value);
}

void mac_set_mac_enable(struct hclge_plf_dev *hdev)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    u_xxvge_rxmac_enable rxmac_enable;
    u_xxvge_txmac_enable txmac_enable;
    u32 rxmac_enable_addr;
    u32 txmac_enable_addr;

    rxmac_enable_addr = XXVGE_RXMAC_ENABLE_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD;
    txmac_enable_addr = XXVGE_TXMAC_ENABLE_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD;
    rxmac_enable.value = hclge_read_dev(&hdev->hw, rxmac_enable_addr);
    rxmac_enable.bits.rx_enable = 1;
    hclge_write_mag_dev(&hdev->hw, rxmac_enable_addr, rxmac_enable.value);

    txmac_enable.value = hclge_read_dev(&hdev->hw, txmac_enable_addr);
    txmac_enable.bits.tx_enable = 1;
    hclge_write_mag_dev(&hdev->hw, txmac_enable_addr, txmac_enable.value);
}

void mac_set_mac_disable(struct hclge_plf_dev *hdev)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    u_xxvge_rxmac_enable rxmac_enable;
    u_xxvge_txmac_enable txmac_enable;
    u32 rxmac_enable_addr;
    u32 txmac_enable_addr;

    rxmac_enable_addr = XXVGE_RXMAC_ENABLE_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD;
    txmac_enable_addr = XXVGE_TXMAC_ENABLE_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD;
    rxmac_enable.value = hclge_read_dev(&hdev->hw, rxmac_enable_addr);
    rxmac_enable.bits.rx_enable = 0;
    hclge_write_mag_dev(&hdev->hw, rxmac_enable_addr, rxmac_enable.value);

    txmac_enable.value = hclge_read_dev(&hdev->hw, txmac_enable_addr);
    txmac_enable.bits.tx_enable = 0;
    hclge_write_mag_dev(&hdev->hw, txmac_enable_addr, txmac_enable.value);
}

STATIC void mac_set_mac_enable_rx(struct hclge_plf_dev *hdev)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    u_xxvge_rxmac_enable rxmac_enable;
    u32 rxmac_enable_addr = XXVGE_RXMAC_ENABLE_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD;

    rxmac_enable.value = hclge_read_dev(&hdev->hw, rxmac_enable_addr);
    rxmac_enable.bits.rx_enable = 1;
    hclge_write_mag_dev(&hdev->hw, rxmac_enable_addr, rxmac_enable.value);
}

void mac_set_mac_disable_rx(struct hclge_plf_dev *hdev)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    u_xxvge_rxmac_enable rxmac_enable;
    u32 rxmac_enable_addr = XXVGE_RXMAC_ENABLE_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD;

    rxmac_enable.value = hclge_read_dev(&hdev->hw, rxmac_enable_addr);
    rxmac_enable.bits.rx_enable = 0;
    hclge_write_mag_dev(&hdev->hw, rxmac_enable_addr, rxmac_enable.value);
}

int mac_get_link_status(struct hclge_plf_dev *hdev)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    u_xxvge_link_status link_status;
    u32 link_status_addr;

    if (test_bit(HCLGE_STATE_DOWN, &hdev->state)) {
        return 0;
    }

    link_status_addr = XXVGE_LINK_STATUS_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD;
    link_status.value = hclge_read_dev(&hdev->hw, link_status_addr);
    if (hdev->hw.mac.mac_mode == XXVGE_MAC_MODE_RGMII) {
        link_status.bits.link_up = 1;
    }
    return link_status.bits.link_up;
}

void mac_link_up_waiting(struct hclge_plf_dev *hdev)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    u_xxvge_link_status link_status;
    u_xxvge_int_status int_status;
    u32 link_status_addr;
    u32 int_status_addr;
    int delay = 1000;

    link_status_addr = XXVGE_LINK_STATUS_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD;
    int_status_addr = XXVGE_INT_STATUS_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD;

    atomic_set(&hdev->hw.mac.int_clear_flag, true);
    while (delay--) {
        int_status.value = hclge_read_dev(&hdev->hw, int_status_addr);
        link_status.value = hclge_read_dev(&hdev->hw, link_status_addr);

        if (int_status.bits.link_up & link_status.bits.link_up) {
            return;
        }
    }
}

STATIC void mac_set_clear_mid(struct hclge_plf_dev *hdev)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    u32 mib_ctr_addr = XXVGE_MIB_CONTROL_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD;
    u_xxvge_mib_control mib_ctr;

    mib_ctr.value = hclge_read_dev(&hdev->hw, mib_ctr_addr);
    mib_ctr.bits.mib_clr = 1;
    hclge_write_mag_dev(&hdev->hw, mib_ctr_addr, mib_ctr.value);
}

void mac_clear_mib_warnning(struct hclge_plf_dev *hdev)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    u32 int_status_addr = XXVGE_INT_STATUS_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD;
    u32 pma_int_status_addr = PMA_XXVGE_INT_STATUS_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD;

    hclge_write_mag_dev(&hdev->hw, int_status_addr, 0xffffffff);
    hclge_write_mag_dev(&hdev->hw, pma_int_status_addr, 0xffffffff);
    mac_set_clear_mid(hdev);
}

void mac_int_handle(struct hclge_plf_dev *hdev)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    u_xxvge_int_status int_status;
    u32 int_status_addr;

    int_status_addr = XXVGE_INT_STATUS_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD;
    int_status.value = hclge_read_dev(&hdev->hw, int_status_addr);

    if (int_status.bits.link_up) {
        /* anti-shake to be implemented */
        mac_cfg_common_link_up_int(hdev, 0);
        hclge_write_mag_dev(&hdev->hw, int_status_addr, int_status.value);
        mac_cfg_common_link_down_int(hdev, 1);
    } else if (int_status.bits.link_down) {
        mac_cfg_common_link_down_int(hdev, 0);
        hclge_write_mag_dev(&hdev->hw, int_status_addr, int_status.value);
        mac_cfg_common_link_up_int(hdev, 1);
    } else {
        /* nothing */
    }

#ifndef DEFINE_HNS_LLT
    hclge_plf_task_schedule(hdev, 0);
#endif
}

STATIC void mac_reset_t(struct hclge_plf_dev *hdev)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    void __iomem *reg_addr;

    reg_addr = hdev->hw.iosub_base + IO_SUBCTRL_SC_NIC_INTF0_RESET_REQ;
    writel((1 << mac->mac_id), reg_addr);
}

STATIC void mac_reset_cfg(struct hclge_plf_dev *hdev)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    void __iomem *reg_addr;

    reg_addr = hdev->hw.iosub_base + IO_SUBCTRL_SC_NIC_INTF0_RESET_REQ;
    writel((1 << (mac->mac_id + 8)), reg_addr); /* cfg_reset shift 8 */
}

STATIC void mac_reset_core(struct hclge_plf_dev *hdev)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    void __iomem *reg_addr;

    reg_addr = hdev->hw.iosub_base + IO_SUBCTRL_SC_NIC_INTF0_RESET_REQ;
    writel((1 << (mac->mac_id + 12)), reg_addr); // core_reset shift 12
}

STATIC void mac_reset_ptp(struct hclge_plf_dev *hdev)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    void __iomem *reg_addr;

    reg_addr = hdev->hw.iosub_base + IO_SUBCTRL_SC_NIC_INTF0_RESET_REQ;
    writel((1 << (mac->mac_id + 20)), reg_addr); // ptp_reset shift 20
}

STATIC void mac_dis_reset_t(struct hclge_plf_dev *hdev)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    void __iomem *reg_addr;

    reg_addr = hdev->hw.iosub_base + IO_SUBCTRL_SC_NIC_INTF0_RESET_DREQ;
    writel((1 << mac->mac_id), reg_addr);
}

STATIC void mac_dis_reset_cfg(struct hclge_plf_dev *hdev)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    void __iomem *reg_addr;

    reg_addr = hdev->hw.iosub_base + IO_SUBCTRL_SC_NIC_INTF0_RESET_DREQ;
    writel((1 << (mac->mac_id + 8)), reg_addr); // cfg_reset shift 8
}

STATIC void mac_dis_reset_core(struct hclge_plf_dev *hdev)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    void __iomem *reg_addr;

    reg_addr = hdev->hw.iosub_base + IO_SUBCTRL_SC_NIC_INTF0_RESET_DREQ;
    writel((1 << (mac->mac_id + 12)), reg_addr); // core_reset shift 12
}

STATIC void mac_dis_reset_ptp(struct hclge_plf_dev *hdev)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    void __iomem *reg_addr;

    reg_addr = hdev->hw.iosub_base + IO_SUBCTRL_SC_NIC_INTF0_RESET_DREQ;
    writel((1 << (mac->mac_id + 20)), reg_addr); // ptp_reset shift 20
}

STATIC int mac_cfg(struct hclge_plf_dev *hdev, int port)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    int ret;

    /* wrok mode : SGMII/RGMII 0 */
    hclge_write_mag_dev(&hdev->hw, (XXVGE_WORK_MODE_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD), 0x0);

    ret = mac_cfg_port(hdev);
    if (ret != 0) {
        dev_err(&hdev->pdev->dev, "mac_cfg_port failed(%d)\n", ret);
        return ret;
    }

    ret = mac_cfg_freq(hdev);
    if (ret != 0) {
        dev_err(&hdev->pdev->dev, "mac_cfg_freq failed(%d)\n", ret);
        return ret;
    }

    mac_cfg_txmac_rf_tx_en(hdev);
    mac_cfg_txmac_bit_delay(hdev, 0xcccd);
    mac_cfg_txmac_replicate(hdev, 0, 1);

    mac_cfg_rxmac_bit_delay(hdev, 0xcccd);
    mac_cfg_rxmac_replicate(hdev, 0, 1);
    mac_disable_1588(hdev);
    mac_enable_ipg_check(hdev);
    mac_cfg_rxmac_rx_ipg(hdev);

    /* sync mode 2 */
    mac_cfg_rxpma_enable(hdev, 2);

    ret = mac_cfg_calendar(hdev, mac->speed);
    if (ret) {
        dev_err(&hdev->pdev->dev, "mac(%d) cfg calendar failed ret=%d\n", mac->mac_id, ret);
        return ret;
    }

    /* PTP 500MHz: 1, 1G: 0 */
    hclge_write_mag_dev(&hdev->hw, (XXVGE_PTP_CLK_FREQ_SEL + mac->mac_id * XXVGE_MAC_REG_PERIOD), 0);

    mac_cfg_width(hdev, XXVGE_PMA_WIDTH_10BIT);
    mac_cfg_reverse(hdev, 0);

    /* Extra configure for mag/comag difference between st_cfg and cfg document */
    /* XXVGE_RTC_CORE_PERIOD, 1588 */
    hclge_write_mag_dev(&hdev->hw,
        (HCLGE_MAG_REG_BASE + XXVGE_PORT_REG_BASE + 0xddc + mac->mac_id * XXVGE_MAC_REG_PERIOD),
        0x200102);
#ifdef CONFIG_FPGA_COMAG
    /* comag disable rx pause */
    hclge_write_dev(
        &hdev->hw, (HCLGE_MAG_REG_BASE + XXVGE_PORT_REG_BASE + HCLGE_COMAG_REG_BASE_OFFSET +\
        0x470 + mac->mac_id * XXVGE_MAC_REG_PERIOD), 0x1);
#endif
    /* MAX_PKT_SIZE, 9728bytes */
    hclge_write_mag_dev(&hdev->hw, (HCLGE_MAG_REG_BASE + XXVGE_PORT_REG_BASE +\
        0x444 + mac->mac_id * XXVGE_MAC_REG_PERIOD), 0x2600);
    /* MIN_PKT_SIZE, 33bytes */
    hclge_write_mag_dev(&hdev->hw, (XXVGE_PORT_REG_BASE + 0x440 + mac->mac_id * XXVGE_MAC_REG_PERIOD), 0x21);

#ifdef CONFIG_PLATFORM_EMU // 确保解复位前的寄存器写完成
    ret = hclge_read_dev(&hdev->hw, XXVGE_INT_ENABLE_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD);
#endif

    return 0;
}

STATIC int mac_init(struct hclge_plf_dev *hdev, int port)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    int ret;

    mac_reset_t(hdev);
    mac_reset_cfg(hdev);
    mac_reset_core(hdev);
    mac_reset_ptp(hdev);

    mac_dis_reset_t(hdev);
    mac_dis_reset_cfg(hdev);
    mac_dis_reset_ptp(hdev);

    /* Auto-negotiation disable: 1 */
    mac_cfg_common_link_sync(hdev, 1);
    mac_cfg_pcs_an_autoneg_en(hdev, 0);

    if (hdev->hw.mac.support_autoneg) {
        hdev->hw.mac.autoneg = true;
        mac_sgmii_cfg_autoneg(hdev, hdev->hw.mac.autoneg);
    }

    ret = mac_cfg(hdev, port);
    if (ret) {
        dev_err(&hdev->pdev->dev, "mac(%d) cfg err(%d)\n", mac->mac_id, ret);
        return ret;
    }

    mac_dis_reset_core(hdev);

    return 0;
}

void mag_disable_igu_egu_txrx(struct hclge_plf_dev *hdev)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    u_igu_mac_en_cfg igu_mac_en_cfg;
    u32 igu_mac_en_cfg_addr;

    /* disable IGU EGU txrx */
    igu_mac_en_cfg_addr = IGU_EGU_CFG_IGU_MAC_EN_CFG_0_REG + mac->mac_id * 0x200;
    igu_mac_en_cfg.value = hclge_read_dev(&hdev->hw, igu_mac_en_cfg_addr);
    igu_mac_en_cfg.bits.cfg_mac_rx_en = 0;
    igu_mac_en_cfg.bits.cfg_mac_tx_en = 0;
    hclge_write_dev(&hdev->hw, igu_mac_en_cfg_addr, igu_mac_en_cfg.value);
    igu_mac_en_cfg.value = hclge_read_dev(&hdev->hw, igu_mac_en_cfg_addr);
}

void mag_enable_igu_egu_txrx(struct hclge_plf_dev *hdev)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    u_igu_mac_en_cfg igu_mac_en_cfg;
    u32 igu_mac_en_cfg_addr;

    /* enable IGU EGU txrx */
    igu_mac_en_cfg_addr = IGU_EGU_CFG_IGU_MAC_EN_CFG_0_REG + mac->mac_id * 0x200;
    igu_mac_en_cfg.value = hclge_read_dev(&hdev->hw, igu_mac_en_cfg_addr);
    igu_mac_en_cfg.bits.cfg_mac_rx_en = 1;
    igu_mac_en_cfg.bits.cfg_mac_tx_en = 1;
    hclge_write_dev(&hdev->hw, igu_mac_en_cfg_addr, igu_mac_en_cfg.value);
}

STATIC void mag_disable_igu_egu_rx(struct hclge_plf_dev *hdev)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    u_igu_mac_en_cfg igu_mac_en_cfg;
    u32 igu_mac_en_cfg_addr;

    /* disable IGU EGU txrx */
    igu_mac_en_cfg_addr = IGU_EGU_CFG_IGU_MAC_EN_CFG_0_REG + mac->mac_id * 0x200;
    igu_mac_en_cfg.value = hclge_read_dev(&hdev->hw, igu_mac_en_cfg_addr);
    igu_mac_en_cfg.bits.cfg_mac_rx_en = 0;
    hclge_write_dev(&hdev->hw, igu_mac_en_cfg_addr, igu_mac_en_cfg.value);
    igu_mac_en_cfg.value = hclge_read_dev(&hdev->hw, igu_mac_en_cfg_addr);
}

STATIC void mag_enable_igu_egu_rx(struct hclge_plf_dev *hdev)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    u_igu_mac_en_cfg igu_mac_en_cfg;
    u32 igu_mac_en_cfg_addr;

    /* enable IGU EGU txrx */
    igu_mac_en_cfg_addr = IGU_EGU_CFG_IGU_MAC_EN_CFG_0_REG + mac->mac_id * 0x200;
    igu_mac_en_cfg.value = hclge_read_dev(&hdev->hw, igu_mac_en_cfg_addr);
    igu_mac_en_cfg.bits.cfg_mac_rx_en = 1;
    hclge_write_dev(&hdev->hw, igu_mac_en_cfg_addr, igu_mac_en_cfg.value);
}

STATIC int mag_set_igu_egu_speed(struct hclge_plf_dev *hdev)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    u_igu_mac_mode_cfg igu_mac_mode_cfg;
    u32 igu_mac_mode_cfg_addr;

    /* cfg IGU_EGU speed */
    igu_mac_mode_cfg_addr = IGU_EGU_CFG_IGU_MAC_MODE_CFG_0_REG + mac->mac_id * 0x200;
    igu_mac_mode_cfg.value = hclge_read_dev(&hdev->hw, igu_mac_mode_cfg_addr);
    if (mac->mac_mode == XXVGE_MAC_MODE_RGMII) {
        igu_mac_mode_cfg.bits.cfg_mac_rate = 3;  // RGMII mode 3
        hclge_write_dev(&hdev->hw, igu_mac_mode_cfg_addr, igu_mac_mode_cfg.value);
    } else if (mac->mac_mode < XXVGE_MAC_MODE_MAX) {
        if (mac->speed == XXVGE_MAC_SPEED_1G || mac->speed == XXVGE_MAC_SPEED_100M ||
            mac->speed == XXVGE_MAC_SPEED_10M) {
            igu_mac_mode_cfg.bits.cfg_mac_rate = 0;
            hclge_write_dev(&hdev->hw, igu_mac_mode_cfg_addr, igu_mac_mode_cfg.value);
        } else if (mac->speed == XXVGE_MAC_SPEED_2_5G) {
            igu_mac_mode_cfg.bits.cfg_mac_rate = 1;
            hclge_write_dev(&hdev->hw, igu_mac_mode_cfg_addr, igu_mac_mode_cfg.value);
        } else {
            dev_err(&hdev->pdev->dev, "not support speed(%d)\n", mac->speed);
            return -1;
        }
    } else {
        dev_err(&hdev->pdev->dev, "not support mac mode(%d)\n", mac->mac_mode);
        return -1;
    }

    return 0;
}

int mag_set_port_enable(struct hclge_plf_dev *hdev)
{
    /* mac enable */
    mac_set_mac_enable(hdev);
    /* enable IGU EGU txrx */
    mag_enable_igu_egu_txrx(hdev);

    /* RGMII 不会linkup，手动触发linkup中断 */
    if (hdev->hw.mac.mac_mode == XXVGE_MAC_MODE_RGMII) {
        hclge_write_mag_dev(&hdev->hw,
            (HCLGE_MAG_REG_BASE + XXVGE_PORT_REG_BASE + 0x408 + hdev->hw.mac.mac_id * XXVGE_MAC_REG_PERIOD), 0x10);
    }

    return 0;
}

int mag_set_port_disable(struct hclge_plf_dev *hdev)
{
    /* mac disable */
    mac_set_mac_disable(hdev);
    /* disable IGU EGU txrx */
    mag_disable_igu_egu_txrx(hdev);

    return 0;
}

STATIC int mag_check_egu_buffer_empty(struct hclge_plf_dev *hdev)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    u_lge_egu_afifo_status lge_egu_afifo_status;
    u_igu_egu_fifo_status igu_egu_fifo_status;
    u_igu_mac_en_cfg igu_mac_en_cfg;
    u32 igu_egu_fifo_status_addr;
    u32 lge_egu_afifo_status_addr;
    u32 igu_mac_en_cfg_addr;
    u32 time_out = 1000;

    igu_egu_fifo_status_addr = IGU_EGU_CFG_IGU_EGU_FIFO_STATUS_0_REG + mac->mac_id * 0x300;
    lge_egu_afifo_status_addr = XXVGE_COMMON_LGE_EGU_AFIFO_STATUS_REG + mac->mac_id * 0x1000;
    igu_mac_en_cfg_addr = IGU_EGU_CFG_IGU_MAC_EN_CFG_0_REG + mac->mac_id * 0x200;

    while (time_out) {
        igu_egu_fifo_status.value = hclge_read_dev(&hdev->hw, igu_egu_fifo_status_addr);
        lge_egu_afifo_status.value = hclge_read_dev(&hdev->hw, lge_egu_afifo_status_addr);
        igu_mac_en_cfg.value = hclge_read_dev(&hdev->hw, igu_mac_en_cfg_addr);
        if ((igu_egu_fifo_status.bits.tx_buf_sta_dfx == 0x80) &&
            (lge_egu_afifo_status.bits.egu_lge_afifo_sts == 0x1) &&
            (igu_mac_en_cfg.bits.mac_rx_en_inner == 0)) {
                return 0;
        }
        udelay(10); // 10 usec
        time_out--;
    }

    return -1;
}

STATIC void mac_switch_speed_pre(struct hclge_plf_dev *hdev)
{
    u32 addr = SSU_REG_BASE + SSU_TX_OQ_SCHEDULE_EN;
    u32 data;

    data = hclge_read_dev(&hdev->hw, addr);
    data &= (~(1 << hdev->id));
    mac_set_mac_disable_rx(hdev);
    mag_disable_igu_egu_rx(hdev);
    hclge_write_dev(&hdev->hw, addr, data);
    udelay(1000); // 1000 usec
}

STATIC void mac_switch_speed_done(struct hclge_plf_dev *hdev)
{
    u32 addr = SSU_REG_BASE + SSU_TX_OQ_SCHEDULE_EN;
    u32 data;

    data = hclge_read_dev(&hdev->hw, addr);
    data |= (1 << hdev->id);
    mac_set_mac_enable_rx(hdev);
    mag_enable_igu_egu_rx(hdev);
    hclge_write_dev(&hdev->hw, addr, data);
}

#if defined(CONFIG_PLATFORM_ASIC) && defined(CONFIG_PLATFORM_SUPPORT_SERDES)
HilinkRate mac_speed_to_serdes_rate(struct hclge_plf_dev *hdev)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;

    if (mac->speed == XXVGE_MAC_SPEED_1G || mac->speed == XXVGE_MAC_SPEED_100M ||
        mac->speed == XXVGE_MAC_SPEED_10M) {
        return HILINK_XGE_1_25G;
    } else if (mac->speed == XXVGE_MAC_SPEED_2_5G) {
        return HILINK_XGE_3_125G;
    } else {
        dev_err(&hdev->pdev->dev, "not support speed(%d)\n", mac->speed);
        return HILINK_XGE_1_25G;
    }
}
#endif

int xxvge_mac_cfg_speed(struct hclge_plf_dev *hdev)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    int ret;

    mac_switch_speed_pre(hdev);

    ret = mag_check_egu_buffer_empty(hdev);
    if (ret) {
        dev_err(&hdev->pdev->dev, "mac(%d) check empty fail ret=%d\n", mac->mac_id, ret);
        /* SSU_TX_OQ_SCHEDULE_EN */
        writel(1, hdev->hw.io_base + SSU_REG_BASE + SSU_TX_OQ_SCHEDULE_EN);
        mag_set_port_enable(hdev);
        return ret;
    }

    ret = mag_set_igu_egu_speed(hdev);
    if (ret) {
        dev_err(&hdev->pdev->dev, "(hdev) ret=%d\n", ret);
        return ret;
    }

#if defined(CONFIG_PLATFORM_ASIC) && defined(CONFIG_PLATFORM_SUPPORT_SERDES)
    if (mac->mac_mode < XXVGE_MAC_MODE_MAX && mac->mac_mode != XXVGE_MAC_MODE_RGMII) {
        /* serdes switch rate api */
        ret = HILINK_DsDataRateSwitch(1, mac->ds_index, mac_speed_to_serdes_rate(hdev), 1);
        if (ret) {
            dev_err(&hdev->pdev->dev, "serdes rate switch fail ret=%d\n", ret);
            return ret;
        }
    }
#endif

    mac_reset_core(hdev);
    ret = mac_cfg(hdev, mac->mac_id);
    if (ret) {
        dev_err(&hdev->pdev->dev, "mac(%d) init fail ret=%d\n", mac->mac_id, ret);
        return ret;
    }

    mac_dis_reset_core(hdev);
    /* check link status */
    mac_link_up_waiting(hdev);
    mac_switch_speed_done(hdev);

    return 0;
}

#ifdef CONFIG_PLATFORM_ASIC
STATIC void mag_set_rgmii_delay(struct hclge_plf_dev *hdev)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    u32 rgmii_intf_delay_addr;

    rgmii_intf_delay_addr = MAG_COMMON_CFG_RGMII_INTF_DELAY_0_REG + mac->mac_id * 0x4;

    if (mac->mac_mode == XXVGE_MAC_MODE_RGMII) {
        hclge_write_dev(&hdev->hw, rgmii_intf_delay_addr, 1);
    }
}
#endif

int xxvge_mag_init(struct hclge_plf_dev *hdev)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    int ret;

    mag_disable_igu_egu_txrx(hdev);

    ret = mag_set_igu_egu_speed(hdev);
    if (ret) {
        dev_err(&hdev->pdev->dev, "(hdev) ret=%d\n", ret);
        return ret;
    }

    /* only FPGA RGMII */
#ifdef CONFIG_PLATFORM_FPGA
    if (mac->mac_mode == XXVGE_MAC_MODE_RGMII) {
        hclge_write_dev(&hdev->hw, (MAG_COMMON_CFG_RGMII_INTF_MUX_0_REG + mac->mac_id * 0x4), 1);
    }
#endif

#ifdef CONFIG_PLATFORM_ASIC
    mag_set_rgmii_delay(hdev);
#endif

    /* mac init */
    ret = mac_init(hdev, mac->mac_id);
    if (ret) {
        dev_err(&hdev->pdev->dev, "mac(%d) init fail ret=%d\n", mac->mac_id, ret);
        return ret;
    }

    return 0;
}

void xge_mac_set_mtu(struct hclge_plf_dev *hdev, int new_mps)
{
    struct hclge_plf_mac *mac = &hdev->hw.mac;
    u32 mac_max_pkt_size_addr = XXVGE_MAC_MAX_PKT_SIZE_REG + mac->mac_id * XXVGE_MAC_REG_PERIOD;

    hclge_write_dev(&hdev->hw, mac_max_pkt_size_addr, new_mps);
}
