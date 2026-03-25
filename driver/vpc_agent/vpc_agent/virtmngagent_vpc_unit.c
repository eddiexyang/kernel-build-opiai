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
#include <linux/interrupt.h>
#include <linux/errno.h>
#include "virtmng_public_def.h"
#include "virtmngagent_vpc_unit.h"

void vmnga_bar_wr(void __iomem *io_base, u32 offset, u32 val)
{
    writel(val, io_base + offset);
}

void vmnga_bar_rd(const void __iomem *io_base, u32 offset, u32 *val)
{
    *val = readl(io_base + offset);
}

void vmnga_set_doorbell(void __iomem *io_base, u32 db_id, u32 val)
{
    vmnga_bar_wr(io_base, db_id * VMNG_DOORBELL_ADDR_SIZE, val);
}

int vmnga_register_vpc_irq_func(void *drvdata, u32 vector_index, irqreturn_t (*callback_func)(int, void *), void *para,
    const char *name)
{
    struct vmnga_vpc_unit *unit = (struct vmnga_vpc_unit *)drvdata;
    unsigned int vector;
    int ret;

    if (unit == NULL) {
        vmng_err("Input parameter is error. (irq_index=%u)\n", vector_index);
        return -EINVAL;
    }
    if (vector_index >= VIRTMNGAGENT_MSIX_MAX) {
        vmng_err("Input parameter is error. (dev_id=%u; irq_index=%u)\n", unit->dev_id, vector_index);
        return -EINVAL;
    }
    vector = unit->msix_ctrl.entries[vector_index].vector;
    ret = request_irq(vector, callback_func, 0, name, para);
    if (ret != 0) {
        vmng_err("Call request_irq failed. (dev_id=%u; irq_index=%u; irq=%u; ret=%d)\n",
                 unit->dev_id, vector_index, vector, ret);
        return ret;
    }
    return 0;
}

int vmnga_unregister_vpc_irq_func(void *drvdata, u32 vector_index, void *para)
{
    struct vmnga_vpc_unit *unit = (struct vmnga_vpc_unit *)drvdata;
    unsigned int vector;

    if (unit == NULL) {
        vmng_err("Input parameter is error. (irq_index=%u)\n", vector_index);
        return -EINVAL;
    }

    if (vector_index >= VIRTMNGAGENT_MSIX_MAX) {
        vmng_err("Input parameter is error. (dev_id=%u; irq_index=%u)\n", unit->dev_id, vector_index);
        return -EINVAL;
    }
    vector = unit->msix_ctrl.entries[vector_index].vector;
    (void)irq_set_affinity_hint(vector, NULL);
    vmng_debug("Get vector value. (dev_id=%u; irq_index=%u; free_irq=%u)\n", unit->dev_id, vector_index, vector);
    free_irq(vector, para);

    return 0;
}
