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
#include <linux/of.h>
#include <linux/delay.h>
#include <linux/securec.h>

#include "agentdrv_unit.h"
#include "agentdrv_platform.h"
#include "devdrv_atu.h"
#include "devdrv_util.h"
#include "apb_comm_drv.h"
#include "resource_comm_drv.h"

struct agentdrv_devctrl g_agent_dev_ctrl[MAX_AGENTCHIP_CNT]; /* need chip_id as index */

void devdrv_dev2chipfunc(u32 dev_id, u32 *chip_id, u32 *func_id)
{
    u32 func_totl = agentdrv_res_get_func_total();
#ifdef CFG_FEATURE_SRIOV
    if ((dev_id >= AGENTDRV_SRIOV_VF_DEVID_START) && (devdrv_is_sriov_enabled(func_totl) == true)) {
        *func_id = (dev_id - AGENTDRV_SRIOV_VF_DEVID_START) % (MAX_AGENTFUNC_CNT - 1) + 1;
        *chip_id = (dev_id - AGENTDRV_SRIOV_VF_DEVID_START + 1 - *func_id) / (MAX_AGENTFUNC_CNT - 1);
        return;
    }
#endif
    if (func_totl == 0) {
        *chip_id = (u32)-1;
        *func_id = (u32)-1;
        devdrv_warn("Function is zero, can not get chip_id and func_id by dev_id.\n");
        return;
    } else if (func_totl == 1 || devdrv_is_sriov_enabled(func_totl)) {
        *chip_id = dev_id;
        *func_id = 0;
    } else {
        *chip_id = dev_id / func_totl;
        *func_id = dev_id % func_totl;
    }
}

void devdrv_chipfunc2dev(int *dev_id, int chip_id, int func_id)
{
    u32 func_totl = agentdrv_res_get_func_total();
    if (devdrv_is_sriov_enabled(func_totl)) {
        if (func_id > 0) {
            *dev_id = (chip_id * (MAX_AGENTFUNC_CNT - 1)) + (func_id - 1) + AGENTDRV_SRIOV_VF_DEVID_START;
        } else {
            *dev_id = chip_id;
        }
    } else {
        *dev_id = chip_id * func_totl + func_id;
    }
}

int agentdrv_unit_init(void)
{
    int ret;

    ret = memset_s((void *)g_agent_dev_ctrl, sizeof(g_agent_dev_ctrl), 0, sizeof(g_agent_dev_ctrl));
    if (ret) {
        devdrv_err("Agent unit memset_s failed. (ret=%d)\n", ret);
        return ret;
    }
    return 0;
}

bool agentdrv_is_dev_valid(const struct agentdrv_devctrl *agent_dev, u32 func_id)
{
    if (agent_dev->valid[func_id] == AGENTDRV_ENABLE) {
        return true;
    } else {
        return false;
    }
}

struct agentdrv_devctrl *agentdrv_get_dev_by_apb_dev(const struct platform_device *apb_pdev)
{
    int dev_id;
    struct agentdrv_devctrl *agent_dev = NULL;

    for (dev_id = 0; dev_id < MAX_AGENTCHIP_CNT; dev_id++) {
        if (g_agent_dev_ctrl[dev_id].apb_pdev == apb_pdev) {
            agent_dev = &g_agent_dev_ctrl[dev_id];
            break;
        }
    }

    return agent_dev;
}

struct agentdrv_devctrl *agentdrv_get_dev(int chip_id)
{
    if ((chip_id < 0) || (chip_id >= MAX_AGENTCHIP_CNT)) {
        return NULL;
    }
    return &g_agent_dev_ctrl[chip_id];
}

struct devdrv_dma_dev *devdrv_get_dma_dev(u32 dev_id)
{
    u32 chip_id = 0;
    u32 func_id = 0;
    struct devdrv_dma_dev *dma_dev = NULL;
    struct agentdrv_devctrl *agent_dev = NULL;

    if (dev_id >= MAX_AGENTDEV_CNT) {
        devdrv_err_spinlock("Device ID is error. (dev_id=%u)\n", dev_id);
        return NULL;
    }

    devdrv_dev2chipfunc(dev_id, &chip_id, &func_id);
    agent_dev = agentdrv_get_dev(chip_id);
    if (agent_dev == NULL) {
        devdrv_err_spinlock("Function agentdrv_get_dev failed, agent_dev is null.\n");
        return NULL;
    }
    if (!agentdrv_is_dev_valid(agent_dev, func_id)) {
        devdrv_err_spinlock("Device ID is invalid. (dev_id=%u)\n", dev_id);
        return NULL;
    }
    dma_dev = agent_dev->p_agentdrv_dma_dev[func_id];

    return dma_dev;
}

struct agentdrv_msg_dev *agentdrv_get_msg_dev(u32 dev_id)
{
    u32 chip_id = 0;
    u32 func_id = 0;
    struct agentdrv_devctrl *agent_dev = NULL;

    if (dev_id >= MAX_AGENTDEV_CNT) {
        devdrv_err("Device ID is error. (dev_id=%u)\n", dev_id);
        return NULL;
    }

    devdrv_dev2chipfunc(dev_id, &chip_id, &func_id);
    agent_dev = agentdrv_get_dev(chip_id);
    if (agent_dev == NULL) {
        devdrv_err("Function agentdrv_get_dev failed, agent_dev is null.\n");
        return NULL;
    }
    if (!agentdrv_is_dev_valid(agent_dev, func_id)) {
        devdrv_err("Device ID is invalid. (dev_id=%u)\n", dev_id);
        return NULL;
    }

    return agent_dev->p_agentdrv_msg_dev[func_id];
}

struct agentdrv_platform_dev *agentdrv_get_platform_dev_by_dma_chan(const struct devdrv_dma_channel *dma_chan)
{
    u32 i;
    u32 j;
    u32 chan_count;
    u32 func_id = 0;

    for (i = 0; i < MAX_AGENTCHIP_CNT; i++) {
        if (g_agent_dev_ctrl[i].p_agentdrv_dma_dev[func_id] == NULL) {
            continue;
        }
        chan_count = g_agent_dev_ctrl[i].p_agentdrv_dma_dev[func_id]->local_chan_num;
        for (j = 0; j < chan_count; j++) {
            if (dma_chan == &(g_agent_dev_ctrl[i].p_agentdrv_dma_dev[func_id]->dma_chan[j])) {
                return g_agent_dev_ctrl[i].platform_dev;
            }
        }
    }
    devdrv_err("Got platform device failed.\n");
    return NULL;
}

int agentdrv_get_msix_offset(struct agentdrv_msg_chan *chan)
{
    struct agentdrv_devctrl *agent_dev = NULL;

    if (chan == NULL) {
        devdrv_err_spinlock("Input parameter is invalid.\n");
        return -EINVAL;
    }

    if (chan->msg_dev->func_id >= MAX_AGENTFUNC_CNT) {
        devdrv_err_spinlock("Input parameter is invalid.\n");
        return -EINVAL;
    }

    agent_dev = (struct agentdrv_devctrl *)chan->msg_dev->priv_pci_unit;

    return (int)agent_dev->shr_para[chan->msg_dev->func_id]->msix_offset;
}

struct devdrv_dma_channel *agentdrv_get_dma_chan_by_id(struct agentdrv_devctrl *agent_dev, u32 chan_id, u32 *func_id)
{
    u32 func_total = agentdrv_res_get_func_total();
    struct devdrv_dma_channel *dma_chan = NULL;
    struct devdrv_dma_dev *dma_dev = NULL;
    u32 fun_idx, entry;

    for (fun_idx = 0; fun_idx < func_total; fun_idx++) {
        dma_dev = agent_dev->p_agentdrv_dma_dev[fun_idx];
        if (dma_dev == NULL) {
            continue;
        }
        for (entry = 0; entry < dma_dev->local_chan_num; entry++) {
            dma_chan = &dma_dev->dma_chan[entry];
            if ((chan_id == dma_chan->chan_id) && (dma_chan->sq_desc_base != NULL)) {
                *func_id = dma_dev->func_id;
                return dma_chan;
            }
        }
        for (entry = 0; entry < dma_dev->remote_chan_num; entry++) {
            if (chan_id == dma_dev->remote_chan[entry]) {
                *func_id = dma_dev->func_id;
                return NULL;
            }
        }
    }

    *func_id = U32_MAX;
    return NULL;
}

void agentdrv_check_dma_urca_status(struct agentdrv_devctrl *agent_dev, u32 chan_id)
{
    void __iomem *addr;
    u32 val;

    addr = (void __iomem *)((char *)agent_dev->dma_chan_base + (u64)chan_id * DEVDRV_DMA_CHAN_OFFSET);
    devdrv_dma_reg_rd(addr, DEVDRV_DMA_QUEUE_ERR_STS, &val);

    if ((val & AGENTDRV_DMA_URCA_MASK) != 0) {
        agent_dev->urca_status = AGENTDRV_DMA_URCA_ERR;
    }
}
