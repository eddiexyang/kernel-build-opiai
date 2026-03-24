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
#ifdef AOS_LLVM_BUILD
#include <linux/iommu.h>
#endif
#include "tsdrv_get_ssid.h"
#include "tsdrv_sync.h"
#include "devdrv_manager_common.h"
#include "devdrv_mailbox.h"
#include "devdrv_devinit.h"
#include "tsdrv_common.h"
#include "tsdrv_log.h"
#include "tsdrv_interface.h"
#include "tsdrv_device.h"

#ifdef CFG_SOC_PLATFORM_CLOUD_V2
struct devdrv_notice_ssid_mailbox {
    struct devdrv_mailbox_message_header head;
    u8 vfid;
    u8 reserved;
    u16 ssid;
    u32 hostpid;
};

static void tsdrv_pack_ssid_msg(u8 vfid, u16 ssid, u32 hostpid, struct devdrv_notice_ssid_mailbox *msg)
{
    msg->head.valid = DEVDRV_MAILBOX_MESSAGE_VALID;
    msg->head.cmd_type = TSDRV_MBOX_NOTICE_SSID;
    msg->head.result = 0;
    msg->vfid = vfid;
    msg->hostpid = hostpid;
    msg->ssid = ssid;
}

static int tsdrv_ssid_ipc_msg_send(u32 devid, u32 hostpid, int ssid)
{
    struct devdrv_notice_ssid_mailbox msg = {0};
    struct tsdrv_ts_resource *ts_res = NULL;
    u32 phy_devid, fid;
    int ret, result;

    if (tsdrv_get_pfvf_type_by_devid(devid) == DEVDRV_SRIOV_TYPE_PF) {
        return 0;
    }
    ret = devdrv_get_pfvf_id_by_devid(devid, &phy_devid, &fid);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to get physic devid and fid. (devid=%u)\n", devid);
        return ret;
    }

    ts_res = tsdrv_get_ts_resoruce(phy_devid, TSDRV_PM_FID, 0);
    tsdrv_pack_ssid_msg((u8)fid, (u16)ssid, hostpid, &msg);
    ret = devdrv_mailbox_kernel_sync_no_feedback(&ts_res->mailbox, (u8 *)&msg,
        sizeof(struct devdrv_notice_ssid_mailbox), &result);
    if (ret == 0) {
        ret = result;
    }
    return ret;
}
#endif

s32 tsdrv_sync_ssid_proc(u32 devid, struct tsdrv_msg_info *msg, u32 *ack_len)
{
    struct tsdrv_msg_sync_ssid *payload = NULL;
    int pid, ssid;
#ifndef AOS_LLVM_BUILD
    int ret;
#endif
    if (msg->header.valid != TSDRV_MSG_SYNC_MAGIC) {
        TSDRV_PRINT_ERR("devid %u invalid message from opposite.\n", devid);
        return -EINVAL;
    }

    payload = (struct tsdrv_msg_sync_ssid *)msg->payload;
    
#ifndef AOS_LLVM_BUILD
    ret = devdrv_query_process_by_host_pid(payload->hpid, devid, DEVDRV_PROCESS_CP1, payload->vfid, &pid);
    if (unlikely(ret)) {
        TSDRV_PRINT_ERR("query pid by host pid %u error vfid %u ret:%d\n", payload->hpid, payload->vfid, ret);
        return -EINVAL;
    }
    ssid = svm_get_pasid(pid, devid);
#else
    pid = current->tgid;
    ssid = AOS_SVAGetCurrentPasid();
#endif
    if (unlikely(ssid < 0)) {
        TSDRV_PRINT_ERR("pid %d get ssid failed, ssid:%d\n", pid, ssid);
        return -EINVAL;
    }
    msg->header.result = 0;
    msg->header.valid = TSDRV_MSG_SYNC_RSP_MAGIC;
    payload->ssid = ssid;
    *ack_len = sizeof(struct tsdrv_msg_info);

#ifdef CFG_SOC_PLATFORM_CLOUD_V2
    ret = tsdrv_ssid_ipc_msg_send(devid, payload->hpid, payload->ssid);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to send ssid to ts. (devid=%u; ret=%d)\n", devid, ret);
        return ret;
    }
#endif

    TSDRV_PRINT_DEBUG("(devid=%u; vfid=%u; hostpid=%d; ssid=%d)\n", devid, payload->vfid, pid, ssid);
    return 0;
}

int tsdrv_msg_sync_ssid(u32 dev_id, u32 tsid, u32 hpid, u32 vfid)
{
    int ssid;
    pid_t vnr;

#ifndef AOS_LLVM_BUILD
    vnr = task_tgid_vnr(current);
    ssid = svm_get_pasid(vnr, dev_id);
#else
    vnr = current->tgid;
    ssid = AOS_SVAGetCurrentPasid();
#endif
    if (unlikely(ssid < 0)) {
        TSDRV_PRINT_ERR("devid %u tsid %u pid %u vnr %d get ssid failed, ssid %d\n", dev_id, tsid, hpid, vnr, ssid);
        return TSDRV_INVALID_SSID;
    }

    TSDRV_PRINT_DEBUG("devid %u tsid %u pid %u vnr %d ssid %d\n", dev_id, tsid, hpid, vnr, ssid);
    return ssid;
}

