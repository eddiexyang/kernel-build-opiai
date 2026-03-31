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
* Create: 2023-9-18
*/

#ifndef SAFETY_RAS_NODE_H
#define SAFETY_RAS_NODE_H

#include <linux/types.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>

#include "dms_sensor.h"

#if defined (CFG_SOC_PLATFORM_CLOUD) || defined (CFG_SOC_PLATFORM_CLOUD_V2)
#include "fpdc.h"
#endif

#if defined (CFG_SOC_PLATFORM_CLOUD) || defined (CFG_SOC_PLATFORM_CLOUD_V2)
enum sensor_idx {
	S_IDX0 = 0,
	S_IDX1 = 1,
	S_IDX2 = 2,
	S_IDX3 = 3,
	S_IDX4 = 4,
	S_IDX5 = 5,
	S_IDX6 = 6,
	S_IDX7 = 7,
	S_IDX8 = 8,
	S_IDX9 = 9,
	S_IDX10 = 10,
	S_IDX_MAX
};

#define DDR_SENSOR_NUM (S_IDX3 + 1) /* 4: 2 ras type + 2 mem type */
#define HBM_SENSOR_NUM (S_IDX10 + 1) /* 8: 4 ras type + 4 mem type + 2 hbm multi-ecc + 1 temp type */
#define HHA_SENSOR_NUM (S_IDX8 + 1)
#define DHA_SENSOR_NUM (S_IDX0 + 1)

#define DDR_SENSOR_MEM_START_IDX (S_IDX2) /* 2:DMS_SEN_TYPE_MEMORY ddr start index; 0~1:ddrc0 ~ ddrc1 */
#define HBM_SENSOR_MEM_START_IDX (S_IDX4) /* 4:DMS_SEN_TYPE_MEMORY hbm start index; 0~3:hbmc0 ~ hbmc3 */

#define MEMORY_UNMASK_IRQ_WORK_DATA 2

enum memory_dev_node {
	MEMORY_NODE_0 = 0,
	MEMORY_NODE_1 = 1,
	MEMORY_NODE_2 = 2,
	MEMORY_NODE_3 = 3,
	MEMORY_NODE_ID_MAX
};

enum memory_sensor_id {
	MEMORY_SENSOR_HBMC = 0,
	MEMORY_SENSOR_DDRC = 1,
	MEMORY_SENSOR_HHA = 2,
	MEMORY_SENSOR_DHA = 3,
	MEMORY_SENSOR_ID_MAX
};

struct memory_fault_event {
	unsigned char device_id;	  /* device id */
	unsigned char sensor_id;	  /* ddrc/hbmc */
	unsigned char sensor_idx;	  /* ddr0~ddr1/hbm0~hbm3 */
	unsigned char module_id;	  /* bios ras Module ID */
	unsigned char submodule_id;   /* HBM0_HBMC00 */
	unsigned int ras_err_type;	 /* ras int type: IERR... */
	unsigned int ras_err_severity; /* error level */
	unsigned int sec_type;
};
#else /* 310 & 610 */
#ifdef CFG_SOC_PLATFORM_MINIV3
#define IDX_CHN_MAX          6U
#else
#define IDX_CHN_MAX          24U
#endif

#ifdef DRV_MEM_GTEST
#define OTHER_FAULT_NUM      1U    /* 规避vcast报错 VCAST_MAX_HEAP_SIZE */
#else
#define OTHER_FAULT_NUM      3U    /* 1 temperature + 1 safety + 1 extend */
#endif

/* chan_max (ras + mem) + 1 temperature + 1 safety + 1 extend */
#define S_IDX_MAX            ((IDX_CHN_MAX * 2U) + OTHER_FAULT_NUM)

enum memory_dev_node {
	MEMORY_NODE_0 = 0,
#ifdef CFG_FEATURE_SUPPORT_2P
	MEMORY_NODE_1 = 1,
#endif
	MEMORY_NODE_ID_MAX
};

enum memory_sensor_id {
	MEMORY_SENSOR_DDRC = 0,
	MEMORY_SENSOR_ID_MAX
};

struct memory_fault_event {
	uint8_t   device_id;         /* device id */
	uint8_t   module_id;         /* DMS_DEV_TYPE_DDR  */
	uint8_t   sensor_id;         /* MEMORY_SENSOR_DDRC */
	uint8_t   assertion;         /* DMS_EVENT_TYPE_RESUME,OCCUR,ONE_TIME */
	uint8_t   sensor_type;       /* DMS_SEN_TYPE_MEMORY,RAS_SENSOR,TEMPERATURE,SAFETY_SENSOR */
	uint16_t  event_type;        /* fault id */
	uint8_t   detail_reason;     /* 内部使用的错误码 */
#ifdef CFG_SOC_PLATFORM_MINIV3
	uint32_t  error_code;        /* MATA RAS故障错误码，[IERR,SERR] */
#endif
};
#endif

#define DMS_SENSOR_SCAN_INTERVAL 300 /* 300 ms */
#define HBM_ECC_SENSOR_SCAN_INTERVAL 1000 /* 1000 ms */

struct memory_error_list {
	struct memory_fault_event error;
	struct list_head list;
};

struct memory_event {
	uint32_t event_status;
	uint32_t error_num;
	struct memory_error_list error_list; /* event list */
	struct mutex mutex;
};

struct memory_dev {
	uint32_t die_id;
	uint32_t sensor_obj_num;
	struct dms_sensor_object_cfg *sensor_obj_table;                /* sensor table pointer */
	struct memory_event sensor_event_queue[S_IDX_MAX];             /* sensor event queue */
	struct dms_node *dev_node;
#ifdef CFG_SOC_PLATFORM_MINIV3
	struct delayed_work unmask_ce_irq_work;
	struct delayed_work unmask_ue_irq_work;
#endif
#if defined (CFG_SOC_PLATFORM_CLOUD) || defined (CFG_SOC_PLATFORM_CLOUD_V2)
	struct delayed_work unmask_irq_work;
	unsigned int work_data[MEMORY_UNMASK_IRQ_WORK_DATA];
	FAULT_NOTIFY_FUNC notify_func;
#endif
};

struct drv_memory_devices {
	uint32_t dev_num;
	struct memory_dev m_dev[MEMORY_NODE_ID_MAX][MEMORY_SENSOR_ID_MAX];
};

#define memory_node_def(type, name, devid, id, m_ops) { \
	.node_type = (type),      \
	.node_id = (id),          \
	.node_name = (name),      \
	.owner_devid = (devid),   \
	.ops = &(m_ops)           \
}

#define memory_sensor_def(type, name, devid, s_id, s_idx, assert_mask, deassert_mask, scan_inter, func) { \
	.sensor_type = (type),                                           \
	.sensor_name = (name),                                           \
	.sensor_class = DMS_DISCRETE_SENSOR_CLASS,                     \
	.sensor_class_cfg = {                                          \
		.discrete_sensor = {                                       \
			.attribute = DMS_SENSOR_ATTRIB_THRES_NONE,             \
			.debounce_time = 0                                     \
		}                                                          \
	},                                                             \
	.scan_interval = scan_inter,                     \
	.proc_flag = DMS_SENSOR_PROC_ENABLE_FLAG,                      \
	.enable_flag = DMS_SENSOR_ENABLE_FALG,                         \
	.pf_scan_func = (func),                                          \
	.private_data = (((devid) << MEMORY_OFFSET_32BIT) | ((unsigned long)(s_id) << MEMORY_OFFSET_16BIT) | (s_idx)), \
	.assert_event_mask = (assert_mask),                              \
	.deassert_event_mask = (deassert_mask)                           \
}

#endif /* SAFETY_RAS_NODE_H */
