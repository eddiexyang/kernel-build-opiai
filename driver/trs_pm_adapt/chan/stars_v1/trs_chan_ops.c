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
#include "soc_res.h"
#include "uda.h"

#include "trs_chan_irq.h"
#include "trs_device_comm.h"
#include "trs_chan_ops_mbox.h"
#include "trs_chan_ops_db.h"
#include "trs_chan_ops_rsv_mem.h"
#include "trs_chan_ops_stars.h"
#include "trs_chan_sqcq.h"
#include "trs_tscpu_chan_sqcq.h"
#include "trs_chan_maint_sqcq.h"
#include "trs_cdqm.h"
#include "trs_chan_update.h"
#include "trs_pm_adapt.h"
#include "trs_chip_def.h"

static u32 trs_chan_ops_get_irq_group(struct trs_id_inst *inst, u32 irq_index)
{
    return 0;
}

int trs_chan_ops_request_irq(struct trs_id_inst *inst, u32 irq_type, int irq_index,
    void *para, void (*handler)(int irq_type, int irq_index, void *para, u32 cqid[], u32 cq_num))
{
    struct trs_chan_irq_attr attr;

    switch (irq_type) {
        case TS_CQ_UPDATE_IRQ:
            attr.name = "cqe_done";
            attr.get_valid_cq = trs_chan_ops_get_valid_cq_list;
            attr.intr_mask_config = trs_chan_ops_intr_mask_config;
            break;
        case TS_FUNC_CQ_IRQ:
            attr.name = "maint_cq";
            attr.get_valid_cq = NULL;
            attr.intr_mask_config = NULL;
            break;
        default:
            trs_err("Unknown irq_type. (irq_type=%d)\n", irq_type);
            return -ENODEV;
    }

    attr.group = trs_chan_ops_get_irq_group(inst, irq_index);
    attr.handler = handler;
    attr.para = para;
    return trs_chan_request_irq(inst, irq_type, irq_index, &attr);
}
EXPORT_SYMBOL(trs_chan_ops_request_irq);

static int trs_chan_adapt_sqe_update(struct trs_id_inst *inst, int pid, u32 sqid, u32 sqeid, void *sqe)
{
    return trs_chan_ops_sqe_update(inst, pid, sqe);
}

int trs_chan_ops_ctrl_sqcq(struct trs_id_inst *inst, struct trs_chan_type *types, u32 id, u32 cmd, u32 para)
{
    int ret = -EINVAL;

    switch (types->type) {
        case CHAN_TYPE_HW:
            ret = trs_stars_chan_ops_ctrl_sqcq(inst, id, cmd, para);
            break;
        case CHAN_TYPE_SW:
            ret = trs_tscpu_chan_ops_ctrl_sqcq(inst, id, cmd, para);
            break;
        case CHAN_TYPE_MAINT:
            ret = trs_chan_ops_ctrl_maint_sqcq(inst, id, cmd, para);
            break;
        default:
            break;
    }

    return ret;
}
EXPORT_SYMBOL(trs_chan_ops_ctrl_sqcq);

int trs_chan_ops_query_sqcq(struct trs_id_inst *inst, struct trs_chan_type *types, u32 id, u32 cmd, u64 *value)
{
    int ret = -EINVAL;
    u32 start, end;

    *value = 0;
    switch (types->type) {
        case CHAN_TYPE_HW:
            ret = trs_stars_chan_ops_query_sqcq(inst, id, cmd, value);
            if ((ret) && (cmd == QUERY_CMD_SQ_DB_PADDR) &&
                (trs_get_trigger_sq_db_cfg(inst, &start, &end))) {
                *value = 0;
                ret = 0;
            }

            /* 1、mia device has own db cfg, so inst my is mia_inst,
             *  then query tail paddr will return fail.
             * 2、xiaoyi mode will alloc sqcq in DEVDRV_PROCESS_USER.
             */
#ifndef EMU_ST
            if (trs_is_support_uio() && uda_is_phy_dev(inst->devid) && trs_is_support_uio_d()) {
                if (cmd == QUERY_CMD_SQ_DB_PADDR) {
                    ret = trs_stars_chan_ops_query_sqcq(inst, id, QUERY_CMD_SQ_TAIL_PADDR, value);
                } else if (cmd == QUERY_CMD_SQ_TAIL_PADDR) {
                    *value = 0;
                    ret = 0;
                }
            }
#endif
            break;
        case CHAN_TYPE_SW:
            ret = trs_tscpu_chan_ops_query_sqcq(inst, id, cmd, value);
            break;
        case CHAN_TYPE_MAINT:
            ret = trs_chan_ops_query_maint_sqcq(inst, id, cmd, value);
            break;
        default:
            break;
    }

    return ret;
}
EXPORT_SYMBOL(trs_chan_ops_query_sqcq);

static struct trs_chan_adapt_ops g_trs_chan_ops = {
    .owner = THIS_MODULE,
    .sq_mem_alloc = trs_chan_ops_sq_mem_alloc,
    .sq_mem_free = trs_chan_ops_sq_mem_free,
    .cq_mem_alloc = trs_chan_ops_cq_mem_alloc,
    .cq_mem_free = trs_chan_ops_cq_mem_free,
    .flush_cache = trs_chan_ops_flush_sqe_cache,
    .invalid_cache = trs_chan_ops_invalid_cqe_cache,
    .cqe_is_valid = trs_chan_ops_cqe_is_valid,
    .get_sq_head_in_cqe = trs_chan_ops_get_sq_head_in_cqe,
    .sqe_update = trs_chan_adapt_sqe_update,
    .cqe_update = trs_chan_ops_cqe_update,
    .sqcq_ctrl = trs_chan_ops_ctrl_sqcq,
    .sqcq_query = trs_chan_ops_query_sqcq,
    .notice_ts = trs_chan_ops_mbox_send,
    .get_irq = trs_chan_get_irq,
    .request_irq = trs_chan_ops_request_irq,
    .free_irq = trs_chan_free_irq,
};

struct trs_chan_adapt_ops *trs_chan_get_adapt_ops(void)
{
    return &g_trs_chan_ops;
}

int trs_chan_ops_init(struct trs_id_inst *inst)
{
#ifdef CFG_FEATURE_CDQM
    struct cdqm_adapt_ops ops;
#endif
    int ret;

#ifdef CFG_FEATURE_CDQM
    ops.send_sync_msg = NULL;
    ops.ssid_query = trs_device_get_ssid;
    ret = cdqm_ts_inst_register(inst, &ops);
    if (ret != 0) {
        return ret;
    }
#endif

    ret = trs_chan_ops_db_init(inst);
    if (ret != 0) {
        goto err_db_init;
    }

    ret = trs_chan_ops_rsv_mem_init(inst);
    if (ret != 0) {
        trs_err("Rsv mem init fail. (devid=%u; tsid=%u; ret=%d)\n", inst->devid, inst->tsid, ret);
        goto err_rsv_mem_init;
    }

    ret = trs_chan_ops_stars_init(inst);
    if (ret != 0) {
        goto err_stars_init;
    }

    return ret;
err_stars_init:
    trs_chan_ops_rsv_mem_uninit(inst);
err_rsv_mem_init:
    trs_chan_ops_db_uninit(inst);
err_db_init:
#ifdef CFG_FEATURE_CDQM
    cdqm_ts_inst_unregister(inst);
#endif
    return -ENODEV;
}

void trs_chan_ops_uninit(struct trs_id_inst *inst)
{
    trs_chan_ops_stars_uninit(inst);
    trs_chan_ops_rsv_mem_uninit(inst);
    trs_chan_ops_db_uninit(inst);
#ifdef CFG_FEATURE_CDQM
    (void)cdqm_ts_inst_unregister(inst);
#endif
}

