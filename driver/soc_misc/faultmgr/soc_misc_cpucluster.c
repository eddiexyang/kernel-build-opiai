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

#include "dms_node_type.h"
#include "dms_sensor_type.h"
#include "drvfault_user_common.h"
#include "dfm_safety_report.h"
#include "dfm_report.h"
#include "soc_misc_init.h"
#include "soc_misc_dms_def.h"
#include "soc_misc_err_info.h"
#include "soc_misc_safety.h"
#include "soc_misc_fault.h"
#include "soc_misc_cpucluster.h"

#define CPU_CLUSTER_SAFETY_SRC      0x2C00
#define CPU_CLUSTER_SAFETY_CLR      0x2C00
#define CPU_CLUSTER_SAFETY_INT_MASK 0x2C04

#define CPU_CLUSTER_SAFETY_ERR_SRC      0x2C00
#define CPU_CLUSTER_SAFETY_ERR_CLR      0x2C00
#define CPU_CLUSTER_SAFETY_ERR_INT_MASK 0x2C08

enum cpu_cluster_safety_err_bit {
    CPU_CLUSTER_SUBCTRL_PARITY_ERR = 0,
    CPU_CLUSTER_ASYNC_GIC_ERR = 1,
    CPU_CLUSTER_ASYNC_GIC_TAISHAN_ERR = 2
};

#define SOC_MISC_SAFETY_EVENT_MAX    (32U)
#define SOC_CPUCLUSTER_NODE_NUM      (4U)

#define TO_STRING(id) #id

/************************************************************************/
/* support TOP RAS fault irq of cpu cluster in BS9SX1A                      */
/************************************************************************/

#define CPU_CLUSTER_DFX_REGS_NUM (9)
static u32 g_cpu_cluster_dfx_regs[CPU_CLUSTER_DFX_REGS_NUM] = {
    0x2C00, 0x2C04, 0x2C08, 0x2C0C, 0x2C14, 0x4000, 0x4004, 0xF200
};

#define CPU_CLUSTER_SAFETY_INT_INFO(reg_base, id) { \
    .emu_id = (CPU_CLUSTER0_EMU_ID + id),           \
    .src_id = 1,                                    \
    .bit_id = CPU_CLUSTER_INT_SAFETY_BIT_ID,        \
    .base_paddr = reg_base,                         \
    .base_size = SZ_64K,                            \
    .base_vaddr = 0,                                \
    .status = {CPU_CLUSTER_SAFETY_SRC, 0x00},       \
    .clear = {CPU_CLUSTER_SAFETY_CLR, 0x00},        \
    .mask = {CPU_CLUSTER_SAFETY_INT_MASK, 0x00},    \
    .check_mask[0] = {DRVFAULT_FILED_VALID, 0x2C0C, 0xFFFFFFF8}, \
    .dfx_reg_num = CPU_CLUSTER_DFX_REGS_NUM,        \
    .dfx_reg_list = g_cpu_cluster_dfx_regs,         \
}

#define CPU_CLUSTER_SAFETY_ERR_INFO(reg_base, id) {  \
    .emu_id = (CPU_CLUSTER0_EMU_ID + id),            \
    .src_id = 1,                                     \
    .bit_id = CPU_CLUSTER_ERR_SAFETY_BIT_ID,         \
    .base_paddr = reg_base,                          \
    .base_size = SZ_64K,                             \
    .base_vaddr = 0,                                 \
    .status = {CPU_CLUSTER_SAFETY_ERR_SRC, 0x07},    \
    .clear = {CPU_CLUSTER_SAFETY_ERR_CLR, 0x07},     \
    .mask = {CPU_CLUSTER_SAFETY_ERR_INT_MASK, 0x07}, \
    .check_mask[0] = {DRVFAULT_FILED_VALID, 0x2C0C, 0xFFFFFFF8}, \
    .dfx_reg_num = CPU_CLUSTER_DFX_REGS_NUM,         \
    .dfx_reg_list = g_cpu_cluster_dfx_regs,          \
}

STATIC struct safety_irq_hw_info g_cpu_cluster_safety_hwinfo[SOC_CPUCLUSTER_NODE_NUM][1] = {
    {
        CPU_CLUSTER_SAFETY_ERR_INFO(CPU_CLUSTER0_REG_BASE, SOC_CPU_CLUSTER_0),
    }, {
        CPU_CLUSTER_SAFETY_ERR_INFO(CPU_CLUSTER1_REG_BASE, SOC_CPU_CLUSTER_1),
    }, {
        CPU_CLUSTER_SAFETY_ERR_INFO(CPU_CLUSTER2_REG_BASE, SOC_CPU_CLUSTER_2),
    }, {
        CPU_CLUSTER_SAFETY_ERR_INFO(CPU_CLUSTER3_REG_BASE, SOC_CPU_CLUSTER_3),
    },
};

#define CPU_CLUSTER_FHI_INTERRUPT_HWINFO(cluster_id) {               \
    .dev_id = 0,                                                     \
    .irq_type = SAFETY_IRQ_TYPE_LPI,                                 \
    .irq_name = "int_ras_safety_cpu_cluster" NUM_TO_STR(cluster_id), \
    .safety_irq_func = soc_cpucluster_safety_handler,                \
    .irq_hwinfo_num = 1,                                             \
    .irq_hwinfo_list = g_cpu_cluster_safety_hwinfo[cluster_id]       \
}

struct safety_irq_info g_safety_cpu_cluster[SOC_CPUCLUSTER_NODE_NUM] = {
    CPU_CLUSTER_FHI_INTERRUPT_HWINFO(0),
    CPU_CLUSTER_FHI_INTERRUPT_HWINFO(1),
    CPU_CLUSTER_FHI_INTERRUPT_HWINFO(2),
    CPU_CLUSTER_FHI_INTERRUPT_HWINFO(3)
};

STATIC int soc_misc_cpucluster_ops_init(struct dms_node *device)
{
    soc_misc_drv_debug("DEBUG (node_name=%.*s)\n", DMS_MAX_DEV_NAME_LEN, device->node_name);
    return 0;
}

STATIC void soc_misc_cpucluster_ops_uninit(struct dms_node *device)
{
    soc_misc_drv_debug("DEBUG (node_name=%.*s)\n", DMS_MAX_DEV_NAME_LEN, device->node_name);
    return;
}

STATIC struct dms_node_operations g_soc_misc_cpucluster_ops = {
    .init = soc_misc_cpucluster_ops_init,
    .uninit = soc_misc_cpucluster_ops_uninit,
    .scan = NULL,
    .fault_diag = NULL,
    .event_notify = NULL,
    .get_link_state = NULL,
    .set_link_state = NULL
};

#define SOC_CPUCLUSTER_DMS_NODE_DEFINE(_id, _ops) { \
    .node_type = DMS_DEV_TYPE_CPU_CLUSTER,          \
    .node_id = _id,                                 \
    .node_name = "SOC-CPUCLUSTER-" #_id,            \
    .capacity = 0x1,                                \
    .permission = 0x1,                              \
    .owner_devid = 0,                               \
    .ops = _ops                                     \
}

static struct dfm_struct soc_misc_cpucluster_dms;
static struct dms_node g_soc_cpucluster_dms_nodes[SOC_CPUCLUSTER_NODE_NUM] = {
    SOC_CPUCLUSTER_DMS_NODE_DEFINE(0, &g_soc_misc_cpucluster_ops),
    SOC_CPUCLUSTER_DMS_NODE_DEFINE(1, &g_soc_misc_cpucluster_ops),
    SOC_CPUCLUSTER_DMS_NODE_DEFINE(2, &g_soc_misc_cpucluster_ops),
    SOC_CPUCLUSTER_DMS_NODE_DEFINE(3, &g_soc_misc_cpucluster_ops),
};

#define CPUCLUSTER_SAFETY_COVERAGE_ITEM(_err_status, _describe, _err_type) { \
    .subsys_id = DMS_DEV_TYPE_CPU_CLUSTER,  \
    .module_id = SOC_MISC_SENSOR_CPUCORE,   \
    .section_type = SOC_ERR_SECTOR_SAFETY,  \
    .ras_code.err_status = _err_status,     \
    .describe = _describe,                  \
    .sensor_type = DMS_SEN_TYPE_RAS_SENSOR, \
    .error_type = _err_type                 \
}

#define CPUCLUSTER_CONVERAGE_NUM 3

const struct ras_fault_converge_item *soc_cpucluster_safety_converge(u32 section_type, u64 ras_code)
{
    static struct ras_fault_converge_item soc_cpucluster_converage_tab[CPUCLUSTER_CONVERAGE_NUM] = {
        CPUCLUSTER_SAFETY_COVERAGE_ITEM(CPU_CLUSTER_SUBCTRL_PARITY_ERR,
            "subctrl_parity_err", RAS_ERROR_TYPE_PARITY),
        CPUCLUSTER_SAFETY_COVERAGE_ITEM(CPU_CLUSTER_ASYNC_GIC_ERR,
            "async_gic_err", RAS_ERROR_TYPE_PARITY),
        CPUCLUSTER_SAFETY_COVERAGE_ITEM(CPU_CLUSTER_ASYNC_GIC_TAISHAN_ERR,
            "async_gic_taishan_err", RAS_ERROR_TYPE_PARITY),
    };

    u32 items_num = (u32)ARRAY_SIZE(soc_cpucluster_converage_tab);
    return soc_safety_converge(soc_cpucluster_converage_tab, items_num, ras_code);
}

#define DEV_NODE_OFFSET 16
#define DEV_NODE_MASK 0xFFFF
#define SENSOR_PRIV_DATA(dev_node) ((u64)(dev_node & DEV_NODE_MASK) << DEV_NODE_OFFSET)

int soc_scan_cpucluster_event(u64 private_data, struct dms_sensor_event_data *data)
{
    u32 node_id;

    node_id = ((private_data >> DEV_NODE_OFFSET) & DEV_NODE_MASK);

    if (node_id >= soc_misc_cpucluster_dms.node_num) {
        soc_misc_drv_err("invalid node id. (node_id=%u, priv=0x%llx)\n", node_id, private_data);
        return -EINVAL;
    }

    (void)dfm_scan_events(&soc_misc_cpucluster_dms, node_id, DMS_SEN_TYPE_RAS_SENSOR, data);
    return 0;
}

int soc_misc_cpucluster_register_dms_node(u32 dev_id)
{
    struct dms_sensor_object_cfg sensor_cfg = SOC_MISC_SENOR_OBJ(
        DMS_SEN_TYPE_RAS_SENSOR, "soc_cpucluster", DMS_DISCRETE_SENSOR_CLASS,
        DMS_SENSOR_ATTRIB_THRES_NONE, 0, DMS_SENSOR_CHECK_INTERVAL_TIME,
        DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, soc_scan_cpucluster_event,
        SENSOR_PRIV_DATA(0), 0xFFFF, 0xFFBF);
    u32 i;
    soc_misc_cpucluster_dms.dev_id = dev_id;

    if (dfm_struct_init(&soc_misc_cpucluster_dms, SOC_CPUCLUSTER_NODE_NUM, 1) != 0) {
        soc_misc_drv_err("init soc misc cpucluster's dfm data failed\n");
        goto _fail;
    }

    for (i = 0; i < SOC_CPUCLUSTER_NODE_NUM; ++i) {
        soc_misc_cpucluster_dms.dev_nodes[i].node = &g_soc_cpucluster_dms_nodes[i];
        soc_misc_cpucluster_dms.dev_nodes[i].post_proc = NULL;
        soc_misc_cpucluster_dms.dev_nodes[i].fpdc_notify = NULL;
        soc_misc_cpucluster_dms.dev_nodes[i].get_converage_node = soc_cpucluster_safety_converge;
        sensor_cfg.private_data = SENSOR_PRIV_DATA(i);
        soc_misc_cpucluster_dms.dev_nodes[i].sensors[0].dms_sensor = sensor_cfg;
    }

    if (dfm_register_nodes(&soc_misc_cpucluster_dms) != 0) {
        soc_misc_drv_err("register soc misc cpucluster dms node failed\n");
        goto _fail;
    }
    return 0;

_fail:
    dfm_struct_final(&soc_misc_cpucluster_dms, SOC_CPUCLUSTER_NODE_NUM, 1);
    return -EFAULT;
}

STATIC int cpucluster_get_node_id(unsigned long long base_paddr, u32 *node_id)
{
    if (base_paddr < CPU_CLUSTER0_REG_BASE || base_paddr > CPU_CLUSTER3_REG_BASE) {
        soc_misc_drv_err("base addr is not belong to cpucluster\n");
        return -EEXIST;
    }

    /* Calculate the number of the cpucluster based on the base address. */
    *node_id = (base_paddr >> SZ_16) - (CPU_CLUSTER0_REG_BASE >> SZ_16);
    return 0;
}

int soc_cpucluster_safety_handler(struct safety_fault_info *safety_fault,
    unsigned int *event_num, struct safety_event **event_list)
{
    struct dfm_safety_module module_info;
    u32 node_id = 0;
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
    module_info.node_type = DMS_DEV_TYPE_CPU_CLUSTER;
    module_info.max_event = SOC_MISC_SAFETY_EVENT_MAX;

    for (i = 0; i < safety_fault->fault_reg_num; ++i) {
        if (cpucluster_get_node_id(safety_fault->fault_status_list[i].base_paddr, &node_id) != 0) {
            soc_misc_drv_err("get node id failed.\n");
            continue;
        }
        module_info.node_id = node_id;
        module_info.dev_node = &soc_misc_cpucluster_dms.dev_nodes[node_id];
        module_info.section_type = SOC_ERR_SECTOR_SAFETY;
        dfm_safety_handler(&module_info, &safety_fault->fault_status_list[i], event_num, *event_list);
    }
    return 0;
}

struct soc_misc_safety_cfg g_cpucluster_safety_cfg = {
    .dms_node_num = SOC_CPUCLUSTER_NODE_NUM,
    .irq_cfg_num = SOC_CPUCLUSTER_NODE_NUM,
    .dfm = &soc_misc_cpucluster_dms,
    .safety_irq_cfgs = g_safety_cpu_cluster,
    .register_dms_node = soc_misc_cpucluster_register_dms_node,
    .post_init_process = NULL,
    .post_uninit_process = NULL,
};
