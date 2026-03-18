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

#ifndef CAN_DRV_COMMON_H
#define CAN_DRV_COMMON_H

#include <linux/types.h>
#include <linux/io.h>
#include <linux/kernel.h>
#ifdef RUN_IN_AOS
#include <linux/bits.h>
#endif

#define SET_INIT_TIMEOUT_US      500
#define WAIT_FOR_TX_HANDLE_US    (50 * 1000)
#define WAIT_CSA_TIMEOUT_MS      50

#define BIT_OF_BYTE              8

#define MTTCAN_NAME_LEN        50
#define MTTCAN_KEY_LEN         50

#ifndef IFNAMSIZ
#define IFNAMSIZ        16
#endif

#define MAX_TDCR_TDCO         127
#define MIN_TDCO_DBITRATE     2500000
#define CAN_CLOCK_FREQ_DGREE  1000
#define MAX_BITRATE           1000000
#define MAX_DATA_BITRATE      5000000

/* address offset of mttcan register */
#define REG_CREL    0x000
#define REG_ENDN    0x004
#define REG_DBTP    0x00C
#define REG_TEST    0x010
#define REG_RWD     0x014
#define REG_CCCR    0x018
#define REG_NBTP    0x01C
#define REG_TSCC    0x020
#define REG_TSCV    0x024
#define REG_TOCC    0x028
#define REG_TOCV    0x02C
#define REG_ECR     0x040
#define REG_PSR     0x044
#define REG_TDCR    0x048
#define REG_IR      0x050
#define REG_IE      0x054
#define REG_ILS     0x058
#define REG_ILE     0x05C
#define REG_GFC     0x080
#define REG_SIDFC   0x084
#define REG_XIDFC   0x088
#define REG_XIDAM   0x090
#define REG_HPMS    0x094
#define REG_NDAT1   0x098
#define REG_NDAT2   0x09C
#define REG_RXF0C   0x0A0
#define REG_RXF0S   0x0A4
#define REG_RXF0A   0x0A8
#define REG_RXBC    0x0AC
#define REG_RXF1C   0x0B0
#define REG_RXF1S   0x0B4
#define REG_RXF1A   0x0B8
#define REG_RXESC   0x0BC
#define REG_TXBC    0x0C0
#define REG_TXFQS   0x0C4
#define REG_TXESC   0x0C8
#define REG_TXBRP   0x0CC
#define REG_TXBAR   0x0D0
#define REG_TXBCR   0x0D4
#define REG_TXBTO   0x0D8
#define REG_TXBCF   0x0DC
#define REG_TXBTIE  0x0E0
#define REG_TXBCIE  0x0E4
#define REG_TXEFC   0x0F0
#define REG_TXEFS   0x0F4
#define REG_TXEFA   0x0F8

#define REG_TTTMC    0x100
#define REG_TTRMC    0x104
#define REG_TTOCF    0x108
#define REG_TTMLM    0x10C
#define REG_TURCF    0x110
#define REG_TTOCN    0x114
#define REG_TTGTP    0x118
#define REG_TTTMK    0x11C
#define REG_TTIR     0x120
#define REG_TTIE     0x124
#define REG_TTILS    0x128
#define REG_TTOST    0x12C
#define REG_TURNA    0x130
#define REG_TTLGT    0x134
#define REG_TTCTC    0x138
#define REG_TTCPT    0x13C
#define REG_TTCSM    0x140

/* TSU Regiser */
#define REG_TSU_CREL    0x160
#define REG_TSU_TSCFG   0x164
#define REG_TSU_TSS1    0x168
#define REG_TSU_TSS2    0x16C

#define REG_TSU_TS0    0x170
#define REG_TSU_TS1    0x174
#define REG_TSU_TS2    0x178
#define REG_TSU_TS3    0x17C

#define REG_TSU_TS4    0x180
#define REG_TSU_TS5    0x184
#define REG_TSU_TS6    0x188
#define REG_TSU_TS7    0x18C

#define REG_TSU_TS8    0x190
#define REG_TSU_TS9    0x194
#define REG_TSU_TS10   0x198
#define REG_TSU_TS11   0x19C

#define REG_TSU_TS12    0x1A0
#define REG_TSU_TS13    0x1A4
#define REG_TSU_TS14    0x1A8
#define REG_TSU_TS15    0x1AC

#define REG_TSU_ATB    0x1B0

#define REG_IR_MASK    0x3FFFFFFF
#define REG_TTIR_MASK  0x0007FFFF

/* address offset of system control register */
#define SUBCTRL_CCLK_STATE     0x53A0
#define SUBCTRL_CCLK_EN        0x03A0
#define SUBCTRL_CCLK_DIS       0x03A4

#define SUBCTRL_RESET_STATE    0x5A48
#define SUBCTRL_RESET_REQ      0x0A48
#define SUBCTRL_RESET_DREQ     0x0A4C

/*             TSU Reg bit define                    */
/* TSCFG: Timestamp Configuration */
#define TSCFG_TBPRE_SHIFT       8
#define TSCFG_TBPRE_MASK        GENMASK(15, 8)

#define TSCFG_SCP_BIT           BIT(2)
#define TSCFG_TBCS_BIT          BIT(1)
#define TSCFG_TSUE_BIT          BIT(0)

#define ATB_TB_MASK             GENMASK_ULL(31, 0)

#define TSS1_BIT_SHIFT          16

/* ************************************************* */
/*             MTTCAN Reg bit define                 */
/* ************************************************* */
/* CREL: Core Release Register */
#define CREL_REL_SHIFT        28
#define CREL_REL_MASK         GENMASK(31, 28)

#define CREL_STEP_SHIFT        24
#define CREL_STEP_MASK         GENMASK(27, 24)

#define CREL_SUBSTEP_SHIFT    20
#define CREL_SUBSTEP_MASK     GENMASK(23, 20)

#define CREL_YEAR_SHIFT        16
#define CREL_YEAR_MASK         GENMASK(19, 16)

#define CREL_MON_SHIFT        8
#define CREL_MON_MASK         GENMASK(15, 8)

#define CREL_DAY_SHIFT        0
#define CREL_DAY_MASK        GENMASK(7, 0)

/* ENDN: Endian Register */
#define ENDN_ETV_DEFAULT    0x87654321U

/* DBTP: Data Bit Timing & Prescaler Register */
#define DBTP_TDC_DIS        0
#define DBTP_TDC_EN         BIT(23) /* Transmitter Delay Compensation */

#define DBTP_DBRP_SHIFT        16
#define DBTP_DBRP_MASK         GENMASK(20, 16) /* Data Bit Rate Prescaler */

#define DBTP_DTSEG1_SHIFT    8
#define DBTP_DTSEG1_MASK     GENMASK(12, 8) /* Data time segment before sample point */

#define DBTP_DTSEG2_SHIFT    4
#define DBTP_DTSEG2_MASK     GENMASK(7, 4) /* Data time segment after sample point */

#define DBTP_DSJW_SHIFT        0
#define DBTP_DSJW_MASK        GENMASK(3, 0) /* Data Synchronization Jump Width */

/* TEST: Test Register */
#define TEST_LBCK_BIT    BIT(4)

/* CCCR: Can Core Control Register */
#define CCCR_NISO_BIT    BIT(15)
#define CCCR_TXP_BIT     BIT(14)
#define CCCR_EFBI_BIT    BIT(13)
#define CCCR_PXHD_BIT    BIT(12)
#define CCCR_UTSU_BIT    BIT(10)
#define CCCR_BRSE_BIT    BIT(9)
#define CCCR_FDOE_BIT    BIT(8)
#define CCCR_TEST_BIT    BIT(7)
#define CCCR_DAR_BIT     BIT(6)
#define CCCR_MON_BIT     BIT(5)
#define CCCR_CSR_BIT     BIT(4)
#define CCCR_CSA_BIT     BIT(3)
#define CCCR_ASM_BIT     BIT(2)
#define CCCR_CCE_BIT     BIT(1)
#define CCCR_INIT_BIT    BIT(0)

/* NBTP: Nominal Bit Timing & Prescaler Register */
#define NBTP_NSJW_SHIFT        25
#define NBTP_NSJW_MASK        GENMASK(31, 25) /* Nominal Synchronization Jump Width */

#define NBTP_NBRP_SHIFT        16
#define NBTP_NBRP_MASK        GENMASK(24, 16) /* Nominal Bit Rate Prescaler */

#define NBTP_NTSEG1_SHIFT    8
#define NBTP_NTSEG1_MASK    GENMASK(15, 8) /* Nominal Time segment before sample point */

#define NBTP_NTSEG2_SHIFT    0
#define NBTP_NTSEG2_MASK    GENMASK(6, 0) /* Nominal Time segment after sample point */

/* TSCC */
#define TSCC_TCP_SHIFT        16
#define TSCC_TCP_MASK        GENMASK(19, 16)

#define TSCC_TSS_SHIFT        0
#define TSCC_TSS_MASK        GENMASK(1, 0)

#define TIMESTAMP_DIS        0
#define TIMESTAMP_INT        1
#define TIMESTAMP_EXT        2

/* TSCV */
#define TSCV_TSC_MASK        GENMASK(15, 0)

/* TOCC */
#define TOCC_TOP_SHIFT        16
#define TOCC_TOP_MASK        GENMASK(31, 16)

#define TOCC_TOS_SHIFT        1
#define TOCC_TOS_MASK        GENMASK(2, 1)

#define TOCC_ETOC_BIT        BIT(0)

/* TOCV */
#define TOCV_TOC_MASK        GENMASK(15, 0)

/* ECR: Error Counter Register */
#define ECR_CEL_SHIFT        16
#define ECR_CEL_MASK         GENMASK(23, 16)

#define ECR_RP_BIT        BIT(15)

#define ECR_REC_SHIFT        8
#define ECR_REC_MASK        GENMASK(14, 8)

#define ECR_TEC_SHIFT        0
#define ECR_TEC_MASK        GENMASK(7, 0)

/* PSR: Protocol Status Register */
#define LEC_TYPE_NO_ERROR       0
#define LEC_TYPE_STUFF_ERROR    1
#define LEC_TYPE_FORM_ERROR     2
#define LEC_TYPE_ACK_ERROR      3
#define LEC_TYPE_BIT1_ERROR     4
#define LEC_TYPE_BIT0_ERROR     5
#define LEC_TYPE_CRC_ERROR      6
#define LEC_TYPE_NO_CHANGE      7

#define PSR_TDCV_SHIFT        16
#define PSR_TDCV_MASK         GENMASK(22, 16)

#define PSR_PXE_BIT         BIT(14)
#define PSR_RFDF_BIT        BIT(13)
#define PSR_RBRS_BIT        BIT(12)
#define PSR_RESI_BIT        BIT(11)

#define PSR_DLEC_SHIFT        8
#define PSR_DLEC_MASK        GENMASK(10, 8)

#define PSR_BO_BIT        BIT(7)
#define PSR_EW_BIT        BIT(6)
#define PSR_EP_BIT        BIT(5)

#define PSR_ACT_SHIFT        3
#define PSR_ACT_MASK        GENMASK(4, 3)

#define PSR_LEC_SHIFT        0
#define PSR_LEC_MASK        GENMASK(2, 0)

/* TDCR: Transmitter Delay Compensation Register */
#define TDCR_TDCO_SHIFT        8
#define TDCR_TDCO_MASK        GENMASK(14, 8)

#define TDCR_TDCF_SHIFT        0
#define TDCR_TDCF_MASK        GENMASK(6, 0)

/* IR: Interrupt Register */
#define IR_ARA_BIT    BIT(29) /* Access to Reserved Address */
#define IR_PED_BIT    BIT(28) /* Protocol Error in Data Phase */
#define IR_PEA_BIT    BIT(27) /* Protocol Error in Arbitration Phase */
#define IR_WDI_BIT    BIT(26) /* Watchdog Interrupt */

#define IR_BO_BIT    BIT(25) /* Bus_Off Status */
#define IR_EW_BIT    BIT(24) /* Error Warning Status */
#define IR_EP_BIT    BIT(23) /* Error Passive Status */

#define IR_ELO_BIT    BIT(22) /* Error Logging Overflow */
#define IR_BEU_BIT    BIT(21) /* Bit Error Uncorrected */
#define IR_BEC_BIT    BIT(20) /* Bit Error Corrected */

#define IR_DRX_BIT    BIT(19) /* Message stored to Dedicated Rx Buffer */
#define IR_TOO_BIT    BIT(18) /* Timeout Occurred */
#define IR_MRAF_BIT   BIT(17) /* Message RAM Access Failure */
#define IR_TSW_BIT    BIT(16) /* Timestamp Wraparound */

#define IR_TEFL_BIT    BIT(15) /* Tx Event FIFO Element Lost */
#define IR_TEFF_BIT    BIT(14) /* Tx Event FIFO Full */
#define IR_TEFW_BIT    BIT(13) /* Tx Event FIFO Watermark Reached */
#define IR_TEFN_BIT    BIT(12) /* Tx Event FIFO New Entry */

#define IR_TFE_BIT    BIT(11) /* Tx FIFO Empty */
#define IR_TCF_BIT    BIT(10) /* Transmission Cancellation Finished */
#define IR_TC_BIT     BIT(9) /* Transmission Completed */
#define IR_HPM_BIT    BIT(8) /* High Priority Message */

#define IR_RF1L_BIT    BIT(7) /* Rx FIFO 1 Message Lost */
#define IR_RF1F_BIT    BIT(6) /* Rx FIFO 1 Full */
#define IR_RF1W_BIT    BIT(5) /* Rx FIFO 1 Watermark Reached */
#define IR_RF1N_BIT    BIT(4) /* Rx FIFO 1 New Message */

#define IR_RF0L_BIT    BIT(3) /* Rx FIFO 0 Message Lost */
#define IR_RF0F_BIT    BIT(2) /* Rx FIFO 0 Full */
#define IR_RF0W_BIT    BIT(1) /* Rx FIFO 0 Watermark Reached */
#define IR_RF0N_BIT    BIT(0) /* Rx FIFO 0 New Message */

#define INT_RF0_MASK    GENMASK(3, 0)
#define INT_RF1_MASK    GENMASK(7, 4)
#define INT_TEF_MASK    GENMASK(15, 12)

#define INT_ERR_PROTO    (IR_PED_BIT | IR_PEA_BIT)
#define INT_ERR_STATUS    (IR_BO_BIT | IR_EW_BIT | IR_EP_BIT)

#define INT_FIFO_LOST    (IR_RF1L_BIT | IR_RF0L_BIT)
#define INT_FIFO_FULL    (IR_RF1F_BIT | IR_RF0F_BIT)

#define INT_RX        (IR_DRX_BIT | IR_HPM_BIT | IR_RF1W_BIT | IR_RF1N_BIT | IR_RF0W_BIT | IR_RF0N_BIT)
#define INT_TX        (IR_TEFW_BIT | IR_TEFN_BIT | IR_TFE_BIT | IR_TC_BIT | IR_TEFL_BIT | IR_TEFF_BIT)

/* ILS: Interrupt Line Select */
#define INT_LINE0_SEL    (INT_RX | INT_FIFO_LOST | INT_FIFO_FULL)
#define INT_LINE1_SEL    (INT_ERR_PROTO | INT_ERR_STATUS | INT_TX | IR_TCF_BIT)

/* IE: Interrupt Enable */
#define INT_ENABLE    (INT_LINE0_SEL | INT_LINE1_SEL)

/* ILE: Interrupt Line Enable */
#define INT_LINE_1    BIT(1)
#define INT_LINE_0    BIT(0)
#define INT_LINE_ALL    (INT_LINE_1 | INT_LINE_0)

#endif /* _CAN_DRV_COMMON__ */
