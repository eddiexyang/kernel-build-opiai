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

#include <linux/io.h>

#include "soc_misc_err_info.h"
#include "soc_misc_init.h"
#include "soc_misc_l3t_arm_ras.h"
#include "soc_misc_safety.h"
#include "dms_node_type.h"
#include "soc_misc_fault.h"
#include "soc_misc_l3t_ras.h"
#include "receiver/fpdc_ras_receiver.h"
#include "drv_ras_common.h"
#include "soc_misc_spec.h"
#include "ascend_kernel_hal.h"
#include "drvfault_user_common.h"

#define L3T_IERR_OFFSET (8U)

#define RAS_L3T_UNMASK_REMAP_SIZE 0x4
#define RAS_UNMASK_CORRECTED 0x4000000U
#define RAS_CORRECTED_ERROR 0x02

 /* L3T RAS IERR TYPE */
enum l3t_ras_ierr {
    L3T_IERR_REQ_ERR_INTERNAL = 0xC5,      /* UEU */
    L3T_IERR_REQ_ERR_HYDRASNP = 0xC6,      /* UEU */
    L3T_IERR_REQ_ERR_SKYROSSNP = 0xC7,     /* UEU */
    L3T_IERR_REQ_ERR_EXTERANL = 0xC8,      /* UEU */
    L3T_IERR_TIMEOUT_ERR_CQ = 0xC9,        /* UER */
    L3T_IERR_ECC_TAG_ERR_DOUBLE = 0x4A,    /* UEU */
    L3T_IERR_ECC_DATA_ERR_DOUBLE = 0x4D,   /* UER */
    L3T_IERR_REQ_ATTR_ERROR_SQ = 0xCE,     /* UER */
    L3T_IERR_REQ_ATTR_ERROR_CPIPE = 0x40,  /* UER */
    L3T_IERR_REQ_ATTR_ERROR_SPIPE = 0x41,  /* UER */
    L3T_IERR_REQ_DAW_MISS = 0xC2,          /* UER */
    L3T_IERR_REQ_STASH_HIT_CPIPE = 0x05,   /* UER */
    L3T_IERR_MSD_OVLP_ERR0 = 0x01,         /* UEO */
    L3T_IERR_MSD_OVLP_ERR1 = 0x02,         /* UEO */
    L3T_IERR_DAW_OVLP_ERR0 = 0x03,         /* UEO */
    L3T_IERR_DAW_OVLP_ERR1 = 0x04,         /* UEO */
    L3T_IERR_ECC_ERR_SINGLE = 0x4B,        /* CE */
};

#define L3T_RAS_COVERGE_NODE(ierr, msg, ras_err) { \
    .subsys_id = DMS_DEV_TYPE_L3T,                \
    .module_id = SOC_MISC_SENSOR_L3T,             \
    .section_type = RAS_SEC_ARM,                  \
    .ras_code.int_status = ierr,                  \
    .describe = msg,                              \
    .sensor_type = DMS_SEN_TYPE_RAS_SENSOR,       \
    .error_type = ras_err,                        \
}

struct ras_fault_converge_item g_soc_misc_l3t_arm_ras_converge_table[L3T_ARM_RAS_CONVERGE_NUM] = {
    L3T_RAS_COVERGE_NODE(L3T_IERR_REQ_ERR_INTERNAL, "req_err_internal", RAS_ERROR_TYPE_IN_CFG_ERR),
    L3T_RAS_COVERGE_NODE(L3T_IERR_REQ_ERR_HYDRASNP, "req_err_hydrasnp", RAS_ERROR_TYPE_IN_CFG_ERR),
    L3T_RAS_COVERGE_NODE(L3T_IERR_REQ_ERR_SKYROSSNP, "req_err_skyrossnp", RAS_ERROR_TYPE_IN_CFG_ERR),
    L3T_RAS_COVERGE_NODE(L3T_IERR_REQ_ERR_EXTERANL, "req_err_exteranl", RAS_ERROR_TYPE_IN_CFG_ERR),
    L3T_RAS_COVERGE_NODE(L3T_IERR_TIMEOUT_ERR_CQ, "timeout_err_cq", RAS_ERROR_TYPE_IN_CFG_ERR),
    L3T_RAS_COVERGE_NODE(L3T_IERR_ECC_TAG_ERR_DOUBLE, "ecc_tag_err_double", RAS_ERROR_TYPE_MBECC),
    L3T_RAS_COVERGE_NODE(L3T_IERR_ECC_DATA_ERR_DOUBLE, "ecc_data_err_double", RAS_ERROR_TYPE_MBECC),
    L3T_RAS_COVERGE_NODE(L3T_IERR_REQ_ATTR_ERROR_SQ, "req_attr_error_sq", RAS_ERROR_TYPE_IN_CFG_ERR),
    L3T_RAS_COVERGE_NODE(L3T_IERR_REQ_ATTR_ERROR_CPIPE, "req_attr_error_cpipe", RAS_ERROR_TYPE_IN_CFG_ERR),
    L3T_RAS_COVERGE_NODE(L3T_IERR_REQ_ATTR_ERROR_SPIPE, "req_attr_error_spipe", RAS_ERROR_TYPE_IN_CFG_ERR),
    L3T_RAS_COVERGE_NODE(L3T_IERR_REQ_DAW_MISS, "req_daw_miss", RAS_ERROR_TYPE_IN_CFG_ERR),
    L3T_RAS_COVERGE_NODE(L3T_IERR_REQ_STASH_HIT_CPIPE, "req_stash_hit_cpipe", RAS_ERROR_TYPE_IN_CFG_ERR),
    L3T_RAS_COVERGE_NODE(L3T_IERR_MSD_OVLP_ERR0, "msd_ovlp_err0", RAS_ERROR_TYPE_IN_CFG_ERR),
    L3T_RAS_COVERGE_NODE(L3T_IERR_MSD_OVLP_ERR1, "msd_ovlp_err1", RAS_ERROR_TYPE_IN_CFG_ERR),
    L3T_RAS_COVERGE_NODE(L3T_IERR_DAW_OVLP_ERR0, "daw_ovlp_err0", RAS_ERROR_TYPE_IN_CFG_ERR),
    L3T_RAS_COVERGE_NODE(L3T_IERR_DAW_OVLP_ERR1, "daw_ovlp_err1", RAS_ERROR_TYPE_IN_CFG_ERR),
    L3T_RAS_COVERGE_NODE(L3T_IERR_ECC_ERR_SINGLE, "ecc_err_single", RAS_ERROR_TYPE_SBECCOverThold),
};

#ifdef CFG_FEATURE_ENTIRETY_UNMASK
struct ras_l3t_unmask_hw_info {
    unsigned long long reg_addr;
    unsigned long long umask_offset;
    unsigned int unmask_val;
};

STATIC struct ras_l3t_unmask_hw_info g_l3t_unmask_hw_info[RAS_L3T_NUM_MAX] = {
    { RAS_L3T0_BASE_ADDR, RAS_L3T0_UNMASK_OFFSET, RAS_L3T0_UNMASK_VAL},
    { RAS_L3T1_BASE_ADDR, RAS_L3T1_UNMASK_OFFSET, RAS_L3T1_UNMASK_VAL},
};
#endif

int soc_misc_l3t_ops_init(struct dms_node *device)
{
    soc_misc_drv_debug("DEBUG (node_name=%.*s)\n", DMS_MAX_DEV_NAME_LEN, device->node_name);
    return 0;
}

void soc_misc_l3t_ops_uninit(struct dms_node *device)
{
    soc_misc_drv_debug("DEBUG (node_name=%.*s)\n", DMS_MAX_DEV_NAME_LEN, device->node_name);
    return;
}

#ifndef AOS_LLVM_BUILD
STATIC int soc_misc_l3t_get_ierr(const struct notify_data *pdata, u32 *ierr)
{
    u32 err_status = 0;
    int ret;

    ret = soc_misc_get_err_status(pdata, &err_status);
    if (ret) {
        soc_misc_drv_err("get err status from ras data failed. (ret=%d)\n", ret);
        return ret;
    }
    *ierr = (err_status >> L3T_IERR_OFFSET) & 0xFF;
    return 0;
}

#ifdef CFG_FEATURE_SAFETY_MANAGER
STATIC bool soc_misc_l3t_is_safety_fault(u32 ierr)
{
    if (ierr == L3T_IERR_ECC_ERR_SINGLE ||
        ierr == L3T_IERR_ECC_DATA_ERR_DOUBLE ||
        ierr == L3T_IERR_ECC_TAG_ERR_DOUBLE) {
        return true;
    }
    return false;
}
#endif

STATIC void write_l3t_reg32(unsigned int dev_id, unsigned long long reg_phy_addr, unsigned int value)
{
#ifdef CFG_FEATURE_ENTIRETY_UNMASK
    (void)ras_int_unmask_reg_write(reg_phy_addr, value, value);
#else
    void __iomem *reg_virt_addr = NULL;
    unsigned int unmask_val = value;

    reg_virt_addr = ioremap(reg_phy_addr, RAS_L3T_UNMASK_REMAP_SIZE);
    if (reg_virt_addr == NULL) {
        soc_misc_drv_err("Failed to ioremap l3t unmask irq register. (devid=%u)\n", dev_id);
        return;
    }

    unmask_val = value;
    writel(unmask_val, (volatile void *)reg_virt_addr);
    iounmap(reg_virt_addr);
    reg_virt_addr = NULL;
#endif

    soc_misc_drv_event("Unmask l3t local ras irq success. (devid=%u)\n", dev_id);
}

#ifdef CFG_FEATURE_ENTIRETY_UNMASK
STATIC void soc_misc_l3t_fault_unmask_multi_type(unsigned int die_id,
    unsigned long long reg_addr, unsigned int reg_val)
{
    unsigned int i;

    for (i = 0; i < RAS_INT_TYPE_NUM_MAX; i++) {
        if ((RAS_INT_TYPE_ENABLE_MASK & (0x01 << i)) != 0) {
            write_l3t_reg32(die_id, reg_addr + (i * RAS_INT_TYPE_OFFSET), reg_val);
        }
    }
}
#endif

void soc_misc_l3t_fault_unmask_task(struct work_struct *work)
{
    struct soc_misc_node *s_dev = container_of(work, struct soc_misc_node, unmask_irq_work.work);
    unsigned long long reg_phy_addr;
    unsigned int i;

#ifndef CFG_FEATURE_ENTIRETY_UNMASK
    unsigned long long l3t_base_addr_arg[RAS_L3T_NUM] = {
        RAS_L3T0_BASE_ADDR, RAS_L3T1_BASE_ADDR, RAS_L3T2_BASE_ADDR, RAS_L3T3_BASE_ADDR
    };
    for (i = 0; i < RAS_L3T_NUM; i++) {
        if (s_dev->work_data[i] == 1) {
            reg_phy_addr = DIE_DEVICE_OFFSET * s_dev->die_id + l3t_base_addr_arg[i] + RAS_L3T_UNMASK_OFFSET;
            write_l3t_reg32(s_dev->die_id, reg_phy_addr, RAS_L3T_UNMASK_VAL);
            s_dev->work_data[i] = 0;
        }
    }
#else
    int ret;
    devdrv_hardware_info_t hardware_info = {0};

    ret = hal_kernel_get_hardware_info(s_dev->die_id, &hardware_info);
    if (ret != 0) {
        soc_misc_drv_err("Failed to invoke hal_kernel_get_hardware_info. (devid=%u; ret=%d)\n", s_dev->die_id, ret);
        return;
    }

    for (i = 0; i < RAS_L3T_NUM; i++) {
        if (s_dev->work_data[i] == 1) {
            reg_phy_addr =
                hardware_info.phy_addr_offset + g_l3t_unmask_hw_info[i].reg_addr + g_l3t_unmask_hw_info[i].umask_offset;
            soc_misc_l3t_fault_unmask_multi_type(s_dev->die_id, reg_phy_addr, g_l3t_unmask_hw_info[i].unmask_val);
            s_dev->work_data[i] = 0;
        }
    }
#endif
}

#if !defined(CFG_SOC_PLATFORM_MINIV2) && !defined(CFG_SOC_PLATFORM_MDC_V51)
STATIC bool soc_misc_l3t_check_unmask(unsigned char l3t_id, unsigned device_id)
{
#ifdef CFG_FEATURE_ENTIRETY_UNMASK
    devdrv_hardware_info_t hardware_info = {0};
    unsigned long long reg_phy_addr;
    void __iomem *reg_virt_addr = NULL;
    unsigned int unmask_val = g_l3t_unmask_hw_info[l3t_id].unmask_val;
    unsigned int i;
    bool is_mask = false;
    int ret;

    ret = hal_kernel_get_hardware_info(device_id, &hardware_info);
    if (ret != 0) {
        soc_misc_drv_err("Failed to invoke hal_kernel_get_hardware_info. (devid=%u)\n", device_id);
        return ret;
    }

    reg_phy_addr = hardware_info.phy_addr_offset +
        g_l3t_unmask_hw_info[l3t_id].reg_addr + g_l3t_unmask_hw_info[l3t_id].umask_offset;

    for (i = 0; i < RAS_INT_TYPE_NUM_MAX; i++) {
        if ((RAS_INT_TYPE_ENABLE_MASK & (0x01 << i)) != 0) {
            reg_virt_addr = ioremap(reg_phy_addr + (i * RAS_INT_TYPE_OFFSET), RAS_L3T_UNMASK_REMAP_SIZE);
            if (reg_virt_addr == NULL) {
                soc_misc_drv_err("Failed to ioremap l3t unmask irq register. (devid=%u)\n", device_id);
                return false;
            }

            unmask_val = readl(reg_virt_addr);
            unmask_val = unmask_val & g_l3t_unmask_hw_info[l3t_id].unmask_val;

            iounmap(reg_virt_addr);
            reg_virt_addr = NULL;
            is_mask = (unmask_val != g_l3t_unmask_hw_info[l3t_id].unmask_val) ? true : false;
            if (is_mask) {
                break;
            }
        }
    }

    return is_mask;
#else
    return true;
#endif
}
#endif

STATIC void l3t_relieve_suppresion(unsigned char device_id, unsigned char oem_sub_module)
{
#if !defined(CFG_SOC_PLATFORM_MINIV2) && !defined(CFG_SOC_PLATFORM_MDC_V51)
    struct soc_misc_node *s_dev = NULL;
    struct drv_soc_misc_ctrl *soc_misc_ctl = soc_misc_get_ctrl();

    s_dev = &soc_misc_ctl->node[device_id][0][SOC_MISC_SENSOR_L3T];

    if (oem_sub_module < RAS_L3T_NUM) {
        s_dev->work_data[oem_sub_module] = 1;
        if (soc_misc_l3t_check_unmask(oem_sub_module, s_dev->die_id)) {
            (void)schedule_delayed_work(&s_dev->unmask_irq_work, msecs_to_jiffies(300000)); /* 300000:5 minutes */
        }
    }
#endif
}

void soc_misc_l3t_fault_handler_arm(const struct notify_data *pdata)
{
    struct cper_sec_proc_arm *pdata_ras = NULL;
    struct cper_arm_err_info *arm_err_info = NULL;
    struct vendor_specific_error_info *vendor_info = NULL;
    struct ras_error error_info = { 0 };
    u32 ierr = 0;
    int ret;
    u8 cluster_id = 0;

    pdata_ras = (struct cper_sec_proc_arm *)pdata->origin_data;
    if (soc_misc_l3t_get_ierr(pdata, &ierr) != 0) {
        soc_misc_drv_err("get l3t ierr failed\n");
        return;
    }

    if (pdata->arm_error_idx >= pdata_ras->err_info_num) {
        soc_misc_drv_err("arm_error_idx is invalid. (arm_error_idx=%u)\n", pdata->arm_error_idx);
        return;
    }
    arm_err_info = (struct cper_arm_err_info *)(pdata_ras + 1) + pdata->arm_error_idx;

    if (soc_misc_arm_get_vendor_info(pdata_ras, &vendor_info)) {
        soc_misc_drv_err("failed to get vendor info\n");
        return;
    }

#ifdef CFG_FEATURE_SAFETY_MANAGER
    if (soc_misc_l3t_is_safety_fault(ierr)) {
        if (soc_misc_safety_notifier(SOC_MISC_SENSOR_L3T, pdata)) {
            soc_misc_drv_err("handle safety fault error\n");
        } else {
            soc_misc_drv_info("handle safety event success\n");
        }
        return;
    }
#endif
    /* Prevent the BIOS from obtaining the correct cluster ID due to chip exceptions.
     * If the cluster ID is invalid, use the first CPU cluster to report the fault.
     */
    ret = soc_misc_get_cpu_cluster_id(pdata_ras, &cluster_id);
    if ((ret != 0) || (cluster_id >= SOC_MISC_MAX_NODE_NUM)) {
        soc_misc_drv_warn("l3t index not found, set 0 by default. ret=%d, cluster id: %u\n", ret, cluster_id);
        cluster_id = 0;
    }

    error_info.device_id = pdata->chip_id;
    error_info.sensor_id = SOC_MISC_SENSOR_L3T;
    error_info.module_id = SOC_MISC_SENSOR_L3T;
    error_info.sub_node_id = cluster_id;
    error_info.ras_code = ierr;
    error_info.ras_err_severity = pdata_ras->affinity_level;
    error_info.sec_type = RAS_SEC_ARM;
    soc_misc_fault_event_handler(&error_info, sizeof(struct ras_error), SOC_MISC_SENSOR_L3T);

    if ((!(arm_err_info->error_info & RAS_UNMASK_CORRECTED)) && (vendor_info != NULL)) {
        l3t_relieve_suppresion(error_info.device_id, vendor_info->oem_sub_module);
    }
}

void soc_misc_l3t_fault_handler_hisi_common(const struct notify_data *pdata)
{
    u32 ierr = 0;
    struct ras_error error_info = { 0 };
    hisi_common_error_info *hisi_error = (hisi_common_error_info *)pdata->origin_data;

    if (soc_misc_l3t_get_ierr(pdata, &ierr) != 0) {
        soc_misc_drv_err("get l3t ierr failed\n");
        return;
    }

#ifdef CFG_FEATURE_SAFETY_MANAGER
    if (soc_misc_l3t_is_safety_fault(ierr)) {
        if (soc_misc_safety_notifier(SOC_MISC_SENSOR_L3T, pdata)) {
            soc_misc_drv_err("handle safety fault error\n");
        } else {
            soc_misc_drv_info("handle safety event success\n");
        }
        return;
    }
#endif

    error_info.device_id = pdata->chip_id;
    error_info.sensor_id = SOC_MISC_SENSOR_L3T;
    error_info.module_id = SOC_MISC_SENSOR_L3T;
    error_info.sub_node_id = hisi_error->device_or_core_id;
    error_info.ras_code = ierr;
    error_info.ras_err_severity = hisi_error->error_severity;
    error_info.sec_type = RAS_SEC_ARM;
    soc_misc_fault_event_handler(&error_info, sizeof(struct ras_error), SOC_MISC_SENSOR_L3T);

    if (error_info.ras_err_severity != RAS_CORRECTED_ERROR) {
        l3t_relieve_suppresion(error_info.device_id, hisi_error->device_or_core_id);
    }
}

void soc_misc_l3t_fault_handler(const struct notify_data *pdata)
{
    guid_t hisi_common_sec_type = CPER_SEC_HISI_COMMON;

    if (soc_misck_check_notify_data(pdata) != 0) {
        soc_misc_drv_err("check l3t notify data failed\n");
        return;
    }

    if (soc_misc_sec_type_guid_cmp(pdata->section_type, &hisi_common_sec_type)) {
        soc_misc_l3t_fault_handler_hisi_common(pdata);
    } else {
        soc_misc_l3t_fault_handler_arm(pdata);
    }
}

#else
void soc_misc_l3t_fault_handler(const struct notify_data *pdata)
{
    soc_misc_drv_err("l3t ras fault handle is not supported\n");
}
#endif


int soc_misc_l3t_sensor_scan(unsigned long long private_data, struct dms_sensor_event_data *data)
{
    int ret;

    ret = soc_misc_fault_event_scan(private_data, data, SOC_MISC_SENSOR_L3T);
    if (ret) {
        soc_misc_drv_err("Scan CPUcore sensor failed. (ret=%d)\n", ret);
        return ret;
    }

    return 0;
}

