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
* Create: 2023-05-29
*/
#ifdef CFG_FEATURE_FAULT_FPDC
#include <linux/sizes.h>
#include <linux/slab.h>

#include "tsdrv_fault_init.h"
#include "tsdrv_common_fault_platform.h"
#include "tsdrv_fault_platform.h"
#include "tsdrv_fault_comm.h"
#include "tsdrv_common.h"
#include "tsdrv_log.h"
#include "dms_node_type.h"
#include "dms_sensor_type.h"
#include "fpdc_ras_receiver.h"
#include "fpdc.h"
#include "drvfault_user_common.h"
#include "dfm_dev_register.h"
#include "dfm_report.h"
#include "dfm_safety_report.h"
#include "drv_ras_common.h"
#include "tsdrv_ras.h"

static u32 tsdrv_get_section_ras_code(struct ras_fault_converge_item *fault_info)
{
    u32 ras_code;

    switch (fault_info->section_type) {
        case RAS_SEC_ARM:
            ras_code = fault_info->ras_code.int_status;
            break;
        case RAS_SEC_GENERIC:
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

static const struct ras_fault_converge_item *tsdrv_fault_converge(struct ras_error *error)
{
    struct ras_fault_converge_item *converge_table = tsdrv_get_fault_converge_table();
    int fault_item_len = tsdrv_get_fault_converge_table_size();
    int i;

    for (i = 0; i < fault_item_len; i++) {
        if (error->node_type == converge_table[i].subsys_id) {
            if ((error->section_type == converge_table[i].section_type) &&
                (error->ras_code == tsdrv_get_section_ras_code(&converge_table[i]))) {
                    return &converge_table[i];
            }
        }
    }
    return NULL;
}

static void tsdrv_update_node_type(const struct notify_data *pdata, struct ras_error *error)
{
    hisi_common_error_info *pdata_common = (hisi_common_error_info *)pdata->origin_data;
    if (pdata_common->module_id == HISI_MODULE_DISP) {
        if (pdata_common->sub_sys_id == HISI_SUBSYS_AICORESUB) {
            error->node_type = DMS_DEV_TYPE_AIC_DISP;
        }
        if (pdata_common->sub_sys_id == HISI_SUBSYS_TSSUB) {
            error->node_type = DMS_DEV_TYPE_TS_DISP;
        }
        if (pdata_common->sub_sys_id == HISI_SUBSYS_DSASUB) {
            error->node_type = DMS_DEV_TYPE_DSA_DISP;
        }
    }
}

static void tsdrv_set_submodule_id(const struct notify_data *pdata, struct ras_error *error)
{
    hisi_common_error_info *pdata_common = (hisi_common_error_info *)pdata->origin_data;
    error->node_id = 0;
    if ((error->node_type == DMS_DEV_TYPE_L2BUF) || (error->node_type == DMS_DEV_TYPE_TSCPU)) {
        error->node_id = pdata_common->device_or_core_id;
    }
    if ((error->node_type == DMS_DEV_TYPE_AIC_DISP) || (error->node_type == DMS_DEV_TYPE_TS_DISP) ||
        (error->node_type == DMS_DEV_TYPE_DSA_DISP)) {
        error->node_id = pdata_common->sub_sys_num;
    }
}

static void tsdrv_parse_ras_data(const struct notify_data *pdata, struct ras_error *error)
{
    error->node_type = pdata->node_type;
    error->event_attr = EVENT_OCCUR_ATTR;
    if (guid_equal(pdata->section_type, &CPER_SEC_HISI_OEM_2)) {
        struct sec_oem_error *pdata_oem = (struct sec_oem_error *)pdata->origin_data;
        error->section_type = RAS_SEC_OEM;
        /* IERR(bits 8~15) and SERR(bits 0~7) will be uploaded as err_status_l in the register list */
        error->ras_code = pdata_oem->err_status_l & OEM_ERR_STATUS_MASK;
        error->node_id = pdata_oem->submodule_id;
    } else if (guid_equal(pdata->section_type, &CPER_SEC_HISI_COMMON)) {
        hisi_common_error_info *pdata_common = (hisi_common_error_info *)pdata->origin_data;
        error->section_type = RAS_SEC_GENERIC;
        /* IERR(bits 8~15) and SERR(bits 0~7) will be uploaded as the 4th member of the register list */
        error->ras_code = pdata_common->register_array[4] & OEM_ERR_STATUS_MASK;
        tsdrv_update_node_type(pdata, error);
        tsdrv_set_submodule_id(pdata, error);
        TSDRV_PRINT_DEBUG("soc_id=%u; socker_id=%u; totem_id=%u; nimbus_id=%u; sub_sys_id=%u; module_id=%u; "
            "sub_module_id=%u; device_or_core_id=%u; function_or_port_id=%u; error_type=%u; error_severity=%u; "
            "sub_sys_num=%u; register_array_size=%u; node_type=0x%x; node_id=%u; ras_code=0x%x\n",
            pdata_common->soc_id, pdata_common->socket_id, pdata_common->totem_id, pdata_common->nimbus_id,
            pdata_common->sub_sys_id, pdata_common->module_id, pdata_common->sub_module_id,
            pdata_common->device_or_core_id, pdata_common->function_or_port_id, pdata_common->error_type,
            pdata_common->error_severity, pdata_common->sub_sys_num, pdata_common->register_array_size,
            error->node_type, error->node_id, error->ras_code);
    } else {
        error->section_type = RAS_SEC_OTHER;
        error->ras_code = RAS_ERR_CODE_INVAILD;
        error->node_id = 0xFFFF;
    }
}

static bool tsdrv_is_tscpu_fault(const struct notify_data *pdata)
{
    if (pdata->node_type == DMS_DEV_TYPE_TSCPU) {
        return true;
    }
    return false;
}

int tsdrv_ras_event_scan(u64 private_data, struct dms_sensor_event_data *data)
{
    u32 devid = get_dev_id_from_priv(private_data);
    int node_type = get_node_type_from_priv(private_data);
    u32 node_id = get_dev_node_from_priv(private_data);
    u32 sensor_type = get_sensor_node_from_priv(private_data);
    struct dfm_struct *dfm = NULL;

    if (data == NULL) {
        TSDRV_PRINT_ERR("Invalid para. (devid=%u;  priv=0x%llx)\n", devid, private_data);
        return -EINVAL;
    }

    dfm = tsdrv_get_dfm_by_node_type(devid, node_type);
    if (dfm == NULL) {
        TSDRV_PRINT_ERR("Failed to get dfm. (devid=%u; node_type=%d)\n", devid, node_type);
        return -EINVAL;
    }

    TSDRV_PRINT_DEBUG("Scan event. (devid=%u; node_type=0x%2X; node_id=%u)\n", devid, node_type, node_id);
    (void)dfm_scan_events(dfm, node_id, (u8)sensor_type, data);
    return 0;
}

static bool tsdrv_ras_is_need_unmask_irq(DMS_DEVICE_NODE_TYPE node_type, u32 ras_code)
{
    if ((node_type == DMS_DEV_TYPE_TS_DISP) || (node_type == DMS_DEV_TYPE_AIC_DISP) ||
        (node_type == DMS_DEV_TYPE_DSA_DISP) || (node_type == DMS_DEV_TYPE_L2BUF) ||
        (node_type == DMS_DEV_TYPE_TSCPU)) {
        return true;
    }
    return false;
}

#ifndef TSDRV_UT
#define SCHEDULE_UNMASK_RAS_IRQ_TIME 300000 /* schedule after 5 min */
#else
#define SCHEDULE_UNMASK_RAS_IRQ_TIME 0
#endif
static void tsdrv_fault_unmask_ras_irq_process(u32 devid, int node_type, u32 node_id, u32 ras_code)
{
    if (fault_is_enable_unmask_ras_irp(devid) &&
        tsdrv_ras_is_need_unmask_irq(node_type, ras_code)) {
        struct dfm_struct *dfm = tsdrv_get_dfm_by_node_type(devid, node_type);
        if (dfm == NULL) {
            TSDRV_PRINT_ERR("Failed to get dfm. (devid=%u; node_type=%d)\n", devid, node_type);
            return;
        }
        if (delayed_work_pending(&dfm->dev_nodes[node_id].unmask_irq_work)) {
            (void)cancel_delayed_work_sync(&dfm->dev_nodes[node_id].unmask_irq_work);
        }
        (void)schedule_delayed_work(&dfm->dev_nodes[node_id].unmask_irq_work,
            msecs_to_jiffies(SCHEDULE_UNMASK_RAS_IRQ_TIME));
    }
}

static int tsdrv_fpdc_para_check(const struct notify_data *pdata)
{
    if (pdata == NULL) {
        TSDRV_PRINT_ERR("The pdata is null.\n");
        return -EINVAL;
    }

    if ((pdata->chip_id > fault_ctrl_get_num()) ||
        (pdata->node_type < DMS_DEV_TYPE_SOC) || (pdata->node_type >= DMS_DEV_TYPE_MAX)) {
        TSDRV_PRINT_ERR("Invalid para. (devid=%u; node_type=0x%x)\n", pdata->chip_id, pdata->node_type);
        return -EINVAL;
    }

    return 0;
}

#ifdef CFG_FEATURE_USE_FAULT_NEW_FRAME_WORK
void tsdrv_ras_fault_sensor_scan_immediately(struct dfm_struct *dfm, u32 node_id, unsigned char sensor_type)
{
    u32 i;

    struct dfm_node *dev_node = &dfm->dev_nodes[node_id];
    for (i = 0; i < dev_node->sensor_num; ++i) {
        if (dev_node->sensors[i].dms_sensor.sensor_type != sensor_type) {
            continue;
        }
        if (dms_sensor_event_notify(dfm->dev_id, &dev_node->sensors[i].dms_sensor) != 0) {
            TSDRV_PRINT_ERR("event notify failed.  (dev_id=%u, node_id=%u, node_type=0x%x, i=%u, sensor_type=0x%x)\n",
                dfm->dev_id, node_id, dev_node->node_type, i, sensor_type);
        }
    }

    return;
}
#endif /* CFG_FEATURE_USE_FAULT_NEW_FRAME_WORK */

void tsdrv_ras_event_handler(const struct notify_data *pdata)
{
    const struct ras_fault_converge_item *converage_node = NULL;
    struct dfm_struct *dfm = NULL;
    struct ras_error error;
    int ret;

    ret = tsdrv_fpdc_para_check(pdata);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to check the parameters.\n");
        return;
    }

    tsdrv_parse_ras_data(pdata, &error);
    dfm = tsdrv_get_dfm_by_node_type(pdata->chip_id, error.node_type);
    if (dfm == NULL) {
        TSDRV_PRINT_ERR("Failed to get dfm. (devid=%u; node_type=0x%x)\n", pdata->chip_id, error.node_type);
        return;
    }

    if (tsdrv_is_tscpu_fault(pdata)) {
        tsdrv_fill_tscpu_sensor_status(&error);
    }

    if (error.node_id >= dfm->node_num) {
        TSDRV_PRINT_ERR("Invalid node id. (node_id=%u; node_num=%u)\n", error.node_id, dfm->node_num);
        return;
    }
    TSDRV_PRINT_DEBUG("Debug. (src_type=%d; node_type=0x%x; node_id=%u; devid=%u; ras_code=0x%x; data_len=0x%x)\n",
        pdata->src_type, error.node_type, error.node_id, pdata->chip_id, error.ras_code, pdata->data_len);

    converage_node = tsdrv_fault_converge(&error);
    if (converage_node == NULL) {
        TSDRV_PRINT_ERR("Invalid fault type. (section_type=%u; ras_code=0x%x; devid=%u; node_id=%u)\n",
            error.section_type, error.ras_code, pdata->chip_id, error.node_id);
        return;
    }

    ret = dfm_add_event(dfm, error.node_id, converage_node);
    if ((ret != 0) && (ret != -EEXIST)) {
        TSDRV_PRINT_ERR("Process fault event info failed. (devid=%u; node_id=%u; tsid=%u; ret=%d)\n",
            pdata->chip_id, error.node_id, 0, ret);
        return;
    }

#ifdef CFG_FEATURE_USE_FAULT_NEW_FRAME_WORK
    if (ret == 0) {
        tsdrv_ras_fault_sensor_scan_immediately(dfm, error.node_id, converage_node->sensor_type);
    }
#endif

    tsdrv_fault_unmask_ras_irq_process(pdata->chip_id, (int)error.node_type, error.node_id, error.ras_code);
}

void tsdrv_init_dms_nodes(u32 devid, int node_type, u32 node_id)
{
    struct dms_node *node = tsdrv_get_dms_node(devid, node_type, node_id);
    node->node_type = node_type;
    node->node_id = node_id;
    memcpy_s(node->node_name, DMS_MAX_DEV_NAME_LEN, fault_node_type_to_name(node_type), DMS_MAX_DEV_NAME_LEN);
    node->capacity = 0x1;
    node->permission = 0x1;
    node->owner_devid = devid;
    node->ops = &tsdrv_dev_node_ops;
}

static int tsdrv_register_one_dms_node(u32 dev_id, int node_type, int sub_node_type, u32 node_num)
{
    struct dms_sensor_object_cfg *sensor_cfg = NULL;
    struct dfm_struct *dfm = NULL;
    u32 i, sensor_idx, sensor_num;
    int ret;

    dfm = tsdrv_get_dfm_by_node_type(dev_id, sub_node_type);
    if (dfm == NULL) {
        TSDRV_PRINT_ERR("Failed to get dfm. (devid=%u; sub_node_type=%d)\n", dev_id, sub_node_type);
        return -EINVAL;
    }

    sensor_cfg = tsdrv_get_sensor_cfg(sub_node_type, &sensor_num);

    dfm->dev_id = dev_id;
    ret = dfm_struct_init(dfm, node_num, sensor_num);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to init dfm data. (devid=%u; sub_node_type=%d)\n", dev_id, node_type);
        return ret;
    }

    for (i = 0; i < node_num; ++i) {
        tsdrv_init_dms_nodes(dev_id, sub_node_type, i);
        dfm->dev_nodes[i].devid = dev_id;
        dfm->dev_nodes[i].node_id = i;
        dfm->dev_nodes[i].node_type = sub_node_type;
        dfm->dev_nodes[i].node = tsdrv_get_dms_node(dev_id, sub_node_type, i);
        dfm->dev_nodes[i].post_proc = NULL;
        dfm->dev_nodes[i].fpdc_notify = tsdrv_ras_event_handler;
        INIT_DELAYED_WORK(&dfm->dev_nodes[i].unmask_irq_work, fault_unmask_ras_irq);
        for (sensor_idx = 0; sensor_idx < sensor_num; ++sensor_idx) {
            sensor_cfg[sensor_idx].private_data =
                SENSOR_PRIV_DATA(dev_id, 0, sub_node_type, i, sensor_cfg[sensor_idx].sensor_type);
            dfm->dev_nodes[i].sensors[sensor_idx].dms_sensor = sensor_cfg[sensor_idx];
        }
    }

    if (dfm_register_nodes(dfm) != 0) {
        TSDRV_PRINT_ERR("Register dms node failed\n");
        goto dfm_register_fail;
    }
    ret = fpdc_register_fault_notifier(node_type, tsdrv_ras_event_handler);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to register fpdc notifier. (ret=%d; node_type=0x%x)\n", ret, node_type);
        goto fpdc_register_fail;
    }

    TSDRV_PRINT_INFO("Register dms nodes success. (devid=%u; sub_node_type=0x%x)\n", dev_id, node_type);
    return 0;

fpdc_register_fail:
    dfm_unregister_nodes(dfm);
dfm_register_fail:
    for (i = 0; i < node_num; ++i) {
        (void)cancel_delayed_work_sync(&dfm->dev_nodes[i].unmask_irq_work);
    }
    dfm_struct_final(dfm, node_num, sensor_num);
    return -EFAULT;
}

static void tsdrv_unregister_one_dms_node(u32 dev_id, int node_type, int sub_node_type, u32 node_num)
{
    struct dfm_struct *dfm = NULL;
    u32 i;

    fpdc_unregister_fault_notifier(node_type);
    dfm = tsdrv_get_dfm_by_node_type(dev_id, sub_node_type);
    if (dfm == NULL) {
        TSDRV_PRINT_ERR("Failed to get dfm. (devid=%u; sub_node_type=%d)\n", dev_id, sub_node_type);
        return;
    }
    dfm_unregister_nodes(dfm);
    for (i = 0; i < node_num; ++i) {
        (void)cancel_delayed_work_sync(&dfm->dev_nodes[i].unmask_irq_work);
    }
    dfm_struct_final(dfm, node_num, 1);
}

int tsdrv_register_ras_dms_node(u32 devid)
{
    struct tsdrv_ras_node_info *node_info = NULL;
    int types_num, ret, i, j;

    tsdrv_get_ras_node_types(&node_info, &types_num);
    for (i = 0; i < types_num; i++) {
        ret = tsdrv_register_one_dms_node(devid, node_info[i].node_type, node_info[i].sub_node_type,
            node_info[i].node_num);
        if (ret != 0) {
            TSDRV_PRINT_ERR("Failed to register dms node. (devid=%u; node_type=0x%x; sub_node_type=0x%x)\n",
                devid, node_info[i].node_type, node_info[i].sub_node_type);
            goto register_fail;
        }
    }

    return 0;

register_fail:
    for (j = i - 1; j >= 0; j--) {
        tsdrv_unregister_one_dms_node(devid, node_info[j].node_type, node_info[j].sub_node_type,
            node_info[j].node_num);
    }
    return ret;
}

void tsdrv_unregister_ras_dms_node(u32 devid)
{
    struct tsdrv_ras_node_info *node_info = NULL;
    int types_num, i;

    tsdrv_get_ras_node_types(&node_info, &types_num);
    for (i = 0; i < types_num; i++) {
        tsdrv_unregister_one_dms_node(devid, node_info[i].node_type, node_info[i].sub_node_type,
            node_info[i].node_num);
    }
}
#endif

