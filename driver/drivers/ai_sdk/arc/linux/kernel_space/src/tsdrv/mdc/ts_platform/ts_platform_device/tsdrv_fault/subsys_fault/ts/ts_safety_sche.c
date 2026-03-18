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
* Create: 2022-9-20
*/
#ifndef TSDRV_UT
#include <linux/errno.h>

#include "subsys_fault_define.h"
#include "subsys_sche_errtype.h"
#include "subsys_safety.h"
#include "dms_node_type.h"
#include "dms_sensor_type.h"
#include "drv_ras_common.h"
#include "drvfault_user_common.h"
#include "ts_safety_sche.h"

#define TS_SCHE_SAFETY_CONVERGE_NODE(bit_id, msg, _sensor_type, ras_err) \
    SCHE_SAFETY_CONVERGE_NODE(HISI_SUBSYS_TSSUB, bit_id, msg, _sensor_type, ras_err)

static struct ras_fault_converge_item g_sche_converge_table[] = {
    TS_SCHE_SAFETY_CONVERGE_NODE(SCHE_INNER_PARITY, "Internal register Parity error", DMS_SEN_TYPE_RAS_SENSOR, 5),
};

int ts_sche_init_safety_cfg(struct subsys_safety_cfg *safety_cfg)
{
    safety_cfg->dump_reg_num = ARRAY_SIZE(g_sche_dump_regs);
    safety_cfg->status_reg_num = ARRAY_SIZE(g_sche_status_regs);
    safety_cfg->dump_regs = g_sche_dump_regs;
    safety_cfg->status_regs = g_sche_status_regs;

    /* set safety int entry data */
    safety_cfg->safety_int_entry.emu_id = TS_SUB_EMU_ID;
    safety_cfg->safety_int_entry.src_id = 1;
    safety_cfg->safety_int_entry.bit_id = 2; /* 2-safety int; 18-safety err */
    safety_cfg->safety_int_entry.bit_mask[0] = 0;

    /* set safety err entry data */
    safety_cfg->safety_err_entry.emu_id = TS_SUB_EMU_ID;
    safety_cfg->safety_err_entry.src_id = 1;
    safety_cfg->safety_err_entry.bit_id = 18; /* 2-safety int; 18-safety err */
    safety_cfg->safety_err_entry.bit_mask[0] = 0x7FF;

    /* set safety converge data */
    safety_cfg->converge_item_num = ARRAY_SIZE(g_sche_converge_table);
    safety_cfg->converge_items = g_sche_converge_table;

    return 0;
}
#else
void ts_sche_init_safety_stub(int devid)
{
    return;
}
#endif
