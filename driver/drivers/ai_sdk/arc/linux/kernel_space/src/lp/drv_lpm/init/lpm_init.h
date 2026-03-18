/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2022. All rights reserved.
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
#ifndef LPM_INIT_H
#define LPM_INIT_H

#include <linux/types.h>
#include <linux/securec.h>
#include <linux/spinlock.h>
#include "lpm_fault_report.h"
#include "dms_dev_node.h"
#include "dms_sensor.h"
#include "dms_sensor_type.h"
#include "dms_define.h"
#include "dms_node_type.h"

#define LPM_FAULT_MODULE_NAME_LEN 10

typedef int32_t (*fn_lpm_fault_init)(uint32_t dev_num);
typedef int32_t (*fn_lpm_fault_exit)(uint32_t dev_num);

struct lpm_init_module_table {
	char module_name[LPM_FAULT_MODULE_NAME_LEN];
	fn_lpm_fault_init fn_init;
	fn_lpm_fault_exit fn_exit;
};

#endif /* LPM_INIT_H */
