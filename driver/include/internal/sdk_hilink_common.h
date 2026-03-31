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

#ifndef SDK_HILINK_COMMON_H
#define SDK_HILINK_COMMON_H

#include "drv_log.h"
#include "securec.h"

#include "lib_bitop_api.h"
#include "sdk_hilink_pub.h"
#include "sdk_hilink_cfg.h"
#include "sdk_hilink_snapshot.h"

#ifndef STATIC
#define STATIC static
#endif

#define HILINK_ERR_SNAPSHOT(snapshot_num, fmt, ...) do { \
    drv_snapshot_bootdot_set(SERDES_MODULE_ID, snapshot_num); \
    drv_err(module_name_serdes, fmt, \
    ##__VA_ARGS__); \
} while (0)
#define HILINK_INFO_SNAPSHOT(snapshot_num, fmt, ...) do { \
    drv_snapshot_bootdot_set(SERDES_MODULE_ID, snapshot_num); \
    drv_info(module_name_serdes, fmt, \
    ##__VA_ARGS__); \
} while (0)
#define HILINK_ERR(fmt, ...) do { \
    drv_err(module_name_serdes, fmt, \
    ##__VA_ARGS__); \
} while (0)
#define HILINK_WARN(fmt, ...) do { \
    drv_warn(module_name_serdes, fmt, \
    ##__VA_ARGS__); \
} while (0)
#define HILINK_INFO(fmt, ...) do { \
    drv_info(module_name_serdes, fmt, \
    ##__VA_ARGS__); \
} while (0)
#define HILINK_DEBUG(fmt, ...) do { \
    drv_debug(module_name_serdes, fmt, \
    ##__VA_ARGS__); \
} while (0)
#define HILINK_DFX_PRINTF(fmt, ...) do { \
    drv_info(module_name_serdes, fmt, \
    ##__VA_ARGS__); \
} while (0)

#define CHECK_RETURN_PRINT(str, val, range, ...) \
    if ((val) >= (range)) { \
        HILINK_ERR("%s %d out of range.\n", #str, val); \
        return __VA_ARGS__; \
    }
#define CHECK_RETURN(val, range, ...) CHECK_RETURN_PRINT(val, val, range, __VA_ARGS__)

#define CHECK_RETURN_SNAPSHOT_PRINT(snapshot_num, str, val, range, ...) \
    if ((val) >= (range)) { \
        HILINK_ERR_SNAPSHOT(snapshot_num, "%s %d out of range.\n", #str, val); \
        return __VA_ARGS__; \
    }
#define CHECK_RETURN_SNAPSHOT(snapshot_num, val, range, ...) \
    CHECK_RETURN_SNAPSHOT_PRINT(snapshot_num, val, val, range, __VA_ARGS__)

#define module_name_serdes "drv_serdes"

/* Hilink Macro Base Address */
#define HILINK0_REG_BASE 0xA4000000
#define HILINK1_REG_BASE 0xA4080000
#define HILINK0_REG_SIZE 0x80000
#define HILINK1_REG_SIZE 0x80000
#define HILINK0_SUBCTRL_BASE 0xA4100000
#define HILINK1_SUBCTRL_BASE 0xA4110000
#define HILINK0_SUBCTRL_SIZE 0x10000
#define HILINK1_SUBCTRL_SIZE 0x10000

#define IO_SUBCTRL_REG_BASE 0xA0120000
#define IO_SUBCTRL_REG_SIZE 0x10000
#define AO_SUBCTRL_REG_BASE 0xC0120000
#define AO_SUBCTRL_REG_SIZE 0x10000

#define TOPAPI_REG_BASE         0xFDE8
#define ABISTAPI_REG_BASE       0xFBF0
#define TOPFSMAPI_REG_BASE      0xFB78
#define DSFSMAPI_REG_BASE       0xFB40

#define MACRO_REG_BASE(macroId) (HILINK0_REG_BASE + (0x80000 * (u32)(macroId)))
#define CP_CSR(reg) (0x40000U + (0x4U * (u32)(reg)))
#define CS_CSR(csId, reg) ((0x40400U + (0x400U * (u32)(csId))) + (0x4U * (reg)))
#define TX_CSR(txId, reg) ((0x48000U + (0x800U * (u32)(txId))) + (0x4U * (reg)))
#define RX_CSR(rxId, reg) ((0x48400U + (0x800U * (u32)(rxId))) + (0x4U * (reg)))
#define ADAP_CSR(laneId, reg) ((0x50000U + (0x400U * (u32)(laneId))) + (0x4U * (reg)))

#define TOPAPI_CSR(reg) (0xFDE8U + (0x4U * (u32)(reg)))
#define CSAPI_CSR(reg) (0xFDCCU + (0x4U * (u32)(reg)))
#define DSAPI_CSR(laneId, reg) ((0xFD94U - (56U * (u32)(laneId))) + (0x4U * (u32)(reg)))
#define ABISTAPI_CSR(reg) (0xFBF0U + (0x4U * (u32)(reg)))
#define ROMAPI_CSR(reg) (0xFBD0U + (0x4U * (u32)(reg)))
#define MACROAPI_CSR(reg) (0xFBB0U + (0x4U * (u32)(reg)))
#define FSMAPI_CSR(laneId, reg) ((0xFB78U - (72U * (u32)(laneId))) + (0x4U * (u32)(reg)))

#define CS_CSR_REG_OFFSET(csId, reg) ((0x400U * (u32)(csId)) + (reg))
#define RX_CSR_REG_OFFSET(rxId, reg) ((0x800U * (u32)(rxId)) + (reg))
#define TX_CSR_REG_OFFSET(txId, reg) ((0x800U * (u32)(txId)) + (reg))
#define ADAP_CSR_REG_OFFSET(laneId, reg) ((0x400U * (u32)(laneId)) + (reg))
#define DSAPI_CSR_REG_OFFSET(laneId, reg) ((reg) - (56U * (u32)(laneId)))
#define DSFSMAPI_CSR_REG_OFFSET(laneId, reg) ((reg) - (56U * (u32)(laneId)))

/* HAL_MsSleep */
#define TIMER_MULTOPLE 100

#define RET_OK              0
#define RET_ERROR           1
#define RET_TIME_OUT        2
#define RET_FAIL            0x7F

#define HILINK_OK     0x0
#define HILINK_FAILED 0xFFFFFFFF

// register len is 4 bytes / 32 bits
#define REG_LEN 0x4
#define SLICE_MAX_NUM 10
#define CS_SLICE_START_ID 8

/* EthScenarioDataRateConfig */
#define PLL_CS_PARA_NUM 9
#define PLL_TX_PARA_NUM 1
#define PLL_RX_PARA_NUM 5
#define PLL0_CS_PARA_ADDR (0x8AFC)
#define PLL1_CS_PARA_ADDR (PLL0_CS_PARA_ADDR + (PLL_CS_PARA_NUM * 0x4))
#define PLL0_TX_PARA_ADDR (PLL1_CS_PARA_ADDR + (PLL_CS_PARA_NUM * 0x4))
#define PLL1_TX_PARA_ADDR (PLL0_TX_PARA_ADDR + (PLL_TX_PARA_NUM * 0x4))
#define PLL0_RX_PARA_ADDR (PLL1_TX_PARA_ADDR + (PLL_TX_PARA_NUM * 0x4))
#define PLL1_RX_PARA_ADDR (PLL0_RX_PARA_ADDR + (PLL_RX_PARA_NUM * 0x4))

/* eth_rx_para_config */
#define DS_ETH_RX_PARA_CONFIG_NUM 7
#define DS_ETH_RATE_NUM 2

/* HILINK_WaitFsmDone */
#define WAIT_FSM_DONE_TIMEOUT_CNT 100
#define GET_FSM_DONE_REG_LEN 5

/* StopSliceFwTask */
#define STOP_SLICE_FW_TASK_TIMEOUT_CNT 5000

/* DsPinEn/Disable */
#define DS_PIN_ENBALE_TIMEOUT_CNT 12
#define DS_PIN_DISBALE_FSM_DONE_SHORT_TIMEOUT_CNT 10
#define DS_PIN_DISBALE_FSM_DONE_LONG_TIMEOUT_CNT 100
#define SDPI_MODE_SATA 2
#define SDPI_MODE_PCIE 0
#define SDPI_MODE_MPHY 8

/* DsPowerModeSwitch */
#define DS_POWER_MODE_SWITCH_TIMEOUT 200
/* TxSdpiCmdExecute */
#define TX_SDPI_CMD_EXECUTE_SLEEP_US 10
#define TX_SDPI_CMD_EXECUTE_TIMEOUT_CNT 12
/* SetTxFFE */
#define TX_DRIVER_CONTROL_CMD_ID 0x40000000U
#define SET_TX_DEE_TX_MARGIN_OFFSET 18

/* hilink_switch_control_mode */
typedef enum {
    HILINK_MODE_PCIE   = 0,
    HILINK_MODE_USB    = 1,
    HILINK_MODE_SATA   = 2,     // SATA/SAS
    HILINK_MODE_DP     = 3,     // DisplayPort
    HILINK_MODE_ETH    = 4,
    HILINK_MODE_CONVIO = 7,     // Converged IO
    HILINK_MODE_MPHY   = 8,
} HiLinkModeE;

typedef enum {
    PS_PCIE_L0 = 0,
    PS_PCIE_L0S,
    PS_PCIE_L1,
    PS_PCIE_L2,
    PS_PCIE_L1pCPM,
    PS_PCIE_L1p1,
    PS_PCIE_L1p2,

    PS_USB_U0,
    PS_USB_U1,
    PS_USB_U2,
    PS_USB_U3,

    PS_SAS_PHYRDY,
    PS_SAS_PARTIAL,
    PS_SAS_SLUMBER,

    PS_MPHY_BURST,
    PS_MPHY_H8,

    PS_P0,
    PS_P6,

    PS_TYPE_END
} PowerSwitchTypeE;

typedef enum {
    CTRL_MODE_PIN = 0,
    CTRL_MODE_REG,
} SdpiMode_e;
typedef struct {
    void __iomem *hilink_reg_base;
    void __iomem *hilink_subctrl_base;
} macro_info_t;

typedef struct {
    void __iomem *io_subctrl_base;
    void __iomem *ao_subctrl_base;
    macro_info_t macro_info[MACRO_MAX];
    struct hilink_macro_cfg macro_cfg[MACRO_MAX];
} HilinkPara;

typedef struct {
    HilinkPara* hilink_para;
    u32 macro;
    u32 id;
} HilinkRegBase;

struct hilink_reg_info {
    u32 base;
    u32 offset;
    u32 len;
};

typedef struct {
    PowerSwitchTypeE type;
    unsigned char mode;
} PowerSwitchConfig_t;

HilinkPara *HILINK_GetParaCfg(void);
void Hilink_SetRegBase(HilinkRegBase* base, HilinkPara* hilink_para, u32 macro, u32 id);
void HAL_UsSleep(u64 us);
void HAL_MsSleep(u64 ms);

bool HILINK_WaitFsmDone(u32 macro);
int StopSliceFwTask(u32 macro, unsigned int sliceMask);
void SwitchControlMode(u32 macro, u32 dsId, SdpiMode_e ctrlMode);
void EthScenarioDataRateConfig(u32 macro, u32 csIndex, DRV_SERDES_FREQUENCY_LEVEL targetRate);
u32 eth_rx_para_config(u32 macro, u32 ds, DRV_SERDES_FREQUENCY_LEVEL targetRate);
u32 HILINK_DsDataRateSwitch(u32 macro, u32 dsIndex, HilinkRate targetRate, u32 dsWidth);
bool LinkDataRateSwitch(u32 macro, u32 sliceMask, HilinkRate targetRate, u32 dsWidth);
u32 HILINK_LinkDataRateSwitch(u32 macro, u32 sliceMask, HilinkRate targetRate, u32 dsWidth);

void HILINK_RegWriteBits(void* baseAddr, u32 reg, u32 highBit, u32 lowBit, u32 val);
u32 HILINK_RegReadBits(void* baseAddr, u32 reg, u32 highBit, u32 lowBit);
void HILINK_RegWriteOffset(void* baseAddr, u32 reg, u32 offset, u32 len, u32 val);
u32 HILINK_RegReadOffset(void* baseAddr, u32 reg, u32 offset, u32 len);

u32 DsPowerModeSwitch(u32 macro, u32 dsIndex, PowerSwitchTypeE type);
u32 TxSdpiCmdExecute(u32 macro, u32 txId, u32 txSdpiCmd);
u32 SetTxFFE(u32 macro, u32 txId, u32 txMargin, u32 txDeemph);

/* REG read/write by register bank */
static inline void HILINK_AORegWriteBits(HilinkPara *hilinkPara, u32 reg, u32 offset, u32 len, u32 val)
{
    HILINK_RegWriteBits((void*)(uintptr_t)(hilinkPara->ao_subctrl_base), reg, offset + len - 1, offset, val);
}

static inline u32 HILINK_AORegReadBits(HilinkPara *hilinkPara, u32 reg, u32 offset, u32 len)
{
    return HILINK_RegReadBits((void*)(uintptr_t)(hilinkPara->ao_subctrl_base), reg, offset + len - 1, offset);
}

static inline void HILINK_IORegWriteBits(HilinkPara *hilinkPara, u32 reg, u32 offset, u32 len, u32 val)
{
    HILINK_RegWriteBits((void*)(uintptr_t)(hilinkPara->io_subctrl_base), reg, offset + len - 1, offset, val);
}

static inline u32 HILINK_IORegReadBits(HilinkPara *hilinkPara, u32 reg, u32 offset, u32 len)
{
    return HILINK_RegReadBits((void*)(uintptr_t)(hilinkPara->io_subctrl_base), reg, offset + len - 1, offset);
}

static inline void HILINK_SubctrlRegWriteBits(HilinkRegBase base, u32 reg, u32 offset, u32 len, u32 val)
{
    HILINK_RegWriteBits((void*)(uintptr_t)(base.hilink_para->macro_info[base.macro].hilink_subctrl_base),
        reg, offset + len - 1, offset, val);
}

static inline u32 HILINK_SubctrlRegReadBits(HilinkRegBase base, u32 reg, u32 offset, u32 len)
{
    return HILINK_RegReadBits((void*)(uintptr_t)(base.hilink_para->macro_info[base.macro].hilink_subctrl_base),
        reg, offset + len - 1, offset);
}

static inline void HILINK_CPRegWriteBits(HilinkRegBase base, u32 reg, u32 offset, u32 len, u32 val)
{
    HILINK_RegWriteBits((void*)(uintptr_t)(base.hilink_para->macro_info[base.macro].hilink_reg_base),
        reg, offset + len - 1, offset, val);
}

static inline u32 HILINK_CPRegReadBits(HilinkRegBase base, u32 reg, u32 offset, u32 len)
{
    return HILINK_RegReadBits((void*)(uintptr_t)(base.hilink_para->macro_info[base.macro].hilink_reg_base),
        reg, offset + len - 1, offset);
}

static inline void HILINK_CSRegWriteBits(HilinkRegBase base, u32 reg, u32 offset, u32 len, u32 val)
{
    HILINK_RegWriteBits((void*)(uintptr_t)(base.hilink_para->macro_info[base.macro].hilink_reg_base),
        CS_CSR_REG_OFFSET(base.id, reg), offset + len - 1, offset, val);
}

static inline u32 HILINK_CSRegReadBits(HilinkRegBase base, u32 reg, u32 offset, u32 len)
{
    return HILINK_RegReadBits((void*)(uintptr_t)(base.hilink_para->macro_info[base.macro].hilink_reg_base),
        CS_CSR_REG_OFFSET(base.id, reg), offset + len - 1, offset);
}

static inline void HILINK_TxRegWriteBits(HilinkRegBase base, u32 reg, u32 offset, u32 len, u32 val)
{
    HILINK_RegWriteBits((void*)(uintptr_t)(base.hilink_para->macro_info[base.macro].hilink_reg_base),
        TX_CSR_REG_OFFSET(base.id, reg), offset + len - 1, offset, val);
}

static inline u32 HILINK_TxRegReadBits(HilinkRegBase base, u32 reg, u32 offset, u32 len)
{
    return HILINK_RegReadBits((void*)(uintptr_t)(base.hilink_para->macro_info[base.macro].hilink_reg_base),
        TX_CSR_REG_OFFSET(base.id, reg), offset + len - 1, offset);
}

static inline void HILINK_RxRegWriteBits(HilinkRegBase base, u32 reg, u32 offset, u32 len, u32 val)
{
    HILINK_RegWriteBits((void*)(uintptr_t)(base.hilink_para->macro_info[base.macro].hilink_reg_base),
        RX_CSR_REG_OFFSET(base.id, reg), offset + len - 1, offset, val);
}

static inline u32 HILINK_RxRegReadBits(HilinkRegBase base, u32 reg, u32 offset, u32 len)
{
    return HILINK_RegReadBits((void*)(uintptr_t)(base.hilink_para->macro_info[base.macro].hilink_reg_base),
        RX_CSR_REG_OFFSET(base.id, reg), offset + len - 1, offset);
}

static inline void HILINK_AdapRegWriteBits(HilinkRegBase base, u32 reg, u32 offset, u32 len, u32 val)
{
    HILINK_RegWriteBits((void*)(uintptr_t)(base.hilink_para->macro_info[base.macro].hilink_reg_base),
        ADAP_CSR_REG_OFFSET(base.id, reg), offset + len - 1, offset, val);
}

static inline u32 HILINK_AdapRegReadBits(HilinkRegBase base, u32 reg, u32 offset, u32 len)
{
    return HILINK_RegReadBits((void*)(uintptr_t)(base.hilink_para->macro_info[base.macro].hilink_reg_base),
        ADAP_CSR_REG_OFFSET(base.id, reg), offset + len - 1, offset);
}

static inline void HILINK_MacroApiRegWriteBits(HilinkRegBase base, u32 reg, u32 offset, u32 len, u32 val)
{
    HILINK_RegWriteBits((void*)(uintptr_t)(base.hilink_para->macro_info[base.macro].hilink_reg_base),
        reg, offset + len - 1, offset, val);
}

static inline u32 HILINK_MacroApiRegReadBits(HilinkRegBase base, u32 reg, u32 offset, u32 len)
{
    return HILINK_RegReadBits((void*)(uintptr_t)(base.hilink_para->macro_info[base.macro].hilink_reg_base),
        reg, offset + len - 1, offset);
}

static inline void HILINK_CsApiRegWriteBits(HilinkRegBase base, u32 reg, u32 offset, u32 len, u32 val)
{
    HILINK_RegWriteBits((void*)(uintptr_t)(base.hilink_para->macro_info[base.macro].hilink_reg_base),
        reg, offset + len - 1, offset, val);
}

static inline u32 HILINK_CsApiRegReadBits(HilinkRegBase base, u32 reg, u32 offset, u32 len)
{
    return HILINK_RegReadBits((void*)(uintptr_t)(base.hilink_para->macro_info[base.macro].hilink_reg_base),
        reg, offset + len - 1, offset);
}

static inline void HILINK_DsApiRegWriteBits(HilinkRegBase base, u32 reg, u32 offset, u32 len, u32 val)
{
    HILINK_RegWriteBits((void*)(uintptr_t)(base.hilink_para->macro_info[base.macro].hilink_reg_base),
        DSAPI_CSR_REG_OFFSET(base.id, reg), offset + len - 1, offset, val);
}

static inline u32 HILINK_DsApiRegReadBits(HilinkRegBase base, u32 reg, u32 offset, u32 len)
{
    return HILINK_RegReadBits((void*)(uintptr_t)(base.hilink_para->macro_info[base.macro].hilink_reg_base),
        DSAPI_CSR_REG_OFFSET(base.id, reg), offset + len - 1, offset);
}

static inline void HILINK_RomApiRegWriteBits(HilinkRegBase base, u32 reg, u32 offset, u32 len, u32 val)
{
    HILINK_RegWriteBits((void*)(uintptr_t)(base.hilink_para->macro_info[base.macro].hilink_reg_base),
        reg, offset + len - 1, offset, val);
}

static inline u32 HILINK_RomApiRegReadBits(HilinkRegBase base, u32 reg, u32 offset, u32 len)
{
    return HILINK_RegReadBits((void*)(uintptr_t)(base.hilink_para->macro_info[base.macro].hilink_reg_base),
        reg, offset + len - 1, offset);
}

#endif