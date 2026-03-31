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
* Create: 2022-9-20
*/

#ifndef TS_SAFETY_SMMU_H
#define TS_SAFETY_SMMU_H

#include "subsys_safety.h"

/*
 * TS AA safety config
 */
int ts_smmu_init_safety_cfg(struct subsys_safety_cfg *safety_cfg);

#endif // TS_SAFETY_SMMU_H
