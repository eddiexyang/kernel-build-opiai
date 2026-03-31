/*
* Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
* Create: 2022-10-15
*/
#ifndef TRS_CHAN_IRQ_H
#define TRS_CHAN_IRQ_H

#include <linux/types.h>

#include "trs_pub_def.h"

struct trs_chan_irq_attr {
    char *name;
    u32 group;
    void *para;

    void (*handler)(int irq_type, int irq_index, void *para, u32 cqid[], u32 cq_num);
    int (*get_valid_cq)(struct trs_id_inst *inst, u32 group, u32 cqid[], u32 cq_id_num, u32 *valid_cq_num);
    void (*intr_mask_config)(struct trs_id_inst *inst, u32 group, u32 irq, int val);
};

int trs_chan_get_irq(struct trs_id_inst *inst, u32 irq_type, u32 irq[], u32 irq_num, u32 *valid_irq_num);
int trs_chan_request_irq(struct trs_id_inst *inst, int irq_type, int irq_index, struct trs_chan_irq_attr *attr);
int trs_chan_free_irq(struct trs_id_inst *inst, int irq_type, int irq_index, void *para);

#endif
