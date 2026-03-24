/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
 * Description: platform_phy
 * Author: huawei
 * Create: 2021-12-28
 */

#ifndef PLATFORM_MAC_H
#define PLATFORM_MAC_H

#include <linux/device.h>
#include "c_union_define_igu_egu_cfg.h"
#include "c_union_define_mag_common_cfg.h"
#include "c_union_define_mag_xxvge_common.h"
#include "c_union_define_rtc_common.h"
#include "c_union_define_xxvge_common.h"
#include "c_union_define_xxvge_global.h"
#include "c_union_define_xxvge_pma_global.h"
#include "c_union_define_xxvge_rxmac.h"
#include "c_union_define_xxvge_rxpma.h"
#include "c_union_define_xxvge_txmac.h"
#include "c_union_define_xxvge_txpma.h"
#include "igu_egu_cfg_reg_offset.h"
#include "mag_common_cfg_reg_offset.h"
#include "rtc_common_reg_offset.h"
#include "xxvge_common_reg_offset.h"
#ifdef CONFIG_PLATFORM_ASIC
#include "sdk_hilink_pub.h"
#endif
#include "hclge_plf_main.h"

#define HCLGE_MAG_REG_BASE 0x200000

#ifdef CONFIG_PLATFORM_FPGA
#define CONFIG_FPGA_COMAG
#endif

/* For FPGA comag cfg, COMAG_REG_BASE 0x300000 */
#define HCLGE_COMAG_REG_BASE_OFFSET 0x100000

#define IGU_EGU_REG_BASE 0
#define IGU_MAC_EN_CFG_REG (HCLGE_MAG_REG_BASE + IGU_EGU_REG_BASE + 0x500)
#define IGU_MAC_MODE_CFG_REG (HCLGE_MAG_REG_BASE + IGU_EGU_REG_BASE + 0x504)

#define RTC_REG_BASE 0x2000

#define XXVGE_COMMON_REG_BASE 0x40000

#define MAG_COMMON_REG_BASE 0x60000

#define XXVGE_PORT_REG_BASE 0x80000
#define XXVGE_MAC_REG_PERIOD 0x10000

#define HCLGE_XXVGE_COMMON_0_BASE (HCLGE_MAG_REG_BASE + XXVGE_PORT_REG_BASE)

#define HCLGE_XXVGE_COMMON_0_INT_EN_REG_ADDR 0x0404
#define HCLGE_MISC_MAC_VECTOR_REG_BASE (HCLGE_XXVGE_COMMON_0_BASE + HCLGE_XXVGE_COMMON_0_INT_EN_REG_ADDR)

#define HCLGE_XXVGE_COMMON_0_LINK_STATUS_REG_ADDR 0x0424
#define HCLGE_LINK_STATUS_REG_BASE_ADDR (HCLGE_XXVGE_COMMON_0_BASE + HCLGE_XXVGE_COMMON_0_LINK_STATUS_REG_ADDR)

/* GLOBAL */
#define XXVGE_GLOBAL_REG_BASE 0
#define XXVGE_WORK_FREQ_L_REG (HCLGE_MAG_REG_BASE + XXVGE_PORT_REG_BASE + 0xd94)
#define XXVGE_WORK_FREQ_H_REG (HCLGE_MAG_REG_BASE + XXVGE_PORT_REG_BASE + 0xd98)
#define XXVGE_WORK_MODE_REG (HCLGE_MAG_REG_BASE + XXVGE_PORT_REG_BASE + 0xd9c)

#define XXVGE_TX_CALEN_DEPTH_REG (HCLGE_MAG_REG_BASE + XXVGE_PORT_REG_BASE + 0xdb8)
#define XXVGE_TX_CALENDAR_TABLE_REG (HCLGE_MAG_REG_BASE + XXVGE_PORT_REG_BASE + 0xdbc)
#define XXVGE_PMA_CALENDAR_TABLE_REG (HCLGE_MAG_REG_BASE + XXVGE_PORT_REG_BASE + 0xf8c)

#define XXVGE_PTP_CLK_FREQ_SEL (HCLGE_MAG_REG_BASE + XXVGE_PORT_REG_BASE + 0xdd0)

#define XXVGE_TXMAC_BASE_FREQ_L_REG (HCLGE_MAG_REG_BASE + XXVGE_PORT_REG_BASE + 0x65c)
#define XXVGE_TXMAC_BASE_FREQ_H_REG (HCLGE_MAG_REG_BASE + XXVGE_PORT_REG_BASE + 0x660)
#define XXVGE_TXMAC_LF_RF_CONTROL_REG (HCLGE_MAG_REG_BASE + XXVGE_PORT_REG_BASE + 0x64c)
#define XXVGE_TXMAC_LANE_1UI_DLY_REG (HCLGE_MAG_REG_BASE + XXVGE_PORT_REG_BASE + 0x67c)
#define XXVGE_TXMAC_REPLICATE_REG (HCLGE_MAG_REG_BASE + XXVGE_PORT_REG_BASE + 0x678)
#define XXVGE_TXMAC_ENABLE_REG (HCLGE_MAG_REG_BASE + XXVGE_PORT_REG_BASE + 0x600)

#define XXVGE_PORT_MODE_REG (HCLGE_MAG_REG_BASE + XXVGE_PORT_REG_BASE + 0x40c)
#define XXVGE_PORT_SPEED_REG (HCLGE_MAG_REG_BASE + XXVGE_PORT_REG_BASE + 0x410)

#define XXVGE_TXPMA_CONTROL_REG (HCLGE_MAG_REG_BASE + XXVGE_PORT_REG_BASE + 0xe40)
#define XXVGE_RXPMA_CONTROL_REG (HCLGE_MAG_REG_BASE + XXVGE_PORT_REG_BASE + 0xe84)
#define XXVGE_RXPMA_ENABLE_REG (HCLGE_MAG_REG_BASE + XXVGE_PORT_REG_BASE + 0xe80)

#define XXVGE_INT_STATUS_REG (HCLGE_MAG_REG_BASE + XXVGE_PORT_REG_BASE + 0x400)
#define XXVGE_INT_ENABLE_REG (HCLGE_MAG_REG_BASE + XXVGE_PORT_REG_BASE + 0x404)
#define XXVGE_AN_CFG_SEL_REG (HCLGE_MAG_REG_BASE + XXVGE_PORT_REG_BASE + 0x418)
#define XXVGE_LINK_STATUS_REG (HCLGE_MAG_REG_BASE + XXVGE_PORT_REG_BASE + 0x424)
#define XXVGE_MAC_MIN_PKT_SIZE_REG (HCLGE_MAG_REG_BASE + XXVGE_PORT_REG_BASE + 0x440)
#define XXVGE_MAC_MAX_PKT_SIZE_REG (HCLGE_MAG_REG_BASE + XXVGE_PORT_REG_BASE + 0x444)
#define XXVGE_LINK_CONTROL_REG (HCLGE_MAG_REG_BASE + XXVGE_PORT_REG_BASE + 0x420)
#define XXVGE_MIB_CONTROL_REG (HCLGE_MAG_REG_BASE + XXVGE_PORT_REG_BASE + 0x478)

#define XXVGE_RXMAC_ENABLE_REG (HCLGE_MAG_REG_BASE + XXVGE_PORT_REG_BASE + 0x7d0)
#define XXVGE_RXMAC_CONTROL_REG (HCLGE_MAG_REG_BASE + XXVGE_PORT_REG_BASE + 0x7d4)
#define XXVGE_RXMAC_CONTROL1_REG (HCLGE_MAG_REG_BASE + XXVGE_PORT_REG_BASE + 0x7d8)
#define XXVGE_RXMAC_LANE_1UI_DLY_REG (HCLGE_MAG_REG_BASE + XXVGE_PORT_REG_BASE + 0x814)
#define XXVGE_RXMAC_REPLICATE_REG (HCLGE_MAG_REG_BASE + XXVGE_PORT_REG_BASE + 0x7f0)

#define XXVGE_PCS_AN_CTRL_REG (HCLGE_MAG_REG_BASE + XXVGE_PORT_REG_BASE + 0x9C0)
#define XXVGE_PCS_AN_LINK_TIME_REG (HCLGE_MAG_REG_BASE + XXVGE_PORT_REG_BASE + 0x9D4)
#define XXVGE_PCS_AN_BASE_PAGE (HCLGE_MAG_REG_BASE + XXVGE_PORT_REG_BASE + 0x9C8)

#define PMA_XXVGE_INT_STATUS_REG (HCLGE_MAG_REG_BASE + XXVGE_PORT_REG_BASE + 0xF00)

#define XXVGE_MIB_BASE_REG (HCLGE_MAG_REG_BASE + XXVGE_PORT_REG_BASE)

#define XXVGE_MAC_PAUSE_PFC_CTRL (HCLGE_MAG_REG_BASE + XXVGE_PORT_REG_BASE + 0x470)
#define XXVGE_MAC_PAUSE_PFC_CTRL_REG(mac_id) (XXVGE_MAC_PAUSE_PFC_CTRL + ((mac_id) << 16))

/* 250M 0x3A35294400 实际芯片core时钟 */
#define PLATFORM_MAC_WORK_FREQ_H    0x3A
#define PLATFORM_MAC_WORK_FREQ_L    0x35294400

/* 1.25M 0x4A817C80 */
#define PLATFORM_MAC_10M_RGMII_BASE_FREQ_H      0
#define PLATFORM_MAC_10M_RGMII_BASE_FREQ_L      0x4A817C80

/* 12.5M 0x2E90EDD00 */
#define PLATFORM_MAC_100M_RGMII_BASE_FREQ_H     0x2
#define PLATFORM_MAC_100M_RGMII_BASE_FREQ_L     0xE90EDD00

/* 125M 0x1D1A94A200 */
#define PLATFORM_MAC_1000M_RGMII_BASE_FREQ_H    0x1D
#define PLATFORM_MAC_1000M_RGMII_BASE_FREQ_L    0x1A94A200

/* 62.5M 0xE8D4A5100 */
#define PLATFORM_MAC_10M_SGMII_BASE_FREQ_H      0xE
#define PLATFORM_MAC_10M_SGMII_BASE_FREQ_L      0x8D4A5100

/* 62.5M 0xE8D4A5100 */
#define PLATFORM_MAC_100M_SGMII_BASE_FREQ_H     0xE
#define PLATFORM_MAC_100M_SGMII_BASE_FREQ_L     0x8D4A5100

/* 62.5M 0xE8D4A5100 */
#define PLATFORM_MAC_1000M_SGMII_BASE_FREQ_H    0xE
#define PLATFORM_MAC_1000M_SGMII_BASE_FREQ_L    0x8D4A5100

/* 156.25M 0x246139CA80 */
#define PLATFORM_MAC_2500M_SGMII_BASE_FREQ_H    0x24
#define PLATFORM_MAC_2500M_SGMII_BASE_FREQ_L    0x6139CA80

enum XXVGE_MAC_MODE {
    XXVGE_MAC_MODE_SGMII = 0,
    XXVGE_MAC_MODE_RGMII = 1,
    XXVGE_MAC_MODE_SFP_1000BASE = 2,
    XXVGE_MAC_MODE_SFP_2500BASE = 3,
    XXVGE_MAC_MODE_MAX
};

enum XXVGE_MAC_SPEED {
    XXVGE_MAC_SPEED_UNKNOWN = 0, /* unknown */
    XXVGE_MAC_SPEED_10M = 10,    /* 10 Mbps */
    XXVGE_MAC_SPEED_100M = 100,  /* 100 Mbps */
    XXVGE_MAC_SPEED_1G = 1000,   /* 1000 Mbps = 1 Gbps */
    XXVGE_MAC_SPEED_2_5G = 2500, /* 2500 Mbps = 2.5 Gbps */
};

enum XXVGE_PMA_WIDTH {
    XXVGE_PMA_WIDTH_10BIT = 0,
    XXVGE_PMA_WIDTH_20BIT,
    XXVGE_PMA_WIDTH_32BIT,
    XXVGE_PMA_WIDTH_40BIT,
};

#ifdef CONFIG_FPGA_COMAG
#define hclge_write_mag_dev(a, reg, value)                                       \
    do {                                                                         \
        hclge_write_reg((a)->io_base, reg, value);                               \
        hclge_write_reg((a)->io_base, reg + HCLGE_COMAG_REG_BASE_OFFSET, value); \
    } while (0)
#else
#define hclge_write_mag_dev(a, reg, value) hclge_write_reg((a)->io_base, reg, value)
#endif

void mag_disable_igu_egu_txrx(struct hclge_plf_dev *hdev);
void mag_enable_igu_egu_txrx(struct hclge_plf_dev *hdev);
void mac_set_mac_disable(struct hclge_plf_dev *hdev);
void mac_set_mac_enable(struct hclge_plf_dev *hdev);
void mac_pause_en_cfg(struct hclge_plf_dev *hdev, u32 tx, u32 rx);
int xxvge_mac_cfg_speed(struct hclge_plf_dev *hdev);
void mac_int_handle(struct hclge_plf_dev *hdev);
void mac_sgmii_cfg_autoneg(struct hclge_plf_dev *hdev, bool enable);
void mac_cfg_common_link_down_int(struct hclge_plf_dev *hdev, u32 enable);
void mac_cfg_common_link_up_int(struct hclge_plf_dev *hdev, u32 enable);
void mac_clear_mib_warnning(struct hclge_plf_dev *hdev);
void mac_link_up_waiting(struct hclge_plf_dev *hdev);
#if defined(CONFIG_PLATFORM_ASIC) && defined(CONFIG_PLATFORM_SUPPORT_SERDES)
HilinkRate mac_speed_to_serdes_rate(struct hclge_plf_dev *hdev);
#endif
void mac_set_mac_disable_rx(struct hclge_plf_dev *hdev);
void xge_mac_set_mtu(struct hclge_plf_dev *hdev, int new_mps);
#endif // PLATFORM_MAC_H
