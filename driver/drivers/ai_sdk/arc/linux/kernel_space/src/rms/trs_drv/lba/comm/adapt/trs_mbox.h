/*
* Copyright (c) Huawei Technologies Co., Ltd. 2022-2023. All rights reserved.
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
* Create: 2022-8-15
*/
#ifndef TRS_MBOX_H
#define TRS_MBOX_H

#include <linux/types.h>

#include "trs_pub_def.h"

struct trs_mbox_ops {
    void (* mbox_release)(void *priv);
    int (* trigger_irq)(void *priv);
    void (* free_irq)(void *priv);
};

struct trs_mbox_chan_attr {
    phys_addr_t base;
    size_t size;

    void *priv;
    struct trs_mbox_ops ops;
};

void *trs_mbox_chan_init(struct trs_id_inst *inst, struct trs_mbox_chan_attr *attr);
void trs_mbox_chan_uninit(struct trs_id_inst *inst);

int trs_mbox_send(struct trs_id_inst *inst, u32 chan_id, void *data, size_t size, int timeout);

void trs_mbox_chan_txdone(void *mbox_chan);
int trs_mbox_get_chan_num(struct trs_id_inst *inst);

#endif /* TRS_MBOX_H */
