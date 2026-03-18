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

#ifndef _DEVDRV_ATU_H_
#define _DEVDRV_ATU_H_

#include <linux/types.h>

#include "devdrv_msg_def.h"

#define ATU_INVALID 0
#define ATU_VALID 1

#define ATU_TYPE_RX_MEM 1
#define ATU_TYPE_RX_IO 2
#define ATU_TYPE_TX_MEM 3
#define ATU_TYPE_TX_IO 4
#define ATU_TYPE_TX_HOST 5

#define ATU_SIZE_ALIGN (4 * 1024)

#define DEVDRV_MAX_RX_ATU_NUM 8
#define DEVDRV_MAX_TX_ATU_NUM 16

#define DEVDRV_ATU_INVALID_DELAY 10 /* ms */

#define DEVDRV_SINGLE_DEV_TX_ATU_NUM 2

#define DEVDRV_EXTERN_PCIE_DDR_SIZE 0x80000000000
#ifdef CFG_SOC_PLATFORM_ESL_FPGA
#define DEVDRV_PCIE_DDR_ATU_OFFSET 0x1000000000  /* 64G */
#else
#define DEVDRV_PCIE_DDR_ATU_OFFSET 0x10000000000 /* 1T */
#endif
#define MAX_H2D_ATU_NUM 6

struct devdrv_iob_atu {
    u32 valid;
    u32 atu_id;
    u64 phy_addr;
    u64 base_addr;
    u64 target_addr;
    u64 size;
};

struct devdrv_p2p_atu_info {
    u32 valid;
    u32 devid; /* dst host devid */
    u32 type;
    u32 host_devid; /* local device in host devid */
    u64 size;
    u32 alloc_times;
    u32 free_times;
};

struct devdrv_h2d_atu_info {
    u64 offset;
    u64 target_addr;
    u64 size;
    u32 valid;
    u32 atu_id;
    u32 devid;
};

struct devdrv_cfg_tx_atu_para {
    u32 local_devid;
    u32 host_devid;
    u32 dst_host_devid;
    u32 atu_type;
};

void devdrv_tx_atu_init(void);
u32 devdrv_alloc_atu_id(u32 local_devid, u32 host_devid, u32 dst_host_devid, u32 atu_type, u64 target_size);
u32 devdrv_get_atu_id(u32 local_devid, u32 host_devid, u32 dst_host_devid, u32 atu_type);
int devdrv_mem_rx_atu_init(u32 devid, const void __iomem *apb_base, struct devdrv_iob_atu atu[], int num);
void devdrv_io_rx_atu_show(u32 devid, const void __iomem *apb_base);
void devdrv_rsv_mem_rx_atu_show(u32 devid, const void __iomem *apb_base);
int devdrv_atu_base_to_target(const struct devdrv_iob_atu atu[], int num, u64 base_addr, u64 *target_addr);
int devdrv_atu_target_to_base(const struct devdrv_iob_atu atu[], int num, u64 target_addr, u64 *base_addr);
int devdrv_set_tx_atu(void __iomem *apb_base, struct devdrv_cfg_tx_atu_para *tx_para,
    u64 target_addr, u64 target_size, struct devdrv_shr_para __iomem *para);
int devdrv_get_dev_tx_atu(const void __iomem *apb_base, struct devdrv_iob_atu atu[], u32 len,
    const struct devdrv_cfg_tx_atu_para *tx_para, u64 phy_addr);
int devdrv_del_dev_tx_atu(void __iomem *apb_base, struct devdrv_iob_atu atu[], u32 len,
    struct devdrv_cfg_tx_atu_para *tx_para, u64 target_addr);
extern int devdrv_get_atu_info(u32 dev_id, int atu_type, struct devdrv_iob_atu **atu, phys_addr_t *host_phy_base);
int devdrv_del_dev_h2d_tx_atu(void __iomem *apb_base, struct devdrv_iob_atu atu[], u32 len,
    const struct devdrv_cfg_tx_atu_para *tx_para, dma_addr_t target_addr);
u32 devdrv_get_h2d_atu_id(u32 devid, dma_addr_t addr);
void devdrv_tx_atu_print_cfg_info(void);
#endif
