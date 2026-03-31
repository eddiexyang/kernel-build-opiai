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
* Create: 2022-7-9
*/

#include <linux/kallsyms.h>

#include "ascend_kernel_hal.h"
#include "trs_pub_def.h"
#include "trs_id.h"

#include "cdq_manager.h"
#include "cdqm_dev.h"
#include "cdqm_core.h"

typedef int (*cdqm_devdrv_get_stars_irq_base)(u32 dev_id, int *irq);
typedef int (*cdqm_devdrv_get_stars_reg_base)(u32 dev_id, u64 *base_addr);

cdqm_devdrv_get_stars_irq_base get_stars_irq_base_func = NULL;

cdqm_devdrv_get_stars_reg_base get_stars_reg_base_func = NULL;

int tsdrv_cdqm_get_local_side(void)
{
    return CDQ_CREATE_BY_DEVICE;
}

static void tsdrv_cdqm_init_cdq_hardware(struct cdqm_dev_manage *cdq_dev, struct cdq_cfg_info *cdq_info,
    int mask_side)
{
    u32 cdq_len = cdq_info->batch_num * cdq_info->batch_size;

    if (mask_side == CDQ_CREATE_BY_DEVICE) {
        tsdrv_cdqm_cdq_f2nf_unmask_ctrl(cdq_dev->io_base, cdq_info->cdq_id);
    } else {
        tsdrv_cdqm_cdq_f2nf_unmask_host(cdq_dev->io_base, cdq_info->cdq_id);
    }

    tsdrv_cdqm_set_cdq_mem_addr(cdq_dev->io_base, cdq_info->cdq_id, cdq_info->mem_addr);
    tsdrv_cdqm_set_cdq_size(cdq_dev->io_base, cdq_info->cdq_id, cdq_len, cdq_info->batch_size);
    tsdrv_cdqm_set_cdq_pid(cdq_dev->io_base, cdq_info->cdq_id, (u32)cdq_info->pid);
    tsdrv_cdqm_set_cdq_streamid(cdq_dev->io_base, cdq_info->cdq_id, cdq_info->sid, cdq_info->ssid);
    tsdrv_cdqm_set_cdq_enable(cdq_dev->io_base, cdq_info->cdq_id, 1);
}

void tsdrv_cdqm_uninit_cdq_hardware(struct cdqm_dev_manage *cdq_dev, u32 cdq_id)
{
    tsdrv_cdqm_set_cdq_enable(cdq_dev->io_base, cdq_id, 0);
    tsdrv_cdqm_set_cdq_mem_addr(cdq_dev->io_base, cdq_id, 0);
    tsdrv_cdqm_set_cdq_size(cdq_dev->io_base, cdq_id, 0, 0);
    tsdrv_cdqm_set_cdq_pid(cdq_dev->io_base, cdq_id, 0);
    tsdrv_cdqm_set_cdq_streamid(cdq_dev->io_base, cdq_id, 0, 0);
    return;
}

int cdqm_dev_adapt_init(struct cdqm_dev_manage *cdq_dev)
{
    tsdrv_cdqm_cdq_irq_mask_init(cdq_dev->io_base);
    tsdrv_cdqm_set_kernel_type(cdq_dev->io_base, CDQM_KERNEL_TYPE);
    return 0;
}

int tsdrv_cdqm_create_msg_handle_by_side(struct cdqm_dev_manage *cdq_dev, struct cdq_cfg_info *cdq_info)
{
    u32 cdq_id;
    struct cdqm_cdq_manage *cdq_manage = NULL;
    struct trs_id_inst inst;
    int ret;

    inst.devid = cdq_dev->devid;
    inst.tsid = cdq_dev->tsid;
    ret = trs_id_alloc(&inst, TRS_CDQM_ID, &cdq_id);
    if (ret != 0) {
        trs_err("Alloc cdqm id fail. (devid=%u; tsid=%u; ret=%d).\n", cdq_dev->devid, cdq_dev->tsid, ret);
        return (int)DRV_ERROR_NO_CDQ_RESOURCES;
    }

    cdq_info->cdq_id = cdq_id;

    cdq_manage = &cdq_dev->cdq_manage[cdq_id];

    mutex_lock(&cdq_manage->cdq_mutex);

    tsdrv_cdqm_init_cdq_hardware(cdq_dev, cdq_info, CDQ_CREATE_BY_HOST);

    cdq_manage->valid = CDQ_VALID;

    cdq_manage->create_side = CDQ_CREATE_BY_HOST;

    mutex_unlock(&cdq_manage->cdq_mutex);

    return 0;
}

int tsdrv_cdqm_create_cdq_id(struct cdqm_dev_manage *cdq_dev, struct cdq_cfg_info *cdq_info)
{
    struct cdqm_cdq_manage *cdq_manage = NULL;
    struct trs_id_inst inst;
    int ret;

    inst.devid = cdq_dev->devid;
    inst.tsid = cdq_dev->tsid;
    ret = trs_id_alloc(&inst, TRS_CDQM_ID, &cdq_info->cdq_id);
    if (ret != 0) {
        trs_err("Alloc cdqm id fail. (devid=%u; tsid=%u).\n", cdq_dev->devid, cdq_dev->tsid);
        return (int)DRV_ERROR_NO_CDQ_RESOURCES;
    }

    cdq_manage = &cdq_dev->cdq_manage[cdq_info->cdq_id];

    mutex_lock(&cdq_manage->cdq_mutex);

    tsdrv_cdqm_init_cdq_hardware(cdq_dev, cdq_info, CDQ_CREATE_BY_DEVICE);

    mutex_unlock(&cdq_manage->cdq_mutex);
    return 0;
}

int tsdrv_cdqm_sync_cdq_info(struct cdqm_dev_manage *cdq_dev, struct cdq_cfg_info *cdq_info)
{
    return cdqm_sync_msg_create(cdq_dev->devid, cdq_dev->tsid, cdq_info);
}

int tsdrv_cdqm_destroy_handle_by_side(u32 cdq_id, struct cdqm_dev_manage *cdq_dev)
{
    struct cdqm_cdq_manage *cdq_manage = &cdq_dev->cdq_manage[cdq_id];
    struct trs_id_inst inst;
    int ret;

    if (cdq_manage->create_side != CDQ_CREATE_BY_HOST) {
        trs_err("not created by host, create side:%u, qid %u.\n", (u32)cdq_manage->create_side, cdq_id);
        return (int)DRV_ERROR_PARA_ERROR;
    }

    mutex_lock(&cdq_manage->cdq_mutex);

    tsdrv_cdqm_uninit_cdq_hardware(cdq_dev, cdq_id);

    cdqm_clear_cdq_mng(cdq_manage);

    mutex_unlock(&cdq_manage->cdq_mutex);

    inst.devid = cdq_dev->devid;
    inst.tsid = cdq_dev->tsid;
    ret = trs_id_free(&inst, TRS_CDQM_ID, cdq_id);
    if (ret != 0) {
        trs_err("Free cdqm id fail. (devid=%u; tsid=%u; id=%u).\n", cdq_dev->devid, cdq_dev->tsid, cdq_id);
        return (int)DRV_ERROR_PARA_ERROR;
    }

    return 0;
}

int tsdrv_cdqm_uninit_cdq_by_side(struct cdqm_dev_manage *cdq_dev, u32 cdq_id)
{
    struct cdqm_destroy_msg destroy_msg;
    struct cdqm_cdq_manage *cdq_manage = &cdq_dev->cdq_manage[cdq_id];
    struct trs_id_inst inst;
    int ret;

    if (cdq_manage->recycle_pid == 0) {
        destroy_msg.pid = cdq_manage->config_info.pid;
    } else {
        destroy_msg.pid = cdq_manage->recycle_pid;
    }
    destroy_msg.cdq_id = cdq_id;

    ret = cdqm_sync_msg_destroy(cdq_dev->devid, cdq_dev->tsid, &destroy_msg);
    if (ret != 0) {
        trs_err("pid %d cdq_id %u host destroy failed:%d.\n", cdq_manage->config_info.pid,
            cdq_manage->config_info.cdq_id, ret);
        return (int)DRV_ERROR_PARA_ERROR;
    }

    tsdrv_cdqm_uninit_cdq_hardware(cdq_dev, cdq_id);

    inst.devid = cdq_dev->devid;
    inst.tsid = cdq_dev->tsid;
    ret = trs_id_free(&inst, TRS_CDQM_ID, cdq_id);
    if (ret != 0) {
        trs_err("Free cdqm id fail. (devid=%u; tsid=%u; id=%u).\n", cdq_dev->devid, cdq_dev->tsid, cdq_id);
        return (int)DRV_ERROR_PARA_ERROR;
    }

    return 0;
}

static void cdqm_f2nf_task(unsigned long data)
{
    struct cdqm_dev_manage *cdq_dev = (struct cdqm_dev_manage *)data;
    struct cdqm_cdq_manage *cdq_manage = NULL;
    u32 l2_stats, l2_bit, qid;
    u32 cdq_g;

    for (cdq_g = 0; cdq_g < MAX_CDQ_GROUP; cdq_g++) {
        tsdrv_cdqm_read_f2nf_int_status1(cdq_dev->io_base, cdq_g, &l2_stats);
        if (l2_stats == 0) {
            continue;
        }

        trs_debug("Show l2 reg info. (cdq_group=%u; sts2_l2=0x%x)\n", cdq_g, l2_stats);

        for (l2_bit = 0; l2_bit < CDQM_NUM_PER_GRP; l2_bit++) {
            if ((l2_stats & (1U << l2_bit)) == 0) {
                continue;
            }

            /* get qid and wake up cdq wait_event */
            qid = l2_bit + cdq_g * CDQM_NUM_PER_GRP;
            cdq_manage = &cdq_dev->cdq_manage[qid];

            tsdrv_cdqm_get_cdq_rdy_idx(cdq_dev->io_base, qid, cdq_manage->config_info.batch_num,
                &cdq_manage->use_batch_head);

            wake_up_interruptible(&cdq_manage->wq);

            trs_debug("Wake up cdq wait event. (devid=%u; tsid=%u; qid=%u)\n", cdq_dev->devid, cdq_dev->tsid, qid);
        }
        tsdrv_cdqm_f2nf_int_clr1(cdq_dev->io_base, cdq_g, l2_stats);
    }
    tsdrv_cdqm_f2nf_int_unmask_l1(cdq_dev->io_base);
}

void f2nf_tasklet_init(struct cdqm_dev_manage *cdq_dev)
{
    tasklet_init(&cdq_dev->f2nf_task, cdqm_f2nf_task, (unsigned long)(uintptr_t)cdq_dev);
}
