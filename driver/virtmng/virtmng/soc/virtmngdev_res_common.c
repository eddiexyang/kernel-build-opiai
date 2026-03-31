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

#include "runenv_config.h"
#include "devdrv_interface.h"
#include "virtmngdev_pci.h"
#include "virtmngdev_res_mng.h"
#include "virtmngdev_res_common.h"

vmngd_resource_info_t *g_vmngd_resource_info[VMNGD_SURPORT_MAX_DEV] = { NULL };

vmngd_resource_unit_t g_vmngd_resource_total = {
    .accelerator = {
        .aiv = { VMNGD_CFG_ELE(AC_AIV)},
        .aic = { VMNGD_CFG_ELE(AC_AIC)},
        .c_core = { VMNGD_CFG_ELE(AC_C_CORE)},
        .dsa = { VMNGD_CFG_ELE(AC_DSA)},
        .ffts = { VMNGD_CFG_ELE(AC_FFTS)},
        .sdma = { VMNGD_CFG_ELE(AC_SDMA)},
        .pcie_dma = { VMNGD_CFG_ELE(AC_PCIE_DMA)},
        .acsq = { VMNGD_CFG_ELE(AC_ACSQ)},
        .rtsq = { VMNGD_CFG_ELE(AC_RTSQ)},
        .event_id = { VMNGD_CFG_ELE(AC_EVENT_ID)},
        .notify_id = { VMNGD_CFG_ELE(AC_NOTIFY_ID)},
        .cdqm = { VMNGD_CFG_ELE(AC_CDQM)},
        .cmo_id = { VMNGD_CFG_ELE(AC_CMO_ID)},
    },
    .dvpp = {
        .jpegd = { VMNGD_CFG_ELE(DVPP_JPEGD)},
        .jpege = { VMNGD_CFG_ELE(DVPP_JPEGE)},
        .vpc = { VMNGD_CFG_ELE(DVPP_VPC)},
        .vdec = { VMNGD_CFG_ELE(DVPP_VDEC)},
        .pngd = { VMNGD_CFG_ELE(DVPP_PNGD)},
        .venc = { VMNGD_CFG_ELE(DVPP_VENC)},
    },
    .cpu = {
        .topic_aicpu_slot = { VMNGD_CFG_ELE(CPU_TOPIC_AICPU)},
        .topic_ctrl_cpu_slot = { VMNGD_CFG_ELE(CPU_TOPIC_CCPU)},
        .host_ctrl_cpu = { VMNGD_CFG_ELE(CPU_HOST_CCPU)},
        .device_aicpu = { VMNGD_CFG_ELE(CPU_DEV_AICPU)},
        .host_aicpu = { VMNGD_CFG_ELE(CPU_HOST_AICPU)},
    },
    .base = {
        .vf = { VMNGD_CFG_ELE(BASE_VF)},
        .vfg = { VMNGD_CFG_ELE(BASE_VFG)},
        .memory = { VMNGD_CFG_ELE(AC_MEMORY)},
        .numa_id = {0}
    },
    .template_name = {0},
    .cfg_info = {0},
    .vfg = NULL,
    .vf_node = {0}
};

/**
 * vmngd_bitmap_check - is bitmap1 and bitmap2 nbits has repeat bit
 * 	@bitmap1: each resource bitmap
 * 	@bitmap2: remain resource bitmap
 * @nbits: number of bits to compare
 *
 * return VMNG_OK: bitmap1 valid
 * return VMNG_ERR: bitmap1 invalid
 */
int vmngd_bitmap_check(unsigned long bitmap1, unsigned long bitmap2, u32 nbits)
{
    unsigned long tmpbitmap;

    (void)bitmap_and(&tmpbitmap, &bitmap1, &bitmap2, nbits);
    return ((tmpbitmap == 0) ? VMNG_OK : VMNG_ERR);
}

int vmngd_resource_info_check(u32 dev_id, u32 vfid, u32 dtype)
{
    if ((dev_id != VMNGD_ID_NO_CHECK) && (dev_id >= VMNGD_SURPORT_MAX_DEV)) {
        vmng_err("dev_id check err. (dev_id=%u)\n", dev_id);
        return VMNG_ERR;
    }

    if ((vfid != VMNGD_ID_NO_CHECK) && ((vfid < VMNGD_VF_NUM_MIN) || (vfid > BASE_VF_NUM))) {
        vmng_err("vf_id check err. (vfid=%u)\n", vfid);
        return VMNG_ERR;
    }
    if ((dtype != VMNGD_ID_NO_CHECK) && (dtype >= VMNG_HW_TYPE_MAX)) {
        vmng_err("dtype check err. (dev_id=%u, vfid=%u, dtype=%u)\n", dev_id, vfid, dtype);
        return VMNG_ERR;
    }

    return VMNG_OK;
}

void vmngd_resource_enquire_stars_proc(struct vmng_stars_res_refresh *refresh, struct vmng_stars_res_static *fixed,
    const vmngd_resource_unit_t *unit)
{
    if ((refresh == NULL) || (fixed == NULL) || (unit == NULL)) {
        vmng_err("Param NULL.\n");
        return;
    }

    refresh->aiv = unit->accelerator.aiv.num;
    refresh->dsa = unit->accelerator.dsa.num;
    refresh->rtsq = unit->accelerator.rtsq.num;
    refresh->cdqm = unit->accelerator.cdqm.num;
    refresh->topic_aicpu_slot = unit->cpu.topic_aicpu_slot.num;
    refresh->host_ctrl_cpu = unit->cpu.host_ctrl_cpu.num;
    refresh->device_aicpu = unit->cpu.device_aicpu.bitmap;
    refresh->host_aicpu = unit->cpu.host_aicpu.num;
    refresh->jpegd = unit->dvpp.jpegd.num;
    refresh->jpege = unit->dvpp.jpege.num;
    refresh->vpc = unit->dvpp.vpc.num;
    refresh->vdec = unit->dvpp.vdec.num;
    refresh->pngd = unit->dvpp.pngd.num;
    refresh->venc = unit->dvpp.venc.num;
    fixed->aic = unit->accelerator.aic.num;
    fixed->c_core = unit->accelerator.c_core.num;
    fixed->ffts = unit->accelerator.ffts.num;
    fixed->sdma = unit->accelerator.sdma.num;
    fixed->pcie_dma = unit->accelerator.pcie_dma.num;
    fixed->acsq = unit->accelerator.acsq.num;
    fixed->event_id = unit->accelerator.event_id.num;
    fixed->notify_id = unit->accelerator.notify_id.num;
    fixed->topic_ctrl_cpu_slot = unit->cpu.topic_ctrl_cpu_slot.num;
}

void vmngd_resource_upate_vf_status(u32 dev_id, u32 vfid, u32 status)
{
    u32 idx;
    vmngd_resource_unit_t *each = NULL;

    mutex_lock(&g_vmngd_resource_info[dev_id]->lock);
    idx = vfid - VMNGD_VF_NUM_MIN;
    each = &g_vmngd_resource_info[dev_id]->vf_cfg_each[idx];
    each->cfg_info.status = status;
    mutex_unlock(&g_vmngd_resource_info[dev_id]->lock);
}

vmngd_resource_info_t *vmngd_get_resource_info(u32 dev_id)
{
    return g_vmngd_resource_info[dev_id];
}

vmngd_resource_unit_t *vmngd_get_resource_total(void)
{
    return &g_vmngd_resource_total;
}

void vmngd_free_resource_info(u32 dev_id)
{
    vmngd_uninit_pf_instance(dev_id);

    if (g_vmngd_resource_info[dev_id] != NULL) {
        mutex_destroy(&g_vmngd_resource_info[dev_id]->lock);
        if (memset_s(g_vmngd_resource_info[dev_id], sizeof(vmngd_resource_info_t), 0, sizeof(vmngd_resource_info_t)) !=
            EOK) {
            vmng_err("Call memset_s failed.(dev_id=%u)\n", dev_id);
        }

        kfree(g_vmngd_resource_info[dev_id]);
        g_vmngd_resource_info[dev_id] = NULL;
    }
}

int vmngd_alloc_resource_info(u32 dev_id)
{
    g_vmngd_resource_info[dev_id] = (vmngd_resource_info_t *)kzalloc(sizeof(vmngd_resource_info_t), GFP_KERNEL);
    if (g_vmngd_resource_info[dev_id] == NULL) {
        vmng_err("Allocate memory for g_vmngd_resource_info fail.(dev_id=%u)\n", dev_id);
        return VMNG_ERR;
    }

    mutex_init(&g_vmngd_resource_info[dev_id]->lock);
    if (memcpy_s(&g_vmngd_resource_info[dev_id]->vf_cfg_total, sizeof(vmngd_resource_unit_t),
        &g_vmngd_resource_total, sizeof(vmngd_resource_unit_t)) != EOK) {
        vmng_err("Call memcpy_s failed.(dev_id=%u)\n", dev_id);
        goto FAIL_EXIT;
    }

    if (memcpy_s(&g_vmngd_resource_info[dev_id]->vf_cfg_remain, sizeof(vmngd_resource_unit_t),
        &g_vmngd_resource_info[dev_id]->vf_cfg_total, sizeof(vmngd_resource_unit_t)) != EOK) {
        vmng_err("Call memcpy_s failed.(dev_id=%u)\n", dev_id);
        goto FAIL_EXIT;
    }

    return VMNG_OK;

FAIL_EXIT:
    vmngd_free_resource_info(dev_id);
    return VMNG_ERR;
}

int vmngd_alloc_vfid_dynamic(u32 dev_id, u32 *fid)
{
    u32 vfid = 0;

    if (fid == NULL) {
        vmng_err("Param NULL.\n");
        return VMNG_ERR;
    }

    if (dev_id >= VMNG_PDEV_MAX) {
        vmng_err("Input parameter is error. (dev_id=%u)\n", dev_id);
        return -EINVAL;
    }

    for (vfid = VMNG_VDEV_FIRST_VFID; vfid <= BASE_VF_NUM; vfid++) {
        if (vmngd_get_ctrl(dev_id, vfid)->status == VMNG_VDEV_STATUS_FREE) {
            break;
        }
    }

    *fid = vfid;

    return VMNG_OK;
}

int vmngd_alloc_vfid_static(u32 dev_id, u32 fid)
{
    if (vmngd_get_ctrl(dev_id, fid)->status != VMNG_VDEV_STATUS_FREE) {
        vmng_err("fid already in used. (dev_id=%u; fid=%u)\n", dev_id, fid);
        return VMNG_ERR;
    }

    return VMNG_OK;
}

int vmngd_set_pg_info(unsigned int dev_id, vmng_common_pg *pg_info)
{
    u32 chip_type = vmngd_get_dev_chip_type(dev_id);
    if (chip_type == HISI_CLOUD_V2) {
        return vmngd_set_pg_info_cloud_v2(dev_id, pg_info);
    }
    return 0;
}
EXPORT_SYMBOL(vmngd_set_pg_info);

int vmngd_sriov_modify_soc_res(unsigned int dev_id, enum vmng_pf_sriov_status sriov_status)
{
    u32 chip_type = vmngd_get_dev_chip_type(dev_id);
    if (chip_type == HISI_CLOUD_V2) {
        return vmngd_sriov_modify_soc_res_cloud_v2(dev_id, sriov_status);
    }
    return 0;
}

#ifdef CFG_FEATURE_SOC_VIRTMNG
STATIC int vmngd_get_rc_pfvf_id_by_devid(u32 dev_id, u32 *pf_id, u32 *vf_id)
{
    if (dev_id >= VMNGD_VF_DEVID_START) {
        *pf_id = (dev_id - VMNGD_VF_DEVID_START) / VMNGD_MAX_VFID_NUM;
        *vf_id = (dev_id - VMNGD_VF_DEVID_START) % VMNGD_MAX_VFID_NUM + 1;
    } else {
        *pf_id = dev_id;
        *vf_id = 0;
    }
    return 0;
}
#endif

// only used in rc mode and sriov, pfid and vfid value would be changed
int vmngd_get_pfvf_id_by_devid(u32 dev_id, u32 *pf_id, u32 *vf_id)
{
#ifdef CFG_FEATURE_SOC_VIRTMNG
    return vmngd_get_rc_pfvf_id_by_devid(dev_id, pf_id, vf_id);
#else
    return devdrv_get_pfvf_id_by_devid(dev_id, pf_id, vf_id);
#endif
}
EXPORT_SYMBOL(vmngd_get_pfvf_id_by_devid);

#ifdef CFG_FEATURE_SOC_VIRTMNG
STATIC int vmngd_get_rc_devid_by_pfvf_id(u32 pf_id, u32 vf_id, u32 *dev_id)
{
    if ((pf_id >= VMNGD_SURPORT_MAX_DEV) || (vf_id > BASE_VF_NUM)) {
        vmng_err("Params is err. (pf_id=%u, vf_id=%u)\n", pf_id, vf_id);
        return -EINVAL;
    }

    // vdev_index = (pf_id * VDAVINCI_MAX_VFID_NUM) + (vfid - 1) + 32
    if (vf_id > 0) {
        *dev_id = pf_id * VMNGD_MAX_VFID_NUM + (vf_id - 1) + VMNGD_VF_DEVID_START;
    } else {
        *dev_id = pf_id;
    }

    return 0;
}
#endif

int vmngd_get_devid_by_pfvf_id(u32 pf_id, u32 vf_id, u32 *dev_id)
{
#ifndef CFG_FEATURE_SOC_VIRTMNG
    if (vmngd_is_sriov_support(pf_id) == true) {
        return devdrv_get_devid_by_pfvf_id(pf_id, vf_id, dev_id);
    }
#else
    if (dbl_get_rc_ep_mode() == DBL_RC_MODE) {
        return vmngd_get_rc_devid_by_pfvf_id(pf_id, vf_id, dev_id);
    }
#endif

    *dev_id = pf_id;

    return 0;
}
EXPORT_SYMBOL(vmngd_get_devid_by_pfvf_id);

#ifdef CFG_FEATURE_SOC_VIRTMNG
STATIC int vmngd_get_rc_pfvf_type_by_devid(u32 dev_id)
{
    u32 pf_id;
    u32 vf_id;
    u32 pfvf_type;

    vmngd_get_rc_pfvf_id_by_devid(dev_id, &pf_id, &vf_id);

    if (vf_id != 0) {
        pfvf_type = VMNGD_TYPE_VF;
    } else {
        pfvf_type = VMNGD_TYPE_PF;
    }

    return pfvf_type;
}
#endif

// mini v3 EP does not insmode ko
int vmngd_get_pfvf_type_by_devid(u32 dev_id)
{
#ifdef CFG_FEATURE_SOC_VIRTMNG
    return vmngd_get_rc_pfvf_type_by_devid(dev_id);
#else
    return devdrv_get_pfvf_type_by_devid(dev_id);
#endif
}
EXPORT_SYMBOL(vmngd_get_pfvf_type_by_devid);

u32 vmngd_get_dev_chip_type(u32 dev_id)
{
#ifdef CFG_FEATURE_SOC_VIRTMNG
    return HISI_MINI_V3;
#else
    return devdrv_get_dev_chip_type(dev_id);
#endif
}

bool vmngd_is_sriov_support(u32 dev_id)
{
#ifndef CFG_FEATURE_SOC_VIRTMNG
    return devdrv_is_sriov_support(dev_id);
#else
    return false;
#endif
}