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

#ifndef __VIRTMNGDEV_RES_COMMON_H__
#define __VIRTMNGDEV_RES_COMMON_H__

#include "virtmngdev_element.h"
#include "virtmngdev_soc_resource.h"
#include "virtmng_public_def.h"

#define VMNGD_VF_NUM_MIN 1
#define VMNGD_VFG_NUM_HALF 0xf
#define VMNGD_VFG_VIP_BITS 2
#define VMNGD_VFG_NORMAL_BITS 1
#define VMNGD_ID_NO_CHECK 0xff
#define VMNGD_MEMORY_MAX_NUMA_NUM 32

#define VMNGD_CFG_ELE(CFG)            \
    .attr = CFG##_ATTR,               \
    .num_per_bit = CFG##_NUM_PER_BIT, \
    .num = CFG##_NUM,                 \
    .bitnum = 64,                     \
    .bitmap = CFG##_BITMAP

#define VMNGD_CFG_ELE_AC(cx)               \
    .aiv = AC_AIV_DIVIDE_##cx,             \
    .aic = AC_AIC_DIVIDE_##cx,             \
    .c_core = AC_C_CORE_DIVIDE_##cx,       \
    .dsa = AC_DSA_DIVIDE_##cx,             \
    .ffts = AC_FFTS_DIVIDE_##cx,           \
    .sdma = AC_SDMA_DIVIDE_##cx,           \
    .pcie_dma = AC_PCIE_DMA_DIVIDE_##cx,   \
    .acsq = AC_ACSQ_DIVIDE_##cx,           \
    .rtsq = AC_RTSQ_DIVIDE_##cx,           \
    .event_id = AC_EVENT_ID_DIVIDE_##cx,   \
    .notify_id = AC_NOTIFY_ID_DIVIDE_##cx, \
    .cdqm = AC_CDQM_DIVIDE_##cx,           \
    .cmo_id = AC_CMO_ID_DIVIDE_##cx

#define VMNGD_CFG_ELE_DVPP(cx)       \
    .jpegd = DVPP_JPEGD_DIVIDE_##cx, \
    .jpege = DVPP_JPEGE_DIVIDE_##cx, \
    .vpc = DVPP_VPC_DIVIDE_##cx,     \
    .vdec = DVPP_VDEC_DIVIDE_##cx,   \
    .pngd = DVPP_PNGD_DIVIDE_##cx,   \
    .venc = DVPP_VENC_DIVIDE_##cx

#define VMNGD_CFG_ELE_CPU(cx)                          \
    .topic_aicpu_slot = CPU_TOPIC_AICPU_DIVIDE_##cx,   \
    .topic_ctrl_cpu_slot = CPU_TOPIC_CCPU_DIVIDE_##cx, \
    .host_ctrl_cpu = CPU_HOST_CCPU_DIVIDE_##cx,        \
    .device_aicpu = CPU_DEV_AICPU_DIVIDE_##cx,         \
    .host_aicpu = CPU_HOST_AICPU_DIVIDE_##cx

#define VMNGD_CFG_ELE_BASE(cx) \
    .vf = 0,                   \
    .vfg = 0,                  \
    .memory = AC_MEMORY_DIVIDE_##cx

typedef struct vmngd_vf_cfg_info {
    enum vmng_vdev_status status;
    u32 dev_id;
    u32 vfid;
    u32 vfg_mode; /* 0:strict, 1:relax */
    u32 vfg_id;   /* pool_id and vfg_id are the same */
    enum vmng_vf_group_type vfg_type;
    u32 dtype;
    u64 token;
    u64 token_max;
    u64 task_timeout;
} vmngd_vf_cfg_info_t;

typedef struct vmngd_vf_cfg_ac {
    vmngd_element_t aiv;
    vmngd_element_t aic;
    vmngd_element_t c_core;
    vmngd_element_t dsa;
    vmngd_element_t ffts;
    vmngd_element_t sdma;
    vmngd_element_t pcie_dma;
    vmngd_element_t acsq; /* 128/16=8 0x1 means 0-7 */
    vmngd_element_t rtsq;
    vmngd_element_t event_id;
    vmngd_element_t notify_id;
    vmngd_element_t cdqm;
    vmngd_element_t cmo_id;
} vmngd_vf_cfg_ac_t; /* accelerator */

/* note: element must be the same as vmngd_vf_cfg_ac_t */
typedef struct vmngd_vf_element_num_ac {
    u32 aiv;
    u32 aic;
    u32 c_core;
    u32 dsa;
    u32 ffts;
    u32 sdma;
    u32 pcie_dma;
    u32 acsq; /* 128/16=8 0x1 means 0-7 */
    u32 rtsq;
    u32 event_id;
    u32 notify_id;
    u32 cdqm;
    u32 cmo_id;
} vmngd_vf_element_num_ac_t; /* accelerator */

typedef struct vmngd_vf_cfg_dvpp {
    vmngd_element_t jpegd;
    vmngd_element_t jpege;
    vmngd_element_t vpc;
    vmngd_element_t vdec;
    vmngd_element_t pngd;
    vmngd_element_t venc;
} vmngd_vf_cfg_dvpp_t;

/* note: element must be the same as vmngd_vf_cfg_dvpp_t */
typedef struct vmngd_vf_element_num_dvpp {
    u32 jpegd;
    u32 jpege;
    u32 vpc;
    u32 vdec;
    u32 pngd;
    u32 venc;
} vmngd_vf_element_num_dvpp_t;

typedef struct vmngd_vf_cfg_cpu {
    vmngd_element_t topic_aicpu_slot;
    vmngd_element_t topic_ctrl_cpu_slot;
    vmngd_element_t host_ctrl_cpu;
    vmngd_element_t device_aicpu;
    vmngd_element_t host_aicpu;
} vmngd_vf_cfg_cpu_t;

/* note: element must be the same as vmngd_vf_cfg_cpu_t */
typedef struct vmngd_vf_element_num_cpu {
    u32 topic_aicpu_slot;
    u32 topic_ctrl_cpu_slot;
    u32 host_ctrl_cpu;
    u32 device_aicpu;
    u32 host_aicpu;
} vmngd_vf_element_num_cpu_t;

typedef struct vmngd_vf_cfg_base {
    vmngd_element_t vf;
    vmngd_element_t vfg;
    vmngd_element_t memory;
    int numa_id[VMNGD_MEMORY_MAX_NUMA_NUM];
} vmngd_vf_cfg_base_t;

typedef struct vmngd_vf_element_num_base {
    u32 vf;
    u32 vfg;
    u32 memory;
} vmngd_vf_element_num_base_t;

typedef struct vmngd_vfg_cfg_info {
    u32 status;
    u32 vfg_id;
    u32 vfg_type;
    unsigned long vf_bitmap;       // the bit whcih is 1 means that vf in this vfg
    vmngd_vf_cfg_ac_t accelerator; // accelerator resource of vfg
    vmngd_vf_cfg_dvpp_t dvpp;      // dvpp resource of vfg
    vmngd_vf_cfg_cpu_t cpu;        // cpu resource of vfg
    struct list_head vf_list_head; // vf list in this vfg
} vmngd_vfg_cfg_info_t;

typedef struct vmngd_vf_resource_unit {
    char template_name[VMNG_VF_TEMP_NAME_LEN];
    vmngd_vf_cfg_info_t cfg_info;
    vmngd_vf_cfg_base_t base;
    vmngd_vf_cfg_ac_t accelerator;
    vmngd_vf_cfg_dvpp_t dvpp;
    vmngd_vf_cfg_cpu_t cpu;
    vmngd_vfg_cfg_info_t *vfg; // the vfg that vf belongs to
    struct list_head vf_node;
} vmngd_resource_unit_t;

typedef struct vmngd_vf_element_num {
    vmngd_vf_element_num_base_t base;
    vmngd_vf_element_num_ac_t accelerator;
    vmngd_vf_element_num_dvpp_t dvpp;
    vmngd_vf_element_num_cpu_t cpu;
} vmngd_vf_element_num_t;

typedef struct vmngd_resource_info {
    u32 status;
    struct mutex lock;
    u32 vf_num;
    enum vmng_split_mode split_mode;
    vmngd_resource_unit_t vf_cfg_total;            /* total vf resource */
    vmngd_resource_unit_t vf_cfg_remain;            /* remain vf resource */
    vmngd_resource_unit_t vf_cfg_each[BASE_VF_NUM]; /* each vf resource */
    vmngd_vfg_cfg_info_t vfg_cfg_each[BASE_VFG_NUM];   /* each vfg info */
} vmngd_resource_info_t;

struct vmngd_resource_ops {
    int (*res_init)(u32 dev_id);
    void (*res_uninit)(u32 dev_id);
    int (*alloc_vfid)(u32 dev_id, u32 dtype, u32 *fid);
    void (*free_vfid)(u32 dev_id, u32 vfid);
    int (*alloc_vf)(struct vmng_ctrl_msg_info *info);
    int (*free_vf)(u32 dev_id, u32 vfid);
    int (*reset_vf)(u32 dev_id, u32 vfid);
    int (*refresh_vf)(struct vmng_ctrl_msg_info *info);
    int (*bind_vf_to_vfg)(u32 dev_id, u32 vfid);
    void (*unbind_vf_to_vfg)(u32 dev_id, u32 vfid);
    void (*update_vdev_ctrl)(u32 dev_id, u32 vfid, struct vmng_vdev_ctrl *ctrl);
    int (*res_enquire)(u32 dev_id, u32 vfid, struct vmng_soc_resource_enquire *info);
    int (*res_enquire_vfg)(u32 dev_id, u32 vfid, struct vmng_soc_res_info *vfg_info);
    int (*pci_client_online)(u32 dev_id, u32 vfid, struct vmng_vdev_ctrl *ctrl);
    int (*pci_client_offline)(u32 dev_id, u32 vfid, struct vmng_vdev_ctrl *ctrl);
    bool (*is_vf)(u32 dev_id);
    int (*unify_devid_to_pfvf_id)(u32 dev_id, u32 *pf_id, u32 *vf_id);
};

int vmngd_bitmap_check(unsigned long bitmap1, unsigned long bitmap2, u32 nbits);
int vmngd_resource_info_check(u32 dev_id, u32 vfid, u32 dtype);
void vmngd_resource_enquire_stars_proc(struct vmng_stars_res_refresh *refresh, struct vmng_stars_res_static *fixed,
    const vmngd_resource_unit_t *unit);
void vmngd_resource_upate_vf_status(u32 dev_id, u32 vfid, u32 status);
vmngd_resource_info_t *vmngd_get_resource_info(u32 dev_id);
vmngd_resource_unit_t *vmngd_get_resource_total(void);
int vmngd_alloc_resource_info(u32 dev_id);
void vmngd_free_resource_info(u32 dev_id);
int vmngd_alloc_vfid_dynamic(u32 dev_id, u32 *fid);
int vmngd_alloc_vfid_static(u32 dev_id, u32 fid);
void vmng_set_device_split_mode(u32 dev_id, enum vmng_split_mode split_mode);
u32 vmngd_get_dev_chip_type(u32 dev_id);
bool vmngd_is_sriov_support(u32 dev_id);
int vmngd_sriov_modify_soc_res(unsigned int dev_id, enum vmng_pf_sriov_status sriov_status);

#endif
