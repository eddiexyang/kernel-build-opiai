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

#ifdef CFG_FEATURE_FAULT
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/securec.h>
#ifndef AOS_LLVM_BUILD
#include <linux/securectype.h>
#endif
#include <linux/cdev.h>
#include <linux/string.h>
#include <linux/notifier.h>
#ifndef AOS_LLVM_BUILD
#include <linux/cper.h>
#endif
#include <linux/uuid.h>
#include <linux/slab.h>
#ifndef AOS_LLVM_BUILD
#include <acpi/ghes.h>
#include <acpi/apei.h>
#endif
#include "tsdrv_fault_init.h"
#include "tsdrv_safety_fault.h"
#include "tsdrv_l2buff_safety.h"
#include "tsdrv_common_fault_platform.h"
#include "tsdrv_log.h"
#include "ascend_kernel_hal.h"
#include "drv_ipc.h"
#include "drv_log.h"
#include "devdrv_manager_comm.h"
#include "devdrv_manager.h"
#include "devdrv_ipc.h"
#include "tsdrv_ipc.h"
#include "dms_sensor_notify.h"
#include "icm_interface.h"
#include "tsmng_interface.h"
#include "tsdrv_aic_safety.h"
#include "tsdrv_aiv_safety.h"
#ifdef CFG_SOC_PLATFORM_MDC_V51
#include "drvfault_user_common.h"
#include "tsdrv_ipc_fault_rcv.h"
#include "tsdrv_ipc_fault_init.h"
#include "dfm_dev_register.h"
#include "dfm_report.h"
#endif

#define GEN_RAS_FAULT_ITEM(_subsys_id, _module_id, _sec_type, _err_status, _describe, _sensor_type, _err_type) \
    { \
        .subsys_id = _subsys_id, \
        .module_id = _module_id, \
        .section_type = _sec_type, \
        .ras_code.err_status = _err_status, \
        .describe = _describe, \
        .sensor_type = _sensor_type, \
        .error_type = _err_type \
    }

#ifdef CFG_FEATURE_FAULT_FPDC
STATIC struct ras_fault_converge_item fault_converge_table[] = {
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, SENSOR_NODE_L2BUF, RAS_SEC_OEM, RAS_SRAM_MULTI_BIT_ECC_UER,
        "multi bit ecc error", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_MBECC),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, SENSOR_NODE_L2BUF, RAS_SEC_OEM, RAS_SRAM_SINGLE_BIT_ECC_CE,
        "single bit ecc error", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_SBECCOverThold),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, SENSOR_NODE_L2BUF, RAS_SEC_OEM, RAS_L2BUF_CFG_UEO_0,
        "l2buff cfg error 0", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, SENSOR_NODE_L2BUF, RAS_SEC_OEM, RAS_L2BUF_CFG_UEU_1,
        "l2buff cfg error 1", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, SENSOR_NODE_L2BUF, RAS_SEC_OEM, RAS_L2BUF_CFG_UEU_2,
        "l2buff cfg error 2", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, SENSOR_NODE_L2BUF, RAS_SEC_OEM, RAS_L2BUF_CFG_UEU_3,
        "l2buff cfg error 3", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, SENSOR_NODE_L2BUF, RAS_SEC_OEM, RAS_L2BUF_CFG_UEU_4,
        "l2buff cfg error 4", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, SENSOR_NODE_L2BUF, RAS_SEC_OEM, RAS_L2BUF_CFG_UEO_5,
        "l2buff cfg error 5", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, SENSOR_NODE_L2BUF, RAS_SEC_OEM, RAS_L2BUF_CFG_UER_6,
        "l2buff cfg error 6", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, SENSOR_NODE_L2BUF, RAS_SEC_OEM, RAS_L2BUF_CFG_UER_7,
        "l2buff cfg error 7", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, SENSOR_NODE_L2BUF, RAS_SEC_OEM, RAS_L2BUF_CFG_UER_8,
        "l2buff cfg error 8", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, SENSOR_NODE_L2BUF, RAS_SEC_OEM, RAS_L2BUF_CFG_UER_9,
        "l2buff cfg error 9", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, SENSOR_NODE_L2BUF, RAS_SEC_OEM, RAS_L2BUF_CFG_UER_10,
        "l2buff cfg error 10", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
#ifdef CFG_SOC_PLATFORM_MINIV3
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, SENSOR_NODE_L2BUF, RAS_SEC_OEM, RAS_L2BUF_CFG_UER_10,
        "l2buff cfg error 11", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, SENSOR_NODE_L2BUF, RAS_SEC_OEM, RAS_L2BUF_CFG_UER_10,
        "l2buff cfg error 12", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, SENSOR_NODE_L2BUF, RAS_SEC_OEM, RAS_L2BUF_CFG_UER_10,
        "l2buff cfg error 13", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, SENSOR_NODE_L2BUF, RAS_SEC_OEM, RAS_L2BUF_CFG_UER_10,
        "l2buff cfg error 14", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_L2BUF, SENSOR_NODE_L2BUF, RAS_SEC_OEM, RAS_L2BUF_CFG_UER_10,
        "l2buff cfg error 15", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
    GEN_RAS_FAULT_ITEM(DMS_DEV_TYPE_TSCPU, SENSOR_NODE_A55, RAS_SEC_OEM, RAS_SRAM_MULTI_BIT_ECC_UER,
        "multi bit ecc error", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_MBECC),
#endif
};
#endif

STATIC struct tsdrv_fault_ctrl fault_ctrl;

STATIC void fault_ctrl_set_num(void)
{
    fault_ctrl.dev_num++;
}

u32 fault_ctrl_get_num(void)
{
    return fault_ctrl.dev_num;
}

struct fault_dev *tsdrv_get_fault_dev(u32 devid)
{
    return &fault_ctrl.fault_dev[devid];
}
EXPORT_SYMBOL_UNRELEASE(tsdrv_get_fault_dev);

STATIC int tsdrv_node_ops_init(struct dms_node *device)
{
    TSDRV_PRINT_DEBUG("Debug. (node_name=%s)\n", device->node_name);
    return 0;
}

STATIC void tsdrv_node_ops_uninit(struct dms_node *device)
{
    TSDRV_PRINT_DEBUG("Debug. (node_name=%s)\n", device->node_name);
    return;
}

u32 get_devnode_id_ipc(DMS_DEVICE_NODE_TYPE node_type)
{
    u32 node_id;

    switch (node_type) {
#ifndef CFG_SOC_PLATFORM_MDC_V51
        case DMS_DEV_TYPE_TS:
            node_id = DEV_NODE_TS;
            break;
#else
        case DMS_DEV_TYPE_TS:
#endif
        case DMS_DEV_TYPE_HWTS_S_TS:
        case DMS_DEV_TYPE_TSCPU:
            node_id = DEV_NODE_HWTS;
            break;
        case DMS_DEV_TYPE_AIC:
            node_id = DEV_NODE_AIC;
            break;
        case DMS_DEV_TYPE_AIV:
            node_id = DEV_NODE_AIV;
            break;
        case DMS_DEV_TYPE_SDMA:
            node_id = DEV_NODE_SDMAM;
            break;
#ifndef CFG_SOC_PLATFORM_MDC_V51
        case DMS_DEV_TYPE_DSA:
            node_id = DEV_NODE_DSA;
            break;
#endif
        default:
            node_id = DEV_NODE_ID_MAX;
            break;
    }

    return node_id;
}

u32 get_sensor_node_id_ipc(DMS_DEVICE_NODE_TYPE node_type)
{
    u32 sensor_node_id;

    switch (node_type) {
#ifndef CFG_SOC_PLATFORM_MDC_V51
        case DMS_DEV_TYPE_TS:
            sensor_node_id = SENSOR_NODE_TS;
            break;
#else
        case DMS_DEV_TYPE_TS:
#endif
        case DMS_DEV_TYPE_HWTS_S_TS:
        case DMS_DEV_TYPE_TSCPU:
            sensor_node_id = SENSOR_NODE_HWTS;
            break;
        case DMS_DEV_TYPE_AIC:
            sensor_node_id = SENSOR_NODE_AIC;
            break;
        case DMS_DEV_TYPE_AIV:
            sensor_node_id = SENSOR_NODE_AIV;
            break;
        case DMS_DEV_TYPE_SDMA:
            sensor_node_id = SENSOR_NODE_SDMAM;
            break;
        case DMS_DEV_TYPE_DSA:
            sensor_node_id = SENSOR_NODE_DSA;
            break;
        default:
            sensor_node_id = SENSOR_NODE_ID_MAX;
            break;
    }

    return sensor_node_id;
}

u32 get_sensor_node_id_fpdc(DMS_DEVICE_NODE_TYPE node_type)
{
    u32 sensor_node_id = SENSOR_NODE_ID_MAX;

    switch (node_type) {
        case DMS_DEV_TYPE_TSCPU:
            return SENSOR_NODE_A55;
        case DMS_DEV_TYPE_L2BUF:
            return SENSOR_NODE_L2BUF;
        default:
            break;
    }
    return sensor_node_id;
}

#ifdef CFG_FEATURE_FAULT_FPDC

u32 tsdrv_get_section_type(DMS_DEVICE_NODE_TYPE node_type)
{
    if (node_type == DMS_DEV_TYPE_L2BUF || node_type == DMS_DEV_TYPE_TSCPU) {
        return RAS_SEC_OEM;
    } else {
        return RAS_SEC_OTHER;
    }
}

STATIC u32 tsdrv_get_section_ras_code(struct ras_fault_converge_item *fault_info)
{
    u32 ras_code;

    switch (fault_info->section_type) {
        case RAS_SEC_ARM:
            ras_code = fault_info->ras_code.int_status;
            break;
        case RAS_SEC_OEM:
            ras_code = lower_32_bits(fault_info->ras_code.err_status);
            break;
        case RAS_SEC_MEM:
            ras_code = fault_info->ras_code.err_type;
            break;
        default:
            ras_code = fault_info->ras_code.int_status;
            break;
    }

    return ras_code;
}
#endif

#ifdef CFG_SOC_PLATFORM_MDC_V51
STATIC int tsdrv_get_heartbeat_fault_event(dfm_event *event)
{
    const char *fault_info = "tscpu heartbeat";
    int ret;
    event->subsys_id = DMS_DEV_TYPE_TSCPU;
    event->module_id = DMS_DEV_TYPE_TSCPU;
    event->section_type = RAS_SEC_OTHER;
    ret = strcpy_s((char *)&event->describe[0], DMS_MAX_EVENT_DATA_LENGTH, fault_info);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Copy fail. (ret=%d)\n", ret);
        return ret;
    }
    event->sensor_type = DMS_SEN_TYPE_HEARTBEAT;
    event->error_type = 0;
    return 0;
}
#endif

int tsdrv_heartbeat_fault_event_add(u32 devid, u32 tsid)
{
#ifndef CFG_SOC_PLATFORM_MDC_V51
    return 0;
#else
    int ret;
    dfm_event event = {0};
    u32 node_id = (tsid == 0) ? 0 : 1;
    struct dfm_struct *dfm = tsdrv_get_dfm_struct(MDC_DEV_NODE_TSCPU);
    ret = tsdrv_get_heartbeat_fault_event(&event);
    if (ret != 0) {
        return ret;
    }
    ret = dfm_add_event(dfm, node_id, &event);
    if ((ret != 0) && (ret != -EEXIST)) {
        TSDRV_PRINT_ERR("Process fault event info failed. (devid=%u; node_id=%u; ret=%d)\n",
            devid, node_id, ret);
        return ret;
    }
    return 0;
#endif
}

#ifdef CFG_FEATURE_FAULT_FPDC
const struct ras_fault_converge_item *tsdrv_fault_converge(u32 section_type, u64 ras_code)
{
    int fault_item_len = (int)ARRAY_SIZE(fault_converge_table);
    int i;

    for (i = 0; i < fault_item_len; i++) {
        if (section_type == fault_converge_table[i].section_type) {
            if ((u32)ras_code == tsdrv_get_section_ras_code(&fault_converge_table[i])) {
                return &fault_converge_table[i];
            }
        }
    }
    return NULL;
}
#endif

#ifdef CFG_FEATURE_FAULT_FPDC

u32 tsdrv_parse_ras_code(const struct notify_data *pdata)
{
    struct sec_oem_error *pdata_oem = (struct sec_oem_error *)(pdata->origin_data);
    u32 ras_code = RAS_ERR_CODE_INVAILD;

    if (pdata_oem != NULL) {
        ras_code = pdata_oem->err_status_l & OEM_ERR_STATUS_MASK;
    }
    return ras_code;
}

#endif


int tsdrv_fault_event_scan(u64 private_data, struct dms_sensor_event_data *data)
{
    return 0;
}
struct dms_node_operations tsdrv_dev_node_ops = {
    .init = tsdrv_node_ops_init,
    .uninit = tsdrv_node_ops_uninit,
    .get_info_list = NULL,
    .get_state = NULL,
    .get_capacity = NULL,
    .set_power_state = NULL,
    .scan = NULL,
    .fault_diag = NULL,
    .event_notify = NULL,
    .get_link_state = NULL,
    .set_link_state = NULL
};

int tsdrv_fault_dev_init(u32 devid)
{
    int ret;
    fault_ctrl_set_num();
    ret = tsdrv_register_safety_irq(devid);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to initialize the TSDRV fault manager device. (devid=%u)\n", devid);
    } else {
        TSDRV_PRINT_INFO("TSDRV fault manager device init successfully. (devid=%u)\n", devid);
    }
    return ret;
}

void tsdrv_fault_dev_exit(u32 devid)
{
    tsdrv_unregister_safety_irq(devid);
}

void tsdrv_fault_init(void)
{
#ifndef CFG_SOC_PLATFORM_MDC_V51
    return 0;
#else
    u32 cmd_type = IPCDRV_TS_ADV_CMDTYPE_ALARM_MSG_REPORT;
    int ret = tsdrv_ipc_handler_register(cmd_type, tsdrv_ipc_fault_event_handler);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to register ipc fault handle. (ret=%d)\n", ret);
    }
#endif
}

void tsdrv_fault_exit(void)
{
#ifndef CFG_SOC_PLATFORM_MDC_V51
    return 0;
#else
    u32 cmd_type = IPCDRV_TS_ADV_CMDTYPE_ALARM_MSG_REPORT;
    tsdrv_ipc_handler_unregister(cmd_type);
#endif
}

bool tsdrv_exist_irq_in_dts(struct safety_irq_info *irq_infos, unsigned int num)
{
#ifndef TSDRV_UT
    u32 i;
    /* If the irq has not configured in dts,
     * maybe not support in current enviroment */
    for (i = 0; i < num; ++i) {
        if (drvfault_find_irq_in_dts(irq_infos[i].irq_name) == 0) {
            TSDRV_PRINT_INFO("irq can be found in dts.(irq name=%s)\n", irq_infos[i].irq_name);
            return true;
        }
    }
    return false;
#else
    return true;
#endif
}

void tsdrv_fill_fault_info(struct safety_event *fault_event, struct ras_message *info)
{
    info->node_type = fault_event->node_type;
    info->node_id = fault_event->node_id;
    info->sensor_type = fault_event->sensor_type;
    info->sensor_status = fault_event->event_type;
    return;
}

int tsdrv_send_fault_info_to_ts(u32 devid, rproc_id_t rproc_id, struct ras_message *info)
{
    int ret;
    rproc_msg_t msg[IPCDRV_RPROC_MSG_LENGTH];
    struct ras_message *payload = NULL;
    struct ipcdrv_message *ipc_msg = NULL;

    ipc_msg = (struct ipcdrv_message *)msg;
    ipc_msg->ipc_msg_header.msg_type = MSGTYPE_DRIVER_SEND;
    ipc_msg->ipc_msg_header.cmd_type = IPCDRV_TS_AIC_AIV_FAULT_INFO;
    ipc_msg->ipc_msg_header.sync_type = IPCDRV_MSG_ASYNC;
    ipc_msg->ipc_msg_header.reserved = 0;
    ipc_msg->ipc_msg_header.msg_length = sizeof(struct ras_message);
    ipc_msg->ipc_msg_header.msg_index = 0;
    payload = (struct ras_message *)ipc_msg->ipcdrv_payload;
    ret = memcpy_s(payload, IPCDRV_MSG_LENGTH, info, sizeof(struct ras_message));
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Memcpy_s failed. (ret=%d)\n", ret);
        return ret;
#endif
    }

    ret = icm_msg_send_async(ICM_FD_BUILD(devid, rproc_id), msg, IPCDRV_RPROC_MSG_LENGTH);
#ifndef TSDRV_UT
    if (ret != 0) {
        TSDRV_PRINT_ERR("Icm_msg_send_async failed. (ret=%d)\n", ret);
    } else {
        TSDRV_PRINT_INFO("Send msg to tsfw. (node_type=0x%x; node_id=0x%x; sensor_type=0x%x; "
            "sensor_status=0x%x)\n", (u32)info->node_type, (u32)info->node_id, (u32)info->sensor_type,
            (u32)info->sensor_status);
    }
#endif
    return ret;
}
#endif /* CFG_FEATURE_FAULT */
