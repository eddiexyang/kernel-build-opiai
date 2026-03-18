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
 *
 * Description: DMS qos related interface
 * Author: huawei
 * Create: 2022-01-15
 */

#ifndef __DMS_MEMORY_INTERFACE_H
#define __DMS_MEMORY_INTERFACE_H

#include "ascend_hal_error.h"

drvError_t dms_memory_get_ddr_bw_util_rate(unsigned int dev_id, unsigned int *rate);
drvError_t dms_memory_get_ecc_statistics(unsigned int dev_id, unsigned int dev_type, unsigned int error_type,
    unsigned int *value, unsigned int len);
drvError_t dms_memory_get_hbm_bw_util_rate(unsigned int dev_id, unsigned int *value);
drvError_t dms_memory_get_ddr_freq(unsigned int dev_id, unsigned int *frequency);
drvError_t dms_memory_get_hbm_temperature(unsigned int dev_id, unsigned int *temperature);

#endif
