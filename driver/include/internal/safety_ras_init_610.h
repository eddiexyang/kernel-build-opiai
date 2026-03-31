/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
 * Create: 2022-01-20
 */
#ifndef SAFETY_RAS_INIT_610_H
#define SAFETY_RAS_INIT_610_H

#include <linux/module.h>
#include <asm/barrier.h>

#include "dms_sensor.h"
#include "dms_sensor_type.h"
#include "drv_log.h"
#include "dms_define.h"
#include "fpdc.h"

#include "memory_log.h"
#include "safety_ras_node.h"
#include "safety_ras_report_610.h"

#define SENSOR_OFFSET_16BIT  16
#define DEVNODE_OFFSET_32BIT 32

#define MEMORY_OFFSET_8BIT   8
#define MEMORY_OFFSET_16BIT  16
#define MEMORY_OFFSET_32BIT  32
#define MEMORY_MASK_8BIT     0xFFU
#define MEMORY_MASK_16BIT    0xFFFFU
#define MEMORY_MASK_32BIT    0xFFFFFFFFU

#define MEM_SENSOR_IRQ       0

int dms_register_dev_node(struct dms_node *node);
int dms_unregister_dev_node(struct dms_node *node);

void ddr_ops_exit(struct dms_node *device);
int32_t ddr_ops_init(struct dms_node *device);

uint32_t memory_dev_num(uint32_t dev_num);
struct drv_memory_devices *memory_get_devices(void);

int32_t ddr_subctrl_safety_irq_init(void);
int32_t ddr_subctrl_safety_irq_destroy(void);

int32_t memory_register_one_node(struct memory_dev *m_dev);
void memory_unregister_one_node(struct memory_dev *m_dev);

int32_t memory_ipc_notifier_register(void);
int32_t memory_ipc_notifier_unregister(void);
int32_t memory_dms_node_init(void);
int32_t memory_dms_node_destroy(void);
int32_t memory_dev_node_register(void);
int32_t memory_dev_node_uregister(void);
#endif /* SAFETY_RAS_INIT_610_H */