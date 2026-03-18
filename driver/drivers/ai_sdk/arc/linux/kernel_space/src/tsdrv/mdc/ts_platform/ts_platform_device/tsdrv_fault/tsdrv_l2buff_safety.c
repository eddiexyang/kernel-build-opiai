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

#include "tsdrv_fault_init.h"
#include "tsdrv_l2buff_safety.h"
#include "tsdrv_common.h"
#include "tsdrv_log.h"
#include "dms_node_type.h"
#include "dms_sensor_type.h"
#include "fpdc_ras_receiver.h"
#include "drvfault_user_common.h"
#include "tsdrv_safety_fault.h"
#include "dfm_dev_register.h"
#include "dfm_report.h"
#include "dfm_safety_report.h"
#include "drv_ras_common.h"

#ifdef CFG_SOC_PLATFORM_MDC_V51
/**
 * Raw secure interrupt register status,
 * Each error status corresponds to one bit of the register.
 */
enum l2buff_safety_err_irq_status {
    L2BUFF_SAFETY_FAIL_MULTI_ECC = 0,
    L2BUFF_SAFETY_FAIL_SIGLE_ECC = 1,
    L2BUFF_SAFETY_ASIL_B_SKYROS_TX_CRC = 2,
    L2BUFF_SAFETY_ASIL_B_BUFF_TIMEOUT = 3,
};
#endif

/**
 * safety fault src id
 * every fault has it's own src id
 */
#define L2BUFF_SAFETY_SRC_ID   1
#define L2BUFF_SAFETY_BITID_10   (10U)
#define L2BUFF_SAFETY_BITID_11   (11U)
#define L2BUFF_SAFETY_BITID_26   (26U)
#define L2BUFF_SAFETY_BITID_27   (27U)
#define REGISTER_LENGTH          (32U)

/**
 * l2buff base addr
 */
#define L2BUFF_NUM    (8U)
#define L2BUFF0_BASE  (0xA0500000U)
#define L2BUFF1_BASE  (0xA0510000U)
#define L2BUFF2_BASE  (0xA0520000U)
#define L2BUFF3_BASE  (0xA0530000U)
#define L2BUFF4_BASE  (0xA0540000U)
#define L2BUFF5_BASE  (0xA0550000U)
#define L2BUFF6_BASE  (0xA0560000U)
#define L2BUFF7_BASE  (0xA0570000U)

/**
 * int_safety_l2buff interrupt status offset
 */
#define L2BUFF_SAFETY_INT_SRC      0x0824
#define L2BUFF_SAFETY_INT_CLR      0x082C
#define L2BUFF_SAFETY_INT_MASK     0x0820

#ifdef CFG_FEATURE_FAULT_FPDC
/* offset between devices */
#ifndef DEVICE_OFFSET
#define DEVICE_OFFSET 0x8000000000
#endif

#define TO_STRING(id) #id

/************************************************************************/
/* L2BUFF RAS FAULT                                                     */
/************************************************************************/

#ifdef CFG_SOC_PLATFORM_MDC_V51
#define L2BUFF_IRQ_HW_INFO(subsys_id, reg_base, irq_bit) { \
    .emu_id = subsys_id,                       \
    .src_id = L2BUFF_SAFETY_SRC_ID,            \
    .bit_id = irq_bit,                         \
    .base_paddr = reg_base,                    \
    .base_size = SZ_64K,                       \
    .base_vaddr = 0,                           \
    .status = { L2BUFF_SAFETY_INT_SRC, 0x01 }, \
    .clear = { L2BUFF_SAFETY_INT_CLR, 0x01 },  \
    .mask = { L2BUFF_SAFETY_INT_MASK, 0x01 },  \
    .check_mask[0] = {DRVFAULT_FILED_VALID, 0x0848, 0xFFFFFFFE}, \
}

STATIC struct safety_irq_hw_info g_l2buff_irq_hwinfo[TSDRV_MAX_DAVINCI_NUM][L2BUFF_NUM] = {
    {
        L2BUFF_IRQ_HW_INFO(AIVECTOR0_EMU_ID, L2BUFF0_BASE, L2BUFF_SAFETY_BITID_11),
        L2BUFF_IRQ_HW_INFO(AIVECTOR2_EMU_ID, L2BUFF1_BASE, L2BUFF_SAFETY_BITID_11),
        L2BUFF_IRQ_HW_INFO(AICORE2_EMU_ID, L2BUFF2_BASE, L2BUFF_SAFETY_BITID_10),
        L2BUFF_IRQ_HW_INFO(AICORE2_EMU_ID, L2BUFF3_BASE, L2BUFF_SAFETY_BITID_11),
        L2BUFF_IRQ_HW_INFO(AICORE3_EMU_ID, L2BUFF4_BASE, L2BUFF_SAFETY_BITID_10),
        L2BUFF_IRQ_HW_INFO(AICORE4_EMU_ID, L2BUFF5_BASE, L2BUFF_SAFETY_BITID_10),
        L2BUFF_IRQ_HW_INFO(AICORE5_EMU_ID, L2BUFF6_BASE, L2BUFF_SAFETY_BITID_11),
        L2BUFF_IRQ_HW_INFO(AICORE9_EMU_ID, L2BUFF7_BASE, L2BUFF_SAFETY_BITID_11),
    }, {
        L2BUFF_IRQ_HW_INFO(AIVECTOR0_EMU_ID, (L2BUFF0_BASE + DEVICE_OFFSET), L2BUFF_SAFETY_BITID_11),
        L2BUFF_IRQ_HW_INFO(AIVECTOR2_EMU_ID, (L2BUFF1_BASE + DEVICE_OFFSET), L2BUFF_SAFETY_BITID_11),
        L2BUFF_IRQ_HW_INFO(AICORE2_EMU_ID, (L2BUFF2_BASE + DEVICE_OFFSET), L2BUFF_SAFETY_BITID_10),
        L2BUFF_IRQ_HW_INFO(AICORE2_EMU_ID, (L2BUFF3_BASE + DEVICE_OFFSET), L2BUFF_SAFETY_BITID_11),
        L2BUFF_IRQ_HW_INFO(AICORE3_EMU_ID, (L2BUFF4_BASE + DEVICE_OFFSET), L2BUFF_SAFETY_BITID_10),
        L2BUFF_IRQ_HW_INFO(AICORE4_EMU_ID, (L2BUFF5_BASE + DEVICE_OFFSET), L2BUFF_SAFETY_BITID_10),
        L2BUFF_IRQ_HW_INFO(AICORE5_EMU_ID, (L2BUFF6_BASE + DEVICE_OFFSET), L2BUFF_SAFETY_BITID_11),
        L2BUFF_IRQ_HW_INFO(AICORE9_EMU_ID, (L2BUFF7_BASE + DEVICE_OFFSET), L2BUFF_SAFETY_BITID_11),
    },
};

/**
 * ras_info.ras_irq_bit equal to index, because L2BUFF0's BitMap is 0
 */
#define L2BUFF_SAFETY_IRQ_INFO(devid, index) { \
    .dev_id = 0,                                        \
    .irq_type = SAFETY_IRQ_TYPE_ARM_RAS,                \
    .irq_name = "int_eri_l2buff" TO_STRING(index),      \
    .safety_irq_func = tsdrv_l2buff_safety_handler,     \
    .irq_hwinfo_num = 1,                                \
    .irq_hwinfo_list = &g_l2buff_irq_hwinfo[devid][index] \
}

#define GEN_DEV_L2BUFF_SAFETY_INFO(devid) { \
    L2BUFF_SAFETY_IRQ_INFO(devid, 0),       \
    L2BUFF_SAFETY_IRQ_INFO(devid, 1),       \
    L2BUFF_SAFETY_IRQ_INFO(devid, 2),       \
    L2BUFF_SAFETY_IRQ_INFO(devid, 3),       \
    L2BUFF_SAFETY_IRQ_INFO(devid, 4),       \
    L2BUFF_SAFETY_IRQ_INFO(devid, 5),       \
    L2BUFF_SAFETY_IRQ_INFO(devid, 6),       \
    L2BUFF_SAFETY_IRQ_INFO(devid, 7)        \
}

STATIC struct safety_irq_info g_l2buff_ras_safety_info[TSDRV_MAX_DAVINCI_NUM][L2BUFF_NUM] = {
    GEN_DEV_L2BUFF_SAFETY_INFO(0),
    GEN_DEV_L2BUFF_SAFETY_INFO(1),
};
#endif
#endif

#ifdef CFG_FEATURE_SAFETY_FAULT
/*
 * int_safety_err_l2buff interrupt status offset
 */
#define L2BUFF_SAFETY_ERR_SRC      0x0834
#define L2BUFF_SAFETY_ERR_CLR      0x083C
#define L2BUFF_SAFETY_ERR_MASK     0x0830

/************************************************************************/
/* L2BUFF SAFETY FAULT                                                  */
/************************************************************************/
#define L2BUFF_SAFETY_ERR_HWINFO(subsys_id, reg_base, irq_bit) { \
    .emu_id = subsys_id,                       \
    .src_id = L2BUFF_SAFETY_SRC_ID,            \
    .bit_id = irq_bit,                         \
    .base_paddr = reg_base,                    \
    .base_size = SZ_64K,                       \
    .base_vaddr = 0,                           \
    .status = { L2BUFF_SAFETY_ERR_SRC, 0x0C }, \
    .clear = { L2BUFF_SAFETY_ERR_CLR, 0x0C },  \
    .mask = { L2BUFF_SAFETY_ERR_MASK, 0x0C },  \
    .check_mask[0] = {DRVFAULT_FILED_VALID, 0x0848, 0xFFFFFFE2}}

#define L2BUFF_SAFETY_INT_HWINFO(subsys_id, reg_base, irq_bit) { \
    .emu_id = subsys_id,                       \
    .src_id = L2BUFF_SAFETY_SRC_ID,            \
    .bit_id = irq_bit,                         \
    .base_paddr = reg_base,                    \
    .base_size = SZ_64K,                       \
    .base_vaddr = 0,                           \
    .status = { L2BUFF_SAFETY_INT_SRC, 0x03 }, \
    .clear = { L2BUFF_SAFETY_INT_CLR, 0x03 },  \
    .mask = { L2BUFF_SAFETY_INT_MASK, 0x03 },  \
    .check_mask[0] = {DRVFAULT_FILED_VALID, 0x0848, 0xFFFFFFE2}}

#endif

static struct dfm_struct l2buff_dms[TSDRV_MAX_DAVINCI_NUM];

int tsdrv_register_l2buff_safety_irq(u32 devid)
{
    unsigned int irq_num = 0;
    struct safety_irq_info *irq_infos = 0;

    if (tsdrv_register_l2buff_dms_node(devid) != 0) {
        TSDRV_PRINT_ERR("register l2buff dms node failed. devid(%d)\n", devid);
        return -EFAULT;
    }

    if (tsdrv_get_chip_type() != CHIP_TYPE_MDC_BS9SX1A) {
        irq_num = L2BUFF_NUM;
        irq_infos = g_l2buff_ras_safety_info[devid];
        if (dfm_register_safety_irq_hwinfo(devid, irq_infos, irq_num) != 0) {
            TSDRV_PRINT_ERR("register ras safety irq info failed. devid(%d)\n", devid);
        }
    }
    return 0;
}

void tsdrv_unregister_l2buff_safety_irq(u32 devid)
{
    unsigned int irq_num = 0;

    if (tsdrv_get_chip_type() != CHIP_TYPE_MDC_BS9SX1A) {
        irq_num = L2BUFF_NUM;
        dfm_unregister_safety_irq_hwinfo(devid, g_l2buff_ras_safety_info[devid], irq_num);
    }
    tsdrv_unregister_l2buff_dms_node(devid);
}

#define L2BUFF_SAFETY_MAX_NUM  (4U)

#ifdef CFG_SOC_PLATFORM_MDC_V51
#define L2BUFF_SAFETY_COVERAGE_ITEM(_err_status, _describe, _sensor_type, _err_type) { \
    .subsys_id = DMS_DEV_TYPE_L2BUF, \
    .module_id = SENSOR_NODE_L2BUF, \
    .section_type = RAS_SEC_OEM, \
    .ras_code.err_status = _err_status, \
    .describe = _describe, \
    .sensor_type = _sensor_type, \
    .error_type = _err_type \
}
STATIC const struct ras_fault_converge_item *l2buff_safety_converge(u32 section_type, u64 ras_code)
{
    static struct ras_fault_converge_item l2buff_converage_tab[] = {
        L2BUFF_SAFETY_COVERAGE_ITEM(L2BUFF_SAFETY_FAIL_MULTI_ECC,
            "Multi-bit ECC error", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_MBECC),
        L2BUFF_SAFETY_COVERAGE_ITEM(L2BUFF_SAFETY_FAIL_SIGLE_ECC,
            "Single-bit ECC error", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_SBECCOverThold),
        L2BUFF_SAFETY_COVERAGE_ITEM(L2BUFF_SAFETY_ASIL_B_SKYROS_TX_CRC,
            "CRC error in the RX channel", DMS_SEN_TYPE_CHECK_SENSOR, 0x1),
        L2BUFF_SAFETY_COVERAGE_ITEM(L2BUFF_SAFETY_ASIL_B_BUFF_TIMEOUT,
            "Buffer timeout error", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_TIMEOUT_ERR),
    };

    u32 items_num = (u32)ARRAY_SIZE(l2buff_converage_tab);
    return tsdrv_safety_converge(l2buff_converage_tab, items_num, section_type, ras_code);
}
#endif

STATIC int l2buff_get_node_id(unsigned long long base_paddr, u32 *node_id)
{
    if (base_paddr < L2BUFF0_BASE || base_paddr > L2BUFF7_BASE) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("base addr %pK is not belong to l2buff\n", (void *)base_paddr);
        return -EEXIST;
#endif
    }

    /* Calculate the number of the l2buff based on the base address. */
    *node_id = (base_paddr >> SZ_16) - (L2BUFF0_BASE >> SZ_16);
    return 0;
}

int tsdrv_l2buff_safety_handler(struct safety_fault_info *safety_fault,
    unsigned int *event_num, struct safety_event **event_list)
{
    struct dfm_safety_module module_info;
    u32 node_id = 0;
    u32 i;

#ifdef CFG_SOC_PLATFORM_MDC_V51
    if (tsdrv_chk_safety_param(safety_fault, event_num) != 0) {
        return -EINVAL;
    }
#endif

    *event_num = 0;
    *event_list = (struct safety_event *)kmalloc(
        sizeof(struct safety_event) * L2BUFF_SAFETY_MAX_NUM, GFP_KERNEL);
    if ((*event_list) == NULL) {
#ifndef TSDRV_UT
        dfm_err("kmalloc safety event list failed\n");
        return -ENOMEM;
#endif
    }

    module_info.dev_id = safety_fault->dev_id;
    module_info.node_type = DMS_DEV_TYPE_L2BUF;
    module_info.max_event = L2BUFF_SAFETY_MAX_NUM;

    for (i = 0; i < safety_fault->fault_reg_num; ++i) {
        if (l2buff_get_node_id(safety_fault->fault_status_list[i].base_paddr, &node_id) != 0) {
#ifndef TSDRV_UT
            TSDRV_PRINT_ERR("get node id failed.(base addr=%pK)\n",
                (void *)safety_fault->fault_status_list[i].base_paddr);
            continue;
#endif
        }

        module_info.node_id = node_id;
        module_info.dev_node = &l2buff_dms[safety_fault->dev_id].dev_nodes[node_id];
        module_info.section_type = RAS_SEC_OEM;
        dfm_safety_handler(&module_info, &safety_fault->fault_status_list[i], event_num, *event_list);
    }
    return 0;
}

int l2buff_safety_notifier(u8 sensor_id, const struct notify_data *pdata)
{
#ifndef TSDRV_UT
    struct sec_oem_error *pdata_oem = NULL;
#if defined(CFG_SOC_PLATFORM_MDC_V51) && defined(CFG_FEATURE_FAULT_FPDC)
    int ret;
#endif

    if (pdata == NULL || pdata->origin_data == NULL) {
        return -EINVAL;
    }

    pdata_oem = (struct sec_oem_error *)(pdata->origin_data);
    TSDRV_PRINT_DEBUG("soc:%u, socket id:%u, nimbus id:%u, module id:%u, submodule id:%u\n",
        pdata_oem->soc_id, pdata_oem->socket_id, pdata_oem->nimbus_id,
        pdata_oem->module_id, pdata_oem->submodule_id);
    if (pdata_oem->submodule_id >= L2BUFF_NUM) {
        TSDRV_PRINT_ERR("invalid l2buff id(%u)\n", pdata_oem->submodule_id);
        return -EINVAL;
    }

#if defined(CFG_SOC_PLATFORM_MDC_V51) && defined(CFG_FEATURE_FAULT_FPDC)
    ret = drvfault_ras_irq_notify(&g_l2buff_ras_safety_info[pdata->chip_id][pdata_oem->submodule_id]);
    if (ret != 0) {
        TSDRV_PRINT_ERR("notify fault drv of fault failed. ret(%d)\n", ret);
        return -EFAULT;
    }
#endif
#endif
    return 0;
}

#define L2BUFF_DMS_NODE_DEFINE(_devid, _id, _ops) { \
    .node_type = DMS_DEV_TYPE_L2BUF,        \
    .node_id = _id,                         \
    .node_name = "L2BUF-" #_id,             \
    .capacity = 0x1,                        \
    .permission = 0x1,                      \
    .owner_devid = _devid,                  \
    .ops = _ops                             \
}

#define GEN_DEV_L2BUFF_DMS_NODE(_devid) { \
    L2BUFF_DMS_NODE_DEFINE(_devid, 0, &tsdrv_dev_node_ops), \
    L2BUFF_DMS_NODE_DEFINE(_devid, 1, &tsdrv_dev_node_ops), \
    L2BUFF_DMS_NODE_DEFINE(_devid, 2, &tsdrv_dev_node_ops), \
    L2BUFF_DMS_NODE_DEFINE(_devid, 3, &tsdrv_dev_node_ops), \
    L2BUFF_DMS_NODE_DEFINE(_devid, 4, &tsdrv_dev_node_ops), \
    L2BUFF_DMS_NODE_DEFINE(_devid, 5, &tsdrv_dev_node_ops), \
    L2BUFF_DMS_NODE_DEFINE(_devid, 6, &tsdrv_dev_node_ops), \
    L2BUFF_DMS_NODE_DEFINE(_devid, 7, &tsdrv_dev_node_ops)  \
}

static struct dms_node g_l2buff_dms_nodes[TSDRV_MAX_DAVINCI_NUM][L2BUFF_NUM] = {
    GEN_DEV_L2BUFF_DMS_NODE(0),
    GEN_DEV_L2BUFF_DMS_NODE(1),
};

STATIC int tsdrv_scan_l2buff_event(u64 private_data, struct dms_sensor_event_data *data)
{
    u32 devid = get_dev_id_from_priv(private_data);
    u32 node_id = get_dev_node_from_priv(private_data);
    u32 sensor_type = get_sensor_node_from_priv(private_data);

    if (node_id >= l2buff_dms[devid].node_num) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("invalid node id. (node_id=%u, priv=0x%llx)\n", node_id, private_data);
        return -EINVAL;
#endif
    }

    (void)dfm_scan_events(&l2buff_dms[devid], node_id, (u8)sensor_type, data);
    return 0;
}

static inline const struct sec_oem_error *ghes_data_to_oem_error(
    const struct notify_data *pdata)
{
    return (const struct sec_oem_error *)(pdata->origin_data);
}

STATIC bool l2buff_fault_is_safety_fault(u32 ierr)
{
#ifdef CFG_SOC_PLATFORM_MDC_V51
    if (ierr == RAS_SRAM_MULTI_BIT_ECC_UER ||
        ierr == RAS_SRAM_SINGLE_BIT_ECC_CE) {
        return true;
    }
#endif
    return false;
}

void l2buff_ras_fpdc_handler(const struct notify_data *pdata)
{
    u32 section_type, ras_code, node_id;
    const struct ras_fault_converge_item *converage_node = NULL;
    int ret;

    if (pdata == NULL || pdata->origin_data == NULL) {
        TSDRV_PRINT_ERR("pdata is null\n");
        return;
    } else if (pdata->chip_id > fault_ctrl_get_num()) {
        TSDRV_PRINT_ERR("invalid chip id(%u)\n", pdata->chip_id);
        return;
    }

    section_type = tsdrv_get_section_type(pdata->node_type);
    node_id = ghes_data_to_oem_error(pdata)->submodule_id;
    ras_code = tsdrv_parse_ras_code(pdata);
    if (pdata->node_type != DMS_DEV_TYPE_L2BUF) {
        TSDRV_PRINT_ERR("invalid node type(node type=0x%x.\n", pdata->node_type);
        return;
    }
    if (node_id >= L2BUFF_NUM) {
        TSDRV_PRINT_ERR("invalid l2buff node id(%u)\n", node_id);
        return;
    }
    TSDRV_PRINT_DEBUG("Debug. (src_type=%d; node_type=0x%x; devid=%u; ras_code=0x%x; data_len=0x%x)\n",
        pdata->src_type, pdata->node_type, pdata->chip_id, ras_code, pdata->data_len);

    converage_node = tsdrv_fault_converge(section_type, (u64)ras_code);
    if (converage_node == NULL) {
        TSDRV_PRINT_ERR("Invalid fault type. (section_type=%u; ras_code=0x%x; devid=%u; node_id=%u)\n",
            section_type, ras_code, pdata->chip_id, node_id);
        return;
    }

    /* L2BUFF single-bit/double-bit ecc error reported by safety */
    if (l2buff_fault_is_safety_fault(ras_code)) {
        if (l2buff_safety_notifier(DMS_DEV_TYPE_L2BUF, pdata) != 0) {
            TSDRV_PRINT_ERR("Handle l2buff safety fault error.\n");
        } else {
            TSDRV_PRINT_INFO("Handle l2buff safety success.\n");
        }
        return;
    }

    ret = dfm_add_event(&l2buff_dms[pdata->chip_id], node_id, converage_node);
    if ((ret != 0) && (ret != -EEXIST)) {
#ifndef TSDRV_UT
        TSDRV_PRINT_ERR("Process fault event info failed. (devid=%u; node_id=%u; tsid=%u; ret=%d)\n",
            pdata->chip_id, node_id, 0, ret);
        return;
#endif
    }
}

/* if sensor scan_module is DMS_SERSOR_SCAN_NOTIFY the scan_interval must be set to 0 */
/* All failures of safety source and RAS source are reported immediately */
static struct dms_sensor_object_cfg l2buff_sensor_cfg[] = {
    /* l2buf_ras */
    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_RAS_SENSOR, "l2buf_ras", DMS_DISCRETE_SENSOR_CLASS,
        DMS_SENSOR_ATTRIB_THRES_NONE, 0, 0,
        DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, tsdrv_scan_l2buff_event,
        SENSOR_PRIV_DATA(0, 0, 0, DMS_SEN_TYPE_RAS_SENSOR), 0xFFFF, 0xFFBF),
    /* l2buf_check */
    GEN_SENOR_TABLE_OBJ(DMS_SEN_TYPE_CHECK_SENSOR, "l2buf_check", DMS_DISCRETE_SENSOR_CLASS,
        DMS_SENSOR_ATTRIB_THRES_NONE, 0, 0,
        DMS_SENSOR_PROC_ENABLE_FLAG, DMS_SENSOR_ENABLE_FALG, tsdrv_scan_l2buff_event,
        SENSOR_PRIV_DATA(0, 0, 0, DMS_SEN_TYPE_CHECK_SENSOR), 0xFFFF, 0xFFBF)
};

#ifdef CFG_SOC_PLATFORM_MDC_V51
static struct irqnum2emuid irqnum2emuidarray[] = {
    {L2BUFF0, AIVECTOR0_EMU_ID},
    {L2BUFF1, AIVECTOR2_EMU_ID},
    {L2BUFF2, AICORE2_EMU_ID},
    {L2BUFF3, AICORE2_EMU_ID},
    {L2BUFF4, AICORE3_EMU_ID},
    {L2BUFF5, AICORE4_EMU_ID},
    {L2BUFF6, AICORE5_EMU_ID},
    {L2BUFF7, AICORE9_EMU_ID}
};

#define GEN_L2BUFF_FAULT_ITEM_RAS(_int_status, _describe, _sensor_type, _err_type) \
    { \
        .subsys_id = DMS_DEV_TYPE_L2BUF, \
        .module_id = SENSOR_NODE_L2BUF, \
        .section_type = L2BUFF_SECTION_TYPE_RAS, \
        .ras_code.int_status = _int_status, \
        .describe = _describe, \
        .sensor_type = _sensor_type, \
        .error_type = _err_type \
    }

#define GEN_L2BUFF_FAULT_ITEM_SAFETY(_int_status, _describe, _sensor_type, _err_type) \
    { \
        .subsys_id = DMS_DEV_TYPE_L2BUF, \
        .module_id = SENSOR_NODE_L2BUF, \
        .section_type = L2BUFF_SECTION_TYPE_SAFETY, \
        .ras_code.int_status = _int_status, \
        .describe = _describe, \
        .sensor_type = _sensor_type, \
        .error_type = _err_type \
    }

STATIC const struct ras_fault_converge_item *l2buff_get_converge_node(
    const struct ras_fault_converge_item *items, u32 items_num, u32 section_type, u64 ras_code)
{
    u32 i;
    for (i = 0; i < items_num; i++) {
        if (section_type == items[i].section_type && ras_code == items[i].ras_code.int_status) {
            return &items[i];
        }
    }
    return NULL;
}

STATIC const struct ras_fault_converge_item *l2buff_converge_bs9sx1a(u32 section_type, u64 ras_code)
{
    static struct ras_fault_converge_item l2buff_converge_table_bs9sx1a[] = {
        /* ras 源 */
        GEN_L2BUFF_FAULT_ITEM_RAS(L2BUFF_RAS_ILLEGAL_SECURITY_ATTRIBUTE_ERR,
            "illegal_security_attribute_error", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
        GEN_L2BUFF_FAULT_ITEM_RAS(L2BUFF_RAS_ILLEGAL_REQUEST_ACCESS,
            "illegal_request_access", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
        GEN_L2BUFF_FAULT_ITEM_RAS(L2BUFF_RAS_ILLEGAL_READ_ACCESS,
            "illegal_read_access", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
        GEN_L2BUFF_FAULT_ITEM_RAS(L2BUFF_RAS_ILLEGAL_WRITE_ACCESS,
            "illegal_write_access", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
        GEN_L2BUFF_FAULT_ITEM_RAS(L2BUFF_RAS_ILLEGAL_ACCESS_THR_CHANNEL,
            "illegal_access_thr_channel", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
        GEN_L2BUFF_FAULT_ITEM_RAS(L2BUFF_RAS_DEBUG_ILLEGAL_ACCESS,
            "debug_illegal_access", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
        GEN_L2BUFF_FAULT_ITEM_RAS(L2BUFF_RAS_ACCESS_ADD_OUT_BOUNDS,
            "access_add_out_bounds", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
        GEN_L2BUFF_FAULT_ITEM_RAS(L2BUFF_RAS_NON_COMPLIANT_RSV_REQUEST,
            "non_compliant_rsv_request", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
        GEN_L2BUFF_FAULT_ITEM_RAS(L2BUFF_RAS_REQUEST_ATTRIBUTE_ERR,
            "request_attribute_error", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
        GEN_L2BUFF_FAULT_ITEM_RAS(L2BUFF_RAS_INIT_NOT_COMPLETE,
            "init_not_complete", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
        GEN_L2BUFF_FAULT_ITEM_RAS(L2BUFF_RAS_ATOMIC_CALCULATION_ERR,
            "atomic_calculation_error", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR),
        /* Safety 源 */
        GEN_L2BUFF_FAULT_ITEM_SAFETY(L2BUFF_SAFETY_BIT0,
            "Multi-bit ECC error", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_MBECC),
        GEN_L2BUFF_FAULT_ITEM_SAFETY(L2BUFF_SAFETY_BIT2,
            "Parity error", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_PARITY),
        GEN_L2BUFF_FAULT_ITEM_SAFETY(L2BUFF_SAFETY_BIT3,
            "CRC check fail", DMS_SEN_TYPE_CHECK_SENSOR, 0x01),
        GEN_L2BUFF_FAULT_ITEM_SAFETY(L2BUFF_SAFETY_BIT4,
            "Service timeout", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_TIMEOUT_ERR),
    };
    u32 items_num = (u32)ARRAY_SIZE(l2buff_converge_table_bs9sx1a);
    return l2buff_get_converge_node(l2buff_converge_table_bs9sx1a, items_num, section_type, ras_code);
}

static struct l2buff_safety_faultinfo l2buff_safety_faultinfo_array[] = {
    {0,
     RAS_ERROR_TYPE_MBECC,
     3,
     DMS_SEN_TYPE_RAS_SENSOR},
    {2,
     RAS_ERROR_TYPE_PARITY,
     3,
     DMS_SEN_TYPE_RAS_SENSOR},
    {3,
     0x01,
     3,
     DMS_SEN_TYPE_CHECK_SENSOR},
    {4,
     RAS_ERROR_TYPE_TIMEOUT_ERR,
     3,
     DMS_SEN_TYPE_RAS_SENSOR}
};

static struct l2buff_safety_silsinfo l2buff_safety_bit_sils_int[] = {
    {L2BUFF0, L2BUFF_SAFETY_BITID_11 % REGISTER_LENGTH},
    {L2BUFF1, L2BUFF_SAFETY_BITID_11 % REGISTER_LENGTH},
    {L2BUFF2, L2BUFF_SAFETY_BITID_10 % REGISTER_LENGTH},
    {L2BUFF3, L2BUFF_SAFETY_BITID_11 % REGISTER_LENGTH},
    {L2BUFF4, L2BUFF_SAFETY_BITID_10 % REGISTER_LENGTH},
    {L2BUFF5, L2BUFF_SAFETY_BITID_10 % REGISTER_LENGTH},
    {L2BUFF6, L2BUFF_SAFETY_BITID_11 % REGISTER_LENGTH},
    {L2BUFF7, L2BUFF_SAFETY_BITID_11 % REGISTER_LENGTH}
};

static struct l2buff_safety_silsinfo l2buff_safety_bit_sils_err[] = {
    {L2BUFF0, L2BUFF_SAFETY_BITID_27 % REGISTER_LENGTH},
    {L2BUFF1, L2BUFF_SAFETY_BITID_27 % REGISTER_LENGTH},
    {L2BUFF2, L2BUFF_SAFETY_BITID_26 % REGISTER_LENGTH},
    {L2BUFF3, L2BUFF_SAFETY_BITID_27 % REGISTER_LENGTH},
    {L2BUFF4, L2BUFF_SAFETY_BITID_26 % REGISTER_LENGTH},
    {L2BUFF5, L2BUFF_SAFETY_BITID_26 % REGISTER_LENGTH},
    {L2BUFF6, L2BUFF_SAFETY_BITID_27 % REGISTER_LENGTH},
    {L2BUFF7, L2BUFF_SAFETY_BITID_27 % REGISTER_LENGTH}
};

static uint32_t l2buff_get_silsbit_by_nodeid(unsigned char node_id, unsigned int bit)
{
    if (bit == 0) {
        return l2buff_safety_bit_sils_int[node_id].bitid_sils;
    } else {
        return l2buff_safety_bit_sils_err[node_id].bitid_sils;
    }
}

static void l2buff_report_safety_fault_to_sils(unsigned int emu_id, unsigned char node_id,
    struct l2buff_safety_faultinfo l2buff_fault_info)
{
    uint32_t event_num = 1;
    struct safety_event fault_list = { 0 };
    static u8 g_event_serial = 0;
    u32 bit_sils = l2buff_get_silsbit_by_nodeid(node_id, l2buff_fault_info.bitid);

    fault_list.emu_id = emu_id;
    fault_list.src_id = 1;
    fault_list.bit_id = bit_sils;
    fault_list.node_type = DMS_DEV_TYPE_L2BUF;
    fault_list.sensor_type = l2buff_fault_info.sensor_type;
    fault_list.event_type = l2buff_fault_info.raserrtype;
    fault_list.node_id = node_id;
    fault_list.sub_node_type = 0;
    fault_list.sub_node_id = 0;
    fault_list.event_severity = l2buff_fault_info.event_severity;
    fault_list.event_assertion = DMS_EVENT_TYPE_OCCUR;
    fault_list.event_serial_num = g_event_serial++;

    drvfault_send_safety_info_to_sils(event_num, &fault_list);
}

static void l2buff_safety_proc(u32 *value, u32 num, const struct notify_data *pdata, u8 emu_id)
{
    hisi_common_error_info *perrorinfo = (hisi_common_error_info *)pdata->origin_data;
    u32 indx = 0, i;
    for (i = 0; i < num; i++) {
        if (value[i] != 0) {
            if (i != 0) {
                indx = i + 1;
            }
            /* indx is bit num, bits 0,2,3,4 are vaild */
            dfm_add_event(&l2buff_dms[pdata->chip_id], perrorinfo->device_or_core_id,
                          l2buff_converge_bs9sx1a(L2BUFF_SECTION_TYPE_SAFETY, indx));
            /* i is array index */
            l2buff_report_safety_fault_to_sils(emu_id, perrorinfo->device_or_core_id,
                                               l2buff_safety_faultinfo_array[i]);
        }
    }
}

static void l2buff_notify_sensor_scan_immediately(unsigned int dev_id, struct dfm_node *dfm_node)
{
    u32 i;
    u32 sensor_num = sizeof(l2buff_sensor_cfg) / sizeof(l2buff_sensor_cfg[0]);
    for (i = 0; i < sensor_num; i++) {
        if (dms_sensor_event_notify(dev_id, &dfm_node->sensors[i].dms_sensor) != 0) {
            TSDRV_PRINT_ERR("Notify sensor scan event failed. (dev_id=%u, sensor_type=%u)\n",
                            dev_id, l2buff_sensor_cfg[i].sensor_type);
        }
    }
}

STATIC void l2buff_fpdc_callback_func(const struct notify_data *pdata)
{
    hisi_common_error_info *perrorinfo = NULL;
    u8 node_id;
    u32 rasvalue;
    u32 value[4];
    u8 emu_id;
    u32 i = 0;
    if (pdata == NULL) {
        TSDRV_PRINT_ERR("Invalid pdata.\n");
        return;
    } else {
        perrorinfo = (hisi_common_error_info *)pdata->origin_data;
        if (perrorinfo == NULL) {
            TSDRV_PRINT_ERR("Invalid origin_data.\n");
            return;
        }
    }
    if (pdata->node_type != DMS_DEV_TYPE_L2BUF || pdata->chip_id > fault_ctrl_get_num()) {
        TSDRV_PRINT_ERR("Invalid para. (node type=0x%x; chip_id=%u)\n", pdata->node_type, pdata->chip_id);
        return;
    }
    if (perrorinfo->module_id != HISI_MODULE_L2BUFFER) {
        TSDRV_PRINT_ERR("Invalid module id. (moduleid=0x%x)\n", perrorinfo->module_id);
        return;
    }
    if (perrorinfo->register_array_size < (sizeof(u32) * L2BUFF_REGISTER_ARRAY_SIZE)) {
        TSDRV_PRINT_ERR("Invalid array size. (size=%u)\n", perrorinfo->register_array_size);
        return;
    }
    if (perrorinfo->device_or_core_id > L2BUFF_SUBNODE_ID_MAX) {
        TSDRV_PRINT_ERR("Invalid device or core id. (id=%u)\n", perrorinfo->device_or_core_id);
        return;
    }
    node_id = perrorinfo->device_or_core_id;
    emu_id = irqnum2emuidarray[perrorinfo->device_or_core_id].emuid;

    /* RAS fault : register address 0x2010, bits 8~15 are vaild */
    rasvalue = (perrorinfo->register_array[L2BUFF_RAS_ERR_STATUSL_INDX] & L2BUFF_RAS_BIT_MASK) >> BIT_NUM_8;
    if (rasvalue >= L2BUFF_RAS_ILLEGAL_SECURITY_ATTRIBUTE_ERR && rasvalue <= L2BUFF_RAS_ATOMIC_CALCULATION_ERR) {
        dfm_add_event(&l2buff_dms[pdata->chip_id], node_id, l2buff_converge_bs9sx1a(L2BUFF_SECTION_TYPE_RAS, rasvalue));
    }
    TSDRV_PRINT_INFO("L2BUFF ras info. (moduleid=0x%x, nodeid=0x%x, emuid=%u, rasvalue=%u)\n",
                     perrorinfo->module_id, node_id, emu_id, rasvalue);

    /* Safety fault : register address 0x0824, int, bits 0 are vaild */
    value[i++] = (perrorinfo->register_array[L2BUFF_SAFETY_RINT_INDX] & L2BUFF_SAFETY_BIT0_MASK);

    /* Safety fault : register address 0x0834, err, bits 2,3,4 are vaild */
    value[i++] = (perrorinfo->register_array[L2BUFF_SAFETY_ERR_RINT_INDX] & L2BUFF_SAFETY_BIT2_MASK) >> BIT_NUM_2;
    value[i++] = (perrorinfo->register_array[L2BUFF_SAFETY_ERR_RINT_INDX] & L2BUFF_SAFETY_BIT3_MASK) >> BIT_NUM_3;
    value[i++] = (perrorinfo->register_array[L2BUFF_SAFETY_ERR_RINT_INDX] & L2BUFF_SAFETY_BIT4_MASK) >> BIT_NUM_4;
    TSDRV_PRINT_INFO("L2BUFF safety info. (moduleid=0x%x, nodeid=0x%x, emuid=%u, "
                     "IntBit0=%u, ErrBit2=%u, ErrBit3=%u, ErrBit4=%u, "
                     "register_array[13]=0x%x, register_array[12]=0x%x)\n",
                     perrorinfo->module_id, node_id, emu_id, value[0], value[1], value[BIT_NUM_2], value[BIT_NUM_3],
                     perrorinfo->register_array[L2BUFF_SAFETY_ERR_RINT_INDX],
                     perrorinfo->register_array[L2BUFF_SAFETY_RINT_INDX]);
    l2buff_safety_proc(value, i, pdata, emu_id);
    l2buff_notify_sensor_scan_immediately(pdata->chip_id, &l2buff_dms[pdata->chip_id].dev_nodes[node_id]);
}
#endif

int tsdrv_register_l2buff_dms_node(u32 dev_id)
{
    u32 sensor_num = sizeof(l2buff_sensor_cfg) / sizeof(l2buff_sensor_cfg[0]);
    u32 i, sensor_idx;

    l2buff_dms[dev_id].dev_id = dev_id;
    if (dfm_struct_init(&l2buff_dms[dev_id], L2BUFF_NUM, sensor_num) != 0) {
        TSDRV_PRINT_ERR("init l2buff's dfm data failed\n");
        goto _fail;
    }

    for (i = 0; i < L2BUFF_NUM; ++i) {
        l2buff_dms[dev_id].dev_nodes[i].node = &g_l2buff_dms_nodes[dev_id][i];
        l2buff_dms[dev_id].dev_nodes[i].post_proc = NULL;
        l2buff_dms[dev_id].dev_nodes[i].coverage_version = EVENT_COVERAGE_V1;
        if (tsdrv_get_chip_type() == CHIP_TYPE_MDC_BS9SX1A) {
#ifdef CFG_SOC_PLATFORM_MDC_V51
            l2buff_dms[dev_id].dev_nodes[i].fpdc_notify = l2buff_fpdc_callback_func;
#endif
        } else {
            l2buff_dms[dev_id].dev_nodes[i].fpdc_notify = l2buff_ras_fpdc_handler;
        }
#ifdef CFG_SOC_PLATFORM_MDC_V51
        l2buff_dms[dev_id].dev_nodes[i].get_converage_node = l2buff_safety_converge;
#endif
        for (sensor_idx = 0; sensor_idx < sensor_num; ++sensor_idx) {
            l2buff_sensor_cfg[sensor_idx].scan_module = DMS_SERSOR_SCAN_NOTIFY;
            l2buff_sensor_cfg[sensor_idx].private_data =
                SENSOR_PRIV_DATA(dev_id, 0, i, l2buff_sensor_cfg[sensor_idx].sensor_type);
            l2buff_dms[dev_id].dev_nodes[i].sensors[sensor_idx].dms_sensor = l2buff_sensor_cfg[sensor_idx];
        }
    }

    if (dfm_register_nodes(&l2buff_dms[dev_id]) != 0) {
        TSDRV_PRINT_ERR("register l2buff dms node failed\n");
        goto _fail;
    }
    return 0;

_fail:
    dfm_struct_final(&l2buff_dms[dev_id], L2BUFF_NUM, sensor_num);
    return -EFAULT;
}

void tsdrv_unregister_l2buff_dms_node(u32 dev_id)
{
    dfm_unregister_nodes(&l2buff_dms[dev_id]);
    dfm_struct_final(&l2buff_dms[dev_id], L2BUFF_NUM, 1);
}
