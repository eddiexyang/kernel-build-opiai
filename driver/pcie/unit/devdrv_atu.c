/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
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
#include <linux/delay.h>

#include "devdrv_atu.h"
#include "nvme_comm_drv.h"
#include "apb_drv.h"
#include "devdrv_dma.h"
#include "agentdrv_common_msg.h"

struct devdrv_p2p_atu_info p2p_atu_table[MAX_AGENTDEV_CNT][DEVDRV_TX_ATU_NUM];
struct mutex p2p_atu_table_lock;
struct devdrv_h2d_atu_info g_h2d_atu_table[MAX_AGENTDEV_CNT][MAX_H2D_ATU_NUM];
int devdrv_del_dev_h2d_tx_atu(void __iomem *apb_base, struct devdrv_iob_atu atu[], u32 len,
    const struct devdrv_cfg_tx_atu_para *tx_para, dma_addr_t target_addr);

void devdrv_tx_atu_init(void)
{
    u32 i, j;

    mutex_init(&p2p_atu_table_lock);
    for (i = 0; i < MAX_AGENTDEV_CNT; i++) {
        for (j = 0; j < DEVDRV_TX_ATU_NUM; j++) {
            p2p_atu_table[i][j].valid = ATU_INVALID;
        }
        for (j = 0; j < MAX_H2D_ATU_NUM; j++) {
            g_h2d_atu_table[i][j].offset = DEVDRV_PCIE_DDR_ATU_OFFSET * (j + 1);
            g_h2d_atu_table[i][j].valid = ATU_INVALID;
        }
    }
}

u64 devdrv_get_base_addr_offset(u32 dev_id)
{
    int i;
    u64 offset = 0;

    mutex_lock(&p2p_atu_table_lock);
    for (i = 0; i < MAX_H2D_ATU_NUM; i++) {
        if (g_h2d_atu_table[dev_id][i].valid == ATU_VALID) {
            continue;
        }
        g_h2d_atu_table[dev_id][i].valid = ATU_VALID;
        offset = g_h2d_atu_table[dev_id][i].offset;
        break;
    }
    mutex_unlock(&p2p_atu_table_lock);

    return offset;
}

void devdrv_free_base_addr_offset(u32 devid, u64 target_addr)
{
    int i;

    mutex_lock(&p2p_atu_table_lock);
    for (i = 0; i < MAX_H2D_ATU_NUM; i++) {
        if (g_h2d_atu_table[devid][i].valid == ATU_INVALID) {
            continue;
        }
        if ((g_h2d_atu_table[devid][i].devid == devid) && (g_h2d_atu_table[devid][i].target_addr == target_addr)) {
            g_h2d_atu_table[devid][i].devid = 0;
            g_h2d_atu_table[devid][i].valid = ATU_INVALID;
            g_h2d_atu_table[devid][i].target_addr = 0;
            g_h2d_atu_table[devid][i].size = 0;
            break;
        }
    }
    mutex_unlock(&p2p_atu_table_lock);

    return;
}

void devdrv_set_h2d_atu_id(u64 offset, u32 atu_id, u32 dev_id, u64 target_addr, u64 size)
{
    u32 index;

    if (offset == (u64)-1) {
        return;
    }
    index = offset / DEVDRV_PCIE_DDR_ATU_OFFSET - 1;

    if (index < MAX_H2D_ATU_NUM) {
        g_h2d_atu_table[dev_id][index].atu_id = atu_id;
        g_h2d_atu_table[dev_id][index].devid = dev_id;
        g_h2d_atu_table[dev_id][index].target_addr = target_addr;
        g_h2d_atu_table[dev_id][index].size = size;
    }
}

u32 devdrv_get_h2d_atu_id(u32 devid, dma_addr_t addr)
{
    u32 atu_id = (u32)-1;
    int i;

    mutex_lock(&p2p_atu_table_lock);
    for (i = 0; i < MAX_H2D_ATU_NUM; i++) {
        if (g_h2d_atu_table[devid][i].valid == ATU_INVALID) {
            continue;
        }
        if ((g_h2d_atu_table[devid][i].devid == devid) && (addr >= g_h2d_atu_table[devid][i].target_addr) &&
            (addr < (g_h2d_atu_table[devid][i].target_addr + g_h2d_atu_table[devid][i].size))) {
            atu_id = g_h2d_atu_table[devid][i].atu_id;
            break;
        }
    }
    mutex_unlock(&p2p_atu_table_lock);
    return atu_id;
}

u32 devdrv_alloc_atu_id(u32 local_devid, u32 host_devid, u32 dst_host_devid, u32 atu_type, u64 target_size)
{
    u32 atu_id;
    u32 devid_for_alloc = dst_host_devid;
    struct devdrv_p2p_atu_info *p2p_atu = NULL;

    if (atu_type == ATU_TYPE_TX_HOST) {
        devid_for_alloc = host_devid;
    }

    mutex_lock(&p2p_atu_table_lock);
    for (atu_id = 0; atu_id < DEVDRV_TX_ATU_NUM; atu_id++) {
        p2p_atu = &p2p_atu_table[local_devid][atu_id];
        if (p2p_atu->valid == ATU_VALID) {
            continue;
        }
        p2p_atu->devid = devid_for_alloc;
        p2p_atu->type = atu_type;
        p2p_atu->valid = ATU_VALID;
        p2p_atu->host_devid = host_devid;
        p2p_atu->size = target_size;
        p2p_atu->alloc_times++;
        break;
    }
    mutex_unlock(&p2p_atu_table_lock);

    return atu_id;
}

u32 devdrv_get_atu_id(u32 local_devid, u32 host_devid, u32 dst_host_devid, u32 atu_type)
{
    u32 atu_id;
    u32 devid_for_cmp = dst_host_devid;
    struct devdrv_p2p_atu_info *p2p_atu = NULL;

    if (atu_type == ATU_TYPE_TX_HOST) {
        devid_for_cmp = host_devid;
    }

    mutex_lock(&p2p_atu_table_lock);
    for (atu_id = 0; atu_id < DEVDRV_TX_ATU_NUM; atu_id++) {
        p2p_atu = &p2p_atu_table[local_devid][atu_id];
        if (p2p_atu->valid == ATU_INVALID) {
            continue;
        }
        if ((p2p_atu->devid == devid_for_cmp) && (p2p_atu->type == atu_type)) {
            break;
        }
    }
    mutex_unlock(&p2p_atu_table_lock);

    return atu_id;
}

void devdrv_free_atu_id(u32 local_devid, u32 atu_id)
{
    mutex_lock(&p2p_atu_table_lock);
    p2p_atu_table[local_devid][atu_id].valid = ATU_INVALID;
    p2p_atu_table[local_devid][atu_id].free_times++;
    mutex_unlock(&p2p_atu_table_lock);
}

/* this addr should be visited by ts, ts online has 40 bit address space, so we only use 64bit space
 * dev2 tx atu config:
 * mem space 256GB: dev0 32GB, dev1 32GB, io reg space, dev3 32GB, ... , dev7 32GB
 * io space 32GB:   io space dev0 4GB,  io space dev1 4GB, hole 4GB, io space dev3 4GB,
 * host: 1TB, 1TB, 1TB, 1TB, 1TB, 1TB, 1TB
 */
STATIC int devdrv_get_atu_base_info(const struct devdrv_cfg_tx_atu_para *tx_para, struct devdrv_shr_para __iomem *para,
    u64* base_addr_ret, u64* base_size_ret, u64* offset_ret)
{
    u32 local_devid = tx_para->local_devid;
    u32 host_devid = tx_para->host_devid;
    u32 dst_host_devid = tx_para->dst_host_devid;
    u32 atu_type = tx_para->atu_type;
    u64 base_size, base_addr, offset;

    if (atu_type == ATU_TYPE_TX_MEM) {
        base_size = DEVDRV_PCIE_DDR_SIZE / DEVDRV_P2P_SUPPORT_MAX_DEVICE;
        base_addr = para->tx_atu_base_addr1 + base_size * dst_host_devid;
    } else if (atu_type == ATU_TYPE_TX_IO) {
        base_size = DEVDRV_PCIE_DDR_SIZE / DEVDRV_P2P_SUPPORT_MAX_DEVICE;
        base_addr = para->tx_atu_base_addr1 + base_size * host_devid;
        base_size = base_size / DEVDRV_P2P_SUPPORT_MAX_DEVICE;
        base_addr = base_addr + base_size * dst_host_devid;
    } else {
        if (para->tx_atu_base_size1 < DEVDRV_PCIE_DDR_SIZE) {
            devdrv_err("Parameter base_size1 invaild. (local_dev_id=%u; host_dev=%u; dst=%u; atu_type=%u; size=%llx)\n",
                local_devid, host_devid, dst_host_devid, atu_type, para->tx_atu_base_size1);
            return -EINVAL;
        }
        offset = devdrv_get_base_addr_offset(local_devid);
        if (offset == 0) {
            devdrv_err("Get base address failed. (local_dev=%u; host_dev=%u; dst=%u; atu_type=%u)\n",
                local_devid, host_devid, dst_host_devid, atu_type);
            return -EINVAL;
        }
        *offset_ret = offset;
        base_size = para->tx_atu_base_size1 / DEVDRV_H2D_SUPPORT_MAX_DEVICE;
        base_addr = para->tx_atu_base_addr1 + offset;
    }
    *base_addr_ret = base_addr;
    *base_size_ret = base_size;

    return 0;
}

int devdrv_set_tx_atu(void __iomem *apb_base, struct devdrv_cfg_tx_atu_para *tx_para,
    u64 target_addr, u64 target_size, struct devdrv_shr_para __iomem *para)
{
    u32 local_devid = tx_para->local_devid;
    u32 host_devid = tx_para->host_devid;
    u32 dst_host_devid = tx_para->dst_host_devid;
    u32 atu_type = tx_para->atu_type;
    struct devdrv_iob_atu atu;
    u32 atu_id, pf_num;
    u64 base_size, base_addr;
    u64 offset = (u32)-1;
    int ret;

    ret = devdrv_get_atu_base_info(tx_para, para, &base_addr, &base_size, &offset);
    if (ret != 0) {
        devdrv_err("Got base information failed. (local_dev=%u; host_dev=%u; dst=%u; atu_type=%u)\n",
            local_devid, host_devid, dst_host_devid, atu_type);
        return -EINVAL;
    }

    if (target_size > base_size) {
        devdrv_err("target_size is invalid. (host_devid=%u; atu_type=%u; target_size=0x%llx; base_size=0x%llx)\n",
            host_devid, atu_type, target_size, base_size);
        goto FREE_BASE_ADDR_OFFSET;
    }

    atu_id = devdrv_alloc_atu_id(local_devid, host_devid, dst_host_devid, atu_type, target_size);
    if (atu_id >= DEVDRV_TX_ATU_NUM) {
        devdrv_err("atu_id overflow. (host_devid=%u; dst_host_devid=%u; atu_id=%u)\n",
                   host_devid, dst_host_devid, atu_id);
        goto FREE_BASE_ADDR_OFFSET;
    }

    if (atu_type == ATU_TYPE_TX_HOST) {
        devdrv_set_h2d_atu_id(offset, atu_id, local_devid, target_addr, target_size);
    }
    atu.target_addr = target_addr;
    atu.base_addr = base_addr;
    atu.size = target_size;

    pf_num = devdrv_get_nvme_pf_num(local_devid);
    devdrv_add_tx_atu(apb_base, atu_id, pf_num, &atu);
    return 0;

FREE_BASE_ADDR_OFFSET:
    if (atu_type == ATU_TYPE_TX_HOST) {
        mutex_lock(&p2p_atu_table_lock);
        g_h2d_atu_table[local_devid][offset / DEVDRV_PCIE_DDR_ATU_OFFSET - 1].valid = ATU_INVALID;
        g_h2d_atu_table[local_devid][offset / DEVDRV_PCIE_DDR_ATU_OFFSET - 1].devid = 0;
        mutex_unlock(&p2p_atu_table_lock);
    }
    return -EINVAL;
}

int devdrv_get_dev_tx_atu(const void __iomem *apb_base, struct devdrv_iob_atu atu[], u32 len,
    const struct devdrv_cfg_tx_atu_para *tx_para, u64 phy_addr)
{
    u32 local_devid = tx_para->local_devid;
    u32 host_devid = tx_para->host_devid;
    u32 dst_host_devid = tx_para->dst_host_devid;
    u32 atu_type = tx_para->atu_type;
    u32 atu_id, pf_num;

    if (atu_type != ATU_TYPE_TX_HOST) {
        atu_id = devdrv_get_atu_id(local_devid, host_devid, dst_host_devid, atu_type);
    } else {
        atu_id = devdrv_get_h2d_atu_id(local_devid, (dma_addr_t)phy_addr);
    }
    if (atu_id >= DEVDRV_TX_ATU_NUM) {
        devdrv_err("Variable atu_id overflow. (host_devid=%u; dst_host_devid=%u)\n", host_devid, dst_host_devid);
        return -EINVAL;
    }
    pf_num = devdrv_get_nvme_pf_num(local_devid);
    if (devdrv_get_tx_atu(apb_base, atu_id, pf_num, &atu[atu_id]) == 0) {
        atu[atu_id].phy_addr = phy_addr;
        atu[atu_id].valid = ATU_VALID;
    } else {
        atu[atu_id].valid = ATU_INVALID;
    }

    return 0;
}

STATIC void devdrv_free_sigle_tx_atu(void __iomem *apb_base, struct devdrv_iob_atu atu[], u32 len,
    u32 local_devid, u32 atu_id)
{
    u32 pf_num;

    if (atu_id >= DEVDRV_TX_ATU_NUM) {
        return;
    }
    devdrv_free_atu_id(local_devid, atu_id);
    atu[atu_id].valid = ATU_INVALID;
    msleep(DEVDRV_ATU_INVALID_DELAY);
    pf_num = devdrv_get_nvme_pf_num(local_devid);
    devdrv_del_tx_atu(apb_base, atu_id, pf_num, &atu[atu_id]);
}

int devdrv_del_dev_tx_atu(void __iomem *apb_base, struct devdrv_iob_atu atu[], u32 len,
    struct devdrv_cfg_tx_atu_para *tx_para, u64 target_addr)
{
    u32 local_devid = tx_para->local_devid;
    u32 host_devid = tx_para->host_devid;
    u32 dst_host_devid = tx_para->dst_host_devid;
    u32 atu_id;

    if (tx_para->atu_type == ATU_TYPE_TX_HOST) {
        (void)devdrv_del_dev_h2d_tx_atu(apb_base, atu, len, tx_para, target_addr);
        return 0;
    }

    atu_id = devdrv_get_atu_id(local_devid, host_devid, dst_host_devid, tx_para->atu_type);
    devdrv_free_sigle_tx_atu(apb_base, atu, len, local_devid, atu_id);

    return 0;
}

int devdrv_del_dev_h2d_tx_atu(void __iomem *apb_base, struct devdrv_iob_atu atu[], u32 len,
    const struct devdrv_cfg_tx_atu_para *tx_para, dma_addr_t target_addr)
{
    u32 local_devid = tx_para->local_devid;
    u32 atu_id;

    atu_id = devdrv_get_h2d_atu_id(local_devid, target_addr);
    if (atu_id > DEVDRV_MAX_TX_ATU_NUM) {
        devdrv_err("Variable atu_id overflow. (local_devid=%u; target_addr=0x%pK)\n",
            local_devid, (void*)(uintptr_t)(u64)target_addr);
        return -EINVAL;
    }
    devdrv_free_base_addr_offset(local_devid, target_addr);
    devdrv_free_sigle_tx_atu(apb_base, atu, len, local_devid, atu_id);

    return 0;
}

/* print already configed tx atu in every error branch in agentdrv_cfg_p2p_tx_atu */
void devdrv_tx_atu_print_cfg_info(void)
{
    struct devdrv_p2p_atu_info *p2p_atu = NULL;
    u32 local_devid;
    u32 atu_id;

    for (local_devid = 0; local_devid < MAX_AGENTDEV_CNT; local_devid++) {
        for (atu_id = 0; atu_id < DEVDRV_TX_ATU_NUM; atu_id++) {
            p2p_atu = &p2p_atu_table[local_devid][atu_id];
            if (p2p_atu->valid == ATU_INVALID) {
                continue;
            }
            devdrv_event("Configured ATU. (local id=%u; host id=%u; dst host id=%u; "
                "type=%u; atu_id=%u; size=0x%llx; alloc_times=%u; free_times=%u)\n",
                local_devid, p2p_atu->host_devid, p2p_atu->devid, p2p_atu->type, atu_id, p2p_atu->size,
                p2p_atu->alloc_times, p2p_atu->free_times);
        }
    }
}

int devdrv_mem_rx_atu_init(u32 devid, const void __iomem *apb_base, struct devdrv_iob_atu atu[], int num)
{
    u32 pf_num;

    pf_num = devdrv_get_nvme_pf_num(devid);
    devdrv_rx_atu_init(apb_base, pf_num, PCI_BAR_MEM, atu, num);
    return 0;
}

void devdrv_io_rx_atu_show(u32 devid, const void __iomem *apb_base)
{
    struct devdrv_iob_atu io_rx_atu[DEVDRV_MAX_RX_ATU_NUM];
    u32 pf_num;

    pf_num = devdrv_get_nvme_pf_num(devid);
    devdrv_rx_atu_init(apb_base, pf_num, PCI_BAR_IO, io_rx_atu, DEVDRV_MAX_RX_ATU_NUM);
}

void devdrv_rsv_mem_rx_atu_show(u32 devid, const void __iomem *apb_base)
{
    struct devdrv_iob_atu rsv_mem_rx_atu[DEVDRV_MAX_RX_ATU_NUM];
    u32 pf_num;

    pf_num = devdrv_get_nvme_pf_num(devid);
    devdrv_rx_atu_init(apb_base, pf_num, PCI_BAR_RSV_MEM, rsv_mem_rx_atu, DEVDRV_MAX_RX_ATU_NUM);
}
