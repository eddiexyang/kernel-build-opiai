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
 *
 * Description:
 * Author: huawei
 * Create: 2023-05-17
 */

#ifndef SAFETY_RAS_MATA_310_H
#define SAFETY_RAS_MATA_310_H

#include <linux/types.h>

#include "dms_sensor.h"
#include "dms_sensor_type.h"
#include "dms_define.h"

#include "safety_ras_node.h"

#define MATA_S_IDX_MAX    0x1U

#ifndef MATA_UT
#define SCHEDULE_UNMASK_RAS_IRQ_PERIOD    300000    /* schedule after 5 min */
#else
#define SCHEDULE_UNMASK_RAS_IRQ_PERIOD    0
#endif

#define MATA_RAS_IRQ_MASK            0x40U    /* bit 6 */
#define INT_TYP0                     0x0U     /* TYP0 is CE */
#define INT_TYP1                     0x1U     /* TYP1 is UE */
#define INT_TYP0_ENA_OFFSET          0x8U
#define INT_TYP1_ENA_OFFSET          0x10U
#define RAS_MATA_CE_MASK             0xFF00U

#define SINGLE_BIT_ECC_INGRESS       0x0E00U
#define SINGLE_BIT_ECC_SDIR          0x0F00U
#define SINGLE_BIT_ECC_DATA_BUFF     0x1100U
#define SINGLE_BIT_ECC_RDQ_BUFF      0x1400U
#define SINGLE_BIT_ECC_SHARE_BUFF    0x1A00U

extern bool g_mata_ras_ce_type;

int32_t mata_dms_node_init(void);
int32_t mata_dms_node_destroy(void);
int32_t mata_dev_node_register(void);
int32_t mata_dev_node_uregister(void);
int32_t mata_fpdc_notifier_register(void);
int32_t mata_fpdc_notifier_unregister(void);

int32_t mata_ops_init(struct dms_node *device);
void mata_ops_exit(struct dms_node *device);
int32_t mata_fault_event_scan(unsigned long long private_data, struct dms_sensor_event_data *data);

void memory_fault_event_free(struct memory_event *event_queue);
void unmask_mata_irq_process(uint32_t devid, uint32_t sensor_id, uint32_t error_code);
int32_t mata_add_fault_event(struct memory_error_list *error_new, struct memory_event *event_queue);
uint32_t mata_dev_num(uint32_t dev_num);
struct drv_memory_devices *mata_get_devices(void);
int32_t get_mata_event_type_by_error_code(uint32_t error_code, uint16_t *event_type);
struct ras_fault_converge_item *mata_parse_fault_table(struct memory_fault_event *event);
#endif
