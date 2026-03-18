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

#include "can_drv_fault_mgr.h"
#include "drvfault_user_common.h"
#include "can_drv_log.h"
#include "can_drv_core.h"
#include "dms_node_type.h"
#include "dms_interface.h"
#include "can_drv_dms_init.h"
#include <linux/kallsyms.h>
#include <linux/timer.h>
#include <linux/module.h>
#include <linux/version.h>
#ifdef RUN_IN_AOS
#include "aos_sys.h"
#endif

#ifdef STATIC_SKIP
  #define STATIC
#else
  #define STATIC static
#endif


/* report fault to devmng */
#define DEV_ID              0
#define EVENT_CODE_ID       0x66

/* Fault detection and recovery */
#define FAULT_DETECTION_TIMER_DELAY     (5 * HZ)    /* 5s */
#define FAULT_DETECTION_TIMER_INTERVAL  (1 * HZ)    /* 1s */
#define MTTCAN_TX_TIMEOUT               (1 * HZ)

#define CAN_ERR_SECTOR_TYPE (0xFFU)
// to do module_id
#define CAN_SAFETY_COVERAGE_ITEM(_err_status, _describe, _err_type) { \
    .subsys_id = DMS_DEV_TYPE_CAN,  \
    .module_id = 0,   \
    .section_type = CAN_ERR_SECTOR_TYPE,  \
    .ras_code.err_status = _err_status,     \
    .describe = _describe,                  \
    .sensor_type = DMS_SEN_TYPE_BUS_SENSOR, \
    .error_type = _err_type                 \
}

struct mttcan_fault_event_node {
    u16 can_id;
    u16 err_num;
    enum event_err_level level;
    enum event_direct direct;

    struct list_head list;
};

const dfm_event *can_converge(const dfm_event *items, u32 items_num,
                              u32 section_type, u64 ras_code)
{
    int i;

    for (i = 0; i < items_num; i++) {
        if (section_type == items[i].section_type) {
            if ((u32)ras_code == items[i].ras_code.err_status) {
                return &items[i];
            }
        }
    }

    mttcan_err("not support fault. section_type = %d, ras_code = %lld\n", section_type, ras_code);
    return NULL;
}

const dfm_event *can_safety_converge(u32 section_type, u64 ras_code)
{
    // To adapt to the dfm_add_event interface, ras_code indicates the event ID.
    // The error_type and event ID are sent to the fault management module.
    static dfm_event can_converage_tab[CAN_CONVERAGE_NUM] = {
        CAN_SAFETY_COVERAGE_ITEM(DMS_ERROR_TS_LOST,
            "bus's timestamp is abnormal.", DMS_ERROR_TS_LOST),
        CAN_SAFETY_COVERAGE_ITEM(DMS_ERROR_RX_TX_ERR,
            "TX/RX events are lost.", DMS_ERROR_RX_TX_ERR),
        CAN_SAFETY_COVERAGE_ITEM(DMS_ERROR_BUS_OVERLOAD,
            "bus overload.", DMS_ERROR_BUS_OVERLOAD),
        CAN_SAFETY_COVERAGE_ITEM(DMS_ERROR_BUS_OFF_BASE,
            "bus bus-off state.", DMS_ERROR_BUS_OFF_BASE),
    };

    u32 items_num = (u32)ARRAY_SIZE(can_converage_tab);
    return can_converge(can_converage_tab, items_num, section_type, ras_code);
}

int can_fault_handle(u16 dev_id, enum dms_event_err_num err_num, enum event_direct direct)
{
#if !defined(DFM_DISABLE)
    int ret = -EFAULT;
    u64 ras_code = (u64)(err_num & 0xFF);
    struct dfm_struct *can_dfm = can_get_drv_dms(dev_id);
    const dfm_event *coverage_node = NULL;

    if (can_dfm == NULL) {
        mttcan_err("can_get_drv_dms fail\n");
        return -EFAULT;
    }

    mttcan_err("can fault info(dev_id = %d, err_num = %d, direct = %d)\n", dev_id, err_num, direct);
    coverage_node = can_safety_converge(CAN_ERR_SECTOR_TYPE, ras_code);
    if (coverage_node == NULL) {
        mttcan_err("can_safety_converge fail\n");
        return -EFAULT;
    }

    switch (direct) {
        case EVENT_DIRECT_ASSERT:
        case EVENT_DIRECT_NOTIFY:
            ret = dfm_add_event(can_dfm, 0, coverage_node); // node_id = 0, each CAN has only one node.
            break;
        case EVENT_DIRECT_DEASSERT:
            ret = dfm_remove_event(can_dfm, 0, coverage_node);
            break;
    }
    if (ret != 0) {
        if (ret != -EEXIST) {
            mttcan_err("dfm add or remove fail, type = %d\n", direct);
        }
        return ret;
    }

    // There is only one dev_node of can and one sensor of each node. so idx = 0.
    ret = dms_sensor_event_notify(0, &can_dfm->dev_nodes[0].sensors[0].dms_sensor);
    if (ret != 0) {
        mttcan_err("call dms_sensor_event_notify to report event fail, ret = %d.\n", ret);
    }

    return ret;
#else
    return 0;
#endif
}

void mttcan_fault_event_handler(struct work_struct *work)
{
    struct list_head *pos = NULL, *n = NULL;
    struct mttcan_fault_event_node *fault_event = NULL;

    struct mttcan_priv *priv = container_of(work, struct mttcan_priv, fault_report_work);
    if (unlikely(priv == NULL)) {
        mttcan_err("mttcan private data is null\n");
        return;
    }

    if (list_empty_careful(&priv->fault_event_list)) {
        mttcan_warn("list of mttcan[%u] is empty.\n", priv->cfg.mttcan_id);
        return;
    }

    list_for_each_safe(pos, n, &priv->fault_event_list) {
        fault_event = list_entry(pos, struct mttcan_fault_event_node, list);
        list_del(&fault_event->list);
        can_fault_handle(fault_event->can_id, (enum dms_event_err_num)fault_event->err_num, fault_event->direct);
        kfree((void *)fault_event);
    }
}

int mttcan_report_fault_event(struct mttcan_priv *priv, enum dms_event_err_num err_num,
    enum event_err_level level, enum event_direct direct)
{
    struct mttcan_fault_event_node *fault_event = NULL;
    if (priv == NULL) {
        mttcan_err("mttcan priv data is null\n");
        return -EFAULT;
    }

    fault_event = (struct mttcan_fault_event_node *)
        kmalloc(sizeof(struct mttcan_fault_event_node), GFP_KERNEL);
    if (fault_event == NULL) {
        mttcan_err("kmalloc fault event failed.\n");
        return -ENOMEM;
    }
    fault_event->can_id = priv->cfg.mttcan_id;
    fault_event->err_num = err_num;
    fault_event->level = level;
    fault_event->direct = direct;

    list_add_tail(&fault_event->list, &priv->fault_event_list);
#ifndef RUN_IN_AOS
    schedule_work(&priv->fault_report_work);
#else
    schedule_work_on(AOS_GetCoreID(), &priv->fault_report_work);
#endif

    return 0;
}

#ifdef SUPPORT_MTTCAN_DFX
static void print_audit_last_logs(struct mttcan_priv *priv)
{
    int log_item;
    int ret;
    for (log_item = 0; log_item < CAN_LOG_MAX_VALUE; ++log_item) {
        if (priv->audit_log_last_msg[log_item][0] == '\0') {
            continue;
        }

        if (need_print_audit_log(priv, (enum can_log_item)log_item)) {
            mttcan_info("%s", priv->audit_log_last_msg[log_item]);
            ret = memset_s(priv->audit_log_last_msg[log_item], AUDIT_LOG_MAX_LEN, 0, AUDIT_LOG_MAX_LEN);
            if (ret != EOK) {
                mttcan_warn("audit_log_last_msg not cleared.(ret=%d)\n", ret);
            }
        }
    }
}
#endif

#ifdef SUPPORT_MTTCAN_FAULT_RECOVERY
void mttcan_fault_recovery_routine(struct timer_list *pram)
{
    struct mttcan_priv *priv = from_timer(priv, pram, fault_revovery_timer);
    struct net_device *ndev = priv->ndev;
    if (ndev == NULL) {
        mttcan_err("run mttcan fault detection and recovery error. ndev is NULL.\n");
        return;
    }

    if (!mttcan_working(priv)) {
        goto routine_out;
    }

    if (priv->last_busy_tx_time != 0 && (jiffies - priv->last_busy_tx_time) > MTTCAN_TX_TIMEOUT) {
        mttcan_tx_complete(ndev);
        mttcan_tx_cancel_finish(ndev);
    }

    if (priv->last_netif_stop_time > 0 &&
        (jiffies - priv->last_netif_stop_time) > FAULT_DETECTION_TIMER_INTERVAL &&
        (!mttcan_tx_fifo_full(priv)) && mttcan_working(priv)) {
        if (netif_queue_stopped(ndev)) {
            netif_wake_queue(ndev);
            priv->last_netif_stop_time = 0;
        }
    }
#ifdef SUPPORT_MTTCAN_DFX
    print_audit_last_logs(priv);
#endif
routine_out:
    mod_timer(&priv->fault_revovery_timer, jiffies + FAULT_DETECTION_TIMER_INTERVAL);
}

void mttcan_init_fault_recovery_timer(struct net_device *ndev)
{
    struct mttcan_priv *priv = netdev_priv(ndev);
    struct timer_list *timer = &priv->fault_revovery_timer;

    timer_setup(timer, mttcan_fault_recovery_routine, 0);
    timer->expires = jiffies + FAULT_DETECTION_TIMER_DELAY;
    add_timer(timer);
    mttcan_info("%s: create mttcan recovery timer finished.\n", priv->cfg.can_name);
}

void mttcan_stop_fault_recovery_timer(struct net_device *ndev)
{
    struct mttcan_priv *priv = netdev_priv(ndev);
    if (del_timer_sync(&priv->fault_revovery_timer)) {
        mttcan_err("delete fault detection and recovery timer failed.\n");
    }
}
#endif

/**
* need_print_audit_log - print audit log to log file
*/
#ifdef SUPPORT_MTTCAN_DFX
bool need_print_audit_log(struct mttcan_priv *priv, enum can_log_item log_item)
{
    unsigned long timeout = priv->audit_log_last_time[log_item] + HZ;
    if (time_before(jiffies, timeout)) {
        return false;
    }

    priv->audit_log_last_time[log_item] = jiffies;
    return true;
}
#endif
