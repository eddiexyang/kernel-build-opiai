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

#include <linux/sizes.h>
#include <linux/slab.h>

#include "dms_node_type.h"
#include "dms_sensor_type.h"
#include "drvfault_user_common.h"
#include "dfm_safety_report.h"
#include "dfm_report.h"
#include "soc_misc_init.h"
#include "soc_misc_err_info.h"
#include "soc_misc_fault.h"
#include "soc_misc_safety.h"
#include "l3t_config.h"
#include "l3t_dms_init.h"
#include "l3t_irq_ras.h"

#define L3T_RAS_SRC              (0x2010)
#define L3T_ERR_STATUSH_IERR     (0x2010)
#define L3T_ERR_MISC1L           (0x2028)
#define L3T_ERR_MISC1H           (0x202C)
#define L3T_RAS_CHECK_MASK       (0x2008)


enum l3t_ras_err {
    L3T_RAS_MSD_OVLP_ERR0 = 0x1,         // msd_ovlp_err0
    L3T_RAS_MSD_OVLP_ERR1 = 0x2,         // msd_ovlp_err1
    L3T_RAS_DAW_OVLP_ERR0 = 0x3,         // daw_ovlp_err0
    L3T_RAS_DAW_OVLP_ERR1 = 0x4,         // daw_ovlp_err1
    L3T_RAS_REQ_DAW_MISS = 0xC2,         // req_daw_miss
    L3T_RAS_REQ_ATTR_ERROR_SQ = 0xCE,    // req_attr_error_sq
    L3T_RAS_REQ_ATTR_ERROR_SPIPE = 0x41, // req_attr_error_spipe
    L3T_RAS_REQ_ATTR_ERROR_CPIPE = 0x40, // req_attr_error_cpipe
    L3T_RAS_REQ_STASH_HIT_CPIPE = 0x05,  // req_stash_hit_cpipe
    L3T_RAS_REQ_ERR_INTERNAL = 0xC5,     // req_err_internal
    L3T_RAS_REQ_ERR_HYDRASNP = 0xC6,     // req_err_hydrasnp
    L3T_RAS_REQ_ERR_SKYROSSNP = 0xC7,    // req_err_skyrossnp
    L3T_RAS_REQ_ERR_EXTERANL = 0xC8,     // req_err_exteranl
};


/************************************************************************/
/* support RAS fault irq of l3t in BS9SX1A                              */
/************************************************************************/

#define L3T_RAS_DFX_REGS_NUM (13)
static u32 g_l3t_ras_dfx_regs[L3T_RAS_DFX_REGS_NUM] = {
    0x2000, 0x2004, 0x2008, 0x200c, 0x2010, 0x2014,
    0x2018, 0x201c, 0x2020, 0x2024, 0x2028, 0x202c, 0x2030, // print importent regs
};
#define L3T_RAS_ERR_INFO(reg_base, id) {                    \
    .emu_id = (CPU_CLUSTER0_EMU_ID + id),                   \
    .src_id = 1,                                            \
    .bit_id = 0,                                            \
    .base_paddr = reg_base,                                 \
    .base_size = SZ_64K,                                    \
    .base_vaddr = 0,                                        \
    .status[0] = {L3T_RAS_SRC, 4, 0xFFFF, 0xFFF0FFFF},      \
    .clear[0] = {L3T_ERR_STATUSH_IERR, 4, 0, 0xFFF0FFFF},   \
    .clear[1] = {L3T_ERR_MISC1L, 8, 0, 0xFFFFFFFFFFFFFFFF}, \
    .check_mask[0] = {L3T_RAS_CHECK_MASK, 4, 0x4, 0x4},     \
    .dfx_reg_num = L3T_RAS_DFX_REGS_NUM,                    \
    .dfx_reg_list = g_l3t_ras_dfx_regs,                     \
}

STATIC struct ras_irq_hw_info g_l3t_ras_hwinfo[L3T_NODE_NUM] = {
    L3T_RAS_ERR_INFO(CPU_CLUSTER0_L3T_REG_BASE, 0),
    L3T_RAS_ERR_INFO(CPU_CLUSTER1_L3T_REG_BASE, 1),
    L3T_RAS_ERR_INFO(CPU_CLUSTER2_L3T_REG_BASE, 2),
    L3T_RAS_ERR_INFO(CPU_CLUSTER3_L3T_REG_BASE, 3)
};

#define L3T_RAS_ERI_INTERRUPT_HWINFO(cluster_id) {               \
    .dev_id = 0,                                                 \
    .irq_type = RAS_IRQ_TYPE_SPI,                             \
    .irq_name = "ras_fault_l3t" NUM_TO_STR(cluster_id),          \
    .safety_irq_func = l3t_irq_ras_handler,                      \
    .irq_hwinfo_num = 1,                                         \
    .irq_hwinfo_list = &g_l3t_ras_hwinfo[cluster_id]             \
}

struct ras_irq_info g_l3t_ras[L3T_NODE_NUM] = {
    L3T_RAS_ERI_INTERRUPT_HWINFO(0),
    L3T_RAS_ERI_INTERRUPT_HWINFO(1),
    L3T_RAS_ERI_INTERRUPT_HWINFO(2),
    L3T_RAS_ERI_INTERRUPT_HWINFO(3)
};

#define L3T_RAS_COVERAGE_ITEM(_err_status, _describe, _sensor_type, _err_type) { \
    .subsys_id = DMS_DEV_TYPE_L3T,          \
    .module_id = 0,                         \
    .section_type = SOC_ERR_SECTOR_RAS,     \
    .ras_code.err_status = _err_status,     \
    .describe = _describe,                  \
    .sensor_type = _sensor_type,            \
    .error_type = _err_type                 \
}

const struct ras_fault_converge_item *l3t_irq_ras_converge(u32 section_type, u64 ras_code)
{
    static struct ras_fault_converge_item l3t_irq_ras_converage_tab[] = {
        L3T_RAS_COVERAGE_ITEM(L3T_RAS_MSD_OVLP_ERR0,
            "msd_ovlp_err0", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
        L3T_RAS_COVERAGE_ITEM(L3T_RAS_MSD_OVLP_ERR1,
            "msd_ovlp_err1", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
        L3T_RAS_COVERAGE_ITEM(L3T_RAS_DAW_OVLP_ERR0,
            "daw_ovlp_err0", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
        L3T_RAS_COVERAGE_ITEM(L3T_RAS_DAW_OVLP_ERR1,
            "daw_ovlp_err1", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
        L3T_RAS_COVERAGE_ITEM(L3T_RAS_REQ_DAW_MISS,
            "req_daw_miss", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
        L3T_RAS_COVERAGE_ITEM(L3T_RAS_REQ_ATTR_ERROR_SQ,
            "req_attr_error_sq", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
        L3T_RAS_COVERAGE_ITEM(L3T_RAS_REQ_ATTR_ERROR_SPIPE,
            "req_attr_error_spipe", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
        L3T_RAS_COVERAGE_ITEM(L3T_RAS_REQ_ATTR_ERROR_CPIPE,
            "req_attr_error_cpipe", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
        L3T_RAS_COVERAGE_ITEM(L3T_RAS_REQ_STASH_HIT_CPIPE,
            "req_stash_hit_cpipe", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
        L3T_RAS_COVERAGE_ITEM(L3T_RAS_REQ_ERR_INTERNAL,
            "req_err_internal", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
        L3T_RAS_COVERAGE_ITEM(L3T_RAS_REQ_ERR_HYDRASNP,
            "req_err_hydrasnp", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
        L3T_RAS_COVERAGE_ITEM(L3T_RAS_REQ_ERR_SKYROSSNP,
            "req_err_skyrossnp", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
        L3T_RAS_COVERAGE_ITEM(L3T_RAS_REQ_ERR_EXTERANL,
            "req_err_exteranl", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
    };

    u32 items_num = (u32)ARRAY_SIZE(l3t_irq_ras_converage_tab);
    return soc_safety_converge(l3t_irq_ras_converage_tab, items_num, ras_code);
}

int l3t_irq_ras_init()
{
    u32 soc_type = soc_misc_get_chip_type();
    u32 dev_id;
    u32 node_id;
    int ret;

    if ((soc_type & CHIP_TYPE_MDC_BS9SX1A) == 0) {
        return 0;
    }
    for (dev_id = 0; dev_id < SOC_MISC_DEVICE_NUM_MAX; dev_id++) {
        for (node_id = 0; node_id < L3T_NODE_NUM; node_id++) {
            if (drvfault_find_irq_in_dts(g_l3t_ras[node_id].irq_name) != 0) {
                soc_misc_drv_info("skip registration because irq is not found in dts.(irq_name=%.*s)\n",
                    NAME_LEN, g_l3t_ras[node_id].irq_name);
                continue;
            }
            ret = drvfault_register_ras_irq_info(&g_l3t_ras[node_id]);
            if (ret != 0) {
                soc_misc_drv_err("l3t register irq ras fault notifier failed."
                    "(dev_id=%u, node_id=%u, ret=%d)\n", dev_id, node_id, ret);
            }
        }
    }

    return 0;
}

void l3t_irq_ras_uninit()
{
    u32 soc_type = soc_misc_get_chip_type();
    u32 dev_id;
    u32 node_id;
    int ret;

    if ((soc_type & CHIP_TYPE_MDC_BS9SX1A) == 0) {
        return;
    }
    for (dev_id = 0; dev_id < SOC_MISC_DEVICE_NUM_MAX; dev_id++) {
        for (node_id = 0; node_id < L3T_NODE_NUM; node_id++) {
            ret = drvfault_unregister_ras_irq_info(0, g_l3t_ras[node_id].irq_name);
            if (ret != 0) {
                soc_misc_drv_err("l3t unregister irq ras fault notifier failed."
                    "(dev_id=%u, node_id=%u, ret=%d)\n", dev_id, node_id, ret);
            }
        }
    }
}

#define L3T_RAS_STATUS_OFFSET  (8)
#define L3T_RAS_ERR_STATUS(reg_val)  (((u32)reg_val >> L3T_RAS_STATUS_OFFSET) & 0xFF)
void l3t_irq_ras_event_report(struct dfm_safety_module* module_info,
    struct safety_fault_status *fault_status)
{
    const dfm_event *event =  NULL;
    u32 ras_ierr = L3T_RAS_ERR_STATUS(fault_status->fault_status);
    u8 cluster_id = (u8)(fault_status->emu_id - CPU_CLUSTER0_EMU_ID);
    int ret;

    event = l3t_irq_ras_converge(SOC_ERR_SECTOR_RAS, ras_ierr);
    if (event == NULL) {
        soc_misc_drv_err("l3t irq ras (node id=%d) get converge item failed.\n", cluster_id);
        return;
    }

    ret = dfm_add_event(&g_l3t_dfm[module_info->dev_id], (u32)cluster_id, event);
    if (ret != 0) {
        if (ret != -EEXIST) {
            soc_misc_drv_err("dfm_add_event failed. (ret=%d).\n", ret);
            return;
        }
    }

    return;
}

int l3t_irq_ras_handler(struct safety_fault_info *safety_fault,
    unsigned int *event_num, struct safety_event **event_list)
{
    struct dfm_safety_module module_info;
    u32 i;

    module_info.dev_id = safety_fault->dev_id;
    for (i = 0; i < safety_fault->fault_reg_num; ++i) {
        l3t_irq_ras_event_report(&module_info, &safety_fault->fault_status_list[i]);
    }
    return 0;
}
