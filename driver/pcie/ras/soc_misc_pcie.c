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

#include <linux/io.h>

#ifdef CFG_FEATURE_DRIVER_QUERY_RAS_ERROR
#include "devdrv_interface.h"
#endif

#include "soc_misc_err_info.h"
#include "soc_misc_init.h"
#include "soc_misc_fault.h"
#include "dms_sensor_type.h"
#include "receiver/fpdc_ras_receiver.h"
#include "drv_ras_common.h"
#include "soc_spec.h"
#include "soc_misc_pcie.h"

#define DEVDRV_SET_CHANNEL_STATUS_FN_NAME "devdrv_set_pcie_channel_status"
typedef void (*devdrv_set_channel_status_fn)(u32);
STATIC devdrv_set_channel_status_fn g_devdrv_set_channel_status_fn = NULL;

STATIC const struct ras_fault_converge_item g_pcie_converge_table[] = {
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_AP_COREX_TX_CPL_MBECC, "multi-bit ECC error", RAS_ERROR_TYPE_MBECC),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_AP_COREX_TX_CPL_SBECC, "single-bit ECC error", RAS_ERROR_TYPE_SBECCOverThold),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_AP_P2P_ADDR_UNMATCH, "P2P addr unmatch", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_AP_CHI_CPU_RAM_MBECC, "multi-bit ECC error", RAS_ERROR_TYPE_MBECC),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_AP_CHI_CPU_RAM_SBECC, "single-bit ECC error", RAS_ERROR_TYPE_SBECCOverThold),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_AP_CHI_P2P_RAM_MBECC, "multi-bit ECC error", RAS_ERROR_TYPE_MBECC),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_AP_CHI_P2P_RAM_SBECC, "single-bit ECC error", RAS_ERROR_TYPE_SBECCOverThold),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_AP_CHI_UNEXPECT_TRANS, "chi unexpect trans", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_AP_IOB_ODR_ERR_BRESP, "iob odr err b resp", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_AP_IOB_ODR_POISON_RDATA, "iob odr poison rdata", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_AP_IOB_ODR_ERR_RRESP, "iob odr err r resp", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_AP_IOB_ODR_PCPL_SBM_MBECC, "multi-bit ECC error", RAS_ERROR_TYPE_MBECC),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_AP_IOB_ODR_PCPL_SBM_SBECC, "single-bit ECC error", RAS_ERROR_TYPE_SBECCOverThold),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_AP_IOB_ODR_NPQ_SBM_MBECC, "multi-bit ECC error", RAS_ERROR_TYPE_MBECC),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_AP_IOB_ODR_NPQ_SBM_SBECC, "single-bit ECC error", RAS_ERROR_TYPE_SBECCOverThold),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_AP_IOB_AXIM_ERR_BRESP, "iob axim err b resp", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_AP_IOB_AXIM_POISON_RDATA, "iob axim poison rdata", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_AP_IOB_AXIM_ERR_RRESP, "iob axim err r resp", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_AP_IOB_AXIM_PCPL_SBM_MBECC, "multi-bit ECC error", RAS_ERROR_TYPE_MBECC),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_AP_IOB_AXIM_PCPL_SBM_SBECC, "single-bit ECC error", RAS_ERROR_TYPE_SBECCOverThold),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_AP_IOB_AXIM_NPQ_SBM_MBECC, "multi-bit ECC error", RAS_ERROR_TYPE_MBECC),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_AP_IOB_AXIM_NPQ_SBM_SBECC, "single-bit ECC error", RAS_ERROR_TYPE_SBECCOverThold),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_AP_IOB_AXIM_RD_TRANS_TMOUT, "axim read transaction Timeout", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_AP_IOB_AXIM_WR_TRANS_TMOUT, "axim write transaction Timeout", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_DMA_LINKDOWN, "dma linkdown", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_DMA_DATA_POISON_REMOTE, "dma data poison remote", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_DMA_DATA_POISON_SQ_READ, "dma data poison sq read", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_DMA_CQ_FULL, "dma cq full", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_DMA_CQ_WRITE_BACK_ERR, "dma cq write back err", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_DMA_DATA_POISON_LOCAL, "dma data poison local", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_DMA_AXI_WRITE_RESPONSE_ERR, "dma axi write response err", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_DMA_AXI_READ_RESPONSE_ERR, "dma axi read response err", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_DMA_READ_RESPONSE_REMOTE, "dma read response remote", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_DMA_WRITE_RESPONSE_REMOTE, "dma write response remote", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_DMA_DROP_OCCUR, "dma drop occur", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_DMA_INVALID_LENGTH_FIELD, "dma invalid length field", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_DMA_INVALID_OPCODE, "dma data invalid opcode", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_DMA_SQBD_READ_RESPONSE_ERR, "dma sqbd read response err", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_DMA_REMOTE_URCA_ERR, "dma remote urca err", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_INT_SRC0_VEC_ERR, "int src0 vecter err", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_INT_SRC1_VEC_ERR, "int src1 vecter err", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_INT_SRC2_VEC_ERR, "int src2 vecter err", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_MSI_COAL_MBECC, "multi-bit ECC error", RAS_ERROR_TYPE_MBECC),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_MSI_COAL_SBECC, "single-bit ECC error", RAS_ERROR_TYPE_SBECCOverThold),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_PTR_TABLE_MBECC, "multi-bit ECC error", RAS_ERROR_TYPE_MBECC),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_PTR_TABLE_SBECC, "single-bit ECC error", RAS_ERROR_TYPE_SBECCOverThold),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_MSIX_COAL_MBECC, "multi-bit ECC error", RAS_ERROR_TYPE_MBECC),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_MSIX_COAL_SBECC, "single-bit ECC error", RAS_ERROR_TYPE_SBECCOverThold),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_MSIX_TABLE_PBA_MBECC, "multi-bit ECC error", RAS_ERROR_TYPE_MBECC),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_MSIX_TABLE_PBA_SBECC, "single-bit ECC error", RAS_ERROR_TYPE_SBECCOverThold),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_DL_VC1_CPL_UPDATE_TIMEOUT, "dl vc1 cpl update timeout", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_DL_VC1_NP_UPDATE_TIMEOUT, "dl vc1 np update timeout", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_DL_VC1_P_UPDATE_TIMEOUT, "dl vc1 p update timeout", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_DL_VC0_CPL_UPDATE_TIMEOUT, "dl vc0 cpl update timeout", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_DL_VC0_NP_UPDATE_TIMEOUT, "dl vc0 np update timeout", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_DL_VC0_P_UPDATE_TIMEOUT, "dl vc0 p update timeout", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_DL_COR_ERR_TIMEOUT, "dl correct err cnt timeout", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_DL_FC_UPDATE_TIMEOUT, "dl fc update timeout", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_DL_MAC_RETRAIN_CNT_OVER, "dl mac retrain cnt over", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_DL_TL_CREDIT_NULL_TIMEOUT, "dl tl credit null timeout", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_DL_NAK_TIMER_TIMEOUT, "dl nak timer timeout", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_DL_DUP_TLP_ACK_CNT_ROLLOVER, "dl dup tlp ack cnt rollover", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_DL_MAC_LINKDOWN, "dl mac linkdown", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_DL_RETRAIN, "dl mac retrain", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_DL_MBECC, "multi-bit ECC error", RAS_ERROR_TYPE_MBECC),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_DL_SBECC, "single-bit ECC error", RAS_ERROR_TYPE_SBECCOverThold),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_DL_INIT_TIMEOUT, "dl init timeout", RAS_ERROR_TYPE_ERROR),

    PCIE_DISP_RAS_COVERGE_NODE(PCIE_RAS_DISP_DATA_RAM_MBECC, "disp data ram 2bit ECC Error", RAS_ERROR_TYPE_MBECC),
    PCIE_DISP_RAS_COVERGE_NODE(PCIE_RAS_DISP_COMMAND_RAM_MBECC, "disp cmd ram 2bit ECC Error", RAS_ERROR_TYPE_MBECC),
    PCIE_DISP_RAS_COVERGE_NODE(PCIE_RAS_DISP_NCDIR_MBECC, "disp ncdir 2bit ECC Error", RAS_ERROR_TYPE_MBECC),
    PCIE_DISP_RAS_COVERGE_NODE(PCIE_RAS_DISP_DATA_RAM_SBECC, "disp data ram 1bit ECC", RAS_ERROR_TYPE_SBECCOverThold),
    PCIE_DISP_RAS_COVERGE_NODE(PCIE_RAS_DISP_CMD_RAM_SBECC, "disp cmd ram 1bit ECC", RAS_ERROR_TYPE_SBECCOverThold),
    PCIE_DISP_RAS_COVERGE_NODE(PCIE_RAS_DISP_NCDIR_SBECC, "disp ncdir 1bit ECC Error", RAS_ERROR_TYPE_SBECCOverThold),
    PCIE_DISP_RAS_COVERGE_NODE(PCIE_RAS_DISP_ERR_RSP_PORT_0, "disp port 0 error response", RAS_ERROR_TYPE_INPUT_ERR),
    PCIE_DISP_RAS_COVERGE_NODE(PCIE_RAS_DISP_ERR_RSP_PORT_1, "disp port 1 error response", RAS_ERROR_TYPE_INPUT_ERR),
    PCIE_DISP_RAS_COVERGE_NODE(PCIE_RAS_DISP_ERR_RSP_PORT_2, "disp port 2 error response", RAS_ERROR_TYPE_INPUT_ERR),
    PCIE_DISP_RAS_COVERGE_NODE(PCIE_RAS_DISP_ERR_RSP_PORT_3, "disp port 3 error response", RAS_ERROR_TYPE_INPUT_ERR),
    PCIE_DISP_RAS_COVERGE_NODE(PCIE_RAS_DISP_ERR_RSP_PORT_4, "disp port 4 error response", RAS_ERROR_TYPE_INPUT_ERR),
    PCIE_DISP_RAS_COVERGE_NODE(PCIE_RAS_DISP_ERR_RSP_PORT_5, "disp port 5 error response", RAS_ERROR_TYPE_INPUT_ERR),
    PCIE_DISP_RAS_COVERGE_NODE(PCIE_RAS_DISP_ERR_RSP_PORT_6, "disp port 6 error response", RAS_ERROR_TYPE_INPUT_ERR),
    PCIE_DISP_RAS_COVERGE_NODE(PCIE_RAS_DISP_ERR_RSP_PORT_7, "disp port 7 error response", RAS_ERROR_TYPE_INPUT_ERR),
    PCIE_DISP_RAS_COVERGE_NODE(PCIE_RAS_DISP_ERR_REP, "disp error request", RAS_ERROR_TYPE_CFG_ERR),
    PCIE_DISP_RAS_COVERGE_NODE(PCIE_RAS_DISP_ERR_WD, "disp write data error", RAS_ERROR_TYPE_INPUT_ERR),

    PCIE_RAS_COVERGE_NODE(PCIE_RAS_SMMU_STE_FETCH_ERR, "smmu ste_fetch error", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_SMMU_CD_FETCH_ERR, "smmu cd_fetch error", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_SMMU_WALK_EABT_ERR, "smmu walk_eabt error", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_SMMU_CMDQ_FETCH_ERR, "smmu cmdq fetch error", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_SMMU_WR_EVENTTQ_TMOUT, "smmu write eventq abort error", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_SMMU_WR_PRIQ_TMOUT, "smmu write priq abort error", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_SMMU_VMS_FETCH_ERR, "smmu vms fetch error", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_SMMU_INNER_RAM_SBECC, "smmu inner ram 1bit ECC err", RAS_ERROR_TYPE_SBECCOverThold),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_SMMU_INNER_RAM_MBECC, "smmu inner ram 2bit ECC error", RAS_ERROR_TYPE_MBECC),

    PCIE_RAS_COVERGE_NODE(PCIE_RAS_AP_IOB_TX_NP_TMOUT, "Tx NonPost Request timeout", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_AP_IOB_TX_P_TMOUT, "Tx Post Request timeout", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_AP_IOB_TX_DATA_BE_MBECC, "Tx Data be buffer 2bit ecc error", RAS_ERROR_TYPE_MBECC),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_AP_IOB_TX_DATA_BE_SBECC, "Tx Data be buff 1bit ecc", RAS_ERROR_TYPE_SBECCOverThold),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_AP_IOB_TX_DATA_MBECC, "Tx Data buffer 2bit ecc error", RAS_ERROR_TYPE_MBECC),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_AP_IOB_TX_DATA_SBECC, "Tx Data buffer 1bit ecc", RAS_ERROR_TYPE_SBECCOverThold),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_AP_IOB_TX_HEAD_MBECC, "Tx Head buffer 2bit ecc error", RAS_ERROR_TYPE_MBECC),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_AP_IOB_TX_HEAD_SBECC, "Tx Head buffer 1bit ecc", RAS_ERROR_TYPE_SBECCOverThold),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_AP_IOB_TX_REQ_TMOUT, "Tx Cfg0 Request timeout", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_AP_IOB_TX_COMPLET_TMOUT, "Tx Completion Transmit timeout", RAS_ERROR_TYPE_ERROR),

    PCIE_RAS_COVERGE_NODE(PCIE_RAS_TL_RX_BUF_WR_FULL, "TL RX buffer fifo overflow", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_TL_PF_VF_NUM_ERR, "PF VF number of TLP is error", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_TL_VC0_CPL_TC_ERR, "TL TX VC0 CPL TC map VC is error", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_TL_VC0_NP_TC_ERR, "TL TX VC0 NP TC map VC is error", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_TL_VC0_P_TC_ERR, "TL TX VC0 P TC map VC is error", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_TL_BUS_MASTER_ERR, "bus master enable is invalid", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_TL_PFX_MAL_ERR, "tl rx tlp prefix without header", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_TL_CPL_CRED_OVF, "tl rx cpl/cpld credit overflow", RAS_ERROR_TYPE_ERROR),

    PCIE_RAS_COVERGE_NODE(PCIE_RAS_MAC_TRACER_MBECC, "sram ecc 2bit err", RAS_ERROR_TYPE_MBECC),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_MAC_TRACER_SBECC, "sram ecc 1bit err", RAS_ERROR_TYPE_SBECCOverThold),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_MAC_ENTER_L1_TMOUT, "enter L1 timeout", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_MAC_DESKEW_OVRF, "deskew fifo overflow", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_MAC_SYMBOL_UNLOCKED, "symbol unlocked", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_MAC_DESKEW_UNLOCKED, "deskew unlocked", RAS_ERROR_TYPE_ERROR),
    PCIE_RAS_COVERGE_NODE(PCIE_RAS_MAC_LINKDOWN, "linkdown", RAS_ERROR_TYPE_ERROR),
};

STATIC const struct pcie_local_ras_event g_pcie_dl_event[] = {
    PCIE_HISI_COMMON_RAS(DL_INT_STATUS, BIT(10), PCIE_RAS_DL_COR_ERR_TIMEOUT, PCIE_PRINT),
    PCIE_HISI_COMMON_RAS(DL_INT_STATUS, BIT(9), PCIE_RAS_DL_FC_UPDATE_TIMEOUT, PCIE_PRINT),
    PCIE_HISI_COMMON_RAS(DL_INT_STATUS, BIT(8), PCIE_RAS_DL_MAC_RETRAIN_CNT_OVER, PCIE_PRINT),
    PCIE_HISI_COMMON_RAS(DL_INT_STATUS, BIT(7), PCIE_RAS_DL_TL_CREDIT_NULL_TIMEOUT, PCIE_PRINT),
    PCIE_HISI_COMMON_RAS(DL_INT_STATUS, BIT(6), PCIE_RAS_DL_NAK_TIMER_TIMEOUT, PCIE_PRINT),
    PCIE_HISI_COMMON_RAS(DL_INT_STATUS, BIT(5), PCIE_RAS_DL_DUP_TLP_ACK_CNT_ROLLOVER, PCIE_PRINT),
    PCIE_HISI_COMMON_RAS(DL_INT_STATUS, BIT(4), PCIE_RAS_DL_MAC_LINKDOWN, PCIE_PRINT),
    PCIE_HISI_COMMON_RAS(DL_INT_STATUS, BIT(3), PCIE_RAS_DL_RETRAIN, PCIE_PRINT),
    PCIE_HISI_COMMON_RAS(DL_INT_STATUS, BIT(2), PCIE_RAS_DL_MBECC, PCIE_REPORT),
    PCIE_HISI_COMMON_RAS(DL_INT_STATUS, BIT(1), PCIE_RAS_DL_SBECC, PCIE_PRINT),
    PCIE_HISI_COMMON_RAS(DL_INT_STATUS, BIT(0), PCIE_RAS_DL_INIT_TIMEOUT, PCIE_REPORT),
};

STATIC const struct pcie_local_ras_event g_pcie_mac_event[] = {
    PCIE_HISI_COMMON_RAS(MAC_REG_MAC_INT_STATUS, BIT(23), PCIE_RAS_MAC_TRACER_MBECC, PCIE_PRINT),
    PCIE_HISI_COMMON_RAS(MAC_REG_MAC_INT_STATUS, BIT(22), PCIE_RAS_MAC_TRACER_SBECC, PCIE_PRINT),
    PCIE_HISI_COMMON_RAS(MAC_REG_MAC_INT_STATUS, BIT(13), PCIE_RAS_MAC_ENTER_L1_TMOUT, PCIE_PRINT),
    PCIE_HISI_COMMON_RAS(MAC_REG_MAC_INT_STATUS, BIT(12), PCIE_RAS_MAC_DESKEW_OVRF, PCIE_PRINT),
    PCIE_HISI_COMMON_RAS(MAC_REG_MAC_INT_STATUS, BIT(10), PCIE_RAS_MAC_SYMBOL_UNLOCKED, PCIE_PRINT),
    PCIE_HISI_COMMON_RAS(MAC_REG_MAC_INT_STATUS, BIT(9), PCIE_RAS_MAC_DESKEW_UNLOCKED, PCIE_PRINT),
    PCIE_HISI_COMMON_RAS(MAC_REG_MAC_INT_STATUS, BIT(1), PCIE_RAS_MAC_LINKDOWN, PCIE_REPORT),
};

STATIC const struct pcie_local_ras_event g_pcie_tl_event[] = {
    PCIE_HISI_COMMON_RAS(TL_INT_STATUS0, BIT(12), PCIE_RAS_TL_RX_BUF_WR_FULL, PCIE_REPORT),
    PCIE_HISI_COMMON_RAS(TL_INT_STATUS0, BIT(10), PCIE_RAS_TL_PF_VF_NUM_ERR, PCIE_REPORT),
    PCIE_HISI_COMMON_RAS(TL_INT_STATUS0, BIT(8), PCIE_RAS_TL_VC0_CPL_TC_ERR, PCIE_REPORT),
    PCIE_HISI_COMMON_RAS(TL_INT_STATUS0, BIT(7), PCIE_RAS_TL_VC0_NP_TC_ERR, PCIE_REPORT),
    PCIE_HISI_COMMON_RAS(TL_INT_STATUS0, BIT(6), PCIE_RAS_TL_VC0_P_TC_ERR, PCIE_REPORT),
    PCIE_HISI_COMMON_RAS(TL_INT_STATUS0, BIT(5), PCIE_RAS_TL_BUS_MASTER_ERR, PCIE_PRINT),
    PCIE_HISI_COMMON_RAS(TL_INT_STATUS0, BIT(4), PCIE_RAS_TL_PFX_MAL_ERR, PCIE_PRINT),
    PCIE_HISI_COMMON_RAS(TL_INT_STATUS0, BIT(3), PCIE_RAS_TL_CPL_CRED_OVF, PCIE_REPORT),
};

STATIC const struct pcie_local_ras_event g_pcie_ap_int_event[] = {
    PCIE_HISI_COMMON_RAS(ABNORMAL_INT_STS, BIT(2), PCIE_RAS_INT_SRC2_VEC_ERR, PCIE_PRINT),
    PCIE_HISI_COMMON_RAS(ABNORMAL_INT_STS, BIT(1), PCIE_RAS_INT_SRC1_VEC_ERR, PCIE_PRINT),
    PCIE_HISI_COMMON_RAS(ABNORMAL_INT_STS, BIT(0), PCIE_RAS_INT_SRC0_VEC_ERR, PCIE_PRINT),
    PCIE_HISI_COMMON_RAS(ECC_ERR_INT_SRC_STS, BIT(7), PCIE_RAS_MSI_COAL_MBECC, PCIE_REPORT),
    PCIE_HISI_COMMON_RAS(ECC_ERR_INT_SRC_STS, BIT(6), PCIE_RAS_MSI_COAL_SBECC, PCIE_PRINT),
    PCIE_HISI_COMMON_RAS(ECC_ERR_INT_SRC_STS, BIT(5), PCIE_RAS_PTR_TABLE_MBECC, PCIE_REPORT),
    PCIE_HISI_COMMON_RAS(ECC_ERR_INT_SRC_STS, BIT(4), PCIE_RAS_PTR_TABLE_SBECC, PCIE_PRINT),
    PCIE_HISI_COMMON_RAS(ECC_ERR_INT_SRC_STS, BIT(3), PCIE_RAS_MSIX_COAL_MBECC, PCIE_REPORT),
    PCIE_HISI_COMMON_RAS(ECC_ERR_INT_SRC_STS, BIT(2), PCIE_RAS_MSIX_COAL_SBECC, PCIE_PRINT),
    PCIE_HISI_COMMON_RAS(ECC_ERR_INT_SRC_STS, BIT(1), PCIE_RAS_MSIX_TABLE_PBA_MBECC, PCIE_REPORT),
    PCIE_HISI_COMMON_RAS(ECC_ERR_INT_SRC_STS, BIT(0), PCIE_RAS_MSIX_TABLE_PBA_SBECC, PCIE_PRINT),
};

STATIC const struct pcie_local_ras_event g_pcie_ap_iob_rx_event[] = {
    PCIE_HISI_COMMON_RAS(IOB_RX_INT_STATUS, BIT(0), PCIE_RAS_AP_IOB_AXIM_ERR_RRESP, PCIE_PRINT),
    PCIE_HISI_COMMON_RAS(IOB_RX_INT_STATUS, BIT(1), PCIE_RAS_AP_IOB_AXIM_ERR_BRESP, PCIE_PRINT),
    PCIE_HISI_COMMON_RAS(IOB_RX_INT_STATUS, BIT(2), PCIE_RAS_AP_IOB_AXIM_POISON_RDATA, PCIE_PRINT),
    PCIE_HISI_COMMON_RAS(IOB_RX_INT_STATUS, BIT(3), PCIE_RAS_AP_IOB_AXIM_RD_TRANS_TMOUT, PCIE_PRINT),
    PCIE_HISI_COMMON_RAS(IOB_RX_INT_STATUS, BIT(4), PCIE_RAS_AP_IOB_AXIM_WR_TRANS_TMOUT, PCIE_PRINT),
};

STATIC const struct pcie_local_ras_event g_pcie_ap_iob_tx_event[] = {
    PCIE_HISI_COMMON_RAS(IOB_TX_INT_STATUS2, GENMASK(31, 16), PCIE_RAS_AP_IOB_TX_NP_TMOUT, PCIE_PRINT),
    PCIE_HISI_COMMON_RAS(IOB_TX_INT_STATUS2, GENMASK(15, 0), PCIE_RAS_AP_IOB_TX_P_TMOUT, PCIE_PRINT),
    PCIE_HISI_COMMON_RAS(IOB_TX_INT_STATUS3, BIT(23), PCIE_RAS_AP_IOB_TX_DATA_MBECC, PCIE_REPORT),
    PCIE_HISI_COMMON_RAS(IOB_TX_INT_STATUS3, BIT(22), PCIE_RAS_AP_IOB_TX_HEAD_MBECC, PCIE_REPORT),
    PCIE_HISI_COMMON_RAS(IOB_TX_INT_STATUS3, BIT(21), PCIE_RAS_AP_IOB_TX_DATA_SBECC, PCIE_PRINT),
    PCIE_HISI_COMMON_RAS(IOB_TX_INT_STATUS3, BIT(20), PCIE_RAS_AP_IOB_TX_HEAD_SBECC, PCIE_PRINT),
    PCIE_HISI_COMMON_RAS(IOB_TX_INT_STATUS3, BIT(16), PCIE_RAS_AP_IOB_TX_REQ_TMOUT, PCIE_REPORT),
    PCIE_HISI_COMMON_RAS(IOB_TX_INT_STATUS3, GENMASK(15, 0), PCIE_RAS_AP_IOB_TX_COMPLET_TMOUT, PCIE_PRINT),
};

STATIC const struct pcie_local_ras_event g_pcie_disp_event[] = {
    PCIE_HISI_COMMON_RAS(DISP_ERR_STATUSL, DISP_DATA_RAM_MBECC, PCIE_RAS_DISP_DATA_RAM_MBECC, PCIE_REPORT),
    PCIE_HISI_COMMON_RAS(DISP_ERR_STATUSL, DISP_CMD_RAM_MBECC, PCIE_RAS_DISP_COMMAND_RAM_MBECC, PCIE_REPORT),
    PCIE_HISI_COMMON_RAS(DISP_ERR_STATUSL, DISP_NCDIR_MBECC, PCIE_RAS_DISP_NCDIR_MBECC, PCIE_REPORT),
    PCIE_HISI_COMMON_RAS(DISP_ERR_STATUSL, DISP_DATA_RAM_SBECC, PCIE_RAS_DISP_DATA_RAM_SBECC, PCIE_REPORT),
    PCIE_HISI_COMMON_RAS(DISP_ERR_STATUSL, DISP_CMD_RAM_SBECC, PCIE_RAS_DISP_CMD_RAM_SBECC, PCIE_REPORT),
    PCIE_HISI_COMMON_RAS(DISP_ERR_STATUSL, DISP_NCDIR_SBECC, PCIE_RAS_DISP_NCDIR_SBECC, PCIE_REPORT),
    PCIE_HISI_COMMON_RAS(DISP_ERR_STATUSL, DISP_ERR_RSP_PORT_0, PCIE_RAS_DISP_ERR_RSP_PORT_0, PCIE_REPORT),
    PCIE_HISI_COMMON_RAS(DISP_ERR_STATUSL, DISP_ERR_RSP_PORT_1, PCIE_RAS_DISP_ERR_RSP_PORT_1, PCIE_REPORT),
    PCIE_HISI_COMMON_RAS(DISP_ERR_STATUSL, DISP_ERR_RSP_PORT_2, PCIE_RAS_DISP_ERR_RSP_PORT_2, PCIE_REPORT),
    PCIE_HISI_COMMON_RAS(DISP_ERR_STATUSL, DISP_ERR_RSP_PORT_3, PCIE_RAS_DISP_ERR_RSP_PORT_3, PCIE_REPORT),
    PCIE_HISI_COMMON_RAS(DISP_ERR_STATUSL, DISP_ERR_RSP_PORT_4, PCIE_RAS_DISP_ERR_RSP_PORT_4, PCIE_REPORT),
    PCIE_HISI_COMMON_RAS(DISP_ERR_STATUSL, DISP_ERR_RSP_PORT_5, PCIE_RAS_DISP_ERR_RSP_PORT_5, PCIE_REPORT),
    PCIE_HISI_COMMON_RAS(DISP_ERR_STATUSL, DISP_ERR_RSP_PORT_6, PCIE_RAS_DISP_ERR_RSP_PORT_6, PCIE_REPORT),
    PCIE_HISI_COMMON_RAS(DISP_ERR_STATUSL, DISP_ERR_RSP_PORT_7, PCIE_RAS_DISP_ERR_RSP_PORT_7, PCIE_REPORT),
    PCIE_HISI_COMMON_RAS(DISP_ERR_STATUSL, DISP_ERR_REP, PCIE_RAS_DISP_ERR_REP, PCIE_REPORT),
    PCIE_HISI_COMMON_RAS(DISP_ERR_STATUSL, DISP_ERR_WD, PCIE_RAS_DISP_ERR_WD, PCIE_REPORT),
};

STATIC const struct pcie_local_ras_event g_pcie_smmu_event[] = {
    PCIE_HISI_COMMON_RAS(SMMU_ERR_STATUS_0, SMMU_STE_FETCH_ERR, PCIE_RAS_SMMU_STE_FETCH_ERR, PCIE_REPORT),
    PCIE_HISI_COMMON_RAS(SMMU_ERR_STATUS_0, SMMU_CD_FETCH_ERR, PCIE_RAS_SMMU_CD_FETCH_ERR, PCIE_REPORT),
    PCIE_HISI_COMMON_RAS(SMMU_ERR_STATUS_0, SMMU_WALK_EABT_ERR, PCIE_RAS_SMMU_WALK_EABT_ERR, PCIE_REPORT),
    PCIE_HISI_COMMON_RAS(SMMU_ERR_STATUS_0, SMMU_CMDQ_FETCH_ERR, PCIE_RAS_SMMU_CMDQ_FETCH_ERR, PCIE_REPORT),
    PCIE_HISI_COMMON_RAS(SMMU_ERR_STATUS_0, SMMU_WR_EVENTTQ_TMOUT, PCIE_RAS_SMMU_WR_EVENTTQ_TMOUT, PCIE_REPORT),
    PCIE_HISI_COMMON_RAS(SMMU_ERR_STATUS_0, SMMU_WR_PRIQ_TMOUT, PCIE_RAS_SMMU_WR_PRIQ_TMOUT, PCIE_REPORT),
    PCIE_HISI_COMMON_RAS(SMMU_ERR_STATUS_0, SMMU_VMS_FETCH_ERR, PCIE_RAS_SMMU_VMS_FETCH_ERR, PCIE_REPORT),
};

const struct ras_fault_converge_item *soc_misc_get_pcie_converge_item(
    const struct ras_error *error_info)
{
    u32 num = sizeof(g_pcie_converge_table) / sizeof(struct ras_fault_converge_item);
    return soc_misc_parse_table_handle(error_info, g_pcie_converge_table, num);
}

STATIC void soc_misc_pcie_print_event(struct ras_error *error)
{
    u32 num = sizeof(g_pcie_converge_table) / sizeof(struct ras_fault_converge_item);
    u32 i;

    for (i = 0; i < num; i++) {
        if (error->ras_code == g_pcie_converge_table[i].ras_code.err_type) {
            soc_misc_drv_event("PCIe ras info. (device_id=%u; module_id=%u; submodule_id=%u; "
                "ras_code=%u; ras_err_type=%u; ras_err_severity=%u; sec_type=0x%x; describe=%s)\n",
                (u32)error->device_id, (u32)error->module_id, (u32)error->sub_node_id, error->ras_code,
                g_pcie_converge_table[i].error_type, error->ras_err_severity, error->sec_type,
                g_pcie_converge_table[i].describe);
            return;
        }
    }
}

#ifdef CFG_FEATURE_DRIVER_QUERY_RAS_ERROR
STATIC int soc_misc_pcie_get_reg_data(u32 dev_id, u64 ip_offset, u64 module_offset, u64 reg_offset, u32 *val)
{
    u64 reg_phy_addr;
    void __iomem *reg_virt_addr;
    u32 chip_id = 0;
    u32 die_id = 0;
    int ret;

    ret = devdrv_get_chip_die_id(dev_id, &chip_id, &die_id);
    if (ret != 0) {
        soc_misc_drv_err("Failed to get chip die id. (devid=%u)\n", dev_id);
        return -EINVAL;
    }
    reg_phy_addr = DEVICE_OFFSET * chip_id + DIE_DEVICE_OFFSET * die_id
        + ip_offset + module_offset + reg_offset;
    reg_virt_addr = ioremap(reg_phy_addr, PCIE_RAS_REG_SIZE);
    if (reg_virt_addr == NULL) {
        soc_misc_drv_err("Failed to ioremap pcie register. (devid=%u; chip_id=%u)\n", dev_id, chip_id);
        return -EINVAL;
    }
    *val = readl(reg_virt_addr);
    iounmap(reg_virt_addr);

    return 0;
}

/* driver query and report */
STATIC int soc_misc_pcie_query_dma_ras_error(struct soc_misc_node *s_dev, bool *error_flag)
{
    u32 val;
    bool urca_err_flag = false;
    bool linkdown_err_flag = false;
    int ret;

    /* PCIE DMA UR/CA error counter */
    urca_err_flag = agentdrv_get_dma_urca_err(s_dev->die_id);

    /* PCIE mac linkdown int status */
    ret = soc_misc_pcie_get_reg_data(s_dev->die_id,
        PCIE_RAS_APB_SLAVE_BASE_ADDR, PCIE_RAS_MAC_REG_BASE, PCIE_RAS_MAC_INT_STATUS_OFFSET, &val);
    if (ret != 0) {
        return ret;
    }
    /* except linkdown */
    if ((val & BIT(1)) != 0) {
        linkdown_err_flag = true;
    }

    if ((urca_err_flag == true) && (linkdown_err_flag == false)) {
        *error_flag = true;
    }

    return 0;
}

void soc_misc_pcie_query_ras_error(struct work_struct *work)
{
    struct ras_error error_info = { 0 };
    bool error_flag = false;
    u32 ret = 0;
    static bool is_reported = false;
    struct soc_misc_node *s_dev = container_of(work, struct soc_misc_node, query_work.work);

    ret = soc_misc_pcie_query_dma_ras_error(s_dev, &error_flag);
    if (ret != 0) {
        goto QUERY_CONTINUE;
    }
    /* DMA URCA ERR only report once */
    if ((error_flag == true) && (is_reported == false)) {
        error_info.device_id = (u8)s_dev->die_id;
        error_info.sensor_id = SOC_MISC_SENSOR_PCIE;
        error_info.module_id = 0;
        error_info.sub_node_id = 0;
        error_info.ras_code = PCIE_RAS_DMA_REMOTE_URCA_ERR;
        error_info.ras_err_severity = PCIE_RAS_SERVERITY_FE;
        error_info.sec_type = RAS_SEC_PCIE;
        soc_misc_pcie_print_event(&error_info);
        soc_misc_fault_event_handler(&error_info, sizeof(struct ras_error), SOC_MISC_SENSOR_PCIE);
        is_reported = true;
    }

QUERY_CONTINUE:
    (void)schedule_delayed_work(&s_dev->query_work, msecs_to_jiffies(PCIE_RAS_DRV_QUERY_ERROR_TIME));
}
#endif /* CFG_FEATURE_DRIVER_QUERY_RAS_ERROR */

int soc_misc_pcie_ops_init(struct dms_node *device)
{
    soc_misc_drv_debug("INIT. (node_name=%.*s)\n", DMS_MAX_DEV_NAME_LEN, device->node_name);
    return 0;
}

void soc_misc_pcie_ops_uninit(struct dms_node *device)
{
    soc_misc_drv_debug("UNINIT. (node_name=%.*s)\n", DMS_MAX_DEV_NAME_LEN, device->node_name);
    return;
}

#ifdef CFG_FEATURE_PCIE_LOCAL_FAULT
void soc_misc_pcie_local_ap_unmask(u64 reg_phy_addr)
{
    void __iomem *reg_virt_addr;
    u32 val;

    /* PCIE LOCAL AP */
    reg_virt_addr = ioremap(reg_phy_addr, PCIE_RAS_REG_SIZE);
    if (reg_virt_addr == NULL) {
        soc_misc_drv_err("Failed to ioremap AP unmask irq register.\n");
        return;
    }
    val = readl(reg_virt_addr);
    val |= BIT(20); /* AP Local common error.include outbound/inbound/apat/pcie_top */
    val |= BIT(21); /* AP EP local error */

    writel(val, (volatile void *)reg_virt_addr);
    iounmap(reg_virt_addr);
    reg_virt_addr = NULL;
}

void soc_misc_pcie_local_core_global_unmask(u64 reg_phy_addr)
{
    void __iomem *reg_virt_addr;
    u32 val;

    reg_virt_addr = ioremap(reg_phy_addr, PCIE_RAS_REG_SIZE);
    if (reg_virt_addr == NULL) {
        soc_misc_drv_err("Failed to ioremap CORE GLOBAL unmask irq register.\n");
        return;
    }
    val = readl(reg_virt_addr);
    val &= ~(BIT(0)); /* tl int or tl_flr_int */
    val &= ~(BIT(1)); /* dl int */
    val &= ~(BIT(2)); /* mac int */

    writel(val, (volatile void *)reg_virt_addr);

    iounmap(reg_virt_addr);
    reg_virt_addr = NULL;
}

void soc_misc_pcie_local_unmask_task(struct work_struct *work)
{
    struct soc_misc_node *s_dev = container_of(work, struct soc_misc_node, unmask_irq_work.work);
    u64 ap_reg_phy_addr = 0;
    u64 core_reg_phy_addr = 0;

    /* NFE */
    if (s_dev->work_data[0] == PCIE_RAS_SERVERITY_NFE) {
        ap_reg_phy_addr = DEVICE_OFFSET * s_dev->die_id +
            PCIE_RAS_APB_SLAVE_BASE_ADDR + PCIE_RAS_AP_MG_REG_BASE + PCIE_RAS_AP_MG_PCIE_UNF_ENA_OFFSET;
        core_reg_phy_addr = DEVICE_OFFSET * s_dev->die_id +
            PCIE_RAS_APB_SLAVE_BASE_ADDR + PCIE_RAS_CORE_GLOBAL_REG_BASE + PCIE_RAS_CORE_GLOBAL_CORE_INT_NFE_MSK_0;
    /* CE */
    } else if (s_dev->work_data[0] == PCIE_RAS_SERVERITY_CE) {
        ap_reg_phy_addr = DEVICE_OFFSET * s_dev->die_id +
            PCIE_RAS_APB_SLAVE_BASE_ADDR + PCIE_RAS_AP_MG_REG_BASE + PCIE_RAS_AP_MG_PCIE_CE_ENA_OFFSET;
        core_reg_phy_addr = DEVICE_OFFSET * s_dev->die_id +
            PCIE_RAS_APB_SLAVE_BASE_ADDR + PCIE_RAS_CORE_GLOBAL_REG_BASE + PCIE_RAS_CORE_GLOBAL_CORE_INT_CE_MSK_0;
    }

    /* PCIE LOCAL AP */
    soc_misc_pcie_local_ap_unmask(ap_reg_phy_addr);
    /* PCIE LOCAL CORE GLOBAL */
    soc_misc_pcie_local_core_global_unmask(core_reg_phy_addr);

    soc_misc_drv_info("Unmask pcie local ras irq success. (devid=%u)\n", s_dev->die_id);
}
#endif /* CFG_FEATURE_PCIE_LOCAL_FAULT */

#ifdef CFG_FEATURE_PCIE_HISI_COMM_FAULT
void soc_misc_pcie_hisi_comm_unmask_task(struct work_struct *work)
{
    struct soc_misc_node *s_dev = container_of(work, struct soc_misc_node, unmask_irq_work.work);
    u32 dev_id, chip_id, die_id;
    u64 reg_phy_addr = 0;
    int ret;

    dev_id = s_dev->die_id;
    ret = devdrv_get_chip_die_id(dev_id, &chip_id, &die_id);
    if (ret != 0) {
        soc_misc_drv_err("Failed to get chip id and die id. (devid=%u)\n", dev_id);
        return;
    }

    /* NFE */
    if (s_dev->work_data[0] == PCIE_RAS_SERVERITY_NFE) {
        reg_phy_addr = CHIP_DEVICE_OFFSET * chip_id + DIE_DEVICE_OFFSET * die_id + RAS_PCIE_NFE_UNMASK_ADDR;
    /* CE */
    } else if (s_dev->work_data[0] == PCIE_RAS_SERVERITY_CE) {
        reg_phy_addr = CHIP_DEVICE_OFFSET * chip_id + DIE_DEVICE_OFFSET * die_id + RAS_PCIE_CE_UNMASK_ADDR;
    }
    ret = ras_int_unmask_reg_write(reg_phy_addr, RAS_PCIE_UNMASK_VAL, RAS_PCIE_UNMASK_VAL);
    if (ret != 0) {
        soc_misc_drv_err("Failed to write pcie ras unmask reg. (devid=%u)\n", dev_id);
        return;
    }

    soc_misc_drv_info("Unmask pcie hisi common ras irq success. (devid=%u)\n", dev_id);
}
#endif /* CFG_FEATURE_PCIE_HISI_COMM_FAULT */

STATIC u32 soc_misc_pcie_get_ras_code(struct ras_error *error_info, u32 error_max_num,
    const struct pcie_local_ras_event *event_list, size_t event_num, const hisi_common_error_info *pdata_ras)
{
    const u32 *reg_array;
    u32 array_size, offset, bits, i;
    u32 ras_report_num = 0;
    u32 ras_print_num = 0;

    reg_array = pdata_ras->register_array;
    array_size = pdata_ras->register_array_size;

    for (i = 0; i < event_num; i++) {
        offset = event_list[i].reg_offset;
        if (offset >= pdata_ras->register_array_size) {
            continue;
        }

        bits = event_list[i].bits;
        if (error_info[0].sensor_id == SOC_MISC_SENSOR_PCIE) { /* pcie local event */
            if ((reg_array[offset] & bits) == 0) { /* pcie local event describe by bit */
                continue;
            }
        } else if ((reg_array[offset] & IERR_MASK) != bits) { /* dispatch/smmu/aa event describe by code number */
            continue;
        }

        error_info[ras_report_num].ras_code = event_list[i].ras_code;
        soc_misc_pcie_print_event(&error_info[ras_report_num]);
        ras_print_num++;

        if (event_list[i].report_type == PCIE_REPORT) {
            ras_report_num++;
            if (ras_report_num >= error_max_num) {
                soc_misc_drv_err("ras_report_num is up to max num.\n");
                break;
            }
        }
    }

    if (ras_print_num == 0) {
        soc_misc_drv_warn("No match event. (dev_id=%u; sub_module=%u)\n",
                          (u32)error_info[0].device_id, pdata_ras->sub_module_id);
    }

    return ras_report_num;
}

STATIC u32 soc_misc_pcie_aa_hisi_comm(struct ras_error *error_info, u32 max_error_num,
    const hisi_common_error_info *pdata_ras)
{
    soc_misc_drv_warn("No pcie_aa event. (dev_id=%u)\n", (u32)error_info[0].device_id);
    return 0;
}

STATIC u32 soc_misc_pcie_disp_hisi_comm(struct ras_error *error_info, u32 max_error_num,
    const hisi_common_error_info *pdata_ras)
{
    size_t event_num = sizeof(g_pcie_disp_event) / sizeof(struct pcie_local_ras_event);
    const struct pcie_local_ras_event *event_list = g_pcie_disp_event;

    return soc_misc_pcie_get_ras_code(error_info, max_error_num, event_list, event_num, pdata_ras);
}

STATIC u32 soc_misc_pcie_smmu_hisi_comm(struct ras_error *error_info, u32 max_error_num,
    const hisi_common_error_info *pdata_ras)
{
    size_t event_num = sizeof(g_pcie_smmu_event) / sizeof(struct pcie_local_ras_event);
    const struct pcie_local_ras_event *event_list = g_pcie_smmu_event;

    return soc_misc_pcie_get_ras_code(error_info, max_error_num, event_list, event_num, pdata_ras);
}

STATIC u32 soc_misc_pcie_local_hisi_comm(struct ras_error *error_info, u32 max_error_num,
    const hisi_common_error_info *pdata_ras)
{
    const struct pcie_local_ras_event *event_list = NULL;
    size_t event_num;
    u32 sub_mod;

    sub_mod = pdata_ras->sub_module_id;
    if (sub_mod >= (u32)PCIE_RAS_SUB_MODULE_MAX) {
        soc_misc_drv_err("Invalid sub_module_id. (dev_id=%u; id=%u)\n", (u32)error_info[0].device_id, sub_mod);
        return 0;
    }

    switch (sub_mod) {
        case PCIE_RAS_AP_IOB_RX:
            event_list = g_pcie_ap_iob_rx_event;
            event_num = sizeof(g_pcie_ap_iob_rx_event) / sizeof(struct pcie_local_ras_event);
            break;
        case PCIE_RAS_AP_IOB_TX:
            event_list = g_pcie_ap_iob_tx_event;
            event_num = sizeof(g_pcie_ap_iob_tx_event) / sizeof(struct pcie_local_ras_event);
            break;
        case PCIE_RAS_AP_INT:
            event_list = g_pcie_ap_int_event;
            event_num = sizeof(g_pcie_ap_int_event) / sizeof(struct pcie_local_ras_event);
            break;
        case PCIE_RAS_TL:
            event_list = g_pcie_tl_event;
            event_num = sizeof(g_pcie_tl_event) / sizeof(struct pcie_local_ras_event);
            break;
        case PCIE_RAS_MAC:
            event_list = g_pcie_mac_event;
            event_num = sizeof(g_pcie_mac_event) / sizeof(struct pcie_local_ras_event);
            break;
        case PCIE_RAS_DL:
            event_list = g_pcie_dl_event;
            event_num = sizeof(g_pcie_dl_event) / sizeof(struct pcie_local_ras_event);
            break;
        default:
            soc_misc_drv_warn("No event list for this sub module. (dev_id=%u; id=%u)\n",
                              (u32)error_info[0].device_id, sub_mod);
            return 0;
    }

    return soc_misc_pcie_get_ras_code(error_info, max_error_num, event_list, event_num, pdata_ras);
}

STATIC u32 soc_misc_pcie_get_hisi_comm_ras_code(struct ras_error *error_info, u32 max_error_num,
    const hisi_common_error_info *pdata_ras)
{
    u32 module_id = pdata_ras->module_id;
    u32 i;

    switch (module_id) {
        case PCIE_AA_HISI_COMM:
            for (i = 0; i < max_error_num; i++) {
                error_info[i].sensor_id = SOC_MISC_SENSOR_ID_MAX; // no PCIE_AA sensor currently
            }
            return soc_misc_pcie_aa_hisi_comm(error_info, max_error_num, pdata_ras);

        case PCIE_DISP_HISI_COMM:
            for (i = 0; i < max_error_num; i++) {
                error_info[i].sensor_id = SOC_MISC_SENSOR_PCIE_DISP;
            }
            return soc_misc_pcie_disp_hisi_comm(error_info, max_error_num, pdata_ras);

        case PCIE_SMMU_HISI_COMM:
            for (i = 0; i < max_error_num; i++) {
                error_info[i].sensor_id = SOC_MISC_SENSOR_ID_MAX; // no PCIE_SMMU sensor currently
            }
            return soc_misc_pcie_smmu_hisi_comm(error_info, max_error_num, pdata_ras);

        case PCIE_LOCAL_HISI_COMM:
            for (i = 0; i < max_error_num; i++) {
                error_info[i].sensor_id = SOC_MISC_SENSOR_PCIE;
            }
            return soc_misc_pcie_local_hisi_comm(error_info, max_error_num, pdata_ras);

        default:
            break;
    }

    soc_misc_drv_err("Not support module id. (dev_id=%u; module_id=%u)\n", (u32)error_info[0].device_id, module_id);
    return 0;
}

STATIC u32 soc_misc_pcie_get_hisi_comm_event(struct ras_error *error_info, u32 error_max_num,
    const struct notify_data *pdata)
{
    u32 i;

    hisi_common_error_info *pdata_ras = (hisi_common_error_info *)pdata->origin_data;

    for (i = 0; i < error_max_num; i++) {
        error_info[i].device_id = pdata->chip_id;
        error_info[i].module_id = pdata_ras->module_id;
        error_info[i].sub_node_id = 0;
        error_info[i].ras_err_severity = pdata_ras->error_severity;
        error_info[i].sec_type = RAS_SEC_PCIE;
    }
    return soc_misc_pcie_get_hisi_comm_ras_code(error_info, error_max_num, pdata_ras);
}

STATIC u32 soc_misc_pcie_get_local_event(struct ras_error *error_info, u32 error_max_num,
    const struct notify_data *pdata)
{
    u32 ras_num = 0;

    struct cper_sec_pcie_local *pdata_ras = (struct cper_sec_pcie_local *)pdata->origin_data;
    error_info[0].device_id = pdata->chip_id;
    error_info[0].sensor_id = SOC_MISC_SENSOR_PCIE;
    error_info[0].module_id = 0;
    error_info[0].sub_node_id = 0;
    error_info[0].ras_code = pdata_ras->err_type;
    error_info[0].ras_err_severity = pdata_ras->err_severity;
    error_info[0].sec_type = RAS_SEC_PCIE;
    ras_num++;

    return ras_num;
}

#define DEVDRV_PCIE_COMMON_CHANNEL_ERR 1U
void soc_misc_linkdown_fault_post_handle(void)
{
    g_devdrv_set_channel_status_fn =
        (devdrv_set_channel_status_fn)(uintptr_t)__kallsyms_lookup_name(DEVDRV_SET_CHANNEL_STATUS_FN_NAME);
    if (g_devdrv_set_channel_status_fn == NULL) {
        soc_misc_drv_info("devdrv_set_channel_status symbol not find.\n");
    } else {
        g_devdrv_set_channel_status_fn(DEVDRV_PCIE_COMMON_CHANNEL_ERR);
    }
    return;
}

void soc_misc_pcie_fault_handler(const struct notify_data *pdata)
{
    struct drv_soc_misc_ctrl *soc_misc_ctl = soc_misc_get_ctrl();
    struct ras_error error_info[PCIE_RAS_MAX_REPORT_NUM] = { 0 };
    struct soc_misc_node *s_dev = NULL;
    u32 ras_num, i;

    if (soc_misck_check_notify_data(pdata) != 0) {
        soc_misc_drv_err("check pcie notify data failed\n");
        return;
    }

    /* all bios pcie ras use PCIE_LOCAL format */
    if (guid_equal(pdata->section_type, &CPER_SEC_HISI_PCIE_LOCAL)) {
        ras_num = soc_misc_pcie_get_local_event(error_info, PCIE_RAS_MAX_REPORT_NUM, pdata);
    } else if (guid_equal(pdata->section_type, &CPER_SEC_HISI_COMMON)) {
        ras_num = soc_misc_pcie_get_hisi_comm_event(error_info, PCIE_RAS_MAX_REPORT_NUM, pdata);
    } else {
        soc_misc_drv_err("Invalid section_type\n");
        return;
    }

    for (i = 0; i < ras_num; i++) {
        soc_misc_fault_event_handler(&error_info[i], sizeof(struct ras_error), error_info[i].sensor_id);
        if (error_info[i].ras_code == PCIE_RAS_MAC_LINKDOWN) {
            soc_misc_linkdown_fault_post_handle(); // mdc linkdown post handle
        }
    }

    /* only Non-fatal(NFE) and CE error need storm-control */
    if ((error_info[0].ras_err_severity == PCIE_RAS_SERVERITY_NFE) ||
        (error_info[0].ras_err_severity == PCIE_RAS_SERVERITY_CE)) {
        s_dev = &soc_misc_ctl->node[error_info[0].device_id][0][SOC_MISC_SENSOR_PCIE];
        /* save err severity to work_data */
        s_dev->work_data[0] = (unsigned char)error_info[0].ras_err_severity;
        (void)cancel_delayed_work_sync(&s_dev->unmask_irq_work);
        (void)schedule_delayed_work(&s_dev->unmask_irq_work, msecs_to_jiffies(PCIE_RAS_SCHEDULE_UNMASK_TIME));
    }
}

int soc_misc_pcie_sensor_scan(unsigned long long private_data, struct dms_sensor_event_data *data)
{
    int ret;

    ret = soc_misc_fault_event_scan(private_data, data, SOC_MISC_SENSOR_PCIE);
    if (ret) {
        soc_misc_drv_err("Scan pcie sensor failed. (ret=%d)\n", ret);
        return ret;
    }

    return 0;
}

int soc_misc_pcie_disp_sensor_scan(unsigned long long private_data, struct dms_sensor_event_data *data)
{
    int ret;

    ret = soc_misc_fault_event_scan(private_data, data, SOC_MISC_SENSOR_PCIE_DISP);
    if (ret) {
        soc_misc_drv_err("Scan pcie_disp sensor failed. (ret=%d)\n", ret);
        return ret;
    }

    return 0;
}
