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

#ifndef TSDRV_L2BUFF_SAFETY_H
#define TSDRV_L2BUFF_SAFETY_H

#if defined(CFG_SOC_PLATFORM_MINIV2) && !defined(CFG_SOC_PLATFORM_HELPER)

#include "drvfault_user_common.h"
#include "fpdc.h"
#include "dms_sensor.h"

#define  L2BUFF_RAS_ERR_STATUSL_INDX         4       // RegName : ERR_STATUSL               0x2010
#define  L2BUFF_SAFETY_RINT_INDX             12      // RegName : L2BUFF_SAFETY_RINT        0x0824
#define  L2BUFF_SAFETY_ERR_RINT_INDX         13      // RegName : L2BUFF_SAFETY_ERR_RINT    0x0834

#define  L2BUFF_SECTION_TYPE_SAFETY          0xAAU
#define  L2BUFF_SECTION_TYPE_RAS             0xBBU

#define L2BUFF_SAFETY_BIT0_MASK              0x00000001      // bit 0 mask
#define L2BUFF_SAFETY_BIT2_MASK              0x00000004      // bit 2 mask
#define L2BUFF_SAFETY_BIT3_MASK              0x00000008      // bit 3 mask
#define L2BUFF_SAFETY_BIT4_MASK              0x00000010      // bit 4 mask

#define L2BUFF_RAS_BIT_MASK                  0x0000FF00      // bit 15:8 mask
#define L2BUFF_REGISTER_ARRAY_SIZE           14
#define L2BUFF_SUBNODE_ID_MAX                7

#define L2BUFF0                              0
#define L2BUFF1                              1
#define L2BUFF2                              2
#define L2BUFF3                              3
#define L2BUFF4                              4
#define L2BUFF5                              5
#define L2BUFF6                              6
#define L2BUFF7                              7

#define BIT_NUM_8                            8
#define BIT_NUM_2                            2
#define BIT_NUM_3                            3
#define BIT_NUM_4                            4

struct l2buff_safety_faultinfo {
    unsigned int bitid;
    unsigned int raserrtype;
    unsigned char event_severity;
    unsigned char sensor_type;
};

struct l2buff_safety_silsinfo {
    unsigned int node_id;                  // L2BUFF0~7
    unsigned int bitid_sils;               // 中断号%32
};

enum l2buff_safety_bit {
    L2BUFF_SAFETY_BIT0 = 0,
    L2BUFF_SAFETY_BIT2 = 2,
    L2BUFF_SAFETY_BIT3 = 3,
    L2BUFF_SAFETY_BIT4 = 4
};

enum l2buff_ras_err {
    L2BUFF_RAS_ILLEGAL_SECURITY_ATTRIBUTE_ERR = 0x3,   // Illegal security attribute error
    L2BUFF_RAS_ILLEGAL_REQUEST_ACCESS = 0x4,           // Illegal request access
    L2BUFF_RAS_ILLEGAL_READ_ACCESS = 0x5,              // Illegal read access
    L2BUFF_RAS_ILLEGAL_WRITE_ACCESS = 0x6,             // Illegal write access
    L2BUFF_RAS_ILLEGAL_ACCESS_THR_CHANNEL = 0x7,       // Illegal access thr channel
    L2BUFF_RAS_DEBUG_ILLEGAL_ACCESS = 0x8,             // debug illegal access
    L2BUFF_RAS_ACCESS_ADD_OUT_BOUNDS = 0x9,            // access add out bounds
    L2BUFF_RAS_NON_COMPLIANT_RSV_REQUEST = 0xA,        // non compliant rsv request
    L2BUFF_RAS_REQUEST_ATTRIBUTE_ERR = 0xB,            // request attribute error
    L2BUFF_RAS_INIT_NOT_COMPLETE = 0xC,                // init not complete
    L2BUFF_RAS_ATOMIC_CALCULATION_ERR = 0xD            // statistics to atomic calculation error
};

struct  irqnum2emuid {
    u8  irqnum;
    u8  emuid;
};

/**
 * register fault irq info to fault drv
 *
 * when where are safety interrupt has occur in modules, fault driver
 * will call notifier function @soc_misc_safety_notifier.
 */
int tsdrv_register_l2buff_safety_irq(u32 devid);

/**
 * unregister fault irq info to fault drv
 *
 * this function be called when driver exit.
 */
void tsdrv_unregister_l2buff_safety_irq(u32 devid);

/**
 * safety irq notifier.
 *
 * this function will be registered to fault driver.
 */
int tsdrv_l2buff_safety_handler(struct safety_fault_info *fault_info,
    unsigned int *event_num, struct safety_event **event_list);

/**
 * send notifier to fault drv
 *
 * In MDC ASCEND610, if an ECC fault is included in the received RAS fault,
 * the fault is actually a safety fault. In this case, the fault driver
 * needs to be notified to send an interrupt masking IPC message to
 * safetyIsland.
 */
int l2buff_safety_notifier(u8 sensor_id, const struct notify_data *pdata);

/* change to dfm template */
int tsdrv_register_l2buff_dms_node(u32 dev_id);
void tsdrv_unregister_l2buff_dms_node(u32 dev_id);

#else
int tsdrv_register_l2buff_safety_irq(u32 devid)
{
    return 0;
}

void tsdrv_unregister_l2buff_safety_irq(u32 devid)
{
}
#endif

#endif
