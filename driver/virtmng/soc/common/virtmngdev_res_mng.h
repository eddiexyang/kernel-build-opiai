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

#ifndef _VIRTMNGDEV_RES_MNG_H
#define _VIRTMNGDEV_RES_MNG_H

#include "soc_res.h"

#include "virtmngdev_element.h"
#include "virtmng_public_def.h"
#include "virtmng_interface.h"

#define VMNG_HALF_CORE_NUM 0x1F4    // 500 --> 0.5
#define VMNG_QUARTER_CORE_NUM 0xFA  // 250 --> 0.25
#define VMNG_SHARE_CORE_NUM 0xFFFF  // (u16)(-1) means share a computing core

#define ATTR_NUM_RESOURCE (1U << 1)
#define ATTR_BITMAP_RESOURCE (1U << 2)
#define ATTR_VFG_SHARE (1U << 3)
#define ATTR_VFG_NECESSARY (1U << 4)
#define ATTR_VFG_FULL_ALLOC (1U << 5)   // alloc all resource in vfg, for 1910B
#define ATTR_RATIO_ALLOC (1U << 6)      // 比例切分
#define ATTR_EQUAL_ALLOC (1U << 7)      // 等分切分
#define ATTR_CUSTOM_ALLOC (1U << 8)     // 自定义切分

struct vmng_resource {
    enum soc_mia_res_type type;
    vmngd_element_t element;
};

struct vmng_resource_base {
    enum soc_mia_res_type type;
    vmngd_element_t element;
    struct list_head res_node;
    int (*alloc_resource)(struct vmng_resource_base *self, struct vmng_resource_base *from, const u32 alloc_num);
    void (*free_resource)(struct vmng_resource_base *self);
    int (*set_resource)(struct vmng_resource_base *self, vmngd_element_t *element);
    int (*get_num)(struct vmng_resource_base *self);
    unsigned long (*get_bitmap)(struct vmng_resource_base *self);
};

typedef struct _vmng_resource_list_t {
    struct list_head resource_list_head;
    struct vmng_resource_base *res_list[MIA_MAX_RES_TYPE];
} vmng_resource_list;

struct vmng_pf_object {
    struct device *dev;
    u32 dev_id;
    u32 chip_type;
    u32 vf_num;
    u32 vf_online_num;      // online vf num of pf
    u32 vfg_num;
    vmng_resource_list total_res;
    vmng_resource_list remain_res;
    struct list_head vfg_list_head;
    struct list_head vf_list_head;
    struct list_head pf_node;
};

struct vmng_vfg_object {
    u32 dev_id;
    u32 vfg_id;
    u32 vfg_mode;
    u32 vf_num;
    enum vmng_vf_group_type vfg_type;
    vmng_resource_list resource;
    struct vmng_pf_object *pf;
    struct list_head vf_list_head;
    struct list_head pf_vfg_node;
};

struct vmng_vf_object {
    char template_name[VMNG_VF_TEMP_NAME_LEN];
    struct device *dev;
    u32 dev_id;
    u32 vfid;
    u32 udevid;
    u64 token;
    u64 token_max;
    u64 task_timeout;
    u32 status;
    u32 dtype;
    u32 vm_full_spec_enable;
    vmng_resource_list resource;
    struct vmng_vfg_object *vfg;
    struct vmng_pf_object *pf;
    struct list_head vfg_vf_node;
    struct list_head pf_vf_node;
};

struct vmng_vf_spec_info {
    char template_name[VMNG_VF_TEMP_NAME_LEN];
    u32 dev_id;
    u32 vfid;
    u32 dtype;
    u32 vfg_id;
    u32 vfg_type;
    u32 vfg_mode;
    u64 token;
    u64 token_max;
    u64 task_timeout;
    u32 vm_full_spec_enable;
    u32 res_list[MIA_MAX_RES_TYPE];
};

struct vmng_pf_object *vmngd_get_pf_object(unsigned int dev_id);
struct vmng_vf_object *vmngd_get_vf_object(unsigned int dev_id, unsigned int vfid);
struct vmng_vfg_object *vmngd_get_vfg_object(struct vmng_pf_object *pf, unsigned int vfg_id);

struct vmng_resource_base *vmngd_get_res_unit(vmng_resource_list *list, enum soc_mia_res_type type);
int get_vf_res_num(struct vmng_vf_object *vf_obj, enum soc_mia_res_type type);
unsigned long get_vf_res_bitmap(struct vmng_vf_object *vf_obj, enum soc_mia_res_type type);
int get_total_res_num(struct vmng_pf_object *pf_obj, enum soc_mia_res_type type);
unsigned long get_total_res_bitmap(struct vmng_pf_object *pf_obj, enum soc_mia_res_type type);

int vmngd_resource_register(u32 dev_id, struct vmng_resource *resource);
void vmngd_resource_unregister(u32 dev_id, struct vmng_resource *resource);
int vmngd_init_pf_instance(unsigned int dev_id);
void vmngd_uninit_pf_instance(unsigned int dev_id);
int vmngd_init_vfg_instance(unsigned int dev_id, unsigned int vfg_id);
void vmngd_uninit_vfg_instance(unsigned int dev_id, unsigned int vfg_id);

int vmngd_alloc_vf_resource(struct vmng_vf_spec_info *spec);
void vmngd_del_vf_resource_from_vfg(unsigned int dev_id, unsigned int vfid);
void vmngd_free_vf_resource(unsigned int dev_id, unsigned int vfid);

// ts
unsigned long get_ts_res_bitmap(struct vmng_vf_object *vf, struct vmng_vfg_object *vfg,
                                enum soc_mia_res_type type);
void vmngd_set_ts_cfg(unsigned int dev_id, unsigned int vfid, vmng_vf_cfg_t *cfg);
int vmngd_set_pg_info_cloud_v2(unsigned int dev_id, vmng_common_pg *pg_info);
int vmngd_sriov_modify_soc_res_cloud_v2(unsigned int dev_id, enum vmng_pf_sriov_status sriov_status);

#endif // _VIRTMNGDEV_RES_MNG_H
