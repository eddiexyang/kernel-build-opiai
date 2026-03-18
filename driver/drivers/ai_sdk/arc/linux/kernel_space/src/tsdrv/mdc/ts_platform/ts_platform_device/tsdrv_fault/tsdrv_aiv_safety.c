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
* Create: 2023-1-7
*/
#include <linux/sizes.h>
#include <linux/slab.h>

#include "tsdrv_fault_init.h"
#include "tsdrv_aiv_safety.h"
#include "tsdrv_log.h"
#include "dms_node_type.h"
#include "dms_sensor_type.h"
#include "fpdc_ras_receiver.h"
#include "drvfault_user_common.h"
#include "tsdrv_safety_fault.h"
#include "dfm_dev_register.h"
#include "dfm_report.h"
#include "dfm_safety_report.h"
#include "tsdrv_ipc_fault_init.h"
#include "drv_ipc.h"
/**
 * Raw secure interrupt register status,
 * Each error status corresponds to one bit of the register.
 */
enum aiv_safety_err_irq_status {
    AIV_SUBCTRL_PARITY_ERR = 0,
    AIV_SUBCTRL_ASIL_SUBCTRL_PARITY_ERR = 1,
    AIV_SUBCTRL_DECODER_ASIL_VIO_APB_M0 = 8,
    AIV_SUBCTRL_SUBCTRL_ASIL_VIO = 12,
};
/**
 * safety fault src id
 * every fault has it's own src id
 */
#define AIV_SAFETY_SRC_ID      1
#define AIV_SAFETY_BITID_7    (7U)
#define AIV_SAFETY_BITID_23   (23U)
/**
 * aiv base addr
 */
#define AIV_NUM    (8U)
#define AIV0_BASE  (0xC11C0000U)
#define AIV1_BASE  (0xC13C0000U)
#define AIV2_BASE  (0xC15C0000U)
#define AIV3_BASE  (0xC17C0000U)
#define AIV4_BASE  (0xC19C0000U)
#define AIV5_BASE  (0xC1BC0000U)
#define AIV6_BASE  (0xC1DC0000U)
#define AIV7_BASE  (0xC1FC0000U)
/**
 * int_safety_aiv interrupt status offset
 */
#define AIV_SAFETY_INT_SRC      0x2C00
#define AIV_SAFETY_INT_CLR      0x2C00
#define AIV_SAFETY_INT_MASK     0x2C04

#define TO_STRING(id) #id
#define AIV_SAFETY_EVENT_MAX_NUM 32
#define AIV_INTERRUPT_ADDR_SHIFT_BIT 21
/**
 * aiv fault DFX register
 */
#define AIV_SUBCTRL_DFX_REGS_NUM  (4)
static u32 g_aiv_subctrl_dfx_regs[AIV_SUBCTRL_DFX_REGS_NUM] = {
    0x2c04, 0x2c08, 0x2c0c, 0x2c14
};

#define AIV_SAFETY_INT_HWINFO(subsys_id, reg_base, irq_bit) { \
    .emu_id = subsys_id, .src_id = AIV_SAFETY_SRC_ID, .bit_id = irq_bit, \
    .base_paddr = reg_base, .base_size = SZ_64K, .base_vaddr = 0,        \
    .status = { AIV_SAFETY_INT_SRC, 0x1103 },    \
    .clear = { AIV_SAFETY_INT_CLR, 0x1103 },     \
    .mask = { AIV_SAFETY_INT_MASK, 0x1103 },     \
    .check_mask[0] = { DRVFAULT_FILED_VALID, 0x2c0c, 0xFFFFEEFC }, \
    .dfx_reg_num = AIV_SUBCTRL_DFX_REGS_NUM,      \
    .dfx_reg_list = g_aiv_subctrl_dfx_regs}

STATIC struct safety_irq_hw_info g_aiv_irq_hwinfo[] = {
    AIV_SAFETY_INT_HWINFO(AIVECTOR0_EMU_ID, AIV0_BASE, AIV_SAFETY_BITID_7),
    AIV_SAFETY_INT_HWINFO(AIVECTOR1_EMU_ID, AIV1_BASE, AIV_SAFETY_BITID_7),
    AIV_SAFETY_INT_HWINFO(AIVECTOR2_EMU_ID, AIV2_BASE, AIV_SAFETY_BITID_7),
    AIV_SAFETY_INT_HWINFO(AIVECTOR3_EMU_ID, AIV3_BASE, AIV_SAFETY_BITID_7),
    AIV_SAFETY_INT_HWINFO(AIVECTOR4_EMU_ID, AIV4_BASE, AIV_SAFETY_BITID_7),
    AIV_SAFETY_INT_HWINFO(AIVECTOR5_EMU_ID, AIV5_BASE, AIV_SAFETY_BITID_7),
    AIV_SAFETY_INT_HWINFO(AIVECTOR6_EMU_ID, AIV6_BASE, AIV_SAFETY_BITID_7),
    AIV_SAFETY_INT_HWINFO(AIVECTOR7_EMU_ID, AIV7_BASE, AIV_SAFETY_BITID_7),
};

#define AIV_SAFETY_IRQ_INFO(index) { \
    .dev_id = 0,                                        \
    .irq_type = SAFETY_IRQ_TYPE_LPI,                \
    .irq_name = "int_ras_safety_aiv_sub" TO_STRING(index),      \
    .safety_irq_func = tsdrv_aiv_safety_handler,     \
    .irq_hwinfo_num = 1,                                \
    .irq_hwinfo_list = &g_aiv_irq_hwinfo[index]}

STATIC struct safety_irq_info g_aiv_safety_info[] = {
    AIV_SAFETY_IRQ_INFO(0),
    AIV_SAFETY_IRQ_INFO(1),
    AIV_SAFETY_IRQ_INFO(2),
    AIV_SAFETY_IRQ_INFO(3),
    AIV_SAFETY_IRQ_INFO(4),
    AIV_SAFETY_IRQ_INFO(5),
    AIV_SAFETY_IRQ_INFO(6),
    AIV_SAFETY_IRQ_INFO(7),
};

#define AIV_COVERAGE_ITEM(_err_status, _describe, _sensor_type, _err_type) { \
    .subsys_id = DMS_DEV_TYPE_AIV, \
    .module_id = 0,               \
    .section_type = RAS_SEC_OEM,  \
    .ras_code.err_status = _err_status, \
    .describe = _describe, \
    .sensor_type = _sensor_type, \
    .error_type = _err_type \
}

const struct ras_fault_converge_item *ts_aiv_safety_converge(u32 section_type, u64 ras_code)
{
    static struct ras_fault_converge_item aiv_converage_tab[] = {
        AIV_COVERAGE_ITEM(AIV_SUBCTRL_PARITY_ERR,
            "Subctrl parity err", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_PARITY),
        AIV_COVERAGE_ITEM(AIV_SUBCTRL_ASIL_SUBCTRL_PARITY_ERR,
            "ASIL parity err", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_PARITY),
        AIV_COVERAGE_ITEM(AIV_SUBCTRL_DECODER_ASIL_VIO_APB_M0,
            "Low-level access to high-level", DMS_SEN_TYPE_SAFETY_SENSOR, RAS_ERROR_TYPE_CFG_ERR_MINIOR),
        AIV_COVERAGE_ITEM(AIV_SUBCTRL_SUBCTRL_ASIL_VIO,
            "ASIL level unauthorized", DMS_SEN_TYPE_SAFETY_SENSOR, RAS_ERROR_TYPE_CFG_ERR_MINIOR),
    };

    u32 items_num = (u32)ARRAY_SIZE(aiv_converage_tab);
    return tsdrv_safety_converge(aiv_converage_tab, items_num, section_type, ras_code);
}

int tsdrv_register_aiv_safety_irq(u32 devid)
{
    if (!tsdrv_exist_irq_in_dts(g_aiv_safety_info, AIV_NUM)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("All safety irq can't be found in dts. (devid=%u)\n", devid);
        return 0;
#endif
    }

    if (dfm_register_safety_irq_hwinfo(devid, g_aiv_safety_info, AIV_NUM) != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("register ras safety irq info failed. (devid=%d)\n", devid);
        return -EFAULT;
#endif
    }
    return 0;
}

void tsdrv_unregister_aiv_safety_irq(u32 devid)
{
    dfm_unregister_safety_irq_hwinfo(devid, g_aiv_safety_info, AIV_NUM);
    return;
}

STATIC int aiv_get_node_id(unsigned long long base_paddr, u32 *node_id)
{
    if (base_paddr < AIV0_BASE || base_paddr > AIV7_BASE) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("base addr %pK is not belong to l2buff\n", (void *)base_paddr);
        return -EEXIST;
#endif
    }

    /* Calculate the number of the aiv based on the base address. */
    *node_id = (base_paddr - AIV0_BASE) >> AIV_INTERRUPT_ADDR_SHIFT_BIT;
    return 0;
}

int tsdrv_aiv_safety_handler(struct safety_fault_info *safety_fault,
    unsigned int *event_num, struct safety_event **event_list)
{
    u32 i, j;
    struct dfm_safety_module module_info;
    struct ras_message info = {0};
    struct safety_event *fault_event = NULL;
    struct dfm_struct *dfm = tsdrv_get_dfm_struct(MDC_DEV_NODE_AIV);
    u32 node_id = 0;

    if (tsdrv_chk_safety_param(safety_fault, event_num) != 0) {
        return -EINVAL;
    }

    *event_num = 0;
    *event_list = (struct safety_event *)kmalloc(
        sizeof(struct safety_event) * AIV_SAFETY_EVENT_MAX_NUM, GFP_KERNEL);
    if ((*event_list) == NULL) {
#ifndef TSDRV_UT
        dfm_err("kmalloc safety event list failed.\n");
        return -ENOMEM;
#endif
    }

    module_info.dev_id = safety_fault->dev_id;
    module_info.node_type = DMS_DEV_TYPE_AIV;
    module_info.max_event = AIV_SAFETY_EVENT_MAX_NUM;

    for (i = 0; i < safety_fault->fault_reg_num; ++i) {
        if (aiv_get_node_id(safety_fault->fault_status_list[i].base_paddr, &node_id) != 0) {
#ifndef TSDRV_UT
            TSDRV_PRINT_ERR("Aiv get node id failed. (base addr=%pK)\n",
                (void *)safety_fault->fault_status_list[i].base_paddr);
            continue;
#endif
        }
        module_info.node_id = node_id;
        module_info.dev_node = &dfm->dev_nodes[node_id];
        module_info.section_type = RAS_SEC_OEM;
        dfm_safety_handler(&module_info, &safety_fault->fault_status_list[i], event_num, *event_list);

        for (j = 0; j < *event_num; ++j) {
            fault_event = *event_list;
            tsdrv_fill_fault_info(&fault_event[j], &info);
            if (tsdrv_send_fault_info_to_ts(safety_fault->dev_id, HISI_RPROC_TSV_TX_RPID1, &info) != 0) {
                continue;
            }
        }
    }
    return 0;
}