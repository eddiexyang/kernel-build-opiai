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
* Create: 2023-02-07
*/

#ifndef TSDRV_SUBSYS_SAFETY_RAS_H
#define TSDRV_SUBSYS_SAFETY_RAS_H

#include "drvfault_user_common.h"
#include "fpdc.h"

void tsdrv_subsys_ras_fpdc_handler(const struct notify_data *pdata);
const struct ras_fault_converge_item *ts_subsys_safety_ras_converge(
    const struct safety_fault_status *fault_status, u32 section_type, u64 ras_code);
#endif
