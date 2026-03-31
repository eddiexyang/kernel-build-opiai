/*
* Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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

#ifndef TRS_PROC_FS_H
#define TRS_PROC_FS_H

#include "trs_ts_inst.h"

void trs_hw_sq_show(struct trs_core_ts_inst *ts_inst, u32 sqid);
void trs_hw_cq_show(struct trs_core_ts_inst *ts_inst, u32 cqid);
void trs_logic_cq_show(struct trs_core_ts_inst *ts_inst, u32 cqid);

void proc_fs_add_ts_inst(struct trs_core_ts_inst *ts_inst);
void proc_fs_del_ts_inst(struct trs_core_ts_inst *ts_inst);
void proc_fs_add_pid(struct trs_proc_ctx *proc_ctx);
void proc_fs_del_pid(struct trs_proc_ctx *proc_ctx);
void trs_proc_fs_init(void);
void trs_proc_fs_uninit(void);

#endif

