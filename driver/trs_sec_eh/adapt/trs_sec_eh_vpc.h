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
* Create: 2022-11-09
*/
#ifndef TRS_SEC_EH_VPC_H
#define TRS_SEC_EH_VPC_H

#include <linux/types.h>

#include "trs_pub_def.h"

int trs_sec_eh_vpc_init(u32 devid);
void trs_sec_eh_vpc_uninit(u32 devid);
int trs_sec_eh_vpc_msg_send(u32 devid, void *msg, size_t size);

#endif