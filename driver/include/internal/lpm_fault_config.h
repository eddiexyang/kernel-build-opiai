/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
*/

#ifndef LPM_FAULT_CONFIG_H
#define LPM_FAULT_CONFIG_H

#include <linux/types.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include "dms_sensor.h"
#include "dms_define.h"
#include "icm_interface.h"
#include "lpm_fault_common.h"

#ifdef LPM_REPORT

// all supported event_types are defined here

// for DMS_SEN_TYPE_HEARTBEAT(0x27)
#define LPM_FAULT_EVENT_HEARTBEAT_LOST     ((uint32_t)HEARTBEAT_ERROR_TYPE_HEARTBEAT_LOST) // 0

// for DMS_SEN_TYPE_TEMPERATURE(0x1)
#define LPM_FAULT_EVENT_TEMPER_HIGH        ((uint32_t)0x7) // Crossed Upper non-critical going high
#define LPM_FAULT_EVENT_TEMPER_LOW         ((uint32_t)0x0) // Crossed Lower non-critical going low

// for DMS_SEN_TYPE_VOLTAGE(0x2)
#define LPM_FAULT_EVENT_VOLTAGE_OVER       ((uint32_t)0x9) // Power Glitch Sensor voltage over
#define LPM_FAULT_EVENT_VOLTAGE_UNDER      ((uint32_t)0x2) // Power Glitch Sensor voltage under

// for DMS_SEN_TYPE_POWER_UNIT(0x9)
#define LPM_FAULT_EVENT_PMU_HIGH           ((uint32_t)0x08) // PMU non-critical high temp
#define LPM_FAULT_EVENT_PMU_SENSOR_BAD     ((uint32_t)0x09) // PMU temp sensor is bad
#define LPM_FAULT_EVENT_PMU_REPORT_TIMEOUT ((uint32_t)0x0A) // PMU reporting timeout(minor)
#define LPM_FAULT_EVENT_PMU_CONFIG_ERR     ((uint32_t)0x0B) // Power Unit config error
#define LPM_FAULT_EVENT_PMU_CONNECT_ERR    ((uint32_t)0x0D) // Power Unit connect error

// for DMS_SEN_TYPE_RAS_SENSOR(0xC0)
#define LPM_FAULT_EVENT_ERROR              ((uint32_t)RAS_ERROR_TYPE_ERROR) // 0x0, module error
#define LPM_FAULT_EVENT_ERROR_CANNOT_FIXED ((uint32_t)RAS_ERROR_TYPE_ERROR_NF)   // 0x1, module error can not be fixed
#define LPM_FAULT_EVENT_INPUT_ERROR        ((uint32_t)RAS_ERROR_TYPE_INPUT_ERR)  // 0x2, input error
#define LPM_FAULT_EVENT_IN_CFG_ERR         ((uint32_t)RAS_ERROR_TYPE_IN_CFG_ERR) // 0x3, internal config error
#define LPM_FAULT_EVENT_CONFIG_ERR         ((uint32_t)RAS_ERROR_TYPE_CFG_ERR)    // 0x4, config error
#define LPM_FAULT_EVENT_PARITY_ERR         ((uint32_t)RAS_ERROR_TYPE_PARITY) // 0x5, parity error
#define LPM_FAULT_EVENT_SBECCOVERTHOLD     ((uint32_t)RAS_ERROR_TYPE_SBECCOverThold) // 0x6, single bit error
#define LPM_FAULT_EVENT_MBECC              ((uint32_t)RAS_ERROR_TYPE_MBECC)      // 0x8, multiple bit ecc error
#define LPM_FAULT_EVENT_BUS_ERR            ((uint32_t)RAS_ERROR_TYPE_BUS_ERR)    // 0x9, bus error
#define LPM_FAULT_EVENT_TIMEOUT_ERR        ((uint32_t)RAS_ERROR_TYPE_TIMEOUT_ERR)    // 0xA, service timeout

// for DMS_SEN_TYPE_SAFETY_SENSOR(0xC3)
#define LPM_FAULT_EVENT_LOCKSTEP_ERR       ((uint32_t)SOC_SAFETY_LOCKSTEP_ERR) // 0x0, lockstep error
// 0xF, unauthorized access failure notification
#define LPM_FAULT_EVENT_UN_ACCESS_NOTIF    ((uint32_t)SOC_SAFETY_UN_ACCESS_NOTIF)

// for DMS_SEN_TYPE_EXTEND_SENSOR(0xC4)
#define LPM_FAULT_EVENT_MONITOR_TIMEOUT    ((uint32_t)0x0) // monitor timeout(minor)
#define LPM_FAULT_EVENT_REPORT_TIMEOUT     ((uint32_t)0x2) // reporting timeout(minor)
#define LPM_FAULT_EVENT_PMU_IRQ_TEST       ((uint32_t)0x4) // PMU IRQ report test failure
#define LPM_FAULT_EVENT_SAFETY_REG_CFG_ERR       ((uint32_t)0x6) // safety reg cfg err
#define LPM_FAULT_EVENT_COMMON_REG_CFG_ERR       ((uint32_t)0x8) // reg cfg err

// for DMS_SEN_TYPE_CHECK_SENSOR(0xC6)
#define LPM_FAULT_EVENT_CRC_CHECK_FAIL     ((uint32_t)0x1) // CRC check fail

// for DMS_SEN_TYPE_SYSTEM_FW_PROGRESS(0xF)
#define LPM_FAULT_EVENT_FW_HANG            ((uint32_t)0x1) // System Firmware Hang
#define LPM_FAULT_EVENT_FW_SOC_ERROR       ((uint32_t)0x4) // System Firmware Error (next SOC boot failure)

// for DMS_SEN_TYPE_POWER_SUPPLY(0x8)
#define LPM_FAULT_EVENT_POWER_INPUT_OUT_RANGE    ((uint32_t)0x5) // Power Supply input out-of-range, but present
#define LPM_FAULT_EVENT_POWER_OUT_RANGE          ((uint32_t)0x7) // Power Supply output out-of-range, but present

// for DMS_SEN_TYPE_CURRENT(0x03)
#define LPM_FAULT_EVENT_CURRENT_CRITICAL_HIGH       ((uint32_t)0x7) // Crossed Lower critical going high

// for DMS_SEN_TYPE_CRYPTO_SENSOR(0xC7)
#define LPM_FAULT_EVENT_SYSTEM_SECURITY_MACHANISM_FAIL      ((uint32_t)0x0) // system-level security mechanism failure
#define LPM_FAULT_EVENT_CRYPTO_MODULE_FAIL                  ((uint32_t)0x2) // crypto algorithm module failure
#define LPM_FAULT_EVENT_CRYPTO_EFUSE_CANNOT_BURNT           ((uint32_t)0x7) // EFUSE cannot be burnt

// for DMS_SEN_TYPE_SCHEDULER_SENSOR(0xC9)
#define LPM_FAULT_EVENT_TSENSOR_EXCEP   ((uint32_t)0x0) // Tsensor signal timeout

// for DMS_SEN_TYPE_CHIP_HARDWARE(0xD1)
#define LPM_FAULT_EVENT_DVFS_VOLT_AICORE_EXCEP   ((uint32_t)0x1)
#define LPM_FAULT_EVENT_DVFS_FREQ_AICORE_EXCEP   ((uint32_t)0x2)
#define LPM_FAULT_EVENT_CURRENT_GET_EXCEP        ((uint32_t)0x3)

int32_t lpm_fault_config_exit(uint32_t dev_num);
int32_t lpm_fault_config_init(uint32_t dev_num);

#else
static inline int32_t lpm_fault_config_exit(uint32_t dev_num)
{
	(void)dev_num;
	return 0;
}
static inline  int32_t lpm_fault_config_init(uint32_t dev_num)
{
	(void)dev_num;
	return 0;
}
#endif

#endif
