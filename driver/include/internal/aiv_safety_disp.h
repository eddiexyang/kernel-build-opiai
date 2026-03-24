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

#ifndef AIC_SAFETY_DISP_H
#define AIC_SAFETY_DISP_H

#include "subsys_safety.h"

/*
 * AIC disp safety config
 */
int aiv_disp_init_safety_cfg0(struct subsys_safety_cfg *safety_cfg);
int aiv_disp_init_safety_cfg1(struct subsys_safety_cfg *safety_cfg);
int aiv_disp_init_safety_cfg2(struct subsys_safety_cfg *safety_cfg);
int aiv_disp_init_safety_cfg3(struct subsys_safety_cfg *safety_cfg);
int aiv_disp_init_safety_cfg4(struct subsys_safety_cfg *safety_cfg);
int aiv_disp_init_safety_cfg5(struct subsys_safety_cfg *safety_cfg);
int aiv_disp_init_safety_cfg6(struct subsys_safety_cfg *safety_cfg);
int aiv_disp_init_safety_cfg7(struct subsys_safety_cfg *safety_cfg);

#endif // AIC_SAFETY_DISP_H
