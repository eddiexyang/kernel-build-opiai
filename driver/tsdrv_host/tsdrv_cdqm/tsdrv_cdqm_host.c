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
#ifndef TSDRV_UT
#ifdef CFG_FEATURE_CDQM
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <asm/atomic.h>
#include <linux/hashtable.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/interrupt.h>

#include "ascend_kernel_hal.h"
#include "securec.h"
#include "devdrv_common.h"
#include "devdrv_interface.h"
#include "tsdrv_device.h"
#include "tsdrv_sync.h"
#include "cdq_manager.h"
#include "tsdrv_cdqm_module.h"

int tsdrv_cdqm_get_local_side(void)
{
    return CDQ_CREATE_BY_HOST;
}

int __attribute__((unused)) tsdrv_cdqm_io_remap_base(u32 devid, enum devdrv_addr_type type, void **__iomem base)
{
    int ret;
    u64 base_addr;
    size_t reg_size;

    ret = devdrv_get_addr_info(devid, type, 0, &base_addr, &reg_size);
    if (ret != 0) {
        TSDRV_PRINT_ERR("dev(%d) host get addr info failed(%d).\n", devid, ret);
        return -EINVAL;
    }

    *base = ioremap(base_addr, reg_size);
    if (*base == NULL) {
        TSDRV_PRINT_ERR("dev(%d) host ioremap failed(%d).\n", devid, ret);
        return -ENOMEM;
    }
    return 0;
}

int tsdrv_cdqm_get_cdqm_reg(u32 devid, u64 *addr, size_t *size)
{
    int ret;

    ret = devdrv_get_addr_info(devid, DEVDRV_ADDR_STARS_CDQM_BASE, 0, addr, size);
    if (ret != 0) {
        TSDRV_PRINT_ERR("dev(%d) get addr info failed(%d).\n", devid, ret);
        return ret;
    }

    return 0;
}

int tsdrv_cdqm_get_intr_reg(u32 devid, u64 *addr, size_t *size)
{
    int ret;

    ret = devdrv_get_addr_info(devid, DEVDRV_ADDR_STARS_INTR_BASE, 0, addr, size);
    if (ret != 0) {
        TSDRV_PRINT_ERR("dev(%d) get intr info failed(%d).\n", devid, ret);
        return ret;
    }

    return 0;
}

int tsdrv_cdqm_get_f2nf_irq(u32 devid, u32 *vector_id, u32 *irq)
{
    int ret;

    ret = devdrv_get_cdqm_irq_vector_id(devid, 0, vector_id);
    if (ret != 0) {
        TSDRV_PRINT_ERR("dev(%d) get irq failed(%d).\n", devid, ret);
        return ret;
    }

    ret = devdrv_get_irq_vector(devid, *vector_id, irq);
    if (ret != 0) {
        TSDRV_PRINT_ERR("dev(%d) get irq vector failed(%d) vector_id %d.\n", devid, ret, *vector_id);
        return ret;
    }

    return 0;
}


int tsdrv_cdqm_drv_manage_adapt_init(struct cdqm_dev_manage *cdq_dev)
{
    int ret;

    ret = tsdrv_msg_sync_cdqm_init(cdq_dev->devid, cdq_dev->tsid, cdq_dev->f2nf_vector_id);
    if (ret != 0) {
        TSDRV_PRINT_ERR("cdqm manage_adapt_init dev %d ts %d irq %d failed:%d", cdq_dev->devid, cdq_dev->tsid,
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
        TSDRV_PRINT_ERR("cdq_id %d illegal.\n", cdq_id);
        return DRV_ERROR_PARA_ERROR;
    }

    cdq_manage = &cdq_dev->cdq_manage[cdq_id];

    mutex_lock(&cdq_manage->cdq_mutex);

    if (cdq_manage->valid != 0) {
        TSDRV_PRINT_ERR("dev_cdq_i %d valid %d.\n", cdq_id, cdq_manage->valid);
        mutex_unlock(&cdq_manage->cdq_mutex);
        return DRV_ERROR_INNER_ERR;
    }

    cdq_manage->valid = CDQ_VALID;
    cdq_manage->config_info = *cdq_info;
    cdq_manage->create_side = CDQ_CREATE_BY_DEVICE;

    mutex_unlock(&cdq_manage->cdq_mutex);

    return 0;
}

int tsdrv_cdqm_create_cdq_id(struct cdqm_dev_manage *cdq_dev, struct cdq_cfg_info *cdq_info, u32 *cdq_id)
{
    int ret;

    ret = tsdrv_msg_sync_cdqm_create(cdq_dev->devid, cdq_dev->tsid, cdq_info, cdq_id);
    if (ret != 0) {
        TSDRV_PRINT_ERR("device create cdq failed:%u, dev:%d tsid %u.\n", ret, cdq_dev->devid, cdq_dev->tsid);
        return ret;
    }

    if (*cdq_id >= MAX_QUEUE_NUM || cdq_dev->cdq_manage[*cdq_id].valid == CDQ_VALID) {
        TSDRV_PRINT_ERR("device create err, qid:%u, valid %u\n", *cdq_id, cdq_dev->cdq_manage[*cdq_id].valid);
        return DRV_ERROR_PARA_ERROR;
    }

    return 0;
}

int tsdrv_cdqm_create_cdq_by_side(struct cdq_cfg_info *create_msg, struct cdqm_dev_manage *cdq_dev)
{
    u32 cdq_id = create_msg->cdq_id;
    struct cdqm_cdq_manage *cdq_manage = NULL;

    cdq_manage = &cdq_dev->cdq_manage[cdq_id];

    mutex_lock(&cdq_manage->cdq_mutex);

    cdq_manage->valid = CDQ_VALID;

    cdq_manage->create_side = CDQ_CREATE_BY_HOST;

    mutex_unlock(&cdq_manage->cdq_mutex);

    return 0;
}

int tsdrv_cdqm_sync_cdq_info(struct cdqm_dev_manage *cdq_dev, struct cdq_cfg_info *create_msg, u32 cdq_id)
{
    return 0;
}

int tsdrv_cdqm_destroy_handle_by_side(u32 devid, u32 tsid, u32 cdq_id, struct cdqm_dev_manage *cdq_dev)
{
    struct cdqm_cdq_manage *cdq_manage = &cdq_dev->cdq_manage[cdq_id];

    if (cdq_manage->create_side != CDQ_CREATE_BY_DEVICE) {
        TSDRV_PRINT_ERR("not created by host, create side:%d, qid %d.\n", cdq_manage->create_side, cdq_id);
        return DRV_ERROR_PARA_ERROR;
    }

    mutex_lock(&cdq_manage->cdq_mutex);

    cdqm_clear_cdq_info(cdq_manage);

    mutex_unlock(&cdq_manage->cdq_mutex);

    return 0;
}
void tsdrv_cdqm_uninit_cdq_hardware(struct cdqm_dev_manage *cdq_dev, u32 cdq_id)
{
    return;
}

int tsdrv_cdqm_uninit_cdq_by_side(u32 devid, u32 tsid, u32 cdq_id, struct cdqm_dev_manage *cdq_dev)
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

    ret = tsdrv_msg_sync_cdqm_destroy(devid, tsid, &destroy_msg);
    if (ret != 0) {
        TSDRV_PRINT_ERR("pid %d cdq_id %d device destroy failed:%d.\n",
            cdq_manage->config_info.pid, cdq_manage->config_info.cdq_id, ret);
        return DRV_ERROR_PARA_ERROR;
    }

    return 0;
}

#endif /* CFG_FEATURE_CDQM */

#else /* TSDRV_UT */

void ut_host_tsdrv_cdqm_host(void)
{
}

#endif /* TSDRV_UT */

