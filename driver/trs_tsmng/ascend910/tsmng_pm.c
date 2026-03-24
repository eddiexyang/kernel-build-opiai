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
 * Create: 2022-08-19
 */
#ifndef TSDRV_UT
#include <linux/securec.h>
#include <linux/uaccess.h>


#include "drv_ipc.h"
#include "devdrv_platform_resource.h"

/* ===must include */
#include "tsmng_interface.h"
#include "tsmng_log.h"
#include "tsmng_common.h"
/* must include=== */

#include "tsmng_pm.h"
#ifdef CFG_FEATURE_LP_ENABLE
#include "tsmng_group.h"
#endif

#define AICORE_SHOW_INFO_COUNT 100
#define DEVDRV_IPC_MOST_FRAME 4
#define TSMNG_GET_AI_POWER_MSG_LEN  2

#define TSMNG_TS_MASK_ALARM_OP_QUERY 0
#define TSMNG_TS_MASK_ALARM_OP_SET 1

#define TSMNG_TS_MASK_ALARM_OPS  0
#define TSMNG_TS_MASK_ALARM_TYPE  1
#define TSMNG_TS_MASK_ALARM_STATUS  2
#define TSMNG_INFORM_TS_MASK_ALARM_MSG_LEN  3

struct devdrv_mailbox_info ipc_info[MAX_CHIP_NUM];
u8 msg_flag[MAX_CHIP_NUM] = {0};
u8 ts_cp_msg[MAX_CHIP_NUM][DEVDRV_COMPUTE_POWER_LENGTH] = {0};
u8 ipc_msg_bitmap[DEVDRV_IPC_MOST_FRAME] = { 1, 3, 7, 15 };

void tsmng_ipc_manager_init(u32 dev_id)
{
#ifdef CFG_FEATURE_LP_ENABLE
    devdrv_ipc_ts_msg_channel_init(dev_id);
#endif

    atomic_set(&ipc_info[dev_id].wait_flag, 0);
    init_waitqueue_head(&ipc_info[dev_id].mailbox_wait_queque);
    ipc_info[dev_id].wait_time = GET_DATA_WAIT_TIMEOUT;
    ipc_info[dev_id].wait_timeout_count = 0;
}

void tsmng_ipc_manager_exit(u32 dev_id)
{
#ifdef CFG_FEATURE_LP_ENABLE
    devdrv_ipc_ts_msg_channel_destroy(dev_id);
#endif
}

static int tsmng_inform_ts_mask_alarm_msg(u8 *data_send, size_t len, u8 *data_rev, u32 dev_id, u32 tsid)
{
    rproc_msg_t msg[IPCDRV_RPROC_MSG_LENGTH] = {0};
    rproc_msg_t ack_buffer[IPCDRV_RPROC_MSG_LENGTH] = {0};
    struct ipcdrv_message *ipc_msg = NULL;
    int ret = 0;

    if (len != TSMNG_INFORM_TS_MASK_ALARM_MSG_LEN) {
        tsmng_drv_err("Invalid Para. (len=%ld)\n", len);
        return -EINVAL;
    }

    ipc_msg = (struct ipcdrv_message *)msg;
    ipc_msg->ipc_msg_header.msg_type = MSGTYPE_DRIVER_SEND;
    ipc_msg->ipc_msg_header.cmd_type = IPCDRV_TS_MASK_ALARM;
    ipc_msg->ipc_msg_header.sync_type = IPCDRV_MSG_SYNC;
    ipc_msg->ipc_msg_header.reserved = 0;
    ipc_msg->ipc_msg_header.msg_length = TSMNG_INFORM_TS_MASK_ALARM_MSG_LEN;
    ipc_msg->ipc_msg_header.msg_index = 0;
    /* playload[0]: query or set [1]: alarm type [2]: set 0-close 1-open */
    ipc_msg->ipcdrv_payload[TSMNG_TS_MASK_ALARM_OPS] = data_send[TSMNG_TS_MASK_ALARM_OPS];
    ipc_msg->ipcdrv_payload[TSMNG_TS_MASK_ALARM_TYPE] = data_send[TSMNG_TS_MASK_ALARM_TYPE];
    ipc_msg->ipcdrv_payload[TSMNG_TS_MASK_ALARM_STATUS] = data_send[TSMNG_TS_MASK_ALARM_STATUS];
#if defined(CFG_SOC_PLATFORM_CLOUD)
    ret = icm_msg_send_sync(ICM_FD_BUILD(dev_id, HISI_RPROC_TX_TS_MBX17), msg, IPCDRV_RPROC_MSG_LENGTH,
        ack_buffer, IPCDRV_RPROC_MSG_LENGTH);
#endif
    if (ret != 0) {
        tsmng_drv_err("Icm_msg_send_sync failed.(ret=%d)\n", ret);
    }
    ipc_msg = (struct ipcdrv_message *)ack_buffer;
    /* playload[0]: query 0-close 1-open; set result 0-ok other-fail */
    data_rev[0] = ipc_msg->ipcdrv_payload[0];

    return ret;
}

int tsmng_inform_ts_mask_alarm(u8 type, u8 mask_switch, u32 dev_id, u32 tsid)
{
    u8 data_send[TSMNG_INFORM_TS_MASK_ALARM_MSG_LEN] = {0};
    u8 data_recv[TSMNG_INFORM_TS_MASK_ALARM_MSG_LEN] = {0};
    int ret;

    data_send[TSMNG_TS_MASK_ALARM_OPS] = TSMNG_TS_MASK_ALARM_OP_SET;
    data_send[TSMNG_TS_MASK_ALARM_TYPE] = type;
    data_send[TSMNG_TS_MASK_ALARM_STATUS] = mask_switch;

    ret = tsmng_inform_ts_mask_alarm_msg(data_send, sizeof(data_send), data_recv, dev_id, tsid);
    if (ret != 0) {
        return ret;
    }
    return (int)data_recv[0];
}
EXPORT_SYMBOL(tsmng_inform_ts_mask_alarm);

int tsmng_query_ts_mask_alarm(u8 type, u8 *mask_switch, u32 dev_id, u32 tsid)
{
    u8 data_send[TSMNG_INFORM_TS_MASK_ALARM_MSG_LEN] = {0};
    u8 data_recv[TSMNG_INFORM_TS_MASK_ALARM_MSG_LEN] = {0};
    int ret;

    data_send[TSMNG_TS_MASK_ALARM_OPS] = TSMNG_TS_MASK_ALARM_OP_QUERY;
    data_send[TSMNG_TS_MASK_ALARM_TYPE] = type;

    ret = tsmng_inform_ts_mask_alarm_msg(data_send, sizeof(data_send), data_recv, dev_id, tsid);
    if (ret != 0) {
        return ret;
    }
    *mask_switch = data_recv[0];
    return 0;
}
EXPORT_SYMBOL(tsmng_query_ts_mask_alarm);

int tsmng_inform_ts_idle(void *data, u32 dev_id, u32 tsid)
{
    rproc_msg_t msg[IPCDRV_RPROC_MSG_LENGTH];
    struct ipcdrv_msg_payload *payload = NULL;
    struct ipcdrv_message *ipc_msg = NULL;
    int ret;

    if (tsid >= DEVDRV_MAX_TS_NUM) {
        tsmng_drv_err("Invalid tsid. (dev_id=%u; tsid=%u)\n", dev_id, tsid);
        return -EINVAL;
    }

    ipc_msg = (struct ipcdrv_message *)msg;
    ipc_msg->ipc_msg_header.msg_type = MSGTYPE_DRIVER_RECEIVE;
    ipc_msg->ipc_msg_header.cmd_type = IPCDRV_TS_INFORM_TS_IDLE;
    ipc_msg->ipc_msg_header.sync_type = IPCDRV_MSG_ASYNC;
    ipc_msg->ipc_msg_header.reserved = 0;
    ipc_msg->ipc_msg_header.msg_length = 1;
    ipc_msg->ipc_msg_header.msg_index = 0;
    payload = (struct ipcdrv_msg_payload *)ipc_msg->ipcdrv_payload;
    payload->result = 0;

    ret = 0;
#if defined(CFG_SOC_PLATFORM_CLOUD)
    ret = icm_msg_send_async(ICM_FD_BUILD(dev_id, HISI_RPROC_TX_TS_MBX17), msg, IPCDRV_RPROC_MSG_LENGTH);
#elif defined(CFG_SOC_PLATFORM_MINIV2)
#elif CFG_SOC_PLATFORM_MINI
    ret = icm_msg_send_async(ICM_FD_BUILD(dev_id, HISI_RPROC_TX_TS), msg, IPCDRV_RPROC_MSG_LENGTH);
#endif
    if (ret != 0) {
        tsmng_drv_err("Icm_msg_send_async failed.\n");
    }

    return ret;
}

/* not called on chips except 310 */
__attribute__((unused)) int devdrv_inform_ts_config_aicore(u32 dev_id, int cmd_type)
{
    rproc_msg_t msg[IPCDRV_RPROC_MSG_LENGTH];
    struct ipcdrv_msg_payload *payload = NULL;
    struct ipcdrv_message *ipc_msg = NULL;
    int ret;

    ipc_msg = (struct ipcdrv_message *)msg;
    ipc_msg->ipc_msg_header.msg_type = MSGTYPE_DRIVER_SEND;

    if ((cmd_type != IPCDRV_TS_INFORM_TS_LIMIT_AICORE) &&
        (cmd_type != IPCDRV_TS_INFORM_TS_CANCEL_LIMIT)) {
        tsmng_drv_err("Invalid cmd_type. (cmd_type=%d)\n", cmd_type);
        return -EINVAL;
    }
    ipc_msg->ipc_msg_header.cmd_type = cmd_type;

    ipc_msg->ipc_msg_header.sync_type = IPCDRV_MSG_ASYNC;
    ipc_msg->ipc_msg_header.reserved = 0;
    ipc_msg->ipc_msg_header.msg_length = 1;
    ipc_msg->ipc_msg_header.msg_index = 0;
    payload = (struct ipcdrv_msg_payload *)ipc_msg->ipcdrv_payload;
    payload->result = 0;

    tsmng_drv_info("Inform ts config aicore. (cmd_type=%d; sync_type=%d)\n",
        ipc_msg->ipc_msg_header.cmd_type, IPCDRV_MSG_ASYNC);

    ret = 0;
#ifdef CFG_SOC_PLATFORM_CLOUD
    ret = icm_msg_send_async(ICM_FD_BUILD(0, HISI_RPROC_TX_TS_MBX17), msg, IPCDRV_RPROC_MSG_LENGTH);
#elif defined(CFG_SOC_PLATFORM_MINIV2)
#elif CFG_SOC_PLATFORM_MINI
    ret = icm_msg_send_async(ICM_FD_BUILD(0, HISI_RPROC_TX_TS), msg, IPCDRV_RPROC_MSG_LENGTH);
#endif
    if (ret != 0) {
        tsmng_drv_err("Icm_msg_send_async failed.\n");
    }

    return ret;
}

static void devdrv_decode_ipc_channel(int cmd_type, rproc_id_t *channel_id)
{
#if defined(CFG_SOC_PLATFORM_CLOUD)
    *channel_id = HISI_RPROC_TX_TS_MBX17;
#elif (defined(CFG_SOC_PLATFORM_MINIV2) && !defined(CFG_SOC_PLATFORM_MINIV3))

#ifdef CFG_SOC_PLATFORM_MDC_V51
    if (cmd_type == IPCDRV_TS_AIV_COMPUTING_POWER) {
        *channel_id = HISI_RPROC_TSV_TX_RPID1;
    } else {
        *channel_id = HISI_RPROC_TSC_TX_RPID1;
    }
#else
    *channel_id = HISI_RPROC_TSC_TX_RPID1;
#endif

#else
    *channel_id = HISI_RPROC_TX_TS;
#endif
}

int tsmng_get_aicore_data(int cmd_type, u32 dev_id, u32 vfid)
{
    rproc_msg_t msg[IPCDRV_RPROC_MSG_LENGTH] = {0};
    struct ipcdrv_msg_payload *payload = NULL;
    struct ipcdrv_message *ipc_msg = NULL;
    int ret = -EINVAL;
    rproc_id_t channel_id;
    static u64 get_aicore_data_tx = 0;
    static u64 get_aicore_data_rx = 0;
    static u64 count = 0;

    if (dev_id >= MAX_CHIP_NUM) {
        tsmng_drv_err("Dev id is invalid. (dev_id=%u; max_chip_num=%d)\n", dev_id, MAX_CHIP_NUM);
        return -EINVAL;
    }

    tsmng_drv_debug("Get aicore data. (cmd_type=%d, dev_id=%u, vfid=%u)\n", cmd_type, dev_id, vfid);
    ipc_msg = (struct ipcdrv_message *)msg;
    ipc_msg->ipc_msg_header.msg_type = MSGTYPE_DRIVER_SEND;
    ipc_msg->ipc_msg_header.cmd_type = cmd_type;
    ipc_msg->ipc_msg_header.sync_type = IPCDRV_MSG_ASYNC;
    ipc_msg->ipc_msg_header.reserved = 0;
    ipc_msg->ipc_msg_header.msg_length = TSMNG_GET_AI_POWER_MSG_LEN;
    ipc_msg->ipc_msg_header.msg_index = 0;
    payload = (struct ipcdrv_msg_payload *)ipc_msg->ipcdrv_payload;
    payload->result = 0;
    payload->vfid = (u8)vfid;
    get_aicore_data_tx++;
    // avoid profiling is mode on, then compute_power_sema will still down, when profiling mode off
    // compute_power_sema is negtive, the down_timeout will time out all the time
    devdrv_decode_ipc_channel(cmd_type, &channel_id);
    atomic_set(&ipc_info[dev_id].wait_flag, 1);
    ret = icm_msg_send_async(ICM_FD_BUILD(dev_id, (unsigned int)channel_id), msg, IPCDRV_RPROC_MSG_LENGTH);
    if (ret != 0) {
        tsmng_drv_err("Icm_msg_send_async failed. ret = %d.\n", ret);
        return ret;
    }

    ret = wait_event_timeout(ipc_info[dev_id].mailbox_wait_queque, (atomic_read(&ipc_info[dev_id].wait_flag) == 0),
        msecs_to_jiffies(GET_DATA_WAIT_TIMEOUT));
    if (ret <= 0) {
        if (count++ % AICORE_SHOW_INFO_COUNT == 0) {
            tsmng_drv_warn("Wait_event_timeout. (ret=%d; tx=%llu; rx=%llu)\n",
                ret, get_aicore_data_tx, get_aicore_data_rx);
        }
        return ret;
    }

    get_aicore_data_rx++;
    return 0;
}

int tsmng_get_aic_data(u32 dev_id, u32 vfid)
{
    return tsmng_get_aicore_data(IPCDRV_TS_AI_COMPUTING_POWER, dev_id, vfid);
}
EXPORT_SYMBOL(tsmng_get_aic_data);

int tsmng_get_aiv_data(u32 dev_id, u32 vfid)
{
    return tsmng_get_aicore_data(IPCDRV_TS_AIV_COMPUTING_POWER, dev_id, vfid);
}
EXPORT_SYMBOL(tsmng_get_aiv_data);

int tsmng_get_ai_computing_power(unsigned long arg)
{
    struct computing_power_arg user_arg = {0};
    u32 dev_id;
    int ret;

    if (copy_from_user(&user_arg, (void *)((uintptr_t)arg), sizeof(struct computing_power_arg)) != 0) {
        tsmng_drv_err("Copy_from_user failed.\n");
        return -EINVAL;
    }

    dev_id = user_arg.dev_id;
    if (dev_id >= MAX_CHIP_NUM) {
        tsmng_drv_err("Dev id is invalid. (dev_id=%u; max_chip_num=%d)\n", dev_id, MAX_CHIP_NUM);
        return -EINVAL;
    }
    ret = tsmng_get_aicore_data(IPCDRV_TS_AI_COMPUTING_POWER, dev_id, 0);
    if (ret != 0) {
        tsmng_drv_err("Tsmng_get_aicore_data failed. (ret=%d)\n", ret);
        return ret;
    }

    ret = memcpy_s((void *)&user_arg.compute_power_msg, sizeof(struct tag_computing_power_msg),
                   (void *)ts_cp_msg[dev_id], AICORE_COMPUTE_SIZE);
    if (ret != 0) {
        tsmng_drv_err("Memcpy_s failed. (ret=%d)\n", ret);
        return ret;
    }
    user_arg.compute_power_msg.system_frequency = *(unsigned int *)(&ts_cp_msg[dev_id][SYSTEM_FLAG_INDEX]);

    if (copy_to_user((void *)((uintptr_t)arg), &user_arg, sizeof(struct computing_power_arg)) != 0) {
        tsmng_drv_err("Copy_to_user failed. (ret=%d)\n", ret);
        return -EINVAL;
    }

    return 0;
}
EXPORT_SYMBOL(tsmng_get_ai_computing_power);

static int tsdrv_chk_ipc_frame_crc16(struct ipcdrv_message *ipc_msg, u32 frame_len)
{
#ifdef CFG_FEATURE_IPC_CRC
    u16 crc;
    struct ipcdrv_msg_payload *payload = (struct ipcdrv_msg_payload *)ipc_msg->ipcdrv_payload;
    crc = icm_crc16((u8 *)ipc_msg, (u16)(sizeof(struct ipcdrv_msg_header) + frame_len));
    return (crc == payload->crc) ? 0 : -EBADMSG;
#else
    return 0;
#endif
}

void devdrv_get_computing_power(void *data, u32 dev_id)
{
    u32 idx;
    int ret;
    u32 tatal_fram;
    u32 last_fram;
    u32 temp;
    struct ipcdrv_message *ipc_msg = NULL;

    ipc_msg = (struct ipcdrv_message *)data;
    idx = ipc_msg->ipc_msg_header.msg_index;
    msg_flag[dev_id] |= 0x1 << idx;

    tatal_fram = (DEVDRV_COMPUTE_POWER_LENGTH + DEVDRV_IPC_PER_FRAM - 1) / DEVDRV_IPC_PER_FRAM;
    last_fram = tatal_fram - 1;

    temp = DEVDRV_IPC_PER_FRAM * idx;

    if (idx < last_fram) {
        if (tsdrv_chk_ipc_frame_crc16(ipc_msg, DEVDRV_IPC_PER_FRAM) != 0) {
            tsmng_drv_err("CRC check failed.\n");
            return;
        }
        ret = memcpy_s((void *)&ts_cp_msg[dev_id][temp], DEVDRV_IPC_PER_FRAM, (void *)ipc_msg->ipcdrv_payload,
                       DEVDRV_IPC_PER_FRAM);
    } else if (idx == last_fram) {
        if (tsdrv_chk_ipc_frame_crc16(ipc_msg, DEVDRV_COMPUTE_POWER_LAST_LEN) != 0) {
            tsmng_drv_err("CRC check failed.\n");
            return;
        }
        ret = memcpy_s((void *)&ts_cp_msg[dev_id][temp], DEVDRV_IPC_PER_FRAM, (void *)ipc_msg->ipcdrv_payload,
                       DEVDRV_COMPUTE_POWER_LAST_LEN);
    } else {
        tsmng_drv_err("Index error. (idx=%u)\n", idx);
        return;
    }

    if (ret != 0) {
        tsmng_drv_err("Copy failed. (idx=%u)\n", idx);
        return;
    }

    if (idx == last_fram) {
        msg_flag[dev_id] &= ipc_msg_bitmap[last_fram];
        if (msg_flag[dev_id] == ipc_msg_bitmap[last_fram]) {
            msg_flag[dev_id] = 0;
            atomic_set(&ipc_info[dev_id].wait_flag, 0);
            wmb();
            wake_up(&ipc_info[dev_id].mailbox_wait_queque);
        }
    }

    return;
}

int tsmng_get_powerinfo(u32 dev_id, u64 *aicore_cycle, u64 *timestamp1, u64 *timestamp2, u32 *system_flag)
{
    u64 ts_ret;

    if (dev_id >= MAX_CHIP_NUM) {
        tsmng_drv_err("Dev id is invalid. (dev_id=%u; max_chip_num=%d)\n", dev_id, MAX_CHIP_NUM);
        return -EINVAL;
    }

    if ((aicore_cycle == NULL) || (timestamp1 == NULL) || (timestamp2 == NULL) || (system_flag == NULL)) {
        tsmng_drv_err("Invalid input null (aicore_cycle=%pK, timestamp1=%pK, timestamp2=%pK, system_flag=%pK)\n",
            aicore_cycle, timestamp1, timestamp2, system_flag);
        return -EINVAL;
    }

    *aicore_cycle = *(unsigned long long *)(&ts_cp_msg[dev_id][AICORE_CYCLE_INDEX]);
    *timestamp1 = *(unsigned long long *)(&ts_cp_msg[dev_id][TIME_STAMP1_INDEX]);
    *timestamp2 = *(unsigned long long *)(&ts_cp_msg[dev_id][TIME_STAMP2_INDEX]);
    *system_flag = *(unsigned int *)(&ts_cp_msg[dev_id][SYSTEM_FLAG_INDEX]);

    ts_ret = *(unsigned long long *)(&ts_cp_msg[dev_id][TS_AICORE_RET_INDEX]);
    return (int)ts_ret;
}
EXPORT_SYMBOL(tsmng_get_powerinfo);

#else
void tsmng_pm_ut_stub(void)
{
    return;
}
#endif
