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
#include "devdrv_user_common.h"

// cmdType define
enum {
    SUB_CMD_NO_TYPE = 0,
    SUB_CMD_SMOKE = 1,
    SUB_CMD_TEMPERATURE = 2,
    SUB_CMD_SLEEP = 3,
    SUB_CMD_RESUME = 4,
    SUB_CMD_FREQUENCE = 5,
    SUB_CMD_HEALTH_STATE = 6,
    SUB_CMD_HEALTH_EVENT = 7,
    SUB_CMD_HEART = 8,
    SUB_CMD_VERSION = 9,
    SUB_CMD_LOG = 10,
    SUB_CMD_EXCEPTION = 11,
    SUB_CMD_AICORE_LIMIT = 80,
    SUB_CMD_QUERY_SOC_TEMP,
    SUB_CMD_QUERY_VOLTAGE,
    SUB_CMD_QUERY_AICFREQ,
    SUB_CMD_QUERY_AIVFREQ,
    SUB_CMD_QUERY_CTRLCPUFREQ,
    SUB_CMD_QUERY_LP_HEALTH,
    SUB_CMD_QUERY_DDR_FREQUENCY,
    SUB_CMD_QUERY_DDR_ECC,
    SUB_CMD_QUERY_SOC_PWC,
    SUB_CMD_QUERY_DDR_TEMP,
    SUB_CMD_QUERY_VRD_TEMP,
    SUB_CMD_DDR_THRESHOLD,
    SUB_CMD_SOC_THRESHOLD = 93,
    SUB_CMD_AICORE_VOL_CURRENT,
    SUB_CMD_HYBIRD_VOL_CURRENT,
    SUB_CMD_TAISHAN_VOL_CURRENT,
    SUB_CMD_DDR_VOL_CURRENT,
    SUB_CMD_NOTIFY_LP_SUSPEND,
    SUB_CMD_NOTIFY_LP_RESUME = 99,
    SUB_CMD_QUERY_DDR_BASE_INFO = 100,
    SUB_CMD_QUERY_DDR_MANUFACTURES_INFO = 101,
    SUB_CMD_QUERY_LP_ACG = 102,
    SUB_CMD_NOTIFY_LP_BIST,
    SUB_CMD_INIT_DDR_STATISTIC = 104,
    SUB_CMD_QUERY_DDR_STATISTIC = 105,
};

enum {
    CMD_INQUIRY = 1,
    CMD_SETTING = 2,
    CMD_NOTIFY  = 3,
    CMD_TEST    = 4,
    CMD_ON          = 5,
    CMD_IMU_SMOKE   = 6,
    CMD_HEART       = 7,
    CMD_DFT_TEST    = 8,
    MAX_CMD_MODE    = 9,
};
#define MDC_IPC_REPEATS 1
#define LPR52_SOURECE_ID 0
#define LPR52_TARGET_ID 0x4

#define LP_ERRCODE_QUERY_FAIL       1
#define LP_ERRCODE_QUERY_TIMEOUT    2
#define LP_ERRCODE_QUERY_PARA_ERR   3
#define LP_ERRCODE_QUERY_NONSUPPORT 4
int lp_get_errcode(u32 err_code)
{
    int ret;
    switch (err_code) {
        case LP_ERRCODE_QUERY_FAIL:
            ret = -ENODATA;
            break;
        case LP_ERRCODE_QUERY_TIMEOUT:
            ret = -ETIMEDOUT;
            break;
        case LP_ERRCODE_QUERY_PARA_ERR:
            ret = -EINVAL;
            break;
        case LP_ERRCODE_QUERY_NONSUPPORT:
            ret = -EOPNOTSUPP;
            break;
        default:
            ret = 0;
            break;
    }

    return ret;
}

static int voltage_lp_ack_check(u32 dev_id, const lpm_ipc_ctrl_t *lpm_ipc, struct dms_ipc_lpm *ack_msg)
{
    u32 lp_error_code;

    int ret = lp_ack_check_common(dev_id, lpm_ipc, ack_msg);
    if (ret) {
        dms_err("Lp ack check failed. (type0=%u; type1=%u)\n", ack_msg->cmd_type0, ack_msg->cmd_type1);
        return ret;
    }

    lp_error_code = *(u32 *)(&(ack_msg->cmd_para0));
    if (lp_error_code != LP_MSG_NO_ERR) {
        dms_err("Get low power module message failed. (type0=%u; type1=%u; lp_err_code=0x%x)\n",
            ack_msg->cmd_type0, ack_msg->cmd_type1, lp_error_code);
        return lp_get_errcode(lp_error_code);
    }

    return 0;
}

static int lp_ack_check_privte(u32 dev_id, const lpm_ipc_ctrl_t *lpm_ipc, struct dms_ipc_lpm *ack_msg)
{
    u32 lp_error_code;

    int ret = lp_ack_check_common(dev_id, lpm_ipc, ack_msg);
    if (ret) {
        dms_err("Lp ack check failed. (type0=%u; type1=%u)\n", ack_msg->cmd_type0, ack_msg->cmd_type1);
        return ret;
    }

    lp_error_code = *(u32 *)(&(ack_msg->cmd_para0));
    if (lp_error_code != LP_MSG_NO_ERR) {
        dms_err("Get low power module message failed. (type0=%u; type1=%u; lp_err_code=0x%x)\n",
            ack_msg->cmd_type0, ack_msg->cmd_type1, lp_error_code);
        return -EINVAL;
    }

    return 0;
}

#define VOLTAGE_INTER_UNIT 5
static int voltage_lp_set_ret(u32 dev_id, const lpm_ipc_ctrl_t *lpm_ipc, struct dms_ipc_lpm *ack_msg,
    u8 *buf, u32 *len)
{
    *(u32 *)buf = (*(u32 *)&ack_msg->cmd_data0 + VOLTAGE_INTER_UNIT);
    *len = sizeof(u32);

    return 0;
}

#define PERCENT_UNIT 100
#define PERCENT_INTER_MEDIATE_DATE 50
static int temp_lp_set_ret(u32 dev_id, const lpm_ipc_ctrl_t *lpm_ipc, struct dms_ipc_lpm *ack_msg,
    u8 *buf, u32 *len)
{
    if ((*(int *)&ack_msg->cmd_data0) < 0) {
        *(u32 *)buf = (*(int *)&ack_msg->cmd_data0 - PERCENT_INTER_MEDIATE_DATE) / PERCENT_UNIT;
    } else {
        *(u32 *)buf = (*(int *)&ack_msg->cmd_data0 + PERCENT_INTER_MEDIATE_DATE) / PERCENT_UNIT;
    }
    *len = sizeof(u32);

    return 0;
}

static int lp_status_ack_set_value(u32 dev_id, const lpm_ipc_ctrl_t *lpm_ipc, struct dms_ipc_lpm *ack_msg,
    u8 *buf, u32 *len)
{
    struct devdrv_lp_status *lp_status = (struct devdrv_lp_status *)buf;

    lp_status->status = *(u32 *)&ack_msg->cmd_data0;
    lp_status->status_info = (unsigned long long)ack_msg->data1;
    *len = sizeof(struct devdrv_lp_status);

    return 0;
}

static int lp_vol_cur_ack_set_value(u32 dev_id, const lpm_ipc_ctrl_t *lpm_ipc, struct dms_ipc_lpm *ack_msg,
    u8 *buf, u32 *len)
{
    struct devdrv_dev_info *vol_cur = (struct devdrv_dev_info *)buf;

    vol_cur->data1 = *(u32 *)&ack_msg->cmd_data0;  // voltage
    vol_cur->data2 = *(int *)&ack_msg->cmd_para0;  // lp_errcode
    vol_cur->data3 = *(u32 *)&ack_msg->data1;      // current
    *len = sizeof(struct devdrv_dev_info);

    return 0;
}

static int lp_acg_ack_set_value(u32 dev_id, const lpm_ipc_ctrl_t *lpm_ipc, struct dms_ipc_lpm *ack_msg,
    u8 *buf, u32 *len)
{
    *(u32 *)buf = *(int *)&ack_msg->cmd_para0;
    *len = sizeof(u32);

    return 0;
}

static int lp_ddr_temp_ack_set_value(u32 dev_id, const lpm_ipc_ctrl_t *lpm_ipc, struct dms_ipc_lpm *ack_msg,
    u8 *buf, u32 *len)
{
    return memcpy_s(buf, DDR_TEMP_TEMPERATURE_SIZE, &ack_msg->cmd_data0, DDR_TEMP_TEMPERATURE_SIZE);
}

static int lp_temp_thold_ack_set_value(u32 dev_id, const lpm_ipc_ctrl_t *lpm_ipc, struct dms_ipc_lpm *ack_msg,
    u8 *buf, u32 *len)
{
    struct devdrv_dev_info *temp_thold = (struct devdrv_dev_info *)buf;
    temp_thold->data1 = *(u32 *)(&(ack_msg->cmd_data0));
    temp_thold->data3 = *(u32 *)(&(ack_msg->data1));

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
    {LPM_POWER,     LPM_SOC_ID,     MSG_HEADS_WORD, MSG_TOTAL_WORD,
        MSG_TOTAL_WORD, sizeof(u64), MDC_IPC_REPEATS, IPC_OBJ_LP, DMS_LP_Q_TX_RPID4_ACPU0,
        NULL, lp_ack_check_privte, lp_ack_set_value_common,
        {SUB_CMD_QUERY_SOC_PWC, CMD_INQUIRY,  OBJ_IMU, 0}},

    /* voltage */
    {LPM_VOLTAGE,   LPM_SOC_ID,     MSG_HEADS_WORD, MSG_TOTAL_WORD,
        0, 0, NO_REPEATS, IPC_OBJ_LP,       DMS_LP_Q_TX_RPID4_ACPU0,
        NULL, voltage_lp_ack_check, voltage_lp_set_ret,
        {SUB_CMD_QUERY_VOLTAGE, CMD_INQUIRY,  OBJ_IMU, 0}},

    /* temperature */
    {LPM_TEMPERATURE,   LPM_SOC_ID,     MSG_HEADS_WORD, MSG_TOTAL_WORD,
        0, 0, MDC_IPC_REPEATS, IPC_OBJ_LP,  DMS_LP_Q_TX_RPID4_ACPU0,
        NULL, lp_ack_check_privte, temp_lp_set_ret,
        {SUB_CMD_QUERY_SOC_TEMP, CMD_INQUIRY,  OBJ_IMU, 0}},

    /* freq */
    {LPM_FREQUENCY,     LPM_CLUSTER_ID, MSG_HEADS_WORD, MSG_TOTAL_WORD,
        MSG_TOTAL_WORD, sizeof(u32), NO_REPEATS, IPC_OBJ_LP, DMS_LP_Q_TX_RPID4_ACPU0,
        NULL, lp_ack_check_privte, lp_ack_set_value_common,
        {SUB_CMD_QUERY_CTRLCPUFREQ, CMD_INQUIRY,  OBJ_IMU, 0}},
    {LPM_FREQUENCY,     LPM_DDR_ID,     MSG_HEADS_WORD, MSG_TOTAL_WORD,
        MSG_TOTAL_WORD, sizeof(u32), NO_REPEATS, IPC_OBJ_LP, DMS_LP_Q_TX_RPID4_ACPU0,
        NULL, lp_ack_check_privte, lp_ack_set_value_common,
        {SUB_CMD_QUERY_DDR_FREQUENCY, CMD_INQUIRY,  OBJ_IMU, 0}},
    {LPM_FREQUENCY,     LPM_AICORE0_ID,  MSG_HEADS_WORD, MSG_TOTAL_WORD,
        MSG_TOTAL_WORD, sizeof(u32), NO_REPEATS, IPC_OBJ_LP, DMS_LP_Q_TX_RPID4_ACPU0,
        NULL, lp_ack_check_privte, lp_ack_set_value_common,
        {SUB_CMD_QUERY_AICFREQ, CMD_INQUIRY,  OBJ_IMU, 0}},
    {LPM_FREQUENCY,     LPM_AICORE1_ID,  MSG_HEADS_WORD, MSG_TOTAL_WORD,
        12, sizeof(u32), NO_REPEATS,             IPC_OBJ_LP, DMS_LP_Q_TX_RPID4_ACPU0,
        NULL, lp_ack_check_privte, lp_ack_set_value_common,
        {SUB_CMD_QUERY_AICFREQ, CMD_INQUIRY,  OBJ_IMU, 0}},
    {LPM_FREQUENCY,     LPM_VECTOR_ID,   MSG_HEADS_WORD, MSG_TOTAL_WORD,
        MSG_TOTAL_WORD, sizeof(u32), NO_REPEATS, IPC_OBJ_LP, DMS_LP_Q_TX_RPID4_ACPU0,
        NULL, lp_ack_check_privte, lp_ack_set_value_common,
        {SUB_CMD_QUERY_AIVFREQ, CMD_INQUIRY,  OBJ_IMU, 0}},

    /* ecc statistics */
    {LPM_ECC_STATISTICS, LPM_DDR_ID,    MSG_HEADS_WORD, MSG_TOTAL_WORD,
        MSG_TOTAL_WORD, sizeof(u64), NO_REPEATS, IPC_OBJ_LP, DMS_LP_Q_TX_RPID4_ACPU0,
        NULL, lp_ack_check_privte, lp_ack_set_value_common,
        {SUB_CMD_QUERY_DDR_ECC, CMD_INQUIRY, OBJ_IMU, 0}},

    /* status */
    {LPM_STATUS,  LPM_SOC_ID,    MSG_HEADS_WORD, MSG_TOTAL_WORD,
        0, 0, NO_REPEATS, IPC_OBJ_LP, DMS_LP_Q_TX_RPID4_ACPU0,
        NULL, NULL, lp_status_ack_set_value,
        {SUB_CMD_QUERY_LP_HEALTH, CMD_INQUIRY,  OBJ_IMU, 0}},

    /* voltage current */
    {LPM_AIC_VOL_CUR,  LPM_SOC_ID,    MSG_HEADS_WORD, MSG_TOTAL_WORD,
        0, 0, NO_REPEATS, IPC_OBJ_LP, DMS_LP_Q_TX_RPID4_ACPU0,
        NULL, NULL, lp_vol_cur_ack_set_value,
        {SUB_CMD_AICORE_VOL_CURRENT, CMD_INQUIRY,  OBJ_IMU, 0}},

    {LPM_HYBIRD_VOL_CUR,  LPM_SOC_ID,    MSG_HEADS_WORD, MSG_TOTAL_WORD,
        0, 0, NO_REPEATS, IPC_OBJ_LP, DMS_LP_Q_TX_RPID4_ACPU0,
        NULL, NULL, lp_vol_cur_ack_set_value,
        {SUB_CMD_HYBIRD_VOL_CURRENT, CMD_INQUIRY,  OBJ_IMU, 0}},

    {LPM_TAISHAN_VOL_CUR,  LPM_SOC_ID,    MSG_HEADS_WORD, MSG_TOTAL_WORD,
        0, 0, NO_REPEATS, IPC_OBJ_LP, DMS_LP_Q_TX_RPID4_ACPU0,
        NULL, NULL, lp_vol_cur_ack_set_value,
        {SUB_CMD_TAISHAN_VOL_CURRENT, CMD_INQUIRY,  OBJ_IMU, 0}},

    {LPM_DDR_VOL_CUR,  LPM_SOC_ID,    MSG_HEADS_WORD, MSG_TOTAL_WORD,
        0, 0, NO_REPEATS, IPC_OBJ_LP, DMS_LP_Q_TX_RPID4_ACPU0,
        NULL, NULL, lp_vol_cur_ack_set_value,
        {SUB_CMD_DDR_VOL_CURRENT, CMD_INQUIRY,  OBJ_IMU, 0}},

    {LPM_ACG,  LPM_SOC_ID,    MSG_HEADS_WORD, MSG_TOTAL_WORD,
        0, 0, NO_REPEATS, IPC_OBJ_LP, DMS_LP_Q_TX_RPID4_ACPU0,
        NULL, NULL, lp_acg_ack_set_value,
        {SUB_CMD_QUERY_LP_ACG, CMD_INQUIRY,  OBJ_IMU, 0}},

    /* Temperature Threshold */
    {LPM_TEMP_DDR,  LPM_SOC_ID,    MSG_HEADS_WORD, MSG_TOTAL_WORD,
        0, 0, NO_REPEATS, IPC_OBJ_LP, DMS_LP_Q_TX_RPID4_ACPU0,
        NULL, NULL, lp_ddr_temp_ack_set_value,
        {SUB_CMD_QUERY_DDR_TEMP, CMD_INQUIRY,  OBJ_IMU, 0}},

    {LPM_DDR_THOLD,  LPM_SOC_ID,    MSG_HEADS_WORD, MSG_TOTAL_WORD,
        0, 0, NO_REPEATS, IPC_OBJ_LP, DMS_LP_Q_TX_RPID4_ACPU0,
        NULL, NULL, lp_temp_thold_ack_set_value,
        {SUB_CMD_DDR_THRESHOLD, CMD_INQUIRY,  LPR52_TARGET_ID, LPR52_SOURECE_ID}},

    {LPM_SOC_THOLD,  LPM_SOC_ID,    MSG_HEADS_WORD, MSG_TOTAL_WORD,
        0, 0, NO_REPEATS, IPC_OBJ_LP, DMS_LP_Q_TX_RPID4_ACPU0,
        NULL, NULL, lp_temp_thold_ack_set_value,
        {SUB_CMD_SOC_THRESHOLD, CMD_INQUIRY,  LPR52_TARGET_ID, LPR52_SOURECE_ID}},

    {LPM_SOC_MIN_THOLD,  LPM_SOC_ID,    MSG_HEADS_WORD, MSG_TOTAL_WORD,
        0, 0, NO_REPEATS, IPC_OBJ_LP, DMS_LP_Q_TX_RPID4_ACPU0,
        NULL, NULL, lp_temp_thold_ack_set_value,
        {SUB_CMD_SOC_THRESHOLD, CMD_INQUIRY,  LPR52_TARGET_ID, LPR52_SOURECE_ID}},
    {0},
};

const lpm_ipc_ctrl_t *get_lpm_ipc_ctrl_from_lp(u32 info_type, u32 core_id)
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


#ifdef CFG_REATURE_GET_INFO_FORM_IPC
/* set ipc ctrl format:
 * {INFO_TYPE,   CORE_ID,    SEND_LEN,   RECV_LEN,
 *      ACK_POS,    ACK_LEN,    REPEATS_TIMES,  IPC_CHAN_TYPE, IPC_CHANNEL
 *      send_ctrl_func,     ack_check_func,  ret_set_func,
 *      {IPC_SEND_D0, IPC_SEND_D1, IPC_SEND_D2, IPC_SEND_D3...}}
 * */
static lpm_ipc_ctrl_t g_set_ipc_format_list[] = {
    /* Temperature Threshold */
    {LPM_SET_DDR_TEMP_THOLD, LPM_SOC_ID, SET_MSG_HEADS_WORD, MSG_TOTAL_WORD,
        0, 0, NO_REPEATS, IPC_OBJ_LP, DMS_LP_Q_TX_RPID4_ACPU0,
        NULL, lp_ack_check_privte, NULL,
        {SUB_CMD_DDR_THRESHOLD, CMD_SETTING,  LPR52_TARGET_ID, LPR52_SOURECE_ID}},
    {LPM_SET_SOC_TEMP_THOLD, LPM_SOC_ID, SET_MSG_HEADS_WORD, MSG_TOTAL_WORD,
        0, 0, NO_REPEATS, IPC_OBJ_LP, DMS_LP_Q_TX_RPID4_ACPU0,
        NULL, lp_ack_check_privte, NULL,
        {SUB_CMD_SOC_THRESHOLD, CMD_SETTING,  LPR52_TARGET_ID, LPR52_SOURECE_ID}},
    {0},
};

lpm_ipc_ctrl_t *set_lpm_ipc_ctrl_from_lp(u32 info_type, u32 core_id)
{
    int i;
    int len;

    /* Find ipc msg table */
    len = sizeof(g_set_ipc_format_list) / sizeof(lpm_ipc_ctrl_t);
    for (i = 0; i < len; i++) {
        if ((core_id == g_set_ipc_format_list[i].core_id) &&
            (info_type == g_set_ipc_format_list[i].info_type)) {
            break;
        }
    }
    if (i == len) {
        return NULL;
    }

    return &g_set_ipc_format_list[i];
}
#endif