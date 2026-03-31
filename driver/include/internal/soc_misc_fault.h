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

#ifndef SOC_MISC_FAULT_H
#define SOC_MISC_FAULT_H

#include "drvfault_user_common.h"
#include "soc_misc_dms_def.h"
#include "drv_type.h"

#define CHIP_TYPE_NOT_SET        (0x0)
#define CHIP_TYPE_MDC_ASCEND610  (0x1)
#define CHIP_TYPE_MDC_BS9SX1A    (0x2)

const struct ras_fault_converge_item *soc_misc_parse_table_handle(const struct ras_error *error_info,
    const struct ras_fault_converge_item *converge_item, u32 converge_item_num);

const struct ras_fault_converge_item *soc_misc_parse_table(
    const struct ras_error *error_info, SOC_MISC_SENSOR_ID_E sensor_id);

void soc_misc_fault_event_handler(struct ras_error *error_info, int len, SOC_MISC_SENSOR_ID_E sensor_id);
int soc_misc_fault_event_scan(unsigned long long private_data, struct dms_sensor_event_data *data,
    SOC_MISC_SENSOR_ID_E sensor_id);

void soc_misc_fault_event_free(SOC_MISC_EVENT_LIST_T *event_queue);
int soc_misc_clear_all_fault_events(void);

const struct ras_fault_converge_item *soc_safety_converge(
    const struct ras_fault_converge_item *items, u32 items_num, u64 ras_code);
int soc_chk_safety_param(const struct safety_fault_info *safety_fault,
    unsigned int *event_num);

u32 soc_misc_get_chip_type(void);

#endif
