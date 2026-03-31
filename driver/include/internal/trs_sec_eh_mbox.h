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
* Create: 2022-11-16
*/

#ifndef TRS_SEC_EH_MBOX_H
#define TRS_SEC_EH_MBOX_H

#include <linux/types.h>

#include "trs_pub_def.h"
#include "trs_mailbox_def.h"

int trs_sec_eh_mb_update(struct trs_id_inst *inst, u16 cmd, void *mb_data);

#endif /* TRS_SEC_EH_SQCQ_H */
