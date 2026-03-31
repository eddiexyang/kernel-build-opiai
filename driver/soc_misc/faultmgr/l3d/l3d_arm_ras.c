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
#include <linux/sizes.h>
#include <linux/slab.h>
#include <linux/workqueue.h>

#include "soc_misc_err_info.h"
#include "soc_misc_init.h"
#include "soc_misc_fault.h"
#include "soc_misc_safety.h"
#include "dms_sensor_type.h"
#include "dms_define.h"
#include "receiver/fpdc_ras_receiver.h"
#include "drv_ras_common.h"
#include "soc_misc_spec.h"
#include "dfm_dev_register.h"
#include "dfm_safety_report.h"
#include "dfm_report.h"
#include "l3d_config.h"
#include "l3d_dms_init.h"
#include "l3d_arm_ras.h"
#include "l3d_arm_ras_unmask.h"

#define L3D_IERR_OFFSET      (8U)
#define RAS_UNMASK_CORRECTED (0x4000000U)
#define RAS_CORRECTED_ERROR  (0x02)

/* L3D ARM RAS IERR TYPE */
enum l3d_arm_ras_ierr {
    L3D_IERR_MULTI_BIT_ECC = 0x01,         /* multi-bit ECC error; UER */
    L3D_IERR_SINGLE_BIT_ECC = 0x02,        /* single-bit ECC error; CE */
    L3D_IERR_INVALID_SEC_ATTR = 0x03,      /* Invalid security attribute error; UEO */
    L3D_IERR_ILLEGEL_REQUEST = 0x04,       /* Illegal access request; UEU */
    L3D_IERR_ILLEGEL_READ = 0x05,          /* illegal read access; UEU */
    L3D_IERR_ILLEGEL_WRITE = 0x06,         /* illegal write access; UEU */
    L3D_IERR_ILLEGEL_THR_CHNL = 0x07,      /* accesses the thr channel illegally; UEU */
    L3D_IERR_ILLEGEL_IN_DEBUG = 0x08,      /* Unauthorized access in debug mode; UEO */
    L3D_IERR_ILLEGEL_ACCESS_ADDR = 0x09,   /* access address out-of-bounds error; UER */
    L3D_IERR_RSV_REQUEST_MISS_RULE = 0x0A, /* rsv request that does not meet the rule; UER  */
    L3D_IERR_REQUEST_ATTR_ERR = 0x0B,      /* request attribute error.UER */
    L3D_IERR_INIT_NOT_COMPOLETE = 0x0C,    /* Operation received when initialization is not complete:UEO */
};

enum l3d_arm_ras_ecc_status {
    L3D_SAFETY_FAIL_MULTI_ECC = 0,
    L3D_SAFETY_FAIL_SIGLE_ECC = 1,
};

#define L3D_ARM_RAS_COVERGE_NODE(ierr, msg, ras_err) { \
    .subsys_id = DMS_DEV_TYPE_L3D,                \
    .module_id = 0,                               \
    .section_type = RAS_SEC_ARM,                  \
    .ras_code.int_status = ierr,                  \
    .describe = msg,                              \
    .sensor_type = DMS_SEN_TYPE_RAS_SENSOR,       \
    .error_type = ras_err,                        \
}

#define L3D_ARM_RAS_ECC_COVERGE_NODE(bit_id, msg, _sensor_type, ras_err) { \
    .subsys_id = DMS_DEV_TYPE_L3D,             \
    .module_id = 0,                            \
    .section_type = SOC_ERR_SECTOR_SAFETY,     \
    .ras_code.int_status = bit_id,             \
    .describe = msg,                           \
    .sensor_type = _sensor_type,               \
    .error_type = ras_err,                     \
}

STATIC struct ras_fault_converge_item g_l3d_arm_ras_converge_tab[] = {
    L3D_ARM_RAS_COVERGE_NODE(L3D_IERR_MULTI_BIT_ECC, "multi-bit ECC error", RAS_ERROR_TYPE_MBECC),
    L3D_ARM_RAS_COVERGE_NODE(L3D_IERR_SINGLE_BIT_ECC, "single-bit ECC error", RAS_ERROR_TYPE_SBECCOverThold),
    L3D_ARM_RAS_COVERGE_NODE(L3D_IERR_INVALID_SEC_ATTR, "invalid security attribute error", RAS_ERROR_TYPE_IN_CFG_ERR),
    L3D_ARM_RAS_COVERGE_NODE(L3D_IERR_ILLEGEL_REQUEST, "illegal access request", RAS_ERROR_TYPE_IN_CFG_ERR),
    L3D_ARM_RAS_COVERGE_NODE(L3D_IERR_ILLEGEL_READ, "illegal read access", RAS_ERROR_TYPE_IN_CFG_ERR),
    L3D_ARM_RAS_COVERGE_NODE(L3D_IERR_ILLEGEL_WRITE, "illegal write access", RAS_ERROR_TYPE_IN_CFG_ERR),
    L3D_ARM_RAS_COVERGE_NODE(L3D_IERR_ILLEGEL_THR_CHNL, "unauthorized access thr channel", RAS_ERROR_TYPE_IN_CFG_ERR),
    L3D_ARM_RAS_COVERGE_NODE(L3D_IERR_ILLEGEL_IN_DEBUG, "unauthorized access in dbg mode", RAS_ERROR_TYPE_IN_CFG_ERR),
    L3D_ARM_RAS_COVERGE_NODE(L3D_IERR_ILLEGEL_ACCESS_ADDR, "access address out-of-bounds", RAS_ERROR_TYPE_IN_CFG_ERR),
    L3D_ARM_RAS_COVERGE_NODE(L3D_IERR_RSV_REQUEST_MISS_RULE, "rsv request miss rule", RAS_ERROR_TYPE_IN_CFG_ERR),
    L3D_ARM_RAS_COVERGE_NODE(L3D_IERR_REQUEST_ATTR_ERR, "request attribute error", RAS_ERROR_TYPE_IN_CFG_ERR),
    L3D_ARM_RAS_COVERGE_NODE(L3D_IERR_INIT_NOT_COMPOLETE, "initialization is not complete", RAS_ERROR_TYPE_IN_CFG_ERR),
};

const struct ras_fault_converge_item *l3d_arm_ras_converge(const struct ras_error *error_info)
{
    u32 num = sizeof(g_l3d_arm_ras_converge_tab) / sizeof(struct ras_fault_converge_item);
    return soc_misc_parse_table_handle(error_info, g_l3d_arm_ras_converge_tab, num);
}

const struct ras_fault_converge_item *l3d_arm_ras_ecc_converge(u32 section_type, u64 ras_code)
{
    static struct ras_fault_converge_item l3d_arm_ras_ecc_converge_tab[] = {
        L3D_ARM_RAS_ECC_COVERGE_NODE(L3D_SAFETY_FAIL_MULTI_ECC, "multi-bit ECC error",
                DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_MBECC),
        L3D_ARM_RAS_ECC_COVERGE_NODE(L3D_SAFETY_FAIL_SIGLE_ECC, "single-bit ECC error",
                DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_SBECCOverThold),
    };

    u32 items_num = (u32)ARRAY_SIZE(l3d_arm_ras_ecc_converge_tab);
    return soc_safety_converge(l3d_arm_ras_ecc_converge_tab, items_num, ras_code);
};

#define L3D_SAFETY_ERR_SRC       (0x0834)
#define L3D_SAFETY_ERR_CLR       (0x083C)
#define L3D_SAFETY_ERR_INT_MASK  (0x0830)
#define SOC_MISC_RAS_EVENT_MAX   (32U)

#define NUM_TO_STR(num) #num

#ifdef CFG_FEATURE_SAFETY_MANAGER
#define L3D_IRQ_HW_INFO(reg_base, id, _mask) { \
    .emu_id = (CPU_CLUSTER0_EMU_ID + id),      \
    .src_id = 1,                               \
    .bit_id = 17,                              \
    .base_paddr = reg_base,                    \
    .base_size = SZ_64K,                       \
    .base_vaddr = 0,                           \
    .status = { L3D_SAFETY_ERR_SRC, _mask },   \
    .clear = { L3D_SAFETY_ERR_CLR, _mask },    \
    .mask = { L3D_SAFETY_ERR_INT_MASK, _mask}, \
    .check_mask[0] = {DRVFAULT_FILED_VALID, 0x0848, 0x0}, \
}

STATIC struct safety_irq_hw_info g_l3d_hw_info[L3D_NODE_NUM] = {
    L3D_IRQ_HW_INFO(CPU_CLUSTER0_L3D_REG_BASE, SOC_CPU_CLUSTER_0, 0x0),
    L3D_IRQ_HW_INFO(CPU_CLUSTER1_L3D_REG_BASE, SOC_CPU_CLUSTER_1, 0x1),
    L3D_IRQ_HW_INFO(CPU_CLUSTER2_L3D_REG_BASE, SOC_CPU_CLUSTER_2, 0x1),
    L3D_IRQ_HW_INFO(CPU_CLUSTER3_L3D_REG_BASE, SOC_CPU_CLUSTER_3, 0x1),
};

#define L3D_SUBSYS_INFO_NODE(cluster_id) {             \
    .dev_id = 0,                                       \
    .irq_type = SAFETY_IRQ_TYPE_ARM_RAS,               \
    .irq_name = "int_eri_l3d" NUM_TO_STR(cluster_id),  \
    .safety_irq_func = l3d_arm_ras_irq_handler,        \
    .irq_hwinfo_num = 1,                               \
    .irq_hwinfo_list = &g_l3d_hw_info[cluster_id] }

STATIC struct safety_irq_info g_l3d_safety_info[L3D_NODE_NUM] = {
    L3D_SUBSYS_INFO_NODE(SOC_CPU_CLUSTER_0),
    L3D_SUBSYS_INFO_NODE(SOC_CPU_CLUSTER_1),
    L3D_SUBSYS_INFO_NODE(SOC_CPU_CLUSTER_2),
    L3D_SUBSYS_INFO_NODE(SOC_CPU_CLUSTER_3),
};
#endif

#ifndef AOS_LLVM_BUILD
STATIC int soc_misc_l3d_get_ierr(const struct notify_data *pdata, u32 *ierr)
{
    u32 err_status = 0;
    int ret;

    ret = soc_misc_get_err_status(pdata, &err_status);
    if (ret) {
        soc_misc_drv_err("get err status from ras data failed. (ret = %d)\n", ret);
        return ret;
    }
    *ierr = (err_status >> L3D_IERR_OFFSET) & 0xFF;

    return 0;
}

#ifdef CFG_FEATURE_SAFETY_MANAGER
STATIC bool soc_misc_l3d_is_safety_fault(u32 ierr)
{
    return ((ierr == L3D_IERR_MULTI_BIT_ECC) ||
            (ierr == L3D_IERR_SINGLE_BIT_ECC));
}
#endif

void soc_misc_l3d_fault_handler_arm(const struct notify_data *pdata)
{
    struct cper_sec_proc_arm *pdata_ras = NULL;
    struct cper_arm_err_info *arm_err_info = NULL;
    struct vendor_specific_error_info *vendor_info = NULL;
    const dfm_event *event =  NULL;
    struct ras_error error_info = { 0 };
    u32 ierr = 0;
    u32 node_id;
    int ret;

    pdata_ras = (struct cper_sec_proc_arm *)pdata->origin_data;
    if (soc_misc_l3d_get_ierr(pdata, &ierr) != 0) {
        soc_misc_drv_err("get l3d ierr failed\n");
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

    /* Prevent the BIOS from obtaining the correct node ID due to chip exceptions.
     * If the node ID is invalid, use the first node to report the fault.
     */
    node_id = (u32)(vendor_info->oem_sub_module);
    if (node_id >= L3D_NODE_NUM) {
        soc_misc_drv_warn("node id invalid, use the first node. (node id=%u)\n", node_id);
        node_id = 0;
    }
#ifdef CFG_FEATURE_SAFETY_MANAGER
    if (soc_misc_l3d_is_safety_fault(ierr)) {
        if ((drvfault_ras_irq_notify(&g_l3d_safety_info[node_id])) != 0) {
            soc_misc_drv_err("handle safety fault error\n");
        } else {
            soc_misc_drv_info("handle safety event success\n");
        }
        return;
    }
#endif
    error_info.ras_code = ierr;
    error_info.sec_type = RAS_SEC_ARM;
    event = l3d_arm_ras_converge(&error_info);
    if (event == NULL) {
        soc_misc_drv_err("l3d (node id=%u) get converge item failed.\n", node_id);
        return;
    }
    ret = dfm_add_event(&g_l3d_dfm[pdata->chip_id], node_id, event);
    if (ret != 0) {
        if (ret != -EEXIST) {
            soc_misc_drv_err("dfm_add_event failed. (ret=%d).\n", ret);
            return;
        }
    }

    if ((!(arm_err_info->error_info & RAS_UNMASK_CORRECTED)) && (vendor_info != NULL)) {
        l3d_relieve_suppresion(pdata->chip_id, vendor_info->oem_sub_module);
    }
}

void soc_misc_l3d_fault_handler_hisi_common(const struct notify_data *pdata)
{
    u32 ierr = 0;
    int ret;
    u32 node_id;
    struct ras_error error_info = { 0 };
    const dfm_event *event =  NULL;
    hisi_common_error_info *hisi_error = (hisi_common_error_info *)pdata->origin_data;

    if (soc_misc_l3d_get_ierr(pdata, &ierr) != 0) {
        soc_misc_drv_err("get l3d ierr failed\n");
        return;
    }

#ifdef CFG_FEATURE_SAFETY_MANAGER
    if (soc_misc_l3d_is_safety_fault(ierr)) {
        if ((drvfault_ras_irq_notify(&g_l3d_safety_info[hisi_error->device_or_core_id])) != 0) {
            soc_misc_drv_err("handle safety fault error\n");
        } else {
            soc_misc_drv_info("handle safety event success\n");
        }
        return;
    }
#endif
    node_id = hisi_error->device_or_core_id;
    error_info.ras_code = ierr;
    error_info.sec_type = RAS_SEC_ARM;
    event = l3d_arm_ras_converge(&error_info);
    if (event == NULL) {
        soc_misc_drv_err("l3d (node id=%u) get converge item failed.\n", node_id);
        return;
    }
    ret = dfm_add_event(&g_l3d_dfm[pdata->chip_id], node_id, event);
    if (ret != 0) {
        if (ret != -EEXIST) {
            soc_misc_drv_err("dfm_add_event failed. (ret = %d).\n", ret);
            return;
        }
    }

    if (error_info.ras_err_severity != RAS_CORRECTED_ERROR) {
        l3d_relieve_suppresion(pdata->chip_id, hisi_error->device_or_core_id);
    }
}

void l3d_arm_ras_fpdc_handler(const struct notify_data *pdata)
{
    guid_t hisi_common_sec_type = CPER_SEC_HISI_COMMON;

    if (soc_misck_check_notify_data(pdata) != 0) {
        soc_misc_drv_err("check l3d notify data failed\n");
        return;
    }

    if (soc_misc_sec_type_guid_cmp(pdata->section_type, &hisi_common_sec_type)) {
        soc_misc_l3d_fault_handler_hisi_common(pdata);
    } else {
        soc_misc_l3d_fault_handler_arm(pdata);
    }
}
#else
void l3d_arm_ras_fpdc_handler(const struct notify_data *pdata)
{
    soc_misc_drv_warn("l3d arm ras fault handle is not supported\n");
}
#endif

int l3d_arm_ras_fpdc_init(u32 dev_id)
{
    int ret;

    ret = fpdc_register_fault_notifier(DMS_DEV_TYPE_L3D, l3d_arm_ras_fpdc_handler);
    if (ret != 0) {
        soc_misc_drv_err("l3d arm ras register fpdc fault notifier failed.(ret=%d, node_type=%u)\n",
            ret, DMS_DEV_TYPE_L3D);
    }
    return 0;
}

#ifdef CFG_FEATURE_SAFETY_MANAGER
int l3d_arm_ras_irq_init(u32 dev_id)
{
    u32 soc_type = soc_misc_get_chip_type();
    u32 node_id;
    int ret;

    /*
     * In the Ascend 610, single-bit-ecc and multi-bit-ecc faults are safety faults.
     * However, the safety fault is reported through the RAS fault "interrupt line".
     *
     * During processing, the @drvfault_ras_irq_notify function needs to be used to
     * send notifications to the faultMng module. Therefore, the register information
     * and callback function of the safety fault need to be registered to the faultMng module.
     */
    if ((soc_type & CHIP_TYPE_MDC_ASCEND610) == 0) {
        return 0;
    }

    for (node_id = 0; node_id < L3D_NODE_NUM; node_id++) {
        ret = drvfault_register_safety_irq_info(&g_l3d_safety_info[node_id]);
        if (ret != 0) {
            soc_misc_drv_err("l3d arm ras register fault notify failed."
                "(dev_id=%u, node_id=%u, ret=%d)\n", dev_id, node_id, ret);
        }
    }
    return 0;
}
#endif

int l3d_arm_ras_fpdc_uninit(u32 dev_id)
{
    int ret;

    ret = fpdc_unregister_fault_notifier(DMS_DEV_TYPE_L3D);
    if (ret != 0) {
        soc_misc_drv_err("l3d arm ras register fault notify failed.(ret=%d, node_type=%u)\n",
            ret, DMS_DEV_TYPE_L3D);
    }

    return 0;
}
#ifdef CFG_FEATURE_SAFETY_MANAGER
int l3d_arm_ras_irq_uninit(u32 dev_id)
{
    u32 soc_type = soc_misc_get_chip_type();
    u32 node_id;
    int ret;

    if ((soc_type & CHIP_TYPE_MDC_ASCEND610) == 0) {
        return 0;
    }

    for (node_id = 0; node_id < L3D_NODE_NUM; node_id++) {
        ret = drvfault_unregister_safety_irq_info(dev_id, g_l3d_safety_info[node_id].irq_name);
        if (ret != 0) {
            soc_misc_drv_err("l3d arm ras unregister fault notifier failed."
                "(dev_id=%u, node_id=%u, ret=%d)\n", dev_id, node_id, ret);
        }
    }

    return 0;
}
#endif


int l3d_arm_ras_init(void)
{
    u32 dev_id;

    for (dev_id = 0; dev_id < SOC_MISC_DEVICE_NUM_MAX; dev_id++) {
        l3d_arm_ras_fpdc_init(dev_id);
#ifdef CFG_FEATURE_SAFETY_MANAGER
        l3d_arm_ras_irq_init(dev_id);
#endif
        l3d_arm_ras_unmask_init(dev_id);
    }

    return 0;
}

int l3d_arm_ras_uninit(void)
{
    u32 dev_id;

    for (dev_id = 0; dev_id < SOC_MISC_DEVICE_NUM_MAX; dev_id++) {
        l3d_arm_ras_fpdc_uninit(dev_id);
#ifdef CFG_FEATURE_SAFETY_MANAGER
        l3d_arm_ras_irq_uninit(dev_id);
#endif
        l3d_arm_ras_unmask_uninit(dev_id);
    }

    return 0;
}

int l3d_arm_ras_irq_handler(struct safety_fault_info *safety_fault,
    unsigned int *event_num, struct safety_event **event_list)
{
    struct dfm_safety_module module_info;
    u32 node_id = 0;
    u32 i;

    if (soc_chk_safety_param(safety_fault, event_num) != 0) {
        return -EINVAL;
    }

    *event_num = 0;
    *event_list = (struct safety_event *)kmalloc(
        sizeof(struct safety_event) * SOC_MISC_RAS_EVENT_MAX, GFP_KERNEL | __GFP_ACCOUNT);
    if ((*event_list) == NULL) {
        dfm_err("kmalloc safety event list failed\n");
        return -ENOMEM;
    }

    module_info.dev_id = safety_fault->dev_id;
    module_info.node_type = DMS_DEV_TYPE_L3D;
    module_info.max_event = SOC_MISC_RAS_EVENT_MAX;

    for (i = 0; i < safety_fault->fault_reg_num; ++i) {
        node_id = (safety_fault->fault_status_list[i].emu_id - CPU_CLUSTER0_EMU_ID);
        if (node_id >= L3D_NODE_NUM) {
            soc_misc_drv_err("emu id invalid. (emu_id=%u)\n", safety_fault->fault_status_list[i].emu_id);
            node_id = 0;
        }
        module_info.node_id = node_id;
        module_info.dev_node = &g_l3d_dfm[safety_fault->dev_id].dev_nodes[node_id];
        module_info.dev_node->coverage_version = EVENT_COVERAGE_V1;
        module_info.dev_node->get_converage_node = l3d_arm_ras_ecc_converge;
        module_info.section_type = SOC_ERR_SECTOR_SAFETY;
        dfm_safety_handler(&module_info, &safety_fault->fault_status_list[i], event_num, *event_list);
    }
    return 0;
}
