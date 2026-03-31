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

#ifndef _NVME_COMM_DRV_H_
#define _NVME_COMM_DRV_H_

#include <asm/io.h>
#include "nvme_drv.h"

struct agentdrv_pfvf_info {
    u32 pf_num;
    u32 func_num;
    u32 vf_num;
    u32 vf_en;
};

void devdrv_nvme_reg_wr(void __iomem *io_base, u32 offset, u32 val);
void devdrv_nvme_reg_rd(const void __iomem *io_base, u32 offset, u32 *val);

int devdrv_get_chip_type(void);
int devdrv_get_board_type(void);
int devdrv_get_iep_nvme_device_id(void);

void devdrv_raise_int_to_h(void __iomem *io_base, u32 pf, u32 vf, u32 irq);

void devdrv_get_nvme_irq_sq_db(void __iomem *io_base, u32 irq_num, u32 db_id[], u32 func_id, u32 *db_num);
void devdrv_get_nvme_irq_cq_db(void __iomem *io_base, u32 irq_num, u32 db_id[], u32 func_id, u32 *db_num);
void devdrv_set_nvme_irq_mask(void __iomem *io_base, u32 func_id, u32 irq_num);
void devdrv_set_nvme_irq_unmask(void __iomem *io_base, u32 func_id, u32 irq_num);
void devdrv_set_nvme_irq_enbale(void __iomem *io_base, u32 func_id, u32 irq_num);
void devdrv_nvme_handle_flr(void __iomem *io_base, u32 irq_num);

int agentdrv_nvme_irq_num2func(u32 irq_num, u32 *func_id);
int devdrv_nvme_agent_irq_num2vector(u32 irq_num, u32 *irq_vector);
u32 devdrv_nvme_agent_irq_vector2num(u32 irq_vector);
u32 devdrv_nvme_get_cpu_id(u32 start_id, u32 total_num, u32 offset);
void devdrv_soc_func2pfvf(u32 devid, u32 func_id, u32 rdie, struct agentdrv_pfvf_info *pfvf_info);
void devdrv_set_nvme_pf_num(u32 devid, u32 pf_num);
u32 devdrv_get_nvme_pf_num(u32 devid);
bool agentdrv_get_soc_doorbell_capability(void);
bool agentdrv_get_sriov_capability(void);
bool devdrv_is_sriov_enabled(u32 func_totl);
#endif
