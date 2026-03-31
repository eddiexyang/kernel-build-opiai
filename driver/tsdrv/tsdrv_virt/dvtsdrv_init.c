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
 * Create: 2022-05-21
 */
#include <linux/module.h>
#ifdef CFG_FEATURE_VFIO
#include <linux/delay.h>
#include "dvtsdrv_init.h"
#include "virtmng_interface.h"
#include "tsdrv_kernel_common.h"
#include "tsdrv_device.h"
#include "tsdrv_id.h"
#include "devdrv_devinit.h"
#include "devdrv_cqsq.h"
#include "devdrv_id.h"
#include "dvtsdrv_sriov_hwinfo.h"
#include "dvtsdrv_id.h"

static int dvtsdrv_instance_check(struct vmngd_client_instance *instance)
{
    if (instance == NULL) {
        TSDRV_PRINT_ERR("The instance is NULL.\n");
        return -EINVAL;
    }

    if (instance->type != VMNGD_CLIENT_TYPE_TSDRV) {
        TSDRV_PRINT_ERR("The instance type is not correct. (type=%d)\n", (int)instance->type);
        return -EINVAL;
    }

    if (instance->vdev_ctrl.dtype >= VMNG_HW_TYPE_MAX) {
        TSDRV_PRINT_ERR("The dtype is invalid. (dtype=%d)\n", instance->vdev_ctrl.dtype);
        return -EINVAL;
    }

    if ((instance->vdev_ctrl.dev_id >= TSDRV_MAX_DAVINCI_NUM) || (instance->vdev_ctrl.vfid >= TSDRV_MAX_FID_NUM)) {
        TSDRV_PRINT_ERR("The devid or vfid is invalid. (devid=%u; vfid=%u)\n", instance->vdev_ctrl.dev_id,
            instance->vdev_ctrl.vfid);
        return -EINVAL;
    }

    TSDRV_PRINT_INFO("Start to init VF instance. (devid=%u; dtype=%u; id_bitmap=0x%x; aic_bitmap=0x%x)\n",
        instance->vdev_ctrl.dev_id, instance->vdev_ctrl.dtype,
        instance->vdev_ctrl.vf_cfg.accelerator.rtsq_slice_bitmap,
        instance->vdev_ctrl.vf_cfg.accelerator.aic_bitmap);
    return 0;
}

static int dvtsdrv_drv_register(u32 devid, u32 tsnum, struct vmngd_client_instance *instance)
{
    struct tsdrv_device *tsdrv_dev = tsdrv_get_dev(devid);
    u32 tsid;
    int ret;

    tsdrv_dev->dev_res[TSDRV_PM_FID].aic_bitmap = instance->vdev_ctrl.vf_cfg.accelerator.aic_bitmap;
    ret = tsdrv_dev_register(devid, TSDRV_PM_FID, (enum TSDRV_CAPACITY)instance->vdev_ctrl.dtype);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Failed to register tsdrv device. (devid=%u; ret=%d)\n", devid, ret);
        return ret;
#endif
    }

    ret = dvtsdrv_basic_module_init(devid, TSDRV_PM_FID, tsnum, &instance->vdev_ctrl.vf_cfg.accelerator);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Failed to init basic module. (devid=%u; ret=%d)\n", devid, ret);
        goto err_basic_module_init;
#endif
    }

    for (tsid = 0; tsid < tsnum; tsid++) {
        tsdrv_set_ids_cur_num(devid, tsid);
        ret = tsdrv_alloc_ts_sqcq(devid, TSDRV_PM_FID, tsid);
        if (ret != 0) {
#ifndef TSDRV_UT
            TSDRV_PRINT_ERR("Failed to alloc ts sqcq. (devid=%u; ret=%d)\n", devid, ret);
            goto err_ts_sqcq_init;
#endif
        }
    }

    return 0;

#ifndef TSDRV_UT
err_ts_sqcq_init:
    dvtsdrv_basic_module_uninit(devid, TSDRV_PM_FID, tsnum);
err_basic_module_init:
    tsdrv_dev_unregister(devid, TSDRV_PM_FID);
    return ret;
#endif
}

static int dvtsdrv_init_instance(struct vmngd_client_instance *instance)
{
    u32 devid, tsnum, phy_devid, vfid;
    int ret;

    ret = dvtsdrv_instance_check(instance);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to check instance. (ret=%d)\n", ret);
        return ret;
    }

    devid = instance->vdev_ctrl.dev_id;
    ret = devdrv_get_pfvf_id_by_devid(devid, &phy_devid, &vfid);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to get pf and vf id. (devid=%u)\n", devid);
        return ret;
    }
    tsdrv_ids_release(phy_devid, TSDRV_PM_FID, tsdrv_get_dev_tsnum(phy_devid));

    ret = tsdrv_device_init(devid);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to init tsdrv device. (devid=%u; ret=%d)\n", devid, ret);
        return ret;
    }

    ret = tsdrv_set_dev_tsnum(devid, DEVDRV_MAX_TS_NUM);
    if (ret != 0) {
        return -ENODEV;
    }
    tsnum = tsdrv_get_dev_tsnum(devid);

    ret = tsdrv_set_vdev_hwinfo(devid, tsnum);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Failed to set vdevice hardware info. (devid=%u; ret=%d)\n", devid, ret);
        goto err_device_init;
#endif
    }

    ret = dvtsdrv_drv_register(devid, tsnum, instance);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Failed to register. (devid=%u; ret=%d)\n", devid, ret);
        goto err_device_init;
#endif
    }

    tsdrv_dev_ref_set(devid, TSDRV_PM_FID, 0);
    tsdrv_set_dev_status(devid, TSDRV_PM_FID, TSDRV_DEV_ACTIVE, TSDRV_PHYSICAL_TYPE);
    tsdrv_fid_num_inc(phy_devid);

    TSDRV_PRINT_INFO("Device init vf instance success. (devid=%u; fid=%u; tsnum=%u; flag=%d)\n",
        devid, TSDRV_PM_FID, tsnum, TSDRV_CONTAINER_TYPE);

    return 0;

#ifndef TSDRV_UT
err_device_init:
    tsdrv_device_exit(devid);
    return ret;
#endif
}

static int dvtsdrv_uninit_instance(struct vmngd_client_instance *instance)
{
    struct tsdrv_dev_resource *dev_res = NULL;
    enum tsdrv_dev_status status;
    u32 devid, fid, tsid, tsnum, phy_devid, vfid;
    int ret;

    ret = dvtsdrv_instance_check(instance);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to check instance. (ret=%d)\n", ret);
        return ret;
    }
    devid = instance->vdev_ctrl.dev_id;
    fid = TSDRV_PM_FID;
    TSDRV_PRINT_DEBUG("Start to unregister. (devid=%u; fid=%u)\n", devid, fid);

    status = tsdrv_get_dev_status(devid, fid);
    if (status != TSDRV_DEV_ACTIVE && status != TSDRV_DEV_REBOOTNOTICE) {
        TSDRV_PRINT_ERR("The status is not active. (devid=%u; fid=%u; status=%d)\n", devid, fid, (int)status);
        return -EINVAL;
    }

    dev_res = tsdrv_get_dev_resource(devid, fid);
    if (dev_res == NULL) {
        return -ENODEV;
    }
    atomic_set(&dev_res->status, TSDRV_DEV_REBOOTING);

    tsdrv_dev_ref_check(devid, fid);

    tsnum = tsdrv_get_dev_tsnum(devid);
    /* cq irq must exit first because some variables will be used in irq , which may cause access to freed memory */
    tsdrv_cq_irq_exit(devid, tsnum);
    tsdrv_mbox_exit(devid, tsnum);

    /* the wq of ts_res may be in waitting ts reports (ts may be in down state), it must be exit before ids destroy
     * because the wq will visit ids addr, eg model id
     */
    mutex_lock(&dev_res->dev_res_lock);
    tsdrv_ts_res_wq_exit(dev_res->ts_resource, tsnum);
    mutex_unlock(&dev_res->dev_res_lock);

    ret = devdrv_get_pfvf_id_by_devid(devid, &phy_devid, &vfid);
    if (ret != 0) {
        TSDRV_PRINT_ERR("Failed to get pf and vf id. (devid=%u)\n", devid);
        return -EINVAL;
    }
    tsdrv_fid_num_dec(phy_devid);
    for (tsid = 0; tsid < tsnum; tsid++) {
        dvtsdrv_free_reserved_ids(devid, fid, tsid);
        dvtsdrv_ids_destroy(devid, fid, tsid);
        if (tsdrv_get_fid_num(phy_devid) == 1) {
            tsdrv_reserved_sqcq_restore(phy_devid, vfid, tsid);
            dvtsdrv_res_ids_restore(phy_devid, TSDRV_PM_FID, tsid);
        }
    }
    tsdrv_dev_unregister(devid, fid);
    tsdrv_device_exit(devid);

    TSDRV_PRINT_INFO("Device vf unregister success. (devid=%u)\n", devid);

    return 0;
}

static int dvtsdrv_reset_instance(struct vmngd_client_instance *instance)
{
    return 0;
}

struct vmngd_client dvtsdrv_client = {
    .type = VMNGD_CLIENT_TYPE_TSDRV,
    .init_instance = dvtsdrv_init_instance,
    .uninit_instance = dvtsdrv_uninit_instance,
    .reset_instance = dvtsdrv_reset_instance,
};

void dvtsdrv_init(void)
{
    int ret;

    ret = vmngd_register_client(&dvtsdrv_client);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Failed to register vmngd client. (ret=%d)\n", ret);
        return;
#endif
    }

    TSDRV_PRINT_INFO("Register vmngd client success.\n");
    return;
}
EXPORT_SYMBOL(dvtsdrv_init);

void dvtsdrv_uninit(void)
{
    int ret;

    ret = vmngd_unregister_client(&dvtsdrv_client);
    if (ret != 0) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Failed to unregister vmngd client. (ret=%d)\n", ret);
        return;
#endif
    }

    TSDRV_PRINT_INFO("Unregister vmngd client success\n");
    return;
}
EXPORT_SYMBOL(dvtsdrv_uninit);

#else

void dvtsdrv_init(void)
{
    return;
}
EXPORT_SYMBOL(dvtsdrv_init);
void dvtsdrv_uninit(void)
{
    return;
}
EXPORT_SYMBOL(dvtsdrv_uninit);
#endif

