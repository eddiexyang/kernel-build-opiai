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
#ifndef __RESOURCE_COMM_DRV_H__
#define __RESOURCE_COMM_DRV_H__

#include <asm/io.h>
#include <linux/platform_device.h>

#include "agentdrv_unit.h"

#define DEVDRV_EACH_DMA_IRQ_NUM 2

struct devdrv_res_dma_common_info {
    u32 chan_num;
    u32 done_irq_base;
    u32 err_irq_base;
    u32 err_flag;
    struct devdrv_dma_sq_cq_info sq_cq_info;
};

struct agent_res_sdi_addr {
    u64 msg_size;
    phys_addr_t msg_base;
    u64 db_size;
    phys_addr_t db_base;
    u64 db_state_size;
    phys_addr_t db_state_base;
    u64 raise_int_size;
    phys_addr_t raise_int_base;
    u64 shr_size;
    phys_addr_t shr_base;
    u32 shr_type;
    u64 bw_ctrl_size;
    phys_addr_t bw_ctrl_base;
    u64 rsv_mem_size;
    phys_addr_t rsv_mem_base;
    u64 topic_sched_resmem_size;
    phys_addr_t topic_sched_resmem_base;
    u64 hbm_size;
    phys_addr_t hbm_base;
};

struct devdrv_dma_chan_dist_info {
    u32 local_use_num;
    u32 local_start_index;
    u32 remote_use_num;
    u32 remote_start_index;
    u32 ts_use_num;
    u32 ts_start_index;
};

void agentdrv_res_init_func_pf_num(void);
void agentdrv_res_init_func_total_num(void);
u32 agentdrv_res_get_func_total(void);
int agentdrv_get_connect_type(void);
u32 agentdrv_res_get_func_pf(void);
void agentdrv_res_set_func_total(u32 totl_num);
void agentdrv_dma_res_common(u32 func_id, u32 dma_irq_base, struct devdrv_res_dma_common_info *out);
void agentdrv_res_sdi_addr(u32 dev_id, u32 func_id, struct agent_res_sdi_addr *sdi_addr);
int agentdrv_ioremap_doorbell_base(void __iomem **doorbell_base, phys_addr_t db_base, u64 db_size);
void __iomem *agentdrv_get_phy_match_flag_addr(struct agentdrv_devctrl *agent_dev, u32 func_idx);
int agentdrv_platform_get_devid(struct platform_device *pdev);
int agentdrv_get_tx_atu_addr(struct platform_device *pdev, u32 func_id, u32 region_index, u64 *start_addr, u64 *size);
void agentdrv_get_dma_max_msg_chan(u32 func_id, u32 *chan_num);
void agentdrv_get_p2p_msg_db_range(u32 func_id, u32 *p2p_db_start, u32 *p2p_db_end);

#endif /* __RESOURCE_COMM_DRV_H__ */
