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
* Create: 2022-9-20
*/

#include <linux/sizes.h>
#include <linux/slab.h>

#include "dms_node_type.h"
#include "dms_sensor_type.h"
#include "drvfault_user_common.h"
#include "dfm_safety_report.h"
#include "dfm_report.h"
#include "soc_misc_init.h"
#include "soc_misc_dms_def.h"
#include "soc_misc_err_info.h"
#include "soc_misc_fault.h"
#include "soc_misc_hac_subsys.h"

#define HAC_SUBSYS_SAFETY_INT_SRC0        0x2C00
#define HAC_SUBSYS_SAFETY_INT_CLR0        0x2C00
#define HAC_SUBSYS_SAFETY_INT_MASK0       0x2C04

#define HAC_SUBSYS_SAFETY_ERR_SRC0        0x2C00
#define HAC_SUBSYS_SAFETY_ERR_CLR0        0x2C00
#define HAC_SUBSYS_SAFETY_ERR_MASK0       0x2C08

#define HAC_SUBSYS_SAFETY_ERR_SRC2        0x2C40
#define HAC_SUBSYS_SAFETY_ERR_CLR2        0x2C40
#define HAC_SUBSYS_SAFETY_ERR_MASK2       0x2C48

enum hac_subsys_safety_err_bit {
    HAC_SUBSYS_SUBCTRL_PARITY_ERR = 0,
    HAC_SUBSYS_ASIL_SUBCTRL_PARITY_ERR = 1,
    HAC_SUBSYS_IO_MUX_PARITY_ERROR = 4,
    HAC_SUBSYS_DECODER_ASIL_VIO_APB_M0 = 8,
    HAC_SUBSYS_DECODER_ASIL_VIO_APB_M1 = 9,
    HAC_SUBSYS_SUBCTRL_ASIL_VIO = 12,
    HAC_SUBSYS_INT_SAFETY_LOCKSTEP_OSTIMER0_SYNCP = 16,
    HAC_SUBSYS_INT_SAFETY_LOCKSTEP_OSTIMER1_SYNCP = 17
};

#define SOC_MISC_SAFETY_EVENT_MAX    (32U)
#define SOC_HACSUBSYS_NODE_NUM      (1U)

/************************************************************************/
/* support TOP RAS fault irq of HAC SUBSYS in BS9SX1A                      */
/************************************************************************/

#define HAC_SUB_DFX_REGS_NUM (35)
static u32 g_hac_sub_dfx_regs[HAC_SUB_DFX_REGS_NUM] = {
    0x2C00, 0x2C04, 0x2C08, 0x2C0C, 0x2C14, 0x2C20, 0x2C24, 0x2C28, 0x2C2C, 0x2C34, 0x2C40, 0x2C44, 0x2C48, \
    0x2C4C, 0x2C54, 0xF200, 0x2100, 0x2104, 0x2108, 0x210C, 0x2110, 0x2114, 0x2118, 0x211C, 0x2120, 0x2124, \
    0x2128, 0x212C, 0x2130, 0x2134, 0x2138, 0x4000, 0x4004, 0x4008, 0x400C
};

#define HAC_SUBSYS_SAFETY_INT_INFO0(reg_base) {       \
    .emu_id = HAC_SUB_EMU_ID,                        \
    .src_id = 1,                                    \
    .bit_id = HAC_SUBSYS_INT_SAFETY_BIT_ID,          \
    .base_paddr = reg_base,                         \
    .base_size = SZ_64K,                            \
    .base_vaddr = 0,                                \
    .status = {HAC_SUBSYS_SAFETY_INT_SRC0, 0x1312},     \
    .clear = {HAC_SUBSYS_SAFETY_INT_CLR0, 0x1312},      \
    .mask = {HAC_SUBSYS_SAFETY_INT_MASK0, 0x1312},  \
    .check_mask[0] = {DRVFAULT_FILED_VALID, 0x2C0C, 0xFFFFECEC}, \
    .dfx_reg_num = HAC_SUB_DFX_REGS_NUM,                               \
    .dfx_reg_list = g_hac_sub_dfx_regs,                           \
}

#define HAC_SUBSYS_SAFETY_ERR_INFO0(reg_base) {       \
    .emu_id = HAC_SUB_EMU_ID,                        \
    .src_id = 1,                                    \
    .bit_id = HAC_SUBSYS_ERR_SAFETY_BIT_ID,          \
    .base_paddr = reg_base,                         \
    .base_size = SZ_64K,                            \
    .base_vaddr = 0,                                \
    .status = {HAC_SUBSYS_SAFETY_ERR_SRC0, 0x01},     \
    .clear = {HAC_SUBSYS_SAFETY_ERR_CLR0, 0x01},      \
    .mask = {HAC_SUBSYS_SAFETY_ERR_MASK0, 0x01},  \
    .check_mask[0] = {DRVFAULT_FILED_VALID, 0x2C0C, 0xFFFFECEC}, \
    .dfx_reg_num = HAC_SUB_DFX_REGS_NUM,                               \
    .dfx_reg_list = g_hac_sub_dfx_regs,                           \
}

#define HAC_SUBSYS_SAFETY_ERR_INFO1(reg_base) {       \
    .emu_id = HAC_SUB_EMU_ID,                        \
    .src_id = 1,                                    \
    .bit_id = HAC_SUBSYS_ERR_SAFETY_BIT_ID,          \
    .base_paddr = reg_base,                         \
    .base_size = SZ_64K,                            \
    .base_vaddr = 0,                                \
    .status = {HAC_SUBSYS_SAFETY_ERR_SRC2, 0x30000},     \
    .clear = {HAC_SUBSYS_SAFETY_ERR_CLR2, 0x30000},      \
    .mask = {HAC_SUBSYS_SAFETY_ERR_MASK2, 0x30000},  \
    .check_mask[0] = {DRVFAULT_FILED_VALID, 0x2C4C, 0xFFFCFFFF}, \
    .dfx_reg_num = HAC_SUB_DFX_REGS_NUM,                               \
    .dfx_reg_list = g_hac_sub_dfx_regs,                           \
}

STATIC struct safety_irq_hw_info g_hac_subsys_safety_hwinfo[] = {
    HAC_SUBSYS_SAFETY_INT_INFO0(HAC_SUBSYS_REG_BASE),
    HAC_SUBSYS_SAFETY_ERR_INFO0(HAC_SUBSYS_REG_BASE),
    HAC_SUBSYS_SAFETY_ERR_INFO1(HAC_SUBSYS_REG_BASE),
};

struct safety_irq_info g_safety_hac_subsys[] = {
    {
        .dev_id = 0,
        .irq_type = SAFETY_IRQ_TYPE_LPI,
        .irq_name = "int_ras_safety_hac_sub",
        .safety_irq_func = soc_hacsubsys_safety_handler,
        .irq_hwinfo_num = 3,
        .irq_hwinfo_list = g_hac_subsys_safety_hwinfo
    }
};

STATIC int soc_misc_hacsubsys_ops_init(struct dms_node *device)
{
    soc_misc_drv_debug("DEBUG (node_name=%.*s)\n", DMS_MAX_DEV_NAME_LEN, device->node_name);
    return 0;
}

STATIC void soc_misc_hacsubsys_ops_uninit(struct dms_node *device)
{
    soc_misc_drv_debug("DEBUG (node_name=%.*s)\n", DMS_MAX_DEV_NAME_LEN, device->node_name);
    return;
}

STATIC struct dms_node_operations g_soc_misc_hacsubsys_ops = {
    .init = soc_misc_hacsubsys_ops_init,
    .uninit = soc_misc_hacsubsys_ops_uninit,
    .scan = NULL,
    .fault_diag = NULL,
    .event_notify = NULL,
    .get_link_state = NULL,
    .set_link_state = NULL
};

#define SOC_HACSUBSYS_DMS_NODE_DEFINE(_id, _ops) { \
    .node_type = DMS_DEV_TYPE_HAC_SUBSYS,          \
    .node_id = _id,                               \
    .node_name = "SOC-HACSUBSYS-" #_id,            \
    .capacity = 0x1,                              \
    .permission = 0x1,                            \
    .owner_devid = 0,                             \
    .ops = _ops                                   \
}

STATIC struct dfm_struct soc_misc_hacsubsys_dms;
static struct dms_node g_soc_hacsubsys_dms_nodes[SOC_HACSUBSYS_NODE_NUM] = {
    SOC_HACSUBSYS_DMS_NODE_DEFINE(0, &g_soc_misc_hacsubsys_ops),
};

#define HACSUBSYS_SAFETY_COVERAGE_ITEM(_err_status, _describe, _sensor_type, _err_type) { \
    .subsys_id = DMS_DEV_TYPE_HAC_SUBSYS,    \
    .module_id = 0,                         \
    .section_type = SOC_ERR_SECTOR_SAFETY,  \
    .ras_code.err_status = _err_status,     \
    .describe = _describe,                  \
    .sensor_type = _sensor_type,            \
    .error_type = _err_type                 \
}

const struct ras_fault_converge_item *soc_hacsubsys_safety_converge(u32 section_type, u64 ras_code)
{
    static struct ras_fault_converge_item soc_hacsubsys_converage_tab[] = {
        HACSUBSYS_SAFETY_COVERAGE_ITEM(HAC_SUBSYS_SUBCTRL_PARITY_ERR,
            "subctrl_parity_err", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_ERROR),
        HACSUBSYS_SAFETY_COVERAGE_ITEM(HAC_SUBSYS_ASIL_SUBCTRL_PARITY_ERR,
            "asil_subctrl_parity_err", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_PARITY),
        HACSUBSYS_SAFETY_COVERAGE_ITEM(HAC_SUBSYS_IO_MUX_PARITY_ERROR,
            "io_mux_parity_error", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_PARITY),
        HACSUBSYS_SAFETY_COVERAGE_ITEM(HAC_SUBSYS_DECODER_ASIL_VIO_APB_M0,
            "decoder_asil_vio_apb_m0", DMS_SEN_TYPE_SAFETY_SENSOR, RAS_ERROR_TYPE_CFG_ERR_MINIOR),
        HACSUBSYS_SAFETY_COVERAGE_ITEM(HAC_SUBSYS_DECODER_ASIL_VIO_APB_M1,
            "decoder_asil_vio_apb_m1", DMS_SEN_TYPE_SAFETY_SENSOR, RAS_ERROR_TYPE_CFG_ERR_MINIOR),
        HACSUBSYS_SAFETY_COVERAGE_ITEM(HAC_SUBSYS_SUBCTRL_ASIL_VIO,
            "subctrl_asil_vio", DMS_SEN_TYPE_SAFETY_SENSOR, RAS_ERROR_TYPE_CFG_ERR_MINIOR),
        HACSUBSYS_SAFETY_COVERAGE_ITEM(HAC_SUBSYS_INT_SAFETY_LOCKSTEP_OSTIMER0_SYNCP,
            "int_safety_lockstep_ostimer0", DMS_SEN_TYPE_SAFETY_SENSOR, RAS_ERROR_TYPE_ERROR),
        HACSUBSYS_SAFETY_COVERAGE_ITEM(HAC_SUBSYS_INT_SAFETY_LOCKSTEP_OSTIMER1_SYNCP,
            "int_safety_lockstep_ostimer1", DMS_SEN_TYPE_SAFETY_SENSOR, RAS_ERROR_TYPE_ERROR),
    };

    u32 items_num = (u32)ARRAY_SIZE(soc_hacsubsys_converage_tab);
    return soc_safety_converge(soc_hacsubsys_converage_tab, items_num, ras_code);
}

#define DEV_NODE_OFFSET 16
#define DEV_NODE_MASK 0xFFFF
#define SENSOR_NODE_OFFSET 0
#define SENSOR_NODE_MASK 0xFFFF
#define SENSOR_PRIV_DATA(dev_node, sensor_node) \
    (((u64)(dev_node & DEV_NODE_MASK) << DEV_NODE_OFFSET) | \
        ((u64)(sensor_node & SENSOR_NODE_MASK) << SENSOR_NODE_OFFSET))

STATIC int soc_scan_hacsubsys_event(u64 private_data, struct dms_sensor_event_data *data)
{
    u32 node_id;
    u32 sensor_type;

    node_id = ((private_data >> DEV_NODE_OFFSET) & DEV_NODE_MASK);
    sensor_type = ((private_data >> SENSOR_NODE_OFFSET) & SENSOR_NODE_MASK);

    if (node_id >= soc_misc_hacsubsys_dms.node_num) {
        soc_misc_drv_err("invalid node id. (node_id=%u, priv=0x%llx)\n", node_id, private_data);
        return -EINVAL;
    }

    (void)dfm_scan_events(&soc_misc_hacsubsys_dms, node_id, sensor_type, data);
    return 0;
}

STATIC int soc_misc_hacsubsys_register_dms_node(u32 dev_id)
{
    struct dms_sensor_object_cfg sensor_cfg[] = {
        SOC_MISC_SENOR_OBJ(DMS_SEN_TYPE_RAS_SENSOR, "soc_hacsubsys", DMS_DISCRETE_SENSOR_CLASS,
            DMS_SENSOR_ATTRIB_THRES_NONE, 0, DMS_SENSOR_CHECK_INTERVAL_TIME,
            DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, soc_scan_hacsubsys_event,
            SENSOR_PRIV_DATA(0, DMS_SEN_TYPE_RAS_SENSOR), 0xFFFF, 0xFFBF),
        SOC_MISC_SENOR_OBJ(DMS_SEN_TYPE_SAFETY_SENSOR, "soc_hacsubsys", DMS_DISCRETE_SENSOR_CLASS,
            DMS_SENSOR_ATTRIB_THRES_NONE, 0, DMS_SENSOR_CHECK_INTERVAL_TIME,
            DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, soc_scan_hacsubsys_event,
            SENSOR_PRIV_DATA(0, DMS_SEN_TYPE_SAFETY_SENSOR), 0xFFFF, 0xFFFF),
    };
    u32 sensor_num = (u32)ARRAY_SIZE(sensor_cfg);
    u32 i;
    u32 sensor_idx;
    soc_misc_hacsubsys_dms.dev_id = dev_id;

    if (dfm_struct_init(&soc_misc_hacsubsys_dms, SOC_HACSUBSYS_NODE_NUM, sensor_num) != 0) {
        soc_misc_drv_err("init soc misc hacsubsys's dfm data failed\n");
        goto _fail;
    }

    for (i = 0; i < SOC_HACSUBSYS_NODE_NUM; ++i) {
        soc_misc_hacsubsys_dms.dev_nodes[i].node = &g_soc_hacsubsys_dms_nodes[i];
        soc_misc_hacsubsys_dms.dev_nodes[i].post_proc = NULL;
        soc_misc_hacsubsys_dms.dev_nodes[i].fpdc_notify = NULL;
        soc_misc_hacsubsys_dms.dev_nodes[i].get_converage_node = soc_hacsubsys_safety_converge;
        for (sensor_idx = 0; sensor_idx < sensor_num; ++sensor_idx) {
            soc_misc_hacsubsys_dms.dev_nodes[i].sensors[sensor_idx].dms_sensor = sensor_cfg[sensor_idx];
        }
    }

    if (dfm_register_nodes(&soc_misc_hacsubsys_dms) != 0) {
        soc_misc_drv_err("register soc misc hacsubsys dms node failed\n");
        goto _fail;
    }
    return 0;

_fail:
    dfm_struct_final(&soc_misc_hacsubsys_dms, SOC_HACSUBSYS_NODE_NUM, sensor_num);
    return -EFAULT;
}

int soc_hacsubsys_safety_handler(struct safety_fault_info *safety_fault,
    unsigned int *event_num, struct safety_event **event_list)
{
    struct dfm_safety_module module_info;
    u32 i;

    if (soc_chk_safety_param(safety_fault, event_num) != 0) {
        return -EINVAL;
    }

    *event_num = 0;
    *event_list = (struct safety_event *)kmalloc(
        sizeof(struct safety_event) * SOC_MISC_SAFETY_EVENT_MAX, GFP_KERNEL | __GFP_ACCOUNT);
    if ((*event_list) == NULL) {
        dfm_err("kmalloc safety event list failed\n");
        return -ENOMEM;
    }

    module_info.dev_id = safety_fault->dev_id;
    module_info.node_type = DMS_DEV_TYPE_HAC_SUBSYS;
    module_info.max_event = SOC_MISC_SAFETY_EVENT_MAX;

    for (i = 0; i < safety_fault->fault_reg_num; ++i) {
        module_info.node_id = 0;
        module_info.dev_node = &soc_misc_hacsubsys_dms.dev_nodes[0];
        module_info.section_type = SOC_ERR_SECTOR_SAFETY;
        dfm_safety_handler(&module_info, &safety_fault->fault_status_list[i], event_num, *event_list);
    }
    return 0;
}

struct soc_misc_safety_cfg g_hac_subsys_safety_cfg = {
    .dms_node_num = SOC_HACSUBSYS_NODE_NUM,
    .irq_cfg_num = SOC_HACSUBSYS_NODE_NUM,
    .dfm = &soc_misc_hacsubsys_dms,
    .safety_irq_cfgs = g_safety_hac_subsys,
    .register_dms_node = soc_misc_hacsubsys_register_dms_node,
    .post_init_process = NULL,
    .post_uninit_process = NULL,
};

