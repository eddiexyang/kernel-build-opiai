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
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/kfifo.h>
#include <linux/delay.h>
#include <linux/time.h>

#include "dms_kernel_version_adapt.h"
#include "dms_define.h"
#include "dms_cmd_def.h"
#include "icm_interface.h"
#include "dms_ipc_common.h"
#include "dms_ipc_interface.h"
#include "kernel_version_adapt.h"

#define DMANAGE_ENTIRE_SOC 0xFF

/* cmd type1: */
#define LPM3_TEMPERATURE 11
#define LPM3_VOLTAGE 12
#define LPM3_HEART_BEAT 25
#define LPM3_AICORE_FREQ 26
#define LPM3_DDR_FREQ 27
#define LPM3_DDR_VOLUME 28
#define LPM3_SUGGEST_LIMIT 29
#define LPM3_CANCEL_LIMIT 30
#define LPM3_TSENSOR_RESET_ALARM 31
#define LPM3_CANCEL_RESET_ALARM 32
#define LPM3_EXCEPTION_INFO 34

/* cmd type0: */
#define LPM3_QUERY_CMD 2
#define LPM3_LOGLEVEL_CMD 3
#define LPM3_NOTIFY_CMD 4
#define LPM3_HEART_BEAT_QUERY 8

/* obj or src id: */
#define LPM3_HEART_ID 3
#define LPM3_PSCI_ID 10
#define LPM3_ID 2
#define DEVICE_CCPU_ID 1
#define LPM3_SMOKE 24
#define LPM3_THRESHOLD 32
#define LPM3_EDP 36

static int soc_lpm_ack_check(u32 dev_id, const lpm_ipc_ctrl_t *lpm_ipc, struct dms_ipc_lpm *ack_msg)
{
    int ret;

    ret = lp_ack_check_common(dev_id, lpm_ipc, ack_msg);
    if (ret) {
        dms_err("Get invalid ipc message. (ret=%d)\n", ret);
        return ret;
    }

    if (ack_msg->source_id != LPM3_ID) {
        dms_err("Get invalid ipc message.\n");
        return -EINVAL;
    }
    return 0;
}

/* ipc ctrl format:
 * {INFO_TYPE,   CORE_ID,    SEND_LEN,   RECV_LEN,
 *      ACK_POS,    ACK_LEN,    REPEATS_TIMES,  IPC_CHAN_TYPE, IPC_CHANNEL
 *      send_ctrl_func,     ack_check_func,  ret_set_func,
 *      {IPC_SEND_D0, IPC_SEND_D1, IPC_SEND_D2, IPC_SEND_D3...}}
 * */
static const lpm_ipc_ctrl_t g_ipc_format_list[] = {
    /* Temperature */
    {LPM_TEMPERATURE,   LPM_SOC_ID,     MSG_HEADS_WORD,   MSG_HEADS_WORD,
        5, 1, NO_REPEATS, IPC_OBJ_LP, DMS_LPM_TX_LPM3,
        NULL, soc_lpm_ack_check, lp_ack_set_value_common,
        {LPM3_TEMPERATURE,    LPM3_QUERY_CMD,  LPM3_PSCI_ID, DEVICE_CCPU_ID, DMANAGE_ENTIRE_SOC}},
    {LPM_TEMPERATURE,   LPM_THERMAL_THRESHOLD_ID,     MSG_HEADS_WORD,   MSG_HEADS_WORD,
        4, 2, NO_REPEATS, IPC_OBJ_LP, DMS_LPM_TX_LPM3,
        NULL, soc_lpm_ack_check, lp_ack_set_value_common,
        {LPM3_THRESHOLD,    LPM3_QUERY_CMD,  LPM3_PSCI_ID, DEVICE_CCPU_ID, DMANAGE_ENTIRE_SOC}},

    /* Freq */
    {LPM_FREQUENCY,     LPM_DDR_ID,     MSG_HEADS_WORD, MSG_HEADS_WORD,
        4, 2, NO_REPEATS, IPC_OBJ_LP, DMS_LPM_TX_LPM3,
        NULL, soc_lpm_ack_check, lp_ack_set_value_common,
        {LPM3_DDR_FREQ,       LPM3_QUERY_CMD,  LPM3_PSCI_ID, DEVICE_CCPU_ID}},

    /* use rate */
    {LPM_USE_RATE,     LPM_AICORE0_ID,  MSG_HEADS_WORD, MSG_HEADS_WORD,
        5, 1, NO_REPEATS, IPC_OBJ_TS, DMS_LPM_TX_TS,
        NULL, NULL, lp_ack_set_value_common,
        {8,       4,  0, 0}},
    {0},
};

const lpm_ipc_ctrl_t *get_lpm_ipc_ctrl_from_lpm(u32 info_type, u32 core_id)
{
    int i;
    int len;

    /* Find ipc msg table */
    len = sizeof(g_ipc_format_list) / sizeof(lpm_ipc_ctrl_t);
    for (i = 0; i < len; i++) {
        if ((core_id == g_ipc_format_list[i].core_id) &&
            (info_type == g_ipc_format_list[i].info_type)) {
            break;
        }
    }
    if (i == len) {
        return NULL;
    }

    return &g_ipc_format_list[i];
}