/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
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
#include <linux/mm.h>
#include <linux/sizes.h>
#include <securec.h>
#include "devdrv_config_p2p.h"
#include "devdrv_common.h"
#include "devdrv_manager_common.h"
#include "devdrv_interface.h"
#include "tsdrv_device.h"

#define SCHED_WORK_PERIOD 10
#define DEVID_CONFIGED      1
#define DEVID_NOT_CONFIG    0
#define TSDRV_HOST_MAX_DAVINCI_NUM 64
#define TASKID_SHARE_MEM_BLOCK_ORDER get_order(SZ_2M)

STATIC struct delayed_work ts_work;
static bool dwork_int = false;

STATIC int devdrv_send_p2p_addr2ts(u32 local_devid, u32 notify_type, u32 devid, u32 status)
{
#ifndef TSDRV_UT
    struct tsdrv_ts_resource *ts_resource = NULL;
    struct p2p_addr_info notify_info;
    int result = 0;
    u8 judge_flag;
    u32 tsid = 0;
    int flag = 0;
    int hostid;
    int ret;

    ts_resource = tsdrv_get_ts_resoruce(local_devid, TSDRV_PM_FID, tsid);
    hostid = agentdrv_get_host_devid(local_devid);
    if (hostid < 0) {
        TSDRV_PRINT_ERR("get hostid failed. devid = %u.\n", local_devid);
        return -EINVAL;
    }

    ret = memset_s(&notify_info, sizeof(struct p2p_addr_info), 0, sizeof(struct p2p_addr_info));
    if (ret != 0) {
        TSDRV_PRINT_ERR("set notify_info to 0 failed, ret = %d.\n", ret);
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

        judge_flag = ((notify_type == DEVDRV_HOST_CFG_NOTIFY) &&
                      (!agentdrv_get_p2p_addr(local_devid, devid, DEVDRV_P2P_HOST_MEM,
                                              &notify_info.data.host_cfg_notify_data.host_mem_addr,
                                              &notify_info.data.host_cfg_notify_data.host_mem_size)));
        if (judge_flag) {
            flag = 1;
        }
    } else {
        flag = 1;
    }

    if (!flag) {
        TSDRV_PRINT_INFO("no valid addr(local_device_%u, dest_device_%u, status_%u, notify_type_%u).\n", local_devid,
            devid, status, notify_type);
        return -EINVAL;
    }

    notify_info.header.valid = DEVDRV_MAILBOX_MESSAGE_VALID;
    notify_info.header.cmd_type = DEVDRV_MAILBOX_CONFIG_P2P_INFO;
    notify_info.header.result = 0;
    notify_info.remote_devid = (u8)devid;
    notify_info.local_devid = (u8)hostid;
    notify_info.status = (u8)status;
    notify_info.notify_type = (u8)notify_type;

    ret = devdrv_mailbox_kernel_sync_no_feedback(&ts_resource->mailbox, (u8 *)&notify_info,
        sizeof(struct p2p_addr_info), &result);
    if ((ret != 0) || (result != 0)) {
        TSDRV_PRINT_ERR("send addr failed(%d),dev_%u to dev_%u,\n", ret, devid,
                        local_devid);
        return -EINVAL;
    }
    return 0;
#endif
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
    if ((num_dev == 0) || (num_dev > TSDRV_MAX_DAVINCI_NUM)) {
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
            TSDRV_PRINT_INFO("devid=%u is not ready yet, status=%d\n", i, (int)status);
            (void)schedule_delayed_work(&ts_work, msecs_to_jiffies(CUR_MSECS_TO_JIFFIES));
            return;
        }
        configed_devid[i] = DEVID_CONFIGED;
        configed_dev_num++;

        ret = devdrv_send_hwts_addr_to_ts(i, 0, i, hostid);
        if (ret != 0) {
            TSDRV_PRINT_ERR("devdrv_send_hwts_addr_to_ts failed, ret(%d).\n", ret);
        }
        TSDRV_PRINT_INFO("p2p config: hostid(%u) devid(%u).\n", hostid, i);
    }
    if (configed_dev_num < num_dev) {
        (void)schedule_delayed_work(&ts_work, msecs_to_jiffies(SCHED_WORK_PERIOD));
    }
}

void devdrv_config_p2p(void)
{
    int ret;

    ret = agentdrv_register_dev_online_proc_func(devdrv_send_p2p_addr2ts);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("agentdrv_register_dev_online_proc_func failed, ret(%d).\n", ret);
        return;
#endif
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
#ifdef CFG_SOC_PLATFORM_MINIV3
    struct taskid_share_mem *idx_addr = NULL;
    struct page *hpage = NULL;
    u32 shm_id, nid;
    u32 i;
    u64 shm_addr;

    shm_addr = TASKID_SHARE_MEM_START_ADDR;
    idx_addr = ioremap(shm_addr, TASKID_SHARE_MEM_SIZE);
    if (idx_addr == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("device(%u) index mem remap fail.\n", devid);
        return;
#endif
    }

#ifndef TSDRV_UT
    memset_io(idx_addr, 0, sizeof(struct taskid_share_mem));
    /* 15 * 2M memory will not free when devdrv_drv_unregister
     * because the memory may still be handled by ts or devmm.
     */
    nid = (u32)devdrv_manager_devid_to_nid(devid, (u32)DEVDRV_TS_NODE_DDR_MEM);
    for (shm_id = 0; shm_id < TASKID_SHARE_MEM_BLOCK_NUM; shm_id++) {
        hpage = alloc_pages_node(nid, GFP_KERNEL | __GFP_ZERO | __GFP_NOWARN | __GFP_COMP,
            TASKID_SHARE_MEM_BLOCK_ORDER);
        if (hpage == NULL) {
            TSDRV_PRINT_ERR("device(%u) alloc shm(%u) on node (%u) failed, cannot use addr translation fuc.\n",
                devid, shm_id, nid);
            break;
        }
        idx_addr->addr[shm_id] = page_to_phys(hpage);
    }
    if (shm_id != TASKID_SHARE_MEM_BLOCK_NUM) {
        for (i = 0; i < shm_id; i++) {
            __free_pages(phys_to_page(idx_addr->addr[i]), TASKID_SHARE_MEM_BLOCK_ORDER);
        }
        iounmap(idx_addr);
        idx_addr = NULL;
        return;
    }
    idx_addr->magic = TASKID_SHARE_MEM_MAGIC;
    idx_addr->count = TASKID_SHARE_MEM_BLOCK_NUM;
    iounmap(idx_addr);
    idx_addr = NULL;
#endif /* TSDRV_UT */
#else
    return;
#endif
}
#endif
