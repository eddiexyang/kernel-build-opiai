/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
 * Create: 2023-05-17
 */

#ifndef SAFETY_RAS_MATA_CONFIG_310_H
#define SAFETY_RAS_MATA_CONFIG_310_H

#include <linux/types.h>
#include "dms_define.h"
#include "dms_sensor.h"
#include "dms_sensor_type.h"
#include "safety_ras_mata_310.h"
#include "safety_ras_config_310.h"
#include "safety_ras_init_310.h"
#include "fpdc.h"

#define MATA_AST_MASK                    0xFFF    /* DMS_SEN_TYPE_RAS_SENSOR assert_event_mask */
#define MATA_DST_MASK                    0xFBF    /* DMS_SEN_TYPE_RAS_SENSOR deassert_event_mask */

/* device node */
enum mata_dev_node {
	MATA_NODE_0 = 0,
	MATA_NODE_ID_MAX
};

enum mata_sensor_id {
	MATA_SENSOR_DDRC = 0,
	MATA_SENSOR_ID_MAX
};

STATIC struct dms_node_operations g_mata_ops = {
	.init = mata_ops_init,
	.uninit = mata_ops_exit,
	.scan = NULL,
	.fault_diag = NULL,
	.event_notify = NULL,
	.get_link_state = NULL,
	.set_link_state = NULL
};

#define MATA_EVENT_SCAN_FUNC mata_fault_event_scan

__attribute__((unused)) STATIC struct dms_node g_mata_dev_node_table[MATA_NODE_ID_MAX][MATA_SENSOR_ID_MAX] = {
	{ /* dev_node 0 */
		memory_node_def(DMS_DEV_TYPE_DDRA, "mata_0", MATA_NODE_0, MATA_SENSOR_DDRC, g_mata_ops),
	}
};

/* mata sensor table */
__attribute__((unused)) STATIC struct dms_sensor_object_cfg g_mata_sensor_table[MATA_NODE_ID_MAX][MATA_SENSOR_ID_MAX]
	= {
	{ /* dev_node 0 */
		memory_sensor_def(DMS_SEN_TYPE_RAS_SENSOR, "mata_0_0", 0UL, MATA_SENSOR_DDRC, 0UL, MATA_AST_MASK, MATA_DST_MASK,
			DMS_SENSOR_SCAN_INTERVAL, MATA_EVENT_SCAN_FUNC)
	}
};

/* MATA ERR CODE */
#define MATA_RAS_ILLEGAL_REQUEST                            0x010E
#define MATA_RAS_ECC_MULTI_BIT_INGRESS_BUF                  0x0202
#define MATA_RAS_ECC_MULTI_BIT_SDIR                         0x0307
#define MATA_RAS_DEFAULT_SLAVE                              0x050E
#define MATA_RAS_ACCESS_FROM_NON_SECURE_ZONE                0x060E
#define MATA_RAS_ECC_MULTI_BIT_DATA_BUF                     0x0702
#define MATA_RAS_RESPONSE_ERROR_FROM_DDRC                   0x0812
#define MATA_RAS_MISS_MSD                                   0x090D
#define MATA_RAS_OVERLAP_MSD                                0x0A0D
#define MATA_RAS_INVERT_MSD                                 0x0B0D
#define MATA_RAS_ACCESS_FROM_SECURE_ZONE                    0x0C0E
#define MATA_RAS_ECC_SINGLE_BIT_INGRESS_BUF                 0x0E02
#define MATA_RAS_ECC_SINGLE_BIT_SDIR                        0x0F07
#define MATA_RAS_ECC_SINGLE_BIT_DATA_BUF                    0x1102
#define MATA_RAS_ECC_MULTI_BIT_RDQ                          0x1302
#define MATA_RAS_ECC_SINGLE_BIT_RDQ                         0x1402
#define MATA_RAS_PQ_TIMEOUT                                 0x1514
#define MATA_RAS_ADDR_OVERFLOW                              0x160D
#define MATA_RAS_ECC_MULTI_BIT_SHARE_BUF                    0x1902
#define MATA_RAS_ECC_SINGLE_BIT_SHARE_BUF                   0x1A02

__attribute__((unused)) STATIC struct ras_fault_converge_item g_mata_fault_converge_table[] = {
	/* MATA */
	DMS_FAULT_CONVERGE_CONFIG(DMS_DEV_TYPE_DDRA, MATA_SENSOR_DDRC, RAS_SEC_OEM, MATA_RAS_ILLEGAL_REQUEST,
		DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR, "illegal request Opcode")
	DMS_FAULT_CONVERGE_CONFIG(DMS_DEV_TYPE_DDRA, MATA_SENSOR_DDRC, RAS_SEC_OEM, MATA_RAS_ECC_MULTI_BIT_INGRESS_BUF,
		DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_MBECC, "ingress buffer multi ecc")
	DMS_FAULT_CONVERGE_CONFIG(DMS_DEV_TYPE_DDRA, MATA_SENSOR_DDRC, RAS_SEC_OEM, MATA_RAS_ECC_MULTI_BIT_SDIR,
		DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_MBECC, "S-Dir multi ecc")
	DMS_FAULT_CONVERGE_CONFIG(DMS_DEV_TYPE_DDRA, MATA_SENSOR_DDRC, RAS_SEC_OEM, MATA_RAS_DEFAULT_SLAVE,
		DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR, "Default slave")
	DMS_FAULT_CONVERGE_CONFIG(DMS_DEV_TYPE_DDRA, MATA_SENSOR_DDRC, RAS_SEC_OEM, MATA_RAS_ACCESS_FROM_NON_SECURE_ZONE,
		DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR, "Access from Non-Security zone")
	DMS_FAULT_CONVERGE_CONFIG(DMS_DEV_TYPE_DDRA, MATA_SENSOR_DDRC, RAS_SEC_OEM, MATA_RAS_ECC_MULTI_BIT_DATA_BUF,
		DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_MBECC, "data buff multi ecc")
	DMS_FAULT_CONVERGE_CONFIG(DMS_DEV_TYPE_DDRA, MATA_SENSOR_DDRC, RAS_SEC_OEM, MATA_RAS_RESPONSE_ERROR_FROM_DDRC,
		DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_ERROR, "Response Error From DDRC")
	DMS_FAULT_CONVERGE_CONFIG(DMS_DEV_TYPE_DDRA, MATA_SENSOR_DDRC, RAS_SEC_OEM, MATA_RAS_MISS_MSD,
		DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR, "MSD miss error")
	DMS_FAULT_CONVERGE_CONFIG(DMS_DEV_TYPE_DDRA, MATA_SENSOR_DDRC, RAS_SEC_OEM, MATA_RAS_OVERLAP_MSD,
		DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR, "MSD overlap error")
	DMS_FAULT_CONVERGE_CONFIG(DMS_DEV_TYPE_DDRA, MATA_SENSOR_DDRC, RAS_SEC_OEM, MATA_RAS_INVERT_MSD,
		DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR, "MSD invert error")
	DMS_FAULT_CONVERGE_CONFIG(DMS_DEV_TYPE_DDRA, MATA_SENSOR_DDRC, RAS_SEC_OEM, MATA_RAS_ACCESS_FROM_SECURE_ZONE,
		DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR, "Access from Security zone")
	DMS_FAULT_CONVERGE_CONFIG(DMS_DEV_TYPE_DDRA, MATA_SENSOR_DDRC, RAS_SEC_OEM, MATA_RAS_ECC_SINGLE_BIT_INGRESS_BUF,
		DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_SBECCOverThold, "ingress buffer single ecc")
	DMS_FAULT_CONVERGE_CONFIG(DMS_DEV_TYPE_DDRA, MATA_SENSOR_DDRC, RAS_SEC_OEM, MATA_RAS_ECC_SINGLE_BIT_SDIR,
		DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_SBECCOverThold, "S-Dir single ecc")
	DMS_FAULT_CONVERGE_CONFIG(DMS_DEV_TYPE_DDRA, MATA_SENSOR_DDRC, RAS_SEC_OEM, MATA_RAS_ECC_SINGLE_BIT_DATA_BUF,
		DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_SBECCOverThold, "data buffer single ecc")
	DMS_FAULT_CONVERGE_CONFIG(DMS_DEV_TYPE_DDRA, MATA_SENSOR_DDRC, RAS_SEC_OEM, MATA_RAS_ECC_MULTI_BIT_RDQ,
		DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_MBECC, "reqdat buffer multi ecc")
	DMS_FAULT_CONVERGE_CONFIG(DMS_DEV_TYPE_DDRA, MATA_SENSOR_DDRC, RAS_SEC_OEM, MATA_RAS_ECC_SINGLE_BIT_RDQ,
		DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_SBECCOverThold, "reqdat buffer single ecc")
	DMS_FAULT_CONVERGE_CONFIG(DMS_DEV_TYPE_DDRA, MATA_SENSOR_DDRC, RAS_SEC_OEM, MATA_RAS_PQ_TIMEOUT,
		DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_TIMEOUT_ERR, "pq timeout error")
	DMS_FAULT_CONVERGE_CONFIG(DMS_DEV_TYPE_DDRA, MATA_SENSOR_DDRC, RAS_SEC_OEM, MATA_RAS_ADDR_OVERFLOW,
		DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_IN_CFG_ERR, "addr_overflow error")
	DMS_FAULT_CONVERGE_CONFIG(DMS_DEV_TYPE_DDRA, MATA_SENSOR_DDRC, RAS_SEC_OEM, MATA_RAS_ECC_MULTI_BIT_SHARE_BUF,
		DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_MBECC, "share buffer multi ecc")
	DMS_FAULT_CONVERGE_CONFIG(DMS_DEV_TYPE_DDRA, MATA_SENSOR_DDRC, RAS_SEC_OEM, MATA_RAS_ECC_SINGLE_BIT_SHARE_BUF,
		DMS_SEN_TYPE_RAS_SENSOR, RAS_ERROR_TYPE_SBECCOverThold, "share buffer single ecc")
};

#endif