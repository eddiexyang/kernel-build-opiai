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

#ifndef SOC_MISC_PCIE_RAS_H
#define SOC_MISC_PCIE_RAS_H

#include "dms_define.h"
#include "fpdc.h"

#define PCIE_AA_HISI_COMM     3
#define PCIE_DISP_HISI_COMM   7
#define PCIE_SMMU_HISI_COMM  13
#define PCIE_LOCAL_HISI_COMM 21

#define PCIE_REPORT   0U
#define PCIE_PRINT    1U

#define IOB_RX_INT_STATUS 0
#define IOB_TX_INT_STATUS2 2
#define IOB_TX_INT_STATUS3 3
#define DISP_ERR_STATUSL 4
#define SMMU_ERR_STATUS_0 4
#define ABNORMAL_INT_STS 0
#define ECC_ERR_INT_SRC_STS 1
#define TL_INT_STATUS0 0
#define MAC_REG_MAC_INT_STATUS 0
#define DL_INT_STATUS 0

struct pcie_local_ras_event {
    u32 reg_offset;
    u32 bits;
    u32 ras_code;     /* pcie_local_ras_err_type */
    u32 report_type;  /* report to fault manager or not */
};

#define PCIE_RAS_COVERGE_NODE(err, msg, type) { \
        .subsys_id = DMS_DEV_TYPE_PCIE,             \
        .module_id = SOC_MISC_SENSOR_PCIE,          \
        .section_type = RAS_SEC_PCIE,               \
        .ras_code.err_type = err,                   \
        .describe = msg,                            \
        .sensor_type = DMS_SEN_TYPE_RAS_SENSOR,     \
        .error_type = type,                         \
}

#define PCIE_DISP_RAS_COVERGE_NODE(err, msg, type) { \
        .subsys_id = DMS_DEV_TYPE_PCIE_DISP,         \
        .module_id = SOC_MISC_SENSOR_PCIE_DISP,      \
        .section_type = RAS_SEC_PCIE,                \
        .ras_code.err_type = err,                    \
        .describe = msg,                             \
        .sensor_type = DMS_SEN_TYPE_RAS_SENSOR,      \
        .error_type = type,                          \
}

#define PCIE_HISI_COMMON_RAS(reg, bit, code, flag) { \
    .reg_offset = reg,   \
    .bits = bit,         \
    .ras_code = code,    \
    .report_type = flag, \
}

/* PCIE LOCAL RAS ERR TYPE */
enum pcie_local_ras_err_type {
    PCIE_RAS_AP_COREX_TX_CPL_MBECC = 0,      /* Core x TX CPL RAM 2bit ECC Error */
    PCIE_RAS_AP_COREX_TX_CPL_SBECC = 1,      /* Core x TX CPL RAM 1bit ECC Error */
    PCIE_RAS_AP_P2P_ADDR_UNMATCH = 4,   /* AP P2P P/CPL unmatch address window */
    PCIE_RAS_AP_CHI_CPU_RAM_MBECC = 32,      /* CHI POST RAM 2bit ECC Error */
    PCIE_RAS_AP_CHI_CPU_RAM_SBECC = 33,      /* CHI POST RAM 1bit ECC Error */
    PCIE_RAS_AP_CHI_P2P_RAM_MBECC = 34,      /* CHI P2P and CPL RAM 2bit ECC Error */
    PCIE_RAS_AP_CHI_P2P_RAM_SBECC = 35,      /* CHI P2P and CPL RAM 1bit ECC Error */
    PCIE_RAS_AP_CHI_UNEXPECT_TRANS = 160,      /* chi unexpected_transaction_rcvd */
    PCIE_RAS_AP_IOB_ODR_ERR_BRESP = 192,      /* iob: axi b channel error response */
    PCIE_RAS_AP_IOB_ODR_POISON_RDATA = 193,      /* iob: axi r channel poison data */
    PCIE_RAS_AP_IOB_ODR_ERR_RRESP = 194,      /* iob: axi r channel error response */
    PCIE_RAS_AP_IOB_ODR_PCPL_SBM_MBECC = 195,      /* iob: pcpl_sbm_ecc_mulbit err */
    PCIE_RAS_AP_IOB_ODR_PCPL_SBM_SBECC = 196,      /* iob: pcpl_sbm_ecc_1bit err */
    PCIE_RAS_AP_IOB_ODR_NPQ_SBM_MBECC = 197,      /* iob: npq_sbm_ecc_mulbit */
    PCIE_RAS_AP_IOB_ODR_NPQ_SBM_SBECC = 198,      /* iob: npq_sbm_ecc_1bit */
    PCIE_RAS_AP_IOB_AXIM_ERR_BRESP = 224,      /* axim: axi b channel error response */
    PCIE_RAS_AP_IOB_AXIM_POISON_RDATA = 225,      /* axim: axi r channel poison data */
    PCIE_RAS_AP_IOB_AXIM_ERR_RRESP = 226,      /* axim: axi r channel error response */
    PCIE_RAS_AP_IOB_AXIM_PCPL_SBM_MBECC = 227,      /* axim: pcpl_sbm_ecc_mulbit err */
    PCIE_RAS_AP_IOB_AXIM_PCPL_SBM_SBECC = 228,      /* axim: pcpl_sbm_ecc_1bit err */
    PCIE_RAS_AP_IOB_AXIM_NPQ_SBM_MBECC = 229,      /* axim: npq_sbm_ecc_mulbit */
    PCIE_RAS_AP_IOB_AXIM_NPQ_SBM_SBECC = 230,      /* axim: npq_sbm_ecc_1bit */
    PCIE_RAS_AP_IOB_AXIM_RD_TRANS_TMOUT = 231,      /* AXI Master Read Transaction Timeout */
    PCIE_RAS_AP_IOB_AXIM_WR_TRANS_TMOUT = 232,      /* AXI Master Write Transaction Timeout */
    PCIE_RAS_DMA_LINKDOWN = 256, /* Link down */
    PCIE_RAS_DMA_DATA_POISON_REMOTE = 257, /* Data poison in remote side */
    PCIE_RAS_DMA_DATA_POISON_SQ_READ = 258, /* Data poison when SQ read */
    PCIE_RAS_DMA_CQ_FULL = 259, /* CQ queue full status */
    PCIE_RAS_DMA_CQ_WRITE_BACK_ERR = 260, /* CQ write back err */
    PCIE_RAS_DMA_DATA_POISON_LOCAL = 261, /* Data posion in local side */
    PCIE_RAS_DMA_AXI_WRITE_RESPONSE_ERR = 262, /* AXI Master write reponse error */
    PCIE_RAS_DMA_AXI_READ_RESPONSE_ERR = 263, /* AXI Master read reponse error */
    PCIE_RAS_DMA_READ_RESPONSE_REMOTE = 264, /* Read reponse error in Remote side */
    PCIE_RAS_DMA_WRITE_RESPONSE_REMOTE = 265, /* Write reponse error in Remote side */
    PCIE_RAS_DMA_DROP_OCCUR = 266, /* Drop occurs */
    PCIE_RAS_DMA_INVALID_LENGTH_FIELD = 267, /* Invalid Length Field occurs */
    PCIE_RAS_DMA_INVALID_OPCODE = 268, /* Invalid Opcode occurs */
    PCIE_RAS_DMA_SQBD_READ_RESPONSE_ERR = 269, /* Submission descriptor read response error */
    PCIE_RAS_DMA_REMOTE_URCA_ERR = 270, /* remote UR/CA error */
    PCIE_RAS_INT_SRC0_VEC_ERR = 300, /* int_src0_func_vec_er_sts from register */
    PCIE_RAS_INT_SRC1_VEC_ERR = 301, /* int_src1_func_vec_er_sts from DMA */
    PCIE_RAS_INT_SRC2_VEC_ERR = 302, /* int_src2_func_vec_er_sts from NVME */
    PCIE_RAS_MSI_COAL_MBECC = 316, /* access MSI Coal Table ecc_data_err_mult */
    PCIE_RAS_MSI_COAL_SBECC = 317, /* access MSI Coal Table ecc_data_err_single */
    PCIE_RAS_PTR_TABLE_MBECC = 318, /* access Pointer Table ecc_data_err_mult */
    PCIE_RAS_PTR_TABLE_SBECC = 319, /* access Pointer Table ecc_data_err_single */
    PCIE_RAS_MSIX_COAL_MBECC = 320, /* access MSIX Coal Table ecc_data_err_mult */
    PCIE_RAS_MSIX_COAL_SBECC = 321, /* access MSIX Coal Table ecc_data_err_single */
    PCIE_RAS_MSIX_TABLE_PBA_MBECC = 322, /* access MSIX Table ecc_data_err_mult */
    PCIE_RAS_MSIX_TABLE_PBA_SBECC = 323, /* access MSIX Table ecc_data_err_single */
    PCIE_RAS_DL_VC1_CPL_UPDATE_TIMEOUT = 480, /* rx_fc_vc1_update_timeout_cpl_int_status */
    PCIE_RAS_DL_VC1_NP_UPDATE_TIMEOUT = 481, /* rx_fc_vc1_update_timeout_np_int_status */
    PCIE_RAS_DL_VC1_P_UPDATE_TIMEOUT = 482, /* rx_fc_vc1_update_timeout_p_int_status */
    PCIE_RAS_DL_VC0_CPL_UPDATE_TIMEOUT = 483, /* rx_fc_vc0_update_timeout_cpl_int_status */
    PCIE_RAS_DL_VC0_NP_UPDATE_TIMEOUT = 484, /* rx_fc_vc0_update_timeout_np_int_status */
    PCIE_RAS_DL_VC0_P_UPDATE_TIMEOUT = 485, /* rx_fc_vc0_update_timeout_p_int_status */
    PCIE_RAS_DL_COR_ERR_TIMEOUT = 486, /* dl_correct_err_cnt_timeout_int_status */
    PCIE_RAS_DL_FC_UPDATE_TIMEOUT = 487, /* rx_fc_update_timeout_int_status */
    PCIE_RAS_DL_MAC_RETRAIN_CNT_OVER = 488, /* dl_mac_retrain_cnt_over_int_status */
    PCIE_RAS_DL_TL_CREDIT_NULL_TIMEOUT = 489, /* tl_dl_credit_null_timeout_int_status */
    PCIE_RAS_DL_NAK_TIMER_TIMEOUT = 490, /* dl_nak_timer_timeout_int_status */
    PCIE_RAS_DL_DUP_TLP_ACK_CNT_ROLLOVER = 491, /* dl_dup_tlp_ack_cnt_rollover_int_status */
    PCIE_RAS_DL_MAC_LINKDOWN = 492, /* link_fail_dl_int_status */
    PCIE_RAS_DL_RETRAIN = 493, /* retrain_dl_int_status */
    PCIE_RAS_DL_MBECC = 494, /* ecc_2b_err_dl_int_status */
    PCIE_RAS_DL_SBECC = 495, /* ecc_1b_err_dl_int_status */
    PCIE_RAS_DL_INIT_TIMEOUT = 496, /* init_timeout_dl_int_status */

    PCIE_RAS_AP_IOB_TX_NP_TMOUT = 621, /* Tx NonPost Request timeout */
    PCIE_RAS_AP_IOB_TX_P_TMOUT = 622, /* Tx Post Request timeout */
    PCIE_RAS_AP_IOB_TX_DATA_BE_MBECC = 631, /* Tx Data be buffer 2bit ecc error */
    PCIE_RAS_AP_IOB_TX_DATA_BE_SBECC = 632, /* Tx Data be buffer 1bit ecc error */
    PCIE_RAS_AP_IOB_TX_DATA_MBECC = 633, /* Tx Data buffer 2bit ecc error */
    PCIE_RAS_AP_IOB_TX_DATA_SBECC = 634, /* Tx Data buffer 1bit ecc error */
    PCIE_RAS_AP_IOB_TX_HEAD_MBECC = 635, /* Tx Head buffer 2bit ecc error */
    PCIE_RAS_AP_IOB_TX_HEAD_SBECC = 636, /* Tx Head buffer 1bit ecc error */
    PCIE_RAS_AP_IOB_TX_REQ_TMOUT = 637, /* Tx Cfg0 Request timeout */
    PCIE_RAS_AP_IOB_TX_COMPLET_TMOUT = 638, /* Tx Completion stuck timeout */

    PCIE_RAS_TL_RX_BUF_WR_FULL = 650, /* TL RX buffer fifo overflow */
    PCIE_RAS_TL_PF_VF_NUM_ERR = 651, /* For RC mode, PF VF number of TLP is error */
    PCIE_RAS_TL_VC0_CPL_TC_ERR = 652, /* TL TX VC0 CPL, TC map VC is error */
    PCIE_RAS_TL_VC0_NP_TC_ERR = 653, /* TL TX VC0 NP, TC map VC is error */
    PCIE_RAS_TL_VC0_P_TC_ERR = 654, /* TL TX VC0 P, TC map VC is error */
    PCIE_RAS_TL_BUS_MASTER_ERR = 655, /* bus master enable is invalid */
    PCIE_RAS_TL_PFX_MAL_ERR = 656, /* tl rx receives a tlp is all prefix without header */
    PCIE_RAS_TL_CPL_CRED_OVF = 657, /* tl rx cpl/cpld credit overflow */

    PCIE_RAS_MAC_TRACER_MBECC = 701, /* ltssm tracer sram ecc 2bit err */
    PCIE_RAS_MAC_TRACER_SBECC = 702, /* ltssm tracer sram ecc 1bit err */
    PCIE_RAS_MAC_ENTER_L1_TMOUT = 703, /* enter L1 timeout */
    PCIE_RAS_MAC_DESKEW_OVRF = 704, /* deskew fifo overflow */
    PCIE_RAS_MAC_SYMBOL_UNLOCKED = 705, /* symbol unlocked */
    PCIE_RAS_MAC_DESKEW_UNLOCKED = 706, /* deskew unlocked */
    PCIE_RAS_MAC_LINKDOWN = 710, /* linkdown */

    PCIE_RAS_DISP_DATA_RAM_MBECC = 1001, /* disp data ram 2bit ECC Error */
    PCIE_RAS_DISP_COMMAND_RAM_MBECC = 1002, /* disp command ram 2bit ECC Error */
    PCIE_RAS_DISP_NCDIR_MBECC = 1003, /* disp ncdir 2bit ECC Error */
    PCIE_RAS_DISP_DATA_RAM_SBECC = 1004, /* disp data ram 1bit ECC Error */
    PCIE_RAS_DISP_CMD_RAM_SBECC = 1005, /* disp command ram 1bit ECC Error */
    PCIE_RAS_DISP_NCDIR_SBECC = 1006, /* disp ncdir 1bit ECC Error */
    PCIE_RAS_DISP_ERR_RSP_PORT_0 = 1007, /* disp port 0 error response */
    PCIE_RAS_DISP_ERR_RSP_PORT_1 = 1008, /* disp port 1 error response */
    PCIE_RAS_DISP_ERR_RSP_PORT_2 = 1009, /* disp port 2 error response */
    PCIE_RAS_DISP_ERR_RSP_PORT_3 = 1010, /* disp port 3 error response */
    PCIE_RAS_DISP_ERR_RSP_PORT_4 = 1011, /* disp port 4 error response */
    PCIE_RAS_DISP_ERR_RSP_PORT_5 = 1012, /* disp port 5 error response */
    PCIE_RAS_DISP_ERR_RSP_PORT_6 = 1013, /* disp port 6 error response */
    PCIE_RAS_DISP_ERR_RSP_PORT_7 = 1014, /* disp port 7 error response */
    PCIE_RAS_DISP_ERR_REP = 1015, /* disp error request */
    PCIE_RAS_DISP_ERR_WD = 1016, /* disp write data error */

    PCIE_RAS_SMMU_STE_FETCH_ERR = 1101, /* smmu ste_fetch error */
    PCIE_RAS_SMMU_CD_FETCH_ERR = 1102, /* smmu cd_fetch error */
    PCIE_RAS_SMMU_WALK_EABT_ERR = 1103, /* smmu walk_eabt error */
    PCIE_RAS_SMMU_CMDQ_FETCH_ERR = 1104, /* smmu cmdq fetch error */
    PCIE_RAS_SMMU_WR_EVENTTQ_TMOUT = 1105, /* smmu write eventq abort error */
    PCIE_RAS_SMMU_WR_PRIQ_TMOUT = 1106, /* smmu write priq abort error */
    PCIE_RAS_SMMU_VMS_FETCH_ERR = 1109, /* smmu vms fetch error */
    PCIE_RAS_SMMU_INNER_RAM_SBECC = 1111, /* smmu inner ram 1bit ECC error */
    PCIE_RAS_SMMU_INNER_RAM_MBECC = 1112, /* smmu inner ram 2bit ECC error */
};

#define PCIE_LOCAL 0
#define PCIE_COMMON 1

#define IERR_MASK 0x0000FF00 /* ras error code mask for dispatch/smmu/aa */

#define DISP_DATA_RAM_MBECC (0x1 << 8)
#define DISP_CMD_RAM_MBECC (0x2 << 8)
#define DISP_NCDIR_MBECC (0x3 << 8)
#define DISP_DATA_RAM_SBECC (0x4 << 8)
#define DISP_CMD_RAM_SBECC (0x5 << 8)
#define DISP_NCDIR_SBECC (0x6 << 8)
#define DISP_ERR_RSP_PORT_0 (0x7 << 8)
#define DISP_ERR_RSP_PORT_1 (0x8 << 8)
#define DISP_ERR_RSP_PORT_2 (0x9 << 8)
#define DISP_ERR_RSP_PORT_3 (0xa << 8)
#define DISP_ERR_RSP_PORT_4 (0xb << 8)
#define DISP_ERR_RSP_PORT_5 (0xc << 8)
#define DISP_ERR_RSP_PORT_6 (0xd << 8)
#define DISP_ERR_RSP_PORT_7 (0xe << 8)
#define DISP_ERR_REP (0xf << 8)
#define DISP_ERR_WD (0x15 << 8)

#define SMMU_STE_FETCH_ERR (1U << 8)
#define SMMU_CD_FETCH_ERR (2U << 8)
#define SMMU_WALK_EABT_ERR (3U << 8)
#define SMMU_CMDQ_FETCH_ERR (4U << 8)
#define SMMU_WR_EVENTTQ_TMOUT (5U << 8)
#define SMMU_WR_PRIQ_TMOUT (6U << 8)
#define SMMU_AXI_TMOUT (7U << 8)
#define SMMU_SKYROS_TMOUT (8U << 8)
#define SMMU_VMS_FETCH_ERR (9U << 8)

enum pcie_local_sub_module {
    PCIE_RAS_AP_RBA = 0,
    PCIE_RAS_AP_APAT = 1,
    PCIE_RAS_AP_IOB_RX = 2,
    PCIE_RAS_AP_IOB_TX = 3,
    PCIE_RAS_AP_TOP = 4,
    PCIE_RAS_AP_DMA = 5,
    PCIE_RAS_AP_NVME = 6,
    PCIE_RAS_AP_INT = 7,
    PCIE_RAS_AP_GLOBAL = 8,
    PCIE_RAS_AP_PMT = 9,
    PCIE_RAS_TL_CORE = 10,
    PCIE_RAS_TL = 11,
    PCIE_RAS_MAC = 12,
    PCIE_RAS_DL = 13,
    PCIE_RAS_HILINK = 14,
    PCIE_RAS_PCS = 15,
    PCIE_RAS_SUB_MODULE_MAX
};

#ifndef DRV_SOC_MISC_UT
#define PCIE_RAS_SCHEDULE_UNMASK_TIME 300000 /* schedule after 5 min */
#else
#define PCIE_RAS_SCHEDULE_UNMASK_TIME 0
#endif

#define PCIE_RAS_DRV_QUERY_ERROR_TIME 5000 /* driver query ras error every 5s */

#define PCIE_RAS_SERVERITY_NFE 0
#define PCIE_RAS_SERVERITY_FE 1
#define PCIE_RAS_SERVERITY_CE 2

/* offset between devices */
#define DEVICE_OFFSET SOC_CHIP_OFFSET
#define PCIE_RAS_REG_SIZE 0x4

#define PCIE_RAS_AP_MG_REG_BASE 0x8000
#define PCIE_RAS_AP_MG_PCIE_CE_ENA_OFFSET 0x8
#define PCIE_RAS_AP_MG_PCIE_UNF_ENA_OFFSET 0x10
#define PCIE_RAS_AP_IOB_TX_REG_BASE 0x0
#define PCIE_RAS_AP_IOB_TX_UR_CA_OFFSET 0x2C0C
#define PCIE_RAS_MAC_REG_BASE 0x70000
#define PCIE_RAS_MAC_INT_STATUS_OFFSET 0x54

#define PCIE_RAS_CHIP_ID_INFO_ADDR 0x36EFD800 /* 0 is p0, 1 is p1,...7 is p7 */
#define PCIE_RAS_MAX_CHIP_ID 8 /* max is 8p */


#define PCIE_RAS_CORE_GLOBAL_REG_BASE   (0x80000*2)
#define PCIE_RAS_CORE_GLOBAL_CORE_INT_CE_MSK_0 0x104
#define PCIE_RAS_CORE_GLOBAL_CORE_INT_NFE_MSK_0 0x124

#define PCIE_RAS_MAX_REPORT_NUM 32U

int soc_misc_pcie_ops_init(struct dms_node *device);
void soc_misc_pcie_ops_uninit(struct dms_node *device);
void soc_misc_pcie_local_unmask_task(struct work_struct *work);
void soc_misc_pcie_hisi_comm_unmask_task(struct work_struct *work);
void soc_misc_pcie_fault_handler(const struct notify_data *pdata);
int soc_misc_pcie_sensor_scan(unsigned long long private_data, struct dms_sensor_event_data *data);
int soc_misc_pcie_disp_sensor_scan(unsigned long long private_data, struct dms_sensor_event_data *data);
void soc_misc_pcie_query_ras_error(struct work_struct *work);
void soc_misc_linkdown_fault_post_handle(void);

#endif
