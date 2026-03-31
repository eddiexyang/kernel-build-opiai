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

#include "sdk_hilink_init.h"
#include "sdk_hilink_cfg.h"
#include "hilink_fw_data_h25.h"

#include <linux/types.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/kthread.h>

#include "hsm_norflash.h"
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

STATIC int g_retWriteAOReg;
static int g_hilinkReadyFlag = 0;
#ifdef CFG_SOC_PLATFORM_MDC_V11
STATIC struct serdes_dts_param g_serdes_cfg[MACRO_MAX];
#endif

STATIC int HILINK_init_macro_info(void)
{
    HilinkPara *hilinkPara = HILINK_GetParaCfg();

    hilinkPara->macro_info[MACRO_0].hilink_reg_base = ioremap(HILINK0_REG_BASE, HILINK0_REG_SIZE);
    if (hilinkPara->macro_info[MACRO_0].hilink_reg_base == NULL) {
        HILINK_ERR_SNAPSHOT(SERDES_INIT_REMAP_REG0_FAIL, "HILINK0_REG ioremap failed.\n");
        return -ENOMEM;
    }

    hilinkPara->macro_info[MACRO_0].hilink_subctrl_base = ioremap(HILINK0_SUBCTRL_BASE, HILINK0_SUBCTRL_SIZE);
    if (hilinkPara->macro_info[MACRO_0].hilink_subctrl_base == NULL) {
        HILINK_ERR_SNAPSHOT(SERDES_INIT_REMAP_SUBCTRL0_FAIL, "HILINK0_SUBCTRL ioremap failed.\n");
        goto HILINK0_REG_MAP_ERR;
    }

    hilinkPara->macro_info[MACRO_1].hilink_reg_base = ioremap(HILINK1_REG_BASE, HILINK1_REG_SIZE);
    if (hilinkPara->macro_info[MACRO_1].hilink_reg_base == NULL) {
        HILINK_ERR_SNAPSHOT(SERDES_INIT_REMAP_REG1_FAIL, "HILINK1_REG ioremap failed.\n");
        goto HILINK0_SUBCTRL_MAP_ERR;
    }

    hilinkPara->macro_info[MACRO_1].hilink_subctrl_base = ioremap(HILINK1_SUBCTRL_BASE, HILINK1_SUBCTRL_SIZE);
    if (hilinkPara->macro_info[MACRO_1].hilink_subctrl_base == NULL) {
        HILINK_ERR_SNAPSHOT(SERDES_INIT_REMAP_SUBCTRL1_FAIL, "HILINK1_SUBCTRL ioremap failed.\n");
        goto HILINK1_REG_MAP_ERR;
    }

    hilinkPara->io_subctrl_base = ioremap(IO_SUBCTRL_REG_BASE, IO_SUBCTRL_REG_SIZE);
    if (hilinkPara->io_subctrl_base == NULL) {
        HILINK_ERR_SNAPSHOT(SERDES_INIT_REMAP_IO_FAIL, "IO_SUBCTRL ioremap failed.\n");
        goto HILINK1_SUBCTRL_MAP_ERR;
    }

    return 0;

HILINK1_SUBCTRL_MAP_ERR:
    iounmap(hilinkPara->macro_info[MACRO_1].hilink_subctrl_base);
    hilinkPara->macro_info[MACRO_1].hilink_subctrl_base = NULL;
HILINK1_REG_MAP_ERR:
    iounmap(hilinkPara->macro_info[MACRO_1].hilink_reg_base);
    hilinkPara->macro_info[MACRO_1].hilink_reg_base = NULL;
HILINK0_SUBCTRL_MAP_ERR:
    iounmap(hilinkPara->macro_info[MACRO_0].hilink_subctrl_base);
    hilinkPara->macro_info[MACRO_0].hilink_subctrl_base = NULL;
HILINK0_REG_MAP_ERR:
    iounmap(hilinkPara->macro_info[MACRO_0].hilink_reg_base);
    hilinkPara->macro_info[MACRO_0].hilink_reg_base = NULL;

    return -ENOMEM;
}

#ifdef FEATURE_HIGH_IMPEDANCE_ON_SUSPEND
STATIC int HILINK_RxHighImpedance(void)
{
    u32 i;
    HilinkPara *hilinkPara = HILINK_GetParaCfg();

    if (hilinkPara->macro_info[MACRO_0].hilink_reg_base == NULL) {
        HILINK_ERR_SNAPSHOT(SERDES_SUSPEND_CHECK_MACRO0_HILINK_REG_BASE_FAIL, "Macro0 hilink_reg_base is null.\n");
        return -ENOMEM;
    }

    if (hilinkPara->macro_info[MACRO_1].hilink_reg_base == NULL) {
        HILINK_ERR_SNAPSHOT(SERDES_SUSPEND_CHECK_MACRO1_HILINK_REG_BASE_FAIL, "Macro1 hilink_reg_base is null.\n");
        return -ENOMEM;
    }

    for (i = 0; i < LANE_MAX; ++i) {
        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[MACRO_0].hilink_reg_base,
            RX_CSR_REG_OFFSET(i, RX_CSR_RX_CSR27_REG),
            RX_CSR_27_BIT0_OFFSET + RX_CSR_27_BIT0_LEN - 1,
            RX_CSR_27_BIT0_OFFSET,
            hilinkPara->macro_cfg[MACRO_0].ds[i].type == SERDES_TYPE_USB ? 0x2 : 0x1);

        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[MACRO_1].hilink_reg_base,
            RX_CSR_REG_OFFSET(i, RX_CSR_RX_CSR27_REG),
            RX_CSR_27_BIT0_OFFSET + RX_CSR_27_BIT0_LEN - 1,
            RX_CSR_27_BIT0_OFFSET,
            hilinkPara->macro_cfg[MACRO_1].ds[i].type == SERDES_TYPE_USB ? 0x2 : 0x1);
    }

    return 0;
}
#endif

STATIC void HILINK_RxLowImpedance(u32 macro)
{
    u32 i;
    u32 rxInitVal;
    HilinkPara *hilinkPara = HILINK_GetParaCfg();

    for (i = 0; i < LANE_MAX; ++i) {
        // avoiding sds_rxX_1k_hiz_sel setting, using default
        rxInitVal = HILINK_RegReadBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
            RX_CSR_REG_OFFSET(i, RX_CSR_RX_CSR80_REG),
            RX_CSR_80_BIT0_OFFSET + RX_CSR_80_BIT0_LEN - 1,
            RX_CSR_80_BIT0_OFFSET);
        rxInitVal = (rxInitVal & 0xFC) + 0x2;
        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
            RX_CSR_REG_OFFSET(i, RX_CSR_RX_CSR80_REG),
            RX_CSR_80_BIT0_OFFSET + RX_CSR_80_BIT0_LEN - 1,
            RX_CSR_80_BIT0_OFFSET,
            rxInitVal);
        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
            RX_CSR_REG_OFFSET(i, RX_CSR_RX_CSR27_REG),
            RX_CSR_27_BIT4_OFFSET + RX_CSR_27_BIT4_LEN - 1,
            RX_CSR_27_BIT4_OFFSET,
            0x1);
        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
            RX_CSR_REG_OFFSET(i, RX_CSR_RX_CSR107_REG),
            RX_CSR_107_BIT9_OFFSET + RX_CSR_107_BIT9_LEN - 1,
            RX_CSR_107_BIT9_OFFSET,
            0xF);
        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
            RX_CSR_REG_OFFSET(i, RX_CSR_RX_CSR27_REG),
            RX_CSR_27_BIT0_OFFSET + RX_CSR_27_BIT0_LEN - 1,
            RX_CSR_27_BIT0_OFFSET,
            hilinkPara->macro_cfg[macro].ds[i].type == SERDES_TYPE_USB ? 0x1 : 0x2);
    }
}

STATIC void HILINK_RefClkInit(u32 macro)
{
    HilinkPara *hilinkPara = HILINK_GetParaCfg();

    if (macro == MACRO_0) {
        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base, CP_CSR_CP_CSR34_REG,
            CP_CSR_34_BIT0_OFFSET + CP_CSR_34_BIT0_LEN - 1,
            CP_CSR_34_BIT0_OFFSET,
            0x0);
        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base, CP_CSR_CP_CSR34_REG,
            CP_CSR_34_BIT3_OFFSET + CP_CSR_34_BIT3_LEN - 1,
            CP_CSR_34_BIT3_OFFSET,
            0x1);
        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base, CP_CSR_CP_CSR34_REG,
            CP_CSR_34_BIT4_OFFSET + CP_CSR_34_BIT4_LEN - 1,
            CP_CSR_34_BIT4_OFFSET,
            0x3);
        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base, CP_CSR_CP_CSR34_REG,
            CP_CSR_34_BIT6_OFFSET + CP_CSR_34_BIT6_LEN - 1,
            CP_CSR_34_BIT6_OFFSET,
            0x3);
    } else {
        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base, CP_CSR_CP_CSR34_REG,
            CP_CSR_34_BIT0_OFFSET + CP_CSR_34_BIT0_LEN - 1,
            CP_CSR_34_BIT0_OFFSET,
            0x3);
        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base, CP_CSR_CP_CSR34_REG,
            CP_CSR_34_BIT3_OFFSET + CP_CSR_34_BIT3_LEN - 1,
            CP_CSR_34_BIT3_OFFSET,
            0x1);
        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base, CP_CSR_CP_CSR34_REG,
            CP_CSR_34_BIT4_OFFSET + CP_CSR_34_BIT4_LEN - 1,
            CP_CSR_34_BIT4_OFFSET,
            0x2);
        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base, CP_CSR_CP_CSR34_REG,
            CP_CSR_34_BIT6_OFFSET + CP_CSR_34_BIT6_LEN - 1,
            CP_CSR_34_BIT6_OFFSET,
            0x1);
    }

    HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base, CP_CSR_CP_CSR34_REG,
        CP_CSR_34_BIT9_OFFSET + CP_CSR_34_BIT9_LEN - 1,
        CP_CSR_34_BIT9_OFFSET,
        0x0);
    HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base, CP_CSR_CP_CSR34_REG,
        CP_CSR_34_BIT8_OFFSET + CP_CSR_34_BIT8_LEN - 1,
        CP_CSR_34_BIT8_OFFSET,
        0x0);

    HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
        CS_CSR_REG_OFFSET(CS0, CS_CSR_CS_CSR58_REG),
        CS_CSR_58_BIT7_OFFSET + CS_CSR_58_BIT7_LEN - 1,
        CS_CSR_58_BIT7_OFFSET,
        macro == MACRO_0 ? 0x0 : 0x3);

    HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base, CP_CSR_CP_CSR34_REG,
        CP_CSR_34_BIT10_OFFSET + CP_CSR_34_BIT10_LEN - 1,
        CP_CSR_34_BIT10_OFFSET,
        0x3);
    HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base, CP_CSR_CP_CSR34_REG,
        CP_CSR_34_BIT13_OFFSET + CP_CSR_34_BIT13_LEN - 1,
        CP_CSR_34_BIT13_OFFSET,
        0x1);
    HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base, CP_CSR_CP_CSR34_REG,
        CP_CSR_34_BIT14_OFFSET + CP_CSR_34_BIT14_LEN - 1,
        CP_CSR_34_BIT14_OFFSET,
        0x0);
    HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base, CP_CSR_CP_CSR34_REG,
        CP_CSR_34_BIT16_OFFSET + CP_CSR_34_BIT16_LEN - 1,
        CP_CSR_34_BIT16_OFFSET,
        0x1);
    HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base, CP_CSR_CP_CSR34_REG,
        CP_CSR_34_BIT19_OFFSET + CP_CSR_34_BIT19_LEN - 1,
        CP_CSR_34_BIT19_OFFSET,
        0x0);

    HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
        CS_CSR_REG_OFFSET(CS1, CS_CSR_CS_CSR58_REG),
        CS_CSR_58_BIT7_OFFSET + CS_CSR_58_BIT7_LEN - 1,
        CS_CSR_58_BIT7_OFFSET,
        0x3);

    HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base, CP_CSR_CP_CSR34_REG,
        CP_CSR_34_BIT18_OFFSET + CP_CSR_34_BIT18_LEN - 1,
        CP_CSR_34_BIT18_OFFSET,
        0x0);

    return;
}

STATIC void HILINK_McuReset(u32 macro)
{
    HilinkPara *hilinkPara = HILINK_GetParaCfg();
    HilinkRegBase base;
    Hilink_SetRegBase(&base, hilinkPara, macro, 0);

    HILINK_CPRegWriteBits(base, CP_CSR_CP_CSR21_REG,
        CP_CSR_21_BIT31_OFFSET, CP_CSR_21_BIT31_LEN,
        0x0);
    HILINK_CPRegWriteBits(base, CP_CSR_CP_CSR21_REG,
        CP_CSR_21_BIT28_OFFSET, CP_CSR_21_BIT28_LEN,
        0x0);
    HILINK_CPRegWriteBits(base, CP_CSR_CP_CSR33_REG,
        CP_CSR_33_BIT30_OFFSET, CP_CSR_33_BIT30_LEN,
        0x1);
    return;
}

STATIC void HILINK_McuRelease(u32 macro, u32 porRstPc)
{
    HilinkRegBase base;
    Hilink_SetRegBase(&base, HILINK_GetParaCfg(), macro, 0);

    HILINK_CPRegWriteBits(base, CP_CSR_CP_CSR21_REG, CP_CSR_21_BIT28_OFFSET, CP_CSR_21_BIT28_LEN, 0x1);
    HILINK_CPRegWriteBits(base, CP_CSR_CP_CSR21_REG, CP_CSR_21_BIT31_OFFSET, CP_CSR_21_BIT31_LEN, 0x1);
    HILINK_CPRegWriteBits(base, CP_CSR_CP_CSR33_REG, CP_CSR_33_BIT30_OFFSET, CP_CSR_33_BIT30_LEN, 0x1);
    HILINK_CPRegWriteBits(base, CP_CSR_CP_CSR28_REG, CP_CSR_28_BIT0_OFFSET, CP_CSR_28_BIT0_LEN, porRstPc);
    HILINK_CPRegWriteBits(base, CP_CSR_CP_CSR33_REG, CP_CSR_33_BIT30_OFFSET, CP_CSR_33_BIT30_LEN, 0x0);
}

STATIC void HILINK_LoadSerDesFw(u32 macro)
{
    HilinkPara *hilinkPara = HILINK_GetParaCfg();
    const u32* fwData = HILINK_GetFirmwareData();
    memcpy_toio(hilinkPara->macro_info[macro].hilink_reg_base, fwData, FIRMWARE_LEN * sizeof(u32));
}

STATIC u32 HILINK_LoadFwToSram(u32 macro)
{
    HilinkPara *hilinkPara = HILINK_GetParaCfg();

    /* Set SRAM_Cnt to 1 */
    HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base, CP_CSR_CP_CSR16_REG,
        CP_CSR_16_BIT0_OFFSET + CP_CSR_16_BIT0_LEN - 1,
        CP_CSR_16_BIT0_OFFSET,
        0x1);
    HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base, CP_CSR_CP_CSR16_REG,
        CP_CSR_16_BIT3_OFFSET + CP_CSR_16_BIT3_LEN - 1,
        CP_CSR_16_BIT3_OFFSET,
        0x1);

    HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_subctrl_base,
        HILINK_SUBCTRL_6_REG,
        HILINK_SUBCTRL_6_BIT0_OFFSET + HILINK_SUBCTRL_6_BIT0_LEN - 1,
        HILINK_SUBCTRL_6_BIT0_OFFSET,
        0x1);

    HAL_UsSleep(1);

    HILINK_McuReset(macro);

    if (HILINK_WaitFsmDone(macro) == false) {
        return RET_FAIL;
    }

    HILINK_LoadSerDesFw(macro);

    return RET_OK;
}

STATIC void HILINK_CsrScenMessageBusInit(u32 macro)
{
    u32 i;
    const u32 msgBus[SERDES_TYPE_NULL] = {1, 0, 0, 0};
    const u32 lane_mode[SERDES_TYPE_NULL] = {0, 1, 0, 0};
    HilinkPara *hilinkPara = HILINK_GetParaCfg();

    for (i = 0; i < DS_MAX; ++i) {
        if (hilinkPara->macro_cfg[macro].ds[i].type < SERDES_TYPE_NULL) {
            HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
                ADAP_CSR_REG_OFFSET(i, ADAP_LANE_CSR_ADAP_CSR1_REG),
                ADAP_LANE_CSR_1_BIT2_OFFSET + ADAP_LANE_CSR_1_BIT2_LEN - 1,
                ADAP_LANE_CSR_1_BIT2_OFFSET,
                msgBus[hilinkPara->macro_cfg[macro].ds[i].type]);
            HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
                ADAP_CSR_REG_OFFSET(i, ADAP_LANE_CSR_ADAP_CSR1_REG),
                ADAP_LANE_CSR_1_BIT1_OFFSET + ADAP_LANE_CSR_1_BIT1_LEN - 1,
                ADAP_LANE_CSR_1_BIT1_OFFSET,
                msgBus[hilinkPara->macro_cfg[macro].ds[i].type]);
            HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
                ADAP_CSR_REG_OFFSET(i, ADAP_LANE_CSR_ADAP_CSR1_REG),
                ADAP_LANE_CSR_1_BIT0_OFFSET + ADAP_LANE_CSR_1_BIT0_LEN - 1,
                ADAP_LANE_CSR_1_BIT0_OFFSET,
                msgBus[hilinkPara->macro_cfg[macro].ds[i].type]);
            HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
                ADAP_CSR_REG_OFFSET(i, ADAP_LANE_CSR_ADAP_CSR0_REG),
                ADAP_LANE_CSR_0_BIT0_OFFSET + ADAP_LANE_CSR_0_BIT0_LEN - 1,
                ADAP_LANE_CSR_0_BIT0_OFFSET,
                lane_mode[hilinkPara->macro_cfg[macro].ds[i].type]);
            if (hilinkPara->macro_cfg[macro].ds[i].type == SERDES_TYPE_PCIE) {
                HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
                    ADAP_CSR_REG_OFFSET(i, ADAP_LANE_CSR_ADAP_CSR12_REG),
                    ADAP_LANE_CSR_13_BIT18_OFFSET + ADAP_LANE_CSR_13_BIT18_LEN - 1, ADAP_LANE_CSR_13_BIT18_OFFSET,
                    0x8);
            }
        }
    }
}

STATIC void HILINK_CsrScenModeInit(u32 macro)
{
    HilinkPara *hilinkPara = HILINK_GetParaCfg();
    u32 clk_div[SERDES_TYPE_NULL] = {0, 0, 1, 1};
    u32 val = 0;
    u32 i;
    /* CSRScenarioInitial */
    HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_subctrl_base,
        HILINK_SUBCTRL_6_REG, HILINK_SUBCTRL_6_BIT0_OFFSET + HILINK_SUBCTRL_6_BIT0_LEN - 1,
        HILINK_SUBCTRL_6_BIT0_OFFSET, 0x0);
    HAL_UsSleep(1);
    HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base, CP_CSR_CP_CSR21_REG,
        CP_CSR_21_BIT22_OFFSET + CP_CSR_21_BIT22_LEN - 1, CP_CSR_21_BIT22_OFFSET, 0x0);
    HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base, CP_CSR_CP_CSR21_REG,
        CP_CSR_21_BIT20_OFFSET + CP_CSR_21_BIT20_LEN - 1, CP_CSR_21_BIT20_OFFSET, 0x0);

    for (i = 0; i < CS_MAX; ++i) {
        if (hilinkPara->macro_cfg[macro].cs[i].type < SERDES_TYPE_NULL) {
            val |= clk_div[hilinkPara->macro_cfg[macro].cs[i].type] << (i * HILINK_MCLK_DIV_OFFSET);
        }
    }
    HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base, CP_CSR_CP_CSR21_REG,
        CP_CSR_21_BIT15_OFFSET + CP_CSR_21_BIT15_LEN - 1, CP_CSR_21_BIT15_OFFSET,
        val);

    HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base, CP_CSR_CP_CSR21_REG,
        CP_CSR_21_BIT22_OFFSET + CP_CSR_21_BIT22_LEN - 1, CP_CSR_21_BIT22_OFFSET, 0x3);
    HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base, CP_CSR_CP_CSR21_REG,
        CP_CSR_21_BIT20_OFFSET + CP_CSR_21_BIT20_LEN - 1, CP_CSR_21_BIT20_OFFSET, 0x3);
    if (macro == MACRO_0) {
        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->io_subctrl_base, IO_SUBCTRL_SC_HILINK0_ICG_EN_REG,
            IO_SUBCTRL_ICG_EN_HILINK0_MCLK_OFFSET + IO_SUBCTRL_ICG_EN_HILINK0_MCLK_LEN - 1,
            IO_SUBCTRL_ICG_EN_HILINK0_MCLK_OFFSET, 0x3);
    } else {
        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->io_subctrl_base, IO_SUBCTRL_SC_HILINK1_ICG_EN_REG,
            IO_SUBCTRL_ICG_EN_HILINK1_MCLK_OFFSET + IO_SUBCTRL_ICG_EN_HILINK1_MCLK_LEN - 1,
            IO_SUBCTRL_ICG_EN_HILINK1_MCLK_OFFSET, 0x3);
    }
}

STATIC void HILINK_CsrScenTXOppositeSiteDetect(u32 macro)
{
    HilinkPara *hilinkPara = HILINK_GetParaCfg();
    u32 i;

    for (i = 0; i < DS_MAX; i++) {
        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
            TX_CSR_REG_OFFSET(i, TX_CSR_TX_CSR57_REG),
            TX_CSR_57_BIT20_OFFSET + TX_CSR_57_BIT20_LEN - 1,
            TX_CSR_57_BIT20_OFFSET, 0x1);
        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
            TX_CSR_REG_OFFSET(i, TX_CSR_TX_CSR57_REG),
            TX_CSR_57_BIT22_OFFSET + TX_CSR_57_BIT22_LEN - 1,
            TX_CSR_57_BIT22_OFFSET, 0x4);
        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
            TX_CSR_REG_OFFSET(i, TX_CSR_TX_CSR26_REG),
            TX_CSR_26_BIT13_OFFSET + TX_CSR_26_BIT13_LEN - 1,
            TX_CSR_26_BIT13_OFFSET, 0x3E8);
        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
            TX_CSR_REG_OFFSET(i, TX_CSR_TX_CSR27_REG),
            TX_CSR_27_BIT13_OFFSET + TX_CSR_27_BIT13_LEN - 1,
            TX_CSR_27_BIT13_OFFSET, 0x64);
        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
            TX_CSR_REG_OFFSET(i, TX_CSR_TX_CSR27_REG),
            TX_CSR_27_BIT2_OFFSET + TX_CSR_27_BIT2_LEN - 1,
            TX_CSR_27_BIT2_OFFSET, 0x3E8);
        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
            TX_CSR_REG_OFFSET(i, TX_CSR_TX_CSR26_REG),
            TX_CSR_26_BIT0_OFFSET + TX_CSR_26_BIT0_LEN - 1,
            TX_CSR_26_BIT0_OFFSET, 0x3E8);
    }
}

STATIC void HILINK_CsrScenRxOutsideClk(u32 macro)
{
    HilinkPara *hilinkPara = HILINK_GetParaCfg();
    u32 i;

    for (i = 0; i < DS_MAX; i++) {
        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
            RX_CSR_REG_OFFSET(i, RX_CSR_RX_CSR92_REG),
            RX_CSR_92_BIT27_OFFSET + RX_CSR_92_BIT27_LEN - 1,
            RX_CSR_92_BIT27_OFFSET, 0x0);
        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
            RX_CSR_REG_OFFSET(i, RX_CSR_RX_CSR107_REG),
            RX_CSR_107_BIT5_OFFSET + RX_CSR_107_BIT5_LEN - 1,
            RX_CSR_107_BIT5_OFFSET, 0x0);
    }
}

STATIC void HILINK_CsrScenTxRxInit(u32 macro)
{
    HilinkPara *hilinkPara = HILINK_GetParaCfg();
    u32 i;

    for (i = 0; i < DS_MAX; i++) {
        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
            RX_CSR_REG_OFFSET(i, RX_CSR_RX_CSR27_REG), RX_CSR_27_BIT9_OFFSET + RX_CSR_27_BIT9_LEN - 1,
            RX_CSR_27_BIT9_OFFSET, 0x1);
        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
            RX_CSR_REG_OFFSET(i, RX_CSR_RX_CSR30_REG),
            RX_CSR_30_BIT9_OFFSET + RX_CSR_30_BIT9_LEN - 1,
            RX_CSR_30_BIT9_OFFSET,
            hilinkPara->macro_cfg[macro].ds[i].type == SERDES_TYPE_USB ? 0x19 : 0x1f);
        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
            RX_CSR_REG_OFFSET(i, RX_CSR_RX_CSR30_REG),
            RX_CSR_30_BIT4_OFFSET + RX_CSR_30_BIT4_LEN - 1,
            RX_CSR_30_BIT4_OFFSET,
            hilinkPara->macro_cfg[macro].ds[i].type == SERDES_TYPE_USB ? 0x19 : 0x0);
        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
            TX_CSR_REG_OFFSET(i, TX_CSR_TX_CSR28_REG), TX_CSR_28_BIT9_OFFSET + TX_CSR_28_BIT9_LEN - 1,
            TX_CSR_28_BIT9_OFFSET, 0x0);
        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
            RX_CSR_REG_OFFSET(i, RX_CSR_RX_CSR26_REG),
            RX_CSR_26_BIT2_OFFSET + RX_CSR_26_BIT2_LEN - 1,
            RX_CSR_26_BIT2_OFFSET, 0x0);
        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
            RX_CSR_REG_OFFSET(i, RX_CSR_RX_CSR26_REG),
            RX_CSR_26_BIT1_OFFSET + RX_CSR_26_BIT1_LEN - 1,
            RX_CSR_26_BIT1_OFFSET,
            hilinkPara->macro_cfg[macro].ds[i].polarity_rx == 1 ? 1 : 0);
        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
            TX_CSR_REG_OFFSET(i, TX_CSR_TX_CSR31_REG), TX_CSR_31_BIT19_OFFSET + TX_CSR_31_BIT19_LEN - 1,
            TX_CSR_31_BIT19_OFFSET,
            hilinkPara->macro_cfg[macro].ds[i].polarity_tx == 1 ? 1 : 0);
        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
            RX_CSR_REG_OFFSET(i, RX_CSR_RX_CSR107_REG), RX_CSR_107_BIT7_OFFSET + RX_CSR_107_BIT7_LEN - 1,
            RX_CSR_107_BIT7_OFFSET, 0x0);
        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
            RX_CSR_REG_OFFSET(i, RX_CSR_RX_CSR75_REG), RX_CSR_75_BIT0_OFFSET + RX_CSR_75_BIT0_LEN - 1,
            RX_CSR_75_BIT0_OFFSET, 0x1);
    }
}

STATIC void HILINK_CsrScenInit(u32 macro)
{
    HilinkPara *hilinkPara = HILINK_GetParaCfg();
    u32 i;

    HILINK_CsrScenModeInit(macro);
    HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base, CP_CSR_CP_CSR34_REG,
        CP_CSR_34_BIT20_OFFSET + CP_CSR_34_BIT20_LEN - 1,
        CP_CSR_34_BIT20_OFFSET, 0x0);
    for (i = 0; i < DS_MAX; i++) {
        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
            RX_CSR_REG_OFFSET(i, RX_CSR_RX_CSR89_REG),
            RX_CSR_89_BIT18_OFFSET + RX_CSR_89_BIT18_LEN - 1,
            RX_CSR_89_BIT18_OFFSET, 0x1);
    }
    HILINK_CsrScenTXOppositeSiteDetect(macro);
    HILINK_CsrScenRxOutsideClk(macro);
    HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base, CP_CSR_CP_CSR13_REG,
        CP_CSR_13_BIT0_OFFSET + CP_CSR_13_BIT0_LEN - 1, CP_CSR_13_BIT0_OFFSET, 0xA);
    HILINK_CsrScenTxRxInit(macro);

    if (hilinkPara->macro_cfg[macro].cs[CS0].type == SERDES_TYPE_ETH) {
        EthScenarioDataRateConfig(macro, CS0, hilinkPara->macro_cfg[macro].cs[CS0].frequency);
        HILINK_INFO("Eth freq: %x\n", hilinkPara->macro_cfg[macro].cs[CS0].frequency);
    }

    HILINK_CsrScenMessageBusInit(macro);

    HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_reg_base,
        CP_CSR_CP_CSR18_REG, CP_CSR_18_BIT0_OFFSET + CP_CSR_18_BIT0_LEN - 1, CP_CSR_18_BIT0_OFFSET, 0x0);
    return;
}

bool hilink_check_all_lane_type(u32 macro, DRV_SERDES_TYPE type)
{
    HilinkPara *hilinkPara = HILINK_GetParaCfg();
    u32 i;

    for (i = 0; i < DS_MAX; ++i) {
        if (hilinkPara->macro_cfg[macro].ds[i].type != type) {
            return false;
        }
    }
    return true;
}

STATIC bool hilink_check_pcie1_using(void)
{
    HilinkPara *hilinkPara = HILINK_GetParaCfg();

    if ((hilinkPara->macro_cfg[MACRO_0].ds[DS2].type != SERDES_TYPE_PCIE) &&
        (hilinkPara->macro_cfg[MACRO_0].ds[DS3].type == SERDES_TYPE_PCIE)) {
        return true;
    }
    return false;
}

STATIC u32 hilink_check_pcie_ssc(u32 macro)
{
    HilinkPara *hilinkPara = HILINK_GetParaCfg();
    u32 val = 0;
    u32 i;
    for (i = 0; i < CS_MAX; ++i) {
        if ((hilinkPara->macro_cfg[macro].cs[i].type == SERDES_TYPE_PCIE) &&
            (hilinkPara->macro_cfg[macro].cs[i].ssc_enable == 1)) {
            val |= 0x1 << i;
        }
    }
    return val;
}

STATIC u32 HILINK_SoftwareInit(u32 macro)
{
    u32 val = 0;
    u32 ret;
    u32 i;
    HilinkPara *hilinkPara = HILINK_GetParaCfg();
    struct hilink_macro_cfg *macro_cfg = &(hilinkPara->macro_cfg[macro]);

    if (hilink_cfg_is_exist_prot(macro_cfg, SERDES_TYPE_PCIE) && !hilink_check_pcie1_using()) {
        val = hilink_check_pcie_ssc(macro);
    }
    HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_subctrl_base,
        HILINK_SUBCTRL_3_REG,
        HILINK_SUBCTRL_3_BIT0_OFFSET + HILINK_SUBCTRL_3_BIT0_LEN - 1,
        HILINK_SUBCTRL_3_BIT0_OFFSET,
        macro == MACRO_0 ? val : 0x0);

    val = 0;
    if (hilink_cfg_is_exist_prot(macro_cfg, SERDES_TYPE_PCIE) && hilink_check_pcie1_using()) {
        val = hilink_check_pcie_ssc(macro);
    }
    HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_subctrl_base,
        HILINK_SUBCTRL_4_REG,
        HILINK_SUBCTRL_4_BIT0_OFFSET + HILINK_SUBCTRL_4_BIT0_LEN - 1,
        HILINK_SUBCTRL_4_BIT0_OFFSET,
        macro == MACRO_0 ? val : 0x0);

    HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_subctrl_base,
        HILINK_SUBCTRL_0_REG,
        HILINK_SUBCTRL_0_BIT0_OFFSET + HILINK_SUBCTRL_0_BIT0_LEN - 1,
        HILINK_SUBCTRL_0_BIT0_OFFSET,
        ((hilinkPara->macro_cfg[macro].cs[CS0].ssc_enable == 1) &&
        (hilinkPara->macro_cfg[macro].cs[CS0].type == SERDES_TYPE_SATA)) ? 0x1 : 0x0);

    val = 0;
    if (hilink_cfg_is_exist_prot(macro_cfg, SERDES_TYPE_PCIE)) {
        val = hilink_check_pcie_ssc(macro);
    }
    HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->macro_info[macro].hilink_subctrl_base,
        HILINK_SUBCTRL_5_REG,
        HILINK_SUBCTRL_5_BIT0_OFFSET + HILINK_SUBCTRL_5_BIT0_LEN - 1,
        HILINK_SUBCTRL_5_BIT0_OFFSET,
        macro == MACRO_1 ? val : 0x0);

    if (hilinkPara->macro_cfg[macro].cs[CS0].type != SERDES_TYPE_ETH) {
        return RET_OK;
    }
    for (i = 0; i < DS_MAX; i++) {
        if (hilinkPara->macro_cfg[macro].ds[i].type == SERDES_TYPE_ETH) {
            ret = eth_rx_para_config(macro, i, hilinkPara->macro_cfg[macro].cs[CS0].frequency);
            if (ret != RET_OK) {
                HILINK_ERR("M%d ds%d eth_rx_para_config fail\n", macro, i);
                return RET_ERROR;
            }
        }
    }
    return RET_OK;
}

STATIC void HILINK_MacroCsr0Init(u32 macro)
{
    HilinkPara *hilinkPara = HILINK_GetParaCfg();
    DRV_SERDES_TYPE mode0 = hilinkPara->macro_cfg[macro].cs[CS0].type;
    DRV_SERDES_TYPE mode1 = hilinkPara->macro_cfg[macro].cs[CS1].type;
    HilinkRegBase base;
    Hilink_SetRegBase(&base, hilinkPara, macro, 0);
    HILINK_MacroApiRegWriteBits(base, FW_MARCOAPI_CSR0_REG, FW_MARCOAPI_CSR_0_BIT30_OFFSET,
        FW_MARCOAPI_CSR_0_BIT30_LEN, 0x1);
    HILINK_MacroApiRegWriteBits(base, FW_MARCOAPI_CSR0_REG, FW_MARCOAPI_CSR_0_BIT29_OFFSET,
        FW_MARCOAPI_CSR_0_BIT29_LEN, macro == MACRO_0 ? 0x0 : 0x1);
    if (mode0 == SERDES_TYPE_ETH) {
        HILINK_MacroApiRegWriteBits(base, FW_MARCOAPI_CSR0_REG,
            FW_MARCOAPI_CSR_0_BIT25_OFFSET, FW_MARCOAPI_CSR_0_BIT25_LEN,
            hilinkPara->macro_cfg[macro].cs[CS0].frequency == SERDES_FREQ_ETH_1_25G ? 0x0 : 0x1);
        HILINK_MacroApiRegWriteBits(base, FW_MARCOAPI_CSR0_REG, FW_MARCOAPI_CSR_0_BIT21_OFFSET,
            FW_MARCOAPI_CSR_0_BIT21_LEN,
            0x1);
    }
    if (mode1 == SERDES_TYPE_USB) {
        HILINK_MacroApiRegWriteBits(base, FW_MARCOAPI_CSR0_REG,
            FW_MARCOAPI_CSR_0_BIT17_OFFSET, FW_MARCOAPI_CSR_0_BIT25_LEN,
            0x1);
    }
    HILINK_MacroApiRegWriteBits(base, FW_MARCOAPI_CSR0_REG,
        FW_MARCOAPI_CSR_0_BIT5_OFFSET, FW_MARCOAPI_CSR_0_BIT5_LEN, 0x0);
}

STATIC void MacroCalibrationApiConfig(u32 macro)
{
    HilinkPara *hilinkPara = HILINK_GetParaCfg();
    HilinkRegBase base;
    u32 val = 0;
    Hilink_SetRegBase(&base, hilinkPara, macro, 0);

    HILINK_MacroCsr0Init(macro);
    if ((hilinkPara->macro_cfg[macro].cs[CS0].type != SERDES_TYPE_NULL) &&
        (hilinkPara->macro_cfg[macro].cs[CS1].type != SERDES_TYPE_NULL)) {
            val = 0x80000000;
    }
    HILINK_MacroApiRegWriteBits(base, FW_MARCOAPI_CSR1_REG, BIT_0, BIT_LEN, val);
}

STATIC void CsCalibrationApiConfig(u32 macro)
{
    HilinkPara *hilinkPara = HILINK_GetParaCfg();
    DRV_SERDES_TYPE mode0 = hilinkPara->macro_cfg[macro].cs[CS0].type;
    DRV_SERDES_TYPE mode1 = hilinkPara->macro_cfg[macro].cs[CS1].type;
    HilinkRegBase base;
    Hilink_SetRegBase(&base, hilinkPara, macro, 0);
    HILINK_CsApiRegWriteBits(base, FW_CSAPI_CSR0_REG, BIT_0, BIT_LEN, 0x1C000000);

    HILINK_CsApiRegWriteBits(base, FW_CSAPI_CSR1_REG, BIT_0, BIT_LEN, 0x0);
    HILINK_CsApiRegWriteBits(base, FW_CSAPI_CSR1_REG,
        FW_CSAPI_CSR_1_BIT30_OFFSET, FW_CSAPI_CSR_1_BIT30_LEN,
        mode0 == SERDES_TYPE_ETH ? 0x1 : 0x0);
    HILINK_CsApiRegWriteBits(base, FW_CSAPI_CSR1_REG,
        FW_CSAPI_CSR_1_BIT28_OFFSET, FW_CSAPI_CSR_1_BIT28_LEN, 0x1);
    HILINK_CsApiRegWriteBits(base, FW_CSAPI_CSR1_REG,
        FW_CSAPI_CSR_1_BIT23_OFFSET, FW_CSAPI_CSR_1_BIT23_LEN,
        hilinkPara->macro_cfg[macro].cs[CS0].ssc_enable == 1 ? 0xD : 0x0);
    HILINK_CsApiRegWriteBits(base, FW_CSAPI_CSR1_REG,
        FW_CSAPI_CSR_1_BIT19_OFFSET, FW_CSAPI_CSR_1_BIT19_LEN, 0x7);
    HILINK_CsApiRegWriteBits(base, FW_CSAPI_CSR1_REG,
        FW_CSAPI_CSR_1_BIT18_OFFSET, FW_CSAPI_CSR_1_BIT18_LEN, 0x0);

    HILINK_CsApiRegWriteBits(base, FW_CSAPI_CSR2_REG, BIT_0, BIT_LEN, 0x0);

    HILINK_CsApiRegWriteBits(base, FW_CSAPI_CSR3_REG, BIT_0, BIT_LEN, 0x0);
    HILINK_CsApiRegWriteBits(base, FW_CSAPI_CSR3_REG, FW_CSAPI_CSR_3_BIT30_OFFSET,
        FW_CSAPI_CSR_3_BIT30_LEN, mode1 == SERDES_TYPE_USB ? 0x1 : 0x0);
    HILINK_CsApiRegWriteBits(base, FW_CSAPI_CSR3_REG, FW_CSAPI_CSR_3_BIT28_OFFSET,
        FW_CSAPI_CSR_3_BIT28_LEN, 0x3);
    HILINK_CsApiRegWriteBits(base, FW_CSAPI_CSR3_REG, FW_CSAPI_CSR_3_BIT23_OFFSET,
        FW_CSAPI_CSR_3_BIT23_LEN,
        hilinkPara->macro_cfg[macro].cs[CS1].ssc_enable == 1 ? 0xD : 0x0);
    HILINK_CsApiRegWriteBits(base, FW_CSAPI_CSR3_REG, FW_CSAPI_CSR_3_BIT19_OFFSET,
        FW_CSAPI_CSR_3_BIT19_LEN, 0x7);

    HILINK_CsApiRegWriteBits(base, FW_CSAPI_CSR4_REG, BIT_0, BIT_LEN, 0x0);
}

STATIC void DsCalibrationApiConfig(u32 macro, u32 sliceMask)
{
    HilinkPara *hilinkPara = HILINK_GetParaCfg();
    HilinkRegBase base;
    DRV_SERDES_TYPE type;
    u32 i;

    for (i = 0; i < DS_MAX; i++) {
        if (((sliceMask >> i) & 0x1) == 0x1) {
            Hilink_SetRegBase(&base, hilinkPara, macro, i);
            type = hilinkPara->macro_cfg[macro].ds[i].type;

            HILINK_DsApiRegWriteBits(base, FW_DSAPI_CSR1_REG, BIT_0, BIT_LEN, 0x0);
            HILINK_DsApiRegWriteBits(base, FW_DSAPI_CSR1_REG, FW_DSAPI_CSR_1_BIT27_OFFSET,
                FW_DSAPI_CSR_1_BIT27_LEN,
                hilinkPara->macro_cfg[macro].ds[i].cs_select == 1 ? 0x1 : 0x0);
            HILINK_DsApiRegWriteBits(base, FW_DSAPI_CSR2_REG, BIT_0, BIT_LEN,
                ((type == SERDES_TYPE_SATA) || (type == SERDES_TYPE_USB)) ? 0x2004 : 0x200E);

            HILINK_DsApiRegWriteBits(base, FW_DSAPI_CSR3_REG, BIT_0, BIT_LEN,
                type == SERDES_TYPE_PCIE ? 0x8 : 0x0);
            HILINK_DsApiRegWriteBits(base, FW_DSAPI_CSR4_REG, BIT_0, BIT_LEN,
                type == SERDES_TYPE_PCIE ? 0x3D0021C0 : 0x0);
            HILINK_DsApiRegWriteBits(base, FW_DSAPI_CSR5_REG, BIT_0, BIT_LEN,
                type == SERDES_TYPE_PCIE ? 0x504EFBE2 : 0x0);
            HILINK_DsApiRegWriteBits(base, FW_DSAPI_CSR6_REG, BIT_0, BIT_LEN,
                type == SERDES_TYPE_PCIE ? 0x400A580 : 0x0);
            HILINK_DsApiRegWriteBits(base, FW_DSAPI_CSR7_REG, BIT_0, BIT_LEN,
                type == SERDES_TYPE_PCIE ? 0x6549F87C : 0x0);
        }
    }
}

STATIC void CalibrationApiConfig(u32 macro, u32 sliceMask)
{
    MacroCalibrationApiConfig(macro);
    CsCalibrationApiConfig(macro);
    DsCalibrationApiConfig(macro, sliceMask);
}

STATIC bool StartTopFsm(u32 macro, u32 fsmStartAddr, u32 checkCoutMax)
{
    u32 checkCount;
    u32 topFsmDone;
    HilinkRegBase base;
    Hilink_SetRegBase(&base, HILINK_GetParaCfg(), macro, 0);
    HILINK_CPRegWriteBits(base, CP_CSR_CP_CSR3_REG,
        CP_CSR_3_BIT0_OFFSET, CP_CSR_3_BIT0_LEN, fsmStartAddr);
    HILINK_CPRegWriteBits(base, CP_CSR_CP_CSR3_REG,
        CP_CSR_3_BIT14_OFFSET, CP_CSR_3_BIT14_LEN, 0x1);
    checkCount = 0;
    do {
        HAL_MsSleep(1);
        checkCount++;
        topFsmDone = HILINK_CPRegReadBits(base, CP_CSR_CP_CSR7_REG,
            CP_CSR_7_BIT16_OFFSET, CP_CSR_7_BIT16_LEN);
    } while ((topFsmDone == 0x0) && (checkCount <= checkCoutMax));
    if (checkCount > checkCoutMax) {
        HILINK_ERR_SNAPSHOT(SERDES_START_TOP_FSW_TIMEOUT_FAIL,
            "Start top fsm timeout. topFsmDone %d\n", topFsmDone);
        return false;
    }
    return true;
}

STATIC bool RunCalibration(u32 macro, u32 dsMask)
{
    CalibrationApiConfig(macro, dsMask);
    if (StartTopFsm(macro, 0x15, RUN_CALIBRATION_TIMEOUT_CNT) == false) {
        HILINK_ERR("Start fsm task timeout while RunCalibration.\n");
        return false;
    }
    return true;
}

STATIC void DlosStartUp(u32 macro, u32 rxIndex, u32 RxLosThsd, u32 RxLosWin)
{
    HilinkRegBase base;
    Hilink_SetRegBase(&base, HILINK_GetParaCfg(), macro, rxIndex);

    HILINK_RxRegWriteBits(base, RX_CSR_RX_CSR24_REG, RX_CSR_24_BIT11_OFFSET, RX_CSR_24_BIT11_LEN, 0x0);
    HILINK_RxRegWriteBits(base, RX_CSR_RX_CSR21_REG, RX_CSR_21_BIT9_OFFSET, RX_CSR_21_BIT9_LEN, 0x0);
    HILINK_RxRegWriteBits(base, RX_CSR_RX_CSR24_REG,
        RX_CSR_24_BIT0_OFFSET, RX_CSR_24_BIT0_LEN, RxLosThsd);
    HILINK_RxRegWriteBits(base, RX_CSR_RX_CSR24_REG,
        RX_CSR_24_BIT3_OFFSET, RX_CSR_24_BIT3_LEN, RxLosWin);
    HILINK_RxRegWriteBits(base, RX_CSR_RX_CSR21_REG, RX_CSR_21_BIT9_OFFSET, RX_CSR_21_BIT9_LEN, 0x1);
    HILINK_RxRegWriteBits(base, RX_CSR_RX_CSR24_REG, RX_CSR_24_BIT11_OFFSET, RX_CSR_24_BIT11_LEN, 0x1);
    HAL_UsSleep(5);
}

STATIC void PplosStartUp(u32 macro, u32 rxIndex, u32 rxPplosDetThr, u32 rxPplosDetWin)
{
    HilinkRegBase base;
    Hilink_SetRegBase(&base, HILINK_GetParaCfg(), macro, rxIndex);

    HILINK_RxRegWriteBits(base, RX_CSR_RX_CSR39_REG, RX_CSR_39_BIT0_OFFSET, RX_CSR_39_BIT0_LEN, 0x0);
    HILINK_RxRegWriteBits(base, RX_CSR_RX_CSR39_REG,
        RX_CSR_39_BIT6_OFFSET, RX_CSR_39_BIT6_LEN, 0x0);
    HILINK_RxRegWriteBits(base, RX_CSR_RX_CSR39_REG,
        RX_CSR_39_BIT3_OFFSET, RX_CSR_39_BIT3_LEN, rxPplosDetThr);
    HILINK_RxRegWriteBits(base, RX_CSR_RX_CSR39_REG,
        RX_CSR_39_BIT1_OFFSET, RX_CSR_39_BIT1_LEN, rxPplosDetWin);
    HILINK_RxRegWriteBits(base, RX_CSR_RX_CSR39_REG, RX_CSR_39_BIT0_OFFSET, RX_CSR_39_BIT0_LEN, 0x1);

    HAL_UsSleep(1);
}

STATIC void MacroConfigAfterCalib(u32 macro, u32 sliceMask)
{
    u32 dsIndex;
    HilinkRegBase base;

    Hilink_SetRegBase(&base, HILINK_GetParaCfg(), macro, 0);
    HILINK_CPRegWriteBits(base, CP_CSR_CP_CSR17_REG,
        CP_CSR_17_BIT9_OFFSET, CP_CSR_17_BIT9_LEN, 0x0);

    for (dsIndex = 0; dsIndex < DS_MAX; dsIndex++) {
        if (((sliceMask >> dsIndex) & 0x1) == 0x1) {
            Hilink_SetRegBase(&base, HILINK_GetParaCfg(), macro, dsIndex);
            HILINK_TxRegWriteBits(base, TX_CSR_TX_CSR13_REG, TX_CSR_13_BIT4_OFFSET, TX_CSR_13_BIT4_LEN, 0x0);
            HILINK_RxRegWriteBits(base, RX_CSR_RX_CSR13_REG, RX_CSR_13_BIT4_OFFSET, RX_CSR_13_BIT4_LEN, 0x0);
            HILINK_RxRegWriteBits(base, RX_CSR_RX_CSR75_REG, BIT_0, 1, 0x0); // only set bit 0

            DlosStartUp(macro, dsIndex, 0x4, 0x7);
            PplosStartUp(macro, dsIndex, 0x2, 0x1);
        }
    }
}

STATIC void CsContAdaptApiConfig(u32 macro)
{
    HilinkRegBase base;
    Hilink_SetRegBase(&base, HILINK_GetParaCfg(), macro, 0);
    HILINK_CsApiRegWriteBits(base, FW_CSAPI_CSR0_REG,
        FW_CSAPI_CSR_0_BIT26_OFFSET, FW_CSAPI_CSR_0_BIT26_LEN, 0x1);
}

STATIC void DsInitAdaptApiConfig(u32 macro, u32 sliceMask)
{
    u32 ds;
    u32 i;
    HilinkRegBase base;
    HilinkPara *hilinkPara = HILINK_GetParaCfg();
    DRV_SERDES_TYPE type;
    const u32 ds_cfg[SERDES_TYPE_NULL][HILINK_DS_ADAPT_API_REG_NUM] = {
        {0x8, 0x3D0021C0, 0x504EFBE2, 0x400A580, 0x64BDB96C, 0x400, 0x0, 0x0},    // PCIE
        {0x0, 0x78000180, 0xD04EFBE3, 0x400A580, 0x64B9B96C, 0x80000400, 0x0, 0x0},   // SATA
        {0x0, 0x780021C0, 0xD04EFBE2, 0x400A580, 0x64B9B96C, 0x400, 0x0, 0x0},    // USB
        {0x820, 0x780021C0, 0xD04EFBE2, 0x400A580, 0x64B9B96C, 0x80000400, 0x0, 0x0}    // ETH
    };
    const u32 ds_reg[HILINK_DS_ADAPT_API_REG_NUM] = {FW_DSAPI_CSR3_REG, FW_DSAPI_CSR4_REG, FW_DSAPI_CSR5_REG,
        FW_DSAPI_CSR6_REG, FW_DSAPI_CSR7_REG, FW_DSAPI_CSR8_REG, FW_DSAPI_CSR9_REG, FW_DSAPI_CSR0_REG};

    for (ds = 0; ds < DS_MAX; ds++) {
        if (((sliceMask >> ds) & 0x1) == 0x1) {
            Hilink_SetRegBase(&base, hilinkPara, macro, ds);
            type = hilinkPara->macro_cfg[macro].ds[ds].type;
            if (type >= SERDES_TYPE_NULL) {
                continue;
            }
            for (i = 0; i < HILINK_DS_ADAPT_API_REG_NUM; ++i) {
                HILINK_DsApiRegWriteBits(base, ds_reg[i], BIT_0, BIT_LEN, ds_cfg[type][i]);
            }
        }
    }
}

STATIC void DsContAdaptApiConfig(u32 macro, u32 sliceMask)
{
    u32 i;
    u32 ds;
    HilinkRegBase base;
    HilinkPara *hilinkPara = HILINK_GetParaCfg();
    DRV_SERDES_TYPE type;
    const u32 ds_cfg[SERDES_TYPE_NULL + 1][HILINK_DS_ADAPT_API_REG_NUM] = {
        {0x8, 0x3D0021C0, 0x504EFBE2, 0x400A580, 0x64BDB96C, 0x400, 0x0, 0x2000000},    // PCIE
        {0x0, 0x78000180, 0xD04EFBE3, 0x400A580, 0x64B9B96C, 0x80000400, 0x0, 0x2000000},   // SATA
        {0x0, 0x780021C0, 0xD04EFBE2, 0x400A580, 0x64B9B96C, 0x400, 0x0, 0x2000000},    // USB
        {0x820, 0x780021C0, 0xD04EFBE2, 0x400A580, 0x64B9B96C, 0x80000400, 0x0, 0x0}    // ETH
    };
    const u32 ds_reg[HILINK_DS_ADAPT_API_REG_NUM] = {FW_DSAPI_CSR3_REG, FW_DSAPI_CSR4_REG, FW_DSAPI_CSR5_REG,
        FW_DSAPI_CSR6_REG, FW_DSAPI_CSR7_REG, FW_DSAPI_CSR8_REG, FW_DSAPI_CSR9_REG, FW_DSAPI_CSR0_REG};

    for (ds = 0; ds < DS_MAX; ds++) {
        if (((sliceMask >> ds) & 0x1) == 0x1) {
            Hilink_SetRegBase(&base, hilinkPara, macro, ds);
            type = hilinkPara->macro_cfg[macro].ds[ds].type;
            if (type >= SERDES_TYPE_NULL) {
                continue;
            }
            for (i = 0; i < HILINK_DS_ADAPT_API_REG_NUM; ++i) {
                HILINK_DsApiRegWriteBits(base, ds_reg[i], BIT_0, BIT_LEN, ds_cfg[type][i]);
            }
        }
    }
}

STATIC bool RunAdaption(u32 macro, u32 sliceMask, u32 initAdaptFullBypassMask)
{
    if (StopSliceFwTask(macro, sliceMask) != RET_OK) {
        HILINK_ERR("Stop slice fw task failed while RunAdaption.\n");
        return false;
    }

    CsContAdaptApiConfig(macro);
    DsInitAdaptApiConfig(macro, sliceMask & (~initAdaptFullBypassMask));
    HAL_MsSleep(1);
    DsContAdaptApiConfig(macro, sliceMask);
    return true;
}

STATIC bool HILINK_MacroInit(u32 macro, u32 sliceMask,  bool runAdaptFlag, u32 initAdaptFullBypassMask)
{
    if (RunCalibration(macro, sliceMask) == false) {
        HILINK_ERR("Calibration failed!");
        return false;
    }
    if (runAdaptFlag == true) {
        MacroConfigAfterCalib(macro, sliceMask);
        HILINK_McuRelease(macro, 0x8C00);
        if (RunAdaption(macro, sliceMask, initAdaptFullBypassMask) == false) {
            HILINK_ERR("MacroInit Error: RunAdaption failed!");
            return false;
        }
    }
    return true;
}

STATIC int HILINK_SerdesPowerOnThread(void* args)
{
    SerdesPowerOnThreadPara_t *para = (SerdesPowerOnThreadPara_t*)(uintptr_t)args;
    u32 macro = para->macro;
    u32 val = para->val;
    u32 readData = 0;
    int ao_subctrl_reg;
    int ret;

    CHECK_RETURN(macro, MACRO_MAX, -1);
    HILINK_INFO("Write AO reg start.\n");
    HILINK_INFO("macro = %d.\n", macro);
    HILINK_INFO("reg val = %x.\n", val);

    if (macro == MACRO_0) {
        ao_subctrl_reg = AO_SC_SDS0_POWER_CTRL;
    } else {
        ao_subctrl_reg = AO_SC_SDS1_POWER_CTRL;
    }

    ret = sec_io_read32(0, (SEC_IO_REGS_ID_TYPE)ao_subctrl_reg, &readData);
    if (ret != 0) {
        HILINK_ERR("Read AO reg failed. (ret=%d)\n", ret);
        g_retWriteAOReg = AO_REG_WRITE_FAILED;
        return RET_ERROR;
    }
    ret = sec_io_write32(0, (SEC_IO_REGS_ID_TYPE)ao_subctrl_reg, readData | val);
    if (ret != 0) {
        HILINK_ERR("Write AO reg failed. (ret=%d)\n", ret);
        g_retWriteAOReg = AO_REG_WRITE_FAILED;
        return RET_ERROR;
    }
    readData = 0x0;
    // write check
    ret = sec_io_read32(0, (SEC_IO_REGS_ID_TYPE)ao_subctrl_reg, &readData);
    if ((ret != 0) || ((readData & val) != val)) {
        HILINK_ERR("READ AO reg failed. (ret=%d)\n", ret);
        HILINK_ERR("Write AO reg failed. reg: 0x%x(0x%x)\n", readData, readData & val);
        g_retWriteAOReg = AO_REG_WRITE_FAILED;
        return RET_ERROR;
    }
    g_retWriteAOReg = AO_REG_WRITE_SUCCESS;
    HILINK_INFO("Write AO reg success.\n");
    return RET_OK;
}

STATIC u32 HILINK_SerdesPowerOn(u32 macro, bool is_awake)
{
    HilinkPara *hilinkPara = HILINK_GetParaCfg();
    struct task_struct *powerTaskInfo = NULL;
    int ao_subctrl_reg;
    u32 timeOutCnt = 0;
    SerdesPowerOnThreadPara_t para;

    if (is_awake == false) {
        if (macro == MACRO_0) {
            ao_subctrl_reg = AO_SC_SDS0_POWER_CTRL;
        } else {
            ao_subctrl_reg = AO_SC_SDS1_POWER_CTRL;
        }
        para.macro = macro;

        g_retWriteAOReg = AO_REG_WRITING;
        para.val = 0x1E0;
        powerTaskInfo =  kthread_run(HILINK_SerdesPowerOnThread, &para, "serdes_power_on_thread");
        if (powerTaskInfo == NULL) {
            HILINK_ERR_SNAPSHOT(SERDES_POWER_ON_CREATE_THREAD0_FAIL,
                "Create thread for AO reg first part failed. (macro=%u)\n", macro);
            return RET_ERROR;
        }
        do {
            timeOutCnt++;
            HAL_MsSleep(1);
        } while ((g_retWriteAOReg == AO_REG_WRITING) && (timeOutCnt < AO_REG_WRITE_TIMEOUT_CNT));
        if (timeOutCnt >= AO_REG_WRITE_TIMEOUT_CNT) {
            HILINK_ERR_SNAPSHOT(SERDES_POWER_ON_REG0_WRITE_TIMEOUT_FAIL, "Wait write AO reg first part timeout.\n");
            return RET_ERROR;
        } else if (g_retWriteAOReg != AO_REG_WRITE_SUCCESS) {
            HILINK_ERR_SNAPSHOT(SERDES_POWER_ON_REG0_WRITE_FAIL, "Wait write AO reg first part failed.\n");
            return RET_ERROR;
        }
    }

    if (macro == MACRO_0) {
        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->io_subctrl_base,
            IO_SUBCTRL_SC_SDS_RESET_DREQ_REG,
            IO_SUBCTRL_SRST_DREQ_ADAP_OFFSET + IO_SUBCTRL_SRST_DREQ_ADAP_LEN - 1,
            IO_SUBCTRL_SRST_DREQ_ADAP_OFFSET,
            0xF);
        HILINK_RegWriteBits((void*)(uintptr_t)hilinkPara->io_subctrl_base,
            IO_SUBCTRL_SC_SDS_RESET_DREQ_REG,
            IO_SUBCTRL_SRST_DREQ_SDS_SDPI_OFFSET + IO_SUBCTRL_SRST_DREQ_SDS_SDPI_LEN - 1,
            IO_SUBCTRL_SRST_DREQ_SDS_SDPI_OFFSET,
            0x3);
    }

    if (is_awake == false) {
        g_retWriteAOReg = AO_REG_WRITING;
        para.val = 0x1F;
        timeOutCnt = 0;
        powerTaskInfo = NULL;
        powerTaskInfo =  kthread_run(HILINK_SerdesPowerOnThread, &para, "serdes_power_on_thread");
        if (powerTaskInfo == NULL) {
            HILINK_ERR_SNAPSHOT(SERDES_POWER_ON_CREATE_THREAD1_FAIL,
                "Create thread for AO reg second part failed. (macro=%u)\n", macro);
            return RET_ERROR;
        }
        do {
            timeOutCnt++;
            HAL_MsSleep(1);
        } while ((g_retWriteAOReg == AO_REG_WRITING) && (timeOutCnt < AO_REG_WRITE_TIMEOUT_CNT));
        if (timeOutCnt >= AO_REG_WRITE_TIMEOUT_CNT) {
            HILINK_ERR_SNAPSHOT(SERDES_POWER_ON_REG1_WRITE_TIMEOUT_FAIL, "Wait write AO reg second part timeout.\n");
            return RET_ERROR;
        } else if (g_retWriteAOReg != AO_REG_WRITE_SUCCESS) {
            HILINK_ERR_SNAPSHOT(SERDES_POWER_ON_REG1_WRITE_FAIL, "Wait write AO reg second part failed.\n");
            return RET_ERROR;
        }
    }
    return RET_OK;
}

STATIC void HILINK_BiasInit(u32 macro)
{
    HILINK_RxLowImpedance(macro);
    return;
}

#ifdef CFG_SOC_PLATFORM_MDC_V11
STATIC u32 HILINK_SetSerdesFFE(u32 macro, bool is_awake)
{
    u32 ret;
    u32 i;

    if ((is_awake == false) || (g_serdes_cfg[macro].mode == PCIE_MODE)) {
        HILINK_INFO("no need to set serdes FFE.\n");
        return RET_OK;
    }

    if (macro >= MACRO_MAX) {
        HILINK_ERR("Invalid macro number.\n");
        return RET_ERROR;
    }

    for (i = 0U; i < LANE_MAX; ++i) {
        /* switch control mode to register mode */
        SwitchControlMode(macro, i, CTRL_MODE_REG);
        /* switch power mode to low power */
        ret = DsPowerModeSwitch(macro, i, 0);
        if (ret != RET_OK) {
            HILINK_ERR("Switch power mode failed. (macro=%u, lane=%u)\n", macro, i);
            SwitchControlMode(macro, i, CTRL_MODE_PIN);
            return RET_ERROR;
        }
        /* set Tx FFE parameter */
        ret = SetTxFFE(macro, i, g_serdes_cfg[macro].tx_margin[i], g_serdes_cfg[macro].tx_deemph[i]);
        /* switch control mode to pin mode */
        SwitchControlMode(macro, i, CTRL_MODE_PIN);
        if (ret != 0) {
            HILINK_ERR("Set serdes FFE failed. (macro=%u, lane=%u)\n", macro, i);
            return ret;
        }
    }

    return ret;
}
#endif

STATIC void hilink_soc_set_apb_mode(macro_info_t *macro_info, unsigned int mode)
{
    void* hilink_subctrl_base = (void*)(uintptr_t)macro_info->hilink_subctrl_base;

    HILINK_RegWriteBits(hilink_subctrl_base,
        HILINK_SUBCTRL_6_REG,
        HILINK_SUBCTRL_6_BIT0_OFFSET + HILINK_SUBCTRL_6_BIT0_LEN - 1,
        HILINK_SUBCTRL_6_BIT0_OFFSET,
        mode);
}

STATIC void hilink_soc_srst_dreq(void* io_subctrl_base)
{
    // hilink0 deactivate soft reset.
    HILINK_RegWriteBits(io_subctrl_base,
        IO_SUBCTRL_SC_HILINK0_RESET_DREQ_REG,
        IO_SUBCTRL_SRST_DREQ_HILINK0_PMA_RX_OFFSET + IO_SUBCTRL_SRST_DREQ_HILINK0_PMA_RX_LEN - 1,
        IO_SUBCTRL_SRST_DREQ_HILINK0_PMA_RX_OFFSET,
        0xF);
    HILINK_RegWriteBits(io_subctrl_base,
        IO_SUBCTRL_SC_HILINK0_RESET_DREQ_REG,
        IO_SUBCTRL_SRST_DREQ_HILINK0_PMA_TX_OFFSET + IO_SUBCTRL_SRST_DREQ_HILINK0_PMA_TX_LEN - 1,
        IO_SUBCTRL_SRST_DREQ_HILINK0_PMA_TX_OFFSET,
        0xF);
    HILINK_RegWriteBits(io_subctrl_base,
        IO_SUBCTRL_SC_HILINK1_RESET_DREQ_REG,
        IO_SUBCTRL_SRST_DREQ_HILINK1_PMA_RX_OFFSET + IO_SUBCTRL_SRST_DREQ_HILINK1_PMA_RX_LEN - 1,
        IO_SUBCTRL_SRST_DREQ_HILINK1_PMA_RX_OFFSET,
        0xF);
    HILINK_RegWriteBits(io_subctrl_base,
        IO_SUBCTRL_SC_HILINK1_RESET_DREQ_REG,
        IO_SUBCTRL_SRST_DREQ_HILINK1_PMA_TX_OFFSET + IO_SUBCTRL_SRST_DREQ_HILINK1_PMA_TX_LEN - 1,
        IO_SUBCTRL_SRST_DREQ_HILINK1_PMA_TX_OFFSET,
        0xF);
}

STATIC void hilink_soc_hilink_clk_en(void* io_subctrl_base)
{
    HILINK_RegWriteBits(io_subctrl_base,
        IO_SUBCTRL_SC_HILINK0_ICG_EN_REG,
        IO_SUBCTRL_ICG_EN_HILINK0_PMA_RX_OFFSET + IO_SUBCTRL_ICG_EN_HILINK0_PMA_RX_LEN - 1,
        IO_SUBCTRL_ICG_EN_HILINK0_PMA_RX_OFFSET,
        0xF);
    HILINK_RegWriteBits(io_subctrl_base,
        IO_SUBCTRL_SC_HILINK0_ICG_EN_REG,
        IO_SUBCTRL_ICG_EN_HILINK0_PMA_TX_OFFSET + IO_SUBCTRL_ICG_EN_HILINK0_PMA_TX_LEN - 1,
        IO_SUBCTRL_ICG_EN_HILINK0_PMA_TX_OFFSET,
        0xF);
    HILINK_RegWriteBits(io_subctrl_base,
        IO_SUBCTRL_SC_HILINK0_ICG_EN_REG,
        IO_SUBCTRL_ICG_EN_HILINK0_RXOCLK_OFFSET + IO_SUBCTRL_ICG_EN_HILINK0_RXOCLK_LEN - 1,
        IO_SUBCTRL_ICG_EN_HILINK0_RXOCLK_OFFSET,
        0xF);
    HILINK_RegWriteBits(io_subctrl_base,
        IO_SUBCTRL_SC_HILINK0_ICG_EN_REG,
        IO_SUBCTRL_ICG_EN_HILINK0_MCLK_OFFSET + IO_SUBCTRL_ICG_EN_HILINK0_MCLK_LEN - 1,
        IO_SUBCTRL_ICG_EN_HILINK0_MCLK_OFFSET,
        0x0);

    HILINK_RegWriteBits(io_subctrl_base,
        IO_SUBCTRL_SC_HILINK1_ICG_EN_REG,
        IO_SUBCTRL_ICG_EN_HILINK1_PMA_RX_OFFSET + IO_SUBCTRL_ICG_EN_HILINK1_PMA_RX_LEN - 1,
        IO_SUBCTRL_ICG_EN_HILINK1_PMA_RX_OFFSET,
        0xF);
    HILINK_RegWriteBits(io_subctrl_base,
        IO_SUBCTRL_SC_HILINK1_ICG_EN_REG,
        IO_SUBCTRL_ICG_EN_HILINK1_PMA_TX_OFFSET + IO_SUBCTRL_ICG_EN_HILINK1_PMA_TX_LEN - 1,
        IO_SUBCTRL_ICG_EN_HILINK1_PMA_TX_OFFSET,
        0xF);
    HILINK_RegWriteBits(io_subctrl_base,
        IO_SUBCTRL_SC_HILINK1_ICG_EN_REG,
        IO_SUBCTRL_ICG_EN_HILINK1_RXOCLK_OFFSET + IO_SUBCTRL_ICG_EN_HILINK1_RXOCLK_LEN - 1,
        IO_SUBCTRL_ICG_EN_HILINK1_RXOCLK_OFFSET,
        0xF);
    HILINK_RegWriteBits(io_subctrl_base,
        IO_SUBCTRL_SC_HILINK1_ICG_EN_REG,
        IO_SUBCTRL_ICG_EN_HILINK1_MCLK_OFFSET + IO_SUBCTRL_ICG_EN_HILINK1_MCLK_LEN - 1,
        IO_SUBCTRL_ICG_EN_HILINK1_MCLK_OFFSET,
        0x0);
}

STATIC void hilink_soc_sram_clk_en(void* io_subctrl_base)
{
    HILINK_RegWriteBits(io_subctrl_base,
        IO_SUBCTRL_SC_HILINK_SRAM_ICG_EN_REG,
        IO_SUBCTRL_ICG_EN_HILINK1_SRAM1_OFFSET + IO_SUBCTRL_ICG_EN_HILINK1_SRAM1_LEN - 1,
        IO_SUBCTRL_ICG_EN_HILINK1_SRAM1_OFFSET,
        0x1);
    HILINK_RegWriteBits(io_subctrl_base,
        IO_SUBCTRL_SC_HILINK_SRAM_ICG_EN_REG,
        IO_SUBCTRL_ICG_EN_HILINK1_SRAM0_OFFSET + IO_SUBCTRL_ICG_EN_HILINK1_SRAM0_LEN - 1,
        IO_SUBCTRL_ICG_EN_HILINK1_SRAM0_OFFSET,
        0x1);
    HILINK_RegWriteBits(io_subctrl_base,
        IO_SUBCTRL_SC_HILINK_SRAM_ICG_EN_REG,
        IO_SUBCTRL_ICG_EN_HILINK0_SRAM1_OFFSET + IO_SUBCTRL_ICG_EN_HILINK0_SRAM1_LEN - 1,
        IO_SUBCTRL_ICG_EN_HILINK0_SRAM1_OFFSET,
        0x1);
    HILINK_RegWriteBits(io_subctrl_base,
        IO_SUBCTRL_SC_HILINK_SRAM_ICG_EN_REG,
        IO_SUBCTRL_ICG_EN_HILINK0_SRAM0_OFFSET + IO_SUBCTRL_ICG_EN_HILINK0_SRAM0_LEN - 1,
        IO_SUBCTRL_ICG_EN_HILINK0_SRAM0_OFFSET,
        0x1);
}

STATIC void hilink_soc_mux_ctrl_init(void)
{
    macro_info_t *macro0_info;
    macro_info_t *macro1_info;
    void* hilink0_subctrl_base;
    void* hilink1_subctrl_base;
    u32 val;

    HilinkPara *hilinkPara = HILINK_GetParaCfg();

    macro0_info = &hilinkPara->macro_info[0];
    macro1_info = &hilinkPara->macro_info[1];
    hilink0_subctrl_base = (void*)(uintptr_t)macro0_info->hilink_subctrl_base;
    hilink1_subctrl_base = (void*)(uintptr_t)macro1_info->hilink_subctrl_base;

    HILINK_RegWriteBits(hilink0_subctrl_base,
        HILINK_SUBCTRL_11_REG,
        HILINK_SUBCTRL_11_BIT2_OFFSET + HILINK_SUBCTRL_11_BIT2_LEN - 1,
        HILINK_SUBCTRL_11_BIT2_OFFSET,
        0x0);
    if (hilinkPara->macro_cfg[MACRO_0].cs[CS0].type == SERDES_TYPE_SATA) {
        // using sata, set 0x2, using both sata and pcie, it coule be any value
        val = 0x2;
    } else if (hilinkPara->macro_cfg[MACRO_0].ds[DS2].type == SERDES_TYPE_PCIE) {
        // just using pcie 0
        val = 0x0;
    } else {
        // using both pcie 0 & 1 or just pcie 1, set 1
        val = 0x1;
    }
    HILINK_RegWriteBits(hilink0_subctrl_base,
        HILINK_SUBCTRL_11_REG,
        HILINK_SUBCTRL_11_BIT0_OFFSET + HILINK_SUBCTRL_11_BIT0_LEN - 1,
        HILINK_SUBCTRL_11_BIT0_OFFSET,
        val);
    HILINK_RegWriteBits(hilink1_subctrl_base,
        HILINK_SUBCTRL_11_REG,
        HILINK_SUBCTRL_11_BIT2_OFFSET + HILINK_SUBCTRL_11_BIT2_LEN - 1,
        HILINK_SUBCTRL_11_BIT2_OFFSET,
        hilinkPara->macro_cfg[MACRO_1].lane_order == 1 ? 1 : 0);
    if (hilinkPara->macro_cfg[MACRO_1].cs[CS0].type == SERDES_TYPE_ETH &&
        hilinkPara->macro_cfg[MACRO_1].cs[CS1].type == SERDES_TYPE_NULL) {
        val = 0x2;
    } else if (hilinkPara->macro_cfg[MACRO_1].cs[CS0].type == SERDES_TYPE_PCIE &&
        hilinkPara->macro_cfg[MACRO_1].cs[CS1].type == SERDES_TYPE_NULL) {
        val = 0x0;
    } else if (hilinkPara->macro_cfg[MACRO_1].cs[CS1].type == SERDES_TYPE_USB) {
        val = 0x1;
    } else {
        val = 0x1;
    }
    HILINK_RegWriteBits(hilink1_subctrl_base,
        HILINK_SUBCTRL_11_REG,
        HILINK_SUBCTRL_11_BIT0_OFFSET + HILINK_SUBCTRL_11_BIT0_LEN - 1,
        HILINK_SUBCTRL_11_BIT0_OFFSET,
        val);
}

STATIC u32 get_soc_lane_ctrl_val(u32 macro, u32 ds, u32 lane_order)
{
    HilinkPara *hilinkPara = HILINK_GetParaCfg();

    if (macro == MACRO_0) {
        // m0: 0=pcie0, 1=pcie1, 2=sata
        if (hilinkPara->macro_cfg[macro].ds[ds].type == SERDES_TYPE_SATA) {
            return 0x2;
        } else if (hilinkPara->macro_cfg[macro].ds[ds].type == SERDES_TYPE_PCIE &&
                   (ds >= 1 && hilinkPara->macro_cfg[macro].ds[ds - 1].type != SERDES_TYPE_PCIE)) {  // pcie1
            return 0x1;
        }
    } else {
        // m1: 1=usb, 2=ge, 3=ge(fun2_fun3_ctrl=1)
        if (hilinkPara->macro_cfg[macro].ds[ds].type == SERDES_TYPE_USB) {
            return 0x1;
        } else if (hilinkPara->macro_cfg[macro].ds[ds].type == SERDES_TYPE_ETH) {
            if (lane_order == 0x1) {
                return 0x3;
            } else {
                return 0x2;
            }
        }
    }
    return 0;
}

STATIC void hilink_soc_lane_ctrl_init(void)
{
    macro_info_t *macro0_info;
    macro_info_t *macro1_info;
    void* hilink0_subctrl_base;
    void* hilink1_subctrl_base;
    u32 lane_order;
    HilinkPara *hilinkPara = HILINK_GetParaCfg();

    macro0_info = &hilinkPara->macro_info[0];
    macro1_info = &hilinkPara->macro_info[1];
    hilink0_subctrl_base = (void*)(uintptr_t)macro0_info->hilink_subctrl_base;
    hilink1_subctrl_base = (void*)(uintptr_t)macro1_info->hilink_subctrl_base;
    lane_order = hilinkPara->macro_cfg[MACRO_0].lane_order;

    HILINK_RegWriteBits(hilink0_subctrl_base, HILINK_SUBCTRL_12_REG,
        HILINK_SUBCTRL_12_BIT6_OFFSET + HILINK_SUBCTRL_12_BIT6_LEN - 1,
        HILINK_SUBCTRL_12_BIT6_OFFSET,
        get_soc_lane_ctrl_val(MACRO_0, LANE_3, lane_order));
    HILINK_RegWriteBits(hilink0_subctrl_base, HILINK_SUBCTRL_12_REG,
        HILINK_SUBCTRL_12_BIT4_OFFSET + HILINK_SUBCTRL_12_BIT4_LEN - 1,
        HILINK_SUBCTRL_12_BIT4_OFFSET,
        get_soc_lane_ctrl_val(MACRO_0, LANE_2, lane_order));
    HILINK_RegWriteBits(hilink0_subctrl_base, HILINK_SUBCTRL_12_REG,
        HILINK_SUBCTRL_12_BIT2_OFFSET + HILINK_SUBCTRL_12_BIT2_LEN - 1,
        HILINK_SUBCTRL_12_BIT2_OFFSET,
        get_soc_lane_ctrl_val(MACRO_0, LANE_1, lane_order));
    HILINK_RegWriteBits(hilink0_subctrl_base, HILINK_SUBCTRL_12_REG,
        HILINK_SUBCTRL_12_BIT0_OFFSET + HILINK_SUBCTRL_12_BIT0_LEN - 1,
        HILINK_SUBCTRL_12_BIT0_OFFSET,
        get_soc_lane_ctrl_val(MACRO_0, LANE_0, lane_order));

    lane_order = hilinkPara->macro_cfg[MACRO_1].lane_order;
    HILINK_RegWriteBits(hilink1_subctrl_base, HILINK_SUBCTRL_12_REG,
        HILINK_SUBCTRL_12_BIT6_OFFSET + HILINK_SUBCTRL_12_BIT6_LEN - 1,
        HILINK_SUBCTRL_12_BIT6_OFFSET,
        get_soc_lane_ctrl_val(MACRO_1, LANE_3, lane_order));
    HILINK_RegWriteBits(hilink1_subctrl_base, HILINK_SUBCTRL_12_REG,
        HILINK_SUBCTRL_12_BIT4_OFFSET + HILINK_SUBCTRL_12_BIT4_LEN - 1,
        HILINK_SUBCTRL_12_BIT4_OFFSET,
        get_soc_lane_ctrl_val(MACRO_1, LANE_2, lane_order));
    HILINK_RegWriteBits(hilink1_subctrl_base, HILINK_SUBCTRL_12_REG,
        HILINK_SUBCTRL_12_BIT2_OFFSET + HILINK_SUBCTRL_12_BIT2_LEN - 1,
        HILINK_SUBCTRL_12_BIT2_OFFSET,
        get_soc_lane_ctrl_val(MACRO_1, LANE_1, lane_order));
    HILINK_RegWriteBits(hilink1_subctrl_base, HILINK_SUBCTRL_12_REG,
        HILINK_SUBCTRL_12_BIT0_OFFSET + HILINK_SUBCTRL_12_BIT0_LEN - 1,
        HILINK_SUBCTRL_12_BIT0_OFFSET,
        get_soc_lane_ctrl_val(MACRO_1, LANE_0, lane_order));
}

STATIC void hilink_soc_mclk_sel(void)
{
    u32 val = 0;
    u32 i;
    macro_info_t *macro0_info;
    macro_info_t *macro1_info;
    void* io_subctrl_base;
    void* hilink0_subctrl_base;
    void* hilink1_subctrl_base;
    HilinkPara *hilinkPara = HILINK_GetParaCfg();

    macro0_info = &hilinkPara->macro_info[0];
    macro1_info = &hilinkPara->macro_info[1];
    hilink0_subctrl_base = (void*)(uintptr_t)macro0_info->hilink_subctrl_base;
    hilink1_subctrl_base = (void*)(uintptr_t)macro1_info->hilink_subctrl_base;
    io_subctrl_base = (void*)(uintptr_t)hilinkPara->io_subctrl_base;

    HILINK_RegWriteBits(io_subctrl_base, IO_SUBCTRL_SC_HILINK0_ICG_DIS_REG,
        IO_SUBCTRL_ICG_DIS_HILINK0_MCLK_OFFSET + IO_SUBCTRL_ICG_DIS_HILINK0_MCLK_LEN - 1,
        IO_SUBCTRL_ICG_DIS_HILINK0_MCLK_OFFSET,
        0x3);
    HILINK_RegWriteBits(io_subctrl_base, IO_SUBCTRL_SC_HILINK1_ICG_DIS_REG,
        IO_SUBCTRL_ICG_DIS_HILINK1_MCLK_OFFSET + IO_SUBCTRL_ICG_DIS_HILINK1_MCLK_LEN - 1,
        IO_SUBCTRL_ICG_DIS_HILINK1_MCLK_OFFSET,
        0x3);

    for (i = 0; i < DS_MAX; i++) {
        val |=  hilinkPara->macro_cfg[MACRO_0].ds[i].cs_select << i;
    }
    HILINK_RegWriteBits(hilink0_subctrl_base, HILINK_SUBCTRL_13_REG,
        HILINK_SUBCTRL_13_BIT0_OFFSET + HILINK_SUBCTRL_13_BIT0_LEN - 1,
        HILINK_SUBCTRL_13_BIT0_OFFSET,
        val);
    val = 0;
    for (i = 0; i < DS_MAX; i++) {
        val |=  hilinkPara->macro_cfg[MACRO_1].ds[i].cs_select << i;
    }
    HILINK_RegWriteBits(hilink1_subctrl_base, HILINK_SUBCTRL_13_REG,
        HILINK_SUBCTRL_13_BIT0_OFFSET + HILINK_SUBCTRL_13_BIT0_LEN - 1,
        HILINK_SUBCTRL_13_BIT0_OFFSET,
        val);
}

STATIC void hilink_soc_crg_mode(u32 macro)
{
    u32 val;
    u32 i;
    macro_info_t *macro_info;
    void* hilink_subctrl_base;

    HilinkPara *hilinkPara = HILINK_GetParaCfg();
    macro_info = &hilinkPara->macro_info[macro];
    hilink_subctrl_base = (void*)(uintptr_t)macro_info->hilink_subctrl_base;

    val = ((hilinkPara->macro_cfg[macro].cs[CS0].type != SERDES_TYPE_NULL) &&
        (hilinkPara->macro_cfg[macro].cs[CS1].type != SERDES_TYPE_NULL)) ? 0 : 1;
    HILINK_RegWriteBits(hilink_subctrl_base, HILINK_SUBCTRL_15_REG,
        HILINK_SUBCTRL_15_BIT8_OFFSET + HILINK_SUBCTRL_15_BIT8_LEN - 1,
        HILINK_SUBCTRL_15_BIT8_OFFSET,
        val);
    val = 0;
    for (i = 0; i < DS_MAX; ++i) {
        val |= (u32)(hilinkPara->macro_cfg[macro].ds[i].type) << (HILINK_CRG_MODE_OFFSET * i);
    }
    HILINK_RegWriteBits(hilink_subctrl_base, HILINK_SUBCTRL_15_REG,
        HILINK_SUBCTRL_15_BIT0_OFFSET + HILINK_SUBCTRL_15_BIT0_LEN - 1,
        HILINK_SUBCTRL_15_BIT0_OFFSET,
        val);
}

STATIC void hilink_soc_los_mask(u32 macro, void* hilink_subctrl_base)
{
    u32 los_val = 0;
    u32 los_mask_val = 0;
    u32 i;
    u32 los[MACRO_MAX][SERDES_TYPE_NULL] = {
        {0, 0, 0, 0},
        {1, 0, 0, 0}
    };
    u32 los_mask[MACRO_MAX][SERDES_TYPE_NULL] = {
        {0, 0, 1, 1},
        {1, 0, 1, 1}
    };
    HilinkPara *hilinkPara = HILINK_GetParaCfg();

    for (i = 0; i < DS_MAX; ++i) {
        if (hilinkPara->macro_cfg[macro].ds[i].type < SERDES_TYPE_NULL) {
            los_val |= los[macro][hilinkPara->macro_cfg[macro].ds[i].type] << i ;
            los_mask_val |= los_mask[macro][hilinkPara->macro_cfg[macro].ds[i].type] << i ;
        }
    }
    HILINK_RegWriteBits(hilink_subctrl_base,
        HILINK_SUBCTRL_18_REG,
        HILINK_SUBCTRL_18_BIT4_OFFSET + HILINK_SUBCTRL_18_BIT4_LEN - 1,
        HILINK_SUBCTRL_18_BIT4_OFFSET,
        los_val);
    HILINK_RegWriteBits(hilink_subctrl_base,
        HILINK_SUBCTRL_18_REG,
        HILINK_SUBCTRL_18_BIT0_OFFSET + HILINK_SUBCTRL_18_BIT0_LEN - 1,
        HILINK_SUBCTRL_18_BIT0_OFFSET,
        los_mask_val);
}

STATIC void hilink_soc_alos_mask(u32 macro, void* hilink_subctrl_base)
{
    u32 alos_val = 0;
    u32 alos_mask_val = 0;
    u32 i;
    u32 alos[MACRO_MAX][SERDES_TYPE_NULL] = {
        {0, 0, 1, 1},
        {0, 0, 1, 1}
    };
    u32 alos_mask[MACRO_MAX][SERDES_TYPE_NULL] = {
        {0, 0, 1, 1},
        {1, 0, 1, 1}
    };
    HilinkPara *hilinkPara = HILINK_GetParaCfg();

    for (i = 0; i < DS_MAX; ++i) {
        if (hilinkPara->macro_cfg[macro].ds[i].type < SERDES_TYPE_NULL) {
            alos_val |= alos[macro][hilinkPara->macro_cfg[macro].ds[i].type] << i ;
            alos_mask_val |= alos_mask[macro][hilinkPara->macro_cfg[macro].ds[i].type] << i ;
        }
    }

    HILINK_RegWriteBits(hilink_subctrl_base,
        HILINK_SUBCTRL_20_REG,
        HILINK_SUBCTRL_20_BIT4_OFFSET + HILINK_SUBCTRL_20_BIT4_LEN - 1,
        HILINK_SUBCTRL_20_BIT4_OFFSET,
        alos_val);
    HILINK_RegWriteBits(hilink_subctrl_base,
        HILINK_SUBCTRL_20_REG,
        HILINK_SUBCTRL_20_BIT0_OFFSET + HILINK_SUBCTRL_20_BIT0_LEN - 1,
        HILINK_SUBCTRL_20_BIT0_OFFSET,
        alos_mask_val);
}

STATIC void hilink_soc_sds_mode_ctrl(u32 macro)
{
    u32 val = 0;
    u32 bit_offset;
    u32 i;
    u32 lane_order_map[DS_MAX] = {DS2, DS3, DS0, DS1};
    HilinkPara *hilinkPara = HILINK_GetParaCfg();

    for (i = 0; i < DS_MAX; i++) {
        if ((hilinkPara->macro_cfg[macro].lane_order == 1) &&
            (hilinkPara->macro_cfg[macro].ds[i].type == SERDES_TYPE_ETH)) {
            bit_offset = HILINK_SDS_MODE_OFFSET * lane_order_map[i];
        } else {
            bit_offset = HILINK_SDS_MODE_OFFSET * i;
        }

        if (hilinkPara->macro_cfg[macro].ds[i].type == SERDES_TYPE_USB) {
            val |= 0x2 << bit_offset;
        } else if (hilinkPara->macro_cfg[macro].ds[i].type == SERDES_TYPE_ETH) {
            val |= 0x4 << bit_offset;
        }
    }
    val = val | (val << 0x10);
    HILINK_RegWriteBits(hilinkPara->macro_info[macro].hilink_subctrl_base,
        HILINK_SUBCTRL_8_REG, BIT_31, BIT_0, val);
}

STATIC void hilink_soc_init(void)
{
    macro_info_t *macro0_info;
    macro_info_t *macro1_info;
    void* io_subctrl_base;
    void* hilink0_subctrl_base;
    void* hilink1_subctrl_base;
    HilinkPara *hilinkPara = HILINK_GetParaCfg();
    u32 val = 0;
    u32 i;
    bool using_pcie = false;

    macro0_info = &hilinkPara->macro_info[0];
    macro1_info = &hilinkPara->macro_info[1];
    hilink0_subctrl_base = (void*)(uintptr_t)macro0_info->hilink_subctrl_base;
    hilink1_subctrl_base = (void*)(uintptr_t)macro1_info->hilink_subctrl_base;
    io_subctrl_base = (void*)(uintptr_t)hilinkPara->io_subctrl_base;

    hilink_soc_set_apb_mode(macro0_info, 0);
    hilink_soc_set_apb_mode(macro1_info, 0);

    HILINK_RegWriteBits(io_subctrl_base,
        IO_SUBCTRL_SC_SDS_ICG_DIS_REG,
        IO_SUBCTRL_ICG_DIS_SDS_SYS_CLK_OFFSET + IO_SUBCTRL_ICG_DIS_SDS_SYS_CLK_LEN - 1,
        IO_SUBCTRL_ICG_DIS_SDS_SYS_CLK_OFFSET,
        0x3);

    hilink_soc_srst_dreq(io_subctrl_base); // cancel soft reset
    HILINK_RegWriteBits(io_subctrl_base, IO_SUBCTRL_SC_SDS_ICG_EN_REG,
        IO_SUBCTRL_ICG_EN_SDS_SYS_CLK_OFFSET + IO_SUBCTRL_ICG_EN_SDS_SYS_CLK_LEN - 1,
        IO_SUBCTRL_ICG_EN_SDS_SYS_CLK_OFFSET,
        0x3);

    for (i = 0; i < DS_MAX; ++i) {
        // check if lane was pcie
        if (hilinkPara->macro_cfg[MACRO_1].ds[i].type == SERDES_TYPE_PCIE) {
            using_pcie = true;
        } else if (hilinkPara->macro_cfg[MACRO_1].ds[i].type == SERDES_TYPE_USB) {
            val |= 1U << i;
        }
    }

    if ((using_pcie == false) || (val == 0)) {
        val = 0;
    }
    HILINK_RegWriteBits(io_subctrl_base, IO_SUBCTRL_SC_HILINK_USB_GE_DIV_MODE_REG,
        IO_SUBCTRL_HILINK_USB_GE_DIV_MODE_OFFSET + IO_SUBCTRL_HILINK_USB_GE_DIV_MODE_LEN - 1,
        IO_SUBCTRL_HILINK_USB_GE_DIV_MODE_OFFSET,
        val);

    hilink_soc_hilink_clk_en(io_subctrl_base);
    hilink_soc_sram_clk_en(io_subctrl_base);
    hilink_soc_mux_ctrl_init();
    hilink_soc_lane_ctrl_init();
    hilink_soc_mclk_sel();
    hilink_soc_crg_mode(MACRO_0);
    hilink_soc_crg_mode(MACRO_1);
    // set SDS MODE_CTRL
    hilink_soc_sds_mode_ctrl(MACRO_0);
    hilink_soc_sds_mode_ctrl(MACRO_1);
    hilink_soc_los_mask(MACRO_0, hilink0_subctrl_base);
    hilink_soc_los_mask(MACRO_1, hilink1_subctrl_base);
    hilink_soc_alos_mask(MACRO_0, hilink0_subctrl_base);
    hilink_soc_alos_mask(MACRO_1, hilink1_subctrl_base);
}

#ifdef CFG_SOC_PLATFORM_MDC_V11
STATIC u32 serdes_parse_dts(void)
{
    u32 macro_id;
    const struct device_node *np;
    struct device_node *child;
    struct serdes_dts_param *cfg;

    np = of_find_compatible_node(NULL, NULL, "hisilicon,hilink25cc");
    if (np == NULL) {
        HILINK_ERR("can't find hilink25cc node.\n");
        return RET_FAIL;
    }

    for_each_child_of_node(np, child) {
        if (of_property_read_u32(child, "id", &macro_id) != 0) {
            HILINK_ERR("%s: macro id not found.\n", child->full_name);
            return RET_FAIL;
        }
        if (macro_id >= MACRO_MAX) {
            HILINK_ERR("%s: macro id %d is invalid.\n", child->full_name, macro_id);
            return RET_FAIL;
        }
        cfg = &g_serdes_cfg[macro_id];

        if (of_property_read_u32(child, "mode", &cfg->mode) != 0) {
            HILINK_ERR("%s: macro mode not found.\n", child->full_name);
            return RET_FAIL;
        }
        if (cfg->mode == PCIE_MODE) {
            HILINK_INFO("%s: pcie mode not need to get FFE param.\n", child->full_name);
            continue;
        }

        if (of_property_read_u32_array(child, "tx-deemph", cfg->tx_deemph, LANE_MAX) != 0) {
            HILINK_ERR("%s: tx-deemph not found.\n", child->full_name);
            return RET_FAIL;
        }
        if (of_property_read_u32_array(child, "tx-margin", cfg->tx_margin, LANE_MAX) != 0) {
            HILINK_ERR("%s: tx-margin not found.\n", child->full_name);
            return RET_FAIL;
        }
    }

    HILINK_INFO("serdes parse dts done.\n");
    return 0;
}
#endif

STATIC u32 HILINK_InitMain(u32 macro, bool is_awake)
{
    u32 ret;
    bool flag;

    // 1. wait till power clk and bias stable
    ret = HILINK_SerdesPowerOn(macro, is_awake);
    if (ret != RET_OK) {
        HILINK_ERR("macro%d SerdesPowerOn fail!\n", macro);
        return RET_FAIL;
    }
    // BIAS setting
    HILINK_BiasInit(macro);

    // 2. PIN scen config
    HILINK_RefClkInit(macro);

    // 3. load firmware to sram
    ret = HILINK_LoadFwToSram(macro);
    if (ret != RET_OK) {
        HILINK_ERR("macro%d LoadFwToSram fail!\n", macro);
        return RET_FAIL;
    }

    // 4. CSR scen config
    HILINK_CsrScenInit(macro);

    // 5. activate
    flag = HILINK_MacroInit(macro, 0x10f, true, 0xf0);
    if (flag == false) {
        HILINK_ERR("macro%d MacroInit fail!\n", macro);
        return RET_FAIL;
    }

    ret = HILINK_SoftwareInit(macro);
    if (ret != RET_OK) {
        HILINK_ERR("macro%d Software Init fail!\n", macro);
        return RET_FAIL;
    }

#ifdef CFG_SOC_PLATFORM_MDC_V11
    // 6. set serdes FFE parameter
    ret = HILINK_SetSerdesFFE(macro, is_awake);
    if (ret != RET_OK) {
        HILINK_ERR("macro%d SetSerdesFFE fail!\n", macro);
        return RET_FAIL;
    }
#endif

    return ret;
}

STATIC u32 HILINK_Init(bool isAwake)
{
    HilinkPara *hilinkPara = HILINK_GetParaCfg();
    u32 ret = RET_OK;

    int retVal = HILINK_init_macro_info();
    if (retVal < 0) {
        HILINK_ERR("Init macro info error.");
        return RET_ERROR;
    }

    if (isAwake == true) {
        hilink_soc_init();

#ifdef CFG_SOC_PLATFORM_MDC_V11
        ret = serdes_parse_dts();
        if (ret != 0) {
            HILINK_ERR("Serdes parse dts fail.\n");
            return ret;
        }
#endif

        ret = HILINK_InitMain(MACRO_0, isAwake);
        if (ret != 0) {
            HILINK_ERR("Serdes init fail.\n");
            return ret;
        }
        ret = HILINK_InitMain(MACRO_1, isAwake);
        if (ret != 0) {
            HILINK_ERR("Serdes init fail.\n");
        } else {
            HILINK_INFO("Serdes init succ.\n");
        }
    } else {
        retVal = hilink_elastic_cfg_init();
        if (retVal != 0) {
            HILINK_ERR("hilink elastic cfg not exist, using default cfg.\n");
            retVal = hilink_get_macro_cfg_by_board_id(&(hilinkPara->macro_cfg[MACRO_0]), MACRO_0,
                EVB_BOARD_ID_BOM2_20T_0);
            if (retVal != 0) {
                HILINK_ERR("hilink get default cfg0 by board id fail.\n");
            }
            retVal = hilink_get_macro_cfg_by_board_id(&(hilinkPara->macro_cfg[MACRO_1]), MACRO_1,
                EVB_BOARD_ID_BOM2_20T_0);
            if (retVal != 0) {
                HILINK_ERR("hilink get default cfg1 by board id fail.\n");
            }
        }
    }
    return ret;
}

STATIC inline void UnmapIo(void* addr)
{
    if (addr != NULL) {
        iounmap(addr);
        addr = NULL;
    }
}

STATIC void HILINK_Uninit(void)
{
    HilinkPara *hilinkPara = HILINK_GetParaCfg();

    UnmapIo(hilinkPara->macro_info[MACRO_0].hilink_reg_base);
    UnmapIo(hilinkPara->macro_info[MACRO_0].hilink_subctrl_base);
    UnmapIo(hilinkPara->macro_info[MACRO_1].hilink_reg_base);
    UnmapIo(hilinkPara->macro_info[MACRO_1].hilink_subctrl_base);
    UnmapIo(hilinkPara->io_subctrl_base);

    return;
}

STATIC s32 hilink_module_probe(struct platform_device *pdev)
{
    u32 ret = RET_OK;

    drv_snapshot_bootdot_init(SERDES_MODULE_ID, SNAPSHOT_STATUS_STARTUP, SERDES_STARTUP_EXPECT);
    HILINK_INFO_SNAPSHOT(SERDES_STARTUP_BEGIN, "Serdes module probe start.\n");

    ret = HILINK_Init(false);
    if (ret != 0) {
        HILINK_ERR("Serdes module init fail.\n");
        return (s32)ret;
    }

    g_hilinkReadyFlag = 1;
    HILINK_INFO_SNAPSHOT(SERDES_STARTUP_EXPECT, "Serdes module init succ.\n");
    return 0;
}

STATIC void hilink_module_remove(struct platform_device *pdev)
{
    u32 ret = RET_OK;
    drv_snapshot_bootdot_init(SERDES_MODULE_ID, SNAPSHOT_STATUS_REMOVE, SERDES_REMOVE_EXPECT);
    HILINK_INFO_SNAPSHOT(SERDES_REMOVE_BEGIN, "Serdes module remove start.\n");

    g_hilinkReadyFlag = 0;
    HILINK_Uninit();

    HILINK_INFO_SNAPSHOT(SERDES_REMOVE_EXPECT, "Serdes module remove end.\n");
    
}

int hilink_module_suspend(void)
{
    int ret = 0;
    drv_snapshot_bootdot_init(SERDES_MODULE_ID, SNAPSHOT_STATUS_SUSPEND, SERDES_SUSPEND_EXPECT);
    HILINK_INFO_SNAPSHOT(SERDES_SUSPEND_BEGIN, "Serdes module suspend start.\n");

    g_hilinkReadyFlag = 0;
#ifdef FEATURE_HIGH_IMPEDANCE_ON_SUSPEND
    ret = HILINK_RxHighImpedance();
    if (ret != 0) {
        HILINK_ERR("Serdes module suspend fail.\n");
        return ret;
    }
#endif
    HILINK_Uninit();

    HILINK_INFO_SNAPSHOT(SERDES_SUSPEND_EXPECT, "Serdes module suspend end.\n");
    return ret;
}
EXPORT_SYMBOL(hilink_module_suspend);

int hilink_module_resume(void)
{
    u32 ret = RET_OK;
    drv_snapshot_bootdot_init(SERDES_MODULE_ID, SNAPSHOT_STATUS_RESUME, SERDES_RESUME_EXPECT);
    HILINK_INFO_SNAPSHOT(SERDES_RESUME_BEGIN, "Serdes module resume start.\n");

    ret = HILINK_Init(true);
    if (ret != 0) {
        HILINK_ERR("Serdes module init fail.\n");
        return (int)ret;
    }

    g_hilinkReadyFlag = 1;
    HILINK_INFO_SNAPSHOT(SERDES_RESUME_EXPECT, "Serdes module resume end.\n");
    return 0;
}
EXPORT_SYMBOL(hilink_module_resume);

int hilink_is_ready(void)
{
    return g_hilinkReadyFlag;
}
EXPORT_SYMBOL(hilink_is_ready);

int hal_kernel_high_priority_suspend(void)
{
    return hilink_module_suspend();
}
EXPORT_SYMBOL(hal_kernel_high_priority_suspend);

int hal_kernel_high_priority_resume(void)
{
    return hilink_module_resume();
}
EXPORT_SYMBOL(hal_kernel_high_priority_resume);

STATIC const struct of_device_id hilink_module_of_match[] = {
    {.compatible = "hisilicon,hilink25cc"},
    { }
};
MODULE_DEVICE_TABLE(of, hilink_module_of_match);

STATIC struct platform_driver hilink_module_driver = {
    .probe = hilink_module_probe,
    .remove = hilink_module_remove,
    .driver = {
        .name  = "hilink25cc",
        .of_match_table = hilink_module_of_match,
    },
};

STATIC int __init hilink_module_driver_init(void)
{
    int ret = platform_driver_register(&hilink_module_driver);
    if (ret != 0) {
        HILINK_ERR("can't register hilink driver: %d\n", ret);
        return ret;
    }

    return 0;
}

STATIC void __exit hilink_module_driver_exit(void)
{
    platform_driver_unregister(&hilink_module_driver);
    HILINK_INFO("hilink driver exit succ.\n");
}

module_init(hilink_module_driver_init);
module_exit(hilink_module_driver_exit);

MODULE_AUTHOR("Huawei Tech. Co., Ltd.");
MODULE_DESCRIPTION("Hisilicon hilink driver");
MODULE_LICENSE("GPL");
