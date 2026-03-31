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

enum IpcCmdMode {
    CMD_ON,          /* 0 */
    CMD_OFF,         /* 1 */
    CMD_INQUIRY,     /* 2 */
    CMD_SETTING,     /* 3 */
    CMD_NOTIFY,      /* 4 */
    CMD_TEST,        /* 5 */
    CMD_IMP_ERRCODE, /* 6 */

    CMD_IMU_MCU_MSG = 5,    /* used for OBJ_AP for mcu */
    CMD_IMU_SMOKE = 6,      /* used with OBJ_AP */
    CMD_IMU_DMP_MSG = 7,

    MAX_CMD_MODE = 10
};

/* CMD TYPE0 */
#define IPC_IMU_SMOKE 1
#define IPC_IMU_TEMP 6
#define IPC_IMU_POWER 6
#define IPC_IMU_VOLTAGE 6
#define IPC_IMU_HEART_BEAT 7
#define IPC_IMU_INFO 8
#define IPC_IMU_DDR 8
#define IPC_IMU_HBM 9
#define IPC_IMU_DEBUG 9
#define IPC_IMU_AICORE 2

/* CMD TYPE1 */
#define IPC_IMU_POWER_1 14
#define IPC_IMU_TEMP_1 11
#define IPC_IMU_FREQ_1 10
#define IPC_IMU_ECC_1 7
#define IPC_IMU_VOLTAGE_1 12
#define IPC_IMU_HEART_BEAT_1 1
#define IPC_IMU_DEBUG_1 0

#define TEMP_FROM_IMU_NUM 3
#define IPC_SEND_THRESHOLD 3

static bool temp_imu_is_need_send(u32 dev_id, const lpm_ipc_ctrl_t *lpm_ipc)
{
    static struct timeval ipc_send_timestamp[DEVICE_NUM_MAX] = {0};
    struct timeval temp_timestamp[DEVICE_NUM_MAX] = {0};

    if (ipc_send_timestamp[dev_id].tv_sec != 0) {
        do_gettimeofday(&temp_timestamp[dev_id]);

        if (temp_timestamp[dev_id].tv_sec - ipc_send_timestamp[dev_id].tv_sec < IPC_SEND_THRESHOLD) {
            return false;
        }
    } else {
        dms_info("It is the first time to get temperature from IMU.\n");
    }

    do_gettimeofday(&ipc_send_timestamp[dev_id]);
    return true;
}

int g_temp_data[DEVICE_NUM_MAX][TEMP_FROM_IMU_NUM] = {0};
#define TEMP_INVALID_VALUE (-127)
static int temp_imu_check_ack(u32 dev_id, const lpm_ipc_ctrl_t *lpm_ipc, struct dms_ipc_lpm *ack_msg)
{
    int ret = lp_ack_check_common(dev_id, lpm_ipc, ack_msg);
    if (ret) {
        return ret;
    }

    g_temp_data[dev_id][0] = *((s8 *)&ack_msg->cmd_data0);  // 0 : soc's max temp
    g_temp_data[dev_id][1] = *((s8 *)&ack_msg->cmd_data1);  // 1 : HBM's max temp
    g_temp_data[dev_id][2] = *((s8 *)&ack_msg->cmd_data2);  // 2 : N DIE's temp

    return 0;
}

struct lpm_vrd {
    char vrd_temp_0;
    char vrd_temp_1;
    char vrd_temp_2;
    char res;
};

#define VRD_SHIFT 8
#define VRD_MASK 0xff
static int temp_imu_set_ret(u32 dev_id, const lpm_ipc_ctrl_t *lpm_ipc, struct dms_ipc_lpm *ack_msg,
    u8 *buf, u32 *len)
{
    int ret;
    struct lpm_vrd *vrd_temp = NULL;

    switch (lpm_ipc->core_id) {
        case LPM_SOC_ID:
            *(s32 *)buf = g_temp_data[dev_id][0]; /* 0 : soc's max temp */
            ret = (g_temp_data[dev_id][0] != TEMP_INVALID_VALUE) ? 0 : -EINVAL;
            break;
        case LPM_HBM_ID:
            *(s32 *)buf = g_temp_data[dev_id][1]; /* 1 : HBM's max temp */
            ret = (g_temp_data[dev_id][1] != TEMP_INVALID_VALUE) ? 0 : -EINVAL;
            break;
        case LPM_N_DIE_ID:
            *(s32 *)buf = g_temp_data[dev_id][2]; /* 2 : N DIE's temp */
            ret = (g_temp_data[dev_id][2] != TEMP_INVALID_VALUE) ? 0 : -EINVAL;
            break;
        default:
            ret = -EINVAL;
            break;
    }

    vrd_temp = (struct lpm_vrd *)&buf[sizeof(s32)];
    vrd_temp->vrd_temp_0 = (char)ack_msg->cmd_data3;
    vrd_temp->vrd_temp_1 = (char)(ack_msg->data1 & VRD_MASK);
    vrd_temp->vrd_temp_2 = (char)((ack_msg->data1 >> VRD_SHIFT) & VRD_MASK);

    return ret;
}


static int power_imu_ack_check(u32 dev_id, const lpm_ipc_ctrl_t *lpm_ipc, struct dms_ipc_lpm *ack_msg)
{
    int ret = lp_ack_check_common(dev_id, lpm_ipc, ack_msg);
    if (ret) {
        dms_err("Lp ack check failed. (dev_id=%u; ipc=%pK; ack=%pK)", dev_id, lpm_ipc, ack_msg);
        return ret;
    }

    if ((ack_msg->source_id != OBJ_IMU_DMP) || (ack_msg->target_id != OBJ_AP_DMP) ||
        (*(u32 *)&ack_msg->cmd_para0 != 0)) {
        dms_err("Get invalid ipc massage. (source_id=%u; target_id=%u; cmd_para=%u)\n",
            ack_msg->source_id, ack_msg->target_id, *(u32 *)&ack_msg->cmd_para0);
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
    /* power */
    {LPM_POWER,     LPM_SOC_ID,    MSG_HEADS_WORD, MSG_TOTAL_WORD,
        MSG_TOTAL_WORD, sizeof(u32), NO_REPEATS, IPC_OBJ_LP, DMS_IMU_TX_MBX25,
        NULL, power_imu_ack_check, lp_ack_set_value_common,
        {IPC_IMU_POWER_1,     IPC_IMU_POWER, OBJ_IMU_DMP, OBJ_AP_DMP}},

    /* voltage */
    {LPM_VOLTAGE,   LPM_SOC_ID,    MSG_HEADS_WORD, MSG_TOTAL_WORD,
        MSG_TOTAL_WORD, sizeof(u32), NO_REPEATS, IPC_OBJ_LP, DMS_IMU_TX_MBX25,
        NULL, lp_ack_check_common, lp_ack_set_value_common,
        {IPC_IMU_VOLTAGE_1,     IPC_IMU_VOLTAGE, OBJ_IMU_DMP, OBJ_AP_DMP}},

    /* temperature */
    {LPM_TEMPERATURE,   LPM_SOC_ID,     MSG_HEADS_WORD, MSG_TOTAL_WORD,
        0, 0, NO_REPEATS, IPC_OBJ_LP, DMS_IMU_TX_MBX25,
        temp_imu_is_need_send, temp_imu_check_ack, temp_imu_set_ret,
        {IPC_IMU_TEMP_1,         IPC_IMU_TEMP, OBJ_IMU_DMP, OBJ_AP_DMP}},
    {LPM_TEMPERATURE,   LPM_N_DIE_ID,   MSG_HEADS_WORD, MSG_TOTAL_WORD,
        0, 0, NO_REPEATS, IPC_OBJ_LP, DMS_IMU_TX_MBX25,
        temp_imu_is_need_send, temp_imu_check_ack, temp_imu_set_ret,
        {IPC_IMU_TEMP_1,         IPC_IMU_TEMP, OBJ_IMU_DMP, OBJ_AP_DMP}},
    {LPM_TEMPERATURE,   LPM_HBM_ID,     MSG_HEADS_WORD, MSG_TOTAL_WORD,
        0, 0, NO_REPEATS, IPC_OBJ_LP, DMS_IMU_TX_MBX25,
        temp_imu_is_need_send, temp_imu_check_ack, temp_imu_set_ret,
        {IPC_IMU_TEMP_1,         IPC_IMU_TEMP, OBJ_IMU_DMP, OBJ_AP_DMP}},

    /* freq */
    {LPM_FREQUENCY,     LPM_AICORE0_ID, MSG_HEADS_WORD, MSG_TOTAL_WORD,
        MSG_TOTAL_WORD, sizeof(u32), NO_REPEATS, IPC_OBJ_LP, DMS_IMU_TX_MBX25,
        NULL, lp_ack_check_common, lp_ack_set_value_common,
        {CMD_INQUIRY,         IPC_IMU_AICORE, OBJ_IMU_DMP, OBJ_AP_DMP}},
    {LPM_FREQUENCY,     LPM_DDR_ID,     MSG_HEADS_WORD, MSG_TOTAL_WORD,
        MSG_TOTAL_WORD, sizeof(u32), NO_REPEATS, IPC_OBJ_LP, DMS_IMU_TX_MBX25,
        NULL, lp_ack_check_common, lp_ack_set_value_common,
        {IPC_IMU_FREQ_1,         IPC_IMU_DDR, OBJ_IMU_DMP, OBJ_AP_DMP}},
    {LPM_FREQUENCY,     LPM_HBM_ID,     MSG_HEADS_WORD, MSG_TOTAL_WORD,
        MSG_TOTAL_WORD, sizeof(u32), NO_REPEATS, IPC_OBJ_LP, DMS_IMU_TX_MBX25,
        NULL, lp_ack_check_common, lp_ack_set_value_common,
        {IPC_IMU_FREQ_1,         IPC_IMU_HBM, OBJ_IMU_DMP, OBJ_AP_DMP}},

    /* ecc statistics */
    {LPM_ECC_STATISTICS, LPM_DDR_ID,    MSG_HEADS_WORD, MSG_TOTAL_WORD,
        MSG_TOTAL_WORD, sizeof(u64), NO_REPEATS, IPC_OBJ_LP, DMS_IMU_TX_MBX25,
        NULL, lp_ack_check_common, lp_ack_set_value_common,
        {IPC_IMU_ECC_1,     IPC_IMU_DDR, OBJ_IMU_DMP, OBJ_AP_DMP}},
    {LPM_ECC_STATISTICS, LPM_HBM_ID,    MSG_HEADS_WORD, MSG_TOTAL_WORD,
        MSG_TOTAL_WORD, sizeof(u64), NO_REPEATS, IPC_OBJ_LP, DMS_IMU_TX_MBX25,
        NULL, lp_ack_check_common, lp_ack_set_value_common,
        {IPC_IMU_ECC_1,     IPC_IMU_HBM, OBJ_IMU_DMP, OBJ_AP_DMP}},
    {0},
};

const lpm_ipc_ctrl_t *get_lpm_ipc_ctrl_from_imu(u32 info_type, u32 core_id)
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
