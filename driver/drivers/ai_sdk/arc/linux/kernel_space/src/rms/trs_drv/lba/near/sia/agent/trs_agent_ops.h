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
* Create: 2022-7-15
*/
#ifndef TRS_AGENT_OPS_H
#define TRS_AGENT_OPS_H

#include <linux/types.h>

void trs_agent_ops_init(void);
int trs_agent_rcv_handle(u32 devid, void *msg, size_t size);

int trs_agent_ops_dev_instance(u32 devid, u32 tsid);
int trs_agent_ops_dev_uninstance(u32 devid, u32 tsid);


#endif /* TRS_AGENT_OPS_H */
