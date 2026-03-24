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
 * Create: 2023-02-09
 */

#include <linux/module.h>
#include <linux/errno.h>
#include <linux/bitmap.h>

#include "runenv_config.h"
#include "devdrv_interface.h"
#include "virtmngdev_common_resource.h"
#include "virtmngdev_res_mng.h"

#define NUMA_MAP_STRING(enum_type) [(enum_type)] = #enum_type
char g_res_string[][32] = {
    NUMA_MAP_STRING(MIA_AC_AIV),
    NUMA_MAP_STRING(MIA_AC_AIC),
    NUMA_MAP_STRING(MIA_AC_C_CORE),
    NUMA_MAP_STRING(MIA_AC_DSA),
    NUMA_MAP_STRING(MIA_AC_FFTS),
    NUMA_MAP_STRING(MIA_AC_SDMA),
    NUMA_MAP_STRING(MIA_AC_PCIE_DMA),
    NUMA_MAP_STRING(MIA_STARS_ACSQ),
    NUMA_MAP_STRING(MIA_STARS_RTSQ),
    NUMA_MAP_STRING(MIA_STARS_STREAM),
    NUMA_MAP_STRING(MIA_STARS_EVENT),
    NUMA_MAP_STRING(MIA_STARS_NOTIFY),
    NUMA_MAP_STRING(MIA_STARS_MODEL),
    NUMA_MAP_STRING(MIA_STARS_CMO),
    NUMA_MAP_STRING(MIA_STARS_CDQ),
    NUMA_MAP_STRING(MIA_MEM_NUMA),
    NUMA_MAP_STRING(MIA_SYS_MEM),
    NUMA_MAP_STRING(MIA_STARS_TOPIC_ACPU_SLOT),
    NUMA_MAP_STRING(MIA_STARS_TOPIC_CCPU_SLOT),
    NUMA_MAP_STRING(MIA_CPU_HOST_CCPU),
    NUMA_MAP_STRING(MIA_CPU_DEV_ACPU),
    NUMA_MAP_STRING(MIA_CPU_HOST_ACPU),
    NUMA_MAP_STRING(MIA_DVPP_JPEGD),
    NUMA_MAP_STRING(MIA_DVPP_JPEGE),
    NUMA_MAP_STRING(MIA_DVPP_VPC),
    NUMA_MAP_STRING(MIA_DVPP_VDEC),
    NUMA_MAP_STRING(MIA_DVPP_PNGD),
    NUMA_MAP_STRING(MIA_DVPP_VENC),
};

/* ---------------------------------------resource list manager------------------------------------*/
struct vmng_resource_base *vmngd_get_res_unit(vmng_resource_list *list, enum soc_mia_res_type type)
{
    if (type >= MIA_MAX_RES_TYPE) {
        vmng_err("Invalid resource type.(type=%d)\n", type);
        return NULL;
    }
    return list->res_list[type];
}

STATIC int vmngd_append_resource(vmng_resource_list *list, struct vmng_resource_base *res_node)
{
    if (list->res_list[res_node->type] != NULL) {
        vmng_err("Resource had append to list. (type=%d)\n", res_node->type);
        return -EINVAL;
    }
    list->res_list[res_node->type] = res_node;
    list_add_tail(&res_node->res_node, &list->resource_list_head);
    return 0;
}

STATIC void vmngd_delete_resource(vmng_resource_list *list, struct vmng_resource_base *res_node)
{
    list_del(&res_node->res_node);
    list->res_list[res_node->type] = NULL;
}

STATIC void vmngd_free_resource_base(struct vmng_resource_base *res)
{
    if (res == NULL) {
        return;
    }
    res->free_resource(res);
    list_del(&res->res_node);
    kfree(res);
}

STATIC void vmngd_free_resource_list(vmng_resource_list *list)
{
    struct vmng_resource_base *res_node = NULL;
    struct list_head *pos = NULL, *n = NULL;

    list_for_each_safe(pos, n, &list->resource_list_head) {
        res_node = list_entry(pos, struct vmng_resource_base, res_node);
        if (res_node != NULL) {
            list->res_list[res_node->type] = NULL;
            vmngd_free_resource_base(res_node);
        }
    }
}

STATIC int vmngd_copy_resource_list(vmng_resource_list *list, vmng_resource_list *from)
{
    struct vmng_resource_base *res_node = NULL;
    struct vmng_resource_base *new_node = NULL;
    int ret = 0;
    int i;

    for (i = 0; i < MIA_MAX_RES_TYPE; ++i) {
        res_node = from->res_list[i];
        if (res_node == NULL) {
            continue;
        }
        new_node = kzalloc(sizeof(struct vmng_resource_base), GFP_KERNEL);
        if (new_node == NULL) {
            vmng_err("Alloc new node failed.\n");
            ret = -ENOMEM;
            goto failed;
        }
        ret = memcpy_s(new_node, sizeof(struct vmng_resource_base), res_node, sizeof(struct vmng_resource_base));
        if (ret != 0) {
            vmng_err("Memcpy res_node to new node failed. (ret=%d)\n", ret);
            kfree(new_node);
            new_node = NULL;
            goto failed;
        }
        ret = vmngd_append_resource(list, new_node);
        if (ret != 0) {
            vmng_err("Append resource to list failed. (ret=%d)\n", ret);
            kfree(new_node);
            new_node = NULL;
            goto failed;
        }
    }
    return 0;

failed:
    for (; i >= 0; --i) {
        new_node = list->res_list[i];
        if (new_node != NULL) {
            vmngd_delete_resource(list, new_node);
            kfree(new_node);
            new_node = NULL;
        }
    }
    return ret;
}

STATIC void vmngd_clear_resource_list(vmng_resource_list *list)
{
    struct vmng_resource_base *res_node = NULL;
    int i;

    for (i = 0; i < MIA_MAX_RES_TYPE; ++i) {
        res_node = list->res_list[i];
        if (res_node != NULL) {
            vmngd_element_clear(&res_node->element);
        }
    }
}

/* ----------------------------------------- vf manager -----------------------------------------*/
STATIC struct vmng_vf_object *vmngd_alloc_vf_object(struct vmng_pf_object *pf, unsigned int vfid)
{
    struct vmng_vf_object *vf_obj = kzalloc(sizeof(struct vmng_vf_object), GFP_KERNEL);
    int ret;

    if (vf_obj == NULL) {
        vmng_err("Kzalloc vf object resource failed.\n");
        return NULL;
    }
    vf_obj->dev_id = pf->dev_id;
    vf_obj->vfid = vfid;
    INIT_LIST_HEAD(&vf_obj->resource.resource_list_head);

    ret = vmngd_copy_resource_list(&vf_obj->resource, &pf->total_res);
    if (ret != 0) {
        vmng_err("Copy resource list failed.(dev_id=%u;vfid=%u;ret=%u)\n", pf->dev_id, vfid, ret);
        kfree(vf_obj);
        vf_obj = NULL;
        return NULL;
    }
    vmngd_clear_resource_list(&vf_obj->resource);

    vf_obj->pf = pf;
    pf->vf_num++;
    list_add_tail(&vf_obj->pf_vf_node, &pf->vf_list_head);
    return vf_obj;
}

struct vmng_vf_object *vmngd_get_vf_object(unsigned int dev_id, unsigned int vfid)
{
    struct vmng_pf_object *pf_node = NULL;
    struct vmng_vf_object *vf_node = NULL;
    struct list_head *pos = NULL, *n = NULL;

    pf_node = vmngd_get_pf_object(dev_id);
    if (pf_node == NULL) {
        return NULL;
    }

    list_for_each_safe(pos, n, &pf_node->vf_list_head) {
        vf_node = list_entry(pos, struct vmng_vf_object, pf_vf_node);
        if ((vf_node != NULL) && (vf_node->vfid == vfid)) {
            return vf_node;
        }
    }
    return NULL;
}

STATIC void vmngd_free_vf_object(struct vmng_vf_object *vf_node)
{
    if (vf_node == NULL) {
        vmng_err("Cannot find vf device.\n");
        return;
    }
    vf_node->pf->vf_num--;
    list_del(&vf_node->pf_vf_node);
    if (vf_node->vfg != NULL) {
        list_del(&vf_node->vfg_vf_node);
        vf_node->vfg = NULL;
    }
    vmngd_free_resource_list(&vf_node->resource);

    kfree(vf_node);
}

int get_vf_res_num(struct vmng_vf_object *vf_obj, enum soc_mia_res_type type)
{
    struct vmng_resource_base *type_res = vmngd_get_res_unit(&vf_obj->resource, type);
    return type_res == NULL ? 0 : type_res->element.num;
}

unsigned long get_vf_res_bitmap(struct vmng_vf_object *vf_obj, enum soc_mia_res_type type)
{
    struct vmng_resource_base *type_res = vmngd_get_res_unit(&vf_obj->resource, type);
    return type_res == NULL ? 0 : type_res->element.bitmap;
}

static inline int get_vfg_res_num(struct vmng_vfg_object *vfg_obj, enum soc_mia_res_type type)
{
    struct vmng_resource_base *type_res = vmngd_get_res_unit(&vfg_obj->resource, type);
    return type_res == NULL ? 0 : type_res->element.num;
}

static inline unsigned long get_vfg_res_bitmap(struct vmng_vfg_object *vfg_obj, enum soc_mia_res_type type)
{
    struct vmng_resource_base *type_res = vmngd_get_res_unit(&vfg_obj->resource, type);
    return type_res == NULL ? 0 : type_res->element.bitmap;
}

/* ----------------------------------------- vfg manager -----------------------------------------*/
STATIC struct vmng_vfg_object *vmngd_alloc_vfg_object(struct vmng_pf_object *pf, unsigned int vfg_id)
{
    struct vmng_vfg_object *vfg_obj = kzalloc(sizeof(struct vmng_vfg_object), GFP_KERNEL);
    int ret;

    if (vfg_obj == NULL) {
        vmng_err("Kzalloc vfg object resource failed.\n");
        return NULL;
    }
    vfg_obj->dev_id = pf->dev_id;
    vfg_obj->vfg_id = vfg_id;
    INIT_LIST_HEAD(&vfg_obj->resource.resource_list_head);
    INIT_LIST_HEAD(&vfg_obj->vf_list_head);

    ret = vmngd_copy_resource_list(&vfg_obj->resource, &pf->total_res);
    if (ret != 0) {
        vmng_err("Copy resource list failed.(dev_id=%u;vfgid=%u;ret=%u)\n", pf->dev_id, vfg_id, ret);
        kfree(vfg_obj);
        vfg_obj = NULL;
        return NULL;
    }
    vmngd_clear_resource_list(&vfg_obj->resource);

    vfg_obj->pf = pf;
    pf->vfg_num++;
    list_add_tail(&vfg_obj->pf_vfg_node, &pf->vfg_list_head);
    vmng_info("Alloc vfg (dev_id=%u;vfg_id=%u)\n", pf->dev_id, vfg_id);
    return vfg_obj;
}

struct vmng_vfg_object *vmngd_get_vfg_object(struct vmng_pf_object *pf, unsigned int vfg_id)
{
    struct vmng_vfg_object *vfg_node = NULL;
    struct list_head *pos = NULL, *n = NULL;

    list_for_each_safe(pos, n, &pf->vfg_list_head) {
        vfg_node = list_entry(pos, struct vmng_vfg_object, pf_vfg_node);
        if ((vfg_node != NULL) && (vfg_node->vfg_id == vfg_id)) {
            return vfg_node;
        }
    }
    return NULL;
}

STATIC void vmngd_free_vfg_object(struct vmng_vfg_object *vfg_node)
{
    struct vmng_vf_object *vf_node = NULL;
    struct list_head *pos = NULL, *n = NULL;

    if (vfg_node == NULL) {
        vmng_err("Cannot find vf device.\n");
        return;
    }
    vfg_node->pf->vfg_num--;
    list_del(&vfg_node->pf_vfg_node);
    vmngd_free_resource_list(&vfg_node->resource);

    // delete all vf
    list_for_each_safe(pos, n, &vfg_node->vf_list_head) {
        vf_node = list_entry(pos, struct vmng_vf_object, vfg_vf_node);
        if (vf_node != NULL) {
            vmngd_free_vf_object(vf_node);
        }
    }

    kfree(vfg_node);
}

/* ----------------------------------------- pf manager -----------------------------------------*/
struct list_head g_device_list = LIST_HEAD_INIT(g_device_list);
STATIC struct vmng_pf_object *vmngd_alloc_pf_object(unsigned int dev_id)
{
    struct vmng_pf_object *pf_obj = kzalloc(sizeof(struct vmng_pf_object), GFP_KERNEL);
    if (pf_obj == NULL) {
        vmng_err("Kzalloc pf object resource failed.\n");
        return NULL;
    }
    pf_obj->dev_id = dev_id;
    INIT_LIST_HEAD(&pf_obj->total_res.resource_list_head);
    INIT_LIST_HEAD(&pf_obj->remain_res.resource_list_head);
    INIT_LIST_HEAD(&pf_obj->vfg_list_head);
    INIT_LIST_HEAD(&pf_obj->vf_list_head);
    list_add_tail(&pf_obj->pf_node, &g_device_list);
    return pf_obj;
}

struct vmng_pf_object *vmngd_get_pf_object(unsigned int dev_id)
{
    struct vmng_pf_object *pf_node = NULL;
    struct list_head *pos = NULL, *n = NULL;

    list_for_each_safe(pos, n, &g_device_list) {
        pf_node = list_entry(pos, struct vmng_pf_object, pf_node);
        if ((pf_node != NULL) && (pf_node->dev_id == dev_id)) {
            return pf_node;
        }
    }
    return NULL;
}

STATIC void vmngd_free_pf_object(struct vmng_pf_object *pf_node)
{
    struct vmng_vfg_object *vfg_node = NULL;
    struct vmng_vf_object *vf_node = NULL;
    struct list_head *pos = NULL, *n = NULL;

    if (pf_node == NULL) {
        vmng_err("Input pf_node is NULL.\n");
        return;
    }
    // delete all vfg
    list_for_each_safe(pos, n, &pf_node->vfg_list_head) {
        vfg_node = list_entry(pos, struct vmng_vfg_object, pf_vfg_node);
        if (vfg_node != NULL) {
            vmngd_free_vfg_object(vfg_node);
        }
    }

    // delete all vf
    list_for_each_safe(pos, n, &pf_node->vf_list_head) {
        vf_node = list_entry(pos, struct vmng_vf_object, pf_vf_node);
        if (vf_node != NULL) {
            vmngd_free_vf_object(vf_node);
        }
    }

    vmngd_free_resource_list(&pf_node->total_res);
    vmngd_free_resource_list(&pf_node->remain_res);
    list_del(&pf_node->pf_node);
    kfree(pf_node);
}

int get_total_res_num(struct vmng_pf_object *pf_obj, enum soc_mia_res_type type)
{
    struct vmng_resource_base *type_res = vmngd_get_res_unit(&pf_obj->total_res, type);
    return type_res == NULL ? 0 : type_res->element.num;
}

unsigned long get_total_res_bitmap(struct vmng_pf_object *pf_obj, enum soc_mia_res_type type)
{
    struct vmng_resource_base *type_res = vmngd_get_res_unit(&pf_obj->total_res, type);
    return type_res == NULL ? 0 : type_res->element.bitmap;
}

int vmngd_init_pf_instance(unsigned int dev_id)
{
    struct vmng_pf_object *pf_obj = NULL;

    pf_obj = vmngd_alloc_pf_object(dev_id);
    if (pf_obj == NULL) {
        vmng_info("Alloc pf failed. (dev_id=%u)\n", dev_id);
        return -ENOMEM;
    }
    return 0;
}

void vmngd_uninit_pf_instance(unsigned int dev_id)
{
    struct vmng_pf_object *pf_obj = NULL;

    pf_obj = vmngd_get_pf_object(dev_id);
    vmngd_free_pf_object(pf_obj);
}

int vmngd_init_vfg_instance(unsigned int dev_id, unsigned int vfg_id)
{
    struct vmng_pf_object *pf_obj = NULL;
    struct vmng_vfg_object *vfg_obj = NULL;

    pf_obj = vmngd_get_pf_object(dev_id);
    if (pf_obj == NULL) {
        vmng_info("Get pf failed. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }
    vfg_obj = vmngd_alloc_vfg_object(pf_obj, vfg_id);
    if (vfg_obj == NULL) {
        vmng_info("Alloc vfg object failed. (dev_id=%u;vfg_id=%u)\n", dev_id, vfg_id);
        return -ENOMEM;
    }
    return 0;
}

void vmngd_uninit_vfg_instance(unsigned int dev_id, unsigned int vfg_id)
{
    struct vmng_pf_object *pf_obj = NULL;
    struct vmng_vfg_object *vfg_obj = NULL;

    pf_obj = vmngd_get_pf_object(dev_id);
    if (pf_obj == NULL) {
        vmng_info("Get pf object failed. (dev_id=%u)\n", dev_id);
        return;
    }
    vfg_obj = vmngd_get_vfg_object(pf_obj, vfg_id);
    if (vfg_obj == NULL) {
        vmng_info("Alloc vfg object failed. (dev_id=%u;vfg_id=%u)\n", dev_id, vfg_id);
        return;
    }
    vmngd_free_vfg_object(vfg_obj);
}

static inline void vmngd_bind_vf_to_vfg(struct vmng_vf_object *vf_node, struct vmng_vfg_object *vfg_node)
{
    vf_node->vfg = vfg_node;
    list_add_tail(&vf_node->vfg_vf_node, &vfg_node->vf_list_head);
}

static inline void vmngd_unbind_vf_from_vfg(struct vmng_vf_object *vf_node)
{
    list_del(&vf_node->vfg_vf_node);
    vf_node->vfg = NULL;
}

STATIC bool vmngd_is_soft_divide(struct vmng_pf_object *pf_node, u32 aic_num)
{
    u32 soft_cut_aic_num;
    u32 pf_aic;

    pf_aic = get_total_res_num(pf_node, MIA_AC_AIC);
    soft_cut_aic_num = pf_aic / BASE_VFG_NUM;

    return aic_num < soft_cut_aic_num;
}

STATIC int vmngd_assign_vfg_by_aic(struct vmng_vf_object *vf_node, struct vmng_vf_spec_info *vf_cfg)
{
    struct vmng_vfg_object *vfg_node = NULL;
    struct list_head *pos = NULL, *n = NULL;
    u32 vfg_aic, vf_aic;

    if (vf_node->pf->chip_type == HISI_MINI_V3) { // miniv3 has only one vfg
        vfg_node = vmngd_get_vfg_object(vf_node->pf, 0);
        if (vfg_node == NULL) {
            vmng_err("Get vfg 0 failed. (dev_id=%u;vfid=%u)\n", vf_node->pf->dev_id, vf_node->vfid);
            return -EINVAL;
        }
        vmngd_bind_vf_to_vfg(vf_node, vfg_node);
        return 0;
    }

    vf_aic = vf_cfg->res_list[MIA_AC_AIC];
    list_for_each_safe(pos, n, &vf_node->pf->vfg_list_head) {
        vfg_node = list_entry(pos, struct vmng_vfg_object, pf_vfg_node);
        if (vfg_node == NULL || vfg_node->vfg_type == VMNG_VF_GROUP_TYPE_VIP) {
            continue;
        }
        vfg_aic = get_vfg_res_num(vfg_node, MIA_AC_AIC);
        if (vmngd_is_soft_divide(vf_node->pf, vf_aic) && vmngd_is_soft_divide(vf_node->pf, vfg_aic)) {
            // If vf is soft divide and vfg has not enough aic, share this vfg with other vfs
            vmngd_bind_vf_to_vfg(vf_node, vfg_node);
            return 0;
        } else if ((!vmngd_is_soft_divide(vf_node->pf, vf_aic)) && (vfg_aic == 0)) {
            // If vf is hard divide, vf use the whole vfg itself.
            vmngd_bind_vf_to_vfg(vf_node, vfg_node);
            return 0;
        }
    }
    vmng_err("Assign vfg failed.(dev_id=%u;vfid=%u;vf_aic=%u)\n",
             vf_node->pf->dev_id, vf_node->vfid, vf_aic);
    return -EINVAL;
}

STATIC int vmngd_assign_vfg(struct vmng_vf_object *vf_node, struct vmng_vf_spec_info *vf_cfg)
{
    struct vmng_vfg_object *vfg_node = NULL;

    if (vf_node->vfg != NULL) {
        vmng_err("VF device had bind to vfg.(dev_id=%u;vfid=%u;vfg_id=%u;in_vfg_id=%u)\n",
                 vf_node->dev_id, vf_node->vfid, vf_node->vfg->vfg_id, vf_cfg->vfg_id);
        return -EINVAL;
    }

    // if vfg_id is -1, assign vfg according to aicore
    if (vf_cfg->vfg_id == U32_MAX) {
        return vmngd_assign_vfg_by_aic(vf_node, vf_cfg);
    }

    vfg_node = vmngd_get_vfg_object(vf_node->pf, vf_cfg->vfg_id);
    if (vfg_node == NULL) {
        vmng_err("Failed to get vfg. (dev_id=%u;vfid=%u;vfg_id=%u)\n",
                 vf_node->dev_id, vf_node->vfid, vf_cfg->vfg_id);
        return -EINVAL;
    }
    vmngd_bind_vf_to_vfg(vf_node, vfg_node);
    return 0;
}

STATIC int vmngd_set_resource(struct vmng_resource_base *self, vmngd_element_t *element)
{
    int ret;
    ret = memcpy_s(&self->element, sizeof(vmngd_element_t), element, sizeof(vmngd_element_t));
    if (ret != 0) {
        vmng_err("Memcpy element failed. resource_type = %d\n", self->type);
        return ret;
    }
    return 0;
}

STATIC int vmngd_get_res_num(struct vmng_resource_base *self)
{
    return self->element.num;
}

STATIC unsigned long vmngd_get_res_bitmap(struct vmng_resource_base *self)
{
    return self->element.bitmap;
}

// AICPU RESOURCE ALLOC RULE
STATIC int vmngd_alloc_vf_aicpu(struct vmng_resource_base *self, struct vmng_resource_base *from,
                                const u32 alloc_aicpu_num)
{
    return vmngd_alloc_aicpu_for_vf(&self->element, &from->element, alloc_aicpu_num);
}

STATIC void vmngd_free_vf_aicpu(struct vmng_resource_base *self)
{
    vmngd_element_clear(&self->element);
}

STATIC struct vmng_resource_base *vmngd_alloc_aicpu_resource(void)
{
    struct vmng_resource_base *aicpu_resource = kzalloc(sizeof(struct vmng_resource_base), GFP_KERNEL);
    if (aicpu_resource == NULL) {
        vmng_err("Kzalloc aicpu resource failed.\n");
        return NULL;
    }

    aicpu_resource->type = MIA_CPU_DEV_ACPU;
    aicpu_resource->alloc_resource = vmngd_alloc_vf_aicpu;
    aicpu_resource->free_resource = vmngd_free_vf_aicpu;
    aicpu_resource->set_resource = vmngd_set_resource;
    aicpu_resource->get_num = vmngd_get_res_num;
    aicpu_resource->get_bitmap = vmngd_get_res_bitmap;
    return aicpu_resource;
}

// RESOURCE NORMAL RULE
STATIC int vmngd_alloc_vf_normal(struct vmng_resource_base *self, struct vmng_resource_base *from,
                                 const u32 alloc_num)
{
    int ret;

    if (from->element.attr & ATTR_NUM_RESOURCE) {
        if (alloc_num > from->element.num) {
            vmng_err("Resource is not enough.(type=%s;remain=%u;alloc_num=%u)\n",
                     g_res_string[from->type], from->element.num, alloc_num);
            return -EINVAL;
        }
        self->type = from->type;
        self->element.attr = from->element.attr;
        self->element.num = alloc_num;
        return 0;
    }

    ret = vmngd_element_alloc(&self->element, &from->element, alloc_num);
    if (ret != 0) {
        vmng_err("Alloc resource failed. (type=%s;remain=%#lx;alloc_num=%u)\n",
                 g_res_string[from->type], from->element.bitmap, alloc_num);
        return ret;
    }
    return VMNG_OK;
}

STATIC void vmngd_free_vf_normal(struct vmng_resource_base *self)
{
    vmngd_element_clear(&self->element);
}

STATIC struct vmng_resource_base *vmngd_alloc_normal_resource(struct vmng_resource *resource)
{
    struct vmng_resource_base *normal_resource = kzalloc(sizeof(struct vmng_resource_base), GFP_KERNEL);
    if (normal_resource == NULL) {
        vmng_err("Kzalloc resource failed. (type=%s)\n", g_res_string[resource->type]);
        return NULL;
    }

    normal_resource->type = resource->type;
    normal_resource->alloc_resource = vmngd_alloc_vf_normal;
    normal_resource->free_resource = vmngd_free_vf_normal;
    normal_resource->set_resource = vmngd_set_resource;
    normal_resource->get_num = vmngd_get_res_num;
    normal_resource->get_bitmap = vmngd_get_res_bitmap;
    return normal_resource;
}

STATIC struct vmng_resource_base *vmngd_alloc_base_resource(struct vmng_resource *resource)
{
    struct vmng_resource_base *res_base = NULL;
    int ret;

    if (resource->type == MIA_CPU_DEV_ACPU) {
        res_base = vmngd_alloc_aicpu_resource();
    } else {
        res_base = vmngd_alloc_normal_resource(resource);
    }

    if (res_base == NULL) {
        return NULL;
    }

    ret = res_base->set_resource(res_base, &resource->element);
    if (ret != 0) {
        kfree(res_base);
        vmng_err("Set resource failed. (type=%d;ret=%d)\n", resource->type, ret);
        return NULL;
    }
    return res_base;
}

STATIC void vmngd_free_base_resource(struct vmng_resource_base *res_base)
{
    kfree(res_base);
}

STATIC int vmngd_add_base_resource(u32 dev_id, struct vmng_resource *resource, vmng_resource_list *res_list)
{
    struct vmng_resource_base *res = NULL;
    int ret;

    res = vmngd_alloc_base_resource(resource);
    if (res == NULL) {
        vmng_err("Alloc base resource failed. (dev_id=%u;type=%d)\n", dev_id, resource->type);
        return -EINVAL;
    }

    ret = vmngd_append_resource(res_list, res);
    if (ret != 0) {
        vmngd_free_base_resource(res);
        vmng_err("Append resource failed. (dev_id=%u;type=%d;ret=%d)\n", dev_id, resource->type, ret);
        return ret;
    }

    return 0;
}

STATIC void vmngd_del_base_resource(vmng_resource_list *res_list, struct vmng_resource *resource)
{
    struct vmng_resource_base *res = NULL;
    res = vmngd_get_res_unit(res_list, resource->type);
    if (res != NULL) {
        vmngd_delete_resource(res_list, res);
        vmngd_free_base_resource(res);
        res = NULL;
    }
}

int vmngd_resource_register(u32 dev_id, struct vmng_resource *resource)
{
    struct vmng_pf_object *pf_node = NULL;
    int ret;

    pf_node = vmngd_get_pf_object(dev_id);
    if (pf_node == NULL) {
        vmng_err("Get pf node failed. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }
    if (resource == NULL) {
        vmng_err("Input resource is NULL. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    ret = vmngd_add_base_resource(dev_id, resource, &pf_node->total_res);
    if (ret != 0) {
        vmng_err("Add base resource to total list failed. (dev_id=%u;type=%d)\n", dev_id, resource->type);
        return ret;
    }

    ret = vmngd_add_base_resource(dev_id, resource, &pf_node->remain_res);
    if (ret != 0) {
        vmngd_del_base_resource(&pf_node->total_res, resource);
        vmng_err("Add base resource to remain list failed. (dev_id=%u;type=%d)\n", dev_id, resource->type);
        return ret;
    }

    return 0;
}

void vmngd_resource_unregister(u32 dev_id, struct vmng_resource *resource)
{
    struct vmng_pf_object *pf_node = NULL;

    pf_node = vmngd_get_pf_object(dev_id);
    if (pf_node == NULL) {
        vmng_err("Get pf node failed. (dev_id=%u)\n", dev_id);
        return;
    }
    vmngd_del_base_resource(&pf_node->total_res, resource);
    vmngd_del_base_resource(&pf_node->remain_res, resource);
    return;
}

static inline void vmngd_adapt_total_aic(unsigned int *pf_aic)
{
#define MILAN_BIN3_AIC_NUM 25
#define MILAN_BIN0_AIC_NUM 24
    if (*pf_aic == MILAN_BIN3_AIC_NUM) {
        *pf_aic = MILAN_BIN0_AIC_NUM;
    }
}

// input is info from host, which are user input data
STATIC void vmngd_get_vf_specification(struct vmng_pf_object *pf_node, struct vmng_vf_spec_info *spec,
                                       u32 *numerator, u32 *denominator)
{
    unsigned int pf_total_aic;

    if (spec->dtype == VMNG_HW_TYPE_C1_4) {
        *numerator = 1;
        *denominator = 4;   // 1 / 4 of davinci chip computing capability
        return;
    } else if (spec->dtype == VMNG_HW_TYPE_C2_4) {
        *numerator = 1;
        *denominator = 2;   // 1 / 2 of davinci chip computing capability
        return;
    }

    pf_total_aic = get_total_res_num(pf_node, MIA_AC_AIC);
    vmngd_adapt_total_aic(&pf_total_aic);
    *numerator = spec->res_list[MIA_AC_AIC];
    *denominator = pf_total_aic;
}

STATIC bool vmngd_is_template_res(enum soc_mia_res_type type)
{
    if (type == MIA_AC_AIC) {
        return true;
    }
    if ((type == MIA_CPU_DEV_ACPU) || (type == MIA_SYS_MEM)) {
        return true;
    }
    if ((type >= MIA_DVPP_JPEGD) && (type <= MIA_DVPP_VENC)) {
        return true;
    }
    if (type == MIA_MEM_NUMA) {
        return true;
    }
    return false;
}

#define MAX_POOL_NUM 8
STATIC int vmngd_cal_resource(struct vmng_pf_object *pf_node, struct vmng_vf_spec_info *spec,
                              u32 *resource_num, u32 size)
{
    struct vmng_resource_base *res_node = NULL;
    struct list_head *pos = NULL, *n = NULL;
    u32 numerator = 0;
    u32 denominator = 0;
    u32 type;

    vmngd_get_vf_specification(pf_node, spec, &numerator, &denominator);
    if ((denominator == 0) || (numerator == 0) || (numerator > denominator)) {
        vmng_err("Invalid value, check aic and memory. (dev_id=%u;numerator=%u;denominator=%u)\n",
                 pf_node->dev_id, numerator, denominator);
        return -EINVAL;
    }
    vmng_info("Get computing spec. (dev_id=%u;vfid=%u;numerator=%u;denominator=%u)\n",
              pf_node->dev_id, spec->vfid, numerator, denominator);

    list_for_each_safe(pos, n, &pf_node->total_res.resource_list_head) {
        res_node = list_entry(pos, struct vmng_resource_base, res_node);
        type = res_node->type;
        if (type < size) {
            if (res_node->element.attr & ATTR_VFG_FULL_ALLOC) {
                resource_num[type] = res_node->element.num;
                continue;
            }
            if (vmngd_is_template_res(type)) {  // if the res is in template, use the data input by user
                resource_num[type] = spec->res_list[type];
                continue;
            }
            if (res_node->element.attr & ATTR_EQUAL_ALLOC) {
                resource_num[type] = vmngd_get_element_bitnum(&res_node->element) / MAX_POOL_NUM;
                continue;
            }
            resource_num[type] = vmngd_get_element_bitnum(&res_node->element) *
                numerator / denominator * res_node->element.num_per_bit;
        }
    }

    if (pf_node->chip_type == HISI_CLOUD_V2) {
        if (resource_num[MIA_AC_AIC] <= 1) {
            vmng_err("Invalid aicore num. (aicore=%u)\n", resource_num[MIA_AC_AIC]);
            return -EINVAL;
        }
        resource_num[MIA_AC_AIV] = 2 * resource_num[MIA_AC_AIC]; // aiv num must be 2 times as much as ai core num
        res_node = pf_node->total_res.res_list[MIA_MEM_NUMA];
        if ((resource_num[MIA_MEM_NUMA] == 0) && (res_node != NULL)) {
            resource_num[MIA_MEM_NUMA] = vmngd_get_element_bitnum(&res_node->element) *
                numerator / denominator * res_node->element.num_per_bit;
        }
    }
    return 0;
}

STATIC struct vmng_vf_object *vmngd_find_share_vf(const struct vmng_vf_object *vf)
{
    struct vmng_vfg_object *vfg = vf->vfg;
    struct vmng_vf_object *share_vf = NULL;
    struct list_head *pos = NULL, *n = NULL;

    list_for_each_safe(pos, n, &vfg->vf_list_head) {
        share_vf = list_entry(pos, struct vmng_vf_object, vfg_vf_node);
        if (share_vf == vf) {
            continue;
        }
        if (share_vf->dtype == vf->dtype) {
            return share_vf;
        }
    }
    return NULL;
}

STATIC int alloc_vf_res_from_remain(struct vmng_vf_object *vf, enum soc_mia_res_type type,
                                    const u32 alloc_num)
{
    struct vmng_resource_base *self = vmngd_get_res_unit(&vf->resource, type);
    struct vmng_resource_base *from = vmngd_get_res_unit(&vf->pf->remain_res, type);
    struct vmng_resource_base *vfg_res = vmngd_get_res_unit(&vf->vfg->resource, type);
    int ret;

    if (self == NULL || from == NULL || vfg_res == NULL) {
        return -EINVAL;
    }

    ret = self->alloc_resource(self, from, alloc_num);
    if (ret != 0) {
        vmng_err("Alloc resource failed. (dev_id=%u;vfid=%u;type=%s;ret=%d)\n",
                 vf->dev_id, vf->vfid, g_res_string[type], ret);
        return ret;
    }
    if (from->element.attr & ATTR_NUM_RESOURCE) {
        vfg_res->element.num += self->element.num;
        from->element.num -= self->element.num;
    } else {
        vmngd_element_add(&vfg_res->element, &self->element);
        vmngd_element_sub(&from->element, &self->element);
    }
    return 0;
}

STATIC int alloc_vf_res_from_vfg(struct vmng_vf_object *vf, enum soc_mia_res_type type,
                                 const u32 alloc_num)
{
    struct vmng_resource_base *self = vmngd_get_res_unit(&vf->resource, type);
    struct vmng_resource_base *from = NULL;
    struct vmng_vf_object *share_vf = NULL;
    int ret;

    share_vf = vmngd_find_share_vf(vf);
    if (share_vf != NULL) {
        from = vmngd_get_res_unit(&share_vf->resource, type);
        if (from == NULL) {
            return -EINVAL;
        }
        ret = self->alloc_resource(self, from, alloc_num);
    } else {
        ret = alloc_vf_res_from_remain(vf, type, alloc_num);
    }
    if (ret != 0) {
        vmng_err("Alloc failed. (dev_id=%u;vfid=%u;type=%s)\n", vf->dev_id, vf->vfid, g_res_string[type]);
    }
    return ret;
}

STATIC int alloc_res_from_full_vfg(struct vmng_vf_object *vf, enum soc_mia_res_type type,
                                   const u32 alloc_num)
{
    struct vmng_resource_base *self = vmngd_get_res_unit(&vf->resource, type);
    struct vmng_resource_base *from = NULL;
    int ret;

    from = vmngd_get_res_unit(&vf->vfg->resource, type);
    if (self == NULL || from == NULL) {
        return -EINVAL;
    }
    if (from->element.num == 0) {
        ret = alloc_vf_res_from_remain(vf, type, alloc_num);
    } else {
        ret = self->alloc_resource(self, from, alloc_num);
    }
    if (ret != 0) {
        vmng_err("Alloc from vfg failed. (dev_id=%u;vfid=%u;type=%s)\n", vf->dev_id, vf->vfid, g_res_string[type]);
    }
    return ret;
}

STATIC int vmngd_recalculate_vfg_resource(struct vmng_resource_base *res, struct vmng_vf_object *vf)
{
    struct vmng_resource_base *share_res = NULL;
    struct vmng_vf_object *vf_node = NULL;
    struct list_head *pos = NULL, *n = NULL;

    // sub share resource in vfg
    list_for_each_safe(pos, n, &vf->vfg->vf_list_head) {
        vf_node = list_entry(pos, struct vmng_vf_object, vfg_vf_node);
        if (vf_node == vf) {
            continue;
        }
        share_res = vmngd_get_res_unit(&vf_node->resource, res->type);
        if (share_res == NULL) {
            vmng_err("Resource is NULL. type=%s\n", g_res_string[res->type]);
            continue;
        }
        vmngd_element_sub(&res->element, &share_res->element);
    }
    share_res = vmngd_get_res_unit(&vf->vfg->resource, res->type);
    vmngd_element_sub(&share_res->element, &res->element);
    return 0;
}

STATIC int vmngd_recalculate_resource(struct vmng_vf_object *vf_node)
{
    struct vmng_resource_base *pf_res = NULL;
    struct vmng_resource_base *res = NULL;
    struct list_head *pos = NULL, *n = NULL;

    list_for_each_safe(pos, n, &vf_node->resource.resource_list_head) {
        res = list_entry(pos, struct vmng_resource_base, res_node);
        vmngd_recalculate_vfg_resource(res, vf_node);
        // return the resource to pf remain resource.
        pf_res = vmngd_get_res_unit(&vf_node->pf->remain_res, res->type);
        vmngd_element_add(&pf_res->element, &res->element);
    }
    return 0;
}

STATIC int vmngd_set_vf_base_info(struct vmng_vf_object *vf_node, struct vmng_vf_spec_info *spec)
{
    int ret;
    ret = strcpy_s(vf_node->template_name, VMNG_VF_TEMP_NAME_LEN, spec->template_name);
    if (ret != 0) {
        vmng_err("Strcpy vf template name failed. (ret=%d)\n", ret);
        return ret;
    }
    vf_node->dtype = spec->dtype;
    vf_node->token = spec->token;
    vf_node->token_max = spec->token_max;
    vf_node->task_timeout = spec->task_timeout;
    vf_node->vfg->vfg_type = spec->vfg_type;
    vf_node->vfg->vfg_mode = spec->vfg_mode;
    spec->vfg_id = vf_node->vfg->vfg_id;
    vf_node->vm_full_spec_enable = spec->vm_full_spec_enable;
    return 0;
}

STATIC int vmngd_alloc_vf_resource_single(struct vmng_vf_object *vf_node, u32 res_list[], u32 len,
    struct vmng_resource_base *res)
{
    if (res->type >= len) {
        return -EINVAL;
    }

    if (res->element.attr & ATTR_VFG_FULL_ALLOC) {
        return alloc_res_from_full_vfg(vf_node, res->type, res_list[res->type]);
    }

    if ((res_list[res->type] == VMNG_SHARE_CORE_NUM)) {
        res_list[res->type] = res->element.num_per_bit * 1;
        return alloc_vf_res_from_vfg(vf_node, res->type, res_list[res->type]);
    }

    if ((res_list[res->type] == 0) && (res->element.attr & ATTR_VFG_NECESSARY) &&
        (vmngd_is_soft_divide(vf_node->pf, res_list[MIA_AC_AIC]))) {
        res_list[res->type] = res->element.num_per_bit * 1;
        return alloc_vf_res_from_vfg(vf_node, res->type, res_list[res->type]);
    }

    return alloc_vf_res_from_remain(vf_node, res->type, res_list[res->type]);
}

int vmngd_alloc_vf_resource(struct vmng_vf_spec_info *spec)
{
    struct vmng_pf_object *pf_node = NULL;
    struct vmng_vf_object *vf_node = NULL;
    struct vmng_resource_base *res = NULL;
    struct list_head *pos = NULL, *n = NULL;

    u32 res_list[MIA_MAX_RES_TYPE] = {0};
    unsigned int dev_id = spec->dev_id;
    unsigned int vfid = spec->vfid;
    int ret;

    pf_node = vmngd_get_pf_object(dev_id);
    if (pf_node == NULL) {
        vmng_err("Get pf node failed. (dev_id=%u;vfid=%u)\n", dev_id, vfid);
        return -EINVAL;
    }
    ret = vmngd_cal_resource(pf_node, spec, res_list, MIA_MAX_RES_TYPE);
    if (ret != 0) {
        vmng_err("Calculate vf resource failed. (dev_id=%u;vfid=%u;ret=%d)\n", dev_id, vfid, ret);
        return ret;
    }

    vf_node = vmngd_alloc_vf_object(pf_node, vfid);
    if (vf_node == NULL) {
        vmng_err("Alloc vf node failed. (dev_id=%u;vfid=%u)\n", dev_id, vfid);
        return -ENOMEM;
    }

    ret = vmngd_assign_vfg(vf_node, spec);
    if (ret != 0) {
        vmng_err("Assign vfg failed.(dev_id=%u;vfid=%u;ret=%d)\n", dev_id, vfid, ret);
        vmngd_free_vf_object(vf_node);
        return ret;
    }
    ret = vmngd_set_vf_base_info(vf_node, spec);
    if (ret != 0) {
        vmng_err("Set vf base info failed.(dev_id=%u;vfid=%u;ret=%d)\n", dev_id, vfid, ret);
        vmngd_free_vf_object(vf_node);
        return ret;
    }

    list_for_each_safe(pos, n, &pf_node->remain_res.resource_list_head) {
        res = list_entry(pos, struct vmng_resource_base, res_node);
        ret = vmngd_alloc_vf_resource_single(vf_node, res_list, MIA_MAX_RES_TYPE, res);
        if (ret != 0) {
            vmng_err("Alloc vf res failed.(dev_id=%u;vfid=%u;type=%d;ret=%d)\n", dev_id, vfid, res->type, ret);
            goto failed;
        }
    }

    return 0;
failed:
    vmngd_recalculate_resource(vf_node);
    vmngd_free_vf_object(vf_node);
    return ret;
}

void vmngd_free_vf_resource(unsigned int dev_id, unsigned int vfid)
{
    struct vmng_vf_object *vf_node = vmngd_get_vf_object(dev_id, vfid);

    if (vf_node == NULL) {
        vmng_err("Vf node is NULL. (dev_id=%u;vfid=%u)\n", dev_id, vfid);
        return;
    }
    vmngd_free_vf_object(vf_node);
}

void vmngd_del_vf_resource_from_vfg(unsigned int dev_id, unsigned int vfid)
{
    struct vmng_vf_object *vf_node = vmngd_get_vf_object(dev_id, vfid);
    if ((vf_node == NULL) || (vf_node->vfg == NULL)) {
        vmng_err("Vf node is NULL. (dev_id=%u;vfid=%u)\n", dev_id, vfid);
        return;
    }
    vmngd_recalculate_resource(vf_node);
}

static inline void vmngd_set_res_bitmap(vmng_resource_list *list, enum soc_mia_res_type type, unsigned long bitmap)
{
    struct vmng_resource_base *res;

    res = vmngd_get_res_unit(list, type);
    if (res == NULL) {
        return;
    }
    vmngd_element_set(&res->element, bitmap);
}

#define BIT_NUM_OF_BYTE 8
int vmngd_set_pg_info_cloud_v2(unsigned int dev_id, vmng_common_pg *pg_info)
{
    struct vmng_pf_object *pf = NULL;
    u64 pg_aiv_bitmap = 0;
    u64 pg_jpegd_bitmap = 0;
    u32 i;

    pf = vmngd_get_pf_object(dev_id);
    if (pf == NULL) {
        vmng_err("Get pf node failed. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    if (pg_info == NULL) {
        vmng_err("Input pg_info is NULL.(dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    for (i = 0; i < sizeof(u32) * BIT_NUM_OF_BYTE; ++i) {
        if (pg_info->aicPara.bitMap & (1ULL << i)) {
            pg_aiv_bitmap |= (0x3UL << (i * 2));    // 0x3 left shift (i * 2) times to get twice the bit
        }
    }
    for (i = 0; i < sizeof(u32) * BIT_NUM_OF_BYTE; ++i) {
        if (pg_info->jpegdPara.bitMap & (1ULL << i)) {
            pg_jpegd_bitmap |= (0x3UL << (i * 2));  // 0x3 left shift (i * 2) times to get twice the bit
        }
    }

    vmngd_set_res_bitmap(&pf->total_res, MIA_AC_AIC, pg_info->aicPara.bitMap);
    vmngd_set_res_bitmap(&pf->total_res, MIA_AC_AIV, pg_aiv_bitmap);
    vmngd_set_res_bitmap(&pf->total_res, MIA_DVPP_VPC, pg_info->vpcPara.bitMap);
    vmngd_set_res_bitmap(&pf->total_res, MIA_DVPP_JPEGD, pg_jpegd_bitmap);
    vmngd_set_res_bitmap(&pf->total_res, MIA_SYS_MEM, pg_info->hbmPara.bitMap);

    vmngd_set_res_bitmap(&pf->remain_res, MIA_AC_AIC, pg_info->aicPara.bitMap);
    vmngd_set_res_bitmap(&pf->remain_res, MIA_AC_AIV, pg_aiv_bitmap);
    vmngd_set_res_bitmap(&pf->remain_res, MIA_DVPP_VPC, pg_info->vpcPara.bitMap);
    vmngd_set_res_bitmap(&pf->remain_res, MIA_DVPP_JPEGD, pg_jpegd_bitmap);
    vmngd_set_res_bitmap(&pf->remain_res, MIA_SYS_MEM, pg_info->hbmPara.bitMap);

    vmng_info("Set pg info success.(dev_id=%u;aic=%#llx;aiv=%#llx;vpc=%#llx;jpegd=%#llx;hbm=%#llx)\n",
              dev_id, pg_info->aicPara.bitMap, pg_aiv_bitmap,
              pg_info->vpcPara.bitMap, pg_jpegd_bitmap, pg_info->hbmPara.bitMap);
    return 0;
}

#define VMNGD_CLOUD_V2_AICPU 0XFC
#define VMNGD_CLOUD_V2_DCPU 0X2
int vmngd_sriov_modify_soc_res_cloud_v2(unsigned int dev_id, enum vmng_pf_sriov_status sriov_status)
{
    struct vmng_pf_object *pf = NULL;

    pf = vmngd_get_pf_object(dev_id);
    if (pf == NULL) {
        vmng_err("Get pf node failed. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }
    if (sriov_status == VMNGH_PF_SRIOV_ENABLE) {
        vmngd_set_res_bitmap(&pf->total_res, MIA_CPU_DEV_ACPU, VMNGD_CLOUD_V2_AICPU | VMNGD_CLOUD_V2_DCPU);
        vmngd_set_res_bitmap(&pf->remain_res, MIA_CPU_DEV_ACPU, VMNGD_CLOUD_V2_AICPU | VMNGD_CLOUD_V2_DCPU);
    } else if (sriov_status == VMNGH_PF_SRIOV_DISABLE) {
        vmngd_set_res_bitmap(&pf->total_res, MIA_CPU_DEV_ACPU, VMNGD_CLOUD_V2_AICPU);
        vmngd_set_res_bitmap(&pf->remain_res, MIA_CPU_DEV_ACPU, VMNGD_CLOUD_V2_AICPU);
    } else {
        vmng_err("Invalid sriov status. (dev_id=%u; sriov_action=%d)\n", dev_id, sriov_status);
        return -EINVAL;
    }
    return 0;
}

unsigned long get_ts_res_bitmap(struct vmng_vf_object *vf, struct vmng_vfg_object *vfg,
                                enum soc_mia_res_type type)
{
    struct vmng_resource_base *type_res = vmngd_get_res_unit(&vf->resource, type);
    if (type_res == NULL) {
        return 0x0UL;
    }
    if ((type_res->element.attr & ATTR_VFG_SHARE) && (vfg != NULL)) {
        return get_vfg_res_bitmap(vfg, type);
    }
    return get_vf_res_bitmap(vf, type);
}

void vmngd_set_ts_cfg(unsigned int dev_id, unsigned int vfid, vmng_vf_cfg_t *cfg)
{
    struct vmng_vf_object *vf = vmngd_get_vf_object(dev_id, vfid);
    struct vmng_vfg_object *vfg = NULL;

    if ((vf == NULL) || (vf->vfg == NULL)) {
        vmng_err("Vf node is NULL. (dev_id=%u;vfid=%u)\n", dev_id, vfid);
        return;
    }
    vfg = vf->vfg;

    cfg->accelerator.c_core_bitmap = (u32)get_ts_res_bitmap(vf, vfg, MIA_AC_C_CORE);
    cfg->accelerator.dsa_bitmap = (u32)get_ts_res_bitmap(vf, vfg, MIA_AC_DSA);
    cfg->accelerator.ffts_bitmap = (u32)get_ts_res_bitmap(vf, vfg, MIA_AC_FFTS);
    cfg->accelerator.sdma_bitmap = (u32)get_ts_res_bitmap(vf, vfg, MIA_AC_SDMA);
    cfg->accelerator.pcie_dma_bitmap = (u32)get_ts_res_bitmap(vf, vfg, MIA_AC_PCIE_DMA);
    cfg->accelerator.acsq_slice_bitmap = (u32)get_ts_res_bitmap(vf, vfg, MIA_STARS_ACSQ);
    cfg->accelerator.rtsq_slice_bitmap = (u32)get_ts_res_bitmap(vf, vfg, MIA_STARS_RTSQ);
    cfg->accelerator.event_slice_bitmap = (u32)get_ts_res_bitmap(vf, vfg, MIA_STARS_EVENT);
    cfg->accelerator.notify_slice_bitmap = (u32)get_ts_res_bitmap(vf, vfg, MIA_STARS_NOTIFY);
    cfg->accelerator.cdq_slice_bitmap = (u32)get_ts_res_bitmap(vf, vfg, MIA_STARS_CDQ);
    cfg->accelerator.cmo_slice_bitmap = (u32)get_ts_res_bitmap(vf, vfg, MIA_STARS_CMO);
    cfg->accelerator.aic_bitmap = (u32)get_ts_res_bitmap(vf, vfg, MIA_AC_AIC);
    cfg->accelerator.aiv_bitmap = (u64)get_ts_res_bitmap(vf, vfg, MIA_AC_AIV);

    cfg->dvpp.jpegd_bitmap = (u32)get_ts_res_bitmap(vf, vfg, MIA_DVPP_JPEGD);
    cfg->dvpp.jpege_bitmap = (u32)get_ts_res_bitmap(vf, vfg, MIA_DVPP_JPEGE);
    cfg->dvpp.vpc_bitmap = (u32)get_ts_res_bitmap(vf, vfg, MIA_DVPP_VPC);
    cfg->dvpp.vdec_bitmap = (u32)get_ts_res_bitmap(vf, vfg, MIA_DVPP_VDEC);
    cfg->dvpp.pngd_bitmap = (u32)get_ts_res_bitmap(vf, vfg, MIA_DVPP_PNGD);
    cfg->dvpp.venc_bitmap = (u32)get_ts_res_bitmap(vf, vfg, MIA_DVPP_VENC);

    cfg->cpu.topic_aicpu_slot_bitmap = (u32)get_ts_res_bitmap(vf, vfg, MIA_STARS_TOPIC_ACPU_SLOT);
    cfg->cpu.topic_ctrl_cpu_slot_bitmap = (u32)get_ts_res_bitmap(vf, vfg, MIA_STARS_TOPIC_CCPU_SLOT);
    cfg->cpu.host_ctrl_cpu_bitmap = (u32)get_ts_res_bitmap(vf, vfg, MIA_CPU_HOST_CCPU);
    cfg->cpu.device_aicpu_bitmap = (u32)get_ts_res_bitmap(vf, vfg, MIA_CPU_DEV_ACPU);
    cfg->cpu.host_aicpu_bitmap = (u64)get_ts_res_bitmap(vf, vfg, MIA_CPU_HOST_ACPU);

    cfg->id.vf_id = (u8)vf->vfid - 1;
    cfg->id.vfg_mode = (u8)vfg->vfg_mode;
    cfg->id.vfg_id = (u8)vfg->vfg_id;
    // mini v3 do not set vip : cfg->id.vip = 0;
    if (dbl_get_rc_ep_mode() == DBL_RC_MODE) {
        cfg->id.vip = 0;
    } else {
        cfg->id.vip = (u8)((vfg->vfg_type == VMNG_VF_GROUP_TYPE_VIP) ? 1 : 0);
    }
    cfg->id.token = vf->token;
    cfg->id.token_max = vf->token_max;
    cfg->id.task_timeout = vf->task_timeout;
}
