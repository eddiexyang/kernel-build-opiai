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
* Create: 2022-10-12
*/

#ifndef TRS_MIA_CFG_H
#define TRS_MIA_CFG_H

#include <linux/types.h>
#include <linux/kref.h>

#include "virtmng_interface.h"

#define MAX_CQ_NUM 2048
struct trs_mia_cqe_done {
    void (*cq_handler)(int irq_type, int irq_index, void *para, u32 cqid[], u32 cq_num);
    void *para;
    u32 cqid[MAX_CQ_NUM];
};

struct trs_mia_cfg {
    struct trs_id_inst inst;
    struct trs_id_inst pm_inst;

    u32 vfid;

    u32 mia_bitnum;
    u32 sia_bitnum;
    u32 rtsq_bitmap;
    u32 event_bitmap;
    u32 notify_bitmap;

    struct mutex mutex;
    u32 cq_num_per_grp;
    u32 cqe_done_irq_num;
    struct trs_mia_cqe_done *cqe_done;
    struct kref ref;
};

int trs_mia_cfg_create(struct trs_id_inst *inst, struct vmngd_client_instance *instance);
void trs_mia_cfg_destroy(struct trs_id_inst *inst);

struct trs_mia_cfg *trs_mia_cfg_get(struct trs_id_inst *inst);
void trs_mia_cfg_put(struct trs_mia_cfg *mia_cfg);

#endif /* TRS_MIA_CFG_H */
