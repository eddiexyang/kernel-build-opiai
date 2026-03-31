/*
* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
* Create: 2023-02-22
*/

#include "dbl/uda.h"
#include "virtmngdev_mia_cfg.h"

int vmngd_add_mia_uda(u32 dev_id, u32 vfid, enum uda_dev_prop prop)
{
    struct vmng_vf_object *vf_obj = NULL;
    struct uda_dev_type uda_type = {0};
    struct uda_dev_para para = {0};
    struct uda_mia_dev_para mia_para = {0};
    u32 udevid;
    int ret;

    vf_obj = vmngd_get_vf_object(dev_id, vfid);
    if (vf_obj == NULL) {
        vmng_err("Get vf object failed. (dev_id=%u;vfid=%u)\n", dev_id, vfid);
        return -EINVAL;
    }

    if (vf_obj->pf->vf_online_num == 0) {
        ret = uda_dev_ctrl(dev_id, UDA_CTRL_TO_MIA);
        if (ret != 0) {
            vmng_err("To mia mode failed.(dev_id=%u;vfid=%u;ret=%d)\n", dev_id, vfid, ret);
            return ret;
        }
    }

    uda_dev_type_pack(&uda_type, UDA_DAVINCI, UDA_ENTITY, UDA_LOCAL, prop);
    uda_dev_para_pack(&para, UDA_INVALID_UDEVID, UDA_INVALID_UDEVID, uda_get_chip_type(dev_id), NULL);
    uda_mia_dev_para_pack(&mia_para, vf_obj->dev_id, vf_obj->vfid - 1);

    ret = uda_add_mia_dev(&uda_type, &para, &mia_para, &udevid);
    if (ret != 0) {
        vmng_err("Add uda mia device failed.(dev_id=%u;vfid=%u;ret=%d)\n",
            vf_obj->dev_id, vf_obj->vfid, ret);
        (void)uda_dev_ctrl(dev_id, UDA_CTRL_TO_SIA);
        return ret;
    }
    vf_obj->udevid = udevid;
    vf_obj->pf->vf_online_num++;

    vmng_info("Add uda mia device success. (dev_id=%u;vfid=%u;udev_id=%d)\n",
        vf_obj->dev_id, vf_obj->vfid, vf_obj->udevid);
    return 0;
}

int vmngd_remove_mia_uda(u32 dev_id, u32 vfid, enum uda_dev_prop prop)
{
    struct vmng_vf_object *vf_obj = NULL;
    struct uda_dev_type uda_type = {0};
    int ret;

    vf_obj = vmngd_get_vf_object(dev_id, vfid);
    if (vf_obj == NULL) {
        vmng_err("Get vf object failed. (dev_id=%u;vfid=%u)\n", dev_id, vfid);
        return -EINVAL;
    }

    uda_dev_type_pack(&uda_type, UDA_DAVINCI, UDA_ENTITY, UDA_LOCAL, prop);
    ret = uda_remove_dev(&uda_type, vf_obj->udevid);
    if (ret != 0) {
        vmng_err("Remove uda mia device failed.(dev_id=%u;vfid=%u;ret=%d)\n",
            vf_obj->dev_id, vf_obj->vfid, ret);
        return ret;
    }
    vf_obj->pf->vf_online_num--;
    if (vf_obj->pf->vf_online_num == 0) {
        (void)uda_dev_ctrl(dev_id, UDA_CTRL_TO_SIA);
    }
    vmng_info("Remove uda mia device success. (dev_id=%u;vfid=%u;udev_id=%d)\n",
        vf_obj->dev_id, vf_obj->vfid, vf_obj->udevid);
    return 0;
}
static int vmngd_set_mia_res(u32 devid, u32 res_type, u64 bitmap, u32 unit_per_bit)
{
    struct res_inst_info inst = {0};
    enum soc_sub_type sub_type;

    sub_type = soc_resmng_subsys_type(res_type);
    if (sub_type == MAX_SOC_SUBSYS_TYPE) {
        return soc_resmng_dev_set_mia_res(devid, res_type, bitmap, unit_per_bit);
    }

    inst.devid = devid;
    inst.sub_type = sub_type;
    inst.subid = 0;
    return soc_resmng_set_mia_res(&inst, res_type, bitmap, unit_per_bit);
}

int vmngd_config_soc_res(u32 dev_id, u32 vfid)
{
    struct vmng_vf_object *vf_obj = NULL;
    struct vmng_resource_base *vf_res = NULL;
    struct vmng_resource_base *vfg_res = NULL;
    unsigned long bitmap;
    u32 unit_per_bit;
    int type;
    int ret;

    vf_obj = vmngd_get_vf_object(dev_id, vfid);
    if (vf_obj == NULL) {
        vmng_err("Get vf object failed. (dev_id=%u;vfid=%u)\n", dev_id, vfid);
        return -EINVAL;
    }

    for (type = 0; type < MIA_MAX_RES_TYPE; ++type) {
        vf_res = vmngd_get_res_unit(&vf_obj->resource, type);
        if (vf_res == NULL) {
            continue;
        }
        if ((vf_obj->vfg != NULL) && (vf_res->element.attr & ATTR_VFG_SHARE)) {
            vfg_res = vmngd_get_res_unit(&vf_obj->vfg->resource, type);
            if (vfg_res == NULL) {
                vmng_err("Can not find vfg res. (udevid=%u;type=%d)\n", vf_obj->udevid, type);
                return -EINVAL;
            }
            bitmap = vfg_res->element.bitmap;
            unit_per_bit = vfg_res->element.num_per_bit;
        } else {
            bitmap = vf_res->element.bitmap;
            unit_per_bit = vf_res->element.num_per_bit;
        }

        ret = vmngd_set_mia_res(vf_obj->udevid, type, bitmap, unit_per_bit);
        if (ret != 0) {
            vmng_err("Set mia res failed. (udevid=%u;type=%d;ret=%d)\n",
                vf_obj->udevid, type, ret);
            return -EINVAL;
        }
        vmng_info("Set mia res. (udevid=%u;type=%d;bitmap=%#lx;num_per_bit=%u)\n",
            vf_obj->udevid, type, bitmap, unit_per_bit);
    }

    ret = soc_resmng_dev_set_mia_info(vf_obj->udevid, vf_obj->vfg->vfg_id);
    if (ret != 0) {
        vmng_err("Set mia res failed. (udevid=%u;type=%d;ret=%d)\n",
            vf_obj->udevid, type, ret);
        return -EINVAL;
    }
    return 0;
}
