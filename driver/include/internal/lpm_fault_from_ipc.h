/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
#ifndef LPM_FAULT_FROM_IPC_H
#define LPM_FAULT_FROM_IPC_H
#include <linux/types.h>

#ifdef LPM_REPORT_FROM_IPC

#if defined(LPM_IPC_V3)
struct lp_event_data {
	uint32_t excep_id; // bbox exception id
	uint8_t timestamp_sec[8];  // 8 bytes for second
	uint8_t timestamp_usec[8]; // 8 bytes for usecond
	uint16_t event_id; // fault event id
} __attribute__((packed));
#endif

// EVENT ID bits
// |31-30-29----------28-27-----25-24-------17-16----------9-8----------0|
// | 2bit|     2bit     |   3bit  |    8bit   |     8bit    |    9bit    |
// | rsv | event_status |   rsv   | node_type | sensor_type | event_type |
// |---------------------------------------------------------------------|
#define EVENT_ID_ASSERTION_BIT  28U
#define EVENT_ID_MODULE_BIT     17U
#define EVENT_ID_SENSOR_BIT     9U

// get event_status
static inline uint32_t lpm_parse_assertion_from_event_id(uint32_t event_id)
{
	return ((event_id >> EVENT_ID_ASSERTION_BIT) & 0x03U);
}

// get node_type
static inline uint32_t lpm_parse_module_from_event_id(uint32_t event_id)
{
	return ((event_id >> EVENT_ID_MODULE_BIT) & 0xFFU);
}

// get sensor_type
static inline uint32_t lpm_parse_sensor_from_event_id(uint32_t event_id)
{
	return ((event_id >> EVENT_ID_SENSOR_BIT) & 0xFFU);
}

// get event_type
static inline uint32_t lpm_parse_errcode_from_event_id(uint32_t event_id)
{
	return (event_id & 0x01FFU);
}

int32_t lpm_fault_from_ipc_init(uint32_t dev_num);
int32_t lpm_fault_from_ipc_exit(uint32_t dev_num);

#else
static inline int32_t lpm_fault_from_ipc_init(uint32_t dev_num)
{
	(void)dev_num;
	return 0;
}
static inline int32_t lpm_fault_from_ipc_exit(uint32_t dev_num)
{
	(void)dev_num;
	return 0;
}
#endif

#endif