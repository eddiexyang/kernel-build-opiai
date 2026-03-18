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

s32 tsdrv_sync_ssid_proc(u32 devid, struct tsdrv_msg_info *msg, u32 *ack_len)
{
    struct tsdrv_msg_sync_ssid *payload = NULL;
    int pid, ssid;
#ifndef AOS_LLVM_BUILD
    int ret;
#endif
    if (msg->header.valid != TSDRV_MSG_SYNC_MAGIC) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("devid %u invalid message from opposite.\n", devid);
        return -EINVAL;
#endif
    }

    payload = (struct tsdrv_msg_sync_ssid *)msg->payload;
    
#ifndef AOS_LLVM_BUILD
    ret = devdrv_query_process_by_host_pid(payload->hpid, devid, DEVDRV_PROCESS_CP1, payload->vfid, &pid);
    if (unlikely(ret)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("query pid by host pid %u error vfid %u ret:%d\n", payload->hpid, payload->vfid, ret);
        return -EINVAL;
#endif
    }
    ssid = svm_get_pasid(pid, devid);
#else
    pid = current->tgid;
    ssid = AOS_SVAGetCurrentPasid();
#endif
    if (unlikely(ssid < 0)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("pid %d get ssid failed, ssid:%d\n", pid, ssid);
        return -EINVAL;
#endif
    }
    msg->header.result = 0;
    msg->header.valid = TSDRV_MSG_SYNC_RSP_MAGIC;
    payload->ssid = ssid;
    *ack_len = sizeof(struct tsdrv_msg_info);

    TSDRV_PRINT_DEBUG("(devid=%u; vfid=%u; hostpid=%d; ssid=%d)\n", devid, payload->vfid, pid, ssid);
    return 0;
}

int tsdrv_msg_sync_ssid(u32 dev_id, u32 tsid, u32 hpid, u32 vfid)
{
    int ssid;
    pid_t vnr;

#ifndef AOS_LLVM_BUILD
#ifndef TSDRV_UT
    vnr = task_tgid_vnr(current);
    ssid = svm_get_pasid(vnr, dev_id);
#endif
#else
    vnr = current->tgid;
    ssid = AOS_SVAGetCurrentPasid();
#endif
#ifndef TSDRV_UT
    if (unlikely(ssid < 0)) {
        TSDRV_PRINT_ERR("devid %u tsid %u pid %u vnr %d get ssid failed, ssid %d\n", dev_id, tsid, hpid, vnr, ssid);
        return TSDRV_INVALID_SSID;
    }

    TSDRV_PRINT_DEBUG("devid %u tsid %u pid %u vnr %d ssid %d\n", dev_id, tsid, hpid, vnr, ssid);
    return ssid;
#endif
}

