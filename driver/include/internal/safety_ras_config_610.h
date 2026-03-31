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
#ifndef SAFETY_RAS_CONFIG_610_H
#define SAFETY_RAS_CONFIG_610_H

#include <linux/notifier.h>
#ifndef AOS_LLVM_BUILD
#include <acpi/ghes.h>
#include <acpi/apei.h>
#endif
#include "dms_define.h"
#include "dms_node_type.h"

#include "safety_ras_init_610.h"
#include "safety_ras_report_610.h"

#define SCAN_FUNC memory_event_scan

#define R_AST_MASK 0xFFFF       /* DMS_SEN_TYPE_RAS_SENSOR assert_event_mask */
#define R_DST_MASK 0xFFBF       /* DMS_SEN_TYPE_RAS_SENSOR deassert_event_mask */
#define M_AST_MASK 0x1FFF       /* DMS_SEN_TYPE_MEMORY assert_event_mask */
#define M_DST_MASK 0xFDF        /* DMS_SEN_TYPE_MEMORY deassert_event_mask */
#define T_AST_MASK 0xFFF        /* DMS_SEN_TYPE_TEMPERATURE assert_event_mask */
#define T_DST_MASK 0xFFF        /* DMS_SEN_TYPE_TEMPERATURE deassert_event_mask */
#define S_AST_MASK 0xFFF        /* DMS_SEN_TYPE_SAFETY_SENSOR assert_event_mask */
#define S_DST_MASK 0xFFF        /* DMS_SEN_TYPE_SAFETY_SENSOR deassert_event_mask */
#define E_AST_MASK 0xFFF        /* DMS_SEN_TYPE_EXTEND_SENSOR assert_event_mask */
#define E_DST_MASK 0xFFF        /* DMS_SEN_TYPE_EXTEND_SENSOR deassert_event_mask */

#endif /* SAFETY_RAS_CONFIG_610_H */