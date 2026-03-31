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
#include "devdrv_interface.h"
#include "virtmngdev_ts.h"
#include "virtmngdev_memory.h"
#include "virtmngdev_res_mng.h"
#include "virtmngdev_resource_register.h"
#include "virtmngdev_client_online.h"
#include "virtmngdev_mia_cfg.h"
#include "virtmngdev_resource.h"

#ifdef CFG_SOC_PLATFORM_MINIV3
STATIC void vmngd_adapt_old_template(struct vmng_vf_spec_info *spec, struct vmng_ctrl_msg_info *info)
{
    if (spec->dtype == VMNG_HW_TYPE_C1_4) {
        spec->res_list[MIA_DVPP_VDEC] = DVPP_VDEC_DIVIDE_1_4;
        spec->res_list[MIA_DVPP_VENC] = DVPP_VENC_DIVIDE_1_4;
        return;
    } else if (spec->dtype == VMNG_HW_TYPE_C2_4) {
        spec->res_list[MIA_DVPP_VDEC] = DVPP_VDEC_DIVIDE_2_4;
        spec->res_list[MIA_DVPP_VENC] = DVPP_VENC_DIVIDE_2_4;
        return;
    }
}
#endif // CFG_SOC_PLATFORM_MINIV3

STATIC int vmngd_adapt_vf_info(struct vmng_vf_spec_info *spec, struct vmng_ctrl_msg_info *info)
{
    int ret;
    // input vf template info
    ret = strcpy_s(spec->template_name, VMNG_VF_TEMP_NAME_LEN, info->vf_cfg.name);
    if (ret != 0) {
        vmng_err("Copy template name failed. (dev_id=%u;vfid=%u;ret=%d)\n", info->dev_id, info->vfid, ret);
        return ret;
    }
    spec->res_list[MIA_AC_AIC] = info->vf_cfg.stars_static.aic;
    spec->res_list[MIA_CPU_DEV_ACPU] = info->vf_cfg.stars_refresh.device_aicpu;
    spec->res_list[MIA_SYS_MEM] = info->vf_cfg.memory.size;
    spec->res_list[MIA_MEM_NUMA] = info->vf_cfg.memory.size;
    spec->res_list[MIA_DVPP_VPC] = info->vf_cfg.stars_refresh.vpc;
    spec->res_list[MIA_DVPP_JPEGD] = info->vf_cfg.stars_refresh.jpegd;
    spec->res_list[MIA_DVPP_JPEGE] = info->vf_cfg.stars_refresh.jpege;
    spec->res_list[MIA_DVPP_VDEC] = info->vf_cfg.stars_refresh.vdec;
    spec->res_list[MIA_DVPP_VENC] = info->vf_cfg.stars_refresh.venc;

    // vf base info
    spec->dev_id = info->dev_id;
    spec->vfid = info->vfid;
    spec->token = info->vf_cfg.vfg.vfg_refresh.token;
    spec->token_max = info->vf_cfg.vfg.vfg_refresh.token_max;
    spec->task_timeout = info->vf_cfg.vfg.vfg_refresh.task_timeout;

    // input vfg info
    spec->vfg_id = info->vf_cfg.vfg.vfg_id;
    spec->vfg_type = info->vf_cfg.vfg.vfg_type;
    spec->vfg_mode = info->vf_cfg.vfg.vfg_refresh.vfg_mode;
    spec->dtype = info->dtype;

#ifdef CFG_SOC_PLATFORM_CLOUD_V2
    // adapt for old template
    spec->token = TOKEN_VAL;
    spec->token_max = TOKEN_MAX;
    spec->task_timeout = TIMEOUT_TOKEN_VAL;
    spec->vm_full_spec_enable = info->vf_cfg.vm_full_spec_enable;
#endif // CFG_SOC_PLATFORM_CLOUD_V2
#ifdef CFG_SOC_PLATFORM_MINIV3
    vmngd_adapt_old_template(spec, info);
#endif // CFG_SOC_PLATFORM_MINIV3

    return 0;
}

STATIC int vmngd_cfg_numa_info(unsigned int dev_id, unsigned int vfid)
{
    struct vmng_vf_object *vf_node = NULL;
    struct vmng_resource_base *numa_res;
    vmngd_element_t *numa_ele;
    u32 numa_id[DEVMM_MAX_NUMA_NUM_OF_PER_DEV];
    u32 numa_num = 0;
    int ret;

    ret = vmngd_resource_numa_id_init(dev_id, numa_id, DEVMM_MAX_NUMA_NUM_OF_PER_DEV, &numa_num);
    if (ret != 0) {
        vmng_err("Numa id init failed.(dev_id=%u;vfid=%u;ret=%d)\n", dev_id, vfid, ret);
        return ret;
    }

    vf_node = vmngd_get_vf_object(dev_id, vfid);
    if (vf_node == NULL) {
        vmng_err("Vf node is NULL. (dev_id=%u;vfid=%u)\n", dev_id, vfid);
        return -EINVAL;
    }
    numa_res = vmngd_get_res_unit(&vf_node->resource, MIA_MEM_NUMA);
    if (numa_res != NULL) {
        numa_ele = &(numa_res->element);
        ret = vmngd_resource_memory_init(dev_id, vfid, numa_id, numa_num, numa_ele);
        if (ret != 0) {
            return ret;
        }
    }
    return 0;
}

STATIC void vmngd_uncfg_numa_info(unsigned int dev_id, unsigned int vfid)
{
    (void)vmngd_resource_memory_uninit(dev_id, vfid);
}

STATIC int vmngd_resource_alloc_adapter(struct vmng_ctrl_msg_info *info)
{
    struct vmng_vf_spec_info spec = {0};
    vmng_vf_cfg_t ts_cfg = {0};
    int ret;

    ret = vmngd_adapt_vf_info(&spec, info);
    if (ret != 0) {
        vmng_err("Adapt vf base info failed. (dev_id=%u;vfid=%u;ret=%d)\n", info->dev_id, info->vfid, ret);
        return ret;
    }
    ret = vmngd_alloc_vf_resource(&spec);
    if (ret != 0) {
        vmng_err("Alloc vf resource failed. (dev_id=%u;vfid=%u;ret=%d)\n", info->dev_id, info->vfid, ret);
        return ret;
    }
    info->vf_cfg.vfg.vfg_id = spec.vfg_id;
    if (vmngd_get_dev_chip_type(info->dev_id) != HISI_CLOUD_V2) {
        return 0;
    }
    // set numa
    ret = vmngd_cfg_numa_info(info->dev_id, info->vfid);
    if (ret != 0) {
        vmng_err("Config numa resource failed. (dev_id=%u;vfid=%u;ret=%d)\n", info->dev_id, info->vfid, ret);
        vmngd_del_vf_resource_from_vfg(info->dev_id, info->vfid);
        vmngd_free_vf_resource(info->dev_id, info->vfid);
        return ret;
    }
    // send to ts
    vmngd_set_ts_cfg(info->dev_id, info->vfid, &ts_cfg);
    vmngd_print_ts_cfg(&ts_cfg);
    ret = vmngd_send_msg_to_ts_create(info->dev_id, info->vfid, &ts_cfg);
    if (ret != 0) {
        vmng_err("Send create msg to ts failed.(dev_id=%u;vfid=%u;ret=%d)\n", info->dev_id, info->vfid, ret);
        vmngd_uncfg_numa_info(info->dev_id, info->vfid);
        vmngd_del_vf_resource_from_vfg(info->dev_id, info->vfid);
        vmngd_free_vf_resource(info->dev_id, info->vfid);
        return ret;
    }
    return 0;
}

STATIC int vmngd_resource_free_adapter(u32 dev_id, u32 vfid)
{
    vmng_vf_cfg_t ts_cfg = {0};

    vmngd_del_vf_resource_from_vfg(dev_id, vfid);
    if (vmngd_get_dev_chip_type(dev_id) == HISI_CLOUD_V2) {
        vmngd_set_ts_cfg(dev_id, vfid, &ts_cfg);
        vmngd_print_ts_cfg(&ts_cfg);
        (void)vmngd_send_msg_to_ts_destory(dev_id, vfid, &ts_cfg);
        // release numa
        vmngd_uncfg_numa_info(dev_id, vfid);
    }
    vmngd_free_vf_resource(dev_id, vfid);
    return 0;
}

STATIC void vmngd_set_pcie_dmabitmap(struct vmng_vdev_ctrl *ctrl, unsigned long drv_pcie_dma_bitmap)
{
    // The source pcie_dma_bitmap in ctrl is pcie dma bitmap used by ts.
    // Add pcie dma bitmap used by driver to ctrl, because qos need this info.
    ctrl->vf_cfg.accelerator.pcie_dma_bitmap |= drv_pcie_dma_bitmap;
}

static inline void vmngd_adapt_total_aic(unsigned int *pf_aic)
{
#define MILAN_BIN3_AIC_NUM 25
#define MILAN_BIN0_AIC_NUM 24
    if (*pf_aic == MILAN_BIN3_AIC_NUM) {
        *pf_aic = MILAN_BIN0_AIC_NUM;
    }
}

STATIC int vmngd_pci_client_online(u32 dev_id, u32 vfid, struct vmng_vdev_ctrl *ctrl)
{
    u32 udevid = ctrl->dev_id;
    unsigned long dma_bitmap = 0;
    struct vmng_vf_object *vf = NULL;
    u32 vf_aic, pf_aic;
    int ret;

    if (!vmngd_is_sriov_support(dev_id)) {
        return vmngd_add_mia_uda(dev_id, vfid, UDA_VIRTUAL);
    }

    vf = vmngd_get_vf_object(dev_id, vfid);
    if (vf == NULL) {
        return -EINVAL;
    }
    vf_aic = get_vf_res_num(vf, MIA_AC_AIC);
    pf_aic = get_total_res_num(vf->pf, MIA_AC_AIC);
    vmngd_adapt_total_aic(&pf_aic);

    vmng_info("Client online notify aicore number. (udev_id=%u;vf_aic=%u;pf_aic=%u)\n", udevid, vf_aic, pf_aic);

#ifndef CFG_FEATURE_SOC_VIRTMNG
    ret = agentdrv_sriov_init_instance(udevid, vf->vm_full_spec_enable, vf_aic, pf_aic, &dma_bitmap);
    if (ret != 0) {
        vmng_err("Sriov uninit pcie instance failed. (udev_id=%u;ret=%d)\n", udevid, ret);
        return ret;
    }
#endif
    vmngd_set_pcie_dmabitmap(ctrl, dma_bitmap);

    ret = vmngd_add_mia_uda(dev_id, vfid, UDA_VIRTUAL);
    if (ret != 0) {
        vmng_err("Add mia device failed. (dev_id=%u;vfid=%u;ret=%d)\n", dev_id, vfid, ret);
#ifndef CFG_FEATURE_SOC_VIRTMNG
        (void)agentdrv_sriov_uninit_instance(udevid);
#endif
        return ret;
    }
    return 0;
}

STATIC int vmngd_pci_client_offline(u32 dev_id, u32 vfid, struct vmng_vdev_ctrl *ctrl)
{
#ifndef CFG_FEATURE_SOC_VIRTMNG
    u32 udevid = ctrl->dev_id;
#endif
    int ret;

    if (!vmngd_is_sriov_support(dev_id)) {
        return vmngd_remove_mia_uda(dev_id, vfid, UDA_VIRTUAL);
    }

    ret = vmngd_remove_mia_uda(dev_id, vfid, UDA_VIRTUAL);
    if (ret != 0) {
        vmng_err("Remove mia device failed. (dev_id=%u;vfid=%u;ret=%d)\n", dev_id, vfid, ret);
    }

#ifndef CFG_FEATURE_SOC_VIRTMNG
    ret = agentdrv_sriov_uninit_instance(udevid);
    if (ret != 0) {
        vmng_err("Sriov uninit pcie instance failed. (udev_id=%u;ret=%d)\n", udevid, ret);
    }
#endif

    return ret;
}

STATIC bool vmngd_is_vf_check(u32 dev_id)
{
    if (vmngd_get_pfvf_type_by_devid(dev_id) == VMNGD_TYPE_VF) {
        return true;
    }
    return false;
}

STATIC void vmngd_resource_update_vdev_ctrl(u32 dev_id, u32 vfid, struct vmng_vdev_ctrl *ctrl)
{
    struct vmng_vf_object *vf = NULL;

    vf = vmngd_get_vf_object(dev_id, vfid);
    if (vf == NULL) {
        return;
    }

    ctrl->core_num = get_vf_res_num(vf, MIA_AC_AIC);
    ctrl->total_core_num = get_total_res_num(vf->pf, MIA_AC_AIC);
    ctrl->vfid = 0;
    vmngd_set_resource_to_vdev_ctrl(dev_id, vfid, &ctrl->vf_cfg);
}

STATIC void vmngd_resource_enquire_base_proc(struct vmng_soc_res_base *base, vmng_resource_list *res)
{
    base->memory = get_res_num(res, MIA_MEM_NUMA);
    base->memory_spec = get_res_num(res, MIA_SYS_MEM);
    base->numa_bitmap = get_res_bitmap(res, MIA_MEM_NUMA);
}

STATIC void vmngd_memory_merge_intervals(struct vmng_vf_memory_info *memory,
    struct vmng_numa_address *numa_addr, u32 numa_number)
{
    unsigned int i;
    unsigned int j;

    if (numa_number > DEVMM_MAX_NUMA_NUM_OF_PER_DEV) {
        return;
    }

    memory->address[0].start = numa_addr[0].start;
    memory->address[0].end = numa_addr[0].end;

    for (i = 1, j = 0; i < numa_number; ++i) {
        if (numa_addr[i].start <= memory->address[j].end) {
            memory->address[j].end = numa_addr[i].end;
        } else {
            // all the numa number is less than 30, and the numa address is continue
            // so the number of ranges is less than 15 afrer merge interval
            if ((j + 1) >= VMNG_NUMA_MAX_NUM) {
                break;
            }
            j++;
            memory->address[j].start = numa_addr[i].start;
            memory->address[j].end = numa_addr[i].end;
        }
    }
    j++;
    memory->number = j;
    for (; j < VMNG_NUMA_MAX_NUM; ++j) {
        memory->address[j].start = 0;
        memory->address[j].end = 0;
    }
    return;
}

STATIC void vmngd_resource_enquire_each_vf_proc(struct vmng_vf_res_info *info, struct vmng_vf_object *vf_node)
{
    struct vmng_numa_address numa_addr[DEVMM_MAX_NUMA_NUM_OF_PER_DEV];
    struct vmng_resource_base *numa_res = NULL;
    u32 numa_id[DEVMM_MAX_NUMA_NUM_OF_PER_DEV];
    u32 numa_num = 0;
    int idx = -1;
    int nid;
    int ret;
    int i;

    if (vf_node->vfg == NULL) {
        vmng_err("vf has none bind to vfg. (dev_id=%u;vfid=%u)\n", vf_node->dev_id, vf_node->vfid);
        return;
    }

    info->dev_id = vf_node->dev_id;
    info->vfid = vf_node->vfid;
    info->vfg.vfg_id = vf_node->vfg->vfg_id;
    info->vfg.vfg_type = vf_node->vfg->vfg_type;
    info->vfg.vfg_refresh.vfg_mode = vf_node->vfg->vfg_mode;
    info->vfg.vfg_refresh.token = vf_node->token;
    info->vfg.vfg_refresh.token_max = vf_node->token_max;
    info->vfg.vfg_refresh.task_timeout = vf_node->task_timeout;

    if (vf_node->pf->chip_type != HISI_CLOUD_V2) {
        return;
    }

    ret = vmngd_resource_numa_id_init(vf_node->dev_id, numa_id, DEVMM_MAX_NUMA_NUM_OF_PER_DEV, &numa_num);
    if (ret != 0) {
        vmng_err("Cannot get numa id. (dev_id=%u;numa_num=%u;ret=%d)\n", vf_node->dev_id, numa_num, ret);
        return;
    }
    numa_res = vmngd_get_res_unit(&vf_node->resource, MIA_MEM_NUMA);
    if (numa_res == NULL) {
        vmng_err("Get not get numa resource info. (dev_id=%u;vfid=%u)\n", vf_node->dev_id, vf_node->vfid);
        return;
    }

    info->memory.number = vmngd_get_element_bitnum(&numa_res->element);
    info->memory.size = (u64)numa_res->element.num;

    for (i = 0; i < info->memory.number; i++) {
        idx = (int)find_next_bit(&numa_res->element.bitmap, numa_res->element.bitnum, idx + 1);
        if (idx >= (int)numa_res->element.bitnum) {
            vmng_err("Find first bit err.(dev_id=%u; vfid=%u)\n", vf_node->dev_id, vf_node->vfid);
            return;
        }

        nid = numa_id[idx];
        numa_addr[i].start = __pfn_to_phys(node_start_pfn(nid));
        numa_addr[i].end = __pfn_to_phys(node_end_pfn(nid));
        set_bit((unsigned int)nid, &info->memory.numa_id.bitmap);
    }
    vmngd_memory_merge_intervals(&info->memory, numa_addr, info->memory.number);
    return;
}

STATIC void vmngd_resource_enquire_stars_res_proc(struct vmng_stars_res_refresh *refresh,
    struct vmng_stars_res_static *fixed, vmng_resource_list *res)
{
    refresh->aiv = get_res_num(res, MIA_AC_AIV);
    refresh->dsa = get_res_num(res, MIA_AC_DSA);
    refresh->rtsq = get_res_num(res, MIA_STARS_RTSQ);
    refresh->cdqm = get_res_num(res, MIA_STARS_CDQ);
    refresh->topic_aicpu_slot = get_res_num(res, MIA_STARS_TOPIC_ACPU_SLOT);
    refresh->host_ctrl_cpu = get_res_num(res, MIA_CPU_HOST_CCPU);
    refresh->device_aicpu = (u32)get_res_bitmap(res, MIA_CPU_DEV_ACPU);    // aicpu need to return bitmap info
    refresh->host_aicpu = get_res_num(res, MIA_CPU_HOST_ACPU);
    refresh->jpegd = get_res_num(res, MIA_DVPP_JPEGD);
    refresh->jpege = get_res_num(res, MIA_DVPP_JPEGE);
    refresh->vpc = get_res_num(res, MIA_DVPP_VPC);
    refresh->vdec = get_res_num(res, MIA_DVPP_VDEC);
    refresh->pngd = get_res_num(res, MIA_DVPP_PNGD);
    refresh->venc = get_res_num(res, MIA_DVPP_VENC);
    fixed->aic = get_res_num(res, MIA_AC_AIC);
    fixed->c_core = get_res_num(res, MIA_AC_C_CORE);
    fixed->ffts = get_res_num(res, MIA_AC_FFTS);
    fixed->sdma = get_res_num(res, MIA_AC_SDMA);
    fixed->pcie_dma = get_res_num(res, MIA_AC_PCIE_DMA);
    fixed->acsq = get_res_num(res, MIA_STARS_ACSQ);
    fixed->event_id = get_res_num(res, MIA_STARS_EVENT);
    fixed->notify_id = get_res_num(res, MIA_STARS_NOTIFY);
    fixed->topic_ctrl_cpu_slot = get_res_num(res, MIA_STARS_TOPIC_CCPU_SLOT);
}

STATIC int vmngd_resource_enquire(u32 dev_id, u32 vfid, struct vmng_soc_resource_enquire *info)
{
    struct vmng_pf_object *pf_node = NULL;
    struct vmng_vf_object *vf_node = NULL;

    if (info == NULL) {
        vmng_err("Info is NULL. (dev_id=%u;vfid=%u)\n", dev_id, vfid);
        return VMNG_ERR;
    }

    pf_node = vmngd_get_pf_object(dev_id);
    if (pf_node == NULL) {
        vmng_err("Get pf object failed.(dev_id=%u;vfid=%u)\n", dev_id, vfid);
        return VMNG_ERR;
    }

    vmngd_resource_enquire_base_proc(&info->total.base, &pf_node->total_res);
    vmngd_resource_enquire_stars_res_proc(&info->total.stars_refresh,
        &info->total.stars_static, &pf_node->total_res);
    vmngd_resource_enquire_base_proc(&info->remain.base, &pf_node->remain_res);
    vmngd_resource_enquire_stars_res_proc(&info->remain.stars_refresh,
        &info->remain.stars_static, &pf_node->remain_res);

    if (vfid == 0) {
        return VMNG_OK;
    }

    vf_node = vmngd_get_vf_object(dev_id, vfid);
    if (vf_node == NULL) {
        vmng_err("Get vf object failed. (dev_id=%u; vfid=%u)\n", dev_id, vfid);
        return VMNG_ERR;
    }
    vmngd_resource_enquire_stars_res_proc(&info->each.stars_refresh, &info->each.stars_static, &vf_node->resource);
    vmngd_resource_enquire_each_vf_proc(&info->each, vf_node);

    if (strcpy_s(info->each.name, VMNG_VF_TEMP_NAME_LEN, vf_node->template_name) != 0) {
        vmng_err("strcpy_s template name failed. (dev_id=%u; vfid=%u)\n", dev_id, vfid);
        return VMNG_ERR;
    }

    return VMNG_OK;
}

STATIC int vmngd_resource_reset_stars(u32 dev_id, u32 vfid)
{
    return vmngd_send_msg_to_ts_reset(dev_id, vfid);
}

STATIC int vmngd_resource_reset(u32 dev_id, u32 vfid)
{
    int ret;

    ret = vmngd_resource_reset_stars(dev_id, vfid);
    if (ret != VMNG_OK) {
        vmng_err("Reset stars err.(dev_id=%u, vfid=%u)", dev_id, vfid);
        return VMNG_ERR;
    }

    return VMNG_OK;
}

STATIC int vmngd_resource_init(unsigned int dev_id)
{
    int ret;
    int i;

    ret = vmngd_init_pf_instance(dev_id);
    if (ret != 0) {
        vmng_err("init pf object failed.(dev_id=%u;ret=%d)\n", dev_id, ret);
        return ret;
    }

    ret = vmngd_register_dev_all_res(dev_id);
    if (ret != 0) {
        vmng_err("Register resource failed. (dev_id=%u;ret=%u)\n", dev_id, ret);
        vmngd_uninit_pf_instance(dev_id);
        return ret;
    }

    for (i = 0; i < BASE_VFG_NUM; ++i) {
        ret = vmngd_init_vfg_instance(dev_id, i);
        if (ret != 0) {
            vmng_err("init vfg instance failed. (dev_id=%u;vfg_id=%u;ret=%d)\n", dev_id, i, ret);
            vmngd_uninit_pf_instance(dev_id);
            return ret;
        }
    }
    return 0;
}

STATIC void vmngd_resource_uninit(unsigned int dev_id)
{
    vmngd_uninit_pf_instance(dev_id);
}

int vmngd_res_ops_init(struct vmngd_resource_ops *ops)
{
    if (ops == NULL) {
        vmng_err("Operation pointer is NULL.\n");
        return VMNG_ERR;
    }

    ops->res_init = vmngd_resource_init;
    ops->res_uninit = vmngd_resource_uninit;
    ops->alloc_vf = vmngd_resource_alloc_adapter;
    ops->free_vf = vmngd_resource_free_adapter;
    ops->reset_vf = vmngd_resource_reset;
    ops->res_enquire = vmngd_resource_enquire;
    ops->update_vdev_ctrl = vmngd_resource_update_vdev_ctrl;
    ops->pci_client_online = vmngd_pci_client_online;
    ops->pci_client_offline = vmngd_pci_client_offline;
    ops->is_vf = vmngd_is_vf_check;
    ops->unify_devid_to_pfvf_id = vmngd_get_pfvf_id_by_devid;

    return VMNG_OK;
}

int vmngd_res_ops_uninit(struct vmngd_resource_ops *ops)
{
    if (ops == NULL) {
        vmng_err("Operation pointer is NULL.\n");
        return VMNG_ERR;
    }

    (void)memset_s(ops, sizeof(struct vmngd_resource_ops), 0, sizeof(struct vmngd_resource_ops));
    return VMNG_OK;
}
