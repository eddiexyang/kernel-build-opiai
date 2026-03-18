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

#ifndef SOC_MISC_DMS_DEF_H
#define SOC_MISC_DMS_DEF_H

#include <linux/types.h>
#include <linux/securec.h>
#include <linux/workqueue.h>
#include <linux/cdev.h>

#include "soc_misc_common.h"
#include "dms_sensor.h"
#include "dms_sensor_type.h"
#include "dms_define.h"
#include "fpdc.h"

#define SOC_UNMASK_IRQ_WORK_DATA 4

/**
 * Maximum number of sensors. The value must be greater than or equal
 * to the maximum number of sensors on the node.
 */
#if defined(CFG_SOC_PLATFORM_MINIV2) || defined(CFG_SOC_PLATFORM_MDC_V51)
#  define SOC_MISC_SENSOR_MAX_NUM 3
#else
#  define SOC_MISC_SENSOR_MAX_NUM 1
#endif

/**
 * sensor id in local module
 */
typedef enum soc_misc_sensor_id {
    SOC_MISC_SENSOR_CPUCORE = 0,
    SOC_MISC_SENSOR_RING = 3,
    SOC_MISC_SENSOR_PCIE = 4,
    SOC_MISC_SENSOR_MN = 5,
    SOC_MISC_SENSOR_RBRG = 6,
    SOC_MISC_SENSOR_HCCS = 7,
    SOC_MISC_SENSOR_PCIE_DISP = 8,
    SOC_MISC_SENSOR_EMMC = 9,
    SOC_MISC_SENSOR_ID_MAX
} SOC_MISC_SENSOR_ID_E;

struct ras_error {
    unsigned char device_id;       /* device id */
    unsigned char sensor_id;       /* L3D/L3T/L2C/IFU/... */
    unsigned char module_id;       /* bios ras Module ID */
    unsigned char sub_node_id;    /* CPUCORE/L3D/L3T */
    unsigned int ras_code;         /* interrupt status: IERR/IRQ STATUS */
    unsigned int ras_err_type;     /* ras int type: reference to enum @ras_error_type */
    unsigned int ras_err_severity; /* error level */
    unsigned int sec_type;
};

typedef struct soc_misc_event_node {
    struct ras_error error;
    struct list_head node;
} SOC_MISC_EVENT_NODE_T;

typedef struct soc_misc_event_list {
    unsigned int event_status;
    unsigned int error_num;
    struct list_head list;
    struct mutex mutex;
} SOC_MISC_EVENT_LIST_T;

struct soc_misc_node {
    unsigned int die_id;
    spinlock_t spinlock;
    unsigned int sensor_obj_num;
    struct dms_sensor_object_cfg *sensor_obj_table; /* sensor table pointer */
    SOC_MISC_EVENT_LIST_T sensor_event_queue[SOC_MISC_SENSOR_MAX_NUM]; /* sensor event queue */
    struct dms_node *dev_node;
    struct delayed_work unmask_irq_work;
    unsigned char  work_data[SOC_UNMASK_IRQ_WORK_DATA];
#ifdef CFG_FEATURE_FAULT_MANAGER
    FAULT_NOTIFY_FUNC fpdc_notify;
#endif
    struct delayed_work query_work;
};

struct drv_soc_misc_ctrl {
    unsigned int dev_num;
    struct soc_misc_node node[SOC_MISC_DEVICE_NUM_MAX][SOC_MISC_MAX_NODE_NUM][SOC_MISC_SENSOR_ID_MAX];
};

struct drv_soc_misc_ctrl *soc_misc_get_ctrl(void);
#if (!defined(CFG_SOC_PLATFORM_MDC_V51) && !defined(CFG_SOC_PLATFORM_MDC_V11))
const struct ras_fault_converge_item *soc_misc_get_cpucore_converge_item(const struct ras_error *error_info);
#endif
const struct ras_fault_converge_item *soc_misc_get_pcie_converge_item(const struct ras_error *error_info);
const struct ras_fault_converge_item *soc_misc_get_emmc_converge_item(const struct ras_error *error_info);
#ifdef CFG_FEATURE_SAFETY_MANAGER
const struct ras_fault_converge_item *soc_misc_get_mn_converge_item(const struct ras_error *error_info);
#endif

/**
 * offset define
 */
#define SOC_MISC_OFFSET_8BIT  8
#define SOC_MISC_OFFSET_16BIT 16
#define SOC_MISC_OFFSET_24BIT 24
#define SOC_MISC_OFFSET_32BIT 32
#define SOC_MISC_MASK_32BIT 0xFFFFFFFF
#define SOC_MISC_DMS_SENSOR_PRIVDATA(dev_id, sensor_type, node_id) \
    (((u64)dev_id << SOC_MISC_OFFSET_32BIT) | \
    ((sensor_type & 0xFF) << SOC_MISC_OFFSET_16BIT) | (node_id & 0xFF))

#define _STR(i) #i

#ifndef CFG_SOC_PLATFORM_MDC_V11
#define SOC_MISC_CPUCORE_NODE(_dev_id, _node_id) { \
    .node_type = DMS_DEV_TYPE_CPU_CORE,            \
    .node_id = _node_id,                           \
    .node_name = "CPUcore-" _STR(_dev_id) "-" _STR(_node_id), \
    .capacity = 0x01,                              \
    .permission = 0x01,                            \
    .owner_devid = _dev_id,                        \
    .owner_device = NULL,                          \
    .ops = &g_soc_misc_cpucore_ops                 \
}
#endif

#define SOC_MISC_RING_NODE(_dev_id, _node_id) {            \
    .node_type = DMS_DEV_TYPE_RING,                        \
    .node_id = _node_id,                                   \
    .node_name = "RING-" _STR(_dev_id) "-" _STR(_node_id), \
    .capacity = 0x01,                                      \
    .permission = 0x01,                                    \
    .owner_devid = _dev_id,                                \
    .owner_device = NULL,                                  \
    .ops = &g_soc_misc_ring_ops                            \
}

#define SOC_MISC_PCIE_NODE(_dev_id, _node_id) {                  \
    .node_type = DMS_DEV_TYPE_PCIE,                              \
    .node_id = _node_id,                                         \
    .node_name = "PCIE-LOCAL-" _STR(_dev_id) "-" _STR(_node_id), \
    .capacity = 0x01,                                            \
    .permission = 0x01,                                          \
    .owner_devid = _dev_id,                                      \
    .owner_device = NULL,                                        \
    .ops = &g_soc_misc_pcie_ops                                  \
}

#define SOC_MISC_PCIE_DISP_NODE(_dev_id, _node_id) {             \
    .node_type = DMS_DEV_TYPE_PCIE_DISP,                         \
    .node_id = _node_id,                                         \
    .node_name = "PCIE-DISP-" _STR(_dev_id) "-" _STR(_node_id),  \
    .capacity = 0x01,                                            \
    .permission = 0x01,                                          \
    .owner_devid = _dev_id,                                      \
    .owner_device = NULL,                                        \
    .ops = &g_soc_misc_pcie_ops                                  \
}

#define SOC_MISC_MN_NODE(_dev_id, _node_id, _emu_name) {         \
    .node_type = DMS_DEV_TYPE_MN,                                \
    .node_id = _node_id,                                         \
    .node_name = _emu_name "-MN-" _STR(_dev_id) "-" _STR(_node_id), \
    .capacity = 0x01,                                            \
    .permission = 0x01,                                          \
    .owner_devid = _dev_id,                                      \
    .owner_device = NULL,                                        \
    .ops = &g_soc_misc_mn_ops                               \
}

#define SOC_MISC_RBRG_NODE(_dev_id, _node_id) {         \
    .node_type = DMS_DEV_TYPE_RBRG,                                \
    .node_id = _node_id,                                         \
    .node_name = "AO-RBRG-" _STR(_dev_id) "-" _STR(_node_id), \
    .capacity = 0x01,                                            \
    .permission = 0x01,                                          \
    .owner_devid = _dev_id,                                      \
    .owner_device = NULL,                                        \
    .ops = &g_soc_misc_rbrg_ops                               \
}

#define SOC_MISC_RESV_NODE(_dev_id, _node_id) { \
    .ops = NULL                                 \
}

#define SOC_MISC_EMMC_NODE(_dev_id, _node_id) {                  \
    .node_type = DMS_DEV_TYPE_EMMC,                              \
    .node_id = _node_id,                                         \
    .node_name = "EMMC-LOCAL-" _STR(_dev_id) "-" _STR(_node_id), \
    .capacity = 0x01,                                            \
    .permission = 0x01,                                          \
    .owner_devid = _dev_id,                                      \
    .owner_device = NULL,                                        \
    .ops = &g_soc_misc_emmc_ops                                  \
}

#define SOC_MISC_SENOR_OBJ(_type, _name, _class, _attr, _debounce, _interval,  \
                           _proc, _enable, _func, _data, _assert, _deassert) { \
    .sensor_type = _type,              \
    .sensor_name = _name,              \
    .sensor_class = _class,            \
    .sensor_class_cfg = {              \
        .discrete_sensor = {           \
            .attribute = _attr,        \
            .debounce_time = _debounce \
        }                              \
    },                                 \
    .scan_interval = _interval,        \
    .proc_flag = _proc,                \
    .enable_flag =  _enable,           \
    .pf_scan_func = _func,             \
    .private_data = _data,             \
    .assert_event_mask = _assert,      \
    .deassert_event_mask = _deassert   \
}

#define SOC_MISC_SENSOR_PRIV_DATA(dev_id, sensor_type, node_id) ( \
     ((u64)dev_id << SOC_MISC_OFFSET_32BIT) |                     \
     ((sensor_type & 0xFF) << SOC_MISC_OFFSET_16BIT) |            \
     (node_id & 0xFF)                                             \
)

#define SOC_MISC_DMS_SENSOR(dev_id, node_id, sensor_type, sensor_name, scan_fun) \
    SOC_MISC_SENOR_OBJ(sensor_type, sensor_name "_ras" _STR(dev_id),              \
        DMS_DISCRETE_SENSOR_CLASS, DMS_SENSOR_ATTRIB_THRES_NONE, 0,               \
        DMS_SENSOR_CHECK_INTERVAL_TIME, DMS_SENSOR_PROC_ENABLE_FLAG,              \
        DMS_SENSOR_ENABLE_FALG, scan_fun,                                         \
        SOC_MISC_SENSOR_PRIV_DATA(dev_id, sensor_type, node_id),                  \
        0xFFFF, 0xFFBF)

/* driver query ras error every 1s */
#define SOC_MISC_DRV_QUERY_RAS_DEFAULT_TIME 1000

#endif /* SOC_CONFIG_H */
