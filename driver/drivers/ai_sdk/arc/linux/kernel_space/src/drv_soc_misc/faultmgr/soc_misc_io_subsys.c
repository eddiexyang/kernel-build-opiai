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
#include "soc_misc_io_subsys.h"

#define IO_SUBSYS_SAFETY_SRC      0x2C00
#define IO_SUBSYS_SAFETY_CLR      0x2C00
#define IO_SUBSYS_SAFETY_INT_MASK 0x2C04

#define IO_SUBSYS_SAFETY_ERR_SRC      0x2C00
#define IO_SUBSYS_SAFETY_ERR_CLR      0x2C00
#define IO_SUBSYS_SAFETY_ERR_INT_MASK 0x2C08

enum io_subsys_safety_err_bit {
    IO_SUBSYS_SUBCTRL_PARITY_ERR = 0,
    IO_SUBSYS_INT_SAFETY_ERR_PARITY = 1,
    IO_SUBSYS_INT_TSENSOR_TIMEOUT_SYNC = 5,
    IO_SUBSYS_DECODER_ASIL_VIO_APB = 8,
    IO_SUBSYS_DECODER_ASIL_VIO_AHB = 10,
    IO_SUBSYS_INT_SMMU_HIGH_SYNC = 12,
    IO_SUBSYS_DECODER_ASIL_VIO_CFGBUS_DBG = 13,
    IO_SUBSYS_DECODER_ASIL_VIO_CFGBUS = 14,
    IO_SUBSYS_INT_SAFETY_SUBCTRL_ASIL = 15,
    IO_SUBSYS_INT_CHAIN_ERR_SYNC = 16
};

#define SOC_MISC_SAFETY_EVENT_MAX    (32U)
#define SOC_IOSUBSYS_NODE_NUM      (1U)

/************************************************************************/
/* support TOP RAS fault irq of cpu cluster in BS9SX1A                      */
/************************************************************************/

#define IO_SUB_DFX_REGS_NUM (28)
static u32 g_io_sub_dfx_regs[IO_SUB_DFX_REGS_NUM] = {
    0x2C00, 0x2C04, 0x2C08, 0x2C0C, 0x2C14, 0xF200, 0x2558, 0x255C, 0x2560, 0x2564, 0x2568, 0x256C, 0x2570, 0x2574, \
    0x2578, 0x257C, 0x2580, 0x2584, 0x2588, 0x258C, 0x2590, 0x2594, 0x2598, 0x259C, 0x25A0, 0x25A4, 0x25A8, 0x25AC
};

#define IO_SUBSYS_SAFETY_INT_INFO(reg_base) {       \
    .emu_id = IO_SUB_EMU_ID,                        \
    .src_id = 1,                                    \
    .bit_id = IO_SUBSYS_INT_SAFETY_BIT_ID,          \
    .base_paddr = reg_base,                         \
    .base_size = SZ_64K,                            \
    .base_vaddr = 0,                                \
    .status = {IO_SUBSYS_SAFETY_SRC, 0x1F523},      \
    .clear = {IO_SUBSYS_SAFETY_CLR, 0x1F523},       \
    .mask = {IO_SUBSYS_SAFETY_INT_MASK, 0x1F523},   \
    .check_mask[0] = {DRVFAULT_FILED_VALID, 0x2C0C, 0xFFFE0ADC}, \
    .dfx_reg_num = IO_SUB_DFX_REGS_NUM,             \
    .dfx_reg_list = g_io_sub_dfx_regs               \
}

#define IO_SUBSYS_SAFETY_ERR_INFO(reg_base) {       \
    .emu_id = IO_SUB_EMU_ID,                        \
    .src_id = 1,                                    \
    .bit_id = IO_SUBSYS_ERR_SAFETY_BIT_ID,          \
    .base_paddr = reg_base,                         \
    .base_size = SZ_64K,                            \
    .base_vaddr = 0,                                \
    .status = {IO_SUBSYS_SAFETY_ERR_SRC, 0x00},     \
    .clear = {IO_SUBSYS_SAFETY_ERR_CLR, 0x00},      \
    .mask = {IO_SUBSYS_SAFETY_ERR_INT_MASK, 0x00},  \
    .check_mask[0] = {DRVFAULT_FILED_VALID, 0x2C0C, 0xFFFE0ADC}, \
    .dfx_reg_num = IO_SUB_DFX_REGS_NUM,             \
    .dfx_reg_list = g_io_sub_dfx_regs,              \
}

STATIC struct safety_irq_hw_info g_io_subsys_safety_hwinfo[] = {
    IO_SUBSYS_SAFETY_INT_INFO(IO_SUBSYS_REG_BASE),
};

struct safety_irq_info g_safety_io_subsys[] = {
    {
        .dev_id = 0,
        .irq_type = SAFETY_IRQ_TYPE_LPI,
        .irq_name = "int_ras_safety_io_sub",
        .safety_irq_func = soc_iosubsys_safety_handler,
        .irq_hwinfo_num = 1,
        .irq_hwinfo_list = g_io_subsys_safety_hwinfo
    }
};

STATIC int soc_misc_iosubsys_ops_init(struct dms_node *device)
{
    soc_misc_drv_debug("DEBUG (node_name=%.*s)\n", DMS_MAX_DEV_NAME_LEN, device->node_name);
    return 0;
}

STATIC void soc_misc_iosubsys_ops_uninit(struct dms_node *device)
{
    soc_misc_drv_debug("DEBUG (node_name=%.*s)\n", DMS_MAX_DEV_NAME_LEN, device->node_name);
    return;
}

STATIC struct dms_node_operations g_soc_misc_iosubsys_ops = {
    .init = soc_misc_iosubsys_ops_init,
    .uninit = soc_misc_iosubsys_ops_uninit,
    .scan = NULL,
    .fault_diag = NULL,
    .event_notify = NULL,
    .get_link_state = NULL,
    .set_link_state = NULL
};

#define SOC_IOSUBSYS_DMS_NODE_DEFINE(_id, _ops) { \
    .node_type = DMS_DEV_TYPE_IO_SUBSYS,          \
    .node_id = _id,                               \
    .node_name = "SOC-IOSUBSYS-" #_id,            \
    .capacity = 0x1,                              \
    .permission = 0x1,                            \
    .owner_devid = 0,                             \
    .ops = _ops                                   \
}

STATIC struct dfm_struct soc_misc_iosubsys_dms;
static struct dms_node g_soc_iosubsys_dms_nodes[SOC_IOSUBSYS_NODE_NUM] = {
    SOC_IOSUBSYS_DMS_NODE_DEFINE(0, &g_soc_misc_iosubsys_ops),
};

#define IOSUBSYS_SAFETY_COVERAGE_ITEM(_err_status, _describe, _sensor_type, _err_type) { \
    .subsys_id = DMS_DEV_TYPE_IO_SUBSYS,    \
    .module_id = 0,                         \
    .section_type = SOC_ERR_SECTOR_SAFETY,  \
    .ras_code.err_status = _err_status,     \
    .describe = _describe,                  \
    .sensor_type = _sensor_type,            \
    .error_type = _err_type                 \
}

const struct ras_fault_converge_item *soc_iosubsys_safety_converge(u32 section_type, u64 ras_code)
{
    static struct ras_fault_converge_item soc_iosubsys_converage_tab[] = {
        IOSUBSYS_SAFETY_COVERAGE_ITEM(IO_SUBSYS_SUBCTRL_PARITY_ERR,
            "subctrl_parity_err", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_PARITY),
        IOSUBSYS_SAFETY_COVERAGE_ITEM(IO_SUBSYS_INT_SAFETY_ERR_PARITY,
            "int_safety_err_parity_subctrl", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_PARITY),
        IOSUBSYS_SAFETY_COVERAGE_ITEM(IO_SUBSYS_INT_TSENSOR_TIMEOUT_SYNC,
            "int_tsensor_timeout_sync", DMS_SEN_TYPE_MODULE_SENSOR2, RAS_ERROR_TYPE_ERROR),
        IOSUBSYS_SAFETY_COVERAGE_ITEM(IO_SUBSYS_DECODER_ASIL_VIO_APB,
            "decoder_asil_vio_apb_m0", DMS_SEN_TYPE_SAFETY_SENSOR, RAS_ERROR_TYPE_CFG_ERR_MINIOR),
        IOSUBSYS_SAFETY_COVERAGE_ITEM(IO_SUBSYS_DECODER_ASIL_VIO_AHB,
            "decoder_asil_vio_ahb_m0", DMS_SEN_TYPE_SAFETY_SENSOR, RAS_ERROR_TYPE_CFG_ERR_MINIOR),
        IOSUBSYS_SAFETY_COVERAGE_ITEM(IO_SUBSYS_INT_SMMU_HIGH_SYNC,
            "int_smmu_high_sync", DMS_SEN_TYPE_SAFETY_SENSOR, RAS_ERROR_TYPE_CFG_ERR_MINIOR),
        IOSUBSYS_SAFETY_COVERAGE_ITEM(IO_SUBSYS_DECODER_ASIL_VIO_CFGBUS_DBG,
            "decoder_asil_vio_cfgbus_dbg", DMS_SEN_TYPE_SAFETY_SENSOR, RAS_ERROR_TYPE_CFG_ERR_MINIOR),
        IOSUBSYS_SAFETY_COVERAGE_ITEM(IO_SUBSYS_DECODER_ASIL_VIO_CFGBUS,
            "decoder_asil_vio_cfgbus", DMS_SEN_TYPE_SAFETY_SENSOR, RAS_ERROR_TYPE_CFG_ERR_MINIOR),
        IOSUBSYS_SAFETY_COVERAGE_ITEM(IO_SUBSYS_INT_SAFETY_SUBCTRL_ASIL,
            "int_safety_subctrl_asil_vio", DMS_SEN_TYPE_SAFETY_SENSOR, RAS_ERROR_TYPE_CFG_ERR_MINIOR),
        IOSUBSYS_SAFETY_COVERAGE_ITEM(IO_SUBSYS_INT_CHAIN_ERR_SYNC,
            "int_chain_err_sync", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_BUS_ERR),
    };

    u32 items_num = (u32)ARRAY_SIZE(soc_iosubsys_converage_tab);
    return soc_safety_converge(soc_iosubsys_converage_tab, items_num, ras_code);
}

#define DEV_NODE_OFFSET 16
#define DEV_NODE_MASK 0xFFFF
#define SENSOR_NODE_OFFSET 0
#define SENSOR_NODE_MASK 0xFFFF
#define SENSOR_PRIV_DATA(dev_node, sensor_node) \
    (((u64)(dev_node & DEV_NODE_MASK) << DEV_NODE_OFFSET) | \
        ((u64)(sensor_node & SENSOR_NODE_MASK) << SENSOR_NODE_OFFSET))

STATIC int soc_scan_iosubsys_event(u64 private_data, struct dms_sensor_event_data *data)
{
    u32 node_id;
    u32 sensor_type;

    node_id = ((private_data >> DEV_NODE_OFFSET) & DEV_NODE_MASK);
    sensor_type = ((private_data >> SENSOR_NODE_OFFSET) & SENSOR_NODE_MASK);

    if (node_id >= soc_misc_iosubsys_dms.node_num) {
        soc_misc_drv_err("invalid node id. (node_id=%u, priv=0x%llx)\n", node_id, private_data);
        return -EINVAL;
    }

    (void)dfm_scan_events(&soc_misc_iosubsys_dms, node_id, sensor_type, data);
    return 0;
}

STATIC int soc_misc_iosubsys_register_dms_node(u32 dev_id)
{
    struct dms_sensor_object_cfg sensor_cfg[] = {
        SOC_MISC_SENOR_OBJ(DMS_SEN_TYPE_RAS_SENSOR, "soc_iosubsys", DMS_DISCRETE_SENSOR_CLASS,
            DMS_SENSOR_ATTRIB_THRES_NONE, 0, DMS_SENSOR_CHECK_INTERVAL_TIME,
            DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, soc_scan_iosubsys_event,
            SENSOR_PRIV_DATA(0, DMS_SEN_TYPE_RAS_SENSOR), 0xFFFF, 0xFFBF),
        SOC_MISC_SENOR_OBJ(DMS_SEN_TYPE_MODULE_SENSOR2, "soc_iosubsys", DMS_DISCRETE_SENSOR_CLASS,
            DMS_SENSOR_ATTRIB_THRES_NONE, 0, DMS_SENSOR_CHECK_INTERVAL_TIME,
            DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, soc_scan_iosubsys_event,
            SENSOR_PRIV_DATA(0, DMS_SEN_TYPE_MODULE_SENSOR2), 0xFFFF, 0xFFFF),
        SOC_MISC_SENOR_OBJ(DMS_SEN_TYPE_SAFETY_SENSOR, "soc_iosubsys", DMS_DISCRETE_SENSOR_CLASS,
            DMS_SENSOR_ATTRIB_THRES_NONE, 0, DMS_SENSOR_CHECK_INTERVAL_TIME,
            DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, soc_scan_iosubsys_event,
            SENSOR_PRIV_DATA(0, DMS_SEN_TYPE_SAFETY_SENSOR), 0xFFFF, 0xFFFF),
    };
    u32 sensor_num = (u32)ARRAY_SIZE(sensor_cfg);
    u32 i;
    u32 sensor_idx;
    soc_misc_iosubsys_dms.dev_id = dev_id;

    if (dfm_struct_init(&soc_misc_iosubsys_dms, SOC_IOSUBSYS_NODE_NUM, sensor_num) != 0) {
        soc_misc_drv_err("init soc misc iosubsys's dfm data failed\n");
        goto _fail;
    }

    for (i = 0; i < SOC_IOSUBSYS_NODE_NUM; ++i) {
        soc_misc_iosubsys_dms.dev_nodes[i].node = &g_soc_iosubsys_dms_nodes[i];
        soc_misc_iosubsys_dms.dev_nodes[i].post_proc = NULL;
        soc_misc_iosubsys_dms.dev_nodes[i].fpdc_notify = NULL;
        soc_misc_iosubsys_dms.dev_nodes[i].get_converage_node = soc_iosubsys_safety_converge;
        for (sensor_idx = 0; sensor_idx < sensor_num; ++sensor_idx) {
            soc_misc_iosubsys_dms.dev_nodes[i].sensors[sensor_idx].dms_sensor = sensor_cfg[sensor_idx];
        }
    }

    if (dfm_register_nodes(&soc_misc_iosubsys_dms) != 0) {
        soc_misc_drv_err("register soc misc iosubsys dms node failed\n");
        goto _fail;
    }
    return 0;

_fail:
    dfm_struct_final(&soc_misc_iosubsys_dms, SOC_IOSUBSYS_NODE_NUM, sensor_num);
    return -EFAULT;
}

int soc_iosubsys_safety_handler(struct safety_fault_info *safety_fault,
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
    module_info.node_type = DMS_DEV_TYPE_IO_SUBSYS;
    module_info.max_event = SOC_MISC_SAFETY_EVENT_MAX;

    for (i = 0; i < safety_fault->fault_reg_num; ++i) {
        module_info.node_id = 0;
        module_info.dev_node = &soc_misc_iosubsys_dms.dev_nodes[0];
        module_info.section_type = SOC_ERR_SECTOR_SAFETY;
        dfm_safety_handler(&module_info, &safety_fault->fault_status_list[i], event_num, *event_list);
    }
    return 0;
}

struct soc_misc_safety_cfg g_io_subsys_safety_cfg = {
    .dms_node_num = SOC_IOSUBSYS_NODE_NUM,
    .irq_cfg_num = SOC_IOSUBSYS_NODE_NUM,
    .dfm = &soc_misc_iosubsys_dms,
    .safety_irq_cfgs = g_safety_io_subsys,
    .register_dms_node = soc_misc_iosubsys_register_dms_node,
    .post_init_process = NULL,
    .post_uninit_process = NULL,
};
