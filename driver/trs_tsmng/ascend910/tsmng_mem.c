/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
 * Create: 2023-05-06
 */
#include <linux/mm.h>
#include <linux/gfp.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/oom.h>

#ifndef EMU_ST
#include "securec.h"
#include "icm_interface.h"
#include "drv_ipc.h"
#include "chip_config.h"
#include "uda.h"
#include "tsmng_log.h"

struct tsmng_ts_mem_attr {
    u64 vaddr;
    u64 paddr;
    u32 mem_size;
    u8 vfid;

    u8 data[11]; // 11 bytes
    u64 rsv;
};

static void tsmng_mem_make_exact(struct page *p, u32 order, size_t size)
{
    struct page *cur_page = NULL;
    u64 alloced_num, page_num, i;

    if (order == 0) {
        return;
    }
    split_page(p, order);
    alloced_num = 1ull << order;
    page_num = PAGE_ALIGN(size) / PAGE_SIZE;

    for (i = page_num; i < alloced_num; i++) {
        cur_page = nth_page(p, i);
        __free_page(cur_page);
    }
}

static void *tsmng_mem_alloc_ddr(int nid, size_t size, u64 *paddr)
{
    u32 order = get_order(size);
    struct page *p = NULL;

    p = alloc_pages_node(nid, GFP_ATOMIC | __GFP_THISNODE | __GFP_ZERO | __GFP_ACCOUNT |
            GFP_HIGHUSER_MOVABLE | __GFP_NOWARN, order);
    if (p == NULL) {
        tsmng_drv_debug("Alloc pages failed. (nid=%u; order=%u)\n", nid, order);
        return NULL;
    }
    tsmng_mem_make_exact(p, order, size);
    *paddr = (u64)page_to_phys(p);

    tsmng_drv_debug("Alloc pages. (nid=%u; order=%u)\n", nid, order);

    return (void *)page_address(p);
}

static void *_tsmng_ts_alloc_mem(u32 devid, size_t size, u64 *paddr)
{
    u32 nids[DBL_NUMA_ID_MAX_NUM];
    int node_num, i;

    node_num = dbl_get_ts_nid(devid, nids, DBL_NUMA_ID_MAX_NUM);
    if ((node_num <= 0) || (node_num > DBL_NUMA_ID_MAX_NUM)) {
        tsmng_drv_err("Invalid node_num. (devid=%u; node_num=%d)\n", devid, node_num);
        return NULL;
    }

    for (i = 0; i < node_num; i++) {
        void *vaddr = tsmng_mem_alloc_ddr(nids[i], size, paddr);
        if (vaddr != NULL) {
            return vaddr;
        }
    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
    (void)oom_type_notifier_call(OOM_TYPE_CGROUP, NULL);
#else
    (void)hisi_oom_notifier_call(HISI_OOM_TYPE_CGROUP, NULL);
#endif

    return NULL;
}

static inline void _tsmng_ts_free_mem(void *vaddr, u32 size)
{
    free_pages_exact(vaddr, size);
}

static bool tsmng_paddr_is_valid(u64 paddr, u32 size, u64 start, u64 end)
{
    if ((ULLONG_MAX - paddr) <= size) {
        return false;
    }

    if ((paddr >= start) && (paddr + size <= end)) {
        return false;
    }

    return true;
}

static bool tsmng_paddr_check(u32 devid, u64 paddr, u32 size)
{
    u64 start, end;

    start = __pfn_to_phys(node_start_pfn(0));
    end = __pfn_to_phys(node_end_pfn(0));

    return tsmng_paddr_is_valid(paddr, size, start, end);
}

void tsmng_ts_free_mem(void *data, u32 devid)
{
    struct icmdrv_ipc_msg_info *ipc_rx = (struct icmdrv_ipc_msg_info *)data;
    struct tsmng_ts_mem_attr *payload = (struct tsmng_ts_mem_attr *)ipc_rx->data;

    if (payload->vaddr != 0) {
        void *vaddr = (void *)(uintptr_t)payload->vaddr;
        if (tsmng_paddr_check(devid, virt_to_phys(vaddr), payload->mem_size)) {
            _tsmng_ts_free_mem(vaddr, payload->mem_size);
        } else {
            tsmng_drv_warn("Abnormal. (cmd=%d; sub_cmd=%d; mem_size=0x%x; vaddr=0x%pK)\n",
                ipc_rx->cmd, ipc_rx->sub_cmd, payload->mem_size, (void *)(uintptr_t)payload->vaddr);
        }
    }

    tsmng_drv_info("Ts mem ops. (cmd=%d; sub_cmd=%d; mem_size=0x%x)\n",
        ipc_rx->cmd, ipc_rx->sub_cmd, payload->mem_size);
}

int tsmng_ts_req_mem(void *data, u32 devid)
{
    struct icmdrv_ipc_msg_info *ipc_rx = (struct icmdrv_ipc_msg_info *)data;
    struct tsmng_ts_mem_attr *payload = (struct tsmng_ts_mem_attr *)ipc_rx->data;
    struct icmdrv_ipc_msg ipc_tx = {0};
    struct uda_mia_dev_para mia_para;
    u32 udevid = devid;
    int ret;

    if (payload->vfid >= 1) {
        uda_mia_dev_para_pack(&mia_para, devid, payload->vfid - 1);
        ret = uda_mia_devid_to_udevid(&mia_para, &udevid);
        if (ret != 0) {
            tsmng_drv_err("Get udevid failed.(dev_id=%u; vfid=%u; ret=%d)\n", devid, payload->vfid, ret);
            return ret;
        }
    }

    payload->vaddr = (u64)(uintptr_t)_tsmng_ts_alloc_mem(udevid, payload->mem_size, &payload->paddr);
    if (payload->vaddr == 0) {
        return -EINVAL;
    }

    ipc_tx.sub_cmd = ICM_SUB_CMD_MEM_ALLOC;
    ipc_tx.cmd = ICM_MAIN_CMD_TS_REQ;
    ipc_tx.cmd_dest = OBJ_CMD_TS;
    ipc_tx.cmd_src = OBJ_CMD_CCPU;
    ipc_tx.len = sizeof(struct tsmng_ts_mem_attr);

    ret = memcpy_s(ipc_tx.data, ICM_MSG_DATA_LENGTH, payload, sizeof(struct tsmng_ts_mem_attr));
    if (ret != 0) {
        tsmng_drv_err("memcpy failed. (ret=%d)\n", ret);
        goto fail;
    }

    ret = icm_msg_send_async(ICM_FD_BUILD(devid, HISI_RPROC_TX_TS_MBX3), (rproc_msg_t *)&ipc_tx,
        IPCDRV_RPROC_MSG_LENGTH);
    if (ret != 0) {
        tsmng_drv_err("Icm_msg_send_async failed. (ret=%d)\n", ret);
        goto fail;
    }

    tsmng_drv_info("Ts mem ops. (devid=%u; cmd=%d; sub_cmd=%d; vfid=%u; mem_size=0x%x)\n",
        udevid, ipc_rx->cmd, ipc_rx->sub_cmd, payload->vfid, payload->mem_size);

    return 0;

fail:
    tsmng_ts_free_mem(data, devid);
    return ret;
}
#else
void tsmng_mem_ut_stub(void)
{
    return;
}
#endif

