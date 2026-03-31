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
 * Create: 2022-12-05
 */

#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/errno.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/bitmap.h>

#include "user_cfg_interface.h"
#include "devdrv_interface.h"
#include "virtmngdev_pci.h"
#include "virtmngdev_resource.h"
#include "virtmngdev_soc_resource.h"
#include "virtmngdev_res_common.h"
#include "virtmngdev_client_online.h"
#include "virtmngdev_common_resource.h"

STATIC vmngd_vf_element_num_t g_resource_alloc_list[] = {
    {
        .base = {VMNGD_CFG_ELE_BASE(1_4)}, /* 1/4 */
        .accelerator = {VMNGD_CFG_ELE_AC(1_4)},
        .dvpp = {VMNGD_CFG_ELE_DVPP(1_4)},
        .cpu = {VMNGD_CFG_ELE_CPU(1_4)},
    },
    {
        .base = {VMNGD_CFG_ELE_BASE(2_4)}, /* 1/2 */
        .accelerator = {VMNGD_CFG_ELE_AC(2_4)},
        .dvpp = {VMNGD_CFG_ELE_DVPP(2_4)},
        .cpu = {VMNGD_CFG_ELE_CPU(2_4)},
    }
};

void vmngd_resource_unit_to_ts_cfg(const vmngd_resource_unit_t *unit, vmng_vf_cfg_t *cfg)
{
    cfg->accelerator.c_core_bitmap = (u32)unit->accelerator.c_core.bitmap;
    cfg->accelerator.dsa_bitmap = (u32)unit->accelerator.dsa.bitmap;
    cfg->accelerator.ffts_bitmap = (u32)unit->accelerator.ffts.bitmap;
    cfg->accelerator.sdma_bitmap = (u32)unit->accelerator.sdma.bitmap;
    cfg->accelerator.pcie_dma_bitmap = (u32)unit->accelerator.pcie_dma.bitmap;
    cfg->accelerator.acsq_slice_bitmap = (u32)unit->accelerator.acsq.bitmap;
    cfg->accelerator.rtsq_slice_bitmap = (u32)unit->accelerator.rtsq.bitmap;
    cfg->accelerator.event_slice_bitmap = (u32)unit->accelerator.event_id.bitmap;
    cfg->accelerator.notify_slice_bitmap = (u32)unit->accelerator.notify_id.bitmap;
    cfg->accelerator.cdq_slice_bitmap = (u32)unit->accelerator.cdqm.bitmap;
    cfg->accelerator.cmo_slice_bitmap = (u32)unit->accelerator.cmo_id.bitmap;

    cfg->accelerator.aic_bitmap = (u32)unit->accelerator.aic.bitmap;
    cfg->accelerator.aiv_bitmap = (u64)unit->accelerator.aiv.bitmap;
    cfg->dvpp.jpegd_bitmap = (u32)unit->dvpp.jpegd.bitmap;
    cfg->dvpp.jpege_bitmap = (u32)unit->dvpp.jpege.bitmap;
    cfg->dvpp.vpc_bitmap = (u32)unit->dvpp.vpc.bitmap;
    cfg->dvpp.vdec_bitmap = (u32)unit->dvpp.vdec.bitmap;
    cfg->dvpp.pngd_bitmap = (u32)unit->dvpp.pngd.bitmap;
    cfg->dvpp.venc_bitmap = (u32)unit->dvpp.venc.bitmap;

    cfg->cpu.topic_aicpu_slot_bitmap = (u32)unit->cpu.topic_aicpu_slot.bitmap;
    cfg->cpu.topic_ctrl_cpu_slot_bitmap = (u32)unit->cpu.topic_ctrl_cpu_slot.bitmap;
    cfg->cpu.host_ctrl_cpu_bitmap = (u32)unit->cpu.host_ctrl_cpu.bitmap;
    cfg->cpu.device_aicpu_bitmap = (u32)unit->cpu.device_aicpu.bitmap;
    cfg->cpu.host_aicpu_bitmap = (u64)unit->cpu.host_aicpu.bitmap;

    cfg->id.vf_id = (u8)unit->cfg_info.vfid - VMNG_VDEV_FIRST_VFID;
    cfg->id.vfg_mode = (u8)unit->cfg_info.vfg_mode;
    cfg->id.vfg_id = (u8)unit->cfg_info.vfg_id;
    cfg->id.vip = 0; // mini v3 do not set vip
    cfg->id.token = unit->cfg_info.token;
    cfg->id.token_max = unit->cfg_info.token_max;
    cfg->id.task_timeout = unit->cfg_info.task_timeout;

    vmngd_print_ts_cfg(cfg);
}

STATIC void vmngd_resource_update_vdev_ctrl(u32 dev_id, u32 vfid, struct vmng_vdev_ctrl *ctrl)
{
    vmngd_resource_unit_t *each = NULL;
    vmngd_resource_unit_t *total = NULL;
    vmngd_resource_info_t *resource_info = vmngd_get_resource_info(dev_id);

    each = &resource_info->vf_cfg_each[vfid - VMNGD_VF_NUM_MIN];
    total = &resource_info->vf_cfg_total;
    ctrl->core_num = each->accelerator.aic.num;
    ctrl->total_core_num = total->accelerator.aic.num;
    ctrl->vfid = 0;
    vmngd_resource_unit_to_ts_cfg(each, &ctrl->vf_cfg);
    vmng_info("Update vdev ctrl ok. (ctrl->dev_id=%u; ctrl->vfid=%u;)\n", ctrl->dev_id, ctrl->vfid);
}

STATIC void vmngd_resource_enquire_base_proc(struct vmng_soc_res_base *base, const vmngd_resource_unit_t *unit)
{
}

STATIC void vmngd_resource_enquire_each_vf_proc(struct vmng_vf_res_info *info, vmngd_resource_unit_t *unit)
{
    vmngd_resource_info_t *resource_info = NULL;

    resource_info = vmngd_get_resource_info(unit->cfg_info.dev_id);
    info->dev_id = unit->cfg_info.dev_id;
    info->vfid = unit->cfg_info.vfid;
    info->vfg.vfg_id = unit->cfg_info.vfg_id;
    info->vfg.vfg_type = (u32)unit->cfg_info.vfg_type;
    info->vfg.vfg_refresh.vfg_mode = unit->cfg_info.vfg_mode;
    info->vfg.vfg_refresh.token = unit->cfg_info.token;
    info->vfg.vfg_refresh.token_max = unit->cfg_info.token_max;
    info->vfg.vfg_refresh.task_timeout = unit->cfg_info.task_timeout;
}

STATIC int vmngd_resource_enquire(u32 dev_id, u32 vfid, struct vmng_soc_resource_enquire *info)
{
    u32 idx;
    vmngd_resource_unit_t *each = NULL;
    vmngd_resource_unit_t *remain = NULL;
    vmngd_resource_unit_t *total = NULL;
    vmngd_resource_info_t *resource_info = NULL;

    if (info == NULL) {
        vmng_err("param NULL.\n");
        return VMNG_ERR;
    }

    if (vmngd_resource_info_check(dev_id, VMNGD_ID_NO_CHECK, VMNGD_ID_NO_CHECK) != VMNG_OK) {
        return VMNG_ERR;
    }

    resource_info = vmngd_get_resource_info(dev_id);
    if (resource_info == NULL) {
        vmng_err("resource info is NULL. (dev_id=%u)\n", dev_id);
        return VMNG_ERR;
    }

    remain = &resource_info->vf_cfg_remain;
    total = &resource_info->vf_cfg_total;
    vmngd_resource_enquire_base_proc(&info->remain.base, remain);
    vmngd_resource_enquire_stars_proc(&info->remain.stars_refresh, &info->remain.stars_static, remain);
    vmngd_resource_enquire_base_proc(&info->total.base, total);
    vmngd_resource_enquire_stars_proc(&info->total.stars_refresh, &info->total.stars_static, total);

    if ((vfid >= VMNGD_VF_NUM_MIN) && (vfid <= BASE_VF_NUM)) {
        idx = vfid - VMNGD_VF_NUM_MIN;
        each = &resource_info->vf_cfg_each[idx];
        if (each->cfg_info.status != VMNG_VDEV_STATUS_FREE) {
            vmngd_resource_enquire_each_vf_proc(&info->each, each);
            vmngd_resource_enquire_stars_proc(&info->each.stars_refresh, &info->each.stars_static, each);
        }
        if (strcpy_s(info->each.name, VMNG_VF_TEMP_NAME_LEN, each->template_name) != 0) {
            vmng_err("Call strcpy_s failed.\n");
            return VMNG_ERR;
        }
    }

    return VMNG_OK;
}

STATIC int vmngd_resource_enquire_vfg(u32 dev_id, u32 vfid, struct vmng_soc_res_info *vfg_info)
{
    vmngd_resource_info_t *resource_info = vmngd_get_resource_info(dev_id);
    vmngd_resource_unit_t *each = NULL;
    vmngd_vfg_cfg_info_t *vfg;
    u32 idx;

    if (vfg_info == NULL) {
        vmng_err("vfg_info is NULL. \n");
        return VMNG_ERR;
    }

    if (resource_info == NULL) {
        vmng_err("Global vmngd resource info is NULL.\n");
        return VMNG_ERR;
    }

    idx = vfid - VMNGD_VF_NUM_MIN;
    each = &resource_info->vf_cfg_each[idx];

    if (each->cfg_info.vfg_id >= BASE_VFG_NUM) {
        vmng_err("vf has not bind to vfg, can't get vfg info. (dev_id=%u,vfid=%u,vfg_id=%u)\n",
            dev_id, vfid, each->cfg_info.vfg_id);
        return VMNG_ERR;
    }
    vfg = &resource_info->vfg_cfg_each[each->cfg_info.vfg_id];
    vfg_info->stars_static.aic = vfg->accelerator.aic.num;
    vfg_info->stars_refresh.device_aicpu = (u32)vfg->cpu.device_aicpu.bitmap;

    return VMNG_OK;
}

STATIC bool vmngd_is_vf_check(u32 dev_id)
{
    if (vmngd_get_pfvf_type_by_devid(dev_id) == VMNGD_TYPE_VF) {
        return true;
    }

    if (vmngd_resource_info_check(dev_id, VMNGD_ID_NO_CHECK, VMNGD_ID_NO_CHECK) != VMNG_OK) {
        return false;
    }

    return false;
}

STATIC enum vmng_vdev_status vmngd_resource_get_vf_status(u32 dev_id, u32 vfid)
{
    vmngd_resource_info_t *resource_info = vmngd_get_resource_info(dev_id);

    return resource_info->vf_cfg_each[vfid - VMNGD_VF_NUM_MIN].cfg_info.status;
}

STATIC int vmngd_resource_release_vfg(u32 dev_id, u32 vfid)
{
    vmngd_resource_info_t *resource_info = vmngd_get_resource_info(dev_id);
    vmngd_resource_unit_t *remain = NULL;
    vmngd_resource_unit_t *each = NULL;
    vmngd_vfg_cfg_info_t *vfg = NULL;
    unsigned long idx;

    idx = vfid - VMNGD_VF_NUM_MIN; /* vfid 1~4 */
    each = &resource_info->vf_cfg_each[idx];
    remain = &resource_info->vf_cfg_remain;

    vfg = &resource_info->vfg_cfg_each[each->cfg_info.vfg_id];
    list_del(&each->vf_node);
    if (list_empty_careful(&vfg->vf_list_head)) {
        vfg->status = VMNG_VFG_FREE;
        vfg->vfg_type = VMNG_VF_GROUP_TYPE_NORMAL;
    }

    if (bitmap_weight(&vfg->vf_bitmap, BASE_VF_NUM) == 1) {
        vfg->vfg_type = VMNG_VF_GROUP_TYPE_VIP;
    }

    each->vfg = NULL;
    vmngd_element_clear(&each->base.vfg);
    each->cfg_info.vfg_mode = 0;
    each->cfg_info.vfg_id = 0;

    return VMNG_OK;
}

STATIC void vmngd_resource_release_stars_proc(vmngd_resource_unit_t *vf, vmngd_resource_unit_t *remain, int offset)
{
    vmngd_element_t *remain_ele = (vmngd_element_t *)((void *)remain + offset);
    vmngd_element_t *vf_ele = (vmngd_element_t *)((void *)vf + offset);
    vmngd_element_t *share_vf_ele;
    vmngd_vfg_cfg_info_t *vfg = vf->vfg;
    vmngd_resource_unit_t *node;
    struct list_head *cur = NULL;
    struct list_head *next = NULL;

    list_for_each_safe(cur, next, &vfg->vf_list_head)
    {
        node = (vmngd_resource_unit_t *)list_entry(cur, vmngd_resource_unit_t, vf_node);
        if (node != vf) {
            share_vf_ele = (vmngd_element_t *)((void *)node + offset);
            // if resource is using by other vf in vfg, vf sub these resource before return to pf
            vmngd_element_sub(vf_ele, share_vf_ele);
        }
    }
    // return resource to pf
    vmngd_element_add(remain_ele, vf_ele);
    vmngd_element_clear(vf_ele);
    return;
}

STATIC int vmngd_resource_release_stars_bitmap(u32 dev_id, u32 vfid)
{
    vmngd_resource_info_t *resource_info = vmngd_get_resource_info(dev_id);
    vmngd_resource_unit_t *remain = NULL;
    vmngd_resource_unit_t *vf = NULL;
    int offset;
    int idx;
    int count;
    int i;

    remain = &resource_info->vf_cfg_remain;
    idx = vfid - VMNGD_VF_NUM_MIN; /* vfid 1~4 */
    vf = &resource_info->vf_cfg_each[idx];

    offset = offsetof(vmngd_resource_unit_t, accelerator);
    count = sizeof(vmngd_vf_cfg_cpu_t) / sizeof(vmngd_element_t) +
            sizeof(vmngd_vf_cfg_dvpp_t) / sizeof(vmngd_element_t) +
            sizeof(vmngd_vf_cfg_ac_t) / sizeof(vmngd_element_t);

    for (i = 0; i < count; ++i) {
        vmngd_resource_release_stars_proc(vf, remain, offset);
        offset += sizeof(vmngd_element_t);
    }
    return VMNG_OK;
}

STATIC int vmngd_resource_release_stars_resource(u32 dev_id, u32 vfid)
{
    return vmngd_resource_release_stars_bitmap(dev_id, vfid);
}

STATIC int vmngd_resource_free(u32 dev_id, u32 vfid)
{
    vmngd_resource_info_t *resource_info = NULL;

    if (vmngd_resource_info_check(dev_id, vfid, VMNGD_ID_NO_CHECK) != 0) {
        return VMNG_ERR;
    }
    resource_info = vmngd_get_resource_info(dev_id);
    if (vmngd_resource_get_vf_status(dev_id, vfid) == VMNG_VDEV_STATUS_FREE) {
        return VMNG_ERR;
    }

    mutex_lock(&resource_info->lock);

    if (vmngd_resource_release_stars_resource(dev_id, vfid) != 0) {
        vmng_err("Release vf resource failed. (vfid=%u)\n", vfid);
    }

    if (vmngd_resource_release_vfg(dev_id, vfid) != 0) {
        vmng_err("Release vfg fail.(vfid=%u)\n", vfid);
    }

    mutex_unlock(&resource_info->lock);
    vmngd_resource_upate_vf_status(dev_id, vfid, VMNG_VDEV_STATUS_FREE);
    return VMNG_OK;
}


STATIC int vmngd_resource_alloc_stars_proc(vmngd_resource_unit_t *vf, vmngd_resource_unit_t *remain,
                                           int offset, u32 alloc_num)
{
    vmngd_element_t *remain_ele = (vmngd_element_t *)((void *)remain + offset);
    vmngd_element_t *vf_ele = (vmngd_element_t *)((void *)vf + offset);
    int ret;

    if (alloc_num > remain_ele->num) {
        vmng_err("Resource is not enough. (remain_num=%u; alloc_num=%u; offset=%u)\n",
            remain_ele->num, alloc_num, offset);
        return -EINVAL;
    }

    // alloc resource from pf remain resource
    ret = vmngd_element_alloc(vf_ele, remain_ele, alloc_num);
    if (ret != 0) {
        vmng_err("Alloc VF resource failed. (ret=%d; remain=%u; alloc_num=%u; offset=%u)\n",
            ret, remain_ele->num, alloc_num, offset);
        return ret;
    }

    // share resoure keep the remain same
    if (!(remain_ele->attr & ATTR_VFG_SHARE)) {
        vmngd_element_sub(remain_ele, vf_ele);
    }

    return 0;
}

STATIC int vmngd_resource_alloc_stars_bitmap(u32 dev_id, u32 vfid, vmngd_vf_element_num_t *stars)
{
    vmngd_resource_info_t *resource_info = vmngd_get_resource_info(dev_id);
    vmngd_resource_unit_t *remain = NULL;
    vmngd_resource_unit_t *vf = NULL;
    int offset;
    int idx;
    int ret;
    u32 *res_num;
    int count;
    int i;

    remain = &resource_info->vf_cfg_remain;
    idx = vfid - VMNGD_VF_NUM_MIN; /* vfid 1~4 */
    vf = &resource_info->vf_cfg_each[idx];

    offset = offsetof(vmngd_resource_unit_t, accelerator);
    res_num = (u32 *)&stars->accelerator;
    count = sizeof(vmngd_vf_cfg_cpu_t) / sizeof(vmngd_element_t) +
            sizeof(vmngd_vf_cfg_dvpp_t) / sizeof(vmngd_element_t) +
            sizeof(vmngd_vf_cfg_ac_t) / sizeof(vmngd_element_t);

    for (i = 0; i < count; ++i) {
        ret = vmngd_resource_alloc_stars_proc(vf, remain, offset, *res_num);
        if (ret != 0) {
            vmng_err("Alloc stars proc failed. (ret=%d)\n", ret);
            goto failed;
        }
        offset += sizeof(vmngd_element_t);
        res_num++;
    }
    return VMNG_OK;
failed:
    for (i = i - 1; i >= 0; --i) {
        offset -= sizeof(vmngd_element_t);
        vmngd_resource_release_stars_proc(vf, remain, offset);
    }
    return ret;
}

STATIC int vmngd_resource_alloc_stars_resource(u32 dev_id, u32 vfid, vmngd_vf_element_num_t *stars)
{
    if (stars == NULL) {
        vmng_err("Param NULL.\n");
        return VMNG_ERR;
    }

    return vmngd_resource_alloc_stars_bitmap(dev_id, vfid, stars);
}

STATIC void vmngd_add_vf_to_vfg(vmngd_resource_unit_t *vf, vmngd_vfg_cfg_info_t *vfg)
{
    if (vfg->status == VMNG_VFG_FREE) {
        vfg->status = VMGN_VFG_OCCUPIED;
    }

    bitmap_set(&vfg->vf_bitmap, vf->cfg_info.vfid - 1, 1);
    list_add_tail(&vf->vf_node, &vfg->vf_list_head);

    if (bitmap_weight(&vfg->vf_bitmap, BASE_VF_NUM) == 1) {
        vfg->vfg_type = VMNG_VF_GROUP_TYPE_VIP;
    } else if (bitmap_weight(&vfg->vf_bitmap, BASE_VF_NUM) > 1) {
        vfg->vfg_type = VMNG_VF_GROUP_TYPE_NORMAL;
    }

    vf->vfg = vfg;
    vf->cfg_info.vfg_type = vfg->vfg_type;
    vf->cfg_info.vfg_id = vfg->vfg_id;
}

STATIC int vmngd_bind_vfg(u32 dev_id, u32 vfid)
{
    vmngd_resource_info_t *resource_info = NULL;
    vmngd_resource_unit_t *vf = NULL;
    vmngd_vfg_cfg_info_t *vfg = NULL;
    unsigned int vfg_id;

    if (vfid < VMNGD_VF_NUM_MIN || vfid > BASE_VF_NUM) {
        return VMNG_ERR;
    }

    resource_info = vmngd_get_resource_info(dev_id);
    vf = &resource_info->vf_cfg_each[vfid - VMNGD_VF_NUM_MIN];
    if (vf->vfg != NULL) {
        return VMNG_OK;
    }

    vfg_id = vf->cfg_info.vfg_id;
    if ((vfg_id != (u32)-1) && (vfg_id >= BASE_VFG_NUM)) { // vfg_id valid when equal to [-1,0]
        vmng_err("invalid vfg id, failed to alloc vfg. (vfg_id=%u)\n", vfg_id);
        return VMNG_ERR;
    }

    resource_info = vmngd_get_resource_info(vf->cfg_info.dev_id);

    if (vfg_id == (u32)-1) { // vfg_id can be set -1 by user
        vfg = &resource_info->vfg_cfg_each[0];
    } else {
        vfg = &resource_info->vfg_cfg_each[vfg_id];
    }
    vmngd_add_vf_to_vfg(vf, vfg);

    return VMNG_OK;
}

STATIC int vmngd_resource_alloc_vfg(struct vmng_ctrl_msg_info *info)
{
    vmngd_resource_info_t *resource_info = vmngd_get_resource_info(info->dev_id);
    vmngd_resource_unit_t *each = NULL;
    u32 idx;
    int ret;

    idx = (u32)(info->vfid - VMNGD_VF_NUM_MIN); /* dev_vfid 1~4 */
    each = &resource_info->vf_cfg_each[idx];
    each->cfg_info.vfg_id = info->vf_cfg.vfg.vfg_id; // user input
    each->cfg_info.vfid = info->vfid;
    each->cfg_info.dev_id = info->dev_id;
    each->cfg_info.dtype = info->dtype;
    each->cfg_info.token = info->vf_cfg.vfg.vfg_refresh.token;
    each->cfg_info.token_max = info->vf_cfg.vfg.vfg_refresh.token_max;
    each->cfg_info.task_timeout = info->vf_cfg.vfg.vfg_refresh.task_timeout;

    if (strcpy_s(each->template_name, VMNG_VF_TEMP_NAME_LEN, info->vf_cfg.name) != 0) {
        vmng_err("Call strcpy_s failed.\n");
        return VMNG_ERR;
    }

    ret = vmngd_bind_vfg(info->dev_id, info->vfid);
    if (ret != VMNG_OK) {
        vmng_err("Bind vf to vfg failed. (ret=%d; dev_id=%u; vfid=%u).\n", ret, info->dev_id, info->vfid);
        return ret;
    }
    info->vf_cfg.vfg.vfg_id = each->cfg_info.vfg_id; // return vfg_id to user

    return VMNG_OK;
}

STATIC void vmngd_set_core_num(struct vmng_ctrl_msg_info *info)
{
    u32 idx;
    vmngd_resource_unit_t *each = NULL;
    vmngd_resource_unit_t *total = NULL;
    vmngd_resource_info_t *resource_info = NULL;

    resource_info = vmngd_get_resource_info(info->dev_id);

    idx = info->vfid - VMNGD_VF_NUM_MIN;
    each = &resource_info->vf_cfg_each[idx];
    total = &resource_info->vf_cfg_total;
    info->core_num = each->accelerator.aic.num;
    info->total_core_num = total->accelerator.aic.num;
}

STATIC int vmngd_resource_alloc(struct vmng_ctrl_msg_info *info)
{
    vmngd_resource_info_t *resource_info = NULL;
    u32 dev_id = info->dev_id;
    u32 vfid = info->vfid;
    u32 alloc_idx;

    if (info->dtype == VMNG_HW_TYPE_C1_4) {
        alloc_idx = 0;
    } else {
        alloc_idx = 1;
    }

    resource_info = vmngd_get_resource_info(dev_id);
    mutex_lock(&resource_info->lock);

    if (vmngd_resource_alloc_vfg(info) != 0) {
        vmng_err("Alloc_vfg fail.(dev_id=%u; aic=%u; vfid=%u)\n", dev_id, info->core_num, vfid);
        mutex_unlock(&resource_info->lock);
        return VMNG_ERR;
    }

    if (vmngd_resource_alloc_stars_resource(dev_id, vfid, &g_resource_alloc_list[alloc_idx]) != VMNG_OK) {
        vmng_err("Alloc stars resource fail.(dev_id=%u; aic=%u; vfid=%u)\n", dev_id, info->core_num, vfid);
        goto EXIT;
    }

    mutex_unlock(&resource_info->lock);
    vmngd_set_core_num(info);
    vmngd_resource_upate_vf_status(dev_id, vfid, VMNG_VDEV_STATUS_ALLOC);
    return VMNG_OK;
EXIT:
    if (vmngd_resource_release_vfg(dev_id, vfid) != 0) {
        vmng_err("Release vfg err.\n");
    }
    mutex_unlock(&resource_info->lock);
    return VMNG_ERR;
}

STATIC void vmngd_set_rc_dynamic_aicpu_info(u32 dev_id, vmngd_resource_info_t *pinfo)
{
    u32 dtype;
    vmngd_resource_get_aicpu_info(dev_id, &pinfo->vf_cfg_total.cpu.device_aicpu);
    pinfo->vf_cfg_remain.cpu.device_aicpu.num = pinfo->vf_cfg_total.cpu.device_aicpu.num;
    pinfo->vf_cfg_remain.cpu.device_aicpu.bitmap = pinfo->vf_cfg_total.cpu.device_aicpu.bitmap;

    for (dtype = VMNG_DTYPE_RANGE_FROM ; dtype <= VMNG_DTYPE_RANGE_TO; dtype++) {
        g_resource_alloc_list[dtype - VMNG_DTYPE_RANGE_FROM].cpu.device_aicpu =
            pinfo->vf_cfg_remain.cpu.device_aicpu.num;
    }

    return;
}

STATIC void vmngd_resource_uninit(u32 dev_id)
{
    if (dev_id >= VMNGD_SURPORT_MAX_DEV) {
        vmng_info("dev_id check invalid.(dev_id=%u)\n", dev_id);
        return;
    }

    vmngd_free_resource_info(dev_id);

    vmng_info("Vmngd_resource_uninit OK.(dev_id=%u)\n", dev_id);

    return;
}

STATIC int vmngd_resource_init(u32 dev_id)
{
    vmngd_resource_info_t *pinfo = NULL;
    int i;

    if (dev_id >= VMNGD_SURPORT_MAX_DEV) {
        vmng_info("dev_id check invalid.(dev_id=%u)\n", dev_id);
        return VMNG_ERR;
    }

    if (vmngd_alloc_resource_info(dev_id) != VMNG_OK) {
        return VMNG_ERR;
    }

    pinfo = vmngd_get_resource_info(dev_id);
    vmngd_set_rc_dynamic_aicpu_info(dev_id, pinfo);

    for (i = 0; i < BASE_VFG_NUM; ++i) {
        INIT_LIST_HEAD(&pinfo->vfg_cfg_each[i].vf_list_head);
        pinfo->vfg_cfg_each[i].status = VMNG_VFG_FREE;
        pinfo->vfg_cfg_each[i].vfg_id = i;
        pinfo->vfg_cfg_each[i].vfg_type = VMNG_VF_GROUP_TYPE_NORMAL;
    }

    for (i = 0; i < BASE_VF_NUM; ++i) {
        INIT_LIST_HEAD(&pinfo->vf_cfg_each[i].vf_node);
    }

    pinfo->status = VMNG_VALID;
    vmng_info("Vmngd_resource_init OK.(dev_id=%u)\n", dev_id);
    return VMNG_OK;
}

int vmngd_res_ops_init(struct vmngd_resource_ops *ops)
{
    if (ops == NULL) {
        vmng_err("Param NULL.\n");
        return VMNG_ERR;
    }

    ops->res_init = vmngd_resource_init;
    ops->res_uninit = vmngd_resource_uninit;
    ops->alloc_vf = vmngd_resource_alloc;
    ops->free_vf = vmngd_resource_free;
    ops->res_enquire = vmngd_resource_enquire;
    ops->res_enquire_vfg = vmngd_resource_enquire_vfg;
    ops->update_vdev_ctrl = vmngd_resource_update_vdev_ctrl;
    ops->is_vf = vmngd_is_vf_check;
    ops->unify_devid_to_pfvf_id = vmngd_get_pfvf_id_by_devid;

    vmng_info("Res ops init mini_v3.\n");
    return VMNG_OK;
}

int vmngd_res_ops_uninit(struct vmngd_resource_ops *ops)
{
    if (ops == NULL) {
        vmng_err("Param NULL.\n");
        return VMNG_ERR;
    }

    (void)memset_s(ops, sizeof(struct vmngd_resource_ops), 0, sizeof(struct vmngd_resource_ops));
    vmng_info("Res ops uninit mini_v3.\n");
    return VMNG_OK;
}