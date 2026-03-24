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
* Create: 2022-11-15
*/

#include <linux/sizes.h>
#include <linux/slab.h>
#include "aos_hwfault.h"

#include "dms_node_type.h"
#include "dms_sensor_type.h"
#include "drvfault_user_common.h"
#include "dfm_safety_report.h"
#include "dfm_report.h"
#include "dfm_safety_report.h"
#include "soc_misc_init.h"
#include "soc_misc_common.h"
#include "soc_misc_dms_def.h"
#include "soc_misc_err_info.h"
#include "soc_misc_fault.h"
#include "soc_misc_gic_ao.h"

#define GIC_AO_SAFETY_INT_SRC0        0x3C14
#define GIC_AO_SAFETY_INT_CLR0        0x3C14
#define GIC_AO_SAFETY_INT_MASK0       0x3C44

#define GIC_AO_SAFETY_ERR_SRC0        0x3C14
#define GIC_AO_SAFETY_ERR_CLR0        0x3C14
#define GIC_AO_SAFETY_ERR_MASK0       0x3C4C

#define GIC_AO_SAFETY_ERR_SRC1        0x3C18
#define GIC_AO_SAFETY_ERR_CLR1        0x3C18
#define GIC_AO_SAFETY_ERR_MASK1       0x3C50

enum gic_ao_safety_err_bit0 {
    GIC_STREAM_BUS_PARITY = 0,
    GIC_SLV0_AW_CHANNEL_PARITY = 1,
    GIC_SLV1_AW_CHANNEL_PARITY = 2,
    GIC_SLV0_AR_CHANNEL_PARITY = 3,
    GIC_MASTER_AW_CHANNEL_PARITY = 5,
    GIC_MASTER_AR_CHANNEL_PARITY = 6,
    GIC_CTIIRQ_ACK_CHECK = 7,
    GIC_INT_SAFETY_CHECK = 9,
    GIC_INT_SAFETY_ERR_CHECK = 10,
    GIC_INT_CE_GIC_CHECK = 11,
    GIC_INT_NFE_GIC_CHECK = 12,
    GIC_INT_FE_GIC_CHECK = 13,
    GIC_INT_RAS_SAFETY_CHECK = 14,
    GIC_FIFO_SAFETY_CHECK = 15
};

enum hgic_ao_safety_err_bit1 {
    GIC_IRQS_LOCK_STEP = 0,
    GIC_NCNTHPIRQ_LOCK_STEP = 1,
    GIC_NCNTPNSIRQ_LOCK_STEP = 2,
    GIC_NCNTPSIRQ_LOCK_STEP = 3,
    GIC_NCNTVIRQ_LOCK_STEP = 4,
    GIC_NDCCIRQ_LOCK_STEP = 5,
    GIC_NCNTHVIRQ_LOCK_STEP = 6,
    GIC_NLEGACYIRQ_LOCK_STEP = 7,
    GIC_NPLEIRQ_LOCK_STEP = 8,
    GIC_NPMBIRQ_LOCK_STEP = 9,
    GIC_NPMUIRQ_LOCK_STEP = 10,
    GIC_NVCPUMNTIRQ_LOCK_STEP = 11,
    GIC_CTIIRQ_LOCK_STEP = 12,
    GIC_MULTI_BIT_ECC = 29,
    GIC_AXIM_READ_CARRAY_POISON = 30
};

#ifdef AOS_LLVM_BUILD
#define SZ_16M                      (0x01000000)
#endif
#define SOC_MISC_SAFETY_EVENT_MAX   (32U)
#define SOC_GICAO_NODE_NUM          (1U)

/************************************************************************/
/* support TOP RAS fault irq of GIC AO in BS9SX1A                      */
/************************************************************************/

#define GIC_AO_DFX_REGS_NUM   (14)
static u32 g_gic_ao_dfx_regs[GIC_AO_DFX_REGS_NUM] = {
    0x2080, 0x100C, 0x3C14, 0x3C18, 0x3C40, 0x3C44, 0x3C48, 0x3C4C, 0x3C50, 0x3C54, 0x3C58, 0x3C5C, 0x3C60, 0x3C64
};

#define GIC_AO_SAFETY_INT_INFO0(reg_base) {         \
    .emu_id = AO_SUB_EMU_ID,                        \
    .src_id = 1,                                    \
    .bit_id = GIC_AO_INT_SAFETY_BIT_ID,             \
    .base_paddr = reg_base,                         \
    .base_size = SZ_16M,                            \
    .base_vaddr = 0,                                \
    .status = {GIC_AO_SAFETY_INT_SRC0, 0x7800},     \
    .clear = {GIC_AO_SAFETY_INT_CLR0, 0x7800},      \
    .mask = {GIC_AO_SAFETY_INT_MASK0, 0x7800},      \
    .check_mask[0] = {DRVFAULT_FILED_VALID, 0x3C60, 0xFFFF0110}, \
    .dfx_reg_num = GIC_AO_DFX_REGS_NUM,             \
    .dfx_reg_list = g_gic_ao_dfx_regs,              \
}

#define GIC_AO_SAFETY_ERR_INFO0(reg_base) {         \
    .emu_id = AO_SUB_EMU_ID,                        \
    .src_id = 1,                                    \
    .bit_id = GIC_AO_ERR_SAFETY_BIT_ID,             \
    .base_paddr = reg_base,                         \
    .base_size = SZ_16M,                            \
    .base_vaddr = 0,                                \
    .status = {GIC_AO_SAFETY_ERR_SRC0, 0x86EF},     \
    .clear = {GIC_AO_SAFETY_ERR_CLR0, 0x86EF},      \
    .mask = {GIC_AO_SAFETY_ERR_MASK0, 0x86EF},      \
    .check_mask[0] = {DRVFAULT_FILED_VALID, 0x3C60, 0xFFFF0110}, \
    .dfx_reg_num = GIC_AO_DFX_REGS_NUM,             \
    .dfx_reg_list = g_gic_ao_dfx_regs,              \
}

#define GIC_AO_SAFETY_ERR_INFO1(reg_base) {         \
    .emu_id = AO_SUB_EMU_ID,                        \
    .src_id = 1,                                    \
    .bit_id = GIC_AO_ERR_SAFETY_BIT_ID,             \
    .base_paddr = reg_base,                         \
    .base_size = SZ_16M,                            \
    .base_vaddr = 0,                                \
    .status = {GIC_AO_SAFETY_ERR_SRC1, 0x60000000}, \
    .clear = {GIC_AO_SAFETY_ERR_CLR1, 0x60000000},  \
    .mask = {GIC_AO_SAFETY_ERR_MASK1, 0x60000000},  \
    .check_mask[0] = {DRVFAULT_FILED_VALID, 0x3C64, 0x9FFFFFFF}, \
    .dfx_reg_num = GIC_AO_DFX_REGS_NUM,             \
    .dfx_reg_list = g_gic_ao_dfx_regs,              \
}

STATIC struct safety_irq_hw_info g_gic_ao_safety_hwinfo[] = {
    GIC_AO_SAFETY_INT_INFO0(GIC_AO_REG_BASE),
    GIC_AO_SAFETY_ERR_INFO0(GIC_AO_REG_BASE),
    GIC_AO_SAFETY_ERR_INFO1(GIC_AO_REG_BASE),
};

struct safety_irq_info g_safety_gic_ao[] = {
    {
        .dev_id = 0,
        .irq_type = SAFETY_IRQ_TYPE_SPI,
        .irq_name = "int_ras_safety_gic",
        .safety_irq_func = soc_gic_ao_safety_handler,
        .irq_hwinfo_num = 3,
        .irq_hwinfo_list = g_gic_ao_safety_hwinfo
    }
};

STATIC int soc_misc_gicao_ops_init(struct dms_node *device)
{
    soc_misc_drv_debug("DEBUG (node_name=%.*s)\n", DMS_MAX_DEV_NAME_LEN, device->node_name);
    return 0;
}

STATIC void soc_misc_gicao_ops_uninit(struct dms_node *device)
{
    soc_misc_drv_debug("DEBUG (node_name=%.*s)\n", DMS_MAX_DEV_NAME_LEN, device->node_name);
    return;
}

STATIC struct dms_node_operations g_soc_misc_gicao_ops = {
    .init = soc_misc_gicao_ops_init,
    .uninit = soc_misc_gicao_ops_uninit,
    .scan = NULL,
    .fault_diag = NULL,
    .event_notify = NULL,
    .get_link_state = NULL,
    .set_link_state = NULL
};

#define SOC_GICAO_DMS_NODE_DEFINE(_id, _ops) {    \
    .node_type = DMS_DEV_TYPE_GIC,                \
    .node_id = _id,                               \
    .node_name = "SOC-GICAO-" #_id,               \
    .capacity = 0x1,                              \
    .permission = 0x1,                            \
    .owner_devid = 0,                             \
    .ops = _ops                                   \
}

STATIC struct dfm_struct soc_misc_gic_ao_dms;
static struct dms_node g_soc_gic_ao_dms_nodes[SOC_GICAO_NODE_NUM] = {
    SOC_GICAO_DMS_NODE_DEFINE(0, &g_soc_misc_gicao_ops),
};

#define GIC_AO_SAFETY_COVERAGE_ITEM(_err_status, _describe, _sensor_type, _err_type) { \
    .subsys_id = DMS_DEV_TYPE_GIC,          \
    .module_id = 0,                         \
    .section_type = SOC_ERR_SECTOR_SAFETY,  \
    .ras_code.err_status = _err_status,     \
    .describe = _describe,                  \
    .sensor_type = _sensor_type,            \
    .error_type = _err_type                 \
}

#define SAFETY_INT_STATUS(status_offset, safety_bit)  (((u32)status_offset << 8) | safety_bit)

const struct ras_fault_converge_item *soc_gic_ao_safety_converge(
    const struct safety_fault_status *fault_status, u32 section_type, u64 ras_code)
{
    static struct ras_fault_converge_item soc_gic_ao_converage_tab[] = {
        /* Group 0 */
        GIC_AO_SAFETY_COVERAGE_ITEM(SAFETY_INT_STATUS(0x3C14, GIC_STREAM_BUS_PARITY),
            "gic_stream_bus_parity", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_PARITY),
        GIC_AO_SAFETY_COVERAGE_ITEM(SAFETY_INT_STATUS(0x3C14, GIC_SLV0_AW_CHANNEL_PARITY),
            "gic_slv0_AW/W/B_channel_parity", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_PARITY),
        GIC_AO_SAFETY_COVERAGE_ITEM(SAFETY_INT_STATUS(0x3C14, GIC_SLV1_AW_CHANNEL_PARITY),
            "gic_slv1_AW/W/B_channel_parity", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_PARITY),
        GIC_AO_SAFETY_COVERAGE_ITEM(SAFETY_INT_STATUS(0x3C14, GIC_SLV0_AR_CHANNEL_PARITY),
            "gic_slv0_AR/R_channel_parity", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_PARITY),
        GIC_AO_SAFETY_COVERAGE_ITEM(SAFETY_INT_STATUS(0x3C14, GIC_MASTER_AW_CHANNEL_PARITY),
            "gic_master_AW/W/B_channel_parity", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_PARITY),
        GIC_AO_SAFETY_COVERAGE_ITEM(SAFETY_INT_STATUS(0x3C14, GIC_MASTER_AR_CHANNEL_PARITY),
            "gic_master_AR/R_channel_parity", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_PARITY),
        GIC_AO_SAFETY_COVERAGE_ITEM(SAFETY_INT_STATUS(0x3C14, GIC_CTIIRQ_ACK_CHECK),
            "ctiirq_ack_check", DMS_SEN_TYPE_SAFETY_SENSOR, SOC_SAFETY_LOCKSTEP_ERR),
        GIC_AO_SAFETY_COVERAGE_ITEM(SAFETY_INT_STATUS(0x3C14, GIC_INT_SAFETY_CHECK),
            "int_safety_check", DMS_SEN_TYPE_EXTEND_SENSOR, EXTEND_ERROR_TYPE_REPORT_CHANNEL_TO_SILS_ERR),
        GIC_AO_SAFETY_COVERAGE_ITEM(SAFETY_INT_STATUS(0x3C14, GIC_INT_SAFETY_ERR_CHECK),
            "int_safety_err_check", DMS_SEN_TYPE_EXTEND_SENSOR, EXTEND_ERROR_TYPE_REPORT_CHANNEL_TO_SILS_ERR),
        GIC_AO_SAFETY_COVERAGE_ITEM(SAFETY_INT_STATUS(0x3C14, GIC_INT_CE_GIC_CHECK),
            "int_ce_gic_check", DMS_SEN_TYPE_EXTEND_SENSOR, EXTEND_ERROR_TYPE_REPORT_CHANNEL_ERR),
        GIC_AO_SAFETY_COVERAGE_ITEM(SAFETY_INT_STATUS(0x3C14, GIC_INT_NFE_GIC_CHECK),
            "int_nfe_gic_check", DMS_SEN_TYPE_EXTEND_SENSOR, EXTEND_ERROR_TYPE_REPORT_CHANNEL_ERR),
        GIC_AO_SAFETY_COVERAGE_ITEM(SAFETY_INT_STATUS(0x3C14, GIC_INT_FE_GIC_CHECK),
            "int_fe_gic_check", DMS_SEN_TYPE_EXTEND_SENSOR, EXTEND_ERROR_TYPE_REPORT_CHANNEL_ERR),
        GIC_AO_SAFETY_COVERAGE_ITEM(SAFETY_INT_STATUS(0x3C14, GIC_INT_RAS_SAFETY_CHECK),
            "int_ras_safety_check", DMS_SEN_TYPE_EXTEND_SENSOR, EXTEND_ERROR_TYPE_REPORT_CHANNEL_ERR),
        GIC_AO_SAFETY_COVERAGE_ITEM(SAFETY_INT_STATUS(0x3C14, GIC_FIFO_SAFETY_CHECK),
            "gic_fifo_safety_check", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_PARITY),

        /* Group 1: lockstep error has mask at 20230510, which request by chip */
        GIC_AO_SAFETY_COVERAGE_ITEM(SAFETY_INT_STATUS(0x3C18, GIC_IRQS_LOCK_STEP),
            "irqs_lock_step", DMS_SEN_TYPE_SAFETY_SENSOR, SOC_SAFETY_LOCKSTEP_ERR),
        GIC_AO_SAFETY_COVERAGE_ITEM(SAFETY_INT_STATUS(0x3C18, GIC_NCNTHPIRQ_LOCK_STEP),
            "ncnthpirq_lock_step", DMS_SEN_TYPE_SAFETY_SENSOR, SOC_SAFETY_LOCKSTEP_ERR),
        GIC_AO_SAFETY_COVERAGE_ITEM(SAFETY_INT_STATUS(0x3C18, GIC_NCNTPNSIRQ_LOCK_STEP),
            "ncntpnsirq_lock_step", DMS_SEN_TYPE_SAFETY_SENSOR, SOC_SAFETY_LOCKSTEP_ERR),
        GIC_AO_SAFETY_COVERAGE_ITEM(SAFETY_INT_STATUS(0x3C18, GIC_NCNTPSIRQ_LOCK_STEP),
            "ncntpsirq_lock_step", DMS_SEN_TYPE_SAFETY_SENSOR, SOC_SAFETY_LOCKSTEP_ERR),
        GIC_AO_SAFETY_COVERAGE_ITEM(SAFETY_INT_STATUS(0x3C18, GIC_NCNTVIRQ_LOCK_STEP),
            "ncntvirq_lock_step", DMS_SEN_TYPE_SAFETY_SENSOR, SOC_SAFETY_LOCKSTEP_ERR),
        GIC_AO_SAFETY_COVERAGE_ITEM(SAFETY_INT_STATUS(0x3C18, GIC_NDCCIRQ_LOCK_STEP),
            "ndccirq_lock_step", DMS_SEN_TYPE_SAFETY_SENSOR, SOC_SAFETY_LOCKSTEP_ERR),
        GIC_AO_SAFETY_COVERAGE_ITEM(SAFETY_INT_STATUS(0x3C18, GIC_NCNTHVIRQ_LOCK_STEP),
            "ncnthvirq_lock_step", DMS_SEN_TYPE_SAFETY_SENSOR, SOC_SAFETY_LOCKSTEP_ERR),
        GIC_AO_SAFETY_COVERAGE_ITEM(SAFETY_INT_STATUS(0x3C18, GIC_NLEGACYIRQ_LOCK_STEP),
            "nlegacyirq_lock_step", DMS_SEN_TYPE_SAFETY_SENSOR, SOC_SAFETY_LOCKSTEP_ERR),
        GIC_AO_SAFETY_COVERAGE_ITEM(SAFETY_INT_STATUS(0x3C18, GIC_NPLEIRQ_LOCK_STEP),
            "npleirq_lock_step", DMS_SEN_TYPE_SAFETY_SENSOR, SOC_SAFETY_LOCKSTEP_ERR),
        GIC_AO_SAFETY_COVERAGE_ITEM(SAFETY_INT_STATUS(0x3C18, GIC_NPMBIRQ_LOCK_STEP),
            "npmbirq_lock_step", DMS_SEN_TYPE_SAFETY_SENSOR, SOC_SAFETY_LOCKSTEP_ERR),
        GIC_AO_SAFETY_COVERAGE_ITEM(SAFETY_INT_STATUS(0x3C18, GIC_NPMUIRQ_LOCK_STEP),
            "npmuirq_lock_step", DMS_SEN_TYPE_SAFETY_SENSOR, SOC_SAFETY_LOCKSTEP_ERR),
        GIC_AO_SAFETY_COVERAGE_ITEM(SAFETY_INT_STATUS(0x3C18, GIC_NVCPUMNTIRQ_LOCK_STEP),
            "nvcpumntirq_lock_step", DMS_SEN_TYPE_SAFETY_SENSOR, SOC_SAFETY_LOCKSTEP_ERR),
        GIC_AO_SAFETY_COVERAGE_ITEM(SAFETY_INT_STATUS(0x3C18, GIC_CTIIRQ_LOCK_STEP),
            "ctiirq_lock_step", DMS_SEN_TYPE_SAFETY_SENSOR, SOC_SAFETY_LOCKSTEP_ERR),
        /* The above lockstep faults have been masked. */

        GIC_AO_SAFETY_COVERAGE_ITEM(SAFETY_INT_STATUS(0x3C18, GIC_MULTI_BIT_ECC),
            "multi_bit_ecc", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_MBECC),
        GIC_AO_SAFETY_COVERAGE_ITEM(SAFETY_INT_STATUS(0x3C18, GIC_AXIM_READ_CARRAY_POISON),
            "AXIM_read_carry_poison", DMS_SEN_TYPE_SAFETY_SENSOR, SOC_SAFETY_READ_DATA_POISON),
};
    /* Find the converge item */
    u32 items_num = (u32)ARRAY_SIZE(soc_gic_ao_converage_tab);
    return soc_safety_converge(soc_gic_ao_converage_tab, items_num,
        SAFETY_INT_STATUS(fault_status->fault_status_offset, ras_code));
}

#define DEV_NODE_OFFSET 16
#define DEV_NODE_MASK 0xFFFF
#define SENSOR_NODE_OFFSET 0
#define SENSOR_NODE_MASK 0xFFFF
#define SENSOR_PRIV_DATA(dev_node, sensor_node) \
    (((u64)(dev_node & DEV_NODE_MASK) << DEV_NODE_OFFSET) | \
        ((u64)(sensor_node & SENSOR_NODE_MASK) << SENSOR_NODE_OFFSET))

STATIC int soc_scan_gic_ao_event(u64 private_data, struct dms_sensor_event_data *data)
{
    u32 node_id;
    u32 sensor_type;

    node_id = ((private_data >> DEV_NODE_OFFSET) & DEV_NODE_MASK);
    sensor_type = ((private_data >> SENSOR_NODE_OFFSET) & SENSOR_NODE_MASK);

    if (node_id >= soc_misc_gic_ao_dms.node_num) {
        soc_misc_drv_err("invalid node id. (node_id=%u, priv=0x%llx)\n", node_id, private_data);
        return -EINVAL;
    }

    (void)dfm_scan_events(&soc_misc_gic_ao_dms, node_id, sensor_type, data);
    return 0;
}

STATIC int soc_misc_gic_ao_register_dms_node(u32 dev_id)
{
    struct dms_sensor_object_cfg sensor_cfg[] = {
        /* SOC_Event */
        SOC_MISC_SENOR_OBJ(DMS_SEN_TYPE_RAS_SENSOR, "soc_gic_ao", DMS_DISCRETE_SENSOR_CLASS,
            DMS_SENSOR_ATTRIB_THRES_NONE, 0, DMS_SENSOR_CHECK_INTERVAL_TIME,
            DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, soc_scan_gic_ao_event,
            SENSOR_PRIV_DATA(0, DMS_SEN_TYPE_RAS_SENSOR), 0xFFFF, 0xFFBF),
        /* SOC_Event1 */
        SOC_MISC_SENOR_OBJ(DMS_SEN_TYPE_SAFETY_SENSOR, "soc_gic_ao", DMS_DISCRETE_SENSOR_CLASS,
            DMS_SENSOR_ATTRIB_THRES_NONE, 0, DMS_SENSOR_CHECK_INTERVAL_TIME,
            DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, soc_scan_gic_ao_event,
            SENSOR_PRIV_DATA(0, DMS_SEN_TYPE_SAFETY_SENSOR), 0xFFFF, 0xFFFF),
        /* SOC_Event2 */
        SOC_MISC_SENOR_OBJ(DMS_SEN_TYPE_EXTEND_SENSOR, "soc_gic_ao", DMS_DISCRETE_SENSOR_CLASS,
            DMS_SENSOR_ATTRIB_THRES_NONE, 0, DMS_SENSOR_CHECK_INTERVAL_TIME,
            DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, soc_scan_gic_ao_event,
            SENSOR_PRIV_DATA(0, DMS_SEN_TYPE_EXTEND_SENSOR), 0xFFFF, 0xFFFF),
    };
    u32 sensor_num = (u32)ARRAY_SIZE(sensor_cfg);
    u32 i;
    u32 sensor_idx;
    soc_misc_gic_ao_dms.dev_id = dev_id;

    if (dfm_struct_init(&soc_misc_gic_ao_dms, SOC_GICAO_NODE_NUM, sensor_num) != 0) {
        soc_misc_drv_err("init soc misc gic ao's dfm data failed\n");
        goto _fail;
    }
    /* Get soc_misc_gic_ao_dms */
    for (i = 0; i < SOC_GICAO_NODE_NUM; ++i) {
        soc_misc_gic_ao_dms.dev_nodes[i].node = &g_soc_gic_ao_dms_nodes[i];
        soc_misc_gic_ao_dms.dev_nodes[i].post_proc = NULL;
        soc_misc_gic_ao_dms.dev_nodes[i].fpdc_notify = NULL;
        soc_misc_gic_ao_dms.dev_nodes[i].coverage_version = EVENT_COVERAGE_V2;
        soc_misc_gic_ao_dms.dev_nodes[i].get_converage_node2 = soc_gic_ao_safety_converge;
        for (sensor_idx = 0; sensor_idx < sensor_num; ++sensor_idx) {
            soc_misc_gic_ao_dms.dev_nodes[i].sensors[sensor_idx].dms_sensor = sensor_cfg[sensor_idx];
        }
    }

    if (dfm_register_nodes(&soc_misc_gic_ao_dms) != 0) {
        soc_misc_drv_err("register soc misc gic ao dms node failed\n");
        goto _fail;
    }
    return 0;

_fail:
    dfm_struct_final(&soc_misc_gic_ao_dms, SOC_GICAO_NODE_NUM, sensor_num);
    return -EFAULT;
}

STATIC void soc_gic_fault_postproc(u32 dev_id, struct safety_fault_status *fault_status,
    struct safety_event *event_list, u32 event_num)
{
    int ret;
    u32 event_id, j;
    struct AosHwFaultInfo gic_fault_info = {0};
    struct AosRegStatus gic_fault_status = {0};

    if (event_list == NULL) {
        soc_misc_drv_err("event_list is null.\n");
        return;
    }

    for (j = 0; j < event_num; j++) {
        event_id = dfm_gen_event_id(event_list->node_type, event_list->sensor_type,
        event_list->event_type);
        gic_fault_info.devId = dev_id;
        gic_fault_info.regNum = 1;
        gic_fault_status.addr = fault_status->base_paddr;
        gic_fault_status.offset = fault_status->fault_status_offset;
        gic_fault_status.val = fault_status->fault_status;
        gic_fault_info.regs = &gic_fault_status;
        ret = AOS_HwFaultHandler(event_id, &gic_fault_info);

        soc_misc_drv_event("AOS_HwFaultHandler: (event_id=%u, offset=%u, val=%u)\n",
            event_id, gic_fault_info.regs->offset, gic_fault_info.regs->val);
        if (ret != 0) {
            soc_misc_drv_err("call AOS_HwFaultHandler failed(ret=%d)\n", ret);
        }
        event_list++;
    }
    return;
}

int soc_gic_ao_safety_handler(struct safety_fault_info *safety_fault,
    unsigned int *event_num, struct safety_event **event_list)
{
    struct dfm_safety_module module_info;
    u32 i, event_num_start;

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
    module_info.node_type = DMS_DEV_TYPE_GIC;
    module_info.max_event = SOC_MISC_SAFETY_EVENT_MAX;

    for (i = 0; i < safety_fault->fault_reg_num; ++i) {
        event_num_start = *event_num;
        module_info.node_id = 0;
        module_info.dev_node = &soc_misc_gic_ao_dms.dev_nodes[0];
        module_info.section_type = SOC_ERR_SECTOR_SAFETY;
        dfm_safety_handler(&module_info, &safety_fault->fault_status_list[i], event_num, *event_list);

        /* Call aoscore's interface */
        soc_gic_fault_postproc(module_info.dev_id, &safety_fault->fault_status_list[i],
            *event_list + event_num_start, *event_num - event_num_start);
    }
    return 0;
}

struct soc_misc_safety_cfg g_gic_ao_safety_cfg = {
    .dms_node_num = SOC_GICAO_NODE_NUM,
    .irq_cfg_num = SOC_GICAO_NODE_NUM,
    .dfm = &soc_misc_gic_ao_dms,
    .safety_irq_cfgs = g_safety_gic_ao,
    .register_dms_node = soc_misc_gic_ao_register_dms_node,
    .post_init_process = NULL,
    .post_uninit_process = NULL,
};
