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

#include <linux/suspend.h>
#include <linux/time.h>
#include <linux/semaphore.h>
#include <linux/delay.h>

#include "dms_pm_chain.h"
#include "dms_define.h"
#include "dms_template.h"
#include "urd_acc_ctrl.h"
#include "dms_basic_info.h"
#include "dms_pm.h"
#include "devdrv_user_common.h"
#include "hisi_rproc.h"
#include "devdrv_ipc.h"
#include "drv_ipc.h"
#include "dms_cmd_def.h"
#include "dms_kernel_version_adapt.h"
#include "kernel_version_adapt.h"
#include "devdrv_ipc_msg.h"
#include "icm_interface.h"
#include "devdrv_manager_common.h"
#ifdef SUPPORT_ASCEND_PM

#define MAX_NAME_LENGTH 32
#define SILS_IPC_MSGTYPE_HAERTBEAT 8
#define SILS_IPC_CMDTYPE_OFF 6
#define SILS_IPC_DATA 0xff
#define SILS_IPC_HEAD_LEN 8
#define SILS_CRC_POLYNOMIAL 0x1021
#define NULL_USHORT 0xFFFF
#define BIT15 0x8000
#define LPR52_SOURECE_ID 0
#define LPR52_TARGET_ID 0x4

STATIC void dms_pm_notify_lp(unsigned int dev_id, u8 cmd_type)
{
    int ret;
    struct devdrv_ipc_imu ipc_msg = {0};
    struct timespec stamp = {0};

    ipc_msg.target_id = LPR52_TARGET_ID;
    ipc_msg.source_id = LPR52_SOURECE_ID;
    ipc_msg.cmd_type0 = CMD_NOTIFY;
    ipc_msg.cmd_type1 = cmd_type;

    stamp = current_kernel_time();
    ipc_msg.data1 = ((unsigned long)stamp.tv_sec) & 0xFFFFFFFF;           // seconds low 32 bits
    ipc_msg.data2 = (((unsigned long)stamp.tv_sec) >> 32) & 0xFFFFFFFF;   // seconds high 32 bits
    ipc_msg.data3 = ((unsigned long)stamp.tv_nsec) & 0xFFFFFFFF;          // nanoseconds  low 32 bits
    ipc_msg.data4 = (((unsigned long)stamp.tv_nsec) >> 32) & 0xFFFFFFFF;  // nanoseconds high 32 bits

    ret = rproc_xfer_async(
        dev_id, HISI_RPROC_LP_Q_TX_RPID4_ACPU3, (rproc_msg_t *)&ipc_msg, sizeof(ipc_msg) / sizeof(rproc_msg_len_t));
    if (ret != 0) {
        dms_warn("rproc_xfer_async not success, ret = %d.\n", ret);
        return;
    }
    dms_info("send power message to lp, (cmd_type=%u).\n", cmd_type);
    return;
}

STATIC void dms_pm_notify_sils(void)
{
    int ret;
    struct drv_safety_island_ipc_info ipc_info = {0};

    dms_info("send power msg to safetyisland.\n");

    ipc_info.cmd_type = SILS_IPC_MSGTYPE_HAERTBEAT;
    ipc_info.cmd = SILS_IPC_CMDTYPE_OFF;
    ipc_info.cmd_dest = SAFETY_ISLAND;
    ipc_info.cmd_src = TAISHAN1_SUBSYS_AOS;
    ipc_info.seq++;
    ipc_info.data_type = IPC_SAFETY_ISLAND_REQUEST;
    ipc_info.len = IPC_GET_SAFETYISLAND_INFO_FUN_TYPE_LEN;
    ipc_info.payload[0] = SILS_IPC_DATA;
    ipc_info.crc = sils_crc16((unsigned char *)&ipc_info, ipc_info.len + SILS_IPC_HEAD_LEN);

    ret = icm_msg_send_async(ICM_FD_BUILD(0, HISI_RPROC_SI_Q_TX_RPID17), (rproc_msg_t *)&ipc_info,
        sizeof(struct drv_safety_island_ipc_info) / sizeof(rproc_msg_t));
    if (ret != 0) {
        dms_err("send power msg to safetyisland failed, ret = %d\n", ret);
    }
    /* stop taishan sils heart */
    ret = devdrv_manager_suspend_proc(0, DEVDRV_SUSPEND);
    if (ret != 0) {
        devdrv_drv_err("close taishan sils heart failed, ret = %d\n", ret);
    }
    return;
}


#define SLEEP_POWER_DELAY_MS 2U
int dms_pm_suspend(u32 dev_id)
{
    int ret = 0;

    dms_pm_notify_sils();
    mdelay(SLEEP_POWER_DELAY_MS);

    dms_pm_notify_lp(dev_id, SUB_CMD_NOTIFY_LP_SUSPEND);

    dms_info("dms start call pm_suspend.\n");
    ret = pm_suspend(PM_SUSPEND_MEM);
    if (ret != 0) {
        dms_err("invokr os interface pm_suspend failed, ret = %d\n", ret);
        return ret;
    }

    dms_pm_notify_lp(dev_id, SUB_CMD_NOTIFY_LP_RESUME);

    return ret;
}

s32 dms_set_power_state(void *feature, char *in,
    u32 in_len, char *out, u32 out_len)
{
    if ((in == NULL) || (in_len != sizeof(struct dms_power_state_st))) {
        dms_err("Invalid parameter. (in_buff=%s; in_len=%u)\n", (in == NULL) ? "NULL" : "OK", in_len);
        return DRV_ERROR_PARA_ERROR;
    }
    struct dms_power_state_st *cfg = (struct dms_power_state_st *)in;
    if (cfg == NULL) {
        dms_err("Invalid para.\n");
        return -EINVAL;
    }
    if (cfg->dev_id >= DEVICE_NUM_MAX) {
        dms_err("Invalid device id. (dev_id=%u)\n", cfg->dev_id);
        return -EINVAL;
    }

    switch ((DSMI_POWER_STATE)cfg->state) {
        case POWER_STATE_SUSPEND:
            return dms_pm_suspend(cfg->dev_id);
        case POWER_STATE_POWEROFF:
        case POWER_STATE_RESET:
        case POWER_STATE_BIST:
            dms_err("Not support! (type=%d)\n", cfg->state);
            return -EOPNOTSUPP;
        default:
            dms_err("Invalid type! (type=%d)\n", cfg->state);
            return -EINVAL;
    }
}
#else
s32 dms_set_power_state(void *feature, char *in,
    u32 in_len, char *out, u32 out_len)
{
    return -EPROTONOSUPPORT;
}
#endif
