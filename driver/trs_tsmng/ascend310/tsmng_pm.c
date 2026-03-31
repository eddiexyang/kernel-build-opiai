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
 * Create: 2022-08-13
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

#define MATH_TWO    2
#define TSMNG_WAIT_TS_POWER_TIMEOUT (10 * 1000)     /* ms */


u8 msg_flag = 0;
u8 ts_cp_msg[DEVDRV_COMPUTE_POWER_LENGTH] = {0};
struct semaphore compute_power_sema;


void tsmng_ipc_manager_init(u32 dev_id)
{
    sema_init(&compute_power_sema, 0);
}

void tsmng_ipc_manager_exit(u32 dev_id)
{
    /* nothing to do */
}

int tsmng_get_ai_computing_power(unsigned long arg)
{
    rproc_msg_t msg[IPCDRV_RPROC_MSG_LENGTH];
    struct ipcdrv_msg_payload *payload = NULL;
    struct ipcdrv_message *ipc_msg = NULL;
    struct computing_power_arg user_arg = { 0 };
    u32 dev_id;
    int ret;

    if (copy_from_user(&user_arg, (void *)((uintptr_t)arg), sizeof(struct computing_power_arg)) != 0) {
        tsmng_drv_err("Copy_from_user failed.\n");
        return -EINVAL;
    }

    dev_id = user_arg.dev_id;
    if (dev_id >= MAX_CHIP_NUM) {
        tsmng_drv_err("Device id %u is more than %u\n", dev_id, MAX_CHIP_NUM);
        return -EINVAL;
    }

    ipc_msg = (struct ipcdrv_message *)msg;
    ipc_msg->ipc_msg_header.msg_type = MSGTYPE_DRIVER_SEND;
    ipc_msg->ipc_msg_header.cmd_type = IPCDRV_TS_AI_COMPUTING_POWER;
    ipc_msg->ipc_msg_header.sync_type = IPCDRV_MSG_ASYNC;
    ipc_msg->ipc_msg_header.reserved = 0;
    ipc_msg->ipc_msg_header.msg_length = 1;
    ipc_msg->ipc_msg_header.msg_index = 0;
    payload = (struct ipcdrv_msg_payload *)ipc_msg->ipcdrv_payload;
    payload->result = 0;

    ret = icm_msg_send_async(ICM_FD_BUILD(dev_id, HISI_RPROC_TX_TS), msg, IPCDRV_RPROC_MSG_LENGTH);
    if (ret) {
        tsmng_drv_err("Icm_msg_send_async failed. (ret=%d)\n", ret);
        return ret;
    }
    ret = down_timeout(&compute_power_sema, msecs_to_jiffies(TSMNG_WAIT_TS_POWER_TIMEOUT));
    if (ret) {
        tsmng_drv_err("Down_timeout failed.\n");
        return -ETIMEDOUT;
    }

    ret = memcpy_s((void *)&user_arg.compute_power_msg, sizeof(ts_cp_msg), (void *)&ts_cp_msg[0], sizeof(ts_cp_msg));
    if (ret) {
        tsmng_drv_err("Memcpy_s fail. (ret=%d)\n", ret);
        return ret;
    }

    if (copy_to_user((void *)((uintptr_t)arg), &user_arg, sizeof(struct computing_power_arg)) != 0) {
        tsmng_drv_err("Copy_to_user failed.\n");
        return -EINVAL;
    }

    return 0;
}
EXPORT_SYMBOL(tsmng_get_ai_computing_power);

/* not called on 310 */
__attribute__((unused)) void devdrv_get_computing_power(void *data, u32 dev_id)
{
    u32 idx;
    int ret;

    struct ipcdrv_message *ipc_msg = NULL;
    struct tag_computing_power_msg *cp_msg;
    ipc_msg = (struct ipcdrv_message *)data;
    cp_msg = (struct tag_computing_power_msg *)ts_cp_msg;

    idx = ipc_msg->ipc_msg_header.msg_index;
    msg_flag |= 0x1 << idx;

    if (idx == 0) {
        ret = memcpy_s((void *)&ts_cp_msg[0], sizeof(ts_cp_msg) / MATH_TWO, (void *)ipc_msg->ipcdrv_payload,
            sizeof(ts_cp_msg) / MATH_TWO);
    } else if (idx == 1) {
        ret = memcpy_s((void *)&ts_cp_msg[DEVDRV_COMPUTE_POWER_LENGTH / MATH_TWO], sizeof(ts_cp_msg) / MATH_TWO,
            (void *)ipc_msg->ipcdrv_payload, sizeof(ts_cp_msg) / MATH_TWO);
    } else {
        tsmng_drv_err("Index error. (idx=%u)\n", idx);
        return;
    }

    if (ret) {
        tsmng_drv_err("Copy failed. (idx=%u)\n", idx);
        return;
    }

    if (idx == 1) {
        msg_flag &= 0x3;
        if (msg_flag == 0x3) {
            msg_flag = 0;
            up(&compute_power_sema);
        }
    }
    return;
}

static int devdrv_inform_ts_config_aicore(u32 dev_id, int cmd_type)
{
    rproc_msg_t msg[IPCDRV_RPROC_MSG_LENGTH];
    struct ipcdrv_msg_payload *payload = NULL;
    struct ipcdrv_message *ipc_msg = NULL;
    int ret;

    ipc_msg = (struct ipcdrv_message *)msg;
    ipc_msg->ipc_msg_header.msg_type = MSGTYPE_DRIVER_SEND;

    if ((cmd_type != IPCDRV_TS_INFORM_TS_LIMIT_AICORE) &&
        (cmd_type != IPCDRV_TS_INFORM_TS_CANCEL_LIMIT)) {
        tsmng_drv_err("Invalid cmd_type. (dev_id=%u; cmd_type=%d)\n", dev_id, cmd_type);
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

    ret = icm_msg_send_async(ICM_FD_BUILD(0, HISI_RPROC_TX_TS), msg, IPCDRV_RPROC_MSG_LENGTH);
    if (ret) {
        tsmng_drv_err("Icm msg send async failed. (ret=%d)\n", ret);
    }

    return ret;
}

int tsmng_inform_ts_limit_aicore(u32 dev_id)
{
    if (dev_id >= MAX_CHIP_NUM) {
        tsmng_drv_err("Device id %u is more than %u\n", dev_id, MAX_CHIP_NUM);
        return -EINVAL;
    }

    return devdrv_inform_ts_config_aicore(dev_id, IPCDRV_TS_INFORM_TS_LIMIT_AICORE);
}
EXPORT_SYMBOL(tsmng_inform_ts_limit_aicore);

int tsmng_inform_ts_cancel_limit_aicore(u32 dev_id)
{
    if (dev_id >= MAX_CHIP_NUM) {
        tsmng_drv_err("Device id %u is more than %u\n", dev_id, MAX_CHIP_NUM);
        return -EINVAL;
    }

    return devdrv_inform_ts_config_aicore(dev_id, IPCDRV_TS_INFORM_TS_CANCEL_LIMIT);
}
EXPORT_SYMBOL(tsmng_inform_ts_cancel_limit_aicore);

int tsmng_inform_ts_idle(void *data, u32 dev_id)
{
    rproc_msg_t msg[IPCDRV_RPROC_MSG_LENGTH];
    struct ipcdrv_msg_payload *payload = NULL;
    struct ipcdrv_message *ipc_msg = NULL;
    int ret;

    ipc_msg = (struct ipcdrv_message *)msg;
    ipc_msg->ipc_msg_header.msg_type = MSGTYPE_DRIVER_RECEIVE;
    ipc_msg->ipc_msg_header.cmd_type = IPCDRV_TS_INFORM_TS_IDLE;
    ipc_msg->ipc_msg_header.sync_type = IPCDRV_MSG_ASYNC;
    ipc_msg->ipc_msg_header.reserved = 0;
    ipc_msg->ipc_msg_header.msg_length = 1;
    ipc_msg->ipc_msg_header.msg_index = 0;
    payload = (struct ipcdrv_msg_payload *)ipc_msg->ipcdrv_payload;
    payload->result = 0;

    ret = icm_msg_send_async(ICM_FD_BUILD(dev_id, HISI_RPROC_TX_TS), msg, IPCDRV_RPROC_MSG_LENGTH);
    if (ret) {
        tsmng_drv_err("Icm_msg_send_async failed. (ret=%d)\n", ret);
    }

    return ret;
}
#else
void tsmng_pm_ut_stub(void)
{
    return;
}
#endif
