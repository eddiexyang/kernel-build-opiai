/*
 * Copyright (C) 2017-2023. Huawei Technologies Co., Ltd. All rights reserved.
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

#ifndef HVTSDRV_INIT_H
#define HVTSDRV_INIT_H
#include <linux/types.h>
#include <linux/module.h>
#include "virtmng_interface.h"
#include "tsdrv_kernel_common.h"
#include "tsdrv_device.h"

bool tsdrv_pm_ctx_is_finished(u32 devid);
int hvtsdrv_instance_check(struct vmngh_client_instance *instance);
void hvtsdrv_vm_res_recycle_work(struct work_struct *work);
void hvtsdrv_sq_mem_uninit(u32 devid, u32 fid, u32 tsid, enum tsdrv_host_flag host_flag);
int hvtsdrv_resource_init(u32 dev_id, u32 fid, enum TSDRV_CAPACITY tsdrv_capacity,
    enum tsdrv_host_flag host_flag);
void hvtsdrv_resource_uninit(u32 dev_id, u32 fid, enum tsdrv_host_flag host_flag);
int hvtsdrv_phy_cbcqsq_init(u32 devid, u32 tsid);
int hvtsdrv_phy_cbcqsq_exit(u32 devid, u32 tsid);
int hvtsdrv_dev_recycle_wq_init(u32 dev_id, u32 fid);
void hvtsdrv_dev_recycle_wq_uninit(u32 dev_id, u32 fid);
int hvtsdrv_dev_logic_cq_init(u32 dev_id, u32 fid);
void hvtsdrv_dev_logic_cq_uninit(u32 dev_id, u32 fid);
int hvtsdrv_init_container_instance(struct vmngh_client_instance *instance);
int hvtsdrv_uninit_container_instance(struct vmngh_client_instance *instance);
int hvtsdrv_init_mia_dev_instance(u32 dev_id, u32 fid, u32 dtype);
int hvtsdrv_uninit_mia_dev_instance(u32 dev_id, u32 fid);

void hvtsdrv_init(void);
void hvtsdrv_uninit(void);
void hvtsdrv_uninit_device(u32 dev_id);
int hvtsdrv_shm_init(u32 devid, u32 fid, enum tsdrv_host_flag host_flag);

#endif

