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

#ifndef TRS_PROC_FS_H
#define TRS_PROC_FS_H

#include "chan_init.h"

void chan_proc_fs_add_ts_inst(struct trs_chan_ts_inst *ts_inst);
void chan_proc_fs_del_ts_inst(struct trs_chan_ts_inst *ts_inst);
void chan_proc_fs_add_chan(struct trs_chan *chan);
void chan_proc_fs_del_chan(struct trs_chan *chan);
void chan_proc_fs_init(void);
void chan_proc_fs_uninit(void);

#endif

