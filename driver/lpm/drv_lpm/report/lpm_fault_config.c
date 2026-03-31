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

#include <linux/kernel.h>
#include "devdrv_manager_comm.h"
#include "dms_interface.h"
#include "dms_sensor_type.h"
#include "dms_dev_node.h"
#include "dms_sensor.h"
#include "dms_node_type.h"
#include "lpm_fault_report.h"
#include "lpm_fault_config.h"

STATIC struct lpm_fault_event_desc *lpm_fault_get_heartbeat_event(uint32_t *event_num)
{
	// DMS_SEN_TYPE_HEARTBEAT
	static struct lpm_fault_event_desc heartbeat_table[] = {
		{
			.event_type = LPM_FAULT_EVENT_HEARTBEAT_LOST,
			.describe   = "LPM hearbeat timeout"
		}
	};
	*event_num = (uint32_t)ARRAY_SIZE(heartbeat_table);
	return &heartbeat_table[0];
}

STATIC uint32_t lpm_fault_heartbeat_assert_mask(uint32_t node_type)
{
	if (node_type == (uint32_t)DMS_DEV_TYPE_LPM) {
		return ((uint32_t)0x1 << LPM_FAULT_EVENT_HEARTBEAT_LOST);
	}
	return 0;
}

STATIC uint32_t lpm_fault_heartbeat_deassert_mask(uint32_t node_type)
{
	if (node_type == (uint32_t)DMS_DEV_TYPE_LPM) {
		return ((uint32_t)0x1 << LPM_FAULT_EVENT_HEARTBEAT_LOST);
	}
	return 0;
}

#if defined(CFG_SOC_PLATFORM_CLOUD) || defined(CFG_SOC_PLATFORM_MDC_V51) || \
	defined(CFG_SOC_PLATFORM_CLOUD_V2) || defined(CFG_SOC_PLATFORM_MINIV3)
STATIC struct lpm_fault_event_desc *lpm_fault_get_temperature_event(uint32_t *event_num)
{
	// DMS_SEN_TYPE_TEMPERATURE
	static struct lpm_fault_event_desc temperature_table[] = {
		{
			.event_type = LPM_FAULT_EVENT_TEMPER_LOW,
			.describe   = "Soc low temperature"
		}, {
			.event_type = LPM_FAULT_EVENT_TEMPER_HIGH,
			.describe   = "Soc high temperature"
		}
	};
	*event_num = (uint32_t)ARRAY_SIZE(temperature_table);
	return &temperature_table[0];
}

STATIC uint32_t lpm_fault_temperature_assert_mask(uint32_t node_type)
{
	if (node_type == (uint32_t)DMS_DEV_TYPE_LPM) {
#if defined(CFG_SOC_PLATFORM_CLOUD) || defined(CFG_SOC_PLATFORM_CLOUD_V2)
		return ((uint32_t)0x1 << LPM_FAULT_EVENT_TEMPER_HIGH);
#elif defined(CFG_SOC_PLATFORM_MDC_V51) || defined(CFG_SOC_PLATFORM_MINIV3)
		return (((uint32_t)0x1 << LPM_FAULT_EVENT_TEMPER_LOW) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_TEMPER_HIGH));
#endif
	}
#if defined (CFG_SOC_PLATFORM_MDC_V51)
	if ((node_type == (uint32_t)DMS_DEV_TYPE_VR) || (node_type == (uint32_t)DMS_DEV_TYPE_PMU)) {
		return((uint32_t)0x1 << LPM_FAULT_EVENT_TEMPER_HIGH);
	}
#elif defined(CFG_SOC_PLATFORM_MINIV3)
	if (node_type == (uint32_t)DMS_DEV_TYPE_VR) {
		return((uint32_t)0x1 << LPM_FAULT_EVENT_TEMPER_HIGH);
	}
#endif
	return 0;
}

STATIC uint32_t lpm_fault_temperature_deassert_mask(uint32_t node_type)
{
	if (node_type == (uint32_t)DMS_DEV_TYPE_LPM) {
#if defined(CFG_SOC_PLATFORM_CLOUD) || defined(CFG_SOC_PLATFORM_CLOUD_V2)
		return ((uint32_t)0x1 << LPM_FAULT_EVENT_TEMPER_HIGH);
#elif defined(CFG_SOC_PLATFORM_MDC_V51) || defined(CFG_SOC_PLATFORM_MINIV3)
		return (((uint32_t)0x1 << LPM_FAULT_EVENT_TEMPER_LOW) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_TEMPER_HIGH));
#endif
	}
#if defined (CFG_SOC_PLATFORM_MDC_V51)
	if ((node_type == (uint32_t)DMS_DEV_TYPE_VR) || (node_type == (uint32_t)DMS_DEV_TYPE_PMU)) {
		return((uint32_t)0x1 << LPM_FAULT_EVENT_TEMPER_HIGH);
	}
#elif defined(CFG_SOC_PLATFORM_MINIV3)
	if (node_type == (uint32_t)DMS_DEV_TYPE_VR) {
		return((uint32_t)0x1 << LPM_FAULT_EVENT_TEMPER_HIGH);
	}
#endif
	return 0;
}

#if defined(CFG_SOC_PLATFORM_MDC_V51)
STATIC struct lpm_fault_event_desc *lpm_fault_get_voltage_event(uint32_t *event_num)
{
	// DMS_SEN_TYPE_VOLTAGE
	static struct lpm_fault_event_desc voltage_table[] = {
		{
			.event_type = LPM_FAULT_EVENT_VOLTAGE_OVER,
			.describe   = "PG sensor over voltage"
		}, {
			.event_type = LPM_FAULT_EVENT_VOLTAGE_UNDER,
			.describe   = "PG sensor under voltage"
		}
	};
	*event_num = (uint32_t)ARRAY_SIZE(voltage_table);
	return &voltage_table[0];
}

STATIC uint32_t lpm_fault_voltage_assert_mask(uint32_t node_type)
{
	if (node_type == (uint32_t)DMS_DEV_TYPE_POWER_GLITCH_SENSOR) {
		return (((uint32_t)0x1 << LPM_FAULT_EVENT_VOLTAGE_OVER) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_VOLTAGE_UNDER));
	}
	return 0;
}

STATIC uint32_t lpm_fault_voltage_deassert_mask(uint32_t node_type)
{
	if (node_type == (uint32_t)DMS_DEV_TYPE_POWER_GLITCH_SENSOR) {
		return (((uint32_t)0x1 << LPM_FAULT_EVENT_VOLTAGE_OVER) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_VOLTAGE_UNDER));
	}
	return 0;
}
#endif

STATIC struct lpm_fault_event_desc *lpm_fault_get_power_supply_event(uint32_t *event_num)
{
	// DMS_SEN_TYPE_POWER_SUPPLY
	static struct lpm_fault_event_desc power_supply_table[] = {
		{
			// one time event
			.event_type = LPM_FAULT_EVENT_POWER_OUT_RANGE,
			.describe   = "Edp warning"
		}, {
			.event_type = LPM_FAULT_EVENT_POWER_INPUT_OUT_RANGE,
			.describe   = "power input out range"
		}
	};
	*event_num = (uint32_t)ARRAY_SIZE(power_supply_table);
	return &power_supply_table[0];
}

STATIC uint32_t lpm_fault_power_supply_assert_mask(uint32_t node_type)
{
#if defined(CFG_SOC_PLATFORM_CLOUD) || defined(CFG_SOC_PLATFORM_CLOUD_V2) || defined(CFG_SOC_PLATFORM_MINIV3)
	if (node_type == (uint32_t)DMS_DEV_TYPE_LPM) {
		return ((uint32_t)0x1 << LPM_FAULT_EVENT_POWER_OUT_RANGE);
	}
#elif defined(CFG_SOC_PLATFORM_MDC_V51)
	if (node_type == (uint32_t)DMS_DEV_TYPE_LPM) {
		return ((uint32_t)0x1 << LPM_FAULT_EVENT_POWER_INPUT_OUT_RANGE);
	}
#endif
	return 0;
}

STATIC uint32_t lpm_fault_power_supply_deassert_mask(uint32_t node_type)
{
#if defined(CFG_SOC_PLATFORM_CLOUD) || defined(CFG_SOC_PLATFORM_CLOUD_V2) || defined(CFG_SOC_PLATFORM_MINIV3)
	// LPM_FAULT_EVENT_POWER_OUT_RANGE is one_time event for lpm
	(void)node_type;
#elif defined(CFG_SOC_PLATFORM_MDC_V51)
	if (node_type == (uint32_t)DMS_DEV_TYPE_LPM) {
		return ((uint32_t)0x1 << LPM_FAULT_EVENT_POWER_INPUT_OUT_RANGE);
	}
#endif
	return 0;
}
#endif

#if defined(CFG_SOC_PLATFORM_CLOUD_V2) || defined(CFG_SOC_PLATFORM_MDC_V51) || defined(CFG_SOC_PLATFORM_MINIV3)
STATIC struct lpm_fault_event_desc *lpm_fault_get_scheduler_sensor_event(uint32_t *event_num)
{
	// DMS_SEN_TYPE_SCHEDULER_SENSOR
	static struct lpm_fault_event_desc scheduler_sensor_table[] = {
		{
			.event_type = LPM_FAULT_EVENT_TSENSOR_EXCEP,
			.describe   = "Tsensor signal timeout"
		}
	};
	*event_num = (uint32_t)ARRAY_SIZE(scheduler_sensor_table);
	return &scheduler_sensor_table[0];
}

STATIC uint32_t lpm_fault_scheduler_sensor_assert_mask(uint32_t node_type)
{
	if ((node_type == (uint32_t)DMS_DEV_TYPE_LPM) || (node_type == (uint32_t)DMS_DEV_TYPE_AO_SUBSYS)) {
		return ((uint32_t)0x1 << LPM_FAULT_EVENT_TSENSOR_EXCEP);
	}
	return 0;
}

STATIC uint32_t lpm_fault_scheduler_sensor_deassert_mask(uint32_t node_type)
{
	if ((node_type == (uint32_t)DMS_DEV_TYPE_LPM) || (node_type == (uint32_t)DMS_DEV_TYPE_AO_SUBSYS)) {
		return ((uint32_t)0x1 << LPM_FAULT_EVENT_TSENSOR_EXCEP);
	}
	return 0;
}
#endif

#if defined(CFG_SOC_PLATFORM_CLOUD_V2) || defined(CFG_SOC_PLATFORM_MINIV3)
STATIC struct lpm_fault_event_desc *lpm_fault_get_chip_hardware_event(uint32_t *event_num)
{
	// DMS_SEN_TYPE_CHIP_HARDWARE
	static struct lpm_fault_event_desc chip_hardware_table[] = {
		{
			.event_type = LPM_FAULT_EVENT_DVFS_VOLT_AICORE_EXCEP,
			.describe   = "Volt adjust error"
		}, {
			.event_type = LPM_FAULT_EVENT_CURRENT_GET_EXCEP,
			.describe   = "Get current error"
		}, {
			.event_type = LPM_FAULT_EVENT_DVFS_FREQ_AICORE_EXCEP,
			.describe   = "Freq adjust error"
		}
	};
	*event_num = (uint32_t)ARRAY_SIZE(chip_hardware_table);
	return &chip_hardware_table[0];
}

STATIC uint32_t lpm_fault_chip_hardware_assert_mask(uint32_t node_type)
{
	if (node_type == (uint32_t)DMS_DEV_TYPE_LPM) {
#if defined(CFG_SOC_PLATFORM_CLOUD_V2)
		return (((uint32_t)0x1 << LPM_FAULT_EVENT_DVFS_VOLT_AICORE_EXCEP) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_CURRENT_GET_EXCEP));
#elif defined(CFG_SOC_PLATFORM_MINIV3)
		return (((uint32_t)0x1 << LPM_FAULT_EVENT_DVFS_VOLT_AICORE_EXCEP) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_CURRENT_GET_EXCEP) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_DVFS_FREQ_AICORE_EXCEP));
#endif
	}
	return 0;
}

STATIC uint32_t lpm_fault_chip_hardware_deassert_mask(uint32_t node_type)
{
	if (node_type == (uint32_t)DMS_DEV_TYPE_LPM) {
#if defined(CFG_SOC_PLATFORM_CLOUD_V2)
		return (((uint32_t)0x1 << LPM_FAULT_EVENT_DVFS_VOLT_AICORE_EXCEP) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_CURRENT_GET_EXCEP));
#elif defined(CFG_SOC_PLATFORM_MINIV3)
		return (((uint32_t)0x1 << LPM_FAULT_EVENT_DVFS_VOLT_AICORE_EXCEP) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_CURRENT_GET_EXCEP) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_DVFS_FREQ_AICORE_EXCEP));
#endif
	}
	return 0;
}
#endif

#if defined(CFG_SOC_PLATFORM_CLOUD) || defined(CFG_SOC_PLATFORM_MDC_V51) || defined(CFG_SOC_PLATFORM_MINIV3)
STATIC struct lpm_fault_event_desc *lpm_fault_get_ras_sensor_event(uint32_t *event_num)
{
	// DMS_SEN_TYPE_RAS_SENSOR
	static struct lpm_fault_event_desc ras_sensor_table[] = {
		{
			.event_type = LPM_FAULT_EVENT_IN_CFG_ERR,
			.describe   = "Pll cfg error"
		}, {
			.event_type = LPM_FAULT_EVENT_PARITY_ERR,
			.describe   = "parity error"
		}, {
			.event_type = LPM_FAULT_EVENT_MBECC,
			.describe   = "multiple bit ecc"
		}, {
			.event_type = LPM_FAULT_EVENT_BUS_ERR,
			.describe   = "Spmi bus error"
		}, {
			.event_type = LPM_FAULT_EVENT_SBECCOVERTHOLD,
			.describe   = "LPM SRAM SBECC"
		}, {
			.event_type = LPM_FAULT_EVENT_ERROR,
			.describe   = "LPM MODULE ERROR"
		}, {
			.event_type = LPM_FAULT_EVENT_ERROR_CANNOT_FIXED,
			.describe   = "module err cannot be fixed"
		}, {
			.event_type = LPM_FAULT_EVENT_TIMEOUT_ERR,
			.describe   = "service timeout"
		}, {
			.event_type = LPM_FAULT_EVENT_INPUT_ERROR,
			.describe   = "input error"
		}, {
			.event_type = LPM_FAULT_EVENT_CONFIG_ERR,
			.describe   = "config error"
		}
	};
	*event_num = (uint32_t)ARRAY_SIZE(ras_sensor_table);
	return &ras_sensor_table[0];
}

STATIC uint32_t lpm_fault_ras_sensor_assert_mask(uint32_t node_type)
{
	switch (node_type) {
#if defined(CFG_SOC_PLATFORM_CLOUD)
	case DMS_DEV_TYPE_LPM:
		return (((uint32_t)0x1 << LPM_FAULT_EVENT_BUS_ERR) |
		((uint32_t)0x1 << LPM_FAULT_EVENT_MBECC));
#elif defined(CFG_SOC_PLATFORM_MDC_V51)
	case DMS_DEV_TYPE_LPM:
		return (((uint32_t)0x1 << LPM_FAULT_EVENT_ERROR_CANNOT_FIXED) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_IN_CFG_ERR) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_BUS_ERR) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_MBECC) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_SBECCOVERTHOLD) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_PARITY_ERR) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_ERROR));
	case DMS_DEV_TYPE_AO_SUBSYS:
		return (((uint32_t)0x1 << LPM_FAULT_EVENT_PARITY_ERR) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_BUS_ERR));
	case DMS_DEV_TYPE_AOSUB_DISP:
		return (((uint32_t)0x1 << LPM_FAULT_EVENT_PARITY_ERR) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_TIMEOUT_ERR));
	case DMS_DEV_TYPE_AOSUB_AA:
		return ((uint32_t)0x1 << LPM_FAULT_EVENT_PARITY_ERR);
	case DMS_DEV_TYPE_AOSUB_SCHE:
		return ((uint32_t)0x1 << LPM_FAULT_EVENT_PARITY_ERR);
	case DMS_DEV_TYPE_AOSUB_SMMU:
		return (((uint32_t)0x1 << LPM_FAULT_EVENT_PARITY_ERR) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_MBECC) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_TIMEOUT_ERR));
#elif defined(CFG_SOC_PLATFORM_MINIV3)
	case DMS_DEV_TYPE_LPM:
#if defined(CFG_SOC_PLATFORM_MDC_V11)
		return (((uint32_t)0x1 << LPM_FAULT_EVENT_IN_CFG_ERR) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_ERROR_CANNOT_FIXED));
#else
		return ((uint32_t)0x1 << LPM_FAULT_EVENT_IN_CFG_ERR);
#endif
	case DMS_DEV_TYPE_AOSUB_DISP:
		return (((uint32_t)0x1 << LPM_FAULT_EVENT_INPUT_ERROR) |
			(((uint32_t)0x1 << LPM_FAULT_EVENT_MBECC)) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_CONFIG_ERR) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_SBECCOVERTHOLD));
#if defined(CFG_SOC_PLATFORM_MDC_V11)
	case DMS_DEV_TYPE_AOSUB_AA:
		return ((uint32_t)0x1 << LPM_FAULT_EVENT_CONFIG_ERR);
	case DMS_DEV_TYPE_AOSUB_SMMU:
		return (((uint32_t)0x1 << LPM_FAULT_EVENT_CONFIG_ERR) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_SBECCOVERTHOLD) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_MBECC));
#endif
#endif
	default:
		return 0;
	}
}

STATIC uint32_t lpm_fault_ras_sensor_deassert_mask(uint32_t node_type)
{
	switch (node_type) {
#if defined(CFG_SOC_PLATFORM_CLOUD)
	case DMS_DEV_TYPE_LPM:
		return (((uint32_t)0x1 << LPM_FAULT_EVENT_BUS_ERR) |
		((uint32_t)0x1 << LPM_FAULT_EVENT_MBECC));
#elif defined(CFG_SOC_PLATFORM_MDC_V51)
	case DMS_DEV_TYPE_LPM:
		return (((uint32_t)0x1 << LPM_FAULT_EVENT_ERROR_CANNOT_FIXED) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_IN_CFG_ERR) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_BUS_ERR) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_MBECC) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_SBECCOVERTHOLD) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_PARITY_ERR) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_ERROR));
	case DMS_DEV_TYPE_AO_SUBSYS:
		return (((uint32_t)0x1 << LPM_FAULT_EVENT_PARITY_ERR) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_BUS_ERR));
	case DMS_DEV_TYPE_AOSUB_DISP:
		return (((uint32_t)0x1 << LPM_FAULT_EVENT_PARITY_ERR) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_TIMEOUT_ERR));
	case DMS_DEV_TYPE_AOSUB_AA:
		return ((uint32_t)0x1 << LPM_FAULT_EVENT_PARITY_ERR);
	case DMS_DEV_TYPE_AOSUB_SCHE:
		return ((uint32_t)0x1 << LPM_FAULT_EVENT_PARITY_ERR);
	case DMS_DEV_TYPE_AOSUB_SMMU:
		return (((uint32_t)0x1 << LPM_FAULT_EVENT_PARITY_ERR) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_MBECC) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_TIMEOUT_ERR));
#elif defined(CFG_SOC_PLATFORM_MINIV3)
	case DMS_DEV_TYPE_LPM:
#if defined(CFG_SOC_PLATFORM_MDC_V11)
		return (((uint32_t)0x1 << LPM_FAULT_EVENT_IN_CFG_ERR) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_ERROR_CANNOT_FIXED));
#else
		return ((uint32_t)0x1 << LPM_FAULT_EVENT_IN_CFG_ERR);
#endif
	case DMS_DEV_TYPE_AOSUB_DISP:
		return (((uint32_t)0x1 << LPM_FAULT_EVENT_INPUT_ERROR) |
			(((uint32_t)0x1 << LPM_FAULT_EVENT_MBECC)) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_CONFIG_ERR));
#if defined(CFG_SOC_PLATFORM_MDC_V11)
	case DMS_DEV_TYPE_AOSUB_AA:
		return ((uint32_t)0x1 << LPM_FAULT_EVENT_CONFIG_ERR);
	case DMS_DEV_TYPE_AOSUB_SMMU:
		return (((uint32_t)0x1 << LPM_FAULT_EVENT_CONFIG_ERR) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_SBECCOVERTHOLD) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_MBECC));
#endif
#endif
	default:
		return 0;
	}
}
#endif

#if defined(CFG_SOC_PLATFORM_MDC_V51) || defined(CFG_SOC_PLATFORM_MINIV3)
STATIC struct lpm_fault_event_desc *lpm_fault_get_power_unit_event(uint32_t *event_num)
{
	// DMS_SEN_TYPE_POWER_UNIT
	static struct lpm_fault_event_desc power_unit_table[] = {
		{
			.event_type = LPM_FAULT_EVENT_PMU_HIGH,
			.describe   = "Pmu high temperature",
		}, {
			.event_type = LPM_FAULT_EVENT_PMU_SENSOR_BAD,
			.describe   = "Pmu Temperature value abnormal"
		}, {
			.event_type = LPM_FAULT_EVENT_PMU_REPORT_TIMEOUT,
			.describe   = "Pmu report timeout"
		}, {
			.event_type = LPM_FAULT_EVENT_PMU_CONFIG_ERR,
			.describe   = "Pmu cfg error"
		}, {
			.event_type = LPM_FAULT_EVENT_PMU_CONNECT_ERR,
			.describe   = "Pmu connect error"
		}
	};
	*event_num = (uint32_t)ARRAY_SIZE(power_unit_table);
	return &power_unit_table[0];
}

STATIC uint32_t lpm_fault_power_unit_assert_mask(uint32_t node_type)
{
	if (node_type == (uint32_t)DMS_DEV_TYPE_LPM) {
#if defined(CFG_SOC_PLATFORM_MINIV3)
		return ((uint32_t)0x1 << LPM_FAULT_EVENT_PMU_CONNECT_ERR);
#else
		return (((uint32_t)0x1 << LPM_FAULT_EVENT_PMU_HIGH) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_PMU_SENSOR_BAD) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_PMU_REPORT_TIMEOUT) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_PMU_CONFIG_ERR));
#endif
	}
#if defined(CFG_SOC_PLATFORM_MDC_V51)
	if ((node_type == (uint32_t)DMS_DEV_TYPE_VR) || (node_type == (uint32_t)DMS_DEV_TYPE_PMU)) {
# else
	if (node_type == (uint32_t)DMS_DEV_TYPE_VR) {
#endif
		return (((uint32_t)0x1 << LPM_FAULT_EVENT_PMU_CONFIG_ERR) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_PMU_CONNECT_ERR));
	}
	return 0;
}

STATIC uint32_t lpm_fault_power_unit_deassert_mask(uint32_t node_type)
{
	if (node_type == (uint32_t)DMS_DEV_TYPE_LPM) {
#if defined(CFG_SOC_PLATFORM_MINIV3)
		return ((uint32_t)0x1 << LPM_FAULT_EVENT_PMU_CONNECT_ERR);
#else
		return (((uint32_t)0x1 << LPM_FAULT_EVENT_PMU_HIGH) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_PMU_SENSOR_BAD) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_PMU_REPORT_TIMEOUT) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_PMU_CONFIG_ERR));
#endif
	}
#if defined(CFG_SOC_PLATFORM_MDC_V51)
	if ((node_type == (uint32_t)DMS_DEV_TYPE_VR) || (node_type == (uint32_t)DMS_DEV_TYPE_PMU)) {
# else
	if (node_type == (uint32_t)DMS_DEV_TYPE_VR) {
#endif
		return (((uint32_t)0x1 << LPM_FAULT_EVENT_PMU_CONFIG_ERR) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_PMU_CONNECT_ERR));
	}
	return 0;
}

STATIC struct lpm_fault_event_desc *lpm_fault_get_current_event(uint32_t *event_num)
{
	// LPM_FAULT_EVENT_CURRENT_CRITICAL_HIGH
	static struct lpm_fault_event_desc current_table[] = {
		{
			.event_type = LPM_FAULT_EVENT_CURRENT_CRITICAL_HIGH,
			.describe   = "current go non-critical high"
		}
	};
	*event_num = (uint32_t)ARRAY_SIZE(current_table);
	return &current_table[0];
}

STATIC uint32_t lpm_fault_current_assert_mask(uint32_t node_type)
{
#if defined (CFG_SOC_PLATFORM_MDC_V51)
	if ((node_type == (uint32_t)DMS_DEV_TYPE_VR) || (node_type == (uint32_t)DMS_DEV_TYPE_PMU)) {
# else
	if (node_type == (uint32_t)DMS_DEV_TYPE_VR) {
#endif
		return ((uint32_t)0x1 << LPM_FAULT_EVENT_CURRENT_CRITICAL_HIGH);
	}

	return 0;
}

STATIC uint32_t lpm_fault_current_deassert_mask(uint32_t node_type)
{
#if defined (CFG_SOC_PLATFORM_MDC_V51)
	if ((node_type == (uint32_t)DMS_DEV_TYPE_VR) || (node_type == (uint32_t)DMS_DEV_TYPE_PMU)) {
# else
	if (node_type == (uint32_t)DMS_DEV_TYPE_VR) {
#endif
		return ((uint32_t)0x1 << LPM_FAULT_EVENT_CURRENT_CRITICAL_HIGH);
	}
	return 0;
}

STATIC struct lpm_fault_event_desc *lpm_fault_get_soc_extend_event(uint32_t *event_num)
{
	// DMS_SEN_TYPE_EXTEND_SENSOR
	static struct lpm_fault_event_desc soc_extend_table[] = {
		{
			.event_type = LPM_FAULT_EVENT_MONITOR_TIMEOUT,
			.describe   = "Soc monitor temperature timeout"
		}, {
			.event_type = LPM_FAULT_EVENT_REPORT_TIMEOUT,
			.describe   = "Soc temperature report timeout"
		}, {
			.event_type = LPM_FAULT_EVENT_PMU_IRQ_TEST,
			.describe   = "Pmu irq test error"
		}, {
			.event_type = LPM_FAULT_EVENT_SAFETY_REG_CFG_ERR,
			.describe   = "register safety cfg error"
		}, {
			.event_type = LPM_FAULT_EVENT_COMMON_REG_CFG_ERR,
			.describe   = "register cfg error"
		}
	};
	*event_num = (uint32_t)ARRAY_SIZE(soc_extend_table);
	return &soc_extend_table[0];
}

STATIC uint32_t lpm_fault_soc_extend_assert_mask(uint32_t node_type)
{
#if defined(CFG_SOC_PLATFORM_MDC_V51)
	if (node_type == (uint32_t)DMS_DEV_TYPE_LPM) {
		return (((uint32_t)0x1 << LPM_FAULT_EVENT_MONITOR_TIMEOUT) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_REPORT_TIMEOUT) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_SAFETY_REG_CFG_ERR) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_COMMON_REG_CFG_ERR));
	}
	if (node_type == (uint32_t)DMS_DEV_TYPE_PMU) {
		return (uint32_t)0x1 << LPM_FAULT_EVENT_PMU_IRQ_TEST;
	}
#else
	if (node_type == (uint32_t)DMS_DEV_TYPE_LPM) {
		return ((uint32_t)0x1 << LPM_FAULT_EVENT_SAFETY_REG_CFG_ERR);
	}
#endif
	return 0;
}

STATIC uint32_t lpm_fault_soc_extend_deassert_mask(uint32_t node_type)
{
#if defined(CFG_SOC_PLATFORM_MDC_V51)
	if (node_type == (uint32_t)DMS_DEV_TYPE_LPM) {
		return (((uint32_t)0x1 << LPM_FAULT_EVENT_MONITOR_TIMEOUT) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_REPORT_TIMEOUT) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_SAFETY_REG_CFG_ERR) |
			((uint32_t)0x1 << LPM_FAULT_EVENT_COMMON_REG_CFG_ERR));
	}
	if (node_type == (uint32_t)DMS_DEV_TYPE_PMU) {
		return (uint32_t)0x1 << LPM_FAULT_EVENT_PMU_IRQ_TEST;
	}
#else
	if (node_type == (uint32_t)DMS_DEV_TYPE_LPM) {
		return ((uint32_t)0x1 << LPM_FAULT_EVENT_SAFETY_REG_CFG_ERR);
	}
#endif
	return 0;
}
#endif

#if defined(CFG_SOC_PLATFORM_MDC_V51)
STATIC struct lpm_fault_event_desc *lpm_fault_get_safety_event(uint32_t *event_num)
{
	// DMS_SEN_TYPE_SAFETY_SENSOR
	static struct lpm_fault_event_desc safety_table[] = {
		{
			.event_type = LPM_FAULT_EVENT_LOCKSTEP_ERR,
			.describe   = "lockstep error"
		}, {
			.event_type = LPM_FAULT_EVENT_UN_ACCESS_NOTIF,
			.describe   = "unauthorized access failure"
		}
	};
	*event_num = (uint32_t)ARRAY_SIZE(safety_table);
	return &safety_table[0];
}

STATIC uint32_t lpm_fault_safety_assert_mask(uint32_t node_type)
{
	switch (node_type) {
	case DMS_DEV_TYPE_LPM:
		return ((uint32_t)0x1 << LPM_FAULT_EVENT_LOCKSTEP_ERR);
	case DMS_DEV_TYPE_AO_SUBSYS:
		return (((uint32_t)0x1 << LPM_FAULT_EVENT_LOCKSTEP_ERR) |
		((uint32_t)0x1 << LPM_FAULT_EVENT_UN_ACCESS_NOTIF));
	case DMS_DEV_TYPE_AOSUB_DISP:
		return (((uint32_t)0x1 << LPM_FAULT_EVENT_LOCKSTEP_ERR) |
		((uint32_t)0x1 << LPM_FAULT_EVENT_UN_ACCESS_NOTIF));
	case DMS_DEV_TYPE_AOSUB_AA:
		return (((uint32_t)0x1 << LPM_FAULT_EVENT_LOCKSTEP_ERR) |
		((uint32_t)0x1 << LPM_FAULT_EVENT_UN_ACCESS_NOTIF));
	case DMS_DEV_TYPE_AOSUB_SCHE:
		return ((uint32_t)0x1 << LPM_FAULT_EVENT_LOCKSTEP_ERR);
	case DMS_DEV_TYPE_AOSUB_SMMU:
		return ((uint32_t)0x1 << LPM_FAULT_EVENT_LOCKSTEP_ERR);
	default:
		return 0;
	}
}

STATIC uint32_t lpm_fault_safety_deassert_mask(uint32_t node_type)
{
	switch (node_type) {
	case DMS_DEV_TYPE_LPM:
		return ((uint32_t)0x1 << LPM_FAULT_EVENT_LOCKSTEP_ERR);
	case DMS_DEV_TYPE_AO_SUBSYS:
		return ((uint32_t)0x1 << LPM_FAULT_EVENT_LOCKSTEP_ERR);
	case DMS_DEV_TYPE_AOSUB_DISP:
		return ((uint32_t)0x1 << LPM_FAULT_EVENT_LOCKSTEP_ERR);
	case DMS_DEV_TYPE_AOSUB_AA:
		return ((uint32_t)0x1 << LPM_FAULT_EVENT_LOCKSTEP_ERR) |
		((uint32_t)0x1 << LPM_FAULT_EVENT_UN_ACCESS_NOTIF);
	case DMS_DEV_TYPE_AOSUB_SCHE:
		return ((uint32_t)0x1 << LPM_FAULT_EVENT_LOCKSTEP_ERR);
	case DMS_DEV_TYPE_AOSUB_SMMU:
		return ((uint32_t)0x1 << LPM_FAULT_EVENT_LOCKSTEP_ERR);
	default:
		return 0;
	}
}

STATIC struct lpm_fault_event_desc *lpm_fault_get_crypto_event(uint32_t *event_num)
{
	// DMS_SEN_TYPE_CRYPTO_SENSOR
	static struct lpm_fault_event_desc crypto_table[] = {
		{
			.event_type = LPM_FAULT_EVENT_CRYPTO_EFUSE_CANNOT_BURNT,
			.describe   = "efuse cannot be burnt"
		}, {
			.event_type = LPM_FAULT_EVENT_SYSTEM_SECURITY_MACHANISM_FAIL,
			.describe   = "sys security mechanism failure"
		}, {
			.event_type = LPM_FAULT_EVENT_CRYPTO_MODULE_FAIL,
			.describe   = "crypto algorithm module failure"
		}
	};
	*event_num = (uint32_t)ARRAY_SIZE(crypto_table);
	return &crypto_table[0];
}

STATIC struct lpm_fault_event_desc *lpm_fault_get_check_sensor_event(uint32_t *event_num)
{
	// DMS_SEN_TYPE_MODULE_SENSOR2
	static struct lpm_fault_event_desc module_sensor2_table[] = {
		{
			.event_type = LPM_FAULT_EVENT_CRC_CHECK_FAIL,
			.describe   = "CRC check fail"
		}
	};

	*event_num = (uint32_t)ARRAY_SIZE(module_sensor2_table);
	return &module_sensor2_table[0];
}

STATIC uint32_t lpm_fault_crypto_assert_mask(uint32_t node_type)
{
	if (node_type == (uint32_t)DMS_DEV_TYPE_PMU) {
		return ((uint32_t)0x1 << LPM_FAULT_EVENT_CRYPTO_EFUSE_CANNOT_BURNT);
	} else if (node_type == (uint32_t)DMS_DEV_TYPE_LPM) {
		return (((uint32_t)0x1 << LPM_FAULT_EVENT_SYSTEM_SECURITY_MACHANISM_FAIL) |
		((uint32_t)0x1 << LPM_FAULT_EVENT_CRYPTO_MODULE_FAIL));
	}
	return 0;
}

STATIC uint32_t lpm_fault_crypto_deassert_mask(uint32_t node_type)
{
	if (node_type == (uint32_t)DMS_DEV_TYPE_PMU) {
		return ((uint32_t)0x1 << LPM_FAULT_EVENT_CRYPTO_EFUSE_CANNOT_BURNT);
	} else if (node_type == (uint32_t)DMS_DEV_TYPE_LPM) {
		return (((uint32_t)0x1 << LPM_FAULT_EVENT_SYSTEM_SECURITY_MACHANISM_FAIL) |
		((uint32_t)0x1 << LPM_FAULT_EVENT_CRYPTO_MODULE_FAIL));
	}
	return 0;
}

STATIC uint32_t lpm_fault_check_sensor_assert_mask(uint32_t node_type)
{
	if ((node_type == (uint32_t)DMS_DEV_TYPE_AOSUB_DISP) || (node_type == (uint32_t)DMS_DEV_TYPE_AOSUB_AA)) {
		return ((uint32_t)0x1 << LPM_FAULT_EVENT_CRC_CHECK_FAIL);
	}

	return 0;
}

STATIC uint32_t lpm_fault_check_sensor_deassert_mask(uint32_t node_type)
{
	if ((node_type == (uint32_t)DMS_DEV_TYPE_AOSUB_DISP) || (node_type == (uint32_t)DMS_DEV_TYPE_AOSUB_AA)) {
		return ((uint32_t)0x1 << LPM_FAULT_EVENT_CRC_CHECK_FAIL);
	}

	return 0;
}

#endif

#if defined(CFG_SOC_PLATFORM_CLOUD) || defined(CFG_SOC_PLATFORM_MDC_V51)
STATIC struct lpm_fault_event_desc *lpm_fault_get_fw_progress_event(uint32_t *event_num)
{
	// DMS_SEN_TYPE_SYSTEM_FW_PROGRESS
	static struct lpm_fault_event_desc fw_progress_table[] = {
		{
			.event_type = LPM_FAULT_EVENT_FW_HANG,
			.describe   = "LPM startup error"
		}, {
			.event_type = LPM_FAULT_EVENT_FW_SOC_ERROR,
			.describe   = "LP FW Err, next SOC boot fail"
		}
	};
	*event_num = (uint32_t)ARRAY_SIZE(fw_progress_table);
	return &fw_progress_table[0];
}

STATIC uint32_t lpm_fault_fw_progress_assert_mask(uint32_t node_type)
{
#if defined(CFG_SOC_PLATFORM_CLOUD)
	if (node_type == (uint32_t)DMS_DEV_TYPE_LPM) {
		return ((uint32_t)0x1 << LPM_FAULT_EVENT_FW_HANG);
	}
#elif defined(CFG_SOC_PLATFORM_MDC_V51)
	if (node_type == (uint32_t)DMS_DEV_TYPE_PMU) {
		return ((uint32_t)0x1 << LPM_FAULT_EVENT_FW_SOC_ERROR);
	}
	if (node_type == (uint32_t)DMS_DEV_TYPE_LPM) {
		return ((uint32_t)0x1 << LPM_FAULT_EVENT_FW_HANG);
	}
#endif

	return 0;
}

STATIC uint32_t lpm_fault_fw_progress_deassert_mask(uint32_t node_type)
{
#if defined(CFG_SOC_PLATFORM_CLOUD)
	if (node_type == (uint32_t)DMS_DEV_TYPE_LPM) {
		return ((uint32_t)0x1 << LPM_FAULT_EVENT_FW_HANG);
	}
#elif defined(CFG_SOC_PLATFORM_MDC_V51)
	if (node_type == (uint32_t)DMS_DEV_TYPE_PMU) {
		return ((uint32_t)0x1 << LPM_FAULT_EVENT_FW_SOC_ERROR);
	}
	if (node_type == (uint32_t)DMS_DEV_TYPE_LPM) {
		return ((uint32_t)0x1 << LPM_FAULT_EVENT_FW_HANG);
	}
#endif
	return 0;
}
#endif

STATIC int32_t lpm_dms_node_ops_init(struct dms_node *node)
{
	if (node != NULL) {
		lpm_log_info("lpm node init: node_name=%s\n", node->node_name);
	}
	return 0;
}

STATIC void lpm_dms_node_ops_uninit(struct dms_node *node)
{
	if (node != NULL) {
		lpm_log_info("lpm node uninit: node_name=%s\n", node->node_name);
	}
}

STATIC struct lpm_fault_node_obj_register *lpm_fault_config_get_node_list(uint32_t *node_type_num)
{
	static struct lpm_fault_node_obj_register node_obj_list[LPM_FAULT_NODE_TYPE_NUM] = {
		{
			"LPM", (uint32_t)DMS_DEV_TYPE_LPM, 0, LPM_FAULT_LPM_NODE_NUM, LPM_FAULT_LPM_SENSOR_NUM,
			lpm_dms_node_ops_init, lpm_dms_node_ops_uninit
#if defined(CFG_SOC_PLATFORM_MDC_V51)
		}, {
			"PMU", (uint32_t)DMS_DEV_TYPE_PMU, 0, LPM_FAULT_PMU_NODE_NUM, LPM_FAULT_PMU_SENSOR_NUM,
			lpm_dms_node_ops_init, lpm_dms_node_ops_uninit
		}, {
			"VR", (uint32_t)DMS_DEV_TYPE_VR, 0, LPM_FAULT_VR_NODE_NUM, LPM_FAULT_VR_SENSOR_NUM,
			lpm_dms_node_ops_init, lpm_dms_node_ops_uninit
		}, {
			"AO subsys", (uint32_t)DMS_DEV_TYPE_AO_SUBSYS, 0, LPM_FAULT_AO_NODE_NUM, LPM_FAULT_AO_SENSOR_NUM,
			lpm_dms_node_ops_init, lpm_dms_node_ops_uninit
		}, {
			"AO sub disp", (uint32_t)DMS_DEV_TYPE_AOSUB_DISP, (uint32_t)DMS_DEV_TYPE_AO_SUBSYS, LPM_FAULT_DISP_NODE_NUM,
			LPM_FAULT_DISP_SENSOR_NUM, lpm_dms_node_ops_init, lpm_dms_node_ops_uninit
		}, {
			"AO sub aa", (uint32_t)DMS_DEV_TYPE_AOSUB_AA, (uint32_t)DMS_DEV_TYPE_AO_SUBSYS, LPM_FAULT_AA_NODE_NUM,
			LPM_FAULT_AA_SENSOR_NUM, lpm_dms_node_ops_init, lpm_dms_node_ops_uninit
		}, {
			"AO sub sche", (uint32_t)DMS_DEV_TYPE_AOSUB_SCHE, (uint32_t)DMS_DEV_TYPE_AO_SUBSYS, LPM_FAULT_SCHE_NODE_NUM,
			LPM_FAULT_SCHE_SENSOR_NUM, lpm_dms_node_ops_init, lpm_dms_node_ops_uninit
		}, {
			"AO sub smmu", (uint32_t)DMS_DEV_TYPE_AOSUB_SMMU, (uint32_t)DMS_DEV_TYPE_AO_SUBSYS, LPM_FAULT_SMMU_NODE_NUM,
			LPM_FAULT_SMMU_SENSOR_NUM, lpm_dms_node_ops_init, lpm_dms_node_ops_uninit
		}, {
			"PG sensor", (uint32_t)DMS_DEV_TYPE_POWER_GLITCH_SENSOR, 0, LPM_FAULT_PG_SENSOR_NODE_NUM,
			LPM_FAULT_PG_SENSOR_NUM, lpm_dms_node_ops_init, lpm_dms_node_ops_uninit
#elif defined(CFG_SOC_PLATFORM_MINIV3)
		}, {
			"AO subsys", (uint32_t)DMS_DEV_TYPE_AO_SUBSYS, 0, LPM_FAULT_AO_NODE_NUM, LPM_FAULT_AO_SENSOR_NUM,
			lpm_dms_node_ops_init, lpm_dms_node_ops_uninit
		}, {
			"AO sub disp", (uint32_t)DMS_DEV_TYPE_AOSUB_DISP, (uint32_t)DMS_DEV_TYPE_AO_SUBSYS, LPM_FAULT_DISP_NODE_NUM,
			LPM_FAULT_DISP_SENSOR_NUM, lpm_dms_node_ops_init, lpm_dms_node_ops_uninit
		}, {
			"VR", (uint32_t)DMS_DEV_TYPE_VR, 0, LPM_FAULT_VR_NODE_NUM, LPM_FAULT_VR_SENSOR_NUM,
			lpm_dms_node_ops_init, lpm_dms_node_ops_uninit
#if defined(CFG_SOC_PLATFORM_MDC_V11)
		}, {
			"AO sub aa", (uint32_t)DMS_DEV_TYPE_AOSUB_AA, (uint32_t)DMS_DEV_TYPE_AO_SUBSYS, LPM_FAULT_AA_NODE_NUM,
			LPM_FAULT_AA_SENSOR_NUM, lpm_dms_node_ops_init, lpm_dms_node_ops_uninit
		}, {
			"AO sub smmu", (uint32_t)DMS_DEV_TYPE_AOSUB_SMMU, (uint32_t)DMS_DEV_TYPE_AO_SUBSYS, LPM_FAULT_SMMU_NODE_NUM,
			LPM_FAULT_SMMU_SENSOR_NUM, lpm_dms_node_ops_init, lpm_dms_node_ops_uninit
#endif
#endif
		}
	};

	*node_type_num = (uint32_t)ARRAY_SIZE(node_obj_list);
	return &node_obj_list[0];
}

STATIC struct lpm_fault_sensor_obj_register *lpm_fault_config_get_sensor_list(uint32_t *sensor_type_num)
{
	static struct lpm_fault_sensor_obj_register sensor_obj_list[LPM_FAULT_SENSOR_TYPE_NUM] = {
		{
			// all chip support
			"LP HEARTBEAT ERROR", DMS_SEN_TYPE_HEARTBEAT, 0x1, lpm_fault_heartbeat_assert_mask,
			lpm_fault_heartbeat_deassert_mask, lpm_fault_get_heartbeat_event
#if defined(CFG_SOC_PLATFORM_CLOUD)
		}, {
			"Soc Temperature", DMS_SEN_TYPE_TEMPERATURE, 0x1, lpm_fault_temperature_assert_mask,
			lpm_fault_temperature_deassert_mask, lpm_fault_get_temperature_event
		}, {
			"Soc Event", DMS_SEN_TYPE_RAS_SENSOR, 0x1, lpm_fault_ras_sensor_assert_mask,
			lpm_fault_ras_sensor_deassert_mask, lpm_fault_get_ras_sensor_event
		}, {
			"LP Start ERROR", DMS_SEN_TYPE_SYSTEM_FW_PROGRESS, 0x1, lpm_fault_fw_progress_assert_mask,
			lpm_fault_fw_progress_deassert_mask, lpm_fault_get_fw_progress_event
		}, {
			"Edp Event", DMS_SEN_TYPE_POWER_SUPPLY, 0x1, lpm_fault_power_supply_assert_mask,
			lpm_fault_power_supply_deassert_mask, lpm_fault_get_power_supply_event
#elif defined(CFG_SOC_PLATFORM_MDC_V51)
		}, {
			"Soc Temperature", DMS_SEN_TYPE_TEMPERATURE, 0x7, lpm_fault_temperature_assert_mask,
			lpm_fault_temperature_deassert_mask, lpm_fault_get_temperature_event
		}, {
			"Soc Event", DMS_SEN_TYPE_RAS_SENSOR, 0xF9, lpm_fault_ras_sensor_assert_mask,
			lpm_fault_ras_sensor_deassert_mask, lpm_fault_get_ras_sensor_event
		}, {
			"Soc Extend", DMS_SEN_TYPE_EXTEND_SENSOR, 0x3, lpm_fault_soc_extend_assert_mask,
			lpm_fault_soc_extend_deassert_mask, lpm_fault_get_soc_extend_event
		}, {
			"Pmu Unit", DMS_SEN_TYPE_POWER_UNIT, 0x7, lpm_fault_power_unit_assert_mask,
			lpm_fault_power_unit_deassert_mask, lpm_fault_get_power_unit_event
		}, {
			"Soc Safety", DMS_SEN_TYPE_SAFETY_SENSOR, 0xF9, lpm_fault_safety_assert_mask,
			lpm_fault_safety_deassert_mask, lpm_fault_get_safety_event
		}, {
			"LP Start ERROR", DMS_SEN_TYPE_SYSTEM_FW_PROGRESS, 0x3, lpm_fault_fw_progress_assert_mask,
			lpm_fault_fw_progress_deassert_mask, lpm_fault_get_fw_progress_event
		}, {
			"Soc Current", DMS_SEN_TYPE_CURRENT, 0x6, lpm_fault_current_assert_mask,
			lpm_fault_current_deassert_mask, lpm_fault_get_current_event
		}, {
			"crypto", DMS_SEN_TYPE_CRYPTO_SENSOR, 0x3, lpm_fault_crypto_assert_mask,
			lpm_fault_crypto_deassert_mask, lpm_fault_get_crypto_event
		}, {
			"Power Supply", DMS_SEN_TYPE_POWER_SUPPLY, 0x1, lpm_fault_power_supply_assert_mask,
			lpm_fault_power_supply_deassert_mask, lpm_fault_get_power_supply_event
		}, {
			"Soc Event4", DMS_SEN_TYPE_CHECK_SENSOR, 0x30, lpm_fault_check_sensor_assert_mask,
			lpm_fault_check_sensor_deassert_mask, lpm_fault_get_check_sensor_event
		}, {
			"Soc Tsensor", DMS_SEN_TYPE_MODULE_SENSOR2, 0x8, lpm_fault_scheduler_sensor_assert_mask,
			lpm_fault_scheduler_sensor_deassert_mask, lpm_fault_get_scheduler_sensor_event
		}, {
			"Voltage", DMS_SEN_TYPE_VOLTAGE, 0x100, lpm_fault_voltage_assert_mask,
			lpm_fault_voltage_deassert_mask, lpm_fault_get_voltage_event
#elif defined(CFG_SOC_PLATFORM_CLOUD_V2)
		}, {
			"Soc Temperature", DMS_SEN_TYPE_TEMPERATURE, 0x1, lpm_fault_temperature_assert_mask,
			lpm_fault_temperature_deassert_mask, lpm_fault_get_temperature_event
		}, {
			"Soc Tsensor", DMS_SEN_TYPE_MODULE_SENSOR2, 0x1, lpm_fault_scheduler_sensor_assert_mask,
			lpm_fault_scheduler_sensor_deassert_mask, lpm_fault_get_scheduler_sensor_event
		}, {
			"Chip Hardware", DMS_SEN_TYPE_CHIP_HARDWARE, 0x1, lpm_fault_chip_hardware_assert_mask,
			lpm_fault_chip_hardware_deassert_mask, lpm_fault_get_chip_hardware_event
		}, {
			"Power Supply", DMS_SEN_TYPE_POWER_SUPPLY, 0x1, lpm_fault_power_supply_assert_mask,
			lpm_fault_power_supply_deassert_mask, lpm_fault_get_power_supply_event
#elif defined(CFG_SOC_PLATFORM_MINIV3)
		}, {
			"Soc Temperature", DMS_SEN_TYPE_TEMPERATURE, 0x9, lpm_fault_temperature_assert_mask,
			lpm_fault_temperature_deassert_mask, lpm_fault_get_temperature_event
		}, {
			"Power Supply", DMS_SEN_TYPE_POWER_SUPPLY, 0x1, lpm_fault_power_supply_assert_mask,
			lpm_fault_power_supply_deassert_mask, lpm_fault_get_power_supply_event
		}, {
			"Soc Tsensor", DMS_SEN_TYPE_MODULE_SENSOR2, 0x1, lpm_fault_scheduler_sensor_assert_mask,
			lpm_fault_scheduler_sensor_deassert_mask, lpm_fault_get_scheduler_sensor_event
		}, {
			"Chip Hardware", DMS_SEN_TYPE_CHIP_HARDWARE, 0x1, lpm_fault_chip_hardware_assert_mask,
			lpm_fault_chip_hardware_deassert_mask, lpm_fault_get_chip_hardware_event
		}, {
			"Pmu Unit", DMS_SEN_TYPE_POWER_UNIT, 0x9, lpm_fault_power_unit_assert_mask,
			lpm_fault_power_unit_deassert_mask, lpm_fault_get_power_unit_event
#if !defined(CFG_SOC_PLATFORM_MDC_V11)
		}, {
			"Soc Event", DMS_SEN_TYPE_RAS_SENSOR, 0x7, lpm_fault_ras_sensor_assert_mask,
			lpm_fault_ras_sensor_deassert_mask, lpm_fault_get_ras_sensor_event
#else
		}, {
			"Soc Event", DMS_SEN_TYPE_RAS_SENSOR, 0x37, lpm_fault_ras_sensor_assert_mask,
			lpm_fault_ras_sensor_deassert_mask, lpm_fault_get_ras_sensor_event
#endif
		}, {
			"Soc Current", DMS_SEN_TYPE_CURRENT, 0x8, lpm_fault_current_assert_mask,
			lpm_fault_current_deassert_mask, lpm_fault_get_current_event
		}, {
			"Soc Extend", DMS_SEN_TYPE_EXTEND_SENSOR, 0x1, lpm_fault_soc_extend_assert_mask,
			lpm_fault_soc_extend_deassert_mask, lpm_fault_get_soc_extend_event
#endif
		}
	};

	*sensor_type_num = (uint32_t)ARRAY_SIZE(sensor_obj_list);
	return &sensor_obj_list[0];
}

STATIC int32_t lpm_fault_config_init_node(void)
{
	int32_t ret;
	uint32_t node_type_id;
	uint32_t node_type_num = 0;
	struct lpm_fault_node_obj_register *node_obj_list = lpm_fault_config_get_node_list(&node_type_num);

	if (node_type_num != LPM_FAULT_NODE_TYPE_NUM) {
		lpm_log_err("lpm config register node failed, node_type_num=%u should be %u\n",
			node_type_num, LPM_FAULT_NODE_TYPE_NUM);
		return -1;
	}

	for (node_type_id = 0; node_type_id < node_type_num; node_type_id++) {
		ret = lpm_fault_register_node_table(&node_obj_list[node_type_id]);
		if (ret != 0) {
			lpm_log_err("lpm config register node failed, node_name=%s\n",
				node_obj_list[node_type_id].node_name);
			return ret;
		}
	}
	return 0;
}

STATIC int32_t lpm_fault_config_init_sensor(void)
{
	int32_t ret;
	uint32_t sensor_type_id;
	uint32_t sensor_type_num = 0;
	struct lpm_fault_sensor_obj_register *sensor_obj_list = lpm_fault_config_get_sensor_list(&sensor_type_num);

	if (sensor_type_num != LPM_FAULT_SENSOR_TYPE_NUM) {
		lpm_log_err("lpm config register sensor failed, sensor_type_num=%u should be %u\n",
			sensor_type_num, LPM_FAULT_SENSOR_TYPE_NUM);
		return -1;
	}

	for (sensor_type_id = 0; sensor_type_id < sensor_type_num; sensor_type_id++) {
		ret = lpm_fault_register_sensor_table(&sensor_obj_list[sensor_type_id]);
		if (ret != 0) {
			lpm_log_err("lpm config register sensor failed, sensor_name=%s\n",
				sensor_obj_list[sensor_type_id].sensor_name);
			return ret;
		}
	}

	return 0;
}

int32_t lpm_fault_config_exit(uint32_t dev_num)
{
	lpm_log_info("lpm fault config exit success, dev_num=%u\n", dev_num);
	return 0;
}

int32_t lpm_fault_config_init(uint32_t dev_num)
{
	int32_t ret;

	// node should init before sensor
	ret = lpm_fault_config_init_node();
	if (ret != 0) {
		lpm_log_err("lpm config register node failed\n");
		return ret;
	}
	ret = lpm_fault_config_init_sensor();
	if (ret != 0) {
		lpm_log_err("lpm config register sensor failed\n");
		return ret;
	}
	lpm_log_info("lpm fault config init success, dev_num=%u\n", dev_num);
	return 0;
}
