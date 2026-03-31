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
#include "soc_misc_mbigen_io.h"

#define MBIGEN_IO_SAFETY_INT_SRC        0xC0
#define MBIGEN_IO_SAFETY_INT_CLR        0xC0
#define MBIGEN_IO_SAFETY_INT_MASK       0xD0

#define MBIGEN_IO_SAFETY_ERR_SRC        0xC0
#define MBIGEN_IO_SAFETY_ERR_CLR        0xC0
#define MBIGEN_IO_SAFETY_ERR_MASK       0xD8

enum mbigen_io_safety_err_bit {
    MBIGEN_IO_MASTER_AW_CHK_ERR = 0,
    MBIGEN_IO_PRDATA_CHK_ERR = 3,
    MBIGEN_IO_PREADY_CHK_ERR = 4,
    MBIGEN_IO_PSLVERR_CHK_ERR = 5,
    MBIGEN_IO_SAFETY_CHK_ERR = 6,
    MBIGEN_IO_SAFETY_ERR_CHK_ERR = 7,
    MBIGEN_IO_EVENT_CHK_ERR = 8
};

#define SOC_MISC_SAFETY_EVENT_MAX    (32U)
#define SOC_MBIGEN_IO_NODE_NUM      (1U)

/************************************************************************/
/* support TOP RAS fault irq of MBIGEN IO in BS9SX1A                      */
/************************************************************************/

#define MBIGEN_IO_DFX_REGS_NUM (10)
static u32 g_mbigen_io_dfx_regs[MBIGEN_IO_DFX_REGS_NUM] = {
    0xC0, 0xC4, 0xd0, 0xd4, 0xd8, 0xdc, 0xe0, 0xe4, 0xe8, 0x480
};

#define MBIGEN_IO_SAFETY_INT_INFO(reg_base) {       \
    .emu_id = IO_SUB_EMU_ID,                        \
    .src_id = 1,                                    \
    .bit_id = MBIGEN_IO_INT_SAFETY_BIT_ID,          \
    .base_paddr = reg_base,                         \
    .base_size = SZ_64K,                            \
    .base_vaddr = 0,                                \
    .status = {MBIGEN_IO_SAFETY_INT_SRC, 0x0},      \
    .clear = {MBIGEN_IO_SAFETY_INT_CLR, 0x0},       \
    .mask = {MBIGEN_IO_SAFETY_INT_MASK, 0x0},       \
    .check_mask[0] = {DRVFAULT_FILED_VALID, 0x00EC, 0xFFFFFE06}, \
    .dfx_reg_num = MBIGEN_IO_DFX_REGS_NUM,          \
    .dfx_reg_list = g_mbigen_io_dfx_regs,           \
}

#define MBIGEN_IO_SAFETY_ERR_INFO(reg_base) {       \
    .emu_id = IO_SUB_EMU_ID,                        \
    .src_id = 1,                                    \
    .bit_id = MBIGEN_IO_ERR_SAFETY_BIT_ID,          \
    .base_paddr = reg_base,                         \
    .base_size = SZ_64K,                            \
    .base_vaddr = 0,                                \
    .status = {MBIGEN_IO_SAFETY_ERR_SRC, 0x1F9},    \
    .clear = {MBIGEN_IO_SAFETY_ERR_CLR, 0x1F9},     \
    .mask = {MBIGEN_IO_SAFETY_ERR_MASK, 0x1F9},     \
    .check_mask[0] = {DRVFAULT_FILED_VALID, 0x00EC, 0xFFFFFE06}, \
    .dfx_reg_num = MBIGEN_IO_DFX_REGS_NUM,          \
    .dfx_reg_list = g_mbigen_io_dfx_regs,           \
}

STATIC struct safety_irq_hw_info g_mbigen_io_safety_hwinfo[] = {
    MBIGEN_IO_SAFETY_ERR_INFO(MBIGEN_IO_REG_BASE),
};

struct safety_irq_info g_safety_mbigen_io[] = {
    {
        .dev_id = 0,
        .irq_type = SAFETY_IRQ_TYPE_LPI,
        .irq_name = "int_event_mbigen_io",
        .safety_irq_func = soc_mbigen_io_safety_handler,
        .irq_hwinfo_num = 1,
        .irq_hwinfo_list = g_mbigen_io_safety_hwinfo
    }
};

STATIC int soc_misc_mbigen_io_ops_init(struct dms_node *device)
{
    soc_misc_drv_debug("DEBUG (node_name=%.*s)\n", DMS_MAX_DEV_NAME_LEN, device->node_name);
    return 0;
}

STATIC void soc_misc_mbigen_io_ops_uninit(struct dms_node *device)
{
    soc_misc_drv_debug("DEBUG (node_name=%.*s)\n", DMS_MAX_DEV_NAME_LEN, device->node_name);
    return;
}

STATIC struct dms_node_operations g_soc_misc_mbigen_io_ops = {
    .init = soc_misc_mbigen_io_ops_init,
    .uninit = soc_misc_mbigen_io_ops_uninit,
    .scan = NULL,
    .fault_diag = NULL,
    .event_notify = NULL,
    .get_link_state = NULL,
    .set_link_state = NULL
};

#define SOC_MBIGEN_IO_DMS_NODE_DEFINE(_id, _ops) { \
    .node_type = DMS_DEV_TYPE_IO_SUBSYS,          \
    .node_id = _id,                               \
    .node_name = "SOC-MBIGENIO-" #_id,            \
    .capacity = 0x1,                              \
    .permission = 0x1,                            \
    .owner_devid = 0,                             \
    .ops = _ops                                   \
}

STATIC struct dfm_struct soc_misc_mbigen_io_dms;
static struct dms_node g_soc_mbigen_io_dms_nodes[SOC_MBIGEN_IO_NODE_NUM] = {
    SOC_MBIGEN_IO_DMS_NODE_DEFINE(0, &g_soc_misc_mbigen_io_ops),
};

#define MBIGEN_IO_SAFETY_COVERAGE_ITEM(_err_status, _describe, _sensor_type, _err_type) { \
    .subsys_id = DMS_DEV_TYPE_IO_SUBSYS,    \
    .module_id = 0,                         \
    .section_type = SOC_ERR_SECTOR_SAFETY,  \
    .ras_code.err_status = _err_status,     \
    .describe = _describe,                  \
    .sensor_type = _sensor_type,            \
    .error_type = _err_type                 \
}

const struct ras_fault_converge_item *soc_mbigen_io_safety_converge(u32 section_type, u64 ras_code)
{
    static struct ras_fault_converge_item soc_mbigen_io_converage_tab[] = {
        MBIGEN_IO_SAFETY_COVERAGE_ITEM(MBIGEN_IO_MASTER_AW_CHK_ERR,
            "master_aw_chk", DMS_SEN_TYPE_MODULE_SENSOR2, RAS_ERROR_TYPE_ERROR_NF),
        MBIGEN_IO_SAFETY_COVERAGE_ITEM(MBIGEN_IO_PRDATA_CHK_ERR,
            "prdata_chk", DMS_SEN_TYPE_MODULE_SENSOR2, RAS_ERROR_TYPE_ERROR_NF),
        MBIGEN_IO_SAFETY_COVERAGE_ITEM(MBIGEN_IO_PREADY_CHK_ERR,
            "pready_chk", DMS_SEN_TYPE_MODULE_SENSOR2, RAS_ERROR_TYPE_ERROR_NF),
        MBIGEN_IO_SAFETY_COVERAGE_ITEM(MBIGEN_IO_PSLVERR_CHK_ERR,
            "pslverr_chk", DMS_SEN_TYPE_MODULE_SENSOR2, RAS_ERROR_TYPE_ERROR_NF),
        MBIGEN_IO_SAFETY_COVERAGE_ITEM(MBIGEN_IO_SAFETY_CHK_ERR,
            "safety_chk", DMS_SEN_TYPE_MODULE_SENSOR2, RAS_ERROR_TYPE_ERROR_NF),
        MBIGEN_IO_SAFETY_COVERAGE_ITEM(MBIGEN_IO_SAFETY_ERR_CHK_ERR,
            "safety_err_chk", DMS_SEN_TYPE_MODULE_SENSOR2, RAS_ERROR_TYPE_ERROR_NF),
        MBIGEN_IO_SAFETY_COVERAGE_ITEM(MBIGEN_IO_EVENT_CHK_ERR,
            "event_chk", DMS_SEN_TYPE_MODULE_SENSOR2, RAS_ERROR_TYPE_ERROR_NF),
    };

    u32 items_num = (u32)ARRAY_SIZE(soc_mbigen_io_converage_tab);
    return soc_safety_converge(soc_mbigen_io_converage_tab, items_num, ras_code);
}

#define DEV_NODE_OFFSET 16
#define DEV_NODE_MASK 0xFFFF
#define SENSOR_PRIV_DATA(dev_node) ((u64)(dev_node & DEV_NODE_MASK) << DEV_NODE_OFFSET)

STATIC int soc_scan_mbigen_io_event(u64 private_data, struct dms_sensor_event_data *data)
{
    u32 node_id;

    node_id = ((private_data >> DEV_NODE_OFFSET) & DEV_NODE_MASK);

    if (node_id >= soc_misc_mbigen_io_dms.node_num) {
        soc_misc_drv_err("invalid node id. (node_id=%u, priv=0x%llx)\n", node_id, private_data);
        return -EINVAL;
    }

    (void)dfm_scan_events(&soc_misc_mbigen_io_dms, node_id, DMS_SEN_TYPE_MODULE_SENSOR2, data);
    return 0;
}

STATIC int soc_misc_mbigen_io_register_dms_node(u32 dev_id)
{
    struct dms_sensor_object_cfg sensor_cfg = SOC_MISC_SENOR_OBJ(
        DMS_SEN_TYPE_MODULE_SENSOR2, "soc_mbigen_io", DMS_DISCRETE_SENSOR_CLASS,
        DMS_SENSOR_ATTRIB_THRES_NONE, 0, DMS_SENSOR_CHECK_INTERVAL_TIME,
        DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, soc_scan_mbigen_io_event,
        SENSOR_PRIV_DATA(0), 0xFFFF, 0xFFFF);
    u32 i;
    soc_misc_mbigen_io_dms.dev_id = dev_id;

    if (dfm_struct_init(&soc_misc_mbigen_io_dms, SOC_MBIGEN_IO_NODE_NUM, 1) != 0) {
        soc_misc_drv_err("init soc misc mbigen io's dfm data failed\n");
        goto _fail;
    }

    for (i = 0; i < SOC_MBIGEN_IO_NODE_NUM; ++i) {
        soc_misc_mbigen_io_dms.dev_nodes[i].node = &g_soc_mbigen_io_dms_nodes[i];
        soc_misc_mbigen_io_dms.dev_nodes[i].post_proc = NULL;
        soc_misc_mbigen_io_dms.dev_nodes[i].fpdc_notify = NULL;
        soc_misc_mbigen_io_dms.dev_nodes[i].get_converage_node = soc_mbigen_io_safety_converge;
        sensor_cfg.private_data = SENSOR_PRIV_DATA(i);
        soc_misc_mbigen_io_dms.dev_nodes[i].sensors[0].dms_sensor = sensor_cfg;
    }

    if (dfm_register_nodes(&soc_misc_mbigen_io_dms) != 0) {
        soc_misc_drv_err("register soc misc mbigen io dms node failed\n");
        goto _fail;
    }
    return 0;

_fail:
    dfm_struct_final(&soc_misc_mbigen_io_dms, SOC_MBIGEN_IO_NODE_NUM, 1);
    return -EFAULT;
}

int soc_mbigen_io_safety_handler(struct safety_fault_info *safety_fault,
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
        module_info.dev_node = &soc_misc_mbigen_io_dms.dev_nodes[0];
        module_info.section_type = SOC_ERR_SECTOR_SAFETY;
        dfm_safety_handler(&module_info, &safety_fault->fault_status_list[i], event_num, *event_list);
    }
    return 0;
}

struct soc_misc_safety_cfg g_mbigen_io_safety_cfg = {
    .dms_node_num = SOC_MBIGEN_IO_NODE_NUM,
    .irq_cfg_num = SOC_MBIGEN_IO_NODE_NUM,
    .dfm = &soc_misc_mbigen_io_dms,
    .safety_irq_cfgs = g_safety_mbigen_io,
    .register_dms_node = soc_misc_mbigen_io_register_dms_node,
    .post_init_process = NULL,
    .post_uninit_process = NULL,
};
