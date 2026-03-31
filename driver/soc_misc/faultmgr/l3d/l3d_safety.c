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
#include "l3d_config.h"
#include "l3d_dms_init.h"
#include "l3d_safety.h"

#define L3D_SAFETY_SRC            (0x0824)
#define L3D_SAFETY_CLR            (0x082C)
#define L3D_SAFETY_INT_MASK       (0x0820)

#define L3D_SAFETY_ERR_SRC        (0x0834)
#define L3D_SAFETY_ERR_CLR        (0x083C)
#define L3D_SAFETY_ERR_INT_MASK   (0x0830)

#define SOC_MISC_SAFETY_EVENT_MAX (32U)

enum l3d_safety_err_irq_status {
    L3D_SAFETY_FAIL_MULTI_ECC = 0,
    L3D_SAFETY_FAIL_SIGLE_ECC = 1,
    L3D_SAFETY_FAIL_REG_PARITY = 2,
    L3D_SAFETY_FAIL_RX_CRC = 3,
    L3D_SAFETY_FAIL_TX_COMPARE = 4,
    L3D_SAFETY_FAIL_MEM_COMPARE = 5,
    L3D_SAFETY_FAIL_INFO_COMPARE = 6,
    L3D_BUFF_TIMEOUT = 7,
    L3D_FAIL_RAM_ADDR = 8,
    L3D_FAIL_RAM_DECODE = 9,
};

#define L3D_SAFETY_COVERGE_NODE(bit_id, msg, _sensor_type, ras_err) { \
    .subsys_id = DMS_DEV_TYPE_L3D,             \
    .module_id = 0,                            \
    .section_type = SOC_ERR_SECTOR_SAFETY,     \
    .ras_code.int_status = bit_id,             \
    .describe = msg,                           \
    .sensor_type = _sensor_type, \
    .error_type = ras_err, \
}

const struct ras_fault_converge_item *l3d_safety_converge(u32 section_type, u64 ras_code)
{
    static struct ras_fault_converge_item l3d_safety_converge_tab[] = {
        L3D_SAFETY_COVERGE_NODE(L3D_SAFETY_FAIL_MULTI_ECC, "multi-bit ECC error",
                DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_MBECC),
        L3D_SAFETY_COVERGE_NODE(L3D_SAFETY_FAIL_SIGLE_ECC, "single-bit ECC error",
                DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_SBECCOverThold),
        L3D_SAFETY_COVERGE_NODE(L3D_SAFETY_FAIL_REG_PARITY, "safety_fail_reg_parity_c",
                DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_PARITY),
        L3D_SAFETY_COVERGE_NODE(L3D_SAFETY_FAIL_RX_CRC, "safety_fail_rx_crc_c",
                DMS_SEN_TYPE_CHECK_SENSOR, 0x1),
        L3D_SAFETY_COVERGE_NODE(L3D_SAFETY_FAIL_TX_COMPARE, "safety_fail_tx_compare_c",
                DMS_SEN_TYPE_SAFETY_SENSOR, RAS_ERROR_TYPE_ERROR),
        L3D_SAFETY_COVERGE_NODE(L3D_SAFETY_FAIL_MEM_COMPARE, "safety_fail_mem_compare_c",
                DMS_SEN_TYPE_SAFETY_SENSOR, RAS_ERROR_TYPE_ERROR),
        L3D_SAFETY_COVERGE_NODE(L3D_SAFETY_FAIL_INFO_COMPARE, "safety_fail_info_compare_c",
                DMS_SEN_TYPE_SAFETY_SENSOR, RAS_ERROR_TYPE_ERROR),
        L3D_SAFETY_COVERGE_NODE(L3D_BUFF_TIMEOUT, "l3d_buff_timeout_c",
                DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_TIMEOUT_ERR),
        L3D_SAFETY_COVERGE_NODE(L3D_FAIL_RAM_ADDR, "safety_fail_ram_addr_c",
                DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_BUS_ERR),
        L3D_SAFETY_COVERGE_NODE(L3D_FAIL_RAM_DECODE, "safety_fail_ram_decode_c",
                DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_MBECC),
    };

    u32 items_num = (u32)ARRAY_SIZE(l3d_safety_converge_tab);
    return soc_safety_converge(l3d_safety_converge_tab, items_num, ras_code);
};

#define L3D_DFX_REGS_NUM (16)
static u32 g_l3d_dfx_regs[L3D_DFX_REGS_NUM] = {
    0x0000, 0x0800, 0x0804, 0x0808, 0x080c, 0x0820, 0x0824, 0x0828,
    0x082c, 0x0830, 0x0834, 0x0838, 0x083c, 0x0840, 0x0844, 0x0848 };

/*
 * support SPI safety irq of l3d in BS9SX1A
 */
#define L3D_SAFETY_ERR_INFO(reg_base, id) {   \
    .emu_id = (CPU_CLUSTER0_EMU_ID + id),     \
    .src_id = 1,                              \
    .bit_id = 17,                             \
    .base_paddr = reg_base,                   \
    .base_size = SZ_64K,                      \
    .base_vaddr = 0,                          \
    .status = {L3D_SAFETY_ERR_SRC, 0x3FD},    \
    .clear = {L3D_SAFETY_ERR_CLR, 0x3FD},     \
    .mask = {L3D_SAFETY_ERR_INT_MASK, 0x3FD}, \
    .check_mask[0] = {DRVFAULT_FILED_VALID, 0x0848, 0xFFFFFC02}, \
    .dfx_reg_num = L3D_DFX_REGS_NUM,          \
    .dfx_reg_list = g_l3d_dfx_regs,           \
}

#define L3D_SAFETY_INT_INFO(reg_base, id) { \
    .emu_id = (CPU_CLUSTER0_EMU_ID + id),   \
    .src_id = 1,                            \
    .bit_id = 1,                            \
    .base_paddr = reg_base,                 \
    .base_size = SZ_64K,                    \
    .base_vaddr = 0,                        \
    .status = {L3D_SAFETY_SRC, 0x00},       \
    .clear = {L3D_SAFETY_CLR, 0x00},        \
    .mask = {L3D_SAFETY_INT_MASK, 0x00},    \
    .check_mask[0] = {DRVFAULT_FILED_VALID, 0x0848, 0xFFFFFC02}, \
    .dfx_reg_num = L3D_DFX_REGS_NUM,        \
    .dfx_reg_list = g_l3d_dfx_regs,         \
}

STATIC struct safety_irq_hw_info g_l3d_safety_hwinfo[L3D_NODE_NUM][1] = {
    {
        L3D_SAFETY_ERR_INFO(CPU_CLUSTER0_L3D_REG_BASE, SOC_CPU_CLUSTER_0),
    }, {
        L3D_SAFETY_ERR_INFO(CPU_CLUSTER1_L3D_REG_BASE, SOC_CPU_CLUSTER_1),
    },
#ifndef CFG_SOC_PLATFORM_MDC_LITE
    {
        L3D_SAFETY_ERR_INFO(CPU_CLUSTER2_L3D_REG_BASE, SOC_CPU_CLUSTER_2),
    }, {
        L3D_SAFETY_ERR_INFO(CPU_CLUSTER3_L3D_REG_BASE, SOC_CPU_CLUSTER_3),
    }
#endif
};

#define L3D_FHI_INTERRUPT_HWINFO(cluster_id) {           \
    .dev_id = 0,                                         \
    .irq_type = SAFETY_IRQ_TYPE_SPI,                     \
    .irq_name = "int_fhi_l3d" NUM_TO_STR(cluster_id),    \
    .safety_irq_func = l3d_safety_handler,               \
    .irq_hwinfo_num = 1,                                 \
    .irq_hwinfo_list = g_l3d_safety_hwinfo[cluster_id]   \
}

struct safety_irq_info g_int_fhi_l3d[L3D_NODE_NUM] = {
    L3D_FHI_INTERRUPT_HWINFO(0),
    L3D_FHI_INTERRUPT_HWINFO(1),
#ifndef CFG_SOC_PLATFORM_MDC_LITE
    L3D_FHI_INTERRUPT_HWINFO(2),
    L3D_FHI_INTERRUPT_HWINFO(3)
#endif
};

#ifdef DRV_SOC_MISC_UT
const struct safety_irq_info *soc_misc_get_l3d_safety_irq_info(void)
{
    return g_int_fhi_l3d;
}
#endif

int l3d_safety_init()
{
    u32 soc_type = soc_misc_get_chip_type();
    u32 dev_id;
    u32 node_id;
    int ret;

    if ((soc_type & CHIP_TYPE_MDC_BS9SX1A) == 0) {
        return 0;
    }
    for (dev_id = 0; dev_id < SOC_MISC_DEVICE_NUM_MAX; dev_id++) {
        for (node_id = 0; node_id < L3D_NODE_NUM; node_id++) {
            if (drvfault_find_irq_in_dts(g_int_fhi_l3d[node_id].irq_name) != 0) {
                soc_misc_drv_info("skip registration because irq is not found in dts.(irq_name=%.*s)\n",
                    NAME_LEN, g_int_fhi_l3d[node_id].irq_name);
                continue;
            }
            ret = drvfault_register_safety_irq_info(&g_int_fhi_l3d[node_id]);
            if (ret != 0) {
                soc_misc_drv_err("l3d register safety fault notifier failed."
                    "(dev_id=%u, node_id=%u, ret=%d)\n", dev_id, node_id, ret);
            }
        }
    }

    return 0;
}

void l3d_safety_uninit()
{
    u32 soc_type = soc_misc_get_chip_type();
    u32 dev_id;
    u32 node_id;
    int ret;

    if ((soc_type & CHIP_TYPE_MDC_BS9SX1A) == 0) {
        return;
    }
    for (dev_id = 0; dev_id < SOC_MISC_DEVICE_NUM_MAX; dev_id++) {
        for (node_id = 0; node_id < L3D_NODE_NUM; node_id++) {
            ret = drvfault_unregister_safety_irq_info(0, g_int_fhi_l3d[node_id].irq_name);
            if (ret != 0) {
                soc_misc_drv_err("l3d unregister safety fault notifier failed."
                    "(dev_id=%u, node_id=%u, ret=%d)\n", dev_id, node_id, ret);
            }
        }
    }
}

int l3d_safety_handler(struct safety_fault_info *safety_fault,
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
    module_info.node_type = DMS_DEV_TYPE_L3D;
    module_info.max_event = SOC_MISC_SAFETY_EVENT_MAX;

    for (i = 0; i < safety_fault->fault_reg_num; ++i) {
        node_id = (safety_fault->fault_status_list[i].emu_id - CPU_CLUSTER0_EMU_ID);
        if (node_id >= L3D_NODE_NUM) {
            soc_misc_drv_err("emu id invalid. (emu_id=%u)\n", safety_fault->fault_status_list[i].emu_id);
            node_id = 0;
        }
        module_info.node_id = node_id;
        module_info.dev_node = &g_l3d_dfm[safety_fault->dev_id].dev_nodes[node_id];
        module_info.dev_node->coverage_version = EVENT_COVERAGE_V1;
        module_info.dev_node->get_converage_node = l3d_safety_converge;
        module_info.section_type = SOC_ERR_SECTOR_SAFETY;
        dfm_safety_handler(&module_info, &safety_fault->fault_status_list[i], event_num, *event_list);
    }
    return 0;
}

