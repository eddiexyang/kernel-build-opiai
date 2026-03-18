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
 * Create: 2022-9-28
 */
#ifndef PROF_UNIT_TEST
#include "virtmng_interface.h"
#include "dvprof_init.h"
#include "prof_drv_dev.h"
#include "prof_def.h"
#include "prof.h"
static int dvprof_instance_check(struct vmngd_client_instance *instance)
{
    if (instance == NULL) {
        prof_err("The instance is NULL.\n");
        return -EINVAL;
    }

    if (instance->type != VMNGD_CLIENT_TYPE_PROFILING) {
        prof_err("The instance type is not correct. (type=%d)\n", (int)instance->type);
        return -EINVAL;
    }

    if ((instance->vdev_ctrl.dev_id >= PROF_DEVICE_NUM_VALUE) || (instance->vdev_ctrl.vfid >= PROF_VFID_NUM_MAX)) {
        prof_err("The devid or vfid is invalid. (devid=%u; vfid=%u)\n", instance->vdev_ctrl.dev_id,
            instance->vdev_ctrl.vfid);
        return -EINVAL;
    }

    prof_info("Start to init VF instance. (devid=%u; vfid=%u; dtype=%u; core_num=0x%x; total_core_num=0x%x)\n",
        instance->vdev_ctrl.dev_id, instance->vdev_ctrl.vfid, instance->vdev_ctrl.dtype,
        instance->vdev_ctrl.core_num, instance->vdev_ctrl.total_core_num);
    return PROF_OK;
}

static int dvprof_init_instance(struct vmngd_client_instance *instance)
{
    int ret;

    ret = dvprof_instance_check(instance);
    if (ret != PROF_OK) {
        prof_err("Failed to check instance. (ret=%d)\n", ret);
        return ret;
    }

    ret = prof_alloc_device_info(instance->vdev_ctrl.dev_id);
    if (ret != PROF_OK) {
        prof_err("Failed to alloc dev info. (devid=%u)\n", instance->vdev_ctrl.dev_id);
        return ret;
    }

    ret = prof_each_device_init(instance->vdev_ctrl.dev_id, instance->vdev_ctrl.core_num,
        instance->vdev_ctrl.total_core_num);
    if (ret != PROF_OK) {
        prof_free_device_info(instance->vdev_ctrl.dev_id);
        prof_err("Failed to prof_each_device_init. (devid=%u)\n", instance->vdev_ctrl.dev_id);
        return ret;
    }

    prof_event("Device init vf instance success. (devid=%u)\n", instance->vdev_ctrl.dev_id);
    return PROF_OK;
}

static int dvprof_uninit_instance(struct vmngd_client_instance *instance)
{
    int ret;

    ret = dvprof_instance_check(instance);
    if (ret != 0) {
        prof_err("Failed to check instance. (ret=%d)\n", ret);
        return ret;
    }
    prof_each_device_uninit(instance->vdev_ctrl.dev_id);
    prof_free_device_info(instance->vdev_ctrl.dev_id);

    prof_event("Device vf unregister success. (devid=%u)\n", instance->vdev_ctrl.dev_id);
    return PROF_OK;
}

static int dvprof_reset_instance(struct vmngd_client_instance *instance)
{
    return 0;
}

struct vmngd_client dvprof_client = {
    .type = VMNGD_CLIENT_TYPE_PROFILING,
    .init_instance = dvprof_init_instance,
    .uninit_instance = dvprof_uninit_instance,
    .reset_instance = dvprof_reset_instance,
};

int dvprof_init(void)
{
    int ret;

    ret = vmngd_register_client(&dvprof_client);
    if (ret != 0) {
        prof_err("Failed to register vmngd client. (ret=%d)\n", ret);
        return ret;
    }

    prof_info("Register vmngd client success.\n");
    return ret;
}

void dvprof_uninit(void)
{
    int ret;

    ret = vmngd_unregister_client(&dvprof_client);
    if (ret != 0) {
        prof_err("Failed to unregister vmngd client. (ret=%d)\n", ret);
        return;
    }

    prof_info("Unregister vmngd client success\n");
    return;
}
#else
void dvprof_init(void)
{
}
module_init(dvprof_init);
#endif
