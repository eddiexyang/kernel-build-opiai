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

#ifndef TSDRV_PDATA_H
#define TSDRV_PDATA_H

#include <linux/platform_device.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TSDRV_INVALID_CPU_CLUSTER 0xFFFF

enum tsdrv_cpu_type {
    TSDRV_CCPU,
    TSDRV_AICPU,
    TSDRV_DCPU,
    TSDRV_TSCPU,
    TSDRV_INVALID
};

/* Device chip info. */
int tsdrv_get_fftscpu_id(u32 devid);
u32 tsdrv_get_ts_num(u32 devid);
int tsdrv_get_chip_id(struct platform_device *pdev, u32 *chip_id);

u32 tsdrv_get_cpu_sclid(u32 devid);
u32 tsdrv_get_cpu_cluster(u32 devid, enum tsdrv_cpu_type cpu_type);

int tsdrv_get_board_slot_id(u32 devid, u32 *board_id, u32 *slot_id);
int tsdrv_get_numa_num_per_dev(void);
int tsdrv_get_partial_good(u32 devid, u32 *aicpu_partial_good_enable);

/* Device addr info. */
int tsdrv_get_gicv3_addr(u32 devid, phys_addr_t *paddr, size_t *size);
int tsdrv_get_sysctrl_addr(u32 devid, phys_addr_t *paddr, size_t *size);
int tsdrv_get_dispatch_addr(u32 devid, phys_addr_t *paddr, size_t *size);

/* TS addr info. */
int tsdrv_get_ts_sysctrl_addr(u32 devid, u32 tsid, phys_addr_t *paddr, size_t *size);
int tsdrv_get_ts_doorbell_addr(u32 devid, u32 tsid, phys_addr_t *paddr, size_t *size);
int tsdrv_get_ts_sram_addr(u32 devid, u32 tsid, phys_addr_t *paddr, size_t *size);
int tsdrv_get_ts_stars_addr(u32 devid, u32 tsid, phys_addr_t *paddr, size_t *size);
int tsdrv_get_tsensor_shm_addr(u32 devid, u32 tsid, phys_addr_t *paddr, size_t *size);
int tsdrv_get_ts_stars_rtsq_addr(u32 devid, u32 tsid, phys_addr_t *paddr, size_t *size);

/* TS irq info. */
int tsdrv_alloc_irqs(u32 devid);

#ifdef CFG_FEATURE_PROF_HWTS_THROUGH
int tsdrv_get_prof_aicore_irq(u32 devid, u32 tsid, u32 *irq);
int tsdrv_get_prof_hwts_log_irq(u32 devid, u32 tsid, u32 *irq);
#endif

u32 tsdrv_get_cq_update_irq_num(void);
int tsdrv_get_cq_update_irq(u32 devid, u32 tsid, u32 *irq);
int tsdrv_get_cq_update_hwirq(u32 *irq, u32 *hwirq);
int tsdrv_get_mailbox_ack_irq(u32 devid, u32 tsid, u32 *irq);
int tsdrv_get_mailbox_ack_hwirq(u32 irq, u32 *hwirq);
u32 tsdrv_get_mailbox_data_ack_irq_num(void);
int tsdrv_get_mailbox_data_ack_irq(u32 devid, u32 tsid, u32 *irq);
int tsdrv_get_mailbox_data_ack_hwirq(u32 irq, u32 *hwirq);
int tsdrv_get_func_cq_irq(u32 devid, u32 tsid, u32 *irq);
int tsdrv_get_func_cq_hwirq(u32 irq, u32 *hwirq);
int tsdrv_get_ts_irq_base(u32 devid, u32 tsid, u32 *irq);
u32 tsdrv_get_disp_nfe_irq_num(void);
int tsdrv_get_disp_nfe_irq(u32 devid, u32 tsid, u32 *irq);
int tsdrv_get_hwirq_from_irq(u32 irq, u32 *hwirq);

void tsdrv_set_platform_device(u32 devid, struct platform_device *pdev);
struct platform_device *tsdrv_get_platform_device(u32 devid);

int tsdrv_get_addr_by_index(u32 devid, u32 addr_index, phys_addr_t *paddr, size_t *size);
int tsdrv_get_trigger_irq_info(u32 devid, struct devdrv_ts_pdata *ts_pdata);

#ifdef __cplusplus
};
#endif

#endif
