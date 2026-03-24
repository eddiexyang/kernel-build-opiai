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
#include <linux/printk.h>
#include <asm/atomic.h>
#include <linux/hashtable.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/mod_devicetable.h>
#include <linux/interrupt.h>
#include <linux/kallsyms.h>

#include "devdrv_manager_comm.h"
#include "tsdrv_interface.h"
#include "tsdrv_platform_interface.h"
#include "devdrv_interface.h"
#include "ascend_kernel_hal.h"
#include "cdq_manager.h"
#include "securec.h"
#include "devdrv_user_common.h"
#include "tsdrv_device.h"
#include "tsdrv_sync.h"
#include "tsdrv_cdqm_module.h"
#include "cdq_manager.h"
#include "tsdrv_cdqm_dev.h"

typedef int (*cdqm_devdrv_get_stars_irq_base)(u32 dev_id, int *irq);
typedef int (*cdqm_devdrv_get_stars_reg_base)(u32 dev_id, u64 *base_addr);

cdqm_devdrv_get_stars_irq_base get_stars_irq_base_func = NULL;
cdqm_devdrv_get_stars_reg_base get_stars_reg_base_func = NULL;
int tsdrv_stars_irq_base[MAX_CHIP_NUM] = {0};

int tsdrv_cdqm_get_local_side(void)
{
    return CDQ_CREATE_BY_DEVICE;
}

STATIC void tsdrv_cdqm_init_cdq_hardware(struct cdqm_dev_manage *cdq_dev, struct cdq_cfg_info *cdq_info, int mask_side)
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
void tsdrv_cdqm_disable_cdq(u32 cdq_id, struct cdqm_dev_manage *cdq_dev)
{
    tsdrv_cdqm_set_cdq_enable(cdq_dev->io_base, cdq_id, 0);
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

int tsdrv_cdqm_set_stars_irq_base(u32 devid, int irq)
{
    if (devid < MAX_CHIP_NUM) {
        tsdrv_stars_irq_base[devid] = irq;
        return 0;
    }
    TSDRV_PRINT_ERR("devid %u.(max:%u)", devid, (u32)MAX_CHIP_NUM);
    return DRV_ERROR_NO_DEVICE;
}
EXPORT_SYMBOL(tsdrv_cdqm_set_stars_irq_base);

STATIC int tsdrv_cdqm_get_stars_irq_base(u32 devid, int *irq)
{
    if (devid < MAX_CHIP_NUM) {
        *irq = tsdrv_stars_irq_base[devid];
        return DRV_ERROR_NONE;
    }
    TSDRV_PRINT_ERR("devid %u.(max:%u)", devid, (u32)MAX_CHIP_NUM);
    return DRV_ERROR_INVALID_DEVICE;
}

int tsdrv_cdqm_get_cdqm_reg(u32 devid, u64 *addr, size_t *size)
{
    int ret;

    ret = tsdrv_get_stars_base_addr(devid, addr);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Get reg base failed. (devid=%u; ret=%d)\n", devid, ret);
        return ret;
    }

    *addr += STARS_CDQM_BASE_ADDR;
    *size = STARS_CDQM_SIZE;

    return 0;
}

int tsdrv_cdqm_get_intr_reg(u32 devid, u64 *addr, size_t *size)
{
    int ret;

    ret = tsdrv_get_stars_base_addr(devid, addr);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Get reg base failed. (devid=%u; ret=%d)\n", devid, ret);
        return ret;
    }

    *addr += STARS_INT_REG_BASE_ADDR;
    *size = STARS_INT_REG_SIZE;

    return 0;
}

int tsdrv_cdqm_get_f2nf_irq(u32 devid, u32 *vector_id, u32 *irq)
{
    int ret;
    int irq_val = (int)(*irq);

    ret = tsdrv_cdqm_get_stars_irq_base(devid, &irq_val);
    if (ret != 0) {
        TSDRV_PRINT_ERR("dev(%u) get irq failed(%d).\n", devid, ret);
        return ret;
    }

    *irq = (u32)irq_val;
    *irq += INT_CDQM_F2NF_NS_CTRL;
    *vector_id = *irq;

    return 0;
}

int tsdrv_cdqm_drv_manage_adapt_init(struct cdqm_dev_manage *cdq_dev)
{
    tsdrv_cdqm_cdq_irq_mask_init(cdq_dev->io_base);
    tsdrv_cdqm_set_kernel_type(cdq_dev->io_base, CDQM_KERNEL_TYPE);
    return 0;
}

STATIC int cdqm_get_free_cdq_id(struct cdqm_dev_manage *cdq_dev)
{
    struct cdqm_cdq_manage *cdq_manage = cdq_dev->cdq_manage;
    int i, cdq_id;

    cdq_id = MAX_CDQM_CDQ_NUM;

    mutex_lock(&cdq_dev->dev_mutex);

    for (i = 0; i < MAX_CDQM_CDQ_NUM; i++) {
        if (cdq_manage[i].valid == CDQ_INVALID) {
            cdq_id = i;
            cdq_manage[i].valid = CDQ_VALID;
            break;
        }
    }
    mutex_unlock(&cdq_dev->dev_mutex);

    return cdq_id;
}

int tsdrv_cdqm_create_msg_handle_by_side(struct cdqm_dev_manage *cdq_dev, struct cdq_cfg_info *cdq_info)
{
    int cdq_id;
    struct cdqm_cdq_manage *cdq_manage = NULL;

    cdq_id = cdqm_get_free_cdq_id(cdq_dev);
    if (cdq_id == MAX_CDQM_CDQ_NUM) {
        TSDRV_PRINT_ERR("dev %u ts %u no free cdq id .\n", cdq_dev->devid, cdq_dev->tsid);
        return (int)DRV_ERROR_NO_CDQ_RESOURCES;
    }

    cdq_info->cdq_id = (u32)cdq_id;

    cdq_manage = &cdq_dev->cdq_manage[cdq_id];

    mutex_lock(&cdq_manage->cdq_mutex);

    tsdrv_cdqm_init_cdq_hardware(cdq_dev, cdq_info, CDQ_CREATE_BY_HOST);

    cdq_manage->valid = CDQ_VALID;

    cdq_manage->create_side = CDQ_CREATE_BY_HOST;

    mutex_unlock(&cdq_manage->cdq_mutex);

    return 0;
}

int tsdrv_cdqm_create_cdq_id(struct cdqm_dev_manage *cdq_dev, struct cdq_cfg_info *cdq_info, u32 *cdq_id)
{
    struct cdqm_cdq_manage *cdq_manage = NULL;

    *cdq_id = (u32)cdqm_get_free_cdq_id(cdq_dev);
    if (*cdq_id == MAX_CDQM_CDQ_NUM) {
        TSDRV_PRINT_ERR("dev %u ts %u no free cdq id .\n", cdq_dev->devid, cdq_dev->tsid);
        return (int)DRV_ERROR_NO_CDQ_RESOURCES;
    }

    cdq_info->cdq_id = *cdq_id;

    cdq_manage = &cdq_dev->cdq_manage[*cdq_id];

    mutex_lock(&cdq_manage->cdq_mutex);

    tsdrv_cdqm_init_cdq_hardware(cdq_dev, cdq_info, CDQ_CREATE_BY_DEVICE);

    mutex_unlock(&cdq_manage->cdq_mutex);
    return 0;
}

int tsdrv_cdqm_sync_cdq_info(struct cdqm_dev_manage *cdq_dev, struct cdq_cfg_info *create_msg, u32 cdq_id)
{
    return tsdrv_msg_sync_cdqm_create(cdq_dev->devid, cdq_dev->tsid, create_msg, &cdq_id);
}

int tsdrv_cdqm_destroy_handle_by_side(u32 devid, u32 tsid, u32 cdq_id, struct cdqm_dev_manage *cdq_dev)
{
    struct cdqm_cdq_manage *cdq_manage = &cdq_dev->cdq_manage[cdq_id];

    if (cdq_manage->create_side != CDQ_CREATE_BY_HOST) {
        TSDRV_PRINT_ERR("not created by host, create side:%u, qid %u.\n", (u32)cdq_manage->create_side, cdq_id);
        return (int)DRV_ERROR_PARA_ERROR;
    }

    mutex_lock(&cdq_manage->cdq_mutex);

    tsdrv_cdqm_uninit_cdq_hardware(cdq_dev, cdq_id);

    cdqm_clear_cdq_info(cdq_manage);

    mutex_unlock(&cdq_manage->cdq_mutex);

    return 0;
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
        TSDRV_PRINT_ERR("pid %d cdq_id %u host destroy failed:%d.\n", cdq_manage->config_info.pid,
            cdq_manage->config_info.cdq_id, ret);
        return (int)DRV_ERROR_PARA_ERROR;
    }

    tsdrv_cdqm_uninit_cdq_hardware(cdq_dev, cdq_id);

    return 0;
}

#endif /* CFG_FEATURE_CDQM */
#else /* TSDRV_UT */

void ut_device_tsdrv_cdqm_dev(void)
{
}

#endif /* TSDRV_UT */
