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
 * Create: 2023-02-06
 */
#include <linux/slab.h>
#include "dms_define.h"
#include "dfm_dev_register.h"
#include "dms_sensor_type.h"
#include "drv_ras_common.h"
#include "dfm_report.h"
#include "fpdc.h"
#include "tsdrv_fault_init.h"
#include "tsdrv_subsys_safety.h"
#include "tsdrv_log.h"
#include "dms_node_type.h"
#include "fpdc_ras_receiver.h"
#include "drvfault_user_common.h"
#include "tsdrv_safety_fault.h"
#include "dfm_safety_report.h"
#include "tsdrv_ipc_fault_init.h"
#include "tsdrv_subsys_safety_ras.h"
/**
 * Raw secure interrupt register status,
 * Each error status corresponds to one bit of the register.
 */
enum ts_safety_ras_err_ecc_bit {
    FATAL_MEMORY_ERR = 1,
    FATAL_DATA_PAYLOAD_BUS_ERR = 3,
    FATAL_READY_SIGNAL_BUS_ERR = 4,
    FATAL_NON_PROTOCOL_BUS_ERR = 5,
    FATAL_PROTOCOL_BUS_ERR = 6,
    FATAL_MEMORY_NO_FLASH_ERR = 10,
    CORRECTABLE_FLASH_MEMORY_ERR = 11,
    FATAL_FLASH_MEMORY_ERR = 12,
    TIMEOUT_FOR_MAIN_AXIM_BUS = 13,
    TIMEOUT_FOR_FLASH_BUS = 14,
    TIMEOUT_FOR_LLPP_BUS = 15,
    UNDEFINED_EXCEPTION = 22,
    PROCESSOR_LIVELOCK = 24,
    P_CHANNEL_LOCKSTEP_COMP_ERR = 26,
};

enum ts_safety_ras_err_lockstep_bit {
    CORE_TO_TOP_LEVEL_LOGIC_PDIF = 0,
    CORE_TO_TOP_LEVEL_LOGIC_SDIF = 1,
    CORE_TO_GOVERNOR_PDIF = 2,
    CORE_TO_GOVERNOR_SDIF = 3,
    CORE_TO_RAM_PDIF = 4,
    CORE_TO_RAM_SDIF = 5,
    GOVERNOR_TO_TOP_LEVEL_LOGIC_PDIF = 6,
    GOVERNOR_TO_TOP_LEVEL_LOGIC_SDIF = 7,
    GOVERNOR_TO_CORE_PDIF = 8,
    GOVERNOR_TO_CORE_SDIF = 9,
    NON_COMPARED_CORE_SIGNALS_PDIF = 10,
    NON_COMPARED_CORE_SIGNALS_SDIF = 11,
    NON_COMPARED_GOVERNOR_PDIF = 12,
    NON_COMPARED_GOVERNOR_SDIF = 13,
    GLOBAL_GOVERNOR_TO_TOP_LEVEL_PDIF = 16,
    GLOBAL_GOVERNOR_TO_TOP_LEVEL_SDIF = 17,
    GLOBAL_GOVERNOR_TO_CORE_PDIF = 18,
    GLOBAL_GOVERNOR_TO_CORE_SDIF = 19,
};

#define TSCPU_NUM                  (2U)
#define TSCPU_REGISTER_ARRAY_SIZE  (2U)
#define TS_SUBSYS_SAFETY_MAX_NUM   (32U)
#define TS_SUBSYS_REG_OFFSET0      (0x718)
#define TS_SUBSYS_REG_OFFSET1      (0x71C)
#define TS_SUBSYS_INT_SAFETY       (0xFFU)
/**
 * safety fault src id
 * every fault has it's own src id
 */
#define TS_SUBSYS_SAFETY_SRC_ID    (1U)
#define TS_SUBSYS_SAFETY_BITID_0   (0U)
// match to bios sdei table
#define TS_ECC_REG_INDEX           (0U)  // reg_name:SC_ERRSTAT198 0x718
#define TS_LOCK_STEP_REG_INDEX     (1U)  // reg_name:SC_ERRSTAT199 0x71C

#define TS_SUBSYS_SAFETY_RAS_COVERAGE_ITEM(_err_status, _describe, _sensor_type, _err_type) { \
    .subsys_id = DMS_DEV_TYPE_TSCPU, \
    .module_id = 0,               \
    .section_type = TS_SUBSYS_INT_SAFETY,  \
    .ras_code.err_status = _err_status, \
    .describe = _describe, \
    .sensor_type = _sensor_type, \
    .error_type = _err_type \
}

#define SAFETY_INT_STATUS(status_offset, safety_bit)  (((u32)status_offset << 8) | safety_bit)

const struct ras_fault_converge_item *ts_subsys_safety_ras_converge(
    const struct safety_fault_status *fault_status, u32 section_type, u64 ras_code)
{
    static struct ras_fault_converge_item ts_subsys_converage_tab[] = {
        /* Group 0 */
        TS_SUBSYS_SAFETY_RAS_COVERAGE_ITEM(SAFETY_INT_STATUS(0x718, FATAL_MEMORY_ERR),
            "fatal memory err", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_MBECC),
        TS_SUBSYS_SAFETY_RAS_COVERAGE_ITEM(SAFETY_INT_STATUS(0x718, FATAL_DATA_PAYLOAD_BUS_ERR),
            "fatal data payload bus err", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_MBECC),
        TS_SUBSYS_SAFETY_RAS_COVERAGE_ITEM(SAFETY_INT_STATUS(0x718, FATAL_READY_SIGNAL_BUS_ERR),
            "fatal ready signal bus err", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_BUS_ERR),
        TS_SUBSYS_SAFETY_RAS_COVERAGE_ITEM(SAFETY_INT_STATUS(0x718, FATAL_NON_PROTOCOL_BUS_ERR),
            "fatal non-protocol bus err", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_BUS_ERR),
        TS_SUBSYS_SAFETY_RAS_COVERAGE_ITEM(SAFETY_INT_STATUS(0x718, FATAL_PROTOCOL_BUS_ERR),
            "fatal protocol bus err", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_BUS_ERR),
        TS_SUBSYS_SAFETY_RAS_COVERAGE_ITEM(SAFETY_INT_STATUS(0x718, FATAL_MEMORY_NO_FLASH_ERR),
            "fatal memory err", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_MBECC),
        TS_SUBSYS_SAFETY_RAS_COVERAGE_ITEM(SAFETY_INT_STATUS(0x718, CORRECTABLE_FLASH_MEMORY_ERR),
            "correctable flash memory err", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_MBECC),
        TS_SUBSYS_SAFETY_RAS_COVERAGE_ITEM(SAFETY_INT_STATUS(0x718, FATAL_FLASH_MEMORY_ERR),
            "fatal flash memory err", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_MBECC),
        TS_SUBSYS_SAFETY_RAS_COVERAGE_ITEM(SAFETY_INT_STATUS(0x718, TIMEOUT_FOR_MAIN_AXIM_BUS),
            "timeout for main AXIM bus", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_BUS_ERR),
        TS_SUBSYS_SAFETY_RAS_COVERAGE_ITEM(SAFETY_INT_STATUS(0x718, TIMEOUT_FOR_FLASH_BUS),
            "timeout for Flash bus", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_BUS_ERR),
        TS_SUBSYS_SAFETY_RAS_COVERAGE_ITEM(SAFETY_INT_STATUS(0x718, TIMEOUT_FOR_LLPP_BUS),
            "timeout for LLPP bus", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_BUS_ERR),
        TS_SUBSYS_SAFETY_RAS_COVERAGE_ITEM(SAFETY_INT_STATUS(0x718, UNDEFINED_EXCEPTION),
            "undefined exception", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_ERROR),
        TS_SUBSYS_SAFETY_RAS_COVERAGE_ITEM(SAFETY_INT_STATUS(0x718, PROCESSOR_LIVELOCK),
            "processor livelock", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_ERROR),
        TS_SUBSYS_SAFETY_RAS_COVERAGE_ITEM(SAFETY_INT_STATUS(0x718, P_CHANNEL_LOCKSTEP_COMP_ERR),
            "R52 P CHANNEL LOCKSTEP err", DMS_SEN_TYPE_SAFETY_SENSOR, SOC_SAFETY_LOCKSTEP_ERR),
        /* Group 1 */
        TS_SUBSYS_SAFETY_RAS_COVERAGE_ITEM(SAFETY_INT_STATUS(0x71C, CORE_TO_TOP_LEVEL_LOGIC_PDIF),
            "core to top-level", DMS_SEN_TYPE_SAFETY_SENSOR, SOC_SAFETY_LOCKSTEP_ERR),
        TS_SUBSYS_SAFETY_RAS_COVERAGE_ITEM(SAFETY_INT_STATUS(0x71C, CORE_TO_TOP_LEVEL_LOGIC_SDIF),
            "core to top-level", DMS_SEN_TYPE_SAFETY_SENSOR, SOC_SAFETY_LOCKSTEP_ERR),
        TS_SUBSYS_SAFETY_RAS_COVERAGE_ITEM(SAFETY_INT_STATUS(0x71C, CORE_TO_GOVERNOR_PDIF),
            "core to governor", DMS_SEN_TYPE_SAFETY_SENSOR, SOC_SAFETY_LOCKSTEP_ERR),
        TS_SUBSYS_SAFETY_RAS_COVERAGE_ITEM(SAFETY_INT_STATUS(0x71C, CORE_TO_GOVERNOR_SDIF),
            "core to governor", DMS_SEN_TYPE_SAFETY_SENSOR, SOC_SAFETY_LOCKSTEP_ERR),
        TS_SUBSYS_SAFETY_RAS_COVERAGE_ITEM(SAFETY_INT_STATUS(0x71C, CORE_TO_RAM_PDIF),
            "core to RAM", DMS_SEN_TYPE_SAFETY_SENSOR, SOC_SAFETY_LOCKSTEP_ERR),
        TS_SUBSYS_SAFETY_RAS_COVERAGE_ITEM(SAFETY_INT_STATUS(0x71C, CORE_TO_RAM_SDIF),
            "core to RAM", DMS_SEN_TYPE_SAFETY_SENSOR, SOC_SAFETY_LOCKSTEP_ERR),
        TS_SUBSYS_SAFETY_RAS_COVERAGE_ITEM(SAFETY_INT_STATUS(0x71C, GOVERNOR_TO_TOP_LEVEL_LOGIC_PDIF),
            "governor to top-level", DMS_SEN_TYPE_SAFETY_SENSOR, SOC_SAFETY_LOCKSTEP_ERR),
        TS_SUBSYS_SAFETY_RAS_COVERAGE_ITEM(SAFETY_INT_STATUS(0x71C, GOVERNOR_TO_TOP_LEVEL_LOGIC_SDIF),
            "governor to top-level", DMS_SEN_TYPE_SAFETY_SENSOR, SOC_SAFETY_LOCKSTEP_ERR),
        TS_SUBSYS_SAFETY_RAS_COVERAGE_ITEM(SAFETY_INT_STATUS(0x71C, GOVERNOR_TO_CORE_PDIF),
            "governor to core", DMS_SEN_TYPE_SAFETY_SENSOR, SOC_SAFETY_LOCKSTEP_ERR),
        TS_SUBSYS_SAFETY_RAS_COVERAGE_ITEM(SAFETY_INT_STATUS(0x71C, GOVERNOR_TO_CORE_SDIF),
            "governor to core", DMS_SEN_TYPE_SAFETY_SENSOR, SOC_SAFETY_LOCKSTEP_ERR),
        TS_SUBSYS_SAFETY_RAS_COVERAGE_ITEM(SAFETY_INT_STATUS(0x71C, NON_COMPARED_CORE_SIGNALS_PDIF),
            "non-compared core signals", DMS_SEN_TYPE_SAFETY_SENSOR, SOC_SAFETY_LOCKSTEP_ERR),
        TS_SUBSYS_SAFETY_RAS_COVERAGE_ITEM(SAFETY_INT_STATUS(0x71C, NON_COMPARED_CORE_SIGNALS_SDIF),
            "non-compared core signals", DMS_SEN_TYPE_SAFETY_SENSOR, SOC_SAFETY_LOCKSTEP_ERR),
        TS_SUBSYS_SAFETY_RAS_COVERAGE_ITEM(SAFETY_INT_STATUS(0x71C, NON_COMPARED_GOVERNOR_PDIF),
            "non-compared governor", DMS_SEN_TYPE_SAFETY_SENSOR, SOC_SAFETY_LOCKSTEP_ERR),
        TS_SUBSYS_SAFETY_RAS_COVERAGE_ITEM(SAFETY_INT_STATUS(0x71C, NON_COMPARED_GOVERNOR_SDIF),
            "non-compared governor", DMS_SEN_TYPE_SAFETY_SENSOR, SOC_SAFETY_LOCKSTEP_ERR),
        TS_SUBSYS_SAFETY_RAS_COVERAGE_ITEM(SAFETY_INT_STATUS(0x71C, GLOBAL_GOVERNOR_TO_TOP_LEVEL_PDIF),
            "global governor to top-level", DMS_SEN_TYPE_SAFETY_SENSOR, SOC_SAFETY_LOCKSTEP_ERR),
        TS_SUBSYS_SAFETY_RAS_COVERAGE_ITEM(SAFETY_INT_STATUS(0x71C, GLOBAL_GOVERNOR_TO_TOP_LEVEL_SDIF),
            "global governor to top-level", DMS_SEN_TYPE_SAFETY_SENSOR, SOC_SAFETY_LOCKSTEP_ERR),
        TS_SUBSYS_SAFETY_RAS_COVERAGE_ITEM(SAFETY_INT_STATUS(0x71C, GLOBAL_GOVERNOR_TO_CORE_PDIF),
            "global governor to core", DMS_SEN_TYPE_SAFETY_SENSOR, SOC_SAFETY_LOCKSTEP_ERR),
        TS_SUBSYS_SAFETY_RAS_COVERAGE_ITEM(SAFETY_INT_STATUS(0x71C, GLOBAL_GOVERNOR_TO_CORE_SDIF),
            "global governor to core", DMS_SEN_TYPE_SAFETY_SENSOR, SOC_SAFETY_LOCKSTEP_ERR),
};
    /* Find the converge item */
    u32 items_num = (u32)ARRAY_SIZE(ts_subsys_converage_tab);
    return tsdrv_safety_converge(ts_subsys_converage_tab, items_num,
        section_type, SAFETY_INT_STATUS(fault_status->fault_status_offset, ras_code));
}

STATIC int tsdrv_subsys_check_notify_data(const struct notify_data *pdata)
{
    hisi_common_error_info *perrorinfo = NULL;
    if ((pdata == NULL) || (pdata->origin_data == NULL)) {
        TSDRV_PRINT_ERR("Notify data is NULL\n");
        return -EINVAL;
    }
    if (pdata->node_type != DMS_DEV_TYPE_TSCPU) {
        TSDRV_PRINT_ERR("Invalid paramters. (node_type=%u)\n", pdata->node_type);
        return -EINVAL;
    }
    if (pdata->data_len != sizeof(hisi_common_error_info)) {
        TSDRV_PRINT_ERR("Ras data len is invalid. (len=%u)\n", pdata->data_len);
        return -EINVAL;
    }
    perrorinfo = (hisi_common_error_info *)pdata->origin_data;
    if (perrorinfo->module_id != (u8)HISI_MODULE_CPU_TS) {
        TSDRV_PRINT_ERR("Invalid module id. (moduleid=%d)\n", perrorinfo->module_id);
        return -EINVAL;
    }
    if (perrorinfo->register_array_size < (sizeof(u32) * TSCPU_REGISTER_ARRAY_SIZE)) {
        TSDRV_PRINT_ERR("Invalid array size. (size=%u)\n", perrorinfo->register_array_size);
        return -EINVAL;
    }
    if (perrorinfo->device_or_core_id >= TSCPU_NUM) {
        TSDRV_PRINT_ERR("Invalid device or core id. (id=%u)\n", perrorinfo->device_or_core_id);
        return -EINVAL;
    }
    return 0;
}

STATIC void tsdrv_subsys_fill_fault_list(struct safety_fault_status *fault_info, u32 emu_id,
    u32 offset, u32 status)
{
    fault_info->emu_id = emu_id;
    fault_info->src_id = TS_SUBSYS_SAFETY_SRC_ID;
    fault_info->bit_id = TS_SUBSYS_SAFETY_BITID_0;
    fault_info->fault_status_offset = offset;
    fault_info->fault_status = status;
    return;
}

STATIC void tsdrv_subsys_fill_module_info(struct dfm_safety_module *module_info, u32 node_id)
{
    struct dfm_struct *dfm = tsdrv_get_dfm_struct(MDC_DEV_NODE_TSCPU);
    module_info->dev_node = &dfm->dev_nodes[node_id];
    module_info->dev_id = 0;
    module_info->node_id = node_id;
    module_info->section_type = TS_SUBSYS_INT_SAFETY;
    module_info->node_type = DMS_DEV_TYPE_TSCPU;
    module_info->max_event = TS_SUBSYS_SAFETY_MAX_NUM;
    return;
}

void tsdrv_subsys_ras_fpdc_handler(const struct notify_data *pdata)
{
    u32 i;
    u32 node_id, emu_id;
    struct dfm_safety_module module_info;
    struct safety_fault_status fault_status_list[TSCPU_REGISTER_ARRAY_SIZE];
    struct safety_event *event_list = NULL;
    hisi_common_error_info *hisi_err = NULL;
    unsigned int event_num = 0;
    u32 offset[TSCPU_REGISTER_ARRAY_SIZE] = { TS_SUBSYS_REG_OFFSET0, TS_SUBSYS_REG_OFFSET1 };
    if (tsdrv_subsys_check_notify_data(pdata) != 0) {
        return;
    }

    event_list = (struct safety_event *)kmalloc(
        sizeof(struct safety_event) * TS_SUBSYS_SAFETY_MAX_NUM, GFP_KERNEL);
    if (event_list == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Kmalloc safety event list failed.\n");
        return;
#endif
    }
    hisi_err = (hisi_common_error_info *)pdata->origin_data;
    node_id = hisi_err->device_or_core_id;
    emu_id = (node_id == 0) ? TSC_SUB_EMU_ID : TSV_SUB_EMU_ID;
    TSDRV_PRINT_INFO("Ts subsys reg info. (module_id=%u, node_id=%u, reg_val0=0x%x, reg_val1=0x%x)\n",
        hisi_err->module_id, node_id, hisi_err->register_array[TS_ECC_REG_INDEX],
        hisi_err->register_array[TS_LOCK_STEP_REG_INDEX]);
    tsdrv_subsys_fill_module_info(&module_info, node_id);

    for (i = 0; i < TSCPU_REGISTER_ARRAY_SIZE; i++) {
        tsdrv_subsys_fill_fault_list(&fault_status_list[i], emu_id, offset[i], hisi_err->register_array[i]);
        dfm_safety_handler(&module_info, &fault_status_list[i], &event_num, event_list);
    }

    drvfault_send_safety_info_to_sils(event_num, event_list);

    if (event_list != NULL) {
        kfree(event_list);
        event_list = NULL;
    }
    return;
}
