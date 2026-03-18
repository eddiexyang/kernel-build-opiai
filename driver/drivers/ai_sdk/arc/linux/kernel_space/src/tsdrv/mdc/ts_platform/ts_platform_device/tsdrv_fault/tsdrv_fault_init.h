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
#include "drvfault_user_common.h"
#include "drv_ipc.h"
#include "tsdrv_common_fault_platform.h"
#ifdef CFG_FEATURE_FAULT_FPDC
#include "fpdc.h"
#include "fpdc_ras_receiver.h"
#endif

#define RAS_SRAM_MULTI_BIT_ECC_UER 0x0106
#define RAS_SRAM_SINGLE_BIT_ECC_CE 0x0206
#define RAS_L2BUF_CFG_UEO_0  0x0301
#define RAS_L2BUF_CFG_UEU_1  0x0401
#define RAS_L2BUF_CFG_UEU_2  0x0501
#define RAS_L2BUF_CFG_UEU_3  0x0601
#define RAS_L2BUF_CFG_UEU_4  0x0701
#define RAS_L2BUF_CFG_UEO_5  0x0801
#define RAS_L2BUF_CFG_UER_6  0x090D
#define RAS_L2BUF_CFG_UER_7  0x0A01
#define RAS_L2BUF_CFG_UER_8  0x0B01
#define RAS_L2BUF_CFG_UER_9  0x0C01
#define RAS_L2BUF_CFG_UER_10 0x0D01
#define RAS_L2BUF_CFG_UER_11 0x0E01
#define RAS_L2BUF_CFG_UER_12 0x0F01
#define RAS_L2BUF_CFG_UER_13 0x1001
#define RAS_L2BUF_CFG_UER_14 0x1101
#define RAS_L2BUF_CFG_UER_15 0x1201

#define RAS_ERR_CODE_INVAILD       0xFFFF
#define OEM_ERR_STATUS_MASK        0xFFFF

#define GEN_DEV_NODE_TABLE_OBJ(_type, _id, _name, _cap, _perm, _devid, _ops) \
    { \
        .node_type = _type, \
        .node_id = _id, \
        .node_name = _name, \
        .capacity = _cap, \
        .permission = _perm, \
        .owner_devid = _devid, \
        .ops = _ops \
    }

enum fault_event_attr {
    EVENT_RESUME_ATTR = 0,
    EVENT_OCCUR_ATTR = 1,
    EVENT_ONE_TIME_ATTR = 2,
    EVENT_ATTR_MAX
};

enum dev_node_id {
    DEV_NODE_HWTS = 0,
    DEV_NODE_A55,
     DEV_NODE_AIC,
#ifndef CFG_SOC_PLATFORM_MDC_V51
    DEV_NODE_TS,
    DEV_NODE_L2BUF,
#endif
    DEV_NODE_SDMAM,
    DEV_NODE_TSC,
    DEV_NODE_TSV,
    DEV_NODE_AIV,
#ifndef CFG_SOC_PLATFORM_MDC_V51
    DEV_NODE_DSA,
#endif
    DEV_NODE_ID_MAX
};

enum sensor_node_id {
    SENSOR_NODE_HWTS = 0,
    SENSOR_NODE_A55,
    SENSOR_NODE_AIC,
    SENSOR_NODE_TS,
    SENSOR_NODE_L2BUF,
    SENSOR_NODE_SDMAM,
    SENSOR_NODE_HEARTBEAT,
    SENSOR_NODE_AIV,
    SENSOR_NODE_DSA,
    SENSOR_NODE_ID_MAX
};

struct ras_error {
    DMS_DEVICE_NODE_TYPE node_type;
    u8 sensor_status;
    u8 event_attr; /* 0:resume 1:occur 2:one time */
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
int tsdrv_fault_event_scan(u64 private_data, struct dms_sensor_event_data *data);
#define TSDRV_SENSOR_SCAN_INTERVAL DMS_SENSOR_CHECK_INTERVAL_TIME /* 100 ms */
#define TSDRV_FAULT_EVENT_SCAN_HWTS tsdrv_fault_event_scan
#define TSDRV_FAULT_EVENT_SCAN_TS tsdrv_fault_event_scan
#define TSDRV_FAULT_EVENT_SCAN_A55 tsdrv_fault_event_scan
#define TSDRV_FAULT_EVENT_SCAN_AIC tsdrv_fault_event_scan
#define TSDRV_FAULT_EVENT_SCAN_L2BUF tsdrv_fault_event_scan
#define TSDRV_FAULT_EVENT_SCAN_SDMA tsdrv_fault_event_scan
#define TSDRV_FAULT_EVENT_SCAN_DSA tsdrv_fault_event_scan

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
#define TS_ID_OFFSET 32
#define TS_ID_MASK 0xFFFF
#define DEV_NODE_OFFSET 16
#define DEV_NODE_MASK 0xFFFF
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

static inline u32 get_dev_id_from_priv(u64 private_data)
{
    return ((private_data >> DEV_ID_OFFSET) & DEV_ID_MASK);
}

static inline u32 get_node_id_from_priv(u64 private_data)
{
    return ((private_data >> TS_ID_OFFSET) & TS_ID_MASK);
}
#define SENSOR_PRIV_DATA(devid, tsid, dev_node, sensor_node) \
    (((u64)(devid & DEV_ID_MASK) << DEV_ID_OFFSET) | \
        ((u64)(tsid & TS_ID_MASK) << TS_ID_OFFSET) | \
        ((u64)(dev_node & DEV_NODE_MASK) << DEV_NODE_OFFSET) | \
        ((u64)(sensor_node & SENSOR_NODE_MASK) << SENSOR_NODE_OFFSET))

#ifdef CFG_FEATURE_FAULT
int tsdrv_heartbeat_fault_event_add(u32 devid, u32 tsid);
struct dms_sensor_object_cfg *tsdrv_get_sensor_obj_table(u32 devid);
int tsdrv_fault_dev_init(u32 devid);
void tsdrv_fault_dev_exit(u32 devid);
void tsdrv_fault_init(void);
void tsdrv_fault_exit(void);
u32 get_devnode_id_ipc(DMS_DEVICE_NODE_TYPE node_type);
u32 get_sensor_node_id_ipc(DMS_DEVICE_NODE_TYPE node_type);
u32 get_sensor_node_id_fpdc(DMS_DEVICE_NODE_TYPE node_type);
bool tsdrv_exist_irq_in_dts(struct safety_irq_info *irq_infos, unsigned int num);
int tsdrv_send_fault_info_to_ts(u32 devid, rproc_id_t rproc_id, struct ras_message *info);
void tsdrv_fill_fault_info(struct safety_event *fault_event, struct ras_message *info);
#ifdef CFG_FEATURE_FAULT_FPDC
bool tsdrv_fault_is_need_converged(DMS_DEVICE_NODE_TYPE node_type);
u32 tsdrv_parse_ras_code(const struct notify_data *pdata);
#endif
u32 tsdrv_get_section_type(DMS_DEVICE_NODE_TYPE node_type);
const struct ras_fault_converge_item *tsdrv_fault_converge(
    u32 section_type, u64 ras_code);
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

static inline int tsdrv_heartbeat_fault_event_add(u32 devid, u32 tsid)
{
    return 0;
}

static inline bool tsdrv_exist_irq_in_dts(struct safety_irq_info *irq_infos, unsigned int num)
{
    return true;
}

static inline void tsdrv_fill_fault_info(struct safety_event *fault_event, struct ras_message *info)
{
}
static inline int tsdrv_send_fault_info_to_ts(u32 devid, rproc_id_t rproc_id, struct ras_message *info)
{
    return 0;
}
#endif /* CFG_FEATURE_FAULT */

#endif /* TSDRV_FAULT_INIT_H */

