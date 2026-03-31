/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
 * Create: 2022-10-27
 */
#ifndef SAFETY_RAS_DHA_610_H
#define SAFETY_RAS_DHA_610_H

#include <linux/types.h>
#include <linux/platform_device.h>

#include "dfm_safety_report.h"
#include "dfm_dev_register.h"

/* DHA SAFETY故障相关 */
enum dha_safety_err_bit {
	DHA_RAS_SKYROS_SIGNALS_INTR  = 0,
	DHA_RAS_SKYROS_PAYLAOD_INTR  = 1,
	DHA_RAS_INGRESS_INTR         = 2,
	DHA_RAS_PQBANK_MIRROR_INTR   = 3,
	DHA_RAS_PQBANK_LOCKSTEP_IBTR = 4,
	DHA_RAS_PQBANK_TIMEOUT_INTR  = 5,
	DHA_RAS_MEMORY_ADDR_INTR     = 6,
	DHA_RAS_DMC_CRC_INTR         = 7,
	DHA_RAS_DMC_CNT_INTR         = 8,
	DHA_RAS_SDMAA_ISO_INTR       = 9,
	DHA_RAS_SDMAA_LOCKSTEP_INTR  = 10,
	DHA_RAS_SDMAA_CRC_CNT_INTR   = 11,
	DHA_RAS_CBUF_LOCKSTEP_INTR   = 12,
	DHA_RAS_SECC_INTR            = 13,
	DHA_RAS_MECC_INTR            = 14,
	DHA_RAS_REG_INTR             = 15
};

enum ddra_dev_node {
	DDRA_NODE_0 = 0U,
	DDRA_NODE_ID_MAX
};

#ifdef CFG_SOC_PLATFORM_MDC_V51LITE
#define DHA_RIGHT_CH_PADDR                     0x81120000U
#define DHA_LEFT_CH_PADDR                      0x81110000U
#define DHA_SAFETY_ERR_CLEAR_REG_OFFSET        0x082CU
#define DHA_RAS_SAFETY_CLEAR_REG_OFFSET        0x082CU

#else
#define DHA_RIGHT_CH_PADDR                     0x81110000U
#define DHA_LEFT_CH_PADDR                      0x81120000U
#define DHA_SAFETY_ERR_CLEAR_REG_OFFSET        0x081CU
#define DHA_RAS_SAFETY_CLEAR_REG_OFFSET        0x0840U
#endif

#define DDR_DHA_RAS_BASE_ADDR_LEN              0x10000U

#define DHA_SAFETY_ERR_STATUS_REG_OFFSET       0x0810U
#define DHA_SAFETY_ERR_MASK_REG_OFFSET         0x0814U

#define DHA_SAFETY_STATUS_REG_OFFSET           0x0820U
#define DHA_SAFETY_MASK_REG_OFFSET             0x0824U
#define DHA_SAFETY_CLEAR_REG_OFFSET            0x082CU

#define DHA_RAS_SAFETY_STATUS_REG_OFFSET       0x0834U
#define DHA_RAS_SAFETY_MASK_REG_OFFSET         0x0838U

#define DHA_SAFETY_MAX_NUM                     16U

#define DEV_NODE_OFFSET                        16U
#define DEV_NODE_MASK                          0xFFFFU
#define SENSOR_NODE_OFFSET                     0x0U
#define SENSOR_NODE_MASK                       0xFFFFU
#define sensor_priv_data(_node_id, _sensor_type) \
	(((uint64_t)((_node_id) & DEV_NODE_MASK) << DEV_NODE_OFFSET) | \
		((uint64_t)((_sensor_type) & SENSOR_NODE_MASK) << SENSOR_NODE_OFFSET))

#define ddra_dha_dms_node_define(_id, _ops) { \
	.node_type = DMS_DEV_TYPE_DDRA,           \
	.node_id = (_id),                           \
	.node_name = "ddra-dha-" #_id,            \
	.capacity = 0x1,                          \
	.permission = 0x1,                        \
	.owner_devid = 0,                         \
	.ops = (_ops)                               \
}

#define ddra_dha_sensor_obj(_type, _name, _class, _attr, _debounce, _interval, \
	_proc, _enable, _func, _data, _assert, _deassert) {                        \
	.sensor_type = (_type),              \
	.sensor_name = (_name),              \
	.sensor_class = (_class),            \
	.sensor_class_cfg = {              \
		.discrete_sensor = {           \
			.attribute = (_attr),        \
			.debounce_time = (_debounce) \
		}                              \
	},                                 \
	.scan_interval = (_interval),        \
	.proc_flag = (_proc),                \
	.enable_flag =  (_enable),           \
	.pf_scan_func = (_func),             \
	.private_data = (_data),             \
	.assert_event_mask = (_assert),      \
	.deassert_event_mask = (_deassert)   \
}

#define dha_safety_coverage_item(_err_status, _describe, _sensor_type, _err_type) { \
	.subsys_id = DMS_DEV_TYPE_DDRA,         \
	.module_id = 0,                         \
	.section_type = 0,                      \
	.ras_code.err_status = (_err_status),     \
	.describe = (_describe),                  \
	.sensor_type = (_sensor_type),            \
	.error_type = (_err_type)                 \
}

#define ddra_dha_interrupt_ras_safety_hwinfo(_reg_base, _status_offset, _clear_offset, _mask_offset) {         \
	.base_paddr = _reg_base,                                     /* interrupt reg base addr */                 \
	.base_size = 0x10000U,                                         /* interrupt reg base size */               \
	.status = {_status_offset, 0xffff},                          /* interrupt status reg */                    \
	.clear = {_clear_offset, 0xffff},                            /* interrupt clear reg */                     \
	.mask = {_mask_offset, 0xffff},                              /* interrupt mask reg */                      \
}

/* DHA SAFETY故障相关 */

/* DHA RAS故障相关 */

enum dha_ras_err_bit {
	DHA_RAS_ERR_REQ_MIRREQERR = 5,
	DHA_RAS_ERR_REQ_DMCADDRERR = 6,
	DHA_RAS_ERR_REQ_MEEVOIDERR = 7,
	DHA_RAS_ERR_REQ_DEFAULT = 8,
	DHA_RAS_ERR_REQ_NONSECERR = 9,
	DHA_RAS_ERR_REQ_SECERR = 10,
	DHA_RAS_ERR_REQ_ERRREQ = 11,
	DHA_RAS_ERR_REQ_MSDMISS = 13,
	DHA_RAS_ERR_REQ_MSDOVERLAP = 14,
	DHA_RAS_ERR_REQ_MSDINVERT = 15,
	DHA_RAS_ERR_REQ_MSD_CCIX_HIT = 16
};

#define BS9SX1A_AO_CFGBUS_RING_BASE            0x81000000U
#define CFGBUS_ID_DHA0                         0x11U

#define DHA_ERR_ADDRL_REG_OFFSET               0x2018U
#define DHA_ERR_ADDRH_REG_OFFSET               0x201cU
#define DHA_ERR_MISC0L_STATUS_REG_OFFSET       0x2020U
#define DHA_ERR_MISC0H_STATUS_REG_OFFSET       0x2024U
#define DHA_ERR_MISC1L_STATUS_REG_OFFSET       0x2028U
#define DHA_ERR_MISC1H_STATUS_REG_OFFSET       0x202cU  /* 2028低位错误记录寄存器，202c高位，读202c时必须先读2028 */

#define DHA_ERR_CTRLL_REG_OFFSET               0x2008U
#define DHA_ERR_STATUSL_REG_OFFSET             0x2010U

#define DHA_RAS_MASK_MISC1H_REG_OFFSET         0x1048U
#define DHA_RAS_MASK_MISC0L_REG_OFFSET         0x104cU

#define RAS_DDRA_DHA_INT_NUM                   2U
#define RAS_DDRA_DHA_NUM                       2U

#define DHA_RAS_MAX_NUM      11U

#define ddra_dha_interrupt_int_hwinfo(_reg_base, _bit_id, _status_offset, _clear_offset, _mask_offset) {       \
	.emu_id = AO_SUB_EMU_ID,                                                                                   \
	.src_id = 1,                                                 /* value equal to:1 + emu_irq_number / 32  */ \
	.bit_id = _bit_id,                                           /* value equal to:emu_irq_number % 32 */      \
	.base_paddr = _reg_base,                                     /* interrupt reg base addr */                 \
	.base_size = 0x10000U,                                       /* interrupt reg base size */                 \
	.status = {_status_offset, 0x2200},                          /* interrupt status reg */                    \
	.clear = {_clear_offset, 0x2200},                            /* interrupt clear reg */                     \
	.mask = {_mask_offset, 0x2200},                              /* interrupt mask reg */                      \
}

#define ddra_dha_interrupt_err_hwinfo(_reg_base, _bit_id, _status_offset, _clear_offset, _mask_offset) {       \
	.emu_id = AO_SUB_EMU_ID,                                                                                   \
	.src_id = 1,                                                 /* value equal to:1 + emu_irq_number / 32  */ \
	.bit_id = _bit_id,                                           /* value equal to:emu_irq_number % 32 */      \
	.base_paddr = _reg_base,                                     /* interrupt reg base addr */                 \
	.base_size = 0x10000U,                                         /* interrupt reg base size */               \
	.status = {_status_offset, 0xddff},                          /* interrupt status reg */                    \
	.clear = {_clear_offset, 0xddff},                            /* interrupt clear reg */                     \
	.mask = {_mask_offset, 0xddff},                              /* interrupt mask reg */                      \
}

#define ddra_dha_interrupt_info(_irq_name, _id) {     \
	.dev_id = 0,                                      \
	.irq_type = SAFETY_IRQ_TYPE_SPI,                  \
	.irq_name = _irq_name,                            \
	.safety_irq_func = ddra_dha_safety_irq_handler,   \
	.irq_hwinfo_num = 3,                              \
	.irq_hwinfo_list = g_safety_ddra_dha_hwinfo[_id]  \
}

/* 清除错误记录信息寄存器低位&高位 */
#define ras_misc1h_ddra_dha_hw_info(_reg_base) {                                          \
	.base_paddr = _reg_base,                                                              \
	.base_size = 0x10000U,                                                                \
	.status[0] = { DHA_ERR_MISC1H_STATUS_REG_OFFSET, 4, 0x1efe0U, 0 },                    \
	.clear[0] = { DHA_ERR_STATUSL_REG_OFFSET, 4, 0, 0xffffffffU },                   \
	.check_mask[0] = { DHA_RAS_MASK_MISC1H_REG_OFFSET, 4, 0xf7bdeU, 0 },                  \
	.check_mask[1] = { DHA_RAS_MASK_MISC0L_REG_OFFSET, 4, 0xf0ff000U, 0 },                \
}

#define ras_eri_dha_interrupt_info(_irq_name, _id) {    \
	.dev_id = 0,                                        \
	.irq_type = RAS_IRQ_TYPE_SPI,                       \
	.irq_name = _irq_name,                              \
	.safety_irq_func = ddra_dha_ras_irq_handler,        \
	.irq_hwinfo_num = 1,                                \
	.irq_hwinfo_list = &g_ras_eri_ddra_dha_hwinfo[_id]  \
}

#define ras_fhi_dha_interrupt_info(_irq_name, _id) {    \
	.dev_id = 0,                                        \
	.irq_type = RAS_IRQ_TYPE_SPI,                       \
	.irq_name = _irq_name,                              \
	.safety_irq_func = ddra_dha_ras_irq_handler,        \
	.irq_hwinfo_num = 1,                                \
	.irq_hwinfo_list = &g_ras_fhi_ddra_dha_hwinfo[_id]  \
}

/* DHA RAS故障相关 */

int32_t ddra_dha_register_safety_irq(void);
int32_t ddra_dha_unregister_safety_irq(void);
const struct ras_fault_converge_item *ddra_dha_safety_coverage(uint32_t section_type, unsigned long long ras_code);
const struct ras_fault_converge_item *ddra_safety_coverge(const struct ras_fault_converge_item *items, \
	uint32_t item_num, unsigned long long ras_code);
void ddra_dha_ras_dfx(void);
int ddra_dha_safety_irq_handler(struct safety_fault_info *safety_fault,
	unsigned int *event_num, struct safety_event **event_list);
int ddra_dha_ras_irq_handler(struct safety_fault_info *safety_fault,
	unsigned int *event_num, struct safety_event **event_list);

extern struct platform_driver g_dha_ras_driver;
extern struct dfm_struct ddra_dha_dms;
 #endif