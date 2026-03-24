/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2023. All rights reserved.
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

#ifndef _CAN_DRV_MTTCAN__
#define _CAN_DRV_MTTCAN__

#include "can_drv_common.h"

/* GFC: Global Filter Configuration */
#define GFC_ANFS_SHIFT    4
#define GFC_ANFS_MASK    GENMASK(5, 4) /* Accept Non-matching Frames Standard */

#define GFC_ANFE_SHIFT    2
#define GFC_ANFE_MASK    GENMASK(3, 2) /* Accept Non-matching Frames Extended */

#define GFC_RRFS_BIT    BIT(1) /* Reject Remote Frames Standard */
#define GFC_RRFE_BIT    BIT(0) /* Reject Remote Frames Extended */

#define ANF_RX_FIFO_0    0x0 /* Accept in Rx FIFO 0 */
#define ANF_RX_FIFO_1    0x1 /* Accept in Rx FIFO 1 */
#define ANF_REJECT       0x3 /* Reject Non-matching Frames */

#define RRFE_FILTER    0x0 /* Filter remote frames with 29-bit extended IDs */
#define RRFE_REJECT    0x1 /* Reject all remote frames with 29-bit extended IDs */

#define RRFS_FILTER    0x0 /* Filter remote frames with 29-bit extended IDs */
#define RRFS_REJECT    0x2 /* Reject all remote frames with 29-bit extended IDs */

/* SIDFC: Standard ID Filter Configuration */
#define SIDFC_LSS_SHIFT        16
#define SIDFC_LSS_MASK        GENMASK(23, 16)

#define SIDFC_FLSSA_SHIFT    2
#define SIDFC_FLSSA_MASK    GENMASK(15, 2)

/* XIDFC: Extended ID Filter Configuration */
#define XIDFC_LES_SHIFT        16
#define XIDFC_LES_MASK        GENMASK(22, 16)

#define XIDFC_FLESA_SHIFT    2
#define XIDFC_FLESA_MASK    GENMASK(15, 2)

/* XIDAM: Extended ID AND Mask */
#define XIDAM_EIDM_SHIFT    0
#define XIDAM_EIDM_MASK        GENMASK(28, 0)

/* HPMS: High Priority Message Status */
#define HP_MSG_LOST        1
#define HP_MSG_STORE_FIFO0    2
#define HP_MSG_STORE_FIFO1    3

#define HPMS_FLST_BIT        BIT(15)

#define HPMS_FIDX_SHIFT        8
#define HPMS_FIDX_MASK        GENMASK(14, 8)

#define HPMS_MSI_SHIFT        6
#define HPMS_MSI_MASK        GENMASK(7, 6)

#define HPMS_BIDX_SHIFT        0
#define HPMS_BIDX_MASK        GENMASK(5, 0)

/* RXF0C: Rx FIFO 0 Configuration */
#define RXF0C_F0OM_SHIFT      31
#define RXF0C_F0OM_BIT        BIT(31)

#define RXF0C_F0WM_SHIFT    24
#define RXF0C_F0WM_MASK        GENMASK(30, 24)

#define RXF0C_F0S_SHIFT        16
#define RXF0C_F0S_MASK        GENMASK(22, 16)

#define RXF0C_F0SA_SHIFT    2
#define RXF0C_F0SA_MASK        GENMASK(15, 2)

/* RXF0S: Rx FIFO 0 Status */
#define RXF0S_RF0L_BIT        BIT(25)
#define RXF0S_F0F_BIT        BIT(24)

#define RXF0S_F0PI_SHIFT    16
#define RXF0S_F0PI_MASK        GENMASK(21, 16)

#define RXF0S_F0GI_SHIFT    8
#define RXF0S_F0GI_MASK        GENMASK(13, 8)

#define RXF0S_F0FL_SHIFT    0
#define RXF0S_F0FL_MASK        GENMASK(6, 0)

/* RXF0A: Rx FIFO 0 Acknowledge */
#define RXF0A_F0AI_SHIFT    0
#define RXF0A_F0AI_MASK        GENMASK(5, 0)

/* RXBC: Rx Buffer Configuration */
#define RXBC_RBSA_SHIFT        2
#define RXBC_RBSA_MASK        GENMASK(15, 2)

/* RXF1C: Rx FIFO 1 Configuration */
#define RXF1C_F0OM_SHIFT      31
#define RXF1C_F0OM_BIT        BIT(31)

#define RXF1C_F1WM_SHIFT    24
#define RXF1C_F1WM_MASK        GENMASK(30, 24)

#define RXF1C_F1S_SHIFT        16
#define RXF1C_F1S_MASK        GENMASK(22, 16)

#define RXF1C_F1SA_SHIFT    2
#define RXF1C_F1SA_MASK        GENMASK(15, 2)

/* RXF1S: Rx FIFO 1 Status */
#define RXF1S_DMS_SHIFT        30
#define RXF1S_DMS_MASK        GENMASK(31, 30)

#define RXF1S_RF1L_BIT        BIT(25)
#define RXF1S_F1F_BIT        BIT(24)

#define RXF1S_F1PI_SHIFT    16
#define RXF1S_F1PI_MASK        GENMASK(21, 16)

#define RXF1S_F1GI_SHIFT    8
#define RXF1S_F1GI_MASK        GENMASK(13, 8)

#define RXF1S_F1FL_SHIFT    0
#define RXF1S_F1FL_MASK        GENMASK(6, 0)

/* RXF1A: Rx FIFO 1 Acknowledge */
#define RXF1A_F1AI_SHIFT    0
#define RXF1A_F1AI_MASK        GENMASK(5, 0)

/* RXESC: Rx Buffer / FIFO Element Size Configuration */
#define RXESC_RBDS_SHIFT    8
#define RXESC_RBDS_MASK        GENMASK(10, 8)

#define RXESC_F1DS_SHIFT    4
#define RXESC_F1DS_MASK        GENMASK(6, 4)

#define RXESC_F0DS_SHIFT    0
#define RXESC_F0DS_MASK        GENMASK(2, 0)

/* TXBC: Tx Buffer Configuration */
#define TXBC_TFQM_SHIFT      30
#define TXBC_TFQM_BIT        BIT(30)

#define TXBC_TFQS_SHIFT        24
#define TXBC_TFQS_MASK        GENMASK(29, 24)

#define TXBC_NDTB_SHIFT        16
#define TXBC_NDTB_MASK        GENMASK(21, 16)

#define TXBC_TBSA_SHIFT        2
#define TXBC_TBSA_MASK        GENMASK(15, 2)

/* TXFQS: Tx FIFO/Queue Status */
#define TXFQS_TFQF_BIT        BIT(21)

#define TXFQS_TFQPI_SHIFT    16
#define TXFQS_TFQPI_MASK    GENMASK(20, 16)

#define TXFQS_TFGI_SHIFT    8
#define TXFQS_TFGI_MASK        GENMASK(12, 8)

#define TXFQS_TFFL_SHIFT    0
#define TXFQS_TFFL_MASK        GENMASK(5, 0)

/* TXESC: Tx Buffer Element Size Configuration */
#define TXESC_TBDS_SHIFT    0
#define TXESC_TBDS_MASK        GENMASK(2, 0)


/* TXEFC: Tx Event FIFO Configuration */
#define TXEFC_EFWM_SHIFT    24
#define TXEFC_EFWM_MASK        GENMASK(29, 24)

#define TXEFC_EFS_SHIFT        16
#define TXEFC_EFS_MASK        GENMASK(21, 16)

#define TXEFC_EFSA_SHIFT    2
#define TXEFC_EFSA_MASK        GENMASK(15, 2)

/* TXEFS: Tx Event FIFO Status */
#define TXEFS_TEFL_BIT        BIT(25)
#define TXEFS_EFF_BIT        BIT(24)

#define TXEFS_EFPI_SHIFT    16
#define TXEFS_EFPI_MASK        GENMASK(20, 16)

#define TXEFS_EFGI_SHIFT    8
#define TXEFS_EFGI_MASK        GENMASK(12, 8)

#define TXEFS_EFFL_SHIFT    0
#define TXEFS_EFFL_MASK        GENMASK(5, 0)

/*                     Message RAM                   */
#define BYTES_OF_WORD        4
#define DATA_OFFSET          8
#define BYTES_OF_MSG_RAM_MAX    4096
#define DATA_BYTE0_OFF        0
#define DATA_BYTE1_OFF        8
#define DATA_BYTE2_OFF        16
#define DATA_BYTE3_OFF        24

/* Size of element header */
#define ELMT_HEADER_SIZE_RXB    8
#define ELMT_HEADER_SIZE_TXB    8

/* Max size of elements */
#define ELMT_SIZE_SIDF        4
#define ELMT_SIZE_XIDF        8
#define ELMT_SIZE_RXB_MAX     72
#define ELMT_SIZE_TXEF        8
#define ELMT_SIZE_TXB_MAX     72
#define ELMT_SIZE_TRIG_MEM    8

/* Standard Message ID Filter Element */
#define FEC_DIS               0
#define FEC_STORE_RXF0        1
#define FEC_STORE_RXF1        2
#define FEC_REJECT            3
#define FEC_SET_PRIORITY      4
#define FEC_SP_STORE_RXF0     5
#define FEC_SP_STORE_RXF1     6
#define FEC_STORE_RXB_DEBUG   7

#define STORE_RX_BUF    0
#define DEBUG_MSG_A     1
#define DEBUG_MSG_B     2
#define DEBUG_MSG_C     3

#define SIDF_SFT_SHIFT            30
#define SIDF_SFT_MASK            GENMASK(31, 30)

#define SIDF_SFEC_SHIFT            27
#define SIDF_SFEC_MASK            GENMASK(29, 27)

#define SIDF_SFID1_SHIFT        16
#define SIDF_SFID1_MASK            GENMASK(26, 16)

#define SIDF_SSYNC            BIT(15)

#define SIDF_SFID2            GENMASK(10, 0)

#define SIDF_SFID2_STORE_SHIFT        9
#define SIDF_SFID2_STORE_MASK        GENMASK(10, 9)

#define SIDF_SFID2_OFFSET        GENMASK(5, 0)

/* Extended Message ID Filter Element */
#define EIDF_F0_EFEC_SHIFT        29
#define EIDF_F0_EFEC_MASK        GENMASK(31, 29)

#define EIDF_F0_EFID1            GENMASK(28, 0)

#define EIDF_F1_EFT_SHIFT        30
#define EIDF_F1_EFT_MASK        GENMASK(31, 30)

#define EIDF_F1_ESYNC            BIT(29)

#define EIDF_F1_EFID2            GENMASK(28, 0)

#define EIDF_F1_EFID2_STORE_SHIFT    9
#define EIDF_F1_EFID2_STORE_MASK    GENMASK(10, 9)

#define EIDF_F1_EFID2_OFFSET        GENMASK(5, 0)

/* Rx Buffer and FIFO Element */
#define RXB_R0_ESI        BIT(31)
#define RXB_R0_XTD        BIT(30)
#define RXB_R0_RTR        BIT(29)
#define RXB_R0_ID_MASK        GENMASK(28, 0)

#define RXB_R1_ANMF           BIT(31)
#define RXB_R1_FIDX_SHIFT     24
#define RXB_R1_FIDX_MASK      GENMASK(30, 24)
#define RXB_R1_FDF            BIT(21)
#define RXB_R1_BRS            BIT(20)
#define RXB_R1_DLC_SHIFT      16
#define RXB_R1_DLC_MASK       GENMASK(19, 16)
#define RXB_R1_TSC_BIT        BIT(4)
#define RXB_R1_RXTSP_MASK     GENMASK(3, 0)
#define RXB_R1_RXTS_MASK      GENMASK(15, 0)

/* Tx Event FIFO Element */
#define TXE_E0_ESI        BIT(31)
#define TXE_E0_XTD        BIT(30)
#define TXE_E0_RTR        BIT(29)

#define TXE_E1_MM_SHIFT        24
#define TXE_E1_MM_MASK         GENMASK(31, 24)
#define TXE_E1_ET_SHIFT        22
#define TXE_E1_ET_MASK        GENMASK(23, 22)
#define TXE_E1_FDF            BIT(21)
#define TXE_E1_BRS            BIT(20)
#define TXE_E1_DLC_SHIFT      16
#define TXE_E1_DLC_MASK       GENMASK(19, 16)
#define TXE_E1_TSC_MASK       BIT(4)
#define TXE_E1_MM_H_SHIFT     8
#define TXE_E1_MM_H_MASK      GENMASK(15, 8)
#define TXE_E1_TXTSP_MASK     GENMASK(3, 0)
#define TXE_E1_TXTS_MASK      GENMASK(15, 0)

/* Tx Buffer Element */
#define TXB_T0_ESI        BIT(31)
#define TXB_T0_XTD        BIT(30)
#define TXB_T0_RTR        BIT(29)
#define TXB_T0_ID_MASK        GENMASK(28, 0)

#define TXB_T1_MML_SHIFT    24
#define TXB_T1_MML_MASK        GENMASK(31, 24)
#define TXB_T1_EFC        BIT(23)
#define TXB_T1_TSCE       BIT(22)
#define TXB_T1_FDF        BIT(21)
#define TXB_T1_BRS        BIT(20)
#define TXB_T1_DLC_SHIFT    16
#define TXB_T1_DLC_MASK        GENMASK(19, 16)
#define TXB_T1_MMH_SHIFT       8
#define TXB_T1_MMH_MASK        GENMASK(15, 8)
#define TXB_T1_TSC_BIT         BIT(4)
#define TXB_T1_TXTSP_SHIFT     0
#define TXB_T1_TXTSP_MASK    GENMASK(3, 0)

/* Configuration file parameters */
#define ELMT_SIDF        0 /* 0 - 128 */
#define ELMT_XIDF        1 /* 0 - 64 */
#define ELMT_RXF0        2 /* 0 - 64 */
#define ELMT_RXF1        3 /* 0 - 64 */
#define ELMT_RXB         4 /* 0 - 64 */
#define ELMT_TXEF        5 /* 0 - 32 */
#define ELMT_TXB         6 /* 0 - 32 */
#define ELMT_TMC         7 /* 0 - 64 */
#define ELMT_TYPE_MAX        8

#define TX_ELMT_BUF        0 /* 0 - element_num[ELMT_TXB] */
#define TX_ELMT_FQ         1 /* 0 - element_num[ELMT_TXB] */
#define TX_ELMT_TYPE_MAX    2

#define WATERMARK_RXF0        0 /* 0 - element_num[ELMT_RXF0] */
#define WATERMARK_RXF1        1 /* 0 - element_num[ELMT_RXF1] */
#define WATERMARK_TXEF        2 /* 0 - element_num[ELMT_TXEF] */
#define WATERMARK_TYPE_MAX    3

#define DSIZE_RXF0        0 /* 8/12/16/20/24/32/48/64 */
#define DSIZE_RXF1        1 /* 8/12/16/20/24/32/48/64 */
#define DSIZE_RXB         2 /* 8/12/16/20/24/32/48/64 */
#define DSIZE_TXB         3 /* 8/12/16/20/24/32/48/64 */
#define DSIZE_TYPE_MAX    4

#define MODE_RXF0        0 /* 0:blocking, 1:overwrite */
#define MODE_RXF1        1 /* 0:blocking, 1:overwrite */
#define MODE_TXFQ        2 /* 0:tx_fifo,  1:tx_queue  */
#define MODE_TYPE_MAX        3

/* Max number of elements */
#define ELMT_NUM_MAX_SIDF    128
#define ELMT_NUM_MAX_XIDF    64

#define ELMT_NUM_MAX_RXF0    64
#define ELMT_NUM_MAX_RXF1    64
#define ELMT_NUM_MAX_RXB    64

#define ELMT_NUM_MAX_TXEF    32
#define ELMT_NUM_MAX_TXB    32
#define ELMT_NUM_MAX_TRIG_MEM    64

/* data size */
#define DATA_SZIE_8    8
#define DATA_SZIE_12    12
#define DATA_SZIE_16    16
#define DATA_SZIE_20    20
#define DATA_SZIE_24    24
#define DATA_SZIE_32    32
#define DATA_SZIE_48    48
#define DATA_SZIE_64    64

/* GPIO REG */
#define GPIO_DATA_REG_OFFSET    0x0000

#define GPIO_DIR_REG_OFFSET     0x0004

struct mttcan_base_addr {
    void __iomem *reg_base;
    void __iomem *mram_base;
    void __iomem *reset_base;
    void __iomem *plat_base;
};

enum tasklet_sched_type {
    TASKLET_SCHEDULE,
    TASKLET_HI_SCHEDULE
};

struct resource;
struct mttcan_config {
    u32 element_num[ELMT_TYPE_MAX];
    u32 tx_element_num[TX_ELMT_TYPE_MAX];
    u32 watermark[WATERMARK_TYPE_MAX];
    u32 dsize[DSIZE_TYPE_MAX];
    bool mode[MODE_TYPE_MAX];
    u32 sid_filter[ELMT_NUM_MAX_SIDF];
    u32 xid_filter[ELMT_NUM_MAX_XIDF << 1];

    u32 global_filter_cfg;
    u32 extend_id_and_mask;
    u32 echo_skb_max;
    u32 poll_weight;
    u32 ts_cnt_prescaler;
    u32 tx_ir_cpu;
    u32 rx_ir_cpu;
    enum tasklet_sched_type tasklet_schedule_type;

    u32 cclk_freq_hz;
    u32 hclk_freq_hz;
    char mttcan_name[MTTCAN_NAME_LEN];
    char can_name[IFNAMSIZ];
    u32 mttcan_id;
    int irq0;
    int irq1;
    struct resource *reg_res;
    struct resource *mram_res;
    struct resource *reset_res;
    struct resource *gpio_res;
    struct resource *plat_res;
};

static inline u32 mttcan_read_reg(const struct mttcan_base_addr *addr, int reg)
{
    return (u32)readl(addr->reg_base + reg);
}

static inline void mttcan_write_reg(const struct mttcan_base_addr *addr, int reg, u32 val)
{
    writel(val, addr->reg_base + reg);
}

static inline void mttcan_request_reset(const struct mttcan_base_addr *addr, u32 bit)
{
    writel(BIT(bit), addr->reset_base);
}

static inline void mttcan_remove_reset(const struct mttcan_base_addr *addr, u32 bit)
{
    writel(BIT(bit), addr->reset_base + BYTES_OF_WORD);
}

struct net_device;
struct sk_buff;
int write_reg_and_report_fault(const struct net_device *ndev, void __iomem *addr_base, int addr_offset, u32 val);

void mttcan_dump_reg(const struct net_device *ndev);
int test_endn_and_print_revision(const struct net_device *ndev);
int mttcan_set_loopback_mode(const struct net_device *ndev, bool enable);
int mttcan_set_bus_monitor_mode(const struct net_device *ndev, bool enable);
int mttcan_set_auto_retrans(const struct net_device *ndev, bool enable);
int mttcan_enable_canfd(const struct net_device *ndev, bool enable);

int mttcan_set_init_mode(const struct net_device *ndev);
int mttcan_set_normal_mode(const struct net_device *ndev);
int mttcan_set_bittiming(struct net_device *ndev);
int mttcan_set_data_bittiming(struct net_device *ndev);

void mttcan_print_revision(const struct net_device *ndev);
int mttcan_wait_clock_stop_ack(const struct net_device *ndev);

int mttcan_config_controller(const struct net_device *ndev);

/* rx msg */
int mttcan_read_rx_fifo0(struct net_device *ndev, int weight);
int mttcan_read_rx_fifo1(struct net_device *ndev, int weight);
int mttcan_read_rx_buffer(struct net_device *ndev, int weight);

/* tx msg */
void mttcan_tx_complete(struct net_device *ndev);
void mttcan_tx_event(struct net_device *ndev);
void mttcan_tx_cancel_finish(struct net_device *ndev);

int mttcan_write_tx_message(const struct net_device *ndev, struct sk_buff *skb, u32 *txb_idx);

#endif /* _CAN_DRV_MTTCAN__ */
