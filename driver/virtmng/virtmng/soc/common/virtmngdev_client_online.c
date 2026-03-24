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
 * Create: 2022-02-16
 */

#include "runenv_config.h"
#include "devdrv_interface.h"
#include "virtmngdev_res_mng.h"
#include "virtmngdev_client_online.h"

unsigned int get_res_num(vmng_resource_list *list, enum soc_mia_res_type type)
{
    struct vmng_resource_base *type_res = vmngd_get_res_unit(list, type);
    return type_res == NULL ? 0 : type_res->element.num;
}

unsigned long get_res_bitmap(vmng_resource_list *list, enum soc_mia_res_type type)
{
    if (list == NULL) {
        vmng_info("get_res_bitmap list = null\n");
        return 0;
    }

    if (list->res_list[type] == NULL) {
        vmng_info("get_res_bitmap list->res_list[type] = null, type = %u\n", type);
        return 0;
    }

    return list->res_list[type]->element.bitmap;
}

#define VMNG_PRINT_INFO(var_struct, var_name, format) vmng_info(#var_name ":" #format "\n", (var_struct.var_name))
void vmngd_print_ts_cfg(const vmng_vf_cfg_t *cfg)
{
    vmng_info("Print vf info to ts begin =======================\n");
    VMNG_PRINT_INFO(cfg->accelerator, c_core_bitmap, "%#x");
    VMNG_PRINT_INFO(cfg->accelerator, dsa_bitmap, "%#x");
    VMNG_PRINT_INFO(cfg->accelerator, ffts_bitmap, "%#x");
    VMNG_PRINT_INFO(cfg->accelerator, sdma_bitmap, "%#x");
    VMNG_PRINT_INFO(cfg->accelerator, pcie_dma_bitmap, "%#x");
    VMNG_PRINT_INFO(cfg->accelerator, acsq_slice_bitmap, "%#x");
    VMNG_PRINT_INFO(cfg->accelerator, rtsq_slice_bitmap, "%#x");
    VMNG_PRINT_INFO(cfg->accelerator, event_slice_bitmap, "%#x");
    VMNG_PRINT_INFO(cfg->accelerator, notify_slice_bitmap, "%#x");
    VMNG_PRINT_INFO(cfg->accelerator, cdq_slice_bitmap, "%#x");

    VMNG_PRINT_INFO(cfg->accelerator, aic_bitmap, "%#x");
    VMNG_PRINT_INFO(cfg->accelerator, aiv_bitmap, "%#llx");
    VMNG_PRINT_INFO(cfg->dvpp, jpegd_bitmap, "%#x");
    VMNG_PRINT_INFO(cfg->dvpp, jpege_bitmap, "%#x");
    VMNG_PRINT_INFO(cfg->dvpp, vpc_bitmap, "%#x");
    VMNG_PRINT_INFO(cfg->dvpp, vdec_bitmap, "%#x");
    VMNG_PRINT_INFO(cfg->dvpp, pngd_bitmap, "%#x");
    VMNG_PRINT_INFO(cfg->dvpp, venc_bitmap, "%#x");

    VMNG_PRINT_INFO(cfg->cpu, topic_aicpu_slot_bitmap, "%#x");
    VMNG_PRINT_INFO(cfg->cpu, topic_ctrl_cpu_slot_bitmap, "%#x");
    VMNG_PRINT_INFO(cfg->cpu, host_ctrl_cpu_bitmap, "%#x");
    VMNG_PRINT_INFO(cfg->cpu, device_aicpu_bitmap, "%#x");
    VMNG_PRINT_INFO(cfg->cpu, host_aicpu_bitmap, "%#llx");

    VMNG_PRINT_INFO(cfg->id, vf_id, "%u");
    VMNG_PRINT_INFO(cfg->id, vfg_mode, "%u");
    VMNG_PRINT_INFO(cfg->id, vfg_id, "%u");
    VMNG_PRINT_INFO(cfg->id, vip, "%u");
    VMNG_PRINT_INFO(cfg->id, token, "%#llx");
    VMNG_PRINT_INFO(cfg->id, token_max, "%#llx");
    VMNG_PRINT_INFO(cfg->id, task_timeout, "%#llx");
    vmng_info("Print vf info to ts end =======================\n");
}

void vmngd_set_resource_to_vdev_ctrl(u32 dev_id, u32 vfid, vmng_vf_cfg_t *cfg)
{
    struct vmng_vf_object *vf_node = NULL;

    vf_node = vmngd_get_vf_object(dev_id, vfid);
    if (vf_node == NULL) {
        vmng_err("vmngd get vf object failed.\n");
        return;
    }

    cfg->accelerator.c_core_bitmap = (u32)get_vf_res_bitmap(vf_node, MIA_AC_C_CORE);
    cfg->accelerator.dsa_bitmap = (u32)get_vf_res_bitmap(vf_node, MIA_AC_DSA);
    cfg->accelerator.ffts_bitmap = (u32)get_vf_res_bitmap(vf_node, MIA_AC_FFTS);
    cfg->accelerator.sdma_bitmap = (u32)get_vf_res_bitmap(vf_node, MIA_AC_SDMA);
    cfg->accelerator.pcie_dma_bitmap = (u32)get_vf_res_bitmap(vf_node, MIA_AC_PCIE_DMA);
    cfg->accelerator.acsq_slice_bitmap = (u32)get_vf_res_bitmap(vf_node, MIA_STARS_ACSQ);
    cfg->accelerator.rtsq_slice_bitmap = (u32)get_vf_res_bitmap(vf_node, MIA_STARS_RTSQ);
    cfg->accelerator.event_slice_bitmap = (u32)get_vf_res_bitmap(vf_node, MIA_STARS_EVENT);
    cfg->accelerator.notify_slice_bitmap = (u32)get_vf_res_bitmap(vf_node, MIA_STARS_NOTIFY);
    cfg->accelerator.cdq_slice_bitmap = (u32)get_vf_res_bitmap(vf_node, MIA_STARS_CDQ);
    cfg->accelerator.cmo_slice_bitmap = (u32)get_vf_res_bitmap(vf_node, MIA_STARS_CMO);
    cfg->accelerator.aic_bitmap = (u32)get_vf_res_bitmap(vf_node, MIA_AC_AIC);
    cfg->accelerator.aiv_bitmap = (u64)get_vf_res_bitmap(vf_node, MIA_AC_AIV);

    cfg->dvpp.jpegd_bitmap = (u32)get_vf_res_bitmap(vf_node, MIA_DVPP_JPEGD);
    cfg->dvpp.jpege_bitmap = (u32)get_vf_res_bitmap(vf_node, MIA_DVPP_JPEGE);
    cfg->dvpp.vpc_bitmap = (u32)get_vf_res_bitmap(vf_node, MIA_DVPP_VPC);
    cfg->dvpp.vdec_bitmap = (u32)get_vf_res_bitmap(vf_node, MIA_DVPP_VDEC);
    cfg->dvpp.pngd_bitmap = (u32)get_vf_res_bitmap(vf_node, MIA_DVPP_PNGD);
    cfg->dvpp.venc_bitmap = (u32)get_vf_res_bitmap(vf_node, MIA_DVPP_VENC);

    cfg->cpu.topic_aicpu_slot_bitmap = (u32)get_vf_res_bitmap(vf_node, MIA_STARS_TOPIC_ACPU_SLOT);
    cfg->cpu.topic_ctrl_cpu_slot_bitmap = (u32)get_vf_res_bitmap(vf_node, MIA_STARS_TOPIC_CCPU_SLOT);
    cfg->cpu.host_ctrl_cpu_bitmap = (u32)get_vf_res_bitmap(vf_node, MIA_CPU_HOST_CCPU);
    cfg->cpu.device_aicpu_bitmap = (u32)get_vf_res_bitmap(vf_node, MIA_CPU_DEV_ACPU);
    cfg->cpu.host_aicpu_bitmap = (u64)get_vf_res_bitmap(vf_node, MIA_CPU_HOST_ACPU);

    cfg->id.vf_id = (u8)vf_node->vfid - VMNG_VDEV_FIRST_VFID;
    cfg->id.vfg_mode = (u8)vf_node->vfg->vfg_mode;
    cfg->id.vfg_id = (u8)vf_node->vfg->vfg_id;
    // mini v3 do not set vip : cfg->id.vip = 0;
    if (dbl_get_rc_ep_mode() == DBL_RC_MODE) {
        cfg->id.vip = 0;
    } else {
        cfg->id.vip = (u8)((vf_node->vfg->vfg_type == VMNG_VF_GROUP_TYPE_VIP) ? 1 : 0);
    }
    cfg->id.token = vf_node->token;
    cfg->id.token_max = vf_node->token_max;
    cfg->id.task_timeout = vf_node->task_timeout;
}