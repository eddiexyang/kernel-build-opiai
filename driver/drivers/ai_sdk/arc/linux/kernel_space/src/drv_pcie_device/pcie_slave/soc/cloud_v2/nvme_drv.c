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
#include "resource_drv.h"

#define REG_SIZE 4

u32 nvme_pf_num[DEVDRV_MAX_DEVICE];

void devdrv_set_nvme_pf_num(u32 devid, u32 pf_num)
{
    nvme_pf_num[devid] = pf_num;
}

u32 devdrv_get_nvme_pf_num(u32 devid)
{
    return nvme_pf_num[devid];
}

int devdrv_get_chip_type(void)
{
    return HISI_CLOUD_V2;
}

int devdrv_get_board_type(void)
{
    int board_type;

    /* read from acpi */
    board_type = BOARD_CLOUD_V2_EVB;

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
        if (regval & 0x1) {
            db_id[num++] = i + irq_vector * AGENTDRV_QCNT_EACH_IRQ;
        }
        regval >>= 1;
    }
    *db_num = num;
}

void devdrv_get_vf_nvme_irq_db(void __iomem *io_base, u32 reg_offset, u32 cq_flag, u32 db_id[], u32 *db_num)
{
    u32 i, num, regval, cqsq_bit;
    /* high 16 bit is cq; low 16 bit is sq */
    devdrv_nvme_reg_rd(io_base, reg_offset, &regval);
    if (cq_flag) {
        regval = regval & AGENTDRV_VF_CQ_MASK;
        cqsq_bit = regval >> AGENTDRV_VF_QCNT_EACH_IRQ;
    } else {
        regval = regval & AGENTDRV_VF_SQ_MASK;
        cqsq_bit = regval;
    }
    devdrv_nvme_reg_wr(io_base, reg_offset, regval);

    num = 0;
    for (i = 0; i < AGENTDRV_VF_QCNT_EACH_IRQ; i++) {
        if (cqsq_bit & 0x1) {
            db_id[num++] = i;
        }
        cqsq_bit >>= 1;
    }
    *db_num = num;
}

void devdrv_get_nvme_irq_sq_db(void __iomem *io_base, u32 irq_vector, u32 db_id[], u32 func_id, u32 *db_num)
{
    u32 reg_offset;
    u32 func_totl = agentdrv_res_get_func_total();

    /* func_totl > 2, and func_id > 0, is vf */
    if (devdrv_is_sriov_enabled(func_totl) && func_id > 0) {
        reg_offset = 0;
        devdrv_get_vf_nvme_irq_db(io_base, reg_offset, 0, db_id, db_num);
    } else {
        reg_offset = AGENTDRV_NVME_SQ_DB_STS_BASE + (irq_vector * REG_SIZE);
        if (irq_vector >= AGENTDRV_NVME_SQ_DB_IRQ_NUM) {
            *db_num = 0;
            return;
        }

        devdrv_get_nvme_irq_db(io_base, reg_offset, irq_vector, db_id, db_num);
    }
}

void devdrv_get_nvme_irq_cq_db(void __iomem *io_base, u32 irq_vector, u32 db_id[], u32 func_id, u32 *db_num)
{
    u32 reg_offset;
    u32 func_totl = agentdrv_res_get_func_total();

    /* func_totl > 2, and func_id > 0, is vf */
    if (devdrv_is_sriov_enabled(func_totl) && func_id > 0) {
        reg_offset = 0;
        devdrv_get_vf_nvme_irq_db(io_base, reg_offset, 1, db_id, db_num);
    } else {
        reg_offset = AGENTDRV_NVME_CQ_DB_STS_BASE + (irq_vector * REG_SIZE);
        if (irq_vector < AGENTDRV_NVME_SQ_DB_IRQ_NUM) {
            *db_num = 0;
            return;
        }

        devdrv_get_nvme_irq_db(io_base, reg_offset, irq_vector - AGENTDRV_NVME_SQ_DB_IRQ_NUM, db_id, db_num);
    }
}

void devdrv_set_nvme_irq_mask(void __iomem *io_base, u32 func_id, u32 irq_vector)
{
    u32 reg_offset;
    u32 func_totl = agentdrv_res_get_func_total();

    /* func_totl > 2, and func_id > 0, is vf */
    if (devdrv_is_sriov_enabled(func_totl) && func_id > 0) {
        reg_offset = REG_SIZE;
    } else {
        reg_offset = AGENTDRV_NVME_INT_VECTOR_MASK_BASE + (REG_SIZE * irq_vector);
    }

    devdrv_nvme_reg_wr(io_base, reg_offset, 0xFFFFFFFF);
}

void devdrv_set_nvme_irq_unmask(void __iomem *io_base, u32 func_id, u32 irq_vector)
{
    u32 reg_offset;
    u32 func_totl = agentdrv_res_get_func_total();

    /* func_totl > 2, and func_id > 0, is vf */
    if (devdrv_is_sriov_enabled(func_totl) && func_id > 0) {
        reg_offset = REG_SIZE;
    } else {
        reg_offset = AGENTDRV_NVME_INT_VECTOR_MASK_BASE + (REG_SIZE * irq_vector);
    }

    devdrv_nvme_reg_wr(io_base, reg_offset, 0);
}

void devdrv_set_nvme_irq_enbale(void __iomem *io_base, u32 func_id, u32 irq_vector)
{
    /* vector */
    devdrv_set_nvme_irq_unmask(io_base, func_id, irq_vector);
}

int agentdrv_nvme_irq_num2func(u32 irq_num, u32 *func_id)
{
    (void)irq_num;
    *func_id = 0;

    return 0;
}

int devdrv_nvme_agent_irq_num2vector(u32 irq_num, u32 *irq_vector)
{
    int ret = 0;

    /* 0 and 1 used by SQ; 4 and 5 used by CQ; irq 2 and 3 not used for pcie */
    if (irq_num >= AGENTDRV_SOC_DB_IRQ_NUM) {
        ret = -1;
    } else if (irq_num >= AGENTDRV_NVME_SQ_DB_IRQ_NUM) {
        *irq_vector = irq_num - AGENTDRV_NVME_SQ_DB_IRQ_NUM;
        ret = 0;
    } else {
        *irq_vector = irq_num;
        ret = 0;
    }

    return ret;
}

u32 devdrv_nvme_agent_irq_vector2num(u32 irq_vector)
{
    u32 irq_num;

    if (irq_vector < AGENTDRV_NVME_SQ_DB_IRQ_NUM) {
        irq_num = irq_vector;
    } else {
        irq_num = irq_vector + AGENTDRV_NVME_SQ_DB_IRQ_NUM;
    }

    return irq_num;
}

u32 devdrv_nvme_get_cpu_id(u32 start_id, u32 total_num, u32 offset)
{
    return start_id;
}

void devdrv_soc_func2pfvf(u32 devid, u32 func_id, u32 rdie, struct agentdrv_pfvf_info *pfvf_info)
{
    u32 func_total = agentdrv_res_get_func_total();
    if (func_total <= AGENTDRV_1PF_OF_FUNC_TOTAL) {
        pfvf_info->func_num = func_id;
        pfvf_info->vf_num = 0;
        pfvf_info->vf_en = 0;
    } else {
        pfvf_info->func_num = 0;
        pfvf_info->vf_num = func_id % func_total;
        pfvf_info->vf_en = (pfvf_info->vf_num == 0 ? 0 : 1);
    }

    if (rdie) {
        pfvf_info->pf_num = DEVDRV_PF_NUM + pfvf_info->func_num;
    } else {
        pfvf_info->pf_num = DEVDRV_PF_NUM;
    }
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
    return true;
}

bool devdrv_is_sriov_enabled(u32 func_totl)
{
    /* if func_totl > die num, sriov is enabled */
    if (func_totl > AGENTDRV_1PF_OF_FUNC_TOTAL) {
        return true;
    } else {
        return false;
    }
}
