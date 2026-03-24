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
#ifndef SAFETY_RAS_REPORT_610_H
#define SAFETY_RAS_REPORT_610_H

#include <linux/types.h>
#include "dms_sensor.h"

#include "safety_ras_init_610.h"

int32_t memory_add_fault_event(struct memory_error_list *error_new, uint8_t que_index);
int32_t memory_del_fault_event(struct memory_error_list *error_new, uint8_t que_index);
int32_t memory_event_scan(unsigned long long private_data, struct dms_sensor_event_data *data);


#endif /* SAFETY_RAS_REPORT_610_H */