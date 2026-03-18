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

#include <asm/io.h>

#include "nvme_comm_drv.h"
#include "devdrv_util.h"
#include "devdrv_interface.h"

#define REG_SIZE 4

u32 nvme_pf_num[DEVDRV_MAX_DEVICE];

void devdrv_set_nvme_pf_num(u32 devid, u32 pf_num)
{
    nvme_pf_num[devid] = pf_num;
}

u32 devdrv_get_nvme_pf_num(u32 devid)
{
    return DEVDRV_PF_NUM;
}

int devdrv_get_chip_type(void)
{
    return HISI_MINI_V3;
}

int devdrv_get_board_type(void)
{
    int board_type;

    /* read from acpi */
    board_type = BOARD_MINI_V3_EVB;

    return board_type;
}

void devdrv_raise_int_to_h(void __iomem *io_base, u32 pf, u32 vf, u32 irq)
{
    u32 regval;

    regval = ((irq << NVME_INT_VECTOR_OFFSET) | (1 << NVME_INT_REQ_OFFSET) | (pf << NVME_INT_PF_NUM_OFFSET) |
              (vf << NVME_INT_VF_NUM_OFFSET));

    devdrv_nvme_reg_wr(io_base, AGENTDRV_NVME_INT_REQ, regval);
}

void devdrv_get_nvme_irq_db(void __iomem *io_base, u32 reg_offset, u32 irq_vector, u32 db_id[], u32 *db_num)
{
    u32 i, num, regval;

    devdrv_nvme_reg_rd(io_base, reg_offset, &regval);
    devdrv_nvme_reg_wr(io_base, reg_offset, regval);

    num = 0;
    for (i = 0; i < AGENTDRV_QCNT_EACH_IRQ; i++) {
        if (regval & 0x1U) {
            db_id[num++] = i + irq_vector * AGENTDRV_QCNT_EACH_IRQ;
        }
        regval >>= 1;
    }
    *db_num = num;
}

void devdrv_get_nvme_irq_sq_db(void __iomem *io_base, u32 irq_vector, u32 db_id[], u32 func_id, u32 *db_num)
{
    u32 reg_offset = AGENTDRV_NVME_SQ_DB_STS_BASE + (irq_vector * REG_SIZE);

    if (irq_vector >= AGENTDRV_NVME_SQ_DB_IRQ_NUM) {
        *db_num = 0;
        return;
    }
    (void)func_id;
    devdrv_get_nvme_irq_db(io_base, reg_offset, irq_vector, db_id, db_num);
}

void devdrv_get_nvme_irq_cq_db(void __iomem *io_base, u32 irq_num, u32 db_id[], u32 func_id, u32 *db_num)
{
    u32 reg_offset = AGENTDRV_NVME_CQ_DB_STS_BASE + (irq_num * REG_SIZE);

    if (irq_num < AGENTDRV_NVME_SQ_DB_IRQ_NUM) {
        *db_num = 0;
        return;
    }
    (void)func_id;
    devdrv_get_nvme_irq_db(io_base, reg_offset, irq_num - AGENTDRV_NVME_SQ_DB_IRQ_NUM, db_id, db_num);
}

void devdrv_set_nvme_irq_mask(void __iomem *io_base, u32 func_id, u32 irq_vector)
{
    u32 reg_offset;
    (void)func_id;

    /* vector */
    reg_offset = AGENTDRV_NVME_INT_VECTOR_MASK_BASE + (REG_SIZE * irq_vector);
    devdrv_nvme_reg_wr(io_base, reg_offset, 0xFFFFFFFFU);
}

void devdrv_set_nvme_irq_unmask(void __iomem *io_base, u32 func_id, u32 irq_vector)
{
    u32 reg_offset;
    (void)func_id;

    /* vector */
    reg_offset = AGENTDRV_NVME_INT_VECTOR_MASK_BASE + (REG_SIZE * irq_vector);
    devdrv_nvme_reg_wr(io_base, reg_offset, 0);
}

void devdrv_set_nvme_irq_enbale(void __iomem *io_base, u32 func_id, u32 irq_num)
{
    devdrv_set_nvme_irq_unmask(io_base, func_id, irq_num);
}

int agentdrv_nvme_irq_num2func(u32 irq_num, u32 *func_id)
{
    (void)irq_num;
    *func_id = 0;
    return 0;
}

int devdrv_nvme_agent_irq_num2vector(u32 irq_num, u32 *irq_vector)
{
    if (irq_num >= AGENTDRV_SOC_DB_IRQ_NUM) {
        return -1;
    } else {
        *irq_vector = irq_num;
        return 0;
    }
}

u32 devdrv_nvme_agent_irq_vector2num(u32 irq_vector)
{
    return irq_vector;
}

u32 devdrv_nvme_get_cpu_id(u32 start_id, u32 total_num, u32 offset)
{
    return start_id;
}

void devdrv_soc_func2pfvf(u32 devid, u32 func_id, u32 rdie, struct agentdrv_pfvf_info *pfvf_info)
{
    pfvf_info->pf_num = devdrv_get_nvme_pf_num(devid);
    pfvf_info->func_num = 0;
    pfvf_info->vf_num = 0;
    pfvf_info->vf_en = 0;
}

bool agentdrv_get_soc_doorbell_capability(void)
{
#ifndef DRV_UT
        return true;
#else
        return false;
#endif
}

bool agentdrv_get_sriov_capability(void)
{
    return false;
}

bool devdrv_is_sriov_enabled(u32 func_totl)
{
    (void)func_totl;
    return false;
}
