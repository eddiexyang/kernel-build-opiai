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
#include <linux/types.h>
#include <linux/dma-direction.h>
#include <linux/dma-mapping.h>
#include <linux/kallsyms.h>
#include <linux/version.h>
#include <linux/module.h>

#include "dbl/chip_config.h"
#include "agentdrv_interface.h"
#include "agentdrv_unit.h"
#include "devdrv_util.h"
#include "apb_comm_drv.h"
#include "resource_drv.h"
#include "kernel_version_adapt.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
static inline void *dma_zalloc_coherent(struct device *dev, size_t size, dma_addr_t *dma_handle, gfp_t flag)
{
    void *ret = dma_alloc_coherent(dev, size, dma_handle, flag | __GFP_ZERO);
    return ret;
}
#endif

/* devid: local devid */
int agentdrv_get_host_devid(u32 dev_id)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    u32 chip_id;
    u32 func_id;

    if (dev_id >= MAX_AGENTDEV_CNT) {
        devdrv_err("Device ID is invalid.(dev_id=%u)\n", dev_id);
        return -EINVAL;
    }
    devdrv_dev2chipfunc(dev_id, &chip_id, &func_id);

    agent_dev = agentdrv_get_dev((int)chip_id);
    if ((agent_dev != NULL) && (agentdrv_is_dev_valid(agent_dev, func_id) == true)) {
        return agent_dev->shr_para[func_id]->host_dev_id;
    }

    return -EINVAL;
}
EXPORT_SYMBOL(agentdrv_get_host_devid);

u32 devdrv_get_dev_chip_type(u32 dev_id)
{
    return devdrv_get_chip_type();
}
EXPORT_SYMBOL(devdrv_get_dev_chip_type);

int agentdrv_numa_addr_unmap(u32 dev_id, u32 vfid)
{
    return 0;
}
EXPORT_SYMBOL(agentdrv_numa_addr_unmap);

int agentdrv_numa_addr_remap(u32 dev_id, u32 vfid, agentdrv_numa_remap_t *numa_remap)
{
    return 0;
}
EXPORT_SYMBOL(agentdrv_numa_addr_remap);

int agentdrv_get_cpu_info(u32 dev_id, struct agentdrv_cpu_info *cpu_info)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    u32 chip_id;
    u32 func_id;

    if ((dev_id >= MAX_AGENTDEV_CNT) || (cpu_info == NULL)) {
        devdrv_err("Device ID is invalid or cpu_info is NULL.(dev_id=%u)\n", dev_id);
        return -EINVAL;
    }
    devdrv_dev2chipfunc(dev_id, &chip_id, &func_id);

    agent_dev = agentdrv_get_dev((int)chip_id);
    if ((agent_dev == NULL) || (agent_dev->cpu_info_valid == DEVDRV_INVALID)) {
        devdrv_err("Call agentdrv_get_dev failed, agent_dev is invalid.(dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    *cpu_info = agent_dev->cpu_info[func_id];

    return 0;
}
EXPORT_SYMBOL(agentdrv_get_cpu_info);

STATIC int agentdrv_get_addr_info_para_check(u32 devid, u32 index, const u64 *addr, const size_t *size)
{
    if (devid >= MAX_AGENTDEV_CNT) {
        devdrv_err("Device ID is invalid.(dev_id=%u)\n", devid);
        return -EINVAL;
    }

    if (index || (addr == NULL) || (size == NULL)) {
        devdrv_err("Input Parameter is error.\n");
        return -EINVAL;
    }
    return 0;
}
int agentdrv_get_addr_info(u32 devid, enum devdrv_addr_type type, u32 index, u64 *addr, size_t *size)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    struct agent_res_sdi_addr sdi_addr = {0};
    u32 chip_id;
    u32 func_id;
    int ret;

    ret = agentdrv_get_addr_info_para_check(devid, index, addr, size);
    if (ret) {
        devdrv_err("Input parameter is error. (dev_id=%u; ret=%d)\n", devid, ret);
        return ret;
    }

    devdrv_dev2chipfunc(devid, &chip_id, &func_id);
    agent_dev = agentdrv_get_dev((int)chip_id);
    if ((agent_dev == NULL) || (agentdrv_is_dev_valid(agent_dev, func_id) == false)) {
        devdrv_err("Call agentdrv_get_dev failed, agent_dev is invalid.(dev_id=%u)\n", devid);
        return -EINVAL;
    }

    agentdrv_res_sdi_addr(chip_id, func_id, &sdi_addr);
    switch (type) {
        case DEVDRV_ADDR_TSDRV_RESV_BASE:
            *addr = sdi_addr.msg_base + DEVDRV_RESERVE_TSDRV_RESV_OFFSET;
            *size = DEVDRV_RESERVE_TSDRV_RESV_SIZE;
            break;
        case DEVDRV_ADDR_DEVMNG_RESV_BASE:
            *addr = sdi_addr.msg_base + DEVDRV_RESERVE_DEVMNG_RESV_OFFSET;
            *size = DEVDRV_RESERVE_DEVMNG_RESV_SIZE;
            break;
        case DEVDRV_ADDR_VF_BANDWIDTH_BASE:
            *addr = sdi_addr.bw_ctrl_base;
            *size = sdi_addr.bw_ctrl_size;
            break;
        case DEVDRV_ADDR_STARS_TOPIC_SCHED_RES_MEM_BASE:
            *addr = sdi_addr.topic_sched_resmem_base;
            *size = sdi_addr.topic_sched_resmem_size;
            break;
        case DEVDRV_ADDR_HBM_BASE:
            *addr = sdi_addr.hbm_base;
            *size = sdi_addr.hbm_size;
            break;
        default:
            devdrv_err("Device type is error. (devid=%d; type=%d)\n", devid, type);
            return -EINVAL;
    }

    return 0;
}
EXPORT_SYMBOL(agentdrv_get_addr_info);

/* return:  0-3, -1 not pcie space addr */
int agentdrv_get_devid_from_phy_addr(phys_addr_t phy_addr)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    struct devdrv_shr_para __iomem *shr_para = NULL;
    int dev_id = -1;
    int chip_id = 0;
    u32 func_id = 0;

    for (chip_id = 0; chip_id < MAX_AGENTCHIP_CNT; chip_id++) {
        agent_dev = agentdrv_get_dev((int)chip_id);
        if (agent_dev == NULL) {
            devdrv_err("Call agentdrv_get_dev failed, agent_dev is null.\n");
            return -EINVAL;
        }

        for (func_id = 0; func_id < agent_dev->func_totl_num; func_id++) {
            if (!agentdrv_is_dev_valid(agent_dev, func_id)) {
                continue;
            }

            shr_para = agent_dev->shr_para[func_id];
            if (((u64)phy_addr >= shr_para->tx_atu_base_addr1) &&
                ((u64)phy_addr < shr_para->tx_atu_base_addr1 + shr_para->tx_atu_base_size1)) {
                devdrv_chipfunc2dev(&dev_id, chip_id, (int)func_id);
                return dev_id;
            }

            if (((u64)phy_addr >= shr_para->tx_atu_base_addr2) &&
                ((u64)phy_addr < shr_para->tx_atu_base_addr2 + shr_para->tx_atu_base_size2)) {
                devdrv_chipfunc2dev(&dev_id, chip_id, (int)func_id);
                return dev_id;
            }
        }
    }

    return -EINVAL;
}
EXPORT_SYMBOL(agentdrv_get_devid_from_phy_addr);

/* input:  devid: device dev number
   output:  chan_id_base: The starting dma channel number assigned to ts
            chan_num: Number of dma channels assigned to ts
            chan_done_irq_base: valid in cloud, The starting number of the dma done interrupt */
int agentdrv_get_ts_dma_chan_info(u32 dev_id, struct agentdrv_ts_dma_chan_info *chan_info)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    struct devdrv_dma_dev *dma_dev = NULL;
    u32 chip_id = 0;
    u32 func_id = 0;
    int ret;
    u32 mode;

    if (dev_id >= MAX_AGENTDEV_CNT) {
        devdrv_err("Device ID is invalid.(dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    if (chan_info == NULL) {
        devdrv_err("Input parameter is error. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    /* rc mode not surport */
    ret = agentdrv_get_rc_ep_mode(&mode);
    if ((ret == 0) && (mode == DEVDRV_PCIE_RC_MODE)) {
        chan_info->chan_id_base = 0;
        chan_info->chan_num = 0;
        chan_info->chan_done_irq_base = 0;
        chan_info->pf_num = 0;
        chan_info->func_total = 0;
        chan_info->msix_offset = 0;
        return 0;
    }

    devdrv_dev2chipfunc(dev_id, &chip_id, &func_id);
    agent_dev = agentdrv_get_dev((int)chip_id);
    if ((agent_dev == NULL) || (agentdrv_is_dev_valid(agent_dev, func_id) == false)) {
        devdrv_err("Call agentdrv_get_dev failed, agent_dev is invalid.(dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    dma_dev = agent_dev->p_agentdrv_dma_dev[func_id];
    if (dma_dev == NULL) {
        devdrv_err("Dma_dev is NULL. (dev_id=%u; func_id=%u)\n", dev_id, func_id);
        return -EINVAL;
    }

    chan_info->chan_num = dma_dev->ts_chan_num;
    chan_info->chan_id_base = dma_dev->ts_chan[0];
    chan_info->chan_done_irq_base = agent_dev->pdev->irq + (chan_info->chan_id_base);
    chan_info->pf_num = devdrv_get_nvme_pf_num(dev_id);
    chan_info->func_total = agentdrv_res_get_func_total();
    chan_info->msix_offset = agent_dev->shr_para[func_id]->msix_offset;
    devdrv_info("Get dma_chan_info. (dev_id=%u; chip_id=%u; func_id=%u; ts_dma_chan_base=%u; num=%u; irq_base=%u;"
        " pf_num=%u; func_total=%u; msix_offset=%u)\n",
        dev_id, chip_id, func_id, chan_info->chan_id_base, chan_info->chan_num, chan_info->chan_done_irq_base,
        chan_info->pf_num, chan_info->func_total, chan_info->msix_offset);

    return 0;
}
EXPORT_SYMBOL(agentdrv_get_ts_dma_chan_info);

STATIC int devdrv_alloc_dma_sq_cq_desc_for_ts(struct devdrv_dma_dev *dma_dev, u32 len, u64 *dma_addr, u64 *phy_addr)
{
    gfp_t gfp_mask = GFP_KERNEL | __GFP_THISNODE | GFP_HIGHUSER_MOVABLE;
    int nid = (int)dma_dev->dev_id;
    struct page *page = NULL;
    dma_addr_t addr;
    u32 chip_type;

    chip_type = devdrv_get_dev_chip_type(dma_dev->dev_id);
    if (chip_type == HISI_CHIP_UNKNOWN) {
        devdrv_err("Got chip type failed, unknown.\n");
        return -ENODEV;
    }

    if (chip_type == HISI_MINI_V2) {
        nid = dma_dev->func_id;
    }

    nid = dbl_get_ts_default_nid((u32)nid);
    page = alloc_pages_node(nid, gfp_mask, get_order(len));
    if (page == NULL) {
        devdrv_err("Alloc page failed. (dev_id=%d; len=%x; order=%d; nid=%d)\n",
                   dma_dev->dev_id, len, get_order(len), nid);
        return -ENOMEM;
    }

    addr = dma_map_page(dma_dev->dev, page, 0, len, DMA_BIDIRECTIONAL);
    if (dma_mapping_error(dma_dev->dev, addr)) {
        devdrv_err("DMA map page failed. (dev_id=%u; len=%x)\n", dma_dev->dev_id, len);
        __free_pages(page, get_order(len));
        return -EFAULT;
    }

    *phy_addr = (u64)page_to_phys(page);
    *dma_addr = (u64)addr;

    return 0;
}

int devdrv_dma_alloc_sq_desc_for_ts(u32 dev_id, u64 *dma_addr, u64 *phy_addr, u32 *len)
{
    struct devdrv_dma_dev *dma_dev = NULL;

    if ((dma_addr == NULL) || (phy_addr == NULL) || (len == NULL)) {
        devdrv_err("Input parameter is NULL. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    dma_dev = devdrv_get_dma_dev(dev_id);
    if (dma_dev == NULL) {
        devdrv_err("Device ID is invalid, dma_dev is NULL. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    *len = DEVDRV_DMA_SQ_DESC_SIZE * DEVDRV_MAX_TS_DMA_CH_SQ_DEPTH;

    return devdrv_alloc_dma_sq_cq_desc_for_ts(dma_dev, *len, dma_addr, phy_addr);
}
EXPORT_SYMBOL(devdrv_dma_alloc_sq_desc_for_ts);

int devdrv_dma_alloc_cq_desc_for_ts(u32 dev_id, u64 *dma_addr, u64 *phy_addr, u32 *len)
{
    struct devdrv_dma_dev *dma_dev = NULL;

    if ((dma_addr == NULL) || (phy_addr == NULL) || (len == NULL)) {
        devdrv_err("Input parameter is NULL. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    dma_dev = devdrv_get_dma_dev(dev_id);
    if (dma_dev == NULL) {
        devdrv_err("Device ID is invalid, dma_dev is NULL. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    *len = DEVDRV_DMA_CQ_DESC_SIZE * DEVDRV_MAX_TS_DMA_CH_CQ_DEPTH;

    return devdrv_alloc_dma_sq_cq_desc_for_ts(dma_dev, *len, dma_addr, phy_addr);
}
EXPORT_SYMBOL(devdrv_dma_alloc_cq_desc_for_ts);

int devdrv_check_dl_dlcmsm_state(void *drvdata)
{
    struct agentdrv_devctrl *devctrl = NULL;

    if (drvdata == NULL) {
        devdrv_err("Input parameter is error.");
        return -EINVAL;
    }

    devctrl = (struct agentdrv_devctrl *)drvdata;

    /* init first time, needn't check dlcmsm */
    if (devctrl->apb_base == NULL) {
        return 0;
    }

    /* get DL DFX_FSM_STATE addr */
    if (devdrv_check_dlcmsm(devctrl->apb_base)) {
        devdrv_err("Check dlcmsm is timeout.\n");
        return -ETIMEDOUT;
    }

    return 0;
}
EXPORT_SYMBOL(devdrv_check_dl_dlcmsm_state);

int agentdrv_get_host_phy_mach_flag(u32 dev_id, u32 *host_flag)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    void __iomem *addr = NULL;
    u32 chip_id;
    u32 func_id;
    u32 val;
    int ret;

    if (dev_id >= MAX_AGENTDEV_CNT) {
        devdrv_err("Input dev_id is error.\n");
        return -EINVAL;
    }

    if (host_flag == NULL) {
        devdrv_err("Input host_flag is error.\n");
        return -EINVAL;
    }

    ret = agentdrv_get_rc_ep_mode(&val);
    if (ret) {
        devdrv_err("Got rc or ep mode failed.\n");
        return -EINVAL;
    }

    if (val == DEVDRV_PCIE_RC_MODE) {
        devdrv_debug("Device rc mode.\n");
        *host_flag = DEVDRV_HOST_PHY_MACH_FLAG;
        return 0;
    }

    devdrv_dev2chipfunc(dev_id, &chip_id, &func_id);

    agent_dev = agentdrv_get_dev((int)chip_id);
    if ((agent_dev == NULL) || (agentdrv_is_dev_valid(agent_dev, func_id) == false)) {
        devdrv_err("Call agentdrv_get_dev failed, agent_dev is invalid. (dev_id=%u)\n", chip_id);
        return -EINVAL;
    }

    addr = agentdrv_get_phy_match_flag_addr(agent_dev, func_id);
    *host_flag = readl(addr);

    return 0;
}
EXPORT_SYMBOL(agentdrv_get_host_phy_mach_flag);

void agentdrv_set_host_phy_mach_flag(u32 dev_id, u32 host_flag)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    void __iomem *addr = NULL;
    u32 chip_id;
    u32 func_id;

    devdrv_dev2chipfunc(dev_id, &chip_id, &func_id);

    agent_dev = agentdrv_get_dev((int)chip_id);
    if ((agent_dev == NULL) || (agentdrv_is_dev_valid(agent_dev, func_id) == false)) {
        devdrv_err("Call agentdrv_get_dev failed, agent_dev is invalid.\n");
        return;
    }

    addr = agentdrv_get_phy_match_flag_addr(agent_dev, func_id);
    writel(host_flag, addr);
}

int agentdrv_read_capability(u32 dev_id, u32 *cap_value)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    u32 chip_id = 0;
    u32 func_id = 0;

    if ((dev_id >= MAX_AGENTDEV_CNT) || (cap_value == NULL)) {
        devdrv_err("Input parameter is invalid.(dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    devdrv_dev2chipfunc(dev_id, &chip_id, &func_id);
    agent_dev = agentdrv_get_dev((int)chip_id);
    if ((agent_dev == NULL) || (agentdrv_is_dev_valid(agent_dev, func_id) == false)) {
        devdrv_err("Call agentdrv_get_dev failed, agent_dev is invalid. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    *cap_value = agent_dev->shr_para[func_id]->capability;

    return 0;
}
EXPORT_SYMBOL(agentdrv_read_capability);

int agentdrv_clear_capability(u32 dev_id)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    u32 chip_id = 0;
    u32 func_id = 0;

    if (dev_id >= MAX_AGENTDEV_CNT) {
        devdrv_err("Device ID is invalid.(dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    devdrv_dev2chipfunc(dev_id, &chip_id, &func_id);
    agent_dev = agentdrv_get_dev((int)chip_id);
    if ((agent_dev == NULL) || (agentdrv_is_dev_valid(agent_dev, func_id) == false)) {
        devdrv_err("Call agentdrv_get_dev failed, agent_dev is invalid. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    agent_dev->shr_para[func_id]->capability = 0;

    return 0;
}
EXPORT_SYMBOL(agentdrv_clear_capability);

int agentdrv_hccs_host_dma_addr_map(struct agentdrv_msg_dev *msg_dev, void *data)
{
    return 0;
}

int agentdrv_hccs_host_dma_addr_unmap(struct agentdrv_msg_dev *msg_dev, void *data)
{
    return 0;
}

void *devdrv_dma_alloc_coherent(struct device *dev, size_t size, dma_addr_t *dma_addr, gfp_t gfp)
{
    if ((size == 0) || (dev == NULL) || (dma_addr == NULL)) {
        devdrv_err("Input parameter is invalid. (size=%lu)\n", size);
        return NULL;
    }

    return dma_alloc_coherent(dev, size, dma_addr, gfp);
}
EXPORT_SYMBOL(devdrv_dma_alloc_coherent);

void *devdrv_dma_zalloc_coherent(struct device *dev, size_t size, dma_addr_t *dma_addr, gfp_t gfp)
{
    if ((size == 0) || (dev == NULL) || (dma_addr == NULL)) {
        devdrv_err("Input parameter is invalid. (size=%lu)\n", size);
        return NULL;
    }

    return dma_zalloc_coherent(dev, size, dma_addr, gfp);
}
EXPORT_SYMBOL(devdrv_dma_zalloc_coherent);

void devdrv_dma_free_coherent(struct device *dev, size_t size, void *addr, dma_addr_t dma_addr)
{
    if ((dev == NULL) || (addr == NULL) || (size == 0)) {
        devdrv_err("Input parameter is invalid. (size=%lu)\n", size);
        return;
    }

    dma_free_coherent(dev, size, addr, dma_addr);
}
EXPORT_SYMBOL(devdrv_dma_free_coherent);

dma_addr_t devdrv_dma_map_single(struct device *dev, void *ptr, size_t size, enum dma_data_direction dir)
{
    if ((dev == NULL) || (ptr == NULL) || (size == 0)) {
        devdrv_err("Input parameter is invalid. (size=%lu)\n", size);
        return (~(dma_addr_t)0);
    }

    return dma_map_single(dev, ptr, size, dir);
}
EXPORT_SYMBOL(devdrv_dma_map_single);

void devdrv_dma_unmap_single(struct device *dev, dma_addr_t addr, size_t size, enum dma_data_direction dir)
{
    if ((dev == NULL) || (size == 0)) {
        devdrv_err("Input parameter is invalid. (size=%lu)\n", size);
        return;
    }

    dma_unmap_single(dev, addr, size, dir);
}
EXPORT_SYMBOL(devdrv_dma_unmap_single);

dma_addr_t devdrv_dma_map_page(struct device *dev, struct page *page,
    size_t offset, size_t size, enum dma_data_direction dir)
{
    if ((dev == NULL) || (page == NULL) || (size == 0)) {
        devdrv_err("Input parameter is invalid. (size=%lu)\n", size);
        return (~(dma_addr_t)0);
    }

    return dma_map_page(dev, page, offset, size, dir);
}
EXPORT_SYMBOL(devdrv_dma_map_page);

void devdrv_dma_unmap_page(struct device *dev, dma_addr_t addr, size_t size, enum dma_data_direction dir)
{
    if ((dev == NULL) || (size == 0)) {
        devdrv_err("Input parameter is invalid.\n");
        return;
    }

    dma_unmap_page(dev, addr, size, dir);
}
EXPORT_SYMBOL(devdrv_dma_unmap_page);

bool agentdrv_get_dma_urca_err(u32 dev_id)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    u32 chip_id;
    u32 func_id;

    devdrv_dev2chipfunc(dev_id, &chip_id, &func_id);

    agent_dev = agentdrv_get_dev((int)chip_id);
    if ((agent_dev == NULL) || (agentdrv_is_dev_valid(agent_dev, func_id) == false)) {
        devdrv_err("Call agentdrv_get_dev failed, agent_dev is invalid.\n");
        return false;
    }

    if (agent_dev->urca_status == AGENTDRV_DMA_URCA_ERR) {
        agent_dev->urca_status = 0;
        return true;
    }

    return false;
}
EXPORT_SYMBOL(agentdrv_get_dma_urca_err);
