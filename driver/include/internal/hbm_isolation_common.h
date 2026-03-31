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
#ifndef HBM_ISOLATION_COMMON_H
#define HBM_ISOLATION_COMMON_H
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/timer.h>
#include <linux/vmalloc.h>
#include <linux/workqueue.h>
#include <linux/kfifo.h>
#include <linux/cper.h>
#include <linux/types.h>
#include "drv_log.h"
#include "drvmem_base_info.h"
#include "dms_sensor.h"

#ifdef STATIC_SKIP
#define STATIC
#else
#define STATIC static
#endif

static inline void hbm_vfree(void **p)
{
	vfree(*p);
	*p = NULL;
}

static inline void hbm_kfree(void **p)
{
	kfree(*p);
	*p = NULL;
}

#define CHECK_PROCESS_DMP				"dmp_daemon"
#define DRV_MEMORY_MODULE_BASIC_INFO	"drv_memory_info"

/* Indicates module where the ECC error occurs */
#define ECC_MODULE_DDRC0			(0x0)
#define ECC_MODULE_DDRC1			(0x1)
#define ECC_MODULE_HBM_MIN			(0x2)
#define ECC_MODULE_HBM_MAX			(0x41)
#define ECC_CONFIG_CRC16			0xFFFF

enum MEMORY_ERROR_SENSOR {
	HBM_MEMORY_SENSOR,
	HBM_MEMORY_ERROR_RECORD_SENSOR,
	HBM_MEMORY_ERROR_SENSOR_MAX
};

#define HBM_MEMORY_ERROR_UE_ECC_TYPE 1

enum HBM_MEMORY_ERROR_RECORD_TYPE {
	HBM_NEW_MULTI_BITS_ECC = 0,
	HBM_NOTICE_ECC_COUNT,
	HBM_UPPER_LIMITS_ECC_COUNT
};

enum ECC_TYPE {
	ECC_TYPE_UNKNOWN = 0,
	ECC_TYPE_SINGLE_BIT,
	ECC_TYPE_MULTI_BIT,
	ECC_TYPE_SCRUB_MULTI_BIT,
};

enum ECC_MODULE_TYPE {
	ECC_MODULE_UNKNOWN = 0,
	ECC_MODULE_DDR,
	ECC_MODULE_HBM,
};

#define OK								0
#define MAX_UNINT16_VALUE				0xFFFF
#define MAX_UNINT32_VALUE				0xFFFFFFFF

// Flash地址分配，结构数据
#define FLASH_ERASE_BLOCK_SIZE			0x10000
#ifdef CFG_HBM_FLASH
#define ECC_FLASH_DIE_SIZE				0x4000
#define HBM_MULTI_INFO_SIZE				0x1020
#define HBM_SINGLE_INFO_SIZE			0x1820
#define HBM_STATISTICS_SIZE				0x20
#define SINGLE_ECC_RECORD_OFFSET		0x1020			  /* single ecc header offset 4128 */
#define SINGLE_ECC_HEADER_RESERVE		24
#define STATISTICS_INFO_SIZE			0x20
#define STATISTICS_INFO_FLASH_OFFSET	0x2840
#else
#define SINGLE_ECC_RECORD_OFFSET		0x78F0
#define SINGLE_ECC_HEADER_RESERVE		16
#define STATISTICS_INFO_SIZE			64
#define STATISTICS_INFO_FLASH_OFFSET	(FLASH_ERASE_BLOCK_SIZE - STATISTICS_INFO_SIZE)
#endif

#define ECC_HEADER_SIGNATURE			0xA5
#define ECC_HEADER_VERSION				0x1
#define ECC_HEADER_VERSION_V2			0x2

#define MAX_FAILED_COUNT				3
#define MAX_UNINT16_VALUE				0xFFFF
#define MAX_REPORTED_ECC_BUFF_SIZE		64

#define NOTICE_MULTI_ECC_COUNT			16
#define ERROR_MULTI_ECC_COUNT			64

#pragma pack(4)
/* ecc_isolated_statistics_record is recorded at the end of flash  */
#ifdef CFG_HBM_FLASH
struct ecc_isolated_statistics_record {
	/* Cumulative number of HBM single bit ECC error reports */
	uint32_t hbm_single_bit_count;
	/* Cumulative number of HBM multiple bit ECC error reports */
	uint32_t hbm_mul_bit_count;
	/* Cumulative number of isolated pages count of HBM single bit  */
	uint32_t hbm_single_isolated_page_count;
	/* Cumulative number of isolated pages count of HBM multiple bit  */
	uint32_t hbm_multi_isolated_page_count;
};
#else
struct ecc_isolated_statistics_record {
	/* Cumulative number of DDRC single bit ECC error reports */
	uint16_t ddr_single_bit_count;
	/* Cumulative number of DDRC multiple bit ECC error reports */
	uint16_t ddr_mul_bit_count;
	/* Cumulative number of HBM single bit ECC error reports */
	uint16_t hbm_single_bit_count;
	/* Cumulative number of HBM multiple bit ECC error reports */
	uint16_t hbm_mul_bit_count;
	/* Cumulative number of isolated pages count of DDRC single bit  */
	uint16_t ddr_single_isolated_page_count;
	/* Cumulative number of isolated pages count of DDRC multiple bit  */
	uint16_t ddr_multi_isolated_page_count;
	/* Cumulative number of isolated pages count of HBM single bit  */
	uint16_t hbm_single_isolated_page_count;
	/* Cumulative number of isolated pages count of HBM multiple bit  */
	uint16_t hbm_multi_isolated_page_count;
};
#endif
struct multi_ecc_err_info_s {
	uint64_t physical_addr;
	uint8_t resv[4]; /* 4 is resv length */
	uint16_t rank;
	uint16_t module_id;
	uint8_t type;
	uint8_t module;
	uint16_t bank;
	uint16_t row;
	uint16_t column;
#ifdef CFG_HBM_FLASH
	uint64_t timestamp;
#endif
};
#pragma pack()

struct new_multi_ecc_list {
	uint64_t physical_addr;
	uint32_t dev_id;
	pid_t occupied_pid;
	uint8_t error_type;
	struct list_head list;
};

struct multi_ecc_event {
	uint32_t error_num;
	struct new_multi_ecc_list ecc_list;
	struct mutex mutex;
};

struct pfn_list {
	unsigned long pfn;
	struct list_head list;
};

struct multi_ecc_pfn_queue {
	struct pfn_list pfns;
	uint32_t pfn_num;
	struct mutex mutex;
};

struct ecc_config_tool_s {
	struct multi_ecc_event multi_ecc_event_queue;
	struct mutex record_lock;
	struct timer_list statistics_timer;
	struct work_struct statistics_task;
	struct work_struct ecc_handle_task;
	DECLARE_KFIFO(reported_multi_ecc_queue, struct multi_ecc_err_info_s, MAX_REPORTED_ECC_BUFF_SIZE);
	uint8_t *ecc_config_data;
	uint8_t *single_ecc_flash_data;
	uint8_t pending[MAX_OS_DEVICE_COUNT];
	struct ecc_isolated_statistics_record ecc_statistics_info[MAX_OS_DEVICE_COUNT];
	struct multi_ecc_pfn_queue un_online_isolated_pfn_queue[MAX_OS_DEVICE_COUNT];
	struct multi_ecc_pfn_queue online_isolated_pfn_queue[MAX_OS_DEVICE_COUNT];
};

#ifdef CFG_HBM_FLASH
int hbm_write_ecc_config_data(uint32_t dev_id);
void hbm_check_statistic_to_bbox(uint8_t *ecc_config_data, uint32_t device_count, uint16_t statistics_info_offset);
#else
int hbm_write_ecc_config_data(void);
void hbm_check_statistic_to_bbox(struct ecc_isolated_statistics_record *statistics_record,
	uint32_t device_count);
#endif
int hbm_add_ecc_event(struct new_multi_ecc_list *new_ecc_event);
int hbm_unisolated_multi_ecc_event_scan(unsigned long long private_data, struct dms_sensor_event_data *data);
int hbm_multi_ecc_fault_event_scan(unsigned long long private_data, struct dms_sensor_event_data *data);
int read_reg32(uint64_t addr, uint32_t *data);
int write_reg32(uint64_t addr, uint32_t data);
struct ecc_config_tool_s *hbm_get_ecc_config_tool(void);
void hbm_add_statistics_event(uint32_t dev_id, uint32_t ecc_recorded_count, uint32_t except_id);
#endif
