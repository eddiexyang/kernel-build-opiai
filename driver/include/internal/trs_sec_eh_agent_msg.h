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
* Create: 2022-11-10
*/

#ifndef TRS_SEC_EH_VPC_H
#define TRS_SEC_EH_VPC_H

#include <linux/types.h>

#include "virtmng_interface.h"

typedef int (*sec_eh_vpc_func)(u32 devid, struct vmng_rx_msg_proc_info *proc_info);

sec_eh_vpc_func trs_sec_eh_get_vpc_func(u32 cmd);

#endif /* TRS_SEC_EH_VPC_H */
