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
* Create: 2022-10-15
*/
#ifndef TRS_PM_AGENTS_H
#define TRS_PM_AGENTS_H
#include "trs_msg.h"

void trs_pm_agent_init(void);
int trs_pm_agent_get_phy_addr(u32 devid, u32 tsid, struct trs_msg_get_phy_addr *info);

#endif
