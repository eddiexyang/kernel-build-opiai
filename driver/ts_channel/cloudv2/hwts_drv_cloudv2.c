/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2023. All rights reserved.
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

#ifndef TSDRV_UT
#include <linux/platform_device.h>
#include <linux/of_address.h>

#include "devdrv_user_common.h"
#include "hwts_drv_config.h"
#include "ts_drv_init.h"
#include "task_struct.h"
#include "ts_aisle_api.h"
#include "stars_event_table_ns_reg_offset.h"
#include "stars_notify_tbl_reg_offset.h"
#include "cp_identity_interface.h"
#include "devdrv_manager_comm.h"
#include "devdrv_interface.h"

/**
 * @ingroup tsch
 * @brief event num of every event table
 */
#define STARS_EVENT_NUM_OF_SINGLE_TABLE    4096

/**
 * @ingroup tsch
 * @brief notify num of every notify table
 */
#define STARS_NOTIFY_NUM_OF_SINGLE_TABLE    512

#define REG_STARS_EVENT_TABLE_OFFSET    0x10000
#define REG_STARS_EVENT_ADDR_OFFSET     0x4

/**
 * @ingroup tsch
 * @brief stars surport 65536 events
 */
#define TS_EVENTS_COUNT     65536

/**
 * @ingroup tsch
 * @brief task scheduler only surport 8192 notifies
 */
#define NOTIFY_ID_NUM_MAX   8192

static u64 g_hwts_base_addr[MAX_TS_NUM] = {0x6a0000000ULL}; /* size 512M */
static u64 g_hwts_reg_size[MAX_TS_NUM] = {0x20000000};

static void __iomem *g_hwts_base_va[CHIP_NUM_MAX][MAX_TS_NUM] = {NULL};
void __iomem *get_hwts_base_va(u32 node_id, u32 ts_id)
{
    if (unlikely(node_id >= CHIP_NUM_MAX)) {
        ts_drv_err("invalid node_id:%u\n", node_id);
        return NULL;
    }

    if (unlikely(ts_id >= MAX_TS_NUM)) {
        ts_drv_err("invalid ts id:%u\n", ts_id);
        return NULL;
    }

    return g_hwts_base_va[node_id][ts_id];
}

static u64 tsdrv_get_addr_chip_die_offset(u32 devid, u32 chipid, u32 dieid)
{
    u64 offset;

    if (devdrv_get_connect_protocol(devid) == CONNECT_PROTOCOL_HCCS) {
        offset = chipid * HCCS_CHIP_BASEADDR_PA_OFFSET + dieid * DIE_BASEADDR_PA_OFFSET + HCCS_BASEADDR_PA_OFFSET;
    } else {
        offset = chipid * CHIP_BASEADDR_PA_OFFSET + dieid * DIE_BASEADDR_PA_OFFSET;
    }
    return offset;
}

int hwts_drv_base_addr_remap(struct drv_hwts_ctrl *hwts)
{
    u32 node_id = (u32)hwts->numa_node;
    u32 chipid, dieid;
    u32 i, j;
    int ret;

    ret =  devdrv_get_chip_die_id(node_id, &chipid, &dieid);
    if (ret != 0) {
        ts_drv_err("Get chipid failed. (nodeid=%u)\n", node_id);
        return ret;
    }
    ts_drv_info("Get chipid info. (nodeid=%u, chipid=%u, dieid=%u)\n", node_id, chipid, dieid);

    for (i = 0; i < MAX_TS_NUM; i++) {
        g_hwts_base_va[node_id][i] = ioremap(g_hwts_base_addr[i] +
            tsdrv_get_addr_chip_die_offset(node_id, chipid, dieid), g_hwts_reg_size[i]);
        if (g_hwts_base_va[node_id][i] == NULL) {
            ts_drv_err("ioremap failed. nodeid(%u), tsid(%u)\n", node_id, i);
            goto ERROR;
        }
    }
    return 0;

ERROR:
    for (j = 0; j < i; j++) {
        iounmap(g_hwts_base_va[node_id][j]);
    }
    return -EINVAL;
}


void hwts_drv_release(struct platform_device *pdev, struct drv_hwts_ctrl *hwts)
{
    int i;

    for (i = 0; i < MAX_TS_NUM; i++) {
        if (g_hwts_base_va[hwts->numa_node][i] != NULL) {
            iounmap(g_hwts_base_va[hwts->numa_node][i]);
            g_hwts_base_va[hwts->numa_node][i] = NULL;
        }
    }
    return;
}

int hwts_drv_init(struct platform_device *pdev, struct drv_hwts_ctrl *hwts, int node_id)
{
    int ret;

    ret = hwts_drv_base_addr_remap(hwts);
    if (ret != 0) {
        ts_drv_err("failed to config mailbox addr.\n");
        return -EINVAL;
    }

    return ret;
}

int send_msg_to_ts_async(int node_id, int ts_id, unsigned int len, const void *msg)
{
    return 0;
}

int ts_write_record_register(u32 dev_id, u32 ts_id, u32 record_type, u32 record_id)
{
    void __iomem *hwts_base_va = NULL;
    u64 hwts_offset, base_addr;
    u32 table_id, k, pfid, vfid;
    pid_t pid = current->tgid;
    int ret;

    ret = devdrv_get_pfvf_id_by_devid(dev_id, &pfid, &vfid);
    if (ret != 0) {
        ts_drv_err("Failed to get pfid and vfid. (devid=%u)\n", dev_id);
        return ret;
    }

    ts_drv_debug("devid %u, pfid %u, vfid %d, tsid %u, type %u, record id %u.\n", dev_id, pfid, vfid,
        ts_id, record_type, record_id);

    hwts_base_va = get_hwts_base_va(pfid, ts_id);
    if (unlikely(hwts_base_va == NULL)) {
        ts_drv_err("get HWTS base virtual address error, devid %u, ts id %u\n", pfid, ts_id);
        return -EINVAL;
    }

    if (record_type == AICPU_MSG_NOTIFY_RECORD) {
        if ((record_id >= NOTIFY_ID_NUM_MAX)) {
            ts_drv_err("devid %u, tsid %u, record id %u invalid.\n", dev_id, ts_id, record_id);
            return -EINVAL;
        }
        base_addr = (u64)SOC_STARS_NOTIFY_TBL_BASE;
        table_id = record_id / STARS_NOTIFY_NUM_OF_SINGLE_TABLE;
        k = record_id & (STARS_NOTIFY_NUM_OF_SINGLE_TABLE - 1);
    } else {
        if ((record_id >= TS_EVENTS_COUNT)) {
            ts_drv_err("devid %u, tsid %u, record id %u invalid.\n", dev_id, ts_id, record_id);
            return -EINVAL;
        }
        base_addr = (u64)SOC_STARS_EVENT_TABLE_NS_BASE;
        table_id = record_id / STARS_EVENT_NUM_OF_SINGLE_TABLE;
        k = record_id & (STARS_EVENT_NUM_OF_SINGLE_TABLE - 1);
    }
    ts_drv_debug("base_addr=0x%pK table_id=%u k=%u\n", (void *)(uintptr_t)base_addr, table_id, k);

    ret = devdrv_get_dev_process(pid);
    if (ret != 0) {
        ts_drv_err("Can not get process. (devid=%u; tsid=%u; ret=%d).\n", dev_id, ts_id, ret);
        return -EINVAL;
    }

    hwts_offset = (u64)(base_addr + (u64)((u64)table_id * REG_STARS_EVENT_TABLE_OFFSET +
                        (u64)k * REG_STARS_EVENT_ADDR_OFFSET));
    if (readl_relaxed((void *)(hwts_base_va + hwts_offset)) != HWTS_RECORD_WAIT_FLAG) {
        writel_relaxed(HWTS_RECORD_WAIT_FLAG, (void *)(hwts_base_va + hwts_offset));
        ts_drv_debug("devid %u, tsid %u, record id %u\n", dev_id, ts_id, record_id);
    }
    devdrv_put_dev_process(pid);

    return 0;
}
#else /* TSDRV_UT */
void ut_device_hwts_drv_test(void)
{
}
#endif /* TSDRV_UT */
