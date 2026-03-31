/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2023. All rights reserved.
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

#ifndef TSDRV_FAULT_INIT_H
#define TSDRV_FAULT_INIT_H

#include <linux/types.h>
#include <linux/uuid.h>
#include <linux/workqueue.h>
#include "tsdrv_kernel_common.h"
#include "dms_dev_node.h"
#include "dms_sensor.h"
#include "dms_sensor_type.h"
#include "dms_define.h"

#ifdef CFG_FEATURE_FAULT_FPDC
#include "fpdc.h"
#include "fpdc_ras_receiver.h"
#endif

#define RAS_ERR_CODE_INVAILD       0xFFFF
#define OEM_ERR_STATUS_MASK        0xFFFF

#pragma pack(1)
struct ras_message {
    u64 ras_err_code;
    u8 node_type;
    u8 node_id;
    u8 sub_node_type;
    u8 sub_node_id;
    u8 sensor_type;
    u8 sensor_status;
    u8 event_attr;
    u8 resv[7]; /* reserved 7 bytes */
};
#pragma pack()

enum fault_event_attr {
    EVENT_RESUME_ATTR = 0,
    EVENT_OCCUR_ATTR = 1,
    EVENT_ONE_TIME_ATTR = 2,
    EVENT_ATTR_MAX
};

enum dev_node_id {
    DEV_NODE_HWTS_STARS = 0,
    DEV_NODE_TS,
    DEV_NODE_AIC,
    DEV_NODE_SDMAM,
    DEV_NODE_AIV,
    DEV_NODE_DSA,
    DEV_NODE_TSFW,
    DEV_NODE_ID_MAX
};

enum sensor_node_id {
    SENSOR_NODE_HWTS_STARS = 0,
    SENSOR_NODE_TS,
    SENSOR_NODE_AIC,
    SENSOR_NODE_SDMAM,
    SENSOR_NODE_AIV,
    SENSOR_NODE_DSA,
    SENSOR_NODE_TSFW,
    SENSOR_NODE_ID_MAX
};

static const char *fault_type_description[RAS_ERROR_TYPE_ERR_TYPE_MAX] = {
    [RAS_ERROR_TYPE_ERROR] = "module error",
    [RAS_ERROR_TYPE_ERROR_NF] = "module error can not be fixed",
    [RAS_ERROR_TYPE_INPUT_ERR] = "input error",
    [RAS_ERROR_TYPE_IN_CFG_ERR] = "internal config error",
    [RAS_ERROR_TYPE_CFG_ERR] = "config error",
    [RAS_ERROR_TYPE_PARITY] = "parity error",
    [RAS_ERROR_TYPE_SBECCOverThold] = "single bit ecc error exceeding the threshold",
    [RAS_ERROR_TYPE_SBECC_NC] = "single bit ecc error not corrected",
    [RAS_ERROR_TYPE_MBECC] = "multiple bit ecc error",
    [RAS_ERROR_TYPE_BUS_ERR] = "bus error",
    [RAS_ERROR_TYPE_TIMEOUT_ERR] = "service timeout",
    [RAS_ERROR_TYPE_HB_TIMEOUT] = "heartbeat timeout",
    [RAS_ERROR_TYPE_KO_INS_FAIL] = "ko insert fail",
    [RAS_ERROR_TYPE_INIT_ABNORMAL] = "Initialization abnormal",
    [RAS_ERROR_TYPE_IN_CFG_ERR_MINOR] = "internal config err minor",
    [RAS_ERROR_TYPE_CFG_ERR_MINIOR] = "config error minor",
};

static inline const char *fault_type_to_description(int type)
{
    if ((type >= RAS_ERROR_TYPE_ERROR) && (type < RAS_ERROR_TYPE_ERR_TYPE_MAX)) {
        return fault_type_description[type];
    }
    return "unknown fault type";
}

#ifndef TSDRV_UT
static char *node_type_name[DMS_DEV_TYPE_MAX] = {
    [DMS_DEV_TYPE_HWTS_S_TS] = "HWTS/Stars",
    [DMS_DEV_TYPE_TS_DISP] = "HWTS/Stars_DISP",
    [DMS_DEV_TYPE_TS] = "TS",
    [DMS_DEV_TYPE_TSCPU] = "TSCPU",
    [DMS_DEV_TYPE_AIC] = "AIC",
    [DMS_DEV_TYPE_AIC_DISP] = "AIC_DISP",
    [DMS_DEV_TYPE_L2BUF] = "L2BUFF",
    [DMS_DEV_TYPE_SDMA] = "SDMA",
    [DMS_DEV_TYPE_AIV] = "AIV",
    [DMS_DEV_TYPE_DSA] = "DSA",
    [DMS_DEV_TYPE_DSA_DISP] = "DSA_DISP",
    [DMS_DEV_TYPE_TSFW] = "TSFW MCU"
};
#else
static char *node_type_name[DMS_DEV_TYPE_MAX];
#endif

static inline char *fault_node_type_to_name(int node_type)
{
    if ((node_type >= DMS_DEV_TYPE_SOC) && (node_type < DMS_DEV_TYPE_MAX)) {
        return node_type_name[node_type];
    }
    return "Unknown node type";
}

struct ras_error {
    DMS_DEVICE_NODE_TYPE node_type;
    u8 sensor_status;
    u8 event_attr; /* 0:resume 1:occur 2:one time */
    u32 node_id;
    u32 section_type;
    u32 ras_code;
};

struct fault_list_node {
    struct ras_error error;
    struct list_head list;
};

struct fault_event {
    u32 status;
    u32 tsid;
    u32 sensor_node;
    u32 error_num;
    struct fault_list_node node;
    spinlock_t spinlock;
};

struct fault_dev_node {
    struct dms_node *node;
    u32 sensor_num;
    struct fault_event sensor_event[DMS_MAX_NODE_SENSOR_COUNT];
};

#define L2BUF_IRQ_UNMASKING 0
#define L2BUF_IRQ_UNMASKED 1

struct fault_dev {
    u32 devid;
    spinlock_t spinlock;
    u32 dev_node_num;
    struct fault_dev_node *dev_node;
    atomic_t unmask_flag;
    struct delayed_work unmask_irq_work;
#ifdef CFG_FEATURE_FAULT_FPDC
    FAULT_NOTIFY_FUNC notify_func;
#endif
};

struct tsdrv_fault_ctrl {
    u32 dev_num;
    struct fault_dev fault_dev[TSDRV_MAX_DAVINCI_NUM];
};

extern struct dms_node_operations tsdrv_dev_node_ops;
#define TSDRV_SENSOR_SCAN_INTERVAL DMS_SENSOR_CHECK_INTERVAL_TIME /* 100 ms */

#define GEN_DMS_NODE(_type, _id, _name, _cap, _perm, _devid, _ops) \
        { \
            .node_type = _type, \
            .node_id = _id, \
            .node_name = _name, \
            .capacity = _cap, \
            .permission = _perm, \
            .owner_devid = _devid, \
            .ops = _ops \
        }

#define GEN_SENOR_TABLE_OBJ(_type, _name, _class, _attr, _debounce, _interval, _proc, _enable, \
        _func, _data, _assert, _deassert) \
        { \
            .sensor_type = _type, \
            .sensor_name = _name, \
            .sensor_class = _class, \
            .sensor_class_cfg = { \
                .discrete_sensor = { \
                    .attribute = _attr, \
                    .debounce_time = _debounce \
                } \
            }, \
            .scan_interval = _interval, \
            .proc_flag = _proc, \
            .enable_flag =  _enable, \
            .pf_scan_func = _func, \
            .private_data = _data, \
            .assert_event_mask = _assert, \
            .deassert_event_mask = _deassert \
        }

#define DEV_ID_OFFSET 48
#define DEV_ID_MASK 0xFFFF
#define TS_ID_OFFSET 40
#define TS_ID_MASK 0xFF
#define DEV_NODE_OFFSET 32
#define DEV_NODE_MASK 0xFF
#define NODE_TYPE_OFFSET 16
#define NODE_TYPE_MASK 0xFFFF
#define SENSOR_NODE_OFFSET 0
#define SENSOR_NODE_MASK 0xFFFF

static inline u32 get_sensor_node_from_priv(u64 private_data)
{
    return ((private_data >> SENSOR_NODE_OFFSET) & SENSOR_NODE_MASK);
}

static inline u32 get_dev_node_from_priv(u64 private_data)
{
    return ((private_data >> DEV_NODE_OFFSET) & DEV_NODE_MASK);
}

static inline u32 get_node_type_from_priv(u64 private_data)
{
    return ((private_data >> NODE_TYPE_OFFSET) & NODE_TYPE_MASK);
}

static inline u32 get_ts_id_from_priv(u64 private_data)
{
    return ((private_data >> TS_ID_OFFSET) & TS_ID_MASK);
}

static inline u32 get_dev_id_from_priv(u64 private_data)
{
    return ((private_data >> DEV_ID_OFFSET) & DEV_ID_MASK);
}

static inline u32 get_node_id_from_priv(u64 private_data)
{
    return ((private_data >> TS_ID_OFFSET) & TS_ID_MASK);
}

#define SENSOR_PRIV_DATA(devid, tsid, node_type, dev_node, sensor_node) \
    (((u64)(devid & DEV_ID_MASK) << DEV_ID_OFFSET) | \
    ((u64)(tsid & TS_ID_MASK) << TS_ID_OFFSET) | \
    ((u64)(dev_node & DEV_NODE_MASK) << DEV_NODE_OFFSET) | \
    ((u64)(node_type & NODE_TYPE_MASK) << NODE_TYPE_OFFSET) | \
    ((u64)(sensor_node & SENSOR_NODE_MASK) << SENSOR_NODE_OFFSET))

#ifdef CFG_FEATURE_FAULT
struct dms_sensor_object_cfg *tsdrv_get_sensor_obj_table(u32 devid);
int fault_get_node_type(u32 devid, u32 node_id);
struct fault_dev *tsdrv_get_fault_dev(u32 devid);
int tsdrv_fault_dev_init(u32 devid);
void tsdrv_fault_dev_exit(u32 devid);
void tsdrv_fault_init(void);
void tsdrv_fault_exit(void);
#ifdef CFG_FEATURE_FAULT_FPDC
#define RAS_L2BUFF_CE       0x0200
#define RAS_L2BUFF_CE_MASK  0xFF00
#endif
u32 fault_ctrl_get_num(void);

#else
static inline int tsdrv_fault_dev_init(u32 devid)
{
    return 0;
}

static inline void tsdrv_fault_dev_exit(u32 devid)
{
}

static inline void tsdrv_fault_init(void)
{
}

static inline void tsdrv_fault_exit(void)
{
}

#ifndef CFG_FEATURE_USE_FAULT_NEW_FRAME_WORK
static inline int tsdrv_heartbeat_fault_event_add(u32 devid, u32 tsid)
{
    return 0;
}
#endif /* CFG_FEATURE_USE_FAULT_NEW_FRAME_WORK */

#endif /* CFG_FEATURE_FAULT */

#endif /* TSDRV_FAULT_INIT_H */

