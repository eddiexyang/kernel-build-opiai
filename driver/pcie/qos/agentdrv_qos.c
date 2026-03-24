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

#include <linux/securec.h>
#include "agentdrv_unit.h"
#include "devdrv_util.h"
#include "agentdrv_qos.h"
#include "agentdrv_platform.h"

STATIC struct qos_reg g_qos_config = {0};
void agentdrv_set_reg(void __iomem *reg_addr, u32 mask, u32 offset, u32 val)
{
    u32 reg_val = readl(reg_addr);
    reg_val &= ~(mask << offset);
    reg_val |= (val & mask) << offset;
    writel(reg_val, reg_addr);
}

u32 agentdrv_get_reg(const void __iomem *reg_addr, u32 mask, u32 offset)
{
    u32 reg_val = readl(reg_addr);
    return (reg_val >> offset) & mask;
}

STATIC void agentdrv_set_qos_config_to_core(void __iomem *base, u32 core_id, const struct qos_master_config_type *cfg)
{
    void __iomem *iob_base = agentdrv_get_ap_iob_rx_base(base, core_id);

    agentdrv_set_qos(iob_base, cfg->qos);
    agentdrv_set_pmg(iob_base, cfg->pmg);
    agentdrv_set_mpam_id(iob_base, cfg->mpamid);
}

STATIC void agentdrv_get_qos_config_from_core(void __iomem *base, u32 core_id, struct qos_master_config_type *cfg)
{
    void __iomem *iob_base = agentdrv_get_ap_iob_rx_base(base, core_id);

    cfg->qos = agentdrv_get_qos(iob_base);
    cfg->pmg = agentdrv_get_pmg(iob_base);
    cfg->mpamid = agentdrv_get_mpam_id(iob_base);
    cfg->type = MASTER_PCIE;
}

STATIC int agentdrv_save_qos(int dev_id, u32 core_id, const struct qos_master_config_type *cfg)
{
    int ret;

    g_qos_config.qos_config.qos = cfg->qos;
    g_qos_config.qos_config.pmg = cfg->pmg;
    g_qos_config.qos_config.mpamid = cfg->mpamid;
    ret = memcpy_s(g_qos_config.qos_config.bitmap, sizeof(u64) * QOS_MASTER_BITMAP_LEN,
        cfg->bitmap, sizeof(u64) * QOS_MASTER_BITMAP_LEN);
    if (ret != 0) {
        devdrv_err("Memcpy bitmap failed. (dev_id=%d; core_id=%u; ret=%d)\n", dev_id, core_id, ret);
        return ret;
    }
    g_qos_config.qos_config_saved = true;

    return 0;
}

STATIC int agentdrv_set_qos_config(int dev_id, const struct qos_master_config_type *cfg)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    u32 chip_id, func_id, core_id, core_num, dma_chan_id, dma_num;
    int count = 0;
    int ret;

    devdrv_dev2chipfunc((u32)dev_id, &chip_id, &func_id);
    agent_dev = agentdrv_get_dev((int)chip_id);
    if ((agent_dev == NULL) || (agent_dev->apb_base == NULL)) {
        devdrv_err("Set qos config failed, no device. (dev_id=%d; chip_id=%u; func_id=%u)\n",
            dev_id, chip_id, func_id);
        return -ENXIO;
    }
    if (pcie_qos_check_set_qos(dev_id, cfg) != 0) {
        devdrv_err("Check set_qos failed. (dev_id=%d; chip_id=%u; func_id=%u)\n",
            dev_id, chip_id, func_id);
        return -EINVAL;
    }

    core_num = agentdrv_get_pcie_core_num();
    for (core_id = 0; core_id < core_num; core_id++) {
        if ((cfg->bitmap[PCIE_CORE_BITMAP] & (1ULL << core_id)) != 0) {
            agentdrv_set_qos_config_to_core(agent_dev->apb_base, core_id, cfg);
            count++;
        }
    }

    if (count > 0) {
        ret = agentdrv_save_qos(dev_id, core_id, cfg);
        if (ret != 0) {
            return ret;
        }
    }

    dma_num = agentdrv_get_pcie_dma_chan_num();
    for (dma_chan_id = 0; dma_chan_id < dma_num; dma_chan_id++) {
        if ((cfg->bitmap[DMA_CHAN_BITMAP] & (1ULL << dma_chan_id)) != 0) {
            agentdrv_set_qos_config_to_dma(agent_dev->apb_base, dma_chan_id, cfg);
            count++;
        }
    }

    if (count <= 0) {
        devdrv_err("No match bitmap. (dev_id=%d; chip_id=%u; func_id=%u; core_bitmap=0x%llx; dma_bitmap=0x%llx)\n",
                   dev_id, chip_id, func_id, cfg->bitmap[PCIE_CORE_BITMAP], cfg->bitmap[DMA_CHAN_BITMAP]);
        return -EINVAL;
    }
    return 0;
}

STATIC int agentdrv_get_qos_config(int dev_id, struct qos_master_config_type *cfg)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    u32 chip_id, func_id, core_id, core_num, dma_chan_id, dma_num;

    devdrv_dev2chipfunc((u32)dev_id, &chip_id, &func_id);
    agent_dev = agentdrv_get_dev((int)chip_id);
    if ((agent_dev == NULL) || (agent_dev->apb_base == NULL) || (cfg == NULL)) {
        devdrv_err("Set qos config failed, no device. (dev_id=%d; chip_id=%u; func_id=%u)\n",
            dev_id, chip_id, func_id);
        return -ENXIO;
    }

    // For not support bitmap conditions
    if ((cfg->bitmap[PCIE_CORE_BITMAP] == 0) && (cfg->bitmap[DMA_CHAN_BITMAP] == 0)) {
        agentdrv_get_qos_config_from_core(agent_dev->apb_base, 0, cfg);
        return 0;
    }

    core_num = agentdrv_get_pcie_core_num();
    for (core_id = 0; core_id < core_num; core_id++) {
        if ((cfg->bitmap[PCIE_CORE_BITMAP] & (1ULL << core_id)) != 0) {
            agentdrv_get_qos_config_from_core(agent_dev->apb_base, core_id, cfg);
            return 0;
        }
    }

    dma_num = agentdrv_get_pcie_dma_chan_num();
    for (dma_chan_id = 0; dma_chan_id < dma_num; dma_chan_id++) {
        if ((cfg->bitmap[DMA_CHAN_BITMAP] & (1ULL << dma_chan_id)) != 0) {
            agentdrv_get_qos_config_from_dma(agent_dev->apb_base, dma_chan_id, cfg);
            return 0;
        }
    }

    devdrv_err("No match bitmap. (dev_id=%d; chip_id=%u; func_id=%u; core_bitmap=0x%llx; dma_bitmap=0x%llx)\n",
               dev_id, chip_id, func_id, cfg->bitmap[PCIE_CORE_BITMAP], cfg->bitmap[DMA_CHAN_BITMAP]);
    return -EINVAL;
}

STATIC int agentdrv_save_rdallow(int dev_id, const struct qos_allow_config_type *cfg)
{
    int ret;

    g_qos_config.rdallow_config.qos_allow_mode = cfg->qos_allow_mode;
    g_qos_config.rdallow_config.qos_allow_ctrl = cfg->qos_allow_ctrl;
    ret = memcpy_s(g_qos_config.rdallow_config.bitmap, sizeof(u64) * QOS_MASTER_BITMAP_LEN,
        cfg->bitmap, sizeof(u64) * QOS_MASTER_BITMAP_LEN);
    if (ret != 0) {
        devdrv_err("Memcpy bitmap failed. (dev_id=%d; ret=%d)\n", dev_id, ret);
        return ret;
    }
    ret = memcpy_s(g_qos_config.rdallow_config.qos_allow_lvl, sizeof(u64) * MAX_QOS_ALLOW_LEVEL,
        cfg->qos_allow_lvl, sizeof(u64) * MAX_QOS_ALLOW_LEVEL);
    if (ret != 0) {
        devdrv_err("Memcpy allow_lvl failed. (dev_id=%d; ret=%d)\n", dev_id, ret);
        return ret;
    }
    g_qos_config.qos_rdallow_saved = true;
    return 0;
}

STATIC int agentdrv_save_wrallow(int dev_id, const struct qos_allow_config_type *cfg)
{
    int ret;
    g_qos_config.wrallow_config.qos_allow_mode = cfg->qos_allow_mode;
    g_qos_config.wrallow_config.qos_allow_ctrl = cfg->qos_allow_ctrl;
    ret = memcpy_s(g_qos_config.wrallow_config.bitmap, sizeof(u64) * QOS_MASTER_BITMAP_LEN,
        cfg->bitmap, sizeof(u64) * QOS_MASTER_BITMAP_LEN);
    if (ret != 0) {
        devdrv_err("Memcpy bitmap failed. (dev_id=%d; ret=%d)\n", dev_id, ret);
        return ret;
    }
    ret = memcpy_s(g_qos_config.wrallow_config.qos_allow_lvl, sizeof(u64) * MAX_QOS_ALLOW_LEVEL,
        cfg->qos_allow_lvl, sizeof(u64) * MAX_QOS_ALLOW_LEVEL);
    if (ret != 0) {
        devdrv_err("Memcpy allow_lvl failed. (dev_id=%d;  ret=%d)\n", dev_id, ret);
        return ret;
    }
    g_qos_config.qos_wrallow_saved = true;
    return 0;
}

STATIC int agentdrv_save_allow(int dev_id, const struct qos_allow_config_type *cfg)
{
    int ret = 0;

    switch (cfg->qos_allow_ctrl) {
        case PCIE_QOS_ALLOW_CTRL_READ:
            ret = agentdrv_save_rdallow(dev_id, cfg);
            break;
        case PCIE_QOS_ALLOW_CTRL_WRITE:
            ret = agentdrv_save_wrallow(dev_id, cfg);
            break;
        default:
            ret = agentdrv_save_rdallow(dev_id, cfg);
            if (ret != 0) {
                return ret;
            }
            ret = agentdrv_save_wrallow(dev_id, cfg);
            if (ret != 0) {
                return ret;
            }
            break;
    }
    return ret;
}

STATIC bool check_sche_base(const struct agentdrv_devctrl *agent_dev)
{
    if ((agent_dev == NULL) || (agent_dev->platform_dev == NULL) ||
        (agent_dev->platform_dev->sche_base == NULL)) {
            return false;
    }
    return true;
}

STATIC int agentdrv_set_allow_config(int dev_id, const struct qos_allow_config_type *cfg)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    u32 chip_id, func_id, core_id, core_num;
    int count = 0;
    int ret;
    void __iomem *sche_base = NULL;

    devdrv_dev2chipfunc((u32)dev_id, &chip_id, &func_id);
    agent_dev = agentdrv_get_dev((int)chip_id);
    if (check_sche_base(agent_dev) == false) {
        devdrv_err("Set allow_config failed, no device. (dev_id=%d; chip_id=%u; func_id=%u)\n",
            dev_id, chip_id, func_id);
        return -ENXIO;
    }

    if (pcie_qos_check_set_allow(dev_id, cfg) != 0) {
        devdrv_err("Check set_allowe failed. (dev_id=%d; chip_id=%u; func_id=%u)\n",
            dev_id, chip_id, func_id);
        return -EINVAL;
    }

    core_num = agentdrv_get_pcie_core_num();
    sche_base = agent_dev->platform_dev->sche_base;
    for (core_id = 0; core_id < core_num; core_id++) {
        if ((cfg->bitmap[PCIE_CORE_BITMAP] & (1ULL << core_id)) != 0) {
            agentdrv_set_allow_to_core(sche_base, core_id, cfg);
            count++;
        }
    }
    if (count <= 0) {
        devdrv_err("No match bitmap. (dev_id=%d; chip_id=%u; func_id=%u; core_bitmap=0x%llx; dma_bitmap=0x%llx)\n",
                   dev_id, chip_id, func_id, cfg->bitmap[PCIE_CORE_BITMAP], cfg->bitmap[DMA_CHAN_BITMAP]);
        return -EINVAL;
    }

    ret = agentdrv_save_allow(dev_id, cfg);
    if (ret != 0) {
        devdrv_err("Save allow_config failed. (dev_id=%d; chip_id=%u; func_id=%u)\n", dev_id, chip_id, func_id);
        return ret;
    }

    return 0;
}

STATIC int agentdrv_get_allow_config(int dev_id, struct qos_allow_config_type *cfg)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    u32 chip_id, func_id, core_id, core_num;
    int ret;
    void __iomem *sche_base = NULL;

    devdrv_dev2chipfunc((u32)dev_id, &chip_id, &func_id);
    agent_dev = agentdrv_get_dev((int)chip_id);
    if (check_sche_base(agent_dev) == false) {
        devdrv_err("Get qos_allow config failed, no device. (dev_id=%d; chip_id=%u; func_id=%u)\n",
            dev_id, chip_id, func_id);
        return -ENXIO;
    }

    ret = pcie_qos_check_get_allow(dev_id, cfg);
    if (ret != 0) {
        devdrv_err("Check qos_allow failed. (dev_id=%d; chip_id=%u; func_id=%u)\n", dev_id, chip_id, func_id);
        return ret;
    }

    sche_base = agent_dev->platform_dev->sche_base;

    // For not support bitmap conditions
    if ((cfg->bitmap[PCIE_CORE_BITMAP] == 0) && (cfg->bitmap[DMA_CHAN_BITMAP] == 0)) {
        agentdrv_get_allow_from_core(sche_base, 0, cfg);
        return 0;
    }

    core_num = agentdrv_get_pcie_core_num();
    for (core_id = 0; core_id < core_num; core_id++) {
        if ((cfg->bitmap[PCIE_CORE_BITMAP] & (1ULL << core_id)) != 0) {
            agentdrv_get_allow_from_core(sche_base, core_id, cfg);
            return 0;
        }
    }

    devdrv_err("No match bitmap. (dev_id=%d; chip_id=%u; func_id=%u; core_bitmap=0x%llx; dma_bitmap=0x%llx)\n",
               dev_id, chip_id, func_id, cfg->bitmap[PCIE_CORE_BITMAP], cfg->bitmap[DMA_CHAN_BITMAP]);
    return -EINVAL;
}

STATIC int agentdrv_save_otsd(int dev_id, const struct qos_otsd_config_type *cfg)
{
    int ret;
    ret = memcpy_s(g_qos_config.ost_config.otsd_lvl, sizeof(u32)*MAX_OTSD_LEVEL,
        cfg->otsd_lvl, sizeof(u32)*MAX_OTSD_LEVEL);
    if (ret != 0) {
        devdrv_err("Save qos_otsd_lvl failed. (dev_id=%d; ret=%d)", dev_id, ret);
        return ret;
    }
    g_qos_config.ost_config.otsd_mode = cfg->otsd_mode;
    g_qos_config.ost_config.master = cfg->master;
    ret = memcpy_s(g_qos_config.ost_config.bitmap, sizeof(u64)*QOS_MASTER_BITMAP_LEN,
        cfg->bitmap, sizeof(u64)*QOS_MASTER_BITMAP_LEN);
    if (ret != 0) {
        devdrv_err("Save qos_otsd_bitmap failed. (dev_id=%d; ret=%d)", dev_id, ret);
        return ret;
    }
    g_qos_config.qos_otsd_saved = true;

    return 0;
}

STATIC int agentdrv_set_otsd_config(int dev_id, const struct qos_otsd_config_type *cfg)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    u32 chip_id, func_id, core_id, core_num;
    int count = 0;
    void __iomem *sche_base = NULL;

    devdrv_dev2chipfunc((u32)dev_id, &chip_id, &func_id);
    agent_dev = agentdrv_get_dev((int)chip_id);
    if (check_sche_base(agent_dev) == false) {
        devdrv_err("Set qos_otsd config failed, no device. (dev_id=%d; chip_id=%u; func_id=%u)\n",
            dev_id, chip_id, func_id);
        return -ENXIO;
    }

    if (pcie_qos_check_set_otsd(dev_id, cfg) != 0) {
        devdrv_err("Check set_otsd failed. (dev_id=%d)\n", dev_id);
        return -EINVAL;
    }

    core_num = agentdrv_get_pcie_core_num();
    sche_base = agent_dev->platform_dev->sche_base;
    for (core_id = 0; core_id < core_num; core_id++) {
        if ((cfg->bitmap[PCIE_CORE_BITMAP] & (1ULL << core_id)) != 0) {
            agentdrv_set_otsd_to_core(sche_base, core_id, cfg);
            count++;
        }
    }

    if (count <= 0) {
        devdrv_err("No match bitmap. (dev_id=%d; chip_id=%u; func_id=%u; core_bitmap=0x%llx; dma_bitmap=0x%llx)\n",
                   dev_id, chip_id, func_id, cfg->bitmap[PCIE_CORE_BITMAP], cfg->bitmap[DMA_CHAN_BITMAP]);
        return -EINVAL;
    }
    (void)agentdrv_save_otsd(dev_id, cfg);
    return 0;
}

STATIC int agentdrv_get_otsd_config(int dev_id, struct qos_otsd_config_type *cfg)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    u32 chip_id, func_id, core_id, core_num;
    void __iomem *sche_base = NULL;

    devdrv_dev2chipfunc((u32)dev_id, &chip_id, &func_id);
    agent_dev = agentdrv_get_dev((int)chip_id);
    if ((check_sche_base(agent_dev) == false) || (cfg == NULL)) {
        devdrv_err("Get qos_allow config failed, no device. (dev_id=%d; chip_id=%u; func_id=%u)\n",
            dev_id, chip_id, func_id);
        return -ENXIO;
    }

    sche_base = agent_dev->platform_dev->sche_base;

    // For not support bitmap conditions
    if ((cfg->bitmap[PCIE_CORE_BITMAP] == 0) && (cfg->bitmap[DMA_CHAN_BITMAP] == 0)) {
        agentdrv_get_otsd_from_core(sche_base, 0, cfg);
        return 0;
    }

    core_num = agentdrv_get_pcie_core_num();
    for (core_id = 0; core_id < core_num; core_id++) {
        if ((cfg->bitmap[PCIE_CORE_BITMAP] & (1ULL << core_id)) != 0) {
            agentdrv_get_otsd_from_core(sche_base, core_id, cfg);
            return 0;
        }
    }

    devdrv_err("No match bitmap. (dev_id=%d; chip_id=%u; func_id=%u; core_bitmap=0x%llx; dma_bitmap=0x%llx)\n",
               dev_id, chip_id, func_id, cfg->bitmap[PCIE_CORE_BITMAP], cfg->bitmap[DMA_CHAN_BITMAP]);
    return -EINVAL;
}

STATIC void agentdrv_resume_qos_config_to_core(void __iomem *base, u32 core_id)
{
    void __iomem *iob_base = agentdrv_get_ap_iob_rx_base(base, core_id);

    if (!g_qos_config.qos_config_saved) {
        devdrv_info("Qos config not save, when agentdrv resume qos config. (core_id=%u)\n", core_id);
        return;
    }

    if ((g_qos_config.qos_config.bitmap[PCIE_CORE_BITMAP] & (1ULL << core_id)) != 0) {
        agentdrv_set_qos(iob_base, g_qos_config.qos_config.qos);
        agentdrv_set_pmg(iob_base, g_qos_config.qos_config.pmg);
        agentdrv_set_mpam_id(iob_base, g_qos_config.qos_config.mpamid);
    }
    return;
}

STATIC void agentdrv_resume_rdallow_config_to_core(void __iomem *base, u32 core_id)
{
    if (!g_qos_config.qos_rdallow_saved) {
        devdrv_info("Qos config not save, when agentdrv resume rdallow config. (core_id=%u)\n", core_id);
        return;
    }

    if ((g_qos_config.rdallow_config.bitmap[PCIE_CORE_BITMAP] & (1ULL << core_id)) != 0) {
        agentdrv_set_allow_to_core(base, core_id, &g_qos_config.rdallow_config);
    }
    return;
}

STATIC void agentdrv_resume_wrallow_config_to_core(void __iomem *base, u32 core_id)
{
    if (!g_qos_config.qos_wrallow_saved) {
        devdrv_info("Qos config not save, when agentdrv resume wrallow config. (core_id=%u)\n", core_id);
        return;
    }

    if ((g_qos_config.wrallow_config.bitmap[PCIE_CORE_BITMAP] & (1ULL << core_id)) != 0) {
        agentdrv_set_allow_to_core(base, core_id, &g_qos_config.wrallow_config);
    }
    return;
}

STATIC void agentdrv_resume_otsd_config_to_core(void __iomem *base, u32 core_id)
{
    if (!g_qos_config.qos_otsd_saved) {
        devdrv_info("Qos config not save, when agentdrv resume otsd config. (core_id=%u)\n", core_id);
        return;
    }

    if ((g_qos_config.ost_config.bitmap[PCIE_CORE_BITMAP] & (1ULL << core_id)) != 0) {
        agentdrv_set_otsd_to_core(base, core_id, &g_qos_config.ost_config);
    }
    return;
}

void agentdrv_resume_qos_config(void)
{
    struct agentdrv_devctrl *agent_dev = NULL;
    int chip_id = 0;
    u32 core_num, core_id;
    void __iomem *sche_base = NULL;
    agent_dev = agentdrv_get_dev(chip_id);
    if ((agent_dev == NULL) || (agent_dev->apb_base == NULL) || (agent_dev->platform_dev == NULL) ||
        (agent_dev->platform_dev->sche_base == NULL)) {
        devdrv_err("Resume qos config failed, point is NULL.\n");
        return;
    }

    devdrv_info("Start resume qos config.\n");
    sche_base = agent_dev->platform_dev->sche_base;
    core_num = agentdrv_get_pcie_core_num();
    for (core_id = 0; core_id < core_num; core_id++) {
        agentdrv_resume_qos_config_to_core(agent_dev->apb_base, core_id);
        agentdrv_resume_rdallow_config_to_core(sche_base, core_id);
        agentdrv_resume_wrallow_config_to_core(sche_base, core_id);
        agentdrv_resume_otsd_config_to_core(sche_base, core_id);
    }

    devdrv_info("Resume qos config success.\n");
    return;
}
EXPORT_SYMBOL(agentdrv_resume_qos_config);

void hal_kernel_agentdrv_get_qos_func(struct qos_master_node *qos_master)
{
    if (qos_master == NULL) {
        devdrv_err("Pcie qos register failed, qos_master_node is NULL.\n");
        return;
    }

    qos_master->set = agentdrv_set_qos_config;
    qos_master->get = agentdrv_get_qos_config;
    qos_master->set_allow = agentdrv_set_allow_config;
    qos_master->get_allow = agentdrv_get_allow_config;
    qos_master->set_otsd = agentdrv_set_otsd_config;
    qos_master->get_otsd = agentdrv_get_otsd_config;
}
EXPORT_SYMBOL(hal_kernel_agentdrv_get_qos_func);