/*
* Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
* Create: 20022-08-03
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

/* Solve the ut compilation problem in the dms_ipc_commmon.h header file for struct dms_ipc_lpm */
#ifndef CONFIG_IPC_COMM_FROM_IMU_V2
#define CONFIG_IPC_COMM_FROM_IMU_V2
#endif
#include "dms_ipc_common.h"
#include "dms_ipc_interface.h"
#include "devdrv_ipc_msg.h"

STATIC int imu_ack_set_value_common(u32 dev_id, const lpm_ipc_ctrl_t *lpm_ipc, struct dms_ipc_lpm *ack_msg,
    u8 *buf, u32 *len)
{
    int i;

    if (lpm_ipc == NULL || ack_msg == NULL) {
        dms_err("Ipc ctrl or ack msg is null. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }
    if (buf == NULL || len == NULL) {
        dms_err("Buf or len is null. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    if ((ack_msg->len > ICM_MSG_DATA_LENGTH) || (*len != ack_msg->len - sizeof(u32))) {
        dms_err("Lpm ipc format invalid. (dev_id=%u; ack_len=%u; len=%u)\n", dev_id, ack_msg->len, *len);
        return -EINVAL;
    }

    /* data[0-3] are used as ack_ret */
    for (i = 0; i < ack_msg->len - sizeof(u32); i++) {
        buf[i] = ack_msg->data[sizeof(u32) + i];
    }
    *len = ack_msg->len - sizeof(u32);

    return 0;
}

STATIC int imu_ack_check_common(u32 dev_id, const lpm_ipc_ctrl_t *lpm_ipc, struct dms_ipc_lpm *ack_msg)
{
    u32 ack_ret;
    unsigned short crc16_value;

    /* crc check */
    crc16_value = sils_crc16((u8 *)ack_msg, ack_msg->len + ICM_IPC_MSG_HEAD_LEN);
    if (crc16_value != ack_msg->crc16) {
        dms_err("crc16 check failed. (cmd_src=%u, cmd_dest=%u).\n", ack_msg->cmd_src, ack_msg->cmd_dest);
        return -EINVAL;
    }

    ack_ret = *(u32 *)ack_msg->data;
    if ((ack_msg->cmd_src != lpm_ipc->ipc_msg.cmd_dest) || (ack_msg->cmd_dest != lpm_ipc->ipc_msg.cmd_src) ||
        (ack_msg->sub_cmd != lpm_ipc->ipc_msg.sub_cmd) || (ack_msg->cmd != lpm_ipc->ipc_msg.cmd) ||
        (ack_ret != 0) || (ack_msg->len > ICM_MSG_DATA_LENGTH)) {
        dms_err("Get invalid ipc message.\n"
                "ack.len = %u, ack_ret = %u.\n"
                "ack.cmd_src = %u, ack.cmd_dest = %u.\n"
                "ack.sub_cmd = %u, ipc.sub_cmd = %u.\n"
                "ack.cmd = %u, ipc.cmd = %u.\n",
                ack_msg->len, ack_ret, ack_msg->cmd_src, ack_msg->cmd_dest,
                ack_msg->sub_cmd, lpm_ipc->ipc_msg.sub_cmd, ack_msg->cmd, lpm_ipc->ipc_msg.cmd);
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
    /* ecc statistics */
    {LPM_ECC_STATISTICS, LPM_HBM_ID, MSG_HEADS_WORD, MSG_TOTAL_WORD,
        MSG_TOTAL_WORD, NA, NO_REPEATS, IPC_OBJ_LP, HISI_RPROC_TX_IMU_MBX29,
        NULL, imu_ack_check_common, imu_ack_set_value_common,
        {ICM_SUB_CMD_HBM_ECC, ICM_MAIN_CMD_MEMORY_REQ, OBJ_CMD_LP, OBJ_CMD_CCPU}},
    {LPM_FREQUENCY, LPM_HBM_ID, MSG_HEADS_WORD, MSG_TOTAL_WORD,
        MSG_TOTAL_WORD, NA, NO_REPEATS, IPC_OBJ_LP, HISI_RPROC_TX_IMU_MBX29,
        NULL, imu_ack_check_common, imu_ack_set_value_common,
        {ICM_SUB_CMD_HBM_FREQ, ICM_MAIN_CMD_MEMORY_REQ, OBJ_CMD_LP, OBJ_CMD_CCPU}},
};

const lpm_ipc_ctrl_t *get_lpm_ipc_ctrl_from_imu_v2(u32 info_type, u32 core_id)
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
