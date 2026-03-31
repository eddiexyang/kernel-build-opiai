/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
 * Create: 2022-01-15
 */
#include <asm/io.h>
#include <linux/kthread.h>
#include <linux/delay.h>

#include "agentdrv_flr.h"
#include "devdrv_util.h"
#include "resource_comm_drv.h"
#include "dma_comm_drv.h"
#include "agentdrv_ctrl.h"
#include "agentdrv_msg.h"

#define AGENTDRV_VF_FLR_INT_MASK_VAL ((0x1U << AGENTDRV_VF_NUM_PER_AGENT_DEV) - 1)

#define AGENTDRV_FLR_INDEX_MASK 0x1U
#define AGENTDRV_FLR_INDEX_OFFSET 1

#define AGENTDRV_OPS_SET 0x1
#define AGENTDRV_OPS_CLEAR 0x0

enum agentdrv_flr_result_type {
    AGENTDRV_FLR_SUCCEEDED = 0,
    AGENTDRV_FLR_FAILED
};

struct agentdrv_thread_data {
    struct agentdrv_devctrl *agent_dev;
    u32 thread_vf_id;
};

#define AGENTDRV_FLR_STOP_BUSINESS_MAX_TIME_S 1

void agentdrv_handle_pcie_flr_mask(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;

    reg_addr = p_dev->io_base + AGENTDRV_TL_CORE_PF0_VF_REG + AGENTDRV_VF_FLR_INT_MASK;
    writel(0xffffffff, reg_addr);

    reg_addr = p_dev->io_base + AGENTDRV_CORE_GLOBAL_CTRL_REG + AGENTDRV_CORE_INT_NI_MSK_0;
    writel(0x00ffffff, reg_addr);

    reg_addr = p_dev->io_base + AGENTDRV_CORE_GLOBAL_CTRL_REG + AGENTDRV_CORE_INT_NI_MSK_1;
    writel(0x07ffffff, reg_addr);

    reg_addr = p_dev->io_base + AGENTDRV_AP_GLOBAL_REG + AGENTDRV_PCIE_NI_MASK;
    writel(0x1, reg_addr);
}

void agentdrv_handle_pcie_flr_unmask(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;
    u32 reg_val;

    reg_addr = p_dev->io_base + AGENTDRV_TL_CORE_PF0_VF_REG + AGENTDRV_VF_FLR_INT_MASK;
    reg_val = readl(reg_addr);
    reg_val &= ~AGENTDRV_VF_FLR_INT_MASK_VAL;
    writel(reg_val, reg_addr);

    reg_addr = p_dev->io_base + AGENTDRV_CORE_GLOBAL_CTRL_REG + AGENTDRV_CORE_INT_NI_MSK_0;
    writel(0x0, reg_addr);

    reg_addr = p_dev->io_base + AGENTDRV_CORE_GLOBAL_CTRL_REG + AGENTDRV_CORE_INT_NI_MSK_1;
    writel(0x0, reg_addr);

    reg_addr = p_dev->io_base + AGENTDRV_AP_GLOBAL_REG + AGENTDRV_PCIE_NI_MASK;
    writel(0x0, reg_addr);
}

STATIC void agentdrv_clear_ap_status(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;
    u32 reg_val;

    reg_addr = p_dev->io_base + AGENTDRV_AP_GLOBAL_REG + AGENTDRV_PCIE_CORE_NI_STATUS;
    reg_val = readl(reg_addr);
    writel(reg_val, reg_addr);
}

void agentdrv_flr_clear_status(struct agentdrv_platform_dev *p_dev)
{
    void __iomem *reg_addr = NULL;
    u32 reg_val;

    reg_addr = p_dev->io_base + AGENTDRV_TL_CORE_PF0_VF_REG + AGENTDRV_VF_FLR_INT_STATUS;
    reg_val = readl(reg_addr);
    writel(reg_val, reg_addr);

    agentdrv_clear_ap_status(p_dev);
}

STATIC void agentdrv_get_flr_index(u32 status_reg_val, u32 *id_array, int len, u32 *vf_num)
{
    u32 i = 0;
    u32 num = 0;

    for (i = 0; i < (u32)len; i++) {
        if (status_reg_val & AGENTDRV_FLR_INDEX_MASK) {
            id_array[num++] = i;
        }
        status_reg_val >>= AGENTDRV_FLR_INDEX_OFFSET;
    }

    *vf_num = num;
}

STATIC int agentdrv_device_flr_stop_business(struct agentdrv_devctrl *agent_dev, u32 func_id)
{
    agentdrv_dev_flr_uninstance(agent_dev, (int)func_id);
    return 0;
}

STATIC int agentdrv_flr_vf_threadfn(void *data)
{
    struct agentdrv_thread_data *thread_data = (struct agentdrv_thread_data *)data;
    struct agentdrv_devctrl *agent_dev = thread_data->agent_dev;
    u32 agent_id = agent_dev->agent_id;
    u32 thread_vf_id = thread_data->thread_vf_id;
    int *thread_state = agent_dev->platform_dev->flr.thread_state;
    u32 func_id;
    int ret;

    func_id = agent_id * AGENTDRV_PFVF_NUM_PER_AGENT_DEV + thread_vf_id + 1;
    ret = agentdrv_device_flr_stop_business(agent_dev, func_id);
    if (ret) {
        devdrv_err("Device_stop_flr_business failed. (dev_id=%u; vf_id=%u; func_id=%u; ret=%d)\n",
            agent_id, thread_vf_id, func_id, ret);
        thread_state[thread_vf_id] = AGENTDRV_THREAD_STATE_ERROR;
        return -EIO;
    }

    thread_state[thread_vf_id] = AGENTDRV_THREAD_STATE_END;
    return 0;
}

STATIC void agentdrv_flr_stop_business(struct agentdrv_devctrl *agent_dev, const u32 *vf_id_array, u32 vf_num)
{
    struct agentdrv_platform_dev *p_dev = agent_dev->platform_dev;
    int agent_id = p_dev->agent_id;
    u32 vf_id, i;
    struct agentdrv_thread_data thread_data[AGENTDRV_VF_NUM_PER_AGENT_DEV] = {{0}};

    for (i = 0; i < vf_num; i++) {
        vf_id = vf_id_array[i];
        p_dev->flr.flr_state[vf_id] = AGENTDRV_FLR_STATE_FLRING;
        p_dev->flr.thread_state[vf_id] = AGENTDRV_THREAD_STATE_START;

        thread_data[vf_id].agent_dev = agent_dev;
        thread_data[vf_id].thread_vf_id = vf_id;
        p_dev->flr.task[vf_id] = kthread_run(agentdrv_flr_vf_threadfn, &thread_data[vf_id], "flr_vf%u_threadfn", vf_id);
        if (IS_ERR(p_dev->flr.task[vf_id])) {
            p_dev->flr.flr_state[vf_id] = AGENTDRV_FLR_STATE_THREAD_RUN_FAIL;
            devdrv_err("Kthread run failed. (dev_id=%d; vf_id=%u)\n", agent_id, vf_id);
        }
    }

    ssleep(AGENTDRV_FLR_STOP_BUSINESS_MAX_TIME_S);
}

STATIC void agentdrv_flr_excep_print(struct agentdrv_platform_dev *p_dev)
{
    int agent_id = p_dev->agent_id;
    struct agentdrv_flr_info *flr_info = &(p_dev->flr.flr_info);
    int i;

    devdrv_warn("Flr_excep info: (dev_id=%d)\n", agent_id);
    for (i = 0; i < AGENTDRV_VF_NUM_PER_AGENT_DEV; i++) {
        devdrv_warn("Vf info. (vf_id=%d; vf_flr_sum_cnt=%llu; vf_flr_excep_cnt=%llu)\n", i,
            flr_info->vf_flr_sum_cnt[i], flr_info->vf_flr_fail_cnt[i]);
    }
}

STATIC void agentdrv_flr_record(struct agentdrv_platform_dev *p_dev, u32 vf_id,
    enum agentdrv_flr_result_type result_type)
{
    struct agentdrv_flr_info *flr_info = &(p_dev->flr.flr_info);

    flr_info->vf_flr_sum_cnt[vf_id]++;

    if (result_type == AGENTDRV_FLR_FAILED) {
        flr_info->vf_flr_fail_cnt[vf_id]++;
    }
}

STATIC int agentdrv_flr_dma_reset(struct agentdrv_devctrl *agent_dev, u32 vf_id)
{
    struct devdrv_dma_dev *dma_dev = NULL;
    u32 dev_id = agent_dev->agent_id;
    void __iomem *io_base = NULL;
    u32 i;
    int ret;

    dma_dev = agent_dev->p_agentdrv_dma_dev[vf_id + 1];
    if (dma_dev == NULL) {
        devdrv_warn("Dma_dev has been released. (dev_id=%u; vf_id=%u)\n", dev_id, vf_id);
        return -EINVAL;
    }

    /* remote dma_chan */
    for (i = 0; i < dma_dev->remote_chan_num; i++) {
        io_base = dma_dev->dma_chan_base + dma_dev->remote_chan[i] * DEVDRV_DMA_CHAN_OFFSET;
        ret = devdrv_dma_ch_cfg_reset(io_base, DEVDRV_SRIOV_DISABLE);
        if (ret != 0) {
            devdrv_err("Dma_ch_cfg_reset remote_schan failed. (dev_id=%u; vf_id=%u; dma_chan=%u; ret=%d)\n",
                dev_id, vf_id, i, ret);
            return ret;
        }
    }

    /* local dma_chan status check */
    for (i = 0; i < dma_dev->local_chan_num; i++) {
        io_base = dma_dev->dma_chan_base + dma_dev->local_chan[i] * DEVDRV_DMA_CHAN_OFFSET;
        ret = devdrv_dma_check_fsm_sts(io_base, DEVDRV_DMA_TIMEOUT);
        if (ret != 0) {
            devdrv_warn("devdrv_dma_check_fsm_sts is not idle. (dev_id=%u; vf_id=%u)\n", dev_id, vf_id);
        }
    }

    return 0;
}

STATIC void agentdrv_check_thread_result(struct agentdrv_platform_dev *p_dev, u32 vf_id)
{
    int agent_id = p_dev->agent_id;
    enum agentdrv_flr_state *flr_state = p_dev->flr.flr_state;
    int *thread_state = p_dev->flr.thread_state;

    if (flr_state[vf_id] == AGENTDRV_FLR_STATE_FLRING) {
        if (thread_state[vf_id] == AGENTDRV_THREAD_STATE_START) {
            flr_state[vf_id] = AGENTDRV_FLR_STATE_STOP_BUSINESS_TIMEOUT;
            devdrv_warn("Flr stop_business timeout. (dev_id=%d; vf_id=%u)\n", agent_id, vf_id);
        } else if (thread_state[vf_id] == AGENTDRV_THREAD_STATE_ERROR) {
            flr_state[vf_id] = AGENTDRV_FLR_STATE_STOP_BUSINESS_ERROR;
            devdrv_err("Flr stop_business error. (dev_id=%d; vf_id=%u)\n", agent_id, vf_id);
        }
    }
}

STATIC void agentdrv_vf_flr_en_bit_ops(struct agentdrv_platform_dev *p_dev, u32 vf_id_per_pf, u8 is_set)
{
    void __iomem *reg_addr = NULL;
    u32 reg_val;

    reg_addr = p_dev->io_base + AGENTDRV_TL_CORE_PF0_VF_REG + AGENTDRV_VF_FLR_EN;
    reg_val = readl(reg_addr);
    if (!!is_set) {
        reg_val |= AGENTDRV_FLR_INDEX_MASK << vf_id_per_pf;
    } else {
        reg_val &= ~(AGENTDRV_FLR_INDEX_MASK << vf_id_per_pf);
    }
    writel(reg_val, reg_addr);
}

STATIC void agentdrv_flr_state_update(struct agentdrv_platform_dev *p_dev, const u32 *vf_id_array, u32 vf_num)
{
    u32 vf_id, i;
    enum agentdrv_flr_result_type total_result_type = AGENTDRV_FLR_SUCCEEDED;

    for (i = 0; i < vf_num; i++) {
        vf_id = vf_id_array[i];
        if (p_dev->flr.flr_state[vf_id] == AGENTDRV_FLR_STATE_FLRING) {
            p_dev->flr.flr_state[vf_id] = AGENTDRV_FLR_STATE_OK;
            agentdrv_flr_record(p_dev, vf_id, AGENTDRV_FLR_SUCCEEDED);
        } else {
            agentdrv_flr_record(p_dev, vf_id, AGENTDRV_FLR_FAILED);
            total_result_type = AGENTDRV_FLR_FAILED;
        }
    }

    if (total_result_type == AGENTDRV_FLR_FAILED) {
        agentdrv_flr_excep_print(p_dev);
    }
}

void agentdrv_flr_task(struct work_struct *work)
{
    struct agentdrv_flr *flr = container_of(work, struct agentdrv_flr, flr_work);
    struct agentdrv_platform_dev *p_dev = container_of(flr, struct agentdrv_platform_dev, flr);
    struct agentdrv_devctrl *agent_dev = NULL;
    int agent_id = p_dev->agent_id;
    void __iomem *vf_status_reg = p_dev->io_base + AGENTDRV_TL_CORE_PF0_VF_REG + AGENTDRV_VF_FLR_INT_STATUS;
    u32 vf_id_array[AGENTDRV_VF_NUM_PER_AGENT_DEV] = {0};
    u32 vf_status_reg_val, vf_id, i;
    u32 vf_num = 0;
    int ret;

    vf_status_reg_val = readl(vf_status_reg);
    if (vf_status_reg_val == 0) {
        devdrv_info("No vf flr int. (dev_id=%d)\n", agent_id);
        goto no_vf_flr_int;
    }

    agentdrv_get_flr_index(vf_status_reg_val, vf_id_array, AGENTDRV_VF_NUM_PER_AGENT_DEV, &vf_num);

    agent_dev = agentdrv_get_dev(agent_id);
    if (agent_dev == NULL) {
        devdrv_err("Agentdrv_get_dev failed. (dev_id=%d)\n", agent_id);
        return;
    }

    agentdrv_flr_stop_business(agent_dev, vf_id_array, vf_num);

    for (i = 0; i < vf_num; i++) {
        vf_id = vf_id_array[i];
        devdrv_set_dma_status(agent_dev->p_agentdrv_dma_dev[vf_id + 1], DEVDRV_DMA_DEAD);
        agentdrv_check_thread_result(p_dev, vf_id);

        agentdrv_msg_release(agent_dev, vf_id + 1);

        agentdrv_vf_flr_en_bit_ops(p_dev, vf_id, AGENTDRV_OPS_SET);
        ret = agentdrv_flr_dma_reset(agent_dev, vf_id);
        if (ret && (flr->flr_state[vf_id] == AGENTDRV_FLR_STATE_FLRING)) {
            flr->flr_state[vf_id] = AGENTDRV_FLR_STATE_DMA_ERROR;
            devdrv_warn("Flr_dma_ops skip. (dev_id=%d; vf_id=%u; ret=%d)\n", agent_id, vf_id, ret);
        }
        agentdrv_vf_flr_en_bit_ops(p_dev, vf_id, AGENTDRV_OPS_CLEAR);

        writel(AGENTDRV_FLR_INDEX_MASK << vf_id, vf_status_reg);
        devdrv_set_dma_status(agent_dev->p_agentdrv_dma_dev[vf_id + 1], DEVDRV_DMA_ALIVE);
    }

no_vf_flr_int:
    agentdrv_clear_ap_status(p_dev);

    agentdrv_flr_state_update(p_dev, vf_id_array, vf_num);

    agentdrv_handle_pcie_flr_unmask(p_dev);
}
