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

#include <linux/slab.h>
#include <linux/vmalloc.h>
#ifndef AOS_LLVM_BUILD
#include <linux/debugfs.h>
#endif
#include <securec.h>

#include "devdrv_common.h"
#include "dbl/chip_config.h"
#include "tsdrv_log.h"
#include "devdrv_manager.h"
#include "tsdrv_ts_alloc_mem.h"
#include "icm_interface.h"
#include "tsdrv_ipc.h"
#include "tsdrv_pdata.h"
#include "tsmng_interface.h"

#ifndef page_to_virt
#define page_to_virt(page) __va(page_to_pfn(page) << PAGE_SHIFT)
#endif

struct tsdrv_ts_mem_node {
    u32 sn;
    u32 size;
    u64 vaddr;
    struct list_head list;
};

struct tsdrv_ts_mem_mgmt {
    u64 alloced_mem_size;
    struct list_head alloced_mem_node_head;
};

static struct tsdrv_ts_mem_mgmt g_mem_mgnt[DEVDRV_MAX_DAVINCI_NUM][DEVDRV_MAX_TS_NUM];

static struct tsdrv_ts_mem_mgmt *tsdrv_get_ts_mem_mgmt(u32 devid, u32 tsid)
{
    return &g_mem_mgnt[devid][tsid];
}

static void tsdrv_pack_alloc_resp_msg(struct tsdrv_mem_alloc_req_msg *req_msg,
    struct tsdrv_mem_alloc_resp_msg *resp_msg, u64 vaddr, u64 paddr)
{
    resp_msg->header.msg_type = MSGTYPE_DRIVER_SEND;
    resp_msg->header.cmd_type = IPCDRV_TS_MEM_OP;
    resp_msg->header.sync_type = IPCDRV_MSG_ASYNC;
    resp_msg->header.reserved = 0;
    resp_msg->header.msg_length = TS_MEM_ALLOC_RESP_MSG_LEN;
    resp_msg->header.msg_index = 0;
    resp_msg->sub_cmd_type = TS_MEM_ALLOC_RESP_CMD;
    resp_msg->sn = req_msg->sn;
    resp_msg->flag = req_msg->flag;
    resp_msg->vaddr = vaddr;
    resp_msg->paddr = paddr;
}

static int tsdrv_init_isolation_mem(u64 vaddr, u32 req_size, u32 isolation_size)
{
    void *post_isolation_vaddr = (void *)(uintptr_t)(vaddr + TS_MEM_ISOLATION_SIZE + req_size);
    void *pre_isolation_vaddr = (void *)(uintptr_t)vaddr;
    int ret;

    ret = memset_s(pre_isolation_vaddr, TS_MEM_ISOLATION_SIZE, TS_MEM_ISOLATION_MAGIC, isolation_size);
    if (ret != 0) {
        return ret;
    }

    ret = memset_s(post_isolation_vaddr, TS_MEM_ISOLATION_SIZE, TS_MEM_ISOLATION_MAGIC, isolation_size);

    return ret;
}

static int tsdrv_create_insert_mem_node(struct tsdrv_mem_alloc_req_msg *req_msg,
    struct tsdrv_ts_mem_mgmt *mem_mgmt, u64 vaddr)
{
    struct tsdrv_ts_mem_node *mem_node = NULL;

    mem_node = kmalloc(sizeof(struct tsdrv_ts_mem_node), GFP_ATOMIC);
    if (mem_node == NULL) {
        TSDRV_PRINT_ERR("Alloc mem node failed.\n");
        return -EINVAL;
    }

    mem_node->size = req_msg->size;
    mem_node->sn = req_msg->sn;
    mem_node->vaddr = vaddr;
    list_add(&mem_node->list, &mem_mgmt->alloced_mem_node_head);
    return 0;
}

static u64 tsdrv_alloc_mem_for_ts(u32 devid, u32 size, u32 *node)
{
    struct page *page = NULL;

#ifdef CFG_SOC_PLATFORM_HELPER
#define MEM_NODE2 2U
#define MEM_NODE3 3U
    *node = MEM_NODE2;
    if (devid == 1U) {
        *node = MEM_NODE3;
    }

    TSDRV_PRINT_INFO("Alloc 4g memory for helper. (size=%u; node=%u)\n", size, *node);
#else
    *node = (u32)dbl_get_ts_default_nid(devid);
    TSDRV_PRINT_INFO("Alloc memory for ts. (size=%u; node=%u)\n", size, *node);
#endif
    page = alloc_pages_node(*node, GFP_ATOMIC | __GFP_THISNODE | GFP_HIGHUSER_MOVABLE, (u32)get_order(size));
    if (page == NULL) {
        TSDRV_PRINT_ERR("Alloc mem fail.\n");
        return 0;
    }
    return (uintptr_t)page_to_virt(page);
}

/* alloced mem distribution: |___isolation___|____request_____|___isolation___| */
static void tsdrv_ts_alloc_mem_proc(u32 devid, struct tsdrv_mem_alloc_req_msg *req_msg,
    struct tsdrv_ts_mem_mgmt *mem_mgmt, u64 *vaddr, u64 *paddr)
{
    u32 size = TS_MEM_ISOLATION_SIZE + req_msg->size + TS_MEM_ISOLATION_SIZE;
    u32 alloc_nid = devid;
    u32 actual_nid;
    int ret;

    TSDRV_PRINT_DEBUG("Alloc mem for ts. (devid=%u; size=%u)\n",
        devid, req_msg->size);
    *vaddr = tsdrv_alloc_mem_for_ts(devid, size, &alloc_nid);
    if (*vaddr == 0) {
        return;
    }

    ret = tsdrv_init_isolation_mem(*vaddr, req_msg->size, TS_MEM_ISOLATION_SIZE);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Init isolation mem failed. (devid=%u)\n", devid);
        goto failed;
    }

    *paddr = (uintptr_t)virt_to_phys((void *)(uintptr_t)*vaddr);
    actual_nid = page_to_nid(pfn_to_page(PFN_DOWN(*paddr)));
    if (alloc_nid != actual_nid) {
        TSDRV_PRINT_ERR("Phy addr is invalid. (devid=%u; alloc_nid=%u; actual_nid=%u)\n",
            devid, alloc_nid, actual_nid);
        goto failed;
    }

    ret = tsdrv_create_insert_mem_node(req_msg, mem_mgmt, *vaddr);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Create or insert mem node failed. (devid=%u)\n", devid);
        goto failed;
    }

    *paddr += TS_MEM_ISOLATION_SIZE;
    return;

failed:
    free_pages((unsigned long)*vaddr, (u32)get_order(size));
    *vaddr = 0;
    return;
}

static void tsdrv_ts_free_mem_proc(struct tsdrv_ts_mem_mgmt *mem_mgmt, u64 vaddr, unsigned long size)
{
    struct tsdrv_ts_mem_node *mem_node = NULL;
    struct list_head *pos = NULL, *n = NULL;

    if (list_empty_careful(&mem_mgmt->alloced_mem_node_head)) {
        return;
    }

    list_for_each_safe(pos, n, &mem_mgmt->alloced_mem_node_head) {
        mem_node = list_entry(pos, struct tsdrv_ts_mem_node, list);
        if (mem_node->vaddr == vaddr) {
            list_del(&mem_node->list);
            kfree((void *)mem_node);
            free_pages(vaddr, (u32)get_order(size));
            return;
        }
    }
}

static void tsdrv_ts_alloc_mem_request(struct notifier_block *nb, unsigned long len, void *data)
{
    struct tsdrv_mem_alloc_req_msg *req_msg = NULL;
    struct tsdrv_ts_mem_mgmt *mem_mgmt = NULL;
    struct tsdrv_ipc_chan *ipc_chan = NULL;
    rproc_msg_t resp_msg[IPCDRV_RPROC_MSG_LENGTH];
    u32 devid;
    u32 tsid;
    u64 vaddr = 0;
    u64 paddr = 0;
    int ret;

    ipc_chan = notifier_block_to_ipc_chan(nb);
    devid = ipc_chan->devid;
    tsid = ipc_chan->tsid;
    req_msg = (struct tsdrv_mem_alloc_req_msg *)data;
    if ((req_msg->sub_cmd_type != TS_MEM_ALLOC_REQ_CMD) ||
        (req_msg->size > TS_MEM_MAX_CONTINUOUS_SIZE)) {
        TSDRV_PRINT_ERR("Cmd or size is invalid. "
            "(devid=%u; tsid=%u; cmd_type=%u; req_size=%u; max_con_size=%d)\n",
            devid, tsid, req_msg->sub_cmd_type, req_msg->size, TS_MEM_MAX_CONTINUOUS_SIZE);
        goto mem_outof_range;
    }

    mem_mgmt = tsdrv_get_ts_mem_mgmt(devid, tsid);
    mem_mgmt->alloced_mem_size += req_msg->size;
    if (mem_mgmt->alloced_mem_size > TS_MEM_MAX_SIZE) {
        mem_mgmt->alloced_mem_size -= req_msg->size;
        TSDRV_PRINT_WARN("Alloc mem size will out of range. "
            "(devid=%u; tsid=%u; req_size=%u; alloced_size=%llu; max_size=0x%x)\n",
            devid, tsid, req_msg->size, mem_mgmt->alloced_mem_size, (u32)TS_MEM_MAX_SIZE);
        goto mem_outof_range;
    }

    tsdrv_ts_alloc_mem_proc(devid, req_msg, mem_mgmt, &vaddr, &paddr);
    if (vaddr == 0) {
        mem_mgmt->alloced_mem_size -= req_msg->size;
        TSDRV_PRINT_WARN("Alloc mem failed. (devid=%u; tsid=%u; req_size=%u)\n",
            devid, tsid, req_msg->size);
    }

mem_outof_range:
    tsdrv_pack_alloc_resp_msg(req_msg, (struct tsdrv_mem_alloc_resp_msg *)resp_msg, vaddr, paddr);
    ret = tsdrv_ipc_msg_send(ipc_chan, resp_msg);
    if (ret && vaddr) {
        tsdrv_ts_free_mem_proc(mem_mgmt, vaddr, TS_MEM_ISOLATION_SIZE + req_msg->size + TS_MEM_ISOLATION_SIZE);
        mem_mgmt->alloced_mem_size -= req_msg->size;
    }

    return;
}

static bool tsdrv_isolation_mem_is_polluted(u64 vaddr, u32 req_size)
{
    u8 *pre_isolation_vaddr = (u8 *)(uintptr_t)vaddr;
    u8 *post_isolation_vaddr = (u8 *)(uintptr_t)(vaddr + TS_MEM_ISOLATION_SIZE + req_size);
    u32 i;

    for (i = 0; i < TS_MEM_ISOLATION_SIZE; i++) {
        if ((*(pre_isolation_vaddr + i) != TS_MEM_ISOLATION_MAGIC) ||
            (*(post_isolation_vaddr + i) != TS_MEM_ISOLATION_MAGIC)) {
            TSDRV_PRINT_ERR("Isolation mem is polluted. (pre=%u; post=%u; magic=%d)\n",
                (u32)(*(pre_isolation_vaddr + i)), (u32)(*(post_isolation_vaddr + i)), TS_MEM_ISOLATION_MAGIC);
            return true;
        }
    }

    return false;
}

u32 tsdrv_get_isolation_mem_num(u32 devid, u32 tsid)
{
    struct tsdrv_ts_mem_mgmt *mem_mgmt = NULL;
    struct tsdrv_ts_mem_node *mem_node = NULL;
    struct list_head *pos = NULL, *n = NULL;
    u32 node_num = 0;

    mem_mgmt = tsdrv_get_ts_mem_mgmt(devid, tsid);
    if (list_empty_careful(&mem_mgmt->alloced_mem_node_head)) {
        TSDRV_PRINT_DEBUG("Allocated mem list is empty. (devid=%u; tsid=%u)\n", devid, tsid);
        return 0;
    }

    list_for_each_safe(pos, n, &mem_mgmt->alloced_mem_node_head) {
        mem_node = list_entry(pos, struct tsdrv_ts_mem_node, list);
        if (tsdrv_isolation_mem_is_polluted(mem_node->vaddr, mem_node->size)) {
            node_num++;
            TSDRV_PRINT_ERR("Node isolation mem has been polluted. (devid=%u; tsid=%u; sn=%u)\n",
                devid, tsid, mem_node->sn);
        }
    }

    return node_num;
}

u64 tsdrv_get_allocated_mem_size(u32 devid, u32 tsid)
{
    struct tsdrv_ts_mem_mgmt *mem_mgmt = NULL;

    mem_mgmt = tsdrv_get_ts_mem_mgmt(devid, tsid);

    return mem_mgmt->alloced_mem_size;
}

#ifdef TSDRV_UT
/* use for ut test. */
static void tsdrv_ts_mem_free(void)
{
    u32 tsid;
    u32 devid;
    struct tsdrv_ts_mem_mgmt *mem_mgmt = NULL;
    struct tsdrv_ts_mem_node *mem_node = NULL;
    struct list_head *pos = NULL, *n = NULL;

    for (devid = 0; devid < DEVDRV_MAX_DAVINCI_NUM; devid++) {
        for (tsid = 0; tsid < DEVDRV_MAX_TS_NUM; tsid++) {
            mem_mgmt = tsdrv_get_ts_mem_mgmt(devid, tsid);
            if (list_empty_careful(&mem_mgmt->alloced_mem_node_head)) {
                continue;
            }

            list_for_each_safe(pos, n, &mem_mgmt->alloced_mem_node_head) {
                mem_node = list_entry(pos, struct tsdrv_ts_mem_node, list);
                list_del(&mem_node->list);
                free_pages((uintptr_t)mem_node->vaddr, (u32)get_order(mem_node->size));
                kfree((void *)mem_node);
            }
        }
    }
}
#endif

void tsdrv_ts_mem_mgmt_init(void)
{
    int ret;
    u32 tsid;
    u32 devid;
    struct tsdrv_ts_mem_mgmt *mem_mgmt = NULL;

    for (devid = 0; devid < DEVDRV_MAX_DAVINCI_NUM; devid++) {
        for (tsid = 0; tsid < DEVDRV_MAX_TS_NUM; tsid++) {
            mem_mgmt = tsdrv_get_ts_mem_mgmt(devid, tsid);
            mem_mgmt->alloced_mem_size = 0;
            INIT_LIST_HEAD(&mem_mgmt->alloced_mem_node_head);
        }
    }

    ret = tsdrv_ipc_handler_register(IPCDRV_TS_MEM_OP, tsdrv_ts_alloc_mem_request);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Register ts mem opertion handler failed.\n");
    }
}

void tsdrv_ts_mem_mgmt_exit(void)
{
    tsdrv_ipc_handler_unregister(IPCDRV_TS_MEM_OP);

#ifdef TSDRV_UT
    tsdrv_ts_mem_free();
#endif
}
