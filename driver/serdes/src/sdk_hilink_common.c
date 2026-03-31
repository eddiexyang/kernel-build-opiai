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

#include <linux/delay.h>

#include "sdk_hilink_common.h"
#include "sdk_hilink_init.h"
#include "sdk_hilink_snapshot.h"

#include "io_subctrl_reg_offset_field.h"
#include "io_subctrl_reg_offset.h"
#include "ao_subctrl_reg_offset_field.h"
#include "ao_subctrl_reg_offset.h"
#include "hilink_subctrl_reg_offset.h"
#include "hilink_subctrl_reg_offset_field.h"
#include "hilink_adap_lane_csr_reg_offset.h"
#include "hilink_adap_lane_csr_reg_offset_field.h"
#include "hilink_cp_csr_reg_offset.h"
#include "hilink_cp_csr_reg_offset_field.h"
#include "hilink_cs_csr_reg_offset.h"
#include "hilink_cs_csr_reg_offset_field.h"
#include "hilink_rx_csr_reg_offset.h"
#include "hilink_rx_csr_reg_offset_field.h"
#include "hilink_tx_csr_reg_offset.h"
#include "hilink_tx_csr_reg_offset_field.h"
#include "hilink_fw_csapi_csr_reg_offset.h"
#include "hilink_fw_csapi_csr_reg_offset_field.h"
#include "hilink_fw_dsapi_csr_reg_offset.h"
#include "hilink_fw_dsapi_csr_reg_offset_field.h"
#include "hilink_fw_marcoapi_csr_reg_offset.h"
#include "hilink_fw_marcoapi_csr_reg_offset_field.h"
#include "hilink_fw_romapi_csr_reg_offset.h"
#include "hilink_fw_romapi_csr_reg_offset_field.h"

HilinkPara g_hilinkPara;

static const PowerSwitchConfig_t g_powerSwitchConfig[PS_TYPE_END] = {
    // PCIE
    { PS_PCIE_L0, 0x0},
    { PS_PCIE_L0S, 0x1},
    { PS_PCIE_L1, 0x2},
    { PS_PCIE_L2, 0x3},
    { PS_PCIE_L1pCPM, 0x4},
    { PS_PCIE_L1p1, 0x5},
    { PS_PCIE_L1p2, 0x6},

    // USB
    { PS_USB_U0, 0x0},
    { PS_USB_U1, 0x1},
    { PS_USB_U2, 0x2},
    { PS_USB_U3, 0x3},

    // SAS
    { PS_SAS_PHYRDY, 0x0},
    { PS_SAS_PARTIAL, 0x1},
    { PS_SAS_SLUMBER, 0x2},

    { PS_MPHY_BURST, 0x0},
    { PS_MPHY_H8, 0x6},

    { PS_P0, 0x0},
    { PS_P6, 0x6},
};

HilinkPara *HILINK_GetParaCfg(void)
{
    return &g_hilinkPara;
}

void Hilink_SetRegBase(HilinkRegBase* base, HilinkPara* hilink_para, u32 macro, u32 id)
{
    base->hilink_para = hilink_para;
    base->macro = macro;
    base->id = id;
}

void HAL_UsSleep(u64 us)
{
    u64 i;
    for (i = 0; i < us; ++i) {
        usleep_range(10, 20);
    }
}

void HAL_MsSleep(u64 ms)
{
    HAL_UsSleep(ms * TIMER_MULTOPLE);
}

void HILINK_RegWriteBits(void* baseAddr, u32 reg, u32 highBit, u32 lowBit, u32 val)
{
    u32 originVal;

    if (highBit < lowBit || highBit > BIT_31) {
        HILINK_ERR("reg[%x] [h:%u-l:%u] invalid\n", reg, highBit, lowBit);
        return;
    }

    if ((highBit - lowBit) >= BIT_31) {
        REG_32_WRITE(baseAddr, reg, val);
        return;
    }

    if (val > ((0x1U << ((highBit - lowBit) + 1U)) - 1U)) {
        HILINK_ERR("reg[%x] [h:%u-l:%u] val %x invalid\n", reg, highBit, lowBit, val);
        return;
    }

    originVal = REG_32_READ(baseAddr, reg);
    SET_BITS(originVal, highBit, lowBit, val);
    REG_32_WRITE(baseAddr, reg, originVal);

    return;
}

u32 HILINK_RegReadBits(void* baseAddr, u32 reg, u32 highBit, u32 lowBit)
{
    u32 originVal;

    if (highBit < lowBit || highBit > BIT_31) {
        HILINK_ERR("[h:%u-l:%u] invalid", highBit, lowBit);
        return 0x0U;
    }

    originVal = REG_32_READ(baseAddr, reg);
    if ((highBit - lowBit) >= BIT_31) {
        return originVal;
    }

    return GET_BITS(originVal, highBit, lowBit);
}

void HILINK_RegWriteOffset(void* baseAddr, u32 reg, u32 offset, u32 len, u32 val)
{
    HILINK_RegWriteBits(baseAddr, reg, offset + len - 1, offset, val);
}

u32 HILINK_RegReadOffset(void* baseAddr, u32 reg, u32 offset, u32 len)
{
    return HILINK_RegReadBits(baseAddr, reg, offset + len - 1, offset);
}

STATIC bool HILINK_GetFsmDoneLane01(u32 macro)
{
    u32 val[GET_FSM_DONE_REG_LEN];
    HilinkPara *hilinkPara = HILINK_GetParaCfg();
    HilinkRegBase base;

    Hilink_SetRegBase(&base, hilinkPara, macro, LANE_0);
    val[0] = HILINK_TxRegReadBits(base, TX_CSR_TX_CSR7_REG, TX_CSR_7_BIT16_OFFSET, TX_CSR_7_BIT16_LEN);
    val[1] = HILINK_RxRegReadBits(base, RX_CSR_RX_CSR7_REG, RX_CSR_7_BIT16_OFFSET, RX_CSR_7_BIT16_LEN);
    Hilink_SetRegBase(&base, hilinkPara, macro, LANE_1);
    val[2] = HILINK_TxRegReadBits(base, TX_CSR_TX_CSR7_REG, TX_CSR_7_BIT16_OFFSET, TX_CSR_7_BIT16_LEN);
    val[3] = HILINK_RxRegReadBits(base, RX_CSR_RX_CSR7_REG, RX_CSR_7_BIT16_OFFSET, RX_CSR_7_BIT16_LEN);

    val[4] = HILINK_CPRegReadBits(base, CP_CSR_CP_CSR7_REG, CP_CSR_7_BIT16_OFFSET, CP_CSR_7_BIT16_LEN);

    if (val[0] == 0 || val[1] == 0 || val[2] == 0 || val[3] == 0 || val[4] == 0) {
        return false;
    } else {
        return true;
    }
}

STATIC bool HILINK_GetFsmDoneLane23(u32 macro)
{
    u32 val[GET_FSM_DONE_REG_LEN];
    HilinkPara *hilinkPara = HILINK_GetParaCfg();
    HilinkRegBase base;

    Hilink_SetRegBase(&base, hilinkPara, macro, LANE_2);
    val[0] = HILINK_TxRegReadBits(base, TX_CSR_TX_CSR7_REG, TX_CSR_7_BIT16_OFFSET, TX_CSR_7_BIT16_LEN);
    val[1] = HILINK_RxRegReadBits(base, RX_CSR_RX_CSR7_REG, RX_CSR_7_BIT16_OFFSET, RX_CSR_7_BIT16_LEN);
    Hilink_SetRegBase(&base, hilinkPara, macro, LANE_3);
    val[2] = HILINK_TxRegReadBits(base, TX_CSR_TX_CSR7_REG, TX_CSR_7_BIT16_OFFSET, TX_CSR_7_BIT16_LEN);
    val[3] = HILINK_RxRegReadBits(base, RX_CSR_RX_CSR7_REG, RX_CSR_7_BIT16_OFFSET, RX_CSR_7_BIT16_LEN);

    val[4] = HILINK_CPRegReadBits(base, CP_CSR_CP_CSR7_REG, CP_CSR_7_BIT16_OFFSET, CP_CSR_7_BIT16_LEN);

    if (val[0] == 0 || val[1] == 0 || val[2] == 0 || val[3] == 0 || val[4] == 0) {
        return false;
    } else {
        return true;
    }
}

STATIC bool HILINK_GetFsmDone(u32 macro, u32 mode)
{
    if (mode == 0x1) {
        return HILINK_GetFsmDoneLane01(macro);
    } else if (mode == 0x2) {
        return (HILINK_GetFsmDoneLane01(macro) && HILINK_GetFsmDoneLane23(macro));
    }
    HILINK_ERR("macro_%d mode %d getFsmDone fail!\n", macro, mode);
    return false;
}

// Wait until the execution of all instruction state machines ends and enter the stop state.
bool HILINK_WaitFsmDone(u32 macro)
{
    u32 cnt = 0;
    u32 mode;
    bool fsmDone;
    HilinkPara *hilinkPara = HILINK_GetParaCfg();
    HilinkRegBase base;

    CHECK_RETURN(macro, MACRO_MAX, false);

    Hilink_SetRegBase(&base, hilinkPara, macro, 0);
    mode = HILINK_CPRegReadBits(base, CP_CSR_CP_CSR0_REG, CP_CSR_0_BIT22_OFFSET, CP_CSR_0_BIT22_LEN);
    if (mode != 0x1 && mode != 0x2) {
        HILINK_ERR_SNAPSHOT(SERDES_GET_FSM_DONE_FAIL, "macro_%d mode %d getFsmDone fail!\n", macro, mode);
        return false;
    }
    HILINK_INFO("get dds mode num %d\n", mode);
    do {
        HAL_UsSleep(1);
        cnt++;
        fsmDone = HILINK_GetFsmDone(macro, mode);
    } while ((fsmDone != true) && (cnt < WAIT_FSM_DONE_TIMEOUT_CNT));
    if (cnt >= WAIT_FSM_DONE_TIMEOUT_CNT) {
        HILINK_ERR_SNAPSHOT(SERDES_WAIT_FSM_DONE_FAIL, "macro_%d WaitFsmDone fail!", macro);
        return false;
    }
    return true;
}

STATIC void ClearSliceTaskEnApi(u32 macro, unsigned int sliceMask)
{
    unsigned int sliceId;
    HilinkPara *hilinkPara = HILINK_GetParaCfg();
    HilinkRegBase base;

    for (sliceId = 0; sliceId < SLICE_MAX_NUM; sliceId++) {
        if (((sliceMask >> sliceId) & 0x1) == 0x1) {
            if (sliceId >= CS_SLICE_START_ID) {
                Hilink_SetRegBase(&base, hilinkPara, macro, sliceId - CS_SLICE_START_ID);
                HILINK_CsApiRegWriteBits(base, FW_CSAPI_CSR0_REG, BIT_0, BIT_LEN, 0x0);
            } else if (sliceId < DS_MAX) {
                Hilink_SetRegBase(&base, hilinkPara, macro, sliceId);
                HILINK_DsApiRegWriteBits(base, FW_DSAPI_CSR0_REG, BIT_0, BIT_LEN, 0x0);
            }
        }
    }
}

STATIC unsigned int StopCsOrDsTask(u32 macro, u32 sliceId)
{
    unsigned int val = 0;
    HilinkPara *hilinkPara = HILINK_GetParaCfg();
    HilinkRegBase base;
    if (sliceId >= CS_SLICE_START_ID) {
        // Wait CS task to complete by checking status of CS_DONE_STATUS_API
        Hilink_SetRegBase(&base, hilinkPara, macro, sliceId - CS_SLICE_START_ID);
        val = HILINK_CsApiRegReadBits(base, FW_CSAPI_CSR5_REG, FW_CSAPI_CSR_5_BIT0_OFFSET,
            FW_CSAPI_CSR_5_BIT0_LEN);
    } else if (sliceId < DS_MAX) {
        // Wait DS task to complete by checking status of DS_DONE_STATUS_API
        Hilink_SetRegBase(&base, hilinkPara, macro, sliceId);
        val = HILINK_DsApiRegReadBits(base, FW_DSAPI_CSR11_REG, FW_DSAPI_CSR_11_BIT31_OFFSET,
            FW_DSAPI_CSR_11_BIT31_LEN);
    }
    return val;
}

int StopSliceFwTask(u32 macro, unsigned int sliceMask)
{
    unsigned int checkCount = 0;
    unsigned int doneMask = 0;
    unsigned int sliceId;
    unsigned int val;

    ClearSliceTaskEnApi(macro, sliceMask);
    // Wait slice task to complete
    do {
        checkCount++;
        HAL_UsSleep(100);
        for (sliceId = 0; sliceId < SLICE_MAX_NUM; sliceId++) {
            if ((((sliceMask >> sliceId) & 0x1) == 0x1) && (((doneMask >> sliceId) & 0x1) == 0x0)) {
                val = StopCsOrDsTask(macro, sliceId);
                doneMask |= val << sliceId;
            }
        }
    } while ((doneMask != sliceMask) && (checkCount < STOP_SLICE_FW_TASK_TIMEOUT_CNT));
    if (checkCount >= STOP_SLICE_FW_TASK_TIMEOUT_CNT) {
        HILINK_ERR_SNAPSHOT(SERDES_STOP_SLICE_FW_TASK_TIMEOUT_FAIL,
            "Stop slice FW Task Fail! sliceMask=%d, DoneMask=%d\n", sliceMask, doneMask);
        return RET_FAIL;
    } else {
        return RET_OK;
    }
}

STATIC void DsSwitchToPCIEPinMode(u32 macro, u32 dsId)
{
    HilinkPara *hilinkPara = HILINK_GetParaCfg();
    HilinkRegBase base;
    Hilink_SetRegBase(&base, hilinkPara, macro, dsId);

    HILINK_TxRegWriteBits(base, TX_CSR_TX_CSR25_REG,
        TX_CSR_25_BIT26_OFFSET, TX_CSR_25_BIT26_LEN, 0x1);
    HILINK_TxRegWriteBits(base, TX_CSR_TX_CSR25_REG,
        TX_CSR_25_BIT25_OFFSET, TX_CSR_25_BIT25_LEN, 0x1);
    HILINK_TxRegWriteBits(base, TX_CSR_TX_CSR25_REG,
        TX_CSR_25_BIT29_OFFSET, TX_CSR_25_BIT29_LEN, 0x1);
    HILINK_TxRegWriteBits(base, TX_CSR_TX_CSR20_REG,
        TX_CSR_20_BIT27_OFFSET, TX_CSR_20_BIT27_LEN, 0x1);
    HILINK_RxRegWriteBits(base, RX_CSR_RX_CSR18_REG,
        RX_CSR_18_BIT28_OFFSET, RX_CSR_18_BIT28_LEN, 0x1);
    HILINK_TxRegWriteBits(base, TX_CSR_TX_CSR20_REG,
        TX_CSR_20_BIT29_OFFSET, TX_CSR_20_BIT29_LEN, 0x1);
    HILINK_RxRegWriteBits(base, RX_CSR_RX_CSR18_REG,
        RX_CSR_18_BIT30_OFFSET, RX_CSR_18_BIT30_LEN, 0x1);
}

STATIC void DsSwitchToUSBPinMode(u32 macro, u32 dsId)
{
    HilinkPara *hilinkPara = HILINK_GetParaCfg();
    HilinkRegBase base;
    Hilink_SetRegBase(&base, hilinkPara, macro, dsId);

    HILINK_TxRegWriteBits(base, TX_CSR_TX_CSR25_REG,
        TX_CSR_25_BIT26_OFFSET, TX_CSR_25_BIT26_LEN, 0x1);
    HILINK_TxRegWriteBits(base, TX_CSR_TX_CSR25_REG,
        TX_CSR_25_BIT25_OFFSET, TX_CSR_25_BIT25_LEN, 0x1);
    HILINK_TxRegWriteBits(base, TX_CSR_TX_CSR25_REG,
        TX_CSR_25_BIT29_OFFSET, TX_CSR_25_BIT29_LEN, 0x1);
    HILINK_TxRegWriteBits(base, TX_CSR_TX_CSR20_REG,
        TX_CSR_20_BIT27_OFFSET, TX_CSR_20_BIT27_LEN, 0x1);
    HILINK_RxRegWriteBits(base, RX_CSR_RX_CSR18_REG,
        RX_CSR_18_BIT28_OFFSET, RX_CSR_18_BIT28_LEN, 0x1);
    HILINK_TxRegWriteBits(base, TX_CSR_TX_CSR20_REG,
        TX_CSR_20_BIT28_OFFSET, TX_CSR_20_BIT28_LEN, 0x1);
    HILINK_RxRegWriteBits(base, RX_CSR_RX_CSR18_REG,
        RX_CSR_18_BIT29_OFFSET, RX_CSR_18_BIT29_LEN, 0x1);
    HILINK_TxRegWriteBits(base, TX_CSR_TX_CSR20_REG,
        TX_CSR_20_BIT29_OFFSET, TX_CSR_20_BIT29_LEN, 0x1);
    HILINK_RxRegWriteBits(base, RX_CSR_RX_CSR18_REG,
        RX_CSR_18_BIT30_OFFSET, RX_CSR_18_BIT30_LEN, 0x1);
}

STATIC void DsSwitchToSATAPinMode(u32 macro, u32 dsId)
{
    HilinkPara *hilinkPara = HILINK_GetParaCfg();
    HilinkRegBase base;
    Hilink_SetRegBase(&base, hilinkPara, macro, dsId);

    HILINK_TxRegWriteBits(base, TX_CSR_TX_CSR25_REG,
        TX_CSR_25_BIT25_OFFSET, TX_CSR_25_BIT25_LEN, 0x1);
    HILINK_TxRegWriteBits(base, TX_CSR_TX_CSR20_REG,
        TX_CSR_20_BIT27_OFFSET, TX_CSR_20_BIT27_LEN, 0x1);
    HILINK_RxRegWriteBits(base, RX_CSR_RX_CSR18_REG,
        RX_CSR_18_BIT28_OFFSET, RX_CSR_18_BIT28_LEN, 0x1);
    HILINK_TxRegWriteBits(base, TX_CSR_TX_CSR20_REG,
        TX_CSR_20_BIT29_OFFSET, TX_CSR_20_BIT29_LEN, 0x1);
    HILINK_RxRegWriteBits(base, RX_CSR_RX_CSR18_REG,
        RX_CSR_18_BIT30_OFFSET, RX_CSR_18_BIT30_LEN, 0x1);
}

STATIC void DsSwitchToMPHYPinMode(u32 macro, u32 dsId)
{
    HilinkPara *hilinkPara = HILINK_GetParaCfg();
    HilinkRegBase base;
    Hilink_SetRegBase(&base, hilinkPara, macro, dsId);

    HILINK_TxRegWriteBits(base, TX_CSR_TX_CSR28_REG,
        TX_CSR_28_BIT2_OFFSET, TX_CSR_28_BIT2_LEN, 0x1);
    HILINK_TxRegWriteBits(base, TX_CSR_TX_CSR25_REG,
        TX_CSR_25_BIT26_OFFSET, TX_CSR_25_BIT26_LEN, 0x1);
    HILINK_RxRegWriteBits(base, RX_CSR_RX_CSR27_REG,
        RX_CSR_27_BIT3_OFFSET, RX_CSR_27_BIT3_LEN, 0x1);
    HILINK_RxRegWriteBits(base, RX_CSR_RX_CSR27_REG,
        RX_CSR_27_BIT2_OFFSET, RX_CSR_27_BIT2_LEN, 0x1);
    HILINK_TxRegWriteBits(base, TX_CSR_TX_CSR20_REG,
        TX_CSR_20_BIT29_OFFSET, TX_CSR_20_BIT29_LEN, 0x1);
    HILINK_RxRegWriteBits(base, RX_CSR_RX_CSR18_REG,
        RX_CSR_18_BIT30_OFFSET, RX_CSR_18_BIT30_LEN, 0x1);
    HILINK_TxRegWriteBits(base, TX_CSR_TX_CSR20_REG,
        TX_CSR_20_BIT27_OFFSET, TX_CSR_20_BIT27_LEN, 0x1);
    HILINK_RxRegWriteBits(base, RX_CSR_RX_CSR18_REG,
        RX_CSR_18_BIT28_OFFSET, RX_CSR_18_BIT28_LEN, 0x1);
}

STATIC void DsSwitchToETHPinMode(u32 macro, u32 dsId)
{
    HilinkPara *hilinkPara = HILINK_GetParaCfg();
    HilinkRegBase base;
    Hilink_SetRegBase(&base, hilinkPara, macro, dsId);

    HILINK_TxRegWriteBits(base, TX_CSR_TX_CSR25_REG,
        TX_CSR_25_BIT25_OFFSET, TX_CSR_25_BIT25_LEN, 0x1);
    HILINK_TxRegWriteBits(base, TX_CSR_TX_CSR20_REG,
        TX_CSR_20_BIT27_OFFSET, TX_CSR_20_BIT27_LEN, 0x1);
    HILINK_RxRegWriteBits(base, RX_CSR_RX_CSR18_REG,
        RX_CSR_18_BIT28_OFFSET, RX_CSR_18_BIT28_LEN, 0x1);
    HILINK_TxRegWriteBits(base, TX_CSR_TX_CSR20_REG,
        TX_CSR_20_BIT29_OFFSET, TX_CSR_20_BIT29_LEN, 0x1);
    HILINK_RxRegWriteBits(base, RX_CSR_RX_CSR18_REG,
        RX_CSR_18_BIT30_OFFSET, RX_CSR_18_BIT30_LEN, 0x1);
}

STATIC void DsSwitchToPinMode(u32 macro, u32 dsId)
{
    HilinkPara *hilinkPara = HILINK_GetParaCfg();
    u32 phyMode;
    HilinkRegBase base;
    Hilink_SetRegBase(&base, hilinkPara, macro, dsId);

    phyMode = HILINK_TxRegReadBits(base, TX_CSR_TX_CSR20_REG,
        TX_CSR_20_BIT8_OFFSET, TX_CSR_20_BIT8_LEN);
    if (phyMode == HILINK_MODE_PCIE) {
        DsSwitchToPCIEPinMode(macro, dsId);
    } else if (phyMode == HILINK_MODE_USB) {
        DsSwitchToUSBPinMode(macro, dsId);
    } else if (phyMode == HILINK_MODE_SATA) {
        DsSwitchToSATAPinMode(macro, dsId);
    } else if (phyMode == HILINK_MODE_MPHY) {
        DsSwitchToMPHYPinMode(macro, dsId);
    } else if (phyMode == HILINK_MODE_ETH) {
        DsSwitchToETHPinMode(macro, dsId);
    } else {
        HILINK_ERR("current mode(%d) not support.\n", phyMode);
    }
}

void SwitchControlMode(u32 macro, u32 dsId, SdpiMode_e ctrlMode)
{
    HilinkPara *hilinkPara = HILINK_GetParaCfg();
    HilinkRegBase base;

    CHECK_RETURN(macro, MACRO_MAX);
    CHECK_RETURN(dsId, LANE_MAX);
    Hilink_SetRegBase(&base, hilinkPara, macro, dsId);

    if (ctrlMode != CTRL_MODE_PIN) { // reg mode
        HILINK_TxRegWriteBits(base, TX_CSR_TX_CSR28_REG,
            TX_CSR_28_BIT2_OFFSET, TX_CSR_28_BIT2_LEN, 0x0);
        HILINK_TxRegWriteBits(base, TX_CSR_TX_CSR25_REG,
            TX_CSR_25_BIT29_OFFSET, TX_CSR_25_BIT29_LEN, 0x0);
        HILINK_TxRegWriteBits(base, TX_CSR_TX_CSR25_REG,
            TX_CSR_25_BIT26_OFFSET, TX_CSR_25_BIT26_LEN, 0x0);
        HILINK_TxRegWriteBits(base, TX_CSR_TX_CSR25_REG,
            TX_CSR_25_BIT25_OFFSET, TX_CSR_25_BIT25_LEN, 0x0);
        HILINK_RxRegWriteBits(base, RX_CSR_RX_CSR27_REG,
            RX_CSR_27_BIT3_OFFSET, RX_CSR_27_BIT3_LEN, 0x0);
        HILINK_RxRegWriteBits(base, RX_CSR_RX_CSR27_REG,
            RX_CSR_27_BIT2_OFFSET, RX_CSR_27_BIT2_LEN, 0x0);
        HILINK_TxRegWriteBits(base, TX_CSR_TX_CSR20_REG,
            TX_CSR_20_BIT27_OFFSET, TX_CSR_20_BIT27_LEN, 0x0);
        HILINK_RxRegWriteBits(base, RX_CSR_RX_CSR18_REG,
            RX_CSR_18_BIT28_OFFSET, RX_CSR_18_BIT28_LEN, 0x0);
        HILINK_TxRegWriteBits(base, TX_CSR_TX_CSR20_REG,
            TX_CSR_20_BIT28_OFFSET, TX_CSR_20_BIT28_LEN, 0x0);
        HILINK_RxRegWriteBits(base, RX_CSR_RX_CSR18_REG,
            RX_CSR_18_BIT29_OFFSET, RX_CSR_18_BIT29_LEN, 0x0);
        HILINK_TxRegWriteBits(base, TX_CSR_TX_CSR20_REG,
            TX_CSR_20_BIT29_OFFSET, TX_CSR_20_BIT29_LEN, 0x0);
        HILINK_RxRegWriteBits(base, RX_CSR_RX_CSR18_REG,
            RX_CSR_18_BIT30_OFFSET, RX_CSR_18_BIT30_LEN, 0x0);
    } else { // Pin mode
        DsSwitchToPinMode(macro, dsId);
    }
}
EXPORT_SYMBOL(SwitchControlMode);

static const u32 pll_cs_para[CS_MAX][PLL_CS_PARA_NUM] = {
    {0x43F8A3D0, 0x32000000, 0x1A000000, 0x370000C6, 0x18D3, 0x4082502, 0x2005400A, 0x11C2507, 0x9C2502},
    {0x43F8AECC, 0x2EE00000, 0x1A000000, 0x370000C6, 0x1001745, 0x4082502, 0x2005400A, 0x1C2507, 0x9C2502}
};
static const u32 pll_tx_para[CS_MAX][PLL_TX_PARA_NUM] = {
    {0xD0199}, {0xD0191}
};
static const u32 pll_rx_para[CS_MAX][PLL_RX_PARA_NUM] = {
    {0x60FC20B0, 0x80802019, 0x7E8034, 0x22524422, 0x2507417},
    {0x68F86170, 0x80802019, 0xBD8034, 0x22524843, 0x2E0911C}
};

void EthScenarioDataRateConfig(u32 macro, u32 csIndex, DRV_SERDES_FREQUENCY_LEVEL targetRate)
{
    u32 i;
    u32 pll_cs_para_addr;
    u32 pll_tx_para_addr;
    u32 pll_rx_para_addr;
    HilinkPara *hilinkPara = HILINK_GetParaCfg();

    CHECK_RETURN(macro, MACRO_MAX);
    CHECK_RETURN(csIndex, CS_MAX);
    if ((targetRate != SERDES_FREQ_ETH_1_25G) && (targetRate != SERDES_FREQ_ETH_3_125G)) {
        HILINK_ERR("targetRate %d out of range.\n", targetRate);
        return;
    }

    if (csIndex == 0) {
        pll_cs_para_addr = PLL0_CS_PARA_ADDR;
        pll_tx_para_addr = PLL0_TX_PARA_ADDR;
        pll_rx_para_addr = PLL0_RX_PARA_ADDR;
    } else {
        pll_cs_para_addr = PLL1_CS_PARA_ADDR;
        pll_tx_para_addr = PLL1_TX_PARA_ADDR;
        pll_rx_para_addr = PLL1_RX_PARA_ADDR;
    }
    HILINK_INFO("CSid: %d, rate: %d\n", csIndex, targetRate);
    for (i = 0; i < PLL_CS_PARA_NUM; i++) {
        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
            pll_cs_para_addr + i * REG_LEN, BIT_31, BIT_0, pll_cs_para[targetRate][i]);
    }
    for (i = 0; i < PLL_TX_PARA_NUM; i++) {
        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
            pll_tx_para_addr + i * REG_LEN, BIT_31, BIT_0, pll_tx_para[targetRate][i]);
    }
    for (i = 0; i < PLL_RX_PARA_NUM; i++) {
        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
            pll_rx_para_addr + i * REG_LEN, BIT_31, BIT_0, pll_rx_para[targetRate][i]);
    }
}
EXPORT_SYMBOL(EthScenarioDataRateConfig);

u32 eth_rx_para_config(u32 macro, u32 ds, DRV_SERDES_FREQUENCY_LEVEL targetRate)
{
    u32 i;
    u32 rate;
    HilinkPara *hilinkPara = HILINK_GetParaCfg();
    HilinkRegBase base;
    const struct hilink_reg_info reg[DS_ETH_RX_PARA_CONFIG_NUM] = {
        {
            .base = RX_CSR_RX_CSR92_REG,
            .offset = RX_CSR_92_BIT9_OFFSET,
            .len = RX_CSR_92_BIT9_LEN
        },
        {
            .base = RX_CSR_RX_CSR90_REG,
            .offset = RX_CSR_90_BIT17_OFFSET,
            .len = RX_CSR_90_BIT17_LEN
        },
        {
            .base = RX_CSR_RX_CSR91_REG,
            .offset = RX_CSR_91_BIT22_OFFSET,
            .len = RX_CSR_91_BIT22_LEN
        },
        {
            .base = RX_CSR_RX_CSR90_REG,
            .offset = RX_CSR_90_BIT22_OFFSET,
            .len = RX_CSR_90_BIT22_LEN
        },
        {
            .base = RX_CSR_RX_CSR91_REG,
            .offset = RX_CSR_91_BIT26_OFFSET,
            .len = RX_CSR_91_BIT26_LEN
        },
        {
            .base = RX_CSR_RX_CSR90_REG,
            .offset = RX_CSR_90_BIT1_OFFSET,
            .len = RX_CSR_90_BIT1_LEN
        },
        {
            .base = RX_CSR_RX_CSR91_REG,
            .offset = RX_CSR_91_BIT6_OFFSET,
            .len = RX_CSR_91_BIT6_LEN
        }
    };
    const u32 reg_val[DS_ETH_RATE_NUM][DS_ETH_RX_PARA_CONFIG_NUM] = {
        {6, 5, 2, 7, 7, 6, 6}, // SERDES_FREQ_ETH_1_25G
        {6, 2, 0, 6, 6, 6, 6}  // SERDES_FREQ_ETH_3_125G
    };

    CHECK_RETURN_SNAPSHOT(SERDES_ETH_RX_PARA_CONFIG_MACRO_CHECK_FAIL, macro, MACRO_MAX, RET_ERROR);
    CHECK_RETURN_SNAPSHOT(SERDES_ETH_RX_PARA_CONFIG_DS_CHECK_FAIL, ds, DS_MAX, RET_ERROR);
    if ((targetRate != SERDES_FREQ_ETH_1_25G) && (targetRate != SERDES_FREQ_ETH_3_125G)) {
        HILINK_ERR_SNAPSHOT(SERDES_ETH_RX_PARA_CONFIG_RATE_CHECK_FAIL, "targetRate %d out of range.\n", targetRate);
        return RET_ERROR;
    }

    Hilink_SetRegBase(&base, hilinkPara, macro, ds);
    rate = (targetRate == SERDES_FREQ_ETH_1_25G ? 0 : 1);
    HILINK_INFO("RATE: %d\n", rate);
    for (i = 0; i < DS_ETH_RX_PARA_CONFIG_NUM; ++i) {
        HILINK_RxRegWriteBits(base, reg[i].base, reg[i].offset, reg[i].len, reg_val[rate][i]);
    }
    return RET_OK;
}

u32 DsDataRateSwitch(u32 macro, u32 dsIndex, HilinkRate targetRate, u32 dsWidth)
{
    u32 rateCmd, readData;
    u32 rxSdpiEventAck, txSdpiEventAck;
    u32 rateSwitchId = 0x10000000;
    u32 txSdpiMode;
    u32 cnt = 0;
    HilinkPara *hilinkPara = HILINK_GetParaCfg();
    CHECK_RETURN(macro, MACRO_MAX, RET_FAIL);
    CHECK_RETURN(dsIndex, DS_MAX, RET_FAIL);
    CHECK_RETURN(targetRate, HILINK_XGE_BUTT, RET_FAIL);
    rateCmd = rateSwitchId + ((dsWidth << DS_DATA_WIDTH_OFFSET) & (0x1C0)) + targetRate;
    txSdpiMode = HILINK_RegReadBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
        TX_CSR_REG_OFFSET(dsIndex, TX_CSR_TX_CSR20_REG),
        TX_CSR_20_BIT14_OFFSET + TX_CSR_20_BIT14_LEN - 1,
        TX_CSR_20_BIT14_OFFSET);
    HILINK_INFO("txSdpiMode: 0x%x\n", txSdpiMode);
    if (txSdpiMode == HILINK_MODE_ETH) { // ETH
        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
            TX_CSR_REG_OFFSET(dsIndex, TX_CSR_TX_CSR13_REG),
            TX_CSR_13_BIT12_OFFSET + TX_CSR_13_BIT12_LEN - 1,
            TX_CSR_13_BIT12_OFFSET,
            0);
        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
            RX_CSR_REG_OFFSET(dsIndex, RX_CSR_RX_CSR13_REG),
            RX_CSR_13_BIT16_OFFSET + RX_CSR_13_BIT16_LEN - 1,
            RX_CSR_13_BIT16_OFFSET,
            0);
    }

    HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
        RX_CSR_REG_OFFSET(dsIndex, RX_CSR_RX_CSR75_REG),
        RX_CSR_75_BIT0_OFFSET + RX_CSR_75_BIT0_LEN - 1,
        RX_CSR_75_BIT0_OFFSET,
        0x1);

    readData = HILINK_RegReadBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
        TX_CSR_REG_OFFSET(dsIndex, TX_CSR_TX_CSR20_REG),
        TX_CSR_20_BIT18_OFFSET + TX_CSR_20_BIT18_LEN - 1,
        TX_CSR_20_BIT18_OFFSET);
    HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
        TX_CSR_REG_OFFSET(dsIndex, TX_CSR_TX_CSR20_REG),
        TX_CSR_20_BIT18_OFFSET + TX_CSR_20_BIT18_LEN - 1,
        TX_CSR_20_BIT18_OFFSET,
        readData | 0x2);

    readData = HILINK_RegReadBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
        RX_CSR_REG_OFFSET(dsIndex, RX_CSR_RX_CSR18_REG),
        RX_CSR_18_BIT18_OFFSET + RX_CSR_18_BIT18_LEN - 1,
        RX_CSR_18_BIT18_OFFSET);
    HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
        RX_CSR_REG_OFFSET(dsIndex, RX_CSR_RX_CSR18_REG),
        RX_CSR_18_BIT18_OFFSET + RX_CSR_18_BIT18_LEN - 1,
        RX_CSR_18_BIT18_OFFSET,
        readData | 0x2);

    HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
        TX_CSR_REG_OFFSET(dsIndex, TX_CSR_TX_CSR19_REG),
        TX_CSR_19_BIT0_OFFSET + TX_CSR_19_BIT0_LEN - 1,
        TX_CSR_19_BIT0_OFFSET,
        rateCmd);
    HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
        RX_CSR_REG_OFFSET(dsIndex, RX_CSR_RX_CSR17_REG),
        RX_CSR_17_BIT0_OFFSET + RX_CSR_17_BIT0_LEN - 1,
        RX_CSR_17_BIT0_OFFSET,
        rateCmd);
    HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
        TX_CSR_REG_OFFSET(dsIndex, TX_CSR_TX_CSR20_REG),
        TX_CSR_20_BIT12_OFFSET + TX_CSR_20_BIT12_LEN - 1,
        TX_CSR_20_BIT12_OFFSET,
        0x1);
    HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
        RX_CSR_REG_OFFSET(dsIndex, RX_CSR_RX_CSR18_REG),
        RX_CSR_18_BIT12_OFFSET + RX_CSR_18_BIT12_LEN - 1,
        RX_CSR_18_BIT12_OFFSET,
        0x1);

    do {
        HAL_UsSleep(10);
        cnt++;
        rxSdpiEventAck = HILINK_RegReadBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
            RX_CSR_REG_OFFSET(dsIndex, RX_CSR_RX_CSR15_REG),
            RX_CSR_15_BIT0_OFFSET + RX_CSR_15_BIT0_LEN - 1,
            RX_CSR_15_BIT0_OFFSET);
        txSdpiEventAck = HILINK_RegReadBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
            TX_CSR_REG_OFFSET(dsIndex, TX_CSR_TX_CSR16_REG),
            TX_CSR_16_BIT17_OFFSET + TX_CSR_16_BIT17_LEN - 1,
            TX_CSR_16_BIT17_OFFSET);
    } while ((rxSdpiEventAck == 0 || txSdpiEventAck == 0) && (cnt < DS_DATA_RATE_SWITCH_TIMEOUT_CNT));
    HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
        RX_CSR_REG_OFFSET(dsIndex, RX_CSR_RX_CSR75_REG),
        RX_CSR_75_BIT0_OFFSET + RX_CSR_75_BIT0_LEN - 1,
        RX_CSR_75_BIT0_OFFSET,
        0x0);
    if (cnt >= DS_DATA_RATE_SWITCH_TIMEOUT_CNT) {
        HILINK_ERR("macro%d ds%d DsDataRateSwitch timeout!", macro, dsIndex);
        return RET_FAIL;
    }
    return RET_OK;
}
EXPORT_SYMBOL(DsDataRateSwitch);

u32 HILINK_DsDataRateSwitch(u32 macro, u32 dsIndex, HilinkRate targetRate, u32 dsWidth)
{
    u32 ret;
    CHECK_RETURN(macro, MACRO_MAX, RET_FAIL);
    CHECK_RETURN(dsIndex, DS_MAX, RET_FAIL);
    CHECK_RETURN(targetRate, HILINK_XGE_BUTT, RET_FAIL);
    SwitchControlMode(macro, dsIndex, CTRL_MODE_REG);
    EthScenarioDataRateConfig(macro, CS0, targetRate);
    ret = DsDataRateSwitch(macro, dsIndex, targetRate, dsWidth);
    if (ret == RET_FAIL) {
        HILINK_ERR("HILINK_DsDataRateSwitch failed.\n");
        goto exit;
    }
    ret = eth_rx_para_config(macro, dsIndex, targetRate);
    if (ret != RET_OK) {
        HILINK_ERR("Eth rx para configuration failed.\n");
    }
exit:
    SwitchControlMode(macro, dsIndex, CTRL_MODE_PIN);
    return ret;
}
EXPORT_SYMBOL(HILINK_DsDataRateSwitch);

bool LinkDataRateSwitch(u32 macro, u32 sliceMask, HilinkRate targetRate, u32 dsWidth)
{
    u32 rateCmd, readData;
    u32 rxSdpiEventAck, txSdpiEventAck;
    u32 rateSwitchId = 0x10000000;
    u32 cnt = 0;
    u32 i;
    HilinkPara *hilinkPara = HILINK_GetParaCfg();
    CHECK_RETURN(macro, MACRO_MAX, false);
    CHECK_RETURN(targetRate, HILINK_XGE_BUTT, false);

    rateCmd = rateSwitchId + ((dsWidth << DS_DATA_WIDTH_OFFSET) & (0x1C0)) + targetRate;

    for (i = 0; i < DS_MAX; i++) {
        if (((sliceMask >> i) & 0x1) == 0x1) {
            HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
                RX_CSR_REG_OFFSET(i, RX_CSR_RX_CSR75_REG),
                RX_CSR_75_BIT0_OFFSET + RX_CSR_75_BIT0_LEN - 1,
                RX_CSR_75_BIT0_OFFSET,
                0x1);
            readData = HILINK_RegReadBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
                TX_CSR_REG_OFFSET(i, TX_CSR_TX_CSR20_REG),
                TX_CSR_20_BIT18_OFFSET + TX_CSR_20_BIT18_LEN - 1,
                TX_CSR_20_BIT18_OFFSET);
            HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
                TX_CSR_REG_OFFSET(i, TX_CSR_TX_CSR20_REG),
                TX_CSR_20_BIT18_OFFSET + TX_CSR_20_BIT18_LEN - 1,
                TX_CSR_20_BIT18_OFFSET,
                readData | 0x2);

            readData = HILINK_RegReadBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
                RX_CSR_REG_OFFSET(i, RX_CSR_RX_CSR18_REG),
                RX_CSR_18_BIT18_OFFSET + RX_CSR_18_BIT18_LEN - 1,
                RX_CSR_18_BIT18_OFFSET);
            HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
                RX_CSR_REG_OFFSET(i, RX_CSR_RX_CSR18_REG),
                RX_CSR_18_BIT18_OFFSET + RX_CSR_18_BIT18_LEN - 1,
                RX_CSR_18_BIT18_OFFSET,
                readData | 0x2);
        }
    }
    for (i = 0; i < DS_MAX; i++) {
        if (((sliceMask >> i) & 0x1) == 0x1) {
            HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
                TX_CSR_REG_OFFSET(i, TX_CSR_TX_CSR19_REG),
                TX_CSR_19_BIT0_OFFSET + TX_CSR_19_BIT0_LEN - 1,
                TX_CSR_19_BIT0_OFFSET,
                rateCmd);
            HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
                RX_CSR_REG_OFFSET(i, RX_CSR_RX_CSR17_REG),
                RX_CSR_17_BIT0_OFFSET + RX_CSR_17_BIT0_LEN - 1,
                RX_CSR_17_BIT0_OFFSET,
                rateCmd);
        }
    }
    for (i = 0; i < DS_MAX; i++) {
        if (((sliceMask >> i) & 0x1) == 0x1) {
            HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
                TX_CSR_REG_OFFSET(i, TX_CSR_TX_CSR20_REG),
                TX_CSR_20_BIT12_OFFSET + TX_CSR_20_BIT12_LEN - 1,
                TX_CSR_20_BIT12_OFFSET,
                0x1);
            HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
                RX_CSR_REG_OFFSET(i, RX_CSR_RX_CSR18_REG),
                RX_CSR_18_BIT12_OFFSET + RX_CSR_18_BIT12_LEN - 1,
                RX_CSR_18_BIT12_OFFSET,
                0x1);
        }
    }

    do {
        HAL_UsSleep(10);
        cnt++;
        rxSdpiEventAck = 1;
        txSdpiEventAck = 1;
        for (i = 0; i < DS_MAX; i++) {
            if (((sliceMask >> i) & 0x1) == 0x1) {
                rxSdpiEventAck &= HILINK_RegReadBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
                    RX_CSR_REG_OFFSET(i, RX_CSR_RX_CSR15_REG),
                    RX_CSR_15_BIT0_OFFSET + RX_CSR_15_BIT0_LEN - 1,
                    RX_CSR_15_BIT0_OFFSET);
                txSdpiEventAck &= HILINK_RegReadBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
                    TX_CSR_REG_OFFSET(i, TX_CSR_TX_CSR16_REG),
                    TX_CSR_16_BIT17_OFFSET + TX_CSR_16_BIT17_LEN - 1,
                    TX_CSR_16_BIT17_OFFSET);
            }
        }
    } while ((rxSdpiEventAck == 0 || txSdpiEventAck == 0) && (cnt < DS_DATA_RATE_SWITCH_TIMEOUT_CNT * DS_MAX));
    for (i = 0; i < DS_MAX; i++) {
        if (((sliceMask >> i) & 0x1) == 0x1) {
            HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
                RX_CSR_REG_OFFSET(i, RX_CSR_RX_CSR75_REG),
                RX_CSR_75_BIT0_OFFSET + RX_CSR_75_BIT0_LEN - 1,
                RX_CSR_75_BIT0_OFFSET,
                0x0);
        }
    }
    if (cnt >= DS_DATA_RATE_SWITCH_TIMEOUT_CNT * DS_MAX) {
        HILINK_ERR("macro%d mask %d LinkDataRateSwitch timeout!", macro, sliceMask);
        return false;
    }
    return true;
}
EXPORT_SYMBOL(LinkDataRateSwitch);

u32 HILINK_LinkDataRateSwitch(u32 macro, u32 sliceMask, HilinkRate targetRate, u32 dsWidth)
{
    u32 ret;
    u32 i;
    CHECK_RETURN(macro, MACRO_MAX, false);
    CHECK_RETURN(targetRate, HILINK_XGE_BUTT, RET_FAIL);
    for (i = 0; i < DS_MAX; i++) {
        if (((sliceMask >> i) & 0x1) == 0x1) {
            SwitchControlMode(macro, i, CTRL_MODE_REG);
        }
    }
    EthScenarioDataRateConfig(macro, CS0, targetRate);
    ret = LinkDataRateSwitch(macro, sliceMask, targetRate, dsWidth);
    if (ret == RET_FAIL) {
        HILINK_ERR("HILINK_DsDataRateSwitch failed.\n");
        goto exit;
    }
    for (i = 0; i < DS_MAX; ++i) {
        if (((sliceMask >> i) & 0x1) == 0x1) {
            ret = eth_rx_para_config(macro, i, targetRate);
            if (ret != RET_OK) {
                HILINK_ERR("Eth rx para configuration failed.\n");
            }
        }
    }
exit:
    for (i = 0; i < DS_MAX; i++) {
        if (((sliceMask >> i) & 0x1) == 0x1) {
            SwitchControlMode(macro, i, CTRL_MODE_PIN);
        }
    }
    return ret;
}
EXPORT_SYMBOL(HILINK_LinkDataRateSwitch);

STATIC const PowerSwitchConfig_t *GetPowerConfig(PowerSwitchTypeE type)
{
    if (type < PS_TYPE_END) {
        if (g_powerSwitchConfig[type].type == type) {
            return &g_powerSwitchConfig[type];
        }
    }
    HILINK_ERR("Power type invalid.(type=%d)\n", type);
    return NULL;
}

STATIC u32 DsPowerModeSwitchStart(u32 macro, u32 dsIndex, PowerSwitchTypeE type)
{
    u32 cmd;
    u32 id;
    u32 readData;
    u32 currPower;
    HilinkPara *hilinkPara = HILINK_GetParaCfg();
    const PowerSwitchConfig_t *pConfig = GetPowerConfig(type);
    if (pConfig == NULL) {
        HILINK_ERR("Get power type failed!\n");
        return HILINK_FAILED;
    }

    currPower = HILINK_RegReadBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
        TX_CSR_REG_OFFSET(dsIndex, TX_CSR_TX_CSR21_REG),
        TX_CSR_21_BIT12_OFFSET + TX_CSR_21_BIT12_LEN - 1,
        TX_CSR_21_BIT12_OFFSET);
    id = 0x0;
    cmd = id + pConfig->mode;
    readData = HILINK_RegReadBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
        TX_CSR_REG_OFFSET(dsIndex, TX_CSR_TX_CSR20_REG),
        TX_CSR_20_BIT18_OFFSET + TX_CSR_20_BIT18_LEN - 1,
        TX_CSR_20_BIT18_OFFSET);
    HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
        TX_CSR_REG_OFFSET(dsIndex, TX_CSR_TX_CSR20_REG),
        TX_CSR_20_BIT18_OFFSET + TX_CSR_20_BIT18_LEN - 1,
        TX_CSR_20_BIT18_OFFSET,
        readData | 0x1);

    readData = HILINK_RegReadBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
        RX_CSR_REG_OFFSET(dsIndex, RX_CSR_RX_CSR18_REG),
        RX_CSR_18_BIT18_OFFSET + RX_CSR_18_BIT18_LEN - 1,
        RX_CSR_18_BIT18_OFFSET);
    HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
        RX_CSR_REG_OFFSET(dsIndex, RX_CSR_RX_CSR18_REG),
        RX_CSR_18_BIT18_OFFSET + RX_CSR_18_BIT18_LEN - 1,
        RX_CSR_18_BIT18_OFFSET,
        readData | 0x1);

    HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
        TX_CSR_REG_OFFSET(dsIndex, TX_CSR_TX_CSR19_REG),
        TX_CSR_19_BIT0_OFFSET + TX_CSR_19_BIT0_LEN - 1,
        TX_CSR_19_BIT0_OFFSET,
        cmd);
    HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
        RX_CSR_REG_OFFSET(dsIndex, RX_CSR_RX_CSR17_REG),
        RX_CSR_17_BIT0_OFFSET + RX_CSR_17_BIT0_LEN - 1,
        RX_CSR_17_BIT0_OFFSET,
        cmd);

    HILINK_INFO("Power mode(%d). cur(%d)\n", pConfig->mode, currPower);
    if (pConfig->mode > currPower) { // switch to lp status
        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
            RX_CSR_REG_OFFSET(dsIndex, RX_CSR_RX_CSR18_REG),
            RX_CSR_18_BIT12_OFFSET + RX_CSR_18_BIT12_LEN - 1,
            RX_CSR_18_BIT12_OFFSET,
            0x1);
        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
            TX_CSR_REG_OFFSET(dsIndex, TX_CSR_TX_CSR20_REG),
            TX_CSR_20_BIT12_OFFSET + TX_CSR_20_BIT12_LEN - 1,
            TX_CSR_20_BIT12_OFFSET,
            0x1);
    } else { // exit lp status
        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
            TX_CSR_REG_OFFSET(dsIndex, TX_CSR_TX_CSR20_REG),
            TX_CSR_20_BIT12_OFFSET + TX_CSR_20_BIT12_LEN - 1,
            TX_CSR_20_BIT12_OFFSET,
            0x1);
        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
            RX_CSR_REG_OFFSET(dsIndex, RX_CSR_RX_CSR18_REG),
            RX_CSR_18_BIT12_OFFSET + RX_CSR_18_BIT12_LEN - 1,
            RX_CSR_18_BIT12_OFFSET,
            0x1);
    }
    return HILINK_OK;
}

u32 DsPowerModeSwitch(u32 macro, u32 dsIndex, PowerSwitchTypeE type)
{
    HilinkPara *hilinkPara = HILINK_GetParaCfg();
    u32 checkCount = 0;
    u32 rxSdpiEventAck;
    u32 txSdpiEventAck;
    u32 ret;
    CHECK_RETURN(macro, MACRO_MAX, RET_FAIL);
    CHECK_RETURN(dsIndex, DS_MAX, RET_FAIL);

    ret = DsPowerModeSwitchStart(macro, dsIndex, type);
    if (ret == HILINK_FAILED) {
        HILINK_ERR("Start power mode switch failed!\n");
        return RET_FAIL;
    }
    do {
        HAL_UsSleep(1);
        checkCount++;
        rxSdpiEventAck = HILINK_RegReadBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
            RX_CSR_REG_OFFSET(dsIndex, RX_CSR_RX_CSR15_REG),
            RX_CSR_15_BIT0_OFFSET + RX_CSR_15_BIT0_LEN - 1,
            RX_CSR_15_BIT0_OFFSET);
        txSdpiEventAck = HILINK_RegReadBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
            TX_CSR_REG_OFFSET(dsIndex, TX_CSR_TX_CSR16_REG),
            TX_CSR_16_BIT17_OFFSET + TX_CSR_16_BIT17_LEN - 1, TX_CSR_16_BIT17_OFFSET);
    } while (((rxSdpiEventAck == 0) || (txSdpiEventAck == 0)) && (checkCount < DS_POWER_MODE_SWITCH_TIMEOUT));
    if (checkCount >= DS_POWER_MODE_SWITCH_TIMEOUT) {
        HILINK_ERR("Ds power mode switch timeout!\n");
        return RET_FAIL;
    }
    return RET_OK;
}
#ifdef CFG_BUILD_DEBUG
EXPORT_SYMBOL(DsPowerModeSwitch);
#endif

u32 TxSdpiCmdExecute(u32 macro, u32 txId, u32 txSdpiCmd)
{
    HilinkPara *hilinkPara = HILINK_GetParaCfg();
    void* hilinkRegBase = hilinkPara->macro_info[macro].hilink_reg_base;
    u32 checkCnt = 0;
    u32 readData;

    HILINK_RegWriteBits(hilinkRegBase,
        TX_CSR_REG_OFFSET(txId, TX_CSR_TX_CSR19_REG),
        TX_CSR_19_BIT0_OFFSET + TX_CSR_19_BIT0_LEN - 1,
        TX_CSR_19_BIT0_OFFSET,
        txSdpiCmd);
    HILINK_RegWriteBits(hilinkRegBase,
        TX_CSR_REG_OFFSET(txId, TX_CSR_TX_CSR20_REG),
        TX_CSR_20_BIT12_OFFSET + TX_CSR_20_BIT12_LEN - 1,
        TX_CSR_20_BIT12_OFFSET,
        0x1);
    do {
        HAL_UsSleep(TX_SDPI_CMD_EXECUTE_SLEEP_US);
        checkCnt++;
        readData = HILINK_RegReadBits(hilinkRegBase, TX_CSR_REG_OFFSET(txId, TX_CSR_TX_CSR16_REG),
            TX_CSR_16_BIT17_OFFSET + TX_CSR_16_BIT17_LEN - 1, TX_CSR_16_BIT17_OFFSET);
    } while ((readData == 0x0U) && (checkCnt < TX_SDPI_CMD_EXECUTE_TIMEOUT_CNT));
    if (checkCnt >= TX_SDPI_CMD_EXECUTE_TIMEOUT_CNT) {
        HILINK_ERR("Tx sdpi cmd execute timeout!\n");
        return HILINK_FAILED;
    }
    readData = HILINK_RegReadBits(hilinkRegBase, TX_CSR_REG_OFFSET(txId, TX_CSR_TX_CSR16_REG),
        TX_CSR_16_BIT0_OFFSET + TX_CSR_16_BIT0_LEN - 1, TX_CSR_16_BIT0_OFFSET);
    return readData;
}

u32 SetTxFFE(u32 macro, u32 txId, u32 txMargin, u32 txDeemph)
{
    HilinkPara *hilinkPara = HILINK_GetParaCfg();
    void* hilinkRegBase;
    u32 txDrvCtrlCmdID = TX_DRIVER_CONTROL_CMD_ID;
    u32 txDrvCtrlCmd;
    u32 readData;

    CHECK_RETURN(macro, MACRO_MAX, HILINK_FAILED);
    CHECK_RETURN(txId, LANE_MAX, HILINK_FAILED);

    hilinkRegBase = hilinkPara->macro_info[macro].hilink_reg_base;

    readData = HILINK_RegReadBits(hilinkRegBase,
        TX_CSR_REG_OFFSET(txId, TX_CSR_TX_CSR20_REG),
        TX_CSR_20_BIT18_OFFSET + TX_CSR_20_BIT18_LEN - 1,
        TX_CSR_20_BIT18_OFFSET);
    HILINK_RegWriteBits(hilinkRegBase,
        TX_CSR_REG_OFFSET(txId, TX_CSR_TX_CSR20_REG),
        TX_CSR_20_BIT18_OFFSET + TX_CSR_20_BIT18_LEN - 1,
        TX_CSR_20_BIT18_OFFSET,
        readData & 0xEF);

    txDrvCtrlCmd = txDrvCtrlCmdID + (txMargin << SET_TX_DEE_TX_MARGIN_OFFSET) + txDeemph;
    if (TxSdpiCmdExecute(macro, txId, txDrvCtrlCmd) == HILINK_FAILED) {
        HILINK_ERR("Tx sdpi cmd execute failed!\n");
        return HILINK_FAILED;
    }
    return HILINK_OK;
}
#ifdef CFG_BUILD_DEBUG
EXPORT_SYMBOL(SetTxFFE);
#endif
