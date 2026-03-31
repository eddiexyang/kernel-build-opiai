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
#ifdef CFG_FEATURE_SURPORT_P2P
#include <linux/workqueue.h>
#include <securec.h>

#include "devdrv_config_p2p.h"
#include "devdrv_common.h"
#include "devdrv_manager_common.h"
#include "devdrv_interface.h"
#include "tsdrv_device.h"

#define SCHED_WORK_PERIOD 10
#define DEVID_CONFIGED 1
#define DEVID_NOT_CONFIG 0
#define TSDRV_HOST_MAX_DAVINCI_NUM 64
STATIC struct delayed_work ts_work;
static bool dwork_int = false;

STATIC int devdrv_send_p2p_addr2ts(u32 local_devid, u32 notify_type, u32 devid, u32 status)
{
    struct tsdrv_ts_resource *ts_resource = NULL;
    struct devdrv_info *dev_info = NULL;
    struct p2p_addr_info notify_info;
    int result = 0;
    u8 judge_flag;
    int flag = 0;
    int hostid;
    int ret;

    dev_info = devdrv_manager_get_devdrv_info(devid);
    if (dev_info == NULL) {
        devdrv_drv_err("dev_info is null or ts is not working.\n");
        return -EINVAL;
    }

    hostid = agentdrv_get_host_devid(local_devid);
    if (hostid < 0) {
#ifndef TSDRV_UT
        devdrv_drv_err("get hostid failed. devid = %u.\n", local_devid);
#endif
        return -EINVAL;
    }

    ret = memset_s(&notify_info, sizeof(struct p2p_addr_info), 0, sizeof(struct p2p_addr_info));
    if (ret != 0) {
        devdrv_drv_err("set notify_info to 0 failed, ret = %d.\n", ret);
        return -EINVAL;
    }

    if (status == DEVDRV_DEV_ONLINE) {
        if (notify_type == DEVDRV_DEV_ONLINE_NOTIFY) {
            if (!agentdrv_get_p2p_addr(local_devid, devid, DEVDRV_P2P_IO_TS_DB,
                &notify_info.data.dev_online_notify_data.ts_db_addr,
                &notify_info.data.dev_online_notify_data.ts_db_size)) {
                flag = 1;
            }
            if (!agentdrv_get_p2p_addr(local_devid, devid, DEVDRV_P2P_IO_TS_SRAM,
                &notify_info.data.dev_online_notify_data.ts_sram_addr,
                &notify_info.data.dev_online_notify_data.ts_sram_size)) {
                flag = 1;
            }
            if (!agentdrv_get_p2p_addr(local_devid, devid, DEVDRV_P2P_IO_HWTS,
                &notify_info.data.dev_online_notify_data.hwts_addr,
                &notify_info.data.dev_online_notify_data.hwts_size)) {
                flag = 1;
            }
        }

        judge_flag = ((notify_type == DEVDRV_HOST_CFG_NOTIFY) && (!agentdrv_get_p2p_addr(local_devid, devid,
            DEVDRV_P2P_HOST_MEM, &notify_info.data.host_cfg_notify_data.host_mem_addr,
            &notify_info.data.host_cfg_notify_data.host_mem_size)));
        if (judge_flag) {
            flag = 1;
        }
    } else {
        flag = 1;
    }

    if (!flag) {
#ifndef TSDRV_UT
        devdrv_drv_info("dev_id(%u):no valid addr(local_device_%u, dest_device_%u, status_%u, notify_type_%u).\n",
            dev_info->dev_id, local_devid, devid, status, notify_type);
#endif
        return -EINVAL;
    }

    notify_info.header.valid = DEVDRV_MAILBOX_MESSAGE_VALID;
    notify_info.header.cmd_type = DEVDRV_MAILBOX_CONFIG_P2P_INFO;
    notify_info.header.result = 0;
    notify_info.remote_devid = (u8)devid;
    notify_info.local_devid = (u8)hostid;
    notify_info.status = (u8)status;
    notify_info.notify_type = (u8)notify_type;

    ts_resource = tsdrv_get_ts_resoruce(devid, TSDRV_PM_FID, 0);
    ret = devdrv_mailbox_kernel_sync_no_feedback(&ts_resource->mailbox, (u8 *)&notify_info,
        sizeof(struct p2p_addr_info), &result);
    if ((ret != 0) || (result != 0)) {
        devdrv_drv_err("dev_id(%u):send addr failed(%d),dev_%u to dev_%u,\n", dev_info->dev_id, ret, devid,
            local_devid);
        return -EINVAL;
    }
    return 0;
}

STATIC void devdrv_configts_p2p_addr(struct work_struct *data)
{
#define CUR_MSECS_TO_JIFFIES 1000

    static u32 configed_devid[TSDRV_MAX_DAVINCI_NUM] = {DEVID_NOT_CONFIG};
    static u32 configed_dev_num = 0;
    enum tsdrv_dev_status status;
    u32 num_dev;
    u32 hostid;
    u32 flag;
    int ret;
    u32 i;

    num_dev = (u32)devdrv_get_dev_num();
    if (num_dev == 0 || num_dev > TSDRV_MAX_DAVINCI_NUM) {
        devdrv_drv_err("get devnum fail, devnum=%u\n", num_dev);
        return;
    }

    for (i = 0; i < num_dev; i++) {
        if (configed_devid[i] == DEVID_CONFIGED) {
            continue;
        }

        flag = devdrv_manager_get_devid_flag(i);
        if (flag == 0) {
            continue;
        }

        hostid = devdrv_manager_get_devid(i);
        if (hostid == TSDRV_HOST_MAX_DAVINCI_NUM) {
            continue;
        }

        status = tsdrv_get_dev_status(i, TSDRV_PM_FID);
        if (status != TSDRV_DEV_ACTIVE) {
            devdrv_drv_info("devid=%u is not ready yet, status=%d\n", i, (int)status);
            (void)schedule_delayed_work(&ts_work, msecs_to_jiffies(CUR_MSECS_TO_JIFFIES));
            return;
        }

        ret = devdrv_send_hwts_addr_to_ts(i, 0, 0, hostid);
        if (ret != 0) {
            devdrv_drv_err("devdrv_send_hwts_addr_to_ts failed, ret(%d).\n", ret);
        }
        devdrv_drv_info("p2p config: hostid(%u) devid(%u).\n", hostid, i);

        configed_devid[i] = DEVID_CONFIGED;
        configed_dev_num++;
    }
    if (configed_dev_num < num_dev) {
        (void)schedule_delayed_work(&ts_work, msecs_to_jiffies(SCHED_WORK_PERIOD));
    }
}

void devdrv_config_p2p(void)
{
    int ret;
    u32 val = 0;

    ret = agentdrv_get_rc_ep_mode(&val);
    if (ret != 0) {
        devdrv_drv_err("get rc or ep mode fail\n");
        return;
    }
    if (val == DEVDRV_PCIE_RC_MODE) {
        return;
    }

    ret = agentdrv_register_dev_online_proc_func(devdrv_send_p2p_addr2ts);
    if (ret != 0) {
        devdrv_drv_err("agentdrv_register_dev_online_proc_func failed, ret(%d).\n", ret);
        return;
    }
    INIT_DELAYED_WORK(&ts_work, devdrv_configts_p2p_addr);
    (void)schedule_delayed_work(&ts_work, msecs_to_jiffies(SCHED_WORK_PERIOD));
    dwork_int = true;
}

void devdrv_deconfig_p2p(void)
{
    if (dwork_int == true) {
        (void)cancel_delayed_work_sync(&ts_work);
        dwork_int = false;
    }
}

void tsdrv_init_taskid_share_memory(u32 devid, u32 tsnum, u32 chipid, u32 dieid)
{
    return;
}
#endif /* CFG_FEATURE_SURPORT_P2P */
