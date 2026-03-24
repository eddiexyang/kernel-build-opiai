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

#ifndef SOC_MISC_ADAPT_H
#define SOC_MISC_ADAPT_H

#include "soc_misc_chip_info.h"

int soc_misc_get_soc_dieid(dms_soc_die_id_t *soc_dieid);
int soc_misc_get_pmu_dieid(dms_soc_die_id_t *pmu_dieid);

#endif
