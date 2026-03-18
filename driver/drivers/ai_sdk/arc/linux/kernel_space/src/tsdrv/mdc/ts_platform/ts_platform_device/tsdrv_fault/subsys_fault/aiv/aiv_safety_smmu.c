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
#include "subsys_smmu_errtype.h"
#include "subsys_safety.h"
#include "dms_node_type.h"
#include "dms_sensor_type.h"
#include "drv_ras_common.h"
#include "drvfault_user_common.h"
#include "aiv_safety_smmu.h"

#define AIV_SMMU_SAFETY_CONVERGE_NODE(bit_id, msg, _sensor_type, ras_err) \
    SMMU_SAFETY_CONVERGE_NODE(HISI_SUBSYS_AIVECTORSUB, bit_id, msg, _sensor_type, ras_err)

static struct ras_fault_converge_item g_smmu_converge_table[] = {
    AIV_SMMU_SAFETY_CONVERGE_NODE(
        SP_SKYROS_INTERFACE_TIMEOUT, "SP Skyros interface times out", DMS_SEN_TYPE_RAS_SENSOR, 0xA),
    AIV_SMMU_SAFETY_CONVERGE_NODE(
        TP_SKYROS_INTERFACE_TIMEOUT, "TP Skyros interface times out", DMS_SEN_TYPE_RAS_SENSOR, 0xA),
    AIV_SMMU_SAFETY_CONVERGE_NODE(
        SP_AXI_INTERFACE_TIMEOUT, "SP AXI interface timeout error", DMS_SEN_TYPE_RAS_SENSOR, 0xA),
    AIV_SMMU_SAFETY_CONVERGE_NODE(
        TP_AXI_INTERFACE_TIMEOUT, "TP AXI interface timeout error", DMS_SEN_TYPE_RAS_SENSOR, 0xA),
    AIV_SMMU_SAFETY_CONVERGE_NODE(
        REGISTER_PARITY_ERROR, "register parity error", DMS_SEN_TYPE_RAS_SENSOR, 5),
    AIV_SMMU_SAFETY_CONVERGE_NODE(
        MEMORY_ECC_MULTI_BIT_ERROR, "Memory ECC multi-bit error", DMS_SEN_TYPE_RAS_SENSOR, 0x8),
};

STATIC int aiv_smmu_init_safety_cfg(struct subsys_safety_cfg *safety_cfg, int emu_id)
{
    safety_cfg->dump_reg_num = ARRAY_SIZE(g_smmu_dump_regs);
    safety_cfg->status_reg_num = ARRAY_SIZE(g_smmu_status_regs);
    safety_cfg->dump_regs = g_smmu_dump_regs;
    safety_cfg->status_regs = g_smmu_status_regs;

    /* set safety int entry data */
    safety_cfg->safety_int_entry.emu_id = emu_id;
    safety_cfg->safety_int_entry.src_id = 1;
    safety_cfg->safety_int_entry.bit_id = 0; /* safety int: 0; safety err: 16 */
    safety_cfg->safety_int_entry.bit_mask[0] = 0;

    /* set safety int entry data */
    safety_cfg->safety_err_entry.emu_id = emu_id;
    safety_cfg->safety_err_entry.src_id = 1;
    safety_cfg->safety_err_entry.bit_id = 16; /* safety int: 0; safety err: 16 */
    safety_cfg->safety_err_entry.bit_mask[0] = 0x17FFFFF;

    /* set safety converge data */
    safety_cfg->converge_item_num = ARRAY_SIZE(g_smmu_converge_table);
    safety_cfg->converge_items = g_smmu_converge_table;

    return 0;
}

int aiv_smmu_init_safety_cfg0(struct subsys_safety_cfg *safety_cfg)
{
    return aiv_smmu_init_safety_cfg(safety_cfg, AIVECTOR0_EMU_ID);
}
int aiv_smmu_init_safety_cfg1(struct subsys_safety_cfg *safety_cfg)
{
    return aiv_smmu_init_safety_cfg(safety_cfg, AIVECTOR1_EMU_ID);
}
int aiv_smmu_init_safety_cfg2(struct subsys_safety_cfg *safety_cfg)
{
    return aiv_smmu_init_safety_cfg(safety_cfg, AIVECTOR2_EMU_ID);
}
int aiv_smmu_init_safety_cfg3(struct subsys_safety_cfg *safety_cfg)
{
    return aiv_smmu_init_safety_cfg(safety_cfg, AIVECTOR3_EMU_ID);
}
int aiv_smmu_init_safety_cfg4(struct subsys_safety_cfg *safety_cfg)
{
    return aiv_smmu_init_safety_cfg(safety_cfg, AIVECTOR4_EMU_ID);
}
int aiv_smmu_init_safety_cfg5(struct subsys_safety_cfg *safety_cfg)
{
    return aiv_smmu_init_safety_cfg(safety_cfg, AIVECTOR5_EMU_ID);
}
int aiv_smmu_init_safety_cfg6(struct subsys_safety_cfg *safety_cfg)
{
    return aiv_smmu_init_safety_cfg(safety_cfg, AIVECTOR6_EMU_ID);
}
int aiv_smmu_init_safety_cfg7(struct subsys_safety_cfg *safety_cfg)
{
    return aiv_smmu_init_safety_cfg(safety_cfg, AIVECTOR7_EMU_ID);
}
#else
void aiv_smmu_init_safety_stub(int dev_id)
{
    return;
}
#endif
