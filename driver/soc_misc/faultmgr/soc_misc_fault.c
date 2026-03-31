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

#include <linux/slab.h>
#include <linux/printk.h>
#if defined(CFG_SOC_PLATFORM_MDC_V51) || defined(CFG_SOC_PLATFORM_MDC_LITE)
#include "drv_notify.h"
#endif
#include "soc_misc_init.h"
#include "soc_misc_common.h"
#include "soc_misc_fault.h"
#include "dms_dev_node.h"
#include "soc_misc_err_info.h"
#include "soc_misc_safety.h"

#ifdef AOS_LLVM_BUILD

#ifndef LOGLEVEL_ERR
#  define LOGLEVEL_ERR  3    /* error conditions */
#endif

#ifndef LOGLEVEL_DEBUG
#define LOGLEVEL_DEBUG  7    /* debug-level messages */
#endif

#endif

STATIC unsigned int g_fault_event_record_type_table[] = {
    RAS_ERROR_TYPE_ERROR,
    RAS_ERROR_TYPE_ERROR_NF,
    RAS_ERROR_TYPE_INPUT_ERR,
    RAS_ERROR_TYPE_IN_CFG_ERR,
    RAS_ERROR_TYPE_CFG_ERR,
    RAS_ERROR_TYPE_PARITY,
    RAS_ERROR_TYPE_SBECC_NC,
    RAS_ERROR_TYPE_MBECC,
    RAS_ERROR_TYPE_BUS_ERR,
    RAS_ERROR_TYPE_TIMEOUT_ERR,
    RAS_ERROR_TYPE_HB_TIMEOUT,
    RAS_ERROR_TYPE_KO_INS_FAIL,
    RAS_ERROR_TYPE_INIT_ABNORMAL,
    RAS_ERROR_TYPE_IN_CFG_ERR_MINOR,
    RAS_ERROR_TYPE_CFG_ERR_MINIOR,
};

STATIC void soc_misc_print_ras_event(const struct ras_error *error, int level);

static inline int soc_misc_check_params(struct drv_soc_misc_ctrl *soc_misc_ctrl, unsigned int dev_id,
    struct dms_sensor_event_data *data, SOC_MISC_SENSOR_ID_E sensor_id)
{
    if ((soc_misc_ctrl == NULL) || (dev_id >= soc_misc_dev_num(soc_misc_ctrl->dev_num)) ||
        (data == NULL) || (sensor_id >= SOC_MISC_SENSOR_ID_MAX)) {
        soc_misc_drv_err("Invalid parameter. (data=%s; misc_ctrl=%s; dev_id=%u; sensor_id=%u)\n",
            data == NULL ? "NULL" : "OK", soc_misc_ctrl == NULL ? "NULL" : "OK", dev_id, sensor_id);
        return -EINVAL;
    }
    return 0;
}

STATIC bool soc_misc_add_fault_event(SOC_MISC_EVENT_NODE_T *error_new, SOC_MISC_EVENT_LIST_T *event_queue)
{
    SOC_MISC_EVENT_NODE_T *event_node = NULL;
    struct list_head *pos = NULL, *n = NULL;
#ifndef UT_VCAST
    mutex_lock(&event_queue->mutex);
    if (!list_empty_careful(&event_queue->list)) {
        list_for_each_safe(pos, n, &event_queue->list) {
            event_node = list_entry(pos, SOC_MISC_EVENT_NODE_T, node);
            if (event_node->error.ras_err_type == error_new->error.ras_err_type) {
                mutex_unlock(&event_queue->mutex);
                return false; /* false means found fault event already in event list, don't need add */
            }
        }
    }
    list_add(&error_new->node, &event_queue->list); /* add new event to queue */
    event_queue->error_num++;
    mutex_unlock(&event_queue->mutex);

    soc_misc_print_ras_event(&error_new->error, LOGLEVEL_DEBUG);
#endif
    return true;
}

STATIC bool soc_misc_clear_fault_event_check(u8 error_type)
{
    unsigned int i;

    /* false means found fault event type in record table, don't need clear */
    for (i = 0; i < sizeof(g_fault_event_record_type_table) / sizeof(unsigned int); i++) {
        if (error_type == g_fault_event_record_type_table[i]) {
            return false;
        }
    }
    return true;
}

STATIC unsigned int soc_misc_get_section_ras_code(const struct ras_fault_converge_item *fault_info)
{
    switch (fault_info->section_type) {
        case RAS_SEC_ARM:
            return fault_info->ras_code.int_status;
        case RAS_SEC_OEM:
            return fault_info->ras_code.err_status & SOC_MISC_MASK_32BIT;
        case RAS_SEC_MEM:
        case RAS_SEC_PCIE:
        case RAS_SEC_EMMC:
            return fault_info->ras_code.err_type;
        default:
            break;
    }

    return fault_info->ras_code.int_status;
}

const struct ras_fault_converge_item *soc_misc_parse_table_handle(const struct ras_error *error_info,
    const struct ras_fault_converge_item *converge_item, u32 converge_item_num)
{
    unsigned int i;

    if (error_info == NULL) {
        soc_misc_drv_err("Invalid para, error_info is NULL\n");
        return NULL;
    }

    for (i = 0; i < converge_item_num; i++) {
        if (error_info->sec_type == converge_item[i].section_type) {
            if (error_info->ras_code == soc_misc_get_section_ras_code(&converge_item[i])) {
                return &converge_item[i];
            }
        }
    }
    return NULL;
}

const struct ras_fault_converge_item *soc_misc_parse_table(
    const struct ras_error *error_info, SOC_MISC_SENSOR_ID_E sensor_id)
{
    switch (sensor_id) {
#if ((defined CFG_FEATURE_PCIE_LOCAL_FAULT) || (defined CFG_FEATURE_PCIE_HISI_COMM_FAULT))
        case SOC_MISC_SENSOR_PCIE:
            return soc_misc_get_pcie_converge_item(error_info);
#endif

#ifdef CFG_FEATURE_SAFETY_MANAGER
        case SOC_MISC_SENSOR_MN:
            return soc_misc_get_mn_converge_item(error_info);
#endif

#ifdef CFG_FEATURE_PCIE_DISP_FAULT
        case SOC_MISC_SENSOR_PCIE_DISP:
            return soc_misc_get_pcie_converge_item(error_info);
#endif

#ifdef CFG_FEATURE_EMMC_HISI_COMM_FAULT
        case SOC_MISC_SENSOR_EMMC:
            return soc_misc_get_emmc_converge_item(error_info);
#endif

        default:
            soc_misc_drv_err("invalid sensor_id. (sensor_id=%u)\n", (u32)sensor_id);
            break;
    }
    return NULL;
}

STATIC void soc_misc_print_ras_event(const struct ras_error *error, int level)
{
    if (level == LOGLEVEL_DEBUG) {
        pr_debug("ras event: device_id(%u), sensor_id(%u), module_id(%u), submodule_id(0x%x),"
            "ras_code=0x%x, ras_err_type=0x%x, ras_err_severity=%u, sec_type=0x%x\n",
            error->device_id, error->sensor_id, error->module_id, error->sub_node_id,
            error->ras_code, error->ras_err_type, error->ras_err_severity, error->sec_type);
    } else {
        soc_misc_drv_event("ras event info: device_id(%u), sensor_id(%u), module_id(%u), submodule_id(0x%x),"
            "ras_code=0x%x, ras_err_type=0x%x, ras_err_severity=%u, sec_type=0x%x\n",
            error->device_id, error->sensor_id, error->module_id, error->sub_node_id,
            error->ras_code, error->ras_err_type, error->ras_err_severity, error->sec_type);
    }
}

int soc_misc_get_sensor_event_queue_index(const struct soc_misc_node *node, unsigned char sensor_type)
{
    int i;
    for (i = 0; (unsigned int)i < node->sensor_obj_num; ++i) {
        if (node->sensor_obj_table[i].sensor_type == sensor_type) {
            return i;
        }
    }

    return -1;
}

STATIC int soc_misc_enqueue_new_event(struct ras_error *error_info,
    SOC_MISC_SENSOR_ID_E sensor_id, SOC_MISC_EVENT_NODE_T *event_new)
{
    const struct ras_fault_converge_item *coverage_node = NULL;
    struct drv_soc_misc_ctrl *soc_misc_ctrl = soc_misc_get_ctrl();
    SOC_MISC_EVENT_LIST_T *event_queue = NULL;
    struct soc_misc_node *s_dev_node = NULL;
    int event_queue_idx = 0;

    coverage_node = soc_misc_parse_table(error_info, sensor_id);
    if (coverage_node == NULL) {
        soc_misc_drv_err("unknow fault error.\n");
        return -EFAULT;
    }
    event_new->error.ras_err_type = coverage_node->error_type;
    error_info->ras_err_type = coverage_node->error_type;

    /* add the new event to event queue */
    s_dev_node = &soc_misc_ctrl->node[error_info->device_id][error_info->sub_node_id][sensor_id];
    event_queue_idx = soc_misc_get_sensor_event_queue_index(s_dev_node, coverage_node->sensor_type);
    if (event_queue_idx < 0 || event_queue_idx >= (int)s_dev_node->sensor_obj_num) {
        soc_misc_drv_err("sensor not be found. (device_id=%d; len=%d; sensor_id=%u, sensor_type=0x%x)\n",
            error_info->device_id, error_info->sub_node_id, sensor_id, coverage_node->sensor_type);
        return -EFAULT;
    }

    event_queue = &s_dev_node->sensor_event_queue[event_queue_idx];
    if (soc_misc_add_fault_event(event_new, event_queue) == false) {
        soc_misc_drv_debug("Fault event is already exist\n");
        return -EEXIST;
    }

    soc_misc_drv_event("Recieve and add an event. (subsys=0x%x, module_id=%u, section_type=%u, ras_code=0x%llx,"
        " dev_id=%u, node_id=%d, sensor_type=0x%x, err_type=0x%x, desc=%*s)\n",
        coverage_node->subsys_id, coverage_node->module_id, coverage_node->section_type,
        coverage_node->ras_code.err_status, error_info->device_id, error_info->sub_node_id, coverage_node->sensor_type,
        coverage_node->error_type, DMS_MAX_EVENT_DATA_LENGTH, coverage_node->describe);

    return 0;
}

void soc_misc_fault_event_handler(struct ras_error *error_info, int len, SOC_MISC_SENSOR_ID_E sensor_id)
{
    struct drv_soc_misc_ctrl *soc_misc_ctrl = soc_misc_get_ctrl();
    SOC_MISC_EVENT_NODE_T *event_new = NULL;
    int ret;

    if (error_info == NULL) {
        soc_misc_drv_err("invalid parameter. error info is NULL \n");
        return;
    }

    if ((error_info->device_id >= soc_misc_dev_num(soc_misc_ctrl->dev_num)) ||
        (len != sizeof(struct ras_error)) || (sensor_id >= SOC_MISC_SENSOR_ID_MAX) ||
        (error_info->sub_node_id >= SOC_MISC_MAX_NODE_NUM)) {
        soc_misc_drv_err("invalid parameter. (device_id=%d; len=%d;sensor_id=%u;node_id=%u)\n",
            error_info->device_id, len, sensor_id, error_info->sub_node_id);
        return;
    }

    event_new = kzalloc(sizeof(SOC_MISC_EVENT_NODE_T), GFP_KERNEL | __GFP_ACCOUNT);
    if (event_new == NULL) {
        soc_misc_drv_err("kzalloc failed.\n");
        goto out;
    }
    ret = memcpy_s(&event_new->error, sizeof(struct ras_error), error_info, sizeof(struct ras_error));
    if (ret != 0) {
        soc_misc_drv_err("memcpy_s failed. (ret=%d).\n", ret);
        goto out;
    }

    ret = soc_misc_enqueue_new_event(error_info, sensor_id, event_new);
    if (ret != 0) {
        if (ret == -EEXIST) {
            soc_misc_drv_info("The event has already be reported\n");
        } else {
            soc_misc_drv_err("add the new event to event queue failed. (ret=%d).\n", ret);
        }
        goto out;
    }
    return;

out:
    if (event_new != NULL) {
        kfree(event_new);
    }
    soc_misc_print_ras_event(error_info, LOGLEVEL_ERR);
    return;
}

STATIC int soc_misc_set_sensor_event_data(struct dms_sensor_event_data_item *sensor_event_data,
    const struct ras_error *ras_err, SOC_MISC_SENSOR_ID_E sensor_id)
{
    const struct ras_fault_converge_item *converge_info = NULL;
    int ret;

    converge_info = soc_misc_parse_table(ras_err, sensor_id);
    if (converge_info == NULL) {
        soc_misc_drv_err("soc_misc_parse_table error. (sensor_id=%u)\n", sensor_id);
        return -EINVAL;
    }

    sensor_event_data->current_value = converge_info->error_type;
    sensor_event_data->data_size = DMS_MAX_EVENT_DATA_LENGTH;
    ret = memcpy_s(sensor_event_data->event_data, DMS_MAX_EVENT_DATA_LENGTH,
        converge_info->describe, sizeof(converge_info->describe));
    if (ret != 0) {
        soc_misc_drv_warn("memcpy_s failed. (ret=%d)\n", ret);
        return ret;
    }

    return 0;
}

static int soc_misc_parse_private_data(unsigned long long private_data,
    unsigned int *dev_id, unsigned int *node_id, unsigned int *sensor_type)
{
    *dev_id = (private_data >> SOC_MISC_OFFSET_32BIT) & SOC_MISC_MASK_32BIT;
    *node_id = private_data & 0xFF;
    if (*node_id >= SOC_MISC_MAX_NODE_NUM) {
        soc_misc_drv_warn("invalid node id. (*node_id=%u)\n", *node_id);
        return -EINVAL;
    }

    *sensor_type = (private_data >> SOC_MISC_OFFSET_16BIT) & 0xFF;
    return 0;
}

int soc_misc_fault_event_scan(unsigned long long private_data, struct dms_sensor_event_data *data,
    SOC_MISC_SENSOR_ID_E sensor_id)
{
    struct drv_soc_misc_ctrl *soc_misc_ctrl = soc_misc_get_ctrl();
    struct dms_sensor_event_data_item *sensor_data = NULL;
    SOC_MISC_EVENT_LIST_T *event_queue = NULL;
    SOC_MISC_EVENT_NODE_T *event_node = NULL;
    struct list_head *pos = NULL, *n = NULL;
    unsigned int dev_id = 0, node_id = 0, sensor_type = 0;
    int event_queue_idx;

    if (soc_misc_parse_private_data(private_data, &dev_id, &node_id, &sensor_type) != 0) {
        soc_misc_drv_err("parse scan private data failed.\n");
        return -EINVAL;
    } else if (soc_misc_check_params(soc_misc_ctrl, dev_id, data, sensor_id) != 0) {
        soc_misc_drv_warn("invalid dev id. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    event_queue_idx = soc_misc_get_sensor_event_queue_index(
        &soc_misc_ctrl->node[dev_id][node_id][sensor_id], sensor_type);
    if (event_queue_idx < 0) {
        soc_misc_drv_warn("invalid sensor type. (private data=0x%llx)\n", private_data);
        return -EINVAL;
    }

    event_queue = &soc_misc_ctrl->node[dev_id][node_id][sensor_id].sensor_event_queue[event_queue_idx];
    data->event_count = 0;
    mutex_lock(&event_queue->mutex);
    /* get every event of the sensor event queue */
    if (list_empty_careful(&event_queue->list)) {
        mutex_unlock(&event_queue->mutex);
        return 0;
    }

    list_for_each_safe(pos, n, &event_queue->list) {
        event_node = list_entry(pos, SOC_MISC_EVENT_NODE_T, node);
        soc_misc_print_ras_event(&event_node->error, LOGLEVEL_DEBUG);
        sensor_data = &(data->sensor_data[data->event_count]);

        if (soc_misc_set_sensor_event_data(sensor_data, &event_node->error, sensor_id) != 0) {
            soc_misc_drv_err("set sensor event data failed. devid(%u), sensor id(%u)\n", dev_id, sensor_id);
            continue;
        }

        if (soc_misc_clear_fault_event_check(sensor_data->current_value)) {
            list_del(&event_node->node);
            kfree(event_node);
            event_node = NULL;
            event_queue->error_num--;
        }

        data->event_count++;
        if (data->event_count == DMS_MAX_SENSOR_EVENT_COUNT) {
            soc_misc_drv_warn("event queue full. (event_num=%u)\n", event_queue->error_num);
            break;
        }
    }
    mutex_unlock(&event_queue->mutex);

    return 0;
}


#ifdef CFG_BUILD_DEBUG
static inline bool soc_misc_node_valid(const struct soc_misc_node *s_dev)
{
    if (s_dev->dev_node == NULL) {
        return false;
    }

    return (s_dev->dev_node->owner_devid != -1) && (s_dev->dev_node->node_id != -1);
}

STATIC void soc_misc_clear_event_queue(struct soc_misc_node *s_dev)
{
    SOC_MISC_EVENT_NODE_T *event_node = NULL;
    SOC_MISC_EVENT_LIST_T *event_queue = NULL;
    struct list_head *pos = NULL, *n = NULL;
    int i;

    if (!soc_misc_node_valid(s_dev)) {
        return;
    }

    for (i = 0; i < s_dev->sensor_obj_num; ++i) {
        event_queue = &s_dev->sensor_event_queue[i];

        mutex_lock(&event_queue->mutex);
        /* get every event of the sensor event queue */
        if (list_empty_careful(&event_queue->list)) {
            mutex_unlock(&event_queue->mutex);
            continue;
        }

        list_for_each_safe(pos, n, &event_queue->list) {
            event_node = list_entry(pos, SOC_MISC_EVENT_NODE_T, node);
            list_del(&event_node->node);
            kfree(event_node);
            event_node = NULL;
            event_queue->error_num--;
        }
        mutex_unlock(&event_queue->mutex);
    }
}

int soc_misc_clear_all_fault_events(void)
{
    struct drv_soc_misc_ctrl *dms_ctl = soc_misc_get_ctrl();
    unsigned int dev_id, cluster_id, sensor_id;
    struct soc_misc_node *s_dev = NULL;

    if (dms_ctl == NULL) {
        soc_misc_drv_err("misc ctrl is null. maybe the driver has uninstall\n");
        return -EFAULT;
    }

    for (dev_id = 0; dev_id < soc_misc_dev_num(dms_ctl->dev_num); dev_id++) {
        for (cluster_id = 0; cluster_id < SOC_MISC_MAX_NODE_NUM; ++cluster_id) {
            for (sensor_id = 0; sensor_id < SOC_MISC_SENSOR_ID_MAX; sensor_id++) {
                s_dev = &dms_ctl->node[dev_id][cluster_id][sensor_id]; /* get each device dev_node */
                soc_misc_clear_event_queue(s_dev);
            }
        }
    }
    return 0;
}
EXPORT_SYMBOL(soc_misc_clear_all_fault_events);
#endif

void soc_misc_fault_event_free(SOC_MISC_EVENT_LIST_T *event_queue)
{
    SOC_MISC_EVENT_NODE_T *event_node = NULL;
    struct list_head *pos = NULL, *n = NULL;

    mutex_lock(&event_queue->mutex);
    event_queue->error_num = 0;
    if (list_empty_careful(&event_queue->list)) {
        mutex_unlock(&event_queue->mutex);
        mutex_destroy(&event_queue->mutex);
        return;
    }
    list_for_each_safe(pos, n, &event_queue->list) {
        event_node = list_entry(pos, SOC_MISC_EVENT_NODE_T, node);
        list_del(&event_node->node);
        kfree(event_node);
        event_node = NULL;
    }
    mutex_unlock(&event_queue->mutex);
    mutex_destroy(&event_queue->mutex);
}

const struct ras_fault_converge_item *soc_safety_converge(
    const struct ras_fault_converge_item *items, u32 items_num, u64 ras_code)
{
    u32 i;

    for (i = 0; i < items_num; i++) {
        if (ras_code == items[i].ras_code.err_status) {
            return &items[i];
        }
    }
    return NULL;
}

int soc_chk_safety_param(const struct safety_fault_info *safety_fault,
    unsigned int *event_num)
{
    if (safety_fault == NULL) {
        soc_misc_drv_err("safety fault is null\n");
        return -EINVAL;
    } else if (event_num == NULL) {
        soc_misc_drv_err("event_num is null\n");
        return -EINVAL;
    }

    if (safety_fault->fault_status_list == NULL) {
        soc_misc_drv_err("fault statis list is null\n");
        return -EINVAL;
    }

    return 0;
}

#if defined(CFG_SOC_PLATFORM_MDC_V51) || defined(CFG_SOC_PLATFORM_MDC_LITE)
u32 soc_misc_get_chip_type(void)
{
#define KEY_CHIP_TYPE      (8)
    int chip_type = -1;
#ifdef CFG_SOC_PLATFORM_MDC_LITE_ESL
    notify_err("error! lite esl return 2.\n");
    chip_type = CHIP_TYPE_MDC_BS9SX1A;
    return chip_type;
#else
    if (centre_notify_get_val(KEY_CHIP_TYPE, &chip_type) != 0) {
        soc_misc_drv_err("get chip type failed.\n");
#ifndef DRV_SOC_MISC_UT
        return CHIP_TYPE_MDC_BS9SX1A;
#endif
    }
    soc_misc_drv_info("chip type %d\n", chip_type);
    return chip_type;
#endif
}
#else
u32 soc_misc_get_chip_type(void)
{
    return CHIP_TYPE_NOT_SET;
}
#endif
