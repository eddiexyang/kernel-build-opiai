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
#ifndef HWTS_DRV_API_H
#define HWTS_DRV_API_H

#include "ts_drv_init.h"
#include "ts_aisle_queue.h"


static inline void hwts_drv_writel_relaxed(void __iomem *reg, unsigned int val)
{
    if (reg == NULL) {
        ts_drv_err("the param is null.\n");
        return;
    }
    writel_relaxed(val, reg);
    dsb(sy);
}


static inline void hwts_drv_writeq_relaxed(void __iomem *reg, u64 val)
{
    if (reg == NULL) {
        ts_drv_err("the param is null.\n");
        return;
    }
    writeq_relaxed(val, reg);
    dsb(sy);
}

/* interfaces for hwts drv init */
int hwts_drv_get_config_info(struct drv_hwts_ctrl *hwts, struct platform_device *pdev);
int hwts_drv_va_base_ioremap(struct platform_device *pdev, struct drv_hwts_ctrl *hwts);
int hwts_drv_mailbox_addr_config(struct drv_hwts_ctrl *hwts);
int hwts_drv_get_irq(struct platform_device *pdev, struct drv_hwts_ctrl *hwts, int node_id);
int hwts_drv_register_irq_to_cores(struct drv_hwts_ctrl *hwts, struct platform_device *pdev);
void hwts_init_mailbox(struct drv_hwts_ctrl *hwts, int node_id);

/* interfaces for resource release */
void hwts_drv_release(struct platform_device *pdev, struct drv_hwts_ctrl *hwts);
void hwts_drv_iounmap_resource(struct platform_device *pdev, struct drv_hwts_ctrl *hwts);

/* lib functions for hwts drv self use */
int hwts_drv_wait_ts_ready(struct drv_hwts_ctrl *hwts);
void __iomem *hwts_get_mailbox_addr(u32 mb_ind);
void __iomem *hwts_get_return_mailbox_addr(u32 mb_ind);

/* functions for hwts interrupt, submit and ack task */
irqreturn_t hwts_drv_irq_callback(int irq, void *data);
void hwts_drv_submit_task(unsigned long data);
int hwts_drv_sched_ack(unsigned int devid, unsigned int subevent_id, const char *msg, unsigned int msg_len, void *priv);
void hwts_handle_ack_success(u32 cpu_index, u32 ts_index);
void hwts_handle_ack_fail(u32 cpu_index, u32 ts_index);
int hwts_drv_init(struct platform_device *pdev, struct drv_hwts_ctrl *hwts, int node_id);

#endif
