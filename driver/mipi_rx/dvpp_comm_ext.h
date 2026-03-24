/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
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
 * Create: 2020-4-1
 */

#ifndef DVPP_COMM_EXT_H__
#define DVPP_COMM_EXT_H__

#include "hi_type.h"
#include "hi_defines.h"
#include "hi_comm_sys.h"
#include "mod_ext.h"

#define IRQ_NAME_MAX_LEN 64U
#define MAX_NR_SMMUS_PER_DEVICE 8U // 2IDE 8个
#define MAX_NR_SMMUS (MAX_NR_SMMUS_PER_DEVICE * DEVICE_NUM)
#define MAX_MODULE_NUM 64

#define ckfn_dvpp_comm_entry() CHECK_FUNC_ENTRY(HI_ID_DVPP_COMM)

#define DVPP_USEC_PER_MSEC (1000LLU)

#ifdef DVPP_UTST
#define DVPP_STATIC
#else
#define DVPP_STATIC static
#endif

struct single_smmu_res {
    hi_u32 smmu_id;
    hi_u32 sid;
    hi_u32 ssid;
    hi_void *sva;
};

typedef struct smmu_resource {
    struct single_smmu_res sg_smmu_res[MAX_NR_SMMUS];
    hi_u32 smmu_num;
} smmu_resource;

struct dvpp_iova_addr {
    hi_u64 iova_addr;
    hi_u64 phy_addr;
    hi_u32 len;
};

enum MODULE_TYPE {
    DVPP_SMMU = 0,
    DVPP_DISPATCH,
    DVPP_SUBCTRL,
    DVPP_SCHE,
    DVPP_JPEGD,
    DVPP_VPC,
    DVPP_VPC_ERR,
    DVPP_JPEGE,
    DVPP_VDEC,
    DVPP_VENC,
    DVPP_CHIP_INFO,
    DVPP_AA,
    DVPP_NSC,
    DVPP_NSC_ERR,
    HI_DVPP_BUTT
};

struct dvpp_module_info {
    enum MODULE_TYPE module_type;
    hi_u32 irq_id[2]; // 0 is for norm irq, 1 is for err irq
    hi_ulong hwirq[2];
    hi_u64 mbg_base; // 设备对应的mbigen基地址
    hi_u64 mbg_size; // mbigen寄存器空间
    hi_u64 mbg_irq[2]; // 设备中断在mbigen的硬中断号, 用于小核清理中断
    hi_u32 smmu_id; // 对应的smmu id
    hi_u32 chip_id; // 多P场景下，每个P对应一个chip
    hi_u32 aa_id; // aa 全局编号, 用于四件套上报
    hi_u32 dispatch_id; // dispatch 全局编号, 用于四件套上报
    hi_u32 subsys_id; // chip内subsys id
    hi_u32 scheduler_id; // subsys内对应的sche序号
    hi_u32 idx_in_subsys; // subsys内对应的序号
    hi_u32 idx_in_scheduler; // schedule内对应的钳位序号
    hi_u32 idx_in_port; // 同一schedule内有多个vpu(如jpegd)情况下schedule内的序号
    hi_void *subctrl_reg_addr;  // subsys控制寄存器基址
    hi_void *scheduler_reg_addr; // sche控制寄存器基址
    hi_void *aa_reg_addr; // aa控制寄存器基址
    hi_void *dispatch_reg_addr; // dispatch控制寄存器基址
    hi_void *reg_addr[3]; // 各vpu私有寄存器表，sys和各vpu驱动自行约束表元素的意义
    hi_u64 reg_addr_phy[3]; // 寄存器物理地址
    hi_u64 reg_size_phy[3]; // 寄存器物理地址大小
    hi_void* irq_cpumask; // 中断CPU mask
    hi_u32 global_id_full; // vpu 对应PG前的全局平铺id索引
};

struct dvpp_subsys_info {
    struct dvpp_module_info module_info[MAX_MODULE_NUM];
    hi_u32 module_num;
};
struct dvpp_stl_info {
    hi_u64 mem_addr_phy;
    hi_u32 mem_size;
    hi_u32 cmd_size;
    hi_u32 interval;
};
struct dvpp_thread_cpu_info {
    hi_void* thread_cpumask[DEVICE_NUM]; // 线程CPU mask
};

struct dvpp_timer_cpu_info {
    hi_u32 timer_index;
};

struct dvpp_stars_prof_info {
    hi_u64 stars_busy_cnt_addr[DEVICE_NUM];
    hi_u64 stars_virt_reg_addr[DEVICE_NUM];
    hi_u64 jpegd_task_cnt_addr[DEVICE_NUM];
    hi_u64 jpege_task_cnt_addr[DEVICE_NUM];
    hi_u64 vpc_task_cnt_addr[DEVICE_NUM];
};

enum STARS_TYPE {
    STARS_BUSY_CNT,
    STARS_VIRT_REG,
    STARS_TASK_JPEGD,
    STARS_TASK_JPEGE,
    STARS_TASK_VPC,
    STARS_BUTT
};

struct dvpp_device_info {
    struct dvpp_subsys_info jpegd_module_info;
    struct dvpp_subsys_info jpege_module_info;
    struct dvpp_subsys_info vpc_module_info;
    struct dvpp_subsys_info nsc_module_info;
    struct dvpp_subsys_info vdec_module_info;
    struct dvpp_subsys_info venc_module_info;
    struct dvpp_subsys_info subctl_module_info;
    struct dvpp_subsys_info sche_module_info;
    struct dvpp_subsys_info smmu_module_info;
    struct dvpp_subsys_info aa_module_info;
    struct dvpp_subsys_info disp_module_info;
    struct dvpp_stl_info vpc_stl_info;
    struct dvpp_thread_cpu_info vdec_thread_cpu_info;
    struct dvpp_thread_cpu_info venc_thread_cpu_info;
    struct dvpp_thread_cpu_info vpc_thread_cpu_info;
    struct dvpp_timer_cpu_info vdec_timer_cpu_info;
    struct dvpp_timer_cpu_info venc_timer_cpu_info;
    struct dvpp_timer_cpu_info vpc_timer_cpu_info;
    struct dvpp_stars_prof_info stars_prof_info;
};

typedef hi_s32 fn_dvpp_comm_smmu_bind(struct smmu_resource *resouce, hi_s32 dev_id, struct dvpp_iova_addr *kiova);
typedef hi_s32 fn_dvpp_comm_smmu_unbind(struct smmu_resource *resouce, struct dvpp_iova_addr *kiova);
typedef struct dvpp_device_info *fn_dvpp_comm_get_device_info(hi_void);
typedef hi_u32 fn_dvpp_comm_get_smmu_num(hi_void);
typedef struct iommu_domain *fn_dvpp_comm_get_smmu_domain(hi_u32 smmu_idx);
typedef hi_u64 fn_dvpp_comm_get_virtual_time(hi_void);
typedef hi_s32 fn_jpege_hal_subctrl_power_up(struct dvpp_module_info* jpege_info, hi_u32 jpege_id);
typedef hi_s32 fn_jpege_hal_subctrl_power_down(struct dvpp_module_info* jpege_info, hi_u32 jpege_id);
typedef hi_s32 fn_jpegd_hal_subctrl_power_up(hi_u32 jpegd_id);
typedef hi_s32 fn_jpegd_hal_subctrl_power_down(hi_u32 jpegd_id);
typedef hi_u32 fn_dvpp_comm_get_stars_core_utilization(enum MODULE_TYPE type, hi_u32 device_id, hi_u32 vf_id);
typedef hi_void fn_dvpp_comm_export_func_lock(hi_void);
typedef hi_void fn_dvpp_comm_export_func_unlock(hi_void);
typedef hi_void fn_dvpp_qos_spin_lock(hi_ulong *flags);
typedef hi_void fn_dvpp_qos_spin_unlock(hi_ulong *flags);
typedef hi_s32 fn_dvpp_comm_calc_dev_id_to_pf_vf_id(hi_u32 ext_phy_dev_id,
    hi_u32 ext_vf_id, hi_u32 *pf_id, hi_u32 *vf_id);
typedef struct {
    fn_dvpp_comm_smmu_bind *pfn_dvpp_comm_smmu_bind;
    fn_dvpp_comm_get_device_info *pfn_dvpp_comm_get_device_info;
    fn_dvpp_comm_smmu_unbind *pfn_dvpp_comm_smmu_unbind;
    fn_dvpp_comm_get_smmu_num *pfn_dvpp_comm_get_smmu_num;
    fn_dvpp_comm_get_smmu_domain *pfn_dvpp_comm_get_smmu_domain;
    fn_dvpp_comm_get_virtual_time *pfn_dvpp_comm_get_virtual_time;
    fn_jpege_hal_subctrl_power_up *pfn_jpege_hal_subctrl_power_up;
    fn_jpege_hal_subctrl_power_down *pfn_jpege_hal_subctrl_power_down;
    fn_jpegd_hal_subctrl_power_up *pfn_jpegd_hal_subctrl_power_up;
    fn_jpegd_hal_subctrl_power_down *pfn_jpegd_hal_subctrl_power_down;
    fn_dvpp_comm_get_stars_core_utilization *pfn_dvpp_comm_get_stars_core_utilization;
    fn_dvpp_comm_export_func_lock *pfn_dvpp_comm_export_func_lock;
    fn_dvpp_comm_export_func_unlock *pfn_dvpp_comm_export_func_unlock;
    fn_dvpp_qos_spin_lock *pfn_dvpp_qos_spin_lock;
    fn_dvpp_qos_spin_unlock *pfn_dvpp_qos_spin_unlock;
    fn_dvpp_comm_calc_dev_id_to_pf_vf_id *pfn_dvpp_comm_calc_dev_id_to_pf_vf_id;
} dvpp_comm_export_func;

#define call_sys_qos_spin_lock(flags) \
        (FUNC_ENTRY(dvpp_comm_export_func, HI_ID_DVPP_COMM)->pfn_dvpp_qos_spin_lock(flags))

#define call_sys_qos_spin_unlock(flags) \
        (FUNC_ENTRY(dvpp_comm_export_func, HI_ID_DVPP_COMM)->pfn_dvpp_qos_spin_unlock(flags))

#define ckfn_dvpp_comm_smmu_bind() \
    (ckfn_dvpp_comm_entry() && (FUNC_ENTRY(dvpp_comm_export_func, HI_ID_DVPP_COMM)->pfn_dvpp_comm_smmu_bind != NULL))
#define call_dvpp_comm_smmu_bind(resource, dev_id, iova) \
    FUNC_ENTRY(dvpp_comm_export_func, HI_ID_DVPP_COMM)->pfn_dvpp_comm_smmu_bind((resource), (dev_id), (iova))
#define ckfn_dvpp_comm_smmu_unbind() \
    (ckfn_dvpp_comm_entry() && (FUNC_ENTRY(dvpp_comm_export_func, HI_ID_DVPP_COMM)->pfn_dvpp_comm_smmu_unbind != NULL))
#define call_dvpp_comm_smmu_unbind(resource, iova) \
    FUNC_ENTRY(dvpp_comm_export_func, HI_ID_DVPP_COMM)->pfn_dvpp_comm_smmu_unbind((resource), (iova))

#define ckfn_dvpp_comm_get_device_info() \
    (ckfn_dvpp_comm_entry() && \
    (FUNC_ENTRY(dvpp_comm_export_func, HI_ID_DVPP_COMM)->pfn_dvpp_comm_get_device_info != NULL))
#define call_dvpp_comm_get_device_info() \
    (FUNC_ENTRY(dvpp_comm_export_func, HI_ID_DVPP_COMM)->pfn_dvpp_comm_get_device_info())

#define ckfn_jpege_hal_subctrl_power_up() \
    (ckfn_dvpp_comm_entry() && \
    (FUNC_ENTRY(dvpp_comm_export_func, HI_ID_DVPP_COMM)->pfn_jpege_hal_subctrl_power_up != NULL))
#define call_jpege_hal_subctrl_power_up(jpege_info, jpege_id) \
    (FUNC_ENTRY(dvpp_comm_export_func, HI_ID_DVPP_COMM)->pfn_jpege_hal_subctrl_power_up(jpege_info, jpege_id))
#define ckfn_jpege_hal_subctrl_power_down() \
    (ckfn_dvpp_comm_entry() && \
    (FUNC_ENTRY(dvpp_comm_export_func, HI_ID_DVPP_COMM)->pfn_jpege_hal_subctrl_power_down != NULL))
#define call_jpege_hal_subctrl_power_down(jpege_info, jpege_id) \
    (FUNC_ENTRY(dvpp_comm_export_func, HI_ID_DVPP_COMM)->pfn_jpege_hal_subctrl_power_down(jpege_info, jpege_id))
#define ckfn_jpegd_hal_subctrl_power_up() \
    (ckfn_dvpp_comm_entry() && \
    (FUNC_ENTRY(dvpp_comm_export_func, HI_ID_DVPP_COMM)->pfn_jpegd_hal_subctrl_power_up != NULL))
#define call_jpegd_hal_subctrl_power_up(jpegd_id) \
    (FUNC_ENTRY(dvpp_comm_export_func, HI_ID_DVPP_COMM)->pfn_jpegd_hal_subctrl_power_up(jpegd_id))
#define ckfn_jpegd_hal_subctrl_power_down() \
    (ckfn_dvpp_comm_entry() && \
    (FUNC_ENTRY(dvpp_comm_export_func, HI_ID_DVPP_COMM)->pfn_jpegd_hal_subctrl_power_down != NULL))
#define call_jpegd_hal_subctrl_power_down(jpegd_id) \
    (FUNC_ENTRY(dvpp_comm_export_func, HI_ID_DVPP_COMM)->pfn_jpegd_hal_subctrl_power_down(jpegd_id))
#define call_dvpp_comm_export_func_lock() \
    (FUNC_ENTRY(dvpp_comm_export_func, HI_ID_DVPP_COMM)->pfn_dvpp_comm_export_func_lock())
#define call_dvpp_comm_export_func_unlock() \
    (FUNC_ENTRY(dvpp_comm_export_func, HI_ID_DVPP_COMM)->pfn_dvpp_comm_export_func_unlock())

#define ckfn_dvpp_comm_get_stars_core_utilization() \
    (ckfn_dvpp_comm_entry() && \
    (FUNC_ENTRY(dvpp_comm_export_func, HI_ID_DVPP_COMM)->pfn_dvpp_comm_get_stars_core_utilization != NULL))
#define call_dvpp_comm_get_stars_core_utilization(type, device_id, vf_id) \
    (FUNC_ENTRY(dvpp_comm_export_func, HI_ID_DVPP_COMM)->pfn_dvpp_comm_get_stars_core_utilization((type), \
    (device_id), (vf_id)))

__inline static hi_bool ckfn_dvpp_comm_get_smmu_num(hi_void)
{
    if (ckfn_dvpp_comm_entry() &&
        (FUNC_ENTRY(dvpp_comm_export_func, HI_ID_DVPP_COMM)->pfn_dvpp_comm_get_smmu_num != NULL)) {
        return HI_TRUE;
    }
    return HI_FALSE;
}

__inline static hi_u32 call_dvpp_comm_get_smmu_num(hi_void)
{
    return FUNC_ENTRY(dvpp_comm_export_func, HI_ID_DVPP_COMM)->pfn_dvpp_comm_get_smmu_num();
}

__inline static hi_bool ckfn_dvpp_comm_get_smmu_domain(hi_void)
{
    if (ckfn_dvpp_comm_entry() &&
        (FUNC_ENTRY(dvpp_comm_export_func, HI_ID_DVPP_COMM)->pfn_dvpp_comm_get_smmu_domain != NULL)) {
        return HI_TRUE;
    }
    return HI_FALSE;
}

__inline static struct iommu_domain *call_dvpp_comm_get_smmu_domain(hi_u32 smmu_idx)
{
    return FUNC_ENTRY(dvpp_comm_export_func, HI_ID_DVPP_COMM)->pfn_dvpp_comm_get_smmu_domain(smmu_idx);
}

__inline static hi_bool ckfn_dvpp_comm_get_virtual_time(hi_void)
{
    if (ckfn_dvpp_comm_entry() &&
        (FUNC_ENTRY(dvpp_comm_export_func, HI_ID_DVPP_COMM)->pfn_dvpp_comm_get_virtual_time != NULL)) {
        return HI_TRUE;
    }
    return HI_FALSE;
}

__inline static hi_u64 call_dvpp_comm_get_virtual_time(hi_void)
{
    return FUNC_ENTRY(dvpp_comm_export_func, HI_ID_DVPP_COMM)->pfn_dvpp_comm_get_virtual_time();
}

__inline static hi_bool ckfn_dvpp_comm_calc_dev_id_to_pf_vf_id(hi_void)
{
    if (ckfn_dvpp_comm_entry() &&
        (FUNC_ENTRY(dvpp_comm_export_func, HI_ID_DVPP_COMM)->pfn_dvpp_comm_calc_dev_id_to_pf_vf_id != NULL)) {
        return HI_TRUE;
    }
    return HI_FALSE;
}

__inline static hi_u64 call_dvpp_comm_calc_dev_id_to_pf_vf_id(hi_u32 ext_phy_dev_id, hi_u32 ext_vf_id,
    hi_u32 *pf_id, hi_u32 *vf_id)
{
    return FUNC_ENTRY(dvpp_comm_export_func, HI_ID_DVPP_COMM)->pfn_dvpp_comm_calc_dev_id_to_pf_vf_id(ext_phy_dev_id,
        ext_vf_id, pf_id, vf_id);
}


hi_u64 get_virtual_time_by_msec(hi_void);

hi_void dvpp_comm_set_device_info(struct dvpp_device_info *info);
struct dvpp_device_info *dvpp_comm_get_device_info(void);

hi_void dvpp_comm_export_func_lock(hi_void);
hi_void dvpp_comm_export_func_unlock(hi_void);
hi_s32 dvpp_comm_init_export_func_lock(hi_void);
hi_void dvpp_comm_destroy_export_func_lock(hi_void);
hi_void device_set_vpc_in_aoscore(hi_bool vpc_in_aoscore);
hi_bool device_get_vpc_in_aoscore(hi_void);
#endif // DVPP_COMM_EXT_H__
