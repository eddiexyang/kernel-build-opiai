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

#include "drvfault_common.h"
#include "soc_misc_err_info.h"
#include "soc_misc_init.h"
#include "soc_misc_fault.h"
#include "soc_misc_safety.h"
#include "dms_sensor_type.h"
#include "soc_misc_spec.h"
#include "soc_misc_dms_def.h"
#include "dfm_safety_report.h"
#include "l3t_config.h"
#include "l3t_dms_init.h"
#include "l3t_safety.h"

#define L3T_SAFETY_SRC            (0x0824)
#define L3T_SAFETY_CLR            (0x082C)
#define L3T_SAFETY_INT_MASK       (0x0820)

#define L3T_SAFETY_ERR_SRC        (0x0834)
#define L3T_SAFETY_ERR_CLR        (0x083C)
#define L3T_SAFETY_ERR_INT_MASK   (0x0830)

#define SOC_MISC_SAFETY_EVENT_MAX (32U)

enum l3t_safety_err_irq_status {
    L3T_CHK_FAIL_REGS = 0,
    L3T_CHK_FAIL_DATRAM = 1,
    L3T_CHK_FAIL_STDRAM = 2,
    L3T_CHK_FAIL_EXTRAM = 3,
    L3T_CHK_FAIL_DIRRAM = 4,
    L3T_CHK_FAIL_RING = 5,
    L3T_CHK_FAIL_CLUSTER = 6,
    L3T_TIMEOUT_VALID = 7,
    L3T_SAFETY_ISO_VALID = 8,
    L3T_CHK_FAIL_PKG_LOST = 9,
    L3T_SINGLE_ECC = 10,
    L3T_DOUBLE_ECC = 11,
    L3T_ADDR_DIRRAM_POISON = 12,
    L3T_ADDR_EXTRAM_POISON = 13,
    L3T_ADDR_STDRAM_POISON = 14,
    L3T_ADDR_DATRAM_POISON = 15,
    L3T_ERR_ECC_DIRRAM_DECODE = 16,
    L3T_ERR_ECC_EXTRAM_DECODE = 17,
    L3T_ERR_ECC_STDRAM_DECODE = 18,
    L3T_ERR_ECC_DATRAM_DECODE = 19
};

#define L3T_SAFETY_COVERGE_NODE(bit_id, msg, _sensor_type, ras_err) { \
    .subsys_id = DMS_DEV_TYPE_L3T,             \
    .module_id = 0,                            \
    .section_type = SOC_ERR_SECTOR_SAFETY,     \
    .ras_code.int_status = bit_id,             \
    .describe = msg,                           \
    .sensor_type = _sensor_type, \
    .error_type = ras_err, \
}

const struct ras_fault_converge_item *l3t_safety_converge(u32 section_type, u64 ras_code)
{
    static struct ras_fault_converge_item l3t_safety_converge_tab[] = {
        L3T_SAFETY_COVERGE_NODE(L3T_CHK_FAIL_REGS, "chk_fail_regs",
            DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_PARITY),
        L3T_SAFETY_COVERGE_NODE(L3T_CHK_FAIL_DATRAM, "chk_fail_datram",
            DMS_SEN_TYPE_SAFETY_SENSOR, SOC_SAFETY_LOCKSTEP_ERR),
        L3T_SAFETY_COVERGE_NODE(L3T_CHK_FAIL_STDRAM, "chk_fail_stdram",
            DMS_SEN_TYPE_SAFETY_SENSOR, SOC_SAFETY_LOCKSTEP_ERR),
        L3T_SAFETY_COVERGE_NODE(L3T_CHK_FAIL_EXTRAM, "chk_fail_extram",
            DMS_SEN_TYPE_SAFETY_SENSOR, SOC_SAFETY_LOCKSTEP_ERR),
        L3T_SAFETY_COVERGE_NODE(L3T_CHK_FAIL_DIRRAM, "chk_fail_dirram",
            DMS_SEN_TYPE_SAFETY_SENSOR, SOC_SAFETY_LOCKSTEP_ERR),
        L3T_SAFETY_COVERGE_NODE(L3T_CHK_FAIL_RING, "chk_fail_ring",
            DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_PARITY),
        L3T_SAFETY_COVERGE_NODE(L3T_CHK_FAIL_CLUSTER, "chk_fail_cluster",
            DMS_SEN_TYPE_SAFETY_SENSOR, SOC_SAFETY_LOCKSTEP_ERR),
        L3T_SAFETY_COVERGE_NODE(L3T_TIMEOUT_VALID, "timeout_valid",
            DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_TIMEOUT_ERR),
        L3T_SAFETY_COVERGE_NODE(L3T_SAFETY_ISO_VALID, "safety_iso_valid",
            DMS_SEN_TYPE_SAFETY_SENSOR, RAS_ERROR_TYPE_CFG_ERR_MINIOR),
        L3T_SAFETY_COVERGE_NODE(L3T_CHK_FAIL_PKG_LOST, "chk_fail_pkg_lost",
            DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_PARITY),
        L3T_SAFETY_COVERGE_NODE(L3T_SINGLE_ECC, "single-bit ECC",
            DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_SBECCOverThold),
        L3T_SAFETY_COVERGE_NODE(L3T_DOUBLE_ECC, "double-bit ECC",
            DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_MBECC),
        L3T_SAFETY_COVERGE_NODE(L3T_ADDR_DIRRAM_POISON, "addr_dirram_poison",
            DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_MBECC),
        L3T_SAFETY_COVERGE_NODE(L3T_ADDR_EXTRAM_POISON, "addr_extram_poison",
            DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_MBECC),
        L3T_SAFETY_COVERGE_NODE(L3T_ADDR_STDRAM_POISON, "addr_stdram_poison",
            DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_MBECC),
        L3T_SAFETY_COVERGE_NODE(L3T_ADDR_DATRAM_POISON, "addr_datram_poison",
            DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_MBECC),
        L3T_SAFETY_COVERGE_NODE(L3T_ERR_ECC_DIRRAM_DECODE, "err_ecc_dirram_decode",
            DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_MBECC),
        L3T_SAFETY_COVERGE_NODE(L3T_ERR_ECC_EXTRAM_DECODE, "err_ecc_extram_decode",
            DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_MBECC),
        L3T_SAFETY_COVERGE_NODE(L3T_ERR_ECC_STDRAM_DECODE, "err_ecc_stdram_decode",
            DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_MBECC),
        L3T_SAFETY_COVERGE_NODE(L3T_ERR_ECC_DATRAM_DECODE, "err_ecc_datram_decode",
            DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_MBECC),
    };

    u32 items_num = (u32)ARRAY_SIZE(l3t_safety_converge_tab);
    return soc_safety_converge(l3t_safety_converge_tab, items_num, ras_code);
};

#define L3T_DFX_REGS_NUM (16)
static u32 g_l3t_dfx_regs[L3T_DFX_REGS_NUM] = {
    0x0004, 0x0800, 0x0804, 0x0808, 0x080c, 0x0820, 0x0824, 0x0828,
    0x082c, 0x0830, 0x0834, 0x0838, 0x083c, 0x0840, 0x0844, 0x0848,
};

#define L3T_SAFETY_ERR_INFO(reg_base, id) {     \
    .emu_id = (CPU_CLUSTER0_EMU_ID + id),       \
    .src_id = 1,                                \
    .bit_id = 16,                               \
    .base_paddr = reg_base,                     \
    .base_size = SZ_64K,                        \
    .base_vaddr = 0,                            \
    .status = {L3T_SAFETY_ERR_SRC, 0xFFAFF},    \
    .clear = {L3T_SAFETY_ERR_CLR, 0xFFAFF},     \
    .mask = {L3T_SAFETY_ERR_INT_MASK, 0xFFAFF}, \
    .check_mask[0] = {DRVFAULT_FILED_VALID, 0x0848, 0xFFF00400}, \
    .dfx_reg_num = L3T_DFX_REGS_NUM,            \
    .dfx_reg_list = g_l3t_dfx_regs,             \
}

#define L3T_SAFETY_INT_INFO(reg_base, id) { \
    .emu_id = (CPU_CLUSTER0_EMU_ID + id),   \
    .src_id = 1,                            \
    .bit_id = 0,                            \
    .base_paddr = reg_base,                 \
    .base_size = SZ_64K,                    \
    .base_vaddr = 0,                        \
    .status = {L3T_SAFETY_SRC, 0x0100},     \
    .clear = {L3T_SAFETY_CLR, 0x0100},      \
    .mask = {L3T_SAFETY_INT_MASK, 0x0100},  \
    .check_mask[0] = {DRVFAULT_FILED_VALID, 0x0848, 0xFFF00400}, \
    .dfx_reg_num = L3T_DFX_REGS_NUM,        \
    .dfx_reg_list = g_l3t_dfx_regs,         \
}

STATIC struct safety_irq_hw_info g_l3t_safety_hwinfo[SOC_MISC_MAX_NODE_NUM][2] = {
    {
        L3T_SAFETY_INT_INFO(CPU_CLUSTER0_L3T_REG_BASE, SOC_CPU_CLUSTER_0),
        L3T_SAFETY_ERR_INFO(CPU_CLUSTER0_L3T_REG_BASE, SOC_CPU_CLUSTER_0),
    }, {
        L3T_SAFETY_INT_INFO(CPU_CLUSTER1_L3T_REG_BASE, SOC_CPU_CLUSTER_1),
        L3T_SAFETY_ERR_INFO(CPU_CLUSTER1_L3T_REG_BASE, SOC_CPU_CLUSTER_1),
    },
#ifndef CFG_SOC_PLATFORM_MDC_LITE
    {
        L3T_SAFETY_INT_INFO(CPU_CLUSTER2_L3T_REG_BASE, SOC_CPU_CLUSTER_2),
        L3T_SAFETY_ERR_INFO(CPU_CLUSTER2_L3T_REG_BASE, SOC_CPU_CLUSTER_2),
    }, {
        L3T_SAFETY_INT_INFO(CPU_CLUSTER3_L3T_REG_BASE, SOC_CPU_CLUSTER_3),
        L3T_SAFETY_ERR_INFO(CPU_CLUSTER3_L3T_REG_BASE, SOC_CPU_CLUSTER_3),
    },
#endif
};

#define L3T_FHI_INTERRUPT_HWINFO(cluster_id) {           \
    .dev_id = 0,                                         \
    .irq_type = SAFETY_IRQ_TYPE_SPI,                     \
    .irq_name = "int_fhi_l3t" NUM_TO_STR(cluster_id),    \
    .safety_irq_func = l3t_safety_handler,               \
    .irq_hwinfo_num = 2,                                 \
    .irq_hwinfo_list = g_l3t_safety_hwinfo[cluster_id]   \
}

struct safety_irq_info g_int_fhi_l3t[SOC_MISC_MAX_NODE_NUM] = {
    L3T_FHI_INTERRUPT_HWINFO(0),
    L3T_FHI_INTERRUPT_HWINFO(1),
#ifndef CFG_SOC_PLATFORM_MDC_LITE
    L3T_FHI_INTERRUPT_HWINFO(2),
    L3T_FHI_INTERRUPT_HWINFO(3)
#endif
};

int l3t_safety_init()
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
            if (drvfault_find_irq_in_dts(g_int_fhi_l3t[node_id].irq_name) != 0) {
                soc_misc_drv_info("skip registration because irq is not found in dts.(irq_name=%.*s)\n",
                    NAME_LEN, g_int_fhi_l3t[node_id].irq_name);
                continue;
            }
            ret = drvfault_register_safety_irq_info(&g_int_fhi_l3t[node_id]);
            if (ret != 0) {
                soc_misc_drv_err("l3t register safety fault notifier failed."
                    "(dev_id=%u, node_id=%u, ret=%d)\n", dev_id, node_id, ret);
            }
        }
    }

    return 0;
}

void l3t_safety_uninit()
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
            ret = drvfault_unregister_safety_irq_info(0, g_int_fhi_l3t[node_id].irq_name);
            if (ret != 0) {
                soc_misc_drv_err("l3t unregister safety fault notifier failed."
                    "(dev_id=%u, node_id=%u, ret=%d)\n", dev_id, node_id, ret);
            }
        }
    }
}

int l3t_safety_handler(struct safety_fault_info *safety_fault,
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
        sizeof(struct safety_event) * SOC_MISC_SAFETY_EVENT_MAX, GFP_KERNEL | __GFP_ACCOUNT);
    if ((*event_list) == NULL) {
        soc_misc_drv_err("kmalloc safety event list failed\n");
        return -ENOMEM;
    }

    module_info.dev_id = safety_fault->dev_id;
    module_info.node_type = DMS_DEV_TYPE_L3T;
    module_info.max_event = SOC_MISC_SAFETY_EVENT_MAX;

    for (i = 0; i < safety_fault->fault_reg_num; ++i) {
        node_id = (safety_fault->fault_status_list[i].emu_id - CPU_CLUSTER0_EMU_ID);
        if (node_id >= L3T_NODE_NUM) {
            soc_misc_drv_err("emu id invalid. (emu_id=%u)\n", safety_fault->fault_status_list[i].emu_id);
            node_id = 0;
        }
        module_info.node_id = node_id;
        module_info.dev_node = &g_l3t_dfm[safety_fault->dev_id].dev_nodes[node_id];
        module_info.dev_node->coverage_version = EVENT_COVERAGE_V1;
        module_info.dev_node->get_converage_node = l3t_safety_converge;
        module_info.section_type = SOC_ERR_SECTOR_SAFETY;
        dfm_safety_handler(&module_info, &safety_fault->fault_status_list[i], event_num, *event_list);
    }
    return 0;
}

#ifdef DRV_SOC_MISC_UT
const struct safety_irq_info *soc_misc_get_l3t_safety_irq_info(void)
{
    return g_int_fhi_l3t;
}
#endif

