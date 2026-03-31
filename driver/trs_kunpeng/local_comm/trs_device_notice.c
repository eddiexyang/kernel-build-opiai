/*
* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
* Create: 2023-08-24
*/
#include "trs_pm_adapt.h"
#include "trs_msg.h"
static struct trs_adapt_notice_ops ops = {NULL};
struct trs_adapt_notice_ops *trs_adapt_get_notice_ops(void)
{
    return &ops;
}
EXPORT_SYMBOL(trs_adapt_get_notice_ops);

void trs_adapt_notice_ops_register(struct trs_adapt_notice_ops *register_ops)
{
    ops.set_ts_status = register_ops->set_ts_status;
    ops.abnormal_proc = register_ops->abnormal_proc;
    ops.sync_id_proc = register_ops->sync_id_proc;
}
EXPORT_SYMBOL(trs_adapt_notice_ops_register);

void trs_adapt_notice_ops_unregister(void)
{
    ops.set_ts_status = NULL;
    ops.abnormal_proc = NULL;
    ops.sync_id_proc = NULL;
}
EXPORT_SYMBOL(trs_adapt_notice_ops_unregister);
