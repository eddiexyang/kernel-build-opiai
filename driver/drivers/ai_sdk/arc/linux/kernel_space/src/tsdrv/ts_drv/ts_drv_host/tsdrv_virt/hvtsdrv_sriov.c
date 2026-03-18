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
 * Create: 2022-05-21
 */
#include "hvtsdrv_init.h"
#ifdef CFG_FEATURE_VFIO
#include <linux/vmalloc.h>
#include <linux/spinlock.h>

#include "securec.h"
#include "tsdrv_log.h"
#include "tsdrv_device.h"
#include "tsdrv_nvme.h"
#include "devdrv_cb.h"
#include "devdrv_cqsq.h"
#include "devdrv_shm.h"
#include "devdrv_id.h"
#include "devdrv_devinit.h"

#include "devdrv_manager_common.h"
#include "devdrv_interface.h"
#include "virtmng_interface.h"
#include "davinci_interface.h"

#ifdef CFG_FEATURE_CDQM
#include "tsdrv_cdqm_module.h"
#endif

#include "hvtsdrv_proc.h"
#include "hvtsdrv_tsagent.h"
#include "hvtsdrv_id.h"
#include "hvtsdrv_sriov_id.h"
#include "hvtsdrv_cqsq.h"

static int hvtsdrv_cdqm_drv_init(u32 devid, u32 tsnum)
{
    return 0;
}

static void hvtsdrv_cdqm_drv_uninit(u32 devid, u32 tsnum)
{
}

static int hvtsdrv_basic_module_init(u32 devid, u32 tsnum)
{
    u32 tsid, i;
    int ret;

    ret = tsdrv_mbox_init(devid, tsnum);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to init mailbox. (devid=%u; ret=%d)\n", devid, ret);
        return ret;
    }

    ret = tsdrv_cq_irq_init(devid, tsnum);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to init cq irq. (devid=%u; ret=%d)\n", devid, ret);
        goto err_cb_irq_init;
    }

    for (tsid = 0; tsid < tsnum; tsid++) {
        ret = hvtsdrv_ids_init(devid, tsid);
        if (ret != 0) {
            TSDRV_PRINT_ERR("Failed to init ids. (devid=%u; tsid=%u; ret=%d)\n", devid, tsid, ret);
            goto err_hvtsdrv_ids_init;
        }
    }
    ret = tsdrv_nvme_init(devid, TSDRV_PM_FID, tsnum);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to init nvme. (devid=%u; ret=%d)\n", devid, ret);
        goto err_nvme_init;
    }

    /* must be initiated after id */
    ret = callback_dev_init(devid, TSDRV_PM_FID, tsnum);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to init callback. (devid=%u; ret=%d)\n", devid, ret);
        goto err_callback_init;
    }

    ret = hvtsdrv_cdqm_drv_init(devid, tsnum);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to init cdqm. (devid=%u; ret=%d)\n", devid, ret);
        goto err_cdqm_drv_init;
    }

    return 0;

err_cdqm_drv_init:
    callback_dev_exit(devid, TSDRV_PM_FID, tsnum);
err_callback_init:
    tsdrv_nvme_exit(devid, TSDRV_PM_FID, tsnum);
err_nvme_init:
err_hvtsdrv_ids_init:
    for (i = 0; i < tsid; i++) {
        hvtsdrv_ids_uninit(devid, i);
    }
    tsdrv_cq_irq_exit(devid, tsnum);
err_cb_irq_init:
    tsdrv_mbox_exit(devid, tsnum);
    return ret;
}

static void hvtsdrv_basic_module_uninit(u32 devid, u32 tsnum)
{
    u32 tsid;

    hvtsdrv_cdqm_drv_uninit(devid, tsnum);
    callback_dev_exit(devid, TSDRV_PM_FID, tsnum);
    tsdrv_nvme_exit(devid, TSDRV_PM_FID, tsnum);
    for (tsid = 0; tsid < tsnum; tsid++) {
        hvtsdrv_ids_uninit(devid, tsid);
    }
    tsdrv_cq_irq_exit(devid, tsnum);
    tsdrv_mbox_exit(devid, tsnum);
}

int hvtsdrv_instance_check(struct vmngh_client_instance *instance)
{
    if (instance == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("The instance is NULL.\n");
        return -EINVAL;
#endif
    }

    if (instance->dev_ctrl == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("The instance dev_ctrl is NULL.\n");
        return -EINVAL;
#endif
    }

    if (instance->type != VMNG_CLIENT_TYPE_TSDRV) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("The instance type is not correct. (type=%d).\n", instance->type);
        return -EINVAL;
#endif
    }

    if (instance->dev_ctrl->dev_id >= TSDRV_MAX_DAVINCI_NUM) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("The devid is invalid. (devid=%u)\n", instance->dev_ctrl->dev_id);
        return -EINVAL;
#endif
    }

    if (instance->dev_ctrl->dtype >= INVALID_CAPACITY) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("The dtype is invalid. (dtype=%d).\n", instance->dev_ctrl->dtype);
        return -EINVAL;
#endif
    }

    if (!tsdrv_pm_ctx_is_finished(instance->dev_ctrl->dev_id)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Device in pm has process running, not support. (devid=%u)\n", instance->dev_ctrl->dev_id);
        return -EINVAL;
#endif
    }

    TSDRV_PRINT_INFO("(devid=%u; fid=%u; dtype=%d; id_bitmap=0x%x; aic_bitmap=0x%x)\n", instance->dev_ctrl->dev_id,
        instance->dev_ctrl->vfid, instance->dev_ctrl->dtype, instance->dev_ctrl->vf_cfg.accelerator.rtsq_slice_bitmap,
        instance->dev_ctrl->vf_cfg.accelerator.aic_bitmap);

    return 0;
}

static int hvtsdrv_drv_register(u32 devid, u32 tsnum, enum TSDRV_CAPACITY capacity)
{
    int ret;

    ret = tsdrv_dev_register(devid, TSDRV_PM_FID, capacity);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Failed to register tsdrv_dev. (devid=%u; ret=%d)\n", devid, ret);
        return ret;
#endif
    }

    ret = hvtsdrv_basic_module_init(devid, tsnum);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Failed to init basic module. (devid=%u; ret=%d)\n", devid, ret);
        tsdrv_dev_unregister(devid, TSDRV_PM_FID);
        return ret;
#endif
    }

    return 0;
}

static void hvtsdrv_drv_unregister(u32 devid, u32 tsnum)
{
    struct tsdrv_dev_resource *dev_res = tsdrv_get_dev_resource(devid, TSDRV_PM_FID);
    hvtsdrv_basic_module_uninit(devid, tsnum);
    tsdrv_dev_unregister(devid, TSDRV_PM_FID);
    atomic_set(&dev_res->status, TSDRV_DEV_ACTIVE);
}

void hvtsdrv_vm_res_recycle_work(struct work_struct *work)
{
    struct tsdrv_dev_resource *dev_res = NULL;
    struct tsdrv_device *tsdrv_dev = NULL;
    u32 devid, fid, tsid, tsnum, phy_devid, vfid;
    int ret;

    dev_res = container_of(work, struct tsdrv_dev_resource, vm_res_recycle_work);
    tsdrv_dev = container_of(dev_res, struct tsdrv_device, dev_res[dev_res->fid]);
    devid = tsdrv_dev->devid;
    fid = dev_res->fid;
    tsnum = tsdrv_get_dev_tsnum(devid);
    /* If TS exception occurs, some resources fail to be reclaimed */
    tsdrv_wait_dev_ids_recycle_complete(devid, fid);

    ret = devdrv_get_pfvf_id_by_devid(devid, &phy_devid, &vfid);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Failed to get phy_devid and vfid. (devid=%u)\n", devid);
        return;
#endif
    }
#ifdef CFG_FEATURE_SUPPORT_VSQ_MNG
    tsdrv_notice_tsagent_dev_destroy(devid, TSDRV_PM_FID);
#endif

    tsdrv_ids_release(devid, fid, tsnum);
    hvtsdrv_drv_unregister(devid, tsnum);

    tsdrv_fid_num_dec(phy_devid);
    for (tsid = 0; tsid < tsnum; tsid++) {
        if (tsdrv_get_fid_num(phy_devid) == 1) {
            tsdrv_reserved_sqcq_restore(phy_devid, vfid, tsid);
        }
    }

    TSDRV_PRINT_INFO("Recycle vm resourse success. (devid=%u; fid=%u)\n", devid, fid);
}

static void hvtsdrv_free_pm_ids(u32 devid, u32 fid, u32 tsnum)
{
    struct tsdrv_ts_resource *ts_res = NULL;
    struct devdrv_ts_sq_info *sq_info = NULL;
    struct devdrv_ts_cq_info *cq_info = NULL;
    struct devdrv_sq_sub_info *sq_sub = NULL;
    struct devdrv_cq_sub_info *cq_sub = NULL;
    u32 tsid, id;

    for (tsid = 0; tsid < tsnum; tsid++) {
        ts_res = tsdrv_get_ts_resoruce(devid, fid, tsid);
        for (id = 0; id < ts_res->sqcq_reserved_num; id++) {
            sq_info = tsdrv_get_sq_info(devid, fid, tsid, ts_res->reserved_sqcq[id].sq_info.index);
            sq_sub = (struct devdrv_sq_sub_info *)sq_info->sq_sub;
            if (sq_sub->phy_addr != 0 || sq_sub->vaddr != 0) {
                sq_sub->phy_addr = 0;
                sq_sub->vaddr = 0;
            }
            cq_info = tsdrv_get_cq_info(devid, fid, tsid, ts_res->reserved_sqcq[id].cq_info.index);
            cq_sub = (struct devdrv_cq_sub_info *)cq_info->cq_sub;
            if (cq_sub->phy_addr != 0 || cq_sub->virt_addr != 0) {
                cq_sub->phy_addr = 0;
                cq_sub->virt_addr = 0;
            }
            TSDRV_PRINT_DEBUG("Free sqcq. (devid=%u; fid=%u; tsid=%u; sqid=%u; cqid=%u)\n", devid, fid, tsid,
                ts_res->reserved_sqcq[id].sq_info.index, ts_res->reserved_sqcq[id].cq_info.index);
        }
    }
    tsdrv_ids_release(devid, fid, tsnum);
}

int hvtsdrv_init_mia_dev_instance(u32 dev_id, u32 fid, u32 dtype)
{
    return 0;
}

int hvtsdrv_uninit_mia_dev_instance(u32 dev_id, u32 fid)
{
    return 0;
}

int hvtsdrv_init_container_instance(struct vmngh_client_instance *instance)
{
    struct tsdrv_dev_resource *dev_res = NULL;
    u32 devid, tsnum, phy_devid, vfid;
    int ret;

    if (hvtsdrv_instance_check(instance) != 0) {
        TSDRV_PRINT_ERR("Failed to check vmngh instance.\n");
        return -EINVAL;
    }

    devid = instance->dev_ctrl->dev_id;
    dev_res = tsdrv_get_dev_resource(devid, TSDRV_PM_FID);
    dev_res->aic_bitmap = instance->dev_ctrl->vf_cfg.accelerator.aic_bitmap;

    tsnum = tsdrv_get_dev_tsnum(devid);
    ret = devdrv_get_pfvf_id_by_devid(devid, &phy_devid, &vfid);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to get phy_devid and vfid. (devid=%u)\n", devid);
        return ret;
    }
    hvtsdrv_free_pm_ids(phy_devid, TSDRV_PM_FID, tsnum);

    atomic_set(&dev_res->status, TSDRV_DEV_INACTIVE);
    ret = hvtsdrv_drv_register(devid, tsnum, (enum TSDRV_CAPACITY)instance->dev_ctrl->dtype);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to register. (devid=%u)\n", devid);
        return ret;
    }

    dev_mnt_vdev_register_client(devid, TSDRV_PM_FID, hvtsdrv_get_vdavinci_fop());
    tsdrv_set_dev_status(devid, TSDRV_PM_FID, TSDRV_DEV_ACTIVE, TSDRV_PHYSICAL_TYPE);

#ifdef CFG_FEATURE_SUPPORT_VSQ_MNG
    ret = tsdrv_notice_tsagent_dev_create(devid, TSDRV_PM_FID);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to notice tsagent to create VF. (devid=%u)\n", devid);
        goto notice_create_vf_fail;
    }
#endif

    ret = hvtsdrv_dev_recycle_wq_init(devid, TSDRV_PM_FID);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to init recycle workqueue. (devid=%u; ret=%d)\n", devid, ret);
        goto dev_recycle_wq_init_fail;
    }
    tsdrv_fid_num_inc(phy_devid);
    TSDRV_PRINT_INFO("Success to init container instance. (devid=%u)\n", devid);
    return 0;

dev_recycle_wq_init_fail:
#ifdef CFG_FEATURE_SUPPORT_VSQ_MNG
    tsdrv_notice_tsagent_dev_destroy(devid, TSDRV_PM_FID);
notice_create_vf_fail:
#endif
    dev_mnt_vdev_unregister_client(devid, TSDRV_PM_FID);
    hvtsdrv_drv_unregister(devid, tsnum);
    return ret;
}

int hvtsdrv_uninit_container_instance(struct vmngh_client_instance *instance)
{
    struct tsdrv_dev_resource *dev_res = NULL;
    enum tsdrv_dev_status status;
    u32 devid;

    if (hvtsdrv_instance_check(instance) != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Failed to check vmngh instance.\n");
        return -EINVAL;
#endif
    }
    devid = instance->dev_ctrl->dev_id;
    TSDRV_PRINT_DEBUG("Start to uninit container instance. (devid=%u)\n", devid);

    status = tsdrv_get_dev_status(devid, TSDRV_PM_FID);
    if (status != TSDRV_DEV_ACTIVE) {
        TSDRV_PRINT_ERR("The device status is not active. (devid=%u; fid=%u; status=%u)\n",
            devid, TSDRV_PM_FID, status);
        return -EINVAL;
    }

    dev_res = tsdrv_get_dev_resource(devid, TSDRV_PM_FID);
    if (dev_res == NULL) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Failed to get dev resource. (devid=%u; fid=%u)\n", devid, TSDRV_PM_FID);
        return -EINVAL;
#endif
    }
    atomic_set(&dev_res->status, TSDRV_DEV_REBOOTING);
    dev_mnt_vdev_unregister_client(devid, TSDRV_PM_FID);

#ifndef TSDRV_UT
    mutex_lock(&dev_res->dev_res_lock);
    if (dev_res->vm_res_wq != NULL) {
        (void)queue_work(dev_res->vm_res_wq, &dev_res->vm_res_recycle_work);
    }
    mutex_unlock(&dev_res->dev_res_lock);
#else
    hvtsdrv_vm_res_recycle_work(&dev_res->vm_res_recycle_work);
#endif

    TSDRV_PRINT_INFO("Success to uninit container instance. (devid=%u)\n", devid);
    return 0;
}
#endif

