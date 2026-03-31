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

#ifndef SOFT_FAULT_DEFINE_H
#define SOFT_FAULT_DEFINE_H

#include <linux/types.h>

#include "dms_define.h"
#include "dms_template.h"
#include "dms_cmd_def.h"
#include "urd_acc_ctrl.h"
#include "drv_log.h"
#include "dms_sensor.h"
#include "dms_sensor_type.h"
#include "dms_define.h"
#include "soft_fault_config.h"

#define MODULE_SOFT "drv_soft_fault"
#define soft_drv_err(fmt, ...) drv_err(MODULE_SOFT, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__)
#define soft_drv_warn(fmt, ...) drv_warn(MODULE_SOFT, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__)
#define soft_drv_info(fmt, ...) drv_info(MODULE_SOFT, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__)
#define soft_drv_event(fmt, ...) drv_event(MODULE_SOFT, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__)
#define soft_drv_debug(fmt, ...) drv_debug(MODULE_SOFT, "<%s:%d> " fmt, current->comm, current->tgid, ##__VA_ARGS__)

#ifndef DRV_SOFT_UT
#define STATIC static
#else
#define STATIC
#endif

#define DMS_MODULE_SOFT_FAULT "soft_fault"
#define SOFT_FAULT_NOTIFIER "soft_fault"

INIT_MODULE_FUNC(soft_fault);
EXIT_MODULE_FUNC(soft_fault);

#define SOFT_NODE_DEF(type, name, devid, id, m_ops) \
{ \
    .list = { \
        .next = NULL, \
        .prev = NULL \
    }, \
    .node_type = (type), \
    .node_id = (id), \
    .pid = (current->tgid), \
    .node_name = (name), \
    .capacity = 0, \
    .permission = 0, \
    .owner_devid = (devid), \
    .owner_device = NULL, \
    .sub_node = {NULL}, \
    .state = 0, \
    .ops = (m_ops) \
} \

#define SOFT_SENSOR_DEF(type, name, devid, s_id, s_idx, assert_mask, deassert_mask, scan_inter, func, pid) \
{ \
    .sensor_type = (type), \
    .sensor_name = (name), \
    .sensor_class = DMS_DISCRETE_SENSOR_CLASS, \
    .sensor_class_cfg = { \
        .discrete_sensor = {DMS_SENSOR_ATTRIB_THRES_NONE, 0} \
    }, \
    .scan_module = DMS_SERSOR_SCAN_PERIOD, \
    .scan_interval = (scan_inter), \
    .proc_flag = DMS_SENSOR_PROC_ENABLE_FLAG, \
    .enable_flag = DMS_SENSOR_ENABLE_FALG, \
    .pf_scan_func = (func), \
    .private_data = (0), \
    .assert_event_mask = (assert_mask), \
    .deassert_event_mask = (deassert_mask), \
    .pid = (pid), \
} \

#define SF_OFFSET_8BIT 8
#define SF_OFFSET_16BIT 16
#define SF_OFFSET_24BIT 24
#define SF_OFFSET_32BIT 32
#define SF_MASK_32BIT 0xFFFFFFFF
#define SF_MASK_16BIT 0xFFFF
#define SF_MASK_8BIT 0xFF

#define SF_USER_NODE_MAX 8
#define SF_USER_NODE_DAVINCI 0
#define SF_USER_NODE_OS 1


#define HEARTBEAT_ERROR_TYPE_HEARTBEAT_RECOVER (HEARTBEAT_ERROR_TYPE_HEARTBEAT_LOST + 2)
#define GENERAL_SOFTWARE_FAULT_MEMORY_OVER_LIMIT_RECOVER (0x0A)
#define GENERAL_SOFTWARE_FAULT_PROCESS_START_FAILED_OR_EXIT_RECOVER (0x0B)
#define OS_INIT (0x09)
#define OS_INIT_DONE (OS_INIT + 1)

struct soft_fault_recover {
    DMS_SENSOR_TYPE_T sensor_type;
    unsigned int occur_val;
    unsigned int recover_val;
};

struct soft_fault {
    DMS_SENSOR_TYPE_T sensor_type;
    unsigned int dev_id;
    unsigned int user_id; /* davinci/pm/iam */
    unsigned int node_id;
    unsigned int sub_id; /* heartbeat module */
    unsigned int err_type;
    unsigned int data_len;
    unsigned char data[DMS_MAX_EVENT_DATA_LENGTH];
};

struct soft_error_list {
    struct soft_fault error;
    struct list_head list;
};

struct soft_event {
    unsigned int event_status;
    unsigned int error_num;
    struct soft_error_list error_list; /* event list */
    struct mutex mutex;
};

struct soft_dev {
    unsigned int dev_id;
    int node_id;
    unsigned int registered;
    unsigned int sensor_obj_num; /* max 8 sensor object */
    struct dms_sensor_object_cfg sensor_obj_table[SF_SUB_ID_MAX]; /* sensor table */
    struct soft_event sensor_event_queue[SF_SUB_ID_MAX]; /* sensor event queue */
    int sensor_obj_registered[SF_SUB_ID_MAX]; /* registered status of corresponding sensor obj and event */
    struct dms_node dev_node;
    struct mutex mutex;
    struct list_head list;
};

struct soft_dev_client {
    pid_t pid; /* one process user */
    unsigned int user_id;
    unsigned int registered;
    unsigned int node_num; /* max 8 node */
    struct mutex mutex;
    struct list_head head;
};

struct drv_soft_ctrl {
    unsigned int user_num[DEV_ID_MAX];
    struct soft_dev_client *s_dev_t[DEV_ID_MAX][SF_USER_MAX];
    struct mutex mutex[DEV_ID_MAX];
};

struct drv_soft_ctrl *soft_get_ctrl(void);
struct dms_node_operations *soft_get_ops(void);
int soft_fault_event_scan(unsigned long long private_data, struct dms_sensor_event_data *data);
int soft_fault_event_handler(struct soft_fault *event);
int soft_register_one_node(struct soft_dev *s_dev);
void soft_unregister_one_node(struct soft_dev *s_dev);
void soft_fault_event_free(struct soft_event *event_queue);
void soft_free_one_node(struct soft_dev_client *client, unsigned int node_type);
void soft_dev_exit(void);
void soft_one_dev_init(struct soft_dev *s_dev);
void soft_one_dev_exit(struct soft_dev *s_dev);

int soft_get_dev_info_list(struct dms_node *device, struct dms_dev_data_attr *info_list);
int soft_get_dev_state(struct dms_node *device, unsigned int *state);
int soft_get_dev_capacity(struct dms_node *device, unsigned long long *capacity);
int soft_set_dev_power_state(struct dms_node *device, DSMI_POWER_STATE power_state);
int soft_ops_init(struct dms_node *device);
void soft_ops_exit(struct dms_node *device);
void soft_client_release(int owner_pid);

#endif /* SOFT_FAULT_DEFINE_H */
