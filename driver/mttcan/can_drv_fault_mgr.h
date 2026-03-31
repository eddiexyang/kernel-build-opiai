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

#ifndef _CAN_DRV_FAULT_MGR_H
#define _CAN_DRV_FAULT_MGR_H

#include <linux/netdevice.h>
#include <linux/workqueue.h>
#include "dms_sensor_type.h"

struct mttcan_priv;

enum dms_event_err_type {
    DMS_ERROR_TS_LOST = 0,
    DMS_ERROR_RX_TX_ERR = 1,
    DMS_ERROR_BUS_OVERLOAD = 2,
    DMS_ERROR_BUS_OFF_BASE = 3,
};

/* The value indicates the event type value */
enum dms_event_err_num {
    DMS_EVENT_TS_LOST        = ((DMS_SEN_TYPE_BUS_SENSOR << 0x8) | DMS_ERROR_TS_LOST),
    DMS_EVENT_RX_TX_ERR      = ((DMS_SEN_TYPE_BUS_SENSOR << 0x8) | DMS_ERROR_RX_TX_ERR),
    DMS_EVENT_BUS_OVERLOAD   = ((DMS_SEN_TYPE_BUS_SENSOR << 0x8) | DMS_ERROR_BUS_OVERLOAD),
    DMS_EVENT_BUS_OFF_BASE   = ((DMS_SEN_TYPE_BUS_SENSOR << 0x8) | DMS_ERROR_BUS_OFF_BASE),
};

enum event_err_num {
    EVENT_ERR_TS_LOST = 0x300,
    EVENT_ERR_TX_EVENT_LOST = 0x310,
    EVENT_ERR_BUS_OVERLOAD = 0x320,
    EVENT_ERR_BUS_OFF_BASE = 0x330,
    EVENT_ERR_WRITE_REG_FAIL = 0x340,
};

enum event_err_level {
    EVENT_ERR_LEVEL_OK = 0,
    EVENT_ERR_LEVEL_WARNING,
    EVENT_ERR_LEVEL_ERROR,
    EVENT_ERR_LEVEL_FATAL,
};

/**
 * event report type
 *
 * refer to @drvfault_event.event.assertion, the value defined as below:
 * 0-resume, 1-occur, 2-one time
 */
enum event_direct {
    EVENT_DIRECT_ASSERT = 0,
    EVENT_DIRECT_DEASSERT = 1,
    EVENT_DIRECT_NOTIFY = 2
};

#define MTTCAN_LOG_VALUE_BASE    0x50000000
#define MTTCAN_OPERATION_LOG     0x1000
#define MTTCAN_SECURITY_LOG      0x2000
#define MTTCAN_RUNNING_LOG       0x4000

enum can_log_item {
    CAN_LOG_DEVICE_UP_FAIL = 0x1,
    CAN_LOG_DEVICE_UP_SUCC = 0x2,
    CAN_LOG_DEVICE_DOWN = 0x3,
    CAN_LOG_RAM_CFG_GET_FAIL = 0x4,
    CAN_LOG_RAM_CFG_GET_SUCC = 0x5,
    CAN_LOG_FILTER_GET_FAIL = 0x6,
    CAN_LOG_FILTER_GET_SUCC = 0x7,
    CAN_LOG_PM_SUSPEND = 0x08,
    CAN_LOG_PM_RESUME_FAIL = 0x09,
    CAN_LOG_PM_RESUME_SUCC = 0x0a,
    CAN_LOG_ENTER_WARNING_STATUS = 0x0b,
    CAN_LOG_ENTER_PASSIVE_STATUS = 0x0c,
    CAN_LOG_ENTER_ACTIVE_STATUS = 0x0d,
    CAN_LOG_ENTER_BUSOFF_STATUS = 0x0e,
    CAN_LOG_BUSLOAD_WARNING = 0x0f,
    CAN_LOG_TX_BUSY = 0x10,
    CAN_LOG_BUSOFF_WARNING = 0x11,
    CAN_LOG_BUSOFF_CFG_GET_FAIL = 0x12,
    CAN_LOG_BUSOFF_CFG_GET_SUCC = 0x13,
    CAN_LOG_TDC_CFG_GET_FAIL = 0x14,
    CAN_LOG_TDC_CFG_GET_SUCC = 0x15,
    CAN_LOG_CFG_GET_FAIL = 0x16,
    CAN_LOG_CFG_GET_SUCC = 0x17,
    CAN_LOG_MAX_VALUE,   /* Used by mttcan_priv struct */
};

#define TO_OPLOG(can_log)  (MTTCAN_LOG_VALUE_BASE + MTTCAN_OPERATION_LOG + (can_log))
#define TO_SECLOG(can_log)  (MTTCAN_LOG_VALUE_BASE + MTTCAN_SECURITY_LOG + (can_log))
#define TO_RUNLOG(can_log)  (MTTCAN_LOG_VALUE_BASE + MTTCAN_RUNNING_LOG + (can_log))

#if defined(CONFIG_CAN_FAULT_REPORT)
int report_fault_to_devmng(
    enum event_err_num err_num,
    enum event_err_level level,
    enum event_direct direct);

void mttcan_fault_event_handler(struct work_struct *work);

int mttcan_report_fault_event(
    struct mttcan_priv *priv,
    enum dms_event_err_num err_num,
    enum event_err_level level,
    enum event_direct direct);

#else
static inline int report_fault_to_devmng(
    enum event_err_num err_num,
    enum event_err_level level,
    enum event_direct direct)
{
    return 0;
}

static inline void mttcan_fault_event_handler(struct work_struct *work)
{
    return 0;
}

static inline int mttcan_report_fault_event(
    struct mttcan_priv *priv,
    enum dms_event_err_num err_num,
    enum event_err_level level,
    enum event_direct direct)
{
    return 0;
}
#endif /* CONFIG_CAN_FAULT_REPORT */


#ifdef SUPPORT_MTTCAN_FAULT_RECOVERY
void mttcan_init_fault_recovery_timer(struct net_device *ndev);
void mttcan_stop_fault_recovery_timer(struct net_device *ndev);
#else
static inline void mttcan_init_fault_recovery_timer(struct net_device *ndev) {}
static inline void mttcan_stop_fault_recovery_timer(struct net_device *ndev) {}
#endif

#endif /* _CAN_DRV_FAULT_MGR_H */
