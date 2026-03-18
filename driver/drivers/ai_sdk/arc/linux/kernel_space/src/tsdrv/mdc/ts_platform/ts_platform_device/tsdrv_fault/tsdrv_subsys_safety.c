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

#include <linux/sizes.h>
#include <linux/slab.h>

#include "tsdrv_fault_init.h"
#include "tsdrv_subsys_safety.h"
#include "tsdrv_log.h"
#include "dms_node_type.h"
#include "dms_sensor_type.h"
#include "fpdc_ras_receiver.h"
#include "drvfault_user_common.h"
#include "tsdrv_safety_fault.h"
#include "dfm_dev_register.h"
#include "dfm_report.h"
#include "dfm_safety_report.h"

/**
 * Raw secure interrupt register status,
 * Each error status corresponds to one bit of the register.
 */
enum subsys_safety_err_irq_status {
    TS_SUBCTRL_PARITY_ERR = 0,
    TS_SUBCTRL_ASIL_SUBCTRL_PARITY_ERR = 1,
    TS_SUBCTRL_DECODER_ASIL_VIO_APB_M0 = 8,
    TS_SUBCTRL_SUBCTRL_ASIL_VIO = 12,
};

/**
 * safety fault src id
 * every fault has it's own src id
 */
#define SUBSYS_SAFETY_SRC_ID      1
#define SUBSYS_SAFETY_BITID_7    (7U)
#define SUBSYS_SAFETY_BITID_23   (23U)

/**
 * subsys base addr
 */
#define TS_SUBSYS0_BASE  (0xB80C0000)

/**
 * int_safety_subsys interrupt status offset
 */
#define SUBSYS_SAFETY_INT_SRC      0x2C00
#define SUBSYS_SAFETY_INT_CLR      0x2C00
#define SUBSYS_SAFETY_INT_MASK     0x2C04

 /**
  * int_safety_err_subsys interrupt status offset
  */
#define SUBSYS_SAFETY_ERR_SRC      0x2C00
#define SUBSYS_SAFETY_ERR_CLR      0x2C00
#define SUBSYS_SAFETY_ERR_MASK     0x2C08

#define SUBSYS_SAFETY_EVENT_MAX_NUM  (32U)
#define TS_SUBSYS_NODE_NUM           (1U)
#define TO_STRING(id) #id
#define SUBSYS_SAFETY_SENSOR_MAX_NUM (2U)
/************************************************************************/
/* subsys SAFETY FAULT                                                  */
/************************************************************************/
#define TS_SUBCTRL_DFX_REGS_NUM  (8)
static u32 g_ts_subctrl_dfx_regs[TS_SUBCTRL_DFX_REGS_NUM] = {
    0x2068, 0x2100, 0x2C00, 0x2C04, 0x2C08, 0x2C0C, 0x2C14, 0xF200
};

#define SUBSYS_SAFETY_ERR_HWINFO(subsys_id, reg_base, irq_bit) { \
    .emu_id = subsys_id,                         \
    .src_id = SUBSYS_SAFETY_SRC_ID,              \
    .bit_id = irq_bit,                           \
    .base_paddr = reg_base,                      \
    .base_size = SZ_64K,                         \
    .base_vaddr = 0,                             \
    .status = { SUBSYS_SAFETY_ERR_SRC, 0x0 }, \
    .clear = { SUBSYS_SAFETY_ERR_CLR, 0x0 },  \
    .mask = { SUBSYS_SAFETY_ERR_MASK, 0x0 },  \
    .check_mask[0] = { DRVFAULT_FILED_VALID, 0x2c0c, 0xFFFFEEFC }, \
    .dfx_reg_num = TS_SUBCTRL_DFX_REGS_NUM,      \
    .dfx_reg_list = g_ts_subctrl_dfx_regs,       \
}

#define SUBSYS_SAFETY_INT_HWINFO(subsys_id, reg_base, irq_bit) { \
    .emu_id = subsys_id,                         \
    .src_id = SUBSYS_SAFETY_SRC_ID,              \
    .bit_id = irq_bit,                           \
    .base_paddr = reg_base,                      \
    .base_size = SZ_64K,                         \
    .base_vaddr = 0,                             \
    .status = { SUBSYS_SAFETY_INT_SRC, 0x1103 },    \
    .clear = { SUBSYS_SAFETY_INT_CLR, 0x1103 },     \
    .mask = { SUBSYS_SAFETY_INT_MASK, 0x1103 },     \
    .check_mask[0] = { DRVFAULT_FILED_VALID, 0x2c0c, 0xFFFFEEFC }, \
    .dfx_reg_num = TS_SUBCTRL_DFX_REGS_NUM,      \
    .dfx_reg_list = g_ts_subctrl_dfx_regs,       \
}

STATIC struct safety_irq_hw_info g_subsys_safety_hwinfo[] = {
    SUBSYS_SAFETY_INT_HWINFO(TS_SUB_EMU_ID, TS_SUBSYS0_BASE, SUBSYS_SAFETY_BITID_7),
};

STATIC struct safety_irq_info g_subsys_safety_irq_info[] = {
    {
        .dev_id = 0,
        .irq_type = SAFETY_IRQ_TYPE_LPI,
        .irq_name = "int_ras_safety_ts_sub",
        .safety_irq_func = tsdrv_subsys_safety_handler,
        .irq_hwinfo_num = 1,
        .irq_hwinfo_list = g_subsys_safety_hwinfo
    }
};


#define TS_SUBSYS_DMS_NODE_DEFINE(_id, _ops) { \
    .node_type = DMS_DEV_TYPE_TS,              \
    .node_id = _id,                            \
    .node_name = "TS-SUBSYS-" #_id,            \
    .capacity = 0x1,                           \
    .permission = 0x1,                         \
    .owner_devid = 0,                          \
    .ops = _ops                                \
}

static struct dfm_struct ts_subsys_dms;
static struct dms_node g_ts_subsys_dms_nodes[TS_SUBSYS_NODE_NUM] = {
    TS_SUBSYS_DMS_NODE_DEFINE(0, &tsdrv_dev_node_ops),
};

#define TS_SUBSYS_COVERAGE_ITEM(_err_status, _describe, _sensor_type, _err_type) { \
    .subsys_id = DMS_DEV_TYPE_TS, \
    .module_id = 0,               \
    .section_type = RAS_SEC_OEM,  \
    .ras_code.err_status = _err_status, \
    .describe = _describe, \
    .sensor_type = _sensor_type, \
    .error_type = _err_type \
}

const struct ras_fault_converge_item *ts_subsys_safety_converge(u32 section_type, u64 ras_code)
{
    static struct ras_fault_converge_item tssubsys_converage_tab[] = {
        TS_SUBSYS_COVERAGE_ITEM(TS_SUBCTRL_PARITY_ERR,
            "Subctrl parity err", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_PARITY),
        TS_SUBSYS_COVERAGE_ITEM(TS_SUBCTRL_ASIL_SUBCTRL_PARITY_ERR,
            "ASIL parity err", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_PARITY),
        TS_SUBSYS_COVERAGE_ITEM(TS_SUBCTRL_DECODER_ASIL_VIO_APB_M0,
            "Low-level access to high-level", DMS_SEN_TYPE_SAFETY_SENSOR, RAS_ERROR_TYPE_CFG_ERR_MINIOR),
        TS_SUBSYS_COVERAGE_ITEM(TS_SUBCTRL_SUBCTRL_ASIL_VIO,
            "ASIL level unauthorized", DMS_SEN_TYPE_SAFETY_SENSOR, RAS_ERROR_TYPE_CFG_ERR_MINIOR),
    };

    u32 items_num = (u32)ARRAY_SIZE(tssubsys_converage_tab);
    return tsdrv_safety_converge(tssubsys_converage_tab, items_num, section_type, ras_code);
}

STATIC int tsdrv_scan_tssubsys_event(u64 private_data, struct dms_sensor_event_data *data)
{
    int ret;
    u32 sensor_type = get_sensor_node_from_priv(private_data);
    if (data == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("The parameter is invalid, data is NULL.\n");
        return -EINVAL;
#endif
    }
    ret = dfm_scan_events(&ts_subsys_dms, 0, (u8)sensor_type, data);
    return ret;
}

static struct dms_sensor_object_cfg g_tssubsys_sensor_cfg[SUBSYS_SAFETY_SENSOR_MAX_NUM] = {
    /* soc_event */
    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_RAS_SENSOR, "ts_safety", DMS_DISCRETE_SENSOR_CLASS,
        DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL,
        DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, tsdrv_scan_tssubsys_event,
        SENSOR_PRIV_DATA(0, 0, 0, DMS_SEN_TYPE_RAS_SENSOR), 0xFFFF, 0xFFBF),
    /* soc_event1 */
    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_SAFETY_SENSOR, "ts_safety1", DMS_DISCRETE_SENSOR_CLASS,
        DMS_SENSOR_ATTRIB_THRES_NONE, 0, TSDRV_SENSOR_SCAN_INTERVAL,
        DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, tsdrv_scan_tssubsys_event,
        SENSOR_PRIV_DATA(0, 0, 0, DMS_SEN_TYPE_SAFETY_SENSOR), 0xFFFF, 0xFFFF),
};

STATIC int tsdrv_register_subsys_dms_node(u32 dev_id)
{
    u32 i, j;
    ts_subsys_dms.dev_id = dev_id;

    if (dfm_struct_init(&ts_subsys_dms, TS_SUBSYS_NODE_NUM, SUBSYS_SAFETY_SENSOR_MAX_NUM) != 0) {
        TSDRV_PRINT_ERR("init ts subsys's dfm data failed\n");
        goto _fail;
    }

    for (i = 0; i < TS_SUBSYS_NODE_NUM; ++i) {
        ts_subsys_dms.dev_nodes[i].node = &g_ts_subsys_dms_nodes[i];
        ts_subsys_dms.dev_nodes[i].post_proc = NULL;
        ts_subsys_dms.dev_nodes[i].fpdc_notify = NULL;
        ts_subsys_dms.dev_nodes[i].get_converage_node = ts_subsys_safety_converge;
        for (j = 0; j < SUBSYS_SAFETY_SENSOR_MAX_NUM; ++j) {
            ts_subsys_dms.dev_nodes[i].sensors[j].dms_sensor = g_tssubsys_sensor_cfg[j];
        }
    }

    if (dfm_register_nodes(&ts_subsys_dms) != 0) {
        TSDRV_PRINT_ERR("register ts subsys dms node failed\n");
        goto _fail;
    }
    return 0;

_fail:
#ifndef TSDRV_UT
    dfm_struct_final(&ts_subsys_dms, TS_SUBSYS_NODE_NUM, SUBSYS_SAFETY_SENSOR_MAX_NUM);
#endif
    return -EFAULT;
}

int tsdrv_register_subsys_safety_irq(u32 devid)
{
    if (tsdrv_register_subsys_dms_node(devid) != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("register ts subsys device node failed. devid(%d)\n", devid);
        return -EFAULT;
#endif
    }
#ifndef TSDRV_UT
    if (!tsdrv_exist_irq_in_dts(g_subsys_safety_irq_info, TS_SUBSYS_NODE_NUM)) {
        TSDRV_PRINT_ERR("All safety irq can't be found in dts. devid(%u)\n", devid);
        return 0;
    }
#endif
    if (dfm_register_safety_irq_hwinfo(devid, g_subsys_safety_irq_info, 1) != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("register ras safety irq info failed. devid(%d)\n", devid);
        return -EFAULT;
#endif
    }

    return 0;
}

void tsdrv_unregister_subsys_safety_irq(u32 devid)
{
    dfm_unregister_safety_irq_hwinfo(devid, g_subsys_safety_irq_info, 1);
    dfm_unregister_nodes(&ts_subsys_dms);
    dfm_struct_final(&ts_subsys_dms, TS_SUBSYS_NODE_NUM, SUBSYS_SAFETY_SENSOR_MAX_NUM);
}

int tsdrv_subsys_safety_handler(struct safety_fault_info *safety_fault,
    unsigned int *event_num, struct safety_event **event_list)
{
    struct dfm_safety_module module_info;
    u32 i;

    if (tsdrv_chk_safety_param(safety_fault, event_num) != 0) {
        return -EINVAL;
    }

    *event_num = 0;
    *event_list = (struct safety_event *)kmalloc(
        sizeof(struct safety_event) * SUBSYS_SAFETY_EVENT_MAX_NUM, GFP_KERNEL);
    if ((*event_list) == NULL) {
#ifndef TSDRV_UT
        dfm_err("kmalloc safety event list failed\n");
        return -ENOMEM;
#endif
    }

    module_info.dev_id = safety_fault->dev_id;
    module_info.node_type = DMS_DEV_TYPE_TS;
    module_info.max_event = SUBSYS_SAFETY_EVENT_MAX_NUM;

    for (i = 0; i < safety_fault->fault_reg_num; ++i) {
        if (safety_fault->fault_status_list[i].fault_status == 0) {
            continue;
        }
        module_info.node_id = 0;
        module_info.dev_node = &ts_subsys_dms.dev_nodes[0];
        module_info.section_type = RAS_SEC_OEM;
        dfm_safety_handler(&module_info, &safety_fault->fault_status_list[i], event_num, *event_list);
    }
    return 0;
}
