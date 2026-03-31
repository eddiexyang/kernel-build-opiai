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
#include "virtmngdev_memory.h"
#include "virtmngdev_common_resource.h"
#include "virtmngdev_resource.h"

static inline int vmngd_check_dvpp_resource(const vmngd_element_t *remain_dvpp_engine_list,
                                            const u32 *alloc_dvpp_engine_list)
{
    int i;
    for (i = 0; i < VMNGD_DVPP_ENGINE_NUM; ++i) {
        if (alloc_dvpp_engine_list[i] > remain_dvpp_engine_list[i].num) {
            return VMNG_ERR;
        }
    }
    return VMNG_OK;
}

static inline void vmngd_alloc_dvpp_engine(vmngd_element_t *vf_dvpp_engine_list,
                                           vmngd_element_t *remain_dvpp_engine_list,
                                           const u32 *alloc_dvpp_engine_list)
{
    int i;
    for (i = 0; i < VMNGD_DVPP_ENGINE_NUM; ++i) {
        vf_dvpp_engine_list[i].num = alloc_dvpp_engine_list[i];
        remain_dvpp_engine_list[i].num -= alloc_dvpp_engine_list[i];
    }
}
void vmngd_release_dvpp_engine(u32 dev_id, struct vmngd_vf_resource_unit *vf)
{
    vmngd_resource_info_t *resource_info = vmngd_get_resource_info(dev_id);
    vmngd_resource_unit_t *remain = &resource_info->vf_cfg_remain;
    vmngd_element_t *remain_dvpp_engine_list = &remain->dvpp.jpegd;
    vmngd_element_t *vf_dvpp_engine_list = &vf->dvpp.jpegd;
    int i;

    for (i = 0; i < VMNGD_DVPP_ENGINE_NUM; ++i) {
        remain_dvpp_engine_list[i].num += vf_dvpp_engine_list[i].num;
        vf_dvpp_engine_list[i].num = 0;
    }
}

int vmngd_alloc_dvpp_for_vf(u32 dev_id, vmngd_resource_unit_t *vf, struct vmng_stars_res_refresh *res)
{
    vmngd_resource_info_t *resource_info = vmngd_get_resource_info(dev_id);
    vmngd_resource_unit_t *remain = &resource_info->vf_cfg_remain;
    vmngd_resource_unit_t *total = &resource_info->vf_cfg_total;
    vmngd_element_t *total_dvpp_engine_list = &total->dvpp.jpegd;
    vmngd_element_t *remain_dvpp_engine_list = &remain->dvpp.jpegd;
    vmngd_element_t *vf_dvpp_engine_list = &vf->dvpp.jpegd;
    u32 *alloc_dvpp_engine_list = &res->jpegd;
    int i;

    for (i = 0; i < VMNGD_DVPP_ENGINE_NUM; ++i) { // adapt for VM computing group
        if (alloc_dvpp_engine_list[i] == (u32)-1) {
            alloc_dvpp_engine_list[i] = total_dvpp_engine_list[i].num *
                                        vf->accelerator.aic.num / total->accelerator.aic.num;
            if (i == VMNGD_DVPP_VENC_INDEX) {
                alloc_dvpp_engine_list[i] = vf->cpu.device_aicpu.num / 2; /* venc is 1/2 of aicpu */
            }
        }
    }
    alloc_dvpp_engine_list[VMNGD_DVPP_PNGD_INDEX] = 0U;

    if (vmngd_check_dvpp_resource(remain_dvpp_engine_list, alloc_dvpp_engine_list) != VMNG_OK) {
        vmng_err("Remain dvpp resource is not enough, (jpegd=%u;jpege=%u;vpc=%u;vdec=%u;pngd=%u;venc=%u)\n",
            remain->dvpp.jpegd.num, remain->dvpp.jpege.num, remain->dvpp.vpc.num,
            remain->dvpp.vdec.num, remain->dvpp.pngd.num, remain->dvpp.venc.num);
        vmng_err("Alloc too many dvpp engine, (jpegd=%u;jpege=%u;vpc=%u;vdec=%u;pngd=%u;venc=%u)\n",
            res->jpegd, res->jpege, res->vpc, res->vdec, res->pngd, res->venc);
        return VMNG_ERR;
    }
    vmngd_alloc_dvpp_engine(vf_dvpp_engine_list, remain_dvpp_engine_list, alloc_dvpp_engine_list);
    vmng_info("Alloc dvpp engine success, (jpegd=%u;jpege=%u;vpc=%u;vdec=%u;pngd=%u;venc=%u)\n",
        res->jpegd, res->jpege, res->vpc, res->vdec, res->pngd, res->venc);
    return VMNG_OK;
}

void vmngd_resource_uninit(u32 dev_id)
{
    if (dev_id >= VMNGD_SURPORT_MAX_DEV) {
        vmng_info("Vmngd resource uninit dev_id check invalid.(dev_id=%u)\n", dev_id);
        return;
    }

    vmngd_free_resource_info(dev_id);

    vmng_info("Vmngd_resource_uninit OK.(dev_id=%u)\n", dev_id);

    return;
}

int vmngd_resource_init(u32 dev_id)
{
    vmngd_resource_info_t *pinfo = NULL;
    int i;

    if (dev_id >= VMNGD_SURPORT_MAX_DEV) {
        vmng_err("Vmngd resource init dev_id check invalid.(dev_id=%u)\n", dev_id);
        return VMNG_ERR;
    }

    if (vmngd_alloc_resource_info(dev_id) != VMNG_OK) {
        return VMNG_ERR;
    }

    pinfo = vmngd_get_resource_info(dev_id);

    vmngd_get_dev_memory_info(dev_id, pinfo);
    vmngd_resource_get_aicpu_info(dev_id, &pinfo->vf_cfg_total.cpu.device_aicpu);
    pinfo->vf_cfg_remain.cpu.device_aicpu.num = pinfo->vf_cfg_total.cpu.device_aicpu.num;
    pinfo->vf_cfg_remain.cpu.device_aicpu.bitmap = pinfo->vf_cfg_total.cpu.device_aicpu.bitmap;

    for (i = 0; i < BASE_VFG_NUM; ++i) {
        INIT_LIST_HEAD(&pinfo->vfg_cfg_each[i].vf_list_head);
        pinfo->vfg_cfg_each[i].status = VMNG_VFG_FREE;
        pinfo->vfg_cfg_each[i].vfg_id = i;
    }

    for (i = 0; i < BASE_VF_NUM; ++i) {
        INIT_LIST_HEAD(&pinfo->vf_cfg_each[i].vf_node);
    }

    pinfo->status = VMNG_VALID;
    vmng_info("Vmngd_resource_init OK.(dev_id=%u)\n", dev_id);
    return VMNG_OK;
}

STATIC int vmngd_bind_vfg_aicpu_info(u32 dev_id, u32 vfid)
{
    vmngd_resource_info_t *resource_info = NULL;
    vmngd_resource_unit_t *vf = NULL;
    vmngd_vfg_cfg_info_t *vfg = NULL;

    if (vfid < VMNGD_VF_NUM_MIN || vfid > BASE_VF_NUM) {
        return VMNG_ERR;
    }

    resource_info = vmngd_get_resource_info(dev_id);
    vf = &resource_info->vf_cfg_each[vfid - VMNGD_VF_NUM_MIN];
    if (vf->vfg == NULL) {
        return VMNG_ERR;
    }
    vfg = vf->vfg;
    bitmap_or(&vfg->cpu.device_aicpu.bitmap,             // add vf aicpu bitmap to vfg aicpu bitmap
        &vfg->cpu.device_aicpu.bitmap, &vf->cpu.device_aicpu.bitmap, CPU_DEV_AICPU_BITMAPNUM);
    return 0;
}

STATIC void vmngd_unbind_vfg_aicpu_info(u32 dev_id, u32 vfid)
{
    vmngd_resource_info_t *resource_info = NULL;
    vmngd_resource_unit_t *vf = NULL;
    vmngd_vfg_cfg_info_t *vfg = NULL;
    unsigned long vfg_bitmap;
    unsigned long vf_bitmap;

    if (vfid < VMNGD_VF_NUM_MIN || vfid > BASE_VF_NUM) {
        return;
    }

    resource_info = vmngd_get_resource_info(dev_id);
    vf = &resource_info->vf_cfg_each[vfid - VMNGD_VF_NUM_MIN];
    if (vf->vfg == NULL) {
        return;
    }
    vfg = vf->vfg;
    vfg_bitmap = vfg->cpu.device_aicpu.bitmap;
    vf_bitmap = vf->cpu.device_aicpu.bitmap;
    // delete vf aicpu bitmap from vfg aicpu bitmap
    // 0 - 1 = 0; 1 - 0 = 1
    bitmap_xor(&vfg_bitmap, &vfg_bitmap, &vf_bitmap, CPU_DEV_AICPU_BITMAPNUM);
    bitmap_and(&vfg_bitmap, &vfg_bitmap, &vfg->cpu.device_aicpu.bitmap, CPU_DEV_AICPU_BITMAPNUM);
    vfg->cpu.device_aicpu.bitmap = vfg_bitmap;
}

STATIC void vmngd_add_vf_to_vfg(vmngd_resource_unit_t *vf, vmngd_vfg_cfg_info_t *vfg)
{
    if (vfg->status == VMNG_VFG_FREE) {
        vfg->status = VMGN_VFG_OCCUPIED;
    }

    vfg->accelerator.aic.num += vf->accelerator.aic.num; // aic is number not bitmap
    list_add_tail(&vf->vf_node, &vfg->vf_list_head);
    bitmap_set(&vfg->vf_bitmap, vf->cfg_info.vfid - 1, 1);
    if (bitmap_weight(&vfg->vf_bitmap, BASE_VF_NUM) == 1) {
        vfg->vfg_type = VMNG_VF_GROUP_TYPE_VIP;
    } else if (bitmap_weight(&vfg->vf_bitmap, BASE_VF_NUM) > 1) {
        vfg->vfg_type = VMNG_VF_GROUP_TYPE_NORMAL;
    }

    vf->cfg_info.vfg_type = vfg->vfg_type;
    vf->vfg = vfg;
    vf->cfg_info.vfg_id = vfg->vfg_id;
}

STATIC void vmngd_del_vf_from_vfg(vmngd_resource_unit_t *vf, vmngd_vfg_cfg_info_t *vfg)
{
    vfg->accelerator.aic.num -= vf->accelerator.aic.num;
    list_del(&vf->vf_node);
    bitmap_clear(&vfg->vf_bitmap, vf->cfg_info.vfid - 1, 1);

    if (bitmap_weight(&vfg->vf_bitmap, BASE_VF_NUM) == 1) {
        vfg->vfg_type = VMNG_VF_GROUP_TYPE_VIP;
    }
    if (bitmap_weight(&vfg->vf_bitmap, BASE_VF_NUM) == 0) {
        vfg->vfg_type = VMNG_VF_GROUP_TYPE_NORMAL;
        vfg->status = VMNG_VFG_FREE;
    }
    vf->cfg_info.vfg_type = vfg->vfg_type;
    vf->vfg = NULL;
}

STATIC int vmngd_bind_vfg(u32 dev_id, u32 vfid)
{
    vmngd_resource_info_t *resource_info = NULL;
    vmngd_resource_unit_t *vf = NULL;
    vmngd_vfg_cfg_info_t *vfg = NULL;
    unsigned int vfg_id;
    int i;

    if (vfid < VMNGD_VF_NUM_MIN || vfid > BASE_VF_NUM) {
        return VMNG_ERR;
    }

    resource_info = vmngd_get_resource_info(dev_id);
    vf = &resource_info->vf_cfg_each[vfid - VMNGD_VF_NUM_MIN];
    if (vf->vfg != NULL) {
        return VMNG_OK;
    }

    vfg_id = vf->cfg_info.vfg_id;
    if ((vfg_id != (u32)-1) && (vfg_id >= BASE_VFG_NUM)) { // vfg_id valid when equal to [-1,0,1,2,3]
        vmng_err("invalid vfg id, failed to alloc vfg. (vfg_id=%u)\n", vfg_id);
        return VMNG_ERR;
    }

    resource_info = vmngd_get_resource_info(vf->cfg_info.dev_id);
    if (vfg_id == (u32)-1) {
        for (i = 0; i < BASE_VFG_NUM; ++i) {
            vfg = &resource_info->vfg_cfg_each[i];
            if ((vf->accelerator.aic.num == 1) && (vfg->accelerator.aic.num <= 1)) {
                break; // alloc a free vfg or a vfg with 1 aicore when requiring 1 aicore
            }
            if ((vf->accelerator.aic.num > 1) && (vfg->status == VMNG_VFG_FREE)) {
                break; // alloc a free vfg when requiring more than 1 aicore
            }
        }
        if (i >= BASE_VFG_NUM) {
            vmng_err("vfg is not enough.\n");
            return VMNG_ERR;
        }
        vmngd_add_vf_to_vfg(vf, vfg);
    } else {
        vfg = &resource_info->vfg_cfg_each[vfg_id];
        vmngd_add_vf_to_vfg(vf, vfg);
    }
    return VMNG_OK;
}

STATIC void vmngd_unbind_vfg(u32 dev_id, u32 vfid)
{
    vmngd_resource_info_t *resource_info = vmngd_get_resource_info(dev_id);
    vmngd_resource_unit_t *vf = NULL;

    if (vfid < VMNGD_VF_NUM_MIN || vfid > BASE_VF_NUM) {
        return;
    }

    vf = &resource_info->vf_cfg_each[vfid - VMNGD_VF_NUM_MIN];
    if (vf->vfg == NULL) {
        return;
    }

    vmngd_del_vf_from_vfg(vf, vf->vfg);
    return;
}

STATIC int vmngd_resource_release_vf(u32 dev_id, u32 vfid)
{
    vmngd_resource_info_t *resource_info = vmngd_get_resource_info(dev_id);
    vmngd_resource_unit_t *remain = NULL;
    vmngd_resource_unit_t *each = NULL;
    u32 idx;

    if (vfid < VMNGD_VF_NUM_MIN || vfid > BASE_VF_NUM) {
        vmng_err("Invalid dev_vfid, (vfid=%u)\n", vfid);
        return VMNG_ERR;
    }

    idx = vfid - VMNGD_VF_NUM_MIN; /* vfid 1~8 */
    each = &resource_info->vf_cfg_each[idx];
    remain = &resource_info->vf_cfg_remain;

    vmngd_element_add(&remain->base.vf, &each->base.vf);
    vmngd_unbind_vfg_aicpu_info(dev_id, vfid);
    vmngd_unbind_vfg(dev_id, vfid);
    vmngd_release_memory_resource(dev_id, each);
    vmngd_release_dvpp_engine(dev_id, each);
    vmngd_element_add(&remain->cpu.device_aicpu, &each->cpu.device_aicpu);
    vmngd_release_aicpu(&each->cpu.device_aicpu, &remain->cpu.device_aicpu);

    vmngd_element_clear(&each->base.vf);

    each->cfg_info.vfid = 0;
    each->cfg_info.dev_id = 0;
    each->cfg_info.vfg_type = 0;
    each->cfg_info.dtype = 0;

    return VMNG_OK;
}

STATIC int vmngd_resource_alloc_vf(struct vmng_ctrl_msg_info *info)
{
    vmngd_resource_info_t *resource_info = vmngd_get_resource_info(info->dev_id);
    vmngd_resource_unit_t *remain = NULL;
    vmngd_resource_unit_t *each = NULL;

    remain = &resource_info->vf_cfg_remain;
    if (remain->base.vf.num == 0) {
        vmng_err("vf is not enough. (remain=%u, bitmap=%#lx)\n", remain->base.vf.num, remain->base.vf.bitmap);
        return VMNG_ERR;
    }

    if (!test_bit((int)(info->vfid - VMNGD_VF_NUM_MIN), &remain->base.vf.bitmap)) { // means this vf is occupied
        vmng_err("vf is occupied, can not alloc vf. (dev_id=%u, vfid=%u)\n", info->dev_id, info->vfid);
        return VMNG_ERR;
    }

    each = &resource_info->vf_cfg_each[info->vfid - VMNGD_VF_NUM_MIN];
    each->accelerator.aic.num = info->vf_cfg.stars_static.aic;
    if (vmngd_alloc_aicpu_for_vf(&each->cpu.device_aicpu, &remain->cpu.device_aicpu,
                                 info->vf_cfg.stars_refresh.device_aicpu)) {
        vmng_err("Alloc aicpu failed.(devid=%u)\n", info->dev_id);
        return VMNG_ERR;
    }
    vmngd_element_sub(&remain->cpu.device_aicpu, &each->cpu.device_aicpu);

    if (vmngd_alloc_dvpp_for_vf(info->dev_id, each, &info->vf_cfg.stars_refresh)) {
        vmngd_element_add(&remain->cpu.device_aicpu, &each->cpu.device_aicpu);
        vmngd_release_aicpu(&each->cpu.device_aicpu, &remain->cpu.device_aicpu);
        vmng_err("Alloc dvpp resource failed. (devid=%u)\n", info->dev_id);
        return VMNG_ERR;
    }

    if (vmngd_alloc_memory_for_vf(info->dev_id, each, info->vf_cfg.memory.size)) {
        vmngd_release_dvpp_engine(info->dev_id, each);
        vmngd_element_add(&remain->cpu.device_aicpu, &each->cpu.device_aicpu);
        vmngd_release_aicpu(&each->cpu.device_aicpu, &remain->cpu.device_aicpu);
        vmng_err("Alloc memory resource failed. (devid=%u)\n", info->dev_id);
        return VMNG_ERR;
    }

    vmngd_element_set(&each->base.vf, 0x1UL << (info->vfid - VMNGD_VF_NUM_MIN));
    vmngd_element_sub(&remain->base.vf, &each->base.vf);

    each->cfg_info.vfid = info->vfid;
    each->cfg_info.dev_id = info->dev_id;
    each->cfg_info.dtype = info->dtype;
    each->cfg_info.vfg_id = info->vf_cfg.vfg.vfg_id;
    remain->accelerator.aic.num += each->accelerator.aic.num;
    if (strcpy_s(each->template_name, VMNG_VF_TEMP_NAME_LEN, info->vf_cfg.name) != 0) {
        vmng_err("Call strcpy_s failed.\n");
        (void)vmngd_resource_release_vf(info->dev_id, info->vfid);
        return VMNG_ERR;
    }

    return VMNG_OK;
}

STATIC int vmngd_resource_alloc_vfg(struct vmng_ctrl_msg_info *info)
{
    vmngd_resource_info_t *resource_info = vmngd_get_resource_info(info->dev_id);
    vmngd_resource_unit_t *each = NULL;
    u32 idx;
    int ret;

    idx = (u32)(info->vfid - VMNGD_VF_NUM_MIN); /* dev_vfid 1~8 */
    each = &resource_info->vf_cfg_each[idx];

    ret = vmngd_bind_vfg(info->dev_id, info->vfid);
    if (ret != VMNG_OK) {
        vmng_err("bind vf to vfg failed, (ret=%d).\n", ret);
        return ret;
    }
    info->vf_cfg.vfg.vfg_id = each->cfg_info.vfg_id;
    vmng_debug("[vf%d]bitmap=0x%lx, num=%u, vf_vip=%u, vfg_id=%u\n", info->vfid, each->base.vfg.bitmap,
        each->base.vfg.num, each->cfg_info.vfg_type, each->cfg_info.vfg_id);

    return VMNG_OK;
}

STATIC int vmngd_resource_alloc(struct vmng_ctrl_msg_info *info)
{
    vmngd_resource_info_t *resource_info = NULL;

    if (info == NULL) {
        vmng_err("param NULL.\n");
        return VMNG_ERR;
    }

    if (info->dev_id >= VMNGD_SURPORT_MAX_DEV) {
        return VMNG_ERR;
    }

    resource_info = vmngd_get_resource_info(info->dev_id);
    mutex_lock(&resource_info->lock);
    if (vmngd_resource_alloc_vf(info) != VMNG_OK) {
        vmng_err("alloc_vf fail.(dev_id=%u,aic=%u)\n", info->dev_id, info->core_num);
        mutex_unlock(&resource_info->lock);
        return VMNG_ERR;
    }

    if (vmngd_resource_alloc_vfg(info) != VMNG_OK) {
        vmng_err("alloc_vfg fail.(dev_id=%u,aic=%u,vfid=%u)\n", info->dev_id, info->core_num, info->vfid);
        (void)vmngd_resource_release_vf(info->dev_id, info->vfid);
        mutex_unlock(&resource_info->lock);
        return VMNG_ERR;
    }

    mutex_unlock(&resource_info->lock);

    vmngd_resource_upate_vf_status(info->dev_id, info->vfid, VMNG_VDEV_STATUS_ALLOC);

    return VMNG_OK;
}

STATIC int vmngd_resource_free(u32 dev_id, u32 vfid)
{
    vmngd_resource_info_t *resource_info = vmngd_get_resource_info(dev_id);

    mutex_lock(&resource_info->lock);
    if (vmngd_resource_release_vf(dev_id, vfid) != VMNG_OK) {
        vmng_err("vmngd_resource_release_vf fail.(vfid=%u)\n", vfid);
    }
    mutex_unlock(&resource_info->lock);

    vmngd_resource_upate_vf_status(dev_id, vfid, VMNG_VDEV_STATUS_FREE);

    return VMNG_OK;
}

STATIC int vmngd_resource_enquire(u32 dev_id, u32 vfid, struct vmng_soc_resource_enquire *info)
{
    vmngd_resource_info_t *resource_info = NULL;
    vmngd_resource_unit_t *each = NULL;
    vmngd_resource_unit_t *remain = NULL;
    vmngd_resource_unit_t *total = NULL;
    u32 idx;

    if (info == NULL) {
        vmng_err("param NULL.\n");
        return VMNG_ERR;
    }
    if (vmngd_resource_info_check(dev_id, VMNGD_ID_NO_CHECK, VMNGD_ID_NO_CHECK) != VMNG_OK) {
        return VMNG_ERR;
    }

    resource_info = vmngd_get_resource_info(dev_id);
    remain = &resource_info->vf_cfg_remain;
    total = &resource_info->vf_cfg_total;

    vmngd_resource_enquire_stars_proc(&info->remain.stars_refresh, &info->remain.stars_static, remain);
    vmngd_resource_enquire_stars_proc(&info->total.stars_refresh, &info->total.stars_static, total);
    info->remain.stars_refresh.device_aicpu = (u32)remain->cpu.device_aicpu.bitmap;
    info->remain.base.memory = remain->base.memory.num >> 10; // 10 convert from kb to mb
    info->remain.base.memory_spec = remain->base.memory.num >> 10; // 10 convert from kb to mb
    info->total.stars_refresh.device_aicpu = (u32)total->cpu.device_aicpu.bitmap;
    info->total.base.memory = total->base.memory.num >> 10; // 10 convert from kb to mb
    info->total.base.memory_spec = total->base.memory.num >> 10; // 10 convert from kb to mb

    if (vfid >= VMNGD_VF_NUM_MIN && vfid <= BASE_VF_NUM) {
        idx = vfid - VMNGD_VF_NUM_MIN;
        each = &resource_info->vf_cfg_each[idx];
        if (each->cfg_info.status != VMNG_VDEV_STATUS_FREE) {
            // Only dev_id, vfid, vfg_id, vfg_type, aic and aicpu_bitmap is effective
            info->each.dev_id = each->cfg_info.dev_id;
            info->each.vfid = each->cfg_info.vfid;
            info->each.vfg.vfg_id = each->cfg_info.vfg_id;
            vmngd_resource_enquire_stars_proc(&info->each.stars_refresh, &info->each.stars_static, each);

            // miniv2 require aicpu bitmap info
            info->each.stars_refresh.device_aicpu = (u32)each->cpu.device_aicpu.bitmap;
            info->each.memory.size = each->base.memory.num >> 10; // 10 convert from kb to mb
            if (strcpy_s(info->each.name, VMNG_VF_TEMP_NAME_LEN, each->template_name) != 0) {
                vmng_err("Call strcpy_s failed.\n");
                return VMNG_ERR;
            }
            if (each->vfg != NULL) {
                info->each.vfg.vfg_type = each->vfg->vfg_type;
                info->vfg.stars_static.aic = each->vfg->accelerator.aic.num;
                info->vfg.stars_refresh.device_aicpu = (u32)each->vfg->cpu.device_aicpu.bitmap;
            }
        }
    } else {
        vmng_info("vfid is 0, can not get vf info, (vfid=%u).\n", vfid);
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
        vfg_info->stars_refresh.device_aicpu = 0xffff; // return all 16(0xffff) cpu as default value
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

STATIC void vmngd_resource_update_vdev_ctrl(u32 dev_id, u32 vfid, struct vmng_vdev_ctrl *ctrl)
{
    vmngd_resource_info_t *resource_info = vmngd_get_resource_info(dev_id);
    vmngd_resource_unit_t *each = NULL;

    each = &resource_info->vf_cfg_each[vfid - VMNGD_VF_NUM_MIN];
    ctrl->vf_cfg.id.vf_id = (u8)vfid;
    ctrl->vf_cfg.id.vfg_id = (u8)each->vfg->vfg_id;
    ctrl->vf_cfg.id.vip = (u8)each->vfg->vfg_type;
    ctrl->vf_cfg.accelerator.aic_bitmap = each->accelerator.aic.num;
    ctrl->vf_cfg.cpu.device_aicpu_bitmap = (u32)each->cpu.device_aicpu.bitmap;
    ctrl->vf_cfg.dvpp.jpegd_bitmap = each->dvpp.jpegd.num;
    ctrl->vf_cfg.dvpp.jpege_bitmap = each->dvpp.jpege.num;
    ctrl->vf_cfg.dvpp.vpc_bitmap = each->dvpp.vpc.num;
    ctrl->vf_cfg.dvpp.vdec_bitmap = each->dvpp.vdec.num;
    ctrl->vf_cfg.dvpp.pngd_bitmap = each->dvpp.pngd.num;
    ctrl->vf_cfg.dvpp.venc_bitmap = each->dvpp.venc.num;
    ctrl->mem_size = each->base.memory.num;
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
    ops->bind_vf_to_vfg = vmngd_bind_vfg_aicpu_info;
    ops->unbind_vf_to_vfg = vmngd_unbind_vfg_aicpu_info;
    ops->update_vdev_ctrl = vmngd_resource_update_vdev_ctrl;

    vmng_info("Res ops init mini_v2.\n");
    return VMNG_OK;
}

int vmngd_res_ops_uninit(struct vmngd_resource_ops *ops)
{
    if (ops == NULL) {
        vmng_err("Param NULL.\n");
        return VMNG_ERR;
    }

    (void)memset_s(ops, sizeof(struct vmngd_resource_ops), 0, sizeof(struct vmngd_resource_ops));

    vmng_info("Res ops uninit mini_v2.\n");
    return VMNG_OK;
}
