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
* Create: 2022-7-9
*/
#include <linux/io.h>

#include "ascend_kernel_hal.h"
#include "devdrv_interface.h"
#include "trs_pub_def.h"

#include "cdq_manager.h"
#include "cdqm_core.h"

int tsdrv_cdqm_get_local_side(void)
{
    return CDQ_CREATE_BY_HOST;
}

int cdqm_dev_adapt_init(struct cdqm_dev_manage *cdq_dev)
{
    int ret;

    /* slice 需要适配 */
    ret = cdqm_sync_msg_init(cdq_dev);
    if (ret != 0) {
        trs_err("cdqm manage_adapt_init dev %d ts %d irq %d failed:%d.\n", cdq_dev->devid, cdq_dev->tsid,
            cdq_dev->f2nf_irq, ret);
        return ret;
    }

    return 0;
}

int tsdrv_cdqm_create_msg_handle_by_side(struct cdqm_dev_manage *cdq_dev, struct cdq_cfg_info *cdq_info)
{
    int cdq_id = cdq_info->cdq_id;
    struct cdqm_cdq_manage *cdq_manage = NULL;

    if (cdq_id >= MAX_CDQM_CDQ_NUM) {
        trs_err("cdq_id %d illegal.\n", cdq_id);
        return DRV_ERROR_PARA_ERROR;
    }

    cdq_manage = &cdq_dev->cdq_manage[cdq_id];

    mutex_lock(&cdq_manage->cdq_mutex);

    if (cdq_manage->valid != 0) {
        trs_err("dev_cdq_i %d valid %d.\n", cdq_id, cdq_manage->valid);
        mutex_unlock(&cdq_manage->cdq_mutex);
        return DRV_ERROR_INNER_ERR;
    }

    cdq_manage->valid = CDQ_VALID;
    cdq_manage->config_info = *cdq_info;
    cdq_manage->create_side = CDQ_CREATE_BY_DEVICE;

    mutex_unlock(&cdq_manage->cdq_mutex);

    return 0;
}

int tsdrv_cdqm_create_cdq_id(struct cdqm_dev_manage *cdq_dev, struct cdq_cfg_info *cdq_info)
{
    int ret;

    ret = cdqm_sync_msg_create(cdq_dev->devid, cdq_dev->tsid, cdq_info);
    if (ret != 0) {
        trs_err("device create cdq failed:%u, dev:%d tsid %u.\n", ret, cdq_dev->devid, cdq_dev->tsid);
        return ret;
    }

    if (cdq_info->cdq_id >= MAX_QUEUE_NUM || cdq_dev->cdq_manage[cdq_info->cdq_id].valid == CDQ_VALID) {
        trs_err("device create err, qid:%u, valid %u\n",
            cdq_info->cdq_id, cdq_info->cdq_id >= MAX_QUEUE_NUM ? 0 : cdq_dev->cdq_manage[cdq_info->cdq_id].valid);
        return DRV_ERROR_PARA_ERROR;
    }

    return 0;
}

int tsdrv_cdqm_sync_cdq_info(struct cdqm_dev_manage *cdq_dev, struct cdq_cfg_info *cdq_info)
{
    return 0;
}

int tsdrv_cdqm_destroy_handle_by_side(u32 cdq_id, struct cdqm_dev_manage *cdq_dev)
{
    struct cdqm_cdq_manage *cdq_manage = &cdq_dev->cdq_manage[cdq_id];

    if (cdq_manage->create_side != CDQ_CREATE_BY_DEVICE) {
        trs_err("not created by host, create side:%d, qid %d.\n", cdq_manage->create_side, cdq_id);
        return DRV_ERROR_PARA_ERROR;
    }

    mutex_lock(&cdq_manage->cdq_mutex);

    cdqm_clear_cdq_mng(cdq_manage);

    mutex_unlock(&cdq_manage->cdq_mutex);

    return 0;
}
void tsdrv_cdqm_uninit_cdq_hardware(struct cdqm_dev_manage *cdq_dev, u32 cdq_id)
{
    return;
}

int tsdrv_cdqm_uninit_cdq_by_side(struct cdqm_dev_manage *cdq_dev, u32 cdq_id)
{
    int ret;
    struct cdqm_destroy_msg destroy_msg;
    struct cdqm_cdq_manage *cdq_manage = &cdq_dev->cdq_manage[cdq_id];

    if (cdq_manage->recycle_pid == 0) {
        destroy_msg.pid = cdq_manage->config_info.pid;
    } else {
        destroy_msg.pid = cdq_manage->recycle_pid;
    }
    destroy_msg.cdq_id = cdq_id;

    ret = cdqm_sync_msg_destroy(cdq_dev->devid, cdq_dev->tsid, &destroy_msg);
    if (ret != 0) {
        trs_err("pid %d cdq_id %d device destroy failed:%d.\n",
            cdq_manage->config_info.pid, cdq_manage->config_info.cdq_id, ret);
        return DRV_ERROR_PARA_ERROR;
    }

    return 0;
}

static void cdqm_f2nf_task(unsigned long data)
{
    struct cdqm_dev_manage *cdq_dev = (struct cdqm_dev_manage *)data;
    struct cdqm_cdq_manage *cdq_manage = NULL;
    u32 sts2_l2, cdq_ps, qid;
    u32 cdq_s;

    // 轮询所有CDQ资源组的STS2_L1寄存器
    for (cdq_s = 0; cdq_s < MAX_CDQ_SLICE; cdq_s++) {
        tsdrv_cdqm_read_f2nf_int_sts2_l2(cdq_dev->io_base, cdq_s, &sts2_l2);
        if (sts2_l2 == 0) {
            continue;
        }

        tsdrv_cdqm_f2nf_int_clr2(cdq_dev->io_base, cdq_s, sts2_l2);
        trs_debug("slice_id %u sts2_l2 0x%x\n", cdq_s, sts2_l2);

        for (cdq_ps = 0; cdq_ps < SLICE_CDQ_NUM; cdq_ps++) {
            if ((sts2_l2 & (1U << cdq_ps)) == 0) {
                continue;
            }
            // wakeup对应队列的wait_event
            qid = cdq_ps + cdq_s * SLICE_CDQ_NUM;
            cdq_manage = &cdq_dev->cdq_manage[qid];

            tsdrv_cdqm_get_cdq_rdy_idx(cdq_dev->io_base, qid, cdq_manage->config_info.batch_num,
                &cdq_manage->use_batch_head);

            wake_up_interruptible(&cdq_manage->wq);

            trs_debug("dev %u ts %u cdq %u waked\n", cdq_dev->devid, cdq_dev->tsid, qid);
        }
        tsdrv_cdqm_set_int_f2nf_clr(cdq_dev->io_base, cdq_s);
    }
}

void f2nf_tasklet_init(struct cdqm_dev_manage *cdq_dev)
{
    /* slice 需要适配 */
    tasklet_init(&cdq_dev->f2nf_task, cdqm_f2nf_task, (unsigned long)(uintptr_t)cdq_dev);
}
