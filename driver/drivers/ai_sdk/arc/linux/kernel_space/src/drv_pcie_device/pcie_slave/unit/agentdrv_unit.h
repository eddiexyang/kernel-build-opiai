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
#ifndef _AGENTDRV_UNIT_H_
#define _AGENTDRV_UNIT_H_

#include <linux/pci.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>

#include "devdrv_interface.h"
#include "dma_drv.h"
#include "apb_drv.h"
#include "devdrv_atu.h"
#include "devdrv_dma.h"
#include "agentdrv_mdc_p2p.h"

/* cloud: ai server smp os has 4 cloud_chips
   mini: 1 mini_chip */
#define MAX_AGENTCHIP_CNT 4 /* unit */
#define AGENTDRV_SRIOV_VF_DEVID_START 32

#define DEVDRV_DMA_IOVA_RANGE_UNINIT 0
#define DEVDRV_DMA_IOVA_RANGE_INIT   1
struct devdrv_dma_iova_addr_range {
    int init_flag;
    u64 start_addr;
    u64 end_addr;
};

struct agentdrv_devctrl {
    u32 valid[MAX_AGENTFUNC_CNT];
    u32 agent_id;
    u32 func_pf_num;
    u32 func_totl_num;
    u32 func_index;
    u32 msi_irq_base[MAX_AGENTFUNC_CNT][AGENTDRV_SOC_DB_IRQ_NUM];
    bool func_rdy[MAX_AGENTFUNC_CNT];
    atomic_t subdev_num;
    int cpu_info_valid;
    struct agentdrv_cpu_info cpu_info[MAX_AGENTFUNC_CNT];
    struct pci_dev *pdev; /* dma pdev */
    struct pci_dev *sdi_pdev;
    struct platform_device *apb_pdev;
    struct devdrv_shr_para __iomem *shr_para[MAX_AGENTFUNC_CNT];
    unsigned long dma_idle_bitmap; /* 1:idle 0:occupied */
    void __iomem *dma_base;
    void __iomem *dma_chan_base;
    void __iomem *sdi_base[MAX_AGENTFUNC_CNT];
    void __iomem *raise_int_base[MAX_AGENTFUNC_CNT];
    void __iomem *apb_base;
    struct mutex mutex;
    u32 dma_host_err_irq[DEVDRV_DMA_CHAN_NUM];
    struct devdrv_dma_dev *p_agentdrv_dma_dev[MAX_AGENTFUNC_CNT];
    struct agentdrv_msg_dev *p_agentdrv_msg_dev[MAX_AGENTFUNC_CNT];
    struct agentdrv_platform_dev *platform_dev;
    struct devdrv_iob_atu mem_rx_atu[DEVDRV_MAX_RX_ATU_NUM];
    struct devdrv_iob_atu tx_atu[DEVDRV_TX_ATU_NUM];
    struct agentdrv_general_interrupt_info interrupt_info[AGENTDRV_GENERAL_INTERRUPT_NUM];
    struct devdrv_dma_iova_addr_range iova_range[MAX_AGENTFUNC_CNT];
    u32 vm_full_spec_flag;
    u32 urca_status;
};

void devdrv_dev2chipfunc(u32 dev_id, u32 *chip_id, u32 *func_id);
void devdrv_chipfunc2dev(int *dev_id, int chip_id, int func_id);
int agentdrv_unit_init(void);
struct agentdrv_devctrl *agentdrv_get_dev(int dev_id);
bool agentdrv_is_dev_valid(const struct agentdrv_devctrl *agent_dev, u32 func_id);
struct agentdrv_devctrl *agentdrv_get_dev_by_apb_dev(const struct platform_device *apb_pdev);
struct agentdrv_platform_dev *agentdrv_get_platform_dev_by_dma_chan(const struct devdrv_dma_channel *dma_chan);
struct agentdrv_msg_dev *agentdrv_get_msg_dev(u32 dev_id);
int agentdrv_get_msix_offset(struct agentdrv_msg_chan *chan);
struct devdrv_dma_channel *agentdrv_get_dma_chan_by_id(struct agentdrv_devctrl *agent_dev, u32 chan_id, u32 *func_id);
void agentdrv_check_dma_urca_status(struct agentdrv_devctrl *agent_dev, u32 chan_id);
#endif
