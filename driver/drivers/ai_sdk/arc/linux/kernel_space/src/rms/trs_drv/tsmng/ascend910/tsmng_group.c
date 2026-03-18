/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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
#include <linux/uaccess.h>
#include <linux/securec.h>
#include <linux/of.h>
#include <linux/delay.h>

/* ===must include */
#include "tsmng_interface.h"
#include "tsmng_log.h"
#include "tsmng_common.h"
/* must include=== */

#include "tsmng_pm.h"
#include "tsmng_group.h"

#include "devdrv_platform_resource.h"
#include "devdrv_user_common.h"
#ifdef AOS_LLVM_BUILD
#include <linux/mutex.h>
#include <linux/delay.h>
#include "devdrv_manager_common.h"
#include "devdrv_info.h"
#include "drv_whitelist.h"
#include "dms_cmd_def.h"
#include "urd_acc_ctrl.h"
#include "dms_template.h"
#include "icm_interface.h"
#include "drv_ipc.h"
#define TS_CORE_UTIL_DELAY_TIME_MIN 50000   // 50ms
#define TS_CORE_UTIL_DELAY_TIME_MAX 55000   // 55ms
#ifdef CFG_FEATURE_IPC_FORMAT_V2
#define FEATURE_IPC_VARIABLE_DEFINE \
    struct icmdrv_ipc_msg ipc_msg = {0}; \
    struct icmdrv_ipc_msg_info ack_msg = {0}; \
    char *pdata = &ack_msg.data[1]; \
    u32 max = ICM_MSG_DATA_LENGTH - 1
#else
#define FEATURE_IPC_VARIABLE_DEFINE \
    struct ipcdrv_message ipc_msg = {0}; \
    struct ipcdrv_message ack_msg = {0}; \
    char *pdata = &ack_msg.ipcdrv_payload[1]; \
    u32 max = IPCDRV_MSG_LENGTH - 1
#endif
static DEFINE_MUTEX(g_aicore_mutex);
#endif
#if defined(CFG_FEATURE_LP_ENABLE)
static void devdrv_comm_handle_ts_rsp(int dev_id, struct ts_ipcdrv_message *rsp_msg, void *p_send_channel_info)
{
    int ret;
    struct ipc_msg_channel_info *send_channel_info = (struct ipc_msg_channel_info *)p_send_channel_info;
    ret = memcpy_s(send_channel_info->rsp_msg, MAX_IPCDRV_MSG_LENGTH, rsp_msg, sizeof(struct ts_ipcdrv_message));
    if (ret != 0) {
        tsmng_drv_err("Memcpy fail. (ret = %d; msg type=%d)\n", ret, rsp_msg->ipc_msg_header.cmd_type);
        return;
    }

    if (tsmng_chk_ipc_crc16((struct ipcdrv_message *)rsp_msg) != 0) {
        tsmng_drv_err("Crc mismatch\n");
        return;
    }
    up(&send_channel_info->msg_sema);
}

static int devdrv_check_create_capability_group_rsp(struct ts_ipcdrv_message *req_msg,
    struct ts_ipcdrv_message *rsp_msg, int retry_time)
{
    struct ipc_operate_group_rsp_payload *rsp_msg_payload = NULL;
    struct ipc_operate_group_req_payload *req_msg_payload = NULL;

    req_msg_payload = (struct ipc_operate_group_req_payload *)req_msg->ipcdrv_payload;
    rsp_msg_payload = (struct ipc_operate_group_rsp_payload *)rsp_msg->ipcdrv_payload;

    if (sizeof(struct ipc_operate_group_rsp_payload) > sizeof(rsp_msg->ipcdrv_payload)) {
        tsmng_drv_err("Payload %ld is more than  = %ld\n", sizeof(struct ipc_operate_group_rsp_payload),
            sizeof(rsp_msg->ipcdrv_payload));
        return -EINVAL;
    }
    if (sizeof(struct ipc_operate_group_req_payload) > sizeof(req_msg->ipcdrv_payload)) {
        tsmng_drv_err("Payload %ld is more than  = %ld\n", sizeof(struct ipc_operate_group_req_payload),
            sizeof(req_msg->ipcdrv_payload));
        return -EINVAL;
    }
    // IPCDRV_TS_OPERATE_GROUP maybe create or delete group
    if (req_msg_payload->operate_flag != rsp_msg_payload->operate_flag) {
        tsmng_log_adust(retry_time, "Req operate_flag payload %d is not equal rsp operate_flag%d\n",
            req_msg_payload->operate_flag, rsp_msg_payload->operate_flag);
        return -EINVAL;
    }
    return 0;
}


struct ipc_msg_channel_info g_ipc_msg_channel_info[MAX_CHIP_NUM][MAX_IPC_MSG_TYPE] = {
    {
        {
            .device_id = 0,
            .msg_type = IPCDRV_TSV_OPERATE_GROUP,
            .send_channel_id = HISI_RPROC_TSV_TX_RPID1,
            .receive_channel_id = HISI_RPROC_TSV_RX_RPID4,
            .msg_serial_num = 0,
            .rsp_msg = {0},
            .ipc_rsp_handle_fun = devdrv_comm_handle_ts_rsp,
            .ipc_rsp_check_handle_fun = devdrv_check_create_capability_group_rsp
        }, {
            .device_id = 0,
            .msg_type = IPCDRV_TSV_GET_GROUP_INFO,
            .send_channel_id = HISI_RPROC_TSV_TX_RPID1,
            .receive_channel_id = HISI_RPROC_TSV_RX_RPID4,
            .msg_serial_num = 0,
            .rsp_msg = {0},
            .ipc_rsp_handle_fun = devdrv_comm_handle_ts_rsp,
            .ipc_rsp_check_handle_fun = NULL
        }, {
            .device_id = 0,
            .msg_type = IPCDRV_TSC_OPERATE_GROUP,
            .send_channel_id = HISI_RPROC_TSC_TX_RPID1,
            .receive_channel_id = HISI_RPROC_TSC_RX_RPID4,
            .msg_serial_num = 0,
            .rsp_msg = {0},
            .ipc_rsp_handle_fun = devdrv_comm_handle_ts_rsp,
            .ipc_rsp_check_handle_fun = devdrv_check_create_capability_group_rsp
        }, {
            .device_id = 0,
            .msg_type = IPCDRV_TSC_GET_GROUP_INFO,
            .send_channel_id = HISI_RPROC_TSC_TX_RPID1,
            .receive_channel_id = HISI_RPROC_TSC_RX_RPID4,
            .msg_serial_num = 0,
            .rsp_msg = {0},
            .ipc_rsp_handle_fun = devdrv_comm_handle_ts_rsp,
            .ipc_rsp_check_handle_fun = NULL
        },
    }
};

// ipc hardware constraints, sender and receive channel must agree and same
// so msg and channel must bind, channel and msg relation is many to one
void devdrv_ipc_ts_msg_channel_init(u32 dev_id)
{
    int i;

    if (dev_id >= MAX_CHIP_NUM) {
        tsmng_drv_err("Device id %u is more than %u\n", dev_id, MAX_CHIP_NUM);
        return;
    }

    for (i = 0; i < MAX_IPC_MSG_TYPE; i++) {
        g_ipc_msg_channel_info[dev_id][i].device_id = dev_id;
        g_ipc_msg_channel_info[dev_id][i].msg_type = g_ipc_msg_channel_info[0][i].msg_type;
        g_ipc_msg_channel_info[dev_id][i].send_channel_id = g_ipc_msg_channel_info[0][i].send_channel_id;
        g_ipc_msg_channel_info[dev_id][i].receive_channel_id = g_ipc_msg_channel_info[0][i].receive_channel_id;
        g_ipc_msg_channel_info[dev_id][i].ipc_rsp_handle_fun = g_ipc_msg_channel_info[0][i].ipc_rsp_handle_fun;
        g_ipc_msg_channel_info[dev_id][i].ipc_rsp_check_handle_fun =
        g_ipc_msg_channel_info[0][i].ipc_rsp_check_handle_fun;
        g_ipc_msg_channel_info[dev_id][i].msg_serial_num = 0;
        mutex_init(&g_ipc_msg_channel_info[dev_id][i].msg_mutex);
        sema_init(&g_ipc_msg_channel_info[dev_id][i].msg_sema, 0);
    }
}

void devdrv_ipc_ts_msg_channel_destroy(u32 dev_id)
{
    int i;

    for (i = 0; i < MAX_IPC_MSG_TYPE; i++) {
        mutex_destroy(&g_ipc_msg_channel_info[dev_id][i].msg_mutex);
    }
}

struct ipc_msg_channel_info *devdrv_find_ipc_channel(unsigned int dev_id, unsigned int msg_type)
{
    int i;
    if (dev_id >= MAX_CHIP_NUM) {
        tsmng_drv_err("Device id %u is more than %u\n", dev_id, MAX_CHIP_NUM);
        return NULL;
    }

    for (i = 0; i < MAX_IPC_MSG_TYPE; i++) {
        if (g_ipc_msg_channel_info[dev_id][i].msg_type == msg_type &&
            g_ipc_msg_channel_info[dev_id][i].device_id == dev_id)
            return &g_ipc_msg_channel_info[dev_id][i];
    }
    return NULL;
}

static int devdrv_check_ipc_ts_rsp_msg(struct ipc_msg_channel_info *send_channel_info,
    struct ts_ipcdrv_message *req_msg, struct ts_ipcdrv_message *rsp_msg, int retry_time)
{
    int ret;
    unsigned int device_id = send_channel_info->device_id;
    unsigned int cmd_type = send_channel_info->msg_type;
    unsigned int send_channel_id = send_channel_info->send_channel_id;

    ret = memcpy_s(rsp_msg, sizeof(struct ts_ipcdrv_message), send_channel_info->rsp_msg, MAX_IPCDRV_MSG_LENGTH);
    if (ret != 0) {
        tsmng_log_adust(retry_time, "Memcpy fail ret=%d.try time=%d,device_id=%d,msg_type=%d,channel_id=%d\n",
            ret, IPC_RETRY_TIME, device_id, cmd_type, send_channel_id);
        return -EAGAIN;
    }
    // reques msg type and response msg type must same
    if (rsp_msg->ipc_msg_header.msg_type != MSGTYPE_DRIVER_RECEIVE) {
        tsmng_log_adust(retry_time, "Rsp msg_type =%d not equal req msg_type =%d\n",
            rsp_msg->ipc_msg_header.msg_type, MSGTYPE_DRIVER_RECEIVE);
        return -EAGAIN;
    }
    // reques cmd type and response cmd type must same
    if (rsp_msg->ipc_msg_header.cmd_type != req_msg->ipc_msg_header.cmd_type) {
        tsmng_log_adust(retry_time, "Rsp cmd_type =%d not equal req cmd_type =%d\n",
            rsp_msg->ipc_msg_header.cmd_type, req_msg->ipc_msg_header.cmd_type);
        return -EAGAIN;
    }

    // reques msg idx and response msg idx must same
    if (rsp_msg->msg_serial_num != req_msg->msg_serial_num) {
        tsmng_log_adust(retry_time, "Rsp msg_index =%d not equal req msg_index =%d\n",
            rsp_msg->msg_serial_num, req_msg->msg_serial_num);
        return -EAGAIN;
    }

    // different businesses have their own criteria
    if (send_channel_info->ipc_rsp_check_handle_fun != NULL) {
        ret = send_channel_info->ipc_rsp_check_handle_fun(req_msg, rsp_msg, retry_time);
        if (ret != 0) {
            tsmng_log_adust(retry_time, "Check rsp fail ret=%d.device id=%d,try time=%d,msg_type=%d,channel_id=%d\n",
                ret, retry_time, device_id, rsp_msg->ipc_msg_header.msg_type, send_channel_id);
            return ret;
        }
    }
    return 0;
}

static int devdrv_ipc_ts_msg_send(struct ipc_msg_channel_info *send_channel_info, struct ts_ipcdrv_message *req_msg,
                                  struct ts_ipcdrv_message *rsp_msg)
{
    int i;
    int ret;
    unsigned int device_id = send_channel_info->device_id;
    unsigned int channel_id = send_channel_info->send_channel_id;
    unsigned int cmd_type = send_channel_info->msg_type;
    unsigned int send_len = sizeof(struct ts_ipcdrv_message) / sizeof(rproc_msg_t);

    for (i = 0; i < IPC_RETRY_TIME; i++) {
        // 1.send request msg
        ret = icm_msg_send_async(ICM_FD_BUILD(device_id, (rproc_id_t)channel_id), (rproc_msg_t *)req_msg, send_len);
        if (ret != 0) {
            tsmng_drv_warn("Send request msg. (ret=%d. try time=%d, device_id=%d, msg_type=%d, channel_id=%d).\n",
                ret, i, device_id, cmd_type, channel_id);
            continue;
        }
        // 2.wati response msg
        ret = down_timeout(&send_channel_info->msg_sema, msecs_to_jiffies(GET_DATA_WAIT_TIMEOUT));
        if (ret != 0) {
            tsmng_drv_warn("Wait respone time out ret=%d. try time=%d,device_id=%d, msg_type=%d,channel_id=%d\n",
                ret, i, device_id, cmd_type, channel_id);
            continue;
        }
        // 3.receive response check receive response msg
        ret = devdrv_check_ipc_ts_rsp_msg(send_channel_info, req_msg, rsp_msg, i);
        if (ret == 0) {
            break;
        }
        tsmng_drv_warn("Rsp msg check. (ret=%d, try time=%d, dev_id=%d, msg_type=%d, channel_id=%d)\n",
            ret, i, device_id, cmd_type, channel_id);
    }
    if (i == IPC_RETRY_TIME) {
        // retry 3 time always fail
        tsmng_drv_err("Send msg fail ret=%d. try time=%d,device_id=%d, msg_type=%d,channel_id=%d\n",
            ret, i, device_id, send_channel_info->msg_type, channel_id);
        return ret;
    }
    return 0;
}

#define TSMNG_CHANNEL_LOCK_MAX_TRY_TIMES (10000)
#define TSMNG_CHANNEL_RETYR_INTERVAL (10U)  // 10ms
static int tsmng_group_try_lock(struct mutex *msg_mutex)
{
    unsigned int i = 0;
    while (!mutex_trylock(msg_mutex)) {
        if (i >= TSMNG_CHANNEL_LOCK_MAX_TRY_TIMES) {
            tsmng_drv_err("Failed to obtain the lock of the IPC channel, (cost time=%u ms).\n",
                i * TSMNG_CHANNEL_RETYR_INTERVAL);
            return -EINVAL;
        }
        msleep(TSMNG_CHANNEL_RETYR_INTERVAL);
        i++;
    }

    return 0;
}

static int tsmng_group_xfer_to_ts_sync(unsigned int device_id, unsigned int msg_type,
    struct ts_ipcdrv_message *req_msg, struct ts_ipcdrv_message *rsp_msg)
{
    int ret;
    struct ipc_msg_channel_info *send_channel_info = NULL;

    if (req_msg == NULL) {
        tsmng_drv_err("Req_msg null");
        return -EINVAL;
    }
    if (rsp_msg == NULL) {
        tsmng_drv_err("Rsp_msg null");
        return -EINVAL;
    }
    // in order to support multi process,find channel mutex and lock info by already known chanel id
    send_channel_info = devdrv_find_ipc_channel(device_id, msg_type);
    if (send_channel_info == NULL) {
        tsmng_drv_err("Can not find send channel device_id = %d, msg_type  = %d\n", device_id, msg_type);
        return -EINVAL;
    }

    if (tsmng_group_try_lock(&send_channel_info->msg_mutex) != 0) {
        return -EINVAL;
    }
    sema_init(&send_channel_info->msg_sema, 0);
    req_msg->ipc_msg_header.msg_type = MSGTYPE_DRIVER_SEND;
    req_msg->ipc_msg_header.cmd_type = msg_type;
    req_msg->ipc_msg_header.sync_type = IPCDRV_MSG_ASYNC;
    req_msg->ipc_msg_header.reserved = 0;
    req_msg->ipc_msg_header.msg_length = MAX_IPCDRV_MSG_LENGTH - TS_IPC_MSG_HEAD_LEN - TS_IPC_MSG_CRC_LEN;
    req_msg->msg_serial_num = send_channel_info->msg_serial_num;
    send_channel_info->msg_serial_num++;

    tsmng_drv_debug("Msg_type = %d, cmd_type = %d,msg_serial_num = %d\n",
        req_msg->ipc_msg_header.msg_type, req_msg->ipc_msg_header.cmd_type, req_msg->msg_serial_num);

    ret = devdrv_ipc_ts_msg_send(send_channel_info, req_msg, rsp_msg);
    if (ret != 0) {
        mutex_unlock(&send_channel_info->msg_mutex);
        tsmng_drv_err("Wait respone time out ret=%d.try time =%d,device_id=%d, msg_type=%d, channel_id=%d\n",
            ret, IPC_RETRY_TIME, device_id, msg_type, send_channel_info->send_channel_id);
        return ret;
    }
    mutex_unlock(&send_channel_info->msg_mutex);
    return 0;
}

int tsmng_group_getinfo_from_ts_sync(unsigned int device_id, int ts_id,
    struct ts_ipcdrv_message *req_msg, struct ts_ipcdrv_message *rsp_msg)
{
    unsigned int msg_type;

    msg_type = (ts_id == DEVDRV_TS_AICORE ? IPCDRV_TSC_GET_GROUP_INFO : IPCDRV_TSV_GET_GROUP_INFO);
    return tsmng_group_xfer_to_ts_sync(device_id, msg_type, req_msg, rsp_msg);
}
EXPORT_SYMBOL(tsmng_group_getinfo_from_ts_sync);

int tsmng_group_operate_to_ts_sync(unsigned int device_id, int ts_id,
    struct ts_ipcdrv_message *req_msg, struct ts_ipcdrv_message *rsp_msg)
{
    unsigned int msg_type;

    msg_type = (ts_id == DEVDRV_TS_AICORE ? IPCDRV_TSC_OPERATE_GROUP : IPCDRV_TSV_OPERATE_GROUP);
    return tsmng_group_xfer_to_ts_sync(device_id, msg_type, req_msg, rsp_msg);
}
EXPORT_SYMBOL(tsmng_group_operate_to_ts_sync);

void devdrv_tsid_decode(unsigned int m_id, unsigned int *tsid)
{
    if (m_id == HISI_RPROC_TSC_RX_RPID4) {
        *tsid = 0;
    } else if (m_id == HISI_RPROC_TSV_RX_RPID4) {
        *tsid = 1;
    }
    tsmng_drv_debug("M_id = %u, tsid = %u\n", m_id, *tsid);
}

void devdrv_handle_ipc_msg(int device_id, void *data)
{
    unsigned int cmd_type;
    ipc_rsp_msg_handle fun_handle;
    struct ipc_msg_channel_info *send_channel_info = NULL;
    struct ts_ipcdrv_message *rsp_msg = (struct ts_ipcdrv_message *)data;

    if (rsp_msg == NULL) {
        tsmng_drv_err("Rsp msg is null\n");
        return;
    }

    cmd_type = rsp_msg->ipc_msg_header.cmd_type;
    send_channel_info = devdrv_find_ipc_channel(device_id, cmd_type);
    if (send_channel_info == NULL) {
        return;
    }
    fun_handle = send_channel_info->ipc_rsp_handle_fun;
    if (fun_handle == NULL) {
        tsmng_drv_warn("Find send chanel but it rsp handle is null cmd_type = %d\n", cmd_type);
        return;
    }
    fun_handle(device_id, rsp_msg, (void *)send_channel_info);
    return;
}
#endif

#ifdef AOS_LLVM_BUILD
static int tsmng_get_spec_group_info(unsigned int device_id, int ts_id, int group_id,
                                     struct ts_group_info *group_info)
{
    int ret;
    struct ts_ipcdrv_message req_msg = {0};
    struct ts_ipcdrv_message rsp_msg = {0};
    struct ipc_rsp_ts_group_info *rsp_group_info = NULL;

    req_msg.ipcdrv_payload[0] = group_id;

    ret = tsmng_group_getinfo_from_ts_sync(device_id, ts_id, &req_msg, &rsp_msg);
    if (ret != 0) {
        tsmng_drv_err("send get ts group info msg failed, ret = %d, device id = %d, ts id=%d\n",
                      ret, device_id, ts_id);
        return ret;
    }

    if (sizeof(struct ipc_rsp_ts_group_info) > sizeof(rsp_msg.ipcdrv_payload)) {
        tsmng_drv_err("sizeof(struct ipc_rsp_ts_group_info) %ld > sizeof(rsp_msg.ipcdrv_payload) %ld\n",
                      sizeof(struct ipc_rsp_ts_group_info), sizeof(rsp_msg.ipcdrv_payload));
        return -EINVAL;
    }
    rsp_group_info = (struct ipc_rsp_ts_group_info *)rsp_msg.ipcdrv_payload;
    group_info->group_id = rsp_group_info->group_id;
    group_info->state = rsp_group_info->state;
    group_info->extend_attribute = rsp_group_info->extend_attribute;
    group_info->aicore_number = rsp_group_info->aicore_number;
    group_info->aivector_number = rsp_group_info->aivector_number;
    group_info->sdma_number = rsp_group_info->sdma_number;
    group_info->aicpu_number = rsp_group_info->aicpu_number;
    group_info->active_sq_number = rsp_group_info->active_sq_number;

    return 0;
}

static int tsmng_check_ts_id(int ts_id)
{
    if (ts_id < 0) {
        tsmng_drv_err("ts id %d is invalid,ts id must big than 0\n", ts_id);
        return -EINVAL;
    }

    if (ts_id >= DEVDRV_TS_MAX) {
        tsmng_drv_err("ts id %d is invalid,in dc ts id must less than %d\n", ts_id, DEVDRV_TS_MAX);
        return -EINVAL;
    }

    return 0;
}

static int tsmng_get_group_info_check_para(struct get_ts_group_para *group_para,
                                           struct ts_group_info *group_info)
{
    int ret;
    int group_id;
    int group_count;

    if (group_para == NULL) {
        tsmng_drv_err("group_para is null\n");
        return -EINVAL;
    }

    if (group_info == NULL) {
        tsmng_drv_err("group_info is null\n");
        return -EINVAL;
    }
    ret = tsmng_check_ts_id(group_para->ts_id);
    if (ret != 0) {
        tsmng_drv_err("ts id %d is invalid\n", group_para->ts_id);
        return ret;
    }

    group_id = group_para->group_id;
    group_count = group_para->group_count;

    if (group_id == -1 && group_count < DEVDRV_TS_GROUP_NUM) {
        tsmng_drv_err("group_id == -1 but group_count %d != %d\n", group_count, DEVDRV_TS_GROUP_NUM);
        return -EINVAL;
    } else if (group_id != -1 && group_count < 1) {
        tsmng_drv_err("group_id != -1 but group_count %d != 1\n", group_count);
        return -EINVAL;
    } else if (group_id >= DEVDRV_TS_GROUP_NUM) {
        tsmng_drv_err("group_id %d must less than %d\n", group_id, DEVDRV_TS_GROUP_NUM);
        return -EINVAL;
    } else {
        return 0;
    }
}

static int tsmng_get_group_info(struct get_ts_group_para *group_para,
                                struct ts_group_info *group_info, int group_info_num)
{
    int i = 0;
    int ret;
    unsigned int device_id = group_para->device_id;
    int ts_id = group_para->ts_id;
    int group_id = group_para->group_id;

    ret = tsmng_get_group_info_check_para(group_para, group_info);
    if (ret != 0) {
        tsmng_drv_err("para check fail ret = %d\n", ret);
        return ret;
    }
    // get specific info
    if (group_id != -1) {
        ret = tsmng_get_spec_group_info(device_id, ts_id, group_id, group_info);
        if (ret != 0) {
            tsmng_drv_err("get ts group info fail device_id = %d, ts_id  = %d, group_id  = %d\n",
                          group_para->device_id, group_para->ts_id, group_id);
            return ret;
        }
        return 0;
    }
    if (group_info_num < DEVDRV_TS_GROUP_NUM) {
        tsmng_drv_err("group info num = %d, less than %d\n", group_info_num, DEVDRV_TS_GROUP_NUM);
        return -EINVAL;
    }
    // get all group info
    for (i = 0; i < DEVDRV_TS_GROUP_NUM; i++) {
        ret = tsmng_get_spec_group_info(device_id, ts_id, i, &group_info[i]);
        if (ret != 0) {
            tsmng_drv_err("get ts group info fail device_id = %d, ts_id  = %d, group_id  = %d\n",
                          group_para->device_id, group_para->ts_id, i);
            return ret;
        }
    }
    return 0;
}

static int devdrv_manager_common_status_to_user(const void *status, unsigned int status_size,
                                                common_status_info_t *common_status)
{
    int ret;

    if (status == NULL) {
        tsmng_drv_err("status is invalid.\n");
        return -EINVAL;
    }

    if (common_status->commoninfo_len < status_size) {
        tsmng_drv_err("struct length Not compatible, input lenght = %d, need length %d\n",
                      common_status->commoninfo_len, status_size);
        return -EINVAL;
    }

    ret = copy_to_user(common_status->commoninfo, status, status_size);
    if (ret) {
        tsmng_drv_err("copy commoninfo failed, ret = %d\n", ret);
        return ret;
    }

    common_status->commoninfo_len = status_size;

    return ret;
}

#define TS_NUM_INVALID (-1)
static int g_ts_node_num = TS_NUM_INVALID;
static int tsmng_get_ts_node_num(void)
{
    int ts_num = 0;
    struct device_node *node = NULL;
    struct device_node *son = NULL;

    node = of_find_compatible_node(NULL, NULL, "hisi,mini-devdrv-device");
    if (node == NULL) {
        tsmng_drv_info("The mini-devdrv-device is not configured in dts.\n");
        return 0;
    }

    son = of_find_node_by_name(node, "ts-0");
    if (son != NULL) {
        tsmng_drv_info("sucess to find ts-0\n");
        ts_num++;
    }

    son = of_find_node_by_name(node, "ts-1");
    if (son != NULL) {
        tsmng_drv_info("sucess to find ts-1\n");
        ts_num++;
    }
    tsmng_drv_info("find ts (%d)\n", ts_num);
    return ts_num;
}

static int tsmng_ts_is_enable(void)
{
    if (g_ts_node_num == TS_NUM_INVALID) {
        g_ts_node_num = tsmng_get_ts_node_num();
    }

    if (g_ts_node_num == 0) {
        return false;
    }

    return true;
}

static int tsmng_create_capability_group(void *feature, char *in, u32 in_len, char *out, u32 out_len)
{
    int ret;
    int ts_id;
    struct ts_group_info group_info = {0};
    struct ts_ipcdrv_message req_msg = {0};
    struct ts_ipcdrv_message rsp_msg = {0};
    struct ipc_operate_group_req_payload *req_payload = NULL;
    struct ipc_operate_group_rsp_payload *rsp_payload = NULL;
    common_status_info_t *common_status = (common_status_info_t *)in;
#ifndef AOS_LLVM_BUILD
    const char *wl_process_name = "dmp_daemon";

    /* check process whitelist */
    ret = whitelist_process_handler(&wl_process_name, 1);
    if (ret != 0) {
        tsmng_drv_err("whitelist_process_handler ret = %d\n", ret);
        return ret;
    }
#endif

    if ((in == NULL) || (in_len != sizeof(common_status_info_t))) {
        tsmng_drv_err("Input data is NULL or input data length is wrong. (in_len=%u; correct_in_len=%lu)\n",
            in_len, sizeof(unsigned long));
        return -EINVAL;
    }
    if (tsmng_ts_is_enable() == false) {
        devdrv_drv_info("can't find ts node, check whether the TS subsystem is deployed in the system.\n");
        return -EOPNOTSUPP;
    }

    if (common_status->dev_id >= DEVDRV_MANGER_MAX_DEVICE_NUM) {
        tsmng_drv_err("Invalid devid(%u)\n", common_status->dev_id);
        return -EINVAL;
    }
    if (common_status->commoninfo_len != sizeof(struct ts_group_info)) {
        tsmng_drv_err("common_status.commoninfo_len = %d not equal %ld\n",
                      common_status->commoninfo_len, sizeof(struct ts_group_info));
        return -EINVAL;
    }
    ret = copy_from_user_safe(&group_info, common_status->commoninfo, sizeof(struct ts_group_info));
    if (ret != 0) {
        tsmng_drv_err("copy commoninfo failed, ret = %d\n", ret);
        return ret;
    }

    req_payload = (struct ipc_operate_group_req_payload *)req_msg.ipcdrv_payload;
    req_payload->group_id = group_info.group_id;
    req_payload->operate_flag = TS_GROUP_OPERATE_CREATE;
    req_payload->extend_attribute = group_info.extend_attribute;
    req_payload->aicore_number = group_info.aicore_number;
    req_payload->aivector_number = group_info.aivector_number;
    req_payload->sdma_number = group_info.sdma_number;
    req_payload->aicpu_number = group_info.aicpu_number;
    req_payload->active_sq_number = group_info.active_sq_number;
    ts_id = common_status->reserver[GET_GROUP_TS_ID_RES_INDEX];
    ret = tsmng_check_ts_id(ts_id);
    if (ret != 0) {
        tsmng_drv_err("ts id %d is invalid\n", ts_id);
        return ret;
    }

    tsmng_drv_info("create capability group, "
                   "device id=%d, ts_id=%d, group_id=%d, extend_attribute=%d, aicore_number=%d\n"
                   "aivector_number=%d, sdma_number=%d, aicpu_number=%d, active_sq_number=%d\n",
                   common_status->dev_id, ts_id, group_info.group_id,
                   group_info.extend_attribute, group_info.aicore_number, group_info.aivector_number,
                   group_info.sdma_number, group_info.aicpu_number, group_info.active_sq_number);

    ret = tsmng_group_operate_to_ts_sync(common_status->dev_id, ts_id, &req_msg, &rsp_msg);
    if (ret != 0) {
        tsmng_drv_err("create ts group send ipc msg failed, ret = %d, device id = %d, ts id=%d\n",
                      ret, common_status->dev_id, ts_id);
        return ret;
    }

    rsp_payload = (struct ipc_operate_group_rsp_payload *)rsp_msg.ipcdrv_payload;
    if (rsp_payload->result != 0) {
        tsmng_drv_err("create group,receive ts response, but rsp_msg.ipcdrv_payload.result = %d\n"
                      "device id=%d, ts id=%d, group_id=%d, extend_attribute=%d, aicore_number=%d\n"
                      "aivector_number=%d, sdma_number=%d, aicpu_number=%d, active_sq_number=%d\n",
                      rsp_payload->result, common_status->dev_id, ts_id, group_info.group_id,
                      group_info.extend_attribute, group_info.aicore_number, group_info.aivector_number,
                      group_info.sdma_number, group_info.aicpu_number, group_info.active_sq_number);
        return -EBADRQC;
    }

    return 0;
}

static int tsmng_delete_capability_group(void *feature, char *in, u32 in_len, char *out, u32 out_len)
{
    int ret;
    int ts_id;
    struct ts_ipcdrv_message req_msg = {0};
    struct ts_ipcdrv_message rsp_msg = {0};
    struct delete_ts_group_info delete_info = {0};
    struct ipc_operate_group_rsp_payload *rsp_payload = NULL;
    struct ipc_operate_group_req_payload *req_del_payload = NULL;
    common_status_info_t *common_status = (common_status_info_t *)in;
#ifndef AOS_LLVM_BUILD
    const char *wl_process_name = "dmp_daemon";

    /* check process whitelist */
    ret = whitelist_process_handler(&wl_process_name, 1);
    if (ret != 0) {
        tsmng_drv_err("whitelist_process_handler ret = %d\n", ret);
        return ret;
    }
#endif

    if (tsmng_ts_is_enable() == false) {
        devdrv_drv_info("can't find ts node, check whether the TS subsystem is deployed in the system.\n");
        return -EOPNOTSUPP;
    }
    if ((in == NULL) || (in_len != sizeof(common_status_info_t))) {
        tsmng_drv_err("Input data is NULL or input data length is wrong. (in_len=%u; correct_in_len=%lu)\n",
            in_len, sizeof(unsigned long));
        return -EINVAL;
    }

    if (common_status->dev_id >= DEVDRV_MANGER_MAX_DEVICE_NUM) {
        tsmng_drv_err("Invalid devid(%u)\n", common_status->dev_id);
        return -EINVAL;
    }
    if (common_status->commoninfo_len != sizeof(struct delete_ts_group_info)) {
        tsmng_drv_err("common_status.commoninfo_len = %d not equal %ld\n",
                      common_status->commoninfo_len, sizeof(struct delete_ts_group_info));
        return -EINVAL;
    }
    ret = copy_from_user_safe(&delete_info, common_status->commoninfo, sizeof(struct delete_ts_group_info));
    if (ret != 0) {
        tsmng_drv_err("copy commoninfo failed, ret = %d\n", ret);
        return ret;
    }
    ts_id = delete_info.ts_id;
    ret = tsmng_check_ts_id(ts_id);
    if (ret != 0) {
        tsmng_drv_err("ts id %d is invalid\n", ts_id);
        return ret;
    }
    req_del_payload = (struct ipc_operate_group_req_payload *)req_msg.ipcdrv_payload;
    req_del_payload->group_id = delete_info.group_id;
    req_del_payload->operate_flag = TS_GROUP_OPERATE_DELETE;
    tsmng_drv_info("delete capability group, ts_id(%d), group_id(%d)\n",
                   ts_id, delete_info.group_id);
    ret = tsmng_group_operate_to_ts_sync(common_status->dev_id, ts_id, &req_msg, &rsp_msg);
    if (ret != 0) {
        tsmng_drv_err("send delete group msg failed, ret = %d, device id = %d, ts id=%d\n",
                      ret, common_status->dev_id, ts_id);
        return ret;
    }
    rsp_payload = (struct ipc_operate_group_rsp_payload *)rsp_msg.ipcdrv_payload;
    if (rsp_payload->result != 0) {
        tsmng_drv_err("get delete group response, but ts return fail rsp_msg->ipcdrv_payload.result = %d,"
                      "ts id = %d, group id = %d\n", rsp_payload->result, ts_id, delete_info.group_id);
        return -EBADRQC;
    }

    return 0;
}

static int tsmng_get_capability_group(void *feature, char *in, u32 in_len, char *out, u32 out_len)
{
    struct ts_group_info group_info[DEVDRV_TS_GROUP_NUM] = {0};
    struct get_ts_group_para group_para = {0};
    common_status_info_t *common_status = NULL;
    int return_num;
    int ret;

    if (tsmng_ts_is_enable() == false) {
        devdrv_drv_info("can't find ts node, check whether the TS subsystem is deployed in the system.\n");
        return -EOPNOTSUPP;
    }
    if ((in == NULL) || (in_len != sizeof(common_status_info_t))) {
        tsmng_drv_err("Input data is NULL or input data length is wrong. (in_len=%u; correct_in_len=%lu)\n",
            in_len, sizeof(unsigned long));
        return -EINVAL;
    }

    common_status = (common_status_info_t *)in;
    if (common_status->dev_id >= DEVDRV_MANGER_MAX_DEVICE_NUM) {
        tsmng_drv_err("Invalid devid(%u)\n", common_status->dev_id);
        return -EINVAL;
    }

    group_para.device_id = common_status->dev_id;
    group_para.ts_id = common_status->reserver[GET_GROUP_TS_ID_RES_INDEX];
    group_para.group_id = common_status->reserver[GET_GROUP_ID_RES_INDEX];
    group_para.group_count = common_status->reserver[GET_GROUP_COUNT_RES_INDEX];
    tsmng_drv_info("get capability group info，dev_id(%u), ts_id(%d), group_id(%d), group_cnt(%d).\n",
        group_para.device_id, group_para.ts_id, group_para.group_id, group_para.group_count);
    ret = tsmng_get_group_info(&group_para, group_info, DEVDRV_TS_GROUP_NUM);
    if (ret != 0) {
        tsmng_drv_err("get group info fail, ret = %d\n", ret);
        return ret;
    }
    return_num = (group_para.group_id == -1 ? DEVDRV_TS_GROUP_NUM : 1);
    ret = devdrv_manager_common_status_to_user(group_info, return_num * sizeof(struct ts_group_info),
                                               common_status);
    if (ret != 0) {
        tsmng_drv_err("devdrv_manager_common_status_to_user failed, ret = %d\n", ret);
        return ret;
    }

    return 0;
}

static int mutex_lock_interruptible_retry(struct mutex *lock)
{
    int ret, i;
    for (i = 0; i < 3; i++) { /* retry 3 times */
        ret = mutex_lock_interruptible(lock);
        if (ret == 0) {
            return 0;
        }
        tsmng_drv_warn("A signal arrived process was interrupted. (ret=%d\n", ret);
    }
    return ret;
}

bool devdrv_manager_is_pf_device(unsigned int dev_id)
{
    return true;
}

static unsigned int tsmng_get_aic_aiv_num(unsigned int cmd_type)
{
    return (cmd_type == DSMI_TS_SUB_AICORE_UTILIZATION_RATE) ? (TSMNG_AI_CORE_NUM) : (TSMNG_AI_VECTOR_NUM);
}

static int tsmng_get_msg_from_ts(u32 dev_id, struct ipcdrv_message *ipc, struct ipcdrv_message *msg_ret,
    u8 *ts_ret, u32 *data_len)
{
    int ret;
    u32 chan_id;

    if ((ipc == NULL) || (msg_ret == NULL)) {
        tsmng_drv_err("ipc or msg_ret pointer is null.\n");
        return -EINVAL;
    }

    if ((ipc->ipc_msg_header.cmd_type == IPCDRV_TS_AICORE_UTILIZATION_RATE) ||
        (ipc->ipc_msg_header.cmd_type == IPCDRV_TS_AICORE_UTILIZATION_RATE_END)) {
        chan_id = HISI_RPROC_TSC_TX_RPID0;
    } else if ((ipc->ipc_msg_header.cmd_type == IPCDRV_TS_VECTORCORE_UTILIZATION_RATE) ||
        (ipc->ipc_msg_header.cmd_type == IPCDRV_TS_VECTORCORE_UTILIZATION_RATE_END)) {
#ifdef CFG_SOC_PLATFORM_MDC_V51
        chan_id = HISI_RPROC_TSV_TX_RPID0;
#else
        chan_id = HISI_RPROC_TSC_TX_RPID0;
#endif
    } else {
        tsmng_drv_err("cmd is not support. (cmd_type=%u)\n", ipc->ipc_msg_header.cmd_type);
        return -EOPNOTSUPP;
    }

    ret = icm_msg_send_sync(ICM_FD_BUILD(dev_id, chan_id), (rproc_msg_t *)ipc,
        sizeof(struct ipcdrv_message) / sizeof(rproc_msg_t), (rproc_msg_t *)msg_ret, IPCDRV_RPROC_MSG_LENGTH);
    if (ret != 0) {
        tsmng_drv_err("icm send msg failed. (cmd_type=%u; ret=%d)\n", ipc->ipc_msg_header.cmd_type, ret);
        return ret;
    }

    *ts_ret = msg_ret->ipcdrv_payload[0];
    *data_len = (u32)(msg_ret->ipc_msg_header.msg_length) - sizeof(struct ipcdrv_msg_header);

    if ((msg_ret->ipc_msg_header.cmd_type != ipc->ipc_msg_header.cmd_type)) {
        tsmng_drv_err("ipc message validity check failed, tsmng_msg.head.cmd_type(%u), "
            "ts_reply_msg.head.cmd_type(%u).\n", ipc->ipc_msg_header.cmd_type, msg_ret->ipc_msg_header.cmd_type);
        return -EIO;
    }
    return ret;
}

static int tsmng_fill_ipcmsg_to_ts(core_utilization_rate_t core_status, struct ipcdrv_message *ipc_msg, int flag)
{
    ipc_msg->ipcdrv_payload[0] = (u8)core_status.vfid;
    ipc_msg->ipc_msg_header.msg_type = MSGTYPE_DRIVER_SEND;
    ipc_msg->ipc_msg_header.sync_type = IPCDRV_MSG_ASYNC;
    ipc_msg->ipc_msg_header.msg_length = sizeof(struct ipcdrv_msg_header) + sizeof(u8);
    ipc_msg->ipc_msg_header.msg_index = 0;

    switch (core_status.cmd_type) {
        case DSMI_TS_SUB_AICORE_UTILIZATION_RATE:
            ipc_msg->ipc_msg_header.cmd_type = (flag == START_MSG) ? (IPCDRV_TS_AICORE_UTILIZATION_RATE) :
                (IPCDRV_TS_AICORE_UTILIZATION_RATE_END);
            break;
        case DSMI_TS_SUB_VECTORCORE_UTILIZATION_RATE:
            ipc_msg->ipc_msg_header.cmd_type = (flag == START_MSG) ? (IPCDRV_TS_VECTORCORE_UTILIZATION_RATE) :
                (IPCDRV_TS_VECTORCORE_UTILIZATION_RATE_END);
            break;
        default:
            tsmng_drv_err("cmd not support. (cmd_type=%u)\n", core_status.cmd_type);
            return -EOPNOTSUPP;
    }

    return 0;
}

static int tsmng_get_core_info(core_utilization_rate_t *core_status, u8 *core, unsigned int core_len, int flag)
{
    int ret;
    unsigned int i;
    u8 ts_ret;
    u32 data_len, core_num;
    FEATURE_IPC_VARIABLE_DEFINE;

    core_num = tsmng_get_aic_aiv_num(core_status->cmd_type);
    if (core_num > max) {
        tsmng_drv_err("core_num exceed max value. (core_num=%u)\n", core_num);
        return -EINVAL;
    }

    ret = tsmng_fill_ipcmsg_to_ts(*core_status, &ipc_msg, flag);
    if (ret != 0) {
        tsmng_drv_err("tsmng_fill_ipcmsg_to_ts failed, ret = %d.\n", ret);
        return ret;
    }

    ret = tsmng_get_msg_from_ts(core_status->dev_id, &ipc_msg, &ack_msg, &ts_ret, &data_len);
    if (ret != 0) {
        tsmng_drv_err("tsmng_get_msg_from_ts failed, ret = %d.\n", ret);
        return ret;
    }

    if (ts_ret != 0) {
        if (ts_ret == TSMNG_AI_PROFILING_CONFLIC_RET) {
            tsmng_drv_warn("ts ack_msg profiling. (ts_ret=%u)\n", ts_ret);
            core_status->core_num = core_num;
            for (i = 0; i < core_len; i++)
                core[i] = TSMNG_AI_PROFILING_VALUE; /* 0xEF: means confilic with profiling */
            return 0;
        } else if (ts_ret == TSMNG_AI_TIMEOUT_RET) {
            goto out;
        }
        return ts_ret;
    }

out:
    if (data_len == 0) {
        tsmng_drv_err("Ack_msg.data_length is zero. (data_len=%u)\n", data_len);
        return -EIO;
    } else if (data_len == 1) { /* receive START_MSG ack, return */
        return 0;
    }

    core_status->core_num = data_len - sizeof(ts_ret);
    if (core_status->core_num > core_len) {
        tsmng_drv_err("core_num(%d) is larger than max_data_length(%d).\n", core_status->core_num, core_len);
        return -EIO;
    }

    ret = memcpy_s(core, core_len, pdata, core_status->core_num);
    if (ret != 0) {
        tsmng_drv_err("Memcpy_s failed. (ret=%d).\n", ret);
        return ret;
    }

    return ts_ret == TSMNG_AI_TIMEOUT_RET ? TSMNG_AI_TIMEOUT_RET : ret;
}

static int tsmng_get_utilization_data(core_utilization_rate_t *core_status, int flag)
{
    int i, j = 0;
    int ret, ts_ret = 0;
    u8 core[TSMNG_AI_CORE_NUM] = {0};
    struct devdrv_info *dev_info = NULL;

    ret = tsmng_get_core_info(core_status, core, TSMNG_AI_CORE_NUM, flag);
    if (ret != 0) {
        if (ret == TSMNG_AI_TIMEOUT_RET) {
            ts_ret = TSMNG_AI_TIMEOUT_RET;
            goto out;
        }
        tsmng_drv_err("Tsmng get aicore info failed. (ret=%d)\n", ret);
        return ret;
    }

    /* profiling */
    if (core[0] == TSMNG_AI_PROFILING_VALUE) {
        ts_ret = TSMNG_AI_PROFILING_CONFLIC_RET;
        goto out;
    }

    /* start message return */
    if (flag == START_MSG) {
        return 0;
    }

out:
    if (!devdrv_manager_is_pf_device(core_status->dev_id)) {
        dev_info = devdrv_manager_get_devdrv_info(core_status->dev_id);
        if (dev_info == NULL) {
            tsmng_drv_err("Get dev_info failed, dev_info is NULL.\n");
            return -EINVAL;
        }

        for (i = 0; i < TSMNG_AI_CORE_NUM; i++) {
            if (((dev_info->aicore_bitmap >> i) & 1) == 1) {
                core[j++] = core[i];
            }
        }

        core_status->core_num = dev_info->ai_core_num;
    }

    ret = copy_to_user_safe(core_status->core_utilization_rate, core, core_status->core_num);
    if (ret != 0) {
        tsmng_drv_err("[dev_id = %d]:core_status.coreinfo copy_to_user_safe failed.\n", core_status->dev_id);
        return ret;
    }

    return (ts_ret != 0) ? ts_ret : ret;
}

static int tsmng_get_ai_info_from_ts_para_check(char *in, u32 in_len, char *out, u32 out_len)
{
    core_utilization_rate_t *core_status = (core_utilization_rate_t *)in;

    if ((in == NULL) || (in_len != sizeof(core_utilization_rate_t))) {
        tsmng_drv_err("input arg is NULL, or in_len is wrong. (in_len=%u)\n", in_len);
        return -EINVAL;
    }

    if ((out == NULL) || (out_len != sizeof(core_status->core_num))) {
        tsmng_drv_err("output arg is NULL, or out_len is wrong. (out_len=%u)\n", out_len);
        return -EINVAL;
    }

    if ((core_status->dev_id >= DEVDRV_MANGER_MAX_DEVICE_NUM) || (core_status->vfid >= VDAVINCI_MAX_VFID_NUM)) {
        tsmng_drv_err("Invalid para. (dev_id=%u; vfid=%u)\n", core_status->dev_id, core_status->vfid);
        return -EINVAL;
    }

    if (core_status->cmd_type >= DSMI_TS_SUB_CMD_MAX_VALUE) {
        tsmng_drv_err("Input cmd_type is not support. (cmd_type=%u)\n", core_status->cmd_type);
        return -EOPNOTSUPP;
    }

    return 0;
}

static int tsmng_get_ai_info_from_ts(void *feature, char *in, u32 in_len, char *out, u32 out_len)
{
    int ret, retry_num = 0;
    core_utilization_rate_t *core_status = (core_utilization_rate_t *)in;

    if (tsmng_ts_is_enable() == false) {
        tsmng_drv_info("can't find ts node, check whether the TS subsystem is deployed in the system.\n");
        return -EOPNOTSUPP;
    }

    ret = tsmng_get_ai_info_from_ts_para_check(in, in_len, out, out_len);
    if (ret != 0) {
        tsmng_drv_err("check parameters failed. (ret=%d)\n", ret);
        return ret;
    }

    ret = mutex_lock_interruptible_retry(&g_aicore_mutex);
    if (ret != 0) {
        tsmng_drv_err("g_aicore_mutex interrupted by a signal. (ret=%d)\n", ret);
        return ret;
    }

    do {
        retry_num++;
        ret = tsmng_get_utilization_data(core_status, START_MSG);
        if (ret != 0) {
            if (ret == TSMNG_AI_PROFILING_CONFLIC_RET) {
                goto out;
            }
            tsmng_drv_err("Starting calculate utilization failed. (ret=%d)\n", ret);
            mutex_unlock(&g_aicore_mutex);
            return -ENODATA;
        }

        usleep_range(TS_CORE_UTIL_DELAY_TIME_MIN, TS_CORE_UTIL_DELAY_TIME_MAX); /* interval 50 ms */

        ret = tsmng_get_utilization_data(core_status, END_MSG);
        if (ret != 0) {
            if (ret == TSMNG_AI_PROFILING_CONFLIC_RET) {
                goto out;
            }
        }
    } while ((ret == TSMNG_AI_TIMEOUT_RET) && (retry_num < 2)); /* 2 times */

    if (ret != 0) {
        if (ret == TSMNG_AI_TIMEOUT_RET) {
            tsmng_drv_warn("Calculate utilization timeout twice. (ret=%d)\n", ret);
            goto out;
        }
        tsmng_drv_err("Ending calculate utilization failed. (ret=%d)\n", ret);
        mutex_unlock(&g_aicore_mutex);
        return -ENODATA;
    }

out:
    *(unsigned int *)out = core_status->core_num;
    mutex_unlock(&g_aicore_mutex);
    return 0;
}

BEGIN_DMS_MODULE_DECLARATION(DMS_MODULE_TSMNG)
BEGIN_FEATURE_COMMAND()
ADD_FEATURE_COMMAND(DMS_MODULE_TSMNG,
    DMS_MAIN_CMD_TRS,
    DMS_SUBCMD_CREATE_GROUP,
    NULL,
    "dmp_daemon",
    DMS_SUPPORT_ALL,
    tsmng_create_capability_group)
ADD_FEATURE_COMMAND(DMS_MODULE_TSMNG,
    DMS_MAIN_CMD_TRS,
    DMS_SUBCMD_DEL_GROUP,
    NULL,
    "dmp_daemon",
    DMS_SUPPORT_ALL,
    tsmng_delete_capability_group)
ADD_FEATURE_COMMAND(DMS_MODULE_TSMNG,
    DMS_MAIN_CMD_TRS,
    DMS_SUBCMD_GET_GROUP,
    NULL,
    NULL,
    DMS_SUPPORT_ALL,
    tsmng_get_capability_group)
ADD_FEATURE_COMMAND(DMS_MODULE_TSMNG,
    DMS_MAIN_CMD_TRS,
    DMS_SUBCMD_GET_AI_INFO,
    NULL,
    NULL,
    DMS_SUPPORT_ALL,
    tsmng_get_ai_info_from_ts)  // 当前仅迁移dms_get_ai_info_from_ts，后续收编linux需要迁移dms_get_ai_info_from_lp
END_FEATURE_COMMAND()
END_MODULE_DECLARATION()

void tsmng_feature_init(void)
{
    tsmng_drv_info("tsmng register feature to dms\n");
    CALL_INIT_MODULE(DMS_MODULE_TSMNG);
}

void tsmng_feature_exit(void)
{
    CALL_EXIT_MODULE(DMS_MODULE_TSMNG);
}
#endif

#ifdef TSDRV_UT
void tsmng_group_ut_stub(void)
{
    return;
}
#endif
