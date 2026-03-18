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

#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/errno.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/bitmap.h>

#include "virtmngdev_pci.h"
#include "virtmngdev_resource.h"

STATIC int vmngd_resource_enquire_vfg(u32 dev_id, u32 vfid, struct vmng_soc_res_info *vfg_info)
{
    if (vfg_info == NULL) {
        vmng_err("vfg_info is NULL. \n");
        return VMNG_ERR;
    }

    vfg_info->stars_refresh.device_aicpu = 0xffff; // return all 16(0xffff) cpu as default value
    return VMNG_OK;
}

int vmngd_res_ops_init(struct vmngd_resource_ops *ops)
{
    if (ops == NULL) {
        vmng_err("Param NULL.\n");
        return VMNG_ERR;
    }

    ops->res_enquire_vfg = vmngd_resource_enquire_vfg;

    vmng_info("Res ops init default.\n");
    return VMNG_OK;
}

int vmngd_res_ops_uninit(struct vmngd_resource_ops *ops)
{
    if (ops == NULL) {
        vmng_err("Param NULL.\n");
        return VMNG_ERR;
    }

    (void)memset_s(ops, sizeof(struct vmngd_resource_ops), 0, sizeof(struct vmngd_resource_ops));

    vmng_info("Res ops uninit default.\n");
    return VMNG_OK;
}
