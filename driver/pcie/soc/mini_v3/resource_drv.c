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

#include <linux/property.h>
#include "nvme_drv.h"

#include "resource_drv.h"
#include "devdrv_msg_def.h"
#include "dma_drv.h"

#ifdef CFG_SOC_PLATFORM_MDC_V11
#  define DEVDRV_MAX_DMA_CH_SQ_DEPTH   0x5000
#  define DEVDRV_MAX_DMA_CH_CQ_DEPTH   0x5000
#else
#  define DEVDRV_MAX_DMA_CH_SQ_DEPTH   0x10000
#  define DEVDRV_MAX_DMA_CH_CQ_DEPTH   0x10000
#endif
#define DEVDRV_DMA_CH_SQ_DESC_RSV    0x400

void agentdrv_res_pf_dma_chan(struct devdrv_dma_dev *dma_dev)
{
    u32 i;

    dma_dev->local_chan_num = DMA_CHAN_LOCAL_USED_NUM;
    for (i = 0; i < dma_dev->local_chan_num; i++) {
        dma_dev->local_chan[i] = DMA_CHAN_LOCAL_USED_START_INDEX + i;
    }

    dma_dev->remote_chan_num = DMA_CHAN_REMOTE_USED_NUM;
    for (i = 0; i < dma_dev->remote_chan_num; i++) {
        dma_dev->remote_chan[i] = DMA_CHAN_REMOTE_USED_START_INDEX + i;
    }

    dma_dev->ts_chan_num = DMA_CHAN_TS_USED_NUM;
    for (i = 0; i < dma_dev->ts_chan_num; i++) {
        dma_dev->ts_chan[i] = DMA_CHAN_TS_USED_START_INDEX + i;
    }
}

void agentdrv_dma_res_common(u32 func_id, u32 dma_irq_base, struct devdrv_res_dma_common_info *out)
{
    out->chan_num = DMA_CHAN_LOCAL_USED_NUM;
    out->done_irq_base = dma_irq_base + DMA_DONE_IRQ_BASE;
    out->err_irq_base = dma_irq_base + DMA_ERR_IRQ_BASE;
    /* cloud only one err irq */
    out->err_flag = (DMA_ERR_IRQ_NUM == 1) ? 0 : 1;
    out->sq_cq_info.sq_depth = DEVDRV_MAX_DMA_CH_SQ_DEPTH;
    out->sq_cq_info.sq_rsv_num = DEVDRV_DMA_CH_SQ_DESC_RSV;
    out->sq_cq_info.cq_depth = DEVDRV_MAX_DMA_CH_CQ_DEPTH;
}

void agentdrv_get_dma_max_msg_chan(u32 func_id, u32 *chan_num)
{
    *chan_num = AGENTDRV_MAX_IO_MSG_CHAN;
}

void agentdrv_get_p2p_msg_db_range(u32 func_id, u32 *p2p_db_start, u32 *p2p_db_end)
{
    *p2p_db_start = AGENTDRV_P2P_MSG_USED_DB_START;
    *p2p_db_end = AGENTDRV_P2P_MSG_USED_DB_END;
}

STATIC phys_addr_t agentdrv_get_shr_base(void)
{
    u32 board_type, version;
    u32 __iomem *sc_ver = NULL;
    static phys_addr_t shr_base = 0;
    if (shr_base != 0) {
        return shr_base;
    }

    sc_ver = ioremap(DEVDRV_SYSCTRL_SC_VER, sizeof(u32));
    if (sc_ver == NULL) {
        shr_base = DEVDRV_SHR_PARA_ADDR;
        devdrv_err("Read SYSCTRL_SC_VER failed\n");
        return shr_base;
    }

    board_type = (*sc_ver & DEVDRV_BOARD_TYPE_MASK) >> DEVDRV_BOARD_TYPE_OFFSET;
    version = *sc_ver & DEVDRV_VERSION_MASK;

    if ((board_type == DEVDRV_BOARD_TYPE_ASIC_FPGA) && (version != DEVDRV_VERSION_ASIC)) {
        shr_base = DEVDRV_SHR_PARA_ADDR_FPGA;
        devdrv_info("Board type is FPGA. (board_type=0x%x; version=0x%x)\n", board_type, version);
    } else if (board_type > DEVDRV_BOARD_TYPE_ESL) {
        shr_base = DEVDRV_SHR_PARA_ADDR;
        devdrv_err("Invalid board type. (board_type=0x%x; version=0x%x)\n", board_type, version);
    } else {
        shr_base = DEVDRV_SHR_PARA_ADDR;
        devdrv_info("Getting board type. (board_type=0x%x; version=0x%x)\n", board_type, version);
    }

    iounmap(sc_ver);
    return shr_base;
}

void agentdrv_res_sdi_addr(u32 dev_id, u32 func_id, struct agent_res_sdi_addr *sdi_addr)
{
    sdi_addr->msg_size = DEVDRV_PCIE_RESERVE_MEM_SIZE;
    sdi_addr->msg_base = dev_id * DEVDRV_CHIP_ADDR_SPACE_SIZE + DEVDRV_PCIE_RESERVE_MEM_ADDR;
    sdi_addr->db_size = AGENTDRV_DB_MEM_SIZE;
    sdi_addr->db_base = dev_id * DEVDRV_CHIP_ADDR_SPACE_SIZE + AGENTDRV_DB_MEM_ADDRESS;
    sdi_addr->shr_size = DEVDRV_SHR_PARA_ADDR_SIZE;
    sdi_addr->shr_base = dev_id * DEVDRV_CHIP_ADDR_SPACE_SIZE + agentdrv_get_shr_base();
    sdi_addr->shr_type = DEVDRV_SHR_MEM_NORMAL;
    sdi_addr->bw_ctrl_size = 0;
    sdi_addr->bw_ctrl_base = 0;
    sdi_addr->rsv_mem_size = sdi_addr->msg_size;
    sdi_addr->rsv_mem_base = sdi_addr->msg_base;
    sdi_addr->raise_int_size = AGENTDRV_SOC_DB_RAISE_INIT_SIZE;
    sdi_addr->raise_int_base = AGENTDRV_SOC_DB_RAISE_INIT_ADDR;
    sdi_addr->db_state_size = AGENTDRV_SOC_DB_IRQ_SIZE;
    sdi_addr->db_state_base = AGENTDRV_SOC_DB_IRQ_ADDR;
    sdi_addr->topic_sched_resmem_size = 0;
    sdi_addr->topic_sched_resmem_base = 0;
}

int agentdrv_ioremap_doorbell_base(void __iomem **doorbell_base, phys_addr_t db_base, u64 db_size)
{
    return 0;
}

void agentdrv_res_db_queue_id2func_id(u32 db_id, u32 *db_id_in_func)
{
    *db_id_in_func = db_id;
}

void agentdrv_res_dma_chan_id2func_id(u32 dma_chan_id, u32 *func_id, u32 *dma_chan_id_in_func)
{
    *func_id = 0;
    *dma_chan_id_in_func = dma_chan_id;
}

void __iomem *agentdrv_get_phy_match_flag_addr(struct agentdrv_devctrl *agent_dev, u32 func_idx)
{
    return (u8 *)agent_dev->shr_para[func_idx] + PHY_MATCH_FLAG_OFFSET_IN_SHR_MEM;
}

int agentdrv_platform_get_devid(struct platform_device *pdev)
{
    return 0;
}
