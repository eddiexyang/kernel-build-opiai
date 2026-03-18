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
#include "tsdrv_aic_safety.h"
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
enum aic_safety_err_irq_status {
    AIC_SUBCTRL_PARITY_ERR = 0,
    AIC_SUBCTRL_ASIL_SUBCTRL_PARITY_ERR = 1,
    AIC_SUBCTRL_INT_TSENSOR_TIMEOUT_SYNC = 5,
    AIC_SUBCTRL_DECODER_ASIL_VIO_APB_M0 = 8,
    AIC_SUBCTRL_SUBCTRL_ASIL_VIO = 12,
};
/**
 * safety fault src id
 * every fault has it's own src id
 */
#define AIC_SAFETY_SRC_ID      1
#define AIC_SAFETY_BITID_7    (7U)
#define AIC_SAFETY_BITID_23   (23U)
/**
 * aic base addr
 */
#define AIC_NUM    (10U)
#define AIC0_BASE  (0xC21C0000U)
#define AIC1_BASE  (0xC23C0000U)
#define AIC2_BASE  (0xC25C0000U)
#define AIC3_BASE  (0xC27C0000U)
#define AIC4_BASE  (0xC29C0000U)
#define AIC5_BASE  (0xC2BC0000U)
#define AIC6_BASE  (0xC2DC0000U)
#define AIC7_BASE  (0xC2FC0000U)
#define AIC8_BASE  (0xC31C0000U)
#define AIC9_BASE  (0xC33C0000U)
/**
 * int_safety_aic interrupt status offset
 */
#define AIC_SAFETY_INT_SRC      0x2C00
#define AIC_SAFETY_INT_CLR      0x2C00
#define AIC_SAFETY_INT_MASK     0x2C04

#define TO_STRING(id) #id
#define AIC_SAFETY_EVENT_MAX_NUM 32
#define AIC_INTERRUPT_ADDR_SHIFT_BIT 21
/**
 * aic fault DFX register
 */
#define AIC_SUBCTRL_DFX_REGS_NUM  (4)
static u32 g_aic_subctrl_dfx_regs[AIC_SUBCTRL_DFX_REGS_NUM] = {
    0x2c04, 0x2c08, 0x2c0c, 0x2c14
};

#define AIC_SAFETY_INT_HWINFO(subsys_id, reg_base, irq_bit) { \
    .emu_id = subsys_id, .src_id = AIC_SAFETY_SRC_ID, .bit_id = irq_bit, \
    .base_paddr = reg_base, .base_size = SZ_64K, .base_vaddr = 0, \
    .status = { AIC_SAFETY_INT_SRC, 0x1123 },    \
    .clear = { AIC_SAFETY_INT_CLR, 0x1123 },     \
    .mask = { AIC_SAFETY_INT_MASK, 0x1123 },     \
    .check_mask[0] = { DRVFAULT_FILED_VALID, 0x2c0c, 0xFFFFEEDC }, \
    .dfx_reg_num = AIC_SUBCTRL_DFX_REGS_NUM,      \
    .dfx_reg_list = g_aic_subctrl_dfx_regs}

STATIC struct safety_irq_hw_info g_aic_irq_hwinfo[] = {
    AIC_SAFETY_INT_HWINFO(AICORE0_EMU_ID, AIC0_BASE, AIC_SAFETY_BITID_7),
    AIC_SAFETY_INT_HWINFO(AICORE1_EMU_ID, AIC1_BASE, AIC_SAFETY_BITID_7),
    AIC_SAFETY_INT_HWINFO(AICORE2_EMU_ID, AIC2_BASE, AIC_SAFETY_BITID_7),
    AIC_SAFETY_INT_HWINFO(AICORE3_EMU_ID, AIC3_BASE, AIC_SAFETY_BITID_7),
    AIC_SAFETY_INT_HWINFO(AICORE4_EMU_ID, AIC4_BASE, AIC_SAFETY_BITID_7),
    AIC_SAFETY_INT_HWINFO(AICORE5_EMU_ID, AIC5_BASE, AIC_SAFETY_BITID_7),
    AIC_SAFETY_INT_HWINFO(AICORE6_EMU_ID, AIC6_BASE, AIC_SAFETY_BITID_7),
    AIC_SAFETY_INT_HWINFO(AICORE7_EMU_ID, AIC7_BASE, AIC_SAFETY_BITID_7),
    AIC_SAFETY_INT_HWINFO(AICORE8_EMU_ID, AIC8_BASE, AIC_SAFETY_BITID_7),
    AIC_SAFETY_INT_HWINFO(AICORE9_EMU_ID, AIC9_BASE, AIC_SAFETY_BITID_7),
};

#define AIC_SAFETY_IRQ_INFO(index) { \
    .dev_id = 0,                                        \
    .irq_type = SAFETY_IRQ_TYPE_LPI,                \
    .irq_name = "int_ras_safety_aic_sub" TO_STRING(index),      \
    .safety_irq_func = tsdrv_aic_safety_handler,     \
    .irq_hwinfo_num = 1,                                \
    .irq_hwinfo_list = &g_aic_irq_hwinfo[index]}

STATIC struct safety_irq_info g_aic_safety_info[] = {
    AIC_SAFETY_IRQ_INFO(0),
    AIC_SAFETY_IRQ_INFO(1),
    AIC_SAFETY_IRQ_INFO(2),
    AIC_SAFETY_IRQ_INFO(3),
    AIC_SAFETY_IRQ_INFO(4),
    AIC_SAFETY_IRQ_INFO(5),
    AIC_SAFETY_IRQ_INFO(6),
    AIC_SAFETY_IRQ_INFO(7),
    AIC_SAFETY_IRQ_INFO(8),
    AIC_SAFETY_IRQ_INFO(9),
};

#define AIC_COVERAGE_ITEM(_err_status, _describe, _sensor_type, _err_type) { \
    .subsys_id = DMS_DEV_TYPE_AIC, \
    .module_id = 0,               \
    .section_type = RAS_SEC_OEM,  \
    .ras_code.err_status = _err_status, \
    .describe = _describe, \
    .sensor_type = _sensor_type, \
    .error_type = _err_type \
}

const struct ras_fault_converge_item *ts_aic_safety_converge(u32 section_type, u64 ras_code)
{
    static struct ras_fault_converge_item aic_converage_tab[] = {
        AIC_COVERAGE_ITEM(AIC_SUBCTRL_PARITY_ERR,
            "Subctrl parity err", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_PARITY),
        AIC_COVERAGE_ITEM(AIC_SUBCTRL_ASIL_SUBCTRL_PARITY_ERR,
            "ASIL parity err", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_PARITY),
        AIC_COVERAGE_ITEM(AIC_SUBCTRL_INT_TSENSOR_TIMEOUT_SYNC,
            "Tensor timeout", DMS_SEN_TYPE_MODULE_SENSOR2, RAS_ERROR_TYPE_ERROR),
        AIC_COVERAGE_ITEM(AIC_SUBCTRL_DECODER_ASIL_VIO_APB_M0,
            "Low-level access to high-level", DMS_SEN_TYPE_SAFETY_SENSOR, RAS_ERROR_TYPE_CFG_ERR_MINIOR),
        AIC_COVERAGE_ITEM(AIC_SUBCTRL_SUBCTRL_ASIL_VIO,
            "ASIL level unauthorized", DMS_SEN_TYPE_SAFETY_SENSOR, RAS_ERROR_TYPE_CFG_ERR_MINIOR),
    };

    u32 items_num = (u32)ARRAY_SIZE(aic_converage_tab);
    return tsdrv_safety_converge(aic_converage_tab, items_num, section_type, ras_code);
}

int tsdrv_register_aic_safety_irq(u32 devid)
{
    if (!tsdrv_exist_irq_in_dts(g_aic_safety_info, AIC_NUM)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("All safety irq can't be found in dts. (devid=%u)\n", devid);
        return 0;
#endif
    }

    if (dfm_register_safety_irq_hwinfo(devid, g_aic_safety_info, AIC_NUM) != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Register ras safety irq info failed. (devid=%d)\n", devid);
        return -EFAULT;
#endif
    }
    return 0;
}

void tsdrv_unregister_aic_safety_irq(u32 devid)
{
    dfm_unregister_safety_irq_hwinfo(devid, g_aic_safety_info, AIC_NUM);
    return;
}

STATIC int aic_get_node_id(unsigned long long base_paddr, u32 *node_id)
{
    if (base_paddr < AIC0_BASE || base_paddr > AIC9_BASE) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Base addr %pK is not belong to l2buff.\n", (void *)base_paddr);
        return -EEXIST;
#endif
    }

    /* Calculate the number of the aic based on the base address. */
    *node_id = (base_paddr - AIC0_BASE) >> AIC_INTERRUPT_ADDR_SHIFT_BIT;
    return 0;
}

int tsdrv_aic_safety_handler(struct safety_fault_info *safety_fault,
    unsigned int *event_num, struct safety_event **event_list)
{
    u32 i, j;
    struct dfm_safety_module module_info;
    struct ras_message info = {0};
    struct safety_event *fault_event = NULL;
    struct dfm_struct *dfm = tsdrv_get_dfm_struct(MDC_DEV_NODE_AIC);
    u32 node_id = 0;

    if (tsdrv_chk_safety_param(safety_fault, event_num) != 0) {
        return -EINVAL;
    }

    *event_num = 0;
    *event_list = (struct safety_event *)kmalloc(
        sizeof(struct safety_event) * AIC_SAFETY_EVENT_MAX_NUM, GFP_KERNEL);
    if ((*event_list) == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("kmalloc safety event list failed.\n");
        return -ENOMEM;
#endif
    }

    module_info.dev_id = safety_fault->dev_id;
    module_info.node_type = DMS_DEV_TYPE_AIC;
    module_info.max_event = AIC_SAFETY_EVENT_MAX_NUM;

    for (i = 0; i < safety_fault->fault_reg_num; ++i) {
        if (aic_get_node_id(safety_fault->fault_status_list[i].base_paddr, &node_id) != 0) {
#ifndef TSDRV_UT
            TSDRV_PRINT_ERR("Aic get node id failed. (base addr=%pK)\n",
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
            if (tsdrv_send_fault_info_to_ts(safety_fault->dev_id, HISI_RPROC_TSC_TX_RPID1, &info) != 0) {
                continue;
            }
        }
    }
    return 0;
}
