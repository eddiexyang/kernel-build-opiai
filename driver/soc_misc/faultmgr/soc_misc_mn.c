/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 * Description:
 * Author: huawei
 * Create: 2021-11-20
 */
#include "soc_misc_err_info.h"
#include "soc_misc_init.h"
#include "soc_misc_mn.h"
#include "soc_misc_safety.h"
#include "dms_node_type.h"
#include "soc_misc_fault.h"

#include "drvfault_user_common.h"

 /* MN RAS IERR TYPE */
enum mn_ras_ierr {
    MN_SAFETY_RX_PARITY = 0,
    MN_SAFETY_TXREQ_COMPARE,
    MN_SAFETY_REG_PARITY,
    MN_SAFETY_SPLIT_CHK,
    MN_SAFETY_MERGE_CHK,
    MN_SAFETY_TIMEOUT_R,
};

#define MN_SAFETY_COVERGE(ierr, msg, _sensor_type, ras_err) { \
    .subsys_id = DMS_DEV_TYPE_MN,                 \
    .module_id = SOC_MISC_SENSOR_MN,              \
    .section_type = SOC_ERR_SECTOR_SAFETY,        \
    .ras_code.int_status = ierr,                  \
    .describe = msg,                              \
    .sensor_type = _sensor_type,                  \
    .error_type = ras_err,                        \
}

STATIC struct ras_fault_converge_item g_soc_misc_mn_converge_table[] = {
    MN_SAFETY_COVERGE(MN_SAFETY_RX_PARITY, "CRC check fail", DMS_SEN_TYPE_CHECK_SENSOR, RAS_ERROR_TYPE_ERROR_NF),
    MN_SAFETY_COVERGE(MN_SAFETY_TXREQ_COMPARE, "lockstep error", DMS_SEN_TYPE_SAFETY_SENSOR, SOC_SAFETY_LOCKSTEP_ERR),
    MN_SAFETY_COVERGE(MN_SAFETY_REG_PARITY, "parity error", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_PARITY),
    MN_SAFETY_COVERGE(MN_SAFETY_SPLIT_CHK, "module error", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_ERROR),
    MN_SAFETY_COVERGE(MN_SAFETY_MERGE_CHK, "module error", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_ERROR),
    MN_SAFETY_COVERGE(MN_SAFETY_TIMEOUT_R, "service timeout", DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_TIMEOUT_ERR),
};

/****************************************************************
* support SPI safety irq of MN in BS9SX1A
***************************************************************/
#define MN_DFX_REGS_NUM (11)
static u32 g_mn_dfx_regs[MN_DFX_REGS_NUM] = {
    0x0000, 0x0004, 0x0820, 0x0824, 0x082C, 0x0830, 0x0834, 0x083C, 0x0840, 0x0844, 0x0848
};
#define MN_SAFETY_ERR_HWINFO(_emu_id, _reg_base, _bit_id) { \
    .emu_id = _emu_id,                        \
    .src_id = 1,                              \
    .bit_id = _bit_id,                        \
    .base_paddr = _reg_base,                  \
    .base_size = SZ_64K,                      \
    .base_vaddr = 0,                          \
    .status = {MN_SAFETY_ERR_SRC, 0x3F},      \
    .clear = {MN_SAFETY_ERR_CLR, 0x3F},       \
    .mask = {MN_SAFETY_ERR_INT_MASK, 0x3F},   \
    .check_mask[0] = {DRVFAULT_FILED_VALID, 0x0848, 0xFFFFFFC0}, \
    .dfx_reg_num = MN_DFX_REGS_NUM,          \
    .dfx_reg_list = g_mn_dfx_regs,            \
}

#define MN_SAFETY_INT_HWINFO(_emu_id, _reg_base, _bit_id) { \
    .emu_id = _emu_id,                        \
    .src_id = 1,                              \
    .bit_id = _bit_id,                        \
    .base_paddr = _reg_base,                  \
    .base_size = SZ_64K,                      \
    .base_vaddr = 0,                          \
    .status = {MN_SAFETY_SRC, 0x0 },          \
    .clear = {MN_SAFETY_CLR, 0x0 },           \
    .mask = {MN_SAFETY_INT_MASK, 0x0 },       \
    .check_mask[0] = {DRVFAULT_FILED_VALID, 0x0848, 0xFFFFFFC0}, \
    .dfx_reg_num = MN_DFX_REGS_NUM,          \
    .dfx_reg_list = g_mn_dfx_regs,            \
}

STATIC struct safety_irq_hw_info g_mn_safety_hwinfo[MN_SUBSYS_NUM][MN_SRC_NUM] = {
    { /* AO SUB */
        MN_SAFETY_INT_HWINFO(AO_SUB_EMU_ID, AO_MN_REG_BASE, AO_MN_INT_SAFETY_BIT_ID),
        MN_SAFETY_ERR_HWINFO(AO_SUB_EMU_ID, AO_MN_REG_BASE, AO_MN_ERR_SAFETY_BIT_ID),
    }, { /* IO SUB */
        MN_SAFETY_INT_HWINFO(IO_SUB_EMU_ID, IO_MN_REG_BASE, IO_MN_INT_SAFETY_BIT_ID),
        MN_SAFETY_ERR_HWINFO(IO_SUB_EMU_ID, IO_MN_REG_BASE, IO_MN_ERR_SAFETY_BIT_ID),
    }
};

#ifndef CFG_SOC_PLATFORM_MDC_LITE
#define MN_SAFETY_INTERRUPT_HWINFO(_id) {           \
    .dev_id = 0,                                         \
    .irq_type = SAFETY_IRQ_TYPE_SPI,                     \
    .irq_name = "int_fhi_mn" NUM_TO_STR(_id),    \
    .safety_irq_func = soc_misc_subsys_irq_handler,      \
    .irq_hwinfo_num = 2,                                 \
    .irq_hwinfo_list = g_mn_safety_hwinfo[_id]  \
}
#else
#define MN_SAFETY_INTERRUPT_HWINFO(_id) {           \
    .dev_id = 0,                                         \
    .irq_type = SAFETY_IRQ_TYPE_SPI,                     \
    .irq_name = "int_cpu_safety_mn" NUM_TO_STR(_id),    \
    .safety_irq_func = soc_misc_subsys_irq_handler,      \
    .irq_hwinfo_num = 2,                                 \
    .irq_hwinfo_list = g_mn_safety_hwinfo[_id]  \
}
#endif

struct safety_irq_info g_int_fhi_mn[MN_SUBSYS_NUM] = {
    MN_SAFETY_INTERRUPT_HWINFO(0),   /* irq int_fhi_mn0 */
    MN_SAFETY_INTERRUPT_HWINFO(1),   /* irq int_fhi_mn1 */
};

const struct ras_fault_converge_item *soc_misc_get_mn_converge_item(const struct ras_error *error_info)
{
    u32 num = sizeof(g_soc_misc_mn_converge_table) / sizeof(struct ras_fault_converge_item);
    return soc_misc_parse_table_handle(error_info, g_soc_misc_mn_converge_table, num);
}

int soc_misc_mn_ops_init(struct dms_node *device)
{
    soc_misc_drv_debug("DEBUG (node_name=%.*s)\n", DMS_MAX_DEV_NAME_LEN, device->node_name);
    return 0;
}

void soc_misc_mn_ops_uninit(struct dms_node *device)
{
    soc_misc_drv_debug("DEBUG (node_name=%.*s)\n", DMS_MAX_DEV_NAME_LEN, device->node_name);
    return;
}

void soc_misc_mn_fault_handler(const struct notify_data *pdata)
{
    soc_misc_drv_warn("mn not have arm ras fault event.\n");
}

int soc_misc_mn_sensor_scan(unsigned long long private_data, struct dms_sensor_event_data *data)
{
    int ret;

    ret = soc_misc_fault_event_scan(private_data, data, SOC_MISC_SENSOR_MN);
    if (ret) {
        soc_misc_drv_err("Scan CPUcore sensor failed. (ret=%d)\n", ret);
        return ret;
    }

    return 0;
}
