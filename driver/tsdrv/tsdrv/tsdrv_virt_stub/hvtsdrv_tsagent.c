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

#include "hvtsdrv_tsagent.h"
void hvtsdrv_fill_trans_info_mbox(u32 devid, u32 fid, u32 tsid, u32 disable_thread,
    struct hvtsdrv_trans_mailbox_ctx *trans_mbox)
{
}

int hvtsdrv_trans_info_msg(struct hvtsdrv_trans_mailbox_ctx *trans_mbox, void *info, size_t msg_len)
{
    return 0;
}

#ifndef TSDRV_UT
s32 hvtsdrv_notice_tsagent_vsq_proc(struct tsdrv_id_inst *id_inst, u32 vsqid, enum vsqcq_type type, u32 sqe_num)
{
    return 0;
}
#endif
